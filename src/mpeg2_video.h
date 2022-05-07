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
#ifndef _MPEG2_VIDEO_STREAM_H_
#define _MPEG2_VIDEO_STREAM_H_

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "DumpUtil.h"
#include "AMArray.h"
#include "AMException.h"
#include "mpeg2_video_parser.h"

//
// Design description
// Since the input bit stream is arbitrary, it can't be guaranteed there is sudden error in the middle of bit stream or bit-stream is not complete.
// In order to solve this issue, we have to design the parser carefully.
// Two concepts:
// 1. basic data structure element, BDSE, for example, sequence_header, group_of_picture_header, picture display extension and so on.
// 2. complex data structure element, CDSE, for example, video sequence, the user defined structure in current implementation.
// Rules:
// 1. If a BDSE has captured a recognized error in the middle of stream, the parsing will cease, and it will be marked as "broken" or removed.
// 2. If a BDSE has captured a unrecognized error in the middle of stream, the whole parsing will cease.
// 3. If a CDSE checked the return code of BDSE mapping, and find the BDSE has failed, it does not need delete BDSE, because BDSE has error flag in it.
// 4. If a CDSE checked the return code of CDSE mapping, and find the CDSE has failed, it does not need delete CDSE, because BDSE in this CDSE will mark where error happened.
// 5. If NO_MORE_DATA result is returned, the BDSE, and its parent CDSE should return immediately
// 6. A BDSE has a member "status", it can be used to mark the current BDSE has error, broken or ok
// 7. For BDSE, the navigation field print, we need use BST_FIELD_PROP_xxxx; for CDSE, still use NAV_FIELD_PROP_xxxx

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

#define PICTURE_START_CODE						0x00
#define SLICE_START_CODE						0x01
#define SLICE_END_CODE							0xAF
#define USER_DATA_START_CODE					0xB2
#define SEQUENCE_HEADER_CODE					0xB3
#define SEQUENCE_ERROR_CODE						0xB4
#define EXTENSION_START_CODE					0xB5
#define SEQUENCE_END_CODE						0xB7
#define GROUP_START_CODE						0xB8
#define SYSTEM_START_CODE_FIRST					0xB9
#define SYSTEM_START_CODE_LAST					0xFF

#define MPV_OBJ_NAME(start_code)	(\
	(start_code) == PICTURE_START_CODE?"Picture Header":(\
	(start_code) >= SLICE_START_CODE && (start_code) <= SLICE_END_CODE?"Slice Data":(\
	(start_code) == USER_DATA_START_CODE?"User Data":(\
	(start_code) == SEQUENCE_HEADER_CODE?"Sequence Header":(\
	(start_code) == SEQUENCE_ERROR_CODE?"Sequence Error":(\
	(start_code) == EXTENSION_START_CODE?"Extension Data":(\
	(start_code) == SEQUENCE_END_CODE?"Sequence End":(\
	(start_code) == GROUP_START_CODE?"GOP Header":(\
	(start_code) >= SYSTEM_START_CODE_FIRST && (start_code) <= SYSTEM_START_CODE_LAST?"System Layer":"Unknown")))))))))

#define PICTURE_CODING_TYPE_SHORTNAME(pct)	(\
	(pct) == 1?"I":(\
	(pct) == 2?"P":(\
	(pct) == 3?"B":(\
	(pct) == 4?"D":""))))

#define MAKE_START_CODE(start_code_byte)		((0x01<<8) | start_code_byte)

#define TOP_FIELD								1
#define BOTTOM_FIELD							2
#define FRAME_PICTURE							3

#define SEQUENCE_EXTENSION_ID					1
#define SEQUENCE_DISPLAY_EXTENSION_ID			2
#define QUANT_MATRIX_EXTENSION_ID				3
#define COPYRIGHT_EXTENSION_ID					4
#define	SEQUENCE_SCALABLE_EXTENSION_ID			5
//---> Reserved
#define PICTURE_DISPLAY_EXTENSION_ID			7
#define	PICTURE_CODING_EXTENSION_ID				8
#define PICTURE_SPATIAL_SCALABLE_EXTENSION_ID	9
#define PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID	10
#define CAMERA_PARAMETERS_EXTENSION_ID			11
#define ITUT_EXTENSION_ID						12

#define MPV_OBJ_NAME_EXT(start_code, ext_id)	(\
	(start_code)!=EXTENSION_START_CODE?MPV_OBJ_NAME(start_code):(\
	(ext_id) == SEQUENCE_EXTENSION_ID?"Sequence extension":(\
	(ext_id) == SEQUENCE_DISPLAY_EXTENSION_ID?"Sequence Display Extension":(\
	(ext_id) == QUANT_MATRIX_EXTENSION_ID?"Quant Matrix Extension":(\
	(ext_id) == COPYRIGHT_EXTENSION_ID?"Copyright Extension":(\
	(ext_id) == SEQUENCE_SCALABLE_EXTENSION_ID?"Sequence Scalable Extension":(\
	(ext_id) == PICTURE_DISPLAY_EXTENSION_ID?"Picture Display Extension":(\
	(ext_id) == PICTURE_CODING_EXTENSION_ID?"Picture coding extension":(\
	(ext_id) == PICTURE_SPATIAL_SCALABLE_EXTENSION_ID?"Picture Spatial Scalable Extension":(\
	(ext_id) == PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID?"Picture Temporal Scalable Extension":(\
	(ext_id) == CAMERA_PARAMETERS_EXTENSION_ID?"Camera Parameters Extension":(\
	(ext_id) == ITUT_EXTENSION_ID?"ITU-T Extension":"Unknown Extension"))))))))))))

extern const char* picture_coding_type_name[8];
extern const char* aspect_ratio_infomation_desc[16];
extern const char* frame_rate_value_desc[16];
extern const char* chroma_format_names[4];
extern const char* mpeg2_video_format_names[8];
extern const char* mpeg2_video_scalable_mode[4];
extern const char* mpeg2_video_timecode_type_names[4];
extern const char* mpeg2_video_counting_type_names[8];
extern const char* profile_and_level_identification_names[8][16];
extern const char* mpeg2_profile_names[8];
extern const char* mpeg2_level_names[16];
extern const char* mpv_syntax_element_names[256];
extern const char* mpv_extension_syntax_element_names[16];

extern int next_start_code(AMBst in_bst);
extern uint8_t* FindNextStartCode(uint8_t* pBuf, unsigned long cbSize);
extern const char* get_profile_and_level_indication_names(int profile_and_level_indication);

extern RET_CODE CreateMPVContext(IMPVContext** ppMPVCtx);

namespace BST {
	namespace MPEG2Video {

	enum MPV_PROFILE
	{
		MPV_PROFILE_UNKNOWN = -1,
		MPV_PROFILE_HIGH,
		MPV_PROFILE_SPATIALLY_SCALABLE,
		MPV_PROFILE_SNR_SCALABLE,
		MPV_PROFILE_MAIN,
		MPV_PROFILE_SIMPLE,
		MPV_PROFILE_422,
		MPV_PROFILE_MULTI_VIEW,
	};

	enum MPV_LEVEL
	{
		MPV_LEVEL_UNKNOWN = -1,
		MPV_LEVEL_HIGHP = 2,
		MPV_LEVEL_HIGH = 4,
		MPV_LEVEL_HIGH_1440 = 6,
		MPV_LEVEL_MAIN = 8,
		MPV_LEVEL_LOW = 10,
	};

	typedef struct {
		unsigned short	code;
		char			val;
		char			len;
	} vlclbf;

	typedef struct {
		unsigned short	val;
		char			len;
	} varbits;

	typedef struct {
		int				leading_bits;
		unsigned char	byte_value;
	} scatterbyte;

	struct CSequenceHeader: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		sequence_header_code[4] = { 0 };
		
		unsigned long		horizontal_size_value:12;
		unsigned long		vertical_size_value:12;
		unsigned long		aspect_ratio_information:4;
		unsigned long		frame_rate_code:4;
		
		unsigned long		bit_rate_value:18;
		unsigned long		marker_bit_0:1;
		unsigned long		vbv_buffer_size_value:10;
		unsigned long		constrained_parameters_flag:1;
		unsigned long		load_intra_quantiser_matrix:1;
		unsigned long		load_non_intra_quantiser_matrix:1;

		unsigned char		intra_quantiser_matrix[64] = { 0 };
		unsigned char		non_intra_quantiser_matrix[64] = { 0 };

		CSequenceHeader()
			: horizontal_size_value(0), vertical_size_value(0), aspect_ratio_information(0), frame_rate_code(0)
			, bit_rate_value(0), marker_bit_0(0), vbv_buffer_size_value(0), constrained_parameters_flag(0)
			, load_intra_quantiser_matrix(0), load_non_intra_quantiser_matrix(0){
		}

		int Map(AMBst in_bst)
		{
			// Assume to never fail for SYNTAX_BITSTREAM_MAP::Map and get the start code
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, sequence_header_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, horizontal_size_value, 12);
				MAP_BST_BITS(in_bst, vertical_size_value, 12);
				MAP_BST_BITS(in_bst, aspect_ratio_information, 4);
				MAP_BST_BITS(in_bst, frame_rate_code, 4);
				
				MAP_BST_BITS(in_bst, bit_rate_value, 18);
				MAP_BST_BITS(in_bst, marker_bit_0, 1);
				
				MAP_BST_BITS(in_bst, vbv_buffer_size_value, 10);
				MAP_BST_BITS(in_bst, constrained_parameters_flag, 1);

				MAP_BST_BITS(in_bst, load_intra_quantiser_matrix, 1);
				if (load_intra_quantiser_matrix){
					for(int i=0;i<64;i++){
						MAP_BST_T_BITS(in_bst, unsigned char, intra_quantiser_matrix[i], 8);
					}
				}

				MAP_BST_BITS(in_bst, load_non_intra_quantiser_matrix, 1);
				if (load_non_intra_quantiser_matrix){
					for(int i=0;i<64;i++){
						MAP_BST_T_BITS(in_bst, unsigned char, non_intra_quantiser_matrix[i], 8);
					}
				}

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR1(sequence_header_code, 32, "should be 00 00 01 B3");
			
			BST_FIELD_PROP_2NUMBER1(horizontal_size_value, 12, "This word forms the 12 least significant bits of horizontal_size")
			BST_FIELD_PROP_2NUMBER1(vertical_size_value, 12, "This word forms the 12 least significant bits of vertical_size")
			BST_FIELD_PROP_2NUMBER1(aspect_ratio_information, 4, aspect_ratio_infomation_desc[aspect_ratio_information])
			BST_FIELD_PROP_2NUMBER1(frame_rate_code, 4, frame_rate_value_desc[frame_rate_code])

			BST_FIELD_PROP_2NUMBER1(bit_rate_value, 18, "The lower 18 bits of bit_rate, and the upper 12 bits are in bit_rate_extension. bit_rate is measured in units of 400 bits/second, rounded upwards.")
			BST_FIELD_PROP_NUMBER  ("marker_bit", 1, marker_bit_0, "")

			BST_FIELD_PROP_2NUMBER1(vbv_buffer_size_value, 10, "the lower 10 bits of vbv_buffer_size, and the upper 8 bits are in vbv_buffer_size_extension.")
			BST_FIELD_PROP_NUMBER1 (constrained_parameters_flag, 1, "This flag (used in ISO/IEC 11172-2) has no meaning in this Specification and shall have the value '0'")

			BST_FIELD_PROP_NUMBER1 (load_intra_quantiser_matrix, 1, "See 6.3.11 &quot;Quant matrix extension&quot;.")
			if (load_intra_quantiser_matrix){
				BST_FIELD_PROP_1D_ARRAY_MATROX1(intra_quantiser_matrix, 8, 8, "See 6.3.11 &quot;Quant matrix extension&quot;.")
			}
			NAV_FIELD_PROP_NUMBER1 (load_non_intra_quantiser_matrix, 1, "See 6.3.11 &quot;Quant matrix extension&quot;.")
			if (load_non_intra_quantiser_matrix){
				BST_FIELD_PROP_1D_ARRAY_MATROX1(non_intra_quantiser_matrix, 8, 8, "See 6.3.11 &quot;Quant matrix extension&quot;.")
			}
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CUserData: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		user_data_start_code[4] = { 0 };
		std::vector<uint8_t>
							user_data;

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, user_data_start_code, 4);
			
			MAP_BST_BEGIN(1);
			try
			{
				while (AMBst_PeekBits(in_bst, 24) != 0x01){
					user_data.push_back(AMBst_GetByte(in_bst));
				}

				// Don't need call next_start_code()
				// Here it should be start_code, or the bit stream data has already reached EOS
			}
			catch(AMException e)
			{
				iRet = e.RetCode();
			}
			MAP_BST_END();

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_FIXSIZE_BINSTR("user_data_start_code", 32, user_data_start_code, 4, "should be 00 00 01 B2");
			int user_data_length = (int)user_data.size();
			if (user_data_length > 0)
			{
				NAV_FIELD_PROP_FIXSIZE_BINSTR("user_data", user_data_length*8, user_data.data(), (unsigned long)user_data_length, "User data is defined by users for their specific applications.");
			}
		DECLARE_FIELDPROP_END()	
	};

	struct CSequenceExtension : public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };

		unsigned long		extension_start_code_identifier:4;
		unsigned long		profile_and_level_indication:8;
		unsigned long		progressive_sequence:1;
		unsigned long		chroma_format:2;
		unsigned long		horizontal_size_extension:2;
		unsigned long		vertical_size_extension:2;
		unsigned long		bit_rate_extension:12;
		unsigned long		marker_bit:1;

		unsigned char		vbv_buffer_size_extension = 0;

		unsigned char		low_delay:1;
		unsigned char		frame_rate_extension_n:2;
		unsigned char		frame_rate_extension_d:5;

		CSequenceExtension()
			: extension_start_code_identifier(0), profile_and_level_indication(0), progressive_sequence(0)
			, chroma_format(0), horizontal_size_extension(0), vertical_size_extension(0), bit_rate_extension(0), marker_bit(0)
			, low_delay(0), frame_rate_extension_n(0), frame_rate_extension_d(0){
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BYTE(in_bst, profile_and_level_indication);
				MAP_BST_BITS(in_bst, progressive_sequence, 1);
				MAP_BST_BITS(in_bst, chroma_format, 2);
				MAP_BST_BITS(in_bst, horizontal_size_extension, 2);
				MAP_BST_BITS(in_bst, vertical_size_extension, 2);
				MAP_BST_BITS(in_bst, bit_rate_extension, 12);
				MAP_BST_BITS(in_bst, marker_bit, 1);

				MAP_BST_BYTE(in_bst, vbv_buffer_size_extension);

				MAP_BST_BITS(in_bst, low_delay, 1);
				MAP_BST_BITS(in_bst, frame_rate_extension_n, 2);
				MAP_BST_BITS(in_bst, frame_rate_extension_d, 5);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		MPV_PROFILE GetProfile() {
			unsigned char Escape_bit = (profile_and_level_indication >> 7) & 0X01;
			unsigned char Profile_identification = (profile_and_level_indication >> 4) & 0x07;
			unsigned char Level_identification = profile_and_level_indication & 0xF;

			if (Escape_bit) {
				if (Profile_identification == 0) {
					switch (Level_identification) {
					case 0xE:
						return MPV_PROFILE_MULTI_VIEW;
					case 0XB:
						return MPV_PROFILE_MULTI_VIEW;
					case 0xA:
						return MPV_PROFILE_MULTI_VIEW;
					case 0x5:
						return MPV_PROFILE_422;
					case 0x2:
						return MPV_PROFILE_422;
					default:
						return MPV_PROFILE_UNKNOWN;
					}
				}
				return MPV_PROFILE_UNKNOWN;
			}

			return (MPV_PROFILE)(Profile_identification);
		}

		MPV_LEVEL GetLevel() {
			unsigned char Escape_bit = (profile_and_level_indication >> 7) & 0X01;
			unsigned char Profile_identification = (profile_and_level_indication >> 4) & 0x07;
			unsigned char Level_identification = profile_and_level_indication & 0xF;

			if (Escape_bit) {
				if (Profile_identification == 0) {
					switch (Level_identification) {
					case 0xE:
						return MPV_LEVEL_LOW;;
					case 0XB:
						return MPV_LEVEL_HIGH_1440;
					case 0xA:
						return MPV_LEVEL_HIGH;
					case 0x5:
						return MPV_LEVEL_MAIN;
					case 0x2:
						return MPV_LEVEL_HIGH;
					default:
						return MPV_LEVEL_UNKNOWN;
					}
				}
				return MPV_LEVEL_UNKNOWN;
			}

			return (MPV_LEVEL)Level_identification;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "Should be 1")
			BST_FIELD_PROP_2NUMBER("profile_and_level_indication", 8, profile_and_level_indication, get_profile_and_level_indication_names(profile_and_level_indication))
			BST_FIELD_PROP_NUMBER ("progressive_sequence", 1, progressive_sequence, progressive_sequence?"the coded video sequence contains only progressive frame-pictures":
																											"the coded video sequence may contain both frame-pictures and field-pictures, and frame-picture may be progressive or interlaced frames.")
			BST_FIELD_PROP_2NUMBER("chroma_format", 2, chroma_format, chroma_format_names[chroma_format])
			BST_FIELD_PROP_2NUMBER("horizontal_size_extension", 2, horizontal_size_extension, "(horizontal_size_extension&lt;&lt;12)|horizontal_size_value")
			BST_FIELD_PROP_2NUMBER("vertical_size_extension", 2, vertical_size_extension, "(vertical_size_extension&lt;&lt;12)|vertical_size_value")
			BST_FIELD_PROP_2NUMBER("bit_rate_extension", 12, bit_rate_extension, "(bit_rate_extension&lt;18)|bit_rate_value")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit, "")
			BST_FIELD_PROP_2NUMBER("vbv_buffer_size_extension", 8, vbv_buffer_size_extension, "(vbv_buffer_size_extension&lt;10)|vbv_buffer_size_value")
			BST_FIELD_PROP_NUMBER ("low_delay", 1, low_delay, low_delay?"the sequence does not contain any B-pictures, that the frame re-ordering delay is not present in the VBV description and that the bitstream may contain \"big pictures\"":
																		"the sequence may contain B-pictures, that the frame re-ordering delay is present in the VBV description and that bitstream shall not contain big pictures")
			BST_FIELD_PROP_2NUMBER("frame_rate_extension_n", 2, frame_rate_extension_n, "frame_rate = frame_rate_value * (frame_rate_extension_n + 1) / (frame_rate_extension_d + 1)")
			BST_FIELD_PROP_2NUMBER("frame_rate_extension_d", 5, frame_rate_extension_d, "frame_rate = frame_rate_value * (frame_rate_extension_n + 1) / (frame_rate_extension_d + 1)")
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CSequenceDisplayExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };

		unsigned char		extension_start_code_identifier:4;
		unsigned char		video_format:3;
		unsigned char		colour_description:1;

		unsigned char		colour_primaries = 0;
		unsigned char		transfer_characteristics = 0;
		unsigned char		matrix_coefficients = 0;

		unsigned long		display_horizontal_size:14;
		unsigned long		marker_bit:1;
		unsigned long		display_vertical_size:14;
		unsigned long		unused_padding:3;

		CSequenceDisplayExtension()
			: extension_start_code_identifier(0), video_format(0), colour_description(0)
			, display_horizontal_size(0), marker_bit(0), display_vertical_size(0), unused_padding(0){
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, video_format, 3);
				MAP_BST_BITS(in_bst, colour_description, 1);

				if (colour_description){
					MAP_BST_BYTE(in_bst, colour_primaries);
					MAP_BST_BYTE(in_bst, transfer_characteristics);
					MAP_BST_BYTE(in_bst, matrix_coefficients);
				}

				MAP_BST_BITS(in_bst, display_horizontal_size, 14);
				MAP_BST_BITS(in_bst, marker_bit, 1);
				MAP_BST_BITS(in_bst, display_vertical_size, 14);

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		const char* GetColourPrimariesName(){
			switch(colour_primaries){
			case 0: return "Forbidden";
			case 1: return "ITU-R BT.709";
			case 2: return "Unspecified Video";
			case 3: return "Reserved";
			case 4: return "ITU-R BT.470-2 System M";
			case 5: return "ITU-R BT.470-2 System B, G";
			case 6: return "SMPTE 170M";
			case 7: return "SMPTE 240M (1987)";
			}
			return "Reserved";
		}

		const char* GetTransferCharacteristicsName(){
			switch(transfer_characteristics){
			case 0: return "Forbidden";
			case 1: return "ITU-R BT.709";
			case 2: return "Unspecified Video";
			case 3: return "Reserved";
			case 4: return "ITU-R BT.470-2 System M (Assumed display gamma 2.2)";
			case 5: return "ITU-R BT.470-2 System B, G (Assumed display gamma 2.8)";
			case 6: return "SMPTE 170M";
			case 7: return "SMPTE 240M (1987)";
			case 8: return "Linear transfer characteristics";
			}
			return "Reserved";
		}

		const char* GetMatrixCoefficientsName(){
			switch(matrix_coefficients){
			case 0: return "Forbidden";
			case 1: return "ITU-R BT.709";
			case 2: return "Unspecified Video";
			case 3: return "Reserved";
			case 4: return "ITU-R BT.470-2 System M";
			case 5: return "ITU-R BT.470-2 System B, G";
			case 6: return "SMPTE 170M";
			case 7: return "SMPTE 240M (1987)";
			}
			return "Reserved";
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER1(extension_start_code_identifier, 4, "should be 2")
			BST_FIELD_PROP_2NUMBER1(video_format, 3, mpeg2_video_format_names[video_format])
			BST_FIELD_PROP_NUMBER1 (colour_description, 1, colour_description?"the presence of colour_primaries, transfer_characteristics and matrix_coefficients in the bitstream":"")
			if (colour_description)
			{
				BST_FIELD_PROP_2NUMBER1(colour_primaries, 8, GetColourPrimariesName())					
				BST_FIELD_PROP_2NUMBER1(transfer_characteristics, 8, GetTransferCharacteristicsName())
				BST_FIELD_PROP_2NUMBER1(matrix_coefficients, 8, GetMatrixCoefficientsName())
			}

			BST_FIELD_PROP_2NUMBER1(display_horizontal_size, 14, "display_horizontal_size and display_vertical_size together define a rectangle which may be considered as the &quot;intended display&apos;s&quot; active region")
			BST_FIELD_PROP_NUMBER1 (marker_bit, 1, "")
			BST_FIELD_PROP_2NUMBER1(display_vertical_size, 14, "display_horizontal_size and display_vertical_size together define a rectangle which may be considered as the &quot;intended display&apos;s&quot; active region")
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CSequenceScalableExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };

		unsigned char		extension_start_code_identifier:4;
		unsigned char		scalable_mode:2;
		unsigned char		unused_padding_0:2;

		unsigned char		layer_id:4;
		unsigned char		unused_padding_1:4;

		union{
			uint8_t				bytes[8] = { 0 };
			struct {
				unsigned short	lower_layer_prediction_horizontal_size:14;
				unsigned short	marker_bit:1;
				unsigned short	unused_padding_2:1;

				unsigned short	lower_layer_prediction_vertical_size:14;
				unsigned short	unused_padding_3:2;

				unsigned char	horizontal_subsampling_factor_m:5;
				unsigned char	unused_padding_4:3;

				unsigned char	horizontal_subsampling_factor_n:5;
				unsigned char	unused_padding_5:3;

				unsigned char	vertical_subsampling_factor_m:5;
				unsigned char	unused_padding_6:3;

				unsigned char	vertical_subsampling_factor_n:5;
				unsigned char	unused_padding_7:3;
			}PACKED;
			struct {
				unsigned char	picture_mux_enable:1;
				unsigned char	mux_to_progressive_sequence:1;
				unsigned char	picture_mux_order:3;
				unsigned char	picture_mux_factor:3;
			}PACKED;
		}PACKED;

		CSequenceScalableExtension()
			: extension_start_code_identifier(0), scalable_mode(0), unused_padding_0(0)
			, layer_id(0), unused_padding_1(0) {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, scalable_mode, 2);
				MAP_BST_BITS(in_bst, layer_id, 4);

				if (scalable_mode == 1)	// spatial scalability
				{
					MAP_BST_BITS(in_bst, lower_layer_prediction_horizontal_size, 14);
					MAP_BST_BITS(in_bst, marker_bit, 1);
					MAP_BST_BITS(in_bst, lower_layer_prediction_vertical_size, 14);
					MAP_BST_BITS(in_bst, horizontal_subsampling_factor_m, 5);
					MAP_BST_BITS(in_bst, horizontal_subsampling_factor_n, 5);
					MAP_BST_BITS(in_bst, vertical_subsampling_factor_m, 5);
					MAP_BST_BITS(in_bst, vertical_subsampling_factor_n, 5);
				}
				else if(scalable_mode == 3)	// temporal scalability
				{
					MAP_BST_BITS(in_bst, picture_mux_enable, 1);
					if (picture_mux_enable) {
						MAP_BST_BITS(in_bst, mux_to_progressive_sequence, 1);
					}
					MAP_BST_BITS(in_bst, picture_mux_order, 3);
					MAP_BST_BITS(in_bst, picture_mux_factor, 3);
				}

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER1(extension_start_code_identifier, 4, "Should be 5")
			BST_FIELD_PROP_2NUMBER1(scalable_mode, 2, mpeg2_video_scalable_mode[scalable_mode])
			BST_FIELD_PROP_NUMBER1 (layer_id, 4	, "an integer which identifies the layers in a scalable hierarchy. The base layer always has layer_id = 0.")

			if (scalable_mode == 1)	// spatial scalability
			{
				BST_FIELD_PROP_2NUMBER1(lower_layer_prediction_horizontal_size, 14, "the horizontal size of the lower layer "
																					"frame which is used for prediction. This shall contain the value contained in horizontal_size (horizontal_size_value and "
																					"horizontal_size_extension) in the lower layer bitstream");
				BST_FIELD_PROP_NUMBER1 (marker_bit, 1, "");
				BST_FIELD_PROP_2NUMBER1(lower_layer_prediction_vertical_size, 14, "the vertical size of the lower layer frame "
																					"which is used for prediction. This shall contain the value contained in vertical_size (vertical_size_value and "
																					"vertical_size_extension) in the lower layer bitstream");
				BST_FIELD_PROP_2NUMBER1(horizontal_subsampling_factor_m, 5, "This affects the spatial scalable upsampling process, as defined in 7.7.2. The value zero is forbidden");
				BST_FIELD_PROP_2NUMBER1(horizontal_subsampling_factor_n, 5, "This affects the spatial scalable upsampling process, as defined in 7.7.2. The value zero is forbidden");
				BST_FIELD_PROP_2NUMBER1(vertical_subsampling_factor_m, 5, "This affects the spatial scalable upsampling process, as defined in 7.7.2. The value zero is forbidden");
				BST_FIELD_PROP_2NUMBER1(vertical_subsampling_factor_n, 5, "This affects the spatial scalable upsampling process, as defined in 7.7.2. The value zero is forbidden");
			}
			else if(scalable_mode == 3)	// temporal scalability
			{
				BST_FIELD_PROP_NUMBER1 (picture_mux_enable, 1, picture_mux_enable?"picture_mux_order and picture_mux_factor are used for re-multiplexing prior to display":"");
				if (picture_mux_enable)
				{
					BST_FIELD_PROP_NUMBER1 (mux_to_progressive_sequence, 1, mux_to_progressive_sequence?"the decoded pictures corresponding to the two layers shall be temporally multiplexed to generate a progressive sequence for display":
																										"the temporal multiplexing is intended to generate an interlaced sequence");
				}
				BST_FIELD_PROP_2NUMBER1(picture_mux_order, 3, "It denotes number of enhancement layer pictures prior to the first base layer picture");
				BST_FIELD_PROP_2NUMBER1(picture_mux_factor, 3, "It denotes number of enhancement layer pictures between consecutive base layer pictures to "
																"allow correct re-multiplexing of base and enhancement layers for display");
			}
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CGroupPicturesHeader: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char	group_start_code[4] = { 0 };

		unsigned long	drop_frame_flag:1;
		unsigned long	time_code_hours:5;
		unsigned long	time_code_minutes:6;
		unsigned long	marker_bit:1;
		unsigned long	time_code_seconds:6;
		unsigned long	time_code_pictures:6;
		unsigned long	closed_gop:1;
		unsigned long	broken_link:1;
		unsigned long	unused_padding:5;

		CGroupPicturesHeader()
			: drop_frame_flag(0), time_code_hours(0), time_code_minutes(0), marker_bit(0)
			, time_code_seconds(0), time_code_pictures(0), closed_gop(0), broken_link(0), unused_padding(0){
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, group_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, drop_frame_flag, 1);
				MAP_BST_BITS(in_bst, time_code_hours, 5);
				MAP_BST_BITS(in_bst, time_code_minutes, 6);
				MAP_BST_BITS(in_bst, marker_bit, 1);
				MAP_BST_BITS(in_bst, time_code_seconds, 6);
				MAP_BST_BITS(in_bst, time_code_pictures, 6);
				MAP_BST_BITS(in_bst, closed_gop, 1);
				MAP_BST_BITS(in_bst, broken_link, 1);
				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("group_start_code", 32, group_start_code, 4, "should be 00 00 01 B8");
			BST_FIELD_PROP_NUMBER1 (drop_frame_flag, 1, drop_frame_flag?"picture numbers 0 and 1 at the start of each minute, except minutes 0, 10, 20, 30, 40, 50 are omitted from the count":
																		"pictures are counted assuming rounding to the nearest integral number of pictures per second, for example 29.97 Hz would be rounded to and counted as 30 Hz")
			BST_FIELD_PROP_2NUMBER1(time_code_hours, 5, "hours")
			BST_FIELD_PROP_2NUMBER1(time_code_minutes, 6, "minutes")
			BST_FIELD_PROP_NUMBER1 (marker_bit, 1, "")
			BST_FIELD_PROP_2NUMBER1(time_code_seconds, 6, "seconds")
			BST_FIELD_PROP_2NUMBER1(time_code_pictures, 6, "frames")
			BST_FIELD_PROP_NUMBER1 (closed_gop, 1, closed_gop?"these B-pictures have been encoded using only backward prediction or intra coding":"")
			BST_FIELD_PROP_NUMBER1 (broken_link, 1, broken_link?"the first consecutive B-Pictures (if any) immediately following the first coded I-frame following the group of picture header may "
																"not be correctly decoded because the reference frame which is used for prediction is not available (because of the action of editing).":
																"")
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CPictureHeader: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char	picture_start_code[4];
		
		unsigned short	temporal_reference:10;
		unsigned short	picture_coding_type:3;
		unsigned short	unused_padding_0:3;

		unsigned short	vbv_delay;

		union {
			// picture_coding_type = = 2
			struct {
				unsigned char	full_pel_forward_vector:1;
				unsigned char	forward_f_code:3;
				unsigned char	unused_padding_1:4;
			}PACKED;
			//picture_coding_type = = 3
			struct {
				unsigned char	full_pel_forward_vector_2:1;
				unsigned char	forward_f_code_2:3;
				unsigned char	full_pel_backward_vector:1;
				unsigned char	backward_f_code:3;
			}PACKED;
		}PACKED;

		std::vector<scatterbyte>
						extra_information_picture;

		CPictureHeader(){}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, picture_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, temporal_reference, 10);
				MAP_BST_BITS(in_bst, picture_coding_type, 3);
				MAP_BST_WORD(in_bst, vbv_delay);

				if (picture_coding_type == 2 || picture_coding_type == 3){
					MAP_BST_BITS(in_bst, full_pel_forward_vector, 1);
					MAP_BST_BITS(in_bst, forward_f_code, 3);
				}
			
				if (picture_coding_type == 3){
					MAP_BST_BITS(in_bst, full_pel_backward_vector, 1);
					MAP_BST_BITS(in_bst, backward_f_code, 3);
				}

				int prev_pos = AMBst_Tell(in_bst);
				while(AMBst_PeekBits(in_bst, 1)){
					scatterbyte sb;
					AMBst_GetBits(in_bst, 1);
					sb.leading_bits = AMBst_Tell(in_bst) - prev_pos;
					MAP_BST_BYTE(in_bst, sb.byte_value);
					extra_information_picture.push_back(sb);
					prev_pos = AMBst_Tell(in_bst);
				}
			
				AMBst_SkipBits(in_bst, 1);	// The final extra_bit_picture('0b')
				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("picture_start_code", 32, picture_start_code, 4, "should be 00 00 01 00");
			BST_FIELD_PROP_2NUMBER("temporal_reference", 10, temporal_reference, "")
			BST_FIELD_PROP_2NUMBER("picture_coding_type", 3, picture_coding_type, picture_coding_type_name[picture_coding_type])
			BST_FIELD_PROP_2NUMBER("vbv_delay", 16, vbv_delay, "")
			if (picture_coding_type == 2 || picture_coding_type == 3)
			{
				BST_FIELD_PROP_NUMBER("full_pel_forward_vector", 1, full_pel_forward_vector, "")
				BST_FIELD_PROP_2NUMBER("forward_f_code", 3, forward_f_code, "")
			}

			if(picture_coding_type == 3)
			{
				BST_FIELD_PROP_NUMBER("full_pel_backward_vector", 1, full_pel_backward_vector, "")
				BST_FIELD_PROP_2NUMBER("backward_f_code", 3, backward_f_code, "")
			}

			for(i=0;i<(int)extra_information_picture.size();i++)
			{
				MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "extra_information_picture_%d", i);
				if (bit_offset)*bit_offset += extra_information_picture[i].leading_bits;
				BST_FIELD_PROP_2NUMBER(szTemp4, 8, extra_information_picture[i].byte_value, "")
			}

			if (bit_offset)*bit_offset += 1;
		DECLARE_FIELDPROP_END()	
	};

	struct CPictureCodingExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char	extension_start_code[4] = { 0 };

		unsigned long	extension_start_code_identifier:4;
		unsigned long	f_code_0_0:4;
		unsigned long	f_code_0_1:4;
		unsigned long	f_code_1_0:4;
		unsigned long	f_code_1_1:4;
		unsigned long	intra_dc_precision:2;
		unsigned long	picture_structure:2;

		unsigned char	top_field_first:1;
		unsigned char	frame_pred_frame_dct:1;
		unsigned char	concealment_motion_vectors:1;
		unsigned char	q_scale_type:1;
		unsigned char	intra_vlc_format:1;
		unsigned char	alternate_scan:1;
		unsigned char	repeat_first_field:1;
		unsigned char	chroma_420_type:1;

		unsigned char	progressive_frame:1;
		unsigned char	composite_display_flag:1;
		unsigned char	unused_padding_0:6;

		// if ( composite_display_flag ) {
		unsigned char	v_axis:1;
		unsigned char	field_sequence:3;
		unsigned char	unused_padding_1:2;

		unsigned char	sub_carrier:1;
		unsigned char	burst_amplitude:7;
		
		unsigned char	sub_carrier_phase;
		// }

		CPictureCodingExtension()
			: extension_start_code_identifier(0), f_code_0_0(0), f_code_0_1(0), f_code_1_0(0), f_code_1_1(0), intra_dc_precision(0), picture_structure(0)
			, top_field_first(0), frame_pred_frame_dct(0), concealment_motion_vectors(0), q_scale_type(0), intra_vlc_format(0), alternate_scan(0), repeat_first_field(0), chroma_420_type(0)
			, progressive_frame(0), composite_display_flag(0), unused_padding_0(0)
			, v_axis(0), field_sequence(0), unused_padding_1(0), sub_carrier(0), burst_amplitude(0), sub_carrier_phase(0){
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			extension_start_code_identifier = AMBst_GetBits(in_bst, 4);

			try
			{
				MAP_BST_BEGIN(2);
				MAP_BST_BITS(in_bst, f_code_0_0, 4);
				MAP_BST_BITS(in_bst, f_code_0_1, 4);
				MAP_BST_BITS(in_bst, f_code_1_0, 4);
				MAP_BST_BITS(in_bst, f_code_1_1, 4);
				MAP_BST_BITS(in_bst, intra_dc_precision, 2);
				MAP_BST_BITS(in_bst, picture_structure, 2);

				MAP_BST_BITS(in_bst, top_field_first, 1);
				MAP_BST_BITS(in_bst, frame_pred_frame_dct, 1);
				MAP_BST_BITS(in_bst, concealment_motion_vectors, 1);
				MAP_BST_BITS(in_bst, q_scale_type, 1);
				MAP_BST_BITS(in_bst, intra_vlc_format, 1);
				MAP_BST_BITS(in_bst, alternate_scan, 1);
				MAP_BST_BITS(in_bst, repeat_first_field, 1);
				MAP_BST_BITS(in_bst, chroma_420_type, 1);

				MAP_BST_BITS(in_bst, progressive_frame, 1);
				MAP_BST_BITS(in_bst, composite_display_flag, 1);

				if (composite_display_flag){
					MAP_BST_BITS(in_bst, v_axis, 1);
					MAP_BST_BITS(in_bst, field_sequence, 3);

					MAP_BST_BITS(in_bst, sub_carrier, 1);
					MAP_BST_BITS(in_bst, burst_amplitude, 7);

					MAP_BST_BYTE(in_bst, sub_carrier_phase);
				}

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			BST_FIELD_PROP_2NUMBER("f_code_0_0", 4, f_code_0_0, "forward horizontal")
			BST_FIELD_PROP_2NUMBER("f_code_0_1", 4, f_code_0_1, "forward vertical")
			BST_FIELD_PROP_2NUMBER("f_code_1_0", 4, f_code_1_0, "backward horizontal")
			BST_FIELD_PROP_2NUMBER("f_code_1_1", 4, f_code_1_1, "backward vertical")
			BST_FIELD_PROP_2NUMBER("intra_dc_precision", 2, intra_dc_precision, intra_dc_precision==0?"8bits precision":(
																				intra_dc_precision==1?"9bits precision":(
																				intra_dc_precision==2?"10bits precision":(
																				intra_dc_precision==3?"11bits precision":"Unknown"))))
			BST_FIELD_PROP_2NUMBER("picture_structure", 2, picture_structure, picture_structure==0?"Reserved":(
																				picture_structure==1?"Top Field":(
																				picture_structure==2?"Bottom":(
																				picture_structure==3?"Frame picture":"Unknown"))))
			BST_FIELD_PROP_NUMBER("top_field_first", 1, top_field_first, "The meaning of this element depends upon picture_structure, progressive_sequence and repeat_first_field.")
			BST_FIELD_PROP_NUMBER("frame_pred_frame_dct", 1, frame_pred_frame_dct, frame_pred_frame_dct?"only frame-DCT and frame prediction are used":"")
			BST_FIELD_PROP_NUMBER("concealment_motion_vectors", 1, concealment_motion_vectors, concealment_motion_vectors?"indicate that motion vectors are coded in intra macroblocks":"indicate that no motion vectors are coded in intra macroblocks")
			BST_FIELD_PROP_NUMBER("q_scale_type", 1, q_scale_type, "This flag affects the inverse quantisation process as described in 7.4.2.2")
			BST_FIELD_PROP_NUMBER("intra_vlc_format", 1, intra_vlc_format, "This flag affects the decoding of transform coefficient data as described in 7.2.2.1")
			BST_FIELD_PROP_NUMBER("alternate_scan", 1, alternate_scan, "This flag affects the decoding of transform coefficient data as described in 7.3")
			BST_FIELD_PROP_NUMBER("repeat_first_field", 1, repeat_first_field, "This flag is applicable only in a frame picture; in a field picture it shall be set to zero and does not affect the decoding process")
			BST_FIELD_PROP_NUMBER("chroma_420_type", 1, chroma_420_type, "")
			BST_FIELD_PROP_NUMBER("progressive_frame", 1, progressive_frame, progressive_frame?"the two fields (of the frame) are actually from the same time instant as one another":
																								"the two fields of the frame are interlaced fields in which an interval of time of the field period exists between (corresponding spatial samples) of the two fields")
			BST_FIELD_PROP_NUMBER("composite_display_flag", 1, composite_display_flag, composite_display_flag?"the following fields that are of use when the input pictures have been coded as (analogue) composite video prior to encoding into a bitstream that complies with this Specification":
																												"these parameters do not occur in the bitstream")
			if (composite_display_flag)
			{
				BST_FIELD_PROP_NUMBER("v_axis", 1, v_axis, "A 1-bit integer used only when the bitstream represents a signal that had previously been encoded according to PAL systems. v_axis is set to 1 on a positive sign, v_axis is set to 0 otherwise")
				BST_FIELD_PROP_2NUMBER("field_sequence", 3, field_sequence, "A 3-bit integer which defines the number of the field in the eight field sequence used in PAL systems or the four field sequence used in NTSC systems")
				BST_FIELD_PROP_NUMBER("sub_carrier", 1, sub_carrier, sub_carrier?"the sub-carrier/line frequency relationship is not correct":"the sub-carrier/line frequency relationship is correct")
				BST_FIELD_PROP_2NUMBER("burst_amplitude", 7, burst_amplitude, "This is a 7-bit integer defining the burst amplitude (for PAL and NTSC only). The amplitude of the sub-carrier burst is quantised as a Recommendation ITU-R BT.601 luminance signal, with the MSB omitted.")
				BST_FIELD_PROP_2NUMBER("sub_carrier_phase", 8, sub_carrier_phase, "")
			}
		DECLARE_FIELDPROP_END()	
	};

	struct CQuantMatrixExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char	extension_start_code[4] = { 0 };

		unsigned char	extension_start_code_identifier:4;
		unsigned char	load_intra_quantiser_matrix:1;
		unsigned char	load_non_intra_quantiser_matrix:1;
		unsigned char	load_chroma_intra_quantiser_matrix:1;
		unsigned char	load_chroma_non_intra_quantiser_matrix:1;

		unsigned char	intra_quantiser_matrix[64] = { 0 };
		unsigned char	non_intra_quantiser_matrix[64] = { 0 };
		unsigned char	chroma_intra_quantiser_matrix[64] = { 0 };
		unsigned char	chroma_non_intra_quantiser_matrix[64] = { 0 };

		CQuantMatrixExtension()
			: extension_start_code_identifier(0)
			, load_intra_quantiser_matrix(0), load_non_intra_quantiser_matrix(0), load_chroma_intra_quantiser_matrix(0), load_chroma_non_intra_quantiser_matrix(0) {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, load_intra_quantiser_matrix, 1);
				if (load_intra_quantiser_matrix){
					MAP_BST_BYTES(in_bst, intra_quantiser_matrix, 64);
				}

				MAP_BST_BITS(in_bst, load_non_intra_quantiser_matrix, 1);
				if (load_non_intra_quantiser_matrix){
					MAP_BST_BYTES(in_bst, non_intra_quantiser_matrix, 64);
				}

				MAP_BST_BITS(in_bst, load_chroma_intra_quantiser_matrix, 1);
				if (load_chroma_intra_quantiser_matrix){
					MAP_BST_BYTES(in_bst, chroma_intra_quantiser_matrix, 64);
				}

				MAP_BST_BITS(in_bst, load_chroma_non_intra_quantiser_matrix, 1);
				if (load_chroma_non_intra_quantiser_matrix){
					MAP_BST_BYTES(in_bst, chroma_non_intra_quantiser_matrix, 64);
				}

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			NAV_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			NAV_FIELD_PROP_NUMBER("load_intra_quantiser_matrix", 1, load_intra_quantiser_matrix, load_intra_quantiser_matrix?"intra_quantiser_matrix follows":"there is no change in the values that shall be used")
			if (load_intra_quantiser_matrix){
				NAV_FIELD_PROP_1D_ARRAY_MATRIX1(intra_quantiser_matrix, 8, 8, "")
			}

			NAV_FIELD_PROP_NUMBER("load_non_intra_quantiser_matrix", 1, load_non_intra_quantiser_matrix, load_non_intra_quantiser_matrix?"non_intra_quantiser_matrix follows":"there is no change in the values that shall be used")
			if (load_non_intra_quantiser_matrix){
				NAV_FIELD_PROP_1D_ARRAY_MATRIX1(non_intra_quantiser_matrix, 8, 8, "")
			}

			NAV_FIELD_PROP_2NUMBER("load_chroma_intra_quantiser_matrix", 1, load_chroma_intra_quantiser_matrix, load_chroma_intra_quantiser_matrix?"chroma_intra_quantiser_matrix follows":"there is no change in the values that shall be used")
			if (load_chroma_intra_quantiser_matrix){
				NAV_FIELD_PROP_1D_ARRAY_MATRIX1(chroma_intra_quantiser_matrix, 8, 8, "")
			}

			NAV_FIELD_PROP_2NUMBER("load_chroma_non_intra_quantiser_matrix", 1, load_chroma_non_intra_quantiser_matrix, load_chroma_non_intra_quantiser_matrix?"chroma_intra_non_quantiser_matrix follows":"there is no change in the values that shall be used")
			if (load_chroma_non_intra_quantiser_matrix){
				NAV_FIELD_PROP_1D_ARRAY_MATRIX1(chroma_non_intra_quantiser_matrix, 8, 8, "")
			}
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CPictureDisplayExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char	extension_start_code[4] = { 0 };
		unsigned char	extension_start_code_identifier:4;
		/*
		if ( progressive_sequence = = 1) {
			if ( repeat_first_field = = '1' ) {
				if ( top_field_first = = '1' )
					number_of_frame_centre_offsets = 3
				else
					number_of_frame_centre_offsets = 2
			} else {
				number_of_frame_centre_offsets = 1
			}
		} else {
			if (picture_structure = = "field") {
				number_of_frame_centre_offsets = 1
			} else {
				if (repeat_first_field = = '1' )
					number_of_frame_centre_offsets = 3
				else
					number_of_frame_centre_offsets = 2
			}
		}
		*/
		unsigned char	number_of_frame_centre_offsets:3;
		unsigned char	unused_padding_0:1;

		union
		{
			uint8_t bytes[5];
			struct {
				unsigned short	frame_centre_horizontal_offset;
				unsigned short	frame_centre_vertical_offset;
				unsigned char	marker_bit_0 : 1;
				unsigned char	marker_bit_1 : 1;
				unsigned char	unused_padding : 6;
			}PACKED;
		}PACKED frame_centre_offsets[3];

		CPictureDisplayExtension(unsigned long progressive_sequence, unsigned long repeat_first_field, unsigned long top_field_first, unsigned long picture_structure)
			: extension_start_code_identifier(0), number_of_frame_centre_offsets(0), unused_padding_0(0), frame_centre_offsets{ {{0}} }{
			if ( progressive_sequence ) {
				if ( repeat_first_field ) {
					if ( top_field_first )
						number_of_frame_centre_offsets = 3;
					else
						number_of_frame_centre_offsets = 2;
				} else {
					number_of_frame_centre_offsets = 1;
				}
			} else {
				if (picture_structure == TOP_FIELD || picture_structure == BOTTOM_FIELD) {
					number_of_frame_centre_offsets = 1;
				} else {
					if (repeat_first_field)
						number_of_frame_centre_offsets = 3;
					else
						number_of_frame_centre_offsets = 2;
				}
			}
		}

		CPictureDisplayExtension()
			: extension_start_code_identifier(0), number_of_frame_centre_offsets(0), unused_padding_0(0), frame_centre_offsets{ {{0}} } {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				if (number_of_frame_centre_offsets == 0)
				{
					do
					{
						unsigned long ulTemp = (unsigned long)AMBst_PeekBits(in_bst, 17);
						if(!(ulTemp&0x01))
							break;

						unsigned short frame_centre_horizontal_offset = AMBst_GetWord(in_bst);
						AMBst_GetBits(in_bst, 1);

						ulTemp = (unsigned long)AMBst_PeekBits(in_bst, 17);
						if(!(ulTemp&0x01))
							break;

						unsigned short frame_centre_vertical_offset = AMBst_GetWord(in_bst);
						AMBst_GetBits(in_bst, 1);

						frame_centre_offsets[number_of_frame_centre_offsets].frame_centre_horizontal_offset = frame_centre_horizontal_offset;
						frame_centre_offsets[number_of_frame_centre_offsets].marker_bit_0 = 1;
						frame_centre_offsets[number_of_frame_centre_offsets].frame_centre_vertical_offset = frame_centre_vertical_offset;
						frame_centre_offsets[number_of_frame_centre_offsets].marker_bit_1 = 1;

						number_of_frame_centre_offsets++;
					}while(number_of_frame_centre_offsets < 3);
				}
				else
				{
					for(int i=0;i<number_of_frame_centre_offsets;i++){
						MAP_BST_WORD(in_bst, frame_centre_offsets[i].frame_centre_horizontal_offset);
						MAP_BST_BITS(in_bst, frame_centre_offsets[i].marker_bit_0, 1);
						MAP_BST_WORD(in_bst, frame_centre_offsets[i].frame_centre_vertical_offset);
						MAP_BST_BITS(in_bst, frame_centre_offsets[i].marker_bit_1, 1);
					}
				}

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			for(i=0;i<number_of_frame_centre_offsets;i++)
			{
				NAV_WRITE_TAG_BEGIN3_1("frame_centre_offset", i, "");
				BST_FIELD_PROP_2NUMBER("frame_centre_horizontal_offset", 16, frame_centre_offsets[i].frame_centre_horizontal_offset, "This is a 16-bit signed integer giving the horizontal offset in units of 1/16th sample. A positive value shall indicate that the centre of the reconstructed frame lies to the right of the centre of the display rectangle.")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, frame_centre_offsets[i].marker_bit_0, "")
				BST_FIELD_PROP_2NUMBER("frame_centre_vertical_offset", 16, frame_centre_offsets[i].frame_centre_vertical_offset, "This is a 16-bit signed integer giving the vertical offset in units of 1/16th sample. A positive value shall indicate that the centre of the reconstructed frame lies below the centre of the display rectangle.")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, frame_centre_offsets[i].marker_bit_1, "")
				NAV_WRITE_TAG_END3("frame_centre_offset", i);
			}
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CTemporalScalableExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };

		unsigned short		extension_start_code_identifier:4;
		unsigned short		reference_select_code:2;
		unsigned short		forward_temporal_reference:10;
		
		unsigned short		marker_bit:1;
		unsigned short		backward_temporal_reference:10;
		unsigned short		unused_padding:5;

		CTemporalScalableExtension()
			: extension_start_code_identifier(0), reference_select_code(0), forward_temporal_reference(0)
			, marker_bit(0), backward_temporal_reference(0), unused_padding(0) {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, reference_select_code, 2);
				MAP_BST_BITS(in_bst, forward_temporal_reference, 10);
			
				MAP_BST_BITS(in_bst, marker_bit, 1);
				MAP_BST_BITS(in_bst, backward_temporal_reference, 10);

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			BST_FIELD_PROP_2NUMBER("reference_select_code", 2, reference_select_code, "This is a 2-bit code that identifies reference frames or reference fields for prediction depending on the picture type.")
			BST_FIELD_PROP_2NUMBER("forward_temporal_reference", 10, forward_temporal_reference, "A 10 bit unsigned integer value which indicates temporal reference of the lower layer frame to be used to provide the forward prediction.")
			BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit, "")
			BST_FIELD_PROP_2NUMBER("backward_temporal_reference", 10, backward_temporal_reference, "A 10 bit unsigned integer value which indicates temporal reference of the lower layer frame to be used to provide the backward prediction.")
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CPictureSpatialScalableExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };

		unsigned short		extension_start_code_identifier:4;
		unsigned short		lower_layer_temporal_reference:10;
		unsigned short		unused_padding_0:2;

		unsigned short		marker_bit_0:1;
		unsigned short		lower_layer_horizontal_offset:15;

		unsigned short		marker_bit_1:1;
		unsigned short		lower_layer_vertical_offset:15;

		unsigned char		spatial_temporal_weight_code_table_index:2;
		unsigned char		lower_layer_progressive_frame:1;
		unsigned char		lower_layer_deinterlaced_field_select:1;
		unsigned char		unused_padding_1:4;

		CPictureSpatialScalableExtension()
			: extension_start_code_identifier(0), lower_layer_temporal_reference(0), unused_padding_0(0)
			, marker_bit_0(0), lower_layer_horizontal_offset(0)
			, marker_bit_1(0), lower_layer_vertical_offset(0)
			, spatial_temporal_weight_code_table_index(0)
			, lower_layer_progressive_frame(0)
			, lower_layer_deinterlaced_field_select(0)
			, unused_padding_1(0) {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, lower_layer_temporal_reference, 10);
			
				MAP_BST_BITS(in_bst, marker_bit_0, 1);
				MAP_BST_BITS(in_bst, lower_layer_horizontal_offset, 15);

				MAP_BST_BITS(in_bst, marker_bit_1, 1);
				MAP_BST_BITS(in_bst, lower_layer_vertical_offset, 15);

				MAP_BST_BITS(in_bst, spatial_temporal_weight_code_table_index, 2);
				MAP_BST_BITS(in_bst, lower_layer_progressive_frame, 1);
				MAP_BST_BITS(in_bst, lower_layer_deinterlaced_field_select, 1);

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			BST_FIELD_PROP_2NUMBER("lower_layer_temporal_reference", 10, lower_layer_temporal_reference, "A 10 bit unsigned integer value which indicates temporal reference of the lower layer frame to be used to provide the prediction.")
			BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "")
			BST_FIELD_PROP_2NUMBER("lower_layer_horizontal_offset", 15, lower_layer_horizontal_offset, "This 15 bit signed (twos complement) integer specifies the horizontal offset (of the top left hand corner) of the upsampled lower layer frame relative to the enhancement layer picture.")
			BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "")
			BST_FIELD_PROP_2NUMBER("lower_layer_vertical_offset", 15, lower_layer_vertical_offset, "This 15 bit signed (twos complement) integer specifies the vertical offset (of the top left hand corner) of the upsampled lower layer picture relative to the enhancement layer picture.")
			BST_FIELD_PROP_2NUMBER("spatial_temporal_weight_code_table_index", 2, spatial_temporal_weight_code_table_index, "This 2-bit integer indicates which table of spatial temporal weight codes is to be used as defined in 7.7.")
			BST_FIELD_PROP_NUMBER("lower_layer_progressive_frame", 1, lower_layer_progressive_frame, lower_layer_progressive_frame?"the lower layer frame is progressive":"the lower layer frame is interlaced")
			BST_FIELD_PROP_NUMBER("lower_layer_deinterlaced_field_select", 1, lower_layer_deinterlaced_field_select, "This flag affects the spatial scalable upsampling process, as defined in 7.7.")
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CCopyrightExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };

		unsigned char		extension_start_code_identifier:4;
		unsigned char		copyright_flag:1;
		unsigned char		unused_padding_0:3;

		unsigned char		copyright_identifier = 0;

		unsigned char		original_or_copy:1;
		unsigned char		reserved:7;

		unsigned long		marker_bit_0:1;
		unsigned long		copyright_number_1:20;
		unsigned long		unused_padding_1:11;

		unsigned long		marker_bit_1:1;
		unsigned long		copyright_number_2:22;
		unsigned long		unused_padding_2:9;

		unsigned long		marker_bit_2:1;
		unsigned long		copyright_number_3:22;
		unsigned long		unused_padding_3:9;

		CCopyrightExtension()
			: extension_start_code_identifier(0), copyright_flag(0), unused_padding_0(0)
			, original_or_copy(0), reserved(0)
			, marker_bit_0(0), copyright_number_1(0), unused_padding_1(0)
			, marker_bit_1(0), copyright_number_2(0), unused_padding_2(0)
			, marker_bit_2(0), copyright_number_3(0), unused_padding_3(0) {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, copyright_flag, 1);

				MAP_BST_BYTE(in_bst, copyright_identifier);

				MAP_BST_BITS(in_bst, original_or_copy, 1);
				MAP_BST_BITS(in_bst, reserved, 7);

				MAP_BST_BITS(in_bst, marker_bit_0, 1);
				MAP_BST_BITS(in_bst, copyright_number_1, 20);

				MAP_BST_BITS(in_bst, marker_bit_1, 1);
				MAP_BST_BITS(in_bst, copyright_number_2, 22);

				MAP_BST_BITS(in_bst, marker_bit_2, 1);
				MAP_BST_BITS(in_bst, copyright_number_3, 22);

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			BST_FIELD_PROP_NUMBER("copyright_flag", 1, copyright_flag, copyright_flag?"it indicates that the source video material encoded in all the coded pictures following the copyright extension, in coding order, up to the next copyright extension or end of sequence code, is copyrighted.":
																						"it does not indicate whether the source video material encoded in all the coded pictures following the copyright extension, in coding order, is copyrighted or not")
			BST_FIELD_PROP_2NUMBER("copyright_identifier", 8, copyright_identifier, "This is an 8-bit integer given by a Registration Authority as designated by ISO/IEC JTC1/SC29.")
			BST_FIELD_PROP_NUMBER("original_or_copy", 1, original_or_copy, original_or_copy?"the material is an original":"it is a copy")
			BST_FIELD_PROP_2NUMBER("reserved", 7, reserved, "")
			BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "")
			BST_FIELD_PROP_2NUMBER("copyright_number_1", 20, copyright_number_1, "representing bits 44 to 63 of copyright_number")
			BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "")
			BST_FIELD_PROP_2NUMBER("copyright_number_2", 22, copyright_number_2, "representing bits 22 to 43 of copyright_number")
			BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "")
			BST_FIELD_PROP_2NUMBER("copyright_number_3", 22, copyright_number_3, "representing bits 0 to 21 of copyright_number")
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CCameraParametersExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };
		union
		{
			uint8_t		Bytes[72] = { 0 };
			struct
			{
				unsigned char	extension_start_code_identifier : 4;
				unsigned char	reserved_0 : 1;
				unsigned char	unused_padding_0 : 3;

				unsigned char	camera_id : 7;
				unsigned char	marker_bit_0 : 1;

				unsigned char	aligned_dword[2];

				unsigned long	height_of_image_device : 22;
				unsigned long	marker_bit_1 : 1;
				unsigned long	unused_padding_1 : 9;

				unsigned long	focal_length : 22;
				unsigned long	marker_bit_2 : 1;
				unsigned long	unused_padding_2 : 9;

				unsigned long	f_number : 22;
				unsigned long	marker_bit_3 : 1;
				unsigned long	unused_padding_3 : 9;

				unsigned long	vertical_angle_of_view : 22;
				unsigned long	marker_bit_4 : 1;
				unsigned long	unused_padding_4 : 9;

				unsigned long	camera_position_x_upper : 16;
				unsigned long	marker_bit_5 : 1;
				unsigned long	unused_padding_5 : 15;

				unsigned long	camera_position_x_lower : 16;
				unsigned long	marker_bit_6 : 1;
				unsigned long	unused_padding_6 : 15;

				unsigned long	camera_position_y_upper : 16;
				unsigned long	marker_bit_7 : 1;
				unsigned long	unused_padding_7 : 15;

				unsigned long	camera_position_y_lower : 16;
				unsigned long	marker_bit_8 : 1;
				unsigned long	unused_padding_8 : 15;

				unsigned long	camera_position_z_upper : 16;
				unsigned long	marker_bit_9 : 1;
				unsigned long	unused_padding_9 : 15;

				unsigned long	camera_position_z_lower : 16;
				unsigned long	marker_bit_10 : 1;
				unsigned long	unused_padding_10 : 15;

				unsigned long	camera_direction_x : 22;
				unsigned long	marker_bit_11 : 1;
				unsigned long	unused_padding_11 : 9;

				unsigned long	camera_direction_y : 22;
				unsigned long	marker_bit_12 : 1;
				unsigned long	unused_padding_12 : 9;

				unsigned long	camera_direction_z : 22;
				unsigned long	marker_bit_13 : 1;
				unsigned long	unused_padding_13 : 9;

				unsigned long	image_plane_vertical_x : 22;
				unsigned long	marker_bit_14 : 1;
				unsigned long	unused_padding_14 : 9;

				unsigned long	image_plane_vertical_y : 22;
				unsigned long	marker_bit_15 : 1;
				unsigned long	unused_padding_15 : 9;

				unsigned long	image_plane_vertical_z : 22;
				unsigned long	marker_bit_16 : 1;
				unsigned long	unused_padding_16 : 9;

				unsigned long	reserved_1;
			}PACKED;
		}PACKED;

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);
				MAP_BST_BITS(in_bst, reserved_0, 1);

				MAP_BST_BITS(in_bst, camera_id, 7);
				MAP_BST_BITS(in_bst, marker_bit_0, 1);

				MAP_BST_BITS(in_bst, height_of_image_device, 22);
				MAP_BST_BITS(in_bst, marker_bit_1, 1);

				MAP_BST_BITS(in_bst, focal_length, 22);
				MAP_BST_BITS(in_bst, marker_bit_2, 1);

				MAP_BST_BITS(in_bst, f_number, 22);
				MAP_BST_BITS(in_bst, marker_bit_3, 1);

				MAP_BST_BITS(in_bst, vertical_angle_of_view, 22);
				MAP_BST_BITS(in_bst, marker_bit_4, 1);

				MAP_BST_BITS(in_bst, camera_position_x_upper, 16);
				MAP_BST_BITS(in_bst, marker_bit_5, 1);

				MAP_BST_BITS(in_bst, camera_position_x_lower, 16);
				MAP_BST_BITS(in_bst, marker_bit_6, 1);

				MAP_BST_BITS(in_bst, camera_position_y_upper, 16);
				MAP_BST_BITS(in_bst, marker_bit_7, 1);

				MAP_BST_BITS(in_bst, camera_position_y_lower, 16);
				MAP_BST_BITS(in_bst, marker_bit_8, 1);

				MAP_BST_BITS(in_bst, camera_position_z_upper, 16);
				MAP_BST_BITS(in_bst, marker_bit_9, 1);

				MAP_BST_BITS(in_bst, camera_position_z_lower, 16);
				MAP_BST_BITS(in_bst, marker_bit_10, 1);

				MAP_BST_BITS(in_bst, camera_direction_x, 22);
				MAP_BST_BITS(in_bst, marker_bit_11, 1);

				MAP_BST_BITS(in_bst, camera_direction_y, 22);
				MAP_BST_BITS(in_bst, marker_bit_12, 1);

				MAP_BST_BITS(in_bst, camera_direction_z, 22);
				MAP_BST_BITS(in_bst, marker_bit_13, 1);

				MAP_BST_BITS(in_bst, image_plane_vertical_x, 22);
				MAP_BST_BITS(in_bst, marker_bit_14, 1);

				MAP_BST_BITS(in_bst, image_plane_vertical_y, 22);
				MAP_BST_BITS(in_bst, marker_bit_15, 1);

				MAP_BST_BITS(in_bst, image_plane_vertical_z, 22);
				MAP_BST_BITS(in_bst, marker_bit_16, 1);

				next_start_code(in_bst);

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			BST_FIELD_PROP_NUMBER ("reserved", 1, reserved_0, "")
			
			BST_FIELD_PROP_2NUMBER("camera_id", 7, camera_id, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_0, "")
			
			BST_FIELD_PROP_2NUMBER("height_of_image_device",22, height_of_image_device, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_1, "")

			BST_FIELD_PROP_2NUMBER("focal_length",22, focal_length, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_2, "")

			BST_FIELD_PROP_2NUMBER("f_number",22, f_number, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_3, "")

			BST_FIELD_PROP_2NUMBER("vertical_angle_of_view",22, f_number, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_4, "")

			BST_FIELD_PROP_2NUMBER("camera_position_x_upper",16, camera_position_x_upper, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_5, "")

			BST_FIELD_PROP_2NUMBER("camera_position_x_lower",16, camera_position_x_lower, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_6, "")

			BST_FIELD_PROP_2NUMBER("camera_position_y_upper",16, camera_position_y_upper, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_7, "")

			BST_FIELD_PROP_2NUMBER("camera_position_y_lower",16, camera_position_y_lower, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_8, "")

			BST_FIELD_PROP_2NUMBER("camera_position_z_upper",16, camera_position_z_upper, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_9, "")

			BST_FIELD_PROP_2NUMBER("camera_position_z_lower",16, camera_position_z_lower, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_10, "")

			BST_FIELD_PROP_2NUMBER("camera_direction_x",22, camera_direction_x, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_11, "")

			BST_FIELD_PROP_2NUMBER("camera_direction_y",22, camera_direction_y, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_12, "")

			BST_FIELD_PROP_2NUMBER("camera_direction_z",22, camera_direction_z, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_13, "")

			BST_FIELD_PROP_2NUMBER("image_plane_vertical_x",22, image_plane_vertical_x, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_14, "")

			BST_FIELD_PROP_2NUMBER("image_plane_vertical_y",22, image_plane_vertical_y, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_15, "")

			BST_FIELD_PROP_2NUMBER("image_plane_vertical_z",22, image_plane_vertical_z, "")
			BST_FIELD_PROP_NUMBER ("marker_bit", 1, marker_bit_16, "")

		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CITUTExtension: public SYNTAX_BITSTREAM_MAP
	{
		unsigned char		extension_start_code[4] = { 0 };
		unsigned char		extension_start_code_identifier:4;
		unsigned char		unused_padding:4;
		CAMBitArray			ITUTdata;

		CITUTExtension()
			: extension_start_code_identifier(0), unused_padding(0) {
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			AMBst_GetBytes(in_bst, extension_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				MAP_BST_BITS(in_bst, extension_start_code_identifier, 4);

				int idx = 0;
				while (AMBst_PeekBits(in_bst, 24) != 1)
				{
					AMBst_GetBits(in_bst, 1)?ITUTdata.BitSet(idx):ITUTdata.BitClear(idx);
					idx++;
					map_status.number_of_fields++;
				}

				// Don't need call next_start_code
				// Here it should be start_code or bit stream reaches EOS

				MAP_BST_END();
			}
			catch(AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			BST_FIELD_PROP_FIXSIZE_BINSTR("extension_start_code", 32, extension_start_code, 4, "should be 00 00 01 B5");
			BST_FIELD_PROP_2NUMBER("extension_start_code_identifier", 4, extension_start_code_identifier, "")
			for(i=0;i<=ITUTdata.UpperBound();i++)
			{
				BST_FIELD_PROP_NUMBER("ITU_T_data", 1, ITUTdata[i], "")
			}
		DECLARE_FIELDPROP_END()	
	};

	struct CContentDescriptionData : public SYNTAX_BITSTREAM_MAP
	{
		struct CPaddingBytes : public SYNTAX_BITSTREAM_MAP
		{
			CAMBitArray				marker_bit;
			std::vector<uint8_t>	padding_byte;
			CPaddingBytes(uint8_t data_length)
			{
				padding_byte.reserve(data_length);
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);

					for (int i = 0; i < (int)padding_byte.size(); i++)
					{
						bsrbarray(in_bst, marker_bit, i);
						bsrb1(in_bst, padding_byte[i], 8);
					}

					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				for (i = 0; i <= (int)padding_byte.size(); i++)
				{
					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit[i], "");
					BST_FIELD_PROP_2NUMBER("padding_byte", 8, padding_byte[i], "");
				}
			DECLARE_FIELDPROP_END()

		};

		struct CCaptureTimeCode : public SYNTAX_BITSTREAM_MAP
		{
			struct CFrameOrFieldCaptureTimeStamp : public SYNTAX_BITSTREAM_MAP
			{
				uint8_t			counting_type;

				uint8_t			marker_bit_0 : 1;
				uint8_t			reserved_0 : 7;

				uint8_t			nframes;

				uint8_t			marker_bit_1 : 1;
				uint8_t			reserved_1 : 7;

				uint8_t			time_discontinuity : 1;
				uint8_t			prior_count_dropped : 1;
				uint8_t			time_offset_part_a : 6;

				uint8_t			marker_bit_2 : 1;
				uint8_t			reserved_2 : 7;

				uint8_t			time_offset_part_b;

				uint8_t			marker_bit_3 : 1;
				uint8_t			reserved_3 : 7;

				uint8_t			time_offset_part_c;

				uint8_t			marker_bit_4 : 1;
				uint8_t			reserved_4 : 7;

				uint8_t			time_offset_part_d;

				uint8_t			marker_bit_5 : 1;
				uint8_t			reserved_5 : 7;

				uint8_t			units_of_seconds : 4;
				uint8_t			tens_of_seconds : 4;

				uint8_t			marker_bit_6 : 1;
				uint8_t			reserved_6 : 7;

				uint8_t			units_of_minutes : 4;
				uint8_t			tens_of_minutes : 4;

				uint8_t			marker_bit_7 : 1;
				uint8_t			reserved_7 : 7;

				uint8_t			units_of_hours : 4;
				uint8_t			tens_of_hours : 4;

				CFrameOrFieldCaptureTimeStamp(uint8_t nCountingType): counting_type(nCountingType)
				{
				}

				int Map(AMBst in_bst)
				{
					SYNTAX_BITSTREAM_MAP::Map(in_bst);
					try
					{
						MAP_BST_BEGIN(0);
						if (counting_type != 0)
						{
							bsrb(in_bst, marker_bit_0, 1);
							bsrb(in_bst, nframes, 8);
						}

						bsrb(in_bst, marker_bit_1, 1);
						bsrb(in_bst, time_discontinuity, 1);
						bsrb(in_bst, prior_count_dropped, 1);
						bsrb(in_bst, time_offset_part_a, 6);

						bsrb(in_bst, marker_bit_2, 1);
						bsrb(in_bst, time_offset_part_b, 8);

						bsrb(in_bst, marker_bit_3, 1);
						bsrb(in_bst, time_offset_part_c, 8);

						bsrb(in_bst, marker_bit_4, 1);
						bsrb(in_bst, time_offset_part_d, 8);

						bsrb(in_bst, marker_bit_5, 1);
						bsrb(in_bst, units_of_seconds, 4);
						bsrb(in_bst, tens_of_seconds, 4);

						bsrb(in_bst, marker_bit_6, 1);
						bsrb(in_bst, units_of_minutes, 4);
						bsrb(in_bst, tens_of_minutes, 4);

						bsrb(in_bst, marker_bit_7, 1);
						bsrb(in_bst, units_of_hours, 4);
						bsrb(in_bst, tens_of_hours, 4);

						MAP_BST_END();
					}
					catch (AMException e)
					{
						return e.RetCode();
					}

					return RET_CODE_SUCCESS;
				}

				int Unmap(AMBst out_bst)
				{
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					if (counting_type != 0)
					{
						BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "");
						BST_FIELD_PROP_2NUMBER1(nframes, 8, "the number of frame time increments to add in deriving the equivalent timestamp");
					}

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "");
					BST_FIELD_PROP_BOOL(time_discontinuity, 
						"the time difference that can be calculated between the current and previous timestamps has no defined meaning", 
						"the time difference that can be calculated between the current and previous timestamps is the ideal display duration of the previous frame or field");

					BST_FIELD_PROP_BOOL(prior_count_dropped, 
						"the counting of one or more values of the nframes parameter was dropped in order to reduce drift accumulation in the remaining parameters of the timestamp", 
						"");

					BST_FIELD_PROP_2NUMBER1(time_offset_part_a, 6, "the most significant bits of time_offset");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "");
					BST_FIELD_PROP_2NUMBER1(time_offset_part_b, 8, "the second most significant bits of time_offset");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_3, "");
					BST_FIELD_PROP_2NUMBER1(time_offset_part_c, 8, "the third most significant bits of time_offset");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_4, "");
					BST_FIELD_PROP_2NUMBER1(time_offset_part_d, 8, "the least significant bits of time_offset");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_5, "");
					BST_FIELD_PROP_2NUMBER1(units_of_seconds, 4, "0 ~ 9");
					BST_FIELD_PROP_2NUMBER1(tens_of_seconds, 4, "0 ~ 5");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_6, "");
					BST_FIELD_PROP_2NUMBER1(units_of_minutes, 4, "0 ~ 9");
					BST_FIELD_PROP_2NUMBER1(tens_of_minutes, 4, "0 ~ 5");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_7, "");
					BST_FIELD_PROP_2NUMBER1(units_of_hours, 4, "0 ~ 9");
					BST_FIELD_PROP_2NUMBER1(tens_of_hours, 4, "0 ~ 2");

				DECLARE_FIELDPROP_END()

			}PACKED;

			uint8_t					marker_bit_0 : 1;
			uint8_t					timecode_type : 2;
			uint8_t					counting_type : 3;
			uint8_t					reserved_0 : 1;
			uint8_t					reserved_1 : 1;

			uint8_t					reserved_2 : 1;
			uint8_t					marker_bit_1 : 1;
			uint8_t					nframes_conversion_code : 1;
			uint8_t					padding_0 : 5;

			uint8_t					clock_divisor : 7;
			uint8_t					marker_bit_2 : 1;

			uint8_t					nframes_multiplier_upper;
			
			uint8_t					marker_bit_3 : 1;
			uint8_t					padding_1 : 7;

			uint8_t					nframes_multiplier_lower;

			CFrameOrFieldCaptureTimeStamp*
									frame_or_field_capture_timestamp1;
			CFrameOrFieldCaptureTimeStamp*
									frame_or_field_capture_timestamp2;

			CCaptureTimeCode()
				: frame_or_field_capture_timestamp1(NULL)
				, frame_or_field_capture_timestamp2(NULL) {
			}

			~CCaptureTimeCode()
			{
				AMP_SAFEDEL2(frame_or_field_capture_timestamp1);
				AMP_SAFEDEL2(frame_or_field_capture_timestamp2);
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_IGNORE_REQUEST;
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);

					bsrb(in_bst, marker_bit_0, 1);
					bsrb(in_bst, timecode_type, 2);
					bsrb(in_bst, counting_type, 3);
					bsrb(in_bst, reserved_0, 1);
					bsrb(in_bst, reserved_1, 1);
					bsrb(in_bst, reserved_2, 1);
					
					if (counting_type != 0)
					{
						bsrb(in_bst, marker_bit_1, 1);
						bsrb(in_bst, nframes_conversion_code, 1);

						bsrb(in_bst, clock_divisor, 7);
						bsrb(in_bst, marker_bit_2, 1);

						bsrb(in_bst, nframes_multiplier_upper, 8);
						bsrb(in_bst, marker_bit_3, 1);

						bsrb(in_bst, nframes_multiplier_lower, 8);
					}

					AMP_NEWT(frame_or_field_capture_timestamp1, CFrameOrFieldCaptureTimeStamp, counting_type);
					if (frame_or_field_capture_timestamp1 == NULL)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED(iRet = frame_or_field_capture_timestamp1->Map(in_bst)))
					{
						AMP_SAFEDEL(frame_or_field_capture_timestamp1);
						printf("[MP2V] Failed to map frame_or_field_capture_timestamp in capture_timecode {retcode: %d}.\n", iRet);
						throw AMException(RET_CODE_MISMATCH);
					}

					if (timecode_type != 3)
					{
						AMP_NEWT(frame_or_field_capture_timestamp2, CFrameOrFieldCaptureTimeStamp, counting_type);
						if (frame_or_field_capture_timestamp2 == NULL)
							throw AMException(RET_CODE_OUTOFMEMORY);

						if (AMP_FAILED(iRet = frame_or_field_capture_timestamp2->Map(in_bst)))
						{
							AMP_SAFEDEL(frame_or_field_capture_timestamp2);
							printf("[MP2V] Failed to map frame_or_field_capture_timestamp in capture_timecode {retcode: %d}.\n", iRet);
							throw AMException(RET_CODE_MISMATCH);
						}
					}

					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "");
				BST_FIELD_PROP_2NUMBER1(timecode_type, 2, mpeg2_video_timecode_type_names[timecode_type]);
				BST_FIELD_PROP_2NUMBER1(counting_type, 3, mpeg2_video_counting_type_names[counting_type]);
				BST_FIELD_PROP_NUMBER("reserved_bit", 1, reserved_0, "");
				BST_FIELD_PROP_NUMBER("reserved_bit", 1, reserved_1, "");
				BST_FIELD_PROP_NUMBER("reserved_bit", 1, reserved_2, "");

				if (counting_type != 0)
				{
					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "");
					BST_FIELD_PROP_NUMBER1(nframes_conversion_code, 1, "indicates a conversion factor to be used in determining the amount of time indicated by the nframes parameters of each frame or field capture timestamp. The factor specified is 1000 + nframes_conversion_code");
					BST_FIELD_PROP_2NUMBER1(clock_divisor, 7, "the number of divisions of the 27 MHz system clock to be applied for generating the equivalent timestamp for each frame or field capture timestamp");
					
					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "");
					BST_FIELD_PROP_2NUMBER1(nframes_multiplier_upper, 8, "");

					BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_3, "");
					BST_FIELD_PROP_2NUMBER1(nframes_multiplier_lower, 8, "");
				}

				if (frame_or_field_capture_timestamp1) {
					NAV_FIELD_PROP_REF2(frame_or_field_capture_timestamp1, "frame_or_field_capture_timestamp");
				}

				if (timecode_type == 3 && frame_or_field_capture_timestamp2)
				{
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "if (timecode_type == '11')", "");
					NAV_FIELD_PROP_REF2(frame_or_field_capture_timestamp2, "frame_or_field_capture_timestamp");
					NAV_WRITE_TAG_END("Tag0");
				}

			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CAdditionalPanScanParameters : public SYNTAX_BITSTREAM_MAP
		{
			uint8_t				marker_bit_0 : 1;
			uint8_t				padding_0 : 7;

			uint8_t				aspect_ratio_information : 4;
			uint8_t				reserved_0 : 1;
			uint8_t				reserved_1 : 1;
			uint8_t				reserved_2 : 1;
			uint8_t				display_size_present : 1;

			uint8_t				marker_bit_1 : 1;
			uint8_t				padding_1 : 7;
			uint8_t				reserved_3 : 1;
			uint8_t				reserved_4 : 1;
			uint8_t				display_horizontal_size_upper : 6;

			uint8_t				marker_bit_2 : 1;
			uint8_t				padding_2 : 7;

			uint8_t				display_horizontal_size_lower;

			uint8_t				marker_bit_3 : 1;
			uint8_t				padding_3 : 7;

			uint8_t				reserved_5 : 1;
			uint8_t				reserved_6 : 1;
			uint8_t				display_vertical_size_upper : 6;

			uint8_t				marker_bit_4 : 1;
			uint8_t				padding_4 : 7;

			uint8_t				display_vertical_size_lower;

			uint8_t				number_of_frame_centre_offsets;

			struct {
				uint8_t				marker_bit_0 : 1;
				uint8_t				padding_0 : 7;

				uint8_t				frame_centre_horizontal_offset_upper;

				uint8_t				marker_bit_1 : 1;
				uint8_t				padding_1 : 7;

				uint8_t				frame_centre_horizontal_offset_lower;

				uint8_t				marker_bit_2 : 1;
				uint8_t				padding_2 : 7;

				uint8_t				frame_centre_vertical_offset_upper;

				uint8_t				marker_bit_3 : 1;
				uint8_t				padding_3 : 7;

				uint8_t				frame_centre_vertical_offset_lower;
			}PACKED frame_centre_offsets[3];

			CAdditionalPanScanParameters(unsigned long progressive_sequence, unsigned long repeat_first_field, unsigned long top_field_first, unsigned long picture_structure) {
				if (progressive_sequence) {
					if (repeat_first_field) {
						if (top_field_first)
							number_of_frame_centre_offsets = 3;
						else
							number_of_frame_centre_offsets = 2;
					}
					else {
						number_of_frame_centre_offsets = 1;
					}
				}
				else {
					if (picture_structure == TOP_FIELD || picture_structure == BOTTOM_FIELD) {
						number_of_frame_centre_offsets = 1;
					}
					else {
						if (repeat_first_field)
							number_of_frame_centre_offsets = 3;
						else
							number_of_frame_centre_offsets = 2;
					}
				}
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					bsrb(in_bst, marker_bit_0, 1);
					bsrb(in_bst, aspect_ratio_information, 4);
					bsrb(in_bst, reserved_0, 1);
					bsrb(in_bst, reserved_1, 1);
					bsrb(in_bst, reserved_2, 1);
					bsrb(in_bst, display_size_present, 1);

					if (display_size_present)
					{
						bsrb(in_bst, marker_bit_1, 1);
						bsrb(in_bst, reserved_3, 1);
						bsrb(in_bst, reserved_4, 1);
						bsrb(in_bst, display_horizontal_size_upper, 6);
						bsrb(in_bst, marker_bit_2, 1);
						bsrb(in_bst, display_horizontal_size_lower, 8);

						bsrb(in_bst, marker_bit_3, 1);
						bsrb(in_bst, reserved_5, 1);
						bsrb(in_bst, reserved_6, 1);
						bsrb(in_bst, display_vertical_size_upper, 6);
						bsrb(in_bst, marker_bit_4, 1);
						bsrb(in_bst, display_vertical_size_lower, 8);
					}

					for (int i = 0; i < AMP_MIN(3, number_of_frame_centre_offsets); i++)
					{
						bsrb(in_bst, frame_centre_offsets[i].marker_bit_0, 1);
						bsrb(in_bst, frame_centre_offsets[i].frame_centre_horizontal_offset_upper, 8);
						bsrb(in_bst, frame_centre_offsets[i].marker_bit_1, 1);
						bsrb(in_bst, frame_centre_offsets[i].frame_centre_horizontal_offset_lower, 8);

						bsrb(in_bst, frame_centre_offsets[i].marker_bit_2, 1);
						bsrb(in_bst, frame_centre_offsets[i].frame_centre_vertical_offset_upper, 8);
						bsrb(in_bst, frame_centre_offsets[i].marker_bit_3, 1);
						bsrb(in_bst, frame_centre_offsets[i].frame_centre_vertical_offset_lower, 8);
					}

					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CActiveRegionWindow : public SYNTAX_BITSTREAM_MAP
		{
			uint8_t			marker_bit_0 : 1;
			uint8_t			padding_0 : 7;

			uint8_t			top_left_x_upper;

			uint8_t			marker_bit_1 : 1;
			uint8_t			padding_1 : 7;

			uint8_t			top_left_x_lower;

			uint8_t			marker_bit_2 : 1;
			uint8_t			padding_2 : 7;

			uint8_t			top_left_y_upper;

			uint8_t			marker_bit_3 : 1;
			uint8_t			padding_3 : 7;

			uint8_t			top_left_y_lower;

			uint8_t			marker_bit_4 : 1;
			uint8_t			padding_4 : 7;

			uint8_t			active_horizontal_size_upper;

			uint8_t			marker_bit_5 : 1;
			uint8_t			padding_5 : 7;

			uint8_t			active_horizontal_size_lower;

			uint8_t			marker_bit_6 : 1;
			uint8_t			padding_6 : 7;

			uint8_t			active_vertical_size_upper;

			uint8_t			marker_bit_7 : 1;
			uint8_t			padding_7 : 7;

			uint8_t			active_vertical_size_lower;

			CActiveRegionWindow()
				: marker_bit_0(0), padding_0(0), top_left_x_upper(0)
				, marker_bit_1(0), padding_1(0), top_left_x_lower(0)
				, marker_bit_2(0), padding_2(0), top_left_y_upper(0)
				, marker_bit_3(0), padding_3(0), top_left_y_lower(0)
				, marker_bit_4(0), padding_4(0), active_horizontal_size_upper(0)
				, marker_bit_5(0), padding_5(0), active_horizontal_size_lower(0)
				, marker_bit_6(0), padding_6(0), active_vertical_size_upper(0)
				, marker_bit_7(0), padding_7(0), active_vertical_size_lower(0){
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);

					bsrb(in_bst, marker_bit_0, 1);
					bsrb(in_bst, top_left_x_upper, 8);

					bsrb(in_bst, marker_bit_1, 1);
					bsrb(in_bst, top_left_x_lower, 8);
					
					bsrb(in_bst, marker_bit_2, 1);
					bsrb(in_bst, top_left_y_upper, 8);

					bsrb(in_bst, marker_bit_3, 1);
					bsrb(in_bst, top_left_y_lower, 8);

					bsrb(in_bst, marker_bit_4, 1);
					bsrb(in_bst, active_horizontal_size_upper, 8);

					bsrb(in_bst, marker_bit_5, 1);
					bsrb(in_bst, active_horizontal_size_lower, 8);

					bsrb(in_bst, marker_bit_6, 1);
					bsrb(in_bst, active_vertical_size_upper, 8);

					bsrb(in_bst, marker_bit_7, 1);
					bsrb(in_bst, active_vertical_size_lower, 8);

					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "")
				BST_FIELD_PROP_2NUMBER1(top_left_x_upper, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "")
				BST_FIELD_PROP_2NUMBER1(top_left_x_lower, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "")
				BST_FIELD_PROP_2NUMBER1(top_left_y_upper, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_3, "")
				BST_FIELD_PROP_2NUMBER1(top_left_y_lower, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_4, "")
				BST_FIELD_PROP_2NUMBER1(active_horizontal_size_upper, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_5, "")
				BST_FIELD_PROP_2NUMBER1(active_horizontal_size_lower, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_6, "")
				BST_FIELD_PROP_2NUMBER1(active_vertical_size_upper, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_7, "")
				BST_FIELD_PROP_2NUMBER1(active_vertical_size_lower, 8, "")

				NAV_WRITE_TAG_WITH_NUMBER_VALUE("top_left_x", top_left_x_upper << 8 | top_left_x_lower, "");
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("top_left_y", top_left_y_upper << 8 | top_left_y_lower, "");
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("active_horizontal_size", active_horizontal_size_upper << 8 | active_horizontal_size_lower, "");
				NAV_WRITE_TAG_WITH_NUMBER_VALUE("active_vertical_size", active_vertical_size_upper << 8 | active_vertical_size_lower, "");
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CCodedPictureLength : public SYNTAX_BITSTREAM_MAP
		{
			uint8_t			marker_bit_0 : 1;
			uint8_t			padding_0 : 7;
			uint8_t			picture_byte_count_part_a;

			uint8_t			marker_bit_1 : 1;
			uint8_t			padding_1 : 7;
			uint8_t			picture_byte_count_part_b;

			uint8_t			marker_bit_2 : 1;
			uint8_t			padding_2 : 7;
			uint8_t			picture_byte_count_part_c;

			uint8_t			marker_bit_3 : 1;
			uint8_t			padding_3 : 7;
			uint8_t			picture_byte_count_part_d;

			CCodedPictureLength()
				: marker_bit_0(0), padding_0(0), picture_byte_count_part_a(0)
				, marker_bit_1(0), padding_1(0), picture_byte_count_part_b(0)
				, marker_bit_2(0), padding_2(0), picture_byte_count_part_c(0)
				, marker_bit_3(0), padding_3(0), picture_byte_count_part_d(0) {
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					bsrb(in_bst, marker_bit_0, 1);
					bsrb(in_bst, picture_byte_count_part_a, 8);
					bsrb(in_bst, marker_bit_1, 1);
					bsrb(in_bst, picture_byte_count_part_b, 8);
					bsrb(in_bst, marker_bit_2, 1);
					bsrb(in_bst, picture_byte_count_part_c, 8);
					bsrb(in_bst, marker_bit_3, 1);
					bsrb(in_bst, picture_byte_count_part_d, 8);
					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "")
				BST_FIELD_PROP_2NUMBER1(picture_byte_count_part_a, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "")
				BST_FIELD_PROP_2NUMBER1(picture_byte_count_part_b, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "")
				BST_FIELD_PROP_2NUMBER1(picture_byte_count_part_c, 8, "")
				BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_3, "")
				BST_FIELD_PROP_2NUMBER1(picture_byte_count_part_d, 8, "")
			DECLARE_FIELDPROP_END()

		}PACKED;

		uint8_t			data_type_upper;
		uint8_t			marker_bit_0 : 1;
		uint8_t			padding_0 : 7;
		uint8_t			data_type_lower;
		uint8_t			marker_bit_1 : 1;
		uint8_t			padding_1 : 7;
		uint8_t			data_length;

		union
		{
			CPaddingBytes*			ptr_padding_bytes;
			CCaptureTimeCode*		ptr_capture_timecode;
			CAdditionalPanScanParameters*
									ptr_additional_pan_scan_parameters;
			CActiveRegionWindow*	ptr_active_region_window;
			CCodedPictureLength*	ptr_coded_picture_length;
		};

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_IGNORE_REQUEST;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			try
			{
				MAP_BST_BEGIN(0);

				bsrb(in_bst, data_type_upper, 8);
				bsrb(in_bst, marker_bit_0, 1);
				bsrb(in_bst, data_type_lower, 8);
				bsrb(in_bst, marker_bit_1, 1);

				bsrb(in_bst, data_length, 8);

				AMBst data_bst = AMBst_Subset(in_bst, data_length * 9);

				uint16_t data_type = data_type_upper << 8 | data_type_lower;
				switch (data_type)
				{
				case 0x0002:	// Capture Timecode
					AMP_NEWT(ptr_capture_timecode, CCaptureTimeCode);
					if (ptr_capture_timecode == NULL)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED((iRet = ptr_capture_timecode->Map(data_bst))))
						printf("[M2PV] Failed to map Capture Timecode in Content description data of picture header {retcode: %d}.\n", iRet);
					break;
				case 0x0003:	// Additional Pan-Scan Parameters
	#if 0
					AMP_NEWT(ptr_capture_timecode, CAdditionalPanScanParameters);
					if (ptr_capture_timecode == NULL)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED((iRet = ptr_capture_timecode->Map(data_bst))))
						printf("[M2PV] Failed to map Capture Timecode in Content description data of picture header {retcode: %d}.\n"), iRet);
	#endif
					break;
				case 0x0004:	// Active Region Window
					AMP_NEWT(ptr_active_region_window, CActiveRegionWindow);
					if (ptr_active_region_window == NULL)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED((iRet = ptr_active_region_window->Map(data_bst))))
						printf("[M2PV] Failed to map Active Region Window in Content description data of picture header {retcode: %d}.\n", iRet);
					break;
				case 0x0005:	// Coded Picture Length
					AMP_NEWT(ptr_coded_picture_length, CCodedPictureLength);
					if (ptr_coded_picture_length == NULL)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED((iRet = ptr_coded_picture_length->Map(data_bst))))
						printf("[M2PV] Failed to map Coded Picture Length in Content description data of picture header {retcode: %d}.\n", iRet);
					break;
				case 0x0001:	// Padding Bytes
				default:
					AMP_NEWT(ptr_padding_bytes, CPaddingBytes, data_length);
					if (ptr_padding_bytes == NULL)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED((iRet = ptr_padding_bytes->Map(data_bst))))
						printf("[M2PV] Failed to map %s in Content description data of picture header {retcode: %d}.\n", data_type==0x0001?_T("Padding Bytes"):_T("Reserved data"), iRet);
				}

				AMBst_Destroy(data_bst);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return iRet;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CMacroblockModes: public SYNTAX_BITSTREAM_MAP
	{
		vlclbf			macroblock_type;
		unsigned char	spatial_temporal_weight_code:2;
		unsigned char	frame_motion_type:2;
		unsigned char	field_motion_type:2;
		unsigned char	dct_type:1;
		unsigned char	unused_padding:1;

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CMotionVector: public SYNTAX_BITSTREAM_MAP
	{
		vlclbf			motion_code_0;
		varbits			motion_residual_0;
		vlclbf			dmvector_0;
		vlclbf			motion_code_1;
		varbits			motion_residual_1;
		vlclbf			dmvector_1;

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		DECLARE_FIELDPROP_END()	
	}PACKED;

	struct CExtensionDataItem : public SYNTAX_BITSTREAM_MAP
	{
		int							extension_data_i = -1;
		unsigned long				extension_start_code = 0;
		unsigned long				extension_id = 0;
		union
		{
			void*						ptr_extension_data = nullptr;
			CSequenceExtension*			ptr_sequence_extension;
			CSequenceDisplayExtension*	ptr_sequence_display_extension;
			CSequenceScalableExtension*	ptr_sequence_scalable_extension;
			CQuantMatrixExtension*		ptr_quant_matrix_extension;
			CCopyrightExtension*		ptr_copyright_extension;
			CPictureDisplayExtension*	ptr_picture_display_extension;
			CPictureCodingExtension*	ptr_picture_coding_extension;
			CPictureSpatialScalableExtension*
										ptr_picture_spatial_scalable_extension;
			CTemporalScalableExtension*	ptr_picture_temporal_scalable_extension;
			CCameraParametersExtension*	ptr_camera_parameter_extension;
			CITUTExtension*				ptr_ITU_T_extension;
		}PACKED;

		CExtensionDataItem(){}

		virtual ~CExtensionDataItem()
		{
			switch (extension_id)
			{
			case SEQUENCE_EXTENSION_ID: AMP_SAFEDEL2(ptr_sequence_extension); break;
			case SEQUENCE_DISPLAY_EXTENSION_ID: AMP_SAFEDEL2(ptr_sequence_display_extension); break;
			case QUANT_MATRIX_EXTENSION_ID: AMP_SAFEDEL2(ptr_quant_matrix_extension); break;
			case COPYRIGHT_EXTENSION_ID: AMP_SAFEDEL2(ptr_copyright_extension); break;
			case SEQUENCE_SCALABLE_EXTENSION_ID: AMP_SAFEDEL2(ptr_sequence_scalable_extension); break;
			case PICTURE_DISPLAY_EXTENSION_ID: AMP_SAFEDEL2(ptr_picture_display_extension); break;
			case PICTURE_CODING_EXTENSION_ID: AMP_SAFEDEL2(ptr_picture_coding_extension); break;
			case PICTURE_SPATIAL_SCALABLE_EXTENSION_ID: AMP_SAFEDEL2(ptr_picture_spatial_scalable_extension); break;
			case PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID: AMP_SAFEDEL2(ptr_picture_temporal_scalable_extension); break;
			case CAMERA_PARAMETERS_EXTENSION_ID: AMP_SAFEDEL2(ptr_camera_parameter_extension); break;
			case ITUT_EXTENSION_ID: AMP_SAFEDEL2(ptr_ITU_T_extension); break;
			}
		}

		void SetExtensionDatai(int i) { extension_data_i = i; }

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_IGNORE_REQUEST;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			uint64_t peek_hdr_value = AMBst_PeekBits(in_bst, 36);

			MAP_BST_BEGIN(1);
			extension_id = peek_hdr_value & 0xF;

			if (extension_id == SEQUENCE_EXTENSION_ID)
			{
				AMP_NEWT(ptr_sequence_extension, CSequenceExtension);
				if (AMP_FAILED((iRet = ptr_sequence_display_extension->Map(in_bst))))
					printf("[MP2V] Failed to map sequence_extension in extension_data {retcode: %d}.\n", iRet);
			}
			else if (extension_id == PICTURE_CODING_EXTENSION_ID)
			{
				AMP_NEWT(ptr_picture_coding_extension, CPictureCodingExtension);
				if (AMP_FAILED((iRet = ptr_picture_coding_extension->Map(in_bst))))
					printf("[MP2V] Failed to map picture_coding_extension in extension_data {retcode: %d}.\n", iRet);
			}
			else if (extension_data_i == 0)
			{
				if (extension_id == SEQUENCE_DISPLAY_EXTENSION_ID)
				{
					AMP_NEWT(ptr_sequence_display_extension, CSequenceDisplayExtension);
					if (AMP_FAILED((iRet = ptr_sequence_display_extension->Map(in_bst))))
						printf("[MP2V] Failed to map sequence_display_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == SEQUENCE_SCALABLE_EXTENSION_ID)
				{
					AMP_NEWT(ptr_sequence_scalable_extension, CSequenceScalableExtension);
					if (AMP_FAILED((iRet = ptr_sequence_scalable_extension->Map(in_bst))))
						printf("[MP2V] Failed to map sequence_scalable_extension in extension_data{retcode: %d}.\n", iRet);
				}
			}
			else if (extension_data_i == 1)
			{
				/* 
				NOTE: i never takes the value 1 because extension_data() never follows a group_of_pictures_header() 
				*/
			}
			else if (extension_data_i == 2)
			{
				if (extension_id == QUANT_MATRIX_EXTENSION_ID)
				{
					AMP_NEWT(ptr_quant_matrix_extension, CQuantMatrixExtension);
					if (AMP_FAILED((iRet = ptr_quant_matrix_extension->Map(in_bst))))
						printf("[MP2V] Failed to map quant_matrix_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == COPYRIGHT_EXTENSION_ID)
				{
					AMP_NEWT(ptr_copyright_extension, CCopyrightExtension);
					if (AMP_FAILED((iRet = ptr_copyright_extension->Map(in_bst))))
						printf("[MP2V] Failed to map copyright_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == PICTURE_DISPLAY_EXTENSION_ID)
				{
					AMP_NEWT(ptr_picture_display_extension, CPictureDisplayExtension);
					if (AMP_FAILED((iRet = ptr_picture_display_extension->Map(in_bst))))
						printf("[MP2V] Failed to map picture_display_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == PICTURE_SPATIAL_SCALABLE_EXTENSION_ID)
				{
					AMP_NEWT(ptr_picture_spatial_scalable_extension, CPictureSpatialScalableExtension);
					if (AMP_FAILED((iRet = ptr_picture_spatial_scalable_extension->Map(in_bst))))
						printf("[MP2V] Failed to map picture_spatial_scalable_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID)
				{
					AMP_NEWT(ptr_picture_temporal_scalable_extension, CTemporalScalableExtension);
					if (AMP_FAILED((iRet = ptr_picture_temporal_scalable_extension->Map(in_bst))))
						printf("[MP2V] Failed to map picture_temporal_scalable_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == CAMERA_PARAMETERS_EXTENSION_ID)
				{
					AMP_NEWT(ptr_camera_parameter_extension, CCameraParametersExtension);
					if (AMP_FAILED((iRet = ptr_camera_parameter_extension->Map(in_bst))))
						printf("[MP2V] Failed to map camera_parameter_extension in extension_data {retcode: %d}.\n", iRet);
				}
				else if (extension_id == ITUT_EXTENSION_ID)
				{
					AMP_NEWT(ptr_ITU_T_extension, CITUTExtension);
					if (AMP_FAILED((iRet = ptr_ITU_T_extension->Map(in_bst))))
						printf("[MP2V] Failed to map ITU_T_extension in extension_data {retcode: %d}.\n", iRet);
				}
			}

			MAP_BST_END();

			return iRet;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		if (extension_id == SEQUENCE_EXTENSION_ID)
		{
			NAV_FIELD_PROP_REF_WITH_TAG1(ptr_sequence_extension, "sequence_extension")
		}
		else if (extension_id == PICTURE_CODING_EXTENSION_ID)
		{
			NAV_FIELD_PROP_REF_WITH_TAG1(ptr_picture_coding_extension, "picture_coding_extension")
		}
		else if (extension_data_i == 0)
		{
			if (extension_id == SEQUENCE_DISPLAY_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_sequence_display_extension, "sequence_display_extension")
			}
			else if (extension_id == SEQUENCE_SCALABLE_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_sequence_scalable_extension, "sequence_scalable_extension")
			}
		}
		else if (extension_data_i == 1)
		{
			/* NOTE ¨C i never takes the value 1 because extension_data()
			never follows a group_of_pictures_header() */
		}
		else if (extension_data_i == 2)
		{
			if (extension_id == QUANT_MATRIX_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_quant_matrix_extension, "quant_matrix_extension")
			}
			else if (extension_id == COPYRIGHT_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_copyright_extension, "copyright_extension")
			}
			else if (extension_id == PICTURE_DISPLAY_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_picture_display_extension, "picture_display_extension")
			}
			else if (extension_id == PICTURE_SPATIAL_SCALABLE_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_picture_spatial_scalable_extension, "picture_spatial_scalable_extension")
			}
			else if (extension_id == PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_picture_temporal_scalable_extension, "picture_temporal_scalable_extension")
			}
			else if (extension_id == CAMERA_PARAMETERS_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_camera_parameter_extension, "camera_parameters_extension")
			}
			else if (extension_id == ITUT_EXTENSION_ID) {
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_ITU_T_extension, "itut_extension")
			}
		}
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CExtensionData: public SYNTAX_BITSTREAM_MAP
	{
		int							m_extension_idx;
		std::vector<CExtensionDataItem>
									m_extension_data_items;

		CExtensionData(int extension_idx) : m_extension_idx(extension_idx){}

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			MAP_BST_BEGIN(0);

			do
			{
				m_extension_data_items.reserve(1);
				CExtensionDataItem& Item = m_extension_data_items.back();
				Item.SetExtensionDatai(m_extension_idx);

				// No more data in current bitstream, quit the current map procedure
				if((iRet = Item.Map(in_bst)) == RET_CODE_NO_MORE_DATA)
					break;

				// Failed to map a extension data, and continue the next steps.
				if (AMP_FAILED(iRet))
					printf("[MP2V] Failed to map extension data {ret: %d}.\n", iRet);

				// if the next_start_code can't be found, then think the current bitstream does not contain any mpeg2 element any more.
				if (AMP_FAILED(next_start_code(in_bst)))
				{
					iRet = RET_CODE_NO_MORE_DATA;
					break;
				}

			}while(AMBst_PeekBits(in_bst, 32) == MAKE_START_CODE(EXTENSION_START_CODE));

			MAP_BST_END();

			return iRet;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			for(i=0;i<(int)m_extension_data_items.size();i++)
			{
				if (bit_offset)*bit_offset = m_extension_data_items[i].bit_pos;
				NAV_FIELD_PROP_OBJECT(m_extension_data_items[i]);

			}
		DECLARE_FIELDPROP_END()	
	};

	struct CExtensionAndUserDatas: public SYNTAX_BITSTREAM_MAP
	{
		struct CExtensionAndUserData: public SYNTAX_BITSTREAM_MAP
		{
			int					extension_and_userdata_idx;
			CExtensionData*		ptr_extension_data;
			CUserData*			ptr_user_data;

			CExtensionAndUserData(int idx)
				: extension_and_userdata_idx(idx)
				, ptr_extension_data(NULL)
				, ptr_user_data(NULL){}

			virtual ~CExtensionAndUserData(){
				AMP_SAFEDEL2(ptr_extension_data);
				AMP_SAFEDEL2(ptr_user_data);
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_SUCCESS;
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				MAP_BST_BEGIN(0);
				if (extension_and_userdata_idx != 1 && AMBst_PeekBits(in_bst, 32) == MAKE_START_CODE(EXTENSION_START_CODE)){
					AMP_NEWT(ptr_extension_data, CExtensionData, extension_and_userdata_idx);
					if(AMP_FAILED((iRet = ptr_extension_data->Map(in_bst))))
						printf("[MP2V] Failed to map extension_data in extension_and_user_data( %d ) {retcode: %d}.\n", extension_and_userdata_idx, iRet);
				}

				if (iRet != RET_CODE_NO_MORE_DATA && AMBst_PeekBits(in_bst, 32) == MAKE_START_CODE(USER_DATA_START_CODE)){
					AMP_NEWT(ptr_user_data, CUserData);
					if (AMP_FAILED((iRet = ptr_user_data->Map(in_bst))))
						printf("[MP2V] Failed to map user_data in extension_and_user_data( %d ) {return: %d}.\n", extension_and_userdata_idx, iRet);
				}
				MAP_BST_END();
				return iRet;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_extension_data, "extension_data")
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_user_data, "user_data")
			DECLARE_FIELDPROP_END()
		}PACKED;

		int										extension_and_userdata_idx;
		std::vector<CExtensionAndUserData*>		datas;

		CExtensionAndUserDatas(int idx): extension_and_userdata_idx(idx){}
		~CExtensionAndUserDatas(){
			for(size_t i=0;i<datas.size();i++){
				AMP_SAFEDEL(datas[i]);
			}
		}

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			MAP_BST_BEGIN(0);
			unsigned long start_code = (unsigned long)AMBst_PeekBits(in_bst, 32);
			while(start_code == MAKE_START_CODE(EXTENSION_START_CODE) || start_code == MAKE_START_CODE(USER_DATA_START_CODE))
			{
				AMP_NEWT1(ptr_data, CExtensionAndUserData, extension_and_userdata_idx);
				datas.push_back(ptr_data);
				if (AMP_FAILED((iRet = ptr_data->Map(in_bst))))
					printf("[MP2V] Failed to map extension_and_userdata (%d) {retcode: %d}.\n", extension_and_userdata_idx, iRet);

				if (iRet == RET_CODE_NO_MORE_DATA)
					break;

				start_code = (unsigned long)AMBst_PeekBits(in_bst, 32);
			}
			MAP_BST_END();

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			for(i=0;i<(int)datas.size();i++){
				NAV_FIELD_PROP_REF(datas[i])
			}
		DECLARE_FIELDPROP_END()	
	};

	struct CUnknownUnit : public SYNTAX_BITSTREAM_MAP
	{
		uint8_t						start_code[4] = { 0 };
		std::vector<uint8_t>		unit_bytes;

		CUnknownUnit(){}

		int Map(AMBst in_bst)
		{
			// Assume to never fail for SYNTAX_BITSTREAM_MAP::Map and get the start code
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, start_code, 4);
			try
			{
				MAP_BST_BEGIN(0);
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		DECLARE_FIELDPROP_END()
	};

	struct CSequenceEnd : public SYNTAX_BITSTREAM_MAP
	{
		uint8_t		sequence_end_code[4] = { 0 };

		int Map(AMBst in_bst)
		{
			// Assume to never fail for SYNTAX_BITSTREAM_MAP::Map and get the start code
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, sequence_end_code, 4);
			try
			{
				MAP_BST_BEGIN(0);
				next_start_code(in_bst);
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		NAV_FIELD_PROP_FIXSIZE_BINSTR("sequence_end_code", 32, sequence_end_code, 4, "Should be 00 00 01 B7");
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CSlice : public SYNTAX_BITSTREAM_MAP
	{
		uint8_t			slice_start_code[4];

		uint16_t		slice_vertical_position_extension : 3;
		uint16_t		priority_breakpoint:7;
		uint16_t		quantiser_scale_code : 5;
		uint16_t		slice_extension_flag : 1;

		uint8_t			intra_slice : 1;
		uint8_t			slice_picture_id_enable : 1;
		uint8_t			slice_picture_id : 6;

		CAMBitArray		extra_bits_slice;
		std::vector<uint8_t>
						extra_information_slice;

		IMPVContext*	m_pMPVCtx;
		uint16_t		vertical_size = 0;
		bool			sequence_scalable_extension_exit = false;

		CSlice(IMPVContext* pCtx) 
			: slice_vertical_position_extension(0), priority_breakpoint(0), quantiser_scale_code(0), slice_extension_flag(0)
			, intra_slice(0), slice_picture_id_enable(0), slice_picture_id(0)
			, m_pMPVCtx(pCtx){
			if (m_pMPVCtx != nullptr)
				m_pMPVCtx->AddRef();
		}

		~CSlice() {
			AMP_SAFERELEASE(m_pMPVCtx);
		}

		int Map(AMBst in_bst)
		{
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			AMBst_GetBytes(in_bst, slice_start_code, 4);

			try
			{
				MAP_BST_BEGIN(1);
				auto seq_hdr = m_pMPVCtx->GetSeqHdr();
				auto seq_ext = m_pMPVCtx->GetSeqExt();

				if (seq_hdr == nullptr || seq_ext == nullptr)
					throw AMException(RET_CODE_NEEDMOREINPUT);

				vertical_size = (uint16_t)(seq_hdr->vertical_size_value | (seq_ext->vertical_size_extension << 12));
				if (vertical_size > 2800) {
					bsrb1(in_bst, slice_vertical_position_extension, 3);
				}

				if (m_pMPVCtx && m_pMPVCtx->GetSeqScalableExt() != nullptr && m_pMPVCtx->GetSeqScalableExt()->scalable_mode == 0){
					sequence_scalable_extension_exit = true;
					bsrb1(in_bst, priority_breakpoint, 7);
				}

				bsrb1(in_bst, quantiser_scale_code, 5);

				int idx_extra_bit = 0;
				if (AMBst_PeekBits(in_bst, 1))
				{
					bsrb1(in_bst, slice_extension_flag, 1);
					bsrb1(in_bst, intra_slice, 1);
					bsrb1(in_bst, slice_picture_id_enable, 1);
					bsrb1(in_bst, slice_picture_id, 6);

					while (AMBst_PeekBits(in_bst, 1)) {
						bsrbarray(in_bst, extra_bits_slice, idx_extra_bit);
						uint8_t uByte;
						bsrb1(in_bst, uByte, 8);
						extra_information_slice.push_back(uByte);
						idx_extra_bit++;
					}
				}
				
				bsrbarray(in_bst, extra_bits_slice, idx_extra_bit);

				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		uint32_t slice_vertical_position = slice_start_code[3];
		BST_FIELD_PROP_FIXSIZE_BINSTR("slice_start_code", 32, slice_start_code, 4, "should be 00 00 01 01~AF");
		int mb_row = slice_vertical_position - 1;
		if (vertical_size > 2800) {
			mb_row += ((int)slice_vertical_position_extension << 7);
			BST_FIELD_PROP_2NUMBER1(slice_vertical_position_extension, 3, "extend the vertical position");
		}
		
		NAV_WRITE_TAG_WITH_NUMBER_VALUE1(mb_row, "macroblock row");

		if (sequence_scalable_extension_exit) {
			BST_FIELD_PROP_2NUMBER1(priority_breakpoint, 3, "the point in the syntax where the bitstream shall be partitioned");
		}

		BST_FIELD_PROP_2NUMBER1(quantiser_scale_code, 5, "");

		if (slice_extension_flag)
		{
			BST_FIELD_PROP_BOOL(slice_extension_flag, "", "");
			BST_FIELD_PROP_BOOL(intra_slice, "Some macroblocks may be intra macroblocks", "any of the macroblocks in the slice are non-intra macroblocks");
			BST_FIELD_PROP_BOOL(slice_picture_id_enable, "slice_picture_id may have a value different from zero", "slice_picture_id is not used");
			BST_FIELD_PROP_2NUMBER1(slice_picture_id, 6, slice_picture_id_enable == 0?"N/A":"application defined");
		}

		if (extra_information_slice.size() > 0)
		{
			NAV_WRITE_TAG_BEGIN2("extra_information_slice");
			for (size_t sid = 0; sid < extra_information_slice.size(); sid++) {
				BST_ARRAY_FIELD_PROP_NUMBER1(extra_bits_slice, (int)sid, 1, "Should be 1");
				BST_ARRAY_FIELD_PROP_NUMBER1(extra_information_slice, (int)sid, 8, "Reserved");
			}
			NAV_WRITE_TAG_END2("extra_information_slice");
		}

		if (extra_bits_slice.Size() > 0 && (size_t)extra_bits_slice.Size() > extra_information_slice.size())
		{
			BST_FIELD_PROP_NUMBER("extra_bit_slice", 1, extra_bits_slice[extra_bits_slice.Size() - 1], "Should be 0");
		}

		DECLARE_FIELDPROP_END()
	};

	struct VideoBitstreamCtx: public CComUnknown, public IMPVContext
	{
	public:
		VideoBitstreamCtx() 
		: m_in_scanning(0)
		, m_unit_split(0)
		, m_reserved_for_future_use(0){
		}
	public:
		DECLARE_IUNKNOWN

		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_IMPVContext)
				return GetCOMInterface((IMPVContext*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

		RET_CODE				SetStartCodeFilters(std::initializer_list<uint16_t> start_code_filters);
		RET_CODE				GetStartCodeFilters(std::vector<uint16_t>& start_code_filters);
		bool					IsStartCodeFiltered(uint16_t start_code);
		void					UpdateStartCode(uint16_t start_code);
		int						GetCurrentLevel();
		SEQHDR					GetSeqHdr();
		RET_CODE				UpdateSeqHdr(SEQHDR seqHdr);
		SEQEXT					GetSeqExt();
		RET_CODE				UpdateSeqExt(SEQEXT seqExt);
		SEQSCAEXT				GetSeqScalableExt();
		RET_CODE				UpdateSeqScalableExt(SEQSCAEXT seqScaExt);
		void					Reset();

	public:
		std::vector<uint16_t>	m_start_code_filters;

		uint8_t					m_in_scanning : 1;
		uint8_t					m_unit_split : 1;	// 0: each byte stream unit need locate the next start code
													// 1: the mapped bitstream is already split, and it is already a complete unit
		uint8_t					m_reserved_for_future_use : 6;

		int8_t					m_phase = -1;		// 0: Finish paring sequence_header and sequence_extension
													// 1: Finish paring group_pictures_heder
													// 2: Finish parsing picture_header and picture_coding_extension
		std::vector<uint16_t>	m_start_codes;
		int						m_curr_level = -1;

		SEQHDR					m_seq_hdr;
		SEQEXT					m_seq_ext;
		SEQSCAEXT				m_seq_scalable_ext;
	};

	struct BYTE_STREAM_UNIT: public SYNTAX_BITSTREAM_MAP
	{
		struct CPicturePayload: public SYNTAX_BITSTREAM_MAP
		{
			CGroupPicturesHeader*		ptr_group_of_pictures_header;	// optional
			CExtensionAndUserDatas*		ptr_extension_and_user_data_1;	// optional
			CPictureHeader*				ptr_picture_header;				// mandatory
			CPictureCodingExtension*	ptr_picture_coding_extension;	// mandatory
			CExtensionAndUserDatas*		ptr_extension_and_user_data_2;	// mandatory
			uint8_t*					ptr_buf;
			int							buf_size;

			CPicturePayload()
				: ptr_group_of_pictures_header(NULL)
				, ptr_extension_and_user_data_1(NULL)
				, ptr_picture_header(NULL)
				, ptr_picture_coding_extension(NULL)
				, ptr_extension_and_user_data_2(NULL)
				, ptr_buf(NULL)
				, buf_size(0){}

			virtual ~CPicturePayload(){
				AMP_SAFEDEL2(ptr_group_of_pictures_header);
				AMP_SAFEDEL2(ptr_extension_and_user_data_1);
				AMP_SAFEDEL2(ptr_picture_header);
				AMP_SAFEDEL2(ptr_picture_coding_extension);
				AMP_SAFEDEL2(ptr_extension_and_user_data_2);
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_SUCCESS;
				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				MAP_BST_BEGIN(0);
				unsigned long start_code = (unsigned long)AMBst_PeekBits(in_bst, 32);
				if (start_code == MAKE_START_CODE(GROUP_START_CODE))
				{
					AMP_NEWT(ptr_group_of_pictures_header, CGroupPicturesHeader);
					if(AMP_FAILED(iRet = ptr_group_of_pictures_header->Map(in_bst)))
						printf("[MP2V] Failed to map group_of_pictures_header {retcode: %d}.\n", iRet);

					if (iRet == RET_CODE_NO_MORE_DATA)goto done;
					AMP_NEWT(ptr_extension_and_user_data_1, CExtensionAndUserDatas, 1);
					if(AMP_FAILED(iRet = ptr_extension_and_user_data_1->Map(in_bst)))
						printf("[MP2V] Failed to map extension_and_user_data(1) {retcode: %d}.\n", iRet);
				}

				if (start_code == MAKE_START_CODE(PICTURE_START_CODE))
				{
					AMP_NEWT(ptr_picture_header, CPictureHeader);
					if(AMP_FAILED(iRet = ptr_picture_header->Map(in_bst)))
						printf("[MP2V] Failed to map picture header {retcode: %d}.\n", iRet);

					if (iRet == RET_CODE_NO_MORE_DATA)goto done;
					AMP_NEWT(ptr_picture_coding_extension, CPictureCodingExtension);
					if(AMP_FAILED(ptr_picture_coding_extension->Map(in_bst)))
						printf("[MP2V] Failed to map picture coding extension {retcode: %d}.\n", iRet);

					if (iRet == RET_CODE_NO_MORE_DATA)goto done;
					AMP_NEWT(ptr_extension_and_user_data_2, CExtensionAndUserDatas, 2);
					if(AMP_FAILED(iRet = ptr_extension_and_user_data_2->Map(in_bst)))
						printf("[MP2V] Failed to map extension_and_user_data(2) {retcode: %d}.\n", iRet);
				}

			done:
				MAP_BST_END();

				return iRet;
			}

			int Unmap(AMBst out_bst)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_REF4(ptr_group_of_pictures_header, "group_of_pictures_header", "");
				if (ptr_extension_and_user_data_1 != NULL && ptr_extension_and_user_data_1->datas.size() > 0){
					NAV_FIELD_PROP_REF4(ptr_extension_and_user_data_1, "extension_and_user_data_1", "");
				}
				NAV_FIELD_PROP_REF4(ptr_picture_header, "picture_header", "");
				NAV_FIELD_PROP_REF4(ptr_picture_coding_extension, "picture_coding_extension", "");
				if (ptr_extension_and_user_data_2 != NULL && ptr_extension_and_user_data_2->datas.size() > 0){
					NAV_FIELD_PROP_REF4(ptr_extension_and_user_data_2, "extension_and_user_data_2", "");
				}
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CSequencePayload: public SYNTAX_BITSTREAM_MAP
		{
			CSequenceHeader*			ptr_sequence_header;
			CSequenceExtension*			ptr_sequence_extension;
			CExtensionAndUserDatas*		ptr_extension_and_user_data_0;

			CSequencePayload()
				: ptr_sequence_header(NULL)
				, ptr_sequence_extension(NULL)
				, ptr_extension_and_user_data_0(NULL){
			}

			virtual ~CSequencePayload()
			{
				AMP_SAFEDEL2(ptr_sequence_header);
				AMP_SAFEDEL2(ptr_sequence_extension);
				AMP_SAFEDEL2(ptr_extension_and_user_data_0);
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_SUCCESS;
			
				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				MAP_BST_BEGIN(0);
				unsigned long start_code = (unsigned long)AMBst_PeekBits(in_bst, 32);
				if (start_code == MAKE_START_CODE(SEQUENCE_HEADER_CODE))
				{
					// Sequence header mapping
					AMP_NEWT(ptr_sequence_header, CSequenceHeader);
					if(AMP_FAILED((iRet = ptr_sequence_header->Map(in_bst))))
						printf("[MP2V] Failed to map sequence header {retcode: %d}.\n", iRet);

					if (iRet == RET_CODE_NO_MORE_DATA)goto done;
					if (AMP_FAILED(next_start_code(in_bst))){iRet = RET_CODE_NO_MORE_DATA; goto done;}

					uint64_t extension_start = AMBst_PeekBits(in_bst, 36);
					if ((extension_start>>4) == MAKE_START_CODE(EXTENSION_START_CODE) && 
						(extension_start&0x0F) == SEQUENCE_EXTENSION_ID)
					{
						// Sequence extension mapping
						AMP_NEWT(ptr_sequence_extension, CSequenceExtension);
						if(AMP_FAILED(iRet = ptr_sequence_extension->Map(in_bst)))
							printf("[MP2V] Failed to map sequence extension {retcode: %d}.\n", iRet);

						if (iRet == RET_CODE_NO_MORE_DATA)goto done;
						if (AMP_FAILED(next_start_code(in_bst))){iRet = RET_CODE_NO_MORE_DATA; goto done;}

						start_code = (unsigned long)AMBst_PeekBits(in_bst, 32);
						if (start_code == MAKE_START_CODE(EXTENSION_START_CODE) ||
							start_code == MAKE_START_CODE(USER_DATA_START_CODE))
						{
							// Extension and user data mapping
							AMP_NEWT(ptr_extension_and_user_data_0, CExtensionAndUserDatas, 0);
							if(AMP_FAILED(iRet = ptr_extension_and_user_data_0->Map(in_bst))){
								printf("[MP2V] Failed to map extension_and_user_data(0) {retcode: %d}.\n", iRet);
							}
						}

						if (AMP_FAILED(next_start_code(in_bst))){iRet = RET_CODE_NO_MORE_DATA; goto done;}
					}
				}

	done:
				MAP_BST_END();
				return iRet;
			}

			int Unmap(AMBst out_bst)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_REF4(ptr_sequence_header, "sequence_header", "");
				NAV_FIELD_PROP_REF4(ptr_sequence_extension, "sequence_extension", "");
				if (ptr_extension_and_user_data_0 != NULL && ptr_extension_and_user_data_0->datas.size() > 0){
					NAV_FIELD_PROP_REF4(ptr_extension_and_user_data_0, "extension_and_userdata_0", "");
				}
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CVideoPayload: public SYNTAX_BITSTREAM_MAP
		{
			CSequencePayload*					ptr_sequence_payload;
			std::vector<CPicturePayload*>		picture_payloads;
			bool								sequence_end_code_exist;

			CVideoPayload(): ptr_sequence_payload(NULL), sequence_end_code_exist(false){}
			virtual ~CVideoPayload()
			{
				AMP_SAFEDEL(ptr_sequence_payload);
				for(size_t i=0;i<picture_payloads.size();i++)
				{
					AMP_SAFEDEL(picture_payloads[i]);
				}
			}

			int Map(AMBst in_bst)
			{
				BOOL bFinished = FALSE;
				int iRet = RET_CODE_SUCCESS;

				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				MAP_BST_BEGIN(0);

				try
				{
					do
					{
						unsigned long start_code = (unsigned long)AMBst_PeekBits(in_bst, 32);
						switch (start_code)
						{
						case MAKE_START_CODE(SEQUENCE_HEADER_CODE):
						{
							if (ptr_sequence_payload != NULL)
							{
								// New picture or sequence is encountered.
								bFinished = TRUE;
								break;
							}
							AMP_NEWT(ptr_sequence_payload, CSequencePayload);
							if (AMP_FAILED((iRet = ptr_sequence_payload->Map(in_bst))))
								printf("[MP2V] Failed to map sequence payload {retcode: %d}.\n", iRet);
						}
						break;
						case MAKE_START_CODE(GROUP_START_CODE):
						case MAKE_START_CODE(PICTURE_START_CODE):
						{
							AMP_NEWT1(ptr_picture_payload, CPicturePayload);
							if (AMP_FAILED(iRet = ptr_picture_payload->Map(in_bst)))
								printf("[MP2V] Failed to map picture payload {retcode: %d}.\n", iRet);

							picture_payloads.push_back(ptr_picture_payload);
						}
						break;
						case MAKE_START_CODE(SEQUENCE_END_CODE):
							AMBst_SkipBits(in_bst, 32);
							sequence_end_code_exist = true;
							bFinished = TRUE;
							iRet = RET_CODE_SUCCESS;
							break;
						default:
							AMBst_SkipBits(in_bst, 32);
							iRet = RET_CODE_SUCCESS;
						}

						// No more data is available in the current bitstream, break the current procedure.
						if (iRet == RET_CODE_NO_MORE_DATA)break;

						// Find the next start code
						if (!bFinished)
							if (AMP_FAILED(next_start_code(in_bst))) {
								iRet = RET_CODE_NO_MORE_DATA;
								break;
							}

					} while (!bFinished);
				}
				catch (AMException e)
				{
					return e.RetCode();
				}

				MAP_BST_END();

				return iRet;
			}

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_REF(ptr_sequence_payload)
				for(i=0;i<(int)picture_payloads.size();i++)
				{
					NAV_FIELD_PROP_REF(picture_payloads[i]);
				}
			DECLARE_FIELDPROP_END()	
		};

		int16_t		start_code_value = -1;
		uint8_t		reserved[6] = { 0 };
		union
		{
			CUnknownUnit*		ptr_unknown_unit =  nullptr;	// reserved					B0
																// reserved					B1
																// sequence_error_code		B4
																// reserved					B6
																// system start codes(Note)	B9 through FF
			CPictureHeader*		ptr_picture_header;				// picture_start_code		00
			CSlice*				ptr_slice;						// slice_start_code			01 through AF
			CUserData*			ptr_user_data;					// user_data_start_code		B2
			CSequenceHeader*	ptr_sequence_header;			// sequence_header_code		B3
			CExtensionDataItem*	ptr_extension_data;				// extension_start_code		B5
			CSequenceEnd*		ptr_sequence_end;				// sequence_end_code		B7
			CGroupPicturesHeader*
								ptr_group_of_pictures_header;	// group_start_code			B8
		}PACKED;

		BYTE_STREAM_UNIT*		next = nullptr;
		BYTE_STREAM_UNIT*		prev = nullptr;
		VideoBitstreamCtx*		ctx_video_bst;

		BYTE_STREAM_UNIT(VideoBitstreamCtx* pCtxVideoBst) 
			: ctx_video_bst(pCtxVideoBst) {
		}

		~BYTE_STREAM_UNIT()
		{
			switch (start_code_value)
			{
			case SEQUENCE_HEADER_CODE: AMP_SAFEDEL2(ptr_sequence_header); break;
			case GROUP_START_CODE: AMP_SAFEDEL2(ptr_group_of_pictures_header); break;
			case PICTURE_START_CODE: AMP_SAFEDEL2(ptr_picture_header); break;
			case SEQUENCE_END_CODE: AMP_SAFEDEL2(ptr_sequence_end); break;
			case USER_DATA_START_CODE: AMP_SAFEDEL2(ptr_user_data); break;
			case EXTENSION_START_CODE: AMP_SAFEDEL2(ptr_extension_data); break;
			default:AMP_SAFEDEL2(ptr_unknown_unit); break;
			}

			AMP_SAFEDEL2(next);
		}

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;

			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			MAP_BST_BEGIN(0);

			try
			{
				if (AMP_FAILED(next_start_code(in_bst)))
				{
					iRet = RET_CODE_NO_MORE_DATA;
				}
				else
				{
					unsigned long ulStartCode = (unsigned long)AMBst_PeekBits(in_bst, 32);
					switch (ulStartCode)
					{
					case MAKE_START_CODE(SEQUENCE_HEADER_CODE):
					{
						start_code_value = SEQUENCE_HEADER_CODE;
						AMP_NEWT(ptr_sequence_header, CSequenceHeader);
						if (AMP_FAILED((iRet = ptr_sequence_header->Map(in_bst))))
							printf("[MP2V] Failed to map sequence header {retcode: %d}.\n", iRet);

						ctx_video_bst->m_phase = 0;
					}
					break;
					case MAKE_START_CODE(GROUP_START_CODE):
					{
						start_code_value = GROUP_START_CODE;
						AMP_NEWT(ptr_group_of_pictures_header, CGroupPicturesHeader);
						if (AMP_FAILED((iRet = ptr_group_of_pictures_header->Map(in_bst))))
							printf("[MP2V] Failed to map group picture header {retcode: %d}.\n", iRet);

						ctx_video_bst->m_phase = 1;
					}
					break;
					case MAKE_START_CODE(PICTURE_START_CODE):
					{
						start_code_value = PICTURE_START_CODE;
						AMP_NEWT(ptr_picture_header, CPictureHeader);
						if (AMP_FAILED((iRet = ptr_picture_header->Map(in_bst))))
							printf("[MP2V] Failed to map picture header {retcode: %d}.\n", iRet);

						ctx_video_bst->m_phase = 2;
					}
					break;
					case MAKE_START_CODE(SEQUENCE_END_CODE):
					{
						start_code_value = SEQUENCE_END_CODE;
						AMP_NEWT(ptr_sequence_end, CSequenceEnd);
						if (AMP_FAILED((iRet = ptr_sequence_end->Map(in_bst))))
							printf("[MP2V] Failed to map sequence end {retcode: %d}.\n", iRet);
					}
					break;
					case MAKE_START_CODE(USER_DATA_START_CODE):
					{
						start_code_value = USER_DATA_START_CODE;
						AMP_NEWT(ptr_user_data, CUserData);
						if (AMP_FAILED((iRet = ptr_user_data->Map(in_bst))))
							printf("[MP2V] Failed to map user data {retcode: %d}.\n", iRet);
					}
					break;
					case MAKE_START_CODE(EXTENSION_START_CODE):
					{
						start_code_value = EXTENSION_START_CODE;
						AMP_NEWT(ptr_extension_data, CExtensionDataItem);

						if (ctx_video_bst->m_start_codes.size() > 0)
						{
							if (ctx_video_bst->m_phase == 0 && ctx_video_bst->m_start_codes.back() == EXTENSION_START_CODE)
								ptr_extension_data->SetExtensionDatai(0);
							else if (ctx_video_bst->m_phase == 1 && ctx_video_bst->m_start_codes.back() == GROUP_START_CODE)
								ptr_extension_data->SetExtensionDatai(1);
							else if (ctx_video_bst->m_phase == 2 && ctx_video_bst->m_start_codes.back() == EXTENSION_START_CODE)
								ptr_extension_data->SetExtensionDatai(2);
						}

						if (AMP_FAILED((iRet = ptr_extension_data->Map(in_bst))))
							printf("[MP2V] Failed to map extension data {retcode: %d}.\n", iRet);
					}
					break;
					default:
					{
						start_code_value = ulStartCode & 0xFF;
						AMP_NEWT(ptr_unknown_unit, CUnknownUnit);
						if (AMP_FAILED((iRet = ptr_unknown_unit->Map(in_bst))))
							printf("[MP2V] Failed to map unknown MPEG video payload unit {retcode: %d}.\n", iRet);
					}
					}
				}
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			MAP_BST_END();

			return iRet;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		switch (start_code_value)
		{
		case SEQUENCE_HEADER_CODE: NAV_FIELD_PROP_REF_WITH_TAG1(ptr_sequence_header, "sequence_header"); break;
		case GROUP_START_CODE: NAV_FIELD_PROP_REF_WITH_TAG1(ptr_group_of_pictures_header, "group_of_pictures_header"); break;
		case PICTURE_START_CODE: NAV_FIELD_PROP_REF_WITH_TAG1(ptr_picture_header, "picture_header"); break;
		case SEQUENCE_END_CODE: NAV_FIELD_PROP_REF_WITH_TAG1(ptr_sequence_end, "sequence_end"); break;
		case USER_DATA_START_CODE: NAV_FIELD_PROP_REF_WITH_TAG1(ptr_user_data, "user_data"); break;
		case EXTENSION_START_CODE: NAV_FIELD_PROP_REF(ptr_extension_data); break;
		default:
		{
			if (start_code_value >= SLICE_START_CODE && start_code_value <= SLICE_END_CODE)
			{
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_unknown_unit, "slice");
			}
			else if (start_code_value >= SYSTEM_START_CODE_FIRST && start_code_value <= SYSTEM_START_CODE_LAST)
			{
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_unknown_unit, "unexpected_system_payload");
			}
			else
			{
				NAV_FIELD_PROP_REF_WITH_TAG1(ptr_unknown_unit, "reserved");
			}
		}
		}
		DECLARE_FIELDPROP_END()
	};

	struct CVideoBitstream : public SYNTAX_BITSTREAM_MAP
	{
		std::vector<BYTE_STREAM_UNIT::CVideoPayload*>
										video_payloads;
		BYTE_STREAM_UNIT*				byte_stream_unit_front = nullptr;
		VideoBitstreamCtx				ctx_video_bst;

		CVideoBitstream() {}
		CVideoBitstream(std::initializer_list<uint16_t> start_code_filters) {
			ctx_video_bst.m_start_code_filters = start_code_filters;
		}
		virtual ~CVideoBitstream(){
			for(size_t i=0;i<video_payloads.size();i++)
			{
				AMP_SAFEDEL(video_payloads[i]);
			}
			AMP_SAFEDEL(byte_stream_unit_front);
		}

		bool ExistBDMVKeyFrame()
		{
			bool bSeqHdr = false, bIFrame = false;
			for (int i = 0; i < (int)video_payloads.size(); i++)
			{
				if (video_payloads[i]->ptr_sequence_payload != nullptr)
					bSeqHdr = true;

				for (int j = 0; j < (int)video_payloads[i]->picture_payloads.size(); j++)
				{
					if (video_payloads[i]->picture_payloads[j]->ptr_picture_header != nullptr &&
						video_payloads[i]->picture_payloads[j]->ptr_picture_header->picture_coding_type == 1)
					{
						bIFrame = true;
						break;
					}
				}
			}

			for (auto p = byte_stream_unit_front; p != nullptr; p = p->next)
			{
				if (p->start_code_value == SEQUENCE_HEADER_CODE)
					bSeqHdr = true;

				if (p->start_code_value == PICTURE_START_CODE && p->ptr_picture_header != nullptr && p->ptr_picture_header->picture_coding_type == 1)
					bIFrame = true;
			}

			return bSeqHdr && bIFrame;
		}

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_ERROR;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			BYTE_STREAM_UNIT::CVideoPayload* ptr_video_payload = NULL;

			ctx_video_bst.m_unit_split = 0;
			
			// Find the next start code
			if (AMP_FAILED(next_start_code(in_bst)))
				return RET_CODE_SUCCESS;

			try
			{
				do {
					AMP_NEWT(ptr_video_payload, BYTE_STREAM_UNIT::CVideoPayload);
					if (ptr_video_payload == nullptr)
					{
						iRet = RET_CODE_OUTOFMEMORY;
						break;
					}

					if (AMP_FAILED(iRet = ptr_video_payload->Map(in_bst)))
					{
						if (iRet != RET_CODE_NO_MORE_DATA)
							printf("[MP2V] Failed to map video payload {retcode: %d}.\n", iRet);
					}

					if (!ptr_video_payload->map_status.error && 
						!!ptr_video_payload->map_status.broken && 
						ptr_video_payload->map_status.number_of_fields <= 0)
					{
						AMP_SAFEDEL3(ptr_video_payload);
					}
					else
					{
						video_payloads.push_back(ptr_video_payload);
						ptr_video_payload = NULL;
					}

					if (iRet == RET_CODE_NO_MORE_DATA)
					{
						iRet = RET_CODE_SUCCESS;
						break;
					}

				}while(AMP_SUCCEEDED(iRet));
			}
			catch(AMException e)
			{
				iRet = e.RetCode();
				AMP_SAFEDEL2(ptr_video_payload);
			}

			return iRet;
		}

		int Map(unsigned char* pBuf, unsigned long cbSize)
		{
			int iRet = RET_CODE_ERROR;
			unsigned char* pStartBuf = pBuf;
			unsigned long  cbBufSize = cbSize;

			ctx_video_bst.m_unit_split = 1;

			AMBst bst = AMBst_CreateFromBuffer(pStartBuf, cbBufSize);

			BYTE_STREAM_UNIT* current_bst_unit = nullptr;
			while (cbSize >= 3)
			{
				// Find the start_code_prefix_one_3bytes
				while (cbSize >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
				{
					cbSize--;
					pBuf++;
				}

				if (cbSize < 4)
					break;

				/*
				picture_start_code			00
				slice_start_code			01 through AF
				reserved					B0
				reserved					B1
				user_data_start_code		B2
				sequence_header_code		B3
				sequence_error_code			B4
				extension_start_code		B5
				reserved					B6
				sequence_end_code			B7s
				group_start_code			B8
				system start codes(Note)	B9 through FF
				*/
				if (ctx_video_bst.m_start_code_filters.size() > 0)
				{
					if (std::find(ctx_video_bst.m_start_code_filters.begin(), ctx_video_bst.m_start_code_filters.end(), pBuf[3]) ==
						ctx_video_bst.m_start_code_filters.end())
					{
						cbSize -= 4;
						pBuf += 4;
						continue;
					}
				}

				uint8_t* pNextStartCode = FindNextStartCode(pBuf + 4, cbSize - 4);

				if (bst != NULL)
					AMBst_Destroy(bst);

				if (pNextStartCode == nullptr)
					bst = AMBst_CreateFromBuffer(pStartBuf, cbBufSize);
				else
					bst = AMBst_CreateFromBuffer(pStartBuf, (int)(pNextStartCode - pStartBuf));

				AMP_NEWT1(ptr_bst_unit, BYTE_STREAM_UNIT, &ctx_video_bst);
				if (ptr_bst_unit == nullptr)
					break;

				ptr_bst_unit->prev = current_bst_unit;
				AMBst_Seek(bst, (cbBufSize - cbSize) * 8);	// in order to keep the bit position information correctly
				if (AMP_FAILED(iRet = ptr_bst_unit->Map(bst)))
					printf("[H264] Failed to map MPEG video byte stream unit beginning with a start code {retcode: %d}.\n", iRet);

				if (current_bst_unit == nullptr)
					byte_stream_unit_front = ptr_bst_unit;
				else
					current_bst_unit->next = ptr_bst_unit;

				current_bst_unit = ptr_bst_unit;

				if (iRet == RET_CODE_NO_MORE_DATA && pNextStartCode == nullptr)
				{
					AMBst_Destroy(bst);
					iRet = RET_CODE_SUCCESS;
					break;
				}
				else if (pNextStartCode == nullptr)
				{
					// Some data is unparsed, but quit the loop anyway
					break;
				}

				cbSize = cbBufSize - (int)(pNextStartCode - pStartBuf);
				pBuf = pNextStartCode;
			}

			AMBst_Destroy(bst);

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN_1("VideoBitstream", "Video Bitstream", 1);
			for(i=0;i<(int)video_payloads.size();i++)
			{
				NAV_FIELD_PROP_REF(video_payloads[i]);
			}
			for (auto p = byte_stream_unit_front; p != nullptr; p = p->next)
			{
				NAV_FIELD_PROP_REF(p);
			}
			NAV_WRITE_TAG_END("VideoBitstream");;
		DECLARE_FIELDPROP_END()	
	};

	}	// namespace MPEG2Video
}	// namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
