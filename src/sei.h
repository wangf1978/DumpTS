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
#ifndef _SEI_H_
#define _SEI_H_

#include "nal_com.h"
#include "dump_data_type.h"

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

// All SEI Payload type for H264/HEVC
#define SEI_PAYLOAD_BUFFERING_PERIOD								0
#define SEI_PAYLOAD_PIC_TIMING										1
#define SEI_PAYLOAD_PAN_SCAN_RECT									2
#define SEI_PAYLOAD_FILLER_PAYLOAD									3
#define SEI_PAYLOAD_USER_DATA_REGISTERED_ITU_T_T35					4
#define SEI_PAYLOAD_USER_DATA_UNREGISTERED							5
#define SEI_PAYLOAD_RECOVERY_POINT									6
#define SEI_PAYLOAD_DEC_REF_PIC_MARKING_REPETITION					7
#define SEI_PAYLOAD_SPARE_PIC										8
#define SEI_PAYLOAD_SCENE_INFO										9
#define SEI_PAYLOAD_SUB_SEQ_INFO									10
#define SEI_PAYLOAD_SUB_SEQ_LAYER_CHARACTERISTICS					11
#define SEI_PAYLOAD_SUB_SEQ_CHARACTERISTICS							12
#define SEI_PAYLOAD_FULL_FRAME_FREEZE								13
#define SEI_PAYLOAD_FULL_FRAME_FREEZE_RELEASE						14
#define SEI_PAYLOAD_PICTURE_SNAPSHOT								15
#define SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_START			16
#define SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_END				17
#define SEI_PAYLOAD_MOTION_CONSTRAINED_SLICE_GROUP_SET				18
#define SEI_PAYLOAD_FILM_GRAIN_CHARACTERISTICS						19
#define SEI_PAYLOAD_DEBLOCKING_FILTER_DISPLAY_PREFERENCE			20
#define SEI_PAYLOAD_STEREO_VIDEO_INFO								21
#define SEI_PAYLOAD_POST_FILTER_HINT								22
#define SEI_PAYLOAD_TONE_MAPPING_INFO								23
#define SEI_PAYLOAD_SCALABILITY_INFO								24
#define SEI_PAYLOAD_SUB_PIC_SCALABLE_LAYER							25
#define SEI_PAYLOAD_NON_REQUIRED_LAYER_REP							26
#define SEI_PAYLOAD_PRIORITY_LAYER_INFO								27
#define SEI_PAYLOAD_H264_LAYERS_NOT_PRESENT							28
#define SEI_PAYLOAD_LAYER_DEPENDENCY_CHANGE							29
#define SEI_PAYLOAD_H264_SCALABLE_NESTING							30
#define SEI_PAYLOAD_BASE_LAYER_TEMPORAL_HRD							31
#define SEI_PAYLOAD_QUALITY_LAYER_INTEGRITY_CHECK					32
#define SEI_PAYLOAD_REDUNDANT_PIC_PROPERTY							33
#define SEI_PAYLOAD_TL0_DEP_REP_INDEX								34
#define SEI_PAYLOAD_TL_SWITCHING_POINT								35
#define SEI_PAYLOAD_PARALLEL_DECODING_INFO							36
#define SEI_PAYLOAD_MVC_SCALABLE_NESTING							37
#define SEI_PAYLOAD_VIEW_SCALABILITY_INFO							38
#define SEI_PAYLOAD_H264_MULTIVIEW_SCENE_INFO						39
#define SEI_PAYLOAD_H264_MULTIVIEW_ACQUISITION_INFO					40
#define SEI_PAYLOAD_NON_REQUIRED_VIEW_COMPONENT						41
#define SEI_PAYLOAD_VIEW_DEPENDENCY_CHANGE							42
#define SEI_PAYLOAD_OPERATION_POINTS_NOT_PRESENT					43
#define SEI_PAYLOAD_BASE_VIEW_TEMPORAL_HRD							44
#define SEI_PAYLOAD_FRAME_PACKING_ARRANGEMENT						45
#define SEI_PAYLOAD_H264_MULTIVIEW_VIEW_POSITION					46
#define SEI_PAYLOAD_DISPLAY_ORIENTATION								47
#define SEI_PAYLOAD_MVCD_SCALABLE_NESTING							48
#define SEI_PAYLOAD_MVCD_VIEW_SCALABILITY_INFO						49
#define SEI_PAYLOAD_H264_DEPTH_REPRESENTATION_INFO					50
#define SEI_PAYLOAD_H264_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO	51
#define SEI_PAYLOAD_DEPTH_TIMING									52
#define SEI_PAYLOAD_DEPTH_SAMPLING_INFO								53
#define SEI_PAYLOAD_STRUCTURE_OF_PICTURES_INFO						128
#define SEI_PAYLOAD_ACTIVE_PARAMETER_SETS							129
#define SEI_PAYLOAD_DECODING_UNIT_INFO								130
#define SEI_PAYLOAD_TEMPORAL_SUB_LAYER_ZERO_INDEX					131
#define SEI_PAYLOAD_HEVC_SCALABLE_NESTING							133
#define SEI_PAYLOAD_REGION_REFRESH_INFO								134
#define SEI_PAYLOAD_NO_DISPLAY										135
#define SEI_PAYLOAD_TIME_CODE										136
#define SEI_PAYLOAD_MASTERING_DISPLAY_COLOUR_VOLUME					137
#define SEI_PAYLOAD_SEGMENTED_RECT_FRAME_PACKING_ARRANGEMENT		138
#define SEI_PAYLOAD_TEMPORAL_MOTION_CONSTRAINED_TILE_SETS			139
#define SEI_PAYLOAD_CHROMA_RESAMPLING_FILTER_HINT					140
#define SEI_PAYLOAD_KNEE_FUNCTION_INFO								141
#define SEI_PAYLOAD_COLOUR_REMAPPING_INFO							142
#define SEI_PAYLOAD_DEINTERLACED_FIELD_IDENTIFICATION				143
#define SEI_PAYLOAD_HEVC_LAYERS_NOT_PRESENT							160
#define SEI_PAYLOAD_INTER_LAYER_CONSTRAINED_TILE_SETS				161
#define SEI_PAYLOAD_BSP_NESTING										162
#define SEI_PAYLOAD_BSP_INITIAL_ARRIVAL_TIME						163
#define SEI_PAYLOAD_SUB_BITSTREAM_PROPERTY							164
#define SEI_PAYLOAD_ALPHA_CHANNEL_INFO								165
#define SEI_PAYLOAD_OVERLAY_INFO									166
#define SEI_PAYLOAD_TEMPORAL_MV_PREDICTION_CONSTRAINTS				167
#define SEI_PAYLOAD_FRAME_FIELD_INFO								168
#define SEI_PAYLOAD_HEVC_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO	176
#define SEI_PAYLOAD_HEVC_DEPTH_REPRESENTATION_INFO					177
#define SEI_PAYLOAD_HEVC_MULTIVIEW_SCENE_INFO						178
#define SEI_PAYLOAD_HEVC_MULTIVIEW_ACQUISITION_INFO					179
#define SEI_PAYLOAD_HEVC_MULTIVIEW_VIEW_POSITION					180
#define SEI_PAYLOAD_ALTERNATIVE_DEPTH_INFO							181

extern const char* sei_payload_type_names[256];
extern const char* sei_GOP_picture_structure_names[8];
extern const char* sei_GOP_picture_type_names[16];
extern const char* sei_HEVC_GOP_picture_structure_names[8];
extern const char* sei_HEVC_GOP_picture_type_names[16];
extern const char* frame_rate_names[16];
extern const char* scene_transition_type_names[7];
extern const char* film_grain_model_id_names[4];
extern const char* blending_mode_id_names[4];
extern const char* vui_colour_primaries_names[256];
extern const char* vui_transfer_characteristics_names[256];
extern const char* vui_matrix_coeffs_descs[256];
extern const char* pic_struct_names[16];

class INALContext;

namespace BST {

	struct SEI_RBSP : public SYNTAX_BITSTREAM_MAP
	{
		struct SEI_MESSAGE : public SYNTAX_BITSTREAM_MAP
		{
			struct SEI_PAYLOAD : public SYNTAX_BITSTREAM_MAP
			{
				struct RESERVED_SEI_MESSAGE : public SYNTAX_BITSTREAM_MAP
				{
					std::vector<uint8_t>		reserved_sei_message_payload_bytes;
					int							payload_size;

					RESERVED_SEI_MESSAGE(int payloadSize) : payload_size(payloadSize) {
						if (payloadSize > 0)
							reserved_sei_message_payload_bytes.reserve(payloadSize);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							uint8_t payload_byte;
							reserved_sei_message_payload_bytes.reserve(payload_size);
							for (int i = 0; i < payload_size; i++) {
								nal_read_b(in_bst, payload_byte, 8, uint8_t);
								reserved_sei_message_payload_bytes.push_back(payload_byte);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved_sei_message_payload_byte",
						(uint32_t)(8ULL * reserved_sei_message_payload_bytes.size()), 
						&reserved_sei_message_payload_bytes[0], 
						(uint32_t)reserved_sei_message_payload_bytes.size(), "");
					DECLARE_FIELDPROP_END()

				};

				struct BUFFERING_PERIOD : public SYNTAX_BITSTREAM_MAP
				{
					struct INITIAL_CPB_REMOVAL_INFO
					{
						uint32_t	initial_cpb_removal_delay;
						uint32_t	initial_cpb_removal_offset;
						uint32_t	initial_alt_cpb_removal_delay;
						uint32_t	initial_alt_cpb_removal_offset;
					}PACKED;

					uint8_t			bp_seq_parameter_set_id : 4;
					uint8_t			irap_cpb_params_present_flag : 1;
					uint8_t			concatenation_flag : 1;
					uint8_t			use_alt_cpb_params_flag : 1;
					uint8_t			reserved_0 : 1;

					uint32_t		cpb_delay_offset = 0;
					uint32_t		dpb_delay_offset = 0;
					uint32_t		au_cpb_removal_delay_delta_minus1 = 0;

					std::vector<INITIAL_CPB_REMOVAL_INFO>
									nal_initial_cpb_removal_info;
					std::vector<INITIAL_CPB_REMOVAL_INFO>
									vcl_initial_cpb_removal_info;

					std::vector<uint8_t>
									reserved_sei_message_payload_bytes;
					int				payload_size;

					INALContext*	ptr_NAL_Context;

					BUFFERING_PERIOD(int payloadSize, INALContext* pNALCtx);

					int Map(AMBst in_bst);

					int Unmap(AMBst out_bst)
					{
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					size_t ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint = false, long long* bit_offset = NULL);

				};

				/*
					- If CpbDpbDelaysPresentFlag is equal to 1 or pic_struct_present_flag is equal to 1, 
					  one picture timing SEI message shall be present in every access unit of the coded video sequence.
					- Otherwise (CpbDpbDelaysPresentFlag is equal to 0 and pic_struct_present_flag is equal to 0), 
					  no picture timing SEI messages shall be present in any access unit of the coded video sequence.		
				*/
				struct PIC_TIMING_H264 : public SYNTAX_BITSTREAM_MAP
				{
					struct CLOCK_TIMESTAMP
					{
						uint64_t		clock_timestamp_flag : 1;
						uint64_t		ct_type : 2;
						uint64_t		nuit_field_based_flag : 1;
						uint64_t		counting_type : 5;
						uint64_t		full_timestamp_flag : 1;
						uint64_t		discontinuity_flag : 1;
						uint64_t		cnt_dropped_flag : 1;
						uint64_t		reserved_0 : 4;

						uint64_t		n_frames : 8;
						uint64_t		seconds_flag : 1;
						uint64_t		seconds_value : 6;
						uint64_t		minutes_flag : 1;
						uint64_t		minutes_value : 6;
						uint64_t		hours_flag : 1;
						uint64_t		hours_value : 5;
						uint64_t		time_offset_length : 5;
						uint64_t		reserved_1 : 15;

						int64_t			time_offset;

						std::string GetTimeCode() {
							char szTimeCode[64] = { 0 };
							if (full_timestamp_flag){
								MBCSPRINTF_S(szTimeCode, 64, "%02dh:%02dm:%02ds.%d", (int)hours_value, (int)minutes_value, (int)seconds_value, (int)n_frames);
							} else {
								if (seconds_flag){
									if (minutes_flag){
										if (hours_flag){
											MBCSPRINTF_S(szTimeCode, 64, "%02dh:%02dm:%02ds.%df", (int)hours_value, (int)minutes_value, (int)seconds_value, (int)n_frames);
										} else {
											MBCSPRINTF_S(szTimeCode, 64, "%02dm:%02ds.%df", (int)minutes_value, (int)seconds_value, (int)n_frames);
										}
									} else {
										MBCSPRINTF_S(szTimeCode, 64, "%02ds.%df", (int)seconds_value, (int)n_frames);
									}
								} else {
									MBCSPRINTF_S(szTimeCode, 64, "%df", (int)n_frames);
								}
							}

							return szTimeCode;
						}
					}PACKED;

					uint32_t		cpb_removal_delay;
					uint32_t		dpb_output_delay;

					uint8_t			CpbDpbDelaysPresentFlag : 1;
					uint8_t			CpbDpbDelaysPresentIfNAL : 1;
					uint8_t			pic_struct_present_flag : 1;
					uint8_t			pic_struct : 5;
					uint8_t			reserved_1[3] = { 0 };

					CLOCK_TIMESTAMP	ClockTS[3];

					std::vector<uint8_t>
									reserved_sei_message_payload_bytes;
					int				payload_size;
					INALContext*	ptr_NAL_Context;

					uint8_t			cpb_removal_delay_length_minus1 = 23;
					uint8_t			dpb_output_delay_length_minus1 = 23;

					PIC_TIMING_H264(int payloadSize, INALContext* pNALCtx);

					int Map(AMBst in_bst);

					int Unmap(AMBst out_bst)
					{
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					if (CpbDpbDelaysPresentFlag)
					{
						BST_FIELD_PROP_2NUMBER1(cpb_removal_delay, (long long)cpb_removal_delay_length_minus1 + 1, 
							"specifies how many clock ticks to wait after removal from the CPB of the access unit associated with the most recent buffering period SEI message in a preceding access unit before removing from the buffer the access unit data associated with the picture timing SEI message");
						BST_FIELD_PROP_2NUMBER1(dpb_output_delay, (long long)dpb_output_delay_length_minus1 + 1,
							"specifies how many clock ticks to wait after removal of an access unit from the CPB before the decoded picture can be output from the DPB");
					}

					if (pic_struct_present_flag)
					{
						BST_FIELD_PROP_2NUMBER1(pic_struct, 4, pic_struct_names[pic_struct]);

						int NumClockTS = 0;
						if (pic_struct >= 0 && pic_struct <= 2)
							NumClockTS = 1;
						else if (pic_struct == 3 || pic_struct == 4 || pic_struct == 7)
							NumClockTS = 2;
						else if (pic_struct == 5 || pic_struct == 6 || pic_struct == 8)
							NumClockTS = 3;

						if (NumClockTS > 0)
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;NumClockTS;i++)", "");
							for (int i = 0; i < NumClockTS; i++)
							{
								NAV_WRITE_TAG_ARRAY_BEGIN0("ClockTS", i, "");
								BST_FIELD_PROP_BOOL1(ClockTS[i], clock_timestamp_flag, 
									"indicates that a number of clock timestamp syntax elements are present and follow immediately", 
									"indicates that the associated clock timestamp syntax elements are not present");
								if (ClockTS[i].clock_timestamp_flag)
								{
									BST_FIELD_PROP_2NUMBER("ct_type", 2, ClockTS[i].ct_type, 
										ClockTS[i].ct_type == 0 ? "progressive" : (
										ClockTS[i].ct_type == 1 ? "interlaced" : (
										ClockTS[i].ct_type == 2 ? "unknown" : "")));
									
									BST_FIELD_PROP_BOOL1(ClockTS[i], nuit_field_based_flag,
										"is used in calculating clockTimestamp", "is used in calculating clockTimestamp");
									BST_FIELD_PROP_2NUMBER("counting_type", 5, ClockTS[i].counting_type, "specifies the method of dropping values of the n_frames");
									BST_FIELD_PROP_BOOL1(ClockTS[i], full_timestamp_flag,
										"specifies that the n_frames syntax element is followed by seconds_value, minutes_value, and hours_value", 
										"specifies that the n_frames syntax element is followed by seconds_flag");
									BST_FIELD_PROP_BOOL1(ClockTS[i], discontinuity_flag,
										"indicates that the difference between the current value of clockTimestamp and the value of clockTimestamp computed from the previous clock timestamp in output order should not be interpreted as the time difference between the times of origin or capture of the associated frames or fields",
										"indicates that the difference between the current value of clockTimestamp and the value of clockTimestamp computed from the previous clock timestamp in output order can be interpreted as the time difference between the times of origin or capture of the associated frames or fields");
									BST_FIELD_PROP_BOOL1(ClockTS[i], cnt_dropped_flag,
										"specifies the skipping of one or more values of n_frames using the counting method specified by counting_type",
										"specifies the skipping of one or more values of n_frames using the counting method specified by counting_type");
									BST_FIELD_PROP_2NUMBER("n_frames", 5, (uint8_t)ClockTS[i].n_frames, "specifies the method of dropping values of the n_frames");

									if (ClockTS[i].full_timestamp_flag)
									{
										BST_FIELD_PROP_2NUMBER("seconds_value", 6, (uint8_t)ClockTS[i].seconds_value, "0..59");
										BST_FIELD_PROP_2NUMBER("minutes_value", 6, (uint8_t)ClockTS[i].minutes_value, "0..59");
										BST_FIELD_PROP_2NUMBER("hours_value",   5, (uint8_t)ClockTS[i].hours_value,   "0..23");
									}
									else
									{
										BST_FIELD_PROP_BOOL1(ClockTS[i], seconds_flag, "", "");
										if (ClockTS[i].seconds_flag)
										{
											BST_FIELD_PROP_2NUMBER("seconds_value", 6, (uint8_t)ClockTS[i].seconds_value, "0..59");
											BST_FIELD_PROP_BOOL1(ClockTS[i], minutes_flag, "", "");
											if (ClockTS[i].minutes_flag)
											{
												BST_FIELD_PROP_2NUMBER("minutes_value", 6, (uint8_t)ClockTS[i].minutes_value, "0..59");
												BST_FIELD_PROP_BOOL1(ClockTS[i], hours_flag, "", "");
												if (ClockTS[i].hours_flag)
												{
													BST_FIELD_PROP_2NUMBER("hours_value", 5, (uint8_t)ClockTS[i].hours_value, "0..23");
												}
											}
										}
									}

									if (ClockTS[i].time_offset_length > 0) {
										BST_FIELD_PROP_2LLNUMBER("time_offset", ClockTS[i].time_offset_length, ClockTS[i].time_offset, "");
									}

								}
								NAV_WRITE_TAG_END("ClockTS");
							}
							NAV_WRITE_TAG_END("Tag0");
						}
					}


					DECLARE_FIELDPROP_END()
				};

				struct PIC_TIMING_H265 : public SYNTAX_BITSTREAM_MAP
				{
					struct DECODE_UNIT
					{
						uint64_t	num_nalus_in_du_minus1;
						uint64_t	du_cpb_removal_delay_increment_minus1;
					};

					union
					{
						struct
						{
							uint32_t		pic_struct : 4;
							uint32_t		source_scan_type : 2;
							uint32_t		duplicate_flag : 1;
							uint32_t		du_common_cpb_removal_delay_flag : 1;
							uint32_t		frame_field_info_present_flag : 1;
							uint32_t		CpbDpbDelaysPresentFlag : 1;
							uint32_t		sub_pic_hrd_params_present_flag : 1;
							uint32_t		sub_pic_cpb_params_in_pic_timing_sei_flag : 1;
							uint32_t		reserved_0 : 20;
						};
						uint32_t			u32_Value_0;
					};

					uint64_t		au_cpb_removal_delay_minus1;
					uint64_t		pic_dpb_output_delay;
					uint64_t		pic_dpb_output_du_delay;
					uint64_t		num_decoding_units_minus1;
					uint64_t		du_common_cpb_removal_delay_increment_minus1;
					std::vector<DECODE_UNIT>
									dus;

					std::vector<uint8_t>
									reserved_sei_message_payload_bytes;
					int				payload_size;
					INALContext*	ptr_NAL_Context;

					uint8_t			du_cpb_removal_delay_increment_length_minus1 = 0;
					uint8_t			dpb_output_delay_du_length_minus1 = 0;
					uint8_t			initial_cpb_removal_delay_length_minus1 = 23;
					uint8_t			au_cpb_removal_delay_length_minus1 = 23;
					uint8_t			dpb_output_delay_length_minus1 = 23;

					PIC_TIMING_H265(int payloadSize, INALContext* pNALCtx);

					int Map(AMBst in_bst);

					int Unmap(AMBst out_bst)
					{
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
						if (frame_field_info_present_flag)
						{
							BST_FIELD_PROP_2NUMBER1(pic_struct, 4, pic_struct_names[pic_struct]);
							BST_FIELD_PROP_2NUMBER1(source_scan_type, 2, source_scan_type == 0 ? "Interlaced" : (
																		 source_scan_type == 1 ? "Progressive" : (
																		 source_scan_type == 2 ? "Unspecified" : "")));
							BST_FIELD_PROP_BOOL(duplicate_flag, "the current picture is indicated to be a duplicate of a previous picture in output order"
															  , "the current picture is not indicated to be a duplicate of a previous picture in output order");
						}

						if (CpbDpbDelaysPresentFlag)
						{
							BST_FIELD_PROP_2NUMBER1(au_cpb_removal_delay_minus1, (long long)au_cpb_removal_delay_length_minus1 + 1,
								"plus 1 is used to calculate the number of clock ticks between the nominal CPB removal times of the access unit associated with the picture timing SEI message and the preceding access unit in decoding order that contained a buffering period SEI message");
							BST_FIELD_PROP_2NUMBER1(pic_dpb_output_delay, (long long)dpb_output_delay_length_minus1 + 1,
								"how many clock ticks to wait after removal of the last decoding unit in an access unit from the CPB before the decoded picture is output from the DPB");
							if (sub_pic_hrd_params_present_flag){
								BST_FIELD_PROP_2NUMBER1(pic_dpb_output_du_delay, (long long)dpb_output_delay_du_length_minus1 + 1,
									"how many sub clock ticks to wait after removal of the last decoding unit in an access unit from the CPB before the decoded picture is output from the DPB");
							}
							if (sub_pic_hrd_params_present_flag && sub_pic_cpb_params_in_pic_timing_sei_flag)
							{
								BST_FIELD_PROP_UE((uint32_t)num_decoding_units_minus1, "plus 1 specifies the number of decoding units in the access unit the picture timing SEI message is associated with");
								BST_FIELD_PROP_BOOL(du_common_cpb_removal_delay_flag, "the syntax element du_common_cpb_removal_delay_increment_minus1 is present"
																					, "the syntax element du_common_cpb_removal_delay_increment_minus1 is not present");
								if (du_common_cpb_removal_delay_flag)
								{
									BST_FIELD_PROP_2NUMBER1(du_common_cpb_removal_delay_increment_minus1, (long long)du_cpb_removal_delay_increment_length_minus1 + 1,
										"plus 1 specifies the duration, in units of clock sub-ticks, between the nominal CPB removal times of any two consecutive decoding units in decoding order in the access unit associated with the picture timing SEI message");
								}

								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=num_decoding_units_minus1;i++)", "");
								for (i = 0; i <= (int)num_decoding_units_minus1; i++) {
									NAV_WRITE_TAG_ARRAY_BEGIN0("decoding_units", i, "");
									BST_FIELD_PROP_UE1((uint32_t)dus[i].num_nalus_in_du_minus1, "num_nalus_in_du_minus1", "plus 1 specifies the number of decoding units in the access unit the picture timing SEI message is associated with");
									if (!du_common_cpb_removal_delay_flag && i < (int)num_decoding_units_minus1)
									{
										BST_FIELD_PROP_2NUMBER("du_cpb_removal_delay_increment_minus1", (long long)du_cpb_removal_delay_increment_length_minus1 + 1, dus[i].du_cpb_removal_delay_increment_minus1, "");
									}
									NAV_WRITE_TAG_END("decoding_units");
								}
								NAV_WRITE_TAG_END("Tag0");
							}
						}
					DECLARE_FIELDPROP_END()
				};

				struct MASTERING_DISPLAY_COLOUR_VOLUME : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t		display_primaries_x[3] = { 0 };
					uint16_t		display_primaries_y[3] = { 0 };
					uint16_t		white_point_x = 0;
					uint16_t		white_point_y = 0;

					uint32_t		max_display_mastering_luminance = 0;
					uint32_t		min_display_mastering_luminance = 0;

					MASTERING_DISPLAY_COLOUR_VOLUME(int payloadSize) {
						UNREFERENCED_PARAMETER(payloadSize);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, display_primaries_x[0], 16, uint16_t);
							nal_read_u(in_bst, display_primaries_y[0], 16, uint16_t);
							nal_read_u(in_bst, display_primaries_x[1], 16, uint16_t);
							nal_read_u(in_bst, display_primaries_y[1], 16, uint16_t);
							nal_read_u(in_bst, display_primaries_x[2], 16, uint16_t);
							nal_read_u(in_bst, display_primaries_y[2], 16, uint16_t);

							nal_read_u(in_bst, white_point_x, 16, uint16_t);
							nal_read_u(in_bst, white_point_y, 16, uint16_t);

							nal_read_u(in_bst, max_display_mastering_luminance, 32, uint32_t);
							nal_read_u(in_bst, min_display_mastering_luminance, 32, uint32_t);

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
						NAV_FIELD_PROP_NUMBER_PRINTF("display_primaries_x0", 16, "the normalized x chromaticity coordinate, of GREEN colour primary component of the mastering display, according to the CIE 1931 definition", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							display_primaries_x[0], display_primaries_x[0], display_primaries_x[0] / 50000, (display_primaries_x[0] % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("display_primaries_y0", 16, "the normalized y chromaticity coordinate, of GREEN colour primary component of the mastering display, according to the CIE 1931 definition", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							display_primaries_y[0], display_primaries_y[0], display_primaries_y[0] / 50000, (display_primaries_y[0] % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("display_primaries_x1", 16, "the normalized x chromaticity coordinate, of BLUE colour primary component of the mastering display, according to the CIE 1931 definition", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							display_primaries_x[1], display_primaries_x[1], display_primaries_x[1] / 50000, (display_primaries_x[1] % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("display_primaries_y1", 16, "the normalized y chromaticity coordinate, of BLUE colour primary component of the mastering display, according to the CIE 1931 definition", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							display_primaries_y[1], display_primaries_y[1], display_primaries_y[1] / 50000, (display_primaries_y[1] % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("display_primaries_x2", 16, "the normalized x chromaticity coordinate, of RED colour primary component of the mastering display, according to the CIE 1931 definition", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							display_primaries_x[2], display_primaries_x[2], display_primaries_x[2] / 50000, (display_primaries_x[2] % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("display_primaries_y2", 16, "the normalized y chromaticity coordinate, of RED colour primary component of the mastering display, according to the CIE 1931 definition", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							display_primaries_y[2], display_primaries_y[2], display_primaries_y[2] / 50000, (display_primaries_y[2] % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("white_point_x", 16, "the normalized x chromaticity coordinates, of the white point", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							white_point_x, white_point_x, white_point_x / 50000, (white_point_x % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("white_point_y", 16, "the normalized y chromaticity coordinates, of the white point", "%-5" PRIu16 "(0X%-4" PRIX16 ") -- %d.%05d",
							white_point_y, white_point_y, white_point_y / 50000, (white_point_y % 50000) * 2)
						NAV_FIELD_PROP_NUMBER_PRINTF("max_display_mastering_luminance", 32, "the nominal maximum display luminance value of the mastering display in units of 0.0001 cd/m2.", "%-10" PRIu32 "(0X%-8" PRIX32 ") -- %d.%04d",
							max_display_mastering_luminance, max_display_mastering_luminance, max_display_mastering_luminance / 10000, max_display_mastering_luminance % 10000);
						NAV_FIELD_PROP_NUMBER_PRINTF("min_display_mastering_luminance", 32, "the nominal minimum display luminance value of the mastering display in units of 0.0001 cd/m2.", "%-10" PRIu32 "(0X%-8" PRIX32 ") -- %d.%04d",
							min_display_mastering_luminance, min_display_mastering_luminance, min_display_mastering_luminance / 10000, min_display_mastering_luminance % 10000)
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct ACTIVE_PARAMETER_SETS : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		active_video_parameter_set_id : 4;
					uint8_t		self_contained_cvs_flag : 1;
					uint8_t		no_parameter_set_update_flag : 1;
					uint8_t		reserved : 2;

					uint8_t		num_sps_ids_minus1 = 0;
					uint8_t		active_seq_parameter_set_id[16] = { 0 };
					std::vector<uint8_t>
								layer_sps_idx;
					
					SEI_PAYLOAD*
								ptr_sei_payload;

					ACTIVE_PARAMETER_SETS(SEI_PAYLOAD* pSEIPayload)
						: active_video_parameter_set_id(0), self_contained_cvs_flag(0), no_parameter_set_update_flag(0), reserved(0)
						, ptr_sei_payload(pSEIPayload){
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);

							nal_read_u(in_bst, active_video_parameter_set_id, 4, uint8_t);
							nal_read_u(in_bst, self_contained_cvs_flag, 1, uint8_t);
							nal_read_u(in_bst, no_parameter_set_update_flag, 1, uint8_t);

							nal_read_ue(in_bst, num_sps_ids_minus1, uint8_t);

							if (num_sps_ids_minus1 > 16)
								return RET_CODE_BUFFER_NOT_COMPATIBLE;

							for (int i = 0; i <= num_sps_ids_minus1; i++) {
								nal_read_ue(in_bst, active_seq_parameter_set_id[i], uint8_t);
							}

							int total_read_bits = AMBst_Tell(in_bst) - bit_pos;

							if (ptr_sei_payload->payload_size * 8 > total_read_bits) {
								AMBst_SkipBits(in_bst, ptr_sei_payload->payload_size*8 - total_read_bits);
							}

							if (ptr_sei_payload != nullptr &&
								ptr_sei_payload->ptr_sei_message != nullptr &&
								ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp != nullptr)
							{
								auto pCtxNAL = ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL;
								if (pCtxNAL != nullptr)
								{
									INALHEVCContext* pNALHEVCCtx = nullptr;
									if (SUCCEEDED(pCtxNAL->QueryInterface(IID_INALHEVCContext, (void**)&pNALHEVCCtx)))
									{
										assert(active_seq_parameter_set_id[0] >= 0 && active_seq_parameter_set_id[0] <= 15);
										pNALHEVCCtx->ActivateSPS((int8_t)active_seq_parameter_set_id[0]);
										AMP_SAFERELEASE(pNALHEVCCtx);
									}
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
						long long orig_bit_offset = bit_offset ? *bit_offset : 0;
						BST_FIELD_PROP_2NUMBER1(active_video_parameter_set_id, 4, "indicates and shall be equal to the value of the vps_video_parameter_set_id of the VPS that is referred to by the VCL NAL units of the access unit associated with the SEI message");
						BST_FIELD_PROP_BOOL(self_contained_cvs_flag, "indicates that each parameter set that is (directly or indirectly) referenced by any VCL NAL unit of the CVS that is not a VCL NAL unit of a RASL picture is present within the CVS at a position that precedes, in decoding order, any NAL unit that (directly or indirectly) references the parameter set.", 
							"indicates that this property may or may not apply");
						BST_FIELD_PROP_BOOL(no_parameter_set_update_flag, "indicates that there is no parameter set update in the CVS", "indicates that there may or may not be parameter set update in the CVS");

						BST_FIELD_PROP_UE(num_sps_ids_minus1, "plus 1 shall be less than or equal to the number of SPSs that are referred to by the VCL NAL units of the access unit associated with the active parameter sets SEI message");
						if (num_sps_ids_minus1 >= 0) {
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0; i&lt;=num_sps_ids_minus1;i++)", "");
							for (int i = 0; i <= num_sps_ids_minus1; i++) {
								BST_ARRAY_FIELD_PROP_UE(active_seq_parameter_set_id, i, "a value of the sps_seq_parameter_set_id of the SPS that may be referred to by any VCL NAL unit of the access unit associated with the SEI message");
							}
							NAV_WRITE_TAG_END("Tag0");
						}

						if (bit_offset)
							*bit_offset = orig_bit_offset + ptr_sei_payload->payload_size;
					DECLARE_FIELDPROP_END()

				};

				struct RECOVERY_POINT : public SYNTAX_BITSTREAM_MAP
				{
					short		recovery_poc_cnt = 0;
					uint8_t		exact_match_flag : 1;
					uint8_t		broken_link_flag : 1;
					uint8_t		reserved : 6;

					RECOVERY_POINT()
						: exact_match_flag(0), broken_link_flag(0), reserved(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_se(in_bst, recovery_poc_cnt, short);
							nal_read_u(in_bst, exact_match_flag, 1, uint8_t);
							nal_read_u(in_bst, broken_link_flag, 1, uint8_t);
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
						BST_FIELD_PROP_SE(recovery_poc_cnt, "the recovery point of decoded pictures in output order");
						BST_FIELD_PROP_BOOL(exact_match_flag, "the match will be exact", "the match may not be exact");
						BST_FIELD_PROP_BOOL(broken_link_flag, "pictures produced by starting the decoding process at the location of a previous IRAP access unit may contain undesirable visual artefacts to the extent that decoded pictures at and subsequent to the access unit associated with the recovery point SEI message in decoding order should not be displayed until the specified recovery point in output order", 
							"no indication is given regarding any potential presence of visual artifacts");
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct RECOVERY_POINT_H264 : public SYNTAX_BITSTREAM_MAP
				{
					uint16_t	recovery_frame_cnt = 0;
					uint8_t		exact_match_flag : 1;
					uint8_t		broken_link_flag : 1;
					uint8_t		changing_slice_group_idc : 2;
					uint8_t		reserved : 4;

					RECOVERY_POINT_H264()
						: exact_match_flag(0), broken_link_flag(0), changing_slice_group_idc(0), reserved(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, recovery_frame_cnt, uint16_t);
							nal_read_u(in_bst, exact_match_flag, 1, uint8_t);
							nal_read_u(in_bst, broken_link_flag, 1, uint8_t);
							nal_read_u(in_bst, changing_slice_group_idc, 2, uint8_t);
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
						BST_FIELD_PROP_UE(recovery_frame_cnt, "the recovery point of decoded pictures in output order");
						BST_FIELD_PROP_BOOL(exact_match_flag, "the match will be exact", "the match may not be exact");
						BST_FIELD_PROP_BOOL(broken_link_flag, "pictures produced by starting the decoding process at the location of a previous IRAP access unit may contain undesirable visual artefacts to the extent that decoded pictures at and subsequent to the access unit associated with the recovery point SEI message in decoding order should not be displayed until the specified recovery point in output order",
							"no indication is given regarding any potential presence of visual artifacts");
						BST_FIELD_PROP_2NUMBER1(changing_slice_group_idc, 2, 
							changing_slice_group_idc==0?"decoded pictures are correct or approximately correct in content at and subsequent to the recovery point in output order when all macroblocks of the primary coded pictures are decoded within the changing slice group period":(
							changing_slice_group_idc==1?"within the changing slice group period no sample values outside the decoded macroblocks covered by slice group 0 are used for inter prediction of any macroblock within slice group 0.":(
							changing_slice_group_idc==2?"within the changing slice group period no sample values outside the decoded macroblocks covered by slice group 1 are used for inter prediction of any macroblock within slice group 1.":"")));
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct PAN_SCAN_RECT : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		pan_scan_rect_id = 0;
					uint8_t			pan_scan_rect_cancel_flag : 1;
					uint8_t			pan_scan_cnt_minus1 : 6;
					uint8_t			pan_scan_rect_persistence_flag : 1;

					int32_t			pan_scan_rect_left_offset[3] = { 0 };
					int32_t			pan_scan_rect_right_offset[3] = { 0 };
					int32_t			pan_scan_rect_top_offset[3] = { 0 };
					int32_t			pan_scan_rect_bottom_offset[3] = { 0 };

					PAN_SCAN_RECT()
						: pan_scan_rect_cancel_flag(0), pan_scan_cnt_minus1(0), pan_scan_rect_persistence_flag(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, pan_scan_rect_id, uint32_t);
							nal_read_u(in_bst, pan_scan_rect_cancel_flag, 1, uint8_t);
							if (!pan_scan_rect_cancel_flag)
							{
								nal_read_ue(in_bst, pan_scan_cnt_minus1, uint8_t);
								if (pan_scan_cnt_minus1 > 2)
								{
									map_status.number_of_fields--;
									printf("[H265] pan_scan_cnt_minus1(%d) shall be in the range of 0 to 2, inclusive.\n", pan_scan_cnt_minus1);
									return RET_CODE_BUFFER_NOT_COMPATIBLE;
								}

								for (int i = 0; i <= pan_scan_cnt_minus1; i++) {
									nal_read_se(in_bst, pan_scan_rect_left_offset[i], int32_t);
									nal_read_se(in_bst, pan_scan_rect_right_offset[i], int32_t);
									nal_read_se(in_bst, pan_scan_rect_top_offset[i], int32_t);
									nal_read_se(in_bst, pan_scan_rect_bottom_offset[i], int32_t);
								}

								nal_read_u(in_bst, pan_scan_rect_persistence_flag, 1, uint8_t);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_UE(pan_scan_rect_id, "contains an identifying number that may be used to identify the purpose of the one or more pan-scan rectangles");
					BST_FIELD_PROP_BOOL(pan_scan_rect_cancel_flag, "indicates that the SEI message cancels the persistence of any previous pan-scan rectangle SEI message in output order that applies to the current layer", 
						"indicates that pan-scan rectangle information follows");
					if (!pan_scan_rect_cancel_flag) {
						BST_FIELD_PROP_UE(pan_scan_cnt_minus1, "the number of pan-scan rectangles that are specified by the SEI message");
						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=pan_scan_cnt_minus1;i++)", "");
						for (i = 0; i <= pan_scan_cnt_minus1; i++) {
							BST_ARRAY_FIELD_PROP_SE(pan_scan_rect_left_offset, i, "");
							BST_ARRAY_FIELD_PROP_SE(pan_scan_rect_right_offset, i, "");
							BST_ARRAY_FIELD_PROP_SE(pan_scan_rect_top_offset, i, "");
							BST_ARRAY_FIELD_PROP_SE(pan_scan_rect_bottom_offset, i, "");
						}
						NAV_WRITE_TAG_END("Tag0");

						BST_FIELD_PROP_BOOL(pan_scan_rect_persistence_flag, "the pan-scan rectangle information persists for the current layer in output order until any of the following conditions are true", 
							"the pan-scan rectangle information applies to the current decoded picture only");
					}
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct USER_DATA_REGISTERED_ITU_T_T35 : public SYNTAX_BITSTREAM_MAP
				{
					struct ST2094_40
					{
						struct Window
						{
							uint16_t	window_upper_left_corner_x;
							uint16_t	window_upper_left_corner_y;
							uint16_t	window_lower_right_corner_x;
							uint16_t	window_lower_right_corner_y;
							uint16_t	center_of_ellipse_x;
							uint16_t	center_of_ellipse_y;
							uint8_t		rotation_angle;
							uint16_t	semimajor_axis_internal_ellipse;
							uint16_t	semimajor_axis_external_ellipse;
							uint16_t	semiminor_axis_external_ellipse;
							uint8_t		overlap_process_option : 1;
							uint8_t		byte_align : 7;
						};

						uint16_t		itu_t_t35_terminal_provider_code;
						uint16_t		itu_t_t35_terminal_provider_oriented_code;
						uint8_t			application_identifier;
						uint8_t			application_version;
						
						uint8_t			num_windows:2;
						uint8_t			byte_align_0 : 6;
						Window			windows[3];

						uint32_t		targeted_system_display_maximum_luminance : 27;
						uint32_t		targeted_system_display_actual_peak_luminance_flag : 1;
						uint32_t		dword_align_1 : 4;

						uint8_t			num_rows_targeted_system_display_actual_peak_luminance : 5;
						uint8_t			byte_align_2 : 3;
						uint8_t			num_cols_targeted_system_display_actual_peak_luminance : 5;
						uint8_t			byte_align_3 : 3;

						uint8_t			targeted_system_display_actual_peak_luminance[32][32];

						uint32_t		maxscl[3][3];
						uint32_t		average_maxrgb[3];

						uint8_t			num_distribution_maxrgb_percentiles[3];
						uint8_t			distribution_maxrgb_percentages[3][16];
						uint8_t			distribution_maxrgb_percentiles[3][16];

						uint16_t		fraction_bright_pixels[3];

						uint8_t			mastering_display_actual_peak_luminance_flag : 1;
						uint8_t			num_rows_mastering_display_actual_peak_luminance : 5;
						uint8_t			byte_align_4 : 2;

						uint8_t			num_cols_mastering_display_actual_peak_luminance : 5;
						uint8_t			byte_align_5 : 3;

						uint8_t			mastering_display_actual_peak_luminance[32][32];

						uint8_t			tone_mapping_flag[3];
						uint16_t		knee_point_x[3];
						uint16_t		knee_point_y[3];
						uint8_t			num_bezier_curve_anchors[3];

						uint16_t		bezier_curve_anchors[3][16];
						uint8_t			color_saturation_mapping_flag[3];
						uint8_t			color_saturation_weight[3];
					};

					union
					{
						struct {
							uint8_t		itu_t_t35_country_code;
							uint8_t		itu_t_t35_country_code_extension_byte;
						}PACKED;
						uint8_t		country_code[2];
					}PACKED;

					std::vector<uint8_t>		itu_t_t35_payload_byte;
					SEI_PAYLOAD*				ptr_sei_payload;

					ST2094_40*					ptr_st2094_40;

					USER_DATA_REGISTERED_ITU_T_T35(SEI_PAYLOAD* pSEIPayload) 
						: ptr_sei_payload(pSEIPayload)
						, ptr_st2094_40(NULL){}

					virtual ~USER_DATA_REGISTERED_ITU_T_T35()
					{
						AMP_SAFEDEL2(ptr_st2094_40);
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_b(in_bst, itu_t_t35_country_code, 8, uint8_t);
							if (itu_t_t35_country_code == 0xFF && map_status.number_of_fields < ptr_sei_payload->payload_size) {
								nal_read_b(in_bst, itu_t_t35_country_code_extension_byte, 8, uint8_t);
							}

							int start_num_of_fields = map_status.number_of_fields;
							itu_t_t35_payload_byte.reserve(ptr_sei_payload->payload_size);
							while (map_status.number_of_fields < ptr_sei_payload->payload_size) {
								uint8_t payload_byte = 0;
								nal_read_b(in_bst, payload_byte, 8, uint8_t);
								itu_t_t35_payload_byte.push_back(payload_byte);
							}

							// For UHDBD dynamic HDR metadata
							if (itu_t_t35_country_code == 0xB5 && itu_t_t35_payload_byte.size() > 6)
							{
								// Try to check whether it is a dynamic HDR metadata
								uint16_t itu_t_t35_terminal_provider_code = itu_t_t35_payload_byte[0] << 8 | itu_t_t35_payload_byte[1];
								uint16_t itu_t_t35_terminal_provider_oriented_code = itu_t_t35_payload_byte[2] << 8 | itu_t_t35_payload_byte[3];

								if (itu_t_t35_terminal_provider_code == 0x003C && itu_t_t35_terminal_provider_oriented_code == 0x0001)
								{
									map_status.number_of_fields = start_num_of_fields;
									AMP_NEWT(ptr_st2094_40, ST2094_40);
									if (ptr_st2094_40 == nullptr)
										throw AMException(RET_CODE_OUTOFMEMORY);

									AMBst bst_st_2094_40 = AMBst_CreateFromBuffer(&itu_t_t35_payload_byte[0], (int)itu_t_t35_payload_byte.size());

									nal_read_u(bst_st_2094_40, ptr_st2094_40->itu_t_t35_terminal_provider_code, 16, uint16_t);
									nal_read_u(bst_st_2094_40, ptr_st2094_40->itu_t_t35_terminal_provider_oriented_code, 16, uint16_t);
									nal_read_u(bst_st_2094_40, ptr_st2094_40->application_identifier, 8, uint8_t);
									nal_read_u(bst_st_2094_40, ptr_st2094_40->application_version, 8, uint8_t);
									nal_read_u(bst_st_2094_40, ptr_st2094_40->num_windows, 2, uint8_t);

									for (uint8_t w = 1; w < ptr_st2094_40->num_windows; w++)
									{
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].window_upper_left_corner_x, 16, uint16_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].window_upper_left_corner_y, 16, uint16_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].window_lower_right_corner_x, 16, uint16_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].center_of_ellipse_x, 16, uint16_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].center_of_ellipse_y, 16, uint16_t);

										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].rotation_angle, 8, uint8_t);

										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].semimajor_axis_internal_ellipse, 16, uint16_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].semimajor_axis_external_ellipse, 16, uint16_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].semiminor_axis_external_ellipse, 16, uint16_t);

										nal_read_u(bst_st_2094_40, ptr_st2094_40->windows[w].overlap_process_option, 1, uint8_t);
									}

									nal_read_u(bst_st_2094_40, ptr_st2094_40->targeted_system_display_maximum_luminance, 27, uint32_t);
									nal_read_u(bst_st_2094_40, ptr_st2094_40->targeted_system_display_actual_peak_luminance_flag, 1, uint32_t);
									if (ptr_st2094_40->targeted_system_display_actual_peak_luminance_flag)
									{
										nal_read_u(bst_st_2094_40, ptr_st2094_40->num_rows_targeted_system_display_actual_peak_luminance, 5, uint8_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->num_cols_targeted_system_display_actual_peak_luminance, 5, uint8_t);

										for (uint8_t i = 0; i < ptr_st2094_40->num_rows_targeted_system_display_actual_peak_luminance; i++)
										{
											for (uint8_t j = 0; j < ptr_st2094_40->num_cols_targeted_system_display_actual_peak_luminance; i++)
											{
												nal_read_u(bst_st_2094_40, ptr_st2094_40->targeted_system_display_actual_peak_luminance[i][j], 4, uint8_t);
											}
										}
									}

									for (uint8_t w = 0; w < ptr_st2094_40->num_windows; w++)
									{
										for (uint8_t i = 0; i < 3; i++)
										{
											nal_read_u(bst_st_2094_40, ptr_st2094_40->maxscl[w][i], 17, uint32_t);
										}

										nal_read_u(bst_st_2094_40, ptr_st2094_40->average_maxrgb[w], 17, uint32_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->num_distribution_maxrgb_percentiles[w], 4, uint8_t);

										for (uint8_t i = 0; i < ptr_st2094_40->num_distribution_maxrgb_percentiles[w]; i++)
										{
											nal_read_u(bst_st_2094_40, ptr_st2094_40->distribution_maxrgb_percentages[w][i], 7, uint8_t);
											nal_read_u(bst_st_2094_40, ptr_st2094_40->distribution_maxrgb_percentiles[w][i], 17, uint32_t);
										}

										nal_read_u(bst_st_2094_40, ptr_st2094_40->fraction_bright_pixels[w], 10, uint16_t);
									}

									nal_read_u(bst_st_2094_40, ptr_st2094_40->mastering_display_actual_peak_luminance_flag, 1, uint8_t);
									if (ptr_st2094_40->mastering_display_actual_peak_luminance_flag)
									{
										nal_read_u(bst_st_2094_40, ptr_st2094_40->num_rows_mastering_display_actual_peak_luminance, 5, uint8_t);
										nal_read_u(bst_st_2094_40, ptr_st2094_40->num_cols_mastering_display_actual_peak_luminance, 5, uint8_t);

										for (uint8_t i = 0; i < ptr_st2094_40->num_rows_mastering_display_actual_peak_luminance; i++)
										{
											for (uint8_t j = 0; j < ptr_st2094_40->num_cols_mastering_display_actual_peak_luminance; i++)
											{
												nal_read_u(bst_st_2094_40, ptr_st2094_40->mastering_display_actual_peak_luminance[i][j], 4, uint8_t);
											}
										}
									}

									for (uint8_t w = 0; w < ptr_st2094_40->num_windows; w++)
									{
										nal_read_u(bst_st_2094_40, ptr_st2094_40->tone_mapping_flag[w], 1, uint8_t);
										if (ptr_st2094_40->tone_mapping_flag[w])
										{
											nal_read_u(bst_st_2094_40, ptr_st2094_40->knee_point_x[w], 12, uint16_t);
											nal_read_u(bst_st_2094_40, ptr_st2094_40->knee_point_y[w], 12, uint16_t);
											nal_read_u(bst_st_2094_40, ptr_st2094_40->num_bezier_curve_anchors[w], 4, uint8_t);
											for (uint8_t i = 0; i < ptr_st2094_40->num_bezier_curve_anchors[w]; i++)
											{
												nal_read_u(bst_st_2094_40, ptr_st2094_40->bezier_curve_anchors[w][i], 10, uint16_t);
											}

											nal_read_u(bst_st_2094_40, ptr_st2094_40->color_saturation_mapping_flag[w], 1, uint8_t);
											if (ptr_st2094_40->color_saturation_mapping_flag[w])
											{
												nal_read_u(bst_st_2094_40, ptr_st2094_40->color_saturation_weight[w], 6, uint8_t);
											}
										}

									}

									AMBst_Destroy(bst_st_2094_40);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					long long orig_bit_pos = 0;
					if (bit_offset != NULL)
						orig_bit_pos = *bit_offset;
					BST_FIELD_PROP_2NUMBER1(itu_t_t35_country_code, 8, itu_t_t35_country_code_names[itu_t_t35_country_code]);

					if (itu_t_t35_country_code == 0xB5 && ptr_st2094_40 != NULL)
					{
						BST_FIELD_PROP_2NUMBER("itu_t_t35_terminal_provider_code", 16, ptr_st2094_40->itu_t_t35_terminal_provider_code, "The value shall be 0x003C");
						BST_FIELD_PROP_2NUMBER("itu_t_t35_terminal_provider_oriented_code", 16, ptr_st2094_40->itu_t_t35_terminal_provider_oriented_code, 
							ptr_st2094_40->itu_t_t35_terminal_provider_oriented_code==1?"ST 2094-40":"Unspecified");
						BST_FIELD_PROP_2NUMBER("application_identifier", 8, ptr_st2094_40->application_identifier, "shall be set to 4");
						BST_FIELD_PROP_2NUMBER("application_version", 8, ptr_st2094_40->application_version, "the application version in the application defining document in ST-2094 suite");
						BST_FIELD_PROP_2NUMBER("num_windows", 2, ptr_st2094_40->num_windows, "the number of processing windows");
						for (uint8_t w = 1; w < ptr_st2094_40->num_windows; w++)
						{
							BST_ARRAY_FIELD_PROP_NUMBER("window_upper_left_corner_x", w, 16, ptr_st2094_40->windows[w].window_upper_left_corner_x, 
								"the x coordinate of the top left pixel of the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("window_upper_left_corner_y", w, 16, ptr_st2094_40->windows[w].window_upper_left_corner_y, 
								"the y coordinate of the top left pixel of the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("window_lower_right_corner_x", w, 16, ptr_st2094_40->windows[w].window_lower_right_corner_x, 
								"the x coordinate of the bottom right pixel of the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("window_lower_right_corner_y", w, 16, ptr_st2094_40->windows[w].window_lower_right_corner_y, 
								"the y coordinate of the bottom pixel of the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("center_of_ellipse_x", w, 16, ptr_st2094_40->windows[w].center_of_ellipse_x, 
								"the x coordinate of the center position of the concentric internal and external ellipses of the elliptical pixel selector in the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("center_of_ellipse_y", w, 16, ptr_st2094_40->windows[w].center_of_ellipse_y, 
								"the y coordinate of the center position of the concentric internal and external ellipses of the elliptical pixel selector in the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("rotation_angle", w, 8, ptr_st2094_40->windows[w].rotation_angle, 
								"the clockwise rotation angle in degree of arc with respect to the positive direction of the x-axis of the concentric internal and external ellipses of the elliptical pixel selector in the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("semimajor_axis_internal_ellipse", w, 16, ptr_st2094_40->windows[w].semimajor_axis_internal_ellipse, 
								"the semi-major axis value of the internal ellipse of the elliptical pixel selector in amount of pixels in the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("semimajor_axis_external_ellipse", w, 16, ptr_st2094_40->windows[w].semimajor_axis_external_ellipse, 
								"the semi-major axis value of the external ellipse of the elliptical pixel selector in amount of pixels in the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("semiminor_axis_external_ellipse", w, 16, ptr_st2094_40->windows[w].semiminor_axis_external_ellipse, 
								"the semi-minor axis value of the external ellipse of the elliptical pixel selector in amount of pixels in the w-th processing window");
							BST_ARRAY_FIELD_PROP_NUMBER("overlap_process_option", w, 1, ptr_st2094_40->windows[w].overlap_process_option, 
								ptr_st2094_40->windows[w].overlap_process_option == 0?"the Weighted Averaging method":"the Layering method");
						}

						BST_FIELD_PROP_2NUMBER("targeted_system_display_maximum_luminance", 27, ptr_st2094_40->targeted_system_display_maximum_luminance, 
							"the nominal maximum display luminance of the targeted system display, in units of 0.0001 candelas per square metre");
						BST_FIELD_PROP_2NUMBER("targeted_system_display_actual_peak_luminance_flag", 1, ptr_st2094_40->targeted_system_display_actual_peak_luminance_flag, 
							"");

						if (ptr_st2094_40->targeted_system_display_actual_peak_luminance_flag)
						{
							BST_FIELD_PROP_2NUMBER("num_rows_targeted_system_display_actual_peak_luminance", 5, ptr_st2094_40->num_rows_targeted_system_display_actual_peak_luminance, 
								"the number of rows in the targeted_system_display_actual_peak_luminance array");
							BST_FIELD_PROP_2NUMBER("num_cols_targeted_system_display_actual_peak_luminance", 5, ptr_st2094_40->num_cols_targeted_system_display_actual_peak_luminance, 
								"the number of columns in the targeted_system_display_actual_peak_luminance array");

							for (uint8_t row = 0; row < ptr_st2094_40->num_rows_targeted_system_display_actual_peak_luminance; row++)
								for (uint8_t col = 0; col < ptr_st2094_40->num_cols_targeted_system_display_actual_peak_luminance; col++)
								{
									BST_2ARRAY_FIELD_PROP_NUMBER("targeted_system_display_actual_peak_luminance", row, col, 4, 
										ptr_st2094_40->targeted_system_display_actual_peak_luminance[row][col], "the normalized actual peak luminance of the targeted system display")
								}
						}

						for (uint8_t w = 0; w < ptr_st2094_40->num_windows; w++)
						{
							for (i = 0; i < 3; i++)
							{
								BST_2ARRAY_FIELD_PROP_NUMBER("maxscl", w, i, 17, ptr_st2094_40->maxscl[w][i], 
									"the maximum of the i-th colour component of linearized RGB values in the w-th processing window in the scene");
							}

							BST_ARRAY_FIELD_PROP_NUMBER("average_maxrgb", w, 17, ptr_st2094_40->average_maxrgb[w], 
								"the average of linearized maxRGB values in the w-th processing window in the scene");
							BST_ARRAY_FIELD_PROP_NUMBER("num_distribution_maxrgb_percentiles", w, 4, ptr_st2094_40->num_distribution_maxrgb_percentiles[w], 
								"the number of linearized maxRGB values at given percentiles in the w-th processing window in the scene");

							for (i = 0; i < ptr_st2094_40->num_distribution_maxrgb_percentiles[w]; i++)
							{
								BST_2ARRAY_FIELD_PROP_NUMBER("distribution_maxrgb_percentages", w, i, 7, ptr_st2094_40->distribution_maxrgb_percentages[w][i], 
									"an integer percentage value corresponding to the i-th percentile linearized RGB value in the w-th processing window in the scene");
								BST_2ARRAY_FIELD_PROP_NUMBER("distribution_maxrgb_percentiles", w, i, 17, ptr_st2094_40->distribution_maxrgb_percentiles[w][i], 
									"the linearized maxRGB value at the i-th percentile in the w-th processing window in the scene")
							}

							BST_ARRAY_FIELD_PROP_NUMBER("fraction_bright_pixels", w, 10, ptr_st2094_40->fraction_bright_pixels[w], 
								"the fraction of selected pixels in the image that contains the brightest pixel in the scene");
						}

						BST_FIELD_PROP_2NUMBER("mastering_display_actual_peak_luminance_flag", 1, ptr_st2094_40->mastering_display_actual_peak_luminance_flag, "");
						if (ptr_st2094_40->mastering_display_actual_peak_luminance_flag)
						{
							BST_FIELD_PROP_2NUMBER("num_rows_mastering_display_actual_peak_luminance", 5, ptr_st2094_40->num_rows_mastering_display_actual_peak_luminance, 
								"the number of rows in the mastering_display_actual_peak_luminance array");
							BST_FIELD_PROP_2NUMBER("num_cols_mastering_display_actual_peak_luminance", 5, ptr_st2094_40->num_cols_mastering_display_actual_peak_luminance, 
								"the number of columns in the mastering_display_actual_peak_luminance array");

							for (uint8_t row = 0; row < ptr_st2094_40->num_rows_mastering_display_actual_peak_luminance; row++)
								for (uint8_t col = 0; col < ptr_st2094_40->num_cols_mastering_display_actual_peak_luminance; col++)
								{
									BST_2ARRAY_FIELD_PROP_NUMBER("mastering_display_actual_peak_luminance", row, col, 4,
										ptr_st2094_40->mastering_display_actual_peak_luminance[row][col], "the normalized actual peak luminance of the mastering display used for mastering the image essence");
								}
						}

						for (uint8_t w = 0; w < ptr_st2094_40->num_windows; w++)
						{
							BST_ARRAY_FIELD_PROP_NUMBER("tone_mapping_flag", w, 1, ptr_st2094_40->tone_mapping_flag[w], "");
							if (ptr_st2094_40->tone_mapping_flag[w])
							{
								BST_ARRAY_FIELD_PROP_NUMBER("knee_point_x", w, 12, ptr_st2094_40->knee_point_x[w], 
									"the x coordinate of the separation point between the linear part and the curved part of the tone mapping function");
								BST_ARRAY_FIELD_PROP_NUMBER("knee_point_y", w, 12, ptr_st2094_40->knee_point_y[w], 
									"the y coordinate of the separation point between the linear part and the curved part of the tone mapping function");
								BST_ARRAY_FIELD_PROP_NUMBER("num_bezier_curve_anchors", w, 4, ptr_st2094_40->num_bezier_curve_anchors[w], 
									"the number of the intermediate anchor parameters of the tone mapping function in the w-th processing window");

								for (i = 0; i < ptr_st2094_40->num_bezier_curve_anchors[w]; i++)
								{
									BST_2ARRAY_FIELD_PROP_NUMBER("bezier_curve_anchors", w, i, 10, ptr_st2094_40->bezier_curve_anchors[w][i],
										"the i-th intermediate anchor parameter of the tone mapping function in the w-th processing window in the scene");
								}

								BST_ARRAY_FIELD_PROP_NUMBER("color_saturation_mapping_flag", w, 1, ptr_st2094_40->color_saturation_mapping_flag[w], "");
								if (ptr_st2094_40->color_saturation_mapping_flag[w])
								{
									BST_ARRAY_FIELD_PROP_NUMBER("color_saturation_weight", w, 6, ptr_st2094_40->color_saturation_weight[w], 
										"a number that shall adjust the colour saturation gain in the w-th processing window in the scene");
								}
							}
						}
					}
					else
					{
						if (itu_t_t35_country_code == 0xFF) {
							BST_FIELD_PROP_2NUMBER1(itu_t_t35_country_code_extension_byte, 8, "");
						}
						for (i = 0; i < (int)itu_t_t35_payload_byte.size(); i++) {
							BST_ARRAY_FIELD_PROP_NUMBER("itu_t_t35_payload_byte", i, itu_t_t35_payload_byte[i], 8, "");
						}
					}

					if (bit_offset != NULL)
						*bit_offset = orig_bit_pos + 8ULL * ((uint64_t)itu_t_t35_payload_byte.size() + 1 + (itu_t_t35_country_code == 0xFF ? 1 : 0));

					DECLARE_FIELDPROP_END()

				};

				struct USER_DATA_UNREGISTERED : public SYNTAX_BITSTREAM_MAP
				{
					/*
						MPEG_cc_data() {
						cc_data()
						marker_bits
						}
					*/
					struct CC_DATA : public SYNTAX_BITSTREAM_MAP
					{
						struct CC_DATA_ITEM {
							uint8_t		marker_bits : 5;
							uint8_t		cc_valid : 1;
							uint8_t		cc_type : 2;
							uint8_t		cc_data_1;
							uint8_t		cc_data_2;
						}PACKED;

						uint8_t		reserved_0 : 1;
						uint8_t		process_cc_data_flag : 1;
						uint8_t		additional_data_flag : 1;
						uint8_t		cc_count : 5;

						uint8_t		reserved_1 = 0;
						uint8_t		marker_bits = 0;

						std::vector<CC_DATA_ITEM>	items;

						CC_DATA()
							: reserved_0(0), process_cc_data_flag(0), additional_data_flag(0), cc_count(0) {
						}

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								nal_read_u(in_bst, reserved_0, 1, uint8_t);
								nal_read_u(in_bst, process_cc_data_flag, 1, uint8_t);
								nal_read_u(in_bst, additional_data_flag, 1, uint8_t);
								nal_read_u(in_bst, cc_count, 5, uint8_t);

								nal_read_u(in_bst, reserved_1, 8, uint8_t);

								if (cc_count)
									items.reserve(cc_count);

								for (int i = 0; i < cc_count; i++) {
									CC_DATA_ITEM cc_data_item;
									nal_read_u(in_bst, cc_data_item.marker_bits, 5, uint8_t);
									nal_read_u(in_bst, cc_data_item.cc_valid, 1, uint8_t);
									nal_read_u(in_bst, cc_data_item.cc_type, 2, uint8_t);
									nal_read_u(in_bst, cc_data_item.cc_data_1, 8, uint8_t);
									nal_read_u(in_bst, cc_data_item.cc_data_2, 8, uint8_t);

									items.push_back(cc_data_item);
								}

								nal_read_u(in_bst, marker_bits, 8, uint8_t);

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
						BST_FIELD_PROP_NUMBER1(reserved_0, 1, "should be 1");
						BST_FIELD_PROP_NUMBER1(process_cc_data_flag, 1, process_cc_data_flag?"the cc_data has to be parsed and its meaning has to be processed":"the cc_data can be discarded");
						BST_FIELD_PROP_NUMBER1(additional_data_flag, 1, additional_data_flag?"indicate the presence of additional user data":"indicate the absence of additional user data");
						BST_FIELD_PROP_2NUMBER1(cc_count, 5, "the number of closed caption constructs following this field");

						BST_FIELD_PROP_2NUMBER1(reserved_1, 8, "should be 0xFF");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;cc_count;i++)", "");
						for (i = 0; i < cc_count; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER("marker_bits", i, 5, items[i].marker_bits, "");
							BST_ARRAY_FIELD_PROP_NUMBER("cc_valid", i, 1, items[i].cc_valid, items[i].cc_valid?"the following two bytes of closed-caption data are valid":"the two closed caption bytes are invalid");
							BST_ARRAY_FIELD_PROP_NUMBER("cc_type", i, 2, items[i].cc_type, cc_type_names[items[i].cc_type]);
							BST_ARRAY_FIELD_PROP_NUMBER("cc_data_1", i, 8, items[i].cc_data_1, "The first byte of a closed caption data pair as defined in CEA-708");
							BST_ARRAY_FIELD_PROP_NUMBER("cc_data_2", i, 8, items[i].cc_data_2, "The second byte of a closed caption data pair as defined in CEA-708");
						}
						NAV_WRITE_TAG_END("Tag0");

						BST_FIELD_PROP_2NUMBER1(marker_bits, 8, "should be 0xff");
						DECLARE_FIELDPROP_END()

					};

					struct GOP_STRUCTURE_MAP : public SYNTAX_BITSTREAM_MAP
					{
						struct GOP_PIC_INFO
						{
							uint8_t		stuffing_bits : 1;
							uint8_t		picture_structure : 3;
							uint8_t		picture_type : 4;
						}PACKED;

						uint16_t		number_of_pictures_in_GOP = 0;
						std::vector<GOP_PIC_INFO>
										GOP_pic_info;

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								nal_read_u(in_bst, number_of_pictures_in_GOP, 16, uint16_t);
								if (number_of_pictures_in_GOP > 0)
									GOP_pic_info.reserve(number_of_pictures_in_GOP);
								for (int i = 0; i < number_of_pictures_in_GOP; i++) {
									GOP_PIC_INFO info;
									nal_read_u(in_bst, info.stuffing_bits, 1, uint8_t);
									nal_read_u(in_bst, info.picture_structure, 3, uint8_t);
									nal_read_u(in_bst, info.picture_type, 4, uint8_t);
									GOP_pic_info.push_back(info);
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
							UNREFERENCED_PARAMETER(out_bst);
							return RET_CODE_ERROR_NOTIMPL;
						}

						DECLARE_FIELDPROP_BEGIN()
						BST_FIELD_PROP_2NUMBER1(number_of_pictures_in_GOP, 16, "the number of pictures in a GOP which this GOP structure map is contained");
						NAV_WRITE_TAG_WITH_ALIAS("Tag0", "for(i=0; i&lt;number_of_pictures_in_GOP;i++)", "");
						for (i = 0; i < number_of_pictures_in_GOP; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER("stuffing_bits", i, 1, GOP_pic_info[i].stuffing_bits, "should be 1");
							BST_ARRAY_FIELD_PROP_NUMBER("picture_structure", i, 3, GOP_pic_info[i].picture_structure, sei_GOP_picture_structure_names[GOP_pic_info[i].picture_structure]);
							BST_ARRAY_FIELD_PROP_NUMBER("picture_type", i,4, GOP_pic_info[i].picture_type, sei_GOP_picture_type_names[GOP_pic_info[i].picture_type]);
						}
						NAV_WRITE_TAG_END("Tag0");
						DECLARE_FIELDPROP_END()

					};

					struct HEVC_GOP_STRUCTURE_MAP : public SYNTAX_BITSTREAM_MAP
					{
						struct GOP_PIC_INFO
						{
							uint8_t		stuffing_bits : 5;
							uint8_t		picture_structure : 3;
							uint8_t		reserved : 1;
							uint8_t		temporal_id : 3;
							uint8_t		picture_type : 4;
						}PACKED;

						uint16_t		number_of_pictures_in_GOP = 0;
						std::vector<GOP_PIC_INFO>
										GOP_pic_info;

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								nal_read_u(in_bst, number_of_pictures_in_GOP, 16, uint16_t);
								if (number_of_pictures_in_GOP > 0)
									GOP_pic_info.reserve(number_of_pictures_in_GOP);
								for (int i = 0; i < number_of_pictures_in_GOP; i++) {
									GOP_PIC_INFO info;
									nal_read_u(in_bst, info.stuffing_bits, 5, uint8_t);
									nal_read_u(in_bst, info.picture_structure, 3, uint8_t);
									nal_read_u(in_bst, info.reserved, 1, uint8_t);
									nal_read_u(in_bst, info.temporal_id, 3, uint8_t);
									nal_read_u(in_bst, info.picture_type, 4, uint8_t);
									GOP_pic_info.push_back(info);
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
							UNREFERENCED_PARAMETER(out_bst);
							return RET_CODE_ERROR_NOTIMPL;
						}

						DECLARE_FIELDPROP_BEGIN()
						BST_FIELD_PROP_2NUMBER1(number_of_pictures_in_GOP, 16, "the number of pictures in a GOP which this GOP structure map is contained");
						NAV_WRITE_TAG_WITH_ALIAS("Tag0", "for(i=0; i&lt;number_of_pictures_in_GOP;i++)", "");
						for (i = 0; i < number_of_pictures_in_GOP; i++) {
							BST_ARRAY_FIELD_PROP_NUMBER("stuffing_bits", i, 5, GOP_pic_info[i].stuffing_bits, "should be 1");
							BST_ARRAY_FIELD_PROP_NUMBER("picture_structure", i, 3, GOP_pic_info[i].picture_structure, sei_HEVC_GOP_picture_structure_names[GOP_pic_info[i].picture_structure]);
							BST_ARRAY_FIELD_PROP_NUMBER("reserved", i, 1, GOP_pic_info[i].reserved, "");
							BST_ARRAY_FIELD_PROP_NUMBER("temporal_id", i, 3, GOP_pic_info[i].temporal_id, "");
							BST_ARRAY_FIELD_PROP_NUMBER("picture_type", i, 4, GOP_pic_info[i].picture_type, sei_HEVC_GOP_picture_type_names[GOP_pic_info[i].picture_type]);
						}
						NAV_WRITE_TAG_END("Tag0");
						DECLARE_FIELDPROP_END()

					};

					struct OFFSET_METADATA : public SYNTAX_BITSTREAM_MAP
					{
						struct PLANE_OFFSET
						{
							uint8_t		Plane_offset_direction_flag : 1;
							uint8_t		Plane_offset_value : 7;
						}PACKED;

						uint8_t		marker_bit_0 : 1;
						uint8_t		reserved_for_future_use_0 : 3;
						uint8_t		frame_rate : 4;

						uint8_t		reserved_for_future_use_1 : 5;
						uint8_t		PTS_32_30 : 3;

						uint16_t	marker_bit_1 : 1;
						uint16_t	PTS_29_15 : 15;

						uint16_t	marker_bit_2 : 1;
						uint16_t	PTS_14_0 : 15;

						uint8_t		marker_bit_3 : 1;
						uint8_t		reserved_for_future_use_2 : 1;
						uint8_t		number_of_offset_sequences : 6;
						uint8_t		number_of_displayed_frames_in_GOP;

						uint16_t	marker_bit_4 : 1;
						uint16_t	reserved_for_future_use_3 : 15;

						std::vector<PLANE_OFFSET>
									plane_offsets;

						int Map(AMBst in_bst)
						{
							int iRet = RET_CODE_SUCCESS;
							SYNTAX_BITSTREAM_MAP::Map(in_bst);

							try
							{
								MAP_BST_BEGIN(0);
								nal_read_u(in_bst, marker_bit_0, 1, uint8_t);
								nal_read_u(in_bst, reserved_for_future_use_0, 3, uint8_t);
								nal_read_u(in_bst, frame_rate, 4, uint8_t);
								nal_read_u(in_bst, reserved_for_future_use_1, 5, uint8_t);
								nal_read_u(in_bst, PTS_32_30, 3, uint8_t);

								nal_read_u(in_bst, marker_bit_1, 1, uint16_t);
								nal_read_u(in_bst, PTS_29_15, 15, uint16_t);

								nal_read_u(in_bst, marker_bit_2, 1, uint16_t);
								nal_read_u(in_bst, PTS_14_0, 15, uint16_t);

								nal_read_u(in_bst, marker_bit_3, 1, uint8_t);
								nal_read_u(in_bst, reserved_for_future_use_2, 1, uint8_t);
								nal_read_u(in_bst, number_of_offset_sequences, 6, uint8_t);

								nal_read_u(in_bst, number_of_displayed_frames_in_GOP, 8, uint8_t);

								nal_read_u(in_bst, marker_bit_4, 1, uint16_t);
								nal_read_u(in_bst, reserved_for_future_use_3, 15, uint16_t);

								if (number_of_offset_sequences > 0 && number_of_displayed_frames_in_GOP > 0)
									plane_offsets.reserve((size_t)number_of_offset_sequences*number_of_displayed_frames_in_GOP);

								for (uint16_t i = 0; i < number_of_offset_sequences; i++)
									for (uint16_t j = 0; j < number_of_displayed_frames_in_GOP; j++) {
										PLANE_OFFSET plane_offset;
										nal_read_u(in_bst, plane_offset.Plane_offset_direction_flag, 1, uint8_t);
										nal_read_u(in_bst, plane_offset.Plane_offset_value, 7, uint8_t);
										plane_offsets.push_back(plane_offset);
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
							UNREFERENCED_PARAMETER(out_bst);
							return RET_CODE_ERROR_NOTIMPL;
						}

						TM_90KHZ GetFirstFramePts() {
							return ((TM_90KHZ)PTS_32_30 << 30) | ((TM_90KHZ)PTS_29_15 << 15) | ((TM_90KHZ)PTS_14_0);
						}

						DECLARE_FIELDPROP_BEGIN()
						BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_0, "");
						BST_FIELD_PROP_NUMBER("reserved_for_future_use", 3, reserved_for_future_use_0, "");
						BST_FIELD_PROP_NUMBER("frame_rate", 4, frame_rate, frame_rate_names[frame_rate]);
						BST_FIELD_PROP_NUMBER("reserved_for_future_use", 5, reserved_for_future_use_1, "");
						BST_FIELD_PROP_2NUMBER_WITH_ALIAS("PTS[32..30]", 3, PTS_32_30, "");

						BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_1, "");
						BST_FIELD_PROP_2NUMBER_WITH_ALIAS("PTS[29..15]", 15, PTS_29_15, "");

						BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_2, "");
						BST_FIELD_PROP_2NUMBER_WITH_ALIAS("PTS[14..0]", 15, PTS_14_0, "");

						NAV_WRITE_TAG_WITH_LL_NUMBER_VALUE("PTS", GetFirstFramePts(), "90KHZ");

						BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_3, "");
						BST_FIELD_PROP_NUMBER("reserved_for_future_use", 1, reserved_for_future_use_2, "");
						BST_FIELD_PROP_NUMBER("number_of_offset_sequences", 6, number_of_offset_sequences, "indicates number of offset_sequence()");

						BST_FIELD_PROP_NUMBER("number_of_displayed_frames_in_GOP", 8, number_of_displayed_frames_in_GOP, "indicates the number of frames to be displayed in a GOP in which this offset_metada() is contained");

						BST_FIELD_PROP_NUMBER("marker_bit", 1, marker_bit_4, "");
						BST_FIELD_PROP_NUMBER("reserved_for_future_use", 15, reserved_for_future_use_3, "");

						if (number_of_offset_sequences > 0)
						{
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(offset_sequence_id = 0;offset_sequence_id&lt;number_of_offset_sequences;offset_sequence_id++)", "");
							for (i = 0; i < number_of_offset_sequences; i++)
							{
								NAV_WRITE_TAG_ARRAY_BEGIN0("offset_sequence", i, "");
								for (int j = 0; j < number_of_displayed_frames_in_GOP; j++) {
									BST_ARRAY_FIELD_PROP_NUMBER("Tag00", j, 1, plane_offsets[(size_t)i*number_of_displayed_frames_in_GOP + j].Plane_offset_direction_flag,
										plane_offsets[(size_t)i*number_of_displayed_frames_in_GOP + j].Plane_offset_direction_flag ? "The associated Graphics plane appears further away from the viewer" :
										"The associated Graphics plane appears closer to the viewer");
									BST_ARRAY_FIELD_PROP_NUMBER("Tag01", j, 7, plane_offsets[(size_t)i*number_of_displayed_frames_in_GOP + j].Plane_offset_value, "indicates amount of the pixels for horizontally shifting the associated Graphics Plane");
								}
								NAV_WRITE_TAG_END("offset_sequence");
							}
							NAV_WRITE_TAG_END("Tag0");

						}

						DECLARE_FIELDPROP_END()

					};

					uint8_t						uuid_iso_iec_11578[16] = { 0 };
					std::vector<uint8_t>		user_data_payload_byte;

					uint32_t					type_indicator;

					union
					{
						CC_DATA*			cc_data;
						GOP_STRUCTURE_MAP*	GOP_structure_map;
						HEVC_GOP_STRUCTURE_MAP*
											HEVC_GOP_structure_map;
						OFFSET_METADATA*	offset_metadata;
						void*				unregistered_data_obj;
					};

					SEI_PAYLOAD*				ptr_sei_payload;

					USER_DATA_UNREGISTERED(SEI_PAYLOAD* pSEIPayload)
						: type_indicator(0)
						, unregistered_data_obj(NULL)
						, ptr_sei_payload(pSEIPayload) {}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						const uint8_t BD_user_data_unregistered_uuid[16] = {0x17, 0xee, 0x8c, 0x60, 0xf8, 0x4d, 0x11, 0xd9, 0x8c, 0xd6, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66};

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_bytes(in_bst, uuid_iso_iec_11578, 16);
							if (memcmp(uuid_iso_iec_11578, BD_user_data_unregistered_uuid, 16) == 0 && ptr_sei_payload->payload_size > 20)
							{
								nal_read_u(in_bst, type_indicator, 32, uint32_t);
								if (type_indicator == 0x47413934)	// Closed_caption: 0x4741 3934 (same as ATSC_indicator for Closed Caption) 'OA94'
								{
									nal_read_ref(in_bst, cc_data, CC_DATA);
								}
								else if (type_indicator == 0x48474D50)	// HEVC GOP structure map: 0x4847 4D50 'HGMP'
								{
									nal_read_ref(in_bst, HEVC_GOP_structure_map, HEVC_GOP_STRUCTURE_MAP);
								}
								else if (type_indicator == 0x47534D50)	// GOP structure map: 0x4753 4D50 'GSMP'
								{
									nal_read_ref(in_bst, GOP_structure_map, GOP_STRUCTURE_MAP);
								}
								else if (type_indicator == 0x4F464D44)	// Offset metadata: 0x4F46 4D44 'OFMD'
								{
									nal_read_ref(in_bst, offset_metadata, OFFSET_METADATA);
								}
								else if (ptr_sei_payload->payload_size > 20)
								{
									user_data_payload_byte.resize((size_t)((int64_t)ptr_sei_payload->payload_size - 20));
									uint8_t* pPayloadBytes = &user_data_payload_byte[0];
									nal_read_bytes(in_bst, pPayloadBytes, ptr_sei_payload->payload_size - 20);
								}
							}
							else if (ptr_sei_payload->payload_size > 16) {
								user_data_payload_byte.resize((size_t)((int64_t)ptr_sei_payload->payload_size - 16));
								uint8_t* pPayloadBytes = &user_data_payload_byte[0];
								nal_read_bytes(in_bst, pPayloadBytes, ptr_sei_payload->payload_size - 16);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
						//BST_FIELD_PROP_FIXSIZE_BINSTR("uuid_iso_iec_11578", 128, uuid_iso_iec_11578, 16, szTemp2);
						BST_FIELD_PROP_UUID1(uuid_iso_iec_11578, "A UUID according to the procedures of ISO/IEC 11578:1996 Annex A");
						MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
							uuid_iso_iec_11578[0x00], uuid_iso_iec_11578[0x01], uuid_iso_iec_11578[0x02], uuid_iso_iec_11578[0x03],
							uuid_iso_iec_11578[0x04], uuid_iso_iec_11578[0x05], uuid_iso_iec_11578[0x06], uuid_iso_iec_11578[0x07],
							uuid_iso_iec_11578[0x08], uuid_iso_iec_11578[0x09], uuid_iso_iec_11578[0x0a], uuid_iso_iec_11578[0x0b],
							uuid_iso_iec_11578[0x0c], uuid_iso_iec_11578[0x0d], uuid_iso_iec_11578[0x0e], uuid_iso_iec_11578[0x0f]);
						if (MBCSICMP(szTemp2, "17ee8c60-f84d-11d9-8cd6-0800200c9a66") == 0)
						{
							if (type_indicator == 0x47413934)	// Closed_caption: 0x4741 3934 (same as ATSC_indicator for Closed Caption) 'OA94'
							{
								BST_FIELD_PROP_2NUMBER1(type_indicator, 32, "Closed_caption: 0x4741 3934 (same as ATSC_indicator for Closed Caption)");
								BST_FIELD_PROP_REF1(cc_data);
							}
							else if (type_indicator == 0x48474D50)	// HEVC GOP structure map: 0x4847 4D50 'HGMP'
							{
								BST_FIELD_PROP_2NUMBER1(type_indicator, 32, "HEVC GOP structure map: 0x4847 4D50");
								BST_FIELD_PROP_REF1(HEVC_GOP_structure_map);
							}
							else if (type_indicator == 0x47534D50)	// GOP structure map: 0x4753 4D50 'GSMP'
							{
								BST_FIELD_PROP_2NUMBER1(type_indicator, 32, "GOP structure map: 0x4753 4D50");
								BST_FIELD_PROP_REF1(GOP_structure_map);
							}
							else if (type_indicator == 0x4F464D44)	// Offset metadata: 0x4F46 4D44 'OFMD'
							{
								BST_FIELD_PROP_2NUMBER1(type_indicator, 32, "Offset metadata: 0x4F46 4D44");
								BST_FIELD_PROP_REF1(offset_metadata);
							}
							else if (ptr_sei_payload->payload_size > 20)
							{
								MBCSPRINTF_S(szTemp3, TEMP3_SIZE, "'%c%c%c%c'", (type_indicator >> 24) & 0xFF, (type_indicator >> 16) & 0xFF, (type_indicator >> 8) & 0xFF, type_indicator & 0xFF);
								BST_FIELD_PROP_2NUMBER1(type_indicator, 32, type_indicator== 0x48445230?"Reserved for BDMV HDR dynamic metadata":(
																			type_indicator== 0x48445232?"hdr_processing_data() of Philips HDR SEI message: 0x4844 5232": szTemp3));
								if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields)) {
								for (i = 0; i < (int)user_data_payload_byte.size(); i++) {
										NAV_ARRAY_FIELD_PROP_NUMBER_("user_data_payload_byte", "", i, 8, user_data_payload_byte[i], "");
									}
									field_prop_idx++;
								}
							}
						}
						else if (MBCSICMP(szTemp2, "dc45e9bd-e6d9-48b7-962c-d820d923eeef") == 0)	// X264 build information
						{
							if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields)) {
								uint8_t* x264_params = &user_data_payload_byte[0];
								std::string str_x264_sei_version((const char*)x264_params, user_data_payload_byte.size());
								unsigned long field_bits = (unsigned long)((uint64_t)user_data_payload_byte.size() * 8);
								//NAV_FIELD_PROP_FIXSIZE_STR("x264_sei_version", field_bits, x264_params, user_data_payload_byte.size(), "");
								BST_FIELD_PROP_FIXSIZE_BINSTR("x264_sei_version", field_bits, x264_params, user_data_payload_byte.size(), str_x264_sei_version.c_str());
								field_prop_idx++;
							}
						}
						else
						{
							if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields)) {
							for (i = 0; i < (int)user_data_payload_byte.size(); i++) {
									NAV_ARRAY_FIELD_PROP_NUMBER_("user_data_payload_byte", "", i, 8, user_data_payload_byte[i], "");
								}
								field_prop_idx++;
							}
						}
					DECLARE_FIELDPROP_END()
				};

				struct SCENE_INFO : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		scene_info_present_flag : 1;
					uint8_t		prev_scene_id_valid_flag : 1;
					uint8_t		scene_transition_type : 6;

					uint32_t	scene_id = 0;
					uint32_t	second_scene_id = 0;

					SCENE_INFO()
						: scene_info_present_flag(0), prev_scene_id_valid_flag(0), scene_transition_type(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, scene_info_present_flag, 1, uint8_t);
							if (scene_info_present_flag) {
								nal_read_u(in_bst, prev_scene_id_valid_flag, 1, uint8_t);
								nal_read_ue(in_bst, scene_id, uint8_t);
								nal_read_ue(in_bst, scene_transition_type, uint8_t);
								if (scene_transition_type > 3) {
									nal_read_ue(in_bst, second_scene_id, uint8_t);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_BOOL(scene_info_present_flag, "indicates that the target pictures belong to the same scene or scene transition", 
						"indicates that the scene or scene transition to which the target pictures belong is unspecified");
					if (scene_info_present_flag) {
						BST_FIELD_PROP_BOOL(prev_scene_id_valid_flag, "specifies that the scene_id value of the picture preceding the first picture of the target pictures in output order is specified by the previous scene information SEI message in decoding order", 
							"specifies that the scene_id value of the picture preceding the first picture of the target pictures in output order is considered unspecified in the semantics of the syntax elements of this SEI message");
						BST_FIELD_PROP_UE(scene_id, "identifies the scene to which the target pictures belong");
						BST_FIELD_PROP_UE(scene_transition_type, scene_transition_type>6?"unknown": scene_transition_type_names[scene_transition_type]);
						if (scene_transition_type > 3) {
							BST_FIELD_PROP_UE(second_scene_id, "identifies the next scene in the gradual scene transition in which the target pictures are involved");
						}
					}
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct PICTURE_SNAPSHOT : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		snapshot_id = 0;

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, snapshot_id, uint32_t);
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
					BST_FIELD_PROP_UE(snapshot_id, "specifies a snapshot identification number");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct PROGRESSIVE_REFINEMENT_SEGMENT_START : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		progressive_refinement_id = 0;
					uint32_t		pic_order_cnt_delta = 0;

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, progressive_refinement_id, uint32_t);
							nal_read_ue(in_bst, pic_order_cnt_delta, uint32_t);
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
					BST_FIELD_PROP_UE(progressive_refinement_id, "specifies an identification number for the progressive refinement operation");
					BST_FIELD_PROP_UE(pic_order_cnt_delta, "specifies the last picture in refinementPicSet in decoding order");
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct PROGRESSIVE_REFINEMENT_SEGMENT_END : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		progressive_refinement_id = 0;

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, progressive_refinement_id, uint32_t);
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
					BST_FIELD_PROP_UE(progressive_refinement_id, "specifies an identification number for the progressive refinement operation");
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct FILM_GRAIN_CHARACTERISTICS : public SYNTAX_BITSTREAM_MAP
				{
					struct COMP_MODEL_INFO
					{
						uint16_t		comp_model_present_flag:1;
						uint16_t		num_intensity_intervals_minus1:8;
						uint16_t		num_model_values_minus1:3;
						uint16_t		reserved : 4;

						std::vector<uint8_t>
									intensity_interval_lower_bound;
						std::vector<uint8_t>
									intensity_interval_upper_bound;
						std::vector<int32_t>
									comp_model_value;

						COMP_MODEL_INFO() : comp_model_present_flag(0), num_intensity_intervals_minus1(0), num_model_values_minus1(0), reserved(0) {
						}
					};

					uint8_t		film_grain_characteristics_cancel_flag : 1;
					uint8_t		film_grain_model_id : 2;
					uint8_t		separate_colour_description_present_flag : 1;
					uint8_t		reserved_0 : 4;

					uint8_t		film_grain_bit_depth_luma_minus8 : 3;
					uint8_t		film_grain_bit_depth_chroma_minus8 : 3;
					uint8_t		film_grain_full_range_flag : 1;
					uint8_t		reserved_1 : 1;

					uint8_t		film_grain_colour_primaries = 0;
					uint8_t		film_grain_transfer_characteristics = 0;
					uint8_t		film_grain_matrix_coeffs = 0;

					uint8_t		blending_mode_id : 2;
					uint8_t		log2_scale_factor : 4;
					uint8_t		film_grain_characteristics_persistence_flag : 1;
					uint8_t		reserved_2 : 1;

					COMP_MODEL_INFO
								comp_model_info[3];

					FILM_GRAIN_CHARACTERISTICS()
						: film_grain_characteristics_cancel_flag(0), film_grain_model_id(0), separate_colour_description_present_flag(0), reserved_0(0)
						, film_grain_bit_depth_luma_minus8(0), film_grain_bit_depth_chroma_minus8(0), film_grain_full_range_flag(0), reserved_1(0)
						, blending_mode_id(0), log2_scale_factor(0), film_grain_characteristics_persistence_flag(0), reserved_2(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, film_grain_characteristics_cancel_flag, 1, uint8_t);
							if (!film_grain_characteristics_cancel_flag) {
								nal_read_u(in_bst, film_grain_model_id, 2, uint8_t);
								nal_read_u(in_bst, separate_colour_description_present_flag, 1, uint8_t);
								if (separate_colour_description_present_flag) {
									nal_read_u(in_bst, film_grain_bit_depth_luma_minus8, 3, uint8_t);
									nal_read_u(in_bst, film_grain_bit_depth_chroma_minus8, 3, uint8_t);
									nal_read_u(in_bst, film_grain_full_range_flag, 1, uint8_t);
									nal_read_u(in_bst, film_grain_colour_primaries, 8, uint8_t);
									nal_read_u(in_bst, film_grain_transfer_characteristics, 8, uint8_t);
									nal_read_u(in_bst, film_grain_matrix_coeffs, 8, uint8_t);
								}
								nal_read_u(in_bst, blending_mode_id, 2, uint8_t);
								nal_read_u(in_bst, log2_scale_factor, 4, uint8_t);

								for (int c = 0; c < 3; c++) {
									nal_read_u(in_bst, comp_model_info[c].comp_model_present_flag, 1, uint8_t);
								}

								for (int c = 0; c < 3; c++) {
									if (comp_model_info[c].comp_model_present_flag) {
										nal_read_u(in_bst, comp_model_info[c].num_intensity_intervals_minus1, 8, uint8_t);
										nal_read_u(in_bst, comp_model_info[c].num_model_values_minus1, 3, uint8_t);
										comp_model_info[c].intensity_interval_lower_bound.reserve((size_t)comp_model_info[c].num_intensity_intervals_minus1 + 1);
										comp_model_info[c].intensity_interval_upper_bound.reserve((size_t)comp_model_info[c].num_intensity_intervals_minus1 + 1);
										comp_model_info[c].comp_model_value.reserve(((size_t)comp_model_info[c].num_intensity_intervals_minus1 + 1)*((size_t)comp_model_info[c].num_model_values_minus1 + 1));
										for (int i = 0; i <= comp_model_info[c].num_intensity_intervals_minus1; i++) {
											uint8_t tmp_byte = 0;
											nal_read_u(in_bst, tmp_byte, 8, uint8_t);
											comp_model_info[c].intensity_interval_lower_bound.push_back(tmp_byte);
											nal_read_u(in_bst, tmp_byte, 8, uint8_t);
											comp_model_info[c].intensity_interval_upper_bound.push_back(tmp_byte);
											for (int j = 0; j <= comp_model_info[c].num_model_values_minus1; j++) {
												int32_t tmpInt = 0;
												nal_read_se(in_bst, tmpInt, int32_t);
												comp_model_info[c].comp_model_value.push_back(tmpInt);
											}
										}
									}
								}

								nal_read_u(in_bst, film_grain_characteristics_persistence_flag, 1, uint8_t);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_BOOL(film_grain_characteristics_cancel_flag, "indicates that the SEI message cancels the persistence of any previous film grain characteristics SEI message in output order that applies to the current layer", 
						"indicates that film grain modeling information follow");
					if (!film_grain_characteristics_cancel_flag) {
						BST_FIELD_PROP_2NUMBER1(film_grain_model_id, 2, film_grain_model_id_names[film_grain_model_id]);
						BST_FIELD_PROP_BOOL(separate_colour_description_present_flag, "indicates that a distinct colour space description for the film grain characteristics is present in the film grain characteristics SEI message syntax", 
							"indicates that the colour description for the film grain characteristics is the same as for the CVS as specified in clause E.3.1");
						if (separate_colour_description_present_flag) {
							BST_FIELD_PROP_2NUMBER1(film_grain_bit_depth_luma_minus8, 3, "plus 8 specifies the bit depth used for the luma component of the film grain characteristics");
							BST_FIELD_PROP_2NUMBER1(film_grain_bit_depth_chroma_minus8, 3, "plus 8 specifies the bit depth used for the Cb and Cr components of the film grain characteristics");
							BST_FIELD_PROP_BOOL(film_grain_full_range_flag, "", "");
							BST_FIELD_PROP_2NUMBER1(film_grain_colour_primaries, 8, vui_colour_primaries_names[film_grain_colour_primaries]);
							BST_FIELD_PROP_2NUMBER1(film_grain_transfer_characteristics, 8, vui_transfer_characteristics_names[film_grain_transfer_characteristics]);
							BST_FIELD_PROP_2NUMBER1(film_grain_matrix_coeffs, 8, vui_matrix_coeffs_descs[film_grain_matrix_coeffs]);
						}

						BST_FIELD_PROP_2NUMBER1(blending_mode_id, 2, blending_mode_id_names[blending_mode_id]);
						BST_FIELD_PROP_2NUMBER1(log2_scale_factor, 4, "a scale factor used in the film grain characterization equations");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(c=0; c&lt;3; c++)", "");
						for (int c = 0; c < 3; c++) {
							BST_ARRAY_FIELD_PROP_NUMBER("comp_model_present_flag", c, 1, comp_model_info[c].comp_model_present_flag, 
								comp_model_info[c].comp_model_present_flag?"indicates that syntax elements specifying modeling of film grain on colour component c are present":"indicates that film grain is not modeled on the c-th colour component");
						}
						NAV_WRITE_TAG_END("Tag0");

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(c=0; c&lt;3; c++)", "");
						for (int c = 0; c < 3; c++) {
							BST_ARRAY_FIELD_PROP_NUMBER("num_intensity_intervals_minus1", c, 8, comp_model_info[c].num_intensity_intervals_minus1, "plus 1 specifies the number of intensity intervals for which a specific set of model values has been estimated");
							BST_ARRAY_FIELD_PROP_NUMBER("num_model_values_minus1", c, 3, comp_model_info[c].num_model_values_minus1, "plus 1 specifies the number of model values present for each intensity interval in which the film grain has been modeled");
							
							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "for(i=0;i&lt;=num_intensity_intervals_minus1[c];i++)", "");
							for (int i = 0; i <= comp_model_info[c].num_intensity_intervals_minus1; i++) {
								BST_2ARRAY_FIELD_PROP_NUMBER("intensity_interval_lower_bound", c, i, 8, comp_model_info[c].intensity_interval_lower_bound[i], "the lower bound of the interval i of intensity levels for which the set of model values applies");
								BST_2ARRAY_FIELD_PROP_NUMBER("intensity_interval_upper_bound", c, i, 8, comp_model_info[c].intensity_interval_upper_bound[i], "the upper bound of the interval i of intensity levels for which the set of model values applies");
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag100", "for(j=0;j&lt;=num_model_values_minus1[c];j++)", "");
								for (int j = 0; j <= comp_model_info[c].num_model_values_minus1; j++) {
									int32_t	comp_model_value = comp_model_info[c].comp_model_value[(size_t)i*((size_t)comp_model_info[c].num_model_values_minus1 + 1) + j];
									BST_3ARRAY_FIELD_PROP_NUMBER("num_model_values_minus1", c, i, j, (long long)quick_log2((comp_model_value >= 0 ? comp_model_value : ((-comp_model_value) + 1)) + 1) * 2 + 1, comp_model_value, 
										"represents each one of the model values present for the colour component c and the intensity interval i");
								}
								NAV_WRITE_TAG_END("Tag100");
							}
							NAV_WRITE_TAG_END("Tag10");
						}
						NAV_WRITE_TAG_END("Tag1");
					}
					BST_FIELD_PROP_BOOL(film_grain_characteristics_persistence_flag, "specifies that the film grain characteristics SEI message persists for the current layer in output order until any of the following conditions are true", 
						"the film grain characteristics SEI message applies to the current decoded picture only");
					DECLARE_FIELDPROP_END()

				};

				// reserved for D.2.14 Post-filter hint SEI message syntax

				struct TONE_MAPPING_INFO : public SYNTAX_BITSTREAM_MAP
				{
					struct PIVOT_LAYOUT
					{
						uint16_t					num_pivots = 0;
						std::vector<uint32_t>		coded_pivot_value;
						std::vector<uint32_t>		target_pivot_value;
					};

					uint32_t		tone_map_id = 0;
					uint8_t			tone_map_cancel_flag : 1;
					uint8_t			tone_map_persistence_flag : 1;
					uint8_t			reserved_0 : 6;

					uint8_t			coded_data_bit_depth = 0;
					uint8_t			target_bit_depth = 0;
					uint32_t		tone_map_model_id;

					union
					{
						uint8_t			bytes[32] = { 0 };
						struct {
							uint32_t	min_value;
							uint32_t	max_value;
						};
						struct {
							uint32_t	sigmoid_midpoint;
							uint32_t	sigmoid_width;
						};
						uint32_t*		start_of_coded_interval;
						PIVOT_LAYOUT*	pivot_layout;
						struct {
							uint8_t		camera_iso_speed_idc;
							uint32_t	camera_iso_speed_value;

							uint8_t		exposure_index_idc;
							uint32_t	exposure_index_value;

							uint8_t		exposure_compensation_value_sign_flag;

							uint16_t	exposure_compensation_value_numerator;
							uint16_t	exposure_compensation_value_denom_idc;
							uint32_t	ref_screen_luminance_white;
							uint32_t	extended_range_white_level;
							uint16_t	nominal_black_level_code_value;
							uint16_t	nominal_white_level_code_value;
							uint16_t	extended_white_level_code_value;
						}PACKED;
					};

					TONE_MAPPING_INFO() 
						: tone_map_cancel_flag(0), tone_map_persistence_flag(0), reserved_0(0)
						, tone_map_model_id(0xFFFFFFFF), start_of_coded_interval(NULL){
					}

					virtual ~TONE_MAPPING_INFO() {
						if (tone_map_model_id == 2) {
							AMP_SAFEDELA2(start_of_coded_interval);
						}
						else if (tone_map_model_id == 3) {
							AMP_SAFEDEL2(pivot_layout);
						}
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, tone_map_id, uint32_t);
							nal_read_u(in_bst, tone_map_cancel_flag, 1, uint8_t);
							if (!tone_map_cancel_flag) {
								nal_read_u(in_bst, tone_map_persistence_flag, 1, uint8_t);
								nal_read_u(in_bst, coded_data_bit_depth, 8, uint8_t);
								nal_read_u(in_bst, target_bit_depth, 8, uint8_t);
								nal_read_ue(in_bst, tone_map_model_id, uint32_t);

								if (tone_map_model_id == 0)
								{
									nal_read_u(in_bst, min_value, 32, uint32_t);
									nal_read_u(in_bst, max_value, 32, uint32_t);
								}
								else if (tone_map_model_id == 1)
								{
									nal_read_u(in_bst, sigmoid_midpoint, 32, uint32_t);
									nal_read_u(in_bst, sigmoid_width, 32, uint32_t);
								}
								else if (tone_map_model_id == 2)
								{
									if (target_bit_depth == 0 || target_bit_depth > 16)
										throw AMException(RET_CODE_OUT_OF_RANGE);

									AMP_NEW(start_of_coded_interval, uint32_t, (size_t)((1LL << target_bit_depth)));
									uint8_t v = ((coded_data_bit_depth + 7) >> 3) << 3;
									if (start_of_coded_interval == NULL)
										throw AMException(RET_CODE_OUTOFMEMORY);
									for (size_t i = 0; start_of_coded_interval != NULL && i < (size_t)((1LL << target_bit_depth)); i++) {
										nal_read_u(in_bst, start_of_coded_interval[i], v, uint32_t);
									}
								}
								else if (tone_map_model_id == 3)
								{
									AMP_NEWT(pivot_layout, PIVOT_LAYOUT);
									if (pivot_layout == nullptr)
										throw AMException(RET_CODE_OUTOFMEMORY);

									nal_read_u(in_bst, pivot_layout->num_pivots, 16, uint16_t);
									pivot_layout->coded_pivot_value.reserve(pivot_layout->num_pivots);
									pivot_layout->target_pivot_value.reserve(pivot_layout->num_pivots);
									uint8_t v = ((coded_data_bit_depth + 7) >> 3) << 3;
									for (int i = 0; i < pivot_layout->num_pivots; i++) {
										uint32_t tmpuint = 0;
										nal_read_u(in_bst, tmpuint, v, uint32_t);
										pivot_layout->coded_pivot_value.push_back(tmpuint);
										nal_read_u(in_bst, tmpuint, v, uint32_t);
										pivot_layout->target_pivot_value.push_back(tmpuint);
									}
								}
								else if (tone_map_model_id == 4)
								{
									nal_read_u(in_bst, camera_iso_speed_idc, 8, uint8_t);
									if (camera_iso_speed_idc == 0xFF) {
										nal_read_u(in_bst, camera_iso_speed_value, 32, uint32_t);
									}
									nal_read_u(in_bst, exposure_index_idc, 8, uint8_t);
									if (exposure_index_idc == 0xFF) {
										nal_read_u(in_bst, exposure_index_value, 32, uint32_t);
									}

									nal_read_u(in_bst, exposure_compensation_value_sign_flag, 1, uint8_t);
									nal_read_u(in_bst, exposure_compensation_value_numerator, 16, uint16_t);
									nal_read_u(in_bst, exposure_compensation_value_denom_idc, 16, uint16_t);
									nal_read_u(in_bst, ref_screen_luminance_white, 32, uint32_t);
									nal_read_u(in_bst, extended_range_white_level, 32, uint32_t);
									nal_read_u(in_bst, nominal_black_level_code_value, 16, uint16_t);
									nal_read_u(in_bst, nominal_white_level_code_value, 16, uint16_t);
									nal_read_u(in_bst, extended_white_level_code_value, 16, uint16_t);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_UE(tone_map_id, "contains an identifying number that may be used to identify the purpose of the tone mapping model");
					BST_FIELD_PROP_BOOL(tone_map_cancel_flag, "indicates that the tone mapping information SEI message cancels the persistence of any previous tone mapping information", "indicates that tone mapping information follows");
					if (!tone_map_cancel_flag) {
						BST_FIELD_PROP_BOOL(tone_map_persistence_flag, "specifies that the tone mapping information persists for the current layer in output order until any of the following conditions are true", 
							"specifies that the tone mapping information applies to the current decoded picture only");
						BST_FIELD_PROP_2NUMBER1(coded_data_bit_depth, 8, "the BitDepthY for interpretation of the luma component");
						BST_FIELD_PROP_2NUMBER1(target_bit_depth, 8, "the bit depth of the output of the dynamic range mapping function");
						BST_FIELD_PROP_UE(tone_map_model_id, "the model utilized for mapping the coded data into the target_bit_depth range");
						if (tone_map_model_id == 0)
						{
							BST_FIELD_PROP_2NUMBER1(min_value, 32, "specifies the RGB sample value that maps to the minimum value in the bit depth indicated by target_bit_depth");
							BST_FIELD_PROP_2NUMBER1(max_value, 32, "specifies the RGB sample value that maps to the maximum value in the bit depth indicated by target_bit_depth");
						}
						else if (tone_map_model_id == 1)
						{
							BST_FIELD_PROP_2NUMBER1(sigmoid_midpoint, 32, "specifies the RGB sample value of the coded data that is mapped to the center point of the target_bit_depth representation");
							BST_FIELD_PROP_2NUMBER1(sigmoid_width, 32, "specifies the distance between two coded data values that approximately correspond to the 5% and 95% values of the target_bit_depth representation, respectively");
						}
						else if (tone_map_model_id == 2)
						{
							uint8_t v = ((coded_data_bit_depth + 7) >> 3) << 3;
							for (i = 0; i < (1 << target_bit_depth); i++) {
								BST_ARRAY_FIELD_PROP_NUMBER("start_of_coded_interval", i, v, start_of_coded_interval[i], "specifies the beginning point of an interval in the coded data");
							}
						}
						else if (tone_map_model_id == 3)
						{
							uint8_t v = ((coded_data_bit_depth + 7) >> 3) << 3;
							BST_FIELD_PROP_2NUMBER("num_pivots", 16, pivot_layout->num_pivots, "specifies the number of pivot points in the piece-wise linear mapping function");
							if (pivot_layout->num_pivots > 0)
							{
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0; i&lt;num_pivots; i++)", "");
								for (i = 0; i < pivot_layout->num_pivots; i++)
								{
									BST_ARRAY_FIELD_PROP_NUMBER("coded_pivot_value", i, v, pivot_layout->coded_pivot_value[i], "specifies the value in the coded_data_bit_depth corresponding to the i-th pivot point");
									BST_ARRAY_FIELD_PROP_NUMBER("target_pivot_value", i, v, pivot_layout->target_pivot_value[i], "specifies the value in the reference target_bit_depth corresponding to the i-th pivot point");
								}
								NAV_WRITE_TAG_END("Tag0");
							}
						}
						else if (tone_map_model_id == 4)
						{
							uint32_t Indicated_values[] = {0xFFFFFFFF, 10, 12, 16, 20, 25, 32, 40, 50, 64, 80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000};
							if (camera_iso_speed_idc >= 1 && camera_iso_speed_idc <= 30) {
								MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%" PRIu32 ", indicates the camera ISO speed for daylight illumination", Indicated_values[camera_iso_speed_idc]);
							} else {
								MBCSCPY(szTemp2, TEMP2_SIZE, camera_iso_speed_idc == 0 ? "Unspecified" : (camera_iso_speed_idc == 255 ? "EXTENDED_ISO" : "Reserved"));
							}
							BST_FIELD_PROP_2NUMBER1(camera_iso_speed_idc, 8, szTemp2);
							if (camera_iso_speed_idc == 0xFF) {
								BST_FIELD_PROP_2NUMBER1(camera_iso_speed_value, 32, "indicates the camera ISO speed for daylight illumination");
							}

							if (exposure_index_idc >= 1 && exposure_index_idc <= 30) {
								MBCSPRINTF_S(szTemp2, TEMP2_SIZE, "%" PRIu32 ", indicates the camera ISO speed for daylight illumination", Indicated_values[exposure_index_idc]);
							} else {
								MBCSCPY(szTemp2, TEMP2_SIZE, exposure_index_idc == 0 ? "Unspecified" : (exposure_index_idc == 255 ? "EXTENDED_ISO" : "Reserved"));
							}
							BST_FIELD_PROP_2NUMBER1(exposure_index_idc, 8, szTemp2);
							if (exposure_index_idc == 0xFF) {
								BST_FIELD_PROP_2NUMBER1(exposure_index_value, 32, "indicates the exposure index setting of the camera");
							}

							BST_FIELD_PROP_NUMBER1(exposure_compensation_value_sign_flag, 1, "specifies the sign of the variable ExposureCompensationValue");
							BST_FIELD_PROP_2NUMBER1(exposure_compensation_value_numerator, 16, "specifies the numerator of the variable ExposureCompensationValue");
							BST_FIELD_PROP_2NUMBER1(exposure_compensation_value_denom_idc, 16, "specifies the denominator of the variable ExposureCompensationValue");
							BST_FIELD_PROP_2NUMBER1(ref_screen_luminance_white, 32, "the reference screen brightness setting for the extended white level");
							BST_FIELD_PROP_2NUMBER1(extended_range_white_level, 32, "the luminance dynamic range for extended dynamic-range display of the associated pictures");
							BST_FIELD_PROP_2NUMBER1(nominal_black_level_code_value, 16, "the luma sample value of the associated decoded pictures to which the nominal black level is assigned");
							BST_FIELD_PROP_2NUMBER1(nominal_white_level_code_value, 16, "the luma sample value of the associated decoded pictures to which the nominal white level is assigned");
							BST_FIELD_PROP_2NUMBER1(extended_white_level_code_value, 16, "the luma sample value of the associated decoded pictures to which the white level associated with an extended dynamic range is assigned");
						}
					}
					DECLARE_FIELDPROP_END()
				};

				struct FRAME_PACKING_ARRANGEMENT : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		frame_packing_arrangement_id = 0;
					uint8_t			frame_packing_arrangement_cancel_flag = 0;
					uint8_t			frame_packing_arrangement_type : 7;
					uint8_t			quincunx_sampling_flag : 1;

					uint8_t			content_interpretation_type : 6;
					uint8_t			spatial_flipping_flag : 1;
					uint8_t			frame0_flipped_flag : 1;

					uint8_t			field_views_flag : 1;
					uint8_t			current_frame_is_frame0_flag : 1;
					uint8_t			frame0_self_contained_flag : 1;
					uint8_t			frame1_self_contained_flag : 1;
					uint8_t			reserved_0 : 4;

					uint8_t			frame0_grid_position_x:4;
					uint8_t			frame0_grid_position_y:4;
					uint8_t			frame1_grid_position_x:4;
					uint8_t			frame1_grid_position_y:4;

					uint8_t			frame_packing_arrangement_reserved_byte = 0;
					uint8_t			frame_packing_arrangement_persistence_flag : 1;
					uint8_t			upsampled_aspect_ratio_flag : 1;
					uint8_t			reserved_1 : 6;

					FRAME_PACKING_ARRANGEMENT()
						: frame_packing_arrangement_type(0)
						, quincunx_sampling_flag(0)
						, content_interpretation_type(0)
						, spatial_flipping_flag(0)
						, frame0_flipped_flag(0)
						, field_views_flag(0)
						, current_frame_is_frame0_flag(0)
						, frame0_self_contained_flag(0)
						, frame1_self_contained_flag(0)
						, reserved_0(0)
						, frame0_grid_position_x(0)
						, frame0_grid_position_y(0)
						, frame1_grid_position_x(0)
						, frame1_grid_position_y(0)
						, frame_packing_arrangement_persistence_flag(0)
						, upsampled_aspect_ratio_flag(0)
						, reserved_1(0)
					{}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);

							nal_read_ue(in_bst, frame_packing_arrangement_id, uint32_t);
							nal_read_u(in_bst, frame_packing_arrangement_cancel_flag, 1, uint8_t);

							if (!frame_packing_arrangement_cancel_flag)
							{
								nal_read_u(in_bst, frame_packing_arrangement_type, 7, uint8_t);
								nal_read_u(in_bst, quincunx_sampling_flag, 1, uint8_t);
								nal_read_u(in_bst, content_interpretation_type, 6, uint8_t);
								nal_read_u(in_bst, spatial_flipping_flag, 1, uint8_t);
								nal_read_u(in_bst, frame0_flipped_flag, 1, uint8_t);
								nal_read_u(in_bst, field_views_flag, 1, uint8_t);
								nal_read_u(in_bst, current_frame_is_frame0_flag, 1, uint8_t);
								nal_read_u(in_bst, frame0_self_contained_flag, 1, uint8_t);
								nal_read_u(in_bst, frame1_self_contained_flag, 1, uint8_t);
								if (!quincunx_sampling_flag && frame_packing_arrangement_type != 5)
								{
									nal_read_u(in_bst, frame0_grid_position_x, 4, uint8_t);
									nal_read_u(in_bst, frame0_grid_position_y, 4, uint8_t);
									nal_read_u(in_bst, frame1_grid_position_x, 4, uint8_t);
									nal_read_u(in_bst, frame1_grid_position_y, 4, uint8_t);
								}
								nal_read_u(in_bst, frame_packing_arrangement_reserved_byte, 8, uint8_t);
								nal_read_u(in_bst, frame_packing_arrangement_persistence_flag, 1, uint8_t);
							}

							nal_read_u(in_bst, upsampled_aspect_ratio_flag, 1, uint8_t);

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
					BST_FIELD_PROP_UE(frame_packing_arrangement_id, "");
					BST_FIELD_PROP_BOOL(frame_packing_arrangement_cancel_flag, "indicates that the frame packing arrangement SEI message cancels the persistence of any previous frame packing arrangement SEI message in output order that applies to the current layer", 
						"indicates that frame packing arrangement information follows");
					if (!frame_packing_arrangement_cancel_flag)
					{
						BST_FIELD_PROP_2NUMBER1(frame_packing_arrangement_type, 7, "indicates the type of packing arrangement of the frames");
						BST_FIELD_PROP_BOOL(quincunx_sampling_flag, "indicates that each colour component plane of each constituent frame is quincunx sampled", "indicates that the colour component planes of each constituent frame are not quincunx sampled");
						BST_FIELD_PROP_2NUMBER1(content_interpretation_type, 6, content_interpretation_type == 0 ? "Unspecified relationship between the frame packed constituent frames" : (
							content_interpretation_type == 1 ? "Indicates that the two constituent frames form the left and right views of a stereo view scene, with frame 0 being associated with the left view and frame 1 being associated with the right view" : (
								content_interpretation_type == 2 ? "Indicates that the two constituent frames form the right and left views of a stereo view scene, with frame 0 being associated with the right view and frame 1 being associated with the left view" :
								"indicates the intended interpretation of the constituent frames")));
						BST_FIELD_PROP_2NUMBER1(spatial_flipping_flag, 1, "");
						BST_FIELD_PROP_2NUMBER1(frame0_flipped_flag, 1, "");
						BST_FIELD_PROP_2NUMBER1(field_views_flag, 1, "");
						BST_FIELD_PROP_2NUMBER1(current_frame_is_frame0_flag, 1, "");
						BST_FIELD_PROP_2NUMBER1(frame0_self_contained_flag, 1, "");
						BST_FIELD_PROP_2NUMBER1(frame1_self_contained_flag, 1, "");
						if (!quincunx_sampling_flag && frame_packing_arrangement_type != 5)
						{
							BST_FIELD_PROP_2NUMBER1(frame0_grid_position_x, 4, "specifies the x component of the ( x, y ) coordinate pair for constituent frame 0");
							BST_FIELD_PROP_2NUMBER1(frame0_grid_position_y, 4, "specifies the y component of the ( x, y ) coordinate pair for constituent frame 0");
							BST_FIELD_PROP_2NUMBER1(frame1_grid_position_x, 4, "specifies the x component of the ( x, y ) coordinate pair for constituent frame 1");
							BST_FIELD_PROP_2NUMBER1(frame1_grid_position_y, 4, "specifies the y component of the ( x, y ) coordinate pair for constituent frame 1");
						}
						BST_FIELD_PROP_2NUMBER1(frame_packing_arrangement_reserved_byte, 8, "");
						BST_FIELD_PROP_BOOL(frame_packing_arrangement_persistence_flag, "persists for the current layer in output order", "applies to the current decoded frame only");

					}
					BST_FIELD_PROP_BOOL(upsampled_aspect_ratio_flag, "", "");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct DISPLAY_ORIENTATION : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		display_orientation_cancel_flag : 1;
					uint8_t		hor_flip : 1;
					uint8_t		ver_flip : 1;
					uint8_t		display_orientation_persistence_flag : 1;
					uint8_t		reserved : 4;
					uint16_t	anticlockwise_rotation = 0;

					DISPLAY_ORIENTATION()
						: display_orientation_cancel_flag(0), hor_flip(0), ver_flip(0), display_orientation_persistence_flag(0), reserved(0), anticlockwise_rotation(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, display_orientation_cancel_flag, 1, uint8_t);
							if (!display_orientation_cancel_flag)
							{
								nal_read_u(in_bst, hor_flip, 1, uint8_t);
								nal_read_u(in_bst, ver_flip, 1, uint8_t);
								nal_read_u(in_bst, anticlockwise_rotation, 16, uint16_t);
								nal_read_u(in_bst, display_orientation_persistence_flag, 1, uint8_t);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
						BST_FIELD_PROP_BOOL(display_orientation_cancel_flag, "cancels the persistence of any previous display orientation SEI message in output order", "display orientation information follows");
						if (!display_orientation_cancel_flag)
						{
							BST_FIELD_PROP_BOOL(hor_flip, "the cropped decoded picture should be flipped horizontally for display", "the decoded picture should not be flipped horizontally");
							BST_FIELD_PROP_BOOL(ver_flip, "the cropped decoded picture should be flipped vertically for display", "the decoded picture should not be flipped vertically");
							BST_FIELD_PROP_2NUMBER1(anticlockwise_rotation, 16, "the recommended anticlockwise rotation of the decoded picture prior to display");
							BST_FIELD_PROP_BOOL(display_orientation_persistence_flag, "the display orientation SEI message persists for the current layer in output order", "the display orientation SEI message applies to the current decoded picture only");
						}
					DECLARE_FIELDPROP_END()

				}PACKED;

				struct STRUCTURE_OF_PICTURES_INFO : public SYNTAX_BITSTREAM_MAP
				{
					struct SOP_ENTRY {
						uint8_t		sop_vcl_nut;
						uint8_t		sop_temporal_id : 3;
						uint8_t		sop_short_term_rps_idx : 5;
						int16_t		sop_poc_delta;
					}PACKED;

					uint16_t		sop_seq_parameter_set_id : 4;
					uint16_t		num_entries_in_sop_minus1 : 12;
					std::vector<SOP_ENTRY>
									sop_entries;

					STRUCTURE_OF_PICTURES_INFO()
						: sop_seq_parameter_set_id(0), num_entries_in_sop_minus1(0){
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, sop_seq_parameter_set_id, uint16_t);
							nal_read_ue(in_bst, num_entries_in_sop_minus1, uint16_t);

							sop_entries.reserve((size_t)num_entries_in_sop_minus1 + 1);
							for (int i = 0; i <= num_entries_in_sop_minus1; i++) {
								SOP_ENTRY sop_entry;
								nal_read_u(in_bst, sop_entry.sop_vcl_nut, 6, uint8_t);
								nal_read_u(in_bst, sop_entry.sop_temporal_id, 3, uint8_t);
								
								if (sop_entry.sop_vcl_nut != 19 && sop_entry.sop_vcl_nut != 20) {
									nal_read_ue(in_bst, sop_entry.sop_short_term_rps_idx, uint8_t);
								}

								if (i > 0) {
									nal_read_se(in_bst, sop_entry.sop_poc_delta, int16_t);
								}

								sop_entries.push_back(sop_entry);
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_UE(sop_seq_parameter_set_id, "");
					BST_FIELD_PROP_UE(num_entries_in_sop_minus1, "");
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;=num_entries_in_sop_minus1;i++)", "");
					for (i = 0; i <= num_entries_in_sop_minus1; i++) {
						BST_ARRAY_FIELD_PROP_NUMBER("sop_vcl_nut", i, 6, sop_entries[i].sop_vcl_nut, "shall be equal to the nal_unit_type value of the picture corresponding to the i-th entry");
						BST_ARRAY_FIELD_PROP_NUMBER("sop_temporal_id", i, 3, sop_entries[i].sop_temporal_id, "shall be equal to the TemporalId value of the picture corresponding to the i-th entry");

						if (sop_entries[i].sop_vcl_nut != 19 && sop_entries[i].sop_vcl_nut != 20) {
							BST_ARRAY_FIELD_PROP_UE1(sop_entries, sop_short_term_rps_idx, i, "shall be equal to the index, into the list of candidate short-term RPSs included in the active SPS");
						}

						if (i > 0) {
							BST_ARRAY_FIELD_PROP_SE1(sop_entries, sop_poc_delta, i, "the value of the variable entryPicOrderCnt[i] for the i-th entry described in the structure of pictures information SEI message");
						}
					}
					NAV_WRITE_TAG_END("Tag0");
					DECLARE_FIELDPROP_END()
				};

				// D.2.19 Decoded picture hash SEI message syntax

				// D.2.20 Active parameter sets SEI message syntax

				// D.2.21 Decoding unit information SEI message syntax

				struct TEMPORAL_SUB_LAYER_ZERO_INDEX : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t		temporal_sub_layer_zero_idx = 0;
					uint8_t		irap_pic_id = 0;

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, temporal_sub_layer_zero_idx, 8, uint8_t);
							nal_read_u(in_bst, irap_pic_id, 8, uint8_t);
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
					BST_FIELD_PROP_2NUMBER1(temporal_sub_layer_zero_idx, 8, "");
					BST_FIELD_PROP_2NUMBER1(irap_pic_id, 8, "an IRAP picture identifier for the current layer");
					DECLARE_FIELDPROP_END()
				}PACKED;

				struct COLOUR_REMAPPING_INFO : public SYNTAX_BITSTREAM_MAP
				{
					uint32_t		colour_remap_id = 0;
					uint8_t			colour_remap_cancel_flag : 1;
					uint8_t			colour_remap_persistence_flag : 1;
					uint8_t			colour_remap_video_signal_info_present_flag : 1;
					uint8_t			colour_remap_full_range_flag : 1;
					uint8_t			reserved_0 : 4;

					uint8_t			colour_remap_primaries = 0;
					uint8_t			colour_remap_transfer_function = 0;
					uint8_t			colour_remap_matrix_coefficients = 0;

					uint8_t			colour_remap_input_bit_depth = 0;
					uint8_t			colour_remap_bit_depth = 0;
					uint8_t			pre_lut_num_val_minus1[3] = { 0 };
					std::vector<uint32_t>
									pre_lut_coded_value[3];
					std::vector<uint32_t>
									pre_lut_target_value[3];

					uint8_t			colour_remap_matrix_present_flag : 1;
					uint8_t			log2_matrix_denom : 4;
					uint8_t			reserved_1 : 3;

					int16_t			colour_remap_coeffs[3][3] = { {0} };

					uint8_t			post_lut_num_val_minus1[3] = { 0 };
					std::vector<uint32_t>
									post_lut_coded_value[3];
					std::vector<uint32_t>
									post_lut_target_value[3];

					COLOUR_REMAPPING_INFO()
						: colour_remap_cancel_flag(0)
						, colour_remap_persistence_flag(0)
						, colour_remap_video_signal_info_present_flag(0)
						, colour_remap_full_range_flag(0)
						, reserved_0(0)
						, colour_remap_matrix_present_flag(0)
						, log2_matrix_denom(0)
						, reserved_1(0) {
					}

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_ue(in_bst, colour_remap_id, uint32_t);
							nal_read_u(in_bst, colour_remap_cancel_flag, 1, uint8_t);

							if (!colour_remap_cancel_flag) {
								nal_read_u(in_bst, colour_remap_persistence_flag, 1, uint8_t);
								nal_read_u(in_bst, colour_remap_video_signal_info_present_flag, 1, uint8_t);
								if (colour_remap_video_signal_info_present_flag)
								{
									nal_read_u(in_bst, colour_remap_full_range_flag, 1, uint8_t);
									nal_read_u(in_bst, colour_remap_primaries, 8, uint8_t);
									nal_read_u(in_bst, colour_remap_transfer_function, 8, uint8_t);
									nal_read_u(in_bst, colour_remap_matrix_coefficients, 8, uint8_t);
								}

								nal_read_u(in_bst, colour_remap_input_bit_depth, 8, uint8_t);
								nal_read_u(in_bst, colour_remap_bit_depth, 8, uint8_t);

								uint8_t coded_v = ((colour_remap_input_bit_depth + 7) >> 3) << 3;
								uint8_t target_v = ((colour_remap_bit_depth + 7) >> 3) << 3;

								for (int c = 0; c < 3; c++) {
									nal_read_u(in_bst, pre_lut_num_val_minus1[c], 8, uint8_t);
									if (pre_lut_num_val_minus1[c] > 0){
										pre_lut_coded_value[c].reserve((size_t)pre_lut_num_val_minus1[c] + 1);
										pre_lut_target_value[c].reserve((size_t)pre_lut_num_val_minus1[c] + 1);
										for (int i = 0; i <= pre_lut_num_val_minus1[c]; i++) {
											uint32_t u32Val = 0;
											nal_read_u(in_bst, u32Val, coded_v, uint32_t);
											pre_lut_coded_value[c].push_back(u32Val);
											nal_read_u(in_bst, u32Val, coded_v, uint32_t);
											pre_lut_target_value[c].push_back(u32Val);
										}
									}
								}

								nal_read_u(in_bst, colour_remap_matrix_present_flag, 1, uint8_t);
								if (colour_remap_matrix_present_flag) {
									nal_read_u(in_bst, log2_matrix_denom, 4, uint8_t);

									for (int c = 0; c < 3; c++)
										for (int i = 0; i < 3; i++) {
											nal_read_se(in_bst, colour_remap_coeffs[c][i], uint16_t);
										}
								}
								
								for (int c = 0; c < 3; c++) {
									nal_read_u(in_bst, post_lut_num_val_minus1[c], 8, uint8_t);
									if (post_lut_num_val_minus1[c] > 0) {
										post_lut_coded_value[c].reserve((size_t)post_lut_num_val_minus1[c] + 1);
										post_lut_target_value[c].reserve((size_t)post_lut_num_val_minus1[c] + 1);
										for (int i = 0; i <= pre_lut_num_val_minus1[c]; i++) {
											uint32_t u32Val = 0;
											nal_read_u(in_bst, u32Val, coded_v, uint32_t);
											post_lut_coded_value[c].push_back(u32Val);
											nal_read_u(in_bst, u32Val, coded_v, uint32_t);
											post_lut_target_value[c].push_back(u32Val);
										}
									}
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
						UNREFERENCED_PARAMETER(out_bst);
						return RET_CODE_ERROR_NOTIMPL;
					}

					DECLARE_FIELDPROP_BEGIN()
					BST_FIELD_PROP_UE(colour_remap_id, "contains an identifying number that may be used to identify the purpose of the colour remapping information");
					BST_FIELD_PROP_BOOL(colour_remap_cancel_flag, "cancels the persistence of any previous colour remapping information SEI message in output order that applies to the current layer", "indicates that colour remapping information follows");
					
					if (!colour_remap_cancel_flag) {
						BST_FIELD_PROP_BOOL(colour_remap_persistence_flag, "the colour remapping information persists for the current layer in output order", 
							"the colour remapping information applies to the current picture only");
						BST_FIELD_PROP_BOOL(colour_remap_video_signal_info_present_flag, "syntax elements colour_remap_full_range_flag, colour_remap_primaries, colour_remap_transfer_function and colour_remap_matrix_coefficients are present", 
							"syntax elements colour_remap_full_range_flag, colour_remap_primaries, colour_remap_transfer_function and colour_remap_matrix_coefficients are not present");

						if (colour_remap_video_signal_info_present_flag){
							BST_FIELD_PROP_BOOL(colour_remap_full_range_flag, "", "");
							BST_FIELD_PROP_2NUMBER1(colour_remap_primaries, 8, vui_colour_primaries_names[colour_remap_primaries]);
							BST_FIELD_PROP_2NUMBER1(colour_remap_transfer_function, 8, vui_transfer_characteristics_names[colour_remap_transfer_function]);
							BST_FIELD_PROP_2NUMBER1(colour_remap_matrix_coefficients, 8, vui_matrix_coeffs_descs[colour_remap_matrix_coefficients]);
						}

						BST_FIELD_PROP_2NUMBER1(colour_remap_input_bit_depth, 8, "the bit depth of the luma and chroma components or the RGB components of the associated pictures");
						BST_FIELD_PROP_2NUMBER1(colour_remap_bit_depth, 8, "the bit depth of the output of the colour remapping function");

						uint8_t coded_v = ((colour_remap_input_bit_depth + 7) >> 3) << 3;
						uint8_t target_v = ((colour_remap_bit_depth + 7) >> 3) << 3;

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(c = 0; c &lt; 3; c++)", "");
						for (int c = 0; c < 3; c++) {
							BST_ARRAY_FIELD_PROP_NUMBER("pre_lut_num_val_minus1", c, 8, pre_lut_num_val_minus1[c], "");
							if (pre_lut_num_val_minus1[c] > 0) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag00", "for(i=0; i&lt;=pre_lut_num_val_minus1[c]; i++)", "");
								for (i = 0; i <= pre_lut_num_val_minus1[c]; i++) {
									BST_2ARRAY_FIELD_PROP_NUMBER("pre_lut_coded_value", c, i, coded_v, pre_lut_coded_value[c][i], "");
									BST_2ARRAY_FIELD_PROP_NUMBER("pre_lut_target_value", c, i, target_v, pre_lut_target_value[c][i], "");
								}
								NAV_WRITE_TAG_END("Tag00");
							}
						}
						NAV_WRITE_TAG_END("Tag0");

						BST_FIELD_PROP_BOOL(colour_remap_matrix_present_flag, "the syntax elements log2_matrix_denom and colour_remap_coeffs[c][i], for c and i in the range of 0 to 2, inclusive, are present", 
							"the syntax elements log2_matrix_denom and colour_remap_coeffs[c][i], for c and i in the range of 0 to 2, inclusive, are not present");
						if (colour_remap_matrix_present_flag) {
							BST_FIELD_PROP_2NUMBER1(log2_matrix_denom, 4, "the base 2 logarithm of the denominator for all matrix coefficients");

							NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(c = 0; c &lt; 3; c++)", "");
							for (int c = 0; c < 3; c++) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag10", "for(i = 0; i &lt; 3; i++)", "");
								for (int i = 0; i < 3; i++) {
									int16_t crc_val = colour_remap_coeffs[c][i];
									long long field_bits = (long long)quick_log2((crc_val >= 0 ? crc_val : ((-crc_val) + 1)) + 1) * 2 + 1;
									BST_2ARRAY_FIELD_PROP_NUMBER("colour_remap_coeffs", c, i, field_bits, crc_val, "the value of the three-by-three colour remapping matrix coefficients");
								}
								NAV_WRITE_TAG_END("Tag10");
							}
							NAV_WRITE_TAG_END("Tag1");

						}

						NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag2", "for(c = 0; c &lt; 3; c++)", "");
						for (int c = 0; c < 3; c++) {
							BST_ARRAY_FIELD_PROP_NUMBER("post_lut_num_val_minus1", c, 8, post_lut_num_val_minus1[c], "");
							if (pre_lut_num_val_minus1[c] > 0) {
								NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag20", "for(i=0; i&lt;=post_lut_num_val_minus1[c]; i++)", "");
								for (i = 0; i <= post_lut_num_val_minus1[c]; i++) {
									BST_2ARRAY_FIELD_PROP_NUMBER("post_lut_coded_value", c, i, coded_v, post_lut_coded_value[c][i], "");
									BST_2ARRAY_FIELD_PROP_NUMBER("post_lut_target_value", c, i, target_v, post_lut_target_value[c][i], "");
								}
								NAV_WRITE_TAG_END("Tag20");
							}
						}
						NAV_WRITE_TAG_END("Tag2");
					}

					DECLARE_FIELDPROP_END()
				};

				struct DEINTERLACED_FIELD_INDENTIFICATION : public SYNTAX_BITSTREAM_MAP
				{
					uint8_t			deinterlaced_picture_source_parity_flag = 0;

					int Map(AMBst in_bst)
					{
						int iRet = RET_CODE_SUCCESS;
						SYNTAX_BITSTREAM_MAP::Map(in_bst);

						try
						{
							MAP_BST_BEGIN(0);
							nal_read_u(in_bst, deinterlaced_picture_source_parity_flag, 1, uint8_t);
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
					BST_FIELD_PROP_BOOL(deinterlaced_picture_source_parity_flag, "indicates that the current picture was deinterlaced using a bottom field picture as the associated source field", 
						"indicates that the current picture was deinterlaced using a top field picture as the associated source field");
					DECLARE_FIELDPROP_END()
				}PACKED;

				int			payload_type;
				int			payload_size;
				uint8_t		nal_unit_type;

				union {
					RESERVED_SEI_MESSAGE*				reserved_sei_message = nullptr;
					BUFFERING_PERIOD*					buffering_period;
					PIC_TIMING_H264*					pic_timing_h264;
					PIC_TIMING_H265*					pic_timing_h265;
					MASTERING_DISPLAY_COLOUR_VOLUME*	mastering_display_colour_volume;
					ACTIVE_PARAMETER_SETS*				active_parameter_sets;
					RECOVERY_POINT*						recovery_point;
					RECOVERY_POINT_H264*				recovery_point_h264;
					PAN_SCAN_RECT*						pan_scan_rect;
					USER_DATA_REGISTERED_ITU_T_T35*		user_data_registered_itu_t_t35;
					USER_DATA_UNREGISTERED*				user_data_unregistered;
					SCENE_INFO*							scene_info;
					PICTURE_SNAPSHOT*					picture_snapshot;
					PROGRESSIVE_REFINEMENT_SEGMENT_START*
														progressive_refinement_segment_start;
					PROGRESSIVE_REFINEMENT_SEGMENT_END*	progressive_refinement_segment_end;
					FILM_GRAIN_CHARACTERISTICS*			film_grain_characteristics;
					TONE_MAPPING_INFO*					tone_mapping_info;
					FRAME_PACKING_ARRANGEMENT*			frame_packing_arrangement;
					DISPLAY_ORIENTATION*				display_orientation;
					STRUCTURE_OF_PICTURES_INFO*			structure_of_pictures_info;
					TEMPORAL_SUB_LAYER_ZERO_INDEX*		temporal_sub_layer_zero_index;
					COLOUR_REMAPPING_INFO*				colour_remapping_info;
					DEINTERLACED_FIELD_INDENTIFICATION*	deinterlaced_field_indentification;
				};

				CAMBitArray	reserved_payload_extension_data_bits;
				uint8_t payload_bit_equal_to_one;
				CAMBitArray payload_bit_equal_to_zero;

				std::vector<uint8_t> left_bytes;
				SEI_MESSAGE* ptr_sei_message;

				SEI_PAYLOAD(uint8_t nalUnitType, int payloadType, int payloadSize, SEI_MESSAGE* pSEIMsg)
					: payload_type(payloadType), payload_size(payloadSize), nal_unit_type(nalUnitType), reserved_sei_message(NULL), payload_bit_equal_to_one(0), ptr_sei_message(pSEIMsg){
				}

				virtual ~SEI_PAYLOAD() {
					switch (payload_type)
					{
					case SEI_PAYLOAD_BUFFERING_PERIOD:
						UNMAP_STRUCT_POINTER5(buffering_period);
						break;
					case SEI_PAYLOAD_PIC_TIMING:
						if (ptr_sei_message != nullptr)
						{
							INALContext* pNALCtx = ptr_sei_message->ptr_sei_rbsp->ctx_NAL;
							NAL_CODING nal_coding_type = pNALCtx->GetNALCoding();
							if (nal_coding_type == NAL_CODING_AVC)
							{
								UNMAP_STRUCT_POINTER5(pic_timing_h264);
							}
							else if (nal_coding_type == NAL_CODING_HEVC)
							{
								UNMAP_STRUCT_POINTER5(pic_timing_h265);
							}
						}
						else
						{
							UNMAP_STRUCT_POINTER5(reserved_sei_message);
						}
						break;
					case SEI_PAYLOAD_PAN_SCAN_RECT:
						UNMAP_STRUCT_POINTER5(pan_scan_rect);
						break;
					case SEI_PAYLOAD_FILLER_PAYLOAD:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_USER_DATA_REGISTERED_ITU_T_T35:
						UNMAP_STRUCT_POINTER5(user_data_registered_itu_t_t35);
						break;
					case SEI_PAYLOAD_USER_DATA_UNREGISTERED:
						UNMAP_STRUCT_POINTER5(user_data_unregistered);
						break;
					case SEI_PAYLOAD_RECOVERY_POINT:
					{
						if (ptr_sei_message && ptr_sei_message->ptr_sei_rbsp && ptr_sei_message->ptr_sei_rbsp->ctx_NAL)
						{
							NAL_CODING coding = ptr_sei_message->ptr_sei_rbsp->ctx_NAL->GetNALCoding();
							if (coding == NAL_CODING_HEVC) {
								UNMAP_STRUCT_POINTER5(recovery_point);
							}
							else if (coding == NAL_CODING_AVC) {
								UNMAP_STRUCT_POINTER5(recovery_point_h264);
							}
						}
						break;
					}
					case SEI_PAYLOAD_DEC_REF_PIC_MARKING_REPETITION:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SPARE_PIC:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SCENE_INFO:
						UNMAP_STRUCT_POINTER5(scene_info);
						break;
					case SEI_PAYLOAD_SUB_SEQ_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_SEQ_LAYER_CHARACTERISTICS:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_SEQ_CHARACTERISTICS:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FULL_FRAME_FREEZE:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FULL_FRAME_FREEZE_RELEASE:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_PICTURE_SNAPSHOT:
						UNMAP_STRUCT_POINTER5(picture_snapshot);
						break;
					case SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_START:
						UNMAP_STRUCT_POINTER5(progressive_refinement_segment_start);
						break;
					case SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_END:
						UNMAP_STRUCT_POINTER5(progressive_refinement_segment_end);
						break;
					case SEI_PAYLOAD_MOTION_CONSTRAINED_SLICE_GROUP_SET:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FILM_GRAIN_CHARACTERISTICS:
						UNMAP_STRUCT_POINTER5(film_grain_characteristics);
						break;
					case SEI_PAYLOAD_DEBLOCKING_FILTER_DISPLAY_PREFERENCE:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_STEREO_VIDEO_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_POST_FILTER_HINT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TONE_MAPPING_INFO:
						UNMAP_STRUCT_POINTER5(tone_mapping_info);
						break;
					case SEI_PAYLOAD_SCALABILITY_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_PIC_SCALABLE_LAYER:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_NON_REQUIRED_LAYER_REP:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_PRIORITY_LAYER_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_LAYERS_NOT_PRESENT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_LAYER_DEPENDENCY_CHANGE:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_SCALABLE_NESTING:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BASE_LAYER_TEMPORAL_HRD:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_QUALITY_LAYER_INTEGRITY_CHECK:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_REDUNDANT_PIC_PROPERTY:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TL0_DEP_REP_INDEX:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TL_SWITCHING_POINT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_PARALLEL_DECODING_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_MVC_SCALABLE_NESTING:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_VIEW_SCALABILITY_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_MULTIVIEW_SCENE_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_MULTIVIEW_ACQUISITION_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_NON_REQUIRED_VIEW_COMPONENT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_VIEW_DEPENDENCY_CHANGE:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_OPERATION_POINTS_NOT_PRESENT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BASE_VIEW_TEMPORAL_HRD:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FRAME_PACKING_ARRANGEMENT:
						UNMAP_STRUCT_POINTER5(frame_packing_arrangement);
						break;
					case SEI_PAYLOAD_H264_MULTIVIEW_VIEW_POSITION:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_DISPLAY_ORIENTATION:
						UNMAP_STRUCT_POINTER5(display_orientation);
						break;
					case SEI_PAYLOAD_MVCD_SCALABLE_NESTING:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_MVCD_VIEW_SCALABILITY_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_DEPTH_REPRESENTATION_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_DEPTH_TIMING:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_DEPTH_SAMPLING_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_STRUCTURE_OF_PICTURES_INFO:
						UNMAP_STRUCT_POINTER5(structure_of_pictures_info);
						break;
					case SEI_PAYLOAD_ACTIVE_PARAMETER_SETS:
						UNMAP_STRUCT_POINTER5(active_parameter_sets);
						break;
					case SEI_PAYLOAD_DECODING_UNIT_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TEMPORAL_SUB_LAYER_ZERO_INDEX:
						UNMAP_STRUCT_POINTER5(temporal_sub_layer_zero_index);
						break;
					case SEI_PAYLOAD_HEVC_SCALABLE_NESTING:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_REGION_REFRESH_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_NO_DISPLAY:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TIME_CODE:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_MASTERING_DISPLAY_COLOUR_VOLUME:
						UNMAP_STRUCT_POINTER5(mastering_display_colour_volume);
						break;
					case SEI_PAYLOAD_SEGMENTED_RECT_FRAME_PACKING_ARRANGEMENT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TEMPORAL_MOTION_CONSTRAINED_TILE_SETS:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_CHROMA_RESAMPLING_FILTER_HINT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_KNEE_FUNCTION_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_COLOUR_REMAPPING_INFO:
						UNMAP_STRUCT_POINTER5(colour_remapping_info);
						break;
					case SEI_PAYLOAD_DEINTERLACED_FIELD_IDENTIFICATION:
						UNMAP_STRUCT_POINTER5(deinterlaced_field_indentification);
						break;
					case SEI_PAYLOAD_HEVC_LAYERS_NOT_PRESENT:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_INTER_LAYER_CONSTRAINED_TILE_SETS:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BSP_NESTING:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BSP_INITIAL_ARRIVAL_TIME:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_BITSTREAM_PROPERTY:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_ALPHA_CHANNEL_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_OVERLAY_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TEMPORAL_MV_PREDICTION_CONSTRAINTS:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FRAME_FIELD_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_DEPTH_REPRESENTATION_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_MULTIVIEW_SCENE_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_MULTIVIEW_ACQUISITION_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_MULTIVIEW_VIEW_POSITION:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					case SEI_PAYLOAD_ALTERNATIVE_DEPTH_INFO:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
						break;
					default:
						UNMAP_STRUCT_POINTER5(reserved_sei_message);
					}
				}

				int Map(AMBst in_bst)
				{
					int iRet = RET_CODE_SUCCESS;
					SYNTAX_BITSTREAM_MAP::Map(in_bst);

					try
					{
						MAP_BST_BEGIN(0);

						switch (payload_type)
						{
						case SEI_PAYLOAD_BUFFERING_PERIOD:
							nal_read_ref(in_bst, buffering_period, BUFFERING_PERIOD, payload_size, ptr_sei_message->ptr_sei_rbsp->ctx_NAL);
							break;
						case SEI_PAYLOAD_PIC_TIMING:
							if (ptr_sei_message != nullptr)
							{
								INALContext* pNALCtx = ptr_sei_message->ptr_sei_rbsp->ctx_NAL;
								NAL_CODING nal_coding_type = pNALCtx->GetNALCoding();
								if (nal_coding_type == NAL_CODING_AVC)
								{
									nal_read_ref(in_bst, pic_timing_h264, PIC_TIMING_H264, payload_size, pNALCtx);
								}
								else if (nal_coding_type == NAL_CODING_HEVC)
								{
									nal_read_ref(in_bst, pic_timing_h265, PIC_TIMING_H265, payload_size, pNALCtx);
								}
							}
							else
							{
								nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							}
							break;
						case SEI_PAYLOAD_PAN_SCAN_RECT:
							nal_read_ref(in_bst, pan_scan_rect, PAN_SCAN_RECT);
							break;
						case SEI_PAYLOAD_FILLER_PAYLOAD:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_USER_DATA_REGISTERED_ITU_T_T35:
							nal_read_ref(in_bst, user_data_registered_itu_t_t35, USER_DATA_REGISTERED_ITU_T_T35, this);
							break;
						case SEI_PAYLOAD_USER_DATA_UNREGISTERED:
							nal_read_ref(in_bst, user_data_unregistered, USER_DATA_UNREGISTERED, this);
							break;
						case SEI_PAYLOAD_RECOVERY_POINT:
						{
							if (ptr_sei_message && ptr_sei_message->ptr_sei_rbsp && ptr_sei_message->ptr_sei_rbsp->ctx_NAL)
							{
								NAL_CODING coding = ptr_sei_message->ptr_sei_rbsp->ctx_NAL->GetNALCoding();
								if (coding == NAL_CODING_HEVC) {
									nal_read_ref(in_bst, recovery_point, RECOVERY_POINT);
								}
								else if (coding == NAL_CODING_AVC) {
									nal_read_ref(in_bst, recovery_point_h264, RECOVERY_POINT_H264);
								}
							}
							break;
						}
						case SEI_PAYLOAD_DEC_REF_PIC_MARKING_REPETITION:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_SPARE_PIC:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_SCENE_INFO:
							nal_read_ref(in_bst, scene_info, SCENE_INFO);
							break;
						case SEI_PAYLOAD_SUB_SEQ_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_SUB_SEQ_LAYER_CHARACTERISTICS:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_SUB_SEQ_CHARACTERISTICS:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_FULL_FRAME_FREEZE:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_FULL_FRAME_FREEZE_RELEASE:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_PICTURE_SNAPSHOT:
							nal_read_ref(in_bst, picture_snapshot, PICTURE_SNAPSHOT);
							break;
						case SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_START:
							nal_read_ref(in_bst, progressive_refinement_segment_start, PROGRESSIVE_REFINEMENT_SEGMENT_START);
							break;
						case SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_END:
							nal_read_ref(in_bst, progressive_refinement_segment_end, PROGRESSIVE_REFINEMENT_SEGMENT_END);
							break;
						case SEI_PAYLOAD_MOTION_CONSTRAINED_SLICE_GROUP_SET:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_FILM_GRAIN_CHARACTERISTICS:
							nal_read_ref(in_bst, film_grain_characteristics, FILM_GRAIN_CHARACTERISTICS);
							break;
						case SEI_PAYLOAD_DEBLOCKING_FILTER_DISPLAY_PREFERENCE:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_STEREO_VIDEO_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_POST_FILTER_HINT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TONE_MAPPING_INFO:
							nal_read_ref(in_bst, tone_mapping_info, TONE_MAPPING_INFO);
							break;
						case SEI_PAYLOAD_SCALABILITY_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_SUB_PIC_SCALABLE_LAYER:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_NON_REQUIRED_LAYER_REP:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_PRIORITY_LAYER_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_H264_LAYERS_NOT_PRESENT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_LAYER_DEPENDENCY_CHANGE:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_H264_SCALABLE_NESTING:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_BASE_LAYER_TEMPORAL_HRD:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_QUALITY_LAYER_INTEGRITY_CHECK:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_REDUNDANT_PIC_PROPERTY:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TL0_DEP_REP_INDEX:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TL_SWITCHING_POINT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_PARALLEL_DECODING_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_MVC_SCALABLE_NESTING:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_VIEW_SCALABILITY_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_H264_MULTIVIEW_SCENE_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_H264_MULTIVIEW_ACQUISITION_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_NON_REQUIRED_VIEW_COMPONENT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_VIEW_DEPENDENCY_CHANGE:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_OPERATION_POINTS_NOT_PRESENT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_BASE_VIEW_TEMPORAL_HRD:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_FRAME_PACKING_ARRANGEMENT:
							nal_read_ref(in_bst, frame_packing_arrangement, FRAME_PACKING_ARRANGEMENT);
							break;
						case SEI_PAYLOAD_H264_MULTIVIEW_VIEW_POSITION:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_DISPLAY_ORIENTATION:
							nal_read_ref(in_bst, display_orientation, DISPLAY_ORIENTATION);
							break;
						case SEI_PAYLOAD_MVCD_SCALABLE_NESTING:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_MVCD_VIEW_SCALABILITY_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_H264_DEPTH_REPRESENTATION_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_H264_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_DEPTH_TIMING:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_DEPTH_SAMPLING_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_STRUCTURE_OF_PICTURES_INFO:
							nal_read_ref(in_bst, structure_of_pictures_info, STRUCTURE_OF_PICTURES_INFO);
							break;
						case SEI_PAYLOAD_ACTIVE_PARAMETER_SETS:
							nal_read_ref(in_bst, active_parameter_sets, ACTIVE_PARAMETER_SETS, this);
							break;
						case SEI_PAYLOAD_DECODING_UNIT_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TEMPORAL_SUB_LAYER_ZERO_INDEX:
							nal_read_ref(in_bst, temporal_sub_layer_zero_index, TEMPORAL_SUB_LAYER_ZERO_INDEX);
							break;
						case SEI_PAYLOAD_HEVC_SCALABLE_NESTING:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_REGION_REFRESH_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_NO_DISPLAY:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TIME_CODE:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_MASTERING_DISPLAY_COLOUR_VOLUME:
							nal_read_ref(in_bst, mastering_display_colour_volume, MASTERING_DISPLAY_COLOUR_VOLUME, payload_size);
							break;
						case SEI_PAYLOAD_SEGMENTED_RECT_FRAME_PACKING_ARRANGEMENT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TEMPORAL_MOTION_CONSTRAINED_TILE_SETS:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_CHROMA_RESAMPLING_FILTER_HINT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_KNEE_FUNCTION_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_COLOUR_REMAPPING_INFO:
							nal_read_ref(in_bst, colour_remapping_info, COLOUR_REMAPPING_INFO);
							break;
						case SEI_PAYLOAD_DEINTERLACED_FIELD_IDENTIFICATION:
							nal_read_ref(in_bst, deinterlaced_field_indentification, DEINTERLACED_FIELD_INDENTIFICATION);
							break;
						case SEI_PAYLOAD_HEVC_LAYERS_NOT_PRESENT:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_INTER_LAYER_CONSTRAINED_TILE_SETS:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_BSP_NESTING:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_BSP_INITIAL_ARRIVAL_TIME:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_SUB_BITSTREAM_PROPERTY:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_ALPHA_CHANNEL_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_OVERLAY_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_TEMPORAL_MV_PREDICTION_CONSTRAINTS:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_FRAME_FIELD_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_HEVC_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_HEVC_DEPTH_REPRESENTATION_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_HEVC_MULTIVIEW_SCENE_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_HEVC_MULTIVIEW_ACQUISITION_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_HEVC_MULTIVIEW_VIEW_POSITION:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						case SEI_PAYLOAD_ALTERNATIVE_DEPTH_INFO:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
							break;
						default:
							nal_read_ref(in_bst, reserved_sei_message, RESERVED_SEI_MESSAGE, payload_size);
						}

						if (!AMBst_IsAligned(in_bst) || (AMBst_Tell(in_bst) - bit_pos) < 8 * payload_size) {
							int idx = 0;
							while (!AMBst_PeekBits(in_bst, 1)) {
								AMBst_GetBits(in_bst, 1) ? reserved_payload_extension_data_bits.BitSet(idx) : reserved_payload_extension_data_bits.BitClear(idx);
								idx++;
							}
							map_status.number_of_fields += idx;

							payload_bit_equal_to_one = (uint8_t)AMBst_GetBits(in_bst, 1);
							map_status.number_of_fields++;

							idx = 0;
							while (!AMBst_IsAligned(in_bst))
							{
								AMBst_GetBits(in_bst, 1) ? payload_bit_equal_to_zero.BitSet(idx) : payload_bit_equal_to_zero.BitClear(idx);
								idx++;
							}
							map_status.number_of_fields += idx;
						}

						int left_bits = payload_size*8 - (AMBst_Tell(in_bst) - bit_pos);
						assert(left_bits < 0 || (left_bits >= 0 && left_bits % 8 == 0));

						if (left_bits > 0)
						{
							left_bytes.resize(left_bits / 8);
							AMBst_GetBytes(in_bst, left_bytes.data(), left_bits / 8);
						}

						MAP_BST_END();
					}
					catch (AMException e)
					{
						if (e.RetCode() == RET_CODE_NO_MORE_DATA) {
							MAP_BST_END();
							return RET_CODE_SUCCESS;
						}
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
				switch (payload_type)
				{
					case SEI_PAYLOAD_BUFFERING_PERIOD:
						BST_FIELD_PROP_REF(buffering_period);
						break;
					case SEI_PAYLOAD_PIC_TIMING:
						if (ptr_sei_message != nullptr)
						{
							INALContext* pNALCtx = ptr_sei_message->ptr_sei_rbsp->ctx_NAL;
							NAL_CODING nal_coding_type = pNALCtx->GetNALCoding();
							if (nal_coding_type == NAL_CODING_AVC)
							{
								BST_FIELD_PROP_REF(pic_timing_h264);
							}
							else if (nal_coding_type == NAL_CODING_HEVC)
							{
								BST_FIELD_PROP_REF(pic_timing_h265);
							}
						}
						else
						{
							BST_FIELD_PROP_REF(reserved_sei_message);
						}
						break;
					case SEI_PAYLOAD_PAN_SCAN_RECT:
						BST_FIELD_PROP_REF(pan_scan_rect);
						break;
					case SEI_PAYLOAD_FILLER_PAYLOAD:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_USER_DATA_REGISTERED_ITU_T_T35:
						BST_FIELD_PROP_REF(user_data_registered_itu_t_t35);
						break;
					case SEI_PAYLOAD_USER_DATA_UNREGISTERED:
						BST_FIELD_PROP_REF(user_data_unregistered);
						break;
					case SEI_PAYLOAD_RECOVERY_POINT:
					{
						if (ptr_sei_message && ptr_sei_message->ptr_sei_rbsp && ptr_sei_message->ptr_sei_rbsp->ctx_NAL)
						{
							NAL_CODING coding = ptr_sei_message->ptr_sei_rbsp->ctx_NAL->GetNALCoding();
							if (coding == NAL_CODING_HEVC) {
								BST_FIELD_PROP_REF(recovery_point);
							}
							else if (coding == NAL_CODING_AVC) {
								BST_FIELD_PROP_REF(recovery_point_h264);
							}
						}

						break;
					}
					case SEI_PAYLOAD_DEC_REF_PIC_MARKING_REPETITION:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SPARE_PIC:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SCENE_INFO:
						BST_FIELD_PROP_REF(scene_info);
						break;
					case SEI_PAYLOAD_SUB_SEQ_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_SEQ_LAYER_CHARACTERISTICS:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_SEQ_CHARACTERISTICS:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FULL_FRAME_FREEZE:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FULL_FRAME_FREEZE_RELEASE:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_PICTURE_SNAPSHOT:
						BST_FIELD_PROP_REF(picture_snapshot);
						break;
					case SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_START:
						BST_FIELD_PROP_REF(progressive_refinement_segment_start);
						break;
					case SEI_PAYLOAD_PROGRESSIVE_REFINEMENT_SEGMENT_END:
						BST_FIELD_PROP_REF(progressive_refinement_segment_end);
						break;
					case SEI_PAYLOAD_MOTION_CONSTRAINED_SLICE_GROUP_SET:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FILM_GRAIN_CHARACTERISTICS:
						BST_FIELD_PROP_REF(film_grain_characteristics);
						break;
					case SEI_PAYLOAD_DEBLOCKING_FILTER_DISPLAY_PREFERENCE:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_STEREO_VIDEO_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_POST_FILTER_HINT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TONE_MAPPING_INFO:
						BST_FIELD_PROP_REF(tone_mapping_info);
						break;
					case SEI_PAYLOAD_SCALABILITY_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_PIC_SCALABLE_LAYER:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_NON_REQUIRED_LAYER_REP:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_PRIORITY_LAYER_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_LAYERS_NOT_PRESENT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_LAYER_DEPENDENCY_CHANGE:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_SCALABLE_NESTING:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BASE_LAYER_TEMPORAL_HRD:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_QUALITY_LAYER_INTEGRITY_CHECK:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_REDUNDANT_PIC_PROPERTY:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TL0_DEP_REP_INDEX:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TL_SWITCHING_POINT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_PARALLEL_DECODING_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_MVC_SCALABLE_NESTING:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_VIEW_SCALABILITY_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_MULTIVIEW_SCENE_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_MULTIVIEW_ACQUISITION_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_NON_REQUIRED_VIEW_COMPONENT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_VIEW_DEPENDENCY_CHANGE:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_OPERATION_POINTS_NOT_PRESENT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BASE_VIEW_TEMPORAL_HRD:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FRAME_PACKING_ARRANGEMENT:
						BST_FIELD_PROP_REF(frame_packing_arrangement);
						break;
					case SEI_PAYLOAD_H264_MULTIVIEW_VIEW_POSITION:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_DISPLAY_ORIENTATION:
						BST_FIELD_PROP_REF(display_orientation);
						break;
					case SEI_PAYLOAD_MVCD_SCALABLE_NESTING:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_MVCD_VIEW_SCALABILITY_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_DEPTH_REPRESENTATION_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_H264_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_DEPTH_TIMING:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_DEPTH_SAMPLING_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_STRUCTURE_OF_PICTURES_INFO:
						BST_FIELD_PROP_REF(structure_of_pictures_info);
						break;
					case SEI_PAYLOAD_ACTIVE_PARAMETER_SETS:
						BST_FIELD_PROP_REF(active_parameter_sets);
						break;
					case SEI_PAYLOAD_DECODING_UNIT_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TEMPORAL_SUB_LAYER_ZERO_INDEX:
						BST_FIELD_PROP_REF(temporal_sub_layer_zero_index);
						break;
					case SEI_PAYLOAD_HEVC_SCALABLE_NESTING:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_REGION_REFRESH_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_NO_DISPLAY:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TIME_CODE:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_MASTERING_DISPLAY_COLOUR_VOLUME:
						BST_FIELD_PROP_REF(mastering_display_colour_volume);
						break;
					case SEI_PAYLOAD_SEGMENTED_RECT_FRAME_PACKING_ARRANGEMENT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TEMPORAL_MOTION_CONSTRAINED_TILE_SETS:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_CHROMA_RESAMPLING_FILTER_HINT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_KNEE_FUNCTION_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_COLOUR_REMAPPING_INFO:
						BST_FIELD_PROP_REF(colour_remapping_info);
						break;
					case SEI_PAYLOAD_DEINTERLACED_FIELD_IDENTIFICATION:
						BST_FIELD_PROP_REF(deinterlaced_field_indentification);
						break;
					case SEI_PAYLOAD_HEVC_LAYERS_NOT_PRESENT:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_INTER_LAYER_CONSTRAINED_TILE_SETS:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BSP_NESTING:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_BSP_INITIAL_ARRIVAL_TIME:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_SUB_BITSTREAM_PROPERTY:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_ALPHA_CHANNEL_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_OVERLAY_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_TEMPORAL_MV_PREDICTION_CONSTRAINTS:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_FRAME_FIELD_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_THREE_DIMENSIONAL_REFERENCE_DISPLAYS_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_DEPTH_REPRESENTATION_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_MULTIVIEW_SCENE_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_MULTIVIEW_ACQUISITION_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_HEVC_MULTIVIEW_VIEW_POSITION:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					case SEI_PAYLOAD_ALTERNATIVE_DEPTH_INFO:
						BST_FIELD_PROP_REF(reserved_sei_message);
						break;
					default:
						BST_FIELD_PROP_REF(reserved_sei_message);
				}

				if (reserved_payload_extension_data_bits.UpperBound() >= 0)
				{
					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i=0;i&lt;reserved_payload_extension_data.length;i++)", "");
					for (int i = 0; i <= reserved_payload_extension_data_bits.UpperBound(); i++) {
						BST_ARRAY_FIELD_PROP_NUMBER("reserved_payload_extension_data_bit", i, 1, reserved_payload_extension_data_bits[i], "");
					}
					NAV_WRITE_TAG_END("Tag0");
				}

				if (payload_bit_equal_to_one) {
					BST_FIELD_PROP_NUMBER1(payload_bit_equal_to_one, 1, "");

					NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "while(!byte_aligned())", "");
					for (int i = 0; i <= payload_bit_equal_to_zero.UpperBound(); i++) {
						BST_ARRAY_FIELD_PROP_NUMBER("payload_bit_equal_to_zero", i, 1, payload_bit_equal_to_zero[i], "");
					}
					NAV_WRITE_TAG_END("Tag1");
				}

				DECLARE_FIELDPROP_END()

			};

			std::vector<uint8_t>		type_ff_bytes;
			uint8_t						last_payload_type_byte = 0;
			std::vector<uint8_t>		size_ff_bytes;
			uint8_t						last_payload_size_byte = 0;
			SEI_PAYLOAD*				sei_payload;
			uint8_t						nal_unit_type;
			SEI_RBSP*					ptr_sei_rbsp;

			SEI_MESSAGE(uint8_t nalUnitType, SEI_RBSP* pSEIRBSP) 
				: sei_payload(NULL), nal_unit_type(nalUnitType), ptr_sei_rbsp(pSEIRBSP){
			}

			virtual ~SEI_MESSAGE() {
				UNMAP_STRUCT_POINTER5(sei_payload);
			}

			int Map(AMBst in_bst)
			{
				int iRet = RET_CODE_SUCCESS;
				SYNTAX_BITSTREAM_MAP::Map(in_bst);

				uint8_t ff_byte;
				int payloadType = 0, payloadSize = 0;

				try
				{
					MAP_BST_BEGIN(0);
					while (AMBst_PeekBits(in_bst, 8) == 0xFF) {
						nal_read_u(in_bst, ff_byte, 8, uint8_t);
						type_ff_bytes.push_back(ff_byte);
						payloadType += 255;
					}

					nal_read_u(in_bst, last_payload_type_byte, 8, uint8_t);
					payloadType += last_payload_type_byte;

					while (AMBst_PeekBits(in_bst, 8) == 0xFF) {
						nal_read_u(in_bst, ff_byte, 8, uint8_t);
						size_ff_bytes.push_back(ff_byte);
						payloadSize += 255;
					}

					nal_read_u(in_bst, last_payload_size_byte, 8, uint8_t);
					payloadSize += last_payload_size_byte;

					int end_pos = AMBst_Tell(in_bst);
					if (end_pos > 0)
						AMBst_MarkEndPos(in_bst, end_pos + payloadSize);

					nal_read_ref(in_bst, sei_payload, SEI_PAYLOAD, nal_unit_type, payloadType, payloadSize, this);

					MAP_BST_END();
					AMBst_MarkEndPos(in_bst, -1);
				}
				catch (AMException e)
				{
					AMBst_MarkEndPos(in_bst, -1);
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
			int payloadType = 0, payloadSize = 0;
			if (type_ff_bytes.size() > 0) {
				payloadType += 255 * (int)type_ff_bytes.size();
				field_prop_idx += (int)type_ff_bytes.size();
			}

			if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields)) {
				payloadType += last_payload_type_byte;
				field_prop_idx++;
			}
			else
				return cbRequired;

			const char* szSEIMsgName = payloadType >= (int)(sizeof(sei_payload_type_names) / sizeof(char*)) ? "reserved_sei_message" : sei_payload_type_names[payloadType];
			NAV_WRITE_TAG_BEGIN2_1("sei_message", szSEIMsgName);				

			if (type_ff_bytes.size() > 0) {
				NAV_FIELD_PROP_FIXSIZE_BINSTR("ff_bytes", ((long long)type_ff_bytes.size() * 8), type_ff_bytes.data(), (unsigned long)type_ff_bytes.size(), "");
			}

			NAV_FIELD_PROP_2NUMBER1(last_payload_type_byte, 8, "");

			if (size_ff_bytes.size() > 0) {
				NAV_FIELD_PROP_FIXSIZE_BINSTR("ff_bytes", ((long long)size_ff_bytes.size() * 8), size_ff_bytes.data(), (unsigned long)size_ff_bytes.size(), "");
				payloadSize += 255 * (int)size_ff_bytes.size();
				field_prop_idx += (int)size_ff_bytes.size();
			}

			if (map_status.status == 0 || (map_status.error == 0 && map_status.number_of_fields > 0 && field_prop_idx < map_status.number_of_fields)) {
				NAV_FIELD_PROP_2NUMBER1(last_payload_size_byte, 8, "");
				payloadSize += last_payload_size_byte;
				field_prop_idx++;
			}

			MBCSPRINTF_S(szTagName, TAGNAME_SIZE, "%s(size: %d)", szSEIMsgName, payloadSize);

			NAV_WRITE_TAG_BEGIN_WITH_ALIAS("sei_payload", szTagName, "");
			BST_FIELD_PROP_REF(sei_payload);
			NAV_WRITE_TAG_END("sei_payload");

			NAV_WRITE_TAG_END2("sei_message");
			DECLARE_FIELDPROP_END()

		};

		std::vector<SEI_MESSAGE*>		sei_messages;
		uint8_t							nal_unit_type;

		RBSP_TRAILING_BITS				rbsp_trailing_bits;
		INALContext*					ctx_NAL;

		SEI_RBSP(uint8_t nalUnitType, INALContext* pCtx)
			: nal_unit_type(nalUnitType), ctx_NAL(pCtx){
		}

		virtual ~SEI_RBSP() {
			for (size_t i = 0; i<sei_messages.size(); i++)
			{
				AMP_SAFEDEL(sei_messages[i]);
			}
		}

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			SEI_MESSAGE* ptr_sei_message = NULL;

			MAP_BST_BEGIN(0);

			bool b_stop_one_bit = false;

			try
			{
				do
				{
					b_stop_one_bit = false;
					AMP_NEWT(ptr_sei_message, SEI_MESSAGE, nal_unit_type, this);
					if (AMP_FAILED(iRet = ptr_sei_message->Map(in_bst)))
						printf("[H265] Failed to map SEI message {retcode: %d}.\n", iRet);

					sei_messages.push_back(ptr_sei_message);
					ptr_sei_message = NULL;

					if (iRet == RET_CODE_NO_MORE_DATA)
					{
						iRet = RET_CODE_SUCCESS;
						break;
					}

					b_stop_one_bit = AMBst_PeekBits(in_bst, 1) ? true : false;

				} while (AMP_SUCCEEDED(iRet) && !b_stop_one_bit);

				if (AMP_SUCCEEDED(iRet) && b_stop_one_bit) {
					nal_read_obj(in_bst, rbsp_trailing_bits);
				}
			}
			catch (AMException e)
			{
				AMP_SAFEDEL2(ptr_sei_message);
				return e.RetCode();
			}

			MAP_BST_END();

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			for (i = 0; i < (int)sei_messages.size(); i++) {
				NAV_FIELD_PROP_REF(sei_messages[i]);
			}
			if (rbsp_trailing_bits.rbsp_trailing_bits.UpperBound() >= 0) {
				NAV_FIELD_PROP_OBJECT(rbsp_trailing_bits);
			}
		DECLARE_FIELDPROP_END()

	};

}	// namespace;


#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
