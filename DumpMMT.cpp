#include "StdAfx.h"
#include <conio.h>
#include <ctype.h>
#include "AMRingBuffer.h"
#include "MMT.h"

extern std::unordered_map<std::string, std::string> g_params;
extern int g_verbose_level;

int g_TLV_packets_per_display = 20;

int ShowMMTTLVPacks()
{
	int nRet = RET_CODE_SUCCESS;
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string szOutputFile;
	std::string szOutputFmt;
	std::string& szInputFile = g_params["input"];

	if (g_params.find("outputfmt") != g_params.end())
		szOutputFmt = g_params["outputfmt"];

	if (g_params.find("output") != g_params.end())
		szOutputFile = g_params["output"];

	CFileBitstream bs(szInputFile.c_str(), 4096, &nRet);

	if (nRet < 0)
	{
		printf("Failed to open the file: %s.\n", szInputFile.c_str());
		return nRet;
	}

	int nParsedTLVPackets = 0;
	int nIPv4Packets = 0, nIPv6Packets = 0, nHdrCompressedIPPackets = 0, nTransmissionControlSignalPackets = 0, nNullPackets = 0, nOtherPackets = 0;

	try
	{
		uint32_t tlv_hdr = 0;
		uint64_t left_bits = 0;
		bs.Tell(&left_bits);

		while (left_bits >= (4ULL << 3))
		{
			tlv_hdr = (uint32_t)bs.PeekBits(32);
			if (((tlv_hdr >> 24) & 0xFF) != 0x7F)
			{
				printf("[MMT/TLV] TLV header should start with 0x7F at file position: %llu.\n", bs.Tell() >> 3);
				nRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				break;
			}

			MMT::TLVPacket* pTLVPacket = nullptr;
			switch ((tlv_hdr >> 16) & 0xFF)
			{
			case MMT::TLV_IPv4_packet:
				pTLVPacket = new MMT::Undefined_TLVPacket();
				nIPv4Packets++;
				break;
			case MMT::TLV_IPv6_packet:
				pTLVPacket = new MMT::IPv6Packet();
				nIPv6Packets++;
				break;
			case MMT::TLV_Header_compressed_IP_packet:
				pTLVPacket = new MMT::HeaderCompressedIPPacket();
				nHdrCompressedIPPackets++;
				break;
			case MMT::TLV_Transmission_control_signal_packet:
				pTLVPacket = new MMT::Undefined_TLVPacket();
				nTransmissionControlSignalPackets++;
				break;
			case MMT::TLV_Null_packet:
				pTLVPacket = new MMT::TLVNullPacket();
				nNullPackets++;
				break;
			default:
				pTLVPacket = new MMT::Undefined_TLVPacket();
				nOtherPackets++;
			}

			if ((nRet = pTLVPacket->Unpack(bs)) < 0)
			{
				printf("Failed to unpack a TLV packet at file position: %llu.\n", pTLVPacket->start_bitpos >> 3);
				delete pTLVPacket;
				break;
			}

			pTLVPacket->Print();

			if (left_bits < (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3))
			{
				delete pTLVPacket;
				break;
			}

			left_bits -= (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3);
			delete pTLVPacket;

			nParsedTLVPackets++;
			if ((nParsedTLVPackets%g_TLV_packets_per_display) == 0)
			{
				printf("Press any key to continue...\n");
				char chk = _getch();
				if (chk == 0x3 || chk == 0x1A)	// Ctrl + C/Z, quit the loop
					break;
			}
		}
	}
	catch (...)
	{
		nRet = RET_CODE_ERROR;
	}

	printf("The total number of TLV packets: %d.\n", nParsedTLVPackets);
	printf("The number of IPv4 TLV packets: %d.\n", nIPv4Packets);
	printf("The number of IPv6 TLV packets: %d.\n", nIPv6Packets);
	printf("The number of Header Compressed IP packets: %d.\n", nHdrCompressedIPPackets);
	printf("The number of Transmission Control Signal TLV packets: %d.\n", nTransmissionControlSignalPackets);
	printf("The number of Null TLV packets: %d.\n", nNullPackets);
	printf("The number of other TLV packets: %d.\n", nOtherPackets);

	return nRet;
}

// define a PLT/MPT tree for the result
using TreePLTMPT = std::vector<std::tuple<MMT::PackageListTable*, std::vector<MMT::MMTPackageTable*>>>;
using TreeCIDPAMsgs = std::map<uint16_t, TreePLTMPT>;

using TreePkgBuf = std::map<uint16_t, AMLinearRingBuffer>;
using TreeCIDPkgBuf = std::map<uint16_t, TreePkgBuf>;

int ProcessPAMessage(TreeCIDPAMsgs& CIDPAMsgs, uint16_t CID, uint64_t package_id, uint8_t* pMsgBuf, int cbMsgBuf)
{
	if (cbMsgBuf <= 0)
		return RET_CODE_INVALID_PARAMETER;

	CBitstream bs(pMsgBuf, ((size_t)cbMsgBuf<<3));
	uint16_t msg_id = (uint16_t)bs.PeekBits(16);
	
	// If the current message is not a PA message, ignore it.
	if (msg_id != 0)
		return RET_CODE_SUCCESS;

	// Make sure the tree PA message is already there for the specified CID
	auto iter = CIDPAMsgs.find(CID);
	if (iter == CIDPAMsgs.end())
		return RET_CODE_INVALID_PARAMETER;

	int iRet = RET_CODE_SUCCESS;
	MMT::PAMessage PAMsg(cbMsgBuf);
	if ((iRet = PAMsg.Unpack(bs)) < 0)
		return iRet;

	for (auto& t : PAMsg.tables)
	{
		if (t == nullptr)
			continue;

		if (t->table_id == 0x80)	// PLT
		{
			// Check whether there is already this PLT
			MMT::PackageListTable* pPLT = (MMT::PackageListTable*)t;
			if (iter->second.size() > 0)
			{
				auto& tuple_tree = iter->second.back();
				if (pPLT->Compare(std::get<0>(tuple_tree)) == 0)
					continue;
			}

			printf("Found a new PLT with package_id: %lld(0X%llX) in header compressed IP packet with CID: %d(0X%X)...\n", package_id, package_id, CID, CID);
			
			// A new PLT is found
			iter->second.push_back(std::make_tuple(pPLT, std::vector<MMT::MMTPackageTable*>()));
			// reset the PLT to NULL to avoid destructing it during PAMessage
			t = nullptr;
		}
		else if (t->table_id == 0x20) // MPT
		{
			bool bFoundExistedMPT = false, bInPLTMPTs = false;
			MMT::MMTPackageTable* pMPT = (MMT::MMTPackageTable*)t;
			// Check whether the current MPT lies at the last PLT or not
			if (iter->second.size() > 0)
			{
				auto& tuple_tree = iter->second.back();
				auto& vMPTs = std::get<1>(tuple_tree);
				for (auto& m : vMPTs)
				{
					if (m->MMT_package_id == pMPT->MMT_package_id)
					{
						bFoundExistedMPT = true;
						break;
					}
				}

				// Already found the existed MPT, continue the next table processing
				if (bFoundExistedMPT == true)
					continue;

				auto& pPLT = std::get<0>(tuple_tree);
				if (pPLT != nullptr)
				{
					for (auto& p : pPLT->package_infos)
					{
						if (pMPT->MMT_package_id == std::get<1>(p))
						{
							bInPLTMPTs = true;
							break;
						}
					}
				}

				if (bInPLTMPTs == true || pPLT == nullptr)
				{
					vMPTs.push_back(pMPT);
					// reset the table to NULL to avoid destructing it during PAMessage
					t = nullptr;

					printf("Found a new MPT with package_id: %lld(0X%llX) in header compressed IP packet with CID: %d(0X%X)...\n", package_id, package_id, CID, CID);
				}
				else
				{
					// It is a new MPT which does NOT belong to any PLT, assume there is a null PLT own it
					iter->second.push_back(std::make_tuple(nullptr, std::vector<MMT::MMTPackageTable*>({ pMPT })));
					t = nullptr;
					printf("Found a new MPT with package_id: %lld(0X%llX) in header compressed IP packet with CID: %d(0X%X)...\n", package_id, package_id, CID, CID);
				}
			}
			else
			{
				// It is a new MPT which does NOT belong to any PLT, assume there is a null PLT own it
				iter->second.push_back(std::make_tuple(nullptr, std::vector<MMT::MMTPackageTable*>({ pMPT })));
				t = nullptr;
				printf("Found a new MPT with package_id: %lld(0X%llX) in header compressed IP packet with CID: %d(0X%X)...\n", package_id, package_id, CID, CID);
			}
		}
	}
	
	return RET_CODE_SUCCESS;
}

int ShowMMTPackageInfo()
{
	int nRet = RET_CODE_SUCCESS;
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string& szInputFile = g_params["input"];

	CFileBitstream bs(szInputFile.c_str(), 4096, &nRet);

	if (nRet < 0)
	{
		printf("Failed to open the file: %s.\n", szInputFile.c_str());
		return nRet;
	}

	int nParsedTLVPackets = 0;
	int nIPv4Packets = 0, nIPv6Packets = 0, nHdrCompressedIPPackets = 0, nTransmissionControlSignalPackets = 0, nNullPackets = 0, nOtherPackets = 0;
	TreeCIDPAMsgs CIDPAMsgs;

	try
	{
		uint32_t tlv_hdr = 0;
		uint64_t left_bits = 0;
		bs.Tell(&left_bits);

		while (left_bits >= (4ULL << 3))
		{
			tlv_hdr = (uint32_t)bs.PeekBits(32);
			if (((tlv_hdr >> 24) & 0xFF) != 0x7F)
			{
				printf("[MMT/TLV] TLV header should start with 0x7F at file position: %llu.\n", bs.Tell() >> 3);
				nRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
				break;
			}

			MMT::TLVPacket* pTLVPacket = nullptr;
			switch ((tlv_hdr >> 16) & 0xFF)
			{
			case MMT::TLV_IPv4_packet:
				pTLVPacket = new MMT::Undefined_TLVPacket();
				nIPv4Packets++;
				break;
			case MMT::TLV_IPv6_packet:
				pTLVPacket = new MMT::IPv6Packet();
				nIPv6Packets++;
				break;
			case MMT::TLV_Header_compressed_IP_packet:
				pTLVPacket = new MMT::HeaderCompressedIPPacket();
				nHdrCompressedIPPackets++;
				break;
			case MMT::TLV_Transmission_control_signal_packet:
				pTLVPacket = new MMT::Undefined_TLVPacket();
				nTransmissionControlSignalPackets++;
				break;
			case MMT::TLV_Null_packet:
				pTLVPacket = new MMT::TLVNullPacket();
				nNullPackets++;
				break;
			default:
				pTLVPacket = new MMT::Undefined_TLVPacket();
				nOtherPackets++;
			}

			if ((nRet = pTLVPacket->Unpack(bs)) < 0)
			{
				printf("Failed to unpack a TLV packet at file position: %llu.\n", pTLVPacket->start_bitpos >> 3);
				delete pTLVPacket;
				break;
			}

			if (left_bits < (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3))
			{
				delete pTLVPacket;
				break;
			}

			// Detect the PLT and MPT, and show them
			if (((tlv_hdr >> 16) & 0xFF) == MMT::TLV_Header_compressed_IP_packet)
			{
				MMT::HeaderCompressedIPPacket* pHeaderCompressedIPPacket = (MMT::HeaderCompressedIPPacket*)pTLVPacket;

				// Check whether it is a control message MMT packet
				if (pHeaderCompressedIPPacket->MMTP_Packet == nullptr ||
					pHeaderCompressedIPPacket->MMTP_Packet->Payload_type != 2 ||
					pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages == nullptr)
					goto Skip;

				uint16_t CID = pHeaderCompressedIPPacket->Context_id;

				// Check there is an existed CID there or not, if it does NOT exist, add one
				if (CIDPAMsgs.find(CID) == CIDPAMsgs.end())
				{
					TreePLTMPT emptyTreePLTMAP;
					CIDPAMsgs[CID] = emptyTreePLTMAP;
				}

				// Check whether the current PA message contains a complete message
				if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0 ||
					pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag == 1)
				{
					for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
					{
						auto& v = std::get<1>(m);
						ProcessPAMessage(CIDPAMsgs, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id, &v[0], v.size());
					}
				}
				else
				{
					// Need add the data into the ring buffer, and parse it later
				}
			}

		Skip:
			left_bits -= (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3);
			delete pTLVPacket;

			nParsedTLVPackets++;
		}
	}
	catch (...)
	{
		nRet = RET_CODE_ERROR;
	}

	printf("\nLayout of MMT:\n");

	// Print PLT, MPT and asset tree
	for (auto &m : CIDPAMsgs)
	{
		printf("\tCID: 0X%X(%d):\n", m.first, m.first);
		for (size_t i = 0; i < m.second.size(); i++)
		{
			auto& plt = std::get<0>(m.second[i]);
			auto& vmpt = std::get<1>(m.second[i]);
			printf("\t\t#%04u PLT (Package List Table):\n", i);
			for (size_t j=0;j<vmpt.size();j++)
			{
				auto& mpt = vmpt[j];

				std::string szLocInfo;
				// Get the packet_id of current MPT
				if (plt != nullptr)
				{
					for (auto& plt_pkg: plt->package_infos)
					{
						if (std::get<1>(plt_pkg) == mpt->MMT_package_id)
						{
							auto& info = std::get<2>(plt_pkg);
							szLocInfo = info.GetLocDesc();
						}
					}
				}

				printf("\t\t\t#%05u MPT (MMT Package Table), Package_id: 0X%llX(%llu), %s:\n", j, mpt->MMT_package_id, mpt->MMT_package_id, szLocInfo.c_str());
				for (size_t k = 0; k < mpt->assets.size(); k++)
				{
					auto& a = mpt->assets[k];
					printf("\t\t\t\t#%05u Asset, asset_id: 0X%llX(%llu):\n", k, a->asset_id, a->asset_id);

					for (auto& info : a->MMT_general_location_infos)
					{
						printf("\t\t\t\t\t%s\n", info.GetLocDesc().c_str());
					}

					size_t left_descs_bytes = a->asset_descriptors_bytes.size();
					CBitstream descs_bs(&a->asset_descriptors_bytes[0], a->asset_descriptors_bytes.size() << 3);
					while (left_descs_bytes > 0)
					{
						uint16_t peek_desc_tag = (uint16_t)descs_bs.PeekBits(16);
						MMT::MMTSIDescriptor* pDescr = nullptr;
						switch (peek_desc_tag)
						{
						case 0x8010:	// Video component Descriptor
							pDescr = new MMT::VideoComponentDescriptor();
							break;
						case 0x8014:
							pDescr = new MMT::MHAudioComponentDescriptor();
							break;
						default:
							pDescr = new MMT::UnimplMMTSIDescriptor();
						}

						if (pDescr->Unpack(descs_bs) >= 0)
						{
							if (peek_desc_tag == 0x8010 || peek_desc_tag == 0x8014)
								pDescr->Print(stdout, 40);

							if (left_descs_bytes < pDescr->descriptor_length + 3UL)
								break;

							left_descs_bytes -= pDescr->descriptor_length + 3UL;

							delete pDescr;
						}
						else
						{
							delete pDescr;
							break;
						}
					}
				}
			}
		}
	}

	// Release all PLT and MPT table in CIDPAMsgs
	for (auto& v : CIDPAMsgs)
	{
		for (auto& t : v.second)
		{
			delete std::get<0>(t);
			auto& vmpt = std::get<1>(t);
			for (auto& m : vmpt)
				delete m;
		}
	}
	
	printf("\n");

	printf("The total number of TLV packets: %d.\n", nParsedTLVPackets);
	printf("The number of IPv4 TLV packets: %d.\n", nIPv4Packets);
	printf("The number of IPv6 TLV packets: %d.\n", nIPv6Packets);
	printf("The number of Header Compressed IP packets: %d.\n", nHdrCompressedIPPackets);
	printf("The number of Transmission Control Signal TLV packets: %d.\n", nTransmissionControlSignalPackets);
	printf("The number of Null TLV packets: %d.\n", nNullPackets);
	printf("The number of other TLV packets: %d.\n", nOtherPackets);

	return nRet;
}

int DumpMMT()
{
	if (g_params.find("showpack") != g_params.end())
	{
		return ShowMMTTLVPacks();
	}
	else if (g_params.find("pid") != g_params.end())
	{

	}
	else
	{
		if (g_params.find("output") == g_params.end())
		{
			// No output file is specified
			// Check whether pid and showinfo is there
			if (g_params.find("showinfo") != g_params.end())
			{
				ShowMMTPackageInfo();
				goto done;
			}
			else
			{
				printf("Please specify a output file name with --output.\r\n");
				goto done;
			}
		}
	}

done:
	return RET_CODE_SUCCESS;
}
