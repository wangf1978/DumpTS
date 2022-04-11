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
#ifndef _MPEG2_TRANSPORT_STREAM_H_
#define _MPEG2_TRANSPORT_STREAM_H_

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "DumpUtil.h"
#include "descriptors_13818_1.h"
#include "crc.h"
#include "systemdef.h"
#include "mpeg2_video.h"
#include "h265_video.h"
#include "h264_video.h"
#include "PayloadBuf.h"

#ifdef _ENABLE_FULL_FEATURES_
#include "dtshd.h"
#include "hdmv_graphic.h"
#include "dolby_audio.h"
#endif

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

// reserved PID list defined in ISO-13818-1
#define PID_PROGRAM_ASSOCIATION_TABLE					0x0000
#define PID_CONDITIONAL_ACCESS_TABLE					0x0001
#define PID_TRANSPORT_STREAM_DESCRIPTION_TABLE			0x0002
#define PID_IPMP_CONTROL_INFORMATION_TABLE				0x0003
#define PID_NULL_PACKET									0x1FFF

#define MPEG_PROGRAM_END_CODE							0xB9
#define PACK_START_CODE									0xBA
#define SYSTEM_HEADER_START_CODE						0xBB

#define IS_PES_PAYLOAD(p)								((p)[0] == 0 && (p)[1] == 0 && (p)[2] == 1 && (p)[3] >= BST::PROGRAM_STREAM_MAP)
#define IS_VIDEO_STREAM_ID(stm_id)						((stm_id&0xF0) == 0xE0)
#define IS_AUDIO_STREAM_ID(stm_id)						((stm_id&0xE0) == 0xC0)

#define AUDIO_STREAM_ID									0xE0
#define FILTER_PID										0x1400//0x1011//0x1011//0x1400
#define TS_PACKET_SIZE									192

#define SID_PROGRAM_STREAM_MAP							0xBC
#define SID_PRIVATE_STREAM_1							0xBD
#define SID_PADDING_STREAM								0xBE
#define SID_PRIVATE_STREAM_2							0xBF
#define SID_ECM											0xF0
#define SID_EMM											0xF1
#define SID_PROGRAM_STREAM_DIRECTORY					0xFF
#define SID_DSMCC_STREAM								0xF2
#define SID_H222_1_TYPE_E								0xF8

#define PID_PROGRAM_ASSOCIATION_TABLE					0x0000
#define PID_CONDITIONAL_ACCESS_TABLE					0x0001
#define PID_TRANSPORT_STREAM_DESCRIPTION_TABLE			0x0002
#define PID_IPMP_CONTROL_INFORMATION_TABLE				0x0003
#define PID_SELECTION_INFORMATION_TABLE					0x001F

#define MPEG2_VIDEO_STREAM								0x02
#define MPEG4_AVC_VIDEO_STREAM							0x1B
#define SMPTE_VC1_VIDEO_STREAM							0xEA
#define MPEG4_MVC_VIDEO_STREAM							0x20
#define HEVC_VIDEO_STREAM								0x24
#define VVC_VIDEO_STREAM								0x33
#define EVC_VIDEO_STREAM								0x35

#define MPEG1_AUDIO_STREAM								0x03
#define MPEG2_AUDIO_STREAM								0x04
#define AAC_AUDIO_STREAM								0x0F	// ISO/IEC 13818-7 Audio with ADTS transport syntax
#define MPEG4_AAC_AUDIO_STREAM							0x11	// ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3

#define HDMV_LPCM_AUDIO_STREAM							0x80
#define DOLBY_AC3_AUDIO_STREAM							0x81
#define DTS_AUDIO_STREAM								0x82
#define DOLBY_LOSSLESS_AUDIO_STREAM						0x83
#define DD_PLUS_AUDIO_STREAM							0x84

#define DTS_HD_EXCEPT_XLL_AUDIO_STREAM					0x85
#define DTS_HD_XLL_AUDIO_STREAM							0x86
#define DRA_AUDIO_STREAM								0x87
#define DRA_EXTENSION_AUDIO_STREAM						0x88
#define DD_PLUS_SECONDARY_AUDIO_STREAM					0xA1
#define DTS_HD_SECONDARY_AUDIO_STREAM					0xA2

#define SESF_TELETEXT_STREAM							0x06
#define TTML_STREAM										0x06

// extend
#define PRESENTATION_GRAPHICS							0x90
#define INTERACTIVE_GRAPHICS							0x91
#define SUBTITLE										0x92
#define MPEG1_VIDEO_STREAM								0x01

/*
	0x0A						Multi-protocol Encapsulation
	0x0B						DSM-CC U-N Messages
	0x0C						DSM-CC Stream Descriptors
	0x0D						DSM-CC Sections (any type, including private data)
*/
#define DSMCC_TYPE_A									0x0A
#define DSMCC_TYPE_B									0x0B
#define DSMCC_TYPE_C									0x0C
#define DSMCC_TYPE_D									0x0D

#define IS_VIDEO_STREAM_TYPE(coding_type)				\
	((coding_type) == MPEG1_VIDEO_STREAM ||				\
	 (coding_type) == MPEG2_VIDEO_STREAM ||				\
	 (coding_type) == MPEG4_AVC_VIDEO_STREAM ||			\
	 (coding_type) == SMPTE_VC1_VIDEO_STREAM ||			\
	 (coding_type) == MPEG4_MVC_VIDEO_STREAM ||			\
	 (coding_type) == HEVC_VIDEO_STREAM ||				\
	 (coding_type) == VVC_VIDEO_STREAM ||				\
	 (coding_type) == EVC_VIDEO_STREAM)

#define  IS_AUDIO_STREAM_TYPE(coding_type)				\
	((coding_type) == MPEG1_AUDIO_STREAM ||				\
	 (coding_type) == MPEG2_AUDIO_STREAM ||				\
	 (coding_type) == AAC_AUDIO_STREAM ||				\
	 (coding_type) == MPEG4_AAC_AUDIO_STREAM ||			\
	 (coding_type) == HDMV_LPCM_AUDIO_STREAM ||			\
	 (coding_type) == DOLBY_AC3_AUDIO_STREAM ||			\
	 (coding_type) == DTS_AUDIO_STREAM ||				\
	 (coding_type) == DOLBY_LOSSLESS_AUDIO_STREAM ||	\
	 (coding_type) == DD_PLUS_AUDIO_STREAM ||			\
	 (coding_type) == DTS_HD_EXCEPT_XLL_AUDIO_STREAM ||	\
	 (coding_type) == DTS_HD_XLL_AUDIO_STREAM ||		\
	 (coding_type) == DRA_AUDIO_STREAM ||				\
	 (coding_type) == DRA_EXTENSION_AUDIO_STREAM ||		\
	 (coding_type) == DD_PLUS_SECONDARY_AUDIO_STREAM ||	\
	 (coding_type) == DTS_HD_SECONDARY_AUDIO_STREAM)

#define IS_NAL_STREAM_TYPE(stm_type)	(\
	(stm_type) == MPEG4_AVC_VIDEO_STREAM ||\
	(stm_type) == MPEG4_MVC_VIDEO_STREAM ||\
	(stm_type) == HEVC_VIDEO_STREAM ||\
	(stm_type) == VVC_VIDEO_STREAM ||\
	(stm_type) == EVC_VIDEO_STREAM)

#define STREAM_TYPE_NAMEA(st)	(\
	(st) == MPEG1_VIDEO_STREAM?"MPEG1 Video":(\
	(st) == MPEG2_VIDEO_STREAM?"MPEG2 Video":(\
	(st) == MPEG4_AVC_VIDEO_STREAM?"MPEG4 AVC Video":(\
	(st) == SMPTE_VC1_VIDEO_STREAM?"VC1 Video":(\
	(st) == MPEG4_MVC_VIDEO_STREAM?"MVC Video":(\
	(st) == HEVC_VIDEO_STREAM?"HEVC Video":(\
	(st) == MPEG1_AUDIO_STREAM?"MPEG1 Audio":(\
	(st) == MPEG2_AUDIO_STREAM?"MPEG2 Audio":(\
	(st) == AAC_AUDIO_STREAM?"AAC Audio":(\
	(st) == MPEG4_AAC_AUDIO_STREAM?"MPEG4 AAC Audio":(\
	(st) == HDMV_LPCM_AUDIO_STREAM?"HDMV LPCM Audio":(\
	(st) == DOLBY_AC3_AUDIO_STREAM?"AC3 Audio":(\
	(st) == DTS_AUDIO_STREAM?"DTS Audio":(\
	(st) == DOLBY_LOSSLESS_AUDIO_STREAM?"Dolby Lossless Audio (TrueHD/Atmos)":(\
	(st) == DD_PLUS_AUDIO_STREAM?"DD+ Audio":(\
	(st) == DTS_HD_EXCEPT_XLL_AUDIO_STREAM?"DTS-HD audio":(\
	(st) == DTS_HD_XLL_AUDIO_STREAM?"DTS-HD Lossless Audio":(\
	(st) == DRA_AUDIO_STREAM?"DRA Audio":(\
	(st) == DRA_EXTENSION_AUDIO_STREAM?"DRA Extension Audio":(\
	(st) == DD_PLUS_SECONDARY_AUDIO_STREAM?"DD+ Secondary Audio":(\
	(st) == DTS_HD_SECONDARY_AUDIO_STREAM?"DTS LBR Audio":(\
	(st) == SESF_TELETEXT_STREAM?"Teletext, ARIB subtitle or TTML":(\
	(st) == PRESENTATION_GRAPHICS?"PGS":(\
	(st) == INTERACTIVE_GRAPHICS?"IGS":(\
	(st) == SUBTITLE?"SUB":(\
	(st) == DSMCC_TYPE_A?"DSM-CC Multi-protocol Encapsulation":(\
	(st) == DSMCC_TYPE_B?"DSM-CC DSM-CC U-N Messages":(\
	(st) == DSMCC_TYPE_C?"DSM-CC DSM-CC Stream Descriptors":(\
	(st) == DSMCC_TYPE_D?"DSM-CC SM-CC Sections":"Unknown")))))))))))))))))))))))))))))

extern const char* stream_type_names[256];
extern const char* table_id_names[256];

#define PES_STREAM_CODING(i)	(\
		(i) == 0xBC?"program_stream_map":(\
		(i) == 0xBD?"private_stream_1":(\
		(i) == 0xBE?"padding_stream":(\
		(i) == 0xBF?"private_stream_2":(\
		(i) >= 0xC0 && (i) <= 0xDF?"ISO/IEC 13818-3 or ISO/IEC 11172-3 or ISO/IEC 13818-7 or ISO/IEC 14496-3 audio stream":(\
		(i) >= 0xE0 && (i) <= 0xEF?"Rec. ITU-T H.262 | ISO/IEC 13818-2, ISO/IEC 11172-2, ISO/IEC 14496-2 or Rec. ITU-T H.264 | ISO/IEC 14496-10 video stream":(\
		(i) == 0xF0?"ECM_stream":(\
		(i) == 0xF1?"EMM_stream":(\
		(i) == 0xF2?"Rec. ITU-T H.222.0 | ISO/IEC 13818-1 Annex A or ISO/IEC 13818-6_DSMCC_stream":(\
		(i) == 0xF3?"ISO/IEC_13522_stream":(\
		(i) == 0xF4?"Rec. ITU-T H.222.1 type A":(\
		(i) == 0xF5?"Rec. ITU-T H.222.1 type B":(\
		(i) == 0xF6?"Rec. ITU-T H.222.1 type C":(\
		(i) == 0xF7?"Rec. ITU-T H.222.1 type D":(\
		(i) == 0xF8?"Rec. ITU-T H.222.1 type E":(\
		(i) == 0xF9?"ancillary_stream":(\
		(i) == 0xFA?"ISO/IEC 14496-1_SL-packetized_stream":(\
		(i) == 0xFB?"ISO/IEC 14496-1_FlexMux_stream":(\
		(i) == 0xFC?"metadata stream":(\
		(i) == 0xFD?"extended_stream_id":(\
		(i) == 0xFE?"reserved data stream":(\
		(i) == 0xFF?"program_stream_directory":""))))))))))))))))))))))

namespace BST {

	struct CPSISection;

	enum MPEG2_SYSTEM_STREAM_ID{
		PROGRAM_STREAM_MAP							= 0xBC,
		PRIVATE_STREAM_1							= 0xBD,
		PADDING_STREAM								= 0xBE,
		PRIVATE_STREAM_2							= 0xBF,
		AUDIO_STREAM_FIRST							= 0xC0,
		AUDIO_STREAM_LAST							= 0xDF,
		VIDEO_STREAM_FIRST							= 0xE0,
		VIDEO_STREAM_LAST							= 0xEF,
		ECM_STREAM									= 0xF0,
		EMM_STREAM									= 0xF1,
		DSMCC_STREAM								= 0xF2,
		ISO_IEC_13522_STREAM						= 0xF3,	// MHEG-5
		H_222_1_TYPE_A								= 0xF4,
		H_222_1_TYPE_B								= 0xF5,
		H_222_1_TYPE_C								= 0xF6,
		H_222_1_TYPE_D								= 0xF7,
		H_222_1_TYPE_E								= 0xF8,
		ANCILLARY_STREAM							= 0xF9,
		ISO_IEC_14496_1_SL_PACKETIZED_STREAM		= 0xFA,
		ISO_IEC_14496_1_FLEXMUX_STREAM				= 0xFB,
		METADATA_STREAM								= 0xFC,
		EXTENDED_STREAM_ID							= 0xFD,
		RESERVED_DATA_STREAM						= 0xFE,
		PROGRAM_STREAM_DIRECTORY					= 0xFF,
	};

	enum PSI_TABLE_ID{
		TID_program_association_section	= 0x00,
		TID_conditional_access_section,
		TID_TS_program_map_section,
		TID_TS_description_section,
		TID_ISO_IEC_14496_scene_description_section,
		TID_ISO_IEC_14496_object_descriptor_section,
		TID_Metadata_section,
		TID_IPMP_Control_Information_section,
		TID_Forbidden = 0xFF
	};

	struct CTPExtraHeader: public DIRECT_ENDIAN_MAP
	{
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		Copy_permission_indicator:2;
				unsigned long		Arrival_time_stamp:30;
#else
				unsigned long		Arrival_time_stamp:30;
				unsigned long		Copy_permission_indicator:2;
#endif
			};
			unsigned long		long_value;
		};

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(long_value);
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER("Copy_permission_indicator", 2, Copy_permission_indicator, Copy_permission_indicator==0?"Plain-Text":"Encrypted")
			NAV_FIELD_PROP_2NUMBER("Arrival_time_stamp", 30, Arrival_time_stamp, "Arrival time stamp")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CTTSExtraHeader: public DIRECT_ENDIAN_MAP
	{
		unsigned long	time_stamp;

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(time_stamp);
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER("timestamp", 32, time_stamp, "additional time stamp for network streaming usage")
		DECLARE_FIELDPROP_END()
	};

	struct CTSPacketHeader: public DIRECT_ENDIAN_MAP
	{
		unsigned char	sync_byte;

		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned short	transport_error_indicator:1;
				unsigned short	payload_unit_start_indicator:1;
				unsigned short	transport_priority:1;
				unsigned short	PID:13;
#else
				unsigned short	PID:13;
				unsigned short	transport_priority:1;
				unsigned short	payload_unit_start_indicator:1;
				unsigned short	transport_error_indicator:1;
#endif
			};
			unsigned short	short_value_0;
		};

#ifdef _BIG_ENDIAN_
		unsigned char	transport_scrambling_control:2;
		unsigned char	adaptation_field_control:2;
		unsigned char	continuity_counter:4;
#else
		unsigned char	continuity_counter:4;
		unsigned char	adaptation_field_control:2;
		unsigned char	transport_scrambling_control:2;
#endif

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0);
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER("sync_byte", 8, sync_byte, "Should be 0x47 for plain-text")
			NAV_FIELD_PROP_2NUMBER("transport_error_indicator", 1, transport_error_indicator, transport_error_indicator?"At least 1 uncorrectable bit error exists in the associated transport stream packet":"")
			NAV_FIELD_PROP_2NUMBER("payload_unit_start_indicator", 1, payload_unit_start_indicator, payload_unit_start_indicator?"Transport stream packets that carry PES packets or transport stream section data":"")
			NAV_FIELD_PROP_2NUMBER("transport_priority", 1, transport_priority, transport_priority?"The associated packet is of greater priority than other packets having the same PID which do not have the bit set to '1'":"")
			NAV_FIELD_PROP_2NUMBER("PID", 13, PID, PID==0x0000?"Program association table":(
												   PID==0x0001?"Conditional access table":(
												   PID==0x0002?"Transport stream description table":(
												   PID==0x0003?"IPMP control information table":(
												   PID==0x1FFF?"Null packet":"")))))
			NAV_FIELD_PROP_2NUMBER("transport_scrambling_control", 2, transport_scrambling_control, transport_scrambling_control==0x00?"Not scrambled":"User-defined")
			NAV_FIELD_PROP_2NUMBER("adaptation_field_control", 2, adaptation_field_control, adaptation_field_control==0x00?"Reserved for future use by ISO/IEC":(
																							adaptation_field_control==0x01?"No adaptation_field, payload only":(
																							adaptation_field_control==0x02?"Adaptation_field only, no payload":(
																							adaptation_field_control==0x03?"Adaptation_field followed by payload":""))))
			NAV_FIELD_PROP_2NUMBER("continuity_counter", 4, continuity_counter, "wraps around to 0 after its maximum value, and shall not be incremented when the adaptation_field_control of the packet equals '00' or '10'")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CSCR: public DIRECT_ENDIAN_MAP{
#ifdef _BIG_ENDIAN_
		unsigned char	reserved:2;
		unsigned char	SCR_base_0:3;
		unsigned char	marker_bit_0:1;
		unsigned char	SCR_base_1:2;
				
		unsigned char	SCR_base_2;

		unsigned char	SCR_base_3:5;
		unsigned char	marker_bit_1:1;
		unsigned char	SCR_base_4:2;

		unsigned char	SCR_base_5;

		unsigned char	SCR_base_6:5;
		unsigned char	marker_bit_2:1;
		unsigned char	SCR_extension_0:2;

		unsigned char	SCR_extension_1:7;
		unsigned char	marker_bit_3:1;
#else
		unsigned char	SCR_base_1:2;
		unsigned char	marker_bit_0:1;
		unsigned char	SCR_base_0:3;
		unsigned char	reserved:2;
				
		unsigned char	SCR_base_2;

		unsigned char	SCR_base_4:2;
		unsigned char	marker_bit_1:1;
		unsigned char	SCR_base_3:5;

		unsigned char	SCR_base_5;

		unsigned char	SCR_extension_0:2;
		unsigned char	marker_bit_2:1;
		unsigned char	SCR_base_6:5;

		unsigned char	marker_bit_3:1;			
		unsigned char	SCR_extension_1:7;
#endif
		long long GetSCRbase(){
			long long SCR_base = SCR_base_0;
			SCR_base = (SCR_base<<2) | SCR_base_1;
			SCR_base = (SCR_base<<8) | SCR_base_2;
			SCR_base = (SCR_base<<5) | SCR_base_3;
			SCR_base = (SCR_base<<2) | SCR_base_4;
			SCR_base = (SCR_base<<8) | SCR_base_5;
			SCR_base = (SCR_base<<5) | SCR_base_6;
			return SCR_base;
		}

		short GetSCRextension(){
			return SCR_extension_0<<7|SCR_extension_1;;
		}

		long long GetSCR(){
			return GetSCRbase()*300LL + GetSCRextension();
		}

		DECLARE_FIELDPROP_BEGIN()
			MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "%lld(0X%llX) (27MHZ)", GetSCR(), GetSCR());
			NAV_WRITE_TAG_BEGIN_1("SCR", szTemp4, 0);
				NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved, "should be 01b")
				NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("SCR_base[32..30]", 3, SCR_base_0, "")
				NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_0, "")
				NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("SCR_base[29..15]", 15, (SCR_base_1<<13)|(SCR_base_2<<5)|SCR_base_3, "")
				NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_1, "")
				NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("SCR_base[14..0]", 15, (SCR_base_4<<13)|(SCR_base_5<<5)|SCR_base_6, "")
				NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_2, "")
				NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("SCR_extension", 9, (SCR_extension_0<<7)|SCR_extension_1, "")
				NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_3, "")
			NAV_WRITE_TAG_END("SCR");
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CPSPackHeader: public ADVANCE_ENDIAN_MAP{

		struct CSystemHeader: public ADVANCE_ENDIAN_MAP{

			struct CStreamPSTDBuffer: public DIRECT_ENDIAN_MAP {
				unsigned char		stream_id;
				union{
					struct{
#ifdef _BIG_ENDIAN_
						unsigned short		marker_bit_11:2;
						unsigned short		P_STD_buffer_bound_scale:1;
						unsigned short		P_STD_buffer_size_bound:13;
#else
						unsigned short		P_STD_buffer_size_bound:13;
						unsigned short		P_STD_buffer_bound_scale:1;
						unsigned short		marker_bit_11:2;
#endif
					}PACKED;

					struct {
#ifdef _BIG_ENDIAN_
						unsigned short		fixed_bits_11_0 : 2;
						unsigned short		fixed_bits_000_0000 : 7;
						unsigned short		stream_id_extension : 7;
#else
						unsigned short		stream_id_extension : 7;
						unsigned short		fixed_bits_000_0000 : 7;
						unsigned short		fixed_bits_11_0 : 2;
#endif
					} PACKED;
					unsigned short	short_value;
				}PACKED;

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER1(stream_id, 8, 
						stream_id==0xB8?"all Audio streams":(
						stream_id==0xB9?"all Video streams":(
						stream_id==0xFD?"the P-STD_buffer_bound_scale and P-STD_buffer_size_bound fields following the stream_id refer to all elementary streams with an extended_stream_id in the program stream, independent of the coded value of the stream_id_extension in the PES header of those streams":(
						stream_id==0xB7?"the following stream_id_extension field shall be interpreted as referring to the stream coding and elementary stream number according to Table 2-27":(
						stream_id==0xBD?"private_stream_1":(
						stream_id==0xBF?"private_stream_2":""))))));
					if (stream_id == 0xB7)
					{
						NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("fixed_bits", 2, fixed_bits_11_0, "Should be 11b");
						NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("fixed_bits", 7, fixed_bits_000_0000, "Should be 000 0000b");
						NAV_FIELD_PROP_2NUMBER1(stream_id_extension, 7, "indicates the coding and elementary stream number of the stream with an extended_stream_id to which the P-STD_buffer_bound_scale and P-STD_buffer_size_bound fields following the stream_id_extension field refer.");
					}
					else
					{
						NAV_FIELD_PROP_2NUMBER_WITH_ALIAS("marker_bits", 2, marker_bit_11, "Should be 11b");
						NAV_FIELD_PROP_NUMBER1(P_STD_buffer_bound_scale, 1, "indicates the scaling factor used to interpret the subsequent P-STD_buffer_size_bound field.");
						MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "%d bytes, %s",
							P_STD_buffer_size_bound*(P_STD_buffer_bound_scale ? 1024 : 128),
							P_STD_buffer_bound_scale ? "the buffer size bound in units of 1024 bytes" : "the buffer size bound in units of 128 bytes");
						NAV_FIELD_PROP_2NUMBER1(P_STD_buffer_size_bound, 13, szTemp4);
					}
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct CFixedHeader: public DIRECT_ENDIAN_MAP{
				unsigned char		system_header_start_code[4];

				union{
					struct {
#ifdef _BIG_ENDIAN_
						unsigned long long	header_length:16;
						unsigned long long	marker_bit_0:1;
						unsigned long long	rate_round:22;
						unsigned long long	marker_bit_1:1;
						unsigned long long	audio_bound:6;
						unsigned long long	fixed_flag:1;
						unsigned long long	CSPS_flag:1;
						unsigned long long	system_audio_clock_flag:1;
						unsigned long long	system_video_clock_flag:1;
						unsigned long long	marker_bit_2:1;
						unsigned long long	video_bound:5;
						unsigned long long	packet_rate_restriction_flag:1;
						unsigned long long	reserved_bits:7;
#else
						unsigned long long	reserved_bits:7;
						unsigned long long	packet_rate_restriction_flag:1;
						unsigned long long	video_bound:5;
						unsigned long long	marker_bit_2:1;
						unsigned long long	system_video_clock_flag:1;
						unsigned long long	system_audio_clock_flag:1;
						unsigned long long	CSPS_flag:1;
						unsigned long long	fixed_flag:1;
						unsigned long long	audio_bound:6;
						unsigned long long	marker_bit_1:1;
						unsigned long long	rate_round:22;
						unsigned long long	marker_bit_0:1;
						unsigned long long	header_length:16;
#endif
					}PACKED;

					unsigned long long	longlong_value;
				}PACKED;

				DECLARE_ENDIAN_BEGIN()
					UINT64_FIELD_ENDIAN(longlong_value)
				DECLARE_ENDIAN_END()

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_FIXSIZE_BINSTR("system_header_start_code", 32, system_header_start_code, 4UL, "should be 00 00 01 bb");
					NAV_FIELD_PROP_2NUMBER1(header_length, 16, "the length in bytes of the system header following the header_length field");
					NAV_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "");
					NAV_FIELD_PROP_2NUMBER1(rate_round, 22, "an integer value greater than or equal to the maximum value of the program_mux_rate field coded in any pack of the program stream");
					NAV_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "");
					NAV_FIELD_PROP_2NUMBER1(audio_bound, 6, "Number of Audio Streams");
					NAV_FIELD_PROP_NUMBER1(fixed_flag, 1, fixed_flag?"fixed bitrate":"variable bitrate");
					NAV_FIELD_PROP_NUMBER1(CSPS_flag, 1, CSPS_flag?"the program stream meets the constraints defined in ISO-13818-1 2.7.9":"");
					NAV_FIELD_PROP_NUMBER1(system_audio_clock_flag, 1, "");
					NAV_FIELD_PROP_NUMBER1(system_video_clock_flag, 1, "");
					NAV_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "");
					NAV_FIELD_PROP_2NUMBER1(video_bound, 5, "Number of Video streams");
					NAV_FIELD_PROP_NUMBER1(packet_rate_restriction_flag, 1, packet_rate_restriction_flag?"":"");
					NAV_FIELD_PROP_2NUMBER("reserved_bits", 7, reserved_bits, "Should be 111 1111b")
				DECLARE_FIELDPROP_END()

			}PACKED;

			CFixedHeader*		FixedHdr;
			std::vector<CStreamPSTDBuffer*>	
								Stream_P_STD_Buffers;

			CSystemHeader() : FixedHdr(NULL) {
			}

			virtual ~CSystemHeader() {
				Unmap();
			}

			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size = 0, unsigned long *stuffing_size = 0)
			{
				unsigned long ulMappedSize = 0;

				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;

				MAP_MEM_TO_STRUCT_POINTER(1, FixedHdr, CFixedHeader);
				while (ulMappedSize < cbSize && (pBuf[ulMappedSize] & 0x80))
				{
					CStreamPSTDBuffer* PSTD = NULL;
					MAP_MEM_TO_STRUCT_POINTER(1, PSTD, CStreamPSTDBuffer);
					Stream_P_STD_Buffers.push_back(PSTD);
				}

				AMP_SAFEASSIGN(desired_size, ulMappedSize);

				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf = NULL, /* In/Out */unsigned long* pcbSize = NULL)
			{
				UNMAP_STRUCT_POINTER(FixedHdr);
				for (size_t i = 0; i < Stream_P_STD_Buffers.size(); i++)
				{
					UNMAP_STRUCT_POINTER(Stream_P_STD_Buffers[i]);
				}
				Stream_P_STD_Buffers.clear();
				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_REF(FixedHdr);
				for (size_t idx = 0; idx < Stream_P_STD_Buffers.size(); idx++) {
					NAV_FIELD_PROP_REF(Stream_P_STD_Buffers[idx]);
				}
			DECLARE_FIELDPROP_END()

		};

		struct CFixedHeader: public DIRECT_ENDIAN_MAP{
			unsigned char		pack_start_code[4];
			CSCR				system_clock_reference;

			union {
				struct {
#ifdef _BIG_ENDIAN_
					unsigned long		program_mux_rate:22;
					unsigned long		marker_bit_0:1;
					unsigned long		marker_bit_1:1;
					unsigned long		reserved:5;
					unsigned long		pack_stuffing_length:3;
#else
					unsigned long		pack_stuffing_length:3;
					unsigned long		reserved:5;
					unsigned long		marker_bit_1:1;
					unsigned long		marker_bit_0:1;
					unsigned long		program_mux_rate:22;
#endif
				}PACKED;
				unsigned long	ulong_value;
			}PACKED;

			unsigned char		stuffing_byte[];

			DECLARE_ENDIAN_BEGIN()
				ULONG_FIELD_ENDIAN(ulong_value)
			DECLARE_ENDIAN_END()

			int GetVarBodySize() { return ENDIANULONG(ulong_value)&0x7; }

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_FIXSIZE_BINSTR("pack_start_code", 32, pack_start_code, 4, "");
				NAV_FIELD_PROP_OBJECT(system_clock_reference)
				NAV_FIELD_PROP_2NUMBER_DESC_F("program_mux_rate", 22, program_mux_rate, "%lld bps", program_mux_rate*50LL*8);
				NAV_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "Should be 1b");
				NAV_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "Should be 1b");
				NAV_FIELD_PROP_2NUMBER1(reserved, 5, "");
				NAV_FIELD_PROP_NUMBER1(pack_stuffing_length, 3, "");
				if (pack_stuffing_length > 0)
				{
					NAV_FIELD_PROP_FIXSIZE_BINSTR1(stuffing_byte, pack_stuffing_length * 8UL, "not greater than 7 bytes filled with fixed 8-bit value equal to '1111 1111'");
				}
			DECLARE_FIELDPROP_END()

		}PACKED;

		CFixedHeader*	FixedHdr;
		CSystemHeader*	SystemHdr;

		CPSPackHeader(): FixedHdr(NULL), SystemHdr(NULL){}

		~CPSPackHeader() {
			Unmap();
		}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size = 0, unsigned long *stuffing_size = 0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_STRUCT_POINTER(1, FixedHdr, CFixedHeader);
			if (ulMappedSize + 4 < cbSize && 
				pBuf[ulMappedSize] == 0 && 
				pBuf[ulMappedSize + 1] == 0 && 
				pBuf[ulMappedSize + 2] == 1 && 
				pBuf[ulMappedSize + 3] == SYSTEM_HEADER_START_CODE)
			{
				MAP_MEM_TO_STRUCT_POINTER2(1, SystemHdr, CSystemHeader);
			}

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf = NULL, /* In/Out */unsigned long* pcbSize = NULL)
		{
			UNMAP_STRUCT_POINTER(FixedHdr)
			UNMAP_STRUCT_POINTER2(SystemHdr)
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_REF_WITH_TAG2_2(FixedHdr, "pack_header", "", 1);
			NAV_FIELD_PROP_REF_WITH_TAG2_2(SystemHdr, "system_header", "", 1);
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CAdaptationField: public ADVANCE_ENDIAN_MAP{

		struct CPCR: public DIRECT_ENDIAN_MAP{
			unsigned long	program_clock_reference_base_0;
#ifdef _BIG_ENDIAN_
			unsigned char	program_clock_reference_base_1:1;
			unsigned char	reserved_0:6;
			unsigned char	program_clock_reference_extension_0:1;
#else
			unsigned char	program_clock_reference_extension_0:1;
			unsigned char	reserved_0:6;
			unsigned char	program_clock_reference_base_1:1;
#endif
			unsigned char	program_clock_reference_extension_1;

			DECLARE_ENDIAN_BEGIN()
				ULONG_FIELD_ENDIAN(program_clock_reference_base_0)
			DECLARE_ENDIAN_END()

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_NUMBER64("program_clock_reference_base", 33, ((long long)program_clock_reference_base_0<<1|program_clock_reference_base_1), "")
				NAV_FIELD_PROP_2NUMBER("reserved", 6, reserved_0, "")
				NAV_FIELD_PROP_2NUMBER("program_clock_reference_extension", 9, ((unsigned long)program_clock_reference_extension_0<<8|program_clock_reference_extension_1), "")
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CTransportPrivateData: public DIRECT_ENDIAN_MAP{
			unsigned char	transport_private_data_length;
			unsigned char	private_data_byte[];

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER("transport_private_data_length", 8, transport_private_data_length, "")
				NAV_FIELD_PROP_FIXSIZE_BINSTR("private_data_byte", ((long long)transport_private_data_length*8), private_data_byte, (unsigned long)transport_private_data_length, "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CAdaptationFieldExtension: public ADVANCE_ENDIAN_MAP{
			
			struct CLegalTimeWindow: public DIRECT_ENDIAN_MAP {
				union {
					struct {
#ifdef _BIG_ENDIAN_
						unsigned short	ltw_valid_flag:1;
						unsigned short	ltw_offset:15;
#else
						unsigned short	ltw_offset:15;
						unsigned short	ltw_valid_flag:1;
#endif
					}PACKED;
					unsigned short	ltw_short_value;
				}PACKED;

				DECLARE_ENDIAN_BEGIN()
					USHORT_FIELD_ENDIAN(ltw_short_value)
				DECLARE_ENDIAN_END()

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER("ltw_valid_flag", 1, ltw_valid_flag, "")
					NAV_FIELD_PROP_2NUMBER("ltw_offset", 15, ltw_offset, "")
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct CPiecewiseRate: public DIRECT_ENDIAN_MAP {
#ifdef _BIG_ENDIAN_
				unsigned char	reserved:2;
				unsigned char	piecewise_rate_0:6;
#else
				unsigned char	piecewise_rate_0:6;
				unsigned char	reserved:2;
#endif
				unsigned short	piecewise_rate_1;

				DECLARE_ENDIAN_BEGIN()
					USHORT_FIELD_ENDIAN(piecewise_rate_1)
				DECLARE_ENDIAN_END()

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved, "")
					NAV_FIELD_PROP_2NUMBER("piecewise_rate", 22, ((unsigned long)piecewise_rate_0<<16 | piecewise_rate_1), "A positive integer specifying a hypothetical bitrate R which is used to define the end times of the Legal Time Windows of transport stream packets of the same PID that follow this packet but do not include the legal_time_window_offset field.")
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct CSeamlessSplice: public DIRECT_ENDIAN_MAP{
#ifdef _BIG_ENDIAN_
				unsigned char	splice_type:4;
				unsigned char	DTS_next_AU_0:3;
				unsigned char	marker_bit_0:1;

				union {
					struct {
						unsigned short	DTS_next_AU_1:15;
						unsigned char	marker_bit_1:1;
					}PACKED;
					unsigned short	DTS_next_AU_1_short_value;
				}PACKED;

				union {
					struct {
						unsigned short	DTS_next_AU_2:15;
						unsigned char	marker_bit_2:1;
					}PACKED;
					unsigned short	DTS_next_AU_2_short_value;
				}PACKED;
#else
				unsigned char	marker_bit_0:1;
				unsigned char	DTS_next_AU_0:3;
				unsigned char	splice_type:4;

				union {
					struct {
						unsigned char	marker_bit_1:1;
						unsigned short	DTS_next_AU_1:15;
					}PACKED;
					unsigned short	DTS_next_AU_1_short_value;
				}PACKED;

				union {
					struct {
						unsigned char	marker_bit_2:1;
						unsigned short	DTS_next_AU_2:15;
					}PACKED;
					unsigned short	DTS_next_AU_2_short_value;
				}PACKED;
#endif

				DECLARE_ENDIAN_BEGIN()
					USHORT_FIELD_ENDIAN(DTS_next_AU_1_short_value)
					USHORT_FIELD_ENDIAN(DTS_next_AU_2_short_value)
				DECLARE_ENDIAN_END()

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER("splice_type", 4, splice_type, "")
					NAV_FIELD_PROP_2NUMBER("DTS_next_AU",  3, DTS_next_AU_0, "DTS_next_AU[32..30]")
					NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_0, "")

					NAV_FIELD_PROP_2NUMBER("DTS_next_AU", 15, DTS_next_AU_1, "DTS_next_AU[29..15]")
					NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_1, "")

					NAV_FIELD_PROP_2NUMBER("DTS_next_AU", 15, DTS_next_AU_2, "DTS_next_AU[14..0]")
					NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_2, "")
				DECLARE_FIELDPROP_END()
			}PACKED;

			unsigned char		adaptation_field_extension_length = 0;
#ifdef _BIG_ENDIAN_
			unsigned char		ltw_flag:1;
			unsigned char		piecewise_rate_flag:1;
			unsigned char		seamless_splice_flag:1;
			unsigned char		reserved:5;
#else
			unsigned char		reserved:5;
			unsigned char		seamless_splice_flag:1;
			unsigned char		piecewise_rate_flag:1;
			unsigned char		ltw_flag:1;
#endif
			CLegalTimeWindow*	ltw;
			CPiecewiseRate*		piecewise_rate;
			CSeamlessSplice*	seamless_splice;

			CAdaptationFieldExtension()
				: reserved(0), seamless_splice_flag(0), piecewise_rate_flag(0), ltw_flag(0), 
				ltw(NULL), piecewise_rate(NULL), seamless_splice(NULL){
			}

			virtual ~CAdaptationFieldExtension(){
				Unmap();
			}

			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
			{
				unsigned long ulMappedSize = 0;

				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;

				MAP_MEM_TO_HDR2(&adaptation_field_extension_length, 2)
				MAP_MEM_TO_STRUCT_POINTER(ltw_flag, ltw, CLegalTimeWindow)
				MAP_MEM_TO_STRUCT_POINTER(piecewise_rate_flag, piecewise_rate, CPiecewiseRate)
				MAP_MEM_TO_STRUCT_POINTER(seamless_splice_flag, seamless_splice, CSeamlessSplice)

				AMP_SAFEASSIGN(desired_size, ulMappedSize);

				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
			{
				UNMAP_STRUCT_POINTER(seamless_splice)
				UNMAP_STRUCT_POINTER(piecewise_rate)
				UNMAP_STRUCT_POINTER(ltw)
				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER("adaptation_field_extension_length", 8, adaptation_field_extension_length, "")
				NAV_FIELD_PROP_2NUMBER("ltw_flag", 1, ltw_flag, "")
				NAV_FIELD_PROP_2NUMBER("piecewise_rate_flag", 1, piecewise_rate_flag, "")
				NAV_FIELD_PROP_2NUMBER("seamless_splice_flag", 1, seamless_splice_flag, "")
				NAV_FIELD_PROP_2NUMBER("reserved", 5, reserved, "")
				if (ltw_flag){
					NAV_FIELD_PROP_REF(ltw)
				}
				if (seamless_splice_flag){
					NAV_FIELD_PROP_REF(piecewise_rate)
				}
				if (seamless_splice_flag){
					NAV_FIELD_PROP_REF(seamless_splice)
				}
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char	adaptation_field_length;

		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned char	discontinuity_indicator:1;
				unsigned char	random_access_indicator:1;
				unsigned char	elementary_stream_priority_indicator:1;
				unsigned char	PCR_flag:1;
				unsigned char	OPCR_flag:1;
				unsigned char	splicing_point_flag:1;
				unsigned char	transport_private_data_flag:1;
				unsigned char	adaptation_field_extension_flag:1;
#else
				unsigned char	adaptation_field_extension_flag:1;
				unsigned char	transport_private_data_flag:1;
				unsigned char	splicing_point_flag:1;
				unsigned char	OPCR_flag:1;
				unsigned char	PCR_flag:1;
				unsigned char	elementary_stream_priority_indicator:1;
				unsigned char	random_access_indicator:1;
				unsigned char	discontinuity_indicator:1;
#endif
			}PACKED;
			unsigned char	byte_value;
		}PACKED;
		CPCR*			program_clock_reference;
		CPCR*			original_program_clock_reference;
		unsigned char	splice_countdown = 0;
		CTransportPrivateData*
						transport_private_data;
		CAdaptationFieldExtension*
						adaptation_field_extension;

		CAdaptationField():program_clock_reference(NULL), original_program_clock_reference(NULL), transport_private_data(NULL), adaptation_field_extension(NULL){
		}

		virtual ~CAdaptationField(){
			Unmap();
		}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			adaptation_field_length = *(pBuf + ulMappedSize);
			ulMappedSize++;

			if (adaptation_field_length > 0){
				byte_value = *(pBuf + ulMappedSize);
				ulMappedSize++;
			}

			MAP_MEM_TO_STRUCT_POINTER(PCR_flag, program_clock_reference, CPCR)
			MAP_MEM_TO_STRUCT_POINTER(OPCR_flag, original_program_clock_reference, CPCR)

			if (splicing_point_flag){
				splice_countdown = *(pBuf + ulMappedSize);
				ulMappedSize++;
			}

			MAP_MEM_TO_STRUCT_POINTER(transport_private_data_flag, transport_private_data, CTransportPrivateData)
			MAP_MEM_TO_STRUCT_POINTER2(adaptation_field_extension_flag, adaptation_field_extension, CAdaptationFieldExtension)

			AMP_SAFEASSIGN(desired_size, adaptation_field_length + 1);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			UNMAP_STRUCT_POINTER(program_clock_reference)
			UNMAP_STRUCT_POINTER(original_program_clock_reference)
			UNMAP_STRUCT_POINTER(transport_private_data)
			UNMAP_STRUCT_POINTER2(adaptation_field_extension)
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER("adaptation_field_length", 8, adaptation_field_length, "")
			if (adaptation_field_length > 0)
			{
				NAV_FIELD_PROP_2NUMBER("discontinuity_indicator", 1, discontinuity_indicator, "")
				NAV_FIELD_PROP_2NUMBER("random_access_indicator", 1, random_access_indicator, "")
				NAV_FIELD_PROP_2NUMBER("elementary_stream_priority_indicator", 1, elementary_stream_priority_indicator, "")
				NAV_FIELD_PROP_2NUMBER("PCR_flag", 1, PCR_flag, "")
				NAV_FIELD_PROP_2NUMBER("OPCR_flag", 1, OPCR_flag, "")
				NAV_FIELD_PROP_2NUMBER("splicing_point_flag", 1, splicing_point_flag, "")
				NAV_FIELD_PROP_2NUMBER("transport_private_data_flag", 1, transport_private_data_flag, "")
				NAV_FIELD_PROP_2NUMBER("adaptation_field_extension_flag", 1, adaptation_field_extension_flag, "")

				NAV_FIELD_PROP_REF(program_clock_reference)

				if (original_program_clock_reference)
				{
					NAV_FIELD_PROP_NUMBER64("original_program_clock_reference_base", 33, ((long long)original_program_clock_reference->program_clock_reference_base_0<<1|original_program_clock_reference->program_clock_reference_base_1), "")
					NAV_FIELD_PROP_2NUMBER("reserved", 6, original_program_clock_reference->reserved_0, "")
					NAV_FIELD_PROP_2NUMBER("original_program_clock_reference_extension", 9, ((unsigned long)original_program_clock_reference->program_clock_reference_extension_0<<8|original_program_clock_reference->program_clock_reference_extension_1), "")
				}

				//NAV_FIELD_PROP_REF(original_program_clock_reference)
				if (splicing_point_flag){
					NAV_FIELD_PROP_2NUMBER("splice_countdown", 8, splice_countdown, "")
				}
				NAV_FIELD_PROP_REF(transport_private_data)
				NAV_FIELD_PROP_REF(adaptation_field_extension)
			}
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CPES: public ADVANCE_ENDIAN_MAP{
		
		struct CPESPacketHeaderAndData : public ADVANCE_ENDIAN_MAP {

			struct CTimeStamp : public DIRECT_ENDIAN_MAP {
#ifdef _BIG_ENDIAN_
				unsigned char	marker_bit_0011 : 4;
				unsigned char	TimeStamp_0 : 3;
				unsigned char	marker_bit_0 : 1;

				union {
					struct {
						unsigned short	TimeStamp_1 : 15;
						unsigned short	marker_bit_1 : 1;
					}PACKED;
					unsigned short	TimeStamp_1_short_value;
				}PACKED;

				union {
					struct {
						unsigned short	TimeStamp_2 : 15;
						unsigned short	marker_bit_2 : 1;
					}PACKED;
					unsigned short	TimeStamp_2_short_value;
				}PACKED;
#else
				unsigned char	marker_bit_0 : 1;
				unsigned char	TimeStamp_0 : 3;
				unsigned char	marker_bit_0011 : 4;

				union {
					struct {
						unsigned short	marker_bit_1 : 1;
						unsigned short	TimeStamp_1 : 15;
					}PACKED;
					unsigned short	TimeStamp_1_short_value;
				}PACKED;

				union {
					struct {
						unsigned short	marker_bit_2 : 1;
						unsigned short	TimeStamp_2 : 15;
					}PACKED;
					unsigned short	TimeStamp_2_short_value;
				}PACKED;
#endif
				DECLARE_ENDIAN_BEGIN()
				USHORT_FIELD_ENDIAN(TimeStamp_1_short_value)
					USHORT_FIELD_ENDIAN(TimeStamp_2_short_value)
					DECLARE_ENDIAN_END()

				long long GetTimeStamp() {
					long long llRet = 0;
					llRet = TimeStamp_0;
					llRet <<= 15;
					llRet |= TimeStamp_1;
					llRet <<= 15;
					llRet |= TimeStamp_2;
					return llRet;
				}
			}PACKED;

			struct CESrate : public DIRECT_ENDIAN_MAP {
#ifdef _BIG_ENDIAN_
				unsigned char	marker_bit_0 : 1;
				unsigned char	ES_rate_0 : 7;

				unsigned char	ES_rate_1;

				unsigned char	ES_rate_2 : 7;
				unsigned char	marker_bit_1 : 1;
#else
				unsigned char	ES_rate_0 : 7;
				unsigned char	marker_bit_0 : 1;

				unsigned char	ES_rate_1;

				unsigned char	marker_bit_1 : 1;
				unsigned char	ES_rate_2 : 7;
#endif

				int GetESrate() {
					return ES_rate_0 << 15 | ES_rate_1 << 7 | ES_rate_2;
				}

				DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_0, "")
					int es_rate = GetESrate();
				NAV_FIELD_PROP_2NUMBER("ES_rate", 22, es_rate, "")
					NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_1, "")
					DECLARE_FIELDPROP_END()
			}PACKED;

			struct CDSMtrickmode : public DIRECT_ENDIAN_MAP {

				enum Trick_mode_control_value {
					Fast_forward = 0,
					Slow_motion = 1,
					Freeze_frame = 2,
					Fast_reverse = 3,
					Slow_reverse = 4
				};

				union {
#ifdef _BIG_ENDIAN_
					struct {
						unsigned char	trick_mode_control : 3;
						unsigned char	field_id : 2;
						unsigned char	intra_slice_refresh : 1;
						unsigned char	frequency_truncation : 2;
					}PACKED;

					struct {
						unsigned char	trick_mode_control : 3;
						unsigned char	rep_cntrl : 5;
					}PACKED;

					struct {
						unsigned char	trick_mode_control : 3;
						unsigned char	field_id : 2;
						unsigned char	reserved_0 : 3;
					}PACKED;

					struct {
						unsigned char	trick_mode_control : 3;
						unsigned char	reserved_1 : 5;
					}PACKED;
#else
					struct {
						unsigned char	frequency_truncation : 2;
						unsigned char	intra_slice_refresh : 1;
						unsigned char	field_id : 2;
						unsigned char	trick_mode_control : 3;
					}PACKED;

					struct {
						unsigned char	rep_cntrl : 5;
						unsigned char	trick_mode_control_2 : 3;
					}PACKED;

					struct {
						unsigned char	reserved_0 : 3;
						unsigned char	field_id_2 : 2;
						unsigned char	trick_mode_control_3 : 3;
					}PACKED;

					struct {
						unsigned char	reserved_1 : 5;
						unsigned char	trick_mode_control_4 : 3;
					}PACKED;
#endif
				}PACKED;

				DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER("trick_mode_control", 3, trick_mode_control, "")
					switch (trick_mode_control) {
					case Fast_forward:
					case Fast_reverse:
					{
						NAV_FIELD_PROP_2NUMBER("field_id", 2, field_id, "")
						NAV_FIELD_PROP_2NUMBER("intra_slice_refresh", 1, intra_slice_refresh, "")
						NAV_FIELD_PROP_2NUMBER("frequency_truncation", 2, frequency_truncation, "")
					}
					break;
					case Slow_motion:
					case Slow_reverse:
					{
						NAV_FIELD_PROP_2NUMBER("rep_cntrl", 5, rep_cntrl, "")
					}
					break;
					case Freeze_frame:
					{
						NAV_FIELD_PROP_2NUMBER("field_id", 2, field_id, "")
						NAV_FIELD_PROP_2NUMBER("reserved_0", 3, reserved_0, "")
					}
					break;
					default:
					{
						NAV_FIELD_PROP_2NUMBER("reserved", 5, reserved_1, "")
					}
					}
				DECLARE_FIELDPROP_END()

			}PACKED;

			struct CAdditionalCopyInfo : public DIRECT_ENDIAN_MAP {
#ifdef _BIG_ENDIAN_
				unsigned char	marker_bit : 1;
				unsigned char	additional_copy_info : 7;
#else
				unsigned char	additional_copy_info : 7;
				unsigned char	marker_bit : 1;
#endif
				DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit, "")
					NAV_FIELD_PROP_2NUMBER("additional_copy_info", 7, additional_copy_info, "")
					DECLARE_FIELDPROP_END()
			}PACKED;

			struct CPESextension : public ADVANCE_ENDIAN_MAP {
				struct CPackHeaderField : public DIRECT_ENDIAN_MAP {
					unsigned char	pack_field_length;
					unsigned char	pack_header_data[];

					int GetVarBodySize() { return pack_field_length*sizeof(unsigned char); }

					DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER("pack_field_length", 8, pack_field_length, "")
						NAV_FIELD_PROP_FIXSIZE_BINSTR("pack_header", ((long long)pack_field_length * 8), pack_header_data, (unsigned long)pack_field_length, "")
						DECLARE_FIELDPROP_END()
				}PACKED;

				struct CProgramPacketSequenceCounter : public DIRECT_ENDIAN_MAP {
#ifdef _BIG_ENDIAN_
					unsigned char	marker_bit_0 : 1;
					unsigned char	program_packet_sequence_counter : 7;
					unsigned char	marker_bit_1 : 1;
					unsigned char	MPEG1_MPEG2_identifier : 1;
					unsigned char	original_stuff_length : 6;
#else
					unsigned char	original_stuff_length : 6;
					unsigned char	MPEG1_MPEG2_identifier : 1;
					unsigned char	marker_bit_1 : 1;
					unsigned char	program_packet_sequence_counter : 7;
					unsigned char	marker_bit_0 : 1;
#endif

					DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_0, "")
						NAV_FIELD_PROP_2NUMBER("program_packet_sequence_counter", 7, program_packet_sequence_counter, "")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_1, "")
						NAV_FIELD_PROP_2NUMBER("MPEG1_MPEG2_identifier", 1, MPEG1_MPEG2_identifier, "")
						NAV_FIELD_PROP_2NUMBER("original_stuff_length", 6, original_stuff_length, "")
						DECLARE_FIELDPROP_END()
				}PACKED;

				struct CPSTDBuffer : public DIRECT_ENDIAN_MAP {
					union {
						struct {
#ifdef _BIG_ENDIAN_
							unsigned short	fixed_bits_01 : 2;
							unsigned short	P_STD_buffer_scale : 1;
							unsigned short	P_STD_buffer_size : 13;
#else
							unsigned short	P_STD_buffer_size : 13;
							unsigned short	P_STD_buffer_scale : 1;
							unsigned short	fixed_bits_01 : 2;
#endif
						}PACKED;
						unsigned short	short_value;
					}PACKED;

					DECLARE_ENDIAN_BEGIN()
					USHORT_FIELD_ENDIAN(short_value)
						DECLARE_ENDIAN_END()

					DECLARE_FIELDPROP_BEGIN()
						NAV_FIELD_PROP_2NUMBER("marker_bit", 2, fixed_bits_01, "should be 01b")
						NAV_FIELD_PROP_2NUMBER("P_STD_buffer_scale", 1, P_STD_buffer_scale, "")
						NAV_FIELD_PROP_2NUMBER("P_STD_buffer_size", 13, P_STD_buffer_size, "")
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct CPESextension2 : public ADVANCE_ENDIAN_MAP {
					union {
						struct {
#ifdef _BIG_ENDIAN_
							unsigned char		marker_bit_0 : 1;
							unsigned char		PES_extension_field_length : 7;
							union {
								struct {
									unsigned char	stream_id_extension_flag : 1;
									unsigned char	stream_id_extension : 7;
								}PACKED;
								struct {
									unsigned char	stream_id_extension_flag_2 : 1;
									unsigned char	reserved_0 : 6;
									unsigned char	tref_extension_flag : 1;
								}PACKED;
							}PACKED;
#else
							unsigned char		PES_extension_field_length : 7;
							unsigned char		marker_bit_0 : 1;
							union {
								struct {
									unsigned char	stream_id_extension : 7;
									unsigned char	stream_id_extension_flag : 1;
								}PACKED;
								struct {
									unsigned char	tref_extension_flag : 1;
									unsigned char	reserved_0 : 6;
									unsigned char	stream_id_extension_flag_2 : 1;
								}PACKED;
							}PACKED;
#endif
						}PACKED;
						unsigned char hdr[2];
					}PACKED;
					CTimeStamp*	TREF;
					unsigned char* reserved;

					CPESextension2() : TREF(NULL), reserved(NULL) {
					}

					virtual ~CPESextension2() {
						Unmap();
					}

					int GetValidDataLength() {
						int valid_data_length = 2;
						if (stream_id_extension_flag && !tref_extension_flag) {
							valid_data_length += 5;
						}
						return valid_data_length;
					}

					int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size = 0, unsigned long *stuffing_size = 0)
					{
						unsigned long ulMappedSize = 0;

						if (pBuf == NULL)
							return RET_CODE_BUFFER_NOT_FOUND;

						MAP_MEM_TO_HDR2(hdr, 2);
						if (stream_id_extension_flag && !tref_extension_flag) {
							MAP_MEM_TO_STRUCT_POINTER(1, TREF, CTimeStamp)
						}

						reserved = pBuf + ulMappedSize;
						ulMappedSize = PES_extension_field_length + 1;

						AMP_SAFEASSIGN(desired_size, ulMappedSize);

						return RET_CODE_SUCCESS;
					}

					int Unmap(/* Out */ unsigned char* pBuf = NULL, /* In/Out */unsigned long* pcbSize = NULL)
					{
						if (stream_id_extension_flag && !tref_extension_flag) {
							UNMAP_STRUCT_POINTER(TREF)
						}

						Endian(false);
						return RET_CODE_SUCCESS;
					}

					DECLARE_FIELDPROP_BEGIN()
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit_0, "")
						NAV_FIELD_PROP_2NUMBER("PES_extension_field_length", 7, PES_extension_field_length, "")
						NAV_FIELD_PROP_2NUMBER("stream_id_extension_flag", 1, stream_id_extension_flag, "")
						if (!stream_id_extension_flag) {
							NAV_FIELD_PROP_2NUMBER("stream_id_extension", 7, stream_id_extension, "")
						}
						else {
							NAV_FIELD_PROP_2NUMBER("reserved", 6, reserved_0, "")
								NAV_FIELD_PROP_2NUMBER("tref_extension_flag", 1, tref_extension_flag, "")

								if (!tref_extension_flag) {
									NAV_FIELD_PROP_2NUMBER("reserved", 4, TREF->marker_bit_0011, "")
										NAV_FIELD_PROP_2NUMBER("TREF_32_30", 3, TREF->TimeStamp_0, "TREF[32..30]")
										NAV_FIELD_PROP_2NUMBER("marker_bit", 1, TREF->marker_bit_0, "")
										NAV_FIELD_PROP_2NUMBER("TREF_29_15", 15, TREF->TimeStamp_1, "TREF[29..15]")
										NAV_FIELD_PROP_2NUMBER("marker_bit", 1, TREF->marker_bit_1, "")
										NAV_FIELD_PROP_2NUMBER("TREF_14_0", 15, TREF->TimeStamp_2, "TREF[14..0]")
										NAV_FIELD_PROP_2NUMBER("marker_bit", 1, TREF->marker_bit_2, "")
								}
						}
						int cbReservedData = PES_extension_field_length + 1 - GetValidDataLength();
						NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved", ((long long)cbReservedData * 8), reserved, (unsigned long)cbReservedData, "")
					DECLARE_FIELDPROP_END()
				}PACKED;

				union {
					struct {
#ifdef _BIG_ENDIAN_
						unsigned char	PES_private_data_flag : 1;
						unsigned char	pack_header_field_flag : 1;
						unsigned char	program_packet_sequence_counter_flag : 1;
						unsigned char	P_STD_buffer_flag : 1;
						unsigned char	Reserved : 3;
						unsigned char	PES_extension_flag_2 : 1;
#else
						unsigned char	PES_extension_flag_2 : 1;
						unsigned char	Reserved : 3;
						unsigned char	P_STD_buffer_flag : 1;
						unsigned char	program_packet_sequence_counter_flag : 1;
						unsigned char	pack_header_field_flag : 1;
						unsigned char	PES_private_data_flag : 1;
#endif
					}PACKED;
					unsigned char	byte_value;
				}PACKED;
				unsigned char*			PES_private_data;
				CPackHeaderField*		pack_header_field;
				CProgramPacketSequenceCounter*
										program_packet_sequence_counter;
				CPSTDBuffer*			P_STD_buffer;
				CPESextension2*			PES_extension2;

				CPESextension() : PES_private_data(NULL), pack_header_field(NULL), program_packet_sequence_counter(NULL), P_STD_buffer(NULL), PES_extension2(NULL) {
				}

				virtual ~CPESextension() {
					Unmap();
				}

				int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size = 0, unsigned long *stuffing_size = 0)
				{
					unsigned long ulMappedSize = 0;

					if (pBuf == NULL)
						return RET_CODE_BUFFER_NOT_FOUND;

					MAP_MEM_TO_HDR2(&byte_value, 1);

					if (PES_private_data_flag) {
						PES_private_data = pBuf + ulMappedSize;
						ulMappedSize += 16;
					}

					MAP_MEM_TO_STRUCT_POINTER(pack_header_field_flag, pack_header_field, CPackHeaderField)
					MAP_MEM_TO_STRUCT_POINTER(program_packet_sequence_counter_flag, program_packet_sequence_counter, CProgramPacketSequenceCounter)
					MAP_MEM_TO_STRUCT_POINTER(P_STD_buffer_flag, P_STD_buffer, CPSTDBuffer)
					MAP_MEM_TO_STRUCT_POINTER2(PES_extension_flag_2, PES_extension2, CPESextension2)

					AMP_SAFEASSIGN(desired_size, ulMappedSize);

					return RET_CODE_SUCCESS;
				}

				int Unmap(/* Out */ unsigned char* pBuf = NULL, /* In/Out */unsigned long* pcbSize = NULL)
				{
					UNMAP_STRUCT_POINTER2(PES_extension2)
						UNMAP_STRUCT_POINTER(P_STD_buffer)
						UNMAP_STRUCT_POINTER(program_packet_sequence_counter)
						UNMAP_STRUCT_POINTER(pack_header_field)

						return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER("PES_private_data_flag", 1, PES_private_data_flag, "")
					NAV_FIELD_PROP_2NUMBER("pack_header_field_flag", 1, pack_header_field_flag, "")
					NAV_FIELD_PROP_2NUMBER("program_packet_sequence_counter_flag", 1, program_packet_sequence_counter_flag, "")
					NAV_FIELD_PROP_2NUMBER("P_STD_buffer_flag", 1, P_STD_buffer_flag, "")
					NAV_FIELD_PROP_2NUMBER("Reserved", 3, Reserved, "")
					NAV_FIELD_PROP_2NUMBER("PES_extension_flag_2", 1, PES_extension_flag_2, "")
					if (PES_private_data_flag) {
						NAV_FIELD_PROP_FIXSIZE_BINSTR("PES_private_data", 128, PES_private_data, 16, "")
					}
					if (pack_header_field_flag) {
						NAV_FIELD_PROP_REF(pack_header_field)
					}
					if (program_packet_sequence_counter_flag) {
						NAV_FIELD_PROP_REF(program_packet_sequence_counter)
					}
					if (P_STD_buffer_flag) {
						NAV_FIELD_PROP_REF(P_STD_buffer)
					}
					if (PES_extension_flag_2) {
						NAV_FIELD_PROP_REF(PES_extension2)
					}
				DECLARE_FIELDPROP_END()
			}PACKED;

			unsigned char	stream_type;

			unsigned short	PES_packet_length;

#ifdef _BIG_ENDIAN_
			unsigned char	marker_bit_10 : 2;
			unsigned char	PES_scrambling_control : 2;
			unsigned char	PES_priority : 1;
			unsigned char	data_alignment_indicator : 1;
			unsigned char	copyright : 1;
			unsigned char	original_or_copy : 1;

			unsigned char	PTS_DTS_flags : 2;
			unsigned char	ESCR_flag : 1;
			unsigned char	ES_rate_flag : 1;
			unsigned char	DSM_trick_mode_flag : 1;
			unsigned char	additional_copy_info_flag : 1;
			unsigned char	PES_CRC_flag : 1;
			unsigned char	PES_extension_flag : 1;
#else
			unsigned char	original_or_copy : 1;
			unsigned char	copyright : 1;
			unsigned char	data_alignment_indicator : 1;
			unsigned char	PES_priority : 1;
			unsigned char	PES_scrambling_control : 2;
			unsigned char	marker_bit_10 : 2;

			unsigned char	PES_extension_flag : 1;
			unsigned char	PES_CRC_flag : 1;
			unsigned char	additional_copy_info_flag : 1;
			unsigned char	DSM_trick_mode_flag : 1;
			unsigned char	ES_rate_flag : 1;
			unsigned char	ESCR_flag : 1;
			unsigned char	PTS_DTS_flags : 2;
#endif

			unsigned char	PES_header_data_length;

			CTimeStamp*		pts;
			CTimeStamp*		dts;
			CSCR*			ESCR;
			CESrate*		ES_rate;
			CDSMtrickmode*	DSM_trick_mode;
			CAdditionalCopyInfo*
							additional_copy_info;
			unsigned short	previous_PES_packet_CRC;
			CPESextension*	PES_extension;
			unsigned char*	stuffing_byte;
			unsigned char	stuffing_byte_count;
			union
			{
				unsigned char*	PES_packet_data_byte;
				MPEG2Video::CVideoBitstream*
								MPEG2_Video;
				H265Video::CVideoBitstream*
								H265_Video;
				H264Video::CVideoBitstream*
								H264_Video;
#ifdef _ENABLE_FULL_FEATURES_
				DolbyAudio::CAudioBitstream*
								Dolby_Audio;
				DTSHD::CAudioBitstream*
								DTSHD_Audio;
				HDMV::Graphic::HDMVGraphic*
								HDMV_Graphic;
#endif
				AACAudio::ADTSBitstream*
								MPEG2_AAC_Audio;
				// MPEG4-Audio
				AACAudio::CLOASAudioStream*
								MPEG4_AAC_Audio;
			};
			unsigned long	PES_Packet_data_count;

			CPESPacketHeaderAndData(unsigned char StreamType)
				: stream_type(StreamType)
				, pts(NULL)
				, dts(NULL)
				, ESCR(NULL)
				, ES_rate(NULL)
				, DSM_trick_mode(NULL)
				, additional_copy_info(NULL)
				, PES_extension(NULL)
				, stuffing_byte(NULL)
				, PES_packet_data_byte(NULL){
			}

			virtual ~CPESPacketHeaderAndData() {
				Unmap();
			}

			DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(PES_packet_length)
				DECLARE_ENDIAN_END()

			bool IsVideoKeyFrame() {
				if (stream_type == HEVC_VIDEO_STREAM)
				{
					if (H265_Video != nullptr)
						return H265_Video->ExistBDMVKeyFrame();
				}
				else if (stream_type == MPEG4_AVC_VIDEO_STREAM)
				{
					if (H264_Video != nullptr)
						return H264_Video->ExistBDMVKeyFrame();
				}
				else if (stream_type == MPEG2_VIDEO_STREAM)
				{
					if (MPEG2_Video != nullptr)
						return MPEG2_Video->ExistBDMVKeyFrame();
				}

				return false;
			}

			/*!	@brief Transfer the accurate data size of whole PES packet as cbSize. */
			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size = 0, unsigned long *stuffing_size = 0)
			{
				unsigned long ulMappedSize = 0;

				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;

				MAP_MEM_TO_HDR2(&PES_packet_length, 5);

				MAP_MEM_TO_STRUCT_POINTER((PTS_DTS_flags & 0x02), pts, CTimeStamp)
				MAP_MEM_TO_STRUCT_POINTER((PTS_DTS_flags & 0x01), dts, CTimeStamp)
				MAP_MEM_TO_STRUCT_POINTER(ESCR_flag, ESCR, CSCR)
				MAP_MEM_TO_STRUCT_POINTER(ES_rate_flag, ES_rate, CESrate)
				MAP_MEM_TO_STRUCT_POINTER(DSM_trick_mode_flag, DSM_trick_mode, CDSMtrickmode)
				MAP_MEM_TO_STRUCT_POINTER(additional_copy_info_flag, additional_copy_info, CAdditionalCopyInfo)
				if (PES_CRC_flag) {
					previous_PES_packet_CRC = *(pBuf + ulMappedSize);
					USHORT_FIELD_ENDIAN(previous_PES_packet_CRC)
					ulMappedSize++;
				}
				MAP_MEM_TO_STRUCT_POINTER2(PES_extension_flag, PES_extension, CPESextension)

				if (PES_packet_length != 0) {
					PES_Packet_data_count = PES_packet_length - (3 + PES_header_data_length);
					if (cbSize < PES_Packet_data_count + PES_header_data_length + 5) {
						Unmap();
						return RET_CODE_BUFFER_TOO_SMALL;
					}
				}
				else
				{
					if (cbSize < (unsigned long)(5 + PES_header_data_length)) {
						Unmap();
						return RET_CODE_BUFFER_TOO_SMALL;
					}
					PES_Packet_data_count = cbSize - (5 + PES_header_data_length);
				}

				stuffing_byte = pBuf + ulMappedSize;
				stuffing_byte_count = (unsigned char)(5 + PES_header_data_length - ulMappedSize);

#ifdef _ENABLE_FULL_FEATURES_
				bool bInScanning = AMTLS_GetEnvInteger(_T("TSScanning"), 0) ? true : false;
#else
				bool bInScanning = false;
#endif
				if (stream_type == MPEG2_VIDEO_STREAM)
				{
					if (!bInScanning)
					{
						CAMBstRef bst(AMBst_CreateFromBuffer(pBuf + 5 + PES_header_data_length, PES_Packet_data_count));
						MAP_MEM_TO_STRUCT_POINTER5(1, MPEG2_Video, MPEG2Video::CVideoBitstream);
					}
					else
					{
						MAP_MEM_TO_STRUCT_POINTER6(1, pBuf + 5 + PES_header_data_length, PES_Packet_data_count,
							MPEG2_Video, MPEG2Video::CVideoBitstream, { SEQUENCE_HEADER_CODE, PICTURE_START_CODE });
					}
				}
				else if (stream_type == HEVC_VIDEO_STREAM)
				{
					if (!bInScanning)
					{
						CAMBstRef bst(AMBst_CreateFromBuffer(pBuf + 5 + PES_header_data_length, PES_Packet_data_count));
						MAP_MEM_TO_STRUCT_POINTER5(1, H265_Video, H265Video::CVideoBitstream);
					}
					else
					{
						MAP_MEM_TO_STRUCT_POINTER6(1, pBuf + 5 + PES_header_data_length, PES_Packet_data_count,
							H265_Video, H265Video::CVideoBitstream, { H265Video::VPS_NUT, H265Video::SPS_NUT, H265Video::PPS_NUT });
					}
				}
				else if (stream_type == MPEG4_AVC_VIDEO_STREAM || stream_type == MPEG4_MVC_VIDEO_STREAM)
				{
					if (!bInScanning)
					{
						AMP_NEWT(H264_Video, H264Video::CVideoBitstream);
						int iMMRet = H264_Video->MapES(pBuf + 5 + PES_header_data_length, PES_Packet_data_count);
						if (iMMRet != RET_CODE_SUCCESS)
							return iMMRet;
					}
					else
					{
						MAP_MEM_TO_STRUCT_POINTER6(1, pBuf + 5 + PES_header_data_length, PES_Packet_data_count,
							H264_Video, H264Video::CVideoBitstream, { H264Video::SPS_NUT, H264Video::PPS_NUT });
					}
				}
#ifdef _ENABLE_FULL_FEATURES_
				else if ((stream_type == DOLBY_AC3_AUDIO_STREAM ||
						  stream_type == DOLBY_LOSSLESS_AUDIO_STREAM && 
								PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
								PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
								PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x76 ||
						 stream_type == DD_PLUS_AUDIO_STREAM/* &&
								PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
								PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
								PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x71*/))
				{
					if (!bInScanning)
					{
						CAMBstRef bst(AMBst_CreateFromBuffer(pBuf + 5 + PES_header_data_length, PES_Packet_data_count));
						MAP_MEM_TO_STRUCT_POINTER5(1, Dolby_Audio, DolbyAudio::CAudioBitstream);
					}
					else
					{
						if (stream_type == DOLBY_AC3_AUDIO_STREAM)
						{
							MAP_MEM_TO_STRUCT_POINTER6(1, pBuf + 5 + PES_header_data_length, PES_Packet_data_count, Dolby_Audio, DolbyAudio::CAudioBitstream);
						}
					}
				}
				else if ((stream_type == DTS_AUDIO_STREAM || stream_type == DTS_HD_EXCEPT_XLL_AUDIO_STREAM || stream_type == DTS_HD_XLL_AUDIO_STREAM || stream_type == DTS_HD_SECONDARY_AUDIO_STREAM) && !bInScanning)
				{
					CAMBstRef bst(AMBst_CreateFromBuffer(pBuf + 5 + PES_header_data_length, PES_Packet_data_count));
					MAP_MEM_TO_STRUCT_POINTER5(1, DTSHD_Audio, DTSHD::CAudioBitstream);
				}
#endif
				else if (stream_type == AAC_AUDIO_STREAM)
				{
					if (bInScanning)
					{
						int cbLend = 0, cbLeft = 0;
						MAP_MEM_TO_STRUCT_POINTER7(1, pBuf + 5 + PES_header_data_length, PES_Packet_data_count, cbLend, cbLeft, MPEG2_AAC_Audio, AACAudio::ADTSBitstream);
					}
					else
					{
						CAMBstRef bst(AMBst_CreateFromBuffer(pBuf + 5 + PES_header_data_length, PES_Packet_data_count));
						MAP_MEM_TO_STRUCT_POINTER5(1, MPEG2_AAC_Audio, AACAudio::ADTSBitstream);
					}
				}
#ifdef _ENABLE_FULL_FEATURES_
				else if ((stream_type == INTERACTIVE_GRAPHICS_STREAM || stream_type == PRESENTATION_GRAPHICS_STREAM) && !bInScanning)
				{
					MAP_MEM_TO_STRUCT_POINTER2(1, HDMV_Graphic, HDMV::Graphic::HDMVGraphic);
				}
#endif
				else if (stream_type == MPEG4_AAC_AUDIO_STREAM)
				{
					if (bInScanning)
					{
						PES_packet_data_byte = pBuf + 5 + PES_header_data_length;
						MPEG4_AAC_Audio = NULL;
					}
					else
					{
						CAMBstRef bst(AMBst_CreateFromBuffer(pBuf + 5 + PES_header_data_length, PES_Packet_data_count));
						MAP_MEM_TO_STRUCT_POINTER5(1, MPEG4_AAC_Audio, AACAudio::CLOASAudioStream);
					}
				}
				else
					PES_packet_data_byte = pBuf + 5 + PES_header_data_length;

				ulMappedSize = PES_Packet_data_count + PES_header_data_length + 5;

				AMP_SAFEASSIGN(desired_size, ulMappedSize);

				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf = NULL, /* In/Out */unsigned long* pcbSize = NULL)
			{
#ifdef _ENABLE_FULL_FEATURES_
				bool bInScanning = AMTLS_GetEnvInteger(_T("TSScanning"), 0) ? true : false;
#else
				bool bInScanning = false;
#endif
				if (stream_type == MPEG2_VIDEO_STREAM)
				{
					UNMAP_STRUCT_POINTER5(MPEG2_Video);
				}
				else if (stream_type == HEVC_VIDEO_STREAM)
				{
					UNMAP_STRUCT_POINTER5(H265_Video);
				}
				else if (stream_type == MPEG4_AVC_VIDEO_STREAM || stream_type == MPEG4_MVC_VIDEO_STREAM)
				{
					UNMAP_STRUCT_POINTER5(H264_Video);
				}
#ifdef _ENABLE_FULL_FEATURES_
				else if ((stream_type == DOLBY_AC3_AUDIO_STREAM ||
						  stream_type == DOLBY_LOSSLESS_AUDIO_STREAM &&
							  PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
							  PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
							  PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x76 ||
						  stream_type == DD_PLUS_AUDIO_STREAM/* &&
							  PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
							  PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
							  PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x71*/))
				{
					if (!bInScanning || stream_type == DOLBY_AC3_AUDIO_STREAM) {
						UNMAP_STRUCT_POINTER5(Dolby_Audio);
					}
				}
				else if ((stream_type == DTS_AUDIO_STREAM || stream_type == DTS_HD_EXCEPT_XLL_AUDIO_STREAM || stream_type == DTS_HD_XLL_AUDIO_STREAM || stream_type == DTS_HD_SECONDARY_AUDIO_STREAM) && !bInScanning)
				{
					UNMAP_STRUCT_POINTER5(DTSHD_Audio);
				}
#endif
				else if (stream_type == AAC_AUDIO_STREAM)
				{
					UNMAP_STRUCT_POINTER5(MPEG2_AAC_Audio);
				}
				else if (stream_type == MPEG4_AAC_AUDIO_STREAM)
				{
					UNMAP_STRUCT_POINTER5(MPEG4_AAC_Audio);
				}
#ifdef _ENABLE_FULL_FEATURES_
				else if ((stream_type == INTERACTIVE_GRAPHICS_STREAM || stream_type == PRESENTATION_GRAPHICS_STREAM) && !bInScanning)
				{
					UNMAP_STRUCT_POINTER2(HDMV_Graphic);
				}
#endif
				PES_packet_data_byte = NULL;
				stuffing_byte = NULL;
				UNMAP_STRUCT_POINTER2(PES_extension)
				UNMAP_STRUCT_POINTER(additional_copy_info)
				UNMAP_STRUCT_POINTER(DSM_trick_mode)
				UNMAP_STRUCT_POINTER(ES_rate)
				UNMAP_STRUCT_POINTER(ESCR)
				UNMAP_STRUCT_POINTER(dts)
				UNMAP_STRUCT_POINTER(pts)
				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(i);
				NAV_FIELD_PROP_2NUMBER("PES_packet_length", 16, PES_packet_length, "")

				NAV_FIELD_PROP_2NUMBER("marker_bit", 2, marker_bit_10, "Should be 2(10b)")
				NAV_FIELD_PROP_2NUMBER("PES_scrambling_control", 2, PES_scrambling_control, "")
				NAV_FIELD_PROP_2NUMBER("PES_priority", 1, PES_priority, "")
				NAV_FIELD_PROP_2NUMBER("data_alignment_indicator", 1, data_alignment_indicator, "")
				NAV_FIELD_PROP_2NUMBER("copyright", 1, copyright, "")
				NAV_FIELD_PROP_2NUMBER("original_or_copy", 1, original_or_copy, "")

				NAV_FIELD_PROP_2NUMBER("PTS_DTS_flags", 2, PTS_DTS_flags, "")
				NAV_FIELD_PROP_2NUMBER("ESCR_flag", 1, ESCR_flag, "")
				NAV_FIELD_PROP_2NUMBER("ES_rate_flag", 1, ES_rate_flag, "")
				NAV_FIELD_PROP_2NUMBER("DSM_trick_mode_flag", 1, DSM_trick_mode_flag, "")
				NAV_FIELD_PROP_2NUMBER("additional_copy_info_flag", 1, additional_copy_info_flag, "")
				NAV_FIELD_PROP_2NUMBER("PES_CRC_flag", 1, PES_CRC_flag, "")
				NAV_FIELD_PROP_2NUMBER("PES_extension_flag", 1, PES_extension_flag, "")

				NAV_FIELD_PROP_2NUMBER("PES_header_data_length", 8, PES_header_data_length, "")

				if (pts) {
					MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "%lld(0X%llX) (90KHZ)", pts->GetTimeStamp(), pts->GetTimeStamp());
					NAV_WRITE_TAG_BEGIN_1("PTS", szTemp4, 0);
						NAV_FIELD_PROP_2NUMBER("reserved", 4, pts->marker_bit_0011, "")
						NAV_FIELD_PROP_2NUMBER("PTS_32_30", 3, pts->TimeStamp_0, "PTS[32..30]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, pts->marker_bit_0, "")
						NAV_FIELD_PROP_2NUMBER("PTS_29_15", 15, pts->TimeStamp_1, "PTS[29..15]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, pts->marker_bit_1, "")
						NAV_FIELD_PROP_2NUMBER("PTS_14_0", 15, pts->TimeStamp_2, "PTS[14..0]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, pts->marker_bit_2, "")
					NAV_WRITE_TAG_END("PTS");
				}

				if (dts) {
					MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "%lld(0X%llX) (90KHZ)", dts->GetTimeStamp(), dts->GetTimeStamp());
					NAV_WRITE_TAG_BEGIN_1("DTS", szTemp4, 0);
						NAV_FIELD_PROP_2NUMBER("reserved", 4, dts->marker_bit_0011, "")
						NAV_FIELD_PROP_2NUMBER("DTS_32_30", 3, dts->TimeStamp_0, "DTS[32..30]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, dts->marker_bit_0, "")
						NAV_FIELD_PROP_2NUMBER("DTS_29_15", 15, dts->TimeStamp_1, "DTS[29..15]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, dts->marker_bit_1, "")
						NAV_FIELD_PROP_2NUMBER("DTS_14_0", 15, dts->TimeStamp_2, "DTS[14..0]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, dts->marker_bit_2, "")
					NAV_WRITE_TAG_END("DTS");
				}

				if (ESCR) {
					MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "%lld(0X%llX) (27MHZ) base: %lld, ext: %d", ESCR->GetSCR(), ESCR->GetSCR(), ESCR->GetSCRbase(), ESCR->GetSCRextension());
					NAV_WRITE_TAG_BEGIN_1("ESCR", szTemp4, 0);
					NAV_FIELD_PROP_2NUMBER("reserved", 2, ESCR->reserved, "")
						NAV_FIELD_PROP_2NUMBER("ESCR_base_32_30", 3, ESCR->SCR_base_0, "ESCR_base[32..30]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, ESCR->marker_bit_0, "")
						NAV_FIELD_PROP_2NUMBER("ESCR_base_29_15", 15, ((unsigned long)ESCR->SCR_base_1 << 13 | (unsigned long)ESCR->SCR_base_2 << 5 | (unsigned long)ESCR->SCR_base_3), "ESCR_base[29..15]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, ESCR->marker_bit_1, "")
						NAV_FIELD_PROP_2NUMBER("ESCR_base_14_0", 15, ((unsigned long)ESCR->SCR_base_4 << 13 | (unsigned long)ESCR->SCR_base_5 << 5 | (unsigned long)ESCR->SCR_base_6), "ESCR_base[14..0]")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, ESCR->marker_bit_2, "")
						NAV_FIELD_PROP_2NUMBER("ESCR_extension", 9, ((unsigned long)ESCR->SCR_extension_0 << 7 | (unsigned long)ESCR->SCR_extension_1), "")
						NAV_FIELD_PROP_2NUMBER("marker_bit", 1, ESCR->marker_bit_3, "")
						NAV_WRITE_TAG_END("ESCR");
				}

				NAV_FIELD_PROP_REF(ES_rate)
				NAV_FIELD_PROP_REF(DSM_trick_mode)
				NAV_FIELD_PROP_REF(additional_copy_info)
				if (PES_CRC_flag) {
					NAV_FIELD_PROP_2NUMBER("previous_PES_packet_CRC", 16, previous_PES_packet_CRC, "")
				}
				NAV_FIELD_PROP_REF(PES_extension)
				NAV_FIELD_PROP_FIXSIZE_BINSTR("stuffing_byte", (unsigned long)stuffing_byte_count * 8, stuffing_byte, (unsigned long)stuffing_byte_count, "")

				if (stream_type == MPEG2_VIDEO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(MPEG2_Video, "MPEG2_Video", "", 1);
				}
				else if (stream_type == HEVC_VIDEO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(H265_Video, "H265_Video", "", 1);
				}
				else if (stream_type == MPEG4_AVC_VIDEO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(H264_Video, "H264_Video", "", 1);
				}
				else if (stream_type == MPEG4_MVC_VIDEO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(H264_Video, "MVC_Video", "", 1);
				}
#ifdef _ENABLE_FULL_FEATURES_
				else if (stream_type == DOLBY_AC3_AUDIO_STREAM ||
						 stream_type == DOLBY_LOSSLESS_AUDIO_STREAM &&
							 PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
							 PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
							 PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x76)
				{
					NAV_FIELD_PROP_REF2_3(Dolby_Audio, "AC3_Audio", "", 1);
				}
				else if (stream_type == DD_PLUS_AUDIO_STREAM)
				{
					if (PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
						PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
						PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x71)
					{
						NAV_FIELD_PROP_REF2_3(Dolby_Audio, "DDPlus_Independent_Audio", "", 1);
					}
					else if (PES_extension_flag && PES_extension && PES_extension->PES_extension_flag_2 &&
						PES_extension->PES_extension2 && PES_extension->PES_extension2->PES_extension_field_length == 1 &&
						PES_extension->PES_extension2->stream_id_extension_flag == 0 && PES_extension->PES_extension2->stream_id_extension == 0x72)
					{
						NAV_FIELD_PROP_REF2_3(Dolby_Audio, "DDPlus_Dependent_Audio", "", 1);
					}
					else
					{
						NAV_FIELD_PROP_REF2_3(Dolby_Audio, "DDPlus_Audio", "", 1);
					}
				}
				else if (stream_type == DTS_AUDIO_STREAM || stream_type == DTS_HD_EXCEPT_XLL_AUDIO_STREAM || stream_type == DTS_HD_XLL_AUDIO_STREAM || stream_type == DTS_HD_SECONDARY_AUDIO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(DTSHD_Audio,  stream_type == DTS_HD_EXCEPT_XLL_AUDIO_STREAM?"DTSHD_Audio":(
														stream_type == DTS_HD_XLL_AUDIO_STREAM ?"DTSHD_XLL_Audio":(
														stream_type == DTS_HD_SECONDARY_AUDIO_STREAM ?"DTS_LBR_Audio":"DTS_Audio")), "", 1);
				}
#endif
				else if (stream_type == AAC_AUDIO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(MPEG2_AAC_Audio, "MPEG2_AAC", "", 1);
				}
				else if (stream_type == MPEG4_AAC_AUDIO_STREAM)
				{
					NAV_FIELD_PROP_REF2_3(MPEG4_AAC_Audio, "MPEG4_AAC", "", 1);
				}
#ifdef _ENABLE_FULL_FEATURES_
				else if ((stream_type == INTERACTIVE_GRAPHICS_STREAM || stream_type == PRESENTATION_GRAPHICS_STREAM))
				{
					NAV_FIELD_PROP_REF2_3(HDMV_Graphic, stream_type == INTERACTIVE_GRAPHICS_STREAM ? "Interactive_Graphic" : (
														stream_type == PRESENTATION_GRAPHICS_STREAM ? "Presentation_Graphic" : "Unknown"), "", 1);
				}
#endif
				else
				{
					NAV_FIELD_PROP_FIXSIZE_BINSTR("PES_packet_data_byte", PES_Packet_data_count*8, PES_packet_data_byte, PES_Packet_data_count, "")
				}
			DECLARE_FIELDPROP_END()	
		}PACKED;

		struct CPESPacketData: public DIRECT_ENDIAN_MAP{
			unsigned short	PES_packet_length;
			unsigned char	PES_packet_data_byte[];

			int GetVarBodySize(){return PES_packet_length;}

			DECLARE_ENDIAN_BEGIN()
				USHORT_FIELD_ENDIAN(PES_packet_length);
			DECLARE_ENDIAN_END()

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(i);
				NAV_FIELD_PROP_2NUMBER("PES_packet_length", 16, PES_packet_length, "")
				NAV_FIELD_PROP_FIXSIZE_BINSTR("PES_packet_data_byte", ((long long)PES_packet_length*8), PES_packet_data_byte, (unsigned long)PES_packet_length, "")
			DECLARE_FIELDPROP_END()	
		}PACKED;


		unsigned char	stream_type;
		union {
			struct {
				unsigned char	packet_start_code_prefix[3];
				unsigned char	stream_id;
			};
			unsigned char prefix_4bytes[4];
		};
		union{
			unsigned char*				pes_data = nullptr;
			CPESPacketHeaderAndData*	pes_header_body;
			CPESPacketData*				pes_body;
		}PACKED;

		CPES(unsigned char StreamType=0xFF): stream_type(StreamType), pes_data(NULL){
		}

		virtual ~CPES(){
			Unmap();
		}

		void SetStreamType(unsigned char StreamType){stream_type = StreamType;}

		bool HasPESHdr() {
			return (stream_id != PROGRAM_STREAM_MAP &&
				stream_id != PRIVATE_STREAM_2 &&
				stream_id != ECM_STREAM &&
				stream_id != EMM_STREAM &&
				stream_id != PROGRAM_STREAM_DIRECTORY &&
				stream_id != DSMCC_STREAM &&
				stream_id != H_222_1_TYPE_E);
		}

		int GetESStartPos()
		{
			if (stream_id != PROGRAM_STREAM_MAP &&
				stream_id != PRIVATE_STREAM_2 &&
				stream_id != ECM_STREAM &&
				stream_id != EMM_STREAM &&
				stream_id != PROGRAM_STREAM_DIRECTORY &&
				stream_id != DSMCC_STREAM &&
				stream_id != H_222_1_TYPE_E)
				return 9 + pes_header_body->PES_header_data_length;
			return 4;
		}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(prefix_4bytes, 4);

			if (!IS_PES_PAYLOAD(prefix_4bytes))
				return RET_CODE_BUFFER_NOT_COMPATIBLE;

			if (stream_id != PROGRAM_STREAM_MAP && 
				stream_id != PRIVATE_STREAM_2 && 
				stream_id != ECM_STREAM &&
				stream_id != EMM_STREAM &&
				stream_id != PROGRAM_STREAM_DIRECTORY &&
				stream_id != DSMCC_STREAM &&
				stream_id != H_222_1_TYPE_E){
				MAP_MEM_TO_STRUCT_POINTER2(1, pes_header_body, CPESPacketHeaderAndData, stream_type)
			} else {
				MAP_MEM_TO_STRUCT_POINTER(1, pes_body, CPESPacketData)	
			}

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (stream_id != PROGRAM_STREAM_MAP && 
				stream_id != PRIVATE_STREAM_2 && 
				stream_id != ECM_STREAM &&
				stream_id != EMM_STREAM &&
				stream_id != PROGRAM_STREAM_DIRECTORY &&
				stream_id != DSMCC_STREAM &&
				stream_id != H_222_1_TYPE_E){
				UNMAP_STRUCT_POINTER2(pes_header_body)
			} else {
				UNMAP_STRUCT_POINTER(pes_body)
			}

			return RET_CODE_SUCCESS;
		}

		const char* GetStreamCoding(unsigned char stream_id){
			switch(stream_id){
			case PROGRAM_STREAM_MAP: return "program_stream_map";
			case PRIVATE_STREAM_1: return "private_stream_1";	
			case PADDING_STREAM: return "padding_stream";
			case PRIVATE_STREAM_2: return "private_stream_2";
			case ECM_STREAM: return "ECM_stream";
			case EMM_STREAM: return "EMM_stream";
			case DSMCC_STREAM: return "Rec. ITU-T H.222.0 | ISO/IEC 13818-1 Annex A or ISO/IEC 13818-6_DSMCC_stream";
			case ISO_IEC_13522_STREAM: return "ISO/IEC_13522_stream";
			case H_222_1_TYPE_A: return "Rec. ITU-T H.222.1 type A";
			case H_222_1_TYPE_B: return "Rec. ITU-T H.222.1 type B";
			case H_222_1_TYPE_C: return "Rec. ITU-T H.222.1 type C";
			case H_222_1_TYPE_D: return "Rec. ITU-T H.222.1 type D";
			case H_222_1_TYPE_E: return "Rec. ITU-T H.222.1 type E";
			case ANCILLARY_STREAM: return "ancillary_stream";
			case ISO_IEC_14496_1_SL_PACKETIZED_STREAM: return "ISO/IEC 14496-1_SL-packetized_stream";
			case ISO_IEC_14496_1_FLEXMUX_STREAM: return "ISO/IEC 14496-1_FlexMux_stream";
			case METADATA_STREAM: return "metadata stream";
			case EXTENDED_STREAM_ID: return "extended_stream_id";
			case RESERVED_DATA_STREAM: return "reserved data stream";
			case PROGRAM_STREAM_DIRECTORY: return "program_stream_directory";
			default: 
				if (stream_id >= AUDIO_STREAM_FIRST && stream_id <= AUDIO_STREAM_LAST){
					return "ISO/IEC 13818-3 or ISO/IEC 11172-3 or ISO/IEC 13818-7 or ISO/IEC 14496-3 audio stream number x xxxx";
				}else if(stream_id >= VIDEO_STREAM_FIRST && stream_id <= VIDEO_STREAM_LAST){
					return "Rec. ITU-T H.262 | ISO/IEC 13818-2, ISO/IEC 11172-2, ISO/IEC 14496-2 or Rec. ITU-T H.264 | ISO/IEC 14496-10 video stream number xxxx";
				}else{
					return "Unknown";
				}
			}
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(i);
			NAV_WRITE_TAG_BEGIN_1("PES_packet", "", 1);
				NAV_FIELD_PROP_FIXSIZE_BINSTR("packet_start_code_prefix", 3*8, packet_start_code_prefix, 3, "Should be 00 00 01")
				NAV_FIELD_PROP_2NUMBER("stream_id", 8, stream_id, GetStreamCoding(stream_id));
				if (stream_id != PROGRAM_STREAM_MAP && 
					stream_id != PRIVATE_STREAM_2 && 
					stream_id != ECM_STREAM &&
					stream_id != EMM_STREAM &&
					stream_id != PROGRAM_STREAM_DIRECTORY &&
					stream_id != DSMCC_STREAM &&
					stream_id != H_222_1_TYPE_E){
					NAV_FIELD_PROP_REF(pes_header_body)
				} else {
					NAV_FIELD_PROP_REF(pes_body)
				}
			NAV_WRITE_TAG_END("PES_packet");
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CPESBufObj: public MEM_MAP_MGR
	{	
		unsigned char stream_type;
		CPES* pPES;

		CPESBufObj(unsigned char StreamType=0xFF, IAMMemMgr* pMemMgr=NULL):MEM_MAP_MGR(-1, pMemMgr), stream_type(StreamType), pPES(NULL){}

		virtual ~CPESBufObj(){Unmap();}

		int Map(unsigned long *desired_size = NULL,unsigned long *stuffing_size = NULL)
		{
			unsigned long  ulMappedSize = 0;
			unsigned char* pBuf = m_binData;
			unsigned long  cbSize = m_cbDataSize; 

			if(pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_STRUCT_POINTER2(1, pPES, CPES, stream_type);

			return RET_CODE_SUCCESS;
		}

		virtual int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			UNMAP_STRUCT_POINTER2(pPES);
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_WRITE_TAG_BEGIN2("PES");
				NAV_FIELD_PROP_REF(pPES);
			NAV_WRITE_TAG_END2("PES");

#if 0
			if (szOutXml != NULL)
			{
				FILE* fp = NULL;
				fopen_s(&fp, "E:\\pes.xml", "wb");
				if (fp != NULL)
				{
					fwrite(szOutXml, 1, cbLen, fp);
					fclose(fp);
				}
			}
#endif
		DECLARE_FIELDPROP_END()
	};

	struct CShortGenericSection: public DIRECT_ENDIAN_MAP{
		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		marker_bit:1;
				unsigned short		reserved:2;
				unsigned short		section_length:12;
#else
				unsigned short		section_length:12;
				unsigned short		reserved:2;
				unsigned short		marker_bit:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned char	section_data[];

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
		DECLARE_ENDIAN_END()

		int GetVarBodySize(){return (ENDIANUSHORT(short_value_0)&0xFFF) - 3;}

		BOOL IsEqual(CShortGenericSection* pShortSection){
			if (pShortSection == NULL || 
				pShortSection->table_id != table_id ||
				pShortSection->section_length != section_length)
				return FALSE;

			return memcmp((void*)pShortSection->section_data, (void*)section_data, section_length)==0?TRUE:FALSE;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(i);
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, "")
			NAV_FIELD_PROP_2NUMBER1(marker_bit, 1, "")
			NAV_FIELD_PROP_2NUMBER1(reserved, 2, "")
			NAV_FIELD_PROP_2NUMBER1(section_length, 12, "")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(section_data, (unsigned long)(section_length-3)*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CLongGenericSection: public ADVANCE_ENDIAN_MAP{
		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		private_indicator:1;
				unsigned short		reserved_0:2;
				unsigned short		private_section_length:12;
#else
				unsigned short		private_section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		private_indicator:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned short		table_id_extension;

#ifdef _BIG_ENDIAN_
		unsigned char		reserved:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved:2;
#endif
		unsigned char		section_number;
		unsigned char		last_section_number;
		unsigned char*		private_data_byte;
		unsigned short		private_data_length;
		unsigned long		CRC_32;

		CLongGenericSection(): private_data_byte(NULL){}
		virtual ~CLongGenericSection(){Unmap();}

		BOOL IsEqual(CLongGenericSection* pLongSection){
			if (pLongSection == NULL)
				return FALSE;

			if (pLongSection->table_id != table_id ||
				pLongSection->short_value_0 != short_value_0 ||
				pLongSection->table_id_extension != table_id_extension ||
				pLongSection->current_next_indicator != current_next_indicator ||
				pLongSection->version_number != version_number ||
				pLongSection->section_number != section_number ||
				pLongSection->last_section_number != last_section_number ||
				pLongSection->private_data_length != private_data_length ||
				pLongSection->CRC_32 != CRC_32)
				return FALSE;

			return memcmp((void*)pLongSection->private_data_byte, (void*)private_data_byte, private_data_length)==0?TRUE:FALSE;
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
			USHORT_FIELD_ENDIAN(table_id_extension)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 8);

			if(!section_syntax_indicator)
				return RET_CODE_BUFFER_NOT_COMPATIBLE;

			if (cbSize < (unsigned long)private_section_length + 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			private_data_byte = pBuf + ulMappedSize;
			ulMappedSize += private_data_length;

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			private_data_length = private_section_length - 9;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				private_data_byte = NULL;
			}
			else
			{
				unsigned long cbRequired = private_section_length + 3;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, 1);
			AMBst_PutBits(bs, 1, private_indicator);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 12, private_section_length);

			AMBst_PutBits(bs, 16, table_id_extension);

			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);

			AMBst_PutBytes(bs, private_data_byte, private_data_length);
			AMBst_PutDWord(bs, CRC_32);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(i);
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, section_syntax_indicator
				?"the section follows the generic section syntax beyond the private_section_length field"
				:"the private_data_bytes immediately follow the private_section_length field")
			NAV_FIELD_PROP_2NUMBER1(private_indicator, 1, "a 1-bit user-definable flag that shall not be specified by ITU-T | ISO/IEC in the future")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(private_section_length, 12, "the number of remaining bytes in the private section immediately following the private_section_length field up to the end of the private_section")
			NAV_FIELD_PROP_2NUMBER1(table_id_extension, 16, "Its use and value are defined by the user.")
			NAV_FIELD_PROP_2NUMBER1(reserved, 2, "")
			NAV_FIELD_PROP_2NUMBER1(version_number, 5, "the version number of the private_section")
			NAV_FIELD_PROP_2NUMBER1(current_next_indicator, 1, current_next_indicator?"the private_section sent is currently applicable":"the private_section sent is not yet applicable and shall be the next private_section with the same section_number and table_id to become valid")

			NAV_FIELD_PROP_2NUMBER1(section_number, 1, "the number of the private_section")
			NAV_FIELD_PROP_2NUMBER1(last_section_number, 1, "the number of the last section of the private table of which this section is a part")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)private_data_length*8, "")
			NAV_FIELD_PROP_2NUMBER1(CRC_32, 32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the entire private section")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CProgramAssociationSection: public ADVANCE_ENDIAN_MAP{

		struct CProgramEntry: public DIRECT_ENDIAN_MAP{
			unsigned short		program_number;
			union{
				struct {
#ifdef _BIG_ENDIAN_
					unsigned short	reserved:3;
					unsigned short	network_PID:13;
#else
					unsigned short	network_PID:13;
					unsigned short	reserved:3;
#endif
				}PACKED;
				struct {
#ifdef _BIG_ENDIAN_
					unsigned short	reserved:3;
					unsigned short	program_map_PID:13;
#else
					unsigned short	program_map_PID:13;
					unsigned short	reserved_2:3;
#endif
				}PACKED;
				unsigned short short_value;
			}PACKED;

			DECLARE_ENDIAN_BEGIN()
				USHORT_FIELD_ENDIAN(program_number)
				USHORT_FIELD_ENDIAN(short_value)
			DECLARE_ENDIAN_END()

		}PACKED;

		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		marker_bit:1;
				unsigned short		reserved_0:2;
				unsigned short		section_length:12;
#else
				unsigned short		section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		marker_bit:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned short		transport_stream_id;
		
#ifdef _BIG_ENDIAN_
		unsigned char		reserved_1:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved_1:2;
#endif

		unsigned char		section_number;
		unsigned char		last_section_number;

		std::vector<CProgramEntry*>	
							program_entries;
		
		unsigned long		CRC_32;

		CProgramAssociationSection(){}
		virtual ~CProgramAssociationSection(){Unmap();}

		BOOL IsEqual(CProgramAssociationSection* pPAS){
			if (pPAS == NULL)
				return FALSE;

			if (pPAS->table_id != table_id ||
				pPAS->short_value_0 != short_value_0 ||
				pPAS->transport_stream_id != transport_stream_id ||
				pPAS->current_next_indicator != current_next_indicator ||
				pPAS->version_number != version_number ||
				pPAS->section_number != section_number ||
				pPAS->last_section_number != last_section_number ||
				pPAS->CRC_32 != CRC_32)
				return FALSE;

			if (pPAS->program_entries.size() != program_entries.size())
				return FALSE;

			for(size_t i=0;i<pPAS->program_entries.size();i++)
				if (pPAS->program_entries[i]->program_map_PID != program_entries[i]->program_map_PID ||
					pPAS->program_entries[i]->short_value != program_entries[i]->short_value)
					return FALSE;

			return TRUE;
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
			USHORT_FIELD_ENDIAN(transport_stream_id)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 8);
			/* transport_stream_id: 2 bytes, (reserved, version_number, current_next_indicator): 1byte, section_number: 1 byte, last_section_number:1 byte, CRC_32: 4bytes*/
			for(unsigned short i=0;i<(section_length - 9)/sizeof(CProgramEntry);i++){
				CProgramEntry* pEntry = NULL;
				MAP_MEM_TO_STRUCT_POINTER(1, pEntry, CProgramEntry)
				program_entries.push_back(pEntry);
			}

			if (ulMappedSize + 4 > cbSize){
				Unmap();
				return RET_CODE_BUFFER_TOO_SMALL;
			}

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL){
				for(size_t i=0;i<program_entries.size();i++){
					UNMAP_STRUCT_POINTER(program_entries[i])
				}
			}
			else
			{
				unsigned long cbRequired = 8;
				cbRequired += (unsigned long)program_entries.size() * 4;
				cbRequired += 4;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int	WriteToBs(AMBst bs){
			unsigned long len = 0;
			Unmap(NULL, &len);
			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, section_syntax_indicator);
			AMBst_PutBits(bs, 1, 0);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 12, len-3);
			AMBst_PutBits(bs, 16, transport_stream_id);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);

			for(size_t i=0;i<program_entries.size();i++){
				AMBst_PutBits(bs, 16, program_entries[i]->program_number);
				AMBst_ReservedBits(bs, 3);
				AMBst_PutBits(bs, 13, program_entries[i]->program_number == 0?program_entries[i]->network_PID:program_entries[i]->program_map_PID);
			}

			AMBst_PutDWord(bs, CRC_32);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER("table_id", 8, table_id, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER("section_syntax_indicator", 1, section_syntax_indicator, "The section_syntax_indicator is a 1-bit field which shall be set to '1'.")
			NAV_FIELD_PROP_2NUMBER("marker_bit", 1, marker_bit, "Should be '0'")
			NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER("section_length", 12, section_length, "the number of bytes of the section, starting immediately following the section_length field")
			NAV_FIELD_PROP_2NUMBER("transport_stream_id", 16, transport_stream_id, "a label to identify this transport stream from any other multiplex within a network")
			NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved_1, "")
			NAV_FIELD_PROP_2NUMBER("version_number", 5, version_number, "the version number of the whole program association table")
			NAV_FIELD_PROP_2NUMBER("current_next_indicator", 1, current_next_indicator, "indicates that the program association table sent is currently applicable, or be the next table to become valid")
			NAV_FIELD_PROP_2NUMBER("section_number", 8, section_number, "the number of this section")
			NAV_FIELD_PROP_2NUMBER("last_section_number", 8, last_section_number, "the number of the last section of the complete program association table.")

			for(i=0;i<(int)program_entries.size();i++){
				NAV_WRITE_TAG_BEGIN3("Program", i+1);
					NAV_FIELD_PROP_2NUMBER("program_number", 16, program_entries[i]->program_number, "")
					NAV_FIELD_PROP_2NUMBER("reserved", 3, program_entries[i]->reserved, "")
					if (program_entries[i]->program_number == 0){
						NAV_FIELD_PROP_2NUMBER("network_PID", 13, program_entries[i]->network_PID, "")
					}else{
						NAV_FIELD_PROP_2NUMBER("program_map_PID", 13, program_entries[i]->program_map_PID, "")
					}
				NAV_WRITE_TAG_END3("Program", i+1);
			}

			NAV_FIELD_PROP_2NUMBER("CRC", 32, CRC_32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the entire program association section")

		DECLARE_FIELDPROP_END()

	};

	struct CConditionalAccessSection: public ADVANCE_ENDIAN_MAP{

		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		marker_bit:1;
				unsigned short		reserved_0:2;
				unsigned short		section_length:12;
#else
				unsigned short		section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		marker_bit:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned char		reserved_1[2];
		
#ifdef _BIG_ENDIAN_
		unsigned char		reserved_2:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved_2:2;
#endif

		unsigned char		section_number;
		unsigned char		last_section_number;

		std::vector<C13818_1_Descriptor*>
							descriptors;

		unsigned long		CRC_32;

		virtual ~CConditionalAccessSection(){Unmap();}

		BOOL IsEqual(CConditionalAccessSection* pCAS){
			if (pCAS == NULL)
				return FALSE;

			if (pCAS->table_id != table_id ||
				pCAS->short_value_0 != short_value_0 ||
				pCAS->current_next_indicator != current_next_indicator ||
				pCAS->version_number != version_number ||
				pCAS->section_number != section_number ||
				pCAS->last_section_number != last_section_number ||
				pCAS->CRC_32 != CRC_32)
				return FALSE;

			if (pCAS->descriptors.size() != descriptors.size())
				return FALSE;

			return TRUE;
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 8);

			if (cbSize < (unsigned long)section_length + 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			cbSize = section_length + 3;
			while(ulMappedSize + 4 < cbSize){
				C13818_1_Descriptor* pDescriptor = NULL;
				MAP_MEM_TO_STRUCT_POINTER2(1, pDescriptor, C13818_1_Descriptor)
				descriptors.push_back(pDescriptor);
			}

			if (ulMappedSize + 4 > cbSize){
				Unmap();
				return RET_CODE_BUFFER_TOO_SMALL;
			}

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				for(size_t i=0;i<descriptors.size();i++){
					UNMAP_STRUCT_POINTER2(descriptors[i])
				}
				descriptors.clear();
			}
			else
			{
				unsigned long cbRequired = 8;
				for(size_t i=0;i<descriptors.size();i++){
					unsigned long descriptor_len = 0;
					if (descriptors[i]->Unmap(NULL, &descriptor_len) == RET_CODE_SUCCESS)
						cbRequired += descriptor_len;
				}

				cbRequired += 4;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			unsigned long len = 0;
			Unmap(NULL, &len);
			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, section_syntax_indicator);
			AMBst_PutBits(bs, 1, 0);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 12, len - 3);

			AMBst_ReservedBits(bs, 18);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);

			for(size_t i=0;i<descriptors.size();i++){
				if (descriptors[i]->WriteToBs(bs) != RET_CODE_SUCCESS)
					return RET_CODE_ERROR;
			}

			AMBst_PutDWord(bs, CRC_32);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, "The section_syntax_indicator is a 1-bit field which shall be set to '1'.")
			NAV_FIELD_PROP_2NUMBER1(marker_bit, 1, "Should be '0'")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(section_length, 12, "the number of bytes of the section, starting immediately following the section_length field")
			NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved", 16, reserved_1, 2,  "")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_2, "")
			NAV_FIELD_PROP_2NUMBER1(version_number, 5, "the version number of the entire conditional access table")
			NAV_FIELD_PROP_2NUMBER1(current_next_indicator, 1, current_next_indicator
					?"the conditional access table sent is currently applicable"
					:"the conditional access table sent is not yet applicable and shall be the next conditional access table to become valid")
			NAV_FIELD_PROP_2NUMBER1(section_number, 8, "the number of this section")
			NAV_FIELD_PROP_2NUMBER1(last_section_number, 8, "the number of the last section of the conditional access table.")

			for(i=0;i<(int)descriptors.size();i++){
				NAV_FIELD_PROP_REF(descriptors[i])
			}

			NAV_FIELD_PROP_2NUMBER("CRC", 32, CRC_32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the entire conditional access section")
		DECLARE_FIELDPROP_END()

	};

	struct CTSProgramMapSection: public ADVANCE_ENDIAN_MAP{

		struct CElementStreamInfo: public ADVANCE_ENDIAN_MAP {
			unsigned char		stream_type;

			union {
				struct {
#ifdef _BIG_ENDIAN_
					unsigned long		reserved_0:3;
					unsigned long		elementary_PID:13;
					unsigned long		reserved_1:4;
					unsigned long		ES_info_length:12;
#else
					unsigned long		ES_info_length:12;
					unsigned long		reserved_1:4;
					unsigned long		elementary_PID:13;
					unsigned long		reserved_0:3;
#endif
				}PACKED;
				unsigned long	long_value;
			}PACKED;

			std::vector<C13818_1_Descriptor*>	descriptors;
			CPSIContext*						ptr_ctx;

			CElementStreamInfo(CPSIContext* pCtx): ptr_ctx(pCtx){}
			virtual ~CElementStreamInfo(){Unmap();}

			DECLARE_ENDIAN_BEGIN()
				ULONG_FIELD_ENDIAN(long_value)
			DECLARE_ENDIAN_END()

			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
			{
				unsigned long ulMappedSize = 0;

				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;

				MAP_MEM_TO_HDR2(&stream_type, 5);

				if (cbSize < (unsigned long)ES_info_length + 5)
					return RET_CODE_BUFFER_TOO_SMALL;

				cbSize = ES_info_length + 5;
				ptr_ctx->loop_idx = 1;
				ptr_ctx->stream_type = stream_type;
				while(ulMappedSize < cbSize){
					C13818_1_Descriptor* pDescriptor = NULL;
					MAP_MEM_TO_STRUCT_POINTER2(1, pDescriptor, C13818_1_Descriptor, ptr_ctx)
					descriptors.push_back(pDescriptor);
				}

				AMP_SAFEASSIGN(desired_size, ulMappedSize);
				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
			{
				if (pcbSize == NULL)
				{
					for(size_t i=0;i<descriptors.size();i++){
						UNMAP_STRUCT_POINTER2(descriptors[i])
					}
					descriptors.clear();
				}
				else
				{
					unsigned long cbRequired = 5;
					for(size_t i=0;i<descriptors.size();i++){
						unsigned long descriptor_len = 0;
						if (descriptors[i]->Unmap(NULL, &descriptor_len) == RET_CODE_SUCCESS)
							cbRequired += descriptor_len;
					}
					UNMAP_GENERAL_UTILITY()
				}
				return RET_CODE_SUCCESS;
			}

			int WriteToBs(AMBst bs){
				AMBst_PutByte(bs, stream_type);
				AMBst_ReservedBits(bs, 3);
				AMBst_PutBits(bs, 13, elementary_PID);
				AMBst_ReservedBits(bs, 4);
				AMBst_PutBits(bs, 12, ES_info_length);

				for(size_t i=0;i<descriptors.size();i++){
					int org_pos = AMBst_Tell(bs);
					if (descriptors[i]->WriteToBs(bs) != RET_CODE_SUCCESS)
						return RET_CODE_ERROR;
					unsigned long ulActualSize = 0;
					descriptors[i]->Unmap(NULL, &ulActualSize);
					printf("[loop#2, descriptor#%zu] Expected write: %d bits, actual written to: %lu bits.\n", i, AMBst_Tell(bs) - org_pos, ulActualSize*8);
				}
				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(stream_type, 8, stream_type_names[stream_type])
				NAV_FIELD_PROP_2NUMBER ("reserved", 3, reserved_0, "")
				NAV_FIELD_PROP_2NUMBER1(elementary_PID, 13, "the PID of the transport stream packets which carry the associated program element")
				NAV_FIELD_PROP_2NUMBER ("reserved", 4, reserved_1, "")
				NAV_FIELD_PROP_2NUMBER1(ES_info_length, 12, "specify the number of bytes of the descriptors of the associated program element immediately following the ES_info_length field")

				for(i=0;i<(int)descriptors.size();i++){
					NAV_FIELD_PROP_REF(descriptors[i])
				}
			DECLARE_FIELDPROP_END()
		};

		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		marker_bit:1;
				unsigned short		reserved_0:2;
				unsigned short		section_length:12;
#else
				unsigned short		section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		marker_bit:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned short		program_number;
		
#ifdef _BIG_ENDIAN_
		unsigned char		reserved_1:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved_1:2;
#endif

		unsigned char		section_number;
		unsigned char		last_section_number;

		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		reserved_2:3;
				unsigned long		PCR_PID:13;
				unsigned long		reserved_3:4;
				unsigned long		program_info_length:12;
#else
				unsigned long		program_info_length:12;
				unsigned long		reserved_3:4;
				unsigned long		PCR_PID:13;
				unsigned long		reserved_2:3;
#endif
			}PACKED;
			unsigned long	long_value;
		}PACKED;

		std::vector<C13818_1_Descriptor*>	
							descriptors;
		std::vector<CElementStreamInfo*>
							elementary_stream_infos;

		unsigned long		CRC_32;
		CPSIContext*		ptr_ctx;

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
			USHORT_FIELD_ENDIAN(program_number)
			ULONG_FIELD_ENDIAN(long_value)
		DECLARE_ENDIAN_END()

		CTSProgramMapSection(CPSIContext* pCtx): ptr_ctx(pCtx){}
		virtual ~CTSProgramMapSection(){Unmap();}

		BOOL IsEqual(CTSProgramMapSection* pPMT){
			if (pPMT == NULL)
				return FALSE;

			if (pPMT->table_id != table_id ||
				pPMT->short_value_0 != short_value_0 ||
				pPMT->current_next_indicator != current_next_indicator ||
				pPMT->version_number != version_number ||
				pPMT->CRC_32 != CRC_32)
				return FALSE;

			if (pPMT->descriptors.size() != descriptors.size())
				return FALSE;

			if (pPMT->elementary_stream_infos.size() != elementary_stream_infos.size())
				return FALSE;

			return TRUE;
		}

		BOOL IsESChanged(CTSProgramMapSection* pPMT)
		{
			if (pPMT == NULL)
				return FALSE;

			if (pPMT->elementary_stream_infos.size() != elementary_stream_infos.size())
				return FALSE;

			for (int i = 0; i < (int)elementary_stream_infos.size(); i++)
			{
				int j = 0;
				for (; j < (int)pPMT->elementary_stream_infos.size(); j++)
				{
					if (elementary_stream_infos[i]->elementary_PID == pPMT->elementary_stream_infos[j]->elementary_PID &&
						elementary_stream_infos[i]->stream_type == pPMT->elementary_stream_infos[j]->stream_type)
						break;
				}

				if (j >= (int)pPMT->elementary_stream_infos.size())
					return FALSE;

				// stream PID and stream type is the same, check the video descriptor in advance
				// At first check registration descriptor.
				CRegistrationDescriptor* pRegistrationDesc = NULL;
				for (int k = 0; k < (int)elementary_stream_infos[i]->descriptors.size(); k++)
					if (elementary_stream_infos[i]->descriptors[k]->descriptor_tag == DT_registration_descriptor)
						pRegistrationDesc = elementary_stream_infos[i]->descriptors[k]->registration_descriptor;

				CRegistrationDescriptor* pRegistrationDesc1 = NULL;
				for (int k = 0; k < (int)pPMT->elementary_stream_infos[j]->descriptors.size(); k++)
					if (pPMT->elementary_stream_infos[j]->descriptors[k]->descriptor_tag == DT_registration_descriptor)
						pRegistrationDesc1 = pPMT->elementary_stream_infos[j]->descriptors[k]->registration_descriptor;

				if (pRegistrationDesc == NULL || pRegistrationDesc1 == NULL || pRegistrationDesc->descriptor_length != pRegistrationDesc1->descriptor_length)
					return FALSE;

				if (memcmp(pRegistrationDesc, pRegistrationDesc1, (size_t)pRegistrationDesc->descriptor_length + 1) != 0)
					return FALSE;
			}

			return TRUE;
		}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 12);

			if (cbSize < (unsigned long)section_length + 3 || cbSize < program_info_length + 12UL)
				return RET_CODE_BUFFER_TOO_SMALL;

			while(ulMappedSize < (unsigned long)program_info_length + 12){
				C13818_1_Descriptor* pDescriptor = NULL;
				ptr_ctx->loop_idx = 0;
				MAP_MEM_TO_STRUCT_POINTER2(1, pDescriptor, C13818_1_Descriptor, ptr_ctx)
				descriptors.push_back(pDescriptor);
			}

			cbSize = section_length + 3;
			while(ulMappedSize + 4 < cbSize){
				CElementStreamInfo* pStreamInfo = NULL;
				MAP_MEM_TO_STRUCT_POINTER2(1, pStreamInfo, CElementStreamInfo, ptr_ctx)
				elementary_stream_infos.push_back(pStreamInfo);
			}

			if (ulMappedSize + 4 > cbSize){
				Unmap();
				return RET_CODE_BUFFER_TOO_SMALL;
			}

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				for(size_t i=0;i<descriptors.size();i++){
					UNMAP_STRUCT_POINTER2(descriptors[i])
				}
				for(size_t i=0;i<elementary_stream_infos.size();i++){
					UNMAP_STRUCT_POINTER2(elementary_stream_infos[i])
				}
				descriptors.clear();
			}
			else
			{
				unsigned long cbRequired = 12;
				for(size_t i=0;i<descriptors.size();i++){
					unsigned long descriptor_len = 0;
					if (descriptors[i]->Unmap(NULL, &descriptor_len) == RET_CODE_SUCCESS)
						cbRequired += descriptor_len;
				}

				for(size_t i=0;i<elementary_stream_infos.size();i++){
					unsigned long stream_info_len = 0;
					if (elementary_stream_infos[i]->Unmap(NULL, &stream_info_len) == RET_CODE_SUCCESS)
						cbRequired += stream_info_len;
				}

				cbRequired += 4;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			unsigned long len = 0;
			Unmap(NULL, &len);
			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, section_syntax_indicator);
			AMBst_PutBits(bs, 1, 0);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 12, len - 3);
			AMBst_PutBits(bs, 16, program_number);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);
			AMBst_ReservedBits(bs, 3);
			AMBst_PutBits(bs, 13, PCR_PID);
			AMBst_ReservedBits(bs, 4);
			AMBst_PutBits(bs, 12, program_info_length);

			for(size_t i=0;i<descriptors.size();i++){
				int org_pos = AMBst_Tell(bs);
				if (descriptors[i]->WriteToBs(bs) != RET_CODE_SUCCESS)
					return RET_CODE_ERROR;
				unsigned long ulActualSize = 0;
				descriptors[i]->Unmap(NULL, &ulActualSize);
				printf("[loop#1, descriptor#%zu] Expected write: %d bits, actual written to: %lu bits.\n", i, AMBst_Tell(bs) - org_pos, ulActualSize*8);
			}

			for(size_t i=0;i<elementary_stream_infos.size();i++){
				int org_pos = AMBst_Tell(bs);
				if (elementary_stream_infos[i]->WriteToBs(bs) != RET_CODE_SUCCESS)
					return RET_CODE_ERROR;
				unsigned long ulActualSize = 0;
				elementary_stream_infos[i]->Unmap(NULL, &ulActualSize);
				printf("[element_stream_info#%zu] Expected write: %d bits, actual written to: %lu bits.\n", i, AMBst_Tell(bs) - org_pos, ulActualSize*8);
			}

			AMBst_PutDWord(bs, CRC_32);
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, "The section_syntax_indicator is a 1-bit field which shall be set to '1'.")
			NAV_FIELD_PROP_2NUMBER1(marker_bit, 1, "Should be '0'")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(section_length, 12, "the number of bytes of the section, starting immediately following the section_length field")
			NAV_FIELD_PROP_2NUMBER1(program_number, 16, "the program to which the program_map_PID is applicable")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_1, "")
			NAV_FIELD_PROP_2NUMBER1(version_number, 5, "the version number of the TS_program_map_section")
			NAV_FIELD_PROP_2NUMBER1(current_next_indicator, 1, current_next_indicator
					?"the TS_program_map_section sent is currently applicable"
					:"the TS_program_map_section sent is not yet applicable and shall be the next conditional access table to become valid")
			NAV_FIELD_PROP_2NUMBER1(section_number, 8, "the number of this section")
			NAV_FIELD_PROP_2NUMBER1(last_section_number, 8, "the number of the last section of the TS_program_map_section.")

			NAV_FIELD_PROP_2NUMBER ("reserved", 3, reserved_2, "")
			NAV_FIELD_PROP_2NUMBER1(PCR_PID, 13, "the PID of the transport stream packets which shall contain the PCR fields valid for the program specified by program_number")
			NAV_FIELD_PROP_2NUMBER ("reserved", 4, reserved_3, "")
			NAV_FIELD_PROP_2NUMBER1(program_info_length, 12, "the number of bytes of the descriptors immediately following the program_info_length field")

			for(i=0;i<(int)descriptors.size();i++){
				NAV_FIELD_PROP_REF(descriptors[i])
			}

			for(i=0;i<(int)elementary_stream_infos.size();i++){
				NAV_WRITE_TAG_BEGIN3("Elementary_Stream", (i + 1));
				NAV_FIELD_PROP_REF(elementary_stream_infos[i])
				NAV_WRITE_TAG_END3("Elementary_Stream", (i + 1));
			}

			NAV_FIELD_PROP_2NUMBER("CRC", 32, CRC_32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the TS_program_map_section")
		DECLARE_FIELDPROP_END()

	};

	typedef CShortGenericSection CShortPrivateSection;
	typedef  CLongGenericSection  CLongPrivateSection;

	struct CTSDescriptionSection: public ADVANCE_ENDIAN_MAP{

		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		marker_bit:1;
				unsigned short		reserved_0:2;
				unsigned short		section_length:12;
#else
				unsigned short		section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		marker_bit:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned char		reserved_1[2];
		
#ifdef _BIG_ENDIAN_
		unsigned char		reserved_2:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved_2:2;
#endif

		unsigned char		section_number;
		unsigned char		last_section_number;

		std::vector<C13818_1_Descriptor*>
							descriptors;

		unsigned long		CRC_32;

		virtual ~CTSDescriptionSection(){Unmap();}

		BOOL IsEqual(CTSDescriptionSection* pTSDS){
			if (pTSDS == NULL)
				return FALSE;

			if (pTSDS->table_id != table_id ||
				pTSDS->short_value_0 != short_value_0 ||
				pTSDS->current_next_indicator != current_next_indicator ||
				pTSDS->version_number != version_number ||
				pTSDS->section_number != section_number ||
				pTSDS->last_section_number != last_section_number ||
				pTSDS->CRC_32 != CRC_32)
				return FALSE;

			if (pTSDS->descriptors.size() != descriptors.size())
				return FALSE;

			return TRUE;
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 8);

			if (cbSize < (unsigned long)section_length + 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			cbSize = section_length + 3;
			while(ulMappedSize + 4 < cbSize){
				C13818_1_Descriptor* pDescriptor = NULL;
				MAP_MEM_TO_STRUCT_POINTER2(1, pDescriptor, C13818_1_Descriptor)
				descriptors.push_back(pDescriptor);
			}

			if (ulMappedSize + 4 > cbSize){
				Unmap();
				return RET_CODE_BUFFER_TOO_SMALL;
			}

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL){
				for(size_t i=0;i<descriptors.size();i++){
					UNMAP_STRUCT_POINTER2(descriptors[i])
				}
				descriptors.clear();
			}
			else
			{
				unsigned long cbRequired = 8;
				for(size_t i=0;i<descriptors.size();i++){
					unsigned long cbSize = 0;
					descriptors[i]->Unmap(NULL, &cbSize);
					cbRequired += cbSize;
				}
				cbRequired += 4;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			unsigned long len = 0;
			Unmap(NULL, &len);

			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, section_syntax_indicator);
			AMBst_PutBits(bs, 1, 0);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 12, len - 3);
			AMBst_ReservedBits(bs, 18);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);

			for(size_t i=0;i<descriptors.size();i++)
				if (descriptors[i]->WriteToBs(bs) != RET_CODE_SUCCESS)
					return RET_CODE_ERROR;

			AMBst_PutDWord(bs, CRC_32);
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, "The section_syntax_indicator is a 1-bit field which shall be set to '1'.")
			NAV_FIELD_PROP_2NUMBER1(marker_bit, 1, "Should be '0'")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(section_length, 12, "the number of bytes of the section, starting immediately following the section_length field")
			NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved", 16, reserved_1, 2,  "")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_2, "")
			NAV_FIELD_PROP_2NUMBER1(version_number, 5, "the version number of the entire transport stream description table")
			NAV_FIELD_PROP_2NUMBER1(current_next_indicator, 1, current_next_indicator
					?"the transport stream description table sent is currently applicable"
					:"the transport stream description table sent is not yet applicable and shall be the next transport stream description table to become valid")
			NAV_FIELD_PROP_2NUMBER1(section_number, 8, "the number of this section")
			NAV_FIELD_PROP_2NUMBER1(last_section_number, 8, "the number of the last section of the transport stream description table.")

			for(i=0;i<(int)descriptors.size();i++){
				NAV_FIELD_PROP_REF(descriptors[i])
			}

			NAV_FIELD_PROP_2NUMBER("CRC", 32, CRC_32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the entire transport stream description section")
		DECLARE_FIELDPROP_END()

	};

	struct CISOIEC14496Section: public ADVANCE_ENDIAN_MAP{
		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		private_indicator:1;
				unsigned short		reserved_0:2;
				unsigned short		ISO_IEC_14496_section_length:12;
#else
				unsigned short		ISO_IEC_14496_section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		private_indicator:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned short		table_id_extension;
		
#ifdef _BIG_ENDIAN_
		unsigned char		reserved_1:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved_1:2;
#endif

		unsigned char		section_number;
		unsigned char		last_section_number;

		unsigned char*		data;
		
		unsigned long		CRC_32;

		CISOIEC14496Section(){data = NULL;}
		virtual ~CISOIEC14496Section(){Unmap();}

		BOOL IsEqual(CISOIEC14496Section* pISOIEC14496Section){
			if (pISOIEC14496Section == NULL)
				return FALSE;

			if (pISOIEC14496Section->table_id != table_id ||
				pISOIEC14496Section->short_value_0 != short_value_0 ||
				pISOIEC14496Section->table_id_extension != table_id_extension ||
				pISOIEC14496Section->current_next_indicator != current_next_indicator ||
				pISOIEC14496Section->version_number != version_number ||
				pISOIEC14496Section->section_number != section_number ||
				pISOIEC14496Section->last_section_number != last_section_number ||
				pISOIEC14496Section->CRC_32 != CRC_32)
				return FALSE;

			return memcmp((void*)pISOIEC14496Section->data, (void*)data, ISO_IEC_14496_section_length - 9)==0?TRUE:FALSE;
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
			USHORT_FIELD_ENDIAN(table_id_extension)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 8);

			if (cbSize < (unsigned long)ISO_IEC_14496_section_length + 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			data = pBuf + ulMappedSize;
			ulMappedSize += ISO_IEC_14496_section_length - 9;

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				data = NULL;
			}
			else
			{
				unsigned long cbRequired = ISO_IEC_14496_section_length + 3;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, section_syntax_indicator);
			AMBst_PutBits(bs, 1, private_indicator);
			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 12, ISO_IEC_14496_section_length);
			AMBst_PutBits(bs, 16, table_id_extension);

			AMBst_ReservedBits(bs, 2);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);

			AMBst_PutBytes(bs, data, ISO_IEC_14496_section_length - 9);
			AMBst_PutDWord(bs, CRC_32);
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(i);
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, "The section_syntax_indicator is a 1-bit field which shall be set to '1'.")
			NAV_FIELD_PROP_2NUMBER1(private_indicator, 1, "This 1-bit field shall not be specified by this Specification")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(ISO_IEC_14496_section_length, 12, "the number of bytes of the section, starting immediately following the section_length field")
			NAV_FIELD_PROP_2NUMBER1(table_id_extension, 16, "")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_1, "")
			NAV_FIELD_PROP_2NUMBER1(version_number, 5, "the version number of the entire conditional access table")
			NAV_FIELD_PROP_2NUMBER1(current_next_indicator, 1, "This 1-bit field shall be set to 1")
			NAV_FIELD_PROP_2NUMBER1(section_number, 8, "the number of this section")
			NAV_FIELD_PROP_2NUMBER1(last_section_number, 8, "the number of the last section of the conditional access table.")

			NAV_FIELD_PROP_FIXSIZE_BINSTR1(data, (unsigned long)(ISO_IEC_14496_section_length - 9)*8, "")

			NAV_FIELD_PROP_2NUMBER("CRC", 32, CRC_32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the entire ISO_IEC_14496_section")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CMetadataSection: public ADVANCE_ENDIAN_MAP{
		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		private_indicator:1;
				unsigned short		random_access_indicator:1;
				unsigned short		decoder_config_flag:1;
				unsigned short		metadata_section_length:12;
#else
				unsigned short		metadata_section_length:12;
				unsigned short		decoder_config_flag:1;
				unsigned short		random_access_indicator:1;
				unsigned short		private_indicator:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;

		unsigned char		metadata_service_id;
		unsigned char		reserved_1;
		
#ifdef _BIG_ENDIAN_
		unsigned char		section_fragment_indication:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		section_fragment_indication:2;
#endif

		unsigned char		section_number;
		unsigned char		last_section_number;

		unsigned char*		metadata_byte;
		
		unsigned long		CRC_32;

		CMetadataSection(){metadata_byte = NULL;}
		virtual ~CMetadataSection(){Unmap();}

		BOOL IsEqual(CMetadataSection* pMetadataSection){
			if (pMetadataSection == NULL)
				return FALSE;

			if (pMetadataSection->table_id != table_id ||
				pMetadataSection->short_value_0 != short_value_0 ||
				pMetadataSection->metadata_service_id != metadata_service_id ||
				pMetadataSection->current_next_indicator != current_next_indicator ||
				pMetadataSection->version_number != version_number ||
				pMetadataSection->section_fragment_indication != section_fragment_indication ||
				pMetadataSection->section_number != section_number ||
				pMetadataSection->last_section_number != last_section_number ||
				pMetadataSection->CRC_32 != CRC_32)
				return FALSE;

			return memcmp((void*)pMetadataSection->metadata_byte, (void*)metadata_byte, metadata_section_length - 9)==0?TRUE:FALSE;
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value_0)
			ULONG_FIELD_ENDIAN(CRC_32)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&table_id, 8);

			if (cbSize < (unsigned long)metadata_section_length + 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			metadata_byte = pBuf + ulMappedSize;
			ulMappedSize += metadata_section_length - 9;

			CRC_32 = *((unsigned long*)(pBuf + ulMappedSize));
			ULONG_FIELD_ENDIAN(CRC_32)
			ulMappedSize += 4;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				metadata_byte = NULL;
			}
			else
			{
				unsigned long cbRequired = metadata_section_length + 3;
				UNMAP_GENERAL_UTILITY()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs){
			AMBst_PutBits(bs, 8, table_id);
			AMBst_PutBits(bs, 1, section_syntax_indicator);
			AMBst_PutBits(bs, 1, private_indicator);
			AMBst_PutBits(bs, 1, random_access_indicator);
			AMBst_PutBits(bs, 1, decoder_config_flag);
			AMBst_PutBits(bs, 12, metadata_section_length);
			
			AMBst_PutBits(bs, 8, metadata_service_id);
			AMBst_ReservedBits(bs, 8);

			AMBst_PutBits(bs, 2, section_fragment_indication);
			AMBst_PutBits(bs, 5, version_number);
			AMBst_PutBits(bs, 1, current_next_indicator);
			AMBst_PutBits(bs, 8, section_number);
			AMBst_PutBits(bs, 8, last_section_number);

			AMBst_PutBytes(bs, metadata_byte, metadata_section_length - 9);
			AMBst_PutDWord(bs, CRC_32);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(i);
			NAV_FIELD_PROP_2NUMBER1(table_id, 8, table_id_names[table_id])
			NAV_FIELD_PROP_2NUMBER1(section_syntax_indicator, 1, "The section_syntax_indicator is a 1-bit field which shall be set to '1'.")
			NAV_FIELD_PROP_2NUMBER1(private_indicator, 1, "This 1-bit field shall not be specified by this Specification")
			NAV_FIELD_PROP_2NUMBER1(random_access_indicator, 1, random_access_indicator?"the metadata carried in this metadata section represents an access point to the metadata service where decoding is possible without information from previous metadata sections. The meaning of a random access point is defined by the format of the metadata":"")
			NAV_FIELD_PROP_2NUMBER1(decoder_config_flag, 1, decoder_config_flag?"decoder configuration information is present in the metadata Access Unit":"")
			NAV_FIELD_PROP_2NUMBER1(metadata_section_length, 12, "the number of bytes of the section, starting immediately following the section_length field")
			NAV_FIELD_PROP_2NUMBER1(metadata_service_id, 8, "the metadata service associated with the metadata Access Unit carried in this metadata section")
			NAV_FIELD_PROP_2NUMBER ("reserved", 8, reserved_1, "")
			NAV_FIELD_PROP_2NUMBER1(section_fragment_indication, 2, section_fragment_indication==0?"A metadata section from a series of metadata sections with data from one metadata Access Unit, but neither the first nor the last one"
				:(section_fragment_indication==1?"The last metadata section from a series of metadata sections with data from one metadata Access Unit."
				:(section_fragment_indication==2?"The first metadata section from a series of metadata sections with data from one metadata Access Unit"
				:"A single metadata section carrying a complete metadata Access Unit")))
			NAV_FIELD_PROP_2NUMBER1(version_number, 5, "the version number of the entire conditional access table")
			NAV_FIELD_PROP_2NUMBER1(current_next_indicator, 1, current_next_indicator?"the Metadata Table sent is currently applicable":"the Metadata Table sent is not yet applicable and shall be the next Metadata Table to become valid")
			NAV_FIELD_PROP_2NUMBER1(section_number, 8, "the number of this section")
			NAV_FIELD_PROP_2NUMBER1(last_section_number, 8, "the number of the last section of the conditional access table.")

			NAV_FIELD_PROP_FIXSIZE_BINSTR1(metadata_byte, (unsigned long)(metadata_section_length - 9)*8, "contiguous bytes from a metadata Access Unit.")

			NAV_FIELD_PROP_2NUMBER("CRC", 32, CRC_32, "contains the CRC value that gives a zero output of the registers in the decoder defined in Annex A after processing the entire metadata_section")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CLongSectionHeader{
		unsigned char		table_id;
		union{
			struct{
#ifdef _BIG_ENDIAN_
				unsigned short		section_syntax_indicator:1;
				unsigned short		private_indicator:1;
				unsigned short		reserved_0:2;
				unsigned short		private_section_length:12;
#else
				unsigned short		private_section_length:12;
				unsigned short		reserved_0:2;
				unsigned short		private_indicator:1;
				unsigned short		section_syntax_indicator:1;
#endif
			}PACKED;
			unsigned short	short_value_0;
		}PACKED;
		
		unsigned short		table_id_extension;

#ifdef _BIG_ENDIAN_
		unsigned char		reserved:2;
		unsigned char		version_number:5;
		unsigned char		current_next_indicator:1;
#else
		unsigned char		current_next_indicator:1;
		unsigned char		version_number:5;
		unsigned char		reserved:2;
#endif
		unsigned char		section_number;
		unsigned char		last_section_number;
	}PACKED;

	struct CPSISection: public MEM_MAP_MGR {
		unsigned char		pointer_field;
		unsigned char		table_id;
		unsigned char		section_syntax_indicator;
		unsigned char		align_0[5];
		union {
			void*							pSection = nullptr;
			CProgramAssociationSection*		program_association_section;
			CConditionalAccessSection*		conditional_access_section;
			CTSProgramMapSection*			program_map_section;
			CLongPrivateSection*			long_private_section;
			CShortPrivateSection*			short_private_section;
			CTSDescriptionSection*			ts_description_section;
			CISOIEC14496Section*			ISO_IEC_14496_scene_description_section;
			CISOIEC14496Section*			ISO_IEC_14496_object_descriptor_section;
			CMetadataSection*				Metadata_section;
			CShortGenericSection*			short_section_object;
			CLongGenericSection*			long_section_object;
		}PACKED;
		CPSIContext*			psi_ctx;

		CPSISection(TS_FORMAT ts_format=TSF_GENERIC): MEM_MAP_MGR(-1, NULL), pointer_field(0), table_id(0xFF), section_syntax_indicator(0), align_0{0}, psi_ctx(NULL){
			psi_ctx = new CPSIContext;
			if (psi_ctx != nullptr){
				psi_ctx->ts_format = ts_format;
				psi_ctx->ptr_section = this;
				psi_ctx->loop_idx = 0xFF;
			}
		}
		~CPSISection(){Unmap(); AMP_SAFEDELA2(psi_ctx);}

		BOOL IsEqual(CPSISection* pPSISection){
			if (pPSISection == NULL || pPSISection->table_id != table_id)
				return FALSE;

			switch(table_id){
			case TID_program_association_section: return pPSISection->program_association_section->IsEqual(program_association_section);
			case TID_conditional_access_section: return pPSISection->conditional_access_section->IsEqual(conditional_access_section);
			case TID_TS_program_map_section: return pPSISection->program_map_section->IsEqual(program_map_section);
			case TID_TS_description_section: return pPSISection->ts_description_section->IsEqual(ts_description_section);
			case TID_ISO_IEC_14496_scene_description_section: return pPSISection->ISO_IEC_14496_scene_description_section->IsEqual(ISO_IEC_14496_scene_description_section);
			case TID_ISO_IEC_14496_object_descriptor_section: return pPSISection->ISO_IEC_14496_object_descriptor_section->IsEqual(ISO_IEC_14496_object_descriptor_section);
			case TID_Metadata_section: return pPSISection->Metadata_section->IsEqual(Metadata_section);
			case TID_IPMP_Control_Information_section: return pPSISection->long_section_object->IsEqual(long_section_object);
			default:
				if (section_syntax_indicator){
					//NAV_FIELD_PROP_REF1(long_private_section)
					return pPSISection->long_private_section->IsEqual(long_private_section);
				}else{
					//NAV_FIELD_PROP_REF1(short_private_section)
					return pPSISection->short_private_section->IsEqual(short_private_section);
				}
			}

			return FALSE;
		}

		CLongSectionHeader* GetGenericHeader(){
			switch(table_id){
			case TID_program_association_section: return (CLongSectionHeader*)&program_association_section->table_id;
			case TID_conditional_access_section: return (CLongSectionHeader*)&conditional_access_section->table_id;
			case TID_TS_program_map_section: return (CLongSectionHeader*)&program_map_section->table_id;
			case TID_TS_description_section: return (CLongSectionHeader*)&ts_description_section->table_id;
			case TID_ISO_IEC_14496_scene_description_section: return (CLongSectionHeader*)&ISO_IEC_14496_scene_description_section->table_id;
			case TID_ISO_IEC_14496_object_descriptor_section: return (CLongSectionHeader*)&ISO_IEC_14496_object_descriptor_section->table_id;
			case TID_Metadata_section: return (CLongSectionHeader*)&Metadata_section->table_id;
			case 0xFF: return NULL;
			default:
				if (section_syntax_indicator)
					return (CLongSectionHeader*)&long_section_object->table_id;
				else
					return NULL;
			}
		}

		unsigned short GetSectionLength(){
			switch(table_id){
			case TID_program_association_section: return program_association_section->section_length + 3;
			case TID_conditional_access_section: return conditional_access_section->section_length + 3;
			case TID_TS_program_map_section: return program_map_section->section_length + 3;
			case TID_TS_description_section: return ts_description_section->section_length + 3;
			case TID_ISO_IEC_14496_scene_description_section: return ISO_IEC_14496_scene_description_section->ISO_IEC_14496_section_length + 3;
			case TID_ISO_IEC_14496_object_descriptor_section: return ISO_IEC_14496_object_descriptor_section->ISO_IEC_14496_section_length + 3;
			case TID_Metadata_section: return Metadata_section->metadata_section_length + 3;
			//case 0xFF: return NULL;
			default:
				if (section_syntax_indicator)
					return long_section_object->private_section_length + 3;
				else
					return short_section_object->section_length + 3;
			}
		}

		int Map(unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;
			unsigned char* pBuf = m_binData;
			unsigned long  cbSize = m_cbDataSize; 

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 1)
				return RET_CODE_BUFFER_TOO_SMALL;

			pointer_field = *pBuf;
			ulMappedSize++;
			ulMappedSize += pointer_field;

			if (ulMappedSize < cbSize)
				table_id = pBuf[ulMappedSize];
			else
				table_id = 0xFF;

			if (ulMappedSize + 3 > cbSize)
				return RET_CODE_BUFFER_TOO_SMALL;

			unsigned short section_length = (pBuf[ulMappedSize+1]<<8|pBuf[ulMappedSize+2])&0XFFF;
			// The maximum number of bytes in a section of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 defined PSI table is
			// 1024 bytes. The maximum number of bytes in a private_section is 4096 bytes.
			// The DSMCC section data is also 4096 (table_id from 0x38 to 0x3F)
			if (section_length > (((pBuf[ulMappedSize]>=0x40 && pBuf[ulMappedSize]<=0xFE) ||
								   (pBuf[ulMappedSize]>=0x38 && pBuf[ulMappedSize]<=0x3F))?4093:1021))
				return RET_CODE_BUFFER_NOT_COMPATIBLE;

			if (ulMappedSize + 3 + section_length > cbSize)
				return RET_CODE_BUFFER_TOO_SMALL;

			if ((pBuf[ulMappedSize+1]>>7)&0x01){
				F_CRC_InicializaTable();
				if (F_CRC_CalculaCheckSum(pBuf + ulMappedSize, (size_t)section_length + 3) != 0){
					printf("[13818-1] current PSI section failed do check-sum.\n");
					return RET_CODE_ERROR_CRC;
				}
			}

			section_syntax_indicator = (pBuf[ulMappedSize+1]>>7)&0x01;
			if((pBuf[ulMappedSize] == TID_program_association_section ||
				pBuf[ulMappedSize] == TID_conditional_access_section ||
				pBuf[ulMappedSize] == TID_TS_program_map_section ||
				pBuf[ulMappedSize] == TID_TS_description_section ||
				pBuf[ulMappedSize] == TID_ISO_IEC_14496_scene_description_section ||
				pBuf[ulMappedSize] == TID_ISO_IEC_14496_object_descriptor_section ||
				pBuf[ulMappedSize] == TID_Metadata_section) && !section_syntax_indicator)
					return RET_CODE_BUFFER_NOT_COMPATIBLE;

			switch(table_id){
			case TID_program_association_section: MAP_MEM_TO_STRUCT_POINTER2(1, program_association_section, CProgramAssociationSection); break;
			case TID_conditional_access_section: MAP_MEM_TO_STRUCT_POINTER2(1, conditional_access_section, CConditionalAccessSection); break;
			case TID_TS_program_map_section: MAP_MEM_TO_STRUCT_POINTER2(1, program_map_section, CTSProgramMapSection, psi_ctx); break;
			case TID_TS_description_section: MAP_MEM_TO_STRUCT_POINTER2(1, ts_description_section, CTSDescriptionSection); break;
			case TID_ISO_IEC_14496_scene_description_section: MAP_MEM_TO_STRUCT_POINTER2(1, ISO_IEC_14496_scene_description_section, CISOIEC14496Section); break;
			case TID_ISO_IEC_14496_object_descriptor_section: MAP_MEM_TO_STRUCT_POINTER2(1, ISO_IEC_14496_object_descriptor_section, CISOIEC14496Section); break;
			case TID_Metadata_section: MAP_MEM_TO_STRUCT_POINTER2(1, Metadata_section, CMetadataSection); break;
			case 0xFF: return RET_CODE_BUFFER_NOT_COMPATIBLE;
			default:
				if (table_id >= 0x40 && table_id <= 0xFE){
					if (section_syntax_indicator){
						MAP_MEM_TO_STRUCT_POINTER2(1, long_private_section, CLongPrivateSection)
					}else{
						MAP_MEM_TO_STRUCT_POINTER (1, short_private_section, CShortPrivateSection)
					}
				}else{
					if (section_syntax_indicator){
						MAP_MEM_TO_STRUCT_POINTER2(1, long_section_object, CLongGenericSection)
					}else{
						MAP_MEM_TO_STRUCT_POINTER (1, short_section_object, CShortGenericSection)
					}
				}
			}
			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL || pBuf == NULL)
			{
				switch(table_id){
				case TID_program_association_section: UNMAP_STRUCT_POINTER2_1(program_association_section); break;
				case TID_conditional_access_section: UNMAP_STRUCT_POINTER2_1(conditional_access_section); break;
				case TID_TS_program_map_section: UNMAP_STRUCT_POINTER2_1(program_map_section); break;
				case TID_TS_description_section: UNMAP_STRUCT_POINTER2_1(ts_description_section); break;
				case TID_ISO_IEC_14496_scene_description_section: UNMAP_STRUCT_POINTER2_1(ISO_IEC_14496_scene_description_section); break;
				case TID_ISO_IEC_14496_object_descriptor_section: UNMAP_STRUCT_POINTER2_1(ISO_IEC_14496_object_descriptor_section); break;
				case TID_Metadata_section: UNMAP_STRUCT_POINTER2_1(Metadata_section); break;
				default:
					if (table_id >= 0x40 && table_id <= 0xFE){
						if (section_syntax_indicator){
							UNMAP_STRUCT_POINTER2_1(long_private_section)
						}else{
							UNMAP_STRUCT_POINTER0_1(short_private_section, CShortPrivateSection)
						}
					}else{
						if (section_syntax_indicator){
							UNMAP_STRUCT_POINTER2_1(long_section_object)
						}else{
							UNMAP_STRUCT_POINTER0_1(short_section_object, CShortGenericSection)
						}
					}
				}

				if (pcbSize)
					*pcbSize += pointer_field + sizeof(pointer_field);
			}
			else
			{
				UNMAP_GENERAL_UTILITY_2()
			}

			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, pointer_field);
			for(int i=0;i<pointer_field;i++)
				AMBst_PutByte(bs, 0);
			switch(table_id){
			case TID_program_association_section: program_association_section->WriteToBs(bs); break;
			case TID_conditional_access_section: conditional_access_section->WriteToBs(bs); break;
			case TID_TS_program_map_section: program_map_section->WriteToBs(bs); break;
			case TID_TS_description_section: ts_description_section->WriteToBs(bs); break;
			case TID_ISO_IEC_14496_scene_description_section: ISO_IEC_14496_scene_description_section->WriteToBs(bs); break;
			case TID_ISO_IEC_14496_object_descriptor_section: ISO_IEC_14496_object_descriptor_section->WriteToBs(bs); break;
			case TID_Metadata_section: Metadata_section->WriteToBs(bs); break;
			default:
				if (table_id >= 0x40 && table_id <= 0xFE){
					if (section_syntax_indicator){
						long_private_section->WriteToBs(bs);
					}else{
						UNMAP_DUPLICATE_STRUCT_POINTER(short_private_section, CShortPrivateSection);
					}
				}else{
					if (section_syntax_indicator){
						long_section_object->WriteToBs(bs);
					}else{
						UNMAP_DUPLICATE_STRUCT_POINTER(short_section_object, CShortGenericSection);
					}
				}
			}
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(i);
			NAV_FIELD_PROP_2NUMBER1(pointer_field, 8, "")
			if (bit_offset)
				*bit_offset += (long long)pointer_field*8;
			switch(table_id){
			case TID_program_association_section: NAV_FIELD_PROP_REF1(program_association_section); break;
			case TID_conditional_access_section: NAV_FIELD_PROP_REF1(conditional_access_section); break;
			case TID_TS_program_map_section: NAV_FIELD_PROP_REF1(program_map_section); break;
			case TID_TS_description_section: NAV_FIELD_PROP_REF1(ts_description_section); break;
			case TID_ISO_IEC_14496_scene_description_section: NAV_FIELD_PROP_REF1(ISO_IEC_14496_scene_description_section); break;
			case TID_ISO_IEC_14496_object_descriptor_section: NAV_FIELD_PROP_REF1(ISO_IEC_14496_object_descriptor_section); break;
			case TID_Metadata_section: NAV_FIELD_PROP_REF1(Metadata_section); break;
			case TID_IPMP_Control_Information_section:
				NAV_WRITE_TAG_BEGIN2("IPMP_Control_Information_section");
					NAV_FIELD_PROP_REF(long_section_object)
				NAV_WRITE_TAG_END2("IPMP_Control_Information_section");
				break;
			default:
				if (table_id >= 0x40 && table_id <= 0xFE){
					if (section_syntax_indicator){
						NAV_FIELD_PROP_REF1(long_private_section);
					}else{
						NAV_FIELD_PROP_REF1(short_private_section);
					}
				}else{
					NAV_WRITE_TAG_BEGIN2("Unknown_Section");
					if (section_syntax_indicator){
						NAV_FIELD_PROP_REF(long_section_object);
					}else{
						NAV_FIELD_PROP_REF(short_section_object);
					}
					NAV_WRITE_TAG_END2("Unknown_Section");
				}
			}
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CPSITable {
		unsigned char	last_section_number;
		unsigned char	version_number;
		CPSISection*	sections[256];

		CPSITable(): last_section_number(0), version_number(0){
			memset(sections, 0, sizeof(sections));
		}

		~CPSITable(){
			for(size_t i=0;i<_countof(sections);i++){
				AMP_SAFEDEL2(sections[i]);
			}
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_WRITE_TAG_BEGIN2("PSI");
			if (last_section_number == 0)
			{
				NAV_FIELD_PROP_REF(sections[0]);
			}
			else
			{
				for(i=0;i<=last_section_number;i++){
					if (sections[i] == NULL)
						continue;

					switch(sections[i]->table_id){
					case TID_program_association_section: NAV_FIELD_PROP_REF3(sections[i], "PAT", i); break;
					case TID_conditional_access_section: NAV_FIELD_PROP_REF3(sections[i], "CAT", i); break;
					case TID_TS_program_map_section: NAV_FIELD_PROP_REF3(sections[i], "PMT", i); break;
					case TID_TS_description_section: NAV_FIELD_PROP_REF3(sections[i], "TSDT", i); break;
					case TID_ISO_IEC_14496_scene_description_section: NAV_FIELD_PROP_REF3(sections[i], "ISO_IEC_14496_scene_description_section", i); break;
					case TID_ISO_IEC_14496_object_descriptor_section: NAV_FIELD_PROP_REF3(sections[i], "ISO_IEC_14496_object_descriptor_section", i); break;
					case TID_Metadata_section: NAV_FIELD_PROP_REF3(sections[i], "Metadata_section", i); break;
					case TID_IPMP_Control_Information_section: NAV_FIELD_PROP_REF3(sections[i], "IPMP_Control_Information_section", i); break;
					default:
						if (sections[i]->table_id >= 0x40 && sections[i]->table_id <= 0xFE){
							if (sections[i]->section_syntax_indicator){
								NAV_FIELD_PROP_REF3(sections[i], "long_private_section", i);
							}else{
								NAV_FIELD_PROP_REF3(sections[i], "short_private_section", i);
							}
						}else{
							if (sections[i]->section_syntax_indicator){
								NAV_FIELD_PROP_REF3(sections[i], "Unknown_section", i);
							}else{
								NAV_FIELD_PROP_REF3(sections[i], "Unknown_section", i);
							}
						}
					}	// end for switch(sections[i]....
				}	// end for(i=0....
			}
			NAV_WRITE_TAG_END2("PSI");
		DECLARE_FIELDPROP_END()
	};

	typedef CPSITable	CPAT;
	typedef CPSITable	CCAT;
	typedef CPSITable	CTDST;

	struct CTransportPacket: public ADVANCE_ENDIAN_MAP{

		CTSPacketHeader*	packet_header;
		CAdaptationField*	adaptation_field;
		unsigned char*		data_byte;

		CTransportPacket():packet_header(NULL), adaptation_field(NULL), data_byte(NULL){}
		virtual ~CTransportPacket(){Unmap();}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_STRUCT_POINTER(1, packet_header, CTSPacketHeader)

			if (packet_header->adaptation_field_control == 0x02 || packet_header->adaptation_field_control == 0x03){
				MAP_MEM_TO_STRUCT_POINTER2(1, adaptation_field, CAdaptationField)
			}

			if (packet_header->adaptation_field_control == 0x01 || packet_header->adaptation_field_control == 0x03) {
				data_byte = pBuf + ulMappedSize;
			}
			else
				data_byte = pBuf + ulMappedSize;

			ulMappedSize = 188;
			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			UNMAP_STRUCT_POINTER2(adaptation_field)
			UNMAP_STRUCT_POINTER(packet_header)
			return RET_CODE_SUCCESS;
		}

		int GetLengthOfDataBytes(){
			return 188 - 4 - (adaptation_field?(adaptation_field->adaptation_field_length+1):0);
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_REF(packet_header)
			NAV_FIELD_PROP_REF(adaptation_field)
			int data_len = GetLengthOfDataBytes();
			if (data_len > 0){
				NAV_FIELD_PROP_FIXSIZE_BINSTR("data_byte", ((long long)data_len*8), data_byte, (unsigned long)data_len, "")
			}
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CVarTSPacket: public MEM_MAP_MGR
	{
		MPEG_SYSTEM_TYPE		mpeg_system_type;
		int64_t					ts_pack_idx = -1;
		union
		{
			CTPExtraHeader*		BD_extra_header;
			CTTSExtraHeader*	TTS_extra_header;
		}PACKED;
		union
		{
			CTransportPacket*	packet;
			unsigned char*		packet_buf;
		}PACKED;
		unsigned char*			FEC;

		CVarTSPacket(MPEG_SYSTEM_TYPE sys_type, int64_t idxTSPack = -1, IAMMemMgr* pMemMgr = NULL)
			: MEM_MAP_MGR(-1, pMemMgr)
			, mpeg_system_type(sys_type)
			, ts_pack_idx(idxTSPack)
			, BD_extra_header(NULL)
			, packet(NULL)
			, FEC(NULL){
		}

		virtual ~CVarTSPacket(){
			Unmap();
		}

		int Map(unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long  ulMappedSize = 0;
			unsigned char* pBuf = m_binData;
			unsigned long  cbSize = m_cbDataSize; 

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			BOOL bEncrypted = FALSE;
			if (mpeg_system_type == MPEG_SYSTEM_BDMV || mpeg_system_type == MPEG_SYSTEM_BDAV){
				MAP_MEM_TO_STRUCT_POINTER(1, BD_extra_header, CTPExtraHeader)
				bEncrypted = BD_extra_header->Copy_permission_indicator != 0?TRUE:FALSE;
			}else if(mpeg_system_type == MPEG_SYSTEM_TTS){
				MAP_MEM_TO_STRUCT_POINTER(1, TTS_extra_header, CTTSExtraHeader)
			}

			if(!bEncrypted){
				MAP_MEM_TO_STRUCT_POINTER2(1, packet, CTransportPacket)
			} else {
				packet_buf = pBuf + ulMappedSize;
			}

			if (mpeg_system_type == MPEG_SYSTEM_TS204 || mpeg_system_type == MPEG_SYSTEM_TS208){
				FEC = pBuf + ulMappedSize;
				ulMappedSize += mpeg_system_type == MPEG_SYSTEM_TS204?16:20;
			}

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			FEC = NULL;
			if (mpeg_system_type == MPEG_SYSTEM_BDMV || mpeg_system_type == MPEG_SYSTEM_BDAV){
				BOOL bEncrypted = BD_extra_header->Copy_permission_indicator != 0?TRUE:FALSE;
				if(!bEncrypted){
					UNMAP_STRUCT_POINTER2(packet)
				}
				else
					packet_buf = NULL;
				UNMAP_STRUCT_POINTER(BD_extra_header)
			} else {
				UNMAP_STRUCT_POINTER2(packet)
				UNMAP_STRUCT_POINTER(TTS_extra_header)
			}
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			if (ts_pack_idx >= 0) {
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("transport_packet", "transport_packet#%" PRId64 "", "", ts_pack_idx);
			} else {
				NAV_WRITE_TAG_BEGIN2("transport_packet");
			}
			BOOL bEncrypted = FALSE;
			if (mpeg_system_type == MPEG_SYSTEM_BDMV || mpeg_system_type == MPEG_SYSTEM_BDAV){
				NAV_FIELD_PROP_REF(BD_extra_header)
				bEncrypted = BD_extra_header && BD_extra_header->Copy_permission_indicator != 0?TRUE:FALSE;
			} else {
				NAV_FIELD_PROP_REF(TTS_extra_header)
			}
			if(!bEncrypted){
				NAV_FIELD_PROP_REF(packet)
			} else {
				NAV_FIELD_PROP_FIXSIZE_BINSTR("ts_packet_data", 188*8, packet_buf, 188, "")
			}
			if (mpeg_system_type == MPEG_SYSTEM_TS204 || mpeg_system_type == MPEG_SYSTEM_TS208){
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(FEC, (mpeg_system_type == MPEG_SYSTEM_TS204?16UL:20UL)*8UL, "forward error correction")
			}
			NAV_WRITE_TAG_END("transport_packet");
		DECLARE_FIELDPROP_END()

	}PACKED;

}	//namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
