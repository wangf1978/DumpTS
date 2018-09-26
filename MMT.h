#pragma once

#include "IP.h"

/*
	AIT		Application Information Table
	AMT		Address Map Table
	AL-FEC	Application Layer Forward Error Correction
	BIT		Broadcaster Information Table
	CA		Conditional Access
	CDT		Common Data Table
	CRI		Clock Relation Information
	CRID	Content Reference Identifier
	DCI		Device Capability Information
	ECM		Entitlement Control Message
	EIT		Event Information Table
	EMM		Entitlement Management Control
	GFD		Generic File Delivery
	HRBM	Hypothetical Receiver Buffer Model
	LCT		Layout Configuration Table
	LDT		Linked Description Table
	MFU		Media Fragment Unit
	MMT		MPEG Media Transport
	MMTP	MMT Protocol
	MPI		Media Presentation Information
	MPT		MMT Package Table
	MPU		Media Processing Unit
	NIT		Network Information Table
	PA		Package Access
	PLT		Package List Table
	SDT		Service Description Table
	SDTT	Software Download Trigger
	TLV		Type Length Value
	URL		Uniform Resource Locator
*/

#include <stdint.h>
#include "Bitstream.h"

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

#define MAX_TLV_PKT_DATA_LENGTH				UINT16_MAX
#define MAX_FULL_TLV_PACKET_LENGTH			(MAX_TLV_PKT_DATA_LENGTH + 4)
#define MMT_FIX_HEADER_FMT_STR				"%s%21s"

namespace MMT
{
	enum TLV_PACKET_TYPE
	{
		TLV_Undefined_packet = 0x0,
		TLV_IPv4_packet = 0x01,
		TLV_IPv6_packet = 0x02,
		TLV_Header_compressed_IP_packet = 0x03,
		TLV_Transmission_control_signal_packet = 0xFE,
		TLV_Null_packet = 0xFF
	};

	#define TLV_PACKET_TYPE_NAMEA(t)	(\
			(t) == TLV_IPv4_packet?"IPv4 packet":(\
			(t) == TLV_IPv6_packet?"IPv6 packet":(\
			(t) == TLV_Header_compressed_IP_packet?"Header compressed IP packet":(\
			(t) == TLV_Transmission_control_signal_packet?"Transmission control signal packet":(\
			(t) == TLV_Null_packet?"Null packet":"Undefined packet")))))

	#define TLV_PACKET_TYEP_NAME	TLV_PACKET_TYPE_NAMEA

	enum ULE_PACKET_TYPE
	{
		ULE_Undefined_packet = 0x0,
		ULE_Bridged_frame = 0x0001,
		ULE_IPv4_packet = 0x0800,
		ULE_Compressed_IP_packet_by_ROHC = 0x22F1,
		ULE_Compressed_IP_packet_by_HCfB = 0x22F2,
		ULE_IPv6_packet = 0x86DD
	};

	#define ULE_PACKET_TYPE_NAMEA(t)	(\
			(t) == ULE_Bridged_frame?"Bridged frame":(\
			(t) == ULE_IPv4_packet?"IPv4 packet":(\
			(t) == ULE_Compressed_IP_packet_by_ROHC?"Compressed IP packet by ROHC":(\
			(t) == ULE_Compressed_IP_packet_by_HCfB?"Compressed IP packet by HCfB":(\
			(t) == ULE_IPv6_packet?"IPv6 packet":"Undefined packet")))))

	#define ULE_PACKET_TYPE_NAME	ULE_PACKET_TYPE_NAMEA

	struct TLVPacket
	{
		uint64_t			start_bitpos;
		uint8_t				upper_2bits : 2;
		uint8_t				upper_6bits : 6;
		uint8_t				Packet_type;
		uint16_t			Data_length;

		virtual int Unpack(CBitstream& bs)
		{
			int nRet = RET_CODE_SUCCESS;
			uint64_t left_bits = 0ULL;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < (4ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			upper_2bits = (uint8_t)bs.GetBits(2);
			upper_6bits = (uint8_t)bs.GetBits(6);
			Packet_type = bs.GetByte();
			Data_length = bs.GetWord();

			return nRet;
		}

		virtual uint32_t LeftBits(CBitstream& bs)
		{
			uint64_t cur_bitpos = bs.Tell();

			if (cur_bitpos > start_bitpos)
			{
				uint64_t whole_packet_bits_size = ((uint64_t)(Data_length + 4)) << 3;
				if (whole_packet_bits_size > cur_bitpos - start_bitpos)
				{
					uint64_t left_bits = whole_packet_bits_size - (cur_bitpos - start_bitpos);
					assert(left_bits <= ((uint64_t)(MAX_FULL_TLV_PACKET_LENGTH) << 3));
					return (uint32_t)left_bits;
				}
			}

			return 0UL;
		}

		virtual void SkipLeftBits(CBitstream& bs)
		{
			uint64_t left_bits = LeftBits(bs);
			if (left_bits > 0)
				bs.SkipBits(left_bits);
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, "-----------------------------------------------------------------------------------------\n");
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %llu\n", szIndent, "file offset", start_bitpos >> 3);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "upper_2bits", upper_2bits);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "upper_6bits", upper_6bits);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "Packet_type", TLV_PACKET_TYEP_NAME(Packet_type));
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Data_length", Data_length);
		}

	}PACKED;

	// Unidirectional Lightweight Encapsulation (ULE) for Transmission of IP Datagrams over an MPEG-2 Transport Stream(TS),
	struct ULEPacket
	{
		uint64_t			start_bitpos;
		uint16_t			Destination_flag : 1;
		uint16_t			Data_length : 15;
		uint64_t			Packet_type : 16;
		uint64_t			Destination_address : 48;
		uint32_t			CRC;

		virtual int Unpack(CBitstream& bs)
		{
			int nRet = RET_CODE_SUCCESS;
			uint64_t left_bits = 0ULL;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < (10ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			Destination_flag = (uint16_t)bs.GetBits(1);
			Data_length = (uint8_t)bs.GetBits(15);
			Packet_type = bs.GetWord();
			if (Destination_flag == 0)
				Destination_address = bs.GetBits(48);

			return nRet;
		}

		virtual uint32_t LeftBits(CBitstream& bs)
		{
			uint64_t cur_bitpos = bs.Tell();

			if (cur_bitpos > start_bitpos)
			{
				uint64_t whole_packet_bits_size = ((uint64_t)(Data_length + 4)) << 3;
				if (whole_packet_bits_size > cur_bitpos - start_bitpos)
				{
					uint64_t left_bits = whole_packet_bits_size - (cur_bitpos - start_bitpos);
					assert(left_bits <= ((uint64_t)(MAX_FULL_TLV_PACKET_LENGTH) << 3));
					return (uint32_t)left_bits;
				}
			}

			return 0UL;
		}

		virtual void SkipLeftBits(CBitstream& bs)
		{
			uint64_t left_bits = LeftBits(bs);
			if (left_bits > 0)
				bs.SkipBits(left_bits);
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, "-----------------------------------------------------------------------------------------\n");
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "Destination_flag", Destination_flag);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "Data_length", Data_length);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "Packet_type", ULE_PACKET_TYPE_NAME(Packet_type));
			if (Destination_flag)
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lld\n", szIndent, "Destination_address", Destination_address);
		}
	}PACKED;

	struct Undefined_TLVPacket : public TLVPacket
	{
		int Unpack(CBitstream& bs)
		{
			int iRet = TLVPacket::Unpack(bs);
			if (iRet < 0)
				return iRet;

			SkipLeftBits(bs);
			return iRet;
		}
	}PACKED;

	using TLVNullPacket = Undefined_TLVPacket;

	struct IPv4Packet : public TLVPacket
	{
		IP::V4::Header		IPv4_Header;
		IP::UDPHeader		UDP_header;


	}PACKED;

	struct IPv6Packet : public TLVPacket
	{
		IP::V6::Header		IPv6_Header;
		union
		{
			IP::UDPPacket*		UDP_packet;
			void*				ptr_Data = nullptr;
		};

		virtual ~IPv6Packet() {
			if (IPv6_Header.payload_type == IP::V6::PROTOCOL_UDP)
				delete UDP_packet;
		}

		int Unpack(CBitstream& bs)
		{
			int iRet = TLVPacket::Unpack(bs);
			if (iRet < 0)
				return iRet;

			if ((iRet = IPv6_Header.Unpack(bs)) < 0)
				goto done;

			if (IPv6_Header.payload_type == IP::V6::PROTOCOL_UDP)
			{
				UDP_packet = new IP::UDPPacket();
				if ((iRet = UDP_packet->Unpack(bs)) < 0)
				{
					delete UDP_packet;
					UDP_packet = nullptr;
					goto done;
				}
			}

		done:
			SkipLeftBits(bs);
			return iRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			TLVPacket::Print(fp, indent);
			IPv6_Header.Print(fp, indent);

			if (IPv6_Header.payload_type == IP::V6::PROTOCOL_UDP)
			{
				if (UDP_packet != nullptr)
					UDP_packet->Print(fp, indent);
			}
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}
		}

	}PACKED;

	struct MMTGeneralLocationInfo
	{
		uint64_t			start_bitpos;
		uint8_t				location_type;
		union
		{
			uint16_t			packet_id;
			struct
			{
				IP::V4::Address	ipv4_src_addr;
				IP::V4::Address	ipv4_dst_addr;
				uint16_t		dst_port;
				uint16_t		packet_id;
			}PACKED MMTP_IPv4;
			struct
			{
				IP::V6::Address	ipv6_src_addr;
				IP::V6::Address	ipv6_dst_addr;
				uint16_t		dst_port;
				uint16_t		packet_id;
			}PACKED MMTP_IPv6;
			struct
			{
				uint16_t		network_id;
				uint16_t		MPEG_2_transport_stream_id;
				uint16_t		reserved : 3;
				uint16_t		MPEG_2_PID : 13;
			}PACKED M2TS_broadcast;
			struct
			{
				IP::V6::Address	ipv6_src_addr;
				IP::V6::Address	ipv6_dst_addr;
				uint16_t		dst_port;
				uint16_t		reserved : 3;
				uint16_t		MPEG_2_PID : 13;
			}PACKED M2TS_IPv6;
			struct
			{
				uint8_t			URL_length;
				uint8_t			URL_byte[256];
			}PACKED URL;
		}PACKED;

		uint16_t GetLength()
		{
			uint16_t nLen = 1;
			if (location_type == 0)
				nLen++;
			else if (location_type == 1)
				nLen += 12;
			else if (location_type == 2)
				nLen += 36;
			else if (location_type == 3)
				nLen += 6;
			else if (location_type == 4)
				nLen += 36;
			else if (location_type == 5)
				nLen += 1 + URL.URL_length;

			return nLen;
		}

		int Unpack(CBitstream& bs)
		{
			uint64_t left_bits = 0;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < 8)
				return RET_CODE_BOX_TOO_SMALL;

			location_type = bs.GetByte();
			left_bits -= 8;

			switch (location_type)
			{
			case 0:
				if (left_bits >= 16)
					packet_id = bs.GetWord();
				break;
			case 1:
				if (left_bits >= (12ULL << 3))
				{
					MMTP_IPv4.ipv4_src_addr.address = bs.GetDWord();
					MMTP_IPv4.ipv4_dst_addr.address = bs.GetDWord();
					MMTP_IPv4.dst_port = bs.GetWord();
					MMTP_IPv4.packet_id = bs.GetWord();
				}
				break;
			case 2:
				if (left_bits >= (36ULL << 3))
				{
					bs.Read(MMTP_IPv6.ipv6_src_addr.address_bytes, 16);
					bs.Read(MMTP_IPv6.ipv6_dst_addr.address_bytes, 16);
					MMTP_IPv6.dst_port = bs.GetWord();
					MMTP_IPv6.packet_id = bs.GetWord();
				}
				break;
			case 3:
				if (left_bits >= (6ULL << 3))
				{
					M2TS_broadcast.network_id = bs.GetWord();
					M2TS_broadcast.MPEG_2_transport_stream_id = bs.GetWord();
					M2TS_broadcast.reserved = (uint16_t)bs.GetBits(3);
					M2TS_broadcast.MPEG_2_PID = (uint16_t)bs.GetBits(13);
				}
				break;
			case 4:
				if (left_bits >= (36ULL << 3))
				{
					bs.Read(M2TS_IPv6.ipv6_src_addr.address_bytes, 16);
					bs.Read(M2TS_IPv6.ipv6_dst_addr.address_bytes, 16);
					M2TS_IPv6.dst_port = bs.GetWord();
					M2TS_IPv6.reserved = (uint16_t)bs.GetBits(3);
					M2TS_IPv6.MPEG_2_PID = (uint16_t)bs.GetBits(13);
				}
				break;
			case 5:
				if (left_bits >= 8)
				{
					URL.URL_length = bs.GetByte();
					left_bits -= 8;
					memset(URL.URL_byte, 0, sizeof(URL.URL_byte));
					if (left_bits >= URL.URL_length)
						bs.Read(URL.URL_byte, URL.URL_length);
					else if ((left_bits >> 3) > 0ULL)
						bs.Read(URL.URL_byte, (size_t)(left_bits >> 3));
				}
				break;
			default:
				return RET_CODE_ERROR_NOTIMPL;
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

			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u, %s\n", szIndent, "location_type", location_type,
				location_type == 0?"MMTP packet of the same IP data flow as IP data flow in which the table including this general_location_info is transmitted":(
				location_type == 1?"MMTP packet of IPv4 data flow":(
				location_type == 2?"MMTP packet of IPv6 data flow":(
				location_type == 3?"MPEG-2 TS packet of broadcasting network by MPEG-2 TS":(
				location_type == 4?"MPEG-2 TS packet of IPv6 data flow":(
				location_type == 5?"URL":"Unknown"))))));

			switch (location_type)
			{
			case 0:
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u(0X%X)\n", szIndent, "packet_id", packet_id, packet_id);
				break;
			case 1:
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv4_src_addr", MMTP_IPv4.ipv4_src_addr.GetIP().c_str());
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv4_dst_addr", MMTP_IPv4.ipv4_dst_addr.GetIP().c_str());

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "dst_port", MMTP_IPv4.dst_port);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u(0X%X)\n", szIndent, "packet_id", MMTP_IPv4.packet_id, MMTP_IPv4.packet_id);
				break;
			case 2:
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv6_src_addr", MMTP_IPv6.ipv6_src_addr.GetIP().c_str());
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv6_dst_addr", MMTP_IPv6.ipv6_dst_addr.GetIP().c_str());

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "dst_port", MMTP_IPv6.dst_port);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u(0X%X)\n", szIndent, "packet_id", MMTP_IPv6.packet_id, MMTP_IPv6.packet_id);
				break;
			case 3:
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d(0X%X)\n", szIndent, "network_id", M2TS_broadcast.network_id, M2TS_broadcast.network_id);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d(0X%X)\n", szIndent, "stream_id", M2TS_broadcast.MPEG_2_transport_stream_id, M2TS_broadcast.MPEG_2_transport_stream_id);

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u(0X%X)\n", szIndent, "packet_id", M2TS_broadcast.MPEG_2_PID, M2TS_broadcast.MPEG_2_PID);
				break;
			case 4:
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv6_src_addr", M2TS_IPv6.ipv6_src_addr.GetIP().c_str());
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv6_dst_addr", M2TS_IPv6.ipv6_dst_addr.GetIP().c_str());

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "dst_port", M2TS_IPv6.dst_port);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u(0X%X)\n", szIndent, "packet_id", M2TS_IPv6.MPEG_2_PID, M2TS_IPv6.MPEG_2_PID);
				break;
			case 5:
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "URL_length", URL.URL_length);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "URL_byte", URL.URL_byte);
				break;
			}

		}
	}PACKED;

	struct Table
	{
		static const char* MMT_SI_table_desc[256];

		uint64_t				start_bitpos;
		uint8_t					table_id;
		uint8_t					version;
		uint16_t				length;

		virtual int Unpack(CBitstream& bs)
		{
			int nRet = RET_CODE_SUCCESS;
			uint64_t left_bits = 0ULL;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < (4ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			table_id = bs.GetByte();
			version = bs.GetByte();
			length = bs.GetWord();

			return nRet;
		}

		virtual uint32_t LeftBits(CBitstream& bs)
		{
			uint64_t cur_bitpos = bs.Tell();

			if (cur_bitpos > start_bitpos)
			{
				uint64_t whole_packet_bits_size = ((uint64_t)(length + 4)) << 3;
				if (whole_packet_bits_size > cur_bitpos - start_bitpos)
				{
					uint64_t left_bits = whole_packet_bits_size - (cur_bitpos - start_bitpos);
					assert(left_bits <= ((uint64_t)(MAX_FULL_TLV_PACKET_LENGTH) << 3));
					return (uint32_t)left_bits;
				}
			}

			return 0UL;
		}

		virtual void SkipLeftBits(CBitstream& bs)
		{
			uint64_t left_bits = LeftBits(bs);
			if (left_bits > 0)
				bs.SkipBits(left_bits);
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, "%s++++++++++++++ MMT Table: %s +++++++++++++\n", szIndent, MMT_SI_table_desc[table_id]);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %llu\n", szIndent, "file offset", start_bitpos >> 3);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "table_id", table_id);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "version", version);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "length", length);
		}
	}PACKED;

	struct UnsupportedTable : public Table
	{
		virtual int Unpack(CBitstream& bs)
		{
			int iRet = Table::Unpack(bs);
			if (iRet < 0)
				return iRet;

			SkipLeftBits(bs);
			return iRet;
		}
	}PACKED;

	struct PackageListTable: public Table
	{
		struct DeliveryInfo
		{
			uint32_t				transport_file_id;
			uint8_t					location_type;

			union
			{
				struct
				{
					IP::V4::Address		ipv4_src_addr;
					IP::V4::Address		ipv4_dst_addr;
					uint16_t			dst_port;
				}PACKED IPv4;
				struct
				{
					IP::V6::Address		ipv6_src_addr;
					IP::V6::Address		ipv6_dst_addr;
					uint16_t			dst_port;
				}PACKED IPv6;
				struct
				{
					uint8_t				URL_length;
					uint8_t				URL_bytes[256];
				}PACKED URL;
			}PACKED;

			uint16_t				descripor_loop_length;

			int Unpack(CBitstream& bs)
			{
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < (5ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				transport_file_id = bs.GetDWord();
				location_type = bs.GetByte();
				left_bits -= (5ULL << 3);

				if (location_type == 0x01)
				{
					if (left_bits < (10ULL << 3))
						return RET_CODE_BOX_TOO_SMALL;

					IPv4.ipv4_src_addr.address = bs.GetDWord();
					IPv4.ipv4_dst_addr.address = bs.GetDWord();
					IPv4.dst_port = bs.GetWord();
					left_bits -= (10ULL << 3);
				}
				else if (location_type == 0x02)
				{
					if (left_bits < (34ULL << 3))
						return RET_CODE_BOX_TOO_SMALL;

					bs.Read(IPv6.ipv6_src_addr.address_bytes, 16);
					bs.Read(IPv6.ipv6_dst_addr.address_bytes, 16);
					IPv6.dst_port = bs.GetWord();
					left_bits -= (34ULL << 3);
				}
				else if (location_type == 0x05)
				{
					if (left_bits < 8ULL)
						return RET_CODE_BOX_TOO_SMALL;

					URL.URL_length = bs.GetByte();
					if (left_bits < ((uint64_t)URL.URL_length << 3))
						return RET_CODE_BOX_TOO_SMALL;
					memset(URL.URL_bytes, 0, sizeof(URL.URL_bytes));
					bs.Read(URL.URL_bytes, URL.URL_length);
					left_bits -= (uint64_t)URL.URL_length << 3;
				}

				if (left_bits < 8)
					return RET_CODE_BOX_TOO_SMALL;

				descripor_loop_length = bs.GetByte();

				// TODO...

				return RET_CODE_SUCCESS;
			}

			virtual void Print(FILE* fp = nullptr, int indent = 0)
			{
				FILE* out = fp ? fp : stdout;
				char szIndent[84];
				memset(szIndent, 0, _countof(szIndent));
				if (indent > 0)
				{
					int ccIndent = AMP_MIN(indent, 80);
					memset(szIndent, ' ', ccIndent);
				}

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "transport_file_id", transport_file_id);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d, %s\n", szIndent, "location_type", location_type,
					location_type == 0?"MMTP packet of the same IP data flow as IP data flow in which the table including this general_location_info is transmitted":(
					location_type == 1?"MMTP packet of IPv4 data flow":(
					location_type == 2?"MMTP packet of IPv6 data flow":(
					location_type == 3?"MPEG-2 TS packet of broadcasting network by MPEG-2 TS":(
					location_type == 4?"MPEG-2 TS packet of IPv6 data flow":(
					location_type == 5?"URL":"Unknown"))))));
				
				if (location_type == 0x01)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv4_src_addr", IPv4.ipv4_src_addr.GetIP().c_str());
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv4_dst_addr", IPv4.ipv4_dst_addr.GetIP().c_str());

					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "dst_port", IPv4.dst_port);
				}
				else if (location_type == 0x02)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv6_src_addr", IPv6.ipv6_src_addr.GetIP().c_str());
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "ipv6_dst_addr", IPv6.ipv6_dst_addr.GetIP().c_str());

					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "dst_port", IPv6.dst_port);
				}
				else if (location_type == 0x05)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "URL_length", URL.URL_length);
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "URL_byte", URL.URL_bytes);
				}

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "descripor_loop_length", descripor_loop_length);
			}

		}PACKED;

		uint8_t					num_of_packages;

		/*
		for (i=0; i<N; i++) {
			MMT_package_id_length
			for (j=0; j<M; j++) {
				MMT_package_id_byte
			}
			MMT_general_location_info ()
		}
		*/
		std::list<std::tuple<uint8_t, uint64_t, MMTGeneralLocationInfo>>
								package_infos;

		uint8_t					num_of_ip_delivery;
		DeliveryInfo*			delivery_infos = nullptr;

		// descriptor ()

		virtual ~PackageListTable()
		{
			AMP_SAFEDELA(delivery_infos);
		}

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = Table::Unpack(bs);
			if (iRet < 0)
				return iRet;

			uint64_t left_bits = 0;
			bs.Tell(&left_bits);

			if (left_bits < (uint64_t)length << 3)
				return RET_CODE_BOX_TOO_SMALL;

			left_bits = (uint64_t)length << 3;
			if (left_bits < 8ULL)
				return RET_CODE_BOX_TOO_SMALL;

			num_of_packages = bs.GetByte();
			left_bits -= 8ULL;
			for (size_t i = 0; i < num_of_packages; i++)
			{
				if (left_bits < 8ULL)
					return RET_CODE_BOX_TOO_SMALL;

				uint8_t MMT_package_id_length = bs.GetByte();
				left_bits -= 8ULL;

				if (left_bits < (uint64_t)MMT_package_id_length << 3)
					return RET_CODE_BOX_TOO_SMALL;

				uint64_t MMT_package_id = 0ULL;
				for (size_t j = 0; j < MMT_package_id_length; j++)
					MMT_package_id = (MMT_package_id << 8) | bs.GetByte();
				left_bits -= (uint64_t)MMT_package_id_length << 3;

				MMTGeneralLocationInfo MMT_general_location_info;
				if (MMT_general_location_info.Unpack(bs) < 0)
					break;

				uint16_t MMT_general_loc_info_size = MMT_general_location_info.GetLength();
				if (left_bits < (uint64_t)MMT_general_loc_info_size << 3)
					return RET_CODE_BOX_INCOMPATIBLE;

				package_infos.push_back(std::make_tuple(MMT_package_id_length, MMT_package_id, MMT_general_location_info));
				left_bits -= (uint64_t)MMT_general_loc_info_size << 3;
			}

			if (left_bits < 8)
				return RET_CODE_BOX_INCOMPATIBLE;

			num_of_ip_delivery = bs.GetByte();
			if (num_of_ip_delivery > 0)
			{
				delivery_infos = new DeliveryInfo[num_of_ip_delivery];
				for (int i = 0; i < num_of_ip_delivery; i++)
				{
					if (delivery_infos[i].Unpack(bs) < 0)
						break;
				}
			}

			return RET_CODE_SUCCESS;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			Table::Print(fp, indent);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "num_of_packages", num_of_packages);

			for (auto& v: package_infos)
			{
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%llX\n", szIndent, "package_id", std::get<1>(v));
				auto& info = std::get<2>(v);
				info.Print(fp, indent + 4);
			}

			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "num_of_ip_delivery", num_of_ip_delivery);
			if (num_of_ip_delivery > 0)
			{
				for (int i = 0; i < num_of_ip_delivery; i++)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "ip_delivery idx", i);
					delivery_infos[i].Print(fp, indent + 4);
				}
			}
		}
	}PACKED;

	struct MMTPackageTable : public Table
	{
		struct Asset
		{
			uint8_t					identifier_type;
			uint32_t				asset_id_scheme;
			uint8_t					asset_id_length;
			uint64_t				asset_id;
			uint32_t				asset_type;
			uint8_t					reserved : 7;
			uint8_t					asset_clock_relation_flag : 1;
			uint8_t					location_count;
			std::vector<MMTGeneralLocationInfo>
									MMT_general_location_infos;
			uint16_t				asset_descriptors_length;
			std::vector<uint8_t>	asset_descriptors_bytes;

			uint32_t GetLength() {
				uint32_t cbLen = sizeof(identifier_type) + sizeof(asset_id_scheme) + sizeof(asset_id_length);
				cbLen += asset_id_length;
				cbLen += sizeof(asset_type);
				cbLen += 2;
				
				for (size_t i = 0; i < location_count; i++)
					cbLen += MMT_general_location_infos[i].GetLength();

				cbLen += 2;
				cbLen += asset_descriptors_length;
				return cbLen;
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < (6ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				identifier_type = bs.GetByte();
				asset_id_scheme = bs.GetDWord();
				asset_id_length = bs.GetByte();
				left_bits -= 6ULL << 3;

				if (left_bits < (uint64_t)asset_id_length << 3)
					return RET_CODE_BOX_TOO_SMALL;

				asset_id = 0ULL;
				for (size_t i = 0; i < asset_id_length; i++)
					asset_id = ((uint64_t)asset_id << 3) | bs.GetByte();

				left_bits -= (uint64_t)asset_id_length << 3;

				if (left_bits < (6ULL<<3))
					return RET_CODE_BOX_TOO_SMALL;

				asset_type = bs.GetDWord();
				reserved = (uint8_t)bs.GetBits(7);
				asset_clock_relation_flag = (uint8_t)bs.GetBits(1);

				location_count = bs.GetByte();
				left_bits -= (6ULL << 3);

				if (location_count > 0)
				{
					MMT_general_location_infos.reserve(location_count);
					for (size_t i = 0; i < location_count; i++)
					{
						if (left_bits < (2ULL << 3))
							return RET_CODE_BOX_TOO_SMALL;

						MMT_general_location_infos.emplace_back();
						auto& back = MMT_general_location_infos.back();

						if ((iRet = back.Unpack(bs)) < 0)
							break;

						uint16_t loc_info_len  = back.GetLength();
						if (left_bits < ((uint64_t)loc_info_len << 3))
						{
							iRet = RET_CODE_BUFFER_NOT_COMPATIBLE;
							break;
						}

						left_bits -= (uint64_t)loc_info_len << 3;
					}
				}

				if (left_bits < (2ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				asset_descriptors_length = bs.GetWord();
				if (left_bits < ((uint64_t)asset_descriptors_length << 3))
					return RET_CODE_BOX_TOO_SMALL;

				if (asset_descriptors_length > 0)
				{
					asset_descriptors_bytes.resize(asset_descriptors_length);
					bs.Read(&asset_descriptors_bytes[0], asset_descriptors_length);
				}

				return iRet;
			}

			virtual void Print(FILE* fp = nullptr, int indent = 0)
			{
				FILE* out = fp ? fp : stdout;
				char szIndent[84];
				memset(szIndent, 0, _countof(szIndent));
				if (indent > 0)
				{
					int ccIndent = AMP_MIN(indent, 80);
					memset(szIndent, ' ', ccIndent);
				}

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d(0X%X)\n", szIndent, "identifier_type", identifier_type, identifier_type);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "asset_id_scheme", asset_id_scheme);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "asset_id_length", asset_id_length);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %llu\n", szIndent, "asset_id", asset_id);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %c%c%c%c (0X%08X), %s\n", szIndent, "asset_type",
					isprint((asset_type >> 24) & 0xFF) ? (asset_type >> 24) & 0xFF : '.',
					isprint((asset_type >> 16) & 0xFF) ? (asset_type >> 16) & 0xFF : '.',
					isprint((asset_type >> 8) & 0xFF) ? (asset_type >> 8) & 0xFF : '.',
					isprint((asset_type) & 0xFF) ? (asset_type) & 0xFF : '.', asset_type, 
					asset_type == 'hvc1'?"HEVC, which includes VPS, SPS and PPS only in MPU metadata":(
					asset_type == 'hev1'?"HEVC, which includes VPS, SPS and PSS in MFU":(
					asset_type == 'mp4a'?"AAC audio":(
					asset_type == 'stpp'?"Timed text (closed-caption and superimposition)":(
					asset_type == 'aapp'?"Application":(
					asset_type == 'asgd'?"Synchronous type general-purpose data":(
					asset_type == 'aagd'?"Asynchronous type general-purpose data":"")))))));
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "asset_clock_relation_flag", asset_clock_relation_flag);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "location_count", location_count);
				for (size_t i = 0; i < MMT_general_location_infos.size(); i++)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "location idx", i);
					MMT_general_location_infos[i].Print(fp, indent + 4);
				}
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "asset_descriptors_length", asset_descriptors_length);
				if (asset_descriptors_length > 0)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": ", szIndent, "asset_desc_bytes");
					for (size_t i = 0; i < AMP_MIN(256UL, asset_descriptors_bytes.size()); i++)
					{
						if (i != 0 && i % 16 == 0)
							fprintf(out, "\n" MMT_FIX_HEADER_FMT_STR "  ", szIndent, "");
						fprintf(out, "%02X ", asset_descriptors_bytes[i]);
					}
					fprintf(out, "\n");
					if (asset_descriptors_bytes.size() > 256)
						fprintf(out, MMT_FIX_HEADER_FMT_STR "  ...\n", szIndent, "");
				}
			}

		}PACKED;

		uint8_t					reserved : 6;
		uint8_t					MPT_mode : 2;

		uint8_t					MMT_package_id_length;
		uint64_t				MMT_package_id;
		
		uint16_t				MPT_descriptors_length;
		std::vector<uint8_t>	MPT_descriptors_bytes;

		uint8_t					number_of_assets;
		std::vector<Asset*>		assets;

		virtual ~MMTPackageTable()
		{
			for (auto& v : assets)
				if (v != nullptr)
					delete v;
		}

		int Unpack(CBitstream& bs)
		{
			int iRet = Table::Unpack(bs);
			if (iRet < 0)
				return iRet;

			uint64_t left_bits = 0;
			bs.Tell(&left_bits);
			if (left_bits < 16ULL)
				return RET_CODE_BOX_TOO_SMALL;

			reserved = (uint8_t)bs.GetBits(6);
			MPT_mode = (uint8_t)bs.GetBits(2);
			MMT_package_id_length = bs.GetByte();
			left_bits -= 16ULL;

			if (left_bits < ((uint64_t)MMT_package_id_length<<3))
				return RET_CODE_BOX_TOO_SMALL;

			MMT_package_id = 0;
			for (size_t i = 0; i < MMT_package_id_length; i++)
				MMT_package_id = (MMT_package_id << 8) | bs.GetByte();

			left_bits -= (uint64_t)MMT_package_id_length << 3;

			if (left_bits < 16ULL)
				return RET_CODE_BOX_TOO_SMALL;

			MPT_descriptors_length = bs.GetWord();
			left_bits -= 16ULL;

			if (MPT_descriptors_length > 0)
			{
				if (left_bits < ((uint64_t)MPT_descriptors_length << 3))
					return RET_CODE_BOX_TOO_SMALL;

				bs.Read(&MPT_descriptors_bytes[0], MPT_descriptors_length);
				left_bits -= (uint64_t)MPT_descriptors_length << 3;
			}

			if (left_bits < 8ULL)
				return RET_CODE_BOX_TOO_SMALL;

			number_of_assets = bs.GetByte();

			for (size_t i = 0; left_bits > 0 && i < number_of_assets; i++)
			{
				Asset* ptr_asset = new Asset();
				if ((iRet = ptr_asset->Unpack(bs)) < 0)
				{
					delete ptr_asset;
					break;
				}
				assets.push_back(ptr_asset);

				uint32_t cbLen = ptr_asset->GetLength();
				if (left_bits < ((uint64_t)cbLen << 3))
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					break;
				}

				left_bits -= (uint64_t)cbLen << 3;
			}

			return iRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			Table::Print(fp, indent);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d(0X%X), %s\n", szIndent, "identifier_type", MPT_mode, MPT_mode,
				MPT_mode == 0?"MPT is processed according to the order of subset":(
				MPT_mode == 1?"After MPT of subset 0 is received, any subset which has the same version number can be processed":(
				MPT_mode == 2?"MPT of subset can be processed arbitrarily":(
				MPT_mode == 3?"reserved":"Unknown"))));
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %llu\n", szIndent, "MMT_package_id", MMT_package_id);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "MPT_descs_length", MPT_descriptors_length);
			if (MPT_descriptors_length > 0)
			{
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": ", szIndent, "MPT_descs_bytes");
				for (size_t i = 0; i < AMP_MIN(256UL, MPT_descriptors_bytes.size()); i++)
				{
					if (i != 0 && i % 16 == 0)
						fprintf(out, "\n" MMT_FIX_HEADER_FMT_STR "  ", szIndent, "");
					fprintf(out, "%02X ", MPT_descriptors_bytes[i]);
				}
				fprintf(out, "\n");
				if (MPT_descriptors_bytes.size() > 256)
					fprintf(out, MMT_FIX_HEADER_FMT_STR "  ...\n", szIndent, "");
			}

			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "number_of_assets", number_of_assets);
			if (number_of_assets > 0)
			{
				for (size_t i = 0; i < assets.size(); i++)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "asset idx", i);
					assets[i]->Print(fp, indent + 4);
				}
			}
		}

	}PACKED;

	struct Message
	{
		uint64_t				start_bitpos;
		uint16_t				message_id;
		uint8_t					version;
		uint32_t				length;

		// If the message payload size is 0, it means that don't know the message payload size
		uint32_t				message_payload_size = 0;

		Message(uint32_t cbPayload = 0) : message_payload_size(cbPayload) {
		}

		virtual ~Message() {}

		static inline const char* GetMessageName(uint16_t msg_id)
		{
			if (msg_id == 0)
				return "PA message";
			else if (msg_id >= 1 && msg_id <= 0xF)
				return "MPI message";
			else if (msg_id >= 0x10 && msg_id <= 0x1F)
				return "MPT message";
			else if (msg_id == 0x0200)
				return "CRI message";
			else if (msg_id == 0x0201)
				return "DCI message";
			else if (msg_id == 0x0202)
				return "AL-FEC message";
			else if (msg_id == 0x0203)
				return "HRBM message";
			else if (msg_id >= 0x0204 && msg_id <= 0x6FFF)
				return "reserved for ISO/IEC (16-bit length message)";
			else if (msg_id >= 0x7000 && msg_id <= 0x7FFF)
				return "reserved for ISO/IEC (32-bit length message)";
			else if (msg_id == 0x8000)
				return "M2 section message";
			else if (msg_id == 0x8001)
				return "CA message";
			else if (msg_id == 0x8002)
				return "M2 short section message";
			else if (msg_id == 0x8003)
				return "Data transmission message";
			else if (msg_id >= 0x8004 && msg_id <= 0xDFFF)
				return "reserved(16-bit length)";
			else if (msg_id >= 0xE000 && msg_id <= 0xEFFF)
				return "message which is prepared by broadcasters(16-bit length)";
			else if (msg_id >= 0xF000 && msg_id <= 0xF7FF)
				return "reserved(32-bit length)";
			else if (msg_id >= 0xF800 && msg_id <= 0xFFFF)
				return "message which is prepared by broadcasters(32-bit length)";

			return "reserved";
		}

		virtual int Unpack(CBitstream& bs)
		{
			int nRet = RET_CODE_SUCCESS;
			uint64_t left_bits = 0ULL;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < (4ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			message_id = bs.GetWord();
			version = bs.GetByte();
			length = bs.GetDWord();

			return nRet;
		}

		virtual uint32_t LeftBits(CBitstream& bs)
		{
			uint64_t cur_bitpos = bs.Tell();

			if (cur_bitpos > start_bitpos)
			{
				uint64_t whole_packet_bits_size = ((uint64_t)(length + 7)) << 3;
				if (whole_packet_bits_size > cur_bitpos - start_bitpos)
				{
					uint64_t left_bits = whole_packet_bits_size - (cur_bitpos - start_bitpos);
					assert(left_bits <= ((uint64_t)(MAX_FULL_TLV_PACKET_LENGTH) << 3));
					return (uint32_t)left_bits;
				}
			}

			return 0UL;
		}

		virtual void SkipLeftBits(CBitstream& bs)
		{
			uint64_t left_bits = LeftBits(bs);
			if (left_bits > 0)
				bs.SkipBits(left_bits);
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, "%s++++++++++++++ Message: %s +++++++++++++\n", szIndent, GetMessageName(message_id));
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "message_id", message_id);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "version", version);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "length", length);
		}
	}PACKED;

	struct UnimplementedMessage : public Message
	{
		int Unpack(CBitstream& bs)
		{
			int iRet = Message::Unpack(bs);
			if (iRet < 0)
				return iRet;

			SkipLeftBits(bs);
			return iRet;
		}
	};

	struct PAMessage: public Message
	{
		uint8_t					number_of_tables;

		// std::vector<std::tuple<table_id(8bit), table_version(8bit) and table_length(16bit)>>
		std::vector<std::tuple<uint8_t, uint8_t, uint16_t>>
								table_headers;

		std::vector<Table*>		tables;
		std::vector<uint8_t>	unparsed_data;

		PAMessage(uint32_t cbPayload = 0) : Message(cbPayload) {
		}

		virtual ~PAMessage() {
			for (auto v : tables)
				if (v != nullptr)
					delete v;
		}

		// Always assume the table information bitstream is complete
		int Unpack(CBitstream& bs)
		{
			int iRet = Message::Unpack(bs);
			if (iRet < 0)
				return iRet;

			uint64_t left_bits = 0;
			bs.Tell(&left_bits);
			if (left_bits < 8ULL)
				return RET_CODE_BOX_TOO_SMALL;

			number_of_tables = bs.GetByte();
			left_bits -= 8ULL;

			for (int i = 0; i < number_of_tables; i++)
			{
				if (left_bits < 32ULL)
					return RET_CODE_BOX_TOO_SMALL;

				uint8_t table_id = bs.GetByte();
				uint8_t table_version = bs.GetByte();
				uint16_t table_length = bs.GetWord();

				left_bits -= 32ULL;
				table_headers.push_back(std::make_tuple(table_id, table_version, table_length));
			}

			while (left_bits >= 32ULL)
			{
				uint32_t peek_dword = (uint32_t)bs.PeekBits(32);
				uint8_t peek_table_id = (peek_dword >> 24) & 0xFF;
				uint16_t peek_table_length = (uint16_t)(peek_dword & 0xFFFF);

				if (left_bits < ((uint64_t)peek_table_length + 4ULL) << 3)
					break;

				Table* ptr_table = nullptr;
				if (peek_table_id == 0)	// PA Table
				{
					// TODO...
					ptr_table = new UnsupportedTable();
				}
				else if (peek_table_id == 0x80)	// PLT
				{
					ptr_table = new PackageListTable();
				}
				else if (peek_table_id == 0x20) // MPT
				{
					ptr_table = new MMTPackageTable();
				}
				else
				{
					// TODO...
					ptr_table = new UnsupportedTable();
				}

				tables.push_back(ptr_table);
				if ((iRet = ptr_table->Unpack(bs)) < 0)
					break;

				left_bits -= ((uint64_t)peek_table_length + 4ULL) << 3;
			}

			return RET_CODE_SUCCESS;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			Message::Print(fp, indent);

			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "number_of_tables", number_of_tables);
			for (int i = 0; i < number_of_tables; i++)
			{
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "table_index", i);
				fprintf(out, "    " MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "table_id", std::get<0>(table_headers[i]));
				fprintf(out, "    " MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "table_version", std::get<1>(table_headers[i]));
				fprintf(out, "    " MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "table_length", std::get<2>(table_headers[i]));
			}

			for (auto& v : tables)
			{
				if (v == nullptr)
					continue;

				v->Print(fp, indent + 4);
			}
		}
	}PACKED;

	// MPEG Media Transport Protocol packet
	struct MMTPPacket
	{
		struct MPU
		{
			struct DataUnit
			{
				uint16_t		data_unit_length;
				union
				{
					uint32_t		movie_fragment_sequence_number;
					uint32_t		item_id;
				}PACKED;
				uint32_t		sample_number;
				uint32_t		offset;
				uint8_t			priority;
				uint8_t			dependency_counter;
				std::vector<uint8_t>
								MFU_data_bytes;
			}PACKED;

			uint16_t			Payload_length;
			
			uint16_t			Fragment_type : 4;
			uint16_t			Time_data_flag : 1;
			uint16_t			Division_index : 2;
			uint16_t			Aggregate_flag : 1;
			uint16_t			Division_number_counter : 8;

			uint32_t			MPU_sequence_number;

			std::list<DataUnit>	Data_Units;

			int32_t				payload_data_len;
			std::vector<uint8_t>
								payload;

			MPU(int nPayloadDataLen) : payload_data_len(nPayloadDataLen) {
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < 64ULL)
					return RET_CODE_BOX_TOO_SMALL;

				Payload_length = bs.GetWord();
				Fragment_type = (uint16_t)bs.GetBits(4);
				Time_data_flag = (uint16_t)bs.GetBits(1);
				Division_index = (uint16_t)bs.GetBits(2);
				Aggregate_flag = (uint16_t)bs.GetBits(1);

				Division_number_counter = bs.GetByte();

				MPU_sequence_number = bs.GetDWord();

				left_bits -= 64;
				int32_t left_payload_data_len = payload_data_len - 8;

				if (Fragment_type == 2)
				{
					if (Time_data_flag == 1)
					{
						if (Aggregate_flag == 0)
						{
							if (left_bits < (14ULL << 3))
								return RET_CODE_BOX_TOO_SMALL;

							Data_Units.emplace_back();
							auto& back = Data_Units.back();
							back.data_unit_length = left_payload_data_len;
							back.movie_fragment_sequence_number = bs.GetDWord();
							back.sample_number = bs.GetDWord();
							back.offset = bs.GetDWord();
							back.priority = bs.GetByte();
							back.dependency_counter = bs.GetByte();

							left_bits -= 14ULL << 3;
							left_payload_data_len -= 14;

							if (left_payload_data_len > 0  && left_bits > 0)
							{
								size_t actual_payload_size = (size_t)AMP_MIN(left_payload_data_len, (left_bits >> 3));
								if (actual_payload_size > 0)
								{
									back.MFU_data_bytes.resize(actual_payload_size);
									bs.Read(&back.MFU_data_bytes[0], actual_payload_size);
									left_bits -= (uint64_t)actual_payload_size << 3;
									left_payload_data_len -= actual_payload_size;
								}
							}
						}
						else
						{
							while (left_payload_data_len > 16)
							{
								if (left_bits < (16ULL << 3))
									return RET_CODE_BOX_TOO_SMALL;

								Data_Units.emplace_back();
								auto& back = Data_Units.back();
								back.data_unit_length = bs.GetWord();
								back.movie_fragment_sequence_number = bs.GetDWord();
								back.sample_number = bs.GetDWord();
								back.offset = bs.GetDWord();
								back.priority = bs.GetByte();
								back.dependency_counter = bs.GetByte();

								if (left_bits < ((uint64_t)back.data_unit_length << 3) + 16 ||
									back.data_unit_length < 14)
									return RET_CODE_BOX_TOO_SMALL;

								left_bits -= 16ULL << 3;
								left_payload_data_len -= 16;

								if (left_payload_data_len > 0 && left_bits > 0)
								{
									size_t actual_payload_size = (size_t)AMP_MIN(back.data_unit_length - 14, (left_bits >> 3));
									if (actual_payload_size > 0)
									{
										back.MFU_data_bytes.resize(actual_payload_size);
										bs.Read(&back.MFU_data_bytes[0], actual_payload_size);
										left_bits -= (uint64_t)actual_payload_size << 3;
										left_payload_data_len -= actual_payload_size;
									}
								}
							}
						}
					}
					else // non-timed data
					{
						if (Aggregate_flag == 0)
						{
							if (left_bits < 32ULL)
								return RET_CODE_BOX_TOO_SMALL;

							Data_Units.emplace_back();
							auto& back = Data_Units.back();
							back.item_id = bs.GetDWord();

							left_bits -= 32ULL;
							left_payload_data_len -= 4;

							if (left_payload_data_len > 0 && left_bits > 0)
							{
								size_t actual_payload_size = (size_t)AMP_MIN(left_payload_data_len, (left_bits >> 3));
								if (actual_payload_size > 0)
								{
									back.MFU_data_bytes.resize(actual_payload_size);
									bs.Read(&back.MFU_data_bytes[0], actual_payload_size);
									left_bits -= (uint64_t)actual_payload_size << 3;
									left_payload_data_len -= actual_payload_size;
								}
							}
						}
						else
						{
							while (left_payload_data_len > 6)
							{
								if (left_bits < (6ULL << 3))
									return RET_CODE_BOX_TOO_SMALL;

								Data_Units.emplace_back();
								auto& back = Data_Units.back();
								back.data_unit_length = bs.GetWord();
								back.item_id = bs.GetDWord();

								if (left_bits < ((uint64_t)back.data_unit_length << 3) + 16 ||
									back.data_unit_length < 4)
									return RET_CODE_BOX_TOO_SMALL;

								left_bits -= 6ULL << 3;
								left_payload_data_len -= 6;

								if (left_payload_data_len > 0 && left_bits > 0)
								{
									size_t actual_payload_size = (size_t)AMP_MIN(back.data_unit_length - 4, (left_bits >> 3));
									if (actual_payload_size > 0)
									{
										back.MFU_data_bytes.resize(actual_payload_size);
										bs.Read(&back.MFU_data_bytes[0], actual_payload_size);
										left_bits -= (uint64_t)actual_payload_size << 3;
										left_payload_data_len -= actual_payload_size;
									}
								}
							} // while (left_payload_data_len > 6)
						}
					}
				}

				if (left_payload_data_len > 0 && left_bits > 0)
				{
					size_t actual_payload_size = (size_t)AMP_MIN(left_payload_data_len, (left_bits >> 3));
					if (actual_payload_size > 0)
					{
						payload.resize(actual_payload_size);
						bs.Read(&payload[0], actual_payload_size);
					}
				}

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

				fprintf(out, "%s%s\n", szIndent, "++++++++++++++++ MPU +++++++++++++");
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Payload_length", Payload_length);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d, %s\n", szIndent, "Fragment_type", Fragment_type,
					Fragment_type == 0?"MPU metadata":(
					Fragment_type == 1?"Movie fragment metadata":(
					Fragment_type == 2?"MFU":"Reserved")));
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Time_data_flag", Time_data_flag);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d, %s\n", szIndent, "Division_index", Division_index,
					Division_index == 0?"Undivided":(
					Division_index == 1?"Divided, Including the head part of the data before division":(
					Division_index == 2?"Divided, Not including the head part and end part of the data before division":(
					Division_index == 3?"Divided, Including the end part of the data before division":"Reserved"))));
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Aggregate_flag", Aggregate_flag);

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Div_number_counter", Division_number_counter);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "MPU_sequence_number", MPU_sequence_number);

				if (payload.size() > 0)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": ", szIndent, "Payload");
					for (size_t i = 0; i < AMP_MIN(256UL, payload.size()); i++)
					{
						if (i != 0 && i % 16 == 0)
							fprintf(out, "\n" MMT_FIX_HEADER_FMT_STR "  ", szIndent, "");
						fprintf(out, "%02X ", payload[i]);
					}
					fprintf(out, "\n");
					if (payload.size() > 256)
						fprintf(out, MMT_FIX_HEADER_FMT_STR "  ...\n", szIndent, "");
				}
			}

		}PACKED;

		struct ControlMessages
		{
			uint8_t				fragmentation_indicator : 2;
			uint8_t				reserved : 4;
			uint8_t				length_extension_flag : 1;
			uint8_t				Aggregate_flag : 1;

			uint8_t				fragment_counter;

			std::list<std::tuple<uint32_t, std::vector<uint8_t>>>
								messages;

			int32_t				payload_data_len;
			std::vector<uint8_t>
								payload;

			ControlMessages(int nPayloadDataLen) : payload_data_len(nPayloadDataLen) {
			}

			~ControlMessages(){
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < 16)
					return RET_CODE_BOX_TOO_SMALL;

				fragmentation_indicator = (uint8_t)bs.GetBits(2);
				reserved = (uint8_t)bs.GetBits(4);
				length_extension_flag = (uint8_t)bs.GetBits(1);
				Aggregate_flag = (uint8_t)bs.GetBits(1);

				fragment_counter = bs.GetByte();

				left_bits -= 16;
				int32_t left_payload_data_len = payload_data_len - 2;

				if (Aggregate_flag == 0)
				{
					if (left_payload_data_len > 0 && left_bits > 0)
					{
						size_t actual_payload_size = (size_t)AMP_MIN(left_payload_data_len, (left_bits >> 3));
						if (actual_payload_size > 0)
						{
							messages.push_back(std::make_tuple(actual_payload_size, std::vector<uint8_t>()));
							auto& message_bytes = std::get<1>(messages.back());
							message_bytes.resize(actual_payload_size);
							bs.Read(&message_bytes[0], actual_payload_size);
							left_payload_data_len = 0;
						}
					}
				}
				else
				{
					while (left_payload_data_len > (length_extension_flag?4:2))
					{
						if (left_bits < ((length_extension_flag ? 4 : 2) << 3))
							return RET_CODE_BOX_TOO_SMALL;

						uint32_t message_length = bs.GetDWord();
						messages.push_back(std::make_tuple(message_length, std::vector<uint8_t>()));
						auto& message_bytes = std::get<1>(messages.back());

						if (left_bits < ((uint64_t)message_length << 3) + (length_extension_flag?32:16) ||
							message_length == 0)
							return RET_CODE_BOX_TOO_SMALL;

						left_bits -= (length_extension_flag ? 32 : 16);
						left_payload_data_len -= (length_extension_flag ? 4 : 2);

						if (left_payload_data_len > 0 && left_bits > 0)
						{
							size_t actual_message_size = AMP_MIN((size_t)message_length, (size_t)left_payload_data_len);
							size_t actual_payload_size = (size_t)AMP_MIN(actual_message_size, (left_bits >> 3));
							if (actual_payload_size > 0)
							{
								message_bytes.resize(actual_payload_size);
								bs.Read(&message_bytes[0], actual_payload_size);
								left_bits -= (uint64_t)actual_payload_size << 3;
								left_payload_data_len -= actual_payload_size;
							}
						}
					} // while (left_payload_data_len > 6)
				}

				if (left_payload_data_len > 0 && left_bits > 0)
				{
					size_t actual_payload_size = (size_t)AMP_MIN(left_payload_data_len, (left_bits >> 3));
					if (actual_payload_size > 0)
					{
						payload.resize(actual_payload_size);
						bs.Read(&payload[0], actual_payload_size);
					}
				}

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

				fprintf(out, "%s%s\n", szIndent, "++++++++++++++Control Messages+++++++++++++");
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d, %s\n", szIndent, "frag_indicator", fragmentation_indicator,
					fragmentation_indicator == 0?"Undivided":(
					fragmentation_indicator == 1?"Divided, Including the head part of the data before division":(
					fragmentation_indicator == 2?"Divided, Not including the head part and end part of the data before division":(
					fragmentation_indicator == 3?"Divided, Including the end part of the data before division":"Reserved"))));
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Len_info_ext_flag", length_extension_flag);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Aggregate_flag", Aggregate_flag);

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "fragment_counter", fragment_counter);

				if (fragmentation_indicator == 0)
				{
					for (auto& v : messages)
					{
						auto& msg_payload = std::get<1>(v);
						CBitstream msg_bs(&msg_payload[0], msg_payload.size() << 3);
						uint16_t peek_msg_id = (uint16_t)msg_bs.PeekBits(16);
						if (peek_msg_id == 0)
						{
							PAMessage* ptr_PAMsg = new PAMessage(msg_payload.size());
							if (ptr_PAMsg->Unpack(msg_bs) >= 0)
							{
								ptr_PAMsg->Print(fp, indent + 4);
							}
							delete ptr_PAMsg;
						}
						else
						{
							UnimplementedMessage* pUnimplMsg = new UnimplementedMessage();
							if (pUnimplMsg->Unpack(msg_bs) >= 0)
							{
								pUnimplMsg->Print(fp, indent + 4);
							}
							delete pUnimplMsg;
						}
					}
				}

				if (payload.size() > 0)
				{
					fprintf(out, MMT_FIX_HEADER_FMT_STR ": ", szIndent, "Payload");
					for (size_t i = 0; i < AMP_MIN(256, payload.size()); i++)
					{
						if (i != 0 && i % 16 == 0)
							fprintf(out, "\n" MMT_FIX_HEADER_FMT_STR "  ", szIndent, "");
						fprintf(out, "%02X ", payload[i]);
					}
					fprintf(out, "\n");
					if (payload.size() > 256)
						fprintf(out, MMT_FIX_HEADER_FMT_STR "  ...\n", szIndent, "");
				}
			}

		}PACKED;

		uint64_t			start_bitpos;
		uint32_t			Version : 2;
		uint32_t			Packet_counter_flag : 1;
		uint32_t			FEC_type : 2;
		uint32_t			Reserved_0 : 1;
		uint32_t			Extension_header_flag : 1;
		uint32_t			RAP_flag : 1;

		uint32_t			Reserved_1 : 2;
		uint32_t			Payload_type : 6;
		uint32_t			Packet_id : 16;

		IP::NTPv4Data::NTPShortFormat
							Delivery_timestamp;
		uint32_t			Packet_sequence_number;

		uint32_t			Packet_counter;

		uint16_t			Extension_header_type;
		uint16_t			Extension_header_length;

		int32_t				packet_data_len;

		union
		{
			uint8_t*			Extension_header_field;
		}PACKED;

		union
		{
			MPU*				ptr_MPU;
			ControlMessages*	ptr_Messages;
			uint8_t*			MMTP_payload_data;
		};

		MMTPPacket(int nPacketDataLen): packet_data_len(nPacketDataLen) {
			Extension_header_field = nullptr;
			MMTP_payload_data = nullptr;
		}

		~MMTPPacket()
		{
			if (Extension_header_flag)
			{
				AMP_SAFEDELA(Extension_header_field);
			}

			if (Payload_type == 0)
			{
				AMP_SAFEDEL(ptr_MPU);
			}
			else if (Payload_type == 2)
			{
				AMP_SAFEDEL(ptr_Messages);
			}
		}

		int Unpack(CBitstream& bs)
		{
			int nRet = RET_CODE_SUCCESS;
			uint64_t left_bits = 0ULL;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < (12ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			Version = (uint8_t)bs.GetBits(2);
			Packet_counter_flag = (uint8_t)bs.GetBits(1);
			FEC_type = (uint8_t)bs.GetBits(2);
			Reserved_0 = (uint8_t)bs.GetBits(1);
			Extension_header_flag = (uint8_t)bs.GetBits(1);
			RAP_flag = (uint8_t)bs.GetBits(1);

			Reserved_1 = (uint8_t)bs.GetBits(2);
			Payload_type = (uint8_t)bs.GetBits(6);

			Packet_id = bs.GetWord();

			Delivery_timestamp.Seconds = bs.GetWord();
			Delivery_timestamp.Fraction = bs.GetWord();
			Packet_sequence_number = bs.GetDWord();

			left_bits -= (12ULL << 3);

			if (Packet_counter_flag)
			{
				if (left_bits < (4ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				Packet_counter = bs.GetDWord();
				left_bits -= (4ULL << 3);
			}

			if (Extension_header_flag)
			{
				if (left_bits < (4ULL << 3))
					return RET_CODE_BOX_TOO_SMALL;

				Extension_header_type = bs.GetWord();
				Extension_header_length = bs.GetWord();
				left_bits -= (4ULL << 3);

				if (left_bits < ((uint64_t)Extension_header_length << 3))
					return RET_CODE_BOX_TOO_SMALL;

				// TODO for other cases
				Extension_header_field = new uint8_t[Extension_header_length];
				if (bs.Read(Extension_header_field, Extension_header_length) < (int)Extension_header_length)
				{
					delete[] Extension_header_field;
					Extension_header_field = nullptr;
					return RET_CODE_BOX_TOO_SMALL;
				}

				left_bits -= (uint64_t)Extension_header_length << 3;
			}

			uint64_t cur_bit_pos = bs.Tell();
			int left_unparsed_data_len = (int)((cur_bit_pos - start_bitpos) >> 3);

			if (Payload_type == 0)
			{
				ptr_MPU = new MPU(packet_data_len - left_unparsed_data_len);
				nRet = ptr_MPU->Unpack(bs);
			}
			else if (Payload_type == 2)
			{
				ptr_Messages = new ControlMessages(packet_data_len - left_unparsed_data_len);
				nRet = ptr_Messages->Unpack(bs);
			}
			else
			{
				printf("Does NOT support payload type: %d.\n", Payload_type);
			}

			return nRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, "%s%s\n", szIndent, "++++++++++++++ MMTP Packet +++++++++++++");
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %llu\n", szIndent, "file offset", start_bitpos >> 3);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%X\n", szIndent, "Version", Version);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Packet_counter_flag", Packet_counter_flag);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %s\n", szIndent, "FEC_type", FEC_type == 0?"Non-protected MMTP packet by AL-FEC":(
																				FEC_type == 1?"Source packet among MMTP packets protected by AL-FEC":(
																				FEC_type == 2?"Repair packet among MMTP packets protected by AL-FEC":(
																				FEC_type == 3?"Reserved":"Unknown"))));
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Extension_header_flag", Extension_header_flag);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "RAP_flag", RAP_flag);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d, %s\n", szIndent, "Payload_type", Payload_type, 
				Payload_type == 0?"MPU, a media-aware fragment of the MPU":(
				Payload_type == 1?"Generic object, A generic object such as a complete MPU or an object of another type":(
				Payload_type == 2?"signalling message, one or more signalling messages or a fragment of a signalling message":(
				Payload_type == 3?"repair symbol":"Undefined"))));
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": 0X%04X(%d)\n", szIndent, "Packet_id", Packet_id, Packet_id);

			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d.%ds\n", szIndent, "Delivery_timestamp", Delivery_timestamp.Seconds, Delivery_timestamp.Fraction);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Pkt_sequence_number", Packet_sequence_number);

			if (Packet_counter_flag)
			{
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "Packet_counter", Packet_counter);
			}

			if (Extension_header_flag)
			{
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "Ext_header_type", Extension_header_type);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %lu\n", szIndent, "Ext_header_length", Extension_header_length);

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": ", szIndent, "header_ext_value");
				for (int i = 0; Extension_header_field != nullptr && i < AMP_MIN(256, Extension_header_length); i++)
				{
					if (i != 0 && i%16 == 0)
						fprintf(out, "\n" MMT_FIX_HEADER_FMT_STR "  ", szIndent, "");
					fprintf(out, "%02X ", Extension_header_field[i]);
				}
				fprintf(out, "\n");
				if (Extension_header_length > 256)
					fprintf(out, MMT_FIX_HEADER_FMT_STR "  ...\n", szIndent, "");
			}

			if (Payload_type == 0)
			{
				if (ptr_MPU != nullptr)
					ptr_MPU->Print(fp, indent + 4);
			}
			else if (Payload_type == 2)
			{
				if (ptr_Messages != nullptr)
					ptr_Messages->Print(fp, indent + 4);
			}
			
		}

	}PACKED;

	struct HeaderCompressedIPPacket : public TLVPacket
	{
		uint16_t				Context_id : 12;
		uint16_t				Sequence_number : 4;

		uint8_t					Header_type;

		union
		{
			IP::V4::Header*		IPv4_header;
			IP::V6::Header*		IPv6_header;
			void*				ptr_header = nullptr;
			uint16_t			IPv4_header_Identifier;
		};

		union
		{
			IP::UDPHeader*		UDP_header;
			void*				pData = nullptr;
		};

		MMTPPacket*				MMTP_Packet = nullptr;

		virtual ~HeaderCompressedIPPacket()
		{
			switch (Header_type)
			{
			case 0x20:
				break;
			case 0x21:
				break;
			case 0x60:
				if (IPv6_header != nullptr)
				{
					if (IPv6_header->payload_type == IP::V6::PROTOCOL_UDP)
					{
						AMP_SAFEDEL(MMTP_Packet);
						AMP_SAFEDEL(UDP_header);
					}

					AMP_SAFEDEL(IPv6_header);
				}
				break;
			case 0x61:
				AMP_SAFEDEL(MMTP_Packet);
				break;
			}
		}

		int Unpack(CBitstream& bs)
		{
			int iRet = TLVPacket::Unpack(bs);
			if (iRet < 0)
				return iRet;

			uint64_t left_bits = 0;
			bs.Tell(&left_bits);

			if (left_bits < 24ULL)
			{
				iRet = RET_CODE_BUFFER_TOO_SMALL;
				goto done;
			}

			Context_id = (uint16_t)bs.GetBits(12);
			Sequence_number = (uint16_t)bs.GetBits(4);

			Header_type = bs.GetByte();
			left_bits -= 24;

			switch (Header_type)
			{
			// Partial IPv4 header and partial UDP header
			// The partial IPv4 header shall be the header excluding packet length, header
			// checksum and extension information from the IPv4 header
			case 0x20:
				break;
			// IPv4 header identifier
			case 0x21:
				if (left_bits < 16)
					goto done;

				IPv4_header_Identifier = bs.GetWord();
				break;
			// Partial IPv6 header and partial UDP header
			// The partial IPv6 header shall be the header excluding payload length from IPv6 header
			// The partial UDP header shall be the header excluding data length and checksum from UDP header
			case 0x60:
				IPv6_header = new IP::V6::Header(true);
				if ((iRet = IPv6_header->Unpack(bs)) < 0)
					goto done;

				if (IPv6_header->payload_type == IP::V6::PROTOCOL_UDP)
				{
					UDP_header = new IP::UDPHeader(true);
					if ((iRet = UDP_header->Unpack(bs)) < 0)
						goto done;

					uint64_t cur_bit_pos = bs.Tell();
					
					MMTP_Packet = new MMTPPacket((int)Data_length + 4 - (int)((cur_bit_pos - start_bitpos)>>3));
					if ((iRet = MMTP_Packet->Unpack(bs)) < 0)
						goto done;
				}
				else
				{
					iRet = RET_CODE_ERROR_NOTIMPL;
					goto done;
				}

				break;
			// No compressed header
			case 0x61:
				{
					uint64_t cur_bit_pos = bs.Tell();
					MMTP_Packet = new MMTPPacket((int)Data_length + 4 - (int)((cur_bit_pos - start_bitpos) >> 3));
					if ((iRet = MMTP_Packet->Unpack(bs)) < 0)
						goto done;
				}
				break;
			default:
				break;
			}

		done:
			SkipLeftBits(bs);
			return iRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			TLVPacket::Print(fp, indent);

			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "Context_id", Context_id);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %u\n", szIndent, "Sequence_number", Sequence_number);
			fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d (0X%X), %s\n", szIndent, "Header_type", Header_type, Header_type, 
				Header_type == 0x20?"Partial IPv4 header and partial UDP header":(
				Header_type == 0x21?"IPv4 header identifier":(
				Header_type == 0x60?"Partial IPv6 header and partial UDP header":(
				Header_type == 0x61?"No compressed header":"Undefined"))));

			if (Header_type == 0x20)
			{

			}
			else if (Header_type == 0x21)
			{

			}
			else if (Header_type == 0x60)
			{
				if (IPv6_header != nullptr)
					IPv6_header->Print(fp, indent);

				if (UDP_header != nullptr)
					UDP_header->Print(fp, indent);
			}
			else if (Header_type == 0x61)
			{

			}

			if (MMTP_Packet != nullptr)
				MMTP_Packet->Print(fp, indent + 4);

		}

	}PACKED;

}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

