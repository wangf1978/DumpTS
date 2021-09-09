#include "StdAfx.h"
#include "MMT.h"

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

std::unordered_map<unsigned char, std::string> g_TLV_SI_descriptors = {
	{0x40, "Network Name Descriptor"},
	{0x41, "Service List Descriptor"},
	{0x43, "Satellite Delivery System Descriptor"},
	{0xCD, "Remote Control Key Descriptor"},
	{0xFE, "System Management Descriptor"},
};

namespace MMT
{

	void PrintPacketIDAssignment()
	{
		printf("\n");
		printf("*****************************************************************************\n");
		printf("          ** Assignment of Packet ID of MMTP transmitting message **\n");
		printf("*****************************************************************************\n");
		printf("            Message                                       Packet ID\n");
		printf("-----------------------------------------------------------------------------\n");
		printf("PA message                                        0x0000 or indirect designation by PLT\n");
		printf("CA message                                        0x0001\n");
		printf("M2 section message which stores ECM*              Indirect designation by MPT\n");
		printf("M2 section message which stores EMM*              Indirect designation by CAT\n");
		printf("M2 section message which stores DCM*              Indirect designation by MPT\n");
		printf("M2 section message which stores DMM*              Indirect designation by MH - SDTT\n");
		printf("M2 section message which stores MH-EIT            0x8000\n");
		printf("M2 section message which stores MH-AIT            0x8001 or indirect designation by MPT\n");
		printf("M2 section message which stores MH-BIT            0x8002\n");
		printf("M2 section message which stores MH-SDTT           0x8003\n");
		printf("M2 section message which stores MH-SDT            0x8004\n");
		printf("M2 short section message which stores MH-TOT      0x8005\n");
		printf("M2 section message which stores MH-CDT            0x8006\n");
		printf("Data transmission message                         0x8007 or indirect designation by MPT\n");
		printf("M2 short section message which stores MH-DIT      0x8008\n");
		printf("M2 section message which stores MH-SIT            0x8009\n");
		printf("M2 section message which stores EMT               Indirect designation by MPT\n");

		printf("\n");
		printf("*****************************************************************************\n");
		printf("                     ** Assignment of Packet ID of MMTP **\n");
		printf("*****************************************************************************\n");
		printf("Packet ID                                         Meaning of Packet ID\n");
		printf("-----------------------------------------------------------------------------\n");
		printf("0x0000                                            PA message\n");
		printf("0x0001                                            CA message\n");
		printf("0x0002                                            AL-FEC message\n");
		printf("0x0003 每 0x00FF                                   Undefined\n");
		printf("0x0100 每 0x7FFF                                   Provided by the Ministry or private standardization organization\n"
			   "                                                  (region which can be assigned except for control message)\n");
		printf("0x8000                                            M2 section message (MH-EIT is stored)\n");
		printf("0x8001                                            M2 section message (MH-AIT is stored)\n");
		printf("0x8002                                            M2 section message (MH-BIT is stored)\n");
		printf("0x8003                                            M2 section message (H-SDTT is stored)\n");
		printf("0x8004                                            M2 section message (MH-SDT is stored)\n");
		printf("0x8005                                            M2 short section message (MH-TOT is stored)\n");
		printf("0x8006                                            M2 section message (MH-CDT is stored)\n");
		printf("0x8007                                            Data transmission message\n");
		printf("0x8008                                            M2 short section message (MH-DIT is stored)\n");
		printf("0x8009                                            M2 section message (MH-SIT is stored)\n");
		printf("0x800A 每 0x8FFF                                   Reserved (provided by the Ministry or private standardization organization)\n");
		printf("0x9000 每 0xFFFF                                   prepared by broadcasters\n");
	}
}
