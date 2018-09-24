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

	// MPEG Media Transport Protocol packet
	struct MMTPPacket
	{
		struct MPU
		{
			uint16_t			Payload_length;
			
			uint16_t			Fragment_type : 4;
			uint16_t			Time_data_flag : 1;
			uint16_t			Division_index : 2;
			uint16_t			Aggregate_flag : 1;
			uint16_t			Division_number_counter : 8;

			uint32_t			MPU_sequence_number;

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
			}

		}PACKED;

		struct ControlMessages
		{
			uint8_t				Division_index : 2;
			uint8_t				Reserved_0 : 4;
			uint8_t				Length_information_extension_flag : 1;
			uint8_t				Aggregate_flag : 1;

			uint8_t				Division_number_counter;

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t left_bits = 0;
				bs.Tell(&left_bits);
				if (left_bits < 16)
					return RET_CODE_BOX_TOO_SMALL;

				Division_index = (uint8_t)bs.GetBits(2);
				Reserved_0 = (uint8_t)bs.GetBits(4);
				Length_information_extension_flag = (uint8_t)bs.GetBits(1);
				Aggregate_flag = (uint8_t)bs.GetBits(1);

				Division_number_counter = bs.GetByte();

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
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d, %s\n", szIndent, "Division_index", Division_index,
					Division_index == 0?"Undivided":(
					Division_index == 1?"Divided, Including the head part of the data before division":(
					Division_index == 2?"Divided, Not including the head part and end part of the data before division":(
					Division_index == 3?"Divided, Including the end part of the data before division":"Reserved"))));
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Length_information_extension_flag", Length_information_extension_flag);
				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Aggregate_flag", Aggregate_flag);

				fprintf(out, MMT_FIX_HEADER_FMT_STR ": %d\n", szIndent, "Division_number_counter", Division_number_counter);
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

		MMTPPacket() {
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

			if (Payload_type == 0)
			{
				ptr_MPU = new MPU();
				nRet = ptr_MPU->Unpack(bs);
			}
			else if (Payload_type == 2)
			{
				ptr_Messages = new ControlMessages();
				nRet = ptr_Messages->Unpack(bs);
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
					fprintf(out, "\n" MMT_FIX_HEADER_FMT_STR "  ...\n", szIndent, "");
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

					MMTP_Packet = new MMTPPacket();
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
				MMTP_Packet = new MMTPPacket();
				if ((iRet = MMTP_Packet->Unpack(bs)) < 0)
					goto done;
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

	// Unidirectional Lightweight Encapsulation (ULE) for Transmission of IP Datagrams over an MPEG-2 Transport Stream(TS),
	struct ULEPacket
	{
		uint64_t			start_bitpos;
		uint16_t			Destination_flag : 1;
		uint16_t			Data_length : 15;
		uint64_t			Packet_type:16;
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
	};

}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

