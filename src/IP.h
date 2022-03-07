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
#pragma once

#include <stdint.h>
#include "Bitstream.h"
#include "DataUtil.h"
#include <algorithm>
#include <list>
#include "dump_data_type.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4101)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4127)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#define IP_FIX_HEADER_FMT_STR				"%s%21s"
#define NTPSHORTTIME_FMT_STR				"%12.6f"	//  5(16bit seconds) + 1(.) + 6(16bit fraction)
#define NTPTIME_FMT_STR						"%17.6f"	// 10(32bit seconds) + 1(.) + 6(32bit fraction)

namespace IP
{
	enum PORT_PROTOCOL
	{
		PORT_PROTOCOL_NTP = 123,

	};

	struct UDPHeader
	{
		uint16_t			Source_port;
		uint16_t			Destination_port;
		uint16_t			Data_length;
		uint16_t			Checksum;

		uint16_t			compressed : 1;
		uint16_t			reserved : 15;

		UDPHeader(bool bCompressed): compressed(bCompressed?1:0) {
		}

		int Unpack(CBitstream& bs)
		{
			uint64_t left_bits = 0ULL;
			bs.Tell(&left_bits);

			uint64_t fixed_hdr_size = compressed ? 4ULL : 8ULL;

			if (left_bits < (fixed_hdr_size << 3))
				return RET_CODE_BOX_TOO_SMALL;

			Source_port = bs.GetWord();
			Destination_port = bs.GetWord();

			if (!compressed)
			{
				Data_length = bs.GetWord();
				Checksum = bs.GetWord();
			}

			return RET_CODE_SUCCESS;
		}

		void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, IP_FIX_HEADER_FMT_STR ": %d\n", szIndent, "source port", Source_port);
			fprintf(out, IP_FIX_HEADER_FMT_STR ": %d\n", szIndent, "dest port", Destination_port);

			if (!compressed)
			{
				fprintf(out, IP_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Data length", Data_length);
				fprintf(out, IP_FIX_HEADER_FMT_STR ": %d(0X%04X)\n", szIndent, "Checksum", Checksum, Checksum);
			}
		}
	}PACKED;

	struct NTPv4Data
	{
		/*
		   0                   1                   2                   3
		   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |          Seconds              |           Fraction            |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		struct NTPShortFormat
		{
			union
			{
				struct
				{
					uint16_t		Seconds;
					uint16_t		Fraction;
				};
				uint32_t			Value;
			};

			int Unpack(CBitstream& bs)
			{
				Seconds = bs.GetWord();
				Fraction = bs.GetWord();
				return RET_CODE_SUCCESS;
			}

			double GetValue() const {
				return Seconds + (double)Fraction / 0x10000L;
			}

			bool IsUnknown() {
				return Seconds == 0 && Fraction == 0;
			}

			void Reset() {
				Seconds = Fraction = 0;
			}
		}PACKED;

		/*
		   0                   1                   2                   3
		   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                            Seconds                            |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                            Fraction                           |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+		
		*/
		struct NTPTimestampFormat
		{
			union
			{
				struct
				{
					uint32_t		Seconds;
					uint32_t		Fraction;
				};
				uint64_t			Value;
			};

			int Unpack(CBitstream& bs)
			{
				Seconds = bs.GetDWord();
				Fraction = bs.GetDWord();
				return RET_CODE_SUCCESS;
			}

			bool IsUnknown() {
				return Seconds == 0 && Fraction == 0;
			}

			void Reset() {
				Seconds = Fraction = 0;
			}

			double GetValue() const {
				return Seconds + (double)Fraction / 0x100000000LL;
			}

			TM_HNS ToHns()
			{
				TM_HNS secondHNS = (TM_HNS)Seconds * 10000000LL;
				return (TM_HNS)((secondHNS + Fraction * 10000000LL / 0x100000000LL));
			}

			TM_90KHZ ToPTS()
			{
				TM_90KHZ second290KHZ = (TM_90KHZ)Seconds * 90000LL;
				return (TM_90KHZ)((second290KHZ + Fraction * 90000LL / 0x100000000LL) % 0x200000000LL);
			}

			TM_90KHZ PTSDiff(const NTPTimestampFormat& tm2)
			{
				int64_t diff_second = (int64_t)Seconds - (int64_t)tm2.Seconds;
				int64_t diff_fraction = (int64_t)Fraction - (int64_t)tm2.Fraction;

				return (TM_90KHZ)((diff_second * 90000LL + diff_fraction * 90000LL / 0x100000000LL) % 0x200000000LL);
			}

			NTPTimestampFormat& IncreaseBy(uint32_t offset, uint32_t time_scale) {
				if (offset == 0)
					return *this;

				uint64_t sum_fractions = Fraction + ((offset * 0x100000000ULL / time_scale) & 0xFFFFFFFFULL);
					
				Fraction = (uint32_t)(sum_fractions & 0xFFFFFFFF);

				if (sum_fractions >= 0x100000000ULL)
					Seconds++;

				Seconds += (uint32_t)(((offset * 0x100000000ULL / time_scale) >> 32) & 0xFFFFFFFFULL);
				return *this;
			}

			NTPTimestampFormat& DecreaseBy(uint32_t offset, uint32_t time_scale)
			{
				if (offset == 0)
					return *this;

				Seconds -= 1;
				uint64_t sum_fractions = 0x100000000ULL + Fraction - ((offset * 0x100000000ULL / time_scale) & 0xFFFFFFFFULL);

				Fraction = (uint32_t)(sum_fractions & 0xFFFFFFFF);

				if (sum_fractions >= 0x100000000ULL)
					Seconds++;

				Seconds += (uint32_t)(((offset * 0x100000000ULL / time_scale) >> 32) & 0xFFFFFFFFULL);
				return *this;
			}

			friend bool operator==(NTPTimestampFormat const &, NTPTimestampFormat const &);
			friend bool operator!=(NTPTimestampFormat const &, NTPTimestampFormat const &);
		}PACKED;

		/*
		   0                   1                   2                   3
		   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                           Era Number                          |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                           Era Offset                          |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  |                           Fraction                            |
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+		
		*/
		struct NTPDateFormat
		{
			uint32_t		Era_Number;
			uint32_t		Era_Offset;
			uint32_t		Fraction;

			int Unpack(CBitstream& bs)
			{
				Era_Number = bs.GetDWord();
				Era_Offset = bs.GetDWord();
				Fraction = bs.GetDWord();
				return RET_CODE_SUCCESS;
			}
		}PACKED;

		/*
		   0                   1                   2                   3
		   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |LI | VN  |Mode |    Stratum     |     Poll      |  Precision   |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                         Root Delay                            |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                         Root Dispersion                       |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                          Reference ID                         |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  +                     Reference Timestamp (64)                  +
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  +                      Origin Timestamp (64)                    +
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  +                      Receive Timestamp (64)                   +
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  +                      Transmit Timestamp (64)                  +
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  .                                                               .
		  .                    Extension Field 1 (variable)               .
		  .                                                               .
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  .                                                               .
		  .                    Extension Field 2 (variable)               .
		  .                                                               .
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                          Key Identifier                       |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		  |                                                               |
		  |                            dgst (128)                         |
		  |                                                               |
		  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+	
		*/

		uint32_t			leap_indicator : 2;
		uint32_t			version : 3;
		uint32_t			mode : 3;
		uint32_t			stratum : 8;
		uint32_t			poll : 8;
		uint32_t			precision : 8;

		// Root Delay (rootdelay): Total round-trip delay to the reference clock, in NTP short format.
		NTPShortFormat		root_delay;
		// Root Dispersion (rootdisp): Total dispersion to the reference clock, in NTP short format.
		NTPShortFormat		root_dispersion;
		uint32_t			reference_identification;
		NTPTimestampFormat	reference_timestamp;
		NTPTimestampFormat	origin_timestamp;
		NTPTimestampFormat	receive_timestamp;
		NTPTimestampFormat	transmit_timestamp;

		int Unpack(CBitstream& bs)
		{
			uint64_t left_bits = 0ULL;
			bs.Tell(&left_bits);

			if (left_bits < (48ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			leap_indicator = (uint32_t)bs.GetBits(2);
			version = (uint32_t)bs.GetBits(3);
			mode = (uint32_t)bs.GetBits(3);
			stratum = bs.GetByte();
			poll = bs.GetByte();
			precision = bs.GetByte();

			root_delay.Unpack(bs);
			root_dispersion.Unpack(bs);

			reference_identification = bs.GetDWord();

			reference_timestamp.Unpack(bs);
			origin_timestamp.Unpack(bs);
			receive_timestamp.Unpack(bs);
			transmit_timestamp.Unpack(bs);

			return RET_CODE_SUCCESS;
		}

		inline static const char* GetModeDesc(uint32_t m) {
			return m == 0?"reserved":(
				   m == 1?"objective active mode":(
				   m == 2?"objective passive mode":(
				   m == 3?"client":(
				   m == 4?"server":(
				   m == 5?"broadcast":(
				   m == 6?"message for NTP control":"reserved for private use"))))));
		}

		void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, IP_FIX_HEADER_FMT_STR ": %s\n", szIndent, "leap_indicator", leap_indicator==0?"Without alarm":(
																					 leap_indicator==1?"Last one minute is 61 seconds":(
																					 leap_indicator==2?"Last one minute is 59 seconds":(
																					 leap_indicator==3?"Alarm":"Unknown"))));
			fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "version", version);
			fprintf(out, IP_FIX_HEADER_FMT_STR ": %s\n", szIndent, "mode", GetModeDesc(mode));
			fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 ", %s\n", szIndent, "stratum", stratum, stratum==0?"indefinite or invalid":(
																							stratum==1?"first reference":(
																							stratum>=2 && stratum <= 15?"":(
																							stratum==16?"without synchronization":"reserved"))));
			fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "poll", poll);
			fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "precision", precision);
			fprintf(out, IP_FIX_HEADER_FMT_STR ": " NTPSHORTTIME_FMT_STR "s\n", szIndent, "root_delay", root_delay.GetValue());
			fprintf(out, IP_FIX_HEADER_FMT_STR ": " NTPSHORTTIME_FMT_STR "s\n", szIndent, "root_dispersion", root_dispersion.GetValue());

			fprintf(out, IP_FIX_HEADER_FMT_STR ": %c%c%c%c (0X%08X)\n", szIndent, "reference_identification", 
				isprint((reference_identification >> 24) & 0xFF) ? (reference_identification >> 24) & 0xFF:'.',
				isprint((reference_identification >> 16) & 0xFF) ? (reference_identification >> 16) & 0xFF : '.', 
				isprint((reference_identification >>  8) & 0xFF) ? (reference_identification >>  8) & 0xFF : '.', 
				isprint((reference_identification)       & 0xFF) ? (reference_identification      ) & 0xFF : '.', reference_identification);

			fprintf(out, IP_FIX_HEADER_FMT_STR ": " NTPTIME_FMT_STR "s, %s\n", szIndent, "reference_timestamp", reference_timestamp.GetValue(),
				reference_timestamp.IsUnknown()?"":DateTimeStr(reference_timestamp.Seconds, 1900, reference_timestamp.Fraction).c_str());
			fprintf(out, IP_FIX_HEADER_FMT_STR ": " NTPTIME_FMT_STR "s, %s\n", szIndent, "origin_timestamp", origin_timestamp.GetValue(),
				origin_timestamp.IsUnknown() ? "" : DateTimeStr(origin_timestamp.Seconds, 1900, origin_timestamp.Fraction).c_str());
			fprintf(out, IP_FIX_HEADER_FMT_STR ": " NTPTIME_FMT_STR "s, %s\n", szIndent, "receive_timestamp", receive_timestamp.GetValue(),
				receive_timestamp.IsUnknown() ? "" : DateTimeStr(receive_timestamp.Seconds, 1900, receive_timestamp.Fraction).c_str());
			fprintf(out, IP_FIX_HEADER_FMT_STR ": " NTPTIME_FMT_STR "s, %s\n", szIndent, "transmit_timestamp", transmit_timestamp.GetValue(),
				transmit_timestamp.IsUnknown() ? "" : DateTimeStr(transmit_timestamp.Seconds, 1900, transmit_timestamp.Fraction).c_str());
		}

	}PACKED;

	struct UDPPacket
	{
		UDPHeader			header;

		union
		{
			uint8_t*			pData = nullptr;
			NTPv4Data*			NTPv4_data;
		};

		uint16_t			compressed : 1;
		uint16_t			reserved : 15;

		UDPPacket(bool bCompressed = false) : header(bCompressed), compressed(bCompressed ? 1 : 0) {
		}
		
		~UDPPacket()
		{
			switch (header.Destination_port)
			{
			case PORT_PROTOCOL_NTP:
				AMP_SAFEDEL(NTPv4_data);
				break;
			default:
				AMP_SAFEDELA(pData);
			}
		}

		int Unpack(CBitstream& bs)
		{
			int iRet = header.Unpack(bs);
			if (iRet < 0)
				return iRet;

			if (header.Data_length < 8)
				return RET_CODE_BUFFER_NOT_COMPATIBLE;

			uint64_t left_bits = 0;
			bs.Tell(&left_bits);

			if (left_bits < ((uint64_t)header.Data_length - 4ULL) << 3)
				return RET_CODE_BOX_TOO_SMALL;

			switch (header.Destination_port)
			{
			case PORT_PROTOCOL_NTP:
				NTPv4_data = new NTPv4Data();
				if ((iRet = NTPv4_data->Unpack(bs)) < 0)
					goto done;

				if (header.Data_length > 56)
					bs.SkipBits(((int64_t)header.Data_length - 56) << 3);

				break;
			default:
				pData = new uint8_t[header.Data_length - 8];
				bs.Read(pData, header.Data_length - 8);
			}

		done:
			return iRet;
		}

		void Print(FILE* fp = nullptr, int indent = 0)
		{
			header.Print(fp, indent);

			switch (header.Destination_port)
			{
			case PORT_PROTOCOL_NTP:
				if (NTPv4_data != nullptr)
					NTPv4_data->Print(fp, indent);
				break;
			default:
				break;
			}
		}

	}PACKED;

	namespace V4
	{
		/*
		Decimal    Keyword     Protocol                             References
		-------    -------     --------                             ----------
		  0                      Reserved                                [JBP]
		  1          ICMP        Internet Control Message         [RFC792,JBP]
		  2          IGMP        Internet Group Management       [RFC1112,JBP]
		  3          GGP         Gateway-to-Gateway                [RFC823,MB]
		  4          IP          IP in IP (encasulation)                 [JBP]
		  5          ST          Stream                   [RFC1190,IEN119,JWF]
		  6          TCP         Transmission Control             [RFC793,JBP]
		  7          UCL         UCL                                      [PK]
		  8          EGP         Exterior Gateway Protocol       [RFC888,DLM1]
		  9          IGP         any private interior gateway            [JBP]
		 10          BBN-RCC-MON BBN RCC Monitoring                      [SGC]
		 11          NVP-II      Network Voice Protocol           [RFC741,SC3]
		 12          PUP         PUP                               [PUP,XEROX]
		 13          ARGUS       ARGUS                                  [RWS4]
		 14          EMCON       EMCON                                   [BN7]
		 15          XNET        Cross Net Debugger              [IEN158,JFH2]
		 16          CHAOS       Chaos                                   [NC3]
		 17          UDP         User Datagram                    [RFC768,JBP]
		 18          MUX         Multiplexing                      [IEN90,JBP]
		 19          DCN-MEAS    DCN Measurement Subsystems             [DLM1]
		 20          HMP         Host Monitoring                  [RFC869,RH6]
		 21          PRM         Packet Radio Measurement                [ZSU]
		 22          XNS-IDP     XEROX NS IDP                 [ETHERNET,XEROX]
		 23          TRUNK-1     Trunk-1                                [BWB6]
		 24          TRUNK-2     Trunk-2                                [BWB6]
		 25          LEAF-1      Leaf-1                                 [BWB6]
		 26          LEAF-2      Leaf-2                                 [BWB6]
		 27          RDP         Reliable Data Protocol           [RFC908,RH6]
		 28          IRTP        Internet Reliable Transaction    [RFC938,TXM]
		 29          ISO-TP4     ISO Transport Protocol Class 4  [RFC905,RC77]
		 30          NETBLT      Bulk Data Transfer Protocol     [RFC969,DDC1]
		 31          MFE-NSP     MFE Network Services Protocol   [MFENET,BCH2]
		 32          MERIT-INP   MERIT Internodal Protocol               [HWB]
		 33          SEP         Sequential Exchange Protocol          [JC120]
		 34          3PC         Third Party Connect Protocol           [SAF3]
		 35          IDPR        Inter-Domain Policy Routing Protocol   [MXS1]
		 36          XTP         XTP                                     [GXC]
		 37          DDP         Datagram Delivery Protocol              [WXC]
		 38          IDPR-CMTP   IDPR Control Message Transport Proto   [MXS1]
		 39          TP++        TP++ Transport Protocol                 [DXF]
		 40          IL          IL Transport Protocol                  [DXP2]
		 41          SIP         Simple Internet Protocol                [SXD]
		 42          SDRP        Source Demand Routing Protocol         [DXE1]
		 43          SIP-SR      SIP Source Route                        [SXD]
		 44          SIP-FRAG    SIP Fragment                            [SXD]
		 45          IDRP        Inter-Domain Routing Protocol     [Sue Hares]
		 46          RSVP        Reservation Protocol             [Bob Braden]
		 47          GRE         General Routing Encapsulation       [Tony Li]
		 48          MHRP        Mobile Host Routing Protocol  [David Johnson]
		 49          BNA         BNA                            [Gary Salamon]
		 50          SIPP-ESP    SIPP Encap Security Payload   [Steve Deering]
		 51          SIPP-AH     SIPP Authentication Header    [Steve Deering]
		 52          I-NLSP      Integrated Net Layer Security  TUBA   [GLENN]
		 53          SWIPE       IP with Encryption                      [JI6]
		 54          NHRP        NBMA Next Hop Resolution Protocol
		 55-60                   Unassigned                              [JBP]
		 61                      any host internal protocol              [JBP]
		 62          CFTP        CFTP                              [CFTP,HCF2]
		 63                      any local network                       [JBP]
		 64          SAT-EXPAK   SATNET and Backroom EXPAK               [SHB]
		 65          KRYPTOLAN   Kryptolan                              [PXL1]
		 66          RVD         MIT Remote Virtual Disk Protocol        [MBG]
		 67          IPPC        Internet Pluribus Packet Core           [SHB]
		 68                      any distributed file system             [JBP]
		 69          SAT-MON     SATNET Monitoring                       [SHB]
		 70          VISA        VISA Protocol                          [GXT1]
		 71          IPCV        Internet Packet Core Utility            [SHB]
		 72          CPNX        Computer Protocol Network Executive    [DXM2]
		 73          CPHB        Computer Protocol Heart Beat           [DXM2]
		 74          WSN         Wang Span Network                       [VXD]
		 75          PVP         Packet Video Protocol                   [SC3]
		 76          BR-SAT-MON  Backroom SATNET Monitoring              [SHB]
		 77          SUN-ND      SUN ND PROTOCOL-Temporary               [WM3]
		 78          WB-MON      WIDEBAND Monitoring                     [SHB]
		 79          WB-EXPAK    WIDEBAND EXPAK                          [SHB]
		 80          ISO-IP      ISO Internet Protocol                   [MTR]
		 81          VMTP        VMTP                                   [DRC3]
		 82          SECURE-VMTP SECURE-VMTP                            [DRC3]
		 83          VINES       VINES                                   [BXH]
		 84          TTP         TTP                                     [JXS]
		 85          NSFNET-IGP  NSFNET-IGP                              [HWB]
		 86          DGP         Dissimilar Gateway Protocol       [DGP,ML109]
		 87          TCF         TCF                                    [GAL5]
		 88          IGRP        IGRP                              [CISCO,GXS]
		 89          OSPFIGP     OSPFIGP                        [RFC1583,JTM4]
		 90          Sprite-RPC  Sprite RPC Protocol              [SPRITE,BXW]
		 91          LARP        Locus Address Resolution Protocol       [BXH]
		 92          MTP         Multicast Transport Protocol            [SXA]
		 93          AX.25       AX.25 Frames                           [BK29]
		 94          IPIP        IP-within-IP Encapsulation Protocol     [JI6]
		 95          MICP        Mobile Internetworking Control Pro.     [JI6]
		 96          SCC-SP      Semaphore Communications Sec. Pro.      [HXH]
		 97          ETHERIP     Ethernet-within-IP Encapsulation       [RXH1]
		 98          ENCAP       Encapsulation Header           [RFC1241,RXB3]
		 99                      any private encryption scheme           [JBP]
		100          GMTP        GMTP                                   [RXB5]
		101-254                  Unassigned                              [JBP]
		255                      Reserved                                [JBP]
		*/
		enum Internet_Protocol
		{
			PROTOCOL_RESERVED = 0,
			PROTOCOL_ICMP,
			PROTOCOL_IGMP,
			PROTOCOL_GGP,
			PROTOCOL_IP,
			PROTOCOL_ST,
			PROTOCOL_TCP,
			PROTOCOL_UCL,
			PROTOCOL_EGP,
			PROTOCOL_IGP,
			PROTOCOL_BBN_RCC_MON,
			PROTOCOL_NVP_II,
			PROTOCOL_PUP,
			PROTOCOL_ARGUS,
			PROTOCOL_EMCON,
			PROTOCOL_XNET,
			PROTOCOL_CHAOS,
			PROTOCOL_UDP,
			PROTOCOL_MUX,
			PROTOCOL_DCN_MEAS,
			PROTOCOL_HMP,
			PROTOCOL_PRM,
			PROTOCOL_XNS_IDP,
			PROTOCOL_TRUNK_1,
			PROTOCOL_TRUNK_2,
			PROTOCOL_LEAF_1,
			PROTOCOL_LEAF_2,
			PROTOCOL_RDP,
			PROTOCOL_IRTP,
			PROTOCOL_ISO_TP4,
			PROTOCOL_NETBLT,
			PROTOCOL_MFE_NSP,
			PROTOCOL_MERIT_INP,
			PROTOCOL_SEP,
			PROTOCOL_3PC,
			PROTOCOL_IDPR,
			PROTOCOL_XTP,
			PROTOCOL_DDP,
			PROTOCOL_IDPR_CMTP,
			PROTOCOL_TPPlusPlus,
			PROTOCOL_IL,
			PROTOCOL_SIP,
			PROTOCOL_SDRP,
			PROTOCOL_SIP_SR,
			PROTOCOL_SIP_FRAG,
			PROTOCOL_IDRP,
			PROTOCOL_RSVP,
			PROTOCOL_GRE,
			PROTOCOL_MHRP,
			PROTOCOL_BNA,
			PROTOCOL_SIPP_ESP,
			PROTOCOL_SIPP_AH,
			PROTOCOL_I_NLSP,
			PROTOCOL_SWIPE,
			PROTOCOL_NHRP,
			PROTOCOL_CFTP = 62,
			PROTOCOL_SAT_EXPAK = 64,
			PROTOCOL_KRYPTOLAN,
			PROTOCOL_RVD,
			PROTOCOL_IPPC,
			PROTOCOL_SAT_MON = 69,
			PROTOCOL_VISA,
			PROTOCOL_IPCV,
			PROTOCOL_CPNX,
			PROTOCOL_CPHB,
			PROTOCOL_WSN,
			PROTOCOL_PVP,
			PROTOCOL_BR_SAT_MON,
			PROTOCOL_SUN_ND,
			PROTOCOL_WB_MON,
			PROTOCOL_WB_EXPAK,
			PROTOCOL_ISO_IP,
			PROTOCOL_VMTP,
			PROTOCOL_SECURE_VMTP,
			PROTOCOL_VINES,
			PROTOCOL_TTP,
			PROTOCOL_NSFNET_IGP,
			PROTOCOL_DGP,
			PROTOCOL_TCF,
			PROTOCOL_IGRP,
			PROTOCOL_OSPFIGP,
			PROTOCOL_Sprite_RPC,
			PROTOCOL_LARP,
			PROTOCOL_MTP,
			PROTOCOL_AX_25,
			PROTOCOL_IPIP,
			PROTOCOL_MICP,
			PROTOCOL_SCC_SP,
			PROTOCOL_ETHERIP,
			PROTOCOL_ENCAP,
			PROTOCOL_GMTP = 100,
		};

		struct Address
		{
			enum Class
			{
				A = 0,
				B,
				C,
				D,
				E,
			};

			uint32_t			address;

			std::string GetIP() {
				char szIP[16];
				sprintf_s(szIP, 16, "%d.%d.%d.%d", (address >> 24) & 0xFF, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF);
				return std::string(szIP);
			}

			Class GetClass()
			{
				uint8_t octet = (address >> 24) & 0xFF;
				if ((octet & 0x80) == 0)
					return A;
				else if ((octet & 0xC0) == 0x80)
					return B;
				else if ((octet & 0xE0) == 0xC0)
					return C;
				else if ((octet & 0xF0) == 0xE0)
					return D;
				
				return E;
			}

		}PACKED;

		struct Header
		{
			uint8_t				Version : 4;
			uint8_t				Header_length : 4;

			uint8_t				Service_type;
			uint16_t			Packet_length;
			uint16_t			Identifier;

			uint16_t			Flag : 3;
			uint16_t			Fragment_offset : 13;

			uint8_t				Lifetime;
			uint8_t				Protocol;

			uint16_t			Header_checksum;
			Address				Source_address;
			Address				Destination_address;
			std::vector<uint32_t>
								Extension_information;

			int Unpack(CBitstream& bs)
			{
				uint64_t left_bits = 0ULL;
				bs.Tell(&left_bits);

				if (left_bits < (20ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				Version = (uint8_t)bs.GetBits(4);
				Header_length = (uint8_t)bs.GetBits(4);

				Service_type = bs.GetByte();
				Packet_length = bs.GetWord();
				Identifier = bs.GetWord();

				Flag = (uint16_t)bs.GetBits(3);
				Fragment_offset = (uint16_t)bs.GetBits(13);

				Lifetime = bs.GetByte();
				Protocol = bs.GetByte();

				Header_checksum = bs.GetWord();
				Source_address.address = bs.GetDWord();
				Source_address.address = bs.GetDWord();

				left_bits -= (20ULL << 3);

				if (Header_length < 5)
					return RET_CODE_BUFFER_NOT_COMPATIBLE;

				if (left_bits + (20ULL << 3) < (uint64_t)Header_length * 32)
					return RET_CODE_BOX_TOO_SMALL;

				if (Header_length > 5)
				{
					Extension_information.reserve(Header_length - 5);
					for (int i = 0; i < Header_length - 5; i++)
						Extension_information.push_back(bs.GetDWord());
				}

				return RET_CODE_SUCCESS;
			}

		};
	}

	namespace V6
	{
		/*
		http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
		Decimal	Keyword	Protocol	IPv6 Extension Header	Reference
		0	HOPOPT	IPv6 Hop-by-Hop Option	Y	[RFC8200]
		1	ICMP	Internet Control Message		[RFC792]
		2	IGMP	Internet Group Management		[RFC1112]
		3	GGP		Gateway-to-Gateway		[RFC823]
		4	IPv4	IPv4 encapsulation		[RFC2003]
		5	ST		Stream		[RFC1190][RFC1819]
		6	TCP		Transmission Control		[RFC793]
		7	CBT		CBT		[Tony_Ballardie]
		8	EGP		Exterior Gateway Protocol		[RFC888][David_Mills]
		9	IGP		any private interior gateway(used by Cisco for their IGRP)		[Internet_Assigned_Numbers_Authority]
		10	BBN-RCC-MON	BBN RCC Monitoring		[Steve_Chipman]
		11	NVP-II	Network Voice Protocol		[RFC741][Steve_Casner]
		12	PUP	PUP		"[Boggs, D., J. Shoch, E. Taft, and R. Metcalfe, ""PUP: An Internetwork Architecture"", XEROX Palo Alto Research Center, CSL-79-10, July 1979; also in IEEE Transactions on Communication, Volume COM-28, Number 4, April 1980.][[XEROX]]"
		13	ARGUS (deprecated)	ARGUS		[Robert_W_Scheifler]
		14	EMCON	EMCON		[<mystery contact>]
		15	XNET	Cross Net Debugger		"[Haverty, J., ""XNET Formats for Internet Protocol Version 4"",
		IEN 158, October 1980.][Jack_Haverty]"
		16	CHAOS	Chaos		[J_Noel_Chiappa]
		17	UDP	User Datagram		[RFC768][Jon_Postel]
		18	MUX	Multiplexing		"[Cohen, D. and J. Postel, ""Multiplexing Protocol"", IEN 90, USC/Information Sciences Institute, May 1979.][Jon_Postel]"
		19	DCN-MEAS	DCN Measurement Subsystems		[David_Mills]
		20	HMP	Host Monitoring		[RFC869][Bob_Hinden]
		21	PRM	Packet Radio Measurement		[Zaw_Sing_Su]
		22	XNS-IDP	XEROX NS IDP		"[""The Ethernet, A Local Area Network: Data Link Layer and Physical Layer Specification"", AA-K759B-TK, Digital Equipment Corporation, Maynard, MA.  Also as: ""The Ethernet - A Local Area Network"", Version 1.0, Digital Equipment Corporation, Intel Corporation, Xerox Corporation, September 1980.  And: ""The Ethernet, A Local Area Network: Data Link Layer and Physical Layer Specifications"", Digital, Intel and Xerox, November 1982. And: XEROX, ""The Ethernet, A Local Area Network: Data Link Layer and Physical Layer Specification"", X3T51/80-50, Xerox Corporation, Stamford, CT., October 1980.][[XEROX]]"
		23	TRUNK-1	Trunk-1		[Barry_Boehm]
		24	TRUNK-2	Trunk-2		[Barry_Boehm]
		25	LEAF-1	Leaf-1		[Barry_Boehm]
		26	LEAF-2	Leaf-2		[Barry_Boehm]
		27	RDP	Reliable Data Protocol		[RFC908][Bob_Hinden]
		28	IRTP	Internet Reliable Transaction		[RFC938][Trudy_Miller]
		29	ISO-TP4	ISO Transport Protocol Class 4		[RFC905][<mystery contact>]
		30	NETBLT	Bulk Data Transfer Protocol		[RFC969][David_Clark]
		31	MFE-NSP	MFE Network Services Protocol		"[Shuttleworth, B., ""A Documentary of MFENet, a National Computer Network"", UCRL-52317, Lawrence Livermore Labs, Livermore, California, June 1977.][Barry_Howard]"
		32	MERIT-INP	MERIT Internodal Protocol		[Hans_Werner_Braun]
		33	DCCP	Datagram Congestion Control Protocol		[RFC4340]
		34	3PC	Third Party Connect Protocol		[Stuart_A_Friedberg]
		35	IDPR	Inter-Domain Policy Routing Protocol		[Martha_Steenstrup]
		36	XTP	XTP		[Greg_Chesson]
		37	DDP	Datagram Delivery Protocol		[Wesley_Craig]
		38	IDPR-CMTP	IDPR Control Message Transport Proto		[Martha_Steenstrup]
		39	TP++	TP++ Transport Protocol		[Dirk_Fromhein]
		40	IL	IL Transport Protocol		[Dave_Presotto]
		41	IPv6	IPv6 encapsulation		[RFC2473]
		42	SDRP	Source Demand Routing Protocol		[Deborah_Estrin]
		43	IPv6-Route	Routing Header for IPv6	Y	[Steve_Deering]
		44	IPv6-Frag	Fragment Header for IPv6	Y	[Steve_Deering]
		45	IDRP	Inter-Domain Routing Protocol		[Sue_Hares]
		46	RSVP	Reservation Protocol		[RFC2205][RFC3209][Bob_Braden]
		47	GRE	Generic Routing Encapsulation		[RFC2784][Tony_Li]
		48	DSR	Dynamic Source Routing Protocol		[RFC4728]
		49	BNA	BNA		[Gary Salamon]
		50	ESP	Encap Security Payload	Y	[RFC4303]
		51	AH	Authentication Header	Y	[RFC4302]
		52	I-NLSP	Integrated Net Layer Security  TUBA		[K_Robert_Glenn]
		53	SWIPE (deprecated)	IP with Encryption		[John_Ioannidis]
		54	NARP	NBMA Address Resolution Protocol		[RFC1735]
		55	MOBILE	IP Mobility		[Charlie_Perkins]
		56	TLSP	"Transport Layer Security Protocol using Kryptonet key management"		[Christer_Oberg]
		57	SKIP	SKIP		[Tom_Markson]
		58	IPv6-ICMP	ICMP for IPv6		[RFC8200]
		59	IPv6-NoNxt	No Next Header for IPv6		[RFC8200]
		60	IPv6-Opts	Destination Options for IPv6	Y	[RFC8200]
		61		any host internal protocol		[Internet_Assigned_Numbers_Authority]
		62	CFTP	CFTP		"[Forsdick, H., ""CFTP"", Network Message, Bolt Beranek and Newman, January 1982.][Harry_Forsdick]"
		63		any local network		[Internet_Assigned_Numbers_Authority]
		64	SAT-EXPAK	SATNET and Backroom EXPAK		[Steven_Blumenthal]
		65	KRYPTOLAN	Kryptolan		[Paul Liu]
		66	RVD	MIT Remote Virtual Disk Protocol		[Michael_Greenwald]
		67	IPPC	Internet Pluribus Packet Core		[Steven_Blumenthal]
		68		any distributed file system		[Internet_Assigned_Numbers_Authority]
		69	SAT-MON	SATNET Monitoring		[Steven_Blumenthal]
		70	VISA	VISA Protocol		[Gene_Tsudik]
		71	IPCV	Internet Packet Core Utility		[Steven_Blumenthal]
		72	CPNX	Computer Protocol Network Executive		[David Mittnacht]
		73	CPHB	Computer Protocol Heart Beat		[David Mittnacht]
		74	WSN	Wang Span Network		[Victor Dafoulas]
		75	PVP	Packet Video Protocol		[Steve_Casner]
		76	BR-SAT-MON	Backroom SATNET Monitoring		[Steven_Blumenthal]
		77	SUN-ND	SUN ND PROTOCOL-Temporary		[William_Melohn]
		78	WB-MON	WIDEBAND Monitoring		[Steven_Blumenthal]
		79	WB-EXPAK	WIDEBAND EXPAK		[Steven_Blumenthal]
		80	ISO-IP	ISO Internet Protocol		[Marshall_T_Rose]
		81	VMTP	VMTP		[Dave_Cheriton]
		82	SECURE-VMTP	SECURE-VMTP		[Dave_Cheriton]
		83	VINES	VINES		[Brian Horn]
		84	TTP	Transaction Transport Protocol		[Jim_Stevens]
		84	IPTM	Internet Protocol Traffic Manager		[Jim_Stevens]
		85	NSFNET-IGP	NSFNET-IGP		[Hans_Werner_Braun]
		86	DGP	Dissimilar Gateway Protocol		"[M/A-COM Government Systems, ""Dissimilar Gateway Protocol Specification, Draft Version"", Contract no. CS901145, November 16, 1987.][Mike_Little]"
		87	TCF	TCF		[Guillermo_A_Loyola]
		88	EIGRP	EIGRP		[RFC7868]
		89	OSPFIGP	OSPFIGP		[RFC1583][RFC2328][RFC5340][John_Moy]
		90	Sprite-RPC	Sprite RPC Protocol		"[Welch, B., ""The Sprite Remote Procedure Call System"", Technical Report, UCB/Computer Science Dept., 86/302, University of California at Berkeley, June 1986.][Bruce Willins]"
		91	LARP	Locus Address Resolution Protocol		[Brian Horn]
		92	MTP	Multicast Transport Protocol		[Susie_Armstrong]
		93	AX.25	AX.25 Frames		[Brian_Kantor]
		94	IPIP	IP-within-IP Encapsulation Protocol		[John_Ioannidis]
		95	MICP (deprecated)	Mobile Internetworking Control Pro.		[John_Ioannidis]
		96	SCC-SP	Semaphore Communications Sec. Pro.		[Howard_Hart]
		97	ETHERIP	Ethernet-within-IP Encapsulation		[RFC3378]
		98	ENCAP	Encapsulation Header		[RFC1241][Robert_Woodburn]
		99		any private encryption scheme		[Internet_Assigned_Numbers_Authority]
		100	GMTP	GMTP		[[RXB5]]
		101	IFMP	Ipsilon Flow Management Protocol		[Bob_Hinden][November 1995, 1997.]
		102	PNNI	PNNI over IP		[Ross_Callon]
		103	PIM	Protocol Independent Multicast		[RFC7761][Dino_Farinacci]
		104	ARIS	ARIS		[Nancy_Feldman]
		105	SCPS	SCPS		[Robert_Durst]
		106	QNX	QNX		[Michael_Hunter]
		107	A/N	Active Networks		[Bob_Braden]
		108	IPComp	IP Payload Compression Protocol		[RFC2393]
		109	SNP	Sitara Networks Protocol		[Manickam_R_Sridhar]
		110	Compaq-Peer	Compaq Peer Protocol		[Victor_Volpe]
		111	IPX-in-IP	IPX in IP		[CJ_Lee]
		112	VRRP	Virtual Router Redundancy Protocol		[RFC5798]
		113	PGM	PGM Reliable Transport Protocol		[Tony_Speakman]
		114		any 0-hop protocol		[Internet_Assigned_Numbers_Authority]
		115	L2TP	Layer Two Tunneling Protocol		[RFC3931][Bernard_Aboba]
		116	DDX	D-II Data Exchange (DDX)		[John_Worley]
		117	IATP	Interactive Agent Transfer Protocol		[John_Murphy]
		118	STP	Schedule Transfer Protocol		[Jean_Michel_Pittet]
		119	SRP	SpectraLink Radio Protocol		[Mark_Hamilton]
		120	UTI	UTI		[Peter_Lothberg]
		121	SMP	Simple Message Protocol		[Leif_Ekblad]
		122	SM (deprecated)	Simple Multicast Protocol		[Jon_Crowcroft][draft-perlman-simple-multicast]
		123	PTP	Performance Transparency Protocol		[Michael_Welzl]
		124	ISIS over IPv4			[Tony_Przygienda]
		125	FIRE			[Criag_Partridge]
		126	CRTP	Combat Radio Transport Protocol		[Robert_Sautter]
		127	CRUDP	Combat Radio User Datagram		[Robert_Sautter]
		128	SSCOPMCE			[Kurt_Waber]
		129	IPLT			[[Hollbach]]
		130	SPS	Secure Packet Shield		[Bill_McIntosh]
		131	PIPE	Private IP Encapsulation within IP		[Bernhard_Petri]
		132	SCTP	Stream Control Transmission Protocol		[Randall_R_Stewart]
		133	FC	Fibre Channel		[Murali_Rajagopal][RFC6172]
		134	RSVP-E2E-IGNORE			[RFC3175]
		135	Mobility Header		Y	[RFC6275]
		136	UDPLite			[RFC3828]
		137	MPLS-in-IP			[RFC4023]
		138	manet	MANET Protocols		[RFC5498]
		139	HIP	Host Identity Protocol	Y	[RFC7401]
		140	Shim6	Shim6 Protocol	Y	[RFC5533]
		141	WESP	Wrapped Encapsulating Security Payload		[RFC5840]
		142	ROHC	Robust Header Compression		[RFC5858]
		143-252		Unassigned		[Internet_Assigned_Numbers_Authority]
		253		Use for experimentation and testing	Y	[RFC3692]
		254		Use for experimentation and testing	Y	[RFC3692]
		255	Reserved			[Internet_Assigned_Numbers_Authority]
		*/

		enum INTERNET_PROTOCOL
		{
			PROTOCOL_HOPOPT = 0,
			PROTOCOL_ICMP =	1,
			PROTOCOL_IGMP =	2,
			PROTOCOL_GGP = 3,
			PROTOCOL_IPv4 = 4,
			PROTOCOL_ST = 5,
			PROTOCOL_TCP = 6,
			PROTOCOL_CBT = 7,
			PROTOCOL_EGP = 8,
			PROTOCOL_IGP = 9,
			PROTOCOL_BBN_RCC_MON = 10,
			PROTOCOL_NVP_II = 11,
			PROTOCOL_PUP = 12,
			PROTOCOL_ARGUS = 13,
			PROTOCOL_EMCON = 14,
			PROTOCOL_XNET = 15,
			PROTOCOL_CHAOS = 16,
			PROTOCOL_UDP = 17,
			PROTOCOL_MUX = 18,
			PROTOCOL_DCN_MEAS = 19,
			PROTOCOL_HMP = 20,
			PROTOCOL_PRM = 21,
			PROTOCOL_XNS_IDP = 22,
			PROTOCOL_TRUNK_1 = 23,
			PROTOCOL_TRUNK_2 = 24,
			PROTOCOL_LEAF_1 = 25,
			PROTOCOL_LEAF_2	= 26,
			PROTOCOL_RDP = 27,
			PROTOCOL_IRTP = 28,
			PROTOCOL_ISO_TP4 = 29,
			PROTOCOL_NETBLT	= 30,
			PROTOCOL_MFE_NSP = 31,
			PROTOCOL_MERIT_INP = 32,
			PROTOCOL_DCCP = 33,
			PROTOCOL_3PC = 34,
			PROTOCOL_IDPR = 35,
			PROTOCOL_XTP = 36,
			PROTOCOL_DDP = 37,
			PROTOCOL_IDPR_CMTP = 38,
			PROTOCOL_TPPlusPlus = 39,
			PROTOCOL_IL = 40,
			PROTOCOL_IPv6 = 41,
			PROTOCOL_SDRP = 42,
			PROTOCOL_IPv6_Route = 43,
			PROTOCOL_IPv6_Frag = 44,
			PROTOCOL_IDRP = 45,
			PROTOCOL_RSVP = 46,
			PROTOCOL_GRE = 47,
			PROTOCOL_DSR = 48,
			PROTOCOL_BNA = 49,
			PROTOCOL_ESP = 50,
			PROTOCOL_AH = 51,
			PROTOCOL_I_NLSP = 52,
			PROTOCOL_SWIPE = 53,
			PROTOCOL_NARP = 54,
			PROTOCOL_MOBILE = 55,
			PROTOCOL_TLSP = 56,
			PROTOCOL_SKIP = 57,
			PROTOCOL_IPv6_ICMP = 58,
			PROTOCOL_IPv6_NoNxt = 59,
			PROTOCOL_IPv6_Opts = 60,
			PROTOCOL_HOST_INTERNEL = 61,
			PROTOCOL_CFTP = 62,
			PROTOCOL_LOCAL_NETWORK = 63,
			PROTOCOL_SAT_EXPAK = 64,
			PROTOCOL_KRYPTOLAN = 65,
			PROTOCOL_RVD = 66,
			PROTOCOL_IPPC = 67,
			PROTOCOL_DISTRIBUTE_FILE_SYSTEM = 68,
			PROTOCOL_SAT_MON = 69,
			PROTOCOL_VISA = 70,
			PROTOCOL_IPCV = 71,
			PROTOCOL_CPNX = 72,
			PROTOCOL_CPHB = 73,
			PROTOCOL_WSN = 74,
			PROTOCOL_PVP = 75,
			PROTOCOL_BR_SAT_MON = 76,
			PROTOCOL_SUN_ND = 77,
			PROTOCOL_WB_MON = 78,
			PROTOCOL_WB_EXPAK = 79,
			PROTOCOL_ISO_IP = 80,
			PROTOCOL_VMTP = 81,
			PROTOCOL_SECURE_VMTP = 82,
			PROTOCOL_VINES = 83,
			PROTOCOL_TTP = 84,
			PROTOCOL_IPTM = 84,
			PROTOCOL_NSFNET_IGP = 85,
			PROTOCOL_DGP = 86,
			PROTOCOL_TCF = 87,
			PROTOCOL_EIGRP = 88,
			PROTOCOL_OSPFIGP = 89,
			PROTOCOL_Sprite_RPC = 90,
			PROTOCOL_LARP = 91,
			PROTOCOL_MTP = 92,
			PROTOCOL_AX_25 = 93,
			PROTOCOL_IPIP = 94,
			PROTOCOL_MICP = 95,
			PROTOCOL_SCC_SP = 96,
			PROTOCOL_ETHERIP = 97,
			PROTOCOL_ENCAP = 98,
			PROTOCOL_PRIVATE_ENCRYPTION_SCHEME = 99,
			PROTOCOL_GMTP = 100,
			PROTOCOL_IFMP = 101,
			PROTOCOL_PNNI = 102,
			PROTOCOL_PIM = 103,
			PROTOCOL_ARIS = 104,
			PROTOCOL_SCPS = 105,
			PROTOCOL_QNX = 106,
			PROTOCOL_A_N = 107,
			PROTOCOL_IPComp = 108,
			PROTOCOL_SNP = 109,
			PROTOCOL_Compaq_Peer = 110,
			PROTOCOL_IPX_in_IP = 111,
			PROTOCOL_VRRP = 112,
			PROTOCOL_PGM = 113,
			PROTOCOL_ZERO_HOP = 114,
			PROTOCOL_L2TP = 115,
			PROTOCOL_DDX = 116,
			PROTOCOL_IATP = 117,
			PROTOCOL_STP = 118,
			PROTOCOL_SRP = 119,
			PROTOCOL_UTI = 120,
			PROTOCOL_SMP = 121,
			PROTOCOL_SM = 122,
			PROTOCOL_PTP = 123,
			PROTOCOL_ISIS_over_IPv4 = 124,
			PROTOCOL_FIRE = 125,
			PROTOCOL_CRTP = 126,
			PROTOCOL_CRUDP = 127,
			PROTOCOL_SSCOPMCE = 128,
			PROTOCOL_IPLT = 129,
			PROTOCOL_SPS = 130,
			PROTOCOL_PIPE = 131,
			PROTOCOL_SCTP = 132,
			PROTOCOL_FC = 133,
			PROTOCOL_RSVP_E2E_IGNORE = 134,
			PROTOCOL_Mobility_Header = 135,
			PROTOCOL_UDPLite = 136,
			PROTOCOL_MPLS_in_IP = 137,
			PROTOCOL_manet = 138,
			PROTOCOL_HIP = 139,
			PROTOCOL_Shim6 = 140,
			PROTOCOL_WESP = 141,
			PROTOCOL_ROHC = 142,
			PROTOCOL_Use_for_experimentation_and_testing_0 = 253,
			PROTOCOL_Use_for_experimentation_and_testing_1 = 254,
			PROTOCOL_Reserved = 255,
		};

		/* Decimal	Keyword	Protocol	IPv6 Extension Header	Reference */
		extern std::tuple<uint8_t, const char*, const char*, bool, const char*> IPv6_protocol_descs[256];

		struct Address
		{
			enum Usage
			{
				Routing = 0,
				Software,
				Host,
				Global_Internet,
				Documentation,
				Private_network,
				Link,
			};

			enum Purpose
			{
				Default_routine = 0,
				Unspecified_address,
				Loopback_address_to_the_local_host,
				IPv4_mapped_addresses,
				IPv4_translated_addresses,
				IPv4_IPv6_translation,
				Discard_prefix,
				Teredo_tunneling,
				ORCHIDv2,
				Addresses_used_in_documentation_and_example_source_code,
				The_6to4_addressing_scheme,
				Unique_local_address,
				Link_local_address,
				Multicast_address
			};

			union
			{
				uint8_t				address_bytes[16];
			};

			std::string GetIP()
			{
				uint16_t parts[8];
				int cbWritten = 0;
				bool bHaveDoubleColon = false;
				int nConsecutiveZero = 0;
				char szIP[40];
				for (size_t i = 0; i < 8; i++)
				{
					parts[i] = (address_bytes[2 * i] << 8) | address_bytes[i * 2 + 1];
					if (parts[i] == 0)
					{
						if (!bHaveDoubleColon)
							nConsecutiveZero++;
						else if((int64_t)_countof(szIP) > (int64_t)cbWritten)
							cbWritten += sprintf_s(szIP + cbWritten, _countof(szIP) - cbWritten, "0%s", i>=7?"":":");
					}
					else if ((int64_t)_countof(szIP) > (int64_t)cbWritten)
					{
						if (nConsecutiveZero > 1)
						{
							cbWritten += sprintf_s(szIP + cbWritten, _countof(szIP) - cbWritten, ":");
							nConsecutiveZero = 0;
							bHaveDoubleColon = true;
						}
						else if (nConsecutiveZero == 1)
						{
							cbWritten += sprintf_s(szIP + cbWritten, _countof(szIP) - cbWritten, "0:");
							nConsecutiveZero = 0;
						}

						if ((int64_t)_countof(szIP) > (int64_t)cbWritten) {
							cbWritten += sprintf_s(szIP + cbWritten, _countof(szIP) - cbWritten, "%x%s", parts[i], i >= 7 ? "" : ":");
						}
					}
				}

				if (bHaveDoubleColon == false && nConsecutiveZero > 0 && (int64_t)_countof(szIP) > (int64_t)cbWritten)
					cbWritten += sprintf_s(szIP + cbWritten, _countof(szIP) - cbWritten, "%s", nConsecutiveZero>1?"::":"0");

				return std::string(szIP);
			}

		}PACKED;

		struct Option
		{
			enum OPTION_TYPE
			{
				Pad1_option = 0,
				PadN_option = 1,
				Jumbo_Payload_option = 0xC2,
				Router_Alert_option = 5,
				Home_Address_option = 0xC9,
			};

			uint8_t				Option_Type = 0;
			uint8_t				Opt_Data_Len = 0;

			virtual ~Option() {}

			uint16_t GetLength() {
				return Option_Type == 0 ? (uint16_t)1 : ((uint16_t)Opt_Data_Len + 2);
			}

			virtual int Unpack(CBitstream& bs)
			{
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < 8)
					return RET_CODE_BOX_TOO_SMALL;

				Option_Type = bs.GetByte();
				if (Option_Type == 0)
					return RET_CODE_SUCCESS;

				left_bits -= 8;
				if (left_bits < 8)
					return RET_CODE_BOX_TOO_SMALL;

				Opt_Data_Len = bs.GetByte();
				left_bits -= 8;
				
				return RET_CODE_SUCCESS;
			}
		}PACKED;

		/*
		PadN Option
		The PadN option is defined in RFC 2460. 
		It is used to insert two or more bytes of padding so that the Hop-by-Hop Options or 
		Destination Options headers fall on 8-byte boundaries and to accommodate the alignment 
		requirements of options. The PadN option has no alignment requirements.
		*/
		struct GeneralOption : public Option
		{
			std::vector<uint8_t>
								Option_Data;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = Option::Unpack(bs);
				if (iRet < 0)
					return iRet;

				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < ((uint64_t)Opt_Data_Len << 3))
					return RET_CODE_BOX_TOO_SMALL;

				if (Opt_Data_Len > 0)
				{
					Option_Data.resize(Opt_Data_Len);
					if (bs.Read(&Option_Data[0], Opt_Data_Len) != Opt_Data_Len)
						return RET_CODE_ERROR;
				}

				return RET_CODE_SUCCESS;
			}
		};

		/*
		Jumbo Payload Option
		The Jumbo Payload option is defined in RFC 2675. 
		It is used to indicate a payload size that is greater than 65,535 bytes. 
		The Jumbo Payload option has the alignment requirement of 4n + 2.
		*/
		struct JumboPayloadOption : public Option
		{
			uint32_t			Jumbo_Payload_Length = 0;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = Option::Unpack(bs);
				if (iRet < 0)
					return iRet;

				if (Opt_Data_Len != 4)
					return RET_CODE_BUFFER_NOT_COMPATIBLE;

				uint64_t left_bits = 0;
				bs.Tell(&left_bits);

				if (left_bits < (4ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				Jumbo_Payload_Length = bs.GetDWord();
				return RET_CODE_SUCCESS;
			}

		}PACKED;

		/*
		Router Alert Option
		The Router Alert option (Option Type 5) is defined in RFC 2711 and is used to
		indicate to a router that the contents of the packet require additional processing. 
		The Router Alert option has the alignment requirement of 2n + 0.
		*/
		struct RouterAlertOption : public Option
		{
			uint16_t			Router_Alert_Value = 0;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = Option::Unpack(bs);
				if (iRet < 0)
					return iRet;

				if (Opt_Data_Len != 4)
					return RET_CODE_BUFFER_NOT_COMPATIBLE;

				uint64_t left_bits = 0;
				bs.Tell(&left_bits);

				if (left_bits < (4ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				Router_Alert_Value = bs.GetWord();
				return RET_CODE_SUCCESS;
			}
		}PACKED;

		/*
		Home Address Option
		The Home Address destination option (Option Type 201) is defined in RFC 6275 
		and is used to indicate the home address of the mobile node. The home address 
		is an address assigned to the mobile node when it is attached to the home link 
		and through which the mobile node is always reachable, regardless of its location 
		on an IPv6 network. For information about when the Home Address option is sent, 
		see Appendix G, "Mobile IPv6." 
		The Home Address option has the alignment requirement of 8n + 6.
		*/
		struct HomeAddressOption : public Option
		{
			Address			Home_Address = { 0 };

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = Option::Unpack(bs);
				if (iRet < 0)
					return iRet;

				if (iRet != 16)
					return RET_CODE_BUFFER_NOT_COMPATIBLE;

				uint64_t left_bits = 0;
				bs.Tell(&left_bits);

				if (left_bits < (16ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				if (bs.Read(Home_Address.address_bytes, 8) != 8)
					return RET_CODE_ERROR;

				return RET_CODE_SUCCESS;
			}
		}PACKED;

		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|Version| Traffic Class |           Flow Label                  |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|         Payload Length        |  Next Header  |   Hop Limit   |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                                                               +
		|                                                               |
		+                         Source Address                        +
		|                                                               |
		+                                                               +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                                                               +
		|                                                               |
		+                      Destination Address                      +
		|                                                               |
		+                                                               +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		struct Header
		{
			struct Hop_by_Hop_Options_Header
			{
				uint8_t				Next_Header = 0;
				uint8_t				Header_Extension_Length = 0;
				std::list<Option*>	Options;

				~Hop_by_Hop_Options_Header()
				{
					for (auto& v : Options)
						if (v != nullptr)
							delete v;
				}

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					uint64_t left_bits = 0;
					bs.Tell(&left_bits);
					if (left_bits < 16)
						return RET_CODE_BOX_TOO_SMALL;

					Next_Header = bs.GetByte();
					Header_Extension_Length = bs.GetByte();

					uint64_t total_bits = ((uint64_t)Header_Extension_Length + 1ULL)<<6;
					if (left_bits < total_bits)
						return RET_CODE_BOX_TOO_SMALL;

					total_bits -= 16;
					while (total_bits > 8)
					{
						Option* ptr_option = nullptr;
						uint8_t option_type = (uint8_t)bs.PeekBits(8);
						switch (option_type)
						{
						case Option::Jumbo_Payload_option:
							ptr_option = new JumboPayloadOption();
							break;
						case Option::Router_Alert_option:
							ptr_option = new RouterAlertOption();
							break;
						case Option::Home_Address_option:
							ptr_option = new HomeAddressOption();
							break;
						default:
							ptr_option = new GeneralOption();
						}

						if ((iRet = ptr_option->Unpack(bs)) < 0)
						{
							delete ptr_option;
							return iRet;
						}

						Options.push_back(ptr_option);
						uint16_t Option_len = ptr_option->GetLength();
						if (Option_len > total_bits)
							return RET_CODE_BUFFER_NOT_COMPATIBLE;
						
						total_bits -= Option_len;
					}
					
					return RET_CODE_SUCCESS;
				}
			};

			/*
			Destination Options Header
			The Destination Options header is used to specify packet delivery 
			parameters for either intermediate destinations or the final destination. 
			This header is identified by the value of 60 in the previous header's Next Header field. 
			The Destination Options header has the same structure as the Hop-by-Hop Options header
			*/
			using Destination_Options_Header = Hop_by_Hop_Options_Header;

			struct Routing_Header
			{
				uint8_t				Next_Header = 0;
				uint8_t				Header_Extension_Length = 0;
				uint8_t				Routing_Type = 0;
				uint8_t				Segments_Left = 0;

				uint8_t*			Routing_Type_Specific_Data = nullptr;

				~Routing_Header(){
					if (Routing_Type_Specific_Data != nullptr)
						delete[] Routing_Type_Specific_Data;
				}

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					uint64_t left_bits = 0;
					bs.Tell(&left_bits);
					if (left_bits < 32)
						return RET_CODE_BOX_TOO_SMALL;

					Next_Header = bs.GetByte();
					Header_Extension_Length = bs.GetByte();
					Routing_Type = bs.GetByte();
					Segments_Left = bs.GetByte();

					uint32_t total_bits = ((uint32_t)Header_Extension_Length + 1UL) << 6;
					if (left_bits < total_bits)
						return RET_CODE_BOX_TOO_SMALL;

					total_bits -= 32;
					int left_bytes = (int)(total_bits >> 3);
					Routing_Type_Specific_Data = new uint8_t[left_bytes];
					if (bs.Read(Routing_Type_Specific_Data, left_bytes) != left_bytes)
						return RET_CODE_ERROR;

					return iRet;
				}
			}PACKED;

			struct Fragment_Header
			{
				uint8_t				Next_Header;
				uint8_t				Reserved_0;
				uint16_t			Fragment_Offset : 13;
				uint16_t			Reserved_1 : 2;
				uint16_t			M_flag : 1;
				uint32_t			Identification;

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					uint64_t left_bits = 0;
					bs.Tell(&left_bits);
					if (left_bits < 64)
						return RET_CODE_BOX_TOO_SMALL;

					Next_Header = bs.GetByte();
					Reserved_0 = bs.GetByte();
					Fragment_Offset = (uint16_t)bs.GetBits(13);
					Reserved_1 = (uint16_t)bs.GetBits(2);
					M_flag = (uint16_t)bs.GetBits(1);
					Identification = bs.GetDWord();

					return iRet;
				}
			}PACKED;

			/*
			Authentication Header
			The Authentication header provides data authentication (verification of the node that sent the packet),
			data integrity (verification that the data was not modified in transit), and anti-replay protection
			(assurance that captured packets cannot be retransmitted and accepted as valid data) for the IPv6 packet,
			including the fields in the IPv6 header that do not change in transit across an IPv6 internetwork.
			The Authentication header, described in RFC 4302, is part of the security architecture for IP, as defined in RFC 4301.
			The Authentication header is identified by the value of 51 in the previous header's Next Header field
			*/
			struct Authentication_Header
			{
				uint8_t			Next_Header = 0;
				uint8_t			Payload_Len = 0;
				uint16_t		Reserved = 0;
				uint32_t		Security_Parameters_Index = 0;
				uint32_t		Sequence_Number = 0;

				uint8_t*		Authentication_Data = nullptr;

				~Authentication_Header() {
					if (Authentication_Data != nullptr)
						delete[] Authentication_Data;
				}

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					uint64_t left_bits = 0;
					bs.Tell(&left_bits);
					if (left_bits < 96)
						return RET_CODE_BOX_TOO_SMALL;

					Next_Header = bs.GetByte();
					Payload_Len = bs.GetByte();
					Reserved = bs.GetWord();
					Security_Parameters_Index = bs.GetDWord();
					Sequence_Number = bs.GetDWord();

					uint32_t total_bits = ((uint32_t)Payload_Len + 2ULL) << 5;
					if (left_bits < total_bits)
						return RET_CODE_BOX_TOO_SMALL;

					total_bits -= 96;
					int left_bytes = (int)(total_bits >> 3);
					Authentication_Data = new uint8_t[left_bytes];
					if (bs.Read(Authentication_Data, left_bytes) != left_bytes)
						return RET_CODE_ERROR;

					return iRet;
				}

			}PACKED;

			struct Encapsulating_Security_Payload_Header
			{

			}PACKED;

			uint32_t			Version : 4;
			uint32_t			Traffic_class : 8;
			uint32_t			Flow_label : 20;

			uint32_t			Payload_length : 16;
			uint32_t			Next_header : 8;
			uint32_t			Hop_limit : 8;

			Address				Source_address;
			Address				Destination_address;
			
			int16_t				payload_type = -1;
			uint16_t			compressed : 1;
			uint16_t			reserved : 15;

			/*
			When more than one extension header is used in the same packet, it is
			recommended that those headers appear in the following order:
			*/
			Hop_by_Hop_Options_Header*
								ptr_Hop_by_Hop_Options_Header = nullptr;
			Destination_Options_Header*
								ptr_Dest_Options_Header_0 = nullptr;
			Routing_Header*		ptr_Routing_Header = nullptr;
			Fragment_Header*	ptr_Fragment_Header = nullptr;
			Authentication_Header*
								ptr_Authentication_Header = nullptr;
			Encapsulating_Security_Payload_Header*
								ptr_ESP = nullptr;
			Destination_Options_Header*
								ptr_Dest_Options_Header_1 = nullptr;
			// For extension headers, implement it later

			Header(bool bCompressed=false) : compressed(bCompressed ? 1 : 0) {
			}

			int Unpack(CBitstream& bs)
			{
				Destination_Options_Header* prev_Dest_Opt_Header = nullptr;
				int iRet = RET_CODE_SUCCESS;
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < (40UL<<3))
					return RET_CODE_BOX_TOO_SMALL;

				Version = (uint32_t)bs.GetBits(4);
				Traffic_class = (uint32_t)bs.GetBits(8);
				Flow_label = (uint32_t)bs.GetBits(20);

				if (!compressed)
					Payload_length = bs.GetWord();

				Next_header = bs.GetByte();
				Hop_limit = bs.GetByte();

				bs.Read(Source_address.address_bytes, 16);
				bs.Read(Destination_address.address_bytes, 16);

				left_bits -= ((compressed?38ULL:40ULL) << 3);
				//uint64_t left_payload_bits = ((uint64_t)Payload_length) << 3;

				bool bExtHeaderFinished = false;
				uint8_t next_hdr = Next_header;

				while (left_bits >= (8ULL << 3) && !bExtHeaderFinished)
				{
					switch (next_hdr)
					{
					case PROTOCOL_HOPOPT:
						if (ptr_Hop_by_Hop_Options_Header != nullptr)
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							goto done;
						}
						ptr_Hop_by_Hop_Options_Header = new Hop_by_Hop_Options_Header();
						break;
					case PROTOCOL_IPv6_Opts:
						if (prev_Dest_Opt_Header != nullptr)
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							goto done;
						}
						prev_Dest_Opt_Header = new Destination_Options_Header();
						break;
					case PROTOCOL_IPv6_Route:
						if (ptr_Routing_Header != nullptr)
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							goto done;
						}

						if (prev_Dest_Opt_Header)
						{
							ptr_Dest_Options_Header_0 = prev_Dest_Opt_Header;
							prev_Dest_Opt_Header = nullptr;
						}
						ptr_Routing_Header = new Routing_Header;
						break;
					case PROTOCOL_IPv6_Frag:
						if (ptr_Fragment_Header != nullptr)
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							goto done;
						}

						ptr_Fragment_Header = new Fragment_Header();
						break;
					case PROTOCOL_AH:
						if (ptr_Authentication_Header != nullptr)
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							goto done;
						}

						ptr_Authentication_Header = new Authentication_Header();
						break;
					case PROTOCOL_ESP:
						if (ptr_ESP != nullptr)
						{
							iRet = RET_CODE_ERROR_NOTIMPL;
							goto done;
						}
						break;
					case PROTOCOL_IPv6_NoNxt:
						bExtHeaderFinished = true;
						iRet = RET_CODE_SUCCESS;
						goto done;
					default:
						payload_type = next_hdr;
						bExtHeaderFinished = true;
						break;
					}
				}

			done:
				AMP_SAFEDEL(prev_Dest_Opt_Header);
				return iRet;
			}

			void Print(FILE* fp = nullptr, int indent = 0)
			{
				FILE* out = fp ? fp : stdout;
				char szIndent[84];
				memset(szIndent, 0, _countof(szIndent));
				if (indent > 0)
				{
					int ccIndent = AMP_MIN(indent, 80);
					memset(szIndent, ' ', ccIndent);
				}

				fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "Version", Version);
				fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "Traffic class", Traffic_class);
				fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "Flow label", Flow_label);
				if (!compressed)
					fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "Payload length", Payload_length);

				fprintf(out, IP_FIX_HEADER_FMT_STR ": %d, %s, %s\n", szIndent, "Next header", Next_header,
					std::get<1>(IPv6_protocol_descs[Next_header]), std::get<2>(IPv6_protocol_descs[Next_header]));
				fprintf(out, IP_FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "Hop limit", Hop_limit);
				
				fprintf(out, IP_FIX_HEADER_FMT_STR ": [%02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X] %s\n", szIndent, "Source address", 
					Source_address.address_bytes[0], Source_address.address_bytes[1], Source_address.address_bytes[2], Source_address.address_bytes[3],
					Source_address.address_bytes[4], Source_address.address_bytes[5], Source_address.address_bytes[6], Source_address.address_bytes[7],
					Source_address.address_bytes[8], Source_address.address_bytes[9], Source_address.address_bytes[10], Source_address.address_bytes[11],
					Source_address.address_bytes[12], Source_address.address_bytes[13], Source_address.address_bytes[14], Source_address.address_bytes[15],
					Source_address.GetIP().c_str());
				fprintf(out, IP_FIX_HEADER_FMT_STR ": [%02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X] %s\n", szIndent, "Dest address", 
					Destination_address.address_bytes[0], Destination_address.address_bytes[1], Destination_address.address_bytes[2], Destination_address.address_bytes[3],
					Destination_address.address_bytes[4], Destination_address.address_bytes[5], Destination_address.address_bytes[6], Destination_address.address_bytes[7],
					Destination_address.address_bytes[8], Destination_address.address_bytes[9], Destination_address.address_bytes[10], Destination_address.address_bytes[11],
					Destination_address.address_bytes[12], Destination_address.address_bytes[13], Destination_address.address_bytes[14], Destination_address.address_bytes[15],
					Destination_address.GetIP().c_str());
			}

		}PACKED;
	}
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED
