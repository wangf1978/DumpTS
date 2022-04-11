#ifndef _MPEG2_DESCRIPTOR_13818_1_H_
#define _MPEG2_DESCRIPTOR_13818_1_H_

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "systemdef.h"
#include "DumpUtil.h"
#include "AMArray.h"
#include "ISO14496_3.h"
#include "descriptors_hdmv.h"

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

// descriptor tag list defined in ISO-13818-1
#define DT_reserved										0x00
#define DT_forbidden									0x01
#define DT_video_stream_descriptor						0x02
#define DT_audio_stream_descriptor						0x03
#define DT_hierarchy_descriptor							0x04
#define DT_registration_descriptor						0x05
#define DT_data_stream_alignment_descriptor				0x06
#define DT_target_background_grid_descriptor			0x07
#define DT_video_window_descriptor						0x08
#define DT_CA_descriptor								0x09
#define DT_ISO_639_language_descriptor					0x0A
#define DT_system_clock_descriptor						0x0B
#define DT_multiplex_buffer_utilization_descriptor		0x0C
#define DT_copyright_descriptor							0x0D
#define DT_maximum_bitrate_descriptor					0x0E
#define DT_private_data_indicator_descriptor			0x0F
#define DT_smoothing_buffer_descriptor					0x10
#define DT_STD_descriptor								0x11
#define DT_IBP_descriptor								0x12
//#define DT_Defined in ISO/IEC 13818-6					0x13
#define DT_MPEG4_video_descriptor						0x1B
#define DT_MPEG4_audio_descriptor						0x1C
#define DT_IOD_descriptor								0x1D
#define DT_SL_descriptor								0x1E
#define DT_FMC_descriptor								0x1F
#define DT_external_ES_ID_descriptor					0x20
#define DT_MuxCode_descriptor							0x21
#define DT_FmxBufferSize_descriptor						0x22
#define DT_multiplexbuffer_descriptor					0x23
#define DT_content_labeling_descriptor					0x24
#define DT_metadata_pointer_descriptor					0x25
#define DT_metadata_descriptor							0x26
#define DT_metadata_STD_descriptor						0x27
#define DT_AVC_video_descriptor							0x28
#define DT_IPMP_descriptor								0x29
#define DT_AVC_timing_and_HRD_descriptor				0x2A
#define DT_MPEG2_AAC_audio_descriptor					0x2B
#define DT_FlexMuxTiming_descriptor						0x2C
#define DT_MPEG4_text_descriptor						0x2D
#define DT_MPEG4_audio_extension_descriptor				0x2E
#define DT_auxiliary_video_stream_descriptor			0x2F
#define DT_SVC_extension_descriptor						0x30
#define DT_MVC_extension_descriptor						0x31
#define DT_J2K_video_descriptor							0x32
#define DT_MVC_operation_point_descriptor				0x33
#define DT_MPEG2_stereoscopic_video_format_descriptor	0x34
#define DT_Stereoscopic_program_info_descriptor			0x35
#define DT_Stereoscopic_video_info_descriptor			0x36
#define DT_Transport_profile_descriptor					0x37
#define DT_HEVC_descriptor								0x38

extern const char* descriptor_tag_names[256];
extern const char* frame_rate_value_names[16];
extern const char* frame_rate_multiple_value_names[16];
extern const char* profile_and_level_identification_names[8][16];
extern const char* get_profile_and_level_indication_names(int profile_and_level_indication);
extern const char* chroma_format_names[4];
extern const char* hierarchy_type_names[16];
extern const char* MPEG4_visual_profile_and_level_names[256];
extern const char* MPEG4_audio_profile_and_level_names[256];

const inline char* get_h264_profile_name(unsigned char profile_idc, 
										 unsigned char constraint_set0_flag, 
										 unsigned char constraint_set1_flag, 
										 unsigned char constraint_set2_flag, 
										 unsigned char constraint_set3_flag, 
										 unsigned char constraint_set4_flag, 
										 unsigned char constraint_set5_flag)
{
	switch(profile_idc)
	{
	case 66: 
		return constraint_set1_flag?"Constrained Baseline profile":"Baseline profile";
	case 77:
		return "Main Profile";
	case 88:
		return "Extended profile";
	case 100:
		return (constraint_set4_flag && constraint_set5_flag)?"Constrained High Profile":(constraint_set4_flag?"Progressive High Profile":"High Profile");
	case 110:
		return constraint_set3_flag?"High 10 Intra Profile":"High 10 Profile";
	case 122:
		return constraint_set3_flag?"High 4:2:2 Intra Profile":"High 4:2:2 Profile";
	case 244:
		return constraint_set3_flag?"High 4:4:4 Intra Profile":"High 4:4:4 Predictive Profile";
	case 44:
		return "CAVLC 4:4:4 Intra Profile";
	case 83:
		return constraint_set5_flag?"Scalable Constrained Baseline Profile":"Scalable Baseline Profile";
	case 86:
		return constraint_set5_flag?"Scalable Constrained High Profile":(constraint_set3_flag?"Scalable High Intra Profile":"Scalable High Profile");
	case 128:
		return "Stereo High Profile";
	case 118:
		return "Multiview High Profile";
	case 138:
		return "Multiview Depth High Profile";
	}

	return "Unknown Profile";
}

const inline char* get_h264_level_name(unsigned char level_idc, 
									   unsigned char constraint_set0_flag, 
									   unsigned char constraint_set1_flag, 
									   unsigned char constraint_set2_flag, 
									   unsigned char constraint_set3_flag, 
									   unsigned char constraint_set4_flag, 
									   unsigned char constraint_set5_flag)
{
/*
In bitstreams conforming to the Baseline, Constrained Baseline, Main, or Extended profiles, the conformance of the
bitstream to a specified level is indicated by the syntax elements level_idc and constraint_set3_flag as follows:
�C If level_idc is equal to 11 and constraint_set3_flag is equal to 1, the indicated level is level 1b.
�C Otherwise (level_idc is not equal to 11 or constraint_set3_flag is not equal to 1), level_idc is equal to a value of ten
times the level number (of the indicated level) specified in Table A-1.
*/
	switch(level_idc)
	{
	case 10: return "1";
	case 11: return constraint_set3_flag?"1b":"1.1";
	case 12: return "1.2";
	case 13: return "1.3";
	case 20: return "2";
	case 21: return "2.1";
	case 22: return "2.2";
	case 30: return "3";
	case 31: return "3.1";
	case 32: return "3.2";
	case 40: return "4";
	case 41: return "4.1";
	case 42: return "4.2";
	case 50: return "5.0";
	case 51: return "5.1";
	case 52: return "5.2";
	}

	return "Unknown";
}

namespace BST {

	struct CPSISection;

	// Different system has different PSI meaning, need let PSI, descriptor to know it.
	struct CPSIContext
	{
		TS_FORMAT			ts_format;
		CPSISection*		ptr_section;
		// For example, in PMT, descriptor may exist in the 1st loop, or 2nd loop, for this case, loop_idx = 0 and 1 separately
		unsigned char		loop_idx;
		unsigned char		stream_type;
	};

	struct CGeneralDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		descriptor_data[];

		int GetVarBodySize(){return descriptor_length;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(descriptor_data, (unsigned long)descriptor_length*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CVideoStreamDescriptor: public DIRECT_ENDIAN_MAP{

		struct CAdditionalInfo {
			unsigned char		profile_and_level_indication;
#ifdef _BIG_ENDIAN_
			unsigned char		chroma_format:2;
			unsigned char		frame_rate_extension_flag:1;
			unsigned char		reserved:5;
#else
			unsigned char		reserved:5;
			unsigned char		frame_rate_extension_flag:1;
			unsigned char		chroma_format:2;
#endif
		}PACKED;

		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

#ifdef _BIG_ENDIAN_
		unsigned char		multiple_frame_rate_flag:1;
		unsigned char		frame_rate_code:4;
		unsigned char		MPEG_1_only_flag:1;
		unsigned char		constrained_parameter_flag:1
		unsigned char		still_picture_flag:1;
#else
		unsigned char		still_picture_flag:1;;
		unsigned char		constrained_parameter_flag:1;
		unsigned char		MPEG_1_only_flag:1;
		unsigned char		frame_rate_code:4;
		unsigned char		multiple_frame_rate_flag:1;
#endif

		CAdditionalInfo		additional_info[];

		int GetVarBodySize(){return MPEG_1_only_flag?0:sizeof(CAdditionalInfo);}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(multiple_frame_rate_flag, 1, "indicates whether multiple frame rates may be present in the video stream")
			NAV_FIELD_PROP_2NUMBER1(frame_rate_code, 4, multiple_frame_rate_flag?frame_rate_multiple_value_names[frame_rate_code]:frame_rate_value_names[frame_rate_code])
			NAV_FIELD_PROP_2NUMBER1(MPEG_1_only_flag, 1, MPEG_1_only_flag?"only ISO/IEC 11172-2 data(MPEG-1)":"both Rec. ITU-T H.262 | ISO/IEC 13818-2 video data and constrained parameter ISO/IEC 11172-2 video data")
			NAV_FIELD_PROP_2NUMBER1(constrained_parameter_flag, 1, constrained_parameter_flag?"shall not contain unconstrained ISO/IEC 11172-2 video data":"may contain both constrained parameters and unconstrained ISO/IEC 11172-2 video streams")
			NAV_FIELD_PROP_2NUMBER1(still_picture_flag, 1, still_picture_flag?"contains only still pictures":"may contain either moving or still picture data")
			if(!MPEG_1_only_flag){
				NAV_FIELD_PROP_2NUMBER1(additional_info->profile_and_level_indication, 8, get_profile_and_level_indication_names(additional_info->profile_and_level_indication))
				NAV_FIELD_PROP_2NUMBER1(additional_info->chroma_format, 2, chroma_format_names[additional_info->chroma_format])
				NAV_FIELD_PROP_2NUMBER1(additional_info->frame_rate_extension_flag, 1, additional_info->frame_rate_extension_flag?"either or both the frame_rate_extension_n and the frame_rate_extension_d fields are non-zero in any video sequences":"")
				NAV_FIELD_PROP_2NUMBER1(additional_info->reserved, 5, "")
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CAudioStreamDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		
#ifdef _BIG_ENDIAN_
		unsigned char		free_format_flag:1;
		unsigned char		ID:1;
		unsigned char		layer:2;
		unsigned char		variable_rate_audio_indicator:1;
		unsigned char		reserved:3;
#else
		unsigned char		reserved:3;
		unsigned char		variable_rate_audio_indicator:1;
		unsigned char		layer:2;
		unsigned char		ID:1;
		unsigned char		free_format_flag:1;
#endif

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(free_format_flag, 1, free_format_flag?"that the audio stream may contain one or more audio frames with the bitrate_index set to '0000'":"bitrate_index is not '0000' in any audio frame of the audio stream");
			NAV_FIELD_PROP_2NUMBER1(ID, 1, ID?"ISO/IEC 11172-3":"extension to lower sampling frequencies");
			NAV_FIELD_PROP_2NUMBER1(layer, 2, layer==0x03?"Layer I":(layer==0x02?"Layer II":(layer==0x01?"Layer III":"reserved")));
			NAV_FIELD_PROP_2NUMBER1(variable_rate_audio_indicator, 1, variable_rate_audio_indicator?"VBR":"CBR");
			NAV_FIELD_PROP_2NUMBER1(reserved, 8, "");
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CHierarchyDescriptor: public DIRECT_ENDIAN_MAP
	{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

#ifdef _BIG_ENDIAN_
		unsigned char		reserved_0:1;
		unsigned char		temporal_scalability_flag:1;
		unsigned char		spatial_scalability_flag:1;
		unsigned char		quality_scalability_flag:1;
		unsigned char		hierarchy_type:4;

		unsigned char		reserved_1:2;
		unsigned char		hierarchy_layer_index:6;

		unsigned char		tref_present_flag:1;
		unsigned char		reserved_2:1;
		unsigned char		hierarchy_embedded_layer_index:6;

		unsigned char		reserved_3:2;
		unsigned char		hierarchy_channel:6;
#else
		unsigned char		hierarchy_type:4;
		unsigned char		quality_scalability_flag:1;
		unsigned char		spatial_scalability_flag:1;
		unsigned char		temporal_scalability_flag:1;
		unsigned char		reserved_0:1;

		unsigned char		hierarchy_layer_index:6;
		unsigned char		reserved_1:2;

		unsigned char		hierarchy_embedded_layer_index:6;
		unsigned char		reserved_2:1;
		unsigned char		tref_present_flag:1;

		unsigned char		hierarchy_channel:6;
		unsigned char		reserved_3:2;
#endif

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");

			NAV_FIELD_PROP_2NUMBER("reserved", 1, reserved_0, "");
			NAV_FIELD_PROP_2NUMBER1(temporal_scalability_flag, 1, temporal_scalability_flag?"the associated program element enhances the frame rate of the bit-stream resulting from the program element referenced by the hierarchy_embedded_layer_index.":"reserved");
			NAV_FIELD_PROP_2NUMBER1(spatial_scalability_flag, 1, spatial_scalability_flag?"the associated program element enhances the spatial resolution of the bit-stream resulting from the program element referenced by the hierarchy_embedded_layer_index":"reserved");
			NAV_FIELD_PROP_2NUMBER1(quality_scalability_flag, 1, quality_scalability_flag?"the associated program element enhances the SNR quality or fidelity of the bit-stream resulting from the program element referenced by the hierarchy_embedded_layer_index":"reserved");
			NAV_FIELD_PROP_2NUMBER1(hierarchy_type, 4, hierarchy_type_names[hierarchy_type]);

			NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved_1, "");
			NAV_FIELD_PROP_2NUMBER1(hierarchy_layer_index, 6, "");

			NAV_FIELD_PROP_2NUMBER1(tref_present_flag, 1, tref_present_flag?"the TREF field may be present in the PES packet headers in the associated elementary stream":"reserved");
			NAV_FIELD_PROP_2NUMBER ("reserved", 1, reserved_2, "");
			NAV_FIELD_PROP_2NUMBER1(hierarchy_embedded_layer_index, 6, "defines the hierarchy_layer_index of the program element that needs to be accessed and be present in decoding order before decoding");

			NAV_FIELD_PROP_2NUMBER ("reserved", 1, reserved_3, "");
			NAV_FIELD_PROP_2NUMBER1(hierarchy_channel, 6, "the intended channel number for the associated program element in an ordered set of transmission channels");
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CRegistrationDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		format_identifier[4];

		unsigned char		additional_identification_info[];

		int GetVarBodySize(){return descriptor_length - 4;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINCHARSTR("format_identifier", 32, format_identifier, 4,"a 32-bit value obtained from a Registration Authority as designated by ISO/IEC JTC 1&amp;SC 29");
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(additional_identification_info, (unsigned long)(descriptor_length - 4)*8, "defined by the assignee of that format_identifier, and once defined they shall not change")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CDataStreamAlignmentDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		alignment_type;

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(alignment_type, 8, alignment_type==0x01?"Slice, or video access unit or Audio Sync word"
				:(alignment_type==0x02?"Video access unit"
				:(alignment_type==0x03?"GOP, or SEQ, or SVC slice or SVC dependency representation"
				:(alignment_type==0x04?"SEQ or SVC dependency representation"
				:(alignment_type==0x05?"MVC slice or MVC view-component subset"
				:(alignment_type==0x06?"MVC view-component subset":""))))));
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CTargetBackgroundGridDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

		union{
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		horizontal_size:14;
				unsigned long		vertical_size:14;
				unsigned long		aspect_ratio_information:4;
#else
				unsigned long		aspect_ratio_information:4;
				unsigned long		vertical_size:14;
				unsigned long		horizontal_size:14;
#endif
			}PACKED;
			unsigned long		long_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(long_value)
		DECLARE_ENDIAN_END()
		
		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(horizontal_size, 14, "The horizontal size of the target background grid in pixels")
			NAV_FIELD_PROP_2NUMBER1(vertical_size, 14, "The vertical size of the target background grid in pixels")
			NAV_FIELD_PROP_2NUMBER1(aspect_ratio_information, 4, aspect_ratio_information==1?"SAR=1.0, DAR=vertical_size/horizontal_size"
				:(aspect_ratio_information==2?"4:3"
				:(aspect_ratio_information==3?"16:9"
				:(aspect_ratio_information==4?"1:2.21":"Unknown"))))
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CVideoWindowDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

		union{
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		horizontal_offset:14;
				unsigned long		vertical_offset:14;
				unsigned long		window_priority:4;
#else
				unsigned long		window_priority:4;
				unsigned long		vertical_offset:14;
				unsigned long		horizontal_offset:14;
#endif
			}PACKED;
			unsigned long		long_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(long_value)
		DECLARE_ENDIAN_END()
		
		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(horizontal_offset, 14, "the horizontal position of the top left pixel of the current video display window or display rectangle if indicated in the picture display extension on the target background grid for display as defined in the target_background_grid_descriptor. The top left pixel of the video window shall be one of the pixels of the target background grid")
			NAV_FIELD_PROP_2NUMBER1(vertical_offset, 14, "the vertical position of the top left pixel of the current video display window or display rectangle if indicated in the picture display extension on the target background grid for display as defined in the target_background_grid_descriptor. The top left pixel of the video window shall be one of the pixels of the target background grid")
			NAV_FIELD_PROP_2NUMBER1(window_priority, 4, "A value of 0 being lowest priority and a value of 15 is the highest priority, i.e., windows with priority 15 are always visible")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CCADescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned short		CA_system_ID;
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned short		reserved:3;
				unsigned short		CA_PID:13;
#else
				unsigned short		CA_PID:13;
				unsigned short		reserved:3;
#endif
			}PACKED;
			unsigned short		short_value;
		}PACKED;

		unsigned char		private_data_byte[];

		int GetVarBodySize(){return descriptor_length - 4;}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(CA_system_ID)
			USHORT_FIELD_ENDIAN(short_value)
		DECLARE_ENDIAN_END()
		
		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(CA_system_ID, 16, "the type of CA system applicable for either the associated ECM and/or EMM streams")
			NAV_FIELD_PROP_2NUMBER1(reserved, 3, "")
			NAV_FIELD_PROP_2NUMBER1(CA_PID, 13, "the PID of the transport stream packets which shall contain either ECM or EMM information for the CA systems as specified with the associated CA_system_ID")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)(descriptor_length - 4)*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CISO639LanguageDescriptor: public DIRECT_ENDIAN_MAP{

		struct CLanguageInfo: public DIRECT_ENDIAN_MAP{
			unsigned char	ISO_639_language_code[3];
			unsigned char	audio_type;

			DECLARE_FIELDPROP_BEGIN()
				MBCSPRINTF_S(szTemp3, _countof(szTemp3), "%c%c%c", ISO_639_language_code[0], ISO_639_language_code[1], ISO_639_language_code[2]);
				NAV_FIELD_PROP_FIXSIZE_STR("ISO_639_language_code", 24, szTemp3, 3, "")
				NAV_FIELD_PROP_2NUMBER1(audio_type, 8, audio_type==0?"Undefined"
					:(audio_type==0x01?"Clean effects"
					:(audio_type==0x02?"Hearing impaired"
					:(audio_type==0x03?"Visual impaired commentary"
					:(audio_type>=0x04 && audio_type<=0x7F?"User Private":"Reserved")))))
			DECLARE_FIELDPROP_END()
		};

		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		CLanguageInfo		language_info[];

		int GetVarBodySize(){return descriptor_length;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			for(i=0;i<(int)(descriptor_length/sizeof(CLanguageInfo));i++){
				NAV_FIELD_PROP_OBJECT(language_info[i])
			}
		DECLARE_FIELDPROP_END()

	};

	struct CSystemClockDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
#ifdef _BIG_ENDIAN_
		unsigned char		external_clock_reference_indicator:1;
		unsigned char		reserved_0:1;
		unsigned char		clock_accuracy_integer:6;

		unsigned char		clock_accuracy_exponent:3;
		unsigned char		reserved_1:5;
#else
		unsigned char		clock_accuracy_integer:6;
		unsigned char		reserved_0:1;
		unsigned char		external_clock_reference_indicator:1;

		unsigned char		reserved_1:5;
		unsigned char		clock_accuracy_exponent:3;
#endif

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")

			NAV_FIELD_PROP_2NUMBER1(external_clock_reference_indicator, 1, external_clock_reference_indicator?"the system clock has been derived from an external frequency reference that may be available at the decoder":"no external frequency reference")
			NAV_FIELD_PROP_2NUMBER("reserved", 1, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(clock_accuracy_integer, 6, "clock_accuracy_integer*10^clock_accuracy_exponent")
			NAV_FIELD_PROP_2NUMBER1(clock_accuracy_exponent, 3, "clock_accuracy_integer*10^clock_accuracy_exponent")
			NAV_FIELD_PROP_2NUMBER("reserved", 5, reserved_1, "")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CMultiplexBufferUtilizationDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		bound_valid_flag:1;
				unsigned long		LTW_offset_lower_bound:15;
				unsigned long		reserved:1;
				unsigned long		LTW_offset_upper_bound:15;
#else
				unsigned long		LTW_offset_upper_bound:15;
				unsigned long		reserved:1;
				unsigned long		LTW_offset_lower_bound:15;
				unsigned long		bound_valid_flag:1;
#endif
			}PACKED;
			unsigned long long_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(long_value)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")

			NAV_FIELD_PROP_2NUMBER1(bound_valid_flag, 1, bound_valid_flag?"LTW_offset_lower_bound and the LTW_offset_upper_bound fields are valid":"LTW_offset_lower_bound and the LTW_offset_upper_bound fields are invalid")
			NAV_FIELD_PROP_2NUMBER1(LTW_offset_lower_bound, 15, "LTW_offset_upper_bound<<15|LTW_offset_lower_bound (90KHZ)")
			NAV_FIELD_PROP_2NUMBER1(reserved, 1, "")
			NAV_FIELD_PROP_2NUMBER1(LTW_offset_upper_bound, 15, "LTW_offset_upper_bound<<15|LTW_offset_lower_bound (90KHZ)")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CCopyrightDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned long		copyright_identifier;

		unsigned char		additional_copyright_info[];

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(copyright_identifier)
		DECLARE_ENDIAN_END()

		int GetVarBodySize(){return descriptor_length - 4;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")

			NAV_FIELD_PROP_2NUMBER1(copyright_identifier, 32, "a 32-bit value obtained from the Registration Authority")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(additional_copyright_info, (unsigned long)(descriptor_length - 4)*8, "defined by the assignee of that copyright_identifier, and once defined, they shall not change")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMaximumBitrateDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		descriptor_length:8;
				unsigned long		reserved:2;
				unsigned long		maximum_bitrate:22;
#else
				unsigned long		maximum_bitrate:22;
				unsigned long		reserved:2;
				unsigned long		descriptor_length:8;
#endif
			}PACKED;
			unsigned long	long_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(long_value)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")

			NAV_FIELD_PROP_2NUMBER1(reserved, 2, "")
			MBCSPRINTF_S(szTemp4, _countof(szTemp4), "upper bound of the bitrate: %lu.%02lu Mbps", 
				(unsigned long)((unsigned long long)maximum_bitrate*400/1024/1024), 
				(unsigned long)((unsigned long long)maximum_bitrate*400*100/1024/1024%100));
			NAV_FIELD_PROP_2NUMBER1(maximum_bitrate, 22, szTemp4)
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CPrivateDataIndicatorDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned long		private_data_indicator;

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(private_data_indicator)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(private_data_indicator, 32, "The value of the private_data_indicator is private and shall not be defined by ITU-T | ISO/IEC")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CSmoothingBufferDescriptor: public DIRECT_ENDIAN_MAP{
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long long		descriptor_tag:8;
				unsigned long long		descriptor_length:8;
				unsigned long long		reserved_0:2;
				unsigned long long		sb_leak_rate:22;
				unsigned long long		reserved_1:2;
				unsigned long long		sb_size:22;
#else
				unsigned long long		sb_size:22;
				unsigned long long		reserved_1:2;
				unsigned long long		sb_leak_rate:22;
				unsigned long long		reserved_0:2;
				unsigned long long		descriptor_length:8;
				unsigned long long		descriptor_tag:8;
#endif
			}PACKED;
			unsigned long long ulonglong_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			UINT64_FIELD_ENDIAN(ulonglong_value)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_0, "")
			MBCSPRINTF_S(szTemp4, _countof(szTemp4), "%lu.%02lu Mbps", 
				(unsigned long)((unsigned long long)sb_leak_rate*400/1024/1024),
				(unsigned long)((unsigned long long)sb_leak_rate*400*100/1024/1024%100)); 
			NAV_FIELD_PROP_2NUMBER1(sb_leak_rate, 22, szTemp4)
			NAV_FIELD_PROP_2NUMBER ("reserved", 2, reserved_1, "")
			NAV_FIELD_PROP_2NUMBER1(sb_size, 22, "bytes, the size of the multiplexing buffer smoothing buffer SBn")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CSTDDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
#ifdef _BIG_ENDIAN_
		unsigned char		reserved:7;
		unsigned char		leak_valid_flag:1;
#else
		unsigned char		leak_valid_flag:1;
		unsigned char		reserved:7;
#endif

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(reserved, 7, "")
			NAV_FIELD_PROP_2NUMBER1(leak_valid_flag, 1, leak_valid_flag?"the transfer of data from the buffer MBn to the buffer EBn in the T-STD uses the leak method as defined in 2.4.2.3"
				:"the vbv_delay fields present in the associated video stream do not have the value 0xFFFF, the transfer of data from the buffer MBn to the buffer EBn uses the vbv_delay method as defined in 2.4.2.3")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CIBPDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned short		closed_gop_flag:1;
				unsigned short		identical_gop_flag:1;
				unsigned short		max_gop_length:14;
#else
				unsigned short		max_gop_length:14;
				unsigned short		identical_gop_flag:1;
				unsigned short		closed_gop_flag:1;
#endif
			}PACKED;
			unsigned short	short_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(short_value)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(closed_gop_flag, 1, closed_gop_flag?"close GOP":"open GOP")
			NAV_FIELD_PROP_2NUMBER1(identical_gop_flag, 1, "all GOPs are the same structre and length")
			NAV_FIELD_PROP_2NUMBER1(max_gop_length, 14, "the maximum number of the coded pictures between any two consecutive I-pictures in the sequence")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMPEG4VideoDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		MPEG4_visual_profile_and_level;

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(MPEG4_visual_profile_and_level, 8, MPEG4_visual_profile_and_level_names[MPEG4_visual_profile_and_level])
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMPEG4AudioDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		MPEG4_audio_profile_and_level;

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(MPEG4_audio_profile_and_level, 8, MPEG4_audio_profile_and_level_names[MPEG4_audio_profile_and_level])
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CIODDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		Scope_of_IOD_label;
		unsigned char		IOD_label;
		unsigned char		InitialObjectDescriptor[];

		int GetVarBodySize(){return descriptor_length>2?(descriptor_length-2):0;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")

			NAV_FIELD_PROP_2NUMBER1(Scope_of_IOD_label, 8, Scope_of_IOD_label==0x10?"IOD_label is unique within the program stream or within the specific program in a transport stream"
				:(Scope_of_IOD_label==0x11?"IOD_label is unique within the transport stream":"the scope of the IOD_label field"))
			NAV_FIELD_PROP_2NUMBER1(IOD_label, 8, "the label of the IOD descriptor")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(InitialObjectDescriptor, (unsigned long)(descriptor_length - 2)*8, "defined in 8.6.3.1 of ISO/IEC 14496-1")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CSLDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned short		ES_ID;

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(ES_ID)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(ES_ID, 16, "specify the identifier of an ISO/IEC 14496-1 SL-packetized stream")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CFMCDescriptor: public DIRECT_ENDIAN_MAP{
		struct CFMC: public DIRECT_ENDIAN_MAP{
			unsigned short	ES_ID;
			unsigned char	FlexMuxChannel;

			DECLARE_ENDIAN_BEGIN()
				USHORT_FIELD_ENDIAN(ES_ID)
			DECLARE_ENDIAN_END()

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(ES_ID, 16, "specifies the identifier of an ISO/IEC 14496-1 SL-packetized stream")
				NAV_FIELD_PROP_2NUMBER1(FlexMuxChannel, 8, "specifies the number of the FlexMux channel used for this SL-packetized stream")
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		CFMC				FMC[];

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			for(i=0;i<(int)(descriptor_length/sizeof(CFMC));i++){
				NAV_WRITE_TAG_BEGIN3("FMC", i);
					NAV_FIELD_PROP_OBJECT(FMC[i])
				NAV_WRITE_TAG_END3("FMC", i);
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CExternalESIDDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned short		External_ES_ID;

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(External_ES_ID)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(External_ES_ID, 16, "an ES_ID identifier, as defined in ISO/IEC 14496-1, to a component of a program")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMuxcodeDescriptor: public ADVANCE_ENDIAN_MAP{

		struct CMuxCodeTableEntry: public ADVANCE_ENDIAN_MAP{

			struct CSubStructure: DIRECT_ENDIAN_MAP {

				struct CSlot: DIRECT_ENDIAN_MAP{
					unsigned char		flexMuxChannel;
					unsigned char		numberOfBytes;

					DECLARE_FIELDPROP_BEGIN()
						NAV_FIELD_PROP_2NUMBER1(flexMuxChannel, 8, "")
						NAV_FIELD_PROP_2NUMBER1(numberOfBytes, 8, "")
					DECLARE_FIELDPROP_END()
				}PACKED;

#ifdef _BIG_ENDIAN_
				unsigned char		slotCount:5;
				unsigned char		repetitionCount:3;
#else
				unsigned char		repetitionCount:3;
				unsigned char		slotCount:5;
#endif

				CSlot				slots[];

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER1(slotCount, 5, "")
					NAV_FIELD_PROP_2NUMBER1(repetitionCount, 3, "")
					for(i=0;i<(int)slotCount;i++){
						NAV_WRITE_TAG_BEGIN3("Slot", i);
							NAV_FIELD_PROP_OBJECT(slots[i])
						NAV_WRITE_TAG_END3("Slot", i);
					}
				DECLARE_FIELDPROP_END()

			}PACKED;

			unsigned char		length;
#ifdef _BIG_ENDIAN_			
			unsigned char		MuxCode:4;
			unsigned char		version:4;
#else
			unsigned char		version:4;
			unsigned char		MuxCode:4;
#endif
			unsigned char		substructureCount;

			CSubStructure**		substructures;

			CMuxCodeTableEntry(): length(0), version(0), MuxCode(0), substructureCount(0), substructures(NULL){}

			virtual ~CMuxCodeTableEntry(){Unmap();}

			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
			{
				unsigned long ulMappedSize = 0;

				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;

				MAP_MEM_TO_HDR2(&length, 3);
				AMP_NEW1(substructures, CSubStructure*, substructureCount);
				if (substructures != nullptr) {
					for (int i = 0; i < substructureCount; i++) {
						MAP_MEM_TO_STRUCT_POINTER(1, substructures[i], CSubStructure)
					}
				}

				AMP_SAFEASSIGN(desired_size, ulMappedSize);

				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
			{
				if (pcbSize == NULL)
				{
					for(int i=0;i<substructureCount;i++){
						UNMAP_STRUCT_POINTER(substructures[i])
					}
					AMP_SAFEDELA2(substructures);
				}
				else
				{
					unsigned long cbRequired = length + 1;
					UNMAP_GENERAL_UTILITY()
				}
				return RET_CODE_SUCCESS;
			}

			int WriteToBs(AMBst bs)
			{
				AMBst_PutBits(bs, 8, length);
				AMBst_PutBits(bs, 4, MuxCode);
				AMBst_PutBits(bs, 4, version);
				AMBst_PutBits(bs, 8, substructureCount);
				for(int i=0;i<substructureCount;i++){
					AMBst_PutBits(bs, 5, substructures[i]->slotCount);
					AMBst_PutBits(bs, 3, substructures[i]->repetitionCount);
					for(int j=0;j<substructures[i]->slotCount;j++){
						AMBst_PutBits(bs, 8, substructures[i]->slots[j].flexMuxChannel);
						AMBst_PutBits(bs, 8, substructures[i]->slots[j].numberOfBytes);
					}
				}
				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(length, 8, "")
				NAV_FIELD_PROP_2NUMBER1(MuxCode, 4, "")
				NAV_FIELD_PROP_2NUMBER1(version, 4, "")
				NAV_FIELD_PROP_2NUMBER1(substructureCount, 8, "")
				for(i=0;i<(int)substructureCount;i++){
					NAV_FIELD_PROP_REF_WITH_TAG3(substructures[i], "substructure", i)
				}
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		CMuxCodeTableEntry*	MuxCodeTableEntry;

		CMuxcodeDescriptor(): descriptor_tag(0), descriptor_length(0), MuxCodeTableEntry(NULL){}
		virtual ~CMuxcodeDescriptor(){Unmap();}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			MAP_MEM_TO_HDR2(&descriptor_tag, 2)
			MAP_MEM_TO_STRUCT_POINTER2(1, MuxCodeTableEntry, CMuxCodeTableEntry)
			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL){
				UNMAP_STRUCT_POINTER2(MuxCodeTableEntry)
			}else{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);
			if (MuxCodeTableEntry)
				MuxCodeTableEntry->WriteToBs(bs);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_REF(MuxCodeTableEntry)
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CFmxBufferSizeDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		//DefaultFlexMuxBufferDescriptor
		//for (i=0; i<descriptor_length; i += 4) {
		//FlexMuxBufferDescriptor()
		//}
		unsigned char		descriptor_data[];

		int GetVarBodySize(){return descriptor_length;}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(descriptor_data, (unsigned long)descriptor_length*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMultiplexBufferDescriptor: public DIRECT_ENDIAN_MAP{
		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long long		descriptor_tag:8;
				unsigned long long		descriptor_length:8;
				unsigned long long		MB_buffer_size:24;
				unsigned long long		TB_leak_rate:24;
#else
				unsigned long long		TB_leak_rate:24;
				unsigned long long		MB_buffer_size:24;
				unsigned long long		descriptor_length:8;
				unsigned long long		descriptor_tag:8;
#endif
			}PACKED;
			unsigned long long ulonglong_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			UINT64_FIELD_ENDIAN(ulonglong_value)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(MB_buffer_size, 8, "bytes");
			MBCSPRINTF_S(szTemp4, _countof(szTemp4), "%lu.%02lu Mbps leaked from transport buffer TBn to multiplex buffer MBn", 
				((unsigned long)((unsigned long long)TB_leak_rate*400/1024/1024)),
				((unsigned long)((unsigned long long)TB_leak_rate*400*100/1024/1024%100)));
			NAV_FIELD_PROP_2NUMBER1(TB_leak_rate, 8, szTemp4);
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CContentLabelingDescriptor: public ADVANCE_ENDIAN_MAP{

		struct CContentReferenceIdRecord: public DIRECT_ENDIAN_MAP{
			unsigned char	content_reference_id_record_length;
			unsigned char	content_reference_id_byte[];

			int GetVarBodySize(){return content_reference_id_record_length;}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(content_reference_id_record_length, 8, "specifies the number of content_reference_id_bytes immediately following this field")
				NAV_FIELD_PROP_FIXSIZE_BINCHARSTR1(content_reference_id_byte, (long long)content_reference_id_record_length*8, "part of a string of one or more contiguous bytes that assigns one or more reference identifications (labels) to the content")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CSTCTimeBase: public DIRECT_ENDIAN_MAP{
#ifdef _BIG_ENDIAN_
			unsigned char	reserved_0:7;
			unsigned char	content_time_base_value_0:1;
#else
			unsigned char	content_time_base_value_0:1;
			unsigned char	reserved_0:7;
#endif

			unsigned char	content_time_base_value_1[4];

#ifdef _BIG_ENDIAN_
			unsigned char	reserved_1:7;
			unsigned char	metadata_time_base_value_0:1;
#else
			unsigned char	metadata_time_base_value_0:1;
			unsigned char	reserved_1:7;
#endif

			unsigned char	metadata_time_base_value_1[4];

			inline unsigned long long content_time_base_value(){
				unsigned long long ullRet = content_time_base_value_0;
				ullRet <<= 8; ullRet |= content_time_base_value_1[0];
				ullRet <<= 8; ullRet |= content_time_base_value_1[1];
				ullRet <<= 8; ullRet |= content_time_base_value_1[2];
				ullRet <<= 8; ullRet |= content_time_base_value_1[3];
				return ullRet;
			}

			inline unsigned long long metadata_time_base_value(){
				unsigned long long ullRet = metadata_time_base_value_0;
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[0];
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[1];
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[2];
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[3];
				return ullRet;
			}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER("reserved", 7, reserved_0, "")
				NAV_FIELD_PROP_2NUMBER("content_time_base_value", 33, content_time_base_value(), "specifies a value in units of 90 kHz of the content time base indicated by the content_time_base_indicator field")
				NAV_FIELD_PROP_2NUMBER("reserved", 7, reserved_1, "")
				NAV_FIELD_PROP_2NUMBER("metadata_time_base_value", 33, metadata_time_base_value(), "coded in units of 90 kHz and coded with the value of the metadata time base at the instant in time in which the time base indicated by content_time_base_indicator reaches the value encoded in the content_time_base_value field.")
			DECLARE_FIELDPROP_END()			
			
		}PACKED;

		struct CNPTTimeBase: public DIRECT_ENDIAN_MAP{
#ifdef _BIG_ENDIAN_
			unsigned char	reserved_0:7;
			unsigned char	content_time_base_value_0:1;
#else
			unsigned char	content_time_base_value_0:1;
			unsigned char	reserved_0:7;
#endif

			unsigned char	content_time_base_value_1[4];

#ifdef _BIG_ENDIAN_
			unsigned char	reserved_1:7;
			unsigned char	metadata_time_base_value_0:1;
#else
			unsigned char	metadata_time_base_value_0:1;
			unsigned char	reserved_1:7;
#endif

			unsigned char	metadata_time_base_value_1[4];

#ifdef _BIG_ENDIAN_
			unsigned char	reserved_2:1;
			unsigned char	contentId:7;
#else
			unsigned char	contentId:7;
			unsigned char	reserved_2:1;
#endif

			inline unsigned long long content_time_base_value(){
				unsigned long long ullRet = content_time_base_value_0;
				ullRet <<= 8; ullRet |= content_time_base_value_1[0];
				ullRet <<= 8; ullRet |= content_time_base_value_1[1];
				ullRet <<= 8; ullRet |= content_time_base_value_1[2];
				ullRet <<= 8; ullRet |= content_time_base_value_1[3];
				return ullRet;
			}

			inline unsigned long long metadata_time_base_value(){
				unsigned long long ullRet = metadata_time_base_value_0;
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[0];
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[1];
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[2];
				ullRet <<= 8; ullRet |= metadata_time_base_value_1[3];
				return ullRet;
			}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER("reserved", 7, reserved_0, "")
				NAV_FIELD_PROP_2NUMBER("content_time_base_value", 33, content_time_base_value(), "specifies a value in units of 90 kHz of the content time base indicated by the content_time_base_indicator field")
				NAV_FIELD_PROP_2NUMBER("reserved", 7, reserved_1, "")
				NAV_FIELD_PROP_2NUMBER("metadata_time_base_value", 33, metadata_time_base_value(), "coded in units of 90 kHz and coded with the value of the metadata time base at the instant in time in which the time base indicated by content_time_base_indicator reaches the value encoded in the content_time_base_value field.")
				NAV_FIELD_PROP_2NUMBER("reserved", 1, reserved_2, "")
				NAV_FIELD_PROP_2NUMBER1(contentId, 7, "specifies the value of the content_Id field in the NPT Reference Descriptor for the applied NPT time base")
			DECLARE_FIELDPROP_END()	
		}PACKED;

		struct COtherTimeBase: public DIRECT_ENDIAN_MAP{
			unsigned char	time_base_association_data_length;
			unsigned char	reserved[];

			int GetVarBodySize(){return time_base_association_data_length;}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(time_base_association_data_length, 8, "");
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(reserved, (unsigned long)time_base_association_data_length*8, "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char				descriptor_tag;
		unsigned char				descriptor_length;
		unsigned short				metadata_application_format;

		unsigned long				metadata_application_format_identifier;

#ifdef _BIG_ENDIAN_
		unsigned char				content_reference_id_record_flag:1;
		unsigned char				content_time_base_indicator:4;
		unsigned char				reserved:3;
#else
		unsigned char				reserved:3;
		unsigned char				content_time_base_indicator:4;
		unsigned char				content_reference_id_record_flag:1;
#endif

		CContentReferenceIdRecord*	content_reference_id_record;
		union {
			CSTCTimeBase*			STC_time_base;
			CNPTTimeBase*			NPT_time_base;
			COtherTimeBase*			other_time_base;
		};
		unsigned char*				private_data_byte;
		unsigned char				private_data_len;

		CContentLabelingDescriptor(): content_reference_id_record(NULL), other_time_base(NULL), private_data_byte(NULL), private_data_len(0){
		}

		virtual ~CContentLabelingDescriptor(){
			Unmap();
		}

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(metadata_application_format)
			ULONG_FIELD_ENDIAN(metadata_application_format_identifier)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 5)
				return RET_CODE_BUFFER_TOO_SMALL;

			AMBst bst_t = AMBst_CreateFromBuffer(pBuf, cbSize);
			descriptor_tag = AMBst_GetByte(bst_t);
			descriptor_length = AMBst_GetByte(bst_t);
			metadata_application_format = AMBst_GetWord(bst_t);
			ulMappedSize = 4;

			if (metadata_application_format == 0xFFFF)
			{
				if (cbSize < 9)
					return RET_CODE_BUFFER_TOO_SMALL;
				metadata_application_format_identifier = AMBst_GetDWord(bst_t);
				ulMappedSize += 4;
			}

			content_reference_id_record_flag = AMBst_GetBits(bst_t, 1);
			content_time_base_indicator = AMBst_GetBits(bst_t, 4);
			reserved = AMBst_GetBits(bst_t, 3);
			ulMappedSize += 1;

			AMBst_Destroy(bst_t);

			if (cbSize < (unsigned long)(descriptor_length + 2))
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_STRUCT_POINTER(content_reference_id_record_flag, content_reference_id_record, CContentReferenceIdRecord);
			if (content_time_base_indicator == 1){
				MAP_MEM_TO_STRUCT_POINTER(1, STC_time_base, CSTCTimeBase)
			}else if(content_time_base_indicator == 2){
				MAP_MEM_TO_STRUCT_POINTER(1, NPT_time_base, CNPTTimeBase)
			}else if(content_time_base_indicator >= 3 && content_time_base_indicator <= 7){
				MAP_MEM_TO_STRUCT_POINTER(1, other_time_base, COtherTimeBase)
			}

			private_data_byte = pBuf + ulMappedSize;
			private_data_len = (unsigned char)(descriptor_length + 2 - ulMappedSize);
			ulMappedSize = descriptor_length + 2;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL){
				if (content_time_base_indicator == 1){
					UNMAP_STRUCT_POINTER(STC_time_base)
				}else if(content_time_base_indicator == 2){
					UNMAP_STRUCT_POINTER(NPT_time_base)
				}else if(content_time_base_indicator >= 3 && content_time_base_indicator <= 7){
					UNMAP_STRUCT_POINTER(other_time_base)
				}
				UNMAP_STRUCT_POINTER(content_reference_id_record)
				private_data_byte = NULL;
			}
			else
			{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);
			AMBst_PutBits(bs, 16, metadata_application_format);
			if (metadata_application_format == 0xFFFF)
				AMBst_PutBits(bs, 32, metadata_application_format_identifier);
			AMBst_PutBits(bs, 1, content_reference_id_record_flag);
			AMBst_PutBits(bs, 4, content_time_base_indicator);
			AMBst_ReservedBits(bs, 3);
			if (content_reference_id_record_flag){
				UNMAP_DUPLICATE_STRUCT_POINTER(content_reference_id_record, CContentReferenceIdRecord)
			}
			if (content_time_base_indicator == 1){
				UNMAP_DUPLICATE_STRUCT_POINTER(STC_time_base, CSTCTimeBase)
			}else if(content_time_base_indicator == 2){
				UNMAP_DUPLICATE_STRUCT_POINTER(NPT_time_base, CNPTTimeBase)
			}else if(content_time_base_indicator >= 3 && content_time_base_indicator <= 7){
				UNMAP_DUPLICATE_STRUCT_POINTER(other_time_base, COtherTimeBase)
			}
			if (private_data_len > 0)
				AMBst_PutBytes(bs, private_data_byte, private_data_len);
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(metadata_application_format, 16, metadata_application_format==0x10?"ISO 15706 (ISAN) encoded in its binary form (see Notes 1 and 3)"
				:(metadata_application_format==0x11?"ISO 15706-2 (V-ISAN) encoded in its binary form (see Notes 2 and 3)"
				:(metadata_application_format==0xFFFF?"Defined by the metadata_application_format_identifier field"
				:(metadata_application_format>=0x0100 && metadata_application_format<=0xFFFE?"User defined":"Reserved"))))
			if (metadata_application_format == 0XFFFF){
				NAV_FIELD_PROP_2NUMBER1(metadata_application_format_identifier, 32, "fully equivalent to the coding of the format_identifier field in the registration_descriptor")
			}
			NAV_FIELD_PROP_2NUMBER1(content_reference_id_record_flag, 1, "signals the presence of a content_reference_id_record in this descriptor")
			NAV_FIELD_PROP_2NUMBER1(content_time_base_indicator, 4, content_time_base_indicator==0?"No content time base defined in this descriptor"
				:(content_time_base_indicator==1?"Use of STC"
				:(content_time_base_indicator==2?"Use of NPT"
				:(content_time_base_indicator>=8 && content_time_base_indicator<=15?"Use of privately defined content time base":"Reserved"))))
			NAV_FIELD_PROP_2NUMBER1(reserved, 3, "")
			if (content_reference_id_record_flag){
				NAV_FIELD_PROP_REF(content_reference_id_record)
			}
			if (content_time_base_indicator == 1){
				NAV_FIELD_PROP_REF(STC_time_base)
			}else if(content_time_base_indicator == 2){
				NAV_FIELD_PROP_REF(NPT_time_base)
			}else if(content_time_base_indicator >= 3 && content_time_base_indicator <= 7){
				NAV_FIELD_PROP_REF(other_time_base)
			}
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)private_data_len*8, "")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CMetadataPointerDescriptor: public ADVANCE_ENDIAN_MAP{

		struct CMetadataLocatorRecord: public DIRECT_ENDIAN_MAP{
			unsigned char			metadata_locator_record_length;
			unsigned char			metadata_locator_record_byte[];

			int GetVarBodySize(){return metadata_locator_record_length;}

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(metadata_locator_record_length, 8, "specifies the number of metadata_locator_record_bytes immediately following")
				NAV_FIELD_PROP_FIXSIZE_BINCHARSTR1(metadata_locator_record_byte, (long long)metadata_locator_record_length*8, "part of a string of one or more contiguous bytes that form the metadata locator record")
			DECLARE_FIELDPROP_END()

		}PACKED;

		struct CMPEGCarriage0: public DIRECT_ENDIAN_MAP{
			unsigned short			program_number;

			DECLARE_ENDIAN_BEGIN()
				USHORT_FIELD_ENDIAN(program_number)
			DECLARE_ENDIAN_END()

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(program_number, 16, "identifies the program_number of the MPEG-2 program")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CMPEGCarriage1: public DIRECT_ENDIAN_MAP{
			unsigned short			program_number;
			unsigned short			transport_stream_location;
			unsigned short			transport_stream_id;

			DECLARE_ENDIAN_BEGIN()
				USHORT_FIELD_ENDIAN(program_number)
				USHORT_FIELD_ENDIAN(transport_stream_location)
				USHORT_FIELD_ENDIAN(transport_stream_id)
			DECLARE_ENDIAN_END()

			DECLARE_FIELDPROP_BEGIN()
				DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
				NAV_FIELD_PROP_2NUMBER1(program_number, 16, "identifies the program_number of the MPEG-2 program")
				NAV_FIELD_PROP_2NUMBER1(transport_stream_location, 16, "")
				NAV_FIELD_PROP_2NUMBER1(transport_stream_id, 16, "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char				descriptor_tag;
		unsigned char				descriptor_length;
		unsigned short				metadata_application_format;

		unsigned long				metadata_application_format_identifier;

		unsigned char				metadata_format;
		unsigned long				metadata_format_identifier;

		unsigned char				metadata_service_id;

#ifdef _BIG_ENDIAN_
		unsigned char				metadata_locator_record_flag:1;
		unsigned char				MPEG_carriage_flags:2;
		unsigned char				reserved:5;
#else
		unsigned char				reserved:5;
		unsigned char				MPEG_carriage_flags:2;
		unsigned char				metadata_locator_record_flag:1;
#endif
		CMetadataLocatorRecord*		metadata_locator_record;
		union {
			CMPEGCarriage0*			MPEG_Carriage_02;
			CMPEGCarriage1*			MPEG_Carriage_1;
		}PACKED;

		unsigned char*				private_data_byte;
		unsigned char				private_data_len;

		CMetadataPointerDescriptor(): metadata_locator_record_flag(0), metadata_locator_record(NULL), MPEG_Carriage_02(NULL), private_data_byte(NULL){}
		virtual ~CMetadataPointerDescriptor(){Unmap();}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 7)
				return RET_CODE_BUFFER_TOO_SMALL;

			AMBst bst_t = AMBst_CreateFromBuffer(pBuf, cbSize);
			descriptor_tag = AMBst_GetByte(bst_t);
			descriptor_length = AMBst_GetByte(bst_t);
			metadata_application_format = AMBst_GetWord(bst_t);
			ulMappedSize = 4;

			if (metadata_application_format == 0xFFFF)
			{
				if (cbSize < 11)
					return RET_CODE_BUFFER_TOO_SMALL;
				metadata_application_format_identifier = AMBst_GetDWord(bst_t);
				ulMappedSize += 4;
			}

			metadata_format = AMBst_GetByte(bst_t);ulMappedSize += 1;
			if (metadata_format == 0xFF)
			{
				if (cbSize < 15)
					return RET_CODE_BUFFER_TOO_SMALL;
				metadata_format_identifier = AMBst_GetDWord(bst_t);
				ulMappedSize += 4;
			}

			metadata_service_id = AMBst_GetByte(bst_t);
			metadata_locator_record_flag = AMBst_GetBits(bst_t, 1);
			MPEG_carriage_flags = AMBst_GetBits(bst_t, 2);
			reserved = AMBst_GetBits(bst_t, 5);
			ulMappedSize += 2;

			AMBst_Destroy(bst_t);

			if (cbSize < (unsigned long)(descriptor_length + 2))
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_STRUCT_POINTER(metadata_locator_record_flag, metadata_locator_record, CMetadataLocatorRecord);
			if (MPEG_carriage_flags == 1){
				MAP_MEM_TO_STRUCT_POINTER(1, MPEG_Carriage_1, CMPEGCarriage1)
			}else if(MPEG_carriage_flags == 0 || MPEG_carriage_flags == 2){
				MAP_MEM_TO_STRUCT_POINTER(1, MPEG_Carriage_02, CMPEGCarriage0)
			}

			private_data_byte = pBuf + ulMappedSize;
			private_data_len = (unsigned char)(descriptor_length + 2 - ulMappedSize);
			ulMappedSize = descriptor_length + 2;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				if (MPEG_carriage_flags == 1){
					UNMAP_STRUCT_POINTER(MPEG_Carriage_1)
				}else if(MPEG_carriage_flags == 0 || MPEG_carriage_flags == 2){
					UNMAP_STRUCT_POINTER(MPEG_Carriage_02)
				}
				UNMAP_STRUCT_POINTER(metadata_locator_record);
				private_data_byte = NULL;
			}
			else
			{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);
			AMBst_PutBits(bs, 16, metadata_application_format);
			if (metadata_application_format == 0xFFFF)
				AMBst_PutBits(bs, 32, metadata_application_format_identifier);

			AMBst_PutBits(bs, 8, metadata_format);
			if (metadata_format == 0xFF)
				AMBst_PutBits(bs, 32, metadata_format_identifier);

			AMBst_PutBits(bs, 8, metadata_service_id);
			AMBst_PutBits(bs, 1, metadata_locator_record_flag);
			AMBst_PutBits(bs, 2, MPEG_carriage_flags);
			AMBst_ReservedBits(bs, 5);

			if (metadata_locator_record_flag){
				UNMAP_DUPLICATE_STRUCT_POINTER(metadata_locator_record, CMetadataLocatorRecord)
			}

			if (MPEG_carriage_flags == 1){
				UNMAP_DUPLICATE_STRUCT_POINTER(MPEG_Carriage_1, CMPEGCarriage1)
			}else if(MPEG_carriage_flags == 0 || MPEG_carriage_flags == 2){
				UNMAP_DUPLICATE_STRUCT_POINTER(MPEG_Carriage_02, CMPEGCarriage0)
			}

			if (private_data_len > 0)
				AMBst_PutBytes(bs, private_data_byte, private_data_len);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(metadata_application_format, 16, metadata_application_format==0x10?"ISO 15706 (ISAN) encoded in its binary form (see Notes 1 and 3)"
				:(metadata_application_format==0x11?"ISO 15706-2 (V-ISAN) encoded in its binary form (see Notes 2 and 3)"
				:(metadata_application_format==0xFFFF?"Defined by the metadata_application_format_identifier field"
				:(metadata_application_format>=0x0100 && metadata_application_format<=0xFFFE?"User defined":"Reserved"))))
			if (metadata_application_format == 0XFFFF){
				NAV_FIELD_PROP_2NUMBER1(metadata_application_format_identifier, 32, "fully equivalent to the coding of the format_identifier field in the registration_descriptor")
			}
			NAV_FIELD_PROP_2NUMBER1(metadata_format, 1, metadata_format==0x10?"ISO/IEC 15938-1 TeM"
				:(metadata_format==0x11?"ISO/IEC 15938-1 BiM"
				:(metadata_format==0x3F?"Defined by metadata application format"
				:(metadata_format==0xFF?"Defined by metadata_format_identifier field"
				:(metadata_format>=0x40 && metadata_format<=0xFE?"Private use":"Reserved")))))
			if (metadata_format == 0xFF){
				NAV_FIELD_PROP_2NUMBER1(metadata_format_identifier, 4, "fully equivalent to the coding of the format_identifier field in the registration_descriptor")
			}
			NAV_FIELD_PROP_2NUMBER1(metadata_service_id, 8, "This 8-bit field references the metadata service. It is used for retrieving a metadata service from within a metadata stream.")
			NAV_FIELD_PROP_2NUMBER1(metadata_locator_record_flag, 1, metadata_locator_record_flag?"indicates that associated metadata is available on a location outside of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 stream, specified in a metadata_locator_record.":"")
			NAV_FIELD_PROP_2NUMBER1(MPEG_carriage_flags, 2, "")
			NAV_FIELD_PROP_2NUMBER1(reserved, 5, "")
			if (metadata_locator_record_flag){
				NAV_FIELD_PROP_REF(metadata_locator_record)
			}
			if (MPEG_carriage_flags == 0 || MPEG_carriage_flags == 2){
				NAV_FIELD_PROP_REF(MPEG_Carriage_02)
			}else if(MPEG_carriage_flags == 1){
				NAV_FIELD_PROP_REF(MPEG_Carriage_1)
			}
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)private_data_len*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMetadataDescriptor: public ADVANCE_ENDIAN_MAP{

		struct CServiceIdentification: public DIRECT_ENDIAN_MAP{
			unsigned char			service_identification_length;
			unsigned char			service_identification_record_byte[];

			int GetVarBodySize(){return service_identification_length;}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(service_identification_length, 8, "")
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(service_identification_record_byte, ((long long)service_identification_length*8), "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CDecoderConfig: public DIRECT_ENDIAN_MAP{
			unsigned char			decoder_config_length;
			unsigned char			decoder_config_byte[];

			int GetVarBodySize(){return decoder_config_length;}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(decoder_config_length, 8, "")
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(decoder_config_byte, ((long long)decoder_config_length*8), "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CDecConfigIdentification: public DIRECT_ENDIAN_MAP{
			unsigned char			dec_config_identification_record_length;
			unsigned char			dec_config_identification_record_byte[];

			int GetVarBodySize(){return dec_config_identification_record_length;}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(dec_config_identification_record_length, 8, "")
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(dec_config_identification_record_byte, ((long long)dec_config_identification_record_length*8), "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct CDecoderConfigMetadataService: public DIRECT_ENDIAN_MAP{
			unsigned char			decoder_config_metadata_service_id;

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(decoder_config_metadata_service_id, 8, "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		struct COtherConfig: public DIRECT_ENDIAN_MAP{
			unsigned char			reserved_data_length;
			unsigned char			reserved[];

			int GetVarBodySize(){return reserved_data_length;}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(reserved_data_length, 8, "")
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(reserved, ((long long)reserved_data_length*8), "")
			DECLARE_FIELDPROP_END()
		}PACKED;

		unsigned char				descriptor_tag;
		unsigned char				descriptor_length;
		unsigned short				metadata_application_format;

		unsigned long				metadata_application_format_identifier;

		unsigned char				metadata_format;
		unsigned long				metadata_format_identifier;

		unsigned char				metadata_service_id;

#ifdef _BIG_ENDIAN_
		unsigned char				decoder_config_flags:3;
		unsigned char				DSMCC_flag:1;
		unsigned char				reserved:4;
#else
		unsigned char				reserved:3;
		unsigned char				DSMCC_flag:1;
		unsigned char				decoder_config_flags:4;
#endif
		CServiceIdentification*		service_identification_record;
		union {
			CDecoderConfig*			decoder_config;
			CDecConfigIdentification*			
									dec_config_identification_record;
			CDecoderConfigMetadataService*
									decoder_config_metadata_service;
			COtherConfig*			reserved_data;
		}PACKED;

		unsigned char*				private_data_byte;
		unsigned char				private_data_len;

		CMetadataDescriptor()
			: decoder_config_flags(0), service_identification_record(NULL), reserved_data(NULL), private_data_byte(NULL), private_data_len(0){}
		virtual ~CMetadataDescriptor(){Unmap();}

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(metadata_application_format_identifier)
			ULONG_FIELD_ENDIAN(metadata_format_identifier)
		DECLARE_ENDIAN_END()

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 7)
				return RET_CODE_BUFFER_TOO_SMALL;

			AMBst bst_t = AMBst_CreateFromBuffer(pBuf, cbSize);
			descriptor_tag = AMBst_GetByte(bst_t);
			descriptor_length = AMBst_GetByte(bst_t);
			metadata_application_format = AMBst_GetWord(bst_t);
			ulMappedSize = 4;

			if (metadata_application_format == 0xFFFF)
			{
				if (cbSize < 11)
					return RET_CODE_BUFFER_TOO_SMALL;
				metadata_application_format_identifier = AMBst_GetDWord(bst_t);
				ulMappedSize += 4;
			}

			metadata_format = AMBst_GetByte(bst_t);ulMappedSize += 1;
			if (metadata_format == 0xFF)
			{
				if (cbSize < 15)
					return RET_CODE_BUFFER_TOO_SMALL;
				metadata_format_identifier = AMBst_GetDWord(bst_t);
				ulMappedSize += 4;
			}

			metadata_service_id = AMBst_GetByte(bst_t);
			decoder_config_flags = AMBst_GetBits(bst_t, 3);
			DSMCC_flag = AMBst_GetBits(bst_t, 1);
			reserved = AMBst_GetBits(bst_t, 4);
			ulMappedSize += 2;

			AMBst_Destroy(bst_t);

			if (cbSize < (unsigned long)(descriptor_length + 2))
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_STRUCT_POINTER(DSMCC_flag, service_identification_record, CServiceIdentification)
			if (decoder_config_flags == 1){
				MAP_MEM_TO_STRUCT_POINTER(1, decoder_config, CDecoderConfig)
			}else if(decoder_config_flags == 3){
				MAP_MEM_TO_STRUCT_POINTER(1, dec_config_identification_record, CDecConfigIdentification)
			}else if(decoder_config_flags == 4){
				MAP_MEM_TO_STRUCT_POINTER(1, decoder_config_metadata_service, CDecoderConfigMetadataService)
			}else if(decoder_config_flags == 5 || decoder_config_flags == 6){
				MAP_MEM_TO_STRUCT_POINTER(1, reserved_data, COtherConfig)
			}

			private_data_byte = pBuf + ulMappedSize;
			private_data_len = (unsigned char)(descriptor_length + 2 - ulMappedSize);
			ulMappedSize = descriptor_length + 2;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				if (decoder_config_flags == 1){
					UNMAP_STRUCT_POINTER(decoder_config)
				}else if(decoder_config_flags == 3){
					UNMAP_STRUCT_POINTER(dec_config_identification_record)
				}else if(decoder_config_flags == 4){
					UNMAP_STRUCT_POINTER(decoder_config_metadata_service)
				}else if(decoder_config_flags == 5 || decoder_config_flags == 6){
					UNMAP_STRUCT_POINTER(reserved_data)
				}
				UNMAP_STRUCT_POINTER(service_identification_record)
				private_data_byte = NULL;
			}
			else
			{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);
			AMBst_PutBits(bs, 16, metadata_application_format);
			if (metadata_application_format == 0xFFFF)
				AMBst_PutBits(bs, 32, metadata_application_format_identifier);

			AMBst_PutBits(bs, 8, metadata_format);
			if (metadata_format == 0xFF)
				AMBst_PutBits(bs, 32, metadata_format_identifier);

			AMBst_PutBits(bs, 8, metadata_service_id);
			AMBst_PutBits(bs, 3, decoder_config_flags);
			AMBst_PutBits(bs, 1, DSMCC_flag);
			AMBst_ReservedBits(bs, 4);

			if (DSMCC_flag){
				UNMAP_DUPLICATE_STRUCT_POINTER(service_identification_record, CServiceIdentification)
			}

			if (decoder_config_flags == 1){
				UNMAP_DUPLICATE_STRUCT_POINTER(decoder_config, CDecoderConfig)
			}else if(decoder_config_flags == 3){
				UNMAP_DUPLICATE_STRUCT_POINTER(dec_config_identification_record, CDecConfigIdentification)
			}else if(decoder_config_flags == 4){
				UNMAP_DUPLICATE_STRUCT_POINTER(decoder_config_metadata_service, CDecoderConfigMetadataService)
			}else if(decoder_config_flags == 5 || decoder_config_flags == 6){
				UNMAP_DUPLICATE_STRUCT_POINTER(reserved_data, COtherConfig)
			}

			if (private_data_len > 0)
				AMBst_PutBytes(bs, private_data_byte, private_data_len);

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER1(metadata_application_format, 16, metadata_application_format==0x10?"ISO 15706 (ISAN) encoded in its binary form (see Notes 1 and 3)"
				:(metadata_application_format==0x11?"ISO 15706-2 (V-ISAN) encoded in its binary form (see Notes 2 and 3)"
				:(metadata_application_format==0xFFFF?"Defined by the metadata_application_format_identifier field"
				:(metadata_application_format>=0x0100 && metadata_application_format<=0xFFFE?"User defined":"Reserved"))))
			if (metadata_application_format == 0XFFFF){
				NAV_FIELD_PROP_2NUMBER1(metadata_application_format_identifier, 32, "fully equivalent to the coding of the format_identifier field in the registration_descriptor")
			}
			NAV_FIELD_PROP_2NUMBER1(metadata_format, 1, metadata_format==0x10?"ISO/IEC 15938-1 TeM"
				:(metadata_format==0x11?"ISO/IEC 15938-1 BiM"
				:(metadata_format==0x3F?"Defined by metadata application format"
				:(metadata_format==0xFF?"Defined by metadata_format_identifier field"
				:(metadata_format>=0x40 && metadata_format<=0xFE?"Private use":"Reserved")))))
			if (metadata_format == 0xFF){
				NAV_FIELD_PROP_2NUMBER1(metadata_format_identifier, 4, "fully equivalent to the coding of the format_identifier field in the registration_descriptor")
			}
			NAV_FIELD_PROP_2NUMBER1(metadata_service_id, 8, "This 8-bit field references the metadata service. It is used for retrieving a metadata service from within a metadata stream.")
			NAV_FIELD_PROP_2NUMBER1(decoder_config_flags, 3, decoder_config_flags==0x00?"No decoder configuration is needed"
				:(decoder_config_flags==0x01?"The decoder configuration is carried in this descriptor in the decoder_config_byte field"
				:(decoder_config_flags==0x02?"The decoder configuration is carried in the same metadata service as to which this metadata descriptor applies"
				:(decoder_config_flags==0x03?"The decoder configuration is carried in a DSM-CC carousel. This value shall only be used if the metadata service to which this descriptor applies is using the same type of DSM-CC carousel."
				:(decoder_config_flags==0x04?"The decoder configuration is carried in another metadata service within the same program, as identified by the decoder_config_metadata_service_id field in this metadata descriptor"
				:(decoder_config_flags==0x07?"Privately defined.":"Reserved"))))))
			NAV_FIELD_PROP_2NUMBER1(DSMCC_flag, 1, "")
			NAV_FIELD_PROP_2NUMBER1(reserved, 4, "")
			if (DSMCC_flag){
				NAV_FIELD_PROP_REF(service_identification_record)
			}
			if (decoder_config_flags == 1){
				NAV_FIELD_PROP_REF(decoder_config)
			}else if(decoder_config_flags == 3){
				NAV_FIELD_PROP_REF(dec_config_identification_record)
			}else if(decoder_config_flags == 4){
				NAV_FIELD_PROP_REF(decoder_config_metadata_service)
			}else if(decoder_config_flags == 5 || decoder_config_flags == 6){
				NAV_FIELD_PROP_REF(reserved_data)
			}

			NAV_FIELD_PROP_FIXSIZE_BINSTR1(private_data_byte, (unsigned long)private_data_len*8, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMetadataSTDDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

#ifdef _BIG_ENDIAN_
		unsigned char		reserved_0:2;
		unsigned char		metadata_input_leak_rate_0:6;
#else
		unsigned char		metadata_input_leak_rate_0:6;
		unsigned char		reserved_0:2;
#endif
		unsigned char		metadata_input_leak_rate_1[2];

#ifdef _BIG_ENDIAN_
		unsigned char		reserved_1:2;
		unsigned char		metadata_buffer_size_0:6;
#else
		unsigned char		metadata_buffer_size_0:6;
		unsigned char		reserved_1:2;
#endif
		unsigned char		metadata_buffer_size_1[2];

#ifdef _BIG_ENDIAN_
		unsigned char		reserved_2:2;
		unsigned char		metadata_output_leak_rate_0:6;
#else
		unsigned char		metadata_output_leak_rate_0:6;
		unsigned char		reserved_2:2;
#endif
		unsigned char		metadata_output_leak_rate_1[2];

		inline unsigned long metadata_input_leak_rate(){
			unsigned long nRet = metadata_input_leak_rate_0;
			nRet <<= 8; nRet |= metadata_input_leak_rate_1[0];
			nRet <<= 8; nRet |= metadata_input_leak_rate_1[1];
			return nRet;
		}

		inline unsigned long metadata_buffer_size(){
			unsigned long nRet = metadata_buffer_size_0;
			nRet <<= 8; nRet |= metadata_buffer_size_1[0];
			nRet <<= 8; nRet |= metadata_buffer_size_1[1];
			return nRet;
		}

		inline unsigned long metadata_output_leak_rate(){
			unsigned long nRet = metadata_output_leak_rate_0;
			nRet <<= 8; nRet |= metadata_output_leak_rate_1[0];
			nRet <<= 8; nRet |= metadata_output_leak_rate_1[1];
			return nRet;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag]);
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "");
			NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved_0, "")
			unsigned long input_leak_rate = metadata_input_leak_rate();
			MBCSPRINTF_S(szTemp4, _countof(szTemp4), "%lu.%02lu Mbps", 
				(unsigned long)((unsigned long long)input_leak_rate*400/1024/1024), 
				(unsigned long)((unsigned long long)input_leak_rate*400*100/1024/1024%100));
			NAV_FIELD_PROP_2NUMBER("metadata_input_leak_rate", 22, input_leak_rate, szTemp4)

			NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved_1, "")
			unsigned long buffer_size = metadata_buffer_size();
			MBCSPRINTF_S(szTemp4, _countof(szTemp4), "%lu.%02lu Mbps", 
				(unsigned long)((unsigned long long)buffer_size*400/1024/1024), 
				(unsigned long)((unsigned long long)buffer_size*400*100/1024/1024%100));
			NAV_FIELD_PROP_2NUMBER("metadata_input_leak_rate", 22, buffer_size, szTemp4)

			NAV_FIELD_PROP_2NUMBER("reserved", 2, reserved_2, "")
			unsigned long output_leak_rate = metadata_output_leak_rate();
			MBCSPRINTF_S(szTemp4, _countof(szTemp4), "%lu.%02lu Mbps", 
				(unsigned long)((unsigned long long)output_leak_rate*400/1024/1024), 
				(unsigned long)((unsigned long long)output_leak_rate*400*100/1024/1024%100));
			NAV_FIELD_PROP_2NUMBER("metadata_input_leak_rate", 22, output_leak_rate, szTemp4)
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CAVCVideoDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		profile_idc;
#ifdef _BIG_ENDIAN_
		unsigned char		constraint_set0_flag:1;
		unsigned char		constraint_set1_flag:1;
		unsigned char		constraint_set2_flag:1;
		unsigned char		constraint_set3_flag:1;
		unsigned char		constraint_set4_flag:1;
		unsigned char		constraint_set5_flag:1;
		unsigned char		AVC_compatible_flags:2;
#else
		unsigned char		AVC_compatible_flags:2;
		unsigned char		constraint_set5_flag:1;
		unsigned char		constraint_set4_flag:1;
		unsigned char		constraint_set3_flag:1;
		unsigned char		constraint_set2_flag:1;
		unsigned char		constraint_set1_flag:1;
		unsigned char		constraint_set0_flag:1;
#endif
		unsigned char		level_idc;

#ifdef _BIG_ENDIAN_
		unsigned char		AVC_still_present:1;
		unsigned char		AVC_24_hour_picture_flag:1;
		unsigned char		Frame_Packing_SEI_not_present_flag:1;
		unsigned char		reserved:5;
#else
		unsigned char		reserved:5;
		unsigned char		Frame_Packing_SEI_not_present_flag:1;
		unsigned char		AVC_24_hour_picture_flag:1;
		unsigned char		AVC_still_present:1;
#endif

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(profile_idc, 8, get_h264_profile_name(profile_idc, constraint_set0_flag, constraint_set1_flag, constraint_set2_flag, constraint_set3_flag, constraint_set4_flag, constraint_set5_flag))
			NAV_FIELD_PROP_2NUMBER1(constraint_set0_flag, 1, "constraint_set0_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set1_flag, 1, "constraint_set1_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set2_flag, 1, "constraint_set2_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set3_flag, 1, "constraint_set3_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set4_flag, 1, "constraint_set4_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set5_flag, 1, "constraint_set5_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(AVC_compatible_flags, 2, "")
			NAV_FIELD_PROP_2NUMBER1(level_idc, 8, get_h264_level_name(level_idc, constraint_set0_flag, constraint_set1_flag, constraint_set2_flag, constraint_set3_flag, constraint_set4_flag, constraint_set5_flag))
			NAV_FIELD_PROP_2NUMBER1(AVC_still_present, 1, AVC_still_present?"the AVC video stream may include AVC still pictures":"the associated AVC video stream shall not contain AVC still pictures")
			NAV_FIELD_PROP_2NUMBER1(AVC_24_hour_picture_flag, 1, AVC_24_hour_picture_flag?"the associated AVC video stream may contain AVC 24-hour pictures":"the associated AVC video stream shall not contain any AVC 24-hour picture")
			NAV_FIELD_PROP_2NUMBER1(Frame_Packing_SEI_not_present_flag, 1, Frame_Packing_SEI_not_present_flag?"the presence of either of these SEI messages is unspecified":"the AVC video stream shall contain either the frame packing arrangement SEI message or stereo video information SEI message")
			NAV_FIELD_PROP_2NUMBER1(reserved, 5, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CAVCTimingAndHRDDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

#ifdef _BIG_ENDIAN_
		unsigned char		hrd_management_valid_flag:1;
		unsigned char		reserved_0:6;
		unsigned char		picture_and_timing_info_present:1;

		unsigned char		_90kHZ_flag:1;
		unsigned char		reserved_1:7;
#else
		unsigned char		picture_and_timing_info_present:1;
		unsigned char		reserved_0:6;
		unsigned char		hrd_management_valid_flag:1;

		unsigned char		reserved_1:7;
		unsigned char		_90kHZ_flag:1;
#endif

		unsigned long		N;
		unsigned long		K;
		/*
		num_units_in_tick is the number of time units of a clock operating at the frequency time_scale Hz that corresponds to
		one increment (called a clock tick) of a clock tick counter. num_units_in_tick shall be greater than 0. A clock tick is the
		minimum interval of time that can be represented in the coded data. For example, when the frame rate of a video signal is
		30 000 �� 1001 Hz, time_scale may be equal to 60 000 and num_units_in_tick may be equal to 1001. See Equation C-1.
		*/
		unsigned long		num_units_in_tick;

#ifdef _BIG_ENDIAN_
		unsigned char		fixed_frame_rate_flag:1;
		unsigned char		temporal_poc_flag:1;
		unsigned char		picture_to_display_conversion_flag:1;
		unsigned char		reserved_2:5;
#else
		unsigned char		reserved_2:5;
		unsigned char		picture_to_display_conversion_flag:1;
		unsigned char		temporal_poc_flag:1;
		unsigned char		fixed_frame_rate_flag:1;
#endif

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 4)
				return RET_CODE_BUFFER_TOO_SMALL;

			AMBst bst_t = AMBst_CreateFromBuffer(pBuf, cbSize);
			descriptor_tag = AMBst_GetByte(bst_t);
			descriptor_length = AMBst_GetByte(bst_t);
			hrd_management_valid_flag = AMBst_GetBits(bst_t, 1);
			reserved_0 = AMBst_GetBits(bst_t, 6);
			picture_and_timing_info_present = AMBst_GetBits(bst_t, 1);
			ulMappedSize += 3;

			if (picture_and_timing_info_present){

				if (cbSize < 9)
					return RET_CODE_BUFFER_TOO_SMALL;

				_90kHZ_flag = AMBst_GetBits(bst_t, 1);
				reserved_1 = AMBst_GetBits(bst_t, 7);

				if(!_90kHZ_flag){
					if (cbSize < 17)
						return RET_CODE_BUFFER_TOO_SMALL;

					N = AMBst_GetDWord(bst_t);
					K = AMBst_GetDWord(bst_t);
					ulMappedSize += 8;
				}

				num_units_in_tick = AMBst_GetDWord(bst_t);
				ulMappedSize += 5;
			}

			fixed_frame_rate_flag = AMBst_GetBits(bst_t, 1);
			temporal_poc_flag = AMBst_GetBits(bst_t, 1);
			picture_to_display_conversion_flag = AMBst_GetBits(bst_t, 1);
			reserved_2 = AMBst_GetBits(bst_t, 1);
			ulMappedSize += 1;

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			// No memory map, don't need do anything
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(hrd_management_valid_flag, 1, hrd_management_valid_flag?"Buffering Period SEI and Picture Timing SEI messages shall be present":"")
			NAV_FIELD_PROP_2NUMBER("reserved", 6, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(picture_and_timing_info_present, 1, picture_and_timing_info_present?"indicates that the 90kHz_flag and parameters for accurate mapping to 90-kHz system clock are included in this descriptor":"")

			if (picture_and_timing_info_present){
				NAV_FIELD_PROP_2NUMBER("90kHZ_flag", 1, _90kHZ_flag, _90kHZ_flag?"N=1, K=300, time_scale=90KHZ":"")
				NAV_FIELD_PROP_2NUMBER("reserved", 7, reserved_1, "")
				if (!_90kHZ_flag){
					NAV_FIELD_PROP_2NUMBER1(N, 32, "time_scale = (N x system_clock_frequency)/K")
					NAV_FIELD_PROP_2NUMBER1(K, 32, "time_scale = (N x system_clock_frequency)/K")
				}
				NAV_FIELD_PROP_2NUMBER1(num_units_in_tick, 32, "the number of time units of a clock operating at the frequency time_scale Hz that corresponds to one increment (called a clock tick) of a clock tick counter")
			}

			NAV_FIELD_PROP_2NUMBER1(fixed_frame_rate_flag, 1, fixed_frame_rate_flag?"the coded frame rate is constant":"")
			NAV_FIELD_PROP_2NUMBER1(temporal_poc_flag, 1, "When the temporal_poc_flag is set to '1' and the fixed_frame_rate_flag is set to '1', then the associated AVC video stream shall carry Picture Order Count (POC) information (PicOrderCnt) whereby pictures are counted in units")
			NAV_FIELD_PROP_2NUMBER1(picture_to_display_conversion_flag, 1, picture_to_display_conversion_flag
				?"may carry display information on coded pictures by providing the pic_struct field in picture_timing SEI messages and/or by providing the Picture Order Count (POC) information (PicOrderCnt), whereby pictures are counted in units"
				:"then picture timing SEI messages in the AVC video stream, if present, shall not contain the pic_struct field, and hence the pic_struct_present_flag shall be set to '0' in the VUI parameters in the AVC video stream")
			NAV_FIELD_PROP_2NUMBER("reserved", 5, reserved_2, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMPEG2AACAudioDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		MPEG2_AAC_profile;
		unsigned char		MPEG2_AAC_channel_configuration;
		unsigned char		MPEG2_AAC_additional_information;

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(MPEG2_AAC_profile, 8, MPEG2_AAC_profile==0?"Main Profile"
				:(MPEG2_AAC_profile==1?"Low Complexity profile (LC)"
				:(MPEG2_AAC_profile==2?"Scalable Sampling Rate profile (SSR)":"Reserved")))
			NAV_FIELD_PROP_2NUMBER1(MPEG2_AAC_channel_configuration, 8, "he number and configuration of audio channels presented to the listener by the AAC decoder")
			NAV_FIELD_PROP_2NUMBER1(MPEG2_AAC_additional_information, 8, MPEG2_AAC_additional_information==0?"AAC data according to ISO/IEC 13818-7:2006"
				:(MPEG2_AAC_additional_information==1?"AAC data with Bandwidth Extension data present according to ISO/IEC 13818-7:2006":"Reserved"))
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CFlexMuxTimingDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned short		FCR_ES_ID;
		unsigned long		FCRResolution;
		unsigned char		FCRLength;
		unsigned char		FmxRateLength;

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(FCR_ES_ID)
			ULONG_FIELD_ENDIAN(FCRResolution)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(FCR_ES_ID, 16, "the ES_ID associated with this clock reference stream")
			NAV_FIELD_PROP_2NUMBER1(FCRResolution, 32, "the resolution of the object time base in cycles per second")
			NAV_FIELD_PROP_2NUMBER1(FCRLength, 8, "the length of the fmxClockReference field in FlexMux packets with index = 238, value is between 0 and 64")
			NAV_FIELD_PROP_2NUMBER1(FmxRateLength, 8, "the length of the fmxRate field in FlexMux packets with index = 238, value is between 1 and 32")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMPEG4TextDescriptor: public ADVANCE_ENDIAN_MAP{

		struct CTextConfig: public ADVANCE_ENDIAN_MAP{

			struct CVideoInformation: public DIRECT_ENDIAN_MAP{
				unsigned short		video_width;
				unsigned short		video_height;
				short				horizontal_offset;
				short				vertical_offset;

				DECLARE_ENDIAN_BEGIN()
					USHORT_FIELD_ENDIAN(video_width)
					USHORT_FIELD_ENDIAN(video_height)
					USHORT_FIELD_ENDIAN(horizontal_offset)
					USHORT_FIELD_ENDIAN(vertical_offset)
				DECLARE_ENDIAN_END()

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER1(video_width, 16, "the width of the video picture in pixels")
					NAV_FIELD_PROP_2NUMBER1(video_height, 16, "the height of the video picture in pixels")
					NAV_FIELD_PROP_SIGNNUMBER1(horizontal_offset, 16, "the horizontal distance in pixels between the left border of the video picture and the left border of the Text Box; a positive distance value indicates that the left border of the Text Box is right of the left border of the video picture")
					NAV_FIELD_PROP_SIGNNUMBER1(vertical_offset, 16, "the vertical distance in pixels between the top border of the video picture and the top border of the Text Box; a positive distance value indicates that the top border of the Text Box is below the top border of the video picture")
				DECLARE_FIELDPROP_END()
			}PACKED;

			unsigned char		tag = 0;
			unsigned char		textFormat = 0;

#ifdef _BIG_ENDIAN_
			unsigned char		contains_associated_video_information:1;
			unsigned char		reserved_0:3;
			unsigned char		duration_flag:1;
			unsigned char		reserved_1:3;
#else
			unsigned char		reserved_1:3;
			unsigned char		duration_flag:1;
			unsigned char		reserved_0:3;
			unsigned char		contains_associated_video_information:1;
#endif
			CVideoInformation*	video_information = nullptr;

			unsigned char		remaining_config_length = 0;
			unsigned char*		remaining_config_data = nullptr;

			CTextConfig()
				: reserved_1(0), duration_flag(0), reserved_0(0), contains_associated_video_information(0){
			}

			virtual ~CTextConfig(){Unmap();}
			
			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0){
				unsigned long ulMappedSize = 0;
				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;
				if (cbSize < 3)
					return RET_CODE_BUFFER_TOO_SMALL;

				MAP_MEM_TO_HDR2(&tag, 3)
				MAP_MEM_TO_STRUCT_POINTER(contains_associated_video_information, video_information, CVideoInformation)
				if (cbSize < ulMappedSize + 1)
					return RET_CODE_BUFFER_TOO_SMALL;
				remaining_config_length = pBuf[ulMappedSize++];
				if (cbSize < ulMappedSize + remaining_config_length)
					return RET_CODE_BUFFER_TOO_SMALL;

				remaining_config_data = pBuf + ulMappedSize;
				ulMappedSize += remaining_config_length;
				AMP_SAFEASSIGN(desired_size, ulMappedSize);

				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL){
				if (pcbSize == NULL){
					UNMAP_STRUCT_POINTER(video_information)
				}else{
					unsigned long cbRequired = 3 + sizeof(CVideoInformation) + 1 + remaining_config_length;
					UNMAP_GENERAL_UTILITY()
				}
				return RET_CODE_SUCCESS;
			}

			int WriteToBs(AMBst bs)
			{
				AMBst_PutBits(bs, 8, tag);
				AMBst_PutBits(bs, 8, textFormat);

				AMBst_PutBits(bs, 1, contains_associated_video_information);
				AMBst_ReservedBits(bs, 3);
				AMBst_PutBits(bs, 1, duration_flag);
				AMBst_ReservedBits(bs, 3);

				UNMAP_DUPLICATE_STRUCT_POINTER(video_information, CVideoInformation)

				AMBst_PutBits(bs, 8, remaining_config_length);
				AMBst_PutBytes(bs, remaining_config_data, remaining_config_length);

				return RET_CODE_SUCCESS;
			}

			DECLARE_FIELDPROP_BEGIN()
				NAV_FIELD_PROP_2NUMBER1(tag, 8, "")
				NAV_FIELD_PROP_2NUMBER1(textFormat, 8, "the format of textData() contained in text access units")
				NAV_FIELD_PROP_2NUMBER1(contains_associated_video_information, 1, "one bit that signals the presence of the video-width, video-height, horizontal-offset, and vertical-offset fields in this TextConfig")
				NAV_FIELD_PROP_2NUMBER("reserved", reserved_0, 3, "")
				NAV_FIELD_PROP_2NUMBER1(duration_flag, 1, "one bit signaling the presence of the duration field in each text access unit")
				NAV_FIELD_PROP_2NUMBER("reserved", reserved_0, 3, "")
				NAV_FIELD_PROP_REF(video_information)
				NAV_FIELD_PROP_2NUMBER1(remaining_config_length, 8, "specifying the number of immediately following remaining-config-data bytes")
				NAV_FIELD_PROP_FIXSIZE_BINSTR1(remaining_config_data, (unsigned long)remaining_config_length*8, "one or more bytes representing remainingConfig(), that is additional configuration data needed to decode and render the text contained in text access units")
			DECLARE_FIELDPROP_END()

		}PACKED;

		unsigned char		descriptor_tag = 0;
		unsigned char		descriptor_length = 0;
		CTextConfig*		textConfig = nullptr;

		CMPEG4TextDescriptor(){}
		virtual ~CMPEG4TextDescriptor(){Unmap();}
			
		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0){
			unsigned long ulMappedSize = 0;
			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;
			if (cbSize < 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_HDR2(&descriptor_tag, 2)
			MAP_MEM_TO_STRUCT_POINTER2(1, textConfig, CTextConfig)
			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL){
			if (pcbSize == NULL){
				UNMAP_STRUCT_POINTER2(textConfig)
			}else{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);
			if (textConfig)
				textConfig->WriteToBs(bs);
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_REF(textConfig)
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMPEG4AudioExtensionDescriptor: public ADVANCE_ENDIAN_MAP{
		unsigned char		descriptor_tag = 0;
		unsigned char		descriptor_length = 0;

#ifdef _BIG_ENDIAN_
		unsigned char		ASC_flag:1;
		unsigned char		reserved:3;
		unsigned char		num_of_loops:4;
#else
		unsigned char		num_of_loops:4;
		unsigned char		reserved:3;
		unsigned char		ASC_flag:1;
#endif
		unsigned char*		audioProfileLevelIndications;

		unsigned char		ASC_size = 0;
		AACAudio::CAudioSpecificConfig*
							audioSpecificConfig;

		CMPEG4AudioExtensionDescriptor()
			: num_of_loops(0), reserved(0), ASC_flag(0), audioProfileLevelIndications(NULL), audioSpecificConfig(NULL){}
		virtual ~CMPEG4AudioExtensionDescriptor(){Unmap();}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0){
			unsigned long ulMappedSize = 0;
			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;
			if (cbSize < 3)
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_HDR2(&descriptor_tag, 3)
			if (cbSize < 3 + num_of_loops*sizeof(char))
				return RET_CODE_BUFFER_TOO_SMALL;
			audioProfileLevelIndications = pBuf + ulMappedSize;
			ulMappedSize += num_of_loops*sizeof(char);

			if (cbSize > ulMappedSize)
			{
				AMBst bst = AMBst_CreateFromBuffer(pBuf + ulMappedSize, cbSize - ulMappedSize);
				MAP_MEM_TO_STRUCT_POINTER5(1, audioSpecificConfig, AACAudio::CAudioSpecificConfig);

				ulMappedSize += AMBst_Tell(bst) >> 3;

				AMBst_Destroy(bst);
			};

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL){
			if (pcbSize == NULL)
			{
				UNMAP_STRUCT_POINTER2(audioSpecificConfig)
				audioProfileLevelIndications = NULL;
			}
			else
			{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);

			AMBst_PutBits(bs, 1, ASC_flag);
			AMBst_ReservedBits(bs, 3);
			AMBst_PutBits(bs, 3, num_of_loops);

			AMBst_PutBytes(bs, audioProfileLevelIndications, num_of_loops);

			if (ASC_flag){
				audioSpecificConfig->WriteToBs(bs);
			}

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(ASC_flag, 1, "A one-bit flag signalling the presence of the ASC_size field in this descriptor")
			NAV_FIELD_PROP_2NUMBER1(reserved, 3, "")
			NAV_FIELD_PROP_2NUMBER1(num_of_loops, 4, "the number of immediately following audioprofileLevelIndication fields")
			for(i=0;i<(int)num_of_loops;i++){
				MBCSPRINTF_S(szTemp4, _countof(szTemp4), "audioProfileLevelIndication_%d", i);
				NAV_FIELD_PROP_2NUMBER(szTemp4, 8, audioProfileLevelIndications[i], audioProfileLevelIndication_names[audioProfileLevelIndications[i]])
			}
			if (ASC_flag)
			{
				NAV_FIELD_PROP_2NUMBER1(ASC_size, 8, "")
				NAV_FIELD_PROP_REF(audioSpecificConfig)
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CAuxiliaryVideoStreamDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned char		aux_video_codedstreamtype;
		unsigned char		si_rbsp[];

		int GetVarBodySize(){return descriptor_length-1;}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(aux_video_codedstreamtype, 8, "indicates the compression coding type of the auxiliary video stream")
			NAV_FIELD_PROP_FIXSIZE_BINSTR1(si_rbsp, (unsigned long)(descriptor_length-1)*8, "Supplemental information RBSP as defined in ISO/IEC 23002-3")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CSVCExtensionDescriptor: public DIRECT_ENDIAN_MAP {
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned short		width;
		unsigned short		height;
		unsigned short		frame_rate;
		unsigned short		average_bitrate;
		unsigned short		maximum_bitrate;

#ifdef _BIG_ENDIAN_
		unsigned char		dependency_id:3;
		unsigned char		reserved_0:5;

		unsigned char		quality_id_start:4;
		unsigned char		quality_id_end:4;

		unsigned char		temporal_id_start:3;
		unsigned char		temporal_id_end:3;
		unsigned char		no_sei_nal_unit_present:1;
		unsigned char		reserved_1:1;
#else
		unsigned char		reserved_0:5;
		unsigned char		dependency_id:3;

		unsigned char		quality_id_end:4;
		unsigned char		quality_id_start:4;

		unsigned char		reserved_1:1;
		unsigned char		no_sei_nal_unit_present:1;
		unsigned char		temporal_id_end:3;
		unsigned char		temporal_id_start:3;
#endif

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(width)
			USHORT_FIELD_ENDIAN(height)
			USHORT_FIELD_ENDIAN(frame_rate)
			USHORT_FIELD_ENDIAN(average_bitrate)
			USHORT_FIELD_ENDIAN(maximum_bitrate)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(width, 16, "the maximum image width resolution, in pixels of the re-assembled AVC video stream")
			NAV_FIELD_PROP_2NUMBER1(height, 16, "the maximum image height resolution, in pixels of the re-assembled AVC video stream")
			NAV_FIELD_PROP_2NUMBER1(frame_rate, 16, "the maximum frame rate, in frames/256 seconds of the re-assembled AVC video stream")
			NAV_FIELD_PROP_2NUMBER1(average_bitrate, 16, "the average bit rate, in kbit per second, of the re-assembled AVC video stream")
			NAV_FIELD_PROP_2NUMBER1(maximum_bitrate, 16, "the maximum bit rate, in kbit per second, of the re-assembled AVC video stream")

			NAV_FIELD_PROP_2NUMBER1(dependency_id, 3, "the value of dependency_id associated with the video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER("reserved", 5, reserved_0, "")

			NAV_FIELD_PROP_2NUMBER1(quality_id_start, 4, "the minimum value of the quality_id of the NAL unit header syntax element of all the NAL units contained in the associated video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(quality_id_end, 4, "the maximum value of the quality_id of the NAL unit header syntax element of all the NAL units contained in the associated video sub-bitstream")

			NAV_FIELD_PROP_2NUMBER1(temporal_id_start, 3, "the minimum value of the temporal_id of the NAL unit header syntax element of all the NAL units contained in the associated video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(temporal_id_end, 3, "the maximum value of the temporal_id of the NAL unit header syntax element of all the NAL units contained in the associated video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(no_sei_nal_unit_present, 1, no_sei_nal_unit_present?"no SEI NAL units are present in the associated video sub-bitstream":"")
			NAV_FIELD_PROP_2NUMBER("reserved", 1, reserved_1, "")
		DECLARE_FIELDPROP_END()

	}PACKED;

	struct CMVCExtensionDescriptor: public DIRECT_ENDIAN_MAP {
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		unsigned short		average_bitrate;
		unsigned short		maximum_bitrate;

		union {
			struct {
#ifdef _BIG_ENDIAN_
				unsigned long		reserved:4;
				unsigned long		view_order_index_min:10;
				unsigned long		view_order_index_max:10;
				unsigned long		temporal_id_start:3;
				unsigned long		temporal_id_end:3;
				unsigned long		no_sei_nal_unit_present:1;
				unsigned long		no_prefix_nal_unit_present:1;
#else
				unsigned long		no_prefix_nal_unit_present:1;
				unsigned long		no_sei_nal_unit_present:1;
				unsigned long		temporal_id_end:3;
				unsigned long		temporal_id_start:3;
				unsigned long		view_order_index_max:10;
				unsigned long		view_order_index_min:10;
				unsigned long		reserved:4;
#endif
			}PACKED;
			unsigned long	long_value;
		}PACKED;

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(average_bitrate)
			USHORT_FIELD_ENDIAN(maximum_bitrate)
			ULONG_FIELD_ENDIAN(long_value)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(average_bitrate, 16, "the average bit rate, in kbit per second, of the re-assembled AVC video stream")
			NAV_FIELD_PROP_2NUMBER1(maximum_bitrate, 16, "the maximum bit rate, in kbit per second, of the re-assembled AVC video stream")
			NAV_FIELD_PROP_2NUMBER1(reserved, 4, "")
			NAV_FIELD_PROP_2NUMBER1(view_order_index_min, 10, "the minimum value of the view order index of all the NAL units contained in the associated MVC video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(view_order_index_max, 10, "the maximum value of the view order index of all the NAL units contained in the associated MVC video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(temporal_id_start, 3, "the minimum value of the temporal_id of the NAL unit header syntax element of all the NAL units contained in the associated MVC video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(temporal_id_end, 3, "the maximum value of the temporal_id of the NAL unit header syntax element of all the NAL units contained in the associated MVC video sub-bitstream")
			NAV_FIELD_PROP_2NUMBER1(no_sei_nal_unit_present, 1, no_sei_nal_unit_present?"no SEI NAL units are present in the associated video sub�Cbitstream":"")
			NAV_FIELD_PROP_2NUMBER1(no_prefix_nal_unit_present, 1, no_prefix_nal_unit_present?"no prefix NAL units are present in either the AVC video sub-bitstream of MVC or MVC video sub-bitstreams"
				:"prefix NAL units are present in the AVC video sub-bitstream of MVC only")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CJ2KVideoDescriptor: public DIRECT_ENDIAN_MAP {
		unsigned char	descriptor_tag;
		unsigned char	descriptor_length;
		unsigned short	profile_and_level;
		unsigned long	horizontal_size;
		unsigned long	vertical_size;
		unsigned long	max_bit_rate;
		unsigned long	max_buffer_size;
		unsigned short	DEN_frame_rate;
		unsigned short	NUM_frame_rate;
		unsigned char	color_specification;
#ifdef _BIG_ENDIAN_
		unsigned char	still_mode:1;
		unsigned char	interlaced_video:1;
		unsigned char	reserved:6;
#else
		unsigned char	reserved:6;
		unsigned char	interlaced_video:1;
		unsigned char	still_mode:1;
#endif
		unsigned char	private_data_byte[];

		DECLARE_ENDIAN_BEGIN()
			USHORT_FIELD_ENDIAN(profile_and_level)
			ULONG_FIELD_ENDIAN(horizontal_size)
			ULONG_FIELD_ENDIAN(vertical_size)
			ULONG_FIELD_ENDIAN(max_bit_rate)
			ULONG_FIELD_ENDIAN(max_buffer_size)
			USHORT_FIELD_ENDIAN(DEN_frame_rate)
			USHORT_FIELD_ENDIAN(NUM_frame_rate)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(profile_and_level, 16, "This field shall be in the range 0x0101-0x04ff and coded as defined in Table A.10 of Rec. ITU-T T.800 | ISO/IEC 15444-1 and indicates broadcast profile and level values")
			NAV_FIELD_PROP_2NUMBER1(horizontal_size, 32, "This field shall be coded the same as Xsiz parameter found in the J2K codestream main header, as defined in Annex A of Rec. ITU-T T.800 | ISO/IEC 15444-1")
			NAV_FIELD_PROP_2NUMBER1(vertical_size, 32, "This field shall be coded the same as Ysiz parameter found in the J2K codestream main header, as defined in Annex A of Rec. ITU-T T.800 | ISO/IEC 15444-1")
			NAV_FIELD_PROP_2NUMBER1(max_bit_rate, 32, "This field may be coded the same as the Maxbr value in the j2k_brat field box specified in Table S.1 and shall not exceed the maximum compressed bit rate value for the profile and level specified in Table S.2")
			NAV_FIELD_PROP_2NUMBER1(max_buffer_size, 32, "This field shall not exceed the Maximum buffer size value for the profile and level specified in the j2k_brat box in Table S.2")
			NAV_FIELD_PROP_2NUMBER1(DEN_frame_rate, 16, "This field shall be coded the same as frat_denominator field in the j2k_frat box specified in Table S.1")
			NAV_FIELD_PROP_2NUMBER1(NUM_frame_rate, 16, "This field shall be coded the same as frat_numerator field in the frat box specified in Table S.1")
			NAV_FIELD_PROP_2NUMBER1(color_specification, 8, "This field shall be coded the same as the bcol_colrc 8-bit field of the j2k_bcol box as specified in Table S.1")
			NAV_FIELD_PROP_2NUMBER1(still_mode, 1, still_mode?"the J2K video stream may include J2K still pictures":"the associated J2K video stream shall not contain J2K still pictures")
			NAV_FIELD_PROP_2NUMBER1(interlaced_video, 1, interlaced_video?"the J2K access unit elementary stream header (see Table S.1) shall include the syntax elements Auf2, fiel_box_code, fic and fio"
				:"syntax elements Auf2, fiel_box_code, fic and fio shall not be present in the J2K access unit elementary stream header")
			NAV_FIELD_PROP_2NUMBER1(reserved, 6, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMVCOperationPointDescriptor: public ADVANCE_ENDIAN_MAP{

		struct CLevel: public ADVANCE_ENDIAN_MAP{

			struct COperationPoint: public DIRECT_ENDIAN_MAP{

				struct CES: public DIRECT_ENDIAN_MAP{
#ifdef _BIG_ENDIAN_
					unsigned char		reserved:2;
					unsigned char		ES_reference:6;
#else
					unsigned char		ES_reference:6;
					unsigned char		reserved:2;
#endif

					DECLARE_FIELDPROP_BEGIN()
						NAV_FIELD_PROP_2NUMBER1(reserved, 2, "")
						NAV_FIELD_PROP_2NUMBER1(ES_reference, 6, "")
					DECLARE_FIELDPROP_END()
				}PACKED;

#ifdef _BIG_ENDIAN_
				unsigned char		reserved:5;
				unsigned char		applicable_temporal_id:3;
#else
				unsigned char		applicable_temporal_id:3;
				unsigned char		reserved:5;
#endif
				unsigned char		num_target_output_views;
				unsigned char		ES_count;

				CES					es[];

				int GetVarBodySize(){return ES_count;}

				DECLARE_FIELDPROP_BEGIN()
					NAV_FIELD_PROP_2NUMBER1(reserved, 5, "")
					NAV_FIELD_PROP_2NUMBER1(applicable_temporal_id, 3, "the highest value of the temporal_id of the VCL NAL units in the re-assembled AVC video stream")
					NAV_FIELD_PROP_2NUMBER1(num_target_output_views, 8, "the value of the number of the views, targeted for output for the associated operation point")
					NAV_FIELD_PROP_2NUMBER1(ES_count, 8, "the number of ES_reference values included in the following group of data elements")
					for(i=0;i<(int)ES_count;i++){
						NAV_WRITE_TAG_BEGIN3("ES", i);
							NAV_FIELD_PROP_OBJECT(es[i])
						NAV_WRITE_TAG_END3("ES", i);
					}
				DECLARE_FIELDPROP_END()				
			}PACKED;

			unsigned char		level_idc = 0;
			unsigned char		operation_points_count = 0;
			COperationPoint**	operation_points;

			CLevel():operation_points(NULL){}
			virtual ~CLevel(){Unmap();}

			int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
			{
				unsigned long ulMappedSize = 0;

				if (pBuf == NULL)
					return RET_CODE_BUFFER_NOT_FOUND;

				if (cbSize < 2)
					return RET_CODE_BUFFER_TOO_SMALL;

				level_idc = pBuf[0];
				operation_points_count = pBuf[1];
				ulMappedSize += 2;

				AMP_NEW1(operation_points, COperationPoint*, operation_points_count);
				if (operation_points != nullptr) {
					for (unsigned char i = 0; i < operation_points_count; i++) {
						MAP_MEM_TO_STRUCT_POINTER(1, operation_points[i], COperationPoint)
					}
				}

				AMP_SAFEASSIGN(desired_size, ulMappedSize);

				return RET_CODE_SUCCESS;
			}

			int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
			{
				for(unsigned char i=0;i<operation_points_count && operation_points != NULL;i++){
					UNMAP_STRUCT_POINTER(operation_points[i])
				}
				AMP_SAFEDELA2(operation_points);
				return RET_CODE_SUCCESS;
			}

		}PACKED;

		unsigned char		descriptor_tag = 0;
		unsigned char		descriptor_length = 0;
		unsigned char		profile_idc = 0;
#ifdef _BIG_ENDIAN_
		unsigned char		constraint_set0_flag:1;
		unsigned char		constraint_set1_flag:1;
		unsigned char		constraint_set2_flag:1;
		unsigned char		constraint_set3_flag:1;
		unsigned char		constraint_set4_flag:1;
		unsigned char		constraint_set5_flag:1;
		unsigned char		AVC_compatible_flags:2;
#else
		unsigned char		AVC_compatible_flags:2;
		unsigned char		constraint_set5_flag:1;
		unsigned char		constraint_set4_flag:1;
		unsigned char		constraint_set3_flag:1;
		unsigned char		constraint_set2_flag:1;
		unsigned char		constraint_set1_flag:1;
		unsigned char		constraint_set0_flag:1;
#endif
		unsigned char		level_count = 0;

		CLevel**			levels;

		CMVCOperationPointDescriptor()
			: AVC_compatible_flags(0), constraint_set5_flag(0), constraint_set4_flag(0), constraint_set3_flag(0)
			, constraint_set2_flag(0), constraint_set1_flag(0), constraint_set0_flag(0), levels(NULL){
		}

		virtual ~CMVCOperationPointDescriptor(){Unmap();}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 2)
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_HDR2(&descriptor_tag, 5);

			AMP_NEW1(levels, CLevel*, level_count);
			if (levels != nullptr) {
				for (unsigned char i = 0; i < level_count; i++) {
					MAP_MEM_TO_STRUCT_POINTER2(1, levels[i], CLevel)
				}
			}

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			if (pcbSize == NULL)
			{
				for(unsigned char i=0;i<level_count && levels != NULL;i++){
					UNMAP_STRUCT_POINTER2(levels[i])
				}
				AMP_SAFEDELA2(levels);
			}
			else
			{
				unsigned long cbRequired = descriptor_length + 2;
				UNMAP_GENERAL_UTILITY()
			}
			return RET_CODE_SUCCESS;
		}

		int WriteToBs(AMBst bs)
		{
			AMBst_PutBits(bs, 8, descriptor_tag);
			AMBst_PutBits(bs, 8, descriptor_length);

			AMBst_PutBits(bs, 8, profile_idc);
			AMBst_PutBits(bs, 1, constraint_set0_flag);
			AMBst_PutBits(bs, 1, constraint_set1_flag);
			AMBst_PutBits(bs, 1, constraint_set2_flag);
			AMBst_PutBits(bs, 1, constraint_set3_flag);
			AMBst_PutBits(bs, 1, constraint_set4_flag);
			AMBst_PutBits(bs, 1, constraint_set5_flag);
			AMBst_PutBits(bs, 2, AVC_compatible_flags);

			AMBst_PutBits(bs, 8, level_count);
			for(int i=0;i<level_count;i++)
			{
				AMBst_PutBits(bs, 8, levels[i]->level_idc);
				AMBst_PutBits(bs, 8, levels[i]->operation_points_count);
				for(int j=0;j<levels[i]->operation_points_count;j++)
				{
					AMBst_ReservedBits(bs, 5);
					AMBst_PutBits(bs, 3, levels[i]->operation_points[j]->applicable_temporal_id);
					AMBst_PutBits(bs, 8, levels[i]->operation_points[j]->num_target_output_views);
					AMBst_PutBits(bs, 8, levels[i]->operation_points[j]->ES_count);
					for(int k=0;k<levels[i]->operation_points[j]->ES_count;k++)
					{
						AMBst_ReservedBits(bs, 2);
						AMBst_PutBits(bs, 6, levels[i]->operation_points[j]->es[k].ES_reference);
					}
				}
			}

			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(profile_idc, 8, get_h264_profile_name(profile_idc, constraint_set0_flag, constraint_set1_flag, constraint_set2_flag, constraint_set3_flag, constraint_set4_flag, constraint_set5_flag))
			NAV_FIELD_PROP_2NUMBER1(constraint_set0_flag, 1, "constraint_set0_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set1_flag, 1, "constraint_set1_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set2_flag, 1, "constraint_set2_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set3_flag, 1, "constraint_set3_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set4_flag, 1, "constraint_set4_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(constraint_set5_flag, 1, "constraint_set5_flag in SPS")
			NAV_FIELD_PROP_2NUMBER1(AVC_compatible_flags, 2, "")
			NAV_FIELD_PROP_2NUMBER1(level_count, 8, "the number of levels for which operation points are described")
			for(i=0;i<(int)level_count;i++){
				NAV_WRITE_TAG_BEGIN3("level", i);
				NAV_FIELD_PROP_2NUMBER1(levels[i]->level_idc, 8, get_h264_level_name(levels[i]->level_idc, constraint_set0_flag, constraint_set1_flag, constraint_set2_flag, constraint_set3_flag, constraint_set4_flag, constraint_set5_flag))
				NAV_FIELD_PROP_2NUMBER1(levels[i]->operation_points_count, 8, "the number of operation points described by the list included in the following group of data elements")
				for(i=0;i<(int)levels[i]->operation_points_count;i++){
					NAV_FIELD_PROP_REF(levels[i]->operation_points[i])
				}
				NAV_WRITE_TAG_END3("level", i);
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CMPEG2StereoscopicVideoFormatDescriptor: public DIRECT_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
		union{
#ifdef _BIG_ENDIAN_
			struct {
				unsigned char		Stereo_video_arrangement_type_present:1;
				unsigned char		arrangement_type:7;
			}PACKED;
			struct {
				unsigned char		Stereo_video_arrangement_type_present_2:1;
				unsigned char		reserved:7;
			}PACKED;
#else
			struct {
				unsigned char		arrangement_type:7;
				unsigned char		Stereo_video_arrangement_type_present:1;
			}PACKED;
			struct {
				unsigned char		reserved:7;
				unsigned char		Stereo_video_arrangement_type_present_2:1;
			}PACKED;
#endif
		}PACKED;

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(Stereo_video_arrangement_type_present, 1, "")
			if (Stereo_video_arrangement_type_present){
				NAV_FIELD_PROP_2NUMBER1(arrangement_type, 7, "")
			}else{
				NAV_FIELD_PROP_2NUMBER1(reserved, 7, "")
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CStereoscopicProgramInfoDescriptor: public DIRECT_ENDIAN_MAP {
		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;
#ifdef _BIG_ENDIAN_
		unsigned char		reserved:5
		unsigned char		stereoscopic_service_type:3;
#else
		unsigned char		stereoscopic_service_type:3;
		unsigned char		reserved:5;
#endif

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER1(stereoscopic_service_type, 3, stereoscopic_service_type==0?"unspecified"
				:(stereoscopic_service_type==1?"2D-only (monoscopic) service"
				:(stereoscopic_service_type==2?"Frame-compatible stereoscopic 3D service"
				:(stereoscopic_service_type==3?"Service-compatible stereoscopic 3D service":"Rec. ITU-T H.222.0 | ISO/IEC 13818-1 reserved"))))
			NAV_FIELD_PROP_2NUMBER1(reserved, 5, "")
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CStereoscopicVideoInfoDescriptor: public ADVANCE_ENDIAN_MAP {
		unsigned char		descriptor_tag = 0;
		unsigned char		descriptor_length = 0;

#ifdef _BIG_ENDIAN_
		unsigned char		reserved_0:7;
		unsigned char		base_video_flag:1;
#else
		unsigned char		base_video_flag:1;
		unsigned char		reserved_0:7;
#endif
		union {
			struct {
				unsigned char		reserved_1:7;
				unsigned char		leftview_flag:1;
			}PACKED;
			struct {
				unsigned char		reserved2_1:7;
				unsigned char		usable_as_2D:1;
				unsigned char		horizontal_upsampling_factor:4;
				unsigned char		vertical_upsampling_factor:4;
			}PACKED;
			unsigned char	char_value[2];
		}PACKED;

		virtual ~CStereoscopicVideoInfoDescriptor() {}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0)
		{
			unsigned long ulMappedSize = 0;

			if (pBuf == NULL)
				return RET_CODE_BUFFER_NOT_FOUND;

			if (cbSize < 4)
				return RET_CODE_BUFFER_TOO_SMALL;

			MAP_MEM_TO_HDR2(&descriptor_tag, 4)
			if (!base_video_flag){
				if (cbSize < 5)
					return RET_CODE_BUFFER_TOO_SMALL;
				char_value[1] = pBuf[ulMappedSize];
				ulMappedSize++;
			}

			AMP_SAFEASSIGN(desired_size, ulMappedSize);

			return RET_CODE_SUCCESS;
		}

		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL)
		{
			return RET_CODE_SUCCESS;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(descriptor_tag, 8, descriptor_tag_names[descriptor_tag])
			NAV_FIELD_PROP_2NUMBER1(descriptor_length, 8, "")
			NAV_FIELD_PROP_2NUMBER ("reserved", 7, reserved_0, "")
			NAV_FIELD_PROP_2NUMBER1(base_video_flag, 1, base_video_flag?"base video stream":"additional view video stream")
			NAV_FIELD_PROP_2NUMBER ("reserved", 7, reserved_1, "")
			if (base_video_flag){
				NAV_FIELD_PROP_2NUMBER1(leftview_flag, 1, leftview_flag?"the left view":"the right view")
			}else{
				NAV_FIELD_PROP_2NUMBER1(usable_as_2D, 1, usable_as_2D?"the additional view video stream may also be used for a 2D video service":"")
				NAV_FIELD_PROP_2NUMBER1(horizontal_upsampling_factor, 4, horizontal_upsampling_factor==0?"Forbidden"
					:(horizontal_upsampling_factor==1?"unspecified"
					:(horizontal_upsampling_factor==2?"Coded resolution is same as coded resolution of base view"
					:(horizontal_upsampling_factor==3?"Coded resolution is 3/4 coded resolution of base view"
					:(horizontal_upsampling_factor==4?"Coded resolution is 2/3 coded resolution of base view"
					:(horizontal_upsampling_factor==5?"Coded resolution is 1/2 coded resolution of base view"
					:(horizontal_upsampling_factor>=6 && horizontal_upsampling_factor<=8?"reserved":"user_private")))))))
				NAV_FIELD_PROP_2NUMBER1(vertical_upsampling_factor, 4, vertical_upsampling_factor==0?"Forbidden"
					:(vertical_upsampling_factor==1?"unspecified"
					:(vertical_upsampling_factor==2?"Coded resolution is same as coded resolution of base view"
					:(vertical_upsampling_factor==3?"Coded resolution is 3/4 coded resolution of base view"
					:(vertical_upsampling_factor==4?"Coded resolution is 2/3 coded resolution of base view"
					:(vertical_upsampling_factor==5?"Coded resolution is 1/2 coded resolution of base view"
					:(vertical_upsampling_factor>=6 && vertical_upsampling_factor<=8?"reserved":"user_private")))))))
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct CHEVCDescriptor : public DIRECT_ENDIAN_MAP
	{
		struct TEMPORAL_ID
		{
#ifdef _BIG_ENDIAN_
			unsigned char	reserved_0 : 5;
			unsigned char	temporal_id_min : 3;

			unsigned char	reserved_1 : 5;
			unsigned char	temporal_id_max : 3;
#else
			unsigned char	temporal_id_min : 3;
			unsigned char	reserved_0 : 5;

			unsigned char	temporal_id_max : 3;
			unsigned char	reserved_1 : 5;
#endif
		}PACKED;

		unsigned char		descriptor_tag;
		unsigned char		descriptor_length;

#ifdef _BIG_ENDIAN_
		unsigned char		profile_space : 2;
		unsigned char		tier_flag : 1;
		unsigned char		profile_idc : 5;

		unsigned long		profile_compatibility_indication;

		unsigned char		progressive_source_flag : 1;
		unsigned char		interlaced_source_flag : 1;
		unsigned char		non_packed_constraint_flag : 1;
		unsigned char		frame_only_constraint_flag : 1;
		unsigned char		reserved_0 : 4;

		unsigned char		reserved_1[5];

		unsigned char		level_idc;

		unsigned char		temporal_layer_subset_flag : 1;
		unsigned char		HEVC_still_present_flag : 1;
		unsigned char		HEVC_24hr_picture_present_flag : 1;
		unsigned char		reserved_2 : 5;
#else
		unsigned char		profile_idc : 5;
		unsigned char		tier_flag : 1;
		unsigned char		profile_space : 2;

		unsigned long		profile_compatibility_indication;

		unsigned char		reserved_0 : 4;
		unsigned char		frame_only_constraint_flag : 1;
		unsigned char		non_packed_constraint_flag : 1;
		unsigned char		interlaced_source_flag : 1;
		unsigned char		progressive_source_flag : 1;

		unsigned char		reserved_1[5];

		unsigned char		level_idc;

		unsigned char		reserved_2 : 5;
		unsigned char		HEVC_24hr_picture_present_flag : 1;
		unsigned char		HEVC_still_present_flag : 1;
		unsigned char		temporal_layer_subset_flag : 1;
#endif

		TEMPORAL_ID			temporal_id[];

		int GetVarBodySize() { return temporal_layer_subset_flag ? sizeof(TEMPORAL_ID) : 0; }

		DECLARE_ENDIAN_BEGIN()
			ULONG_FIELD_ENDIAN(profile_compatibility_indication)
		DECLARE_ENDIAN_END()

		DECLARE_FIELDPROP_BEGIN()
			NAV_FIELD_PROP_2NUMBER1(profile_space, 2, "")
			NAV_FIELD_PROP_NUMBER1(tier_flag, 1, "")
			NAV_FIELD_PROP_2NUMBER1(profile_idc, 5, "")

			NAV_FIELD_PROP_2NUMBER1(profile_compatibility_indication, 32, "");

			NAV_FIELD_PROP_NUMBER1(progressive_source_flag, 1, progressive_source_flag ? "" : "")
			NAV_FIELD_PROP_NUMBER1(interlaced_source_flag, 1, interlaced_source_flag ? "" : "")
			NAV_FIELD_PROP_NUMBER1(non_packed_constraint_flag, 1, non_packed_constraint_flag ? "" : "")
			NAV_FIELD_PROP_NUMBER1(frame_only_constraint_flag, 1, frame_only_constraint_flag ? "" : "")

			NAV_FIELD_PROP_2NUMBER("reserved", 4, reserved_0, "")

			NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved", 40, reserved_1, 5, "")

			NAV_FIELD_PROP_NUMBER1(temporal_layer_subset_flag, 1, temporal_layer_subset_flag ?
				"the syntax elements describing a subset of temporal layers are included in this descriptor" :
				"the syntax elements temporal_id_min and temporal_id_max are not included in this descriptor")
			NAV_FIELD_PROP_NUMBER1(HEVC_still_present_flag, 1, HEVC_still_present_flag ?
				"the HEVC video stream or the HEVC highest temporal sub - layer representation may include HEVC still pictures" :
				"the associated HEVC video stream shall not contain HEVC still pictures")
			NAV_FIELD_PROP_NUMBER1(HEVC_24hr_picture_present_flag, 1, HEVC_24hr_picture_present_flag ?
				"that the associated HEVC video stream or the HEVC highest temporal sub - layer representation may contain HEVC 24 - hour pictures" :
				"the associated HEVC video stream shall not contain any HEVC 24 - hour pictures")
			NAV_FIELD_PROP_2NUMBER("reserved", 5, reserved_2, "")

			if (temporal_layer_subset_flag)
			{
				NAV_FIELD_PROP_2NUMBER("reserved", 5, temporal_id->reserved_0, "")
					NAV_FIELD_PROP_2NUMBER("temporal_id_min", 3, temporal_id->temporal_id_min, "the minimum value of the TemporalId")
					NAV_FIELD_PROP_2NUMBER("reserved", 5, temporal_id->reserved_1, "")
					NAV_FIELD_PROP_2NUMBER("temporal_id_max", 3, temporal_id->temporal_id_max, "the maximum value of the TemporalId")
			}
		DECLARE_FIELDPROP_END()
	}PACKED;

	struct C13818_1_Descriptor: public ADVANCE_ENDIAN_MAP{
		unsigned char		descriptor_tag;
		unsigned char		sub_tag;

		union {
			CVideoStreamDescriptor* video_stream_descriptor;
			CAudioStreamDescriptor* audio_stream_descriptor;
			CHierarchyDescriptor* hierarchy_descriptor;
			union
			{
				CRegistrationDescriptor* registration_descriptor;								// sub tag: 0
				CHDMVRegistrationDescriptor* HDMV_registration_descriptor;						// sub tag: 1
				CHDMVVideoRegistrationDescriptor* HDMV_video_registration_descriptor;			// sub tag: 2
				CVC1RegistrationDescriptor* VC1_registration_descriptor;						// sub tag: 3
				CHDMVLPCMAudioRegistrationDescriptor* HDMV_LPCM_audio_registration_descriptor;	// sub tag: 4
				CAC3RegistrationDescriptor* AC3_registration_descriptor;						// sub tag: 5
				CDRARegistrationDescriptor* DRA_registration_descriptor;						// sub tag: 6
			}PACKED;
			CDataStreamAlignmentDescriptor* data_stream_alignment_descriptor;
			CTargetBackgroundGridDescriptor* target_background_grid_descriptor;
			CVideoWindowDescriptor* video_window_descriptor;
			CCADescriptor* CA_descriptor;
			CISO639LanguageDescriptor* ISO_639_language_descriptor;
			CSystemClockDescriptor* system_clock_descriptor;
			CMultiplexBufferUtilizationDescriptor* multiplex_buffer_utilization_descriptor;
			CCopyrightDescriptor* copyright_descriptor;
			CMaximumBitrateDescriptor* maximum_bitrate_descriptor;
			CPrivateDataIndicatorDescriptor* private_data_indicator_descriptor;
			CSmoothingBufferDescriptor* smoothing_buffer_descriptor;
			CSTDDescriptor* STD_descriptor;
			CIBPDescriptor* IBP_descriptor;
			CMPEG4VideoDescriptor* MPEG4_video_descriptor;
			CMPEG4AudioDescriptor* MPEG4_audio_descriptor;
			CIODDescriptor* IOD_descriptor;
			CSLDescriptor* SL_descriptor;
			CFMCDescriptor* FMC_descriptor;
			CExternalESIDDescriptor* external_ES_ID_descriptor;
			CMuxcodeDescriptor* MuxCode_descriptor;
			CFmxBufferSizeDescriptor* FmxBufferSize_descriptor;
			CMultiplexBufferDescriptor* multiplexbuffer_descriptor;
			CContentLabelingDescriptor* content_labeling_descriptor;
			CMetadataPointerDescriptor* metadata_pointer_descriptor;
			CMetadataDescriptor* metadata_descriptor;
			CMetadataSTDDescriptor* metadata_STD_descriptor;
			CAVCVideoDescriptor* AVC_video_descriptor;
			CAVCTimingAndHRDDescriptor* AVC_timing_and_HRD_descriptor;
			CMPEG2AACAudioDescriptor* MPEG2_AAC_audio_descriptor;
			CFlexMuxTimingDescriptor* FlexMuxTiming_descriptor;
			CMPEG4TextDescriptor* MPEG4_text_descriptor;
			CMPEG4AudioExtensionDescriptor* MPEG4_audio_extension_descriptor;
			CAuxiliaryVideoStreamDescriptor* auxiliary_video_stream_descriptor;
			CSVCExtensionDescriptor* SVC_extension_descriptor;
			CMVCExtensionDescriptor* MVC_extension_descriptor;
			CJ2KVideoDescriptor* J2K_video_descriptor;
			CMVCOperationPointDescriptor* MVC_operation_point_descriptor;
			CMPEG2StereoscopicVideoFormatDescriptor* MPEG2_stereoscopic_video_format_descriptor;
			CStereoscopicProgramInfoDescriptor* Stereoscopic_program_info_descriptor;
			CStereoscopicVideoInfoDescriptor* Stereoscopic_video_info_descriptor;
			CAC3AudioDescriptor* AC3_audio_descriptor;		// 0x81
			CCaptionServiceDescriptor* Caption_service_descriptor;	// 0x86
			CHDMVCopyControlDescriptor* HDMV_copy_control_descriptor;	// 0x88
			CPartialTransportStreamDescriptor* Partial_transport_stream_descriptor;	// 0x63
			CBDSystemUseDescriptor* BD_system_use_descriptor;	// 0x89
			CGeneralDescriptor* pDescriptor;
		};

		CPSIContext*	ptr_ctx;

		C13818_1_Descriptor(CPSIContext* pCtx=NULL): descriptor_tag(1), sub_tag(0), pDescriptor(NULL), ptr_ctx(pCtx){}
		virtual ~C13818_1_Descriptor(){Unmap();}

		BOOL IsEqual(C13818_1_Descriptor* descriptor){
			if (descriptor == NULL || descriptor->descriptor_tag != descriptor_tag)
				return FALSE;

			return TRUE;
		}

		int Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size=0, unsigned long *stuffing_size=0);
		int Unmap(/* Out */ unsigned char* pBuf=NULL, /* In/Out */unsigned long* pcbSize=NULL);
		int WriteToBs(AMBst bs);
		size_t ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint=false, long long* bit_offset = NULL);

	}PACKED;

}	// namespace BST

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
