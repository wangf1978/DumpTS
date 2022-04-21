/*

MIT License

Copyright (c) 2022 Ravin.Wang(wangf1978@hotmail.com)

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

enum AVC_NAL_UNIT_TYPE
{
	AVC_CS_NON_IDR_PIC	= 1,	// Coded slice of a non-IDR picture
	AVC_CSD_PARTITION_A = 2,	// Coded slice data partition A
	AVC_CSD_PARTITION_B = 3,	// Coded slice data partition B
	AVC_CSD_PARTITION_C = 4,	// Coded slice data partition C
	AVC_CS_IDR_PIC		= 5,	// Coded slice of an IDR picture
	AVC_SEI_NUT			= 6,	// Supplemental enhancement information
	AVC_SPS_NUT			= 7,	// Sequence	parameter set
	AVC_PPS_NUT			= 8,	// Picture parameter set
	AVC_AUD_NUT			= 9,	// Access unit delimiter
	AVC_EOS_NUT			= 10,	// End of Sequence
	AVC_EOB_NUT			= 11,	// End of stream
	AVC_FD_NUT			= 12,	// Filler data
	AVC_SPS_EXT_NUT		= 13,	// Sequence parameter set extension
	AVC_PREFIX_NUT		= 14,	// Prefix NAL unit
	AVC_SUB_SPS_NUT		= 15,	// Subset sequence parameter set
	AVC_DPS_NUT			= 16,	// Depth parameter set
	AVC_SL_WO_PARTITION	= 19,	// Coded slice of an auxiliary coded picture without partitioning
	AVC_SL_EXT			= 20,	// Coded slice extension
	AVC_SL_EXT_DVIEW	= 21,	// Coded slice extension for depth view components
};

enum HEVC_NAL_UNIT_TYPE
{
	HEVC_TRAIL_N		= 0,
	HEVC_TRAIL_R		= 1,
	HEVC_TSA_N			= 2,
	HEVC_TSA_R			= 3,
	HEVC_STSA_N			= 4,
	HEVC_STSA_R			= 5,
	HEVC_RADL_N			= 6,
	HEVC_RADL_R			= 7,
	HEVC_RASL_N			= 8,
	HEVC_RASL_R			= 9,
	HEVC_RSV_VCL_N10	= 10,
	HEVC_RSV_VCL_N12	= 12,
	HEVC_RSV_VCL_N14	= 14,
	HEVC_RSV_VCL_R11	= 11,
	HEVC_RSV_VCL_R13	= 13,
	HEVC_RSV_VCL_R15	= 15,
	HEVC_BLA_W_LP		= 16,
	HEVC_BLA_W_RADL		= 17,
	HEVC_BLA_N_LP		= 18,
	HEVC_IDR_W_RADL		= 19,
	HEVC_IDR_N_LP		= 20,
	HEVC_CRA_NUT		= 21,
	HEVC_RSV_IRAP_VCL22	= 22,
	HEVC_RSV_IRAP_VCL23	= 23,
	HEVC_RSV_VCL24		= 24,
	HEVC_RSV_VCL31		= 31,
	HEVC_VPS_NUT		= 32,
	HEVC_SPS_NUT		= 33,
	HEVC_PPS_NUT		= 34,
	HEVC_AUD_NUT		= 35,
	HEVC_EOS_NUT		= 36,
	HEVC_EOB_NUT		= 37,
	HEVC_FD_NUT			= 38,
	HEVC_PREFIX_SEI_NUT	= 39,
	HEVC_SUFFIX_SEI_NUT	= 40,
	HEVC_RSV_NVCL41		= 41,
	HEVC_RSV_NVCL47		= 47,
	HEVC_UNSPEC48		= 48,
	HEVC_UNSPEC63		= 63,
};

enum VVC_NAL_UNIT_TYPE
{
	VVC_TRAIL_NUT		= 0,
	VVC_STSA_NUT		= 1,
	VVC_RADL_NUT		= 2,
	VVC_RASL_NUT		= 3,
	VVC_RSV_VCL_4		= 4,
	VVC_RSV_VCL_5		= 5,
	VVC_RSV_VCL_6		= 6,
	VVC_IDR_W_RADL		= 7,
	VVC_IDR_N_LP		= 8,
	VVC_CRA_NUT			= 9,
	VVC_GDR_NUT			= 10,
	VVC_RSV_IRAP_11		= 11,
	VVC_OPI_NUT			= 12,
	VVC_DCI_NUT			= 13,
	VVC_VPS_NUT			= 14,
	VVC_SPS_NUT			= 15,
	VVC_PPS_NUT			= 16,
	VVC_PREFIX_APS_NUT	= 17,
	VVC_SUFFIX_APS_NUT	= 18,
	VVC_PH_NUT			= 19,
	VVC_AUD_NUT			= 20,
	VVC_EOS_NUT			= 21,
	VVC_EOB_NUT			= 22,
	VVC_PREFIX_SEI_NUT	= 23,
	VVC_SUFFIX_SEI_NUT	= 24,
	VVC_FD_NUT			= 25,
	VVC_RSV_NVCL_26		= 26,
	VVC_RSV_NVCL_27		= 27,
	VVC_UNSPEC_28		= 28,
	VVC_UNSPEC_29		= 29,
	VVC_UNSPEC_30		= 30,
	VVC_UNSPEC_31		= 31,
};

enum HEVC_PICTURE_TYPE
{
	PIC_TYPE_UNKNOWN = 0,	// None of the below picture type
	PIC_TYPE_IDR = 1,		// All units in picture unit are IDR unit
	PIC_TYPE_CRA,			// All units in picture unit are IDR unit
	PIC_TYPE_IRAP,			// The nal_unit_type of each unit in picture unit is in the range of BLA_W_LP to RSV_IRAP_VCL23, inclusive
	PIC_TYPE_LEADING,		// Leading picture, it is a RADL or RASL		
	PIC_TYPE_TRAILING,		// Trailing picture, it is a TRAIL, TSA or STSA picture
};

#define HEVC_PICTURE_TYPE_NAMEA(t)	(\
	(t) == PIC_TYPE_IDR?"IDR":(\
	(t) == PIC_TYPE_CRA?"CRA" : (\
	(t) == PIC_TYPE_IRAP?"IRAP":(\
	(t) == PIC_TYPE_LEADING?"Leading":(\
	(t) == PIC_TYPE_TRAILING?"Trailing":"Unknown")))))

#define HEVC_PICTURE_TYPE_NAMEW(t)	(\
	(t) == PIC_TYPE_IDR?L"IDR":(\
	(t) == PIC_TYPE_CRA?L"CRA" : (\
	(t) == PIC_TYPE_IRAP?L"IRAP":(\
	(t) == PIC_TYPE_LEADING?L"Leading":(\
	(t) == PIC_TYPE_TRAILING?L"Trailing":L"Unknown")))))

#ifdef _UNICODE
#define HEVC_PICTRUE_TYPE_NAME(t)	HEVC_PICTURE_TYPE_NAMEW(t)
#else
#define HEVC_PICTURE_TYPE_NAME(t)	HEVC_PICTURE_TYPE_NAMEA(t)
#endif

enum AVC_PICTURE_SLICE_TYPE
{
	AVC_PIC_SLICE_I = 0,
	AVC_PIC_SLICE_P,
	AVC_PIC_SLICE_B,
	AVC_PIC_SLICE_SI,
	AVC_PIC_SLICE_SP,
	AVC_PIC_SLICE_SI_I,
	AVC_PIC_SLICE_SI_I_SP_P,
	AVC_PIC_SLICE_SI_I_SP_P_B
};

#define AVC_PIC_SLICE_TYPE_NAMEA(st)	(\
	(st) == AVC_PIC_SLICE_I?"I":(\
	(st) == AVC_PIC_SLICE_P?"P":(\
	(st) == AVC_PIC_SLICE_B?"B":(\
	(st) == AVC_PIC_SLICE_SI?"SI":(\
	(st) == AVC_PIC_SLICE_SP?"SP":(\
	(st) == AVC_PIC_SLICE_SI_I?"SI":(\
	(st) == AVC_PIC_SLICE_SI_I_SP_P?"SP":(\
	(st) == AVC_PIC_SLICE_SI_I_SP_P_B?"B":""))))))))

#define AVC_PIC_SLICE_TYPE_NAMEW(st)	(\
	(st) == AVC_PIC_SLICE_I?L"I":(\
	(st) == AVC_PIC_SLICE_P?L"P":(\
	(st) == AVC_PIC_SLICE_B?L"B":(\
	(st) == AVC_PIC_SLICE_SI?L"SI":(\
	(st) == AVC_PIC_SLICE_SP?L"SP":(\
	(st) == AVC_PIC_SLICE_SI_I?L"SI":(\
	(st) == AVC_PIC_SLICE_SI_I_SP_P?L"SP":(\
	(st) == AVC_PIC_SLICE_SI_I_SP_P_B?L"B":L""))))))))

#ifdef _UNICODE
#define AVC_PIC_SLICE_TYPE_NAME(st)	AVC_PIC_SLICE_TYPE_NAMEW(st)
#else
#define AVC_PIC_SLICE_TYPE_NAME(st)	AVC_PIC_SLICE_TYPE_NAMEA(st)
#endif

#define HEVC_B_SLICE					0
#define HEVC_P_SLICE					1
#define HEVC_I_SLICE					2

#define HEVC_PIC_SLICE_TYPE_NAMEA(st)	(\
	(st) == HEVC_B_SLICE?"B":(\
	(st) == HEVC_P_SLICE?"P":(\
	(st) == HEVC_I_SLICE?"I":"")))

#define HEVC_PIC_SLICE_TYPE_NAMEW(st)	(\
	(st) == HEVC_B_SLICE?L"B":(\
	(st) == HEVC_P_SLICE?L"P":(\
	(st) == HEVC_I_SLICE?L"I":L"")))

#define VVC_B_SLICE						0
#define VVC_P_SLICE						1
#define VVC_I_SLICE						2

#define VVC_PIC_SLICE_TYPE_NAMEA(st)	(\
	(st) == VVC_B_SLICE?"B":(\
	(st) == VVC_P_SLICE?"P":(\
	(st) == VVC_I_SLICE?"I":"")))

#define VVC_PIC_SLICE_TYPE_NAMEW(st)	(\
	(st) == VVC_B_SLICE?L"B":(\
	(st) == VVC_P_SLICE?L"P":(\
	(st) == VVC_I_SLICE?L"I":L"")))

#define VVC_PIC_SLICE_TYPE_NAMEA(st)	(\
	(st) == VVC_B_SLICE?"B":(\
	(st) == VVC_P_SLICE?"P":(\
	(st) == VVC_I_SLICE?"I":"")))

#define VVC_PIC_SLICE_TYPE_NAMEW(st)	(\
	(st) == VVC_B_SLICE?L"B":(\
	(st) == VVC_P_SLICE?L"P":(\
	(st) == VVC_I_SLICE?L"I":L"")))

#define IS_HEVC_PARAMETERSET_NAL(nal_unit_type)	(\
	(nal_unit_type) == HEVC_VPS_NUT ||\
	(nal_unit_type) == HEVC_SPS_NUT ||\
	(nal_unit_type) == HEVC_PPS_NUT)

#define IS_AVC_PARAMETERSET_NAL(nal_unit_type)	(\
	(nal_unit_type) == AVC_SPS_NUT ||\
	(nal_unit_type) == AVC_PPS_NUT)

#define IS_VVC_PARAMETERSET_NAL(nal_unit_type)	(\
	(nal_unit_type) == VVC_OPI_NUT ||\
	(nal_unit_type) == VVC_DCI_NUT ||\
	(nal_unit_type) == VVC_VPS_NUT ||\
	(nal_unit_type) == VVC_SPS_NUT ||\
	(nal_unit_type) == VVC_PPS_NUT)

#define IS_HEVC_VCL_NAL(nal_unit_type)	(\
	((nal_unit_type) >= HEVC_TRAIL_N  && (nal_unit_type) <= HEVC_RASL_R) ||\
	((nal_unit_type) >= HEVC_BLA_W_LP && (nal_unit_type) <= HEVC_CRA_NUT))

#define IS_AVC_VCL_NAL(nal_unit_type)	(\
	(nal_unit_type) >= AVC_CS_NON_IDR_PIC && (nal_unit_type) <= AVC_CS_IDR_PIC)

#define IS_VVC_VCL_NAL(nal_unit_type)	(\
	(nal_unit_type) >= VVC_TRAIL_NUT && (nal_unit_type) <= VVC_RSV_IRAP_11)

#define IS_PARSABLE_HEVC_NAL(nal_unit_type)	\
										(IS_HEVC_PARAMETERSET_NAL(nal_unit_type) || IS_HEVC_VCL_NAL(nal_unit_type))

#define IS_PARSABLE_AVC_NAL(nal_unit_type)	\
										(IS_AVC_PARAMETERSET_NAL(nal_unit_type) || IS_AVC_VCL_NAL(nal_unit_type))

#define IS_TRAIL(nal_unit_type)			(nal_unit_type >= HEVC_TRAIL_N && nal_unit_type <= HEVC_TRAIL_R)

#define IS_TSA(nal_unit_type)			(nal_unit_type >= HEVC_TSA_N && nal_unit_type <= HEVC_TSA_R)

#define IS_STSA(nal_unit_type)			(nal_unit_type >= HEVC_STSA_N && nal_unit_type <= HEVC_STSA_R)

#define IS_RADL(nal_unit_type)			(nal_unit_type >= HEVC_RADL_N && nal_unit_type <= HEVC_RADL_R)

#define IS_RASL(nal_unit_type)			(nal_unit_type >= HEVC_RASL_N && nal_unit_type <= HEVC_RASL_R)

#define IS_IRAP(nal_unit_type)			(nal_unit_type >= HEVC_BLA_W_LP && nal_unit_type <= HEVC_RSV_IRAP_VCL23)

#define IS_BLA(nal_unit_type)			(nal_unit_type >= HEVC_BLA_W_LP && nal_unit_type <= HEVC_BLA_N_LP)

#define IS_IDR(nal_unit_type)			(nal_unit_type >= HEVC_IDR_W_RADL && nal_unit_type <= HEVC_IDR_N_LP)

#define IS_CRA(nal_unit_type)			(nal_unit_type == HEVC_CRA_NUT)

#define IS_SLNR(nal_unit_type)			(nal_unit_type == TRAIL_N || nal_unit_type == TSA_N || nal_unit_type == STSA_N || nal_unit_type == RADL_N || \
										 nal_unit_type == RASL_N || nal_unit_type == RSV_VCL_N10 || nal_unit_type == RSV_VCL_N12 || nal_unit_type == RSV_VCL_N14)

#define IS_LEADING(nal_unit_type)		(IS_RADL(nal_unit_type) || IS_RASL(nal_unit_type))

#define IS_TRAILING(nal_unit_type)		(IS_TRAIL(nal_unit_type) || IS_TSA(nal_unit_type) || IS_STSA(nal_unit_type))

#define IS_VVC_TRAIL(nal_unit_type)		(nal_unit_type == VVC_TRAIL_NUT)

#define IS_VVC_STSA(nal_unit_type)		(nal_unit_type >= VVC_STSA_NUT)

#define IS_VVC_IDR(nal_unit_type)		(nal_unit_type >= VVC_IDR_W_RADL && nal_unit_type <= VVC_IDR_N_LP)

#define IS_VVC_RADL(nal_unit_type)		(nal_unit_type == VVC_RADL_NUT)

#define IS_VVC_RASL(nal_unit_type)		(nal_unit_type == VVC_RASL_NUT)

#define IS_VVC_IRAP(nal_unit_type)		(nal_unit_type >= VVC_IDR_W_RADL && nal_unit_type <= VVC_RSV_IRAP_11)

#define IS_VVC_CRA(nal_unit_type)		(nal_unit_type == VVC_CRA_NUT)

#define IS_VVC_LEADING(nal_unit_type)	(IS_VVC_RADL(nal_unit_type) || IS_VVC_RASL(nal_unit_type))

#define IS_VVC_TRAILING(nal_unit_type)	(IS_VVC_TRAIL(nal_unit_type) || IS_VVC_STSA(nal_unit_type))


struct NAL_UNIT_ENTRY
{
	uint64_t	file_offset;
	uint32_t	NU_offset;							// The offset in current NAL ring buffer, for example, an AU buffer
	uint32_t	NU_length;							// The current NNAL unit length, if the NAL byte-stream is streamed instead of file
	uint8_t		leading_bytes;						// Specify how many bytes are leading at the header of NAL_Unit, normally it consists of
													// zero byte(optional) + start_code_prefix(00 00 01)
	uint8_t		reserved_for_use[3];

	union
	{
		uint8_t		bytes_reserved[24];
		struct
		{
			uint16_t	forbidden_zero_bit_vvc : 1;
			uint16_t	nuh_reserved_zero_bit : 1;
			uint16_t	nuh_layer_id_vvc : 6;
			uint16_t	nal_unit_type_vvc : 5;
			uint16_t	nuh_temporal_id_plus1_vvc : 3;

			// Only valid for VVC VCL unit
			uint32_t	ph_pic_order_cnt_lsb : 19;
			uint32_t	dword_align_0 : 13;

			int32_t		PicOrderCntVal;
		}PACKED;
		struct
		{
			uint16_t	forbidden_zero_bit : 1;
			uint16_t	nal_unit_type_hevc : 6;
			uint16_t	nuh_layer_id_hevc : 6;
			uint16_t	nuh_temporal_id_plus1 : 3;

			uint16_t	no_output_of_prior_pics_flag : 1;
			uint16_t	word_align_0 : 15;
		}PACKED;
		struct
		{
			uint16_t	forbidden_zero_bit_avc : 1;
			uint16_t	nal_unit_type_avc : 6;
			uint16_t	nal_ref_idc : 2;

			// Only valid for AVC VCL unit
			uint16_t	frame_mbs_only_flag : 1;
			uint16_t	field_pic_flag : 1;
			uint16_t	bottom_field_flag : 1;
			uint16_t	pic_order_cnt_type : 2;
			uint16_t	byte_align_0 : 2;

			uint16_t	frame_num;
			uint16_t	pic_order_cnt_lsb;
			uint16_t	idr_pic_id;

			int32_t		delta_pic_order_cnt_bottom;
			int32_t		delta_pic_order_cnt_0;
			int32_t		delta_pic_order_cnt_1;
		}PACKED;
	}PACKED;

	// In AVC, we need follow the spec "7.4.1.2.4 Detection of the first VCL NAL unit of a primary coded picture"
	// to check whether the current VCL NAL unit is the first slice or not , and mark this value
	// In HEVC, there is already the flag, only read it directly, and then it is ok
	uint8_t		first_slice_segment_in_pic_flag : 1;
	uint8_t		byte_align_1 : 7;

	int8_t		slice_type;

	int16_t		slice_pic_parameter_set_id;

	NAL_UNIT_ENTRY() 
		: file_offset(UINT64_MAX), NU_offset(0), NU_length(0), leading_bytes(0), reserved_for_use{ 0 }
		, bytes_reserved{ 0 }, first_slice_segment_in_pic_flag(0), byte_align_1(0), slice_type(0), slice_pic_parameter_set_id(0){
	}

	// VVC
	NAL_UNIT_ENTRY(uint64_t pos, uint8_t nLeadingBytes, uint16_t zero_bit, uint16_t nuhReservedZeroBit, uint16_t nuType, uint16_t nuhLayerID, uint16_t nuhTemporalIDPlus1) :
		file_offset(pos), NU_offset(0), NU_length(UINT32_MAX), leading_bytes(nLeadingBytes),
		forbidden_zero_bit_vvc(zero_bit), nuh_reserved_zero_bit(nuhReservedZeroBit), nuh_layer_id_vvc(nuhLayerID), nal_unit_type_vvc(nuType), nuh_temporal_id_plus1_vvc(nuhTemporalIDPlus1),
		ph_pic_order_cnt_lsb(0), PicOrderCntVal(0), first_slice_segment_in_pic_flag(0), slice_type(-1), slice_pic_parameter_set_id(0) {}

	NAL_UNIT_ENTRY(uint64_t pos, uint32_t offset, uint32_t len, uint8_t nLeadingBytes, uint16_t zero_bit, uint16_t nuhReservedZeroBit, uint16_t nuType, uint16_t nuhLayerID, uint16_t nuhTemporalIDPlus1) :
		file_offset(pos), NU_offset(offset), NU_length(len), leading_bytes(nLeadingBytes),
		forbidden_zero_bit_vvc(zero_bit), nuh_reserved_zero_bit(nuhReservedZeroBit), nuh_layer_id_vvc(nuhLayerID), nal_unit_type_vvc(nuType), nuh_temporal_id_plus1_vvc(nuhTemporalIDPlus1),
		ph_pic_order_cnt_lsb(0), PicOrderCntVal(0), first_slice_segment_in_pic_flag(0), slice_type(-1), slice_pic_parameter_set_id(0) {}

	// HEVC
	NAL_UNIT_ENTRY(uint64_t pos, uint8_t nLeadingBytes, uint16_t zero_bit, uint16_t nuType, uint16_t nuhLayerID, uint16_t nuhTemporalIDPlus1) :
		file_offset(pos), NU_offset(0), NU_length(UINT32_MAX), leading_bytes(nLeadingBytes), 
		forbidden_zero_bit(zero_bit), nal_unit_type_hevc(nuType), nuh_layer_id_hevc(nuhLayerID), nuh_temporal_id_plus1(nuhTemporalIDPlus1),
		no_output_of_prior_pics_flag(0), first_slice_segment_in_pic_flag(0), slice_type(-1), slice_pic_parameter_set_id(0) {}

	NAL_UNIT_ENTRY(uint64_t pos, uint32_t offset, uint32_t len, uint8_t nLeadingBytes, uint16_t zero_bit, uint16_t nuType, uint16_t nuhLayerID, uint16_t nuhTemporalIDPlus1) :
		file_offset(pos), NU_offset(offset), NU_length(len), leading_bytes(nLeadingBytes), 
		forbidden_zero_bit(zero_bit), nal_unit_type_hevc(nuType), nuh_layer_id_hevc(nuhLayerID), nuh_temporal_id_plus1(nuhTemporalIDPlus1),
		no_output_of_prior_pics_flag(0), first_slice_segment_in_pic_flag(0), slice_type(-1), slice_pic_parameter_set_id(0) {}

	// AVC
	NAL_UNIT_ENTRY(uint64_t pos, uint8_t nLeadingBytes, uint8_t zero_bit, uint8_t nalRefIdc, uint8_t nuType) :
		file_offset(pos), NU_offset(0), NU_length(UINT32_MAX), leading_bytes(nLeadingBytes), 
		forbidden_zero_bit_avc(zero_bit), nal_unit_type_avc(nuType), nal_ref_idc(nalRefIdc),
		frame_num(0), first_slice_segment_in_pic_flag(0), slice_type(-1), slice_pic_parameter_set_id(0) {}

	NAL_UNIT_ENTRY(uint64_t pos, uint32_t offset, uint32_t len, uint8_t nLeadingBytes, uint8_t zero_bit, uint8_t nalRefIdc, uint8_t nuType) :
		file_offset(pos), NU_offset(offset), NU_length(len), leading_bytes(nLeadingBytes), 
		forbidden_zero_bit_avc(zero_bit), nal_unit_type_avc(nuType), nal_ref_idc(nalRefIdc),
		frame_num(0), first_slice_segment_in_pic_flag(0), slice_type(-1), slice_pic_parameter_set_id(0) {}

	bool Is_The_First_AVC_VCL_NU(NAL_UNIT_ENTRY& last_nu) {

		// the first AVC VCL NAL Unit since there is no valid last NAL unit
		if (last_nu.file_offset == UINT64_MAX)
			return true;

		if (nal_unit_type_avc != AVC_CS_NON_IDR_PIC &&
			nal_unit_type_avc != AVC_CSD_PARTITION_A &&
			nal_unit_type_avc != AVC_CS_IDR_PIC)
			return false;

		assert(last_nu.nal_unit_type_avc == AVC_CS_NON_IDR_PIC ||
			   last_nu.nal_unit_type_avc == AVC_CSD_PARTITION_A ||
			   last_nu.nal_unit_type_avc == AVC_CS_IDR_PIC);

		/*
			frame_num differs in value. The value of frame_num used to test this condition is the value of frame_num that appears
			in the syntax of the slice header, regardless of whether that value is inferred to have been equal to 0 for subsequent
			use in the decoding process due to the presence of memory_management_control_operation equal to 5.
			NOTE 1: A consequence of the above statement is that a primary coded picture having frame_num equal to 1 cannot
			contain a memory_management_control_operation equal to 5 unless some other condition listed below is fulfilled for the
			next primary coded picture that follows after it (if any).
		*/
		if (frame_num != last_nu.frame_num)
			return true;

		/* pic_parameter_set_id differs in value. */
		if (slice_pic_parameter_set_id != last_nu.slice_pic_parameter_set_id)
			return true;

		/* field_pic_flag differs in value. */
		if (field_pic_flag != last_nu.field_pic_flag)
			return true;

		/* bottom_field_flag is present in both and differs in value. */
		if (!frame_mbs_only_flag && field_pic_flag &&
			!last_nu.frame_mbs_only_flag && last_nu.field_pic_flag &&
			bottom_field_flag != last_nu.bottom_field_flag)
			return true;

		/* nal_ref_idc differs in value with one of the nal_ref_idc values being equal to 0 */
		if ((nal_ref_idc == 0 || last_nu.nal_ref_idc == 0) && nal_ref_idc != last_nu.nal_ref_idc)
			return true;

		/* pic_order_cnt_type is equal to 0 for both and either pic_order_cnt_lsb differs in value, or delta_pic_order_cnt_bottom differs in value */
		if (pic_order_cnt_type == 0 && last_nu.pic_order_cnt_type == 0)
		{
			if (pic_order_cnt_lsb != last_nu.pic_order_cnt_lsb ||
				delta_pic_order_cnt_bottom != last_nu.delta_pic_order_cnt_bottom)
				return true;
		}

		/* pic_order_cnt_type is equal to 1 for both and either delta_pic_order_cnt[ 0 ] differs in value, or delta_pic_order_cnt[ 1 ] differs in value */
		if (pic_order_cnt_type == 1 && last_nu.pic_order_cnt_type == 1)
		{
			if (delta_pic_order_cnt_0 != last_nu.delta_pic_order_cnt_0 ||
				delta_pic_order_cnt_1 != last_nu.delta_pic_order_cnt_1)
				return true;
		}

		/* IdrPicFlag differs in value. */
		if ((nal_unit_type_avc == 5 && last_nu.nal_unit_type_avc != 5) ||
			(nal_unit_type_avc != 5 && last_nu.nal_unit_type_avc == 5))
			return true;

		/* IdrPicFlag is equal to 1 for both and idr_pic_id differs in value. */
		if (nal_unit_type_avc == 5 && last_nu.nal_unit_type_avc == 5 &&
			idr_pic_id != last_nu.idr_pic_id)
			return true;

		return false;
	}
};

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

