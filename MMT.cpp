#include "StdAfx.h"
#include "MMT.h"
#include <conio.h>
#include <ctype.h>

extern std::unordered_map<std::string, std::string> g_params;
extern int g_verbose_level;

int g_TLV_packets_per_display = 20;

int DumpMMT()
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
			if (((tlv_hdr>>24) & 0xFF) != 0x7F)
			{
				printf("[MMT/TLV] TLV header should start with 0x7F at file position: %llu.\n", bs.Tell()>>3);
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

const char* MMT::Table::MMT_SI_table_desc[256] = {
	"PA Table",
	"Subset 0 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Subset 1 MPI Table to subset 14 MPI Table",
	"Complete MPI Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Subset 0 MP Table to subset 14 MP Table",
	"Complete MP Table",
	"CRI Table",
	"DCI Table",
	"reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC",
	"reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC",
	"reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC",
	"reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC",
	"reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC",
	"reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC","reserved for ISO/IEC", "reserved for ISO/IEC","reserved for ISO/IEC",
	"PLT", "LCT", "ECM", "ECM", "EMM", "EMM", "CAT(MH)", "DCM", "DCM", "DMM", "DMM", "MH-EIT (present and next program of self-stream)",
	"MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)",
	"MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)", "MH-EIT (schedule of self-stream)",
	"MH-AIT (AIT controlled application)",
	"MH-BIT",
	"MH-SDTT",
	"MH-SDT (self-stream)",
	"MH-SDT (other stream)",
	"MH-TOT",
	"MH-CDT",
	"DDM Table",
	"DAM Table",
	"DCC Table",
	"EMT",
	"Reserved", "Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
	"Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
	"Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
	"Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved",
	"Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters",
	"Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters",
	"Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters",
	"Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters","Reserved for broadcasters",
};

const std::unordered_map<uint16_t, std::tuple<const char*, const char*>> MMT::MMT_SI_Descriptor_Descs = 
{
	{0x0000, {"CRI Descriptor", ""}},
	{0x0001, {"MPU Time stamp Descriptor", ""}},
	{0x0002, {"Dependency relationship Descriptor", ""}},
	{0x0003, {"GFDT Descriptor", ""}},
	{0x8000, {"Asset group Descriptor", ""}},
	{0x8001, {"Event package Descriptor", ""}},
	{0x8002, {"Background color designation Descriptor", ""}},
	{0x8003, {"MPU presentation area designation Descriptor", ""}},
	{0x8004, {"Access control Descriptor", ""}},
	{0x8005, {"Scramble system Descriptor", ""}},
	{0x8006, {"Message certification system Descriptor", ""}},
	{0x8007, {"Emergency information Descriptor (MH)", ""}},
	{0x8008, {"MH-MPEG-4 audio Descriptor", ""}},
	{0x8009, {"MH-MPEG-4 audio extension Descriptor", ""}},
	{0x800A, {"MH-HEVCvideo Descriptor", ""}},
	{0x800C, {"MH-Event group Descriptor", ""}},
	{0x800D, {"MH-Service list Descriptor", ""}},
	{0x8010, {"Video component Descriptor", ""}},
	{0x8011, {"MH-Stream identification Descriptor", ""}},
	{0x8012, {"MH-Content Descriptor", ""}},
	{0x8013, {"MH-Parental rate Descriptor", ""}},
	{0x8014, {"MH-Audio component Descriptor", ""}},
	{0x8015, {"MH-Object area Descriptor", ""}},
	{0x8016, {"MH-Series Descriptor", ""}},
	{0x8017, {"MH-SI transmission parameter Descriptor", ""}},
	{0x8018, {"MH-Broadcaster name Descriptor", "" } },
	{0x8019, {"MH-Service Descriptor", "" } },
	{0x801A, {"IPdata flow Descriptor", "" } },
	{0x801B, {"MH-CAstarting Descriptor", "" } },
	{0x801C, {"MH-Type Descriptor", "" } },
	{0x801D, {"MH-Info Descriptor", "" } },
	{0x801E, {"MH-Expire Descriptor", "" } },
	{0x801F, {"MH-CompressionType Descriptor", "" } },
	{0x8020, {"MH-Data coding system Descriptor", "" } },
	{0x8021, {"UTC-NPT reference Descriptor", "" } },
	{0x8023, {"MH-Local time offset Descriptor", "" } },
	{0x8024, {"MH-Component group Descriptor", "" } },
	{0x8025, {"MH-Logo transmission Descriptor", "" } },
	{0x8026, {"MPU Extension time stamp Descriptor", "" } },
	{0x8027, {"MPU download content Descriptor", "" } },
	{0x8028, {"MH-Network download content Descriptor", "" } },
	{0x8029, {"MH-Application Descriptor", "" } },
	{0x802A, {"MH-Transmission protocol Descriptor", "" } },
	{0x802B, {"MH-Simple application location Descriptor", "" } },
	{0x802C, {"MH-Application boundary authority setting Descriptor", "" } },
	{0x802D, {"MH-Starting priority information Descriptor", "" } },
	{0x802E, {"MH-Cache information Descriptor", "" } },
	{0x802F, {"MH-Probabilistic application delay Descriptor", "" } },
	{0x8030, {"Link destination PU Descriptor", "" } },
	{0x8031, {"Lock cache designation Descriptor", "" } },
	{0x8032, {"Unlock cache designation Descriptor", "" } },
	{0x8033, {"MH-Download protection Descriptor*3", "" } },
	{0x8034, {"Application service Descriptor", "" } },
	{0x8035, {"MPU node Descriptor", "" } },
	{0x8036, {"PU configuration Descriptor", "" } },
	{0x8037, {"MH-Layered coding Descriptor", "" } },
	{0x8038, {"Content copy control Descriptor", "" } },
	{0x8039, {"Content usage control Descriptor", "" } },
	{0x803A, {"MH-External application control Descriptor", "" } },
	{0x803B, {"MH-Video recording and reproduction application Descriptor", "" } },
	{0x803C, {"MH-Simple video recording and reproduction application location Descriptor", "" } },
	{0x803D, {"MH-Application valid term Descriptor", "" } },
	{0x803E, {"Related broadcaster Descriptor", "" } },
	{0x803F, {"Multimedia service information Descriptor", "" } },
	{0x8040, {"Emergency news Descriptor", "" } },
	{0x8041, {"MH-CA contract information Descriptor", "" } },
	{0x8042, {"MH-CA service Descriptor", "" } },
	{0xF000, {"MH-Link Descriptor", "" } },
	{0xF001, {"MH-Short format event Descriptor", "" } },
	{0xF002, {"MH-Extension format event Descriptor", "" } },
	{0xF003, {"Event message Descriptor", "" } },
};