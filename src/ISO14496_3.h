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
#ifndef _MPEG4_14496_3_H_
#define _MPEG4_14496_3_H_

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "DumpUtil.h"
#include "AMArray.h"
#include "AMBitStream.h"
#include "ISO14496_1.h"
#include <tuple>
#include <unordered_map>
#include <vector>
#include <math.h>
#include <stdexcept>
#include "dump_data_type.h"
#include "AMSHA1.h"
#include "AudChMap.h"
#include "MSE.h"

#define ZERO_HCB								0
#define FIRST_PAIR_HCB							5
#define ESC_HCB									11
#define QUAD_LEN								4
#define PAIR_LEN								2
#define NOISE_HCB								13
#define INTENSITY_HCB2							14
#define INTENSITY_HCB							15
#define ESC_FLAG								16

#define WINLEN_IDX(window_length)				(window_length==960?0:(window_length==1024?1:2))


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

#define ADTS_HEADER_SIZE	7

#define AAC_CHECK_LEFT_BITS(required_bits)	\
				if (left_bits < 5){\
					iRet = RET_CODE_BOX_TOO_SMALL;\
					goto done;\
				}

extern const char* audioProfileLevelIndication_names[256];

namespace BST {

	namespace AACAudio {

		extern const char* Audio_Object_Type_Names[42];

		extern const char* ADTS_profile_ObjectType_names[2][4];

		using VLC_ITEM = std::tuple<int64_t, uint8_t, uint64_t>;
		using Huffman_Codebook = std::vector<VLC_ITEM>;

		using VLC_ITEM_QUAD = std::tuple<std::tuple<int64_t, int, int, int, int>, uint8_t, uint64_t>;
		using Spectrum_Huffman_Codebook_Quad = std::vector<VLC_ITEM_QUAD>;

		using VLC_ITEM_PAIR = std::tuple<std::tuple<int64_t, int, int>, uint8_t, uint64_t>;
		using Spectrum_Huffman_Codebook_Pair = std::vector<VLC_ITEM_PAIR>;

		using HCB_BST_NODE = const int[2];
		extern std::tuple<bool, int8_t, uint8_t, HCB_BST_NODE*, size_t, const char*> spectrum_hcb_params[32];
		
		extern const std::tuple<const char*, int> audioProfileLevelIndication_Descs[256];
		extern const char* Audio_Object_Type_Names[42];
		extern const std::tuple<int, const char*> samplingFrequencyIndex_names[16];
		extern const std::tuple<int, const char*, const char*> channelConfiguration_names[16];
		extern const uint8_t PRED_SFB_MAX[13];
		extern const Huffman_Codebook scalefactor_hcb;
		extern const int scalefactor_hcb_bst[241][2];
		extern const Huffman_Codebook rvlc_hcb;
		extern const int rvlc_hcb_bst[37][2];
		extern const Huffman_Codebook rvlc_esc_hcb;
		extern const int rvlc_esc_hcb_bst[107][2];
		extern const std::tuple<const uint16_t*, size_t> swb_offset_window[3][12][4];
		extern const Spectrum_Huffman_Codebook_Quad shcb1;
		extern const int shcb1_bst[161][2];
		extern const Spectrum_Huffman_Codebook_Quad shcb2;
		extern const int shcb2_bst[161][2];
		extern const Spectrum_Huffman_Codebook_Quad shcb3;
		extern const int shcb3_bst[161][2];
		extern const Spectrum_Huffman_Codebook_Quad shcb4;
		extern const int shcb4_bst[161][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb5;
		extern const int shcb5_bst[161][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb6;
		extern const int shcb6_bst[161][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb7;
		extern const int shcb7_bst[127][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb8;
		extern const int shcb8_bst[127][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb9;
		extern const int shcb9_bst[337][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb10;
		extern const int shcb10_bst[337][2];
		extern const Spectrum_Huffman_Codebook_Pair shcb11;
		extern const int shcb11_bst[577][2];

		extern uint16_t CELP_Layer0_frameLen4[64];
		extern uint16_t CELP_Layer1_5_frameLen4[20];
		extern uint16_t CELP_Layer0_frameLen5[64][4];
		extern uint16_t CELP_Layer1_5_frameLen5[20][4];
		extern uint16_t CELP_Layer0_frameLen3[64][2];

		extern int hcb_read_value(CBitstream& bs, const int hcb[][2], int array_size, int* read_bits = nullptr, uint64_t* code = nullptr);
		extern bool hcb_vlc(const int hcb[][2], int array_size, int value, uint64_t& code, int start_idx = 0);
		extern int shcb_read_value(CBitstream& bs, uint8_t spectrum_huffman_codebook_idx);
		extern int16_t huffman_getescape(CBitstream& bs, int16_t sp);

		inline uint32_t esc_codword(uint16_t u16Val)
		{
			int8_t nValBits = floor_quick_log2(u16Val) + 1;
			int8_t nPreBits = nValBits - 4;
			return (((1 << nPreBits) - 2) << (nValBits - 1)) | (u16Val&((1 << (nValBits - 1)) - 1));
		}

		inline int8_t esc_bitlen(uint16_t u16Val)
		{
			int8_t nValBits = floor_quick_log2(u16Val) + 1;
			int8_t nPreBits = nValBits - 4;
			return nValBits - 1 + nPreBits;
		}

		enum OBJECT_TYPE
		{
			AAC_main								= 1,
			AAC_LC									= 2,
			AAC_SSR									= 3,
			AAC_LTP									= 4,
			SBR										= 5,
			AAC_Scalable							= 6,
			TwinVQ 									= 7,
			CELP									= 8,
			HVXC									= 9,
			TTSI									= 12,
			Main_synthetic							= 13,
			Wavetable_synthesis						= 14,
			General_MIDI							= 15,
			Algorithmic_Synthesis_and_Audio_FX		= 16,
			ER_AAC_LC								= 17,
			ER_AAC_LTP								= 19,
			ER_AAC_scalable							= 20,
			ER_TwinVQ								= 21,
			ER_BSAC									= 22,
			ER_AAC_LD								= 23,
			ER_CELP									= 24,
			ER_HVXC									= 25,
			ER_HILN									= 26,
			ER_Parametric							= 27,
			SSC										= 28,
			PS										= 29,
			MPEG_Surround							= 30,
			Audio_Object_escape						= 31,
			Layer_1									= 32,
			Layer_2									= 33,
			Layer_3									= 34,
			DST										= 35,
			ALS										= 36,
			SLS										= 37,
			SLS_Non_Core							= 38,
			ER_AAC_ELD								= 39,
			SMR_Simple								= 40,
			SMR_Main								= 41,
		};

		enum WINDOW_SEQUENCE
		{
			ONLY_LONG_SEQUENCE = 0,
			LONG_START_SEQUENCE,
			EIGHT_SHORT_SEQUENCE,
			LONG_STOP_SEQUENCE,
		};

		#define WINDOW_SEQUENCE_NAMEA(ws)	(\
			(ws) == 0?"ONLY_LONG_SEQUENCE":(\
			(ws) == 1?"LONG_START_SEQUENCE":(\
			(ws) == 2?"EIGHT_SHORT_SEQUENCE":(\
			(ws) == 3?"LONG_STOP_SEQUENCE":"Unknown"))))

		enum SYNTAX_ELEMENT
		{
			ID_SCE = 0,
			ID_CPE,
			ID_CCE,
			ID_LFE,
			ID_DSE,
			ID_PCE,
			ID_FIL,
			ID_END,
		};

		#define SYNTAX_ELEMENT_NAME(x) (\
				DECL_ENUM_ITEMA(x, ID_SCE)\
				DECL_ENUM_ITEMA(x, ID_CPE)\
				DECL_ENUM_ITEMA(x, ID_CCE)\
				DECL_ENUM_ITEMA(x, ID_LFE)\
				DECL_ENUM_ITEMA(x, ID_DSE)\
				DECL_ENUM_ITEMA(x, ID_PCE)\
				DECL_ENUM_ITEMA(x, ID_FIL)\
				DECL_ENUM_LAST_ITEMA(x, ID_END, "Unknown"))))))))))

		#define WINDOW_NUM(ws)		((ws) == 2?8:1)

		#define MAX_LTP_LONG_SFB	40

		#define MPE					0
		#define RPE					1

		class IMP4AACContext;

		struct CAudioSpecificConfig : public SYNTAX_BITSTREAM_MAP {

			struct CGASpecificConfig : public BITSTREAM_MAP {

				struct CProgramConfigElement : public BITSTREAM_MAP {

					union CElementConfig {
						struct {
							unsigned char	element_is_cpe : 1;
							unsigned char	element_tag_select : 4;
						}PACKED;
						struct {
							unsigned char	cc_element_is_ind_sw : 1;
							unsigned char	valid_cc_element_tag_select : 4;
						}PACKED;
						unsigned char	lfe_element_tag_select;
						unsigned char	assoc_data_element_tag_select;
						unsigned char	char_value = 0;
					}PACKED;

					unsigned long		element_instance_tag : 4;
					unsigned long		object_type : 2;
					unsigned long		sampling_frequency_index : 4;
					unsigned long		num_front_channel_elements : 4;
					unsigned long		num_side_channel_elements : 4;
					unsigned long		num_back_channel_elements : 4;
					unsigned long		num_lfe_channel_elements : 2;
					unsigned long		num_assoc_data_elements : 3;
					unsigned long		num_valid_cc_elements : 4;
					unsigned long		mono_mixdown_present : 1;

					unsigned char		mono_mixdown_element_number;
					unsigned char		stereo_mixdown_present;
					unsigned char		stereo_mixdown_element_number;
					unsigned char		matrix_mixdown_idx_present;
					unsigned char		matrix_mixdown_idx;
					unsigned char		pseudo_surround_enable;

					CElementConfig		front_channel_elements[15];
					CElementConfig		side_channel_elements[15];
					CElementConfig		back_channel_elements[15];
					CElementConfig		lfe_channel_element_tag_select[3];
					CElementConfig		assoc_data_element_tag_select[7];
					CElementConfig		valid_cc_elements[15];

					unsigned char		comment_field_bytes = 0;
					unsigned char		comment_field_data[255] = { 0 };

					CProgramConfigElement() 
						: element_instance_tag(0), object_type(0), sampling_frequency_index(0)
						, num_front_channel_elements(0), num_side_channel_elements(0), num_back_channel_elements(0), num_lfe_channel_elements(0)
						, num_assoc_data_elements(0), num_valid_cc_elements(0), mono_mixdown_present(0){
					}

					virtual ~CProgramConfigElement() { Unmap(); }

					int Map(AMBst bst) {
						if (bst == NULL)
							return RET_CODE_BUFFER_NOT_FOUND;

						try {
							element_instance_tag = (unsigned char)AMBst_GetBits(bst, 4);
							object_type = (unsigned char)AMBst_GetBits(bst, 2);
							sampling_frequency_index = (unsigned char)AMBst_GetBits(bst, 4);
							num_front_channel_elements = (unsigned char)AMBst_GetBits(bst, 4);
							num_side_channel_elements = (unsigned char)AMBst_GetBits(bst, 4);
							num_back_channel_elements = (unsigned char)AMBst_GetBits(bst, 4);
							num_lfe_channel_elements = (unsigned char)AMBst_GetBits(bst, 2);
							num_assoc_data_elements = (unsigned char)AMBst_GetBits(bst, 3);
							num_valid_cc_elements = (unsigned char)AMBst_GetBits(bst, 4);

							if((mono_mixdown_present = (unsigned long)AMBst_GetBits(bst, 1)))
								mono_mixdown_element_number = (unsigned char)AMBst_GetBits(bst, 4);

							if((stereo_mixdown_present = (unsigned char)AMBst_GetBits(bst, 1)))
								stereo_mixdown_element_number = (unsigned char)AMBst_GetBits(bst, 4);

							if((matrix_mixdown_idx_present = (unsigned char)AMBst_GetBits(bst, 1))) {
								matrix_mixdown_idx = (unsigned char)AMBst_GetBits(bst, 2);
								pseudo_surround_enable = (unsigned char)AMBst_GetBits(bst, 1);
							}

							for (unsigned long i = 0; i < num_front_channel_elements; i++) {
								front_channel_elements[i].element_is_cpe = (unsigned char)AMBst_GetBits(bst, 1);
								front_channel_elements[i].element_tag_select = (unsigned char)AMBst_GetBits(bst, 4);
							}

							for (unsigned long i = 0; i < num_side_channel_elements; i++) {
								side_channel_elements[i].element_is_cpe = (unsigned char)AMBst_GetBits(bst, 1);
								side_channel_elements[i].element_tag_select = (unsigned char)AMBst_GetBits(bst, 4);
							}

							for (unsigned long i = 0; i < num_back_channel_elements; i++) {
								back_channel_elements[i].element_is_cpe = (unsigned char)AMBst_GetBits(bst, 1);
								back_channel_elements[i].element_tag_select = (unsigned char)AMBst_GetBits(bst, 4);
							}

							for (unsigned long i = 0; i < num_lfe_channel_elements; i++) {
								lfe_channel_element_tag_select[i].lfe_element_tag_select = (unsigned char)AMBst_GetBits(bst, 4);
							}

							for (unsigned long i = 0; i < num_assoc_data_elements; i++) {
								assoc_data_element_tag_select[i].assoc_data_element_tag_select = (unsigned char)AMBst_GetBits(bst, 4);
							}

							for (unsigned long i = 0; i < num_valid_cc_elements; i++) {
								valid_cc_elements[i].cc_element_is_ind_sw = (unsigned char)AMBst_GetBits(bst, 1);
								valid_cc_elements[i].valid_cc_element_tag_select = (unsigned char)AMBst_GetBits(bst, 4);
							}

							AMBst_Realign(bst);

							comment_field_bytes = (unsigned char)AMBst_GetBits(bst, 8);
							if (comment_field_bytes > 0 && AMBst_GetBytes(bst, comment_field_data, comment_field_bytes) != comment_field_bytes)
								return RET_CODE_BUFFER_TOO_SMALL;

							return RET_CODE_SUCCESS;

						}
						catch (...)
						{
							return RET_CODE_BUFFER_TOO_SMALL;
						}

						return RET_CODE_SUCCESS;
					}

					int Unmap(AMBst bst = NULL) {
						if (bst == NULL)
							return RET_CODE_SUCCESS;

						AMBst_PutBits(bst, 4, element_instance_tag);
						AMBst_PutBits(bst, 2, object_type);
						AMBst_PutBits(bst, 4, sampling_frequency_index);
						AMBst_PutBits(bst, 4, num_front_channel_elements);
						AMBst_PutBits(bst, 4, num_side_channel_elements);
						AMBst_PutBits(bst, 4, num_back_channel_elements);
						AMBst_PutBits(bst, 2, num_lfe_channel_elements);
						AMBst_PutBits(bst, 3, num_assoc_data_elements);
						AMBst_PutBits(bst, 4, num_valid_cc_elements);
						AMBst_PutBits(bst, 1, mono_mixdown_present);
						if (mono_mixdown_present)
							AMBst_PutBits(bst, 4, mono_mixdown_element_number);

						if (stereo_mixdown_present)
							AMBst_PutBits(bst, 4, stereo_mixdown_element_number);

						if (matrix_mixdown_idx_present) {
							AMBst_PutBits(bst, 2, matrix_mixdown_idx);
							AMBst_PutBits(bst, 1, pseudo_surround_enable);
						}

						for (unsigned long i = 0; i < num_front_channel_elements; i++) {
							AMBst_PutBits(bst, 1, front_channel_elements[i].element_is_cpe);
							AMBst_PutBits(bst, 4, front_channel_elements[i].element_tag_select);
						}

						for (unsigned long i = 0; i < num_side_channel_elements; i++) {
							AMBst_PutBits(bst, 1, side_channel_elements[i].element_is_cpe);
							AMBst_PutBits(bst, 4, side_channel_elements[i].element_tag_select);
						}

						for (unsigned long i = 0; i < num_back_channel_elements; i++) {
							AMBst_PutBits(bst, 1, back_channel_elements[i].element_is_cpe);
							AMBst_PutBits(bst, 4, back_channel_elements[i].element_tag_select);
						}

						for (unsigned long i = 0; i < num_lfe_channel_elements; i++)
							AMBst_PutBits(bst, 4, lfe_channel_element_tag_select[i].lfe_element_tag_select);

						for (unsigned long i = 0; i < num_assoc_data_elements; i++)
							AMBst_PutBits(bst, 4, assoc_data_element_tag_select[i].assoc_data_element_tag_select);

						for (unsigned long i = 0; i < num_valid_cc_elements; i++) {
							AMBst_PutBits(bst, 1, valid_cc_elements[i].cc_element_is_ind_sw);
							AMBst_PutBits(bst, 4, valid_cc_elements[i].valid_cc_element_tag_select);
						}

						AMBst_Realign(bst);
						AMBst_PutBits(bst, 8, comment_field_bytes);
						AMBst_PutBytes(bst, comment_field_data, comment_field_bytes);

						return RET_CODE_SUCCESS;
					}

					DECLARE_FIELDPROP_BEGIN()
					NAV_WRITE_TAG_BEGIN("program_config_element");
					NAV_FIELD_PROP_2NUMBER1(element_instance_tag, 4, "")
						NAV_FIELD_PROP_2NUMBER1(object_type, 2, object_type == 0 ? "AAC Main" : (object_type == 1 ? "AAC LC" : (object_type == 2 ? "AAC SSR" : "AAC LTP")))
						NAV_FIELD_PROP_2NUMBER1(sampling_frequency_index, 4, std::get<1>(samplingFrequencyIndex_names[sampling_frequency_index]))
						NAV_FIELD_PROP_2NUMBER1(num_front_channel_elements, 4, "The number of audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel elements")
						NAV_FIELD_PROP_2NUMBER1(num_side_channel_elements, 4, "Number of elements to the side as above")
						NAV_FIELD_PROP_2NUMBER1(num_back_channel_elements, 4, "As number of side and front channel elements, for back channels")
						NAV_FIELD_PROP_2NUMBER1(num_lfe_channel_elements, 2, "number of LFE channel elements associated with this program")
						NAV_FIELD_PROP_2NUMBER1(num_assoc_data_elements, 3, "The number of associated data elements for this program")
						NAV_FIELD_PROP_2NUMBER1(num_valid_cc_elements, 4, "The number of CCE's that can add to the audio data for this program")
						NAV_FIELD_PROP_2NUMBER1(mono_mixdown_present, 1, "One bit, indicating the presence of the mono mixdown element")
						if (mono_mixdown_present) {
							NAV_FIELD_PROP_2NUMBER1(mono_mixdown_element_number, 4, "The number of a specified SCE that is the mono mixdown")
						}
					NAV_FIELD_PROP_2NUMBER1(stereo_mixdown_present, 1, "One bit, indicating that there is a stereo mixdown present")
					if (mono_mixdown_present) {
						NAV_FIELD_PROP_2NUMBER1(stereo_mixdown_element_number, 4, "The number of a specified CPE that is the stereo mixdown element")
					}
					NAV_FIELD_PROP_2NUMBER1(matrix_mixdown_idx_present, 1, "One bit indicating the presence of matrix mixdown information by means of a stereo matrix coefficient index (Table 4.70). For all configurations other than the 3/2 format this bit must be zero")
					if (mono_mixdown_present) {
						NAV_FIELD_PROP_2NUMBER1(matrix_mixdown_idx, 2, "Two bit field, specifying the index of the mixdown coefficient to be used in the 5-channel to 2-channel matrix-mixdown. Possible matrix coefficients are listed in Table 4.70.")
							NAV_FIELD_PROP_2NUMBER1(pseudo_surround_enable, 1, "One bit, indicating the possibility of mixdown for pseudo surround reproduction ()")
					}

					NAV_WRITE_TAG_BEGIN2_1("front_channels", "audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel elements");
					for (i = 0; i < (int)num_front_channel_elements; i++) {
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, instance_tag: %d", front_channel_elements[i].element_is_cpe ? "CPE" : "SCE", front_channel_elements[i].element_tag_select);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("front_channel", "[%d]", szTemp2, i);
							NAV_FIELD_PROP_2NUMBER("front_element_is_cpe", 1, front_channel_elements[i].element_is_cpe, front_channel_elements[i].element_is_cpe ? "CPE" : "SCE")
							NAV_FIELD_PROP_2NUMBER("front_element_tag_select", 4, front_channel_elements[i].element_tag_select, "the instance_tag of the SCE/CPE addressed as a front element")
						NAV_WRITE_TAG_END("front_channel");
					}
					NAV_WRITE_TAG_END2("front_channels");

					NAV_WRITE_TAG_BEGIN2_1("side_channels", "elements to the side");
					for (i = 0; i < (int)num_side_channel_elements; i++) {
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, instance_tag: %d", side_channel_elements[i].element_is_cpe ? "CPE" : "SCE", side_channel_elements[i].element_tag_select);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("side_channel", "[%d]", szTemp2, i);
							NAV_FIELD_PROP_2NUMBER("side_element_is_cpe", 1, side_channel_elements[i].element_is_cpe, side_channel_elements[i].element_is_cpe ? "CPE" : "SCE")
							NAV_FIELD_PROP_2NUMBER("side_element_tag_select", 4, side_channel_elements[i].element_tag_select, "the instance_tag of the SCE/CPE addressed as a side element")
						NAV_WRITE_TAG_END("side_channel");
					}
					NAV_WRITE_TAG_END2("side_channels");

					NAV_WRITE_TAG_BEGIN2_1("back_channels", "side and front channel elements, for back channels");
					for (i = 0; i < (int)num_back_channel_elements; i++) {
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, instance_tag: %d", back_channel_elements[i].element_is_cpe ? "CPE" : "SCE", back_channel_elements[i].element_tag_select);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("back_channel", "[%d]", szTemp2, i);
							NAV_FIELD_PROP_2NUMBER("back_element_is_cpe", 1, back_channel_elements[i].element_is_cpe, back_channel_elements[i].element_is_cpe ? "CPE" : "SCE")
							NAV_FIELD_PROP_2NUMBER("back_element_tag_select", 4, back_channel_elements[i].element_tag_select, "the instance_tag of the SCE/CPE addressed as a back element")
						NAV_WRITE_TAG_END("back_channel");
					}
					NAV_WRITE_TAG_END2("back_channels");

					NAV_WRITE_TAG_BEGIN2_1("lfe_channels", "LFE channel elements associated with this program");
					for (i = 0; i < (int)num_lfe_channel_elements; i++) {
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "instance_tag: %d", lfe_channel_element_tag_select[i].lfe_element_tag_select);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("lfe_channel", "[%d]", szTemp2, i);
							NAV_FIELD_PROP_2NUMBER("lfe_element_tag_select", 4, lfe_channel_element_tag_select[i].lfe_element_tag_select, "instance_tag of the LFE addressed")
						NAV_WRITE_TAG_END("lfe_channel");
					}
					NAV_WRITE_TAG_END2("lfe_channels");

					NAV_WRITE_TAG_BEGIN2_1("assoc_data_elements", "associated data elements for this program");
					for (i = 0; i < (int)num_assoc_data_elements; i++) {
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "instance_tag: %d", assoc_data_element_tag_select[i].assoc_data_element_tag_select);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("assoc_data", "[%d]", szTemp2, i);
							NAV_FIELD_PROP_2NUMBER("assoc_data_element_tag_select", 4, assoc_data_element_tag_select[i].assoc_data_element_tag_select, "instance_tag of the DSE addressed")
						NAV_WRITE_TAG_END("assoc_data");
					}
					NAV_WRITE_TAG_END2("assoc_data_elements");

					NAV_WRITE_TAG_BEGIN2_1("valid_cc_elements", "CCE's that can add to the audio data for this program");
					for (i = 0; i < (int)num_valid_cc_elements; i++) {
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "instance_tag: %d", valid_cc_elements[i].valid_cc_element_tag_select);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("valid_cc", "[%d]", szTemp2, i);
							NAV_FIELD_PROP_2NUMBER("cc_element_is_ind_sw", 1, valid_cc_elements[i].cc_element_is_ind_sw, "One bit, indicating that the corresponding CCE is an independently switched coupling channel")
							NAV_FIELD_PROP_2NUMBER("valid_cc_element_tag_select", 4, valid_cc_elements[i].valid_cc_element_tag_select, "instance_tag of the CCE addressed")
						NAV_WRITE_TAG_END3("valid_cc", i);
					}
					NAV_WRITE_TAG_END2("valid_cc_elements");

					if (bit_offset)
						*bit_offset = (*bit_offset + 7) / 8 * 8;

					NAV_FIELD_PROP_2NUMBER1(comment_field_bytes, 8, "The length, in bytes, of the following comment field")
					NAV_FIELD_PROP_FIXSIZE_BINSTR1(comment_field_data, (unsigned long)comment_field_bytes * 8, "The data in the comment field")
					NAV_WRITE_TAG_END("program_config_element");
					DECLARE_FIELDPROP_END()
				}PACKED;

				unsigned char		frameLengthFlag;			// 1 bits
				unsigned char		dependsOnCoreCoder;			// 1 bits
				unsigned short		coreCoderDelay;				// 14 bits
				unsigned char		extensionFlag;				// 1 bits
				CProgramConfigElement*
									program_config_element;
				unsigned char		layerNr;					// 3 bits

				unsigned short		numOfSubFrame : 5;
				unsigned short		layer_length : 11;

				unsigned char		aacSectionDataResilienceFlag : 1;
				unsigned char		aacScalefactorDataResilienceFlag : 1;
				unsigned char		aacSpectralDataResilienceFlag : 1;
				unsigned char		extensionFlag3 : 1;

				unsigned char		audioObjectType;
				unsigned char		channelConfiguration;

				CGASpecificConfig(unsigned char audioObjectTypeParam, unsigned char channelConfigurationParam)
					: program_config_element(NULL), audioObjectType(audioObjectTypeParam), channelConfiguration(channelConfigurationParam) {
					aacScalefactorDataResilienceFlag = 0;
					aacScalefactorDataResilienceFlag = 0;
					aacSpectralDataResilienceFlag = 0;
				}
				virtual ~CGASpecificConfig() { Unmap(); }

				int Map(AMBst bst) {
					if (bst == NULL)
						return RET_CODE_BUFFER_NOT_FOUND;

					try {
						frameLengthFlag = (unsigned char)AMBst_GetBits(bst, 1);
						dependsOnCoreCoder = (unsigned char)AMBst_GetBits(bst, 1);
						if (dependsOnCoreCoder)
							coreCoderDelay = (unsigned short)AMBst_GetBits(bst, 14);
						extensionFlag = (unsigned char)AMBst_GetBits(bst, 1);
						if (!channelConfiguration) {
							MAP_MEM_TO_STRUCT_POINTER5(1, program_config_element, CProgramConfigElement)
						}
						if (audioObjectType == 6 || audioObjectType == 20) {
							layerNr = (unsigned char)AMBst_GetBits(bst, 3);
						}
						if (extensionFlag) {
							if (audioObjectType == 22) {
								numOfSubFrame = (unsigned char)AMBst_GetBits(bst, 5);
								layer_length = (unsigned short)AMBst_GetBits(bst, 11);
							}
							if (audioObjectType == 17 || audioObjectType == 19 || audioObjectType == 20 || audioObjectType == 23) {
								aacSectionDataResilienceFlag = (unsigned char)AMBst_GetBits(bst, 1);
								aacScalefactorDataResilienceFlag = (unsigned char)AMBst_GetBits(bst, 1);
								aacSpectralDataResilienceFlag = (unsigned char)AMBst_GetBits(bst, 1);
							}
							extensionFlag3 = (unsigned char)AMBst_GetBits(bst, 1);
						}
					}
					catch (...)
					{
						return RET_CODE_BUFFER_TOO_SMALL;
					}

					return RET_CODE_SUCCESS;
				}

				int Unmap(AMBst bst = NULL) {
					if (bst == NULL)
					{
						if (!channelConfiguration) {
							UNMAP_STRUCT_POINTER5(program_config_element)
						}
					}
					else
					{
						AMBst_PutBits(bst, 1, frameLengthFlag);
						AMBst_PutBits(bst, 1, dependsOnCoreCoder);
						if (dependsOnCoreCoder)
							AMBst_PutBits(bst, 14, coreCoderDelay);
						AMBst_PutBits(bst, 1, extensionFlag);

						if (!channelConfiguration)
							program_config_element->Unmap(bst);

						if (audioObjectType == 6 || audioObjectType == 20)
							AMBst_PutBits(bst, 3, layerNr);

						if (extensionFlag) {
							if (audioObjectType == 22) {
								AMBst_PutBits(bst, 5, numOfSubFrame);
								AMBst_PutBits(bst, 11, layer_length);
							}

							if (audioObjectType == 17 || audioObjectType == 19 ||
								audioObjectType == 20 || audioObjectType == 23) {
								AMBst_PutBits(bst, 1, aacSectionDataResilienceFlag);
								AMBst_PutBits(bst, 1, aacScalefactorDataResilienceFlag);
								AMBst_PutBits(bst, 1, aacSpectralDataResilienceFlag);
							}

							AMBst_PutBits(bst, 1, extensionFlag3);
						}
					}
					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
				NAV_WRITE_TAG_BEGIN2("GASpecificConfig");
				NAV_FIELD_PROP_2NUMBER1(frameLengthFlag, 1, audioObjectType == AAC_SSR ? "frameLengthFlag should be set 0, A 256/32 lines IMDCT is used"
					: (audioObjectType == ER_AAC_LD ? (frameLengthFlag ? "480 line IMDCT is used and frameLength is set to 480" : "512 lines IMDCT is used and frameLength is set to 512")
						: (frameLengthFlag ? "960/120 line IMDCT is used and frameLength is set to 960" : "1024/128 lines IMDCT is used and frameLength is set to 1024")))
					NAV_FIELD_PROP_2NUMBER1(dependsOnCoreCoder, 1, "Signals that a core coder has been used in an underlying base layer of a scalable AAC configuration")
					if (dependsOnCoreCoder) {
						NAV_FIELD_PROP_2NUMBER1(coreCoderDelay, 14, "The delay in samples that has to be applied to the up-sampled (if necessary) core decoder output, before the MDCT calculation")
					}
				NAV_FIELD_PROP_2NUMBER1(extensionFlag, 1, "Shall be '0' for audio object types 1, 2, 3, 4, 6, 7. Shall be '1' for audio object types 17, 19, 20, 21, 22, 23.")
					if (!channelConfiguration) {
						NAV_FIELD_PROP_REF(program_config_element)
					}
				if (audioObjectType == 6 || audioObjectType == 20) {
					NAV_FIELD_PROP_2NUMBER1(layerNr, 3, "A 3-bit field indicating the AAC layer number in a scalable configuration. The first AAC layer is signaled by a value of 0")
				}
				if (extensionFlag) {
					if (audioObjectType == 22) {
						NAV_FIELD_PROP_2NUMBER1(numOfSubFrame, 5, "A 5-bit unsigned integer value representing the number of the sub-frames which are grouped and transmitted in a super-frame")
							NAV_FIELD_PROP_2NUMBER1(layer_length, 11, "An 11-bit unsigned integer value representing the average length of the large-step layers in bytes")
					}

					if (audioObjectType == 17 || audioObjectType == 19 ||
						audioObjectType == 20 || audioObjectType == 23) {
						NAV_FIELD_PROP_2NUMBER1(aacSectionDataResilienceFlag, 1, "This flag signals a different coding scheme of AAC section data. If codebook 11 is used, this scheme transmits additional information about the maximum absolute value for spectral lines. This allows error detection of spectral lines that are larger than this value.")
							NAV_FIELD_PROP_2NUMBER1(aacScalefactorDataResilienceFlag, 1, "This flag signals a different coding scheme of the AAC scalefactor data, that is more resilient against errors as the original one")
							NAV_FIELD_PROP_2NUMBER1(aacSpectralDataResilienceFlag, 1, "This flag signals a different coding scheme (HCR) of the AAC spectral data, that is more resilient against errors as the original one")
					}

					NAV_FIELD_PROP_2NUMBER1(extensionFlag3, 1, "Extension flag for the future use. Shall be '0'")
				}
				NAV_WRITE_TAG_END("GASpecificConfig");
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct CCelpSpecificConfig : public ADVANCE_ENDIAN_MAP {

				struct CCelpHeader : public ADVANCE_ENDIAN_MAP {
					unsigned char		ExcitationMode : 1;
					unsigned char		SampleRateMode : 1;
					unsigned char		FineRateControl : 1;
					unsigned char		reserved_0 : 5;
					union {
						struct {
							unsigned char		RPE_Configuration : 3;
							unsigned char		reserved_1 : 5;
						}PACKED;
						struct {
							unsigned char		MPE_Configuration : 5;
							unsigned char		NumEnhLayers : 2;
							unsigned char		BandwidthScalabilityMode : 1;
						}PACKED;
					}PACKED;
				}PACKED;

				unsigned char		isBaseLayer;		// 1 bit
				union {
					CCelpHeader*	CelpHeader;
					struct {
						unsigned char	isBWSLayer;
						union {
							unsigned char	BWS_configuration;
							unsigned char	CELP_BRS_id;
						};
					}PACKED;
				}PACKED;

			}PACKED;

			struct CHVXCconfig : public DIRECT_ENDIAN_MAP {
				unsigned char		HVXCvarMode : 1;
				unsigned char		HVXCrateMode : 2;
				unsigned char		extensionFlag : 1;
			}PACKED;

			struct CTTSSpecificConfig : public ADVANCE_ENDIAN_MAP {
				struct CTTSSequence {
					unsigned long TTS_Sequence_ID : 5;
					unsigned long Language_Code : 18;
					unsigned long Gender_Enable : 1;
					unsigned long Age_Enable : 1;
					unsigned long Speech_Rate_Enable : 1;
					unsigned long Prosody_Enable : 1;
					unsigned long Video_Enable : 1;
					unsigned long Lip_Shape_Enable : 1;
					unsigned long Trick_Mode_Enable : 1;
				}PACKED;
			}PACKED;

			struct CStructuredAudioSpecificConfig : public ADVANCE_ENDIAN_MAP {

				struct symbol {
					unsigned short	sym;		// 16 bits
				}PACKED;

				struct sym_name {
					unsigned char	length;		// 4 bits
					unsigned char	name[15];
				}PACKED;

				struct symtable {
					unsigned char	length;
					sym_name*		name;
				}PACKED;

				struct COrchToken : public ADVANCE_ENDIAN_MAP {
					struct CStrToken {
						unsigned char length;
						unsigned char str[255];
					}PACKED;

					unsigned char		token;
					union {
						symbol			sym;
						float			fval;
						unsigned int	nval;
						CStrToken*		str;
						unsigned char	bval;
					};
				}PACKED;

				struct COrcFile : public ADVANCE_ENDIAN_MAP {
					unsigned short		length;
					COrchToken*			data;
				}PACKED;

				struct CInstrEvent : public ADVANCE_ENDIAN_MAP {
					unsigned char		has_label;
					symbol				label;
					symbol				iname_sym;
					float				dur;
					unsigned char		num_pf;
					float*				value;
				}PACKED;

				struct CControlEvent : public ADVANCE_ENDIAN_MAP {
					unsigned char		has_label;
					symbol				label;
					symbol				varsym;
					float				value;
				}PACKED;

				struct CTableEvent : public ADVANCE_ENDIAN_MAP
				{
					symbol				tname;
					unsigned char		destory;
				};

			}PACKED;

			unsigned char		audioObjectType_1 = 0;					// 5 bits
			unsigned char		audioObjectTypeExt_1 = 0;				// 6 bits

			unsigned char		samplingFrequencyIndex = 0;				// 4 bits
			unsigned long		samplingFrequency = 0;					// 24 bits

			unsigned char		channelConfiguration = 0;				// 4 bits

			unsigned char		extensionSamplingFrequencyIndex = 0;	// 4 bits
			unsigned long		extensionSamplingFrequency = 0;			// 24 bit

			unsigned char		audioObjectType_2 = 0;					// 5 bits
			unsigned char		audioObjectTypeExt_2 = 0;				// 6 bits

			int					config_data_len = 0;
			union {
				CGASpecificConfig*	GASpecificConfig;
				unsigned char*		config_data;
			}PACKED;

			// In order to check whether the AudioSpecificConfig is changed or not
			// The upper layer may generate sha1_value for it, and then it can be used to identify the content of this part
			AMSHA1_RET			sha1_value;

			CAudioSpecificConfig() { config_data_len = 0; config_data = NULL; memset(sha1_value, 0, sizeof(sha1_value)); }
			~CAudioSpecificConfig() {
				unsigned audioObjectType = (audioObjectType_1 == 31) ? (32 + audioObjectTypeExt_1) : audioObjectType_1;
				if (audioObjectType_1 == 5)
					audioObjectType = (audioObjectType_2 == 31) ? (32 + audioObjectTypeExt_2) : audioObjectType_2;

				if (audioObjectType == 1 || audioObjectType == 2 || audioObjectType == 3 || audioObjectType == 4 ||
					audioObjectType == 6 || audioObjectType == 7 || audioObjectType == 17 || audioObjectType == 19 ||
					audioObjectType == 20 || audioObjectType == 21 || audioObjectType == 22 || audioObjectType == 23)
				{
					UNMAP_STRUCT_POINTER5(GASpecificConfig);
				}
			}

			int GetAudioObjectType()
			{
				int audioObjectType = (audioObjectType_1 == 31) ? (32 + audioObjectTypeExt_1) : audioObjectType_1;
				if (audioObjectType_1 == 5)
					audioObjectType = (audioObjectType_2 == 31) ? (32 + audioObjectTypeExt_2) : audioObjectType_2;
				return audioObjectType;
			}

			int GetSamplingFrequency()
			{
				auto audio_object_type = GetAudioObjectType();

				if (audio_object_type == AAC_main ||
					audio_object_type == AAC_LC ||
					audio_object_type == AAC_SSR ||
					audio_object_type == AAC_LTP ||
					audio_object_type == AAC_Scalable ||
					audio_object_type == TwinVQ ||
					audio_object_type == ER_AAC_LC ||
					audio_object_type == ER_AAC_LTP ||
					audio_object_type == ER_AAC_scalable ||
					audio_object_type == ER_TwinVQ ||
					audio_object_type == ER_BSAC ||
					audio_object_type == ER_AAC_LD)
				{
					int nSamplingRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, -1 };

					if (samplingFrequencyIndex == 0xF)
						return (int)samplingFrequency;
					else if (nSamplingRates[samplingFrequencyIndex] > 0)
						return nSamplingRates[samplingFrequencyIndex];
				}

				return 0;	// Unknown value
			}

			int GetNumOfChannels()
			{
				/*
				1: 1ch (1/0)
				2: 2ch (2/0)
				3: 3ch (3/0)
				4: 4ch (3/1)
				5: 5ch (3/2)
				6: 5.1ch (3/2.1)
				7: 7.1ch (5/2.1)
				11: 6.1ch (3/0/3.1)
				12: 7.1ch (3/2/2.1)
				13: 22.2ch (3/3/3-5/2/3-3/0/0+2)
				14: 7.1ch (2/0/0-3/0/2-0/0/0+1)
				0: program_config_element() is used.
				(in case of 3ch(2/1), 4ch(2/2) and 2-audio signals (dual mono) (1/0+1/0))	
				*/
				int numOfChannels[] = { 0, 1, 2, 3, 4, 5, 6, 8, 0, 0, 0, 7, 8, 24, 8, 0 };
				if (channelConfiguration >= _countof(numOfChannels))
					return 0;

				// if channel_configuration is 0, need parse program_config_element to get the channel configuration
				if (channelConfiguration == 0 && GASpecificConfig != nullptr && GASpecificConfig->program_config_element != nullptr)
				{
					auto pce = GASpecificConfig->program_config_element;
					if (pce->num_front_channel_elements == 1 &&
						pce->num_side_channel_elements == 0 &&
						pce->num_back_channel_elements == 1 &&
						pce->num_lfe_channel_elements == 0 &&
						pce->num_valid_cc_elements == 0 &&
						pce->front_channel_elements[0].element_is_cpe == 1 &&
						pce->front_channel_elements[0].element_tag_select == 0 &&
						pce->back_channel_elements[0].element_is_cpe == 0 &&
						pce->back_channel_elements[0].element_tag_select == 0)
					{
						// L+R+MS(mono surround)
						return 3;
					}
					else if (pce->num_front_channel_elements == 1 &&
						pce->num_side_channel_elements == 0 &&
						pce->num_back_channel_elements == 1 &&
						pce->num_lfe_channel_elements == 0 &&
						pce->num_valid_cc_elements == 0 &&
						pce->front_channel_elements[0].element_is_cpe == 1 &&
						pce->front_channel_elements[0].element_tag_select == 0 &&
						pce->back_channel_elements[0].element_is_cpe == 1 &&
						pce->back_channel_elements[0].element_tag_select == 1)
					{
						// L+R+Ls+Rs
						return 4;
					}
					else if (pce->num_front_channel_elements == 2 &&
						pce->num_side_channel_elements == 0 &&
						pce->num_back_channel_elements == 0 &&
						pce->num_lfe_channel_elements == 0 &&
						pce->num_valid_cc_elements == 0 &&
						pce->front_channel_elements[0].element_is_cpe == 0 &&
						pce->front_channel_elements[0].element_tag_select == 0 &&
						pce->back_channel_elements[0].element_is_cpe == 0 &&
						pce->back_channel_elements[0].element_tag_select == 1)
					{
						// Dual Mono
						return 2;
					}
				}

				return numOfChannels[channelConfiguration];
			}

			CH_MAPPING GetChannelMapping()
			{
				if (channelConfiguration >= _countof(aac_channel_configurations))
					return CH_MAPPING();

				return aac_channel_configurations[channelConfiguration];
			}

			int Map(AMBst in_bst) {
				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				try {
					MAP_BST_BEGIN(0);
					audioObjectType_1 = (unsigned char)AMBst_GetBits(in_bst, 5);
					if (audioObjectType_1 == 31)
						audioObjectTypeExt_1 = (unsigned char)AMBst_GetBits(in_bst, 6);

					unsigned audioObjectType = (audioObjectType_1 == 31) ? (32 + audioObjectTypeExt_1) : audioObjectType_1;

					samplingFrequencyIndex = (unsigned char)AMBst_GetBits(in_bst, 4);
					if (samplingFrequencyIndex == 0xf)
						samplingFrequency = (unsigned long)AMBst_GetBits(in_bst, 24);

					channelConfiguration = (unsigned char)AMBst_GetBits(in_bst, 4);

					int sbrPresentFlag = -1;
					unsigned char extensionAudioObjectType = 0;
					if (audioObjectType_1 == 5)
					{
						extensionAudioObjectType = audioObjectType_1;
						sbrPresentFlag = 1;
						extensionSamplingFrequencyIndex = (unsigned char)AMBst_GetBits(in_bst, 4);
						if (extensionSamplingFrequencyIndex == 0xf)
							extensionSamplingFrequency = (unsigned long)AMBst_GetBits(in_bst, 24);

						audioObjectType_2 = (unsigned char)AMBst_GetBits(in_bst, 5);
						if (audioObjectType_2 == 31)
							audioObjectTypeExt_2 = (unsigned char)AMBst_GetBits(in_bst, 6);

						audioObjectType = (audioObjectType_2 == 31) ? (32 + audioObjectTypeExt_2) : audioObjectType_2;
					}

					if (audioObjectType == 1 || audioObjectType == 2 || audioObjectType == 3 || audioObjectType == 4 ||
						audioObjectType == 6 || audioObjectType == 7 || audioObjectType == 17 || audioObjectType == 19 ||
						audioObjectType == 20 || audioObjectType == 21 || audioObjectType == 22 || audioObjectType == 23)
					{
						MAP_MEM_TO_STRUCT_POINTER5_1(in_bst, 1, GASpecificConfig, CGASpecificConfig, audioObjectType, channelConfiguration);
					}
					else
					{
						// Implement the object later
					}

					MAP_BST_END();
				}
				catch (...)
				{
					return RET_CODE_BUFFER_TOO_SMALL;
				}

				SYNTAX_BITSTREAM_MAP::EndMap(in_bst);

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL) {
				if (out_bst == NULL)
					return RET_CODE_ERROR;

				if (WriteToBs(out_bst) != RET_CODE_SUCCESS) {
					return RET_CODE_ERROR;
				}
				AMBst_Flush(out_bst);
				return RET_CODE_SUCCESS;
			}

			int WriteToBs(AMBst bs)
			{
				AMBst_PutBits(bs, 5, audioObjectType_1);
				if (audioObjectType_1 == 31)
					AMBst_PutBits(bs, 6, audioObjectTypeExt_1);

				unsigned audioObjectType = (audioObjectType_1 == 31) ? (32 + audioObjectTypeExt_1) : audioObjectType_1;

				AMBst_PutBits(bs, 4, samplingFrequencyIndex);
				if (samplingFrequencyIndex == 0x0f)
					AMBst_PutBits(bs, 24, samplingFrequency);

				AMBst_PutBits(bs, 4, channelConfiguration);

				if (audioObjectType_1 == 5) {
					AMBst_PutBits(bs, 4, extensionSamplingFrequencyIndex);
					if (extensionSamplingFrequencyIndex == 0x0f)
						AMBst_PutBits(bs, 24, extensionSamplingFrequency);

					AMBst_PutBits(bs, 5, audioObjectType_2);
					if (audioObjectType_2 == 31)
						AMBst_PutBits(bs, 6, audioObjectTypeExt_2);

					audioObjectType = (audioObjectType_2 == 31) ? (32 + audioObjectTypeExt_2) : audioObjectType_2;
				}

				if (audioObjectType == 1 || audioObjectType == 2 || audioObjectType == 3 || audioObjectType == 4 ||
					audioObjectType == 6 || audioObjectType == 7 || audioObjectType == 17 || audioObjectType == 19 ||
					audioObjectType == 20 || audioObjectType == 21 || audioObjectType == 22 || audioObjectType == 23)
				{
					GASpecificConfig->Unmap(bs);
				}

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN2("AudioSpecificConfig");
			unsigned audioObjectType = (audioObjectType_1 == 31) ? (32 + audioObjectTypeExt_1) : audioObjectType_1;
			NAV_FIELD_PROP_2NUMBER("audioObjectType", 5, audioObjectType_1, Audio_Object_Type_Names[audioObjectType])
				if (audioObjectType_1 == 31) {
					NAV_FIELD_PROP_2NUMBER("audioObjectTypeExt", 6, audioObjectTypeExt_1, "")
				}

			NAV_FIELD_PROP_2NUMBER1(samplingFrequencyIndex, 4, std::get<1>(samplingFrequencyIndex_names[samplingFrequencyIndex]));
				if (samplingFrequencyIndex == 0xf) {
					NAV_FIELD_PROP_2NUMBER1(samplingFrequency, 24, "")
				}
			NAV_FIELD_PROP_2NUMBER1(channelConfiguration, 4, std::get<2>(channelConfiguration_names[channelConfiguration]))
				int sbrPresentFlag = -1;
			unsigned char extensionAudioObjectType = 0;
			if (audioObjectType_1 == 5)
			{
				extensionAudioObjectType = audioObjectType_1;
				sbrPresentFlag = 1;
				NAV_FIELD_PROP_2NUMBER1(extensionSamplingFrequencyIndex, 4, std::get<1>(samplingFrequencyIndex_names[extensionSamplingFrequencyIndex]))
					if (extensionSamplingFrequencyIndex == 0xf) {
						NAV_FIELD_PROP_2NUMBER1(extensionSamplingFrequency, 24, "")
					}

				audioObjectType = (audioObjectType_2 == 31) ? (32 + audioObjectTypeExt_2) : audioObjectType_2;
				NAV_FIELD_PROP_2NUMBER("audioObjectType", 5, audioObjectType_2, Audio_Object_Type_Names[audioObjectType])
					if (audioObjectType_1 == 31) {
						NAV_FIELD_PROP_2NUMBER("audioObjectTypeExt", 6, audioObjectTypeExt_2, "")
					}
			}

			if (audioObjectType == 1 || audioObjectType == 2 || audioObjectType == 3 || audioObjectType == 4 ||
				audioObjectType == 6 || audioObjectType == 7 || audioObjectType == 17 || audioObjectType == 19 ||
				audioObjectType == 20 || audioObjectType == 21 || audioObjectType == 22 || audioObjectType == 23)
			{
				NAV_FIELD_PROP_REF(GASpecificConfig)
			}
			NAV_WRITE_TAG_END2("AudioSpecificConfig");;
			DECLARE_FIELDPROP_END()

		}PACKED;

		// AAC Audio Stream
		struct AAC_SYNTAX_BITSTREAM
		{
			uint64_t	bit_pos;				// start bit position

			AAC_SYNTAX_BITSTREAM() : bit_pos(UINT64_MAX) { }

			virtual int Unpack(CBitstream& bs) {
				bit_pos = bs.Tell();
				return RET_CODE_SUCCESS;
			}
		}PACKED;

		struct PROGRAM_CONFIG_ELEMENT
		{
			union ElementConfig {
				struct {
					uint8_t			element_is_cpe : 1;
					uint8_t			element_tag_select : 4;
				}PACKED;
				struct {
					uint8_t			cc_element_is_ind_sw : 1;
					uint8_t			valid_cc_element_tag_select : 4;
				}PACKED;
				uint8_t			lfe_element_tag_select;
				uint8_t			assoc_data_element_tag_select;
				uint8_t			char_value = 0;
			}PACKED;

			uint32_t			element_instance_tag : 4;
			uint32_t			object_type : 2;
			uint32_t			sampling_frequency_index : 4;
			uint32_t			num_front_channel_elements : 4;
			uint32_t			num_side_channel_elements : 4;
			uint32_t			num_back_channel_elements : 4;
			uint32_t			num_lfe_channel_elements : 2;
			uint32_t			num_assoc_data_elements : 3;
			uint32_t			num_valid_cc_elements : 4;
			uint32_t			mono_mixdown_present : 1;

			uint8_t				mono_mixdown_element_number = 0;
			uint8_t				stereo_mixdown_present = 0;
			uint8_t				stereo_mixdown_element_number = 0;
			uint8_t				matrix_mixdown_idx_present = 0;
			uint8_t				matrix_mixdown_idx = 0;
			uint8_t				pseudo_surround_enable = 0;

			ElementConfig		front_channel_elements[15];
			ElementConfig		side_channel_elements[15];
			ElementConfig		back_channel_elements[15];
			ElementConfig		lfe_channel_element_tag_select[3];
			ElementConfig		assoc_data_element_tag_select[7];
			ElementConfig		valid_cc_elements[15];

			uint8_t				comment_field_bytes = 0;
			uint8_t				comment_field_data[255] = { 0 };

			PROGRAM_CONFIG_ELEMENT()
				: element_instance_tag(0), object_type(0), sampling_frequency_index(0), num_front_channel_elements(0)
				, num_side_channel_elements(0), num_back_channel_elements(0), num_lfe_channel_elements(0), num_assoc_data_elements(0)
				, num_valid_cc_elements(0), mono_mixdown_present(0){
			}
			~PROGRAM_CONFIG_ELEMENT() {}

			CH_MAPPING GetChannelMapping()
			{
				if (num_front_channel_elements == 1 &&
					num_side_channel_elements == 0 &&
					num_back_channel_elements == 1 &&
					num_lfe_channel_elements == 0 &&
					num_valid_cc_elements == 0)
				{
					if (front_channel_elements[0].element_is_cpe == 1 &&
						front_channel_elements[0].element_tag_select == 0 &&
						back_channel_elements[0].element_is_cpe == 0 &&
						back_channel_elements[0].element_tag_select == 0)
						return CH_MAPPING(CH_MAPPING_CAT_BS, { BS_L, BS_R, CH_LOC_MS });
					else if (front_channel_elements[0].element_is_cpe == 1 &&
						front_channel_elements[0].element_tag_select == 0 &&
						back_channel_elements[0].element_is_cpe == 1 &&
						back_channel_elements[0].element_tag_select == 1)
						return CH_MAPPING(CH_MAPPING_CAT_BS, { BS_L, BS_R, BS_Ls, BS_Rs });
				}
				else if (num_front_channel_elements == 2 &&
					num_side_channel_elements == 0 &&
					num_back_channel_elements == 0 &&
					num_lfe_channel_elements == 0 &&
					num_valid_cc_elements == 0)
				{
					if (front_channel_elements[0].element_is_cpe == 0 &&
						front_channel_elements[0].element_tag_select == 0 &&
						front_channel_elements[1].element_is_cpe == 0 &&
						front_channel_elements[1].element_tag_select == 1)
						return CH_MAPPING(CH_MAPPING_CAT_BS, { BS_C, CH_LOC_DUAL });
				}

				return CH_MAPPING(CH_MAPPING_CAT_BS);
			}

			int Unpack(CBitstream& bs) {
				try {
					element_instance_tag = (uint8_t)bs.GetBits(4);
					object_type = (uint8_t)bs.GetBits(2);
					sampling_frequency_index = (uint8_t)bs.GetBits(4);
					num_front_channel_elements = (uint8_t)bs.GetBits(4);
					num_side_channel_elements = (uint8_t)bs.GetBits(4);
					num_back_channel_elements = (uint8_t)bs.GetBits(4);
					num_lfe_channel_elements = (uint8_t)bs.GetBits(2);
					num_assoc_data_elements = (uint8_t)bs.GetBits(3);
					num_valid_cc_elements = (uint8_t)bs.GetBits(4);

					if((mono_mixdown_present = (uint32_t)bs.GetBits(1)))
						mono_mixdown_element_number = (uint8_t)bs.GetBits(4);

					if((stereo_mixdown_present = (uint8_t)bs.GetBits(1)))
						stereo_mixdown_element_number = (uint8_t)bs.GetBits(4);

					if((matrix_mixdown_idx_present = (uint8_t)bs.GetBits(1))) {
						matrix_mixdown_idx = (uint8_t)bs.GetBits(2);
						pseudo_surround_enable = (uint8_t)bs.GetBits(1);
					}

					for (uint32_t i = 0; i < num_front_channel_elements; i++) {
						front_channel_elements[i].element_is_cpe = (uint8_t)bs.GetBits(1);
						front_channel_elements[i].element_tag_select = (uint8_t)bs.GetBits(4);
					}

					for (uint32_t i = 0; i < num_side_channel_elements; i++) {
						side_channel_elements[i].element_is_cpe = (uint8_t)bs.GetBits(1);
						side_channel_elements[i].element_tag_select = (uint8_t)bs.GetBits(4);
					}

					for (uint32_t i = 0; i < num_back_channel_elements; i++) {
						back_channel_elements[i].element_is_cpe = (uint8_t)bs.GetBits(1);
						back_channel_elements[i].element_tag_select = (uint8_t)bs.GetBits(4);
					}

					for (uint32_t i = 0; i < num_lfe_channel_elements; i++) {
						lfe_channel_element_tag_select[i].lfe_element_tag_select = (uint8_t)bs.GetBits(4);
					}

					for (uint32_t i = 0; i < num_assoc_data_elements; i++) {
						assoc_data_element_tag_select[i].assoc_data_element_tag_select = (uint8_t)bs.GetBits(4);
					}

					for (uint32_t i = 0; i < num_valid_cc_elements; i++) {
						valid_cc_elements[i].cc_element_is_ind_sw = (uint8_t)bs.GetBits(1);
						valid_cc_elements[i].valid_cc_element_tag_select = (uint8_t)bs.GetBits(4);
					}

					bs.Realign();

					comment_field_bytes = (uint8_t)bs.GetBits(8);
					if (comment_field_bytes > 0 && bs.Read(comment_field_data, comment_field_bytes) != comment_field_bytes)
						return RET_CODE_BUFFER_TOO_SMALL;

					return RET_CODE_SUCCESS;
				}
				catch (...)
				{
					return RET_CODE_BUFFER_TOO_SMALL;
				}

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN("program_config_element");
			NAV_FIELD_PROP_2NUMBER1(element_instance_tag, 4, "")
			NAV_FIELD_PROP_2NUMBER1(object_type, 2, object_type == 0 ? "AAC Main" : (object_type == 1 ? "AAC LC" : (object_type == 2 ? "AAC SSR" : "AAC LTP")))
			NAV_FIELD_PROP_2NUMBER1(sampling_frequency_index, 4, std::get<1>(samplingFrequencyIndex_names[sampling_frequency_index]))
			NAV_FIELD_PROP_2NUMBER1(num_front_channel_elements, 4, "The number of audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel elements")
			NAV_FIELD_PROP_2NUMBER1(num_side_channel_elements, 4, "Number of elements to the side as above")
			NAV_FIELD_PROP_2NUMBER1(num_back_channel_elements, 4, "As number of side and front channel elements, for back channels")
			NAV_FIELD_PROP_2NUMBER1(num_lfe_channel_elements, 2, "number of LFE channel elements associated with this program")
			NAV_FIELD_PROP_2NUMBER1(num_assoc_data_elements, 3, "The number of associated data elements for this program")
			NAV_FIELD_PROP_2NUMBER1(num_valid_cc_elements, 4, "The number of CCE's that can add to the audio data for this program")
			NAV_FIELD_PROP_2NUMBER1(mono_mixdown_present, 1, "One bit, indicating the presence of the mono mixdown element")
			if (mono_mixdown_present) {
				NAV_FIELD_PROP_2NUMBER1(mono_mixdown_element_number, 4, "The number of a specified SCE that is the mono mixdown")
			}
			NAV_FIELD_PROP_2NUMBER1(stereo_mixdown_present, 1, "One bit, indicating that there is a stereo mixdown present")
			if (mono_mixdown_present) {
				NAV_FIELD_PROP_2NUMBER1(stereo_mixdown_element_number, 4, "The number of a specified CPE that is the stereo mixdown element")
			}
			NAV_FIELD_PROP_2NUMBER1(matrix_mixdown_idx_present, 1, "One bit indicating the presence of matrix mixdown information by means of a stereo matrix coefficient index (Table 4.70). For all configurations other than the 3/2 format this bit must be zero")
			if (mono_mixdown_present) {
				NAV_FIELD_PROP_2NUMBER1(matrix_mixdown_idx, 2, "Two bit field, specifying the index of the mixdown coefficient to be used in the 5-channel to 2-channel matrix-mixdown. Possible matrix coefficients are listed in Table 4.70.")
					NAV_FIELD_PROP_2NUMBER1(pseudo_surround_enable, 1, "One bit, indicating the possibility of mixdown for pseudo surround reproduction ()")
			}
			NAV_WRITE_TAG_BEGIN2_1("front_channels", "audio syntactic elements in the front channels, front center to back center, symmetrically by left and right, or alternating by left and right in the case of single channel elements");
			for (i = 0; i < (int)num_front_channel_elements; i++) {
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, instance_tag: %d", front_channel_elements[i].element_is_cpe ? "CPE" : "SCE", front_channel_elements[i].element_tag_select);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("front_channel", "[%d]", szTemp2, i);
					NAV_FIELD_PROP_2NUMBER("front_element_is_cpe", 1, front_channel_elements[i].element_is_cpe, front_channel_elements[i].element_is_cpe ? "CPE" : "SCE")
					NAV_FIELD_PROP_2NUMBER("front_element_tag_select", 4, front_channel_elements[i].element_tag_select, "the instance_tag of the SCE/CPE addressed as a front element")
				NAV_WRITE_TAG_END("front_channel");
			}
			NAV_WRITE_TAG_END2("front_channels");

			NAV_WRITE_TAG_BEGIN2_1("side_channels", "elements to the side");
			for (i = 0; i < (int)num_side_channel_elements; i++) {
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, instance_tag: %d", side_channel_elements[i].element_is_cpe ? "CPE" : "SCE", side_channel_elements[i].element_tag_select);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("side_channel", "[%d]", szTemp2, i);
					NAV_FIELD_PROP_2NUMBER("side_element_is_cpe", 1, side_channel_elements[i].element_is_cpe, side_channel_elements[i].element_is_cpe ? "CPE" : "SCE")
					NAV_FIELD_PROP_2NUMBER("side_element_tag_select", 4, side_channel_elements[i].element_tag_select, "the instance_tag of the SCE/CPE addressed as a side element")
				NAV_WRITE_TAG_END("side_channel");
			}
			NAV_WRITE_TAG_END2("side_channels");

			NAV_WRITE_TAG_BEGIN2_1("back_channels", "side and front channel elements, for back channels");
			for (i = 0; i < (int)num_back_channel_elements; i++) {
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, instance_tag: %d", back_channel_elements[i].element_is_cpe ? "CPE" : "SCE", back_channel_elements[i].element_tag_select);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("back_channel", "[%d]", szTemp2, i);
					NAV_FIELD_PROP_2NUMBER("back_element_is_cpe", 1, back_channel_elements[i].element_is_cpe, back_channel_elements[i].element_is_cpe ? "CPE" : "SCE")
					NAV_FIELD_PROP_2NUMBER("back_element_tag_select", 4, back_channel_elements[i].element_tag_select, "the instance_tag of the SCE/CPE addressed as a back element")
				NAV_WRITE_TAG_END("back_channel");
			}
			NAV_WRITE_TAG_END2("back_channels");

			NAV_WRITE_TAG_BEGIN2_1("lfe_channels", "LFE channel elements associated with this program");
			for (i = 0; i < (int)num_lfe_channel_elements; i++) {
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "instance_tag: %d", lfe_channel_element_tag_select[i].lfe_element_tag_select);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("lfe_channel", "[%d]", szTemp2, i);
					NAV_FIELD_PROP_2NUMBER("lfe_element_tag_select", 4, lfe_channel_element_tag_select[i].lfe_element_tag_select, "instance_tag of the LFE addressed")
				NAV_WRITE_TAG_END("lfe_channel");
			}
			NAV_WRITE_TAG_END2("lfe_channels");

			NAV_WRITE_TAG_BEGIN2_1("assoc_data_elements", "associated data elements for this program");
			for (i = 0; i < (int)num_assoc_data_elements; i++) {
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "instance_tag: %d", assoc_data_element_tag_select[i].assoc_data_element_tag_select);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("assoc_data", "[%d]", szTemp2, i);
					NAV_FIELD_PROP_2NUMBER("assoc_data_element_tag_select", 4, assoc_data_element_tag_select[i].assoc_data_element_tag_select, "instance_tag of the DSE addressed")
				NAV_WRITE_TAG_END("assoc_data");
			}
			NAV_WRITE_TAG_END2("assoc_data_elements");

			NAV_WRITE_TAG_BEGIN2_1("valid_cc_elements", "CCE's that can add to the audio data for this program");
			for (i = 0; i < (int)num_valid_cc_elements; i++) {
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "instance_tag: %d", valid_cc_elements[i].valid_cc_element_tag_select);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("valid_cc", "[%d]", szTemp2, i);
					NAV_FIELD_PROP_2NUMBER("cc_element_is_ind_sw", 1, valid_cc_elements[i].cc_element_is_ind_sw, "One bit, indicating that the corresponding CCE is an independently switched coupling channel")
					NAV_FIELD_PROP_2NUMBER("valid_cc_element_tag_select", 4, valid_cc_elements[i].valid_cc_element_tag_select, "instance_tag of the CCE addressed")
				NAV_WRITE_TAG_END("valid_cc");
			}
			NAV_WRITE_TAG_END2("valid_cc_elements");

			if (bit_offset)
				*bit_offset = (*bit_offset + 7) / 8 * 8;

			NAV_FIELD_PROP_2NUMBER1(comment_field_bytes, 8, "The length, in bytes, of the following comment field")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(comment_field_data, (uint32_t)comment_field_bytes * 8, "The data in the comment field")
			NAV_WRITE_TAG_END("program_config_element");
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct GASpecificConfig : public AAC_SYNTAX_BITSTREAM
		{
			uint16_t			frameLengthFlag : 1;			// 1 bits
			uint16_t			dependsOnCoreCoder : 1;			// 1 bits
			uint16_t			coreCoderDelay : 14;			// 14 bits
			uint8_t				extensionFlag;					// 1 bits
			PROGRAM_CONFIG_ELEMENT*
								program_config_element;
			uint8_t				layerNr;						// 3 bits

			uint16_t			numOfSubFrame : 5;
			uint16_t			layer_length : 11;

			uint8_t				aacSectionDataResilienceFlag : 1;
			uint8_t				aacScalefactorDataResilienceFlag : 1;
			uint8_t				aacSpectralDataResilienceFlag : 1;
			uint8_t				extensionFlag3 : 1;

			uint8_t				audioObjectType;
			uint8_t				channelConfiguration;

			GASpecificConfig(uint8_t audioObjectTypeParam, uint8_t channelConfigurationParam)
				: program_config_element(NULL), audioObjectType(audioObjectTypeParam), channelConfiguration(channelConfigurationParam) {
				aacSectionDataResilienceFlag = 0;
				aacScalefactorDataResilienceFlag = 0;
				aacSpectralDataResilienceFlag = 0;
			}

			virtual ~GASpecificConfig() {
				AMP_SAFEDEL(program_config_element);
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				try {
					frameLengthFlag = (uint16_t)bs.GetBits(1);
					dependsOnCoreCoder = (uint16_t)bs.GetBits(1);
					if (dependsOnCoreCoder)
						coreCoderDelay = (uint16_t)bs.GetBits(14);
					extensionFlag = (uint8_t)bs.GetBits(1);
					if (!channelConfiguration) {
						AMP_NEWT(program_config_element, PROGRAM_CONFIG_ELEMENT);
						if ((iRet = program_config_element->Unpack(bs)) < 0)
						{
							AMP_SAFEDEL3(program_config_element);
							return iRet;
						}
					}
					if (audioObjectType == 6 || audioObjectType == 20) {
						layerNr = (uint8_t)bs.GetBits(3);
					}
					if (extensionFlag) {
						if (audioObjectType == 22) {
							numOfSubFrame = (uint8_t)bs.GetBits(5);
							layer_length = (uint16_t)bs.GetBits(11);
						}
						if (audioObjectType == 17 || audioObjectType == 19 || audioObjectType == 20 || audioObjectType == 23) {
							aacSectionDataResilienceFlag = (uint8_t)bs.GetBits(1);
							aacScalefactorDataResilienceFlag = (uint8_t)bs.GetBits(1);
							aacSpectralDataResilienceFlag = (uint8_t)bs.GetBits(1);
						}
						extensionFlag3 = (uint8_t)bs.GetBits(1);
					}
				}
				catch (...)
				{
					return RET_CODE_BUFFER_TOO_SMALL;
				}

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(frameLengthFlag, 1, audioObjectType == AAC_SSR ? "frameLengthFlag should be set 0, A 256/32 lines IMDCT is used"
				: (audioObjectType == ER_AAC_LD ? (frameLengthFlag ? "480 line IMDCT is used and frameLength is set to 480" : "512 lines IMDCT is used and frameLength is set to 512")
					: (frameLengthFlag ? "960/120 line IMDCT is used and frameLength is set to 960" : "1024/128 lines IMDCT is used and frameLength is set to 1024")))
				NAV_FIELD_PROP_2NUMBER1(dependsOnCoreCoder, 1, "Signals that a core coder has been used in an underlying base layer of a scalable AAC configuration")
				if (dependsOnCoreCoder) {
					NAV_FIELD_PROP_2NUMBER1(coreCoderDelay, 14, "The delay in samples that has to be applied to the up-sampled (if necessary) core decoder output, before the MDCT calculation")
				}
			NAV_FIELD_PROP_2NUMBER1(extensionFlag, 1, "Shall be '0' for audio object types 1, 2, 3, 4, 6, 7. Shall be '1' for audio object types 17, 19, 20, 21, 22, 23.")
				if (!channelConfiguration) {
					NAV_FIELD_PROP_REF(program_config_element)
				}
			if (audioObjectType == 6 || audioObjectType == 20) {
				NAV_FIELD_PROP_2NUMBER1(layerNr, 3, "A 3-bit field indicating the AAC layer number in a scalable configuration. The first AAC layer is signaled by a value of 0")
			}
			if (extensionFlag) {
				if (audioObjectType == 22) {
					NAV_FIELD_PROP_2NUMBER1(numOfSubFrame, 5, "A 5-bit unsigned integer value representing the number of the sub-frames which are grouped and transmitted in a super-frame")
						NAV_FIELD_PROP_2NUMBER1(layer_length, 11, "An 11-bit unsigned integer value representing the average length of the large-step layers in bytes")
				}

				if (audioObjectType == 17 || audioObjectType == 19 ||
					audioObjectType == 20 || audioObjectType == 23) {
					NAV_FIELD_PROP_2NUMBER1(aacSectionDataResilienceFlag, 1, "This flag signals a different coding scheme of AAC section data. If codebook 11 is used, this scheme transmits additional information about the maximum absolute value for spectral lines. This allows error detection of spectral lines that are larger than this value.")
						NAV_FIELD_PROP_2NUMBER1(aacScalefactorDataResilienceFlag, 1, "This flag signals a different coding scheme of the AAC scalefactor data, that is more resilient against errors as the original one")
						NAV_FIELD_PROP_2NUMBER1(aacSpectralDataResilienceFlag, 1, "This flag signals a different coding scheme (HCR) of the AAC spectral data, that is more resilient against errors as the original one")
				}

				NAV_FIELD_PROP_2NUMBER1(extensionFlag3, 1, "Extension flag for the future use. Shall be '0'")
			}
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CelpSpecificConfig
		{
			struct CELPHEADER {
				uint8_t		ExcitationMode : 1;
				uint8_t		SampleRateMode : 1;
				uint8_t		FineRateControl : 1;
				uint8_t		reserved_0 : 5;
				union {
					struct {
						uint8_t		RPE_Configuration : 3;
						uint8_t		reserved_1 : 5;
					}PACKED;
					struct {
						uint8_t		MPE_Configuration : 5;
						uint8_t		NumEnhLayers : 2;
						uint8_t		BandwidthScalabilityMode : 1;
					}PACKED;
					uint8_t			u8Val = 0;
				}PACKED;

				CELPHEADER()
					: ExcitationMode(0), SampleRateMode(0), FineRateControl(0), reserved_0(0) {
				}

				int Unpack(CBitstream& bs)
				{
					ExcitationMode = (uint8_t)bs.GetBits(1);
					SampleRateMode = (uint8_t)bs.GetBits(1);
					FineRateControl = (uint8_t)bs.GetBits(1);

					if (ExcitationMode == RPE)
					{
						RPE_Configuration = (uint8_t)bs.GetBits(3);
					}
					else
					{
						MPE_Configuration = (uint8_t)bs.GetBits(5);
						NumEnhLayers = (uint8_t)bs.GetBits(2);
						BandwidthScalabilityMode = (uint8_t)bs.GetBits(1);
					}

					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_NUMBER1(ExcitationMode, 1, ExcitationMode ? "the Regular-Pulse Excitation tool is used" : "the Multi-Pulse Excitation tool is used");
					NAV_FIELD_PROP_NUMBER1(SampleRateMode, 1, SampleRateMode ? "16 kHz Sampling rate" : "8 kHz Sampling rate");
					NAV_FIELD_PROP_NUMBER1(FineRateControl, 1, FineRateControl ? "fine rate control in very fine steps is enabled" : "fine rate control in very fine steps is disabled");

					if (ExcitationMode)
					{
						NAV_FIELD_PROP_2NUMBER1(RPE_Configuration, 3, "an identifier which configures the MPEG-4 CELP coder using the Regular-Pulse "
							"Excitation tool.This parameter directly determines the set of allowed bitrates(Table 3.53) and the number of "
							"subframes in a CELP frame(Table 3.54)");
					}
					else
					{
						NAV_FIELD_PROP_2NUMBER1(MPE_Configuration, 5, "configures the MPEG-4 CELP coder using the Multi-Pulse Excitation "
							"tool.This parameter determines the variables nrof_subframes and nrof_subframes_bws.This parameter also "
							"specifies the number of bits for shape_positions[i], shape_signs[i], shape_enh_positions[i][j] and shape_enh_signs[i][j]");
						NAV_FIELD_PROP_2NUMBER1(NumEnhLayers, 2, "the number of enhancement layers that are used");
						NAV_FIELD_PROP_NUMBER1(BandwidthScalabilityMode, 1, BandwidthScalabilityMode ? "Bandwidth scalability is enabled" : "Bandwidth scalability is disabled");
					}
				DECLARE_FIELDPROP_END()
			}PACKED;

			uint8_t		isBaseLayer;		// 1 bit
			union {
				CELPHEADER	CelpHeader;
				struct {
					uint8_t	isBWSLayer;
					union {
						uint8_t	BWS_configuration;
						uint8_t	CELP_BRS_id;
					};
				}PACKED;
			}PACKED;

			uint8_t				samplingFrequencyIndex;

			CelpSpecificConfig(uint8_t nSamplingFrequencyIndex)
				: isBaseLayer(0), samplingFrequencyIndex(nSamplingFrequencyIndex) {
			}

			int Unpack(CBitstream& bs)
			{
				isBaseLayer = (uint8_t)bs.GetBits(1);
				if (isBaseLayer)
				{
					CelpHeader.Unpack(bs);
				}
				else
				{
					isBWSLayer = (uint8_t)bs.GetBits(1);
					if (isBWSLayer)
						BWS_configuration = (uint8_t)bs.GetBits(2);
					else
						CELP_BRS_id = (uint8_t)bs.GetBits(2);
				}

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_NUMBER_BEGIN("isBaseLayer", 1, isBaseLayer, isBaseLayer?"the corresponding layer is the base layer":
				"the corresponding layer is an bandwidth scalable or bitrate scalable enhancement layer");
			if (isBaseLayer)
			{
				NAV_WRITE_TAG_BEGIN2_1("CelpHeader", "");
				NAV_FIELD_PROP_OBJECT(CelpHeader);
				NAV_WRITE_TAG_END("CelpHeader");
			}
			else
			{
				NAV_FIELD_PROP_NUMBER_BEGIN("isBWSLayer", 1, isBWSLayer, isBWSLayer?"the corresponding layer is the bandwidth scalable enhancement layer":
					"the corresponding layer is the bitrate scalable enhancement layer");
				
				if (isBWSLayer)
				{
					NAV_FIELD_PROP_2NUMBER1(BWS_configuration, 2, "configures the bandwidth extension tool, and specifies the number of bits for shape_bws_positions[i], shape_bws_signs[i]");
				}
				else
				{
					NAV_FIELD_PROP_2NUMBER1(CELP_BRS_id, 2, "representing the order of the bitrate scalable enhancement layers, where the first enhancement layer has the value of '1'.The value of '0' should not be used");
				}

				NAV_FIELD_PROP_NUMBER_END("isBWSLayer");
			}
			NAV_FIELD_PROP_END("isBaseLayer");
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct TTSSpecificConfig
		{
			struct TTS_Sequence
			{
				uint32_t			TTS_Sequence_ID : 5;
				uint32_t			Language_Code : 18;
				uint32_t			Gender_Enable : 1;
				uint32_t			Age_Enable : 1;
				uint32_t			Speech_Rate_Enable : 1;
				uint32_t			Prosody_Enable : 1;
				uint32_t			Video_Enable : 1;
				uint32_t			Lip_Shape_Enable : 1;
				uint32_t			Trick_Mode_Enable : 1;
				uint32_t			padding : 2;

				TTS_Sequence()
					: TTS_Sequence_ID(0), Language_Code(0), Gender_Enable(0), Age_Enable(0), Speech_Rate_Enable(0), Prosody_Enable(0)
					, Video_Enable(0), Lip_Shape_Enable(0), Trick_Mode_Enable(0), padding(0) {
				}

				virtual int Unpack(CBitstream& bs)
				{
					TTS_Sequence_ID = (uint32_t)bs.GetBits(5);
					Language_Code = (uint32_t)bs.GetBits(18);
					Gender_Enable = (uint32_t)bs.GetBits(1);
					Age_Enable = (uint32_t)bs.GetBits(1);
					Speech_Rate_Enable = (uint32_t)bs.GetBits(1);
					Prosody_Enable = (uint32_t)bs.GetBits(1);
					Video_Enable = (uint32_t)bs.GetBits(1);
					Lip_Shape_Enable = (uint32_t)bs.GetBits(1);
					Trick_Mode_Enable = (uint32_t)bs.GetBits(1);
					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER1(TTS_Sequence_ID, 5, "uniquely identify each TTS object appearing in one scene");
					NAV_FIELD_PROP_2NUMBER1(Language_Code, 18, "");
					NAV_FIELD_PROP_NUMBER1(Gender_Enable, 1, "");
					NAV_FIELD_PROP_NUMBER1(Age_Enable, 1, "");
					NAV_FIELD_PROP_NUMBER1(Speech_Rate_Enable, 1, "");
					NAV_FIELD_PROP_NUMBER1(Prosody_Enable, 1, "");
					NAV_FIELD_PROP_NUMBER1(Video_Enable, 1, "");
					NAV_FIELD_PROP_NUMBER1(Lip_Shape_Enable, 1, "");
					NAV_FIELD_PROP_NUMBER1(Trick_Mode_Enable, 1, "");
				DECLARE_FIELDPROP_END()
			}PACKED;

			TTS_Sequence		tts_sequence;

			virtual ~TTSSpecificConfig() {}

			virtual int Unpack(CBitstream& bs) {
				return tts_sequence.Unpack(bs);
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_WRITE_TAG_BEGIN("TTS_Sequence");
				NAV_FIELD_PROP_OBJECT(tts_sequence);
				NAV_WRITE_TAG_END("TTS_Sequence");
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct ALSSpecificConfig
		{
			uint32_t			als_id;
			uint32_t			samp_freq;
			uint32_t			samples;
			uint16_t			channels;
			uint8_t				file_type : 3;
			uint8_t				resolution : 3;
			uint8_t				floating : 1;
			uint8_t				msb_first : 1;
			uint16_t			frame_length;
			uint8_t				random_access;
			uint16_t			ra_flag:2;
			uint16_t			adapt_order:1;
			uint16_t			coef_table:2;
			uint16_t			long_term_prediction:1;
			uint16_t			max_order:10;
			uint16_t			block_switching : 2;
			uint16_t			bgmc_mode : 1;
			uint16_t			sb_part : 1;
			uint16_t			joint_stereo : 1;
			uint16_t			mc_coding : 1;
			uint16_t			chan_config : 1;
			uint16_t			chan_sort : 1;
			uint16_t			crc_enabled : 1;
			uint16_t			RLSLMS : 1;
			uint16_t			reserved : 5;
			uint16_t			aux_data_enabled : 1;
			uint16_t			chan_config_info;
			std::vector<uint16_t>
								chan_poses;
			// byte_align; 0..7 bslbf
			uint32_t			header_size;
			uint32_t			trailer_size;
			std::vector<uint8_t>
								orig_header;
			std::vector<uint8_t>
								orig_trailer;
			uint32_t			crc;
			std::vector<uint32_t>
								ra_unit_size;
			uint32_t			aux_size;
			std::vector<uint8_t>
								aux_data;

			int Unpack(CBitstream& bs)
			{
				als_id = bs.GetDWord();
				samp_freq = bs.GetDWord();
				samples = bs.GetDWord();
				channels = bs.GetWord();
				file_type = (uint8_t)bs.GetBits(3);
				resolution = (uint8_t)bs.GetBits(3);
				floating = (uint8_t)bs.GetBits(1);
				msb_first = (uint8_t)bs.GetBits(1);
				frame_length = bs.GetWord();
				random_access = bs.GetByte();
				ra_flag = (uint16_t)bs.GetBits(2);
				ra_flag = (uint16_t)bs.GetBits(1);
				adapt_order = (uint16_t)bs.GetBits(2);
				coef_table = (uint16_t)bs.GetBits(1);
				long_term_prediction = (uint16_t)bs.GetBits(10);
				max_order = (uint16_t)bs.GetBits(2);
				block_switching = (uint16_t)bs.GetBits(1);
				bgmc_mode = (uint16_t)bs.GetBits(1);
				sb_part = (uint16_t)bs.GetBits(1);
				joint_stereo = (uint16_t)bs.GetBits(1);
				mc_coding = (uint16_t)bs.GetBits(1);
				chan_config = (uint16_t)bs.GetBits(1);
				chan_sort = (uint16_t)bs.GetBits(1);
				crc_enabled = (uint16_t)bs.GetBits(1);
				RLSLMS = (uint16_t)bs.GetBits(1);
				reserved = (uint16_t)bs.GetBits(5);
				aux_data_enabled = (uint16_t)bs.GetBits(1);

				if (chan_config)
					chan_config_info = bs.GetWord();

				if (chan_sort)
				{
					chan_poses.reserve(channels);
					for (size_t c = 0; c <= channels; c++)
					{
						chan_poses.push_back(quick_ceil_log2(channels + 1));
					}
				}
				bs.Realign();

				header_size = bs.GetDWord();
				trailer_size = bs.GetDWord();

				if (header_size > 0 && header_size != UINT32_MAX)
				{
					orig_header.reserve(header_size);
					for (size_t i = 0; i < header_size; i++)
						orig_header.push_back(bs.GetByte());
				}

				if (trailer_size > 0 && trailer_size != UINT32_MAX)
				{
					orig_trailer.reserve(trailer_size);
					for (size_t i = 0; i < trailer_size; i++)
						orig_trailer.push_back(bs.GetByte());
				}

				if (crc_enabled)
					crc = bs.GetDWord();

				if ((ra_flag == 2) && (random_access > 0)) {
					ra_unit_size.reserve(((samples - 1) / (frame_length + 1)));
					for (size_t f = 0; f < ((samples - 1) / (frame_length + 1)); f++)
						ra_unit_size.push_back(bs.GetDWord());
				}

				if (aux_data_enabled)
				{
					aux_size = bs.GetDWord();
					aux_data.reserve(aux_size);
					for (size_t i = 0; i < aux_size; i++)
						aux_data.push_back(bs.GetByte());
				}

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(als_id, 32, "ALS identifier, should be 0x414C5300");
				NAV_FIELD_PROP_2NUMBER1(samp_freq, 32, "Sampling frequency in Hz");
				NAV_FIELD_PROP_2NUMBER1(samples, 32, "Number of samples (per channel). If samples = 0xFFFFFFFF (Hex), the number of samples is not specified");
				NAV_FIELD_PROP_2NUMBER1(channels, 16, channels==0?"mono":(channels==1?"stereo":"Number of channels-1"));
				NAV_FIELD_PROP_2NUMBER1(file_type, 3, file_type == 0 ? "Unknown/raw file" : (file_type == 1 ? "wave file" : (file_type == 2 ? "aiff file" : (file_type == 3 ? "bwf file" : "reserved"))));
				NAV_FIELD_PROP_2NUMBER1(resolution, 3, resolution == 0 ? "8-bit" : (resolution == 1 ? "16-bit" : (resolution == 2 ? "24-bit" : (resolution == 3 ? "32-bit" : "reserved"))));
				NAV_FIELD_PROP_NUMBER1(floating, 1, floating ? "IEEE 32-bit floating-point" : "integer");
				NAV_FIELD_PROP_NUMBER1(msb_first, 1, msb_first ? "big-endian" : "little-endian");
				NAV_FIELD_PROP_2NUMBER1(frame_length, 16, "plus 1 signals a frame length");
				NAV_FIELD_PROP_2NUMBER1(random_access, 8, random_access==0?"no RA is used":(random_access ==1?"each frame is an RA frame":"Distance between RA frames"));
				NAV_FIELD_PROP_2NUMBER1(ra_flag, 2, ra_flag == 0 ? "the size of random access units is not stored" : (
													ra_flag == 1 ? "the size of random access units is stored at the beginning of frame_data()" : (
													ra_flag == 2 ? "the size of random access units is stored at the end of ALSSpecificConfig()":"")));
				NAV_FIELD_PROP_NUMBER1(adapt_order, 1, adapt_order ? "Adaptive Order: on" : "Adaptive Order: off");
				NAV_FIELD_PROP_NUMBER1(coef_table, 2, coef_table == 3 ? "no entropy coding" : "Table index of Rice code parameters for entropy coding of predictor coefficients");
				NAV_FIELD_PROP_NUMBER1(long_term_prediction, 1, long_term_prediction ? "Long term prediction (LTP): on" : "Long term prediction (LTP): off");
				NAV_FIELD_PROP_NUMBER1(max_order, 10, "Maximum prediction order");
				NAV_FIELD_PROP_NUMBER1(block_switching, 2, block_switching == 0 ? "no block switching" : (
														   block_switching == 1 ? "up to 3 levels" : (
														   block_switching == 2 ? "4 levels" : (
														   block_switching == 3 ? "5 levels" : "Unknown"))));
				NAV_FIELD_PROP_NUMBER1(bgmc_mode, 1, bgmc_mode ? "BGMC Mode: on" : "BGMC Mode: off");
				NAV_FIELD_PROP_NUMBER1(sb_part, 1, bgmc_mode == 0 ? (sb_part ? "1:4 partition, one ec_sub bit in block_data" : "no partition, no ec_sub bit in block_data") : 
					(sb_part ? "1:2:4:8 partition, two ec_sub bits in block_data" : "1:4 partition, one ec_sub bit in block_data"));
				NAV_FIELD_PROP_NUMBER1(joint_stereo, 1, joint_stereo ? "Joint Stereo: on" : "Joint Stereo: off");
				NAV_FIELD_PROP_NUMBER1(mc_coding, 1, mc_coding ? "Extended inter-channel coding: on" : "Extended inter-channel coding: off");
				NAV_FIELD_PROP_NUMBER1(chan_config, 1, chan_config ? "a chan_config_info field is present" : "a chan_config_info field is NOT present");
				NAV_FIELD_PROP_NUMBER1(chan_sort, 1, chan_sort ? "Channel rearrangement: on" : "Channel rearrangement: off");
				NAV_FIELD_PROP_NUMBER1(crc_enabled, 1, crc_enabled ? "the crc field is present" : "the crc field is NOT present");
				NAV_FIELD_PROP_NUMBER1(RLSLMS, 1, RLSLMS ? "Use RLS-LMS predictor: on" : "Use RLS-LMS predictor: off");
				NAV_FIELD_PROP_NUMBER1(reserved, 5, "");
				NAV_FIELD_PROP_NUMBER1(aux_data_enabled, 1, aux_data_enabled ? "auxiliary data is present" : "auxiliary data is NOT present");

				if (chan_config) {
					NAV_FIELD_PROP_2NUMBER1(chan_config_info, 16, "Mapping of channels to loudspeaker locations. "
						"Each bit indicates whether a channel for a "
						"particular predefined location exists");
				}

				if (chan_sort)
				{
					NAV_WRITE_TAG_BEGIN2_1("chan_pos", "the original channel positions");
					for (size_t c = 0; c <= channels; c++)
					{
						NAV_ARRAY_FIELD_PROP_2NUMBER_("chan_pos", "ch#", c, true, quick_ceil_log2(channels + 1), chan_poses[c], "");
					}
					NAV_WRITE_TAG_END("chan_pos");
				}
				
				if (bit_offset)
					*bit_offset = (*bit_offset % 8) == 0 ? 0 : (8 - *bit_offset % 8);

				NAV_FIELD_PROP_2NUMBER1(header_size, 32, "Header size of original audio file in bytes");
				NAV_FIELD_PROP_2NUMBER1(trailer_size, 32, "Trailer size of original audio file in bytes");

				if (header_size > 0 && header_size != UINT32_MAX)
				{
					NAV_WRITE_TAG_BEGIN2_1("orig_header", "Header of original audio file");
					for (size_t i = 0; i < orig_header.size(); i++)
					{
						NAV_ARRAY_FIELD_PROP_2NUMBER_("orig_header", "", i, true, 8, orig_header[i], "");
					}
					NAV_WRITE_TAG_END("orig_header");
				}

				if (trailer_size > 0 && trailer_size != UINT32_MAX)
				{
					NAV_WRITE_TAG_BEGIN2_1("orig_trailer", "Trailer of original audio file");
					for (size_t i = 0; i < orig_trailer.size(); i++)
					{
						NAV_ARRAY_FIELD_PROP_2NUMBER_("orig_trailer", "", i, true, 8, orig_trailer[i], "");
					}
					NAV_WRITE_TAG_END("orig_trailer");
				}

				if (crc_enabled)
				{
					NAV_FIELD_PROP_2NUMBER1(crc, 32, "32-bit CCITT-32 CRC checksum of the original audio data bytes");
				}

				if ((ra_flag == 2) && (random_access > 0)) {
					NAV_WRITE_TAG_BEGIN2_1("ra_unit_size", "Distances (in bytes) between the random access frames, i.e.the sizes of the random access units, where the number of frames is");
					for (size_t i = 0; i < ra_unit_size.size(); i++)
					{
						NAV_ARRAY_FIELD_PROP_2NUMBER_("ra_unit_size", "", i, true, 32, ra_unit_size[i], "");
					}
					NAV_WRITE_TAG_END("ra_unit_size");
				}

				if (aux_data_enabled)
				{
					NAV_FIELD_PROP_2NUMBER1(aux_size, 32, "Size of the aux_data field in bytes");
					NAV_WRITE_TAG_BEGIN2_1("aux_data", "Auxiliary data (not required for decoding)");
					for (size_t i = 0; i < aux_data.size(); i++)
					{
						NAV_ARRAY_FIELD_PROP_2NUMBER_("aux_data", "", i, true, 32, aux_data[i], "");
					}
					NAV_WRITE_TAG_END("aux_data");
				}
			DECLARE_FIELDPROP_END()

		};

		struct AudioSpecificConfig : public MPEG4System::DecoderSpecificInfo
		{
			uint8_t				audioObjectType_1 = 0;					// 5 bits
			uint8_t				audioObjectTypeExt_1 = 0;				// 6 bits

			uint8_t				samplingFrequencyIndex = 0;				// 4 bits
			uint32_t			samplingFrequency = 0;					// 24 bits

			uint8_t				channelConfiguration = 0;				// 4 bits

			uint8_t				extensionSamplingFrequencyIndex = 0;	// 4 bits
			uint32_t			extensionSamplingFrequency = 0;			// 24 bit

			uint8_t				audioObjectType_2 = 0;					// 5 bits
			uint8_t				audioObjectTypeExt_2 = 0;				// 6 bits

			uint8_t				audioObjectType = 0;

			uint8_t				extensionChannelConfiguration = 0;		// 4 bits

			uint8_t				sacPayloadEmbedding : 1;			// for SpatialSpecificConfig
			uint8_t				fillBits : 5;						// for ALSSpecificConfig()
			uint8_t				padding_0 : 2;

			union
			{
				GASpecificConfig*	GASpecConfig;
				CelpSpecificConfig*	CelpSpecConfig;
				TTSSpecificConfig*	TTSSpecConfig;
				ALSSpecificConfig*	ALSSpecConfig;
				void*				pConfig = nullptr;
			};

			AudioSpecificConfig()
				: audioObjectType(0), sacPayloadEmbedding(0), fillBits(0), padding_0(0){
			}

			virtual ~AudioSpecificConfig()
			{
				switch (audioObjectType)
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 7:
				case 17:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
					AMP_SAFEDEL2(GASpecConfig);
					break;
				case 8:
					AMP_SAFEDEL2(CelpSpecConfig);
					break;
				case 9:
					break;
				case 12:
					AMP_SAFEDEL2(TTSSpecConfig);
					break;
				case 36:
					AMP_SAFEDEL2(ALSSpecConfig);
					break;
				}
			}

			bool IsGASpecificConfigAvail()
			{
				switch (audioObjectType)
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 7:
				case 17:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
					return true;
				}

				return false;
			}

			INLINE uint8_t GetAudioObjectType()
			{
				if (audioObjectType_1 < 0x1F)
					return audioObjectType_1;

				return 32 + audioObjectTypeExt_1;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = MPEG4System::DecoderSpecificInfo::Unpack(bs);
				if (iRet < 0)
					return iRet;

				int sbrPresentFlag = -1;
				int psPresentFlag = -1;
				uint8_t extensionAudioObjectType = 0;
				uint64_t left_bits = (uint64_t)LeftBytes(bs)<<3;

				AAC_CHECK_LEFT_BITS(5);
				audioObjectType_1 = (uint8_t)bs.GetBits(5);
				left_bits -= 5;

				if (audioObjectType_1 == 0x1F)
				{
					AAC_CHECK_LEFT_BITS(6);
					audioObjectTypeExt_1 = (uint8_t)bs.GetBits(6);
					left_bits -= 6;
					audioObjectType = 32 + audioObjectTypeExt_1;
				}
				else
					audioObjectType = audioObjectType_1;

				AAC_CHECK_LEFT_BITS(4);
				samplingFrequencyIndex = (uint8_t)bs.GetBits(4);
				left_bits -= 4;
				if (samplingFrequency == 0xF)
				{
					AAC_CHECK_LEFT_BITS(24);
					samplingFrequency = (uint32_t)bs.GetBits(24);
					left_bits -= 24;
				}

				AAC_CHECK_LEFT_BITS(4);
				channelConfiguration = (uint8_t)bs.GetBits(4);
				left_bits -= 4;

				if (audioObjectType_1 == 5 || audioObjectType_1 == 29)
				{
					extensionAudioObjectType = 5;
					sbrPresentFlag = 1;
					if (audioObjectType_1 == 29)
						psPresentFlag = 1;

					AAC_CHECK_LEFT_BITS(4);
					extensionSamplingFrequencyIndex = (uint8_t)bs.GetBits(4);
					left_bits -= 4;

					if (extensionSamplingFrequencyIndex == 0xF) {
						AAC_CHECK_LEFT_BITS(24);
						extensionSamplingFrequency = (uint32_t)bs.GetBits(24);
						left_bits -= 24;
					}

					AAC_CHECK_LEFT_BITS(5);
					audioObjectType_2 = (uint8_t)bs.GetBits(5);
					left_bits -= 5;

					if (audioObjectType_2 == 0x1F)
					{
						AAC_CHECK_LEFT_BITS(6);
						audioObjectTypeExt_2 = (uint8_t)bs.GetBits(6);
						left_bits -= 6;
						audioObjectType = 32 + audioObjectTypeExt_2;
					}
					else
						audioObjectType = audioObjectType_2;

					if (audioObjectType == 22)
					{
						AAC_CHECK_LEFT_BITS(4);
						extensionChannelConfiguration = (uint8_t)bs.GetBits(4);
						left_bits -= 4;
					}
				}
				else
					extensionAudioObjectType = 0;

				switch (audioObjectType)
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 7:
				case 17:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
					AMP_NEWT(GASpecConfig, GASpecificConfig, audioObjectType, channelConfiguration);
					if ((iRet = GASpecConfig->Unpack(bs)) < 0)
						goto done;
					break;
				case 8:
					AMP_NEWT(CelpSpecConfig, CelpSpecificConfig, samplingFrequencyIndex);
					if ((iRet = CelpSpecConfig->Unpack(bs)) < 0)
						goto done;
					break;
				case 9:
					// HvxcSpecificConfig()
					break;
				case 12:
					AMP_NEWT(TTSSpecConfig, TTSSpecificConfig);
					if ((iRet = TTSSpecConfig->Unpack(bs)) < 0)
						goto done;
					break;
				case 13:
				case 14:
				case 15:
				case 16:
					// StructuredAudioSpecificConfig();
					break;
				case 24:
					// ErrorResilientCelpSpecificConfig();
					break;
				case 25:
					// ErrorResilientHvxcSpecificConfig();
					break;
				case 26:
				case 27:
					// ParametricSpecificConfig();
					break;
				case 28:
					// SSCSpecificConfig();
					break;
				case 30:
					sacPayloadEmbedding = (uint8_t)bs.GetBits(1);
					// SpatialSpecificConfig();
					break;
				case 32:
				case 33:
				case 34:
					// MPEG_1_2_SpecificConfig();
					break;
				case 35:
					// DSTSpecificConfig();
					break;
				case 36:
					fillBits = (uint8_t)bs.GetBits(5);
					AMP_NEWT(ALSSpecConfig, ALSSpecificConfig);
					if ((iRet = ALSSpecConfig->Unpack(bs)) < 0)
						goto done;
					break;
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
				BASECLASS_IMPLEMENT(MPEG4System::DecoderSpecificInfo);
				uint8_t ConcludedAudioObjectType = 0;
				if (audioObjectType_1 < 0x1F)
				{
					ConcludedAudioObjectType = audioObjectType_1;
					NAV_FIELD_PROP_2NUMBER("audioObjectType", 5, audioObjectType_1, Audio_Object_Type_Names[audioObjectType_1]);
				}
				else
				{
					ConcludedAudioObjectType = 32 + audioObjectTypeExt_1;
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN("audioObjectType", "", ConcludedAudioObjectType, Audio_Object_Type_Names[ConcludedAudioObjectType]);
						NAV_FIELD_PROP_2NUMBER("audioObjectType", 5, audioObjectType_1, "");
						NAV_FIELD_PROP_2NUMBER("audioObjectTypeExt", 5, audioObjectTypeExt_1, "");
					NAV_WRITE_TAG_END("audioObjectType");
				}

				if (samplingFrequencyIndex < 0xF)
				{
					NAV_FIELD_PROP_2NUMBER1(samplingFrequencyIndex, 4, std::get<1>(samplingFrequencyIndex_names[samplingFrequencyIndex]));
				}
				else
				{
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN("SamplingFrequency", "", samplingFrequency, "HZ");
						NAV_FIELD_PROP_2NUMBER1(samplingFrequencyIndex, 4, "Should be 0xF");
						NAV_FIELD_PROP_2NUMBER1(samplingFrequency, 24, "HZ");
					NAV_WRITE_TAG_END("SamplingFrequency");
				}

				NAV_FIELD_PROP_2NUMBER1(channelConfiguration, 4, std::get<2>(channelConfiguration_names[channelConfiguration]));

				if (audioObjectType_1 == 5 || audioObjectType_1 == 29)
				{
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE("extensionAudioObjectType", 5, "");
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE("sbrPresentFlag", 1, "");
					if (audioObjectType_1 == 29)
					{
						NAV_WRITE_TAG_WITH_1NUMBER_VALUE("psPresentFlag", 1, "");
					}

					MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%s, the output sampling frequency of the extension tool corresponding to the extensionAudioObjectType",
						std::get<1>(samplingFrequencyIndex_names[extensionSamplingFrequencyIndex]));
					NAV_FIELD_PROP_NUMBER_BEGIN("extensionSamplingFrequencyIndex", 4, extensionSamplingFrequencyIndex, szTemp2);
					if (extensionSamplingFrequencyIndex == 0xF) {
						NAV_FIELD_PROP_2NUMBER1(extensionSamplingFrequency, 24, "The output sampling frequency of the extension tool corresponding to the extensionAudioObjectType");
					}
					NAV_FIELD_PROP_NUMBER_END("extensionSamplingFrequencyIndex");

					if (audioObjectType_2 < 0x1F)
					{
						ConcludedAudioObjectType = audioObjectType_2;
						NAV_FIELD_PROP_2NUMBER("audioObjectType", 5, audioObjectType_2, Audio_Object_Type_Names[audioObjectType_2]);
					}
					else
					{
						ConcludedAudioObjectType = 32 + audioObjectTypeExt_2;

						NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN("audioObjectType", "", ConcludedAudioObjectType, Audio_Object_Type_Names[ConcludedAudioObjectType]);
							NAV_FIELD_PROP_2NUMBER("audioObjectType", 5, audioObjectType_2, "");
							NAV_FIELD_PROP_2NUMBER("audioObjectTypeExt", 5, audioObjectTypeExt_2, "");
						NAV_WRITE_TAG_END("audioObjectType");
					}

					if (ConcludedAudioObjectType == 22)
					{
						NAV_FIELD_PROP_2NUMBER1(extensionChannelConfiguration, 4, "the channel configuration of the BSAC extensions");
					}
				}

				switch (ConcludedAudioObjectType)
				{
				case 1:
				case 2:
				case 3:
				case 4:
				case 6:
				case 7:
				case 17:
				case 19:
				case 20:
				case 21:
				case 22:
				case 23:
					if (GASpecConfig != nullptr)
					{
						NAV_FIELD_PROP_REF2_1(GASpecConfig, "GASpecificConfig", "");
					}
					break;
				case 8:
					if (CelpSpecConfig != nullptr)
					{
						NAV_FIELD_PROP_REF2_1(CelpSpecConfig, "CelpSpecificConfig", "");
					}
					break;
				case 9:
					// HvxcSpecificConfig()
					break;
				case 12:
					if (TTSSpecConfig != nullptr)
					{
						NAV_FIELD_PROP_REF2_1(TTSSpecConfig, "TTSSpecificConfig", "");
					}
					break;
				case 13:
				case 14:
				case 15:
				case 16:
					// StructuredAudioSpecificConfig();
					break;
				case 24:
					// ErrorResilientCelpSpecificConfig();
					break;
				case 25:
					// ErrorResilientHvxcSpecificConfig();
					break;
				case 26:
				case 27:
					// ParametricSpecificConfig();
					break;
				case 28:
					// SSCSpecificConfig();
					break;
				case 30:
					NAV_FIELD_PROP_NUMBER1(sacPayloadEmbedding, 1, "");
					// SpatialSpecificConfig();
					break;
				case 32:
				case 33:
				case 34:
					// MPEG_1_2_SpecificConfig();
					break;
				case 35:
					// DSTSpecificConfig();
					break;
				case 36:
					NAV_FIELD_PROP_2NUMBER1(fillBits, 5, "Fill bits for byte alignment of ALSSpecificConfig() relative to the start of AudioSpecificConfig()");
					if (ALSSpecConfig != nullptr)
					{
						NAV_FIELD_PROP_REF2_1(ALSSpecConfig, "ALSSpecificConfig", "");
					}
					break;
				}

			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CAudioStreamContext
		{
			AudioSpecificConfig*
							audio_specific_config;
			uint16_t		ltp_prev_lag;
			uint8_t			common_window;
			uint8_t			Object_Type_ID;
			uint8_t			sampling_frequency_index;	// Passed from ADTS header or ADIF stream

			uint16_t GetWindowLength()
			{
				if (audio_specific_config != nullptr)
				{
					if (audio_specific_config->GetAudioObjectType() == ER_AAC_LD)
					{
						if (audio_specific_config->GASpecConfig &&
							audio_specific_config->GASpecConfig->frameLengthFlag)
							return 960;
						else
							return 1024;
					}
					else
					{
						if (audio_specific_config->GASpecConfig &&
							audio_specific_config->GASpecConfig->frameLengthFlag)
							return 1920;
						else
							return 2048;
					}
				}

				return 2048;
			}

			uint8_t GetSamplingFrequencyIndex() {
				if (audio_specific_config)
					return audio_specific_config->samplingFrequencyIndex;
				
				return sampling_frequency_index;
			}

		}PACKED;

		struct EXTENSION_PAYLOAD
		{
			enum
			{
				EXT_FIL = 0,
				EXT_FILL_DATA = 1,
				EXT_DATA_ELEMENT = 2,
				EXT_DYNAMIC_RANGE = 11,
				EXT_SAC_DATA = 12,
				EXT_SBR_DATA = 13,
				EXT_SBR_DATA_CRC = 14,
			};

			struct DYNAMIC_RANGE_INFO
			{
				struct EXCLUDED_CHANNELS
				{
					CAMBitArray		exclude_mask;
					CAMBitArray		additional_excluded_chns;

					int Unpack(CBitstream& bs)
					{
						int n = 0, num_excl_chan = 7, i = 0;
						for (; i < 7; i++)
							bs.GetBits(1) ? exclude_mask.BitSet(i) : exclude_mask.BitClear(i);

						n++;
						int nAdditionalExcludedChns = (int)bs.GetBits(1);
						nAdditionalExcludedChns ? additional_excluded_chns.BitSet(n - 1) : additional_excluded_chns.BitClear(n - 1);
						while (nAdditionalExcludedChns == 1) {
							for (i = num_excl_chan; i < num_excl_chan + 7; i++)
								bs.GetBits(1) ? exclude_mask.BitSet(i) : exclude_mask.BitClear(i);
							n++;
							num_excl_chan += 7;
						}
						return n;
					}
				};

				uint8_t			pce_tag_present : 1;
				uint8_t			excluded_chns_present : 1;
				uint8_t			drc_bands_present : 1;
				uint8_t			prog_ref_level_present : 1;
				uint8_t			reserved : 4;

				uint8_t			pce_instance_tag : 4;
				uint8_t			drc_tag_reserved_bits : 4;

				EXCLUDED_CHANNELS
								excluded_channels;

				uint8_t			drc_band_incr : 4;
				uint8_t			drc_interpolation_scheme : 4;

				std::vector<uint8_t>
								drc_band_top;

				uint8_t			prog_ref_level : 7;
				uint8_t			prog_ref_level_reserved_bits : 1;

				std::vector<uint8_t>
								dyn_rng;		// dyn_rng_sgn: 1, dyn_rng_ctl: 7

				int Unpack(CBitstream& bs)
				{
					int n = 1, nRet = 0;
					int drc_num_bands = 1;
					pce_tag_present = (uint8_t)bs.GetBits(1);
					if (pce_tag_present)
					{
						pce_instance_tag = (uint8_t)bs.GetBits(4);
						drc_tag_reserved_bits = (uint8_t)bs.GetBits(4);
						n++;
					}

					excluded_chns_present = (uint8_t)bs.GetBits(1);
					if (excluded_chns_present)
					{
						n += excluded_channels.Unpack(bs);
					}

					drc_bands_present = (uint8_t)bs.GetBits(1);
					if (drc_bands_present)
					{
						drc_band_incr = (uint8_t)bs.GetBits(4);
						drc_interpolation_scheme = (uint8_t)bs.GetBits(4);

						n++;
						drc_num_bands += drc_band_incr;
						for (int i = 0; i < drc_num_bands; i++) {
							drc_band_top.push_back(bs.GetByte());
							n++;
						}
					}

					prog_ref_level_present = bs.GetBits(1);
					if (prog_ref_level_present)
					{
						prog_ref_level = (uint8_t)bs.GetBits(7);
						prog_ref_level_reserved_bits = (uint8_t)bs.GetBits(1);
						n++;
					}

					for (int i = 0; i < drc_num_bands; i++)
					{
						dyn_rng.push_back(bs.GetByte());
						n++;
					}

					return n;
				}

			};

			struct SAC_EXTENSION_DATA
			{
				uint8_t			ancType : 2;
				uint8_t			ancStart : 1;
				uint8_t			ancStop : 1;
				uint8_t			byte_align : 4;
				std::vector<uint8_t>
								ancDataSegmentBytes;

				SAC_EXTENSION_DATA(int cntBytes)
					: ancType(0), ancStart(0), ancStop(0), byte_align(0){
					if (cntBytes > 1)
						ancDataSegmentBytes.resize((size_t)cntBytes - 1);
				}

				int Unpack(CBitstream& bs)
				{
					ancType = (uint8_t)bs.GetBits(2);
					ancStart = (uint8_t)bs.GetBits(1);
					ancStop = (uint8_t)bs.GetBits(1);

					for (size_t i = 0; i < ancDataSegmentBytes.size(); i++) {
						ancDataSegmentBytes.push_back(bs.GetByte());
					}
					return (int)(ancDataSegmentBytes.size() + 1);
				}
			};

			struct SBR_EXTENSION_DATA
			{
				struct SBR_HEADER
				{
					union
					{
						struct
						{
							uint16_t		bs_amp_res : 1;
							uint16_t		bs_start_freq : 4;
							uint16_t		bs_stop_freq : 4;
							uint16_t		bs_xover_band : 3;
							uint16_t		bs_reserved : 2;
							uint16_t		bs_header_extra_1 : 1;
							uint16_t		bs_header_extra_2 : 1;

							uint8_t			bs_freq_scale : 2;
							uint8_t			bs_alter_scale : 1;
							uint8_t			bs_noise_bands : 2;
							uint8_t			byte_align_0 : 3;

							uint8_t			bs_limiter_bands : 2;
							uint8_t			bs_limiter_gains : 2;
							uint8_t			bs_interpol_freq : 1;
							uint8_t			bs_smoothing_mode : 1;
							uint8_t			byte_align_1 : 2;
						} PACKED;
						uint8_t				ubytes[4] = { 0 };
					} PACKED;

					int Unpack(CBitstream& bs)
					{
						bs_amp_res = (uint16_t)bs.GetBits(1);
						bs_start_freq = (uint16_t)bs.GetBits(4);
						bs_stop_freq = (uint16_t)bs.GetBits(4);
						bs_xover_band = (uint16_t)bs.GetBits(3);
						bs_reserved = (uint16_t)bs.GetBits(2);
						bs_header_extra_1 = (uint16_t)bs.GetBits(1);
						bs_header_extra_2 = (uint16_t)bs.GetBits(1);

						if (bs_header_extra_1)
						{
							bs_freq_scale = (uint8_t)bs.GetBits(2);
							bs_alter_scale = (uint8_t)bs.GetBits(1);
							bs_noise_bands = (uint8_t)bs.GetBits(2);
						}

						if (bs_header_extra_2)
						{
							bs_limiter_bands = (uint8_t)bs.GetBits(2);
							bs_limiter_gains = (uint8_t)bs.GetBits(2);
							bs_interpol_freq = (uint8_t)bs.GetBits(1);
						}

						return RET_CODE_SUCCESS;
					}
				};

				struct SBR_GRID
				{
					enum
					{
						FIXFIX = 0,
						FIXVAR = 1,
						VARFIX = 2,
						VARVAR = 3,
					};

					uint8_t			bs_frame_class : 2;

					uint8_t			ch;

					int Unpack(CBitstream& bs)
					{
						bs_frame_class = (uint8_t)bs.GetBits(2);
						switch (bs_frame_class)
						{
						case FIXFIX:
							break;
						case FIXVAR:
							break;
						case VARFIX:
							break;
						case VARVAR:
							break;
						}

						return RET_CODE_SUCCESS;
					}

				};

				struct SBR_SINGLE_CHANNEL_ELEMENT
				{
					uint8_t			bs_data_extra : 1;
					uint8_t			bs_reserved : 4;

				};

				struct SBR_DATA
				{

				};

				uint16_t		bs_sbr_crc_bits : 10;
				uint16_t		bs_header_flag : 1;
				uint16_t		word_align : 5;

				int				id_aac;
				int				crc_flag;

				SBR_EXTENSION_DATA(int nIdAAC, int nCrcFlag) 
					: bs_sbr_crc_bits(0), bs_header_flag(0), word_align(0), id_aac(nIdAAC), crc_flag(nCrcFlag) {
				}

				int Unpack(CBitstream& bs)
				{
					return RET_CODE_ERROR_NOTIMPL;
				}
			};

			struct SBR_DATA_CRC
			{
				uint16_t		bs_sbr_crc_bits : 10;
				uint16_t		bs_header_flag : 1;
				uint16_t		reserved_0 : 5;


			};

			struct FILL_DATA
			{
				uint8_t			fill_nibble : 4;
				uint8_t			reserved_0 : 4;
				std::vector<uint8_t>
								fill_bytes;

				FILL_DATA(uint16_t cnt): fill_nibble(0), reserved_0(0){
					if (cnt > 1)
						fill_bytes.resize(cnt - 1);
				}

				int Unpack(CBitstream& bs)
				{
					fill_nibble = (uint8_t)bs.GetBits(4);	/* must be '0000' */
					for (size_t i = 0; i < fill_bytes.size(); i++)
						fill_bytes[i] = bs.GetByte();	/* must be '10100101' */
					return RET_CODE_SUCCESS;
				}
			};

			struct DATA_ELEMENT
			{
				uint8_t			data_element_version : 4;
				uint8_t			reserved_0 : 4;

				std::vector<uint8_t>
								dataElementLengthPart;
				std::vector<uint8_t>
								data_element_bytes;

				EXTENSION_PAYLOAD*
								extension_payload = nullptr;

				DATA_ELEMENT(EXTENSION_PAYLOAD* pExtPayload) 
					: data_element_version(0), reserved_0(0), extension_payload(pExtPayload) {
				}

				int Unpack(CBitstream& bs)
				{
					data_element_version = (uint8_t)bs.GetBits(4);
					if (data_element_version == 0)
					{
						int dataElementLength = 0;
						do
						{
							dataElementLengthPart.push_back(bs.GetByte());
							dataElementLength += dataElementLengthPart.back();
						} while (dataElementLengthPart.back() == 255);
						for (int i = 0; i < dataElementLength; i++)
							data_element_bytes.push_back(bs.GetByte());
					}
					else
					{
						if (extension_payload != nullptr)
							extension_payload->align = 0;
					}

					return RET_CODE_SUCCESS;
				}
			};

			uint64_t		start_bitpos;
			uint8_t			extension_type : 4;
			uint8_t			reserved : 4;

			uint8_t			align = 4;
			int16_t			parsed_payload_cnt = 0;

			union
			{
				DYNAMIC_RANGE_INFO*		dynamic_range_info;
				SAC_EXTENSION_DATA*		sac_extension_data;
				SBR_EXTENSION_DATA*		sbr_extension_data;
				FILL_DATA*				fill_data;
				DATA_ELEMENT*			data_element;
			};

			CAMBitArray		other_bits;

			~EXTENSION_PAYLOAD() {
				if (extension_type == EXT_SBR_DATA ||
					extension_type == EXT_SBR_DATA_CRC)
				{
					AMP_SAFEDEL2(sbr_extension_data);
				}
				else
				{
					switch (extension_type)
					{
					case EXT_DYNAMIC_RANGE:
						AMP_SAFEDEL2(dynamic_range_info);
						break;
					case EXT_SAC_DATA:
						AMP_SAFEDEL2(sac_extension_data);
						break;
					case EXT_FILL_DATA:
						AMP_SAFEDEL2(fill_data);
						break;
					case EXT_DATA_ELEMENT:
						AMP_SAFEDEL2(data_element);
					}
				}
			}

			int Unpack(CBitstream& bs, int cnt)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t start_bitpos = bs.Tell();
				extension_type = (uint8_t)bs.GetBits(4);

				//
				// Fill elements containing an extension_payload() with an extension_type of EXT_SBR_DATA or
				// EXT_SBR_DATA_CRC shall not contain any other extension_payload of any other extension_type.
				if (extension_type == EXT_SBR_DATA ||
					extension_type == EXT_SBR_DATA_CRC)
				{
					sbr_extension_data = new SBR_EXTENSION_DATA(-1, extension_type == EXT_SBR_DATA ? 0 : 1);
					iRet = sbr_extension_data->Unpack(bs);
					if (iRet >= 0)
					{
						// Skip the left bits
						uint64_t cur_bitpos = bs.Tell();
						bs.SkipBits(((int64_t)cnt << 3) - (cur_bitpos - start_bitpos));
					}
				}
				else
				{
					switch (extension_type)
					{
					case EXT_DYNAMIC_RANGE:
						dynamic_range_info = new DYNAMIC_RANGE_INFO();
						iRet = dynamic_range_info->Unpack(bs);
						break;
					case EXT_SAC_DATA:
						sac_extension_data = new SAC_EXTENSION_DATA(cnt);
						iRet = sac_extension_data->Unpack(bs);
						break;
					case EXT_FILL_DATA:
						fill_data = new FILL_DATA(cnt);
						iRet = fill_data->Unpack(bs);
						break;
					case EXT_DATA_ELEMENT:
						data_element = new DATA_ELEMENT(this);
						iRet = data_element->Unpack(bs);
						if(iRet < 0 || data_element->data_element_version == 0)
							break;
					case EXT_FIL:
					default:
						for (int i = 0; i < ((cnt - 1) << 3) + align; i++)
							bs.GetBits(1) ? other_bits.BitSet(i) : other_bits.BitClear(i);
						break;
					}
				}

				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(extension_type, 4, "");
				if (extension_type == EXT_SBR_DATA ||
					extension_type == EXT_SBR_DATA_CRC)
				{

				}
				else
				{
					switch (extension_type)
					{
					case EXT_DYNAMIC_RANGE:
						break;
					case EXT_FIL:
						NAV_WRITE_TAG_BEGIN("fill_bits");
						for (i = 0; i <= other_bits.UpperBound(); i++) {
							NAV_FIELD_PROP_NUMBER_ALIAS_F("bit", "[%03d]", 1, "%d", other_bits[i], "", i);
						}
						NAV_WRITE_TAG_END("fill_bits");
						break;
					case EXT_FILL_DATA:
						NAV_FIELD_PROP_2NUMBER("fill_nibble", 4, fill_data->fill_nibble, "must be 0000b");
						for (i = 0; i < (int)fill_data->fill_bytes.size(); i++) {
							NAV_FIELD_PROP_2NUMBER_ALIAS_F("byte", "fill_byte[%2d]", fill_data->fill_bytes[i], "0X%02X(%d)", fill_data->fill_bytes[i], "must be 10100101b", i);
						}
						break;
					}
				}
			DECLARE_FIELDPROP_END()
		};

		struct ICS_INFO
		{
			struct LTP_DATA
			{
				uint16_t		ltp_lag_update : 1;
				uint16_t		ltp_lag : 11;
				uint16_t		ltp_coef : 3;
				uint16_t		reserved : 1;
				CAMBitArray		ltp_long_used;
				ICS_INFO*		ics_info = nullptr;

				LTP_DATA(ICS_INFO* pICSInfo) 
					: ltp_lag_update(0), ltp_lag(0), ltp_coef(0), reserved(0), ics_info(pICSInfo) {
				}

				int Unpack(CBitstream& bs)
				{
					if (ER_AAC_LD == ics_info->ctx_audio_stream->audio_specific_config->audioObjectType_1)
					{
						ltp_lag_update = (uint16_t)bs.GetBits(1);
						if (ltp_lag_update)
						{
							ltp_lag = (uint16_t)bs.GetBits(10);
							ics_info->ctx_audio_stream->ltp_prev_lag = ltp_lag;
						}
						else
							ltp_lag = ics_info->ctx_audio_stream->ltp_prev_lag;

						ltp_coef = (uint16_t)bs.GetBits(3);
						for (int sfb = 0; sfb < AMP_MIN(ics_info->max_sfb, MAX_LTP_LONG_SFB); sfb++)
							bs.GetBits(1) ? ltp_long_used.BitSet(sfb) : ltp_long_used.BitClear(sfb);
					}
					else
					{
						ltp_lag = (uint16_t)bs.GetBits(11);
						ltp_coef = (uint16_t)bs.GetBits(3);
						if (ics_info->window_sequence != EIGHT_SHORT_SEQUENCE) {
							for (int sfb = 0; sfb < AMP_MIN(ics_info->max_sfb, MAX_LTP_LONG_SFB); sfb++)
								bs.GetBits(1) ? ltp_long_used.BitSet(sfb) : ltp_long_used.BitClear(sfb);
						}
					}

					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
				if (ER_AAC_LD == ics_info->ctx_audio_stream->audio_specific_config->audioObjectType_1)
				{
					NAV_FIELD_PROP_NUMBER1(ltp_lag_update, 1, "");
					if (ltp_lag_update)
					{
						NAV_FIELD_PROP_2NUMBER1(ltp_lag, 10, "");
					}
					else
					{
						NAV_WRITE_TAG_WITH_ALIAS("ltp_lag", "ltp_lag = ltp_prev_lag", "");
					}
					NAV_FIELD_PROP_2NUMBER1(ltp_coef, 3, "");

					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(sfb=0;sfb&lt;min(max_sfb,MAX_LTP_LONG_SFB);sfb++)", "");
					for (int sfb = 0; sfb < AMP_MIN(ics_info->max_sfb, MAX_LTP_LONG_SFB); sfb++)
					{
						NAV_ARRAY_FIELD_PROP_NUMBER_("ltp_long_used", "sfb", sfb, 1, ltp_long_used[sfb], ltp_long_used[sfb] ? "used" : "NOT used");
					}
					NAV_WRITE_TAG_END("Tag0");
				}
				else
				{
					NAV_FIELD_PROP_2NUMBER1(ltp_lag, 11, "");
					NAV_FIELD_PROP_2NUMBER1(ltp_coef, 3, "");
					if (ics_info->window_sequence != EIGHT_SHORT_SEQUENCE) {
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(sfb=0;sfb&lt;min(max_sfb,MAX_LTP_LONG_SFB);sfb++)", "");
						for (int sfb = 0; sfb < AMP_MIN(ics_info->max_sfb, MAX_LTP_LONG_SFB); sfb++)
						{
							NAV_ARRAY_FIELD_PROP_NUMBER_("ltp_long_used", "sfb", sfb, 1, ltp_long_used[sfb], ltp_long_used[sfb] ? "used" : "NOT used");
						}
						NAV_WRITE_TAG_END("Tag0");
					}
				}
				DECLARE_FIELDPROP_END()
			};

			uint8_t			ics_reserved_bit : 1;
			uint8_t			window_sequence : 2;
			uint8_t			window_shape : 1;
			uint8_t			reserved_0 : 4;

			uint8_t			max_sfb;
			uint8_t			scale_factor_grouping;

			uint8_t			predictor_data_present : 1;
			uint8_t			predictor_reset : 1;
			uint8_t			predictor_reset_group_number : 5;
			uint8_t			reserved_1 : 1;

			uint8_t			ltp_data_present_0 : 1;
			uint8_t			ltp_data_present_1 : 1;
			uint8_t			reserved_2 : 6;

			uint8_t			num_window_groups = 1;
			uint8_t			num_windows = 1;
			uint8_t			group_len[7] = { 1, 0, 0, 0, 0, 0, 0 };
			uint8_t			num_swb = 0;

			uint16_t		sect_sfb_offset[8][15 * 8];
			uint16_t		swb_offset[52];

			LTP_DATA*		ltp_data_0 = nullptr;
			LTP_DATA*		ltp_data_1 = nullptr;

			CAMBitArray		prediction_used;

			CAudioStreamContext*
							ctx_audio_stream;

			ICS_INFO(CAudioStreamContext* pCtxAudioStream) : ctx_audio_stream(pCtxAudioStream) {
			}

			~ICS_INFO()
			{
				AMP_SAFEDEL2(ltp_data_0);
				AMP_SAFEDEL2(ltp_data_1);
			}

			int Unpack(CBitstream& bs)
			{
				ics_reserved_bit = (uint8_t)bs.GetBits(1);
				window_sequence = (uint8_t)bs.GetBits(2);
				window_shape = (uint8_t)bs.GetBits(1);

				uint8_t idx_sf = ctx_audio_stream->GetSamplingFrequencyIndex();

				if (window_sequence == EIGHT_SHORT_SEQUENCE)
				{
					max_sfb = (uint8_t)bs.GetBits(4);
					num_windows = 8;
					scale_factor_grouping = (uint8_t)bs.GetBits(7);
					for (int i = 0; i < 7; i++)
					{
						if (scale_factor_grouping&(1 << (6 - i)))
							group_len[num_window_groups - 1]++;
						else
						{
							num_window_groups++;
							group_len[num_window_groups - 1] = 1;
						}
					}
				}
				else
				{
					max_sfb = (uint8_t)bs.GetBits(6);
					predictor_data_present = (uint8_t)bs.GetBits(1);
					if (predictor_data_present)
					{
						// If there is no audio_specific_config in audio context, deem it as MPEG2 AAC stream syntax
						if (ctx_audio_stream->audio_specific_config == NULL ||
							ctx_audio_stream->audio_specific_config->audioObjectType_1 == 1)
						{
							predictor_reset = (uint8_t)bs.GetBits(1);
							if (predictor_reset)
								predictor_reset_group_number = (uint8_t)bs.GetBits(5);

							for (int sfb = 0; sfb < AMP_MIN(max_sfb, PRED_SFB_MAX[idx_sf]); sfb++)
							{
								bs.GetBits(1) ? prediction_used.BitSet(sfb) : prediction_used.BitClear(sfb);
							}
						}
						else
						{
							ltp_data_present_0 = (uint8_t)bs.GetBits(1);
							if (ltp_data_present_0)
							{
								ltp_data_0 = new LTP_DATA(this);
								ltp_data_0->Unpack(bs);
							}

							if (ctx_audio_stream->common_window)
							{
								ltp_data_present_1 = (uint8_t)bs.GetBits(1);
								if (ltp_data_present_1)
								{
									ltp_data_1 = new LTP_DATA(this);
									ltp_data_1->Unpack(bs);
								}
							}
						}
					}
				}

				uint16_t window_length = ctx_audio_stream->GetWindowLength();
				int idx_window_length = WINLEN_IDX(window_length);

				if (idx_sf >= 12)
				{
					printf("[AAC] No available sampling frequency index which ranges from 0 to 11 inclusively.");
					return RET_CODE_BUFFER_NOT_COMPATIBLE;
				}

				num_swb = (uint8_t)std::get<1>(swb_offset_window[idx_window_length][idx_sf][1]);
				if (num_swb > 0)
					num_swb--;

				// number of scalefactor bands transmitted per group should be not greater than number of scalefactor window bands
				if (max_sfb > num_swb)
				{
					printf("[AAC] max_sfb(%d) is expected to be not greater than num_swb(%d).\n", max_sfb, num_swb);
					return RET_CODE_BUFFER_NOT_COMPATIBLE;
				}

				auto swb_offsets = std::get<0>(swb_offset_window[idx_window_length][idx_sf][window_sequence]);
				for (uint8_t i = 0; i < num_swb; i++)
					swb_offset[i] = swb_offsets[i];
				swb_offset[num_swb] = window_length >> (window_sequence == EIGHT_SHORT_SEQUENCE ? 4 : 1);

				if (window_sequence == EIGHT_SHORT_SEQUENCE)
				{
					/* preparation of sect_sfb_offset for short blocks */
					for (uint8_t g = 0; g < num_window_groups; g++)
					{
						uint16_t width;
						uint8_t sect_sfb = 0;
						uint16_t offset = 0;

						for (uint8_t i = 0; i < num_swb; i++)
						{
							if (i + 1 == num_swb)
								width = (window_length >> 4) - swb_offsets[i];
							else
								width = swb_offsets[i + 1] - swb_offsets[i];
							
							width *= group_len[g];
							sect_sfb_offset[g][sect_sfb++] = offset;
							offset += width;
						}
						sect_sfb_offset[g][sect_sfb] = offset;
					}
				}
				else
				{
					for (uint8_t i = 0; i < num_swb; i++)
						sect_sfb_offset[0][i] = swb_offset[i];
					
					sect_sfb_offset[0][num_swb] = swb_offset[num_swb];
				}

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_NUMBER1(ics_reserved_bit, 1, "flag reserved for future use. Shall be '0'");
			NAV_FIELD_PROP_2NUMBER1(window_sequence, 2, WINDOW_SEQUENCE_NAMEA(window_sequence));
			NAV_FIELD_PROP_NUMBER1(window_shape, 1, "determines what window is used for the right hand part of this analysis window");
			if (window_sequence == EIGHT_SHORT_SEQUENCE)
			{
				NAV_FIELD_PROP_2NUMBER1(max_sfb, 4, "number of scalefactor bands transmitted per group either in ics_info(), aac_scalable_main_element(), or aac_scalable_extension_element().");
				NAV_FIELD_PROP_2NUMBER1(scale_factor_grouping, 7, "a bit field that contains information about grouping of short spectral data");
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(num_window_groups, "number of groups of windows which share one set of scalefactors");
			}
			else
			{
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(num_window_groups, "number of groups of windows which share one set of scalefactors");
				NAV_FIELD_PROP_2NUMBER1(max_sfb, 6, "number of scalefactor bands transmitted per group either in ics_info(), aac_scalable_main_element(), or aac_scalable_extension_element().");
				MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)predictor_data_present);
				NAV_FIELD_PROP_BEGIN("predictor_data_present", 1, szTemp3,
					predictor_data_present ? "predictor data present" : "predictor data NOT present", bit_offset ? *bit_offset : -1LL, "I");

				if (predictor_data_present)
				{
					if (ctx_audio_stream->audio_specific_config->audioObjectType_1 == 1)
					{
						MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)predictor_reset);
						NAV_FIELD_PROP_BEGIN("predictor_reset", 1, szTemp3, predictor_reset ? "predictor reset is applied in current frame" : "predictor reset is NOT applied in current frame", bit_offset ? *bit_offset : -1LL, "I");
						NAV_FIELD_PROP_2NUMBER1(predictor_reset_group_number, 5, "specifying the reset group to be reset in current frame");
						NAV_FIELD_PROP_END("predictor_reset");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(sfb=0;sfb&lt;min(max_sfb,PRED_SFB_MAX);sfb++)", "");
						for (int sfb = 0; sfb < AMP_MIN(max_sfb, PRED_SFB_MAX[ctx_audio_stream->audio_specific_config->samplingFrequencyIndex]); sfb++)
						{
							NAV_ARRAY_FIELD_PROP_NUMBER_("prediction_used", "sfb", sfb, 1, prediction_used[sfb], prediction_used[sfb] ? "used" : "NOT used");
						}
						NAV_WRITE_TAG_END("Tag0");
					}
					else
					{
						MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)ltp_data_present_0);
						NAV_FIELD_PROP_BEGIN("ltp_data_present", 1, szTemp3,
							ltp_data_present_0 ? "ltp_data() present" : "ltp_data() NOT present", bit_offset ? *bit_offset : -1LL, "I");
						if (ltp_data_present_0) {
							NAV_FIELD_PROP_REF2_1(ltp_data_0, "ltp_data", "");
						}
						NAV_FIELD_PROP_END("ltp_data_present");

						NAV_WRITE_TAG_WITH_1NUMBER_VALUE_BEGIN("common_window", "", ctx_audio_stream->common_window, "");
						if (ctx_audio_stream->common_window)
						{
							MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)ltp_data_present_1);
							NAV_FIELD_PROP_BEGIN("ltp_data_present", 1, szTemp3,
								ltp_data_present_1 ? "ltp_data() present" : "ltp_data() NOT present", bit_offset ? *bit_offset : -1LL, "I");
							if (ltp_data_present_1) {
								NAV_FIELD_PROP_REF2_1(ltp_data_1, "ltp_data", "");
							}
							NAV_FIELD_PROP_END("ltp_data_present");
						}
						NAV_WRITE_TAG_END("common_window");
					}
				}

				NAV_FIELD_PROP_END("predictor_data_present");
			}
			DECLARE_FIELDPROP_END()

		};

		struct INDIVIDUAL_CHANNEL_STREAM
		{
			struct SECTION_DATA
			{
				std::vector<uint8_t>
									sect_cb[7];
				std::vector<uint32_t>
									sect_start[7];
				std::vector<uint32_t>
									sect_end[7];
				std::vector<uint8_t>
									sfb_cb[7];
				uint8_t				sect_esc_val;
				uint8_t				num_sec[7] = { 0 };
				INDIVIDUAL_CHANNEL_STREAM*
									individual_channel_stream = nullptr;

				SECTION_DATA(INDIVIDUAL_CHANNEL_STREAM* pIndividualChannelStream)
					: individual_channel_stream(pIndividualChannelStream) {
				}

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					if (individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE)
					{
						sect_esc_val = (1 << 3) - 1;
					}
					else
					{
						sect_esc_val = (1 << 5) - 1;
					}

					for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
					{
						uint32_t k = 0, i = 0;
						while (k < individual_channel_stream->ics_info->max_sfb)
						{
							bool aacSectionDataResilienceFlag = false;
							auto audio_specific_config = individual_channel_stream->ctx_audio_stream->audio_specific_config;
							if (audio_specific_config != nullptr &&
								audio_specific_config->IsGASpecificConfigAvail() &&
								audio_specific_config->GASpecConfig != nullptr &&
								audio_specific_config->GASpecConfig->aacSectionDataResilienceFlag)
							{
								sect_cb[g].push_back((uint8_t)bs.GetBits(5));
								aacSectionDataResilienceFlag = true;
							}
							else
							{
								sect_cb[g].push_back((uint8_t)bs.GetBits(4));
							}

							uint32_t sect_len = 0;
							uint8_t sect_len_incr = 0;
							if (!aacSectionDataResilienceFlag ||
								sect_cb[g].back() < 11 || (sect_cb[g].back() > 11 && sect_cb[g].back() < 16)) {
								while ((sect_len_incr = (uint8_t)bs.GetBits(individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE ? 3 : 5)) == sect_esc_val)
									sect_len += sect_esc_val;
							}
							else
								sect_len_incr = 1;

							sect_len += sect_len_incr;
							sect_start[g].push_back(k);
							sect_end[g].push_back(k + sect_len);
							for (uint32_t sfb = k; sfb < k + sect_len; sfb++)
							{
								sfb_cb[g].push_back(sect_cb[g][i]);
							}

							k += sect_len;
							i++;
						}

						num_sec[g] = i;
					}

					return iRet;
				}

				DECLARE_FIELDPROP_BEGIN()
				NAV_WRITE_TAG_WITH_NUMBER_VALUE1(sect_esc_val, "");
				if (individual_channel_stream->ics_info->num_window_groups > 0)
				{
					char szAlias[256];
					MBCSPRINTF_S(szAlias, _countof(szAlias), "for(g=0;g&lt;num_window_groups(%d);g++)", individual_channel_stream->ics_info->num_window_groups);
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", szAlias, "the section data for each window group");
					for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
					{
						int k = 0, i = 0;
						while (k < individual_channel_stream->ics_info->max_sfb)
						{
							bool aacSectionDataResilienceFlag = false;
							auto audio_specific_config = individual_channel_stream->ctx_audio_stream->audio_specific_config;
							if (audio_specific_config != nullptr &&
								audio_specific_config->IsGASpecificConfigAvail() &&
								audio_specific_config->GASpecConfig != nullptr &&
								audio_specific_config->GASpecConfig->aacSectionDataResilienceFlag)
							{
								NAV_2ARRAY_FIELD_PROP_2NUMBER_("sect_cb", "group", g, "", i, 5, sect_cb[g][i], "spectrum Huffman codebook used for section i in group g");
								aacSectionDataResilienceFlag = true;
							}
							else
							{
								NAV_2ARRAY_FIELD_PROP_2NUMBER_("sect_cb", "group", g, "", i, 4, sect_cb[g][i], "spectrum Huffman codebook used for section i in group g");
							}

							size_t sect_len = sect_end[g][i] - sect_start[g][i];
							uint8_t sect_len_incr = 0;
							if (!aacSectionDataResilienceFlag ||
								sect_cb[g].back() < 11 || (sect_cb[g].back() > 11 && sect_cb[g].back() < 16)) {
								uint8_t sect_len_incr_bitlen = individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE ? 3 : 5;
								while (sect_len > sect_esc_val)
								{
									sect_len -= sect_esc_val;
									NAV_FIELD_PROP_2NUMBER("sect_len_incr", sect_len_incr_bitlen, sect_esc_val, "");
								}

								if (sect_len > 0) {
									NAV_FIELD_PROP_2NUMBER("sect_len_incr", sect_len_incr_bitlen, sect_len, "");
								}
							}

							k += sect_end[g][i] - sect_start[g][i];
							i++;
						}

						if (sect_start[g].size() > 0) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(sfb=0;sfb&lt;sect_end[g];sfb++)", "scalefactor band codebooks");
							for (uint32_t sfb = sect_start[g][0]; sfb < sect_end[g].back(); sfb++)
							{
								NAV_WRITE_TAG_WITH_ALIAS_VALUEFMTSTR_AND_NUMBER_VALUE("sfb_cb", "sfb_cb[group#%d][sfb#%d]", "%d(0X%X)", sfb_cb[g][sfb],
									"the code-book of scalefactor band index within group", g, sfb);
							}
							NAV_WRITE_TAG_END("Tag00");
						}
					}
					NAV_WRITE_TAG_END("Tag0");
				}
				DECLARE_FIELDPROP_END()
			};

			struct SCALE_FACTOR_DATA
			{
				int16_t				dpcm_is_position[8][64] = { {0} };
				int16_t				dpcm_noise_nrg[8][64] = { {0} };
				int16_t				dpcm_sf[8][64] = { {0} };
				int16_t				dpcm_is_last_position = 0;

				int16_t				esc_dpcm_is_position[8][64] = { {0} };
				int16_t				esc_dpcm_noise_nrg[8][64] = { {0} };
				int16_t				esc_dpcm_sf[8][64] = { {0} };
				int16_t				esc_dpcm_is_last_position = 0;

				uint8_t				sf_concealment = 0;
				uint8_t				rev_global_gain = 0;
				uint16_t			length_of_rvlc_sf =0;

				uint8_t				sf_escapes_present = 0;
				uint8_t				length_of_rvlc_escapes = 0;
				int16_t				dpcm_noise_last_position = 0;

				INDIVIDUAL_CHANNEL_STREAM*
									individual_channel_stream = nullptr;

				SCALE_FACTOR_DATA(INDIVIDUAL_CHANNEL_STREAM* pIndividualChannelStream)
					: individual_channel_stream(pIndividualChannelStream) {
				}

				int Unpack(CBitstream& bs)
				{
					bool aacScalefactorDataResilienceFlag = false;
					auto audio_specific_config = individual_channel_stream->ctx_audio_stream->audio_specific_config;
					if (audio_specific_config != nullptr && 
						audio_specific_config->IsGASpecificConfigAvail() && 
						audio_specific_config->GASpecConfig->aacScalefactorDataResilienceFlag)
						aacScalefactorDataResilienceFlag = true;

					bool noise_pcm_flag = false;
					if (!aacScalefactorDataResilienceFlag)
					{
						noise_pcm_flag = true;
						for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
						{
							for (uint8_t sfb = 0; sfb < individual_channel_stream->ics_info->max_sfb; sfb++)
							{
								if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
								{
									if (individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB ||
										individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB2)
									{
										dpcm_is_position[g][sfb] = (int16_t)hcb_read_value(bs, scalefactor_hcb_bst, _countof(scalefactor_hcb_bst));
									}
									else
									{
										if (individual_channel_stream->section_data->sfb_cb[g][sfb] == NOISE_HCB)
										{
											if (noise_pcm_flag)
											{
												noise_pcm_flag = false;
												dpcm_noise_nrg[g][sfb] = (int16_t)bs.GetBits(9);
											}
											else
											{
												dpcm_noise_nrg[g][sfb] = (int16_t)hcb_read_value(bs, scalefactor_hcb_bst, _countof(scalefactor_hcb_bst));
											}
										}
										else
										{
											dpcm_sf[g][sfb] = (int16_t)hcb_read_value(bs, scalefactor_hcb_bst, _countof(scalefactor_hcb_bst));
										}
									}
								}
							}
						}
					}
					else
					{
						bool intensity_used = false;
						bool noise_used = false;

						sf_concealment = (uint8_t)bs.GetBits(1);
						rev_global_gain = bs.GetByte();

						if (individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE)
							length_of_rvlc_sf = (uint16_t)bs.GetBits(11);
						else
							length_of_rvlc_sf = (uint16_t)bs.GetBits(9);

						for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
						{
							for (uint8_t sfb = 0; sfb < individual_channel_stream->ics_info->max_sfb; sfb++)
							{
								if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
								{
									if (individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB ||
										individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB2)
									{
										intensity_used = true;
										dpcm_is_position[g][sfb] = (int16_t)hcb_read_value(bs, rvlc_hcb_bst, _countof(rvlc_hcb_bst));
									}
									else
									{
										if (individual_channel_stream->section_data->sfb_cb[g][sfb] == NOISE_HCB)
										{
											if (!noise_used)
											{
												noise_used = true;
												dpcm_noise_nrg[g][sfb] = (int16_t)bs.GetBits(9);
											}
											else
											{
												dpcm_noise_nrg[g][sfb] = (int16_t)hcb_read_value(bs, rvlc_hcb_bst, _countof(rvlc_hcb_bst));
											}
										}
										else
										{
											dpcm_sf[g][sfb] = (int16_t)hcb_read_value(bs, rvlc_hcb_bst, _countof(rvlc_hcb_bst));
										}
									}
								}	// if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
							}
						}

						if (intensity_used)
						{
							dpcm_is_last_position = (int16_t)hcb_read_value(bs, rvlc_hcb_bst, _countof(rvlc_hcb_bst));
						}

						noise_used = false;
						sf_escapes_present = (uint8_t)bs.GetBits(1);
						if (sf_escapes_present)
						{
							length_of_rvlc_escapes = bs.GetByte();

							/*
							4.6.2.3.2 Decoding of scalefactors
							In the case of sf_escapes_present==1, a decoded value of is used as ESC_FLAG. It signals that an escape
							value exists, that has to be added to +7 or subtracted from -7 in order to find the actual scalefactor value. This
							escape value is Huffman encoded.
							*/
							for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
							{
								for (uint8_t sfb = 0; sfb < individual_channel_stream->ics_info->max_sfb; sfb++)
								{
									if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
									{
										if ((individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB ||
											 individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB2) && (
												dpcm_is_position[g][sfb] == 7 || dpcm_is_position[g][sfb] == -7))
										{
											esc_dpcm_is_position[g][sfb] = (int16_t)hcb_read_value(bs, rvlc_esc_hcb_bst, _countof(rvlc_esc_hcb_bst));
										}
										else
										{
											if (individual_channel_stream->section_data->sfb_cb[g][sfb] == NOISE_HCB)
											{
												if (!noise_used)
													noise_used = true;
												else
												{
													if (dpcm_noise_nrg[g][sfb] == 7 || dpcm_noise_nrg[g][sfb] == -7)
													{
														esc_dpcm_noise_nrg[g][sfb] = (int16_t)hcb_read_value(bs, rvlc_esc_hcb_bst, _countof(rvlc_esc_hcb_bst));
													}
												}
											}
											else
											{
												if (dpcm_sf[g][sfb] == 7 || dpcm_sf[g][sfb] == -7)
												{
													esc_dpcm_sf[g][sfb] = (int16_t)hcb_read_value(bs, rvlc_esc_hcb_bst, _countof(rvlc_esc_hcb_bst));
												}
											}
										}

									} // if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
								}
							}

							if (intensity_used && (dpcm_is_last_position == 7 || dpcm_is_last_position == -7))
							{
								esc_dpcm_is_last_position = (int16_t)hcb_read_value(bs, rvlc_esc_hcb_bst, _countof(rvlc_esc_hcb_bst));
							}
						}

						if (noise_used){
							dpcm_noise_last_position = (int16_t)bs.GetBits(9);
						}
					}

					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
				bool aacScalefactorDataResilienceFlag = false;
				auto audio_specific_config = individual_channel_stream->ctx_audio_stream->audio_specific_config;
				if (audio_specific_config != nullptr &&
					audio_specific_config->IsGASpecificConfigAvail() &&
					audio_specific_config->GASpecConfig->aacScalefactorDataResilienceFlag)
					aacScalefactorDataResilienceFlag = true;

				if (!aacScalefactorDataResilienceFlag)
				{
					bool noise_pcm_flag = false;
					NAV_WRITE_TAG_WITH_NUMBER_VALUE1(noise_pcm_flag, "");
					if (individual_channel_stream->ics_info->num_window_groups > 0)
					{
						char szAlias[256];
						MBCSPRINTF_S(szAlias, _countof(szAlias), "for(g=0;g&lt;num_window_groups(%d);g++)", individual_channel_stream->ics_info->num_window_groups);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", szAlias, "");
						for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
						{
							MBCSPRINTF_S(szAlias, _countof(szAlias), "for(sfb=0;sfb&lt;max_sfb(%d);sfb++)", individual_channel_stream->ics_info->max_sfb);
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", szAlias, "");
							for (uint8_t sfb = 0; sfb < individual_channel_stream->ics_info->max_sfb; sfb++)
							{
								if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
								{
									if (individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB ||
										individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB2)
									{
										NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag000", "dpcm_is_position[g#%d][sfb#%d]",
											dpcm_is_position[g][sfb], "Differentially encoded intensity stereo position", g, sfb);
										NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag001", "hcod_sf[dpcm_is_position[g#%d][sfb#%d]]",
											std::get<1>(scalefactor_hcb[dpcm_is_position[g][sfb]]), 
											"0X%" PRIX64 "", std::get<2>(scalefactor_hcb[dpcm_is_position[g][sfb]]),
											"The Huffman codeword of differentially encoded intensity stereo position", g, sfb);
									}
									else
									{
										if (individual_channel_stream->section_data->sfb_cb[g][sfb] == NOISE_HCB)
										{
											if (noise_pcm_flag)
											{
												noise_pcm_flag = false;
												NAV_2ARRAY_FIELD_PROP_2NUMBER_("Tag002", "g", g, "sfb", sfb, 9, dpcm_noise_nrg[g][sfb], "Differentially encoded noise energy");
											}
											else
											{
												NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag002", "dpcm_noise_nrg[g#%d][sfb#%d]",
													dpcm_noise_nrg[g][sfb], "Differentially encoded noise energy", g, sfb);
												NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag003", "hcod_sf[dpcm_noise_nrg[g#%d][sfb#%d]]",
													std::get<1>(scalefactor_hcb[dpcm_noise_nrg[g][sfb]]),
													"0X%" PRIX64 "", std::get<2>(scalefactor_hcb[dpcm_noise_nrg[g][sfb]]),
													"The Huffman codeword of differentially encoded noise energy", g, sfb);
											}
										}
										else
										{
											NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag002", "dpcm_sf[g#%d][sfb#%d]",
												dpcm_sf[g][sfb], "Differential coded scalefactor", g, sfb);
											NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag003", "hcod_sf[dpcm_sf[g#%d][sfb#%d]]",
												std::get<1>(scalefactor_hcb[dpcm_sf[g][sfb]]),
												"0X%" PRIX64 "", std::get<2>(scalefactor_hcb[dpcm_sf[g][sfb]]),
												"The Huffman codeword of Differential coded scalefactor", g, sfb);
										}
									}
								}
							}
							NAV_WRITE_TAG_END("Tag00");
						}
						NAV_WRITE_TAG_END("Tag0");
					}
				}
				else
				{
					bool intensity_used = false;
					bool noise_used = false;

					NAV_FIELD_PROP_NUMBER1(sf_concealment, 1, sf_concealment ? "the scale factors of the last frame are dissimilar to those of the current frame" 
																			 : "the scale factors of the last frame are similar to those of the current frame");
					NAV_FIELD_PROP_2NUMBER1(rev_global_gain, 8, "contains the last scalefactor value as a start value for the backward decoding");

					NAV_FIELD_PROP_2NUMBER1(length_of_rvlc_sf, individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE?11:9, 
						"A data field that contains the length of the current RVLC data part in bits, including the DPCM start value for PNS");

					char szAlias[256];
					MBCSPRINTF_S(szAlias, _countof(szAlias), "for(g=0;g&lt;num_window_groups(%d);g++)", individual_channel_stream->ics_info->num_window_groups);
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", szAlias, "");
					for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
					{
						MBCSPRINTF_S(szAlias, _countof(szAlias), "for(sfb=0;sfb&lt;max_sfb(%d);sfb++)", individual_channel_stream->ics_info->max_sfb);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", szAlias, "");
						for (uint8_t sfb = 0; sfb < individual_channel_stream->ics_info->max_sfb; sfb++)
						{
							if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
							{
								if (individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB ||
									individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB2)
								{
									intensity_used = true;

									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag000", "dpcm_is_position[g#%d][sfb#%d]",
										dpcm_is_position[g][sfb], "Differentially encoded intensity stereo position", g, sfb);
									auto iter = std::find_if(rvlc_hcb.cbegin(), rvlc_hcb.cend(), [&](const auto& item) {return dpcm_is_position[g][sfb] == std::get<0>(item); });
									if (iter != rvlc_hcb.cend())
									{
										NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag001", "rvlc_cod_sf[dpcm_is_position[g#%d][sfb#%d]]",
											std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
											"The Huffman codeword of differentially encoded intensity stereo position", g, sfb);
									}
								}
								else
								{
									if (individual_channel_stream->section_data->sfb_cb[g][sfb] == NOISE_HCB)
									{
										if (!noise_used)
										{
											noise_used = true;
											NAV_2ARRAY_FIELD_PROP_2NUMBER_("Tag000", "g", g, "sfb", sfb, 9, dpcm_noise_nrg[g][sfb], "Differentially encoded noise energy");
										}
										else
										{
											NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag000", "dpcm_noise_nrg[g#%d][sfb#%d]",
												dpcm_noise_nrg[g][sfb], "Differentially encoded noise energy", g, sfb);
											auto iter = std::find_if(rvlc_hcb.cbegin(), rvlc_hcb.cend(), [&](const auto& item) {return dpcm_noise_nrg[g][sfb] == std::get<0>(item);});
											if (iter != rvlc_hcb.cend())
											{
												NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag001", "rvlc_cod_sf[dpcm_noise_nrg[g#%d][sfb#%d]]",
													std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
													"The Huffman codeword of differentially encoded noise energy", g, sfb);
											}
										}
									}
									else
									{
										NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag000", "dpcm_sf[g#%d][sfb#%d]",
											dpcm_sf[g][sfb], "Differential coded scalefactor", g, sfb);
										auto iter = std::find_if(rvlc_hcb.cbegin(), rvlc_hcb.cend(), [&](const auto& item) {return std::get<0>(item) == dpcm_sf[g][sfb]; });
										if (iter != rvlc_hcb.cend())
										{
											NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag001", "rvlc_cod_sf[dpcm_sf[g#%d][sfb#%d]]",
												std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
												"The Huffman codeword of Differential coded scalefactor", g, sfb);
										}
									}
								}
							}	// if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
						}
					}
					NAV_WRITE_TAG_END("Tag0");

					if (intensity_used)
					{
						NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag1", "dpcm_is_last_position",
							dpcm_is_last_position, "DPCM value allowing backward decoding of intensity stereo data part");
						auto iter = std::find_if(rvlc_hcb.cbegin(), rvlc_hcb.cend(), [&](const auto& item) {return std::get<0>(item) == dpcm_is_last_position; });
						if (iter != rvlc_hcb.cend())
						{
							NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag2", "rvlc_cod_sf[dpcm_is_last_position]",
								std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
								"The Huffman codeword of DPCM value allowing backward decoding of intensity stereo data part");
						}
					}

					noise_used = false;
					NAV_FIELD_PROP_NUMBER_BEGIN("sf_escapes_present", 1, sf_escapes_present, 
						sf_escapes_present ? "there are escapes coded in the bitstream payload" : "No escapes coded in the bitstream payload");
					if (sf_escapes_present)
					{
						NAV_FIELD_PROP_2NUMBER1(length_of_rvlc_escapes, 8, "the length of the current RVLC escape data part in bits");

						MBCSPRINTF_S(szAlias, _countof(szAlias), "for(g=0;g&lt;num_window_groups(%d);g++)", individual_channel_stream->ics_info->num_window_groups);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag3", szAlias, "");
						for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
						{
							MBCSPRINTF_S(szAlias, _countof(szAlias), "for(sfb=0;sfb&lt;max_sfb(%d);sfb++)", individual_channel_stream->ics_info->max_sfb);
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag30", szAlias, "");
							for (uint8_t sfb = 0; sfb < individual_channel_stream->ics_info->max_sfb; sfb++)
							{
								if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
								{
									if ((individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB ||
										individual_channel_stream->section_data->sfb_cb[g][sfb] == INTENSITY_HCB2) && (
											dpcm_is_position[g][sfb] == 7 || dpcm_is_position[g][sfb] == -7))
									{
										NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag300", "dpcm_is_position[g#%d][sfb#%d]",
											esc_dpcm_is_position[g][sfb], "Differentially encoded intensity stereo position", g, sfb);
										auto iter = std::find_if(rvlc_esc_hcb.cbegin(), rvlc_esc_hcb.cend(), [&](const auto& item) {return esc_dpcm_is_position[g][sfb] == std::get<0>(item); });
										if (iter != rvlc_esc_hcb.cend())
										{
											NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag301", "rvlc_esc_sf[dpcm_is_position[g#%d][sfb#%d]]",
												std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
												"The Huffman codeword of differentially encoded intensity stereo position", g, sfb);
										}
									}
									else
									{
										if (individual_channel_stream->section_data->sfb_cb[g][sfb] == NOISE_HCB)
										{
											if (!noise_used)
												noise_used = true;
											else
											{
												if (dpcm_noise_nrg[g][sfb] == 7 || dpcm_noise_nrg[g][sfb] == -7)
												{
													NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag300", "dpcm_noise_nrg[g#%d][sfb#%d]",
														esc_dpcm_noise_nrg[g][sfb], "Differentially encoded noise energy", g, sfb);
													auto iter = std::find_if(rvlc_esc_hcb.cbegin(), rvlc_esc_hcb.cend(), [&](const auto& item) {return esc_dpcm_noise_nrg[g][sfb] == std::get<0>(item); });
													if (iter != rvlc_esc_hcb.cend())
													{
														NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag301", "rvlc_esc_sf[dpcm_noise_nrg[g#%d][sfb#%d]]",
															std::get<1>(*iter), "0X%" PRIX64"", std::get<2>(*iter),
															"The Huffman codeword of differentially encoded noise energy", g, sfb);
													}
												}
											}
										}
										else
										{
											if (dpcm_sf[g][sfb] == 7 || dpcm_sf[g][sfb] == -7)
											{
												NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag300", "dpcm_sf[g#%d][sfb#%d]",
													esc_dpcm_sf[g][sfb], "Differential coded scalefactor", g, sfb);
												auto iter = std::find_if(rvlc_esc_hcb.cbegin(), rvlc_esc_hcb.cend(), [&](const auto& item) {return std::get<0>(item) == esc_dpcm_sf[g][sfb]; });
												if (iter != rvlc_esc_hcb.cend())
												{
													NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag301", "rvlc_cod_sf[dpcm_sf[g#%d][sfb#%d]]",
														std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
														"The Huffman codeword of Differential coded scalefactor", g, sfb);
												}
											}
										}
									}
								} // if (individual_channel_stream->section_data->sfb_cb[g][sfb] != ZERO_HCB)
							}
							NAV_WRITE_TAG_END("Tag30");
						}
						NAV_WRITE_TAG_END("Tag3");

						if (intensity_used && (dpcm_is_last_position == 7 || dpcm_is_last_position == -7))
						{
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("Tag4", "dpcm_is_last_position",
								esc_dpcm_is_last_position, "DPCM value allowing backward decoding of intensity stereo data part");
							auto iter = std::find_if(rvlc_esc_hcb.cbegin(), rvlc_esc_hcb.cend(), [&](const auto& item) {return std::get<0>(item) == esc_dpcm_is_last_position; });
							if (iter != rvlc_esc_hcb.cend())
							{
								NAV_FIELD_PROP_NUMBER_ALIAS_F("Tag5", "rvlc_esc_sf[dpcm_is_last_position]",
									std::get<1>(*iter), "0X%" PRIX64 "", std::get<2>(*iter),
									"The Huffman codeword of DPCM value allowing backward decoding of intensity stereo data part");
							}
						}
					}
					NAV_FIELD_PROP_NUMBER_END("sf_escapes_present");

					if (noise_used) {
						NAV_FIELD_PROP_2NUMBER1(dpcm_noise_last_position, 9, "DPCM value allowing backward decoding of PNS data part");
					}
				}
				DECLARE_FIELDPROP_END()
			}PACKED;

			struct TNS_DATA
			{
				uint8_t				n_filt[8] = { 0 };
				uint8_t				coef_res[8] = { 0 };
				uint8_t				length[8][3] = { {0} };
				uint8_t				order[8][3] = { {0} };
				uint8_t				direction[8][3] = { {0} };
				uint8_t				coef_compress[8][3] = { {0} };
				uint8_t				coef[8][3][32] = { {{0}} };

				INDIVIDUAL_CHANNEL_STREAM*
									individual_channel_stream = nullptr;

				TNS_DATA(INDIVIDUAL_CHANNEL_STREAM* pIndividualChannelStream)
					: individual_channel_stream(pIndividualChannelStream) {
				}

				int Unpack(CBitstream& bs)
				{
					bool window_with_128_spectral_lines = individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE ? true : false;
					for (uint8_t w = 0; w < individual_channel_stream->ics_info->num_windows; w++)
					{
						n_filt[w] = (uint8_t)bs.GetBits(window_with_128_spectral_lines ? 1 : 2);
						if (n_filt[w])
							coef_res[w] = (uint8_t)bs.GetBits(1);
						for (uint8_t filt = 0; filt < n_filt[w]; filt++)
						{
							length[w][filt] = (uint8_t)bs.GetBits(window_with_128_spectral_lines ? 4 : 6);
							order[w][filt] = (uint8_t)bs.GetBits(window_with_128_spectral_lines ? 3 : 5);
							if (order[w][filt])
							{
								direction[w][filt] = (uint8_t)bs.GetBits(1);
								coef_compress[w][filt] = (uint8_t)bs.GetBits(1);
								uint8_t coef_len = coef_res[w] + 3 - coef_compress[w][filt];
								for (uint8_t i = 0; i < order[w][filt]; i++)
									coef[w][filt][i] = (uint8_t)bs.GetBits(coef_len);
							} // if (order[w][filt])
						} // for (uint8_t filt = 0; filt < n_filt[w]; filt++)
					} // for (uint8_t w = 0; w <...

					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
				bool window_with_128_spectral_lines = individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE ? true : false;
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(w=0;w&lt;num_windows;w++)", "");
				for (uint8_t w = 0; w < individual_channel_stream->ics_info->num_windows; w++)
				{
					MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "n_filt[window#%d]", w);
					MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", n_filt[w]);
					NAV_FIELD_PROP_WITH_ALIAS_BEGIN("n_filt", szTemp4, window_with_128_spectral_lines ? 1 : 2, szTemp3,
						"number of noise shaping filters used for window w", bit_offset ? *bit_offset : -1LL, "I");
					if (n_filt[w])
					{
						MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "the resolution of the transmitted filter coefficients for window w: %d", coef_res[w] + 3);
						NAV_ARRAY_FIELD_PROP_NUMBER_("coef_res", "window#", w, 1, coef_res[w], szTemp4);
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(filt=0;filt&lt;n_filt[w];filt++)", "");
						for (uint8_t filt = 0; filt < n_filt[w]; filt++) {
							NAV_2ARRAY_FIELD_PROP_2NUMBER_("length", "window#", w, "filt#", filt, window_with_128_spectral_lines ? 4 : 6, length[w][filt], 
								"length of the region to which one filter is applied in window w (in units of scalefactor bands)");
							MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "order[window#%d][filt#%d]", w, filt);
							MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", order[w][filt]);
							NAV_FIELD_PROP_WITH_ALIAS_BEGIN("order", szTemp4, window_with_128_spectral_lines ? 3 : 5, szTemp3,
								"order of one noise shaping filter applied to window w", bit_offset ? *bit_offset : -1LL, "I");
							if (order[w][filt])
							{
								NAV_2ARRAY_FIELD_PROP_NUMBER_("direction", "window#", w, "filt#", filt, 1, direction[w][filt], direction[w][filt] ? "the filter is applied in downward direction" : "the filter is applied in upward direction");
								NAV_2ARRAY_FIELD_PROP_NUMBER_("coef_compress", "window#", w, "filt#", filt, 1, coef_compress[w][filt], coef_compress[w][filt] ? 
									"the most significant bit of the coefficients of the noise shaping filter filt in window w are omitted from transmission" : 
									"the most significant bit of the coefficients of the noise shaping filter filt in window w are NOT omitted from transmission");
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag000", "for(i=0;i&lt;order[w][filt];i++)", "");
								uint8_t coef_len = coef_res[w] + 3 - coef_compress[w][filt];
								for (uint8_t i = 0; i < order[w][filt]; i++) {
									NAV_FIELD_PROP_2NUMBER_ALIAS_F("coef", "coef[window#%d][filt#%d][%d]", coef_len, "%d(0X%X)", coef[w][filt][i], "coefficients of one noise shaping filter applied to window w", w, filt, i);
								}
								NAV_WRITE_TAG_END("Tag000");
							}
							NAV_FIELD_PROP_END("order");
						}
						NAV_WRITE_TAG_END("Tag00");
					}
					NAV_FIELD_PROP_END("n_filt");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()
			};

			struct LTP_DATA
			{
				uint16_t			ltp_lag_update : 1;
				uint16_t			ltp_lag : 11;
				uint16_t			ltp_coef : 3;
				uint16_t			reserved : 1;

				CAMBitArray			ltp_long_used;

				INDIVIDUAL_CHANNEL_STREAM*
									individual_channel_stream = nullptr;

				LTP_DATA(INDIVIDUAL_CHANNEL_STREAM* pIndividualChannelStream)
					: ltp_lag_update(0), ltp_lag(0), ltp_coef(0), reserved(0), individual_channel_stream(pIndividualChannelStream) {
				}

				int Unpack(CBitstream& bs)
				{
					if (individual_channel_stream == nullptr ||
						individual_channel_stream->ctx_audio_stream == nullptr ||
						individual_channel_stream->ctx_audio_stream->audio_specific_config == nullptr)
						return RET_CODE_ERROR;

					if (individual_channel_stream->ctx_audio_stream->audio_specific_config->GetAudioObjectType() == ER_AAC_LD)
					{
						ltp_lag_update = (uint16_t)bs.GetBits(1);
						if (ltp_lag_update)
							ltp_lag = (uint16_t)bs.GetBits(10);
						else
							ltp_lag = individual_channel_stream->ctx_audio_stream->ltp_prev_lag;

						ltp_coef = (uint16_t)bs.GetBits(3);
						for (uint8_t sfb = 0; sfb < AMP_MIN(MAX_LTP_LONG_SFB, individual_channel_stream->ics_info->max_sfb); sfb++)
						{
							bs.GetBits(1) ? ltp_long_used.BitSet(sfb) : ltp_long_used.BitClear(sfb);
						}
					}
					else
					{
						ltp_lag = (uint16_t)bs.GetBits(11);
						ltp_coef = (uint16_t)bs.GetBits(3);
						if (individual_channel_stream->ics_info->window_sequence != EIGHT_SHORT_SEQUENCE) {
							for (uint8_t sfb = 0; sfb < AMP_MIN(MAX_LTP_LONG_SFB, individual_channel_stream->ics_info->max_sfb); sfb++)
							{
								bs.GetBits(1) ? ltp_long_used.BitSet(sfb) : ltp_long_used.BitClear(sfb);
							}
						}
					}

					return RET_CODE_SUCCESS;
				}
			};

			/*
			Table 4.56 Syntax of spectral_data()
			*/
			struct SPECTRAL_DATA
			{
				INDIVIDUAL_CHANNEL_STREAM*
								individual_channel_stream = nullptr;
				uint16_t		hcb_index[8][64][1024] = { { {0}} };
				int16_t			spectral[8][64][1024] = { { {0}} };

				SPECTRAL_DATA(INDIVIDUAL_CHANNEL_STREAM* pIndividualChannelStream)
					: individual_channel_stream(pIndividualChannelStream) {
				}

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					const Spectrum_Huffman_Codebook_Quad* shcb_quad[] = { nullptr,
						&shcb1, &shcb2, &shcb3, &shcb4, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
						nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
					};
					const Spectrum_Huffman_Codebook_Pair* shcb_pair[] = { nullptr,
						nullptr, nullptr, nullptr, nullptr,&shcb5, &shcb6, &shcb7, &shcb8, &shcb9, &shcb10, &shcb11, nullptr, nullptr, nullptr, nullptr,
						&shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11
					};

					uint16_t nshort = individual_channel_stream->ctx_audio_stream->GetWindowLength()>>4;
					for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++)
					{
						for (uint8_t i = 0; i < individual_channel_stream->section_data->num_sec[g]; i++)
						{
							uint16_t p = 0;
							uint8_t sect_cb = individual_channel_stream->section_data->sect_cb[g][i];
							uint16_t inc = (sect_cb >= FIRST_PAIR_HCB) ? 2 : 4;
							if (sect_cb != ZERO_HCB &&
								sect_cb != NOISE_HCB &&
								sect_cb != INTENSITY_HCB &&
								sect_cb != INTENSITY_HCB2)
							{
								for (uint16_t k = individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_start[g][i]];
											  k < individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_end[g][i]]; k += inc)
								{
									// Didn't do any decoding, only skip the bits
									int shcb_index = -1;
									if((shcb_index = shcb_read_value(bs, sect_cb)) < 0)
									{
										printf("[AAC] Failed to read value from spectral data.\n");
										iRet = RET_CODE_ERROR;
										goto done;
									}

									hcb_index[g][i][k] = (uint16_t)shcb_index;

									if (shcb_quad[sect_cb] != nullptr)
									{
										spectral[g][i][p]   = std::get<1>(std::get<0>((*shcb_quad[sect_cb])[shcb_index]));
										spectral[g][i][p+1] = std::get<2>(std::get<0>((*shcb_quad[sect_cb])[shcb_index]));
										spectral[g][i][p+2] = std::get<3>(std::get<0>((*shcb_quad[sect_cb])[shcb_index]));
										spectral[g][i][p+3] = std::get<4>(std::get<0>((*shcb_quad[sect_cb])[shcb_index]));
									}
									else if (shcb_pair[sect_cb] != nullptr)
									{
										spectral[g][i][p]   = std::get<1>(std::get<0>((*shcb_pair[sect_cb])[shcb_index]));
										spectral[g][i][p+1] = std::get<2>(std::get<0>((*shcb_pair[sect_cb])[shcb_index]));
									}

									// Extract the sign bits
									if (std::get<0>(spectrum_hcb_params[sect_cb]))
									{
										for (uint8_t s = 0; s < inc; s++)
										{
											if (spectral[g][i][p + s])
												if (bs.GetBits(1))
													spectral[g][i][p + s] = -spectral[g][i][p + s];
										}
									}

									if (sect_cb == 11 || (sect_cb >= 16 && sect_cb <= 31))
									{
										assert(shcb_pair[sect_cb] != nullptr);
										spectral[g][i][p] = huffman_getescape(bs, spectral[g][i][p]);
										spectral[g][i][p + 1] = huffman_getescape(bs, spectral[g][i][p + 1]);
									}

									p += inc;
								}
							}
							else
							{
								p += individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_end[g][i]] -
									 individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_start[g][i]];
							}
						}
					}

				done:
					return iRet;
				}

				DECLARE_FIELDPROP_BEGIN()
				const Spectrum_Huffman_Codebook_Quad* shcb_quad[] = { nullptr,
					&shcb1, &shcb2, &shcb3, &shcb4, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
					nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
				};
				const Spectrum_Huffman_Codebook_Pair* shcb_pair[] = { nullptr,
					nullptr, nullptr, nullptr, nullptr,&shcb5, &shcb6, &shcb7, &shcb8, &shcb9, &shcb10, &shcb11, nullptr, nullptr, nullptr, nullptr,
					&shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11, &shcb11
				};
				MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "for(g=0;g&lt;num_window_groups(%d);g++)", individual_channel_stream->ics_info->num_window_groups);
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", szTemp2, "");
				for (uint8_t g = 0; g < individual_channel_stream->ics_info->num_window_groups; g++) {
					MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "for(i=0;i&lt;num_sec[%d](%d);i++)", g, individual_channel_stream->section_data->num_sec[g]);
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", szTemp2, "");
					for (uint8_t i = 0; i < individual_channel_stream->section_data->num_sec[g]; i++) {
						uint8_t sect_cb = individual_channel_stream->section_data->sect_cb[g][i];
						uint16_t inc = (sect_cb >= FIRST_PAIR_HCB) ? 2 : 4, p = 0;
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS_DESC_F("sect_cb", std::get<5>(spectrum_hcb_params[sect_cb]), 
							"sect_cb:%d, unsigned: %s, dimension: %d, LAV: %d", sect_cb, 
							std::get<0>(spectrum_hcb_params[sect_cb])?"yes":"no",
							std::get<1>(spectrum_hcb_params[sect_cb]),
							std::get<2>(spectrum_hcb_params[sect_cb]));
						
						if (sect_cb != ZERO_HCB &&
							sect_cb != NOISE_HCB &&
							sect_cb != INTENSITY_HCB &&
							sect_cb != INTENSITY_HCB2)
						{
							MBCSPRINTF_S(szTemp2, TEMP2_SIZE, 
								"for(k=sect_sfb_offset[g#%d][sect_start[g#%d][i#%d]](%d);k&lt;sect_sfb_offset[g#%d][sect_end[g#%d][i#%d]](%d);)",
								g, g, i, individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_start[g][i]],
								g, g, i, individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_end[g][i]]);
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0000", szTemp2, "");

							for (uint16_t k = individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_start[g][i]];
								          k < individual_channel_stream->ics_info->sect_sfb_offset[g][individual_channel_stream->section_data->sect_end[g][i]]; k += inc)
							{
								szTemp2[0] = 0;
								if (inc == 2)
								{
									MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%d, %d", spectral[g][i][p], spectral[g][i][p + 1]);
								}
								else if (inc == 4)
								{
									MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%d, %d, %d, %d", spectral[g][i][p], spectral[g][i][p + 1], spectral[g][i][p + 2], spectral[g][i][p + 3]);
								}

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS_F("sp_data", "[%04d - %04d]", szTemp2, k, k + inc - 1);
								if (shcb_quad[sect_cb] != nullptr)
								{
									uint8_t field_bits = std::get<1>((*shcb_quad[sect_cb])[hcb_index[g][i][k]]);
									uint64_t field_value = std::get<2>((*shcb_quad[sect_cb])[hcb_index[g][i][k]]);
									NAV_FIELD_PROP_NUMBER_BEGIN("hcb", field_bits, field_value, "");

									auto& tu = std::get<0>((*shcb_quad[sect_cb])[hcb_index[g][i][k]]);

									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("sp", "[%04d]", std::get<1>(tu), "w", p);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("sp", "[%04d]", std::get<2>(tu), "x", p + 1);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("sp", "[%04d]", std::get<3>(tu), "y", p + 2);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("sp", "[%04d]", std::get<4>(tu), "z", p + 3);
								}
								else if (shcb_pair[sect_cb] != nullptr)
								{
									uint8_t field_bits = std::get<1>((*shcb_pair[sect_cb])[hcb_index[g][i][k]]);
									uint64_t field_value = std::get<2>((*shcb_pair[sect_cb])[hcb_index[g][i][k]]);
									NAV_FIELD_PROP_NUMBER_BEGIN("hcb", field_bits, field_value, "");

									auto& tu = std::get<0>((*shcb_pair[sect_cb])[hcb_index[g][i][k]]);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("sp", "[%04d]", std::get<1>(tu), "y", p);
									NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("sp", "[%04d]", std::get<2>(tu), "z", p + 1);
								}

								NAV_FIELD_PROP_NUMBER_END("hcb");

								// Show sign bits
								if (std::get<0>(spectrum_hcb_params[sect_cb]))
								{
									NAV_WRITE_TAG_BEGIN2("sign_bits");
									for (uint8_t s = 0; s < inc; s++)
									{
										if (spectral[g][i][p + s] < 0)
										{
											NAV_FIELD_PROP_NUMBER_ALIAS_F("sign_bit", "[%d]", 1, "%d", 1, "", s);
										}
										else if(spectral[g][i][p + s] > 0)
										{
											NAV_FIELD_PROP_NUMBER_ALIAS_F("sign_bit", "[%d]", 1, "%d", 0, "", s);
										}
									}
									NAV_WRITE_TAG_END("sign_bits");
								}

								// Show escape bits
								if (sect_cb == 11 || (sect_cb >= 16 && sect_cb <= 31))
								{
									assert(shcb_pair[sect_cb] != nullptr);

									auto& tu = std::get<0>((*shcb_pair[sect_cb])[hcb_index[g][i][k]]);
									if (std::get<1>(tu) == ESC_FLAG)
									{
										NAV_FIELD_PROP_2NUMBER("hcod_esc_y", esc_bitlen(spectral[g][i][p]), esc_codword(spectral[g][i][p]), "");
									}

									if (std::get<2>(tu) == ESC_FLAG)
									{
										NAV_FIELD_PROP_2NUMBER("hcod_esc_z", esc_bitlen(spectral[g][i][p + 1]), esc_codword(spectral[g][i][p + 1]), "");
									}
								}
								NAV_WRITE_TAG_END("sp_data");

								p += inc;
							}

							NAV_WRITE_TAG_END("Tag0000");
						}

						NAV_WRITE_TAG_END("sect_cb");
					}
					NAV_WRITE_TAG_END("Tag00");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()
			};

			struct PULSE_DATA
			{
				uint8_t			number_pulse : 2;
				uint8_t			pulse_start_sfb : 6;
				std::vector<std::tuple<uint8_t, uint8_t>>
								pulse;

				PULSE_DATA() : number_pulse(0), pulse_start_sfb(0) {
				}

				int Unpack(CBitstream& bs)
				{
					number_pulse = (uint8_t)bs.GetBits(2);
					pulse_start_sfb = (uint8_t)bs.GetBits(6);

					for (int i = 0; i < number_pulse + 1; i++) {
						uint8_t pulse_offset = (uint8_t)bs.GetBits(5);
						uint8_t pulse_amp = (uint8_t)bs.GetBits(4);

						pulse.push_back(std::make_tuple(pulse_offset, pulse_amp));
					}

					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(number_pulse, 2, "how many pulse escapes are used. The number of pulse escapes is from 1 to 4");
				NAV_FIELD_PROP_2NUMBER1(pulse_start_sfb, 6, "the index of the lowest scalefactor band where the pulse escape is achieved");
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;number_pulse+1;i++)", "");
				for (int i = 0; i < number_pulse + 1; i++) {
					NAV_ARRAY_FIELD_PROP_2NUMBER("pulse_offset", i, 5, std::get<0>(pulse[i]), "the offset");
					NAV_ARRAY_FIELD_PROP_2NUMBER("pulse_amp", i, 4, std::get<1>(pulse[i]), "the unsigned magnitude of the pulse");
				}
				NAV_WRITE_TAG_END("Tag0");
				DECLARE_FIELDPROP_END()
			};

			struct GAIN_CONTROL_DATA
			{
				uint8_t				max_band : 2;
				uint8_t				reserved : 6;

				uint8_t				adjust_num[4][8] = { {0} };
				uint8_t				alevcode[4][8][7] = { {{0}} };
				uint8_t				aloccode[4][8][7] = { {{0}} };

				INDIVIDUAL_CHANNEL_STREAM*
									individual_channel_stream = nullptr;

				GAIN_CONTROL_DATA(INDIVIDUAL_CHANNEL_STREAM* pIndividualChannelStream)
					: max_band(0), reserved(0),individual_channel_stream(pIndividualChannelStream) {
				}

				int Unpack(CBitstream& bs)
				{
					uint8_t bd, wd, ad;
					max_band = (uint8_t)bs.GetBits(2);
					if (individual_channel_stream->ics_info->window_sequence == ONLY_LONG_SEQUENCE)
					{
						for (bd = 1; bd <= max_band; bd++) {
							for (wd = 0; wd < 1; wd++) {
								adjust_num[bd][wd] = (uint8_t)bs.GetBits(3);
								for (ad = 0; ad < adjust_num[bd][wd]; ad++) {
									alevcode[bd][wd][ad] = (uint8_t)bs.GetBits(4);
									aloccode[bd][wd][ad] = (uint8_t)bs.GetBits(5);
								}
							}
						}
					}
					else if (individual_channel_stream->ics_info->window_sequence == LONG_START_SEQUENCE)
					{
						for (bd = 1; bd <= max_band; bd++) {
							for (wd = 0; wd < 2; wd++) {
								adjust_num[bd][wd] = (uint8_t)bs.GetBits(3);
								for (ad = 0; ad < adjust_num[bd][wd]; ad++) {
									alevcode[bd][wd][ad] = (uint8_t)bs.GetBits(4);
									if (wd == 0)
										aloccode[bd][wd][ad] = (uint8_t)bs.GetBits(4);
									else
										aloccode[bd][wd][ad] = (uint8_t)bs.GetBits(2);
								}
							}
						}
					}
					else if (individual_channel_stream->ics_info->window_sequence == EIGHT_SHORT_SEQUENCE)
					{
						for (bd = 1; bd <= max_band; bd++) {
							for (wd = 0; wd < 8; wd++) {
								adjust_num[bd][wd] = (uint8_t)bs.GetBits(3);
								for (ad = 0; ad < adjust_num[bd][wd]; ad++) {
									alevcode[bd][wd][ad] = (uint8_t)bs.GetBits(4);
									aloccode[bd][wd][ad] = (uint8_t)bs.GetBits(2);
								}
							}
						}
					}
					else if (individual_channel_stream->ics_info->window_sequence == LONG_STOP_SEQUENCE)
					{
						for (bd = 1; bd <= max_band; bd++) {
							for (wd = 0; wd < 2; wd++) {
								adjust_num[bd][wd] = (uint8_t)bs.GetBits(3);
								for (ad = 0; ad < adjust_num[bd][wd]; ad++) {
									alevcode[bd][wd][ad] = (uint8_t)bs.GetBits(4);
									if (wd == 0)
										aloccode[bd][wd][ad] = (uint8_t)bs.GetBits(4);
									else
										aloccode[bd][wd][ad] = (uint8_t)bs.GetBits(5);
								}
							}
						}
					}

					return RET_CODE_SUCCESS;
				}

				DECLARE_FIELDPROP_BEGIN()

				DECLARE_FIELDPROP_END()
			};

			uint32_t			global_gain;
			uint32_t			pulse_data_present : 1;
			uint32_t			tns_data_present : 1;
			uint32_t			gain_control_data_present : 1;
			uint32_t			length_of_reordered_spectral_data : 14;
			uint32_t			length_of_longest_codeword : 6;
			uint32_t			reserved : 1;

			ICS_INFO*			ics_info;

			SECTION_DATA*		section_data = nullptr;
			SCALE_FACTOR_DATA*	scale_factor_data = nullptr;
			PULSE_DATA*			pulse_data = nullptr;
			TNS_DATA*			tns_data = nullptr;
			GAIN_CONTROL_DATA*	gain_control_data = nullptr;

			SPECTRAL_DATA*		spectral_data = nullptr;

			bool				common_window;
			bool				scale_flag;
			CAudioStreamContext*
								ctx_audio_stream = nullptr;

			INDIVIDUAL_CHANNEL_STREAM(CAudioStreamContext* pCtxAudioStream, bool bCommonWindow, bool bScaleFlag, ICS_INFO* pICSInfo = nullptr)
				: ics_info(pICSInfo), common_window(bCommonWindow), scale_flag(bScaleFlag), ctx_audio_stream(pCtxAudioStream) {
			}

			~INDIVIDUAL_CHANNEL_STREAM() {
				if (!common_window && !scale_flag && ics_info != nullptr)
				{
					delete ics_info; ics_info = nullptr;
				}

				AMP_SAFEDEL2(section_data);
				AMP_SAFEDEL2(scale_factor_data);
				AMP_SAFEDEL2(pulse_data);
				AMP_SAFEDEL2(tns_data);
				AMP_SAFEDEL2(gain_control_data);
				AMP_SAFEDEL2(spectral_data);
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				global_gain = bs.GetByte();
				if (!common_window && !scale_flag)
				{
					ics_info = new ICS_INFO(ctx_audio_stream);
					if ((iRet = ics_info->Unpack(bs)) < 0)
						goto done;
				}

				section_data = new SECTION_DATA(this);
				if ((iRet = section_data->Unpack(bs)) < 0)
					goto done;

				scale_factor_data = new SCALE_FACTOR_DATA(this);
				if ((iRet = scale_factor_data->Unpack(bs)) < 0)
					goto done;

				if (!scale_flag){
					pulse_data_present = (uint32_t)bs.GetBits(1);
					if (pulse_data_present){
						pulse_data = new PULSE_DATA();
						if ((iRet = pulse_data->Unpack(bs)) < 0)
							goto done;
					}

					tns_data_present = (uint32_t)bs.GetBits(1);
					if (tns_data_present){
						tns_data = new TNS_DATA(this);
						if ((iRet = tns_data->Unpack(bs)) < 0)
							goto done;
					}

					gain_control_data_present = (uint32_t)bs.GetBits(1);
					if (gain_control_data_present){
						gain_control_data = new GAIN_CONTROL_DATA(this);
						if ((iRet = gain_control_data->Unpack(bs)) < 0)
							goto done;
					}
				}

				if (ctx_audio_stream->audio_specific_config == NULL || (
					ctx_audio_stream->audio_specific_config != nullptr &&
					ctx_audio_stream->audio_specific_config->GASpecConfig != nullptr &&
					!ctx_audio_stream->audio_specific_config->GASpecConfig->aacSpectralDataResilienceFlag))
				{
					// TODO...
					// spectral_data();
					spectral_data = new SPECTRAL_DATA(this);
					if ((iRet = spectral_data->Unpack(bs)) < 0)
						goto done;
				}
				else
				{
					length_of_reordered_spectral_data = (uint32_t)bs.GetBits(14);
					length_of_longest_codeword = (uint32_t)bs.GetBits(6);
				}

			done:

				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(global_gain, 8, "representing the value of the first scalefactor.It is also the start value for the following differential coded scalefactors");
			if (!common_window && !scale_flag)
			{
				NAV_FIELD_PROP_REF2_2(ics_info, "ics_info", "");
			}
			NAV_FIELD_PROP_REF2_2(section_data, "section_data", "");
			NAV_FIELD_PROP_REF2_2(scale_factor_data, "scale_factor_data", "");

			if (!scale_flag) {
				MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)pulse_data_present);
				NAV_FIELD_PROP_BEGIN("pulse_data_present", 1, szTemp3,
					pulse_data_present ? "pulse data present" : "pulse data NOT present", bit_offset ? *bit_offset : -1LL, "I");

				if (pulse_data_present) {
					NAV_FIELD_PROP_REF2_2(pulse_data, "pulse_data", "");
				}
				NAV_FIELD_PROP_END("pulse_data_present");

				MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)tns_data_present);
				NAV_FIELD_PROP_BEGIN("tns_data_present", 1, szTemp3,
					tns_data_present ? "tns data present" : "tns data NOT present", bit_offset ? *bit_offset : -1LL, "I");
				if (tns_data_present) {
					NAV_FIELD_PROP_REF2_2(tns_data, "tns_data", "");
				}
				NAV_FIELD_PROP_END("tns_data_present");

				MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "%d", (int)gain_control_data_present);
				NAV_FIELD_PROP_BEGIN("gain_control_data_present", 1, szTemp3,
					gain_control_data_present ? "gain_control_data present" : "gain_control_data NOT present", bit_offset ? *bit_offset : -1LL, "I");
				if (gain_control_data_present) {
					NAV_FIELD_PROP_REF2_2(gain_control_data, "gain_control_data", "");
				}
				NAV_FIELD_PROP_END("gain_control_data_present");
			}

			if (ctx_audio_stream->audio_specific_config == nullptr || (
				ctx_audio_stream->audio_specific_config != nullptr &&
				ctx_audio_stream->audio_specific_config->GASpecConfig != nullptr &&
				!ctx_audio_stream->audio_specific_config->GASpecConfig->aacSpectralDataResilienceFlag))
			{
				NAV_FIELD_PROP_REF2_2(spectral_data, "spectral_data", "");
			}
			else
			{

			}

			DECLARE_FIELDPROP_END()

		};

		struct SINGLE_CHANNEL_ELEMENT
		{
			uint32_t		element_insatance_tag : 4;
			uint32_t		reserved : 28;

			INDIVIDUAL_CHANNEL_STREAM*
							individual_channel_stream;

			CAudioStreamContext*
							ctx_audio_stream = nullptr;

			SINGLE_CHANNEL_ELEMENT(CAudioStreamContext* pCtxAudioStream) :
				individual_channel_stream(nullptr),
				ctx_audio_stream(pCtxAudioStream) {
			}

			virtual ~SINGLE_CHANNEL_ELEMENT()
			{
				AMP_SAFEDEL(individual_channel_stream);
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				element_insatance_tag = (uint8_t)bs.GetBits(4);

				individual_channel_stream = new INDIVIDUAL_CHANNEL_STREAM(ctx_audio_stream, 0, 0, nullptr);
				if ((iRet = individual_channel_stream->Unpack(bs)) < 0)
					goto done;

			done:
				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(element_insatance_tag, 4, "Unique instance tag for syntactic elements. All syntactic elements containing instance tags may occur more than once, but must have a unique element_instance_tag in each audio frame.");
			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("individual_channel_stream", "individual_channel_stream(0,0)", "");
			NAV_FIELD_PROP_REF(individual_channel_stream);
			NAV_WRITE_TAG_END("individual_channel_stream");
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CHANNEL_PAIR_ELEMENT
		{
			uint8_t			element_insatance_tag : 4;
			uint8_t			common_window : 1;
			uint8_t			ms_mask_present : 2;
			uint8_t			reserved_0 : 1;

			ICS_INFO*		ics_info = nullptr;
			CAMBitArray		ms_used;

			INDIVIDUAL_CHANNEL_STREAM*	individual_channel_streams[2] = { 0 };

			CAudioStreamContext*
							ctx_audio_stream = nullptr;

			CHANNEL_PAIR_ELEMENT(CAudioStreamContext* pCtxAudioStream) :
				ctx_audio_stream(pCtxAudioStream) {
			}

			virtual ~CHANNEL_PAIR_ELEMENT() {
				AMP_SAFEDEL2(individual_channel_streams[0]);
				AMP_SAFEDEL2(individual_channel_streams[1]);
				AMP_SAFEDEL2(ics_info);
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				element_insatance_tag = (uint8_t)bs.GetBits(4);
				common_window = (uint8_t)bs.GetBits(1);

				if (common_window)
				{
					ics_info = new ICS_INFO(ctx_audio_stream);
					if ((iRet = ics_info->Unpack(bs)) < 0)
					{
						delete ics_info;
						ics_info = NULL;
						goto done;
					}

					ms_mask_present = (uint8_t)bs.GetBits(2);
					if (ms_mask_present == 1)
					{
						for (uint8_t g = 0; g < ics_info->num_window_groups; g++)
						{
							for (uint8_t sfb = 0; sfb < ics_info->max_sfb; sfb++)
							{
								bs.GetBits(1) ? ms_used.BitSet(g*ics_info->num_window_groups + sfb) : ms_used.BitClear(g*ics_info->num_window_groups + sfb);
							}
						}
					}
				}
				
				individual_channel_streams[0] = new INDIVIDUAL_CHANNEL_STREAM(ctx_audio_stream, common_window, 0, common_window ? ics_info : nullptr);
				if ((iRet = individual_channel_streams[0]->Unpack(bs)) < 0)
					goto done;

				individual_channel_streams[1] = new INDIVIDUAL_CHANNEL_STREAM(ctx_audio_stream, common_window, 0, common_window ? ics_info : nullptr);
				if ((iRet = individual_channel_streams[1]->Unpack(bs)) < 0)
					goto done;

			done:
				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(element_insatance_tag, 4, "Unique instance tag for syntactic elements. All syntactic elements containing instance tags may occur more than once, but must have a unique element_instance_tag in each audio frame.");
			NAV_FIELD_PROP_NUMBER1(common_window, 1, "a flag indicating whether the two individual_channel_streams share a common ics_info or not");
			if (common_window)
			{
				NAV_FIELD_PROP_REF1(ics_info);
				NAV_FIELD_PROP_2NUMBER1(ms_mask_present, 2, ms_mask_present==0?"Independent":(
															ms_mask_present==1?"1 bit mask of ms_used is located in the layer sfb side information part(layer_sfb_si()).":(
															ms_mask_present==2?"All ms_used are ones":(
															ms_mask_present==3?"2 bit mask of stereo_info is located in the layer sfb side information part layer_sfb_si())":"Unknown"))));
				if (ms_mask_present == 1)
				{
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(g=0;g&lt;num_window_groups;g++)", "");
					for (uint8_t g = 0; g < ics_info->num_window_groups; g++)
					{
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(sfb=0;sfb&lt;max_sfb;sfb++)", "");
						for (uint8_t sfb = 0; sfb < ics_info->max_sfb; sfb++)
						{
							NAV_2ARRAY_FIELD_PROP_NUMBER_("ms_used", "group#", g, "sfb#", sfb, 1, ms_used[g*ics_info->num_window_groups + sfb], ms_used[g*ics_info->num_window_groups + sfb] ? "M/S coding used" : "M/S coding NOT used");
						}
						NAV_WRITE_TAG_END("Tag00");
					}
					NAV_WRITE_TAG_END("Tag0");
				}
			}

			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("individual_channel_stream", "individual_channel_stream(common_window,0)", "");
			NAV_FIELD_PROP_REF(individual_channel_streams[0]);
			NAV_WRITE_TAG_END("individual_channel_stream");

			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("individual_channel_stream", "individual_channel_stream(common_window,0)", "");
			NAV_FIELD_PROP_REF(individual_channel_streams[1]);
			NAV_WRITE_TAG_END("individual_channel_stream");

			DECLARE_FIELDPROP_END()

		};

		struct DATA_STREAM_ELEMENT
		{
			uint8_t		element_instance_tag : 4;
			uint8_t		data_byte_align_flag : 1;
			uint8_t		reserved_0 : 3;

			uint8_t		count = 0;
			uint8_t		esc_count = 0;
			std::vector<uint8_t>
						data_stream_bytes;

			DATA_STREAM_ELEMENT()
				: element_instance_tag(0), data_byte_align_flag(0), reserved_0(0) {
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				
				element_instance_tag = (uint8_t)bs.GetBits(4);
				data_byte_align_flag = (uint8_t)bs.GetBits(1);

				int cnt = count = bs.GetByte();
				esc_count = cnt == 0xFF ? bs.GetByte() : 0;
				cnt += esc_count;

				if (data_byte_align_flag)
					bs.Realign();

				if (cnt > 0)
				{
					data_stream_bytes.resize(cnt);
					for (int i = 0; i < cnt; i++)
						data_stream_bytes[i] = bs.GetByte();
				}

				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(element_instance_tag, 4, "");
				NAV_FIELD_PROP_NUMBER1(data_byte_align_flag, 1, "");
				NAV_FIELD_PROP_2NUMBER_ALIAS_F("count", "cnt = count", 8, "0X%0X(%d)", count, "");
				if (count == 0xFF) {
					NAV_FIELD_PROP_2NUMBER_ALIAS_F("esc_count", "cnt += esc_count", 8, "0X%X(%d)", esc_count, "");
				}
				int cnt = count + esc_count;
				NAV_WRITE_TAG_WITH_1NUMBER_VALUE("cnt", count + esc_count, "the number of data stream bytes");
				if (data_byte_align_flag)
				{
					if (bit_offset && (*bit_offset % 8) != 0)
						*bit_offset += 8 - (*bit_offset % 8);
				}
				if (cnt > 0)
				{
					NAV_WRITE_TAG_BEGIN2_1("data_stream_bytes", "data stream bytes till to the end");
					int nLenDecimal = (int)(log10(cnt) + 1);
					char szFmtStr[32];
					MBCSPRINTF_S(szFmtStr, 32, "[%%0%dd]", nLenDecimal);
					for (int i = 0; i < cnt; i++)
					{
						NAV_FIELD_PROP_2NUMBER_ALIAS_F("byte", szFmtStr, 8, "0X%02X(%d)", data_stream_bytes[i], "", i);
					}
					NAV_WRITE_TAG_END("data_stream_bytes");
				}
			DECLARE_FIELDPROP_END()
		};

		struct FILL_ELEMENT
		{
			uint8_t		count = 0;
			uint8_t		esc_count = 0;

			std::vector<EXTENSION_PAYLOAD*>
						extension_payloads;

			~FILL_ELEMENT() {
				for (auto v : extension_payloads)
					delete v;
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_ERROR;
				int cnt = count = (uint8_t)bs.GetBits(4);
				if (cnt == 15)
				{
					esc_count = (uint8_t)bs.GetByte();
					cnt += esc_count - 1;
				}

				if (cnt == 0)
					iRet = RET_CODE_SUCCESS;

				while (cnt > 0) {
					uint64_t start_bitpos = bs.Tell();
					EXTENSION_PAYLOAD* pExtPayload = new EXTENSION_PAYLOAD();
					if ((iRet = pExtPayload->Unpack(bs, cnt)) < 0)
						break;

					extension_payloads.push_back(pExtPayload);

					uint64_t end_bitpos = bs.Tell();
					assert((end_bitpos - start_bitpos) % 8 == 0);
					cnt -= (int)((end_bitpos - start_bitpos) >> 3);
				}

				if (iRet >= 0)
					assert(cnt == 0);

				return iRet;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER_ALIAS_F("count", "cnt = count", 4, "%d(0X%X)", count, "Initial value for length of fill data");
				if (count == 15)
				{
					int cnt = count + esc_count - 1;
					NAV_FIELD_PROP_2NUMBER1(esc_count, 8, "Incremental value of length of fill data");
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(cnt, "value equal to the total length in bytes of all subsequent extensioin_payload()'s");
				}
				NAV_WRITE_TAG_BEGIN2("extension_payloads");
				for (i = 0; i < (int)extension_payloads.size(); i++) {
					NAV_WRITE_TAG_ARRAY_BEGIN__("extension_payload", "", "", i, "%s", "");
					NAV_FIELD_PROP_REF(extension_payloads[i]);
					NAV_WRITE_TAG_END("extension_payload");
				}
				NAV_WRITE_TAG_END("extension_payloads");
			DECLARE_FIELDPROP_END()

		};

		struct RAW_DATA_BLOCK : public AAC_SYNTAX_BITSTREAM
		{
			struct DATA_ELEMENT
			{
				uint64_t	start_bitpos = 0;
				uint8_t		id_syn_ele = 0;
				uint8_t		reserved[3] = { 0 };
				union
				{
					SINGLE_CHANNEL_ELEMENT*	single_channel_element;
					CHANNEL_PAIR_ELEMENT*	channel_pair_element;
					DATA_STREAM_ELEMENT*	data_stream_element;
					PROGRAM_CONFIG_ELEMENT*	program_config_element;
					FILL_ELEMENT*			fill_element;
					void*					ptr_element = nullptr;
				};

				DATA_ELEMENT(uint8_t IDSynEle, CAudioStreamContext* pAudStmCtx) : id_syn_ele(IDSynEle) {
					switch (id_syn_ele)
					{
					case ID_SCE:
						single_channel_element = new SINGLE_CHANNEL_ELEMENT(pAudStmCtx);
						break;
					case ID_CPE:
						channel_pair_element = new CHANNEL_PAIR_ELEMENT(pAudStmCtx);
						break;
					case ID_CCE:
						break;
					case ID_LFE:
						break;
					case ID_DSE:
						data_stream_element = new DATA_STREAM_ELEMENT();
						break;
					case ID_PCE:
						program_config_element = new PROGRAM_CONFIG_ELEMENT();
						break;
					case ID_FIL:
						fill_element = new FILL_ELEMENT();
						break;
					}
				}

				int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_ERROR_NOTIMPL;
					switch (id_syn_ele)
					{
					case ID_SCE:
						iRet = single_channel_element->Unpack(bs);
						break;
					case ID_CPE:
						iRet = channel_pair_element->Unpack(bs);
						break;
					case ID_CCE:
						break;
					case ID_LFE:
						break;
					case ID_DSE:
						iRet = data_stream_element->Unpack(bs);
						break;
					case ID_PCE:
						iRet = program_config_element->Unpack(bs);
						break;
					case ID_FIL:
						iRet = fill_element->Unpack(bs);
						break;
					}

					return iRet;
				}

				~DATA_ELEMENT()
				{
					switch (id_syn_ele)
					{
					case ID_SCE:
						AMP_SAFEDEL2(single_channel_element);
						break;
					case ID_CPE:
						AMP_SAFEDEL2(channel_pair_element);
						break;
					case ID_CCE:
						break;
					case ID_LFE:
						break;
					case ID_DSE:
						AMP_SAFEDEL2(data_stream_element);
						break;
					case ID_PCE:
						AMP_SAFEDEL2(program_config_element);
						break;
					case ID_FIL:
						AMP_SAFEDEL2(fill_element);
						break;
					}
				}
			}PACKED;

			std::vector<DATA_ELEMENT*>
									data_elements;
			CAudioStreamContext*	ctx_audio_stream = nullptr;

			RAW_DATA_BLOCK(CAudioStreamContext* pAudStmCtx)
				: ctx_audio_stream(pAudStmCtx)
			{
			}

			virtual ~RAW_DATA_BLOCK()
			{
				for (auto& v : data_elements)
				{
					AMP_SAFEDEL2(v);
				}
			}

			int Unpack(CBitstream& bs, int start_bit_offset);
			int Unpack(CBitstream& bs);
			int Unpack(AMBst in_bst);

			DECLARE_FIELDPROP_BEGIN()

			for (size_t idx = 0; idx < data_elements.size(); idx++)
			{
				if (bit_offset)
					*bit_offset = data_elements[idx]->start_bitpos;

				NAV_FIELD_PROP_2NUMBER("id_syn_ele", 3, data_elements[idx]->id_syn_ele, SYNTAX_ELEMENT_NAME(data_elements[idx]->id_syn_ele));
				switch (data_elements[idx]->id_syn_ele)
				{
				case ID_SCE:
					NAV_FIELD_PROP_REF2_1(data_elements[idx]->single_channel_element, "single_channel_element", "");
					break;
				case ID_CPE:
					NAV_FIELD_PROP_REF2_1(data_elements[idx]->channel_pair_element, "channel_pair_element", "");
					break;
				case ID_CCE:
					break;
				case ID_LFE:
					break;
				case ID_DSE:
					NAV_FIELD_PROP_REF2_1(data_elements[idx]->data_stream_element, "data_stream_element", "");
					break;
				case ID_PCE:
					NAV_FIELD_PROP_REF2_1(data_elements[idx]->program_config_element, "program_config_element", "");
					break;
				case ID_FIL:
					NAV_FIELD_PROP_REF2_1(data_elements[idx]->fill_element, "Fill_element", "");
					break;
				case ID_END:
					break;
				}
			}

			DECLARE_FIELDPROP_END()

		};

		struct CAudioBitstream : public AAC_SYNTAX_BITSTREAM
		{
			RAW_DATA_BLOCK*			ptr_raw_data_block = nullptr;
			CAudioStreamContext		ctx_audio_bst;

			CAudioBitstream(AudioSpecificConfig* AudSpecConf) {
				ctx_audio_bst.audio_specific_config = AudSpecConf;
			}

			virtual ~CAudioBitstream()
			{
				AMP_SAFEDEL2(ptr_raw_data_block);
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = AAC_SYNTAX_BITSTREAM::Unpack(bs);
				if (iRet < 0)
					return iRet;

				AMP_NEWT(ptr_raw_data_block, RAW_DATA_BLOCK, &ctx_audio_bst);
				return ptr_raw_data_block->Unpack(bs);
			}

			DECLARE_FIELDPROP_BEGIN()
			if (ptr_raw_data_block != nullptr)
			{
				NAV_WRITE_TAG_BEGIN2_1("raw_data_block", "Payloads for the audio object types AAC main, AAC SSR, AAC LC and AAC LTP");
				NAV_FIELD_PROP_REF(ptr_raw_data_block);
				NAV_WRITE_TAG_END("raw_data_block");
			}
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct ADTSBitstream : public SYNTAX_BITSTREAM_MAP
		{
			struct ADTSFrame : public SYNTAX_BITSTREAM_MAP
			{
				struct ADTSFixedHeader : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t	syncword : 12;
					uint16_t	ID : 1;
					uint16_t	layer : 2;
					uint16_t	protection_absent : 1;

					uint16_t	profile_ObjectType : 2;
					uint16_t	sampling_frequency_index : 4;
					uint16_t	private_bit : 1;
					uint16_t	channel_configuration : 3;
					uint16_t	original_copy : 1;
					uint16_t	home : 1;
					uint16_t	word_aligned_bits : 4;

					ADTSFixedHeader() {
						syncword = 0;
					}

					bool EqualTo(const ADTSFixedHeader& adts_fixed_header)
					{
						return (adts_fixed_header.ID == ID &&
								adts_fixed_header.layer == layer &&
								//adts_fixed_header.protection_absent == protection_absent &&
								adts_fixed_header.profile_ObjectType == profile_ObjectType &&
								adts_fixed_header.sampling_frequency_index == sampling_frequency_index &&
								adts_fixed_header.channel_configuration == channel_configuration &&
								adts_fixed_header.original_copy == original_copy &&
								adts_fixed_header.home == home) ? true : false;
					}

					bool operator==(const ADTSFixedHeader& adts_fixed_header){
						return EqualTo(adts_fixed_header);
					}

					bool operator!=(const ADTSFixedHeader& adts_fixed_header){
						return !EqualTo(adts_fixed_header);
					}

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, syncword, 12);
							bsrb1(in_bst, ID, 1);
							bsrb1(in_bst, layer, 2);
							bsrb1(in_bst, protection_absent, 1);

							bsrb1(in_bst, profile_ObjectType, 2);
							bsrb1(in_bst, sampling_frequency_index, 4);
							bsrb1(in_bst, private_bit, 1);
							bsrb1(in_bst, channel_configuration, 3);
							bsrb1(in_bst, original_copy, 1);
							bsrb1(in_bst, home, 1);
							MAP_BST_END();
						}
						catch (AMException e)
						{
							return e.RetCode();
						}

						SYNTAX_BITSTREAM_MAP::EndMap(in_bst);
						return RET_CODE_SUCCESS;
					}

					int Unmap(AMBst out_bst)
					{
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
						BST_FIELD_PROP_2NUMBER1(syncword, 12, "Should be 0xFFF");
						BST_FIELD_PROP_BOOL(ID, "The audio data in the ADTS stream are MPEG-2 AAC", "the audio data in the ADTS stream are MPEG-4 AAC ");
						BST_FIELD_PROP_2NUMBER1(layer, 2, "Indicates which layer is used");
						BST_FIELD_PROP_BOOL(protection_absent, "error_check() data is NOT present", "error_check() data is present");
						BST_FIELD_PROP_2NUMBER1(profile_ObjectType, 2, ADTS_profile_ObjectType_names[ID][profile_ObjectType]);
						BST_FIELD_PROP_2NUMBER1(sampling_frequency_index, 4, std::get<1>(samplingFrequencyIndex_names[sampling_frequency_index]));
						BST_FIELD_PROP_NUMBER1(private_bit, 1, "This one is only informative");
						BST_FIELD_PROP_2NUMBER1(channel_configuration, 3, std::get<2>(channelConfiguration_names[channel_configuration]));
						BST_FIELD_PROP_BOOL(original_copy, "Audio is copyrighted", "Audio is not copyrighted");
						BST_FIELD_PROP_BOOL(home, "Original media", "Copy of original media");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct ADTSVariableHeader : public SYNTAX_BITSTREAM_MAP
				{
					union
					{
						struct
						{
							uint32_t	copyright_identification_bit : 1;
							uint32_t	copyright_identification_start : 1;
							uint32_t	aac_frame_length : 13;
							uint32_t	adts_buffer_fullness : 11;
							uint32_t	number_of_raw_data_blocks_in_frame : 2;
							uint32_t	dword_aligned_bits : 4;
						}PACKED;

						uint8_t		uBytes[4] = { 0 };
					}PACKED;

					int Map(AMBst in_bst)
					{
						SYNTAX_BITSTREAM_MAP::Map(in_bst);
						try
						{
							MAP_BST_BEGIN(0);
							bsrb1(in_bst, copyright_identification_bit, 1);
							bsrb1(in_bst, copyright_identification_start, 1);
							bsrb1(in_bst, aac_frame_length, 13);
							bsrb1(in_bst, adts_buffer_fullness, 11);
							bsrb1(in_bst, number_of_raw_data_blocks_in_frame, 2);
							MAP_BST_END();
						}
						catch (AMException e)
						{
							return e.RetCode();
						}

						SYNTAX_BITSTREAM_MAP::EndMap(in_bst);
						return RET_CODE_SUCCESS;
					}

					int Unmap(AMBst out_bst)
					{
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_NUMBER1(copyright_identification_bit, 1, "One the bits of the 72-bit copyright identification field");
					BST_FIELD_PROP_BOOL(copyright_identification_start, "start of copyright identification in this audio frame", "no start of copyright identification in this audio frame");
					BST_FIELD_PROP_2NUMBER1(aac_frame_length, 13, "Length of the frame including headers and error_check in bytes");
					BST_FIELD_PROP_2NUMBER1(adts_buffer_fullness, 11, "");
					BST_FIELD_PROP_2NUMBER1(number_of_raw_data_blocks_in_frame, 2, "A field indicating how many raw data blocks are multiplexed (number_of_raw_data_blocks_in_frame+1)");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct ADTSErrorCheck
				{
					uint16_t	crc_check;
				}PACKED;

				struct ADTSHeaderErrorCheck
				{
					uint16_t	raw_data_block_position[4];
					uint16_t	crc_check;
				}PACKED;

				struct ADTSRawDataBlockErrorCheck
				{
					uint16_t	crc_check;
				}PACKED;

				ADTSFixedHeader		adts_fixed_header;
				ADTSVariableHeader	adts_variable_header;
				CAudioStreamContext audio_ctx;

				union
				{
					struct
					{
						ADTSErrorCheck*			adts_error_check;
						RAW_DATA_BLOCK*			raw_data_block;
					}PACKED;
					struct
					{
						ADTSHeaderErrorCheck*	adts_header_error_check;
						RAW_DATA_BLOCK*			raw_data_blocks[4];
						ADTSRawDataBlockErrorCheck*
												adts_raw_data_block_error_checks[4];
					}PACKED;
				}PACKED;

				ADTSFrame() {
					memset(&audio_ctx, 0, sizeof(audio_ctx));
					adts_header_error_check = NULL;
					memset(raw_data_blocks, 0, sizeof(raw_data_blocks));
					memset(adts_raw_data_block_error_checks, 0, sizeof(adts_raw_data_block_error_checks));
				}

				virtual ~ADTSFrame()
				{
					if (adts_variable_header.number_of_raw_data_blocks_in_frame == 0)
					{
						AMP_SAFEDEL2(adts_error_check);
						AMP_SAFEDEL2(raw_data_block);
					}
					else
					{
						AMP_SAFEDEL2(adts_header_error_check);
						for (uint32_t i = 0; i < adts_variable_header.number_of_raw_data_blocks_in_frame; i++)
						{
							AMP_SAFEDEL2(raw_data_blocks[i]);
							AMP_SAFEDEL2(adts_raw_data_block_error_checks[i]);
						}
					}
				}

				int Map(AMBst in_bst);

				int Unmap(AMBst out_bst)
				{
					return RET_CODE_ERROR_NOTIMPL;
				}

				DECLARE_FIELDPROP_BEGIN()
					NAV_WRITE_TAG_BEGIN2("adts_fixed_header");
						NAV_FIELD_PROP_OBJECT(adts_fixed_header);
					NAV_WRITE_TAG_END2("adts_fixed_header");
					NAV_WRITE_TAG_BEGIN2("adts_variable_header");
						NAV_FIELD_PROP_OBJECT(adts_variable_header);
					NAV_WRITE_TAG_END2("adts_variable_header");

					if (adts_variable_header.number_of_raw_data_blocks_in_frame == 0)
					{
						if (adts_fixed_header.protection_absent == 0)
						{
							NAV_WRITE_TAG_BEGIN2("adts_error_check");
							NAV_FIELD_PROP_2NUMBER("crc_check", 16, adts_error_check->crc_check, "");
							NAV_WRITE_TAG_END2("adts_error_check");
						}
						NAV_WRITE_TAG_BEGIN2("raw_data_block");
							NAV_FIELD_PROP_REF(raw_data_block);
						NAV_WRITE_TAG_END2("raw_data_block");
					}
					else
					{
						if (adts_fixed_header.protection_absent == 0)
						{
							NAV_WRITE_TAG_BEGIN2("adts_header_error_check");
							for (i = 1; i <= (int)adts_variable_header.number_of_raw_data_blocks_in_frame; i++)
							{
								NAV_ARRAY_FIELD_PROP_2NUMBER("raw_data_block_position", i, 16, adts_header_error_check->raw_data_block_position[i], "");
							}
							NAV_FIELD_PROP_2NUMBER("crc_check", 16, adts_header_error_check->crc_check, "");
							NAV_WRITE_TAG_END2("adts_header_error_check");
						}

						for (i = 0; i <= (int)adts_variable_header.number_of_raw_data_blocks_in_frame; i++)
						{
							NAV_WRITE_TAG_BEGIN3("raw_data_block", i);
							NAV_FIELD_PROP_REF(raw_data_blocks[i]);
							NAV_WRITE_TAG_END3("raw_data_block", i);

							if (adts_fixed_header.protection_absent == 0)
							{
								NAV_ARRAY_FIELD_PROP_2NUMBER("crc_check", i, 16, adts_raw_data_block_error_checks[i]->crc_check, "");
							}
						}
					}
				DECLARE_FIELDPROP_END()

			}PACKED;

			std::vector<ADTSFrame*>		adts_frames;

			virtual ~ADTSBitstream()
			{
				for (size_t i = 0; i<adts_frames.size(); i++)
				{
					AMP_SAFEDEL(adts_frames[i]);
				}
			}

			/*!	@brief It is used to process the continuous input data 
				@param pBuf the input buffer
				@param cbSize the input buffer size
				@param pcbLend indicate how many bytes of data are lend from the previous left data
				@param pcbLeft indicate how many bytes of data is not processed, it may be greater than cbSize because it may lend data from previous process
			*/
			int Process(unsigned char* pBuf, unsigned long cbSize, int* pcbLend = NULL, int* pcbLeft = NULL);

			/*!	@brief It is used to process the current input data, and ignore the previous data. */
			int Map(unsigned char* pBuf, unsigned long cbSize, bool bOptOnlyMapCompleteFrame=false, int* pcbLeft = NULL);

			/*!	@brief It is used to process from the current position of bitstream. */
			int Map(AMBst in_bst);

			int Unmap(AMBst out_bst)
			{
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN_1("AudioBitstream", "AAC Audio Bitstream", 1);
			for (size_t idx = 0; idx < adts_frames.size(); idx++)
			{
				NAV_FIELD_PROP_REF(adts_frames[idx]);
			}
			NAV_WRITE_TAG_END("AudioBitstream");
			DECLARE_FIELDPROP_END()

			AMLinearRingBuffer	ring_buffer_not_processed = nullptr;

		};

		struct CStreamMuxConfig : public SYNTAX_BITSTREAM_MAP
		{
			uint32_t			audioMuxVersion : 1;
			uint32_t			audioMuxVersionA : 1;
			uint32_t			taraBufferFullness : 24;
			uint32_t			numSubFrames : 6;

			uint8_t				allStreamsSameTimeFraming : 1;
			uint8_t				numProgram : 4;
			uint8_t				otherDataPresent : 1;
			uint8_t				crcCheckPresent : 1;
			uint8_t				reserved_0 : 1;

			uint8_t				numLayer[16] = { 0 };
			uint8_t				progSIndx[16 * 8] = { 0 };
			uint8_t				laySIndx[16 * 8] = { 0 };
			int8_t				streamID[16][8] = { {0} };
			bool				useSameConfig[16][8] = { {0} };

			uint32_t			ascLen[16][8] = { {0} };
			std::shared_ptr<CAudioSpecificConfig>
								AudioSpecificConfig[16][8];
			AMBitArray			fillBits[16][8] = { {nullptr} };
			uint8_t				frameLengthType[128] = { 0 };
			union
			{
				struct {
					uint8_t				latmBufferFullness[128];
					uint8_t				coreFrameOffset[16][8];
				};
				uint16_t			frameLength[128] = { 0 };
				uint8_t				CELPframeLengthTableIndex[128];
				uint8_t				HVXCframeLengthTableIndex[128];
			};

			uint32_t			otherDataLenBits = 0;
			uint8_t				crcCheckSum = 0;

			CStreamMuxConfig() {
				for (uint8_t prog = 0; prog < 16; prog++)
				{
					for (uint8_t lay = 0; lay < 8; lay++)
					{
						streamID[prog][lay] = (int8_t)-1;
					}
				}
			}

			~CStreamMuxConfig() {
				for (uint8_t prog = 0; prog < 16; prog++)
				{
					for (uint8_t lay = 0; lay < 8; lay++)
					{
						AudioSpecificConfig[prog][lay] = nullptr;
						if (fillBits[prog][lay])
						{
							AM_DestoryBitArray(fillBits[prog][lay]);
							fillBits[prog][lay] = NULL;
						}
					}
				}
			}

			std::shared_ptr<CAudioSpecificConfig> GetAudioSpecificConfig(int8_t StmID)
			{
				if (StmID < 0)
					return nullptr;

				for (uint8_t prog = 0; prog < 16; prog++)
				{
					for (uint8_t lay = 0; lay < 8; lay++)
					{
						if (streamID[prog][lay] == StmID)
							return AudioSpecificConfig[prog][lay];
					}
				}

				return nullptr;
			}

			RET_CODE GetProgAndLayByStreamID(int8_t StmID, uint8_t& ret_prog, uint8_t& ret_lay)
			{
				if (StmID < 0)
					return RET_CODE_INVALID_PARAMETER;

				for (uint8_t prog = 0; prog < 16; prog++)
				{
					for (uint8_t lay = 0; lay < 8; lay++)
					{
						if (streamID[prog][lay] == StmID)
						{
							ret_prog = prog;
							ret_lay = lay;
							return RET_CODE_SUCCESS;
						}
					}
				}

				return RET_CODE_OUT_OF_RANGE;
			}

			int Map(AMBst in_bst) {
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);

					bsrb1(in_bst, audioMuxVersion, 1);
					if (audioMuxVersion == 1) {
						bsrb1(in_bst, audioMuxVersionA, 1);
					}
					else
						audioMuxVersionA = 0;

					if (audioMuxVersionA == 0)
					{
						if (audioMuxVersion == 1)
						{
							taraBufferFullness = AMBst_LatmGetValue(in_bst);
							map_status.number_of_fields++;
						}
						uint8_t	streamCnt = 0;
						bsrb1(in_bst, allStreamsSameTimeFraming, 1);
						bsrb1(in_bst, numSubFrames, 6);
						bsrb1(in_bst, numProgram, 4);

						for (uint8_t prog = 0; prog <= numProgram; prog++)
						{
							bsrb1(in_bst, numLayer[prog], 3);
							for (uint8_t lay = 0; lay <= numLayer[prog]; lay++)
							{
								progSIndx[streamCnt] = prog; laySIndx[streamCnt] = lay;
								streamID[prog][lay] = streamCnt++;

								if (prog == 0 && lay == 0) {
									useSameConfig[prog][lay] = 0;
								}
								else {
									bsrb1(in_bst, useSameConfig[prog][lay], 1);
								}

								if (!useSameConfig[prog][lay])
								{
									if (audioMuxVersion == 0)
									{
										CAudioSpecificConfig* pAudioSpecificConfig = nullptr;
										bsrbreadref(in_bst, pAudioSpecificConfig, CAudioSpecificConfig);
										AudioSpecificConfig[prog][lay] = std::shared_ptr<CAudioSpecificConfig>(pAudioSpecificConfig);
									}
									else
									{
										ascLen[prog][lay] = AMBst_LatmGetValue(in_bst);
										map_status.number_of_fields++;
										int bit_pos = AMBst_Tell(in_bst);
										CAudioSpecificConfig* pAudioSpecificConfig = nullptr;
										bsrbreadref(in_bst, pAudioSpecificConfig, CAudioSpecificConfig);
										AudioSpecificConfig[prog][lay] = std::shared_ptr<CAudioSpecificConfig>(pAudioSpecificConfig);
										int left_bits = ascLen[prog][lay] - AMBst_Tell(in_bst) - bit_pos;

										fillBits[prog][lay] = AM_CreateBitArray(left_bits);
										map_status.number_of_fields++;
									}
								}

								bsrb1(in_bst, frameLengthType[streamID[prog][lay]], 3);
								if (frameLengthType[streamID[prog][lay]] == 0)
								{
									bsrb1(in_bst, latmBufferFullness[streamID[prog][lay]], 8);
									if (!allStreamsSameTimeFraming) {

										if (AudioSpecificConfig[prog][lay] == NULL)
											throw AMException(RET_CODE_ERROR_NOTIMPL, _T("No AudioSpecificConfig is available"));

										if (lay == 0)
											throw AMException(RET_CODE_BUFFER_NOT_COMPATIBLE, _T("Buffer is incompatible"));

										int AudioObjectType = AudioSpecificConfig[prog][lay]->GetAudioObjectType();
										int prevAudioObjectType = AudioSpecificConfig[prog][lay - 1]->GetAudioObjectType();
										if ((AudioObjectType == 6 || AudioObjectType == 20) &&
											(prevAudioObjectType == 8 || prevAudioObjectType == 24)) {
											bsrb1(in_bst, coreFrameOffset[prog][lay], 6);
										}
									}
								}
								else if (frameLengthType[streamID[prog][lay]] == 1)
								{
									bsrb1(in_bst, frameLength[streamID[prog][lay]], 9);
								}
								else if (frameLengthType[streamID[prog][lay]] == 4 ||
									frameLengthType[streamID[prog][lay]] == 5 ||
									frameLengthType[streamID[prog][lay]] == 3) {
									bsrb1(in_bst, CELPframeLengthTableIndex[streamID[prog][lay]], 6);
								}
								else if (frameLengthType[streamID[prog][lay]] == 6 ||
									frameLengthType[streamID[prog][lay]] == 7)
								{
									bsrb1(in_bst, HVXCframeLengthTableIndex[streamID[prog][lay]], 1);
								}
							}
						}

						bsrb1(in_bst, otherDataPresent, 1);
						if (otherDataPresent)
						{
							if (audioMuxVersion == 1)
							{
								otherDataLenBits = AMBst_LatmGetValue(in_bst);
							}
							else
							{
								otherDataLenBits = 0; /* helper variable 32bit */
								uint8_t otherDataLenEsc = 0;
								do {
									bsrb1(in_bst, otherDataLenEsc, 1);
									otherDataLenBits = (otherDataLenBits << 8) | AMBst_GetByte(in_bst);
								} while (otherDataLenEsc);
							}
							map_status.number_of_fields++;
						}

						bsrb1(in_bst, crcCheckPresent, 1);
						if (crcCheckPresent)
							bsrb1(in_bst, crcCheckSum, 8);
					}
					else
					{
						/* tbd */
					}

					MAP_BST_END();
				}
				catch (AMException& e)
				{
					return e.RetCode();
				}
				catch (std::out_of_range&)
				{
					return RET_CODE_NO_MORE_DATA;
				}

				SYNTAX_BITSTREAM_MAP::EndMap(in_bst);

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL) {
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				BST_FIELD_PROP_BOOL(audioMuxVersion, "", "");
				if (audioMuxVersion) {
					BST_FIELD_PROP_BOOL(audioMuxVersionA, "reserved for future extensions", "");
				}
				else
				{
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE1(audioMuxVersionA, audioMuxVersionA ? "reserved for future extensions" : "default");
				}
				if (audioMuxVersion == 0) {
					if (audioMuxVersionA == 1) {
						BST_FIELD_PROP_2NUMBER1(taraBufferFullness, LatmValue_num_of_bits(taraBufferFullness), 
							"A helper variable indicating the state of the bit reservoir in the course of encoding the LATM status information");
					}

					uint8_t	streamCnt = 0;
					NAV_WRITE_TAG_WITH_1NUMBER_VALUE("streamCnt", 0, "The count of streams present in this structure");
					BST_FIELD_PROP_NUMBER1(allStreamsSameTimeFraming, 1, "A data element indicating whether all payloads, which are multiplexed in PayloadMux(), share a common time base");
					BST_FIELD_PROP_2NUMBER1_F(numSubFrames, 6, "%d PayloadMux() frames are multiplexed", numSubFrames + 1);
					BST_FIELD_PROP_2NUMBER1_F(numProgram, 4, "%d programs are multiplexed", numProgram + 1);
					NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("Tag", "for (prog = 0; prog &lt;= numProgram:%d; prog++)", "", numProgram);
					for (uint16_t prog = 0; prog <= numProgram; prog++) {
						BST_ARRAY_FIELD_PROP_NUMBER1_F(numLayer, prog, 3, "%d scalable layers are multiplexed", numLayer[prog] + 1);
						NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("Tag", "for (lay = 0; lay &lt;= numLayer:%d; lay++)", "", numLayer[prog]);
						for (uint8_t lay = 0; lay <= numLayer[prog]; lay++) {
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("tag", "progSIndx[streamCnt:%d]", progSIndx[streamCnt], "", streamCnt);
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("tag", "laySIndx[streamCnt:%d]", laySIndx[streamCnt], "", streamCnt);
							NAV_WRITE_TAG_WITH_ALIAS_AND_NUMBER_VALUE("tag", "streamID[prog:%d][lay:%d]=%d", streamID[prog][lay], "", prog, lay, streamCnt);
							streamCnt++;
							if (prog == 0 && lay == 0) {
								NAV_WRITE_TAG_WITH_1NUMBER_VALUE("useSameConfig", 0, "AudioSpecificConfig() is present");
							}
							else
							{
								BST_FIELD_PROP_2NUMBER("useSameConfig", 1, useSameConfig[prog][lay], useSameConfig[prog][lay]
									? "AudioSpecificConfig() is not present. AudioSpecificConfig() in the previous layer or program should be applied"
									: "AudioSpecificConfig() is present");
							}

							if (!useSameConfig[prog][lay])
							{
								if (audioMuxVersion == 0) {
									BST_FIELD_PROP_REF2_1(AudioSpecificConfig[prog][lay], "AudioSpecificConfig", "");
								}
								else
								{
									BST_FIELD_PROP_2NUMBER("ascLen", LatmValue_num_of_bits(ascLen[prog][lay]), ascLen[prog][lay], "");
									BST_FIELD_PROP_REF2_1(AudioSpecificConfig[prog][lay], "AudioSpecificConfig", "");

									const uint8_t* fillBits_buf = AM_GetBitArrayBuffer(fillBits[prog][lay]);
									unsigned int fillBits_len = AM_GetBitArraySize(fillBits[prog][lay]);
									BST_FIELD_PROP_FIXSIZE_BINSTR("fillBits", fillBits_len, fillBits_buf, (fillBits_len + 7) / 8, "");
								}
							}

							BST_FIELD_PROP_2NUMBER_ALIAS_F_("Tag", "frameLengthType[streamID[prog:%d][lay:%d]:%d]", 3, frameLengthType[streamID[prog][lay]], 
								frameLengthType[streamID[prog][lay]] == 0?"Payload with variable frame length":(
								frameLengthType[streamID[prog][lay]] == 1?"Payload with fixed frame length":(
								frameLengthType[streamID[prog][lay]] == 2?"Reserved":(
								frameLengthType[streamID[prog][lay]] == 3?"Payload for a CELP object with one of 2 kinds of frame length":(
								frameLengthType[streamID[prog][lay]] == 4?"Payload for a CELP or ER_CELP object with fixed frame length":(
								frameLengthType[streamID[prog][lay]] == 5?"Payload for an ER_CELP object with one of 4 kinds of frame length":(
								frameLengthType[streamID[prog][lay]] == 6?"Payload for a HVXC or ER_HVXC object with fixed frame length":(
								frameLengthType[streamID[prog][lay]] == 7?"Payload for an HVXC or ER_HVXC object with one of 4 kinds of frame length":""))))))), 
								prog, lay, streamID[prog][lay]);

							if (frameLengthType[streamID[prog][lay]] == 0)
							{
								BST_FIELD_PROP_2NUMBER_ALIAS_F_("Tag", "latmBufferFullness[streamID[prog:%d][lay:%d]:%d]", 8, latmBufferFullness[streamID[prog][lay]], 
									"the state of the bit reservoir in the course of encoding the first access unit of a particular program and layer in an AudioMuxElement()",
									prog, lay, streamID[prog][lay]);
								if (!allStreamsSameTimeFraming) {
									int AudioObjectType = AudioSpecificConfig[prog][lay]->GetAudioObjectType();
									int prevAudioObjectType = AudioSpecificConfig[prog][lay - 1]->GetAudioObjectType();
									if ((AudioObjectType == 6 || AudioObjectType == 20) &&
										(prevAudioObjectType == 8 || prevAudioObjectType == 24)) {
										BST_FIELD_PROP_2NUMBER("coreFrameOffset", 6, coreFrameOffset[prog][lay], "");
									}
								}
							}
							else if (frameLengthType[streamID[prog][lay]] == 1)
							{
								MBCSPRINTF_S(szTemp4, TEMP4_SIZE, "The payload length in bits is specified as %d", 8 * (frameLength[streamID[prog][lay]] + 20));
								BST_FIELD_PROP_2NUMBER_ALIAS_F_("Tag", "frameLength[streamID[prog:%d][lay:%d]:%d]", 9, frameLength[streamID[prog][lay]],
									szTemp4, prog, lay, streamID[prog][lay]);
							}
							else if (frameLengthType[streamID[prog][lay]] == 4 ||
								frameLengthType[streamID[prog][lay]] == 5 ||
								frameLengthType[streamID[prog][lay]] == 3) {
								BST_FIELD_PROP_2NUMBER_ALIAS_F_("Tag", "CELPframeLengthTableIndex[streamID[prog:%d][lay:%d]:%d]", 9, CELPframeLengthTableIndex[streamID[prog][lay]],
									"one of two indices for pointing out the frame length for a CELP or ER_CELP object. (Table 1.47 and Table 1.48)", 
									prog, lay, streamID[prog][lay]);
							}
							else if (frameLengthType[streamID[prog][lay]] == 6 ||
								frameLengthType[streamID[prog][lay]] == 7)
							{
								BST_FIELD_PROP_2NUMBER_ALIAS_F_("Tag", "HVXCframeLengthTableIndex[streamID[prog:%d][lay:%d]:%d]", 9, HVXCframeLengthTableIndex[streamID[prog][lay]],
									"one of two indices for pointing out the frame length for a HVXC or ER_HVXC object. (Table 1.46)",
									prog, lay, streamID[prog][lay]);
							}
						}
						NAV_WRITE_TAG_END("Tag");
					}
					NAV_WRITE_TAG_END("Tag");

					BST_FIELD_PROP_BOOL(otherDataPresent, "The other data than audio payload otherData is multiplexed", "The other data than audio payload otherData is not multiplexed");
					if (otherDataPresent)
					{
						if (audioMuxVersion == 1)
						{
							BST_FIELD_PROP_2NUMBER1(otherDataLenBits, LatmValue_num_of_bits(otherDataLenBits),
								"A helper variable indicating the length in bits of the other data");
						}
						else
						{
							BST_FIELD_PROP_2NUMBER1(otherDataLenBits, otherDataLenBits>0xFFFFFF?36:(otherDataLenBits>0xFFFF?27:(otherDataLenBits>0xFF?18:9)),
								"A helper variable indicating the length in bits of the other data");
						}
					}
					BST_FIELD_PROP_BOOL(crcCheckPresent, "CRC check bits are present", "CRC check bits are not present");
					if (crcCheckPresent)
					{
						BST_FIELD_PROP_2NUMBER1(crcCheckSum, 8, "CRC error detection data");
					}
				}
			DECLARE_FIELDPROP_END()
		};

		struct CPayloadLengthInfo : public SYNTAX_BITSTREAM_MAP
		{
			CStreamMuxConfig*		_StreamMuxConfig = NULL;

			struct {
				uint32_t			MuxSlotLengthCoded : 2;
				uint32_t			MuxSlotLengthBits : 30;
			}MuxSlotLength[128];

			uint8_t					numChunk;
			uint8_t					streamIndx[16] = { 0 };
			uint8_t					progCIndx[128] = { 0 };
			uint8_t					layCIndx[128] = { 0 };
			bool					AuEndFlag[128] = { 0 };

			CPayloadLengthInfo(CStreamMuxConfig* StreamMuxConfig) : _StreamMuxConfig(StreamMuxConfig) {
			}

			int Map(AMBst in_bst)
			{
				if (_StreamMuxConfig == NULL)
					return RET_CODE_ERROR;

				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					if (_StreamMuxConfig->allStreamsSameTimeFraming)
					{
						for (uint8_t prog = 0; prog <= _StreamMuxConfig->numProgram; prog++){
							for (uint8_t lay = 0; lay <= _StreamMuxConfig->numLayer[prog]; lay++) {
								if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 0)
								{
									MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits = 0;
									uint8_t tmp = 0;
									do {
										bsrb1(in_bst, tmp, 8);
										MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits += tmp;
									} while (tmp == 255);
									MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits *= 8;
								}
								else
								{
									if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 5 ||
										_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 7 ||
										_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 3)
									{
										bsrb1(in_bst, MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded, 2);
										if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 7)
										{
											uint8_t frameLen[2][4] = { {40, 28, 2, 0}, {80, 40, 25, 3} };
											MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
												frameLen[_StreamMuxConfig->HVXCframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
										}
										else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 5)
										{
											if (lay == 0)
												MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
												CELP_Layer0_frameLen5[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
											else
												MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
												CELP_Layer1_5_frameLen5[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
										}
										else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 3)
										{
											if (lay == 0)
												MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
												CELP_Layer0_frameLen3[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
										}
									}
									else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 1)
										MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
										8 * (20 + _StreamMuxConfig->frameLength[_StreamMuxConfig->streamID[prog][lay]]);
									else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 6)
									{
										MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											_StreamMuxConfig->HVXCframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]] ? 80 : 40;
									}
									else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 4)
									{
										if (lay == 0)
											MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											CELP_Layer0_frameLen4[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]];
										else
											MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											CELP_Layer1_5_frameLen4[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]];
									}
								}
							}
						}
					}
					else
					{
						bsrb1(in_bst, numChunk, 4);
						for (uint8_t chunkCnt = 0; chunkCnt <= numChunk; chunkCnt++) {
							bsrb1(in_bst, streamIndx[chunkCnt], 4);

							uint8_t prog = progCIndx[chunkCnt] = _StreamMuxConfig->progSIndx[streamIndx[chunkCnt]];
							uint8_t lay = layCIndx[chunkCnt] = _StreamMuxConfig->laySIndx[streamIndx[chunkCnt]];
							if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 0)
							{
								MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits = 0;
								uint8_t tmp = 0;
								do {
									bsrb1(in_bst, tmp, 8);
									MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits += tmp;
								} while (tmp == 255);
								bsrb1(in_bst, AuEndFlag[_StreamMuxConfig->streamID[prog][lay]], 1);
							}
							else
							{
								if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 5 ||
									_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 7 ||
									_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 3)
								{
									bsrb1(in_bst, MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded, 2);
									if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 7)
									{
										uint8_t frameLen[2][4] = { {40, 28, 2, 0}, {80, 40, 25, 3} };
										MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											frameLen[_StreamMuxConfig->HVXCframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
									}
									else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 5)
									{
										if (lay == 0)
											MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											CELP_Layer0_frameLen5[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
										else
											MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											CELP_Layer1_5_frameLen5[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
									}
									else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 3)
									{
										if (lay == 0)
											MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
											CELP_Layer0_frameLen3[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]][MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthCoded];
									}
								}
								else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 1)
									MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
									8 * (20 + _StreamMuxConfig->frameLength[_StreamMuxConfig->streamID[prog][lay]]);
								else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 6)
								{
									MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
										_StreamMuxConfig->HVXCframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]] ? 80 : 40;
								}
								else if (_StreamMuxConfig->frameLengthType[_StreamMuxConfig->streamID[prog][lay]] == 4)
								{
									if (lay == 0)
										MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
										CELP_Layer0_frameLen4[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]];
									else
										MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits =
										CELP_Layer1_5_frameLen4[_StreamMuxConfig->CELPframeLengthTableIndex[_StreamMuxConfig->streamID[prog][lay]]];
								}
							}
						}
					}
					MAP_BST_END();
				}
				catch (...)
				{
				}
				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()

			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CPayloadMux : public SYNTAX_BITSTREAM_MAP
		{
			CPayloadLengthInfo*			_PayloadLengthInfo = NULL;
			CStreamMuxConfig*			_StreamMuxConfig = NULL;

			CPayloadMux(CStreamMuxConfig* StreamMuxConfig, CPayloadLengthInfo* PayloadLengthInfo)
				: _PayloadLengthInfo(PayloadLengthInfo)
				, _StreamMuxConfig(StreamMuxConfig) {
			}

			int Map(AMBst in_bst)
			{
				if (_StreamMuxConfig == NULL)
					return RET_CODE_ERROR;

				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					if (_StreamMuxConfig->allStreamsSameTimeFraming)
					{
						for (uint8_t prog = 0; prog <= _StreamMuxConfig->numProgram; prog++) {
							for (uint8_t lay = 0; lay <= _StreamMuxConfig->numLayer[prog]; lay++) {
								AMBst_SkipBits(in_bst, _PayloadLengthInfo->MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits);
								map_status.number_of_fields++;
							}
						}
					}
					else
					{
						for (uint8_t chunkCnt = 0; chunkCnt <= _PayloadLengthInfo->numChunk; chunkCnt++) {
							uint8_t prog = _PayloadLengthInfo->progCIndx[chunkCnt];
							uint8_t lay = _PayloadLengthInfo->layCIndx[chunkCnt];
							AMBst_SkipBits(in_bst, _PayloadLengthInfo->MuxSlotLength[_StreamMuxConfig->streamID[prog][lay]].MuxSlotLengthBits);
							map_status.number_of_fields++;
						}
					}

					MAP_BST_END();
				}
				catch (AMException& e)
				{
					return e.RetCode();
				}
				catch (std::out_of_range&)
				{
					return RET_CODE_NO_MORE_DATA;
				}
				catch (...)
				{
					return RET_CODE_ERROR;
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()

			DECLARE_FIELDPROP_END()
		};

		struct CAudioMuxElement : public SYNTAX_BITSTREAM_MAP
		{
			bool					_muxConfigPresent;

			uint8_t					useSameStreamMux : 1;
			uint8_t					reserved_0 : 7;

			CStreamMuxConfig*		StreamMuxConfig = NULL;
			CPayloadLengthInfo*		PayloadLengthInfo[64] = { 0 };
			CPayloadMux*			PayloadMux[64] = { 0 };

			AMBitArray				otherDataBits = NULL;

			IMP4AACContext*			ptr_context_MP4AAC;

			CAudioMuxElement(bool muxConfigPresent)
				: _muxConfigPresent(muxConfigPresent)
				, ptr_context_MP4AAC(nullptr){
			}

			~CAudioMuxElement()
			{
				if (otherDataBits != NULL) {
					AM_DestoryBitArray(otherDataBits);
					otherDataBits = NULL;
				}

				if (StreamMuxConfig->audioMuxVersionA == 0)
				{
					for (uint8_t i = 0; i < StreamMuxConfig->numSubFrames; i++)
					{
						UNMAP_STRUCT_POINTER5(PayloadMux[i]);
						UNMAP_STRUCT_POINTER5(PayloadLengthInfo[i]);
					}
				}

				if (!useSameStreamMux && StreamMuxConfig != NULL) {
					UNMAP_STRUCT_POINTER5(StreamMuxConfig);
				}
			}

			void UpdateCtx(IMP4AACContext* ctx)
			{
				ptr_context_MP4AAC = ctx;
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					if (_muxConfigPresent)
					{
						bsrb1(in_bst, useSameStreamMux, 1);
						if (!useSameStreamMux) {
							bsrbreadref(in_bst, StreamMuxConfig, CStreamMuxConfig);
						}
					}

					if (StreamMuxConfig == NULL)
						return RET_CODE_ERROR_NOTIMPL;

					if (StreamMuxConfig->audioMuxVersionA == 0)
					{
						for (uint8_t i = 0; i < StreamMuxConfig->numSubFrames; i++)
						{
							MAP_MEM_TO_STRUCT_POINTER5_1(in_bst, 1, PayloadLengthInfo[i], CPayloadLengthInfo, StreamMuxConfig);
							MAP_MEM_TO_STRUCT_POINTER5_1(in_bst, 1, PayloadMux[i], CPayloadMux, StreamMuxConfig, PayloadLengthInfo[i]);
						}
					}

					if (StreamMuxConfig->otherDataPresent && StreamMuxConfig->otherDataLenBits > 0)
					{
						otherDataBits = AM_CreateBitArray(StreamMuxConfig->otherDataLenBits);
						for (uint32_t i = 0; i < StreamMuxConfig->otherDataLenBits; i++) {
							bsrbarray1(in_bst, otherDataBits, i);
						}
					}

					AMBst_Realign(in_bst);

					MAP_BST_END();
				}
				catch (AMException& e)
				{
					return e.RetCode();
				}
				catch (std::out_of_range&)
				{
					return RET_CODE_NO_MORE_DATA;
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			if (_muxConfigPresent)
			{
				NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("Tag", "if (muxConfigPresent)", "");
					BST_FIELD_PROP_NUMBER1(useSameStreamMux, 1, useSameStreamMux 
						? "The multiplexing configuration is not present. The previous configuration should be applied" 
						: "The multiplexing configuration is present");

					if (!useSameStreamMux)
					{
						NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("Tag", "if (!useSameStreamMux)", "");
						NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("Tag", "StreamMuxConfig()", "");
								BST_FIELD_PROP_REF(StreamMuxConfig);
							NAV_WRITE_TAG_END("Tag");
						NAV_WRITE_TAG_END("Tag");
					}
				NAV_WRITE_TAG_END("Tag");
			}

			if (StreamMuxConfig->audioMuxVersionA == 0)
			{
				if (StreamMuxConfig->numSubFrames > 0)
				{
					NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("Tag", "for(i=0;i&lt;=numSubFrames:%d;i++)", "", StreamMuxConfig->numSubFrames);
					for (uint8_t i = 0; i < StreamMuxConfig->numSubFrames; i++)
					{
						BST_FIELD_PROP_REF(PayloadLengthInfo[i]);
						BST_FIELD_PROP_REF(PayloadMux[i]);
					}
					NAV_WRITE_TAG_END("Tag");
				}
			}
			DECLARE_FIELDPROP_END()

		};

		struct CErrorProtectionSpecificConfig : public SYNTAX_BITSTREAM_MAP
		{
			uint8_t					number_of_predefined_set;
			uint8_t					interleave_type : 2;
			uint8_t					bit_stuffing : 3;
			uint8_t					number_of_concatenated_frame : 3;

			uint8_t					number_of_class[8];

		}PACKED;

		struct CAudioSyncStreamFrame : public SYNTAX_BITSTREAM_MAP
		{
			uint16_t				syncword = 0;
			uint16_t				audioMuxLengthBytes = 0;
			CAudioMuxElement*		AudioMuxElement = NULL;

			~CAudioSyncStreamFrame() {
				UNMAP_STRUCT_POINTER5(AudioMuxElement);
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					bsrb1(in_bst, syncword, 11);
					bsrb1(in_bst, audioMuxLengthBytes, 13);

					MAP_MEM_TO_STRUCT_POINTER5_1(in_bst, 1, AudioMuxElement, CAudioMuxElement, 1);

					MAP_BST_END();
				}
				catch (AMException& e)
				{
					return e.RetCode();
				}
				catch (std::out_of_range&)
				{
					return RET_CODE_NO_MORE_DATA;
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
				BST_FIELD_PROP_2NUMBER1(syncword, 11, "Should be 0x2B7");
				BST_FIELD_PROP_2NUMBER1(audioMuxLengthBytes, 13, "the byte length of the subsequent AudioMuxElement() with byte alignment");
				NAV_WRITE_AUTOEXPAND_TAG_BEGIN_WITH_ALIAS_F("AudioMuxElement", "AudioMuxElement(muxConfigPresent=1)", "");
				BST_FIELD_PROP_REF(AudioMuxElement);
				NAV_WRITE_TAG_END("AudioMuxElement");
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CEPAudioSyncStreamFrame : public SYNTAX_BITSTREAM_MAP
		{
			uint16_t				syncword;
			uint8_t					futureUse;
			uint16_t				audioMuxLengthBytes;
			uint8_t					frameCounter;
			uint32_t				headerParity;
			CAudioMuxElement*		AudioMuxElement = NULL;

			~CEPAudioSyncStreamFrame() {
				UNMAP_STRUCT_POINTER5(AudioMuxElement);
			}

			int Map(AMBst in_bst)
			{
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				try
				{
					MAP_BST_BEGIN(0);
					bsrb1(in_bst, syncword, 11);
					bsrb1(in_bst, audioMuxLengthBytes, 13);

					MAP_MEM_TO_STRUCT_POINTER5_1(in_bst, 1, AudioMuxElement, CAudioMuxElement, 1);

					MAP_BST_END();
				}
				catch (AMException& e)
				{
					return e.RetCode();
				}
				catch (std::out_of_range&)
				{
					return RET_CODE_NO_MORE_DATA;
				}

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}
		}PACKED;

		struct CLOASAudioStream : public SYNTAX_BITSTREAM_MAP
		{
			CAudioSyncStreamFrame*		sync_stream_frame = NULL;

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_ERROR;
				SYNTAX_BITSTREAM_MAP::Map(in_bst);
				MAP_BST_BEGIN(0);
				try
				{
					
					bool bContinue = false;
					do
					{
						while (AMBst_PeekBits(in_bst, 11) != 0x2B7)
							AMBst_SkipBits(in_bst, 8);

						// Check whether it is a real compatible LOAS/LATM header
						// TODO...
						bContinue = false;
					} while (bContinue);
				}
				catch (...)
				{
					return RET_CODE_NEEDMOREINPUT;
				}

				try
				{
					AMP_NEWT1(ptr_sync_stream_frame, CAudioSyncStreamFrame);
					if (ptr_sync_stream_frame == nullptr)
						throw AMException(RET_CODE_OUTOFMEMORY);

					if (AMP_FAILED(iRet = ptr_sync_stream_frame->Map(in_bst)))
					{
						if (ptr_sync_stream_frame->map_status.number_of_fields == 0)
						{
							if (iRet == RET_CODE_NO_MORE_DATA)
								iRet = RET_CODE_SUCCESS;

							AMP_SAFEDEL2(ptr_sync_stream_frame);
						}
						else
							printf("[AAC] Failed to map MPEG4-AAC Audio frame {retcode: %d}.\n", iRet);
					}
					else
						sync_stream_frame = ptr_sync_stream_frame;
				}
				catch (AMException& e)
				{
					return e.RetCode();
				}
				catch (std::out_of_range&)
				{
					return RET_CODE_NO_MORE_DATA;
				}

				MAP_BST_END();

				return RET_CODE_SUCCESS;
			}

			int Unmap(AMBst out_bst = NULL)
			{
				UNREFERENCED_PARAMETER(out_bst);
				return RET_CODE_ERROR_NOTIMPL;
			}

			DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN_1("AudioBitstream", "AAC Audio Bitstream", 1);
				NAV_FIELD_PROP_REF(sync_stream_frame);
			NAV_WRITE_TAG_END("AudioBitstream");
			DECLARE_FIELDPROP_END()

		}PACKED;

		//
		// For ADTS or LOAS enumerator
		//
		using MP4AMuxStreamConfig = std::shared_ptr<BST::AACAudio::CStreamMuxConfig>;
		using MP4AMuxElement = std::shared_ptr<BST::AACAudio::CAudioMuxElement>;
		class IMP4AACContext : public IUnknown
		{
		public:
			virtual MP4AMuxStreamConfig			
									GetMuxStreamConfig()=0;
			virtual RET_CODE		UpdateMuxStreamConfig(MP4AMuxStreamConfig mux_stream_config)=0;
			virtual MP4AMuxElement	CreateAudioMuxElement(bool muxConfigPresent)=0;
			virtual void			Reset() = 0;

		public:
			IMP4AACContext() {}
			virtual ~IMP4AACContext() {}
		};

		class IMP2AACContext : public IUnknown
		{
		public:
			virtual void			Reset() = 0;

		public:
			IMP2AACContext() {}
			virtual ~IMP2AACContext() {}
		};

		class CLOASParser : public CComUnknown, public IMSEParser
		{
		public:
			CLOASParser(RET_CODE* pRetCode = nullptr);
			virtual ~CLOASParser();

			DECLARE_IUNKNOWN
			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_IMSEParser)
					return GetCOMInterface((IMSEParser*)this, ppvObj);

				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

		public:
			MEDIA_SCHEME_TYPE		GetSchemeType() { return MEDIA_SCHEME_LOAS_LATM; }
			RET_CODE				SetEnumerator(IUnknown* pEnumerator, uint32_t options);
			RET_CODE				ProcessInput(uint8_t* pBuf, size_t cbBuf);
			RET_CODE				ProcessOutput(bool bDrain = false);
			RET_CODE				ProcessAU(uint8_t* pBuf, size_t cbBuf);
			RET_CODE				GetContext(IUnknown** ppCtx);
			RET_CODE				Reset();

		protected:
			IMP4AACContext*			m_pCtxMP4AAC = nullptr;
			ILOASEnumerator*		m_loas_enum = nullptr;
			uint32_t				m_loas_enum_options = UINT32_MAX;
			AMLinearRingBuffer		m_rbRawBuf = nullptr;
			bool					m_bSynced = false;

		private:
			bool					VerifyAudioMuxElement(bool muxConfigPresent, uint8_t* pAudioMuxElement, int cbAudioMuxElement);
		};

		extern RET_CODE CreateMP2AACContext(IMP2AACContext** ppMP2AACCtx);
		extern RET_CODE CreateMP4AACContext(IMP4AACContext** ppMP4AACCtx);

	}	// namespace AAC-Audio

}	// namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
