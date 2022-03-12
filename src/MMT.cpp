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
	std::unordered_map<uint32_t/*(CID<<16) | packet_id*/, uint64_t> MMTPPacket::MMTP_packet_counts;
	std::unordered_map<uint32_t/*(CID<<16) | packet_id*/, std::set<uint32_t>> MMTPPacket::MMTP_packet_asset_types;

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
		printf("0x0003 - 0x00FF                                   Undefined\n");
		printf("0x0100 - 0x7FFF                                   Provided by the Ministry or private standardization organization\n"
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
		printf("0x800A - 0x8FFF                                   Reserved (provided by the Ministry or private standardization organization)\n");
		printf("0x9000 - 0xFFFF                                   prepared by broadcasters\n");
	}

	void PrintMMTSITable()
	{
		printf("\n");
		printf("*****************************************************************************\n");
		printf("             ** Assignment of identifier of table of MMT-SI **\n");
		printf("*****************************************************************************\n");
		printf("Table ID                                          Table Name\n");
		printf("-----------------------------------------------------------------------------\n");
		printf("0x00                                              PA Table                                        \n");
		printf("0x01                                              Subset 0 MPI Table                              \n");
		printf("0x02 - 0x0F                                       Subset 1 MPI Table to subset 14 MPI Table       \n");
		printf("0x10                                              Complete MPI Table                              \n");
		printf("0x11 - 0x1F                                       Subset 0 MP Table to subset 14 MP Table         \n");
		printf("0x20                                              Complete MP Table                               \n");
		printf("0x21                                              CRI Table                                       \n");
		printf("0x22                                              DCI Table                                       \n");
		printf("0x23 - 0x7F                                       reserved for ISO/IEC (16-bit length table)      \n");
		printf("0x80                                              PLT                                             \n");
		printf("0x81                                              LCT                                             \n");
		printf("0x82 - 0x83                                       ECM*1                                           \n");
		printf("0x84 - 0x85                                       EMM*1                                           \n");
		printf("0x86                                              CAT (MH)                                        \n");
		printf("0x87 - 0x88                                       DCM                                             \n");
		printf("0x89 - 0x8A                                       DMM                                             \n");
		printf("0x8B                                              MH-EIT (present and next program of self-stream)\n");
		printf("0x8C - 0x9B                                       MH-EIT (schedule of self-stream)                \n");
		printf("0x9C                                              MH-AIT (AIT controlled application)             \n");
		printf("0x9D                                              MH-BIT                                          \n");
		printf("0x9E                                              MH-SDTT                                         \n");
		printf("0x9F                                              MH-SDT (self-stream)                            \n");
		printf("0xA0                                              MH-SDT (other stream)                           \n");
		printf("0xA1                                              MH-TOT                                          \n");
		printf("0xA2                                              MH-CDT                                          \n");
		printf("0xA3                                              DDM Table                                       \n");
		printf("0xA4                                              DAM Table                                       \n");
		printf("0xA5                                              DCC Table                                       \n");
		printf("0xA6                                              EMT                                             \n");
		printf("0xA7                                              MH-DIT                                          \n");
		printf("0xA8                                              MH-SIT                                          \n");
		printf("0xA9 - 0xDF                                       Reserved (provided by the Ministry or private standardization organization)\n");
		printf("0xE0 - 0xFF                                       Table which is prepared by broadcasters\n");
	}

	void PrintMMTSIMessage()
	{
		printf("\n");
		printf("*****************************************************************************\n");
		printf("             ** Assignment of message identifier of MMT-SI **\n");
		printf("*****************************************************************************\n");
		printf("Message ID                 Message\n");
		printf("-----------------------------------------------------------------------------\n");
		printf("0x0000                     PA message\n");
		printf("0x0001 - 0x000F            MPI message\n");
		printf("0x0010 - 0x001F            MPT message\n");
		printf("0x0200                     CRI message\n");
		printf("0x0201                     DCI message\n");
		printf("0x0202                     AL-FEC message\n");
		printf("0x0203                     HRBM message\n");
		printf("0x0204 - 0x6FFF            reserved for ISO/IEC (16-bit length message)\n");
		printf("0x7000 - 0x7FFF            reserved for ISO/IEC (32-bit length message)\n");
		printf("0x8000                     M2 section message*1\n");
		printf("0x8001                     CA message*1\n");
		printf("0x8002                     M2 short section message\n");
		printf("0x8003                     Data transmission message\n");
		printf("0x8004 - 0xDFFF            reserved (message whose length field is 16 bits)\n"
			   "                           (provided by the Ministry or private standardization organization)\n");
		printf("0xE000 - 0xEFFF            message which is prepared by broadcasters (message whose length field is 16 bits)\n");
		printf("0xF000 - 0xF7FF            reserved (message whose length field is 32 bits)\n"
			   "                           (provided by the Ministry or private standardization organization)\n");
		printf("0xF800 - 0xFFFF            message which is prepared by broadcasters (message whose length field is 32 bits)\n");
	}

	void PrintMMTSIDescriptor()
	{
		printf("\n");
		printf("*****************************************************************************\n");
		printf("             ** Assignment of descriptor tag of MMT-SI **\n");
		printf("*****************************************************************************\n");
		printf("Descriptor tag value                              Descriptor name\n");
		printf("-----------------------------------------------------------------------------\n");

		printf("0x0000                                            CRI Descriptor*2\n");
		printf("0x0001                                            MPU Time stamp Descriptor*1\n");
		printf("0x0002                                            Dependency relationship Descriptor *1\n");
		printf("0x0003                                            GFDT Descriptor*2\n");
		printf("0x0004 - 0x3FFF                                   reserved for ISO/IEC (8-bit length descriptor)\n");
		printf("0x4000 - 0x6FFF                                   reserved for ISO/IEC (16-bit length descriptor)\n");
		printf("0x7000 - 0x7FFF                                   reserved for ISO/IEC (32-bit length descriptor)\n");
		printf("0x8000                                            Asset group Descriptor\n");
		printf("0x8001                                            Event package Descriptor\n");
		printf("0x8002                                            Background color designation Descriptor\n");
		printf("0x8003                                            MPU presentation area designation Descriptor\n");
		printf("0x8004                                            Access control Descriptor*1\n");
		printf("0x8005                                            Scramble system Descriptor*1\n");
		printf("0x8006                                            Message certification system Descriptor\n");
		printf("0x8007                                            Emergency information Descriptor (MH)*1\n");
		printf("0x8008                                            MH-MPEG-4 audio Descriptor\n");
		printf("0x8009                                            MH-MPEG-4 audio extension Descriptor\n");
		printf("0x800A                                            MH-HEVC video Descriptor\n");
		printf("0x800B                                            Reserved (those of which descriptor length field is 8 bits)\n"
			   "                                                  (provided by the Ministry or private standardization organization)\n");
		printf("0x800C                                            MH-Event group Descriptor\n");
		printf("0x800D                                            MH-Service list Descriptor\n");
		printf("0x800E - 0x800F                                   Reserved (those of which descriptor length field is 8 bits)\n"
			   "                                                  (provided by the Ministry or private standardization organization)\n");
		printf("0x8010                                            Video component Descriptor\n");
		printf("0x8011                                            MH-Stream identification Descriptor\n");
		printf("0x8012                                            MH-Content Descriptor\n");
		printf("0x8013                                            MH-Parental rate Descriptor\n");
		printf("0x8014                                            MH-Audio component Descriptor\n");
		printf("0x8015                                            MH-Object area Descriptor\n");
		printf("0x8016                                            MH-Series Descriptor\n");
		printf("0x8017                                            MH-SI transmission parameter Descriptor\n");
		printf("0x8018                                            MH-Broadcaster name Descriptor\n");
		printf("0x8019                                            MH-Service Descriptor\n");
		printf("0x801A                                            IP data flow Descriptor\n");
		printf("0x801B                                            MH-CA starting Descriptor\n");
		printf("0x801C                                            MH-Type Descriptor\n");
		printf("0x801D                                            MH-Info Descriptor\n");
		printf("0x801E                                            MH-Expire Descriptor\n");
		printf("0x801F                                            MH-CompressionType Descriptor\n");
		printf("0x8020                                            MH-Data coding system Descriptor\n");
		printf("0x8021                                            UTC-NPT reference Descriptor\n");
		printf("0x8022                                            Reserved (those of which descriptor length field is 8 bits)\n"
			   "                                                  (provided by the Ministry or private standardization organization)\n");
		printf("0x8023                                            MH-Local time offset Descriptor\n");
		printf("0x8024                                            MH-Component group Descriptor\n");
		printf("0x8025                                            MH-Logo transmission Descriptor\n");
		printf("0x8026                                            MPU Extension time stamp Descriptor\n");
		printf("0x8027                                            MPU download content Descriptor\n");
		printf("0x8028                                            MH-Network download content Descriptor\n");
		printf("0x8029                                            MH-Application Descriptor\n");
		printf("0x802A                                            MH-Transmission protocol Descriptor\n");
		printf("0x802B                                            MH-Simple application location Descriptor\n");
		printf("0x802C                                            MH-Application boundary authority setting Descriptor\n");
		printf("0x802D                                            MH-Starting priority information Descriptor\n");
		printf("0x802E                                            MH-Cache information Descriptor\n");
		printf("0x802F                                            MH-Probabilistic application delay Descriptor\n");
		printf("0x8030                                            Link destination PU Descriptor\n");
		printf("0x8031                                            Lock cache designation Descriptor\n");
		printf("0x8032                                            Unlock cache designation Descriptor\n");
		printf("0x8033                                            MH-Download protection Descriptor*3\n");
		printf("0x8034                                            Application service Descriptor\n");
		printf("0x8035                                            MPU node Descriptor\n");
		printf("0x8036                                            PU configuration Descriptor\n");
		printf("0x8037                                            MH-Layered coding Descriptor\n");
		printf("0x8038                                            Content copy control Descriptor\n");
		printf("0x8039                                            Content usage control Descriptor\n");
		printf("0x803A                                            MH-External application control Descriptor\n");
		printf("0x803B                                            MH-Video recording and reproduction application Descriptor\n");
		printf("0x803C                                            MH-Simple video recording and reproduction application location Descriptor\n");
		printf("0x803D                                            MH-Application valid term Descriptor\n");
		printf("0x803E                                            Related broadcaster Descriptor\n");
		printf("0x803F                                            Multimedia service information Descriptor\n");
		printf("0x8040                                            Emergency news Descriptor\n");
		printf("0x8041                                            MH-CA contract information Descriptor*3\n");
		printf("0x8042                                            MH-CA service Descriptor*3\n");
		printf("0x8043 - 0xEBFF                                   Reserved (those of which descriptor length field is 8 bits)\n"
			   "                                                  (provided by the Ministry or private standardization organization)\n");
		printf("0xEC00 - 0xEFFF                                   Descriptor which is prepared by broadcasters\n"
			   "                                                  (those of which descriptor length field is 8 bits)\n");
		printf("0xF000                                            MH-Link Descriptor\n");
		printf("0xF001                                            MH-Short format event Descriptor\n");
		printf("0xF002                                            MH-Extension format event Descriptor\n");
		printf("0xF003                                            Event message Descriptor\n");
		printf("0xF004                                            MH-stuffing Descriptor*4\n");
		printf("0xF005                                            MH-broadcast ID Descriptor*4\n");
		printf("0xF006                                            MH-network identification Descriptor*4\n");
		printf("0xF007 - 0xFBFF                                   Reserved (those of which descriptor length field is 16 bits)\n"
			  "                                                   (provided by the Ministry or private standardization organization)\n");
		printf("0xFC00 - 0xFFFF                                   Descriptor which is prepared by broadcasters\n"
			   "                                                  (those of which descriptor length field is 16 bits)\n");
	}
}
