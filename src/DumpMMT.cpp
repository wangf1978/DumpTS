/*

MIT License

Copyright (c) 2021 Ravin.Wang(wangf1978@hotmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include "platcomm.h"
#include <ctype.h>
#include "AMRingBuffer.h"
#include "MMT.h"
#include "ESRepacker.h"
#include "PayloadBuf.h"
#include <chrono>

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern int g_verbose_level;

#define DEFAULT_TLV_PACKETS_PER_DISPLAY 20
int g_TLV_packets_per_display = DEFAULT_TLV_PACKETS_PER_DISPLAY;

static const uint8_t NTP_dest_IPv6_Address[16] = {
0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01 };

using MPUSeqTM = std::tuple<uint32_t/*mpu_sequence_number*/,
							IP::NTPv4Data::NTPTimestampFormat/*mpu_presentation_time*/,
							uint32_t/*timescale*/,
							uint16_t/*mpu_decoding_time_offset*/,
							std::vector<
								std::tuple<uint16_t/*dts_pts offset*/,
										   uint16_t/*pts_offset*/>>,
							uint32_t/* the packet sequence number of the first MMTP packet*/
							>;

using MPUTMTree = std::list<MPUSeqTM>;

using MPUTMTrees = std::unordered_map<uint32_t/*CID<<16 | packet_id*/, MPUTMTree>;

using MPUContext = std::unordered_map<uint32_t/*CID<<16 | packet_id*/, 
	std::tuple<
		uint64_t	/* prev MPU sequence number*/,
		MPUSeqTM	/* current MPU sequence time information*/,
		int16_t		/* Access-Unit index of the current MPU */>>;

MPUContext g_MPUContext;

enum SHOW_TLV_PACK_OPTION
{
	SHOW_TLV_ALL = 0,
	SHOW_TLV_IPv4,
	SHOW_TLV_IPv6,
	SHOW_TLV_HCIP,
	SHOW_TLV_TCS,
	SHOW_TLV_NTP,
};

bool NeedShowMMTTable()
{
	return (g_params.find("showinfo") != g_params.end() ||
			g_params.find("showPLT")  != g_params.end() ||
			g_params.find("showMPT")  != g_params.end() ||
			g_params.find("showCAT")  != g_params.end() ||
			g_params.find("showEIT")  != g_params.end());
}

bool NeedShowMMTTLVPacket()
{
	return (g_params.find("showIPv4pack") != g_params.end() ||
			g_params.find("showIPv6pack") != g_params.end() ||
			g_params.find("showHCIPpack") != g_params.end() ||
			g_params.find("showTCSpack")  != g_params.end());
}

void PrintMPUTMTrees(MPUTMTrees& MPU_tm_trees, bool bFull = false)
{
	int ccWritten = 0;
	char szIdx[64] = { 0 };
	int idx = 0;
	int64_t filter_MPUSeqNumber = -1LL;

	auto iterStart = g_params.find("start");
	auto iterEnd = g_params.find("end");
	auto iterMPUSeqNo = g_params.find("MPUseqno");

	int64_t nStart = -1LL, nEnd = INT64_MAX, iVal= -1LL;
	if (iterStart != g_params.end())
	{
		iVal = ConvertToLongLong(iterStart->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nStart = (uint32_t)iVal;
	}

	if (iterEnd != g_params.end())
	{
		iVal = ConvertToLongLong(iterEnd->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nEnd = (uint32_t)iVal;
	}

	if (iterMPUSeqNo != g_params.end())
	{
		iVal = ConvertToLongLong(iterMPUSeqNo->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			filter_MPUSeqNumber = (uint32_t)iVal;
	}

	// find the maximum MPU_sequence_number and timescale
	uint32_t max_MPU_seq_no = 0, max_timescale = 0;
	for (const auto& t : MPU_tm_trees)
	{
		for (const auto& item : t.second)
		{
			if (max_MPU_seq_no < std::get<0>(item))
				max_MPU_seq_no = std::get<0>(item);

			if (max_timescale < std::get<2>(item))
				max_timescale = std::get<2>(item);
		}
	}

	ccWritten = MBCSPRINTF_S(szIdx, sizeof(szIdx) / sizeof(szIdx[0]), "%zu", MPU_tm_trees.size());
	char szFmt1[256] = { 0 }, szFmt2[256] = { 0 }, szFmt3[256] = { 0 };
	MBCSPRINTF_S(szFmt1, sizeof(szFmt1) / sizeof(szFmt1[0]), "%%%dd, CID: 0x%%04X(%%5d), packet_id: 0x%%04X(%%5d):\n", ccWritten);

	for (const auto& t : MPU_tm_trees)
	{
		printf((const char*)szFmt1, idx, (t.first >> 16) & 0xFFFF, (t.first >> 16) & 0xFFFF, t.first & 0xFFFF, t.first & 0xFFFF);

		long long idx2 = 0;
		ccWritten = MBCSPRINTF_S(szIdx, sizeof(szIdx) / sizeof(szIdx[0]), "%zu", t.second.size());
		int ccWritten2 = MBCSPRINTF_S(szIdx, sizeof(szIdx) / sizeof(szIdx[0]), "%X", max_MPU_seq_no);
		int ccWritten3 = MBCSPRINTF_S(szIdx, sizeof(szIdx) / sizeof(szIdx[0]), "%" PRIu32, max_MPU_seq_no);
		MBCSPRINTF_S(szFmt2, sizeof(szFmt2) / sizeof(szFmt2[0]),
			"    %%%dd, MPU(SeqNo: 0x%%0%dX(%%%d" PRIu32 "), presentation_time: %" NTPTIME_FMT_STR "s, timescale: %%%zusHZ, decoding_time_offset: %%5u), start_pkt_seqno: 0x%%08X\n",
			ccWritten, ccWritten2, ccWritten3, GetReadableNum(max_timescale).size());

		for (const auto& item : t.second)
		{
			if (!((int64_t)std::get<0>(item) >= nStart && (int64_t)std::get<0>(item) < nEnd))
				continue;

			if (filter_MPUSeqNumber >= 0 && std::get<0>(item) != filter_MPUSeqNumber)
				continue;

			printf((const char*)szFmt2, idx2,
				std::get<0>(item), std::get<0>(item),
				std::get<1>(item).GetValue(),
				GetReadableNum(std::get<2>(item)).c_str(),
				std::get<3>(item),
				std::get<5>(item));
			auto& offsets = std::get<4>(item);

			if (bFull)
			{
				int idx_sub = 0;
				ccWritten = MBCSPRINTF_S(szIdx, sizeof(szIdx) / sizeof(szIdx[0]), "%zu", offsets.size());
				MBCSPRINTF_S(szFmt3, sizeof(szFmt3) / sizeof(szFmt3[0]),
					"        %%%dd, dts_pts_offset: 0x%%04X(%%5d), pts_offset: 0x%%04X(%%5d), pts/dts(90KHZ): (%%" PRId64 "/%%" PRId64 ")\n", ccWritten);

				IP::NTPv4Data::NTPTimestampFormat dts = std::get<1>(item);
				dts.DecreaseBy(std::get<3>(item), std::get<2>(item));
				IP::NTPv4Data::NTPTimestampFormat pts = dts;
				for (const auto& o : offsets)
				{
					uint16_t dts_pts_offset = std::get<0>(o);
					uint16_t pts_offset = std::get<1>(o);
					
					pts = dts;
					pts.IncreaseBy(dts_pts_offset, std::get<2>(item));

					printf((const char*)szFmt3, idx_sub, dts_pts_offset, dts_pts_offset, pts_offset, pts_offset, pts.ToPTS(), dts.ToPTS());

					dts.IncreaseBy(pts_offset, std::get<2>(item));
					idx_sub++;
				}
			}

			idx2++;
		}

		idx++;
	}
}

int ShowMMTTLVPacks(SHOW_TLV_PACK_OPTION option)
{
	int nRet = RET_CODE_SUCCESS;
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string& szInputFile = g_params["input"];

	auto iterStart = g_params.find("start");
	auto iterEnd = g_params.find("end");
	auto iterPKTSeqNo = g_params.find("PKTseqno");
	auto iterPKTId = g_params.find("pid");

	int64_t nStart = -1LL, nEnd = INT64_MAX, filter_PKTSeqNumber = -1LL, iVal = -1LL, filter_packet_id = -1LL;
	if (iterStart != g_params.end())
	{
		iVal = ConvertToLongLong(iterStart->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nStart = (uint32_t)iVal;
	}

	if (iterEnd != g_params.end())
	{
		iVal = ConvertToLongLong(iterEnd->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nEnd = (uint32_t)iVal;
	}

	if (iterPKTSeqNo != g_params.end())
	{
		if (ConvertToInt(iterPKTSeqNo->second, iVal) && iVal >= 0 && iVal <= UINT32_MAX)
			filter_PKTSeqNumber = iVal;
	}

	if (iterPKTId != g_params.end())
	{
		if (ConvertToInt(iterPKTId->second, iVal) && iVal >= 0 && iVal <= UINT16_MAX)
			filter_packet_id = iVal;
	}

	bool bFilterMMTPpacket = false;
	if (nStart >= 0 || nEnd < INT64_MAX || filter_PKTSeqNumber >= 0)
		bFilterMMTPpacket = true;

	CFileBitstream bs(szInputFile.c_str(), 4096, &nRet);

	if (nRet < 0)
	{
		printf("Failed to open the file: %s.\n", szInputFile.c_str());
		return nRet;
	}

	int nParsedTLVPackets = 0;
	int nIPv4Packets = 0, nIPv6Packets = 0, nHdrCompressedIPPackets = 0, nTransmissionControlSignalPackets = 0, nNullPackets = 0, nOtherPackets = 0;
	int nFilterPackets = 0;

	int nLocateSyncTry = 0;
	bool bFindSync = false;
	uint64_t file_pos = 0ULL;
	int64_t NTP_pkt_idx = 0;
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
				bFindSync = false;
				if (nLocateSyncTry > 10)
				{
					printf("[MMT/TLV] TLV header should start with 0x7F at file position: %" PRIu64 ".\n", bs.Tell() >> 3);
					nRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
					break;
				}
				else
				{
					bs.SkipBits(8);
					continue;
				}
			}
			else
			{
				if (bFindSync == false)
				{
					nLocateSyncTry++;
					bFindSync = true;
				}
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
				pTLVPacket = new MMT::TransmissionControlSignalPacket();
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
				printf("Failed to unpack a TLV packet at file position: %" PRIu64 ".\n", pTLVPacket->start_bitpos >> 3);
				delete pTLVPacket;
				break;
			}

			bool bFiltered = false;
			if ((option) == SHOW_TLV_ALL ||
				(option == SHOW_TLV_IPv4 && ((tlv_hdr >> 16) & 0xFF) == MMT::TLV_IPv4_packet) ||
				((option == SHOW_TLV_IPv6 || option == SHOW_TLV_NTP) && ((tlv_hdr >> 16) & 0xFF) == MMT::TLV_IPv6_packet) ||
				(option == SHOW_TLV_HCIP && ((tlv_hdr >> 16) & 0xFF) == MMT::TLV_Header_compressed_IP_packet) ||
				(option == SHOW_TLV_TCS  && ((tlv_hdr >> 16) & 0xFF) == MMT::TLV_Transmission_control_signal_packet))
			{
				if (bFilterMMTPpacket == false || (bFilterMMTPpacket == true && ((tlv_hdr >> 16) & 0xFF) == MMT::TLV_Header_compressed_IP_packet &&
					((filter_PKTSeqNumber < 0 &&
					((MMT::HeaderCompressedIPPacket*)pTLVPacket)->MMTP_Packet->Packet_sequence_number >= nStart &&
					((MMT::HeaderCompressedIPPacket*)pTLVPacket)->MMTP_Packet->Packet_sequence_number < nEnd) ||
					filter_PKTSeqNumber == ((MMT::HeaderCompressedIPPacket*)pTLVPacket)->MMTP_Packet->Packet_sequence_number)))
				{
					if (filter_packet_id < 0 || (
						((tlv_hdr >> 16) & 0xFF) == MMT::TLV_Header_compressed_IP_packet &&
						((MMT::HeaderCompressedIPPacket*)pTLVPacket)->MMTP_Packet != NULL &&
						((MMT::HeaderCompressedIPPacket*)pTLVPacket)->MMTP_Packet->Packet_id == filter_packet_id))
					{
						bool bNTP = false;
						if (option == SHOW_TLV_NTP)
						{
							MMT::IPv6Packet* pIPv6packet = (MMT::IPv6Packet*)pTLVPacket;
							if (pIPv6packet != NULL && (pIPv6packet->IPv6_Header.Hop_limit == 0x11 || pIPv6packet->IPv6_Header.Hop_limit == 0x20))
							{
								if (memcmp(pIPv6packet->IPv6_Header.Destination_address.address_bytes, NTP_dest_IPv6_Address, 16) == 0)
								{
									if (pIPv6packet->UDP_packet->header.Source_port == 456 && pIPv6packet->UDP_packet->header.Destination_port == 123)
									{
										bNTP = true;
										auto pNTPData = pIPv6packet->UDP_packet->NTPv4_data;
										TM_HNS hns = pNTPData->transmit_timestamp.ToHns();
										TM_HNS rx_hns = pNTPData->receive_timestamp.ToHns();

										uint64_t curr_file_pos = bs.Tell();
										printf("NTP Pkt#%8" PRId64 " [LEA:%d,WM:%9s,S:%d,PO:%d,Prec:%d] Tx: %" PRId64 ".%03" PRId64 " (ms)[%s],size: %" PRIu64 "(bytes)\n",
											NTP_pkt_idx,
											pNTPData->leap_indicator,
											pNTPData->mode == 5?"broadcast":(
											pNTPData->mode == 3?"client":(
											pNTPData->mode == 4?"server":(
											pNTPData->mode == 1?"active": (
											pNTPData->mode == 2?"passive":(
											pNTPData->mode == 6?"message":""))))),
											pNTPData->stratum,
											pNTPData->poll,
											pNTPData->precision,
											hns/10000L, hns/10%1000,
											pNTPData->transmit_timestamp.IsUnknown() ? "" : DateTimeStr(pNTPData->transmit_timestamp.Seconds, 1900, pNTPData->transmit_timestamp.Fraction).c_str(),
											(uint64_t)pTLVPacket->Data_length + 4
										);

										file_pos = curr_file_pos;

										NTP_pkt_idx++;
									}
								}
							}
						}
						
						if (!bNTP)
						{
							pTLVPacket->Print();
							bFiltered = true;
						}
					}
				}
			}

			if (left_bits < (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3))
			{
				delete pTLVPacket;
				break;
			}

			left_bits -= (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3);
			delete pTLVPacket;

			nParsedTLVPackets++;

			if (bFiltered)
			{
				nFilterPackets++;
				if ((nFilterPackets%g_TLV_packets_per_display) == 0 && g_TLV_packets_per_display != std::numeric_limits<decltype(g_TLV_packets_per_display)>::max())
				{
					printf("Press any key to continue('q': quit)...\n");
					char chk = _getch();
					if (chk == 0x3 || chk == 0x1A || chk == 'q' || chk == 'Q')	// Ctrl + C/Z, quit the loop
						break;
				}
			}
		}
	}
	catch (...)
	{
		nRet = RET_CODE_ERROR;
	}

	if (option == SHOW_TLV_NTP)
	{
		printf("--------------------------------------------------------------\n");
		printf("(*) LEA: leap indicator, 0: without alarm; 1: Last one minute is 61 seconds, 2: Last one minute is 59 seconds, 3: Alarm\n");
		printf("(*) WM: work mode, S: stratum, PO: poll, Prec: precision, Rx: receive timestamp, Tx: transmit timestamp\n");
		printf("\n");
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

using CATables = std::list<MMT::CATable*>;
using CIDCATables = std::map<uint16_t, CATables>;

using MHEITables = std::list<MMT::MHEITTable*>;
using CIDMHEITables = std::map<uint16_t, MHEITables>;

uint32_t FindAssetType(const TreeCIDPAMsgs& CIDPAMsgs, uint16_t CID, uint16_t packet_id)
{
	auto iterCID = CIDPAMsgs.find(CID);
	if (iterCID == CIDPAMsgs.cend())
		return 0;

	auto& iterM = iterCID->second.back();
	auto& mpts = std::get<1>(iterM);

	// check whether packet lies at the MPT or not
	if (mpts.size() > 0)
	{
		auto& mpt = mpts.back();
		for (auto& a : mpt->assets)
		{
			for (auto& loc : a->MMT_general_location_infos)
			{
				if (loc.UsePacketID(packet_id))
				{
					return a->asset_type;
				}
			}
		}
	}

	return 0;
}

int UpdateMPUTMTrees(uint16_t CID, MMT::MMTPackageTable* pMPT, MPUTMTrees* ptr_MPU_tm_trees)
{
	if (pMPT == nullptr || ptr_MPU_tm_trees == nullptr)
		return -1;

	size_t left_descs_bytes = pMPT->MPT_descriptors_bytes.size();
	if (left_descs_bytes > 0)
	{
		// Don't parse the descriptor in the first loop
	}

	for (size_t k = 0; k < pMPT->assets.size(); k++)
	{
		auto& a = pMPT->assets[k];
		int32_t pkt_id = -1;
		for (auto& info : a->MMT_general_location_infos)
		{
			pkt_id = info.GetPacketID();
			if (pkt_id > 0)
				break;
		}

		if (pkt_id <= 0)
			continue;

		uint32_t CID_pkt_id = (CID << 16) | pkt_id;
		auto iter = ptr_MPU_tm_trees->find(CID_pkt_id);
		if (iter == ptr_MPU_tm_trees->end())
			continue;

		size_t left_descs_bytes = a->asset_descriptors_bytes.size();
		CBitstream descs_bs(&a->asset_descriptors_bytes[0], a->asset_descriptors_bytes.size() << 3);
		MMT::UnimplMMTSIDescriptor UnimplDescr;
		MMT::MMTSIDescriptor* pDescr = nullptr;
		while (left_descs_bytes > 0)
		{
			uint16_t peek_desc_tag = (uint16_t)descs_bs.PeekBits(16);

			MMT::MPUTimestampDescriptor* pMPUTmDesc = nullptr;
			MMT::MPUExtendedTimestampDescriptor* pMPUTmExtDesc = nullptr;
			
			switch (peek_desc_tag)
			{
			case 0x0001:	// MPUTimestampDescriptor
				pMPUTmDesc = new MMT::MPUTimestampDescriptor();
				pDescr = pMPUTmDesc;
				break;
			case 0x8026:	// MPUExtendedTimestampDescriptor
				pMPUTmExtDesc = new MMT::MPUExtendedTimestampDescriptor();
				pDescr = pMPUTmExtDesc;
				break;
			default:
				pDescr = &UnimplDescr;
			}

			if (pDescr->Unpack(descs_bs) >= 0)
			{
				if (left_descs_bytes < (size_t)pDescr->descriptor_length + 3UL)
				{
					AMP_SAFEDEL(pMPUTmDesc);
					AMP_SAFEDEL(pMPUTmExtDesc);
					break;
				}

				left_descs_bytes -= (size_t)pDescr->descriptor_length + 3UL;

				if (pMPUTmDesc != nullptr)
				{
					for (auto& e : pMPUTmDesc->MPU_sequence_present_timestamps)
					{
						bool bFound = false;
						for(auto riter = iter->second.rbegin();riter != iter->second.rend();riter++)
						{
							auto MPU_sequence_number = std::get<0>(*riter);
							auto MPU_presentation_time = std::get<1>(*riter);
							if (MPU_sequence_number == std::get<0>(e))
							{
								bFound = true;
								if (MPU_presentation_time != std::get<1>(e))
								{
									printf("Unexpected case!!! 2 MPU time descriptor entry with the same MPU sequence number(%" PRIu32 ") has different presentation time.\n", 
										MPU_sequence_number);
								}
								break;
							}
						}

						if (bFound == false)
						{
							iter->second.push_back(std::make_tuple(std::get<0>(e), std::get<1>(e), 0, 0, std::vector<std::tuple<uint16_t, uint16_t>>(), 0));
						}
					}
				}
				else if (pMPUTmExtDesc != nullptr)
				{
					for (auto& e : pMPUTmExtDesc->ts_offsets)
					{
						for (auto riter = iter->second.rbegin(); riter != iter->second.rend(); riter++)
						{
							auto MPU_sequence_number = std::get<0>(*riter);
							if (MPU_sequence_number == e.mpu_sequence_number)
							{
								// Update the entry
								if (pMPUTmExtDesc->timescale_flag)
									std::get<2>(*riter) = pMPUTmExtDesc->timescale;
								else
									std::get<2>(*riter) = 90000;

								std::get<3>(*riter) = e.mpu_decoding_time_offset;

								std::get<4>(*riter).clear();
								for (auto& o : e.offsets)
								{
									std::get<4>(*riter).push_back({std::get<0>(o), 
										pMPUTmExtDesc->pts_offset_type == 2?std::get<1>(o):(
										pMPUTmExtDesc->pts_offset_type == 1?pMPUTmExtDesc->default_pts_offset:0)});
								}

								break;
							}
						}
					}
				}
			}

			AMP_SAFEDEL(pMPUTmDesc);
			AMP_SAFEDEL(pMPUTmExtDesc);
		}
	}


	return 0;
}

int ProcessCAMessage(
	CIDCATables& CIDCATables,
	uint16_t CID,
	uint64_t packet_id,
	uint8_t* pMsgBuf,
	int cbMsgBuf,
	bool* pbChange = nullptr,
	int dumpOptions = 0)
{
	if (cbMsgBuf <= 0)
		return RET_CODE_INVALID_PARAMETER;

	CBitstream bs(pMsgBuf, ((size_t)cbMsgBuf << 3));
	uint16_t msg_id = (uint16_t)bs.PeekBits(16);

	// If the current message is not a CA message, ignore it.
	if (msg_id != 0x8001)
		return RET_CODE_SUCCESS;

	// Make sure the tree PA message is already there for the specified CID
	auto iter = CIDCATables.find(CID);
	if (iter == CIDCATables.end())
		return RET_CODE_INVALID_PARAMETER;

	int iRet = RET_CODE_SUCCESS;
	MMT::CAMessage CAMsg(cbMsgBuf);
	if ((iRet = CAMsg.Unpack(bs)) < 0)
		return iRet;

	if (CAMsg.table == nullptr)
		return iRet;

	bool bCATChanged = false;
	if (CAMsg.table->table_id == 0x86)
	{
		if (iter->second.size() == 0)
		{
			iter->second.push_back((MMT::CATable*)CAMsg.table);
			CAMsg.table = nullptr;
			bCATChanged = true;
		}
		else
		{
			MMT::CATable* pCATable = iter->second.back();
			if (pCATable->version != CAMsg.table->version)
			{
				iter->second.push_back((MMT::CATable*)CAMsg.table);
				CAMsg.table = nullptr;
				bCATChanged = true;
			}
		}

		if (pbChange)
			*pbChange = bCATChanged;

		if (bCATChanged)
		{
			if (dumpOptions&(DUMP_CAT | DUMP_MEDIA_INFO_VIEW))
			{
				printf("Found a new CAT with packet_id: %" PRIu64 "(0X%" PRIX64 ") in header compressed IP packet with CID: %d(0X%X)...\n", packet_id, packet_id, CID, CID);

				if (dumpOptions&DUMP_CAT)
				{
					iter->second.back()->Print(stdout, 4);
				}
			}
		}
	}

	return RET_CODE_SUCCESS;
}

int ProcessMHEIMessage(
	CIDMHEITables& CIDMHEITbls,
	uint16_t CID,
	uint64_t packet_id,
	uint8_t* pMsgBuf,
	int cbMsgBuf,
	bool* pbChange = nullptr,
	int dumpOptions = 0)
{
	if (cbMsgBuf <= 0)
		return RET_CODE_INVALID_PARAMETER;

	CBitstream bs(pMsgBuf, ((size_t)cbMsgBuf << 3));
	uint16_t msg_id = (uint16_t)bs.PeekBits(16);

	// If the current message is not a CA message, ignore it.
	if (msg_id != 0x8001)
		return RET_CODE_SUCCESS;

	// Make sure the tree PA message is already there for the specified CID
	auto iter = CIDMHEITbls.find(CID);
	if (iter == CIDMHEITbls.end())
		return RET_CODE_INVALID_PARAMETER;

	int iRet = RET_CODE_SUCCESS;
	MMT::MH_EITMessage MHEIMsg(cbMsgBuf);
	if ((iRet = MHEIMsg.Unpack(bs)) < 0)
		return iRet;

	if (MHEIMsg.table == nullptr)
		return iRet;

	bool bCATChanged = false;
	if (MHEIMsg.table->table_id == 0x86)
	{
		if (iter->second.size() == 0)
		{
			iter->second.push_back((MMT::MHEITTable*)MHEIMsg.table);
			MHEIMsg.table = nullptr;
			bCATChanged = true;
		}
		else
		{
			MMT::MHEITTable* pCATable = iter->second.back();
			if (pCATable->version != MHEIMsg.table->version)
			{
				iter->second.push_back((MMT::MHEITTable*)MHEIMsg.table);
				MHEIMsg.table = nullptr;
				bCATChanged = true;
			}
		}

		if (pbChange)
			*pbChange = bCATChanged;

		if (bCATChanged)
		{
			if (dumpOptions&(DUMP_CAT | DUMP_MEDIA_INFO_VIEW))
			{
				printf("Found a new MH-EIT with packet_id: %" PRIu64 "(0X%" PRIX64 ") in header compressed IP packet with CID: %d(0X%X)...\n", packet_id, packet_id, CID, CID);

				if (dumpOptions&DUMP_CAT)
				{
					iter->second.back()->Print(stdout, 4);
				}
			}
		}
	}

	return RET_CODE_SUCCESS;
}

int ProcessPAMessage(
	TreeCIDPAMsgs& CIDPAMsgs, 
	uint16_t CID, 
	uint64_t packet_id, 
	uint8_t* pMsgBuf, 
	int cbMsgBuf, 
	bool* pbChange=nullptr, 
	int dumpOptions = 0, 
	const std::vector<uint16_t>* filter_packets_ids = nullptr,
	MPUTMTrees* ptr_MPU_tm_trees = NULL)
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

			if (g_verbose_level > 0)
				printf("Found a new PLT with packet_id: %" PRIu64 "(0X%" PRIX64 ")(version: %d) in header compressed IP packet with CID: %d(0X%X)...\n", packet_id, packet_id, t->version, CID, CID);

			std::string szLocInfo;
			// Get the packet_id of current MPT
			for (auto& plt_pkg : pPLT->package_infos)
			{
				uint64_t pkg_id = std::get<1>(plt_pkg);
				auto& info = std::get<2>(plt_pkg);
				szLocInfo = info.GetLocDesc();

				if (g_verbose_level > 0)
					printf("\tFound a package with package_id: %" PRIu64 "(0X%" PRIX64 "), %s.\n", pkg_id, pkg_id, szLocInfo.c_str());
			}

			if (g_verbose_level > 0 || (dumpOptions&DUMP_PLT))
				pPLT->Print(stdout, 8);
			
			// A new PLT is found
			iter->second.push_back(std::make_tuple(pPLT, std::vector<MMT::MMTPackageTable*>()));
			// reset the PLT to NULL to avoid destructing it during PAMessage
			t = nullptr;
			if (pbChange)
				*pbChange = true;
		}
		else if (t->table_id == 0x20) // MPT
		{
			bool bFoundExistedMPT = false, bInPLTMPTs = false, bFoundNewMPT = false;;
			MMT::MMTPackageTable* pMPT = (MMT::MMTPackageTable*)t;

			//printf("\n=============MPT version: %d=============\n", t->version);

			// Check whether the current MPT lies at the last PLT or not
			if (iter->second.size() > 0)
			{
				// Check whether there are MPU time-stamp descriptor and/or MPU extended time-stamp descriptor
				// And try to update it.
				UpdateMPUTMTrees(CID, pMPT, ptr_MPU_tm_trees);

				auto& tuple_tree = iter->second.back();
				auto& vMPTs = std::get<1>(tuple_tree);
				for (auto& m : vMPTs)
				{
					if (m->MMT_package_id == pMPT->MMT_package_id && m->version == pMPT->version)
					{
						bFoundExistedMPT = true;
						break;
					}
				}

				// Already found the existed MPT, continue the next table processing
				if (bFoundExistedMPT == true && !(dumpOptions&DUMP_MPT))
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
					bFoundNewMPT = true;
					if (pbChange)
						*pbChange = true;
					if (dumpOptions&(DUMP_MPT | DUMP_MEDIA_INFO_VIEW))
						printf("Found a new MPT with packet_id: %" PRIu64 "(0X%" PRIX64 ") in header compressed IP packet with CID: %d(0X%X)...\n", packet_id, packet_id, CID, CID);
				}
				else
				{
					// It is a new MPT which does NOT belong to any PLT, assume there is a null PLT own it
					iter->second.push_back(std::make_tuple(nullptr, std::vector<MMT::MMTPackageTable*>({ pMPT })));
					t = nullptr;
					bFoundNewMPT = true;
					if (pbChange)
						*pbChange = true;
					if (dumpOptions&(DUMP_MPT | DUMP_MEDIA_INFO_VIEW))
						printf("Found a new MPT with packet_id: %" PRIu64 "(0X%" PRIX64 ") in header compressed IP packet with CID: %d(0X%X)...\n", packet_id, packet_id, CID, CID);
				}
			}
			else
			{
				// It is a new MPT which does NOT belong to any PLT, assume there is a null PLT own it
				iter->second.push_back(std::make_tuple(nullptr, std::vector<MMT::MMTPackageTable*>({ pMPT })));
				t = nullptr;
				bFoundNewMPT = true;
				if (pbChange)
					*pbChange = true;
				if (dumpOptions&(DUMP_MPT | DUMP_MEDIA_INFO_VIEW))
					printf("Found a new MPT with packet_id: %" PRIu64 "(0X%" PRIX64 ") in header compressed IP packet with CID: 0X%X(%u)...\n",
						packet_id, packet_id, CID, CID);
			}

			if ((dumpOptions&DUMP_MPT))
			{
				printf("%21s: 0x%X\n", "table_id", pMPT->table_id);
				printf("%21s: 0x%X\n", "version", pMPT->version);
				printf("%21s: %d(%s)\n", "MPT_mode", pMPT->MPT_mode, 
					pMPT->MPT_mode == 0?"MPT is processed according to the order of subset":(
					pMPT->MPT_mode == 1?"After MPT of subset 0 is received, any subset which has the same version number can be processed":(
					pMPT->MPT_mode == 2?"MPT of subset can be processed arbitrarily":"Unknown")));
				printf("%21s: 0x%" PRIX64 "\n", "MMT_package_id", pMPT->MMT_package_id);

				size_t left_descs_bytes = pMPT->MPT_descriptors_bytes.size();
				if (left_descs_bytes > 0)
				{
					CBitstream descs_bs(&pMPT->MPT_descriptors_bytes[0], pMPT->MPT_descriptors_bytes.size() << 3);
					while (left_descs_bytes > 0)
					{
						uint16_t peek_desc_tag = (uint16_t)descs_bs.PeekBits(16);
						MMT::MMTSIDescriptor* pDescr = nullptr;
						switch (peek_desc_tag)
						{
						case 0x0001:	// MPUTimestampDescriptor
							pDescr = new MMT::MPUTimestampDescriptor();
							break;
						case 0x8010:	// Video component Descriptor
							pDescr = new MMT::VideoComponentDescriptor();
							break;
						case 0x8014:
							pDescr = new MMT::MHAudioComponentDescriptor();
							break;
						case 0x8011:	// MH-stream identification descriptor:
							pDescr = new MMT::MHStreamIdentifierDescriptor();
							break;
						case 0x8020:
							pDescr = new MMT::MHDataComponentDescriptor();
							break;
						case 0x8026:	// MPUExtendedTimestampDescriptor
							pDescr = new MMT::MPUExtendedTimestampDescriptor();
							break;
						case 0x8038:	// Content copy control Descriptor
							pDescr = new MMT::ContentCopyControlDescriptor();
							break;
						case 0x8039:	// Content usage control Descriptor
							pDescr = new MMT::ContentUsageControlDescriptor();
							break;
						default:
							pDescr = new MMT::UnimplMMTSIDescriptor();
						}

						if (pDescr->Unpack(descs_bs) >= 0)
						{
							pDescr->Print(stdout, 16);

							if (left_descs_bytes < (size_t)pDescr->descriptor_length + 3UL)
								break;

							left_descs_bytes -= (size_t)pDescr->descriptor_length + 3UL;

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

			if (bFoundNewMPT && pMPT != nullptr)
			{
				uint32_t MPT_CID_packet_id = (uint32_t)(((uint64_t)CID << 16) | packet_id);
				auto iterMPTCIDPktAsset = MMT::MMTPPacket::MMTP_packet_asset_types.find(MPT_CID_packet_id);
				if (iterMPTCIDPktAsset == MMT::MMTPPacket::MMTP_packet_asset_types.end())
					MMT::MMTPPacket::MMTP_packet_asset_types[MPT_CID_packet_id].insert('mmpt');
				else
					iterMPTCIDPktAsset->second.insert('mmpt');

				for (size_t k = 0; k < pMPT->assets.size(); k++)
				{
					// Check whether the current packet_id should be selected or not
					bool bPacketIDFiltered = false;
					if (filter_packets_ids == nullptr || filter_packets_ids->size() == 0)
						bPacketIDFiltered = true;

					auto& a = pMPT->assets[k];
					for (auto& info : a->MMT_general_location_infos)
					{
						if (bPacketIDFiltered == false &&
							std::find(filter_packets_ids->cbegin(), filter_packets_ids->cend(), info.packet_id) != filter_packets_ids->cend())
							bPacketIDFiltered = true;
					}

					if (dumpOptions&(DUMP_MPT | DUMP_MEDIA_INFO_VIEW) && bPacketIDFiltered)
					{
						printf("\t#%05" PRIu32 " Asset, asset_id: 0X%" PRIX64 "(%" PRIu64 "), asset_type: %c%c%c%c(0X%08X):\n", (uint32_t)k, a->asset_id, a->asset_id,
							isprint((a->asset_type >> 24) & 0xFF) ? ((a->asset_type >> 24) & 0xFF) : '.',
							isprint((a->asset_type >> 16) & 0xFF) ? ((a->asset_type >> 16) & 0xFF) : '.',
							isprint((a->asset_type >> 8) & 0xFF) ? ((a->asset_type >> 8) & 0xFF) : '.',
							isprint((a->asset_type) & 0xFF) ? ((a->asset_type) & 0xFF) : '.',
							a->asset_type);
					}

					for (auto& info : a->MMT_general_location_infos)
					{
						if (dumpOptions&(DUMP_MPT | DUMP_MEDIA_INFO_VIEW) && bPacketIDFiltered)
							printf("\t\t%s\n", info.GetLocDesc().c_str());

						if (info.location_type == 0 || info.location_type == 1 || info.location_type == 2)
						{
							uint32_t CID_packet_id = (CID << 16) | info.packet_id;
							auto iterCIDPktAsset = MMT::MMTPPacket::MMTP_packet_asset_types.find(CID_packet_id);
							if (iterCIDPktAsset == MMT::MMTPPacket::MMTP_packet_asset_types.end())
								MMT::MMTPPacket::MMTP_packet_asset_types[CID_packet_id].insert(a->asset_type);
							else
								iterCIDPktAsset->second.insert(a->asset_type);
						}
					}

					size_t left_descs_bytes = a->asset_descriptors_bytes.size();
					CBitstream descs_bs(&a->asset_descriptors_bytes[0], a->asset_descriptors_bytes.size() << 3);
					while (left_descs_bytes > 0)
					{
						uint16_t peek_desc_tag = (uint16_t)descs_bs.PeekBits(16);
						MMT::MMTSIDescriptor* pDescr = nullptr;
						switch (peek_desc_tag)
						{
						case 0x0001:	// MPUTimestampDescriptor
							pDescr = new MMT::MPUTimestampDescriptor();
							break;
						case 0x8010:	// Video component Descriptor
							pDescr = new MMT::VideoComponentDescriptor();
							break;
						case 0x8014:
							pDescr = new MMT::MHAudioComponentDescriptor();
							break;
						case 0x8011:	// MH-stream identification descriptor:
							pDescr = new MMT::MHStreamIdentifierDescriptor();
							break;
						case 0x8020:
							pDescr = new MMT::MHDataComponentDescriptor();
							break;
						case 0x8026:	// MPUExtendedTimestampDescriptor
							pDescr = new MMT::MPUExtendedTimestampDescriptor();
							break;
						default:
							pDescr = new MMT::UnimplMMTSIDescriptor();
						}

						if (pDescr->Unpack(descs_bs) >= 0)
						{
							if ((dumpOptions&DUMP_MPT) && bPacketIDFiltered)
							{
								pDescr->Print(stdout, 28);
							}
							else if (peek_desc_tag == 0x8010 || peek_desc_tag == 0x8014 || peek_desc_tag == 0x8020)
							{
								if (dumpOptions&(DUMP_MPT | DUMP_MEDIA_INFO_VIEW) && bPacketIDFiltered)
									pDescr->Print(stdout, 28);
							}

							if (left_descs_bytes < (size_t)pDescr->descriptor_length + 3UL)
								break;

							left_descs_bytes -= (size_t)pDescr->descriptor_length + 3UL;

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
	
	return RET_CODE_SUCCESS;
}

int ProcessMFU(MMT::HeaderCompressedIPPacket* pHeaderCompressedIPPacket, uint32_t asset_type,
	uint8_t* pMFUData, int cbMFUData, CESRepacker* pESRepacker = nullptr, MPUTMTrees* ptr_MPU_tm_trees=nullptr)
{
	if (pHeaderCompressedIPPacket->MMTP_Packet == NULL ||
		pHeaderCompressedIPPacket->MMTP_Packet->Payload_type != 0 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	if (pESRepacker == nullptr)
		return RET_CODE_NOT_INITIALIZED;

	PROCESS_DATA_INFO data_info;

	if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->fragmentation_indicator == 0)
		data_info.indicator = FRAG_INDICATOR_COMPLETE;
	else if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->fragmentation_indicator == 1)
		data_info.indicator = FRAG_INDICATOR_FIRST;
	else if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->fragmentation_indicator == 2)
		data_info.indicator = FRAG_INDICATOR_MIDDLE;
	else if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->fragmentation_indicator == 3)
		data_info.indicator = FRAG_INDICATOR_LAST;
	else
		return RET_CODE_ERROR_NOTIMPL;

	data_info.CID = pHeaderCompressedIPPacket->Context_id;
	data_info.packet_id = pHeaderCompressedIPPacket->MMTP_Packet->Packet_id;
	data_info.packet_sequence_number = pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number;
	data_info.MPU_sequence_number = pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number;

	// Check MPU sequence number is changed or not
	if (asset_type == 'hev1' || asset_type == 'hvc1' || asset_type == 'mp4a')
	{
		bool bMPUStartPoint = false;
		uint32_t CID_pkt_id = (pHeaderCompressedIPPacket->Context_id << 16) | pHeaderCompressedIPPacket->MMTP_Packet->Packet_id;
		auto iterCIDPktMPUSeqNo = g_MPUContext.find(CID_pkt_id);
		if (iterCIDPktMPUSeqNo != g_MPUContext.end())
		{
			if (std::get<0>(iterCIDPktMPUSeqNo->second) != pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number)
			{
				if (!pHeaderCompressedIPPacket->MMTP_Packet->RAP_flag && (asset_type == 'hev1' || asset_type == 'hvc1'))
					printf("Unexpected case! the video signal MPU sequence is changed, but RAP_flag is NOT 1.\n");
				bMPUStartPoint = true;
			}
		}
		else if (pHeaderCompressedIPPacket->MMTP_Packet->RAP_flag)
			bMPUStartPoint = true;

		if (bMPUStartPoint)
		{
			auto MPU_time_tree = ptr_MPU_tm_trees->find(CID_pkt_id);
			if (MPU_time_tree == ptr_MPU_tm_trees->end())
			{
				printf("Failed to find MPU time descriptor for the asset with CID:0x%04X and pkt_id: 0x%04X.\n",
					pHeaderCompressedIPPacket->Context_id, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id);
				iterCIDPktMPUSeqNo = g_MPUContext.end();
			}
			else
			{
				bool bFoundMPUSeqTmEntry = false;
				// Update the MPU time information to generate the PTS/DTS
				for (auto& MPU_time_entry : MPU_time_tree->second)
				{
					if (std::get<0>(MPU_time_entry) == pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number)
					{
						bFoundMPUSeqTmEntry = true;

						std::get<5>(MPU_time_entry) = pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number;

						if (iterCIDPktMPUSeqNo != g_MPUContext.end())
						{
							std::get<0>(iterCIDPktMPUSeqNo->second) = pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number;
							std::get<1>(iterCIDPktMPUSeqNo->second) = MPU_time_entry;
							std::get<2>(iterCIDPktMPUSeqNo->second) = 0;
						}
						else
						{
							auto insert_ret = g_MPUContext.emplace(CID_pkt_id, std::make_tuple(
								pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number,
								MPU_time_entry, 0));
							if (insert_ret.second)
								iterCIDPktMPUSeqNo = insert_ret.first;
						}

						break;
					}
				}

				if (bFoundMPUSeqTmEntry == false)
				{
					printf("Failed to find MPU time descriptor, can't generate PTS/DTS for the MPU with MPU_sequence_number: 0x%08X.\n",
						pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number);
					iterCIDPktMPUSeqNo = g_MPUContext.end();
				}
				else
				{
					// Update all pts and dts of the new MPU into re-packer
					std::vector<TM_90KHZ> dtses, ptses;
					auto& mpu_seq_tm = std::get<1>(iterCIDPktMPUSeqNo->second);
					IP::NTPv4Data::NTPTimestampFormat mpu_presentation_time = std::get<1>(mpu_seq_tm);
					uint32_t time_scale = std::get<2>(mpu_seq_tm);
					uint16_t mpu_decoding_time_offset = std::get<3>(mpu_seq_tm);
					std::get<5>(mpu_seq_tm) = pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number;
					const auto& pts_offset = std::get<4>(mpu_seq_tm);

					dtses.reserve(pts_offset.size());
					ptses.reserve(pts_offset.size());

					int64_t total_pts_offst = -1LL * mpu_decoding_time_offset;
					IP::NTPv4Data::NTPTimestampFormat dts = mpu_presentation_time;
					dts.DecreaseBy(mpu_decoding_time_offset, time_scale);
					IP::NTPv4Data::NTPTimestampFormat pts = dts;
					//printf("MPU Sequence No: 0x%X\n", pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number);
					for (size_t i = 0; i < pts_offset.size(); i++)	
					{
						dtses.push_back(dts.ToPTS());
						pts = dts;
						pts.IncreaseBy(std::get<0>(pts_offset[i]), time_scale);
						ptses.push_back(pts.ToPTS());

						//printf("    pts: %" PRId64 ", dts: %" PRId64 ", pts_dts_diff: %" PRId64 "\n", pts.ToPTS(), dts.ToPTS(), pts.PTSDiff(dts));

						dts.IncreaseBy(std::get<1>(pts_offset[i]), time_scale);
					}

					pESRepacker->SetNextMPUPtsDts((int)ptses.size(), ptses.data(), dtses.data());
				}
			}
		}
	}

	return pESRepacker->Process(pMFUData, cbMFUData, &data_info);
}

int ShowInfoProcessCAMessage(
	CIDCATables& CIDCATbls, uint16_t CID, 
	MMT::HeaderCompressedIPPacket* pHeaderCompressedIPPacket, std::vector<uint8_t>& fullCAMessage,
	std::set<uint64_t>& CID_EMM_packet_id_set,
	int dumpopt)
{
	// Check there is an existed CID there or not, if it does NOT exist, add one
	if (CIDCATbls.find(CID) == CIDCATbls.end())
	{
		CIDCATbls[CID] = CATables();
	}

	bool bCAMessageChanged = false;
	// Check whether the current CA message contains a complete message
	if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 3 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag == 1)
	{
		if (fullCAMessage.size() > 0)
		{
			for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
			{
				auto& v = std::get<1>(m);
				if (v.size() > 0)
				{
					size_t orig_size = fullCAMessage.size();
					fullCAMessage.resize(orig_size + v.size());
					memcpy(&fullCAMessage[orig_size], &v[0], v.size());
				}
			}

			//print_mem(&fullCAMessage[0], (int)fullCAMessage.size(), 4);
			if (ProcessCAMessage(CIDCATbls, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
				&fullCAMessage[0], (int)fullCAMessage.size(), &bCAMessageChanged, dumpopt) == 0 && bCAMessageChanged)
			{
				// TODO...
			}

			fullCAMessage.clear();
		}
		else
		{
			for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
			{
				bool bChanged = false;
				auto& v = std::get<1>(m);

				//print_mem(&v[0], (int)v.size(), 4);
				if (ProcessCAMessage(CIDCATbls, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
					&v[0], (int)v.size(), &bChanged, dumpopt) == 0 && bChanged)
				{
					// TODO...
				}

				if (bChanged && bCAMessageChanged == false)
					bCAMessageChanged = true;
			}
		}

		if (bCAMessageChanged)
		{
			CID_EMM_packet_id_set.clear();
			for (const auto& CIDCA : CIDCATbls)
			{
				uint64_t CID_packet_id = ((uint64_t)CIDCA.first) << 16;

				for (const auto& entry : CIDCA.second)
				{
					MMT::CATable* pCATable = (MMT::CATable*)entry;
					for (const auto& desc : pCATable->descriptors)
					{
						if (desc->descriptor_tag == 0x8004)
						{
							MMT::AccessControlDescriptor* ac_desc = (MMT::AccessControlDescriptor*)desc;
							if (ac_desc->MMT_general_location_info.location_type == 0 ||
								ac_desc->MMT_general_location_info.location_type == 1 ||
								ac_desc->MMT_general_location_info.location_type == 2)
							{
								CID_packet_id = CID_packet_id & 0xFFFFFFFFFFFF0000ULL;
								CID_packet_id |= ac_desc->MMT_general_location_info.GetPacketID();
								CID_EMM_packet_id_set.insert(CID_packet_id);
							}
						}
						else
							continue;
					}
				}
			}
		}
	}
	else
	{
		assert(pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages.size() <= 1);
		// Need add the data into the ring buffer, and parse it later
		for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
		{
			auto& v = std::get<1>(m);
			if (v.size() > 0)
			{
				size_t orig_size = fullCAMessage.size();
				fullCAMessage.resize(orig_size + v.size());
				memcpy(&fullCAMessage[orig_size], &v[0], v.size());
			}
		}
	}

	return RET_CODE_SUCCESS;
}

int ShowInfoProcessMHEIMessage(
	CIDMHEITables& CIDMHEITbls, uint16_t CID,
	MMT::HeaderCompressedIPPacket* pHeaderCompressedIPPacket, std::vector<uint8_t>& fullMHEIMessage,
	int dumpopt)
{
	if (CIDMHEITbls.find(CID) == CIDMHEITbls.end())
	{
		CIDMHEITbls[CID] = MHEITables();
	}

	bool bMHEIMessageChanged = false;
	// Check whether the current MH-EI message contains a complete message
	if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 3 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag == 1)
	{
		if (fullMHEIMessage.size() > 0)
		{
			for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
			{
				auto& v = std::get<1>(m);
				if (v.size() > 0)
				{
					size_t orig_size = fullMHEIMessage.size();
					fullMHEIMessage.resize(orig_size + v.size());
					memcpy(&fullMHEIMessage[orig_size], &v[0], v.size());
				}
			}

			//print_mem(&fullMHEIMessage[0], (int)fullMHEIMessage.size(), 4);
			if (ProcessMHEIMessage(CIDMHEITbls, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
				&fullMHEIMessage[0], (int)fullMHEIMessage.size(), &bMHEIMessageChanged, dumpopt) == 0 && bMHEIMessageChanged)
			{
				// TODO...
			}

			fullMHEIMessage.clear();
		}
		else
		{
			for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
			{
				bool bChanged = false;
				auto& v = std::get<1>(m);

				//print_mem(&v[0], (int)v.size(), 4);
				if (ProcessMHEIMessage(CIDMHEITbls, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
					&v[0], (int)v.size(), &bChanged, dumpopt) == 0 && bChanged)
				{
					// TODO...
				}

				if (bChanged && bMHEIMessageChanged == false)
					bMHEIMessageChanged = true;
			}
		}
	}
	else
	{
		assert(pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages.size() <= 1);
		// Need add the data into the ring buffer, and parse it later
		for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
		{
			auto& v = std::get<1>(m);
			if (v.size() > 0)
			{
				size_t orig_size = fullMHEIMessage.size();
				fullMHEIMessage.resize(orig_size + v.size());
				memcpy(&fullMHEIMessage[orig_size], &v[0], v.size());
			}
		}
	}

	return RET_CODE_SUCCESS;
}

int ShowInfoProcessPAMessage(
	TreeCIDPAMsgs& CIDPAMsgs, uint16_t CID, 
	MMT::HeaderCompressedIPPacket* pHeaderCompressedIPPacket, std::vector<uint8_t>& fullPAMessage,
	std::vector<uint16_t>& fitler_packet_ids, int dumpopt)
{
	// Check there is an existed CID there or not, if it does NOT exist, add one
	if (CIDPAMsgs.find(CID) == CIDPAMsgs.end())
	{
		TreePLTMPT emptyTreePLTMAP;
		CIDPAMsgs[CID] = emptyTreePLTMAP;
	}

	// Check whether the current PA message contains a complete message
	if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 3 ||
		pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag == 1)
	{
		if (fullPAMessage.size() > 0)
		{
			for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
			{
				auto& v = std::get<1>(m);
				if (v.size() > 0)
				{
					size_t orig_size = fullPAMessage.size();
					fullPAMessage.resize(orig_size + v.size());
					memcpy(&fullPAMessage[orig_size], &v[0], v.size());
				}
			}

			ProcessPAMessage(CIDPAMsgs, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id, &fullPAMessage[0], (int)fullPAMessage.size(), nullptr, dumpopt, &fitler_packet_ids);

			fullPAMessage.clear();
		}
		else
		{
			for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
			{
				auto& v = std::get<1>(m);
				ProcessPAMessage(CIDPAMsgs, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id, &v[0], (int)v.size(), nullptr, dumpopt, &fitler_packet_ids);
			}
		}
	}
	else
	{
		assert(pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages.size() <= 1);
		// Need add the data into the ring buffer, and parse it later
		for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
		{
			auto& v = std::get<1>(m);
			if (v.size() > 0)
			{
				size_t orig_size = fullPAMessage.size();
				fullPAMessage.resize(orig_size + v.size());
				memcpy(&fullPAMessage[orig_size], &v[0], v.size());
			}
		}
	}

	return RET_CODE_SUCCESS;
}

int ShowMMTPackageInfo()
{
	int dumpopt = 0;
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

	if (g_params.find("showinfo") != g_params.end())
		dumpopt |= DUMP_MEDIA_INFO_VIEW;

	if (g_params.find("showMPT") != g_params.end())
		dumpopt |= DUMP_MPT;

	if (g_params.find("showPLT") != g_params.end())
		dumpopt |= DUMP_PLT;

	if (g_params.find("showCAT") != g_params.end())
		dumpopt |= DUMP_CAT;

	if (g_params.find("showEIT") != g_params.end())
		dumpopt |= DUMP_MH_EIT;

	const char* sp;
	const char* ep;
	int64_t i64Val = -1LL;
	std::vector<uint16_t> fitler_packet_ids;
	auto pidIter = g_params.find("pid");
	if (pidIter != g_params.end())
	{
		size_t iterPID = pidIter->second.find("&");
		if (iterPID == 0 || iterPID == std::string::npos)//Only one pid/packet_id this case
		{
			sp = pidIter->second.c_str();
			ep = sp + pidIter->second.length();
			if (iterPID == 0)
				sp++;
			if (ConvertToInt((char*)sp, (char*)ep, i64Val) == false || i64Val < 0 || i64Val > UINT16_MAX)
			{
				printf("Please specify a valid packet_id.\n");
				return RET_CODE_ERROR;
			}
			fitler_packet_ids.push_back((uint16_t)i64Val);
		}
		else//Two pids/packet_ids
		{
			std::string PIDs;
			PIDs = pidIter->second.substr(0, iterPID);
			sp = PIDs.c_str();
			ep = sp + PIDs.length();
			if (ConvertToInt((char*)sp, (char*)ep, i64Val) == false || i64Val < 0 || i64Val > UINT16_MAX)
			{
				printf("Please specify a valid packet_id.\n");
				return RET_CODE_ERROR;
			}
			fitler_packet_ids.push_back((uint16_t)i64Val);

			PIDs = pidIter->second.substr(iterPID + 1, pidIter->second.length());
			sp = PIDs.c_str();
			ep = sp + PIDs.length();
			if (ConvertToInt((char*)sp, (char*)ep, i64Val) == false || i64Val < 0 || i64Val > UINT16_MAX)
			{
				printf("Please specify a valid packet_id for the second pid.\n");
				return RET_CODE_ERROR;
			}
			fitler_packet_ids.push_back((uint16_t)i64Val);
		}
	}

	int filter_CID = -1;
	auto iterCID = g_params.find("CID");
	if (iterCID != g_params.end())
	{
		sp = iterCID->second.c_str();
		ep = sp + iterCID->second.length();
		if (ConvertToInt((char*)sp, (char*)ep, i64Val) == true && i64Val >= 0 && i64Val <= UINT16_MAX)
			filter_CID = (int)i64Val;
	}

	int nParsedTLVPackets = 0;
	int nIPv4Packets = 0, nIPv6Packets = 0, nHdrCompressedIPPackets = 0, nTransmissionControlSignalPackets = 0, nNullPackets = 0, nOtherPackets = 0;
	TreeCIDPAMsgs CIDPAMsgs;
	CIDCATables CIDCATbls;
	CIDMHEITables CIDMHEITbls;
	std::vector<uint8_t> fullPAMessage;
	std::vector<uint8_t> fullCAMessage;
	std::vector<uint8_t> fullMHEIMessage;
	std::set<uint64_t> CID_EMM_packet_id_set;

	int nLocateSyncTry = 0;
	bool bFindSync = false;
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
				bFindSync = false;
				if (nLocateSyncTry > 10)
				{
					printf("[MMT/TLV] TLV header should start with 0x7F at file position: %" PRIu64 ".\n", bs.Tell() >> 3);
					nRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
					break;
				}
				else
				{
					bs.SkipBits(8);
					continue;
				}
			}
			else
			{
				if (bFindSync == false)
				{
					nLocateSyncTry++;
					bFindSync = true;
				}
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
				pTLVPacket = new MMT::TransmissionControlSignalPacket();
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
				printf("Failed to unpack a TLV packet at file position: %" PRIu64 ".\n", pTLVPacket->start_bitpos >> 3);
				delete pTLVPacket;
				break;
			}

			if (left_bits < (((uint64_t)pTLVPacket->Data_length + 4ULL) << 3))
			{
				printf("The buffer is not enough(left-bits: 0x%" PRIX64 ", required bits: 0x%" PRIX32 ")!\n",
					left_bits, (uint32_t)(((uint32_t)pTLVPacket->Data_length + 4UL) << 3));
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
				if (filter_CID >= 0 && filter_CID != CID)
					goto Skip;

				if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == 0x1)	// Process CA message
				{
					ShowInfoProcessCAMessage(CIDCATbls, CID, pHeaderCompressedIPPacket, fullCAMessage, CID_EMM_packet_id_set, dumpopt);
				}
				else if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == 0x8000)
				{
					ShowInfoProcessMHEIMessage(CIDMHEITbls, CID, pHeaderCompressedIPPacket, fullMHEIMessage, dumpopt);
				}
				else
				{
					ShowInfoProcessPAMessage(CIDPAMsgs, CID, pHeaderCompressedIPPacket, fullPAMessage,fitler_packet_ids, dumpopt);
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

	if (dumpopt&DUMP_MEDIA_INFO_VIEW)
	{
		printf("\nLayout of MMT:\n");

		// Print PLT, MPT and asset tree
		for (auto &m : CIDPAMsgs)
		{
			printf("\tCID: 0X%X(%d):\n", m.first, m.first);
			for (size_t i = 0; i < m.second.size(); i++)
			{
				auto& plt = std::get<0>(m.second[i]);
				auto& vmpt = std::get<1>(m.second[i]);
				if (plt != NULL)
					printf("\t\t#%04" PRIu32 " PLT (Package List Table):\n", (uint32_t)i);
				for (size_t j = 0; j < vmpt.size(); j++)
				{
					auto& mpt = vmpt[j];

					std::string szLocInfo;
					// Get the packet_id of current MPT
					if (plt != nullptr)
					{
						for (auto& plt_pkg : plt->package_infos)
						{
							if (std::get<1>(plt_pkg) == mpt->MMT_package_id)
							{
								auto& info = std::get<2>(plt_pkg);
								szLocInfo = info.GetLocDesc();
							}
						}
					}

					const char* sztab = plt != nullptr ? "\t\t\t" : "\t\t";

					printf("%s--------------------------------------------------------------------------------\n", sztab);
					printf("%s#%05" PRIu32 " MPT (MMT Package Table), Package_id: 0X%" PRIX64 "(%" PRIu64 "), %s:\n", 
						sztab, (uint32_t)j, mpt->MMT_package_id, mpt->MMT_package_id, szLocInfo.c_str());
					for (size_t k = 0; k < mpt->assets.size(); k++)
					{
						auto& a = mpt->assets[k];
						printf("%s\t#%05" PRIu32 " Asset, asset_id: 0X%" PRIX64 "(%" PRIu64 "), asset_type: %c%c%c%c(0X%08" PRIX32 "):\n", 
							sztab, (uint32_t)k, a->asset_id, a->asset_id,
							isprint((a->asset_type >> 24) & 0xFF) ? ((a->asset_type >> 24) & 0xFF) : '.',
							isprint((a->asset_type >> 16) & 0xFF) ? ((a->asset_type >> 16) & 0xFF) : '.',
							isprint((a->asset_type >> 8) & 0xFF) ? ((a->asset_type >> 8) & 0xFF) : '.',
							isprint((a->asset_type) & 0xFF) ? ((a->asset_type) & 0xFF) : '.',
							a->asset_type);

						for (auto& info : a->MMT_general_location_infos)
						{
							printf("%s\t\t%s\n", sztab, info.GetLocDesc().c_str());
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
							case 0x8020:
								pDescr = new MMT::MHDataComponentDescriptor();
								break;
							default:
								pDescr = new MMT::UnimplMMTSIDescriptor();
							}

							if (pDescr->Unpack(descs_bs) >= 0)
							{
								if (peek_desc_tag == 0x8010 || peek_desc_tag == 0x8014 || peek_desc_tag == 0x8020)
									pDescr->Print(stdout, plt ? 40 : 32);

								if (left_descs_bytes < (size_t)pDescr->descriptor_length + 3UL)
									break;

								left_descs_bytes -= (size_t)pDescr->descriptor_length + 3UL;

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
	for (const auto& iter : MMT::MMTPPacket::MMTP_packet_counts)
	{
		char pkt_desc[256] = { 0 };
		uint16_t pkt_id = (iter.first & 0xFFFF);
		const auto& iterAsset = MMT::MMTPPacket::MMTP_packet_asset_types.find(iter.first);
		if (iterAsset != MMT::MMTPPacket::MMTP_packet_asset_types.end())
		{
			for (const auto a : iterAsset->second)
			{
				MBCSPRINTF_S(pkt_desc, sizeof(pkt_desc), "%s%s", 
					pkt_desc[0] == '\0' ? "" : ",", 
					a == 'hev1' || a == 'hvc1'?" - HEVC Video Stream":(
					a == 'mp4a'?" - MPEG-4 AAC Audio Stream":(
					a == 'stpp'?" - Timed text":(
					a == 'aapp'?" - Application":(
					a == 'asgd'?" - Synchronous type general-purpose data":(
					a == 'aagd'?" - Asynchronous type general-purpose data":(
					a == 'mmpt'?" - MPT":"")))))));
			}
		}
		else
		{
			if (CID_EMM_packet_id_set.find((uint64_t)iter.first) != CID_EMM_packet_id_set.end())
				MBCSPRINTF_S(pkt_desc, sizeof(pkt_desc), "%s%s", pkt_desc[0] == '\0' ? "" : ",", " - EMM");
			else
			{
				MBCSPRINTF_S(pkt_desc, sizeof(pkt_desc), "%s%s",
					pkt_desc[0] == '\0' ? "" : ",",
					(pkt_id) == 0x0000?" - PA Message":(
					(pkt_id) == 0x0001?" - CA Message":(
					(pkt_id) == 0x8000?" - MH-EIT":(
					(pkt_id) == 0x8001?" - MH-AIT":(
					(pkt_id) == 0x8002?" - MH-BIT":(
					(pkt_id) == 0x8003?" - H-SDTT":(
					(pkt_id) == 0x8004?" - MH-SDT":(
					(pkt_id) == 0x8005?" - MH-TOT":(
					(pkt_id) == 0x8006?" - MH-CDT":(
					(pkt_id) == 0x8007?" - Data Message":(
					(pkt_id) == 0x8008?" - MH-DIT":(
					(pkt_id) == 0x8009?" - MH-SIT":""))))))))))));
			}
		}

		printf("    CID: 0x%04x, packet_id: 0x%04X, count: %26s%s\n", (iter.first >> 16) & 0xFFFF, iter.first & 0xFFFF,GetReadableNum(iter.second).c_str(), pkt_desc);
	}
	printf("The number of Transmission Control Signal TLV packets: %d.\n", nTransmissionControlSignalPackets);
	printf("The number of Null TLV packets: %d.\n", nNullPackets);
	printf("The number of other TLV packets: %d.\n", nOtherPackets);

	return nRet;
}

void NotifyAUStartPoint(TM_90KHZ pts, TM_90KHZ dts, const PROCESS_DATA_INFO* data_info, const ACCESS_UNIT_INFO* au_info, void* pCtx)
{
	static int64_t idx_AU_Notified = 0;

	char szAUItem[256] = { 0 };
	size_t ccLog = sizeof(szAUItem) / sizeof(szAUItem[0]);

	CODEC_ID codec_id = CODEC_ID_UNKNOWN;
	CESRepacker* pESRepacker = (CESRepacker*)pCtx;
	
	if (pESRepacker != nullptr)
	{
		codec_id = pESRepacker->GetConfig().codec_id;
	}

	const char* au_type_name = (codec_id == CODEC_ID_V_MPEGH_HEVC ? HEVC_PICTURE_TYPE_NAMEA(au_info->picture_type) : (
								codec_id == CODEC_ID_V_MPEG4_AVC ? AVC_PIC_SLICE_TYPE_NAMEA(au_info->avc_picture_slice_type) : nullptr));

	int ccWritten = 0,ccWrittenOnce = 0;
	if (au_type_name == nullptr)
	{
		ccWrittenOnce = MBCSPRINTF_S(szAUItem, ccLog, "#%-10" PRId64 ", MPU/packet sequence number: 0x%08X/0x%08X",
			idx_AU_Notified,
			data_info->MPU_sequence_number, data_info->packet_sequence_number);
	}
	else
	{
		ccWrittenOnce = MBCSPRINTF_S(szAUItem, ccLog, "#%-10" PRId64 ", MPU/packet sequence number: 0x%08X/0x%08X, %-8s",
			idx_AU_Notified,
			data_info->MPU_sequence_number, data_info->packet_sequence_number, au_type_name);
	}

	if (ccWrittenOnce > 0)
		ccWritten += ccWrittenOnce;

	if (ccWrittenOnce > 0)
	{
		if (pts >= 0 && pts <= 0x1FFFFFFFFLL)
			ccWrittenOnce = MBCSPRINTF_S(szAUItem + ccWritten, ccLog - ccWritten, ", pts: 0x%09" PRIX64 "(%10" PRId64 ")", pts, pts);
		else
			ccWrittenOnce = MBCSPRINTF_S(szAUItem + ccWritten, ccLog - ccWritten, ", pts: %-23s", "N/A");

		if (ccWrittenOnce > 0)
			ccWritten += ccWrittenOnce;
	}

	if (ccWrittenOnce > 0)
	{
		if (dts >= 0 && dts <= 0x1FFFFFFFFLL)
			ccWrittenOnce = MBCSPRINTF_S(szAUItem + ccWritten, ccLog - ccWritten, ", dts: 0x%09" PRIX64 "(%10" PRId64 ")", dts, dts);
		else
			ccWrittenOnce = MBCSPRINTF_S(szAUItem + ccWritten, ccLog - ccWritten, ", dts: %-23s", "N/A");

		if (ccWrittenOnce > 0)
			ccWritten += ccWrittenOnce;
	}

	printf("%s.\n", szAUItem);

	idx_AU_Notified++;
}

int DumpMMTOneStream()
{
	int nRet = RET_CODE_SUCCESS;
	int nFilterTLVPackets = 0, nFilterMFUs = 0, nParsedTLVPackets = 0;
	TreeCIDPAMsgs CIDPAMsgs;
	CIDCATables CIDCATbls;
	std::vector<uint8_t> fullPAMessage;
	std::vector<uint8_t> fullCAMessage;
	uint32_t asset_type[2] = { 0, 0 };
	CESRepacker* pESRepacker[2] = { nullptr };
	int filter_asset_idx = -1;
	std::set<uint64_t> CID_MPT_packet_id_set;
	std::set<uint64_t> CID_EMM_packet_id_set;
	MPUTMTrees MPU_tm_trees;
	int dumpOptions = 0;

	auto start_time = std::chrono::high_resolution_clock::now();

	if (g_params.find("input") == g_params.end())
		return -1;

	const char* sp;
	const char* ep;
	int64_t i64Val = -1LL;
	int PIDN = -1;
	uint32_t src_packet_id[2] = { UINT32_MAX, UINT32_MAX };
	std::string Outputfiles[2];
	auto pidIter = g_params.find("pid");
	auto iterParam = g_params.find("output");
	bool bDumpMPU = false;
	if (pidIter != g_params.end())
	{
		size_t iterPID = pidIter->second.find("&");
		if (iterPID == 0 || iterPID == std::string::npos)//Only one pid this case
		{
			if (ConvertToInt(pidIter->second, i64Val) == false || i64Val < 0 || i64Val > UINT16_MAX)
			{
				printf("Please specify a valid packet_id.\n");
				return RET_CODE_ERROR;
			}
			if (iterParam != g_params.end())
				Outputfiles[0] = iterParam->second;
			src_packet_id[0] = (uint16_t)i64Val;
			bDumpMPU = (src_packet_id[0] >= 0x9000 && src_packet_id[0] <= 0xFFFF);
			PIDN = 0;
		}
		else//Two packet_ids
		{
			std::string PIDs;
			PIDs = pidIter->second.substr(0, iterPID);
			sp = PIDs.c_str();
			ep = sp + PIDs.length();
			if (ConvertToInt((char*)sp, (char*)ep, i64Val) == false || i64Val < 0 || i64Val > UINT16_MAX)
			{
				printf("Please specify a valid packet_id.\n");
				return RET_CODE_ERROR;
			}
			src_packet_id[0] = (uint16_t)i64Val;
			if (iterParam != g_params.end())
			{
				Outputfiles[0] = iterParam->second;
				if (Outputfiles[0].rfind(".") == std::string::npos)// No dot in output file name
					Outputfiles[0] += ".";
				Outputfiles[1] = Outputfiles[0];
				Outputfiles[0].insert(Outputfiles[0].rfind("."), "_" + PIDs);
			}

			PIDs = pidIter->second.substr(iterPID + 1, pidIter->second.length());
			sp = PIDs.c_str();
			ep = sp + PIDs.length();
			if (ConvertToInt((char*)sp, (char*)ep, i64Val) == false || i64Val < 0 || i64Val > UINT16_MAX)
			{
				printf("Please specify a valid packet_id for the second pid.\n");
				return RET_CODE_ERROR;
			}
			src_packet_id[1] = (uint16_t)i64Val;
			if (iterParam != g_params.end())
				Outputfiles[1].insert(Outputfiles[1].rfind("."), "_" + PIDs);
			PIDN = 1;

			for (int i = 0; i <= PIDN; i++)
			{
				if ((bDumpMPU = (src_packet_id[i] >= 0x9000 && src_packet_id[i] <= 0xFFFF)))
					break;
			}
		}
	}

	int filter_CID = -1;
	auto iterCID = g_params.find("CID");
	if (iterCID != g_params.end())
	{
		sp = iterCID->second.c_str();
		ep = sp + iterCID->second.length();
		if (ConvertToInt((char*)sp, (char*)ep, i64Val) == true && i64Val >= 0 && i64Val <= UINT16_MAX)
			filter_CID = (int)i64Val;
	}

	std::string& szInputFile = g_params["input"];
	bool bExplicitVideoStreamDump = g_params.find("video") == g_params.end() ? false : true;
	uint32_t video_asset_type = -1;
	if (bExplicitVideoStreamDump)
	{
		if (MBCSICMP(g_params["video"].c_str(), "hvc1") == 0)
			video_asset_type = 'hvc1';
		else if (MBCSICMP(g_params["video"].c_str(), "hev1") == 0)
			video_asset_type = 'hev1';
	}

	bool bListMMTPpacket = g_params.find("listMMTPpacket") != g_params.end();
	bool bListMMTPpayload = g_params.find("listMMTPpayload") != g_params.end();

	int64_t filter_MPUSeqNumber = -1LL;
	auto iterMPUSeqNo = g_params.find("MPUseqno");
	if (iterMPUSeqNo != g_params.end())
	{
		if (ConvertToInt(iterMPUSeqNo->second, i64Val) && i64Val >= 0 && i64Val <= UINT32_MAX)
			filter_MPUSeqNumber = i64Val;
	}

	int64_t filter_PKTSeqNumber = -1LL;
	auto iterPKTSeqNo = g_params.find("PKTseqno");
	if (iterPKTSeqNo != g_params.end())
	{
		if (ConvertToInt(iterPKTSeqNo->second, i64Val) && i64Val >= 0 && i64Val <= UINT32_MAX)
			filter_PKTSeqNumber = i64Val;
	}

	bool bShowDU = g_params.find("showDU") != g_params.end();

	bool bOutputByMFU = g_params.find("MFU") != g_params.end();

	if (filter_CID >= 0)
	{
		for (int i = 0; i <= PIDN; i++)
		{
			if (src_packet_id[i] > 0 && src_packet_id[i] <= UINT16_MAX)
				MPU_tm_trees.emplace((filter_CID << 16) | src_packet_id[0], MPUTMTree());
		}
	}

	int nShowFullListMPUtime = 0;	// 1: listMPUtime command is issued, 2: need show full list
	auto iterListMPUtime = g_params.find("listMPUtime");
	if (iterListMPUtime != g_params.end())
	{
		if (MBCSICMP(iterListMPUtime->second.c_str(), "full") == 0)
			nShowFullListMPUtime = 2;
		else
			nShowFullListMPUtime = 1;
	}

	bool bShowPTS = false;
	auto iterShowPTS = g_params.find("showpts");
	if (iterShowPTS != g_params.end())
	{
		if (iterCID == g_params.end())
		{
			printf("Please specify the CID.\n");
			return -1;
		}

		if (pidIter == g_params.end())
		{
			printf("Please specify the packet_id.\n");
			return -1;
		}

		if (PIDN >= 1)
		{
			printf("Only support show the pts for the only one stream.\n");
			return -1;
		}

		bShowPTS = true;
	}

	auto iterStart = g_params.find("start");
	auto iterEnd = g_params.find("end");

	int64_t nStart = -1LL, nEnd = INT64_MAX, iVal = -1LL;
	if (iterStart != g_params.end())
	{
		iVal = ConvertToLongLong(iterStart->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nStart = (uint32_t)iVal;
	}

	if (iterEnd != g_params.end())
	{
		iVal = ConvertToLongLong(iterEnd->second);
		if (iVal >= 0 && iVal <= UINT32_MAX)
			nEnd = (uint32_t)iVal;
	}

	CFileBitstream bs(szInputFile.c_str(), 4096, &nRet);

	if (nRet < 0)
	{
		printf("Failed to open the file: %s.\n", szInputFile.c_str());
		return nRet;
	}

	int nLocateSyncTry = 0;
	bool bFindSync = false;
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
				bFindSync = false;
				if (nLocateSyncTry > 10)
				{
					printf("[MMT/TLV] TLV header should start with 0x7F at file position: %" PRIu64 ".\n", bs.Tell() >> 3);
					nRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
					break;
				}
				else
				{
					bs.SkipBits(8);
					continue;
				}
			}
			else
			{
				if (bFindSync == false)
				{
					nLocateSyncTry++;
					bFindSync = true;
				}
			}

			MMT::TLVPacket* pTLVPacket = nullptr;
			switch ((tlv_hdr >> 16) & 0xFF)
			{
			case MMT::TLV_Header_compressed_IP_packet:
				pTLVPacket = new MMT::HeaderCompressedIPPacket();
				break;
			default:
				pTLVPacket = new MMT::Undefined_TLVPacket();
			}

			if ((nRet = pTLVPacket->Unpack(bs)) < 0)
			{
				printf("Failed to unpack a TLV packet at file position: %" PRIu64 ".\n", pTLVPacket->start_bitpos >> 3);
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
				bool bFiltered = false;
				MMT::HeaderCompressedIPPacket* pHeaderCompressedIPPacket = (MMT::HeaderCompressedIPPacket*)pTLVPacket;

				uint16_t CID = pHeaderCompressedIPPacket->Context_id;
				if (filter_CID >= 0 && filter_CID != CID)
					goto Skip;

				// Check whether it is a control message MMT packet
				if (pHeaderCompressedIPPacket->MMTP_Packet == nullptr || (
					(PIDN >= 0) &&
					(pHeaderCompressedIPPacket->MMTP_Packet->Payload_type != 2) &&
					(pHeaderCompressedIPPacket->MMTP_Packet->Packet_id != src_packet_id[0]&&
					 pHeaderCompressedIPPacket->MMTP_Packet->Packet_id != src_packet_id[1])))
					goto Skip;

				if (PIDN == -1 ||
					pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == src_packet_id[0] ||
					pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == src_packet_id[1])
				{
					bFiltered = true;
					nFilterTLVPackets++;
				}

				if (bListMMTPpacket && bFiltered)
				{
					if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number >= nStart &&
						pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number <  nEnd)
						pHeaderCompressedIPPacket->MMTP_Packet->PrintListItem();
				}

				if (filter_MPUSeqNumber != -1LL && pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 0)
				{
					if (filter_MPUSeqNumber != pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number)
						bFiltered = false;
				}

				// Only support MPU payload at present
				if (bListMMTPpayload)
				{
					if (bFiltered && pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 0)
					{
						if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number >= nStart &&
							pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number < nEnd)
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->PrintListItem();
					}

					if (pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 2)
					{
						bool isMPT = false;
						uint64_t CID_MPT_packet_id = (pHeaderCompressedIPPacket->Context_id << 16) | pHeaderCompressedIPPacket->MMTP_Packet->Packet_id;
						if (CID_MPT_packet_id_set.find(CID_MPT_packet_id) != CID_MPT_packet_id_set.end())
							isMPT = true;
						
						if (bFiltered/* || ((pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == 0 || isMPT) && bDumpMPU)*/)
						{
							if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number >= nStart &&
								pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number < nEnd)
								pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->PrintListItem(nullptr, 0, isMPT);
						}
					}
				}

				// Check whether need show the DU(s) in the MMTP payload
				if (bShowDU &&
					(filter_PKTSeqNumber == -1LL || filter_PKTSeqNumber == pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number) &&
					((src_packet_id[0] != UINT32_MAX && pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == src_packet_id[0]) ||
					 (src_packet_id[1] != UINT32_MAX && pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == src_packet_id[1])))
				{
					if (pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 0)
					{
						if ((filter_MPUSeqNumber == -1LL || filter_MPUSeqNumber == pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number) &&
							(pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number >= nStart && 
							 pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number <  nEnd))
						{
							if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->Fragment_type == 2)
							{
								printf("packet_sequence_number: 0x%08X, packet_id: 0x%04X, MPU sequence number: 0x%08X:\n",
									pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number,
									pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
									pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->MPU_sequence_number);

								size_t idxDU = 0;
								for (auto& du : pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->Data_Units)
								{
									if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->timed_flag)
										printf("    DataUnit#%zu(MF_sequence_number: 0X%08X, sample_number: 0x%08X, offset: 0x%08X, priority: 0x%02X, dependency_counter: 0x%02X):\n",
											idxDU, du.movie_fragment_sequence_number, du.sample_number, du.offset, du.priority, du.dependency_counter);
									else
										printf("    DataUnit#%zu(item_id: 0x%08X):\n", idxDU, du.item_id);

									print_mem(du.MFU_data_bytes.data(), (int)du.MFU_data_bytes.size(), 4);
									printf("\n");
									idxDU++;
								}
							}
						}
					}
					else if (pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 0x2 &&
							(pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number >= nStart && 
							 pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number <  nEnd))
					{
						printf("packet_sequence_number: 0x%08X, packet_id: 0x%04X, fragmentation_indicator: %6s, aggr: %d\n",
							pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number,
							pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0?"1+ Msg":(
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0?"Header":(
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0?"Middle":"Tail")),
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag);

						size_t idxMSG = 0;
						for (auto& msg : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
						{
							uint32_t len = std::get<0>(msg);
							auto& msg_bytes = std::get<1>(msg);
							printf("    Message#%zu(message length: %u):\n", idxMSG, len);
							print_mem(msg_bytes.data(), (int)msg_bytes.size(), 4);
							printf("\n");
							idxMSG++;
						}
					}
				}

				if (PIDN >= 0)
					filter_asset_idx = pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == src_packet_id[0] ? 0 : 1;

				if (pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 0 && PIDN >= 0)
				{
					// Create a new ES re-packer to repack the NAL unit to an Annex-B bitstream
					for (int i = 0; i <= PIDN; i++) {
						if (pESRepacker[i] == nullptr)
						{
							ES_REPACK_CONFIG config;
							memset(&config, 0, sizeof(config));

							ES_BYTE_STREAM_FORMAT dstESFmt = ES_BYTE_STREAM_RAW;
							if (asset_type[i] == 'hvc1' || asset_type[i] == 'hev1')
							{
								config.codec_id = CODEC_ID_V_MPEGH_HEVC;
								dstESFmt = ES_BYTE_STREAM_HEVC_ANNEXB;
							}
							else if (asset_type[i] == 'mp4a')
							{
								config.codec_id = CODEC_ID_A_MPEG4_AAC;
								dstESFmt = ES_BYTE_LOAS_AudioSyncStream;
							}
							else if (asset_type[i] == 'stpp')
								config.codec_id = CODEC_ID_MMT_ASSET_STPP;
							else if (asset_type[i] == 'aapp')
								config.codec_id = CODEC_ID_MMT_ASSET_AAPP;
							else if (asset_type[i] == 'asgd')
								config.codec_id = CODEC_ID_MMT_ASSET_ASGD;
							else if (asset_type[i] == 'aagd')
								config.codec_id = CODEC_ID_MMT_ASSET_AAGD;
							else
							{
								if (bExplicitVideoStreamDump && PIDN == 0 && (
									video_asset_type == 'hvc1' || video_asset_type == 'hev1'))
								{
									asset_type[i] = video_asset_type;
									config.codec_id = CODEC_ID_V_MPEGH_HEVC;
									dstESFmt = ES_BYTE_STREAM_HEVC_ANNEXB;
								}
								else
								//
								// Now skip the data before a valid MPT at default
								//
								//if (bExplicitVideoStreamDump)
								{
									// skip it
									if (Outputfiles[i].length() > 0)
									{
										printf("Video stream(asset_type: %d), need find known asset type for video stream at first!!!!\n", asset_type[i]);
									}

									continue;
								}
							}
							//memset(config.es_output_file_path, 0, sizeof(config.es_output_file_path));
							strcpy_s(config.es_output_file_path, _countof(config.es_output_file_path), Outputfiles[i].c_str());

							if (config.codec_id == CODEC_ID_V_MPEGH_HEVC || config.codec_id == CODEC_ID_V_MPEG4_AVC)
								pESRepacker[i] = new CNALRepacker(ES_BYTE_STREAM_NALUNIT_WITH_LEN, dstESFmt);
							else if(config.codec_id == CODEC_ID_A_MPEG4_AAC)
								pESRepacker[i] = new CMPEG4AACLOASRepacker(ES_BYTE_LATM_AudioMuxElement, dstESFmt);
							else
								pESRepacker[i] = new CESRepacker(ES_BYTE_STREAM_RAW, dstESFmt);

							if (bShowPTS)
								pESRepacker[i]->SetAUStartPointCallback(NotifyAUStartPoint, (void*)pESRepacker[i]);

							pESRepacker[i]->Config(config);
							pESRepacker[i]->Open(nullptr);
						}
					}

					for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->Data_Units)
					{
						uint8_t actual_fragmenttion_indicator = pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->Aggregate_flag == 1 ? 
							0 : (uint8_t)pHeaderCompressedIPPacket->MMTP_Packet->ptr_MPU->fragmentation_indicator;
						
						if (bFiltered)
						{
							if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number >= nStart &&
								pHeaderCompressedIPPacket->MMTP_Packet->Packet_sequence_number < nEnd)
							{
								ProcessMFU(pHeaderCompressedIPPacket, asset_type[filter_asset_idx],
									&m.MFU_data_bytes[0], (int)m.MFU_data_bytes.size(), pESRepacker[filter_asset_idx], &MPU_tm_trees);

								if (m.MFU_data_bytes.size() > 0 && g_verbose_level == 99)
								{
									fprintf(stdout, MMT_FIX_HEADER_FMT_STR ": %s \n", "", "NAL Unit",
										actual_fragmenttion_indicator == 0 ? "Complete" : (
										actual_fragmenttion_indicator == 1 ? "First" : (
										actual_fragmenttion_indicator == 2 ? "Middle" : (
										actual_fragmenttion_indicator == 3 ? "Last" : "Unknown"))));
									for (size_t i = 0; i < m.MFU_data_bytes.size(); i++)
									{
										if (i % 32 == 0)
											fprintf(stdout, "\n" MMT_FIX_HEADER_FMT_STR "  ", "", "");
										fprintf(stdout, "%02X ", m.MFU_data_bytes[i]);
									}
									fprintf(stdout, "\n");
								}
							}
						}

						if (actual_fragmenttion_indicator || actual_fragmenttion_indicator == 3)
							nFilterMFUs++;
					}
				}
				else if (pHeaderCompressedIPPacket->MMTP_Packet->Payload_type == 0x2)
				{
					if (pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == 0x1)	// Process CA message
					{
						// Check there is an existed CID there or not, if it does NOT exist, add one
						if (CIDCATbls.find(CID) == CIDCATbls.end())
						{
							CIDCATbls[CID] = CATables();
						}

						bool bCAMessageChanged = false;
						// Check whether the current CA message contains a complete message
						if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0 ||
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 3 ||
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag == 1)
						{
							if (fullCAMessage.size() > 0)
							{
								for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
								{
									auto& v = std::get<1>(m);
									if (v.size() > 0)
									{
										size_t orig_size = fullCAMessage.size();
										fullCAMessage.resize(orig_size + v.size());
										memcpy(&fullCAMessage[orig_size], &v[0], v.size());
									}
								}

								//print_mem(&fullCAMessage[0], (int)fullCAMessage.size(), 4);
								if (ProcessCAMessage(CIDCATbls, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
									&fullCAMessage[0], (int)fullCAMessage.size(), &bCAMessageChanged, dumpOptions) == 0 && bCAMessageChanged)
								{
									// TODO...
								}

								fullCAMessage.clear();
							}
							else
							{
								for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
								{
									bool bChanged = false;
									auto& v = std::get<1>(m);

									//print_mem(&v[0], (int)v.size(), 4);
									if (ProcessCAMessage(CIDCATbls, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
										&v[0], (int)v.size(), &bChanged, dumpOptions) == 0 && bChanged)
									{
										// TODO...
									}

									if (bChanged && bCAMessageChanged == false)
										bCAMessageChanged = true;
								}
							}

							if (bCAMessageChanged)
							{
								CID_EMM_packet_id_set.clear();
								for (const auto& CIDCA : CIDCATbls)
								{
									uint64_t CID_packet_id = ((uint64_t)CIDCA.first) << 16;

									for (const auto& entry : CIDCA.second)
									{
										MMT::CATable* pCATable = (MMT::CATable*)entry;
										for (const auto& desc : pCATable->descriptors)
										{
											if (desc->descriptor_tag == 0x8004)
											{
												MMT::AccessControlDescriptor* ac_desc = (MMT::AccessControlDescriptor*)desc;
												if (ac_desc->MMT_general_location_info.location_type == 0 ||
													ac_desc->MMT_general_location_info.location_type == 1 ||
													ac_desc->MMT_general_location_info.location_type == 2)
												{
													CID_packet_id = CID_packet_id & 0xFFFFFFFFFFFF0000ULL;
													CID_packet_id |= ac_desc->MMT_general_location_info.GetPacketID();
													CID_EMM_packet_id_set.insert(CID_packet_id);
												}
											}
											else
												continue;
										}
									}
								}
							}
						}
						else
						{
							assert(pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages.size() <= 1);
							// Need add the data into the ring buffer, and parse it later
							for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
							{
								auto& v = std::get<1>(m);
								if (v.size() > 0)
								{
									size_t orig_size = fullCAMessage.size();
									fullCAMessage.resize(orig_size + v.size());
									memcpy(&fullCAMessage[orig_size], &v[0], v.size());
								}
							}
						}
					}
					else
					{
						// Check there is an existed CID there or not, if it does NOT exist, add one
						if (CIDPAMsgs.find(CID) == CIDPAMsgs.end())
						{
							TreePLTMPT emptyTreePLTMAP;
							CIDPAMsgs[CID] = emptyTreePLTMAP;
						}

						bool bPAMessageChanged = false;
						// Check whether the current PA message contains a complete message
						if (pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 0 ||
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->fragmentation_indicator == 3 ||
							pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->Aggregate_flag == 1)
						{
							if (fullPAMessage.size() > 0)
							{
								for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
								{
									auto& v = std::get<1>(m);
									if (v.size() > 0)
									{
										size_t orig_size = fullPAMessage.size();
										fullPAMessage.resize(orig_size + v.size());
										memcpy(&fullPAMessage[orig_size], &v[0], v.size());
									}
								}

								bool bChanged = false;
								if (ProcessPAMessage(CIDPAMsgs, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
									&fullPAMessage[0], (int)fullPAMessage.size(), &bChanged,
									dumpOptions, nullptr, &MPU_tm_trees) == 0 && bChanged)
								{
									for (int i = 0; i <= PIDN; i++)
									{
										uint32_t new_asset_type = FindAssetType(CIDPAMsgs, CID, src_packet_id[i]);
										if (new_asset_type != 0 && new_asset_type != asset_type[i])
										{
											//printf("assert_type from 0x%X to 0x%X.\n", asset_type, new_asset_type);
											asset_type[i] = new_asset_type;
										}
									}
								}

								if (bChanged && pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == 0)
									bPAMessageChanged = true;

								fullPAMessage.clear();
							}
							else
							{
								for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
								{
									bool bChanged = false;
									auto& v = std::get<1>(m);
									if (ProcessPAMessage(CIDPAMsgs, CID, pHeaderCompressedIPPacket->MMTP_Packet->Packet_id,
										&v[0], (int)v.size(), &bChanged,
										dumpOptions, nullptr, &MPU_tm_trees) == 0 && bChanged)
									{
										for (int i = 0; i <= PIDN; i++)
										{
											uint32_t new_asset_type = FindAssetType(CIDPAMsgs, CID, src_packet_id[i]);
											if (new_asset_type != 0 && new_asset_type != asset_type[i])
											{
												//printf("assert_type from 0x%X to 0x%X.\n", asset_type, new_asset_type);
												asset_type[i] = new_asset_type;
											}
										}
									}

									if (bChanged && pHeaderCompressedIPPacket->MMTP_Packet->Packet_id == 0 && bPAMessageChanged == false)
										bPAMessageChanged = true;
								}
							}

							if (bPAMessageChanged)
							{
								CID_MPT_packet_id_set.clear();
								for (const auto& CIDPA : CIDPAMsgs)
								{
									uint64_t CID_packet_id = ((uint64_t)CIDPA.first) << 16;

									for (const auto& entry : CIDPA.second)
									{
										auto PLT = std::get<0>(entry);
										if (PLT == nullptr)
											continue;

										for (auto &pkg_info : PLT->package_infos)
										{
											auto& pkg_loc_info = std::get<2>(pkg_info);
											CID_packet_id = CID_packet_id & 0xFFFFFFFFFFFF0000ULL;
											if (pkg_loc_info.location_type == 0 ||
												pkg_loc_info.location_type == 1 ||
												pkg_loc_info.location_type == 2)
											{
												CID_packet_id |= pkg_loc_info.GetPacketID();
												CID_MPT_packet_id_set.insert(CID_packet_id);
											}
										}
									}
								}
							}
						}
						else
						{
							assert(pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages.size() <= 1);
							// Need add the data into the ring buffer, and parse it later
							for (auto& m : pHeaderCompressedIPPacket->MMTP_Packet->ptr_Messages->messages)
							{
								auto& v = std::get<1>(m);
								if (v.size() > 0)
								{
									size_t orig_size = fullPAMessage.size();
									fullPAMessage.resize(orig_size + v.size());
									memcpy(&fullPAMessage[orig_size], &v[0], v.size());
								}
							}
						}
					}
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

	for (int i = 0; i <= 1; i++) {
		if (pESRepacker[i] != nullptr)
		{
			pESRepacker[i]->Drain();

			pESRepacker[i]->Close();
			delete pESRepacker[i];
			pESRepacker[i] = nullptr;
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();

	printf("Total cost: %f ms\n", std::chrono::duration<double, std::milli>(end_time - start_time).count());

	printf("Total input TLV packets: %d.\n", nParsedTLVPackets);
	if (src_packet_id[0] != UINT32_MAX && src_packet_id[1] != UINT32_MAX)
	{
		printf("Total MMT/TLV packets with the packet_id(0X%X&0X%X): %d\n", src_packet_id[0], src_packet_id[1], nFilterTLVPackets);
		printf("Total MFUs in MMT/TLV packets with the packet_id(0X%X&0X%X): %d\n", src_packet_id[0], src_packet_id[1], nFilterMFUs);
	}
	else if (src_packet_id[0] != UINT32_MAX)
	{
		printf("Total MMT/TLV packets with the packet_id(0X%X): %d\n", src_packet_id[0], nFilterTLVPackets);
		printf("Total MFUs in MMT/TLV packets with the packet_id(0X%X): %d\n", src_packet_id[0], nFilterMFUs);
	}

	if (nShowFullListMPUtime > 0)
		PrintMPUTMTrees(MPU_tm_trees, nShowFullListMPUtime == 2 ? true : false);

	return nRet;
}

int DumpPartialMMT()
{
	printf("Not implemented!\n");
	return -1;
}

int RefactorMMT()
{
	printf("Not implemented!\n");
	return -1;
}

int DumpMMT()
{
	int nDumpRet = 0;

	auto iter_showpack = g_params.find("showpack");
	if ((iter_showpack) != g_params.end() ||
		(iter_showpack = g_params.find("showIPv4pack")) != g_params.end() ||
		(iter_showpack = g_params.find("showIPv6pack")) != g_params.end() ||
		(iter_showpack = g_params.find("showHCIPpack")) != g_params.end() ||
		(iter_showpack = g_params.find("showTCSpack")) != g_params.end() ||
		(iter_showpack = g_params.find("showNTP")) != g_params.end())
	{
		int64_t display_pages = DEFAULT_TLV_PACKETS_PER_DISPLAY;
		const char* szPages = iter_showpack->second.c_str();
		if (ConvertToInt((char*)szPages, (char*)szPages + iter_showpack->second.length(), display_pages))
		{
			if (display_pages <= 0)
				g_TLV_packets_per_display = std::numeric_limits<decltype(g_TLV_packets_per_display)>::max();
			else
				g_TLV_packets_per_display = decltype(g_TLV_packets_per_display)(display_pages);
		}

		SHOW_TLV_PACK_OPTION option = SHOW_TLV_ALL;
		if (STRICMP(iter_showpack->first.c_str(), "showIPv4pack") == 0)
		{
			option = SHOW_TLV_IPv4;
		}
		else if (STRICMP(iter_showpack->first.c_str(), "showIPv6pack") == 0)
		{
			option = SHOW_TLV_IPv6;
		}
		else if (STRICMP(iter_showpack->first.c_str(), "showHCIPpack") == 0)
		{
			option = SHOW_TLV_HCIP;
		}
		else if (STRICMP(iter_showpack->first.c_str(), "showTCSpack") == 0)
		{
			option = SHOW_TLV_TCS;
		}
		else if (STRICMP(iter_showpack->first.c_str(), "showNTP") == 0)
		{
			option = SHOW_TLV_NTP;
		}

		return ShowMMTTLVPacks(option);
	}
	else if (g_params.find("listMMTPpacket") == g_params.end() && 
			 g_params.find("listMMTPpayload") == g_params.end() &&
			(g_params.find("pid") == g_params.end() || (NeedShowMMTTable() && g_params.find("output") == g_params.end())))
	{
		if (g_params.find("output") == g_params.end())
		{
			// No output file is specified
			// Check whether pid and showinfo is there
			if (g_params.find("showinfo") != g_params.end() ||
				g_params.find("showPLT")  != g_params.end() ||
				g_params.find("showMPT")  != g_params.end() ||
				g_params.find("showCAT")  != g_params.end() ||
				g_params.find("showEIT")  != g_params.end())
			{
				ShowMMTPackageInfo();
				goto done;
			}
			else
			{
				printf("Please specify a output file name with --output.\n");
				goto done;
			}
		}
	}
	else if (g_params.find("pid") != g_params.end() || g_params.find("listMMTPpayload") != g_params.end() || g_params.find("listMMTPpacket") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0) || 
			g_params.find("listMMTPpacket") != g_params.end() ||
			g_params.find("listMMTPpayload") != g_params.end())
		{
			nDumpRet = DumpMMTOneStream();
		}
		else if (str_output_fmt.compare("mmt") == 0)
		{
			if (g_params.find("destpid") == g_params.end())
				nDumpRet = DumpPartialMMT();
			else
				nDumpRet = RefactorMMT();
		}
	}
	else
	{
		printf("Unsupported!\n");
	}

done:
	return nDumpRet;
}

