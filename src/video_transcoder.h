#ifndef __VIDEO_TRANSCODER_H__
#define __VIDEO_TRANSCODER_H__

#pragma once

#include <stdint.h>
#include <inttypes.h>

#define VTC_BUILD 1

#ifdef _WIN32
	#define VTC_DLL_IMPORT __declspec(dllimport)
	#define VTC_DLL_EXPORT __declspec(dllexport)
#else
	#if defined(__GNUC__) && (__GNUC__ >= 4)
		#define VTC_DLL_IMPORT
		#define VTC_DLL_EXPORT __attribute__((visibility("default")))
	#else
		#define VTC_DLL_IMPORT
		#define VTC_DLL_EXPORT
	#endif
#endif

/* 
	If the upper caller wants to link against a shared library version of
	libvtc from a Microsoft Visual Studio or similar development environment
	will need to define VTC_API_IMPORTS before including this header.
 */
#ifdef VTC_API_IMPORTS
	#define VTC_API VTC_DLL_IMPORT
#elif defined(VTC_API_EXPORTS)
	#define VTC_API VTC_DLL_EXPORT
#else
	#define VTC_API
#endif

//
// Return Code for the video transcoder API
//
#define VTC_RET_FALSE						1
#define VTC_RET_OK							0
#define VTC_RET_ERROR					   -1
#define VTC_RET_ERROR_NOTIMPL			   -2
#define VTC_RET_INVALID_PARAMETER		   -7
#define VTC_RET_OUTOFMEMORY				   -8

#define VTC_RET_NOT_INITIALIZED			 -500
#define VTC_RET_TIME_OUT				 -512
#define VTC_RET_ERROR_STATE_TRANSITION	 -541
#define VTC_RET_NO_MORE_DATA			 -546

#define VTC_RET_NEEDMOREINPUT			-1000

#define VTC_RET_VDEC_NOT_FOUND			-4001
#define VTC_RET_VDEC_FAILURE			-4002
#define VTC_RET_VBUF_FAILURE			-4003
#define VTC_RET_VENC_FAILURE			-4004
#define VTC_RET_PIPEFULL				-4005

#define VTC_SUCCEEDED(ret)				((ret) >= 0)
#define VTC_FAILED(ret)					((ret) <  0)


#define INVALID_ES_PTS					   -1
#define INVALID_ES_DTS					   -1	

//
// Parameter definition
//
#define VTC_INFINITE				0xFFFFFFFF	// specified the wait time is infinite

#define IS_AVC_FOURCC(fourcc)		((fourcc) == 'avc1' || (fourcc) == 'avc2' || (fourcc) == 'avc3' || (fourcc) == 'avc4' ||\
									 (fourcc) == 'H264' || (fourcc) == 'h264')

#define IS_HEVC_FOURCC(fourcc)		((fourcc) == 'hvc1' || (fourcc) == 'hev1' ||\
									 (fourcc) == 'HEVC' || (fourcc) == 'hevc' || (fourcc) == 'H265' || (fourcc) == 'h265')

#define IS_MP2V_FOURCC(fourcc)		((fourcc) == 'mp2v')

#define IS_AV1_FOURCC(fourcc)		((fourcc) == 'av01' || (fourcc) == 'AV01')

#ifdef __cplusplus
extern "C" {
#endif

typedef void* vtc_handle;
typedef int32_t VTC_BOOL;

typedef enum _VTC_3STATE_SWITCH
{
	VTC_SWITCH_AUTO = -1,
	VTC_SWITCH_OFF	=  0,
	VTC_SWITCH_ON	=  1,
} VTC_3STATE_SWITCH;

typedef enum _VTC_V_PROFILE
{
	//
	// For MPEG2 Video Profile
	//
	VTC_MPV_PROFILE_UNKNOWN = -1,
	VTC_MPV_PROFILE_HIGH,
	VTC_MPV_PROFILE_SPATIALLY_SCALABLE,
	VTC_MPV_PROFILE_SNR_SCALABLE,
	VTC_MPV_PROFILE_MAIN,
	VTC_MPV_PROFILE_SIMPLE,
	VTC_MPV_PROFILE_422,
	VTC_MPV_PROFILE_MULTI_VIEW,

	//
	// For AVC/H.264 Video Profile
	//
	VTC_AVC_PROFILE_UNKNOWN = -1,
	VTC_AVC_PROFILE_BASELINE = 66,
	VTC_AVC_PROFILE_CONSTRAINED_BASELINE,
	VTC_AVC_PROFILE_MAIN = 77,
	VTC_AVC_PROFILE_EXTENDED = 88,
	VTC_AVC_PROFILE_HIGH = 100,
	VTC_AVC_PROFILE_PROGRESSIVE_HIGH,
	VTC_AVC_PROFILE_CONSTRAINED_HIGH,
	VTC_AVC_PROFILE_HIGH_10 = 110,
	VTC_AVC_PROFILE_HIGH_10_INTRA,
	VTC_AVC_PROFILE_PROGRESSIVE_HIGH_10,
	VTC_AVC_PROFILE_HIGH_422 = 122,
	VTC_AVC_PROFILE_HIGH_422_INTRA,
	VTC_AVC_PROFILE_HIGH_444_PREDICTIVE = 244,
	VTC_AVC_PROFILE_HIGH_444_INTRA,
	VTC_AVC_PROFILE_CAVLC_444_INTRA_PROFIILE = 44,
	VTC_AVC_PROFILE_MULTIVIEW_HIGH = 118,
	VTC_AVC_PROFILE_STEREO_HIGH = 128,
	VTC_AVC_PROFILE_SCALABLE_BASELINE = 83,
	VTC_AVC_PROFILE_SCALABLE_CONSTRAINED_BASELINE,
	VTC_AVC_PROFILE_SCALABLE_HIGH = 86,
	VTC_AVC_PROFILE_SCALABLE_CONSTRAINED_HIGH,
	VTC_AVC_PROFILE_SCALABLE_HIGH_INTRA = 89,
	VTC_AVC_PROFILE_MULTIVIEW_DEPTH_HIGH = 138,


	//
	// For HEVC/H.265 Video Profile
	//
	VTC_HEVC_PROFILE_UNKNOWN = -1,
	VTC_HEVC_PROFILE_MONOCHROME = 0,
	VTC_HEVC_PROFILE_MONOCHROME_10,
	VTC_HEVC_PROFILE_MONOCHROME_12,
	VTC_HEVC_PROFILE_MONOCHROME_16,
	VTC_HEVC_PROFILE_MAIN,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN,
	VTC_HEVC_PROFILE_MAIN_10,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN_10,
	VTC_HEVC_PROFILE_MAIN_12,
	VTC_HEVC_PROFILE_MAIN_STILL_PICTURE,
	VTC_HEVC_PROFILE_MAIN_10_STILL_PICTURE,
	VTC_HEVC_PROFILE_MAIN_422_10,
	VTC_HEVC_PROFILE_MAIN_422_12,
	VTC_HEVC_PROFILE_MAIN_444,
	VTC_HEVC_PROFILE_HIGH_THROUGHPUT_444,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444,
	VTC_HEVC_PROFILE_MAIN_444_10,
	VTC_HEVC_PROFILE_HIGH_THROUGHPUT_444_10,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_MAIN_444_10,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_10,
	VTC_HEVC_PROFILE_MAIN_444_12,
	VTC_HEVC_PROFILE_HIGH_THROUGHPUT_444_14,
	VTC_HEVC_PROFILE_SCREEN_EXTENDED_HIGH_THROUGHPUT_444_14,
	VTC_HEVC_PROFILE_MAIN_INTRA,
	VTC_HEVC_PROFILE_MAIN_10_INTRA,
	VTC_HEVC_PROFILE_MAIN_12_INTRA,
	VTC_HEVC_PROFILE_MAIN_422_10_INTRA,
	VTC_HEVC_PROFILE_MAIN_422_12_INTRA,
	VTC_HEVC_PROFILE_MAIN_444_INTRA,
	VTC_HEVC_PROFILE_MAIN_444_10_INTRA,
	VTC_HEVC_PROFILE_MAIN_444_12_INTRA,
	VTC_HEVC_PROFILE_MAIN_444_16_INTRA,
	VTC_HEVC_PROFILE_MAIN_444_STILL_PICTURE,
	VTC_HEVC_PROFILE_MAIN_444_16_STILL_PICTURE,
	VTC_HEVC_PROFILE_HIGH_THROUGHPUT_444_16_INTRA,
} VTC_V_PROFILE;

typedef enum _VTC_V_TIER
{
	//
	// For HEVC/H.265 Video Tier
	//
	VTC_HEVC_TIER_UNKNOWN = -1,
	VTC_HEVC_TIER_MAIN,
	VTC_HEVC_TIER_HIGH
} VTC_V_TIER;

typedef enum _VTC_V_LEVEL
{
	//
	// For MPEG2 Video Level
	//
	VTC_MPV_LEVEL_UNKNOWN	= -1,
	VTC_MPV_LEVEL_HIGHP		=  2,
	VTC_MPV_LEVEL_HIGH		=  4,
	VTC_MPV_LEVEL_HIGH_1440 =  6,
	VTC_MPV_LEVEL_MAIN		=  8,
	VTC_MPV_LEVEL_LOW		= 10,

	//
	// For AVC/H.264 Video Level
	//
	VTC_AVC_LEVEL_UNKNOWN	= -1,
	VTC_AVC_LEVEL_1			= 100,
	VTC_AVC_LEVEL_1b,
	VTC_AVC_LEVEL_1_1		= 110,
	VTC_AVC_LEVEL_1_2		= 120,
	VTC_AVC_LEVEL_1_3		= 130,
	VTC_AVC_LEVEL_2			= 200,
	VTC_AVC_LEVEL_2_1		= 210,
	VTC_AVC_LEVEL_2_2		= 220,
	VTC_AVC_LEVEL_3			= 300,
	VTC_AVC_LEVEL_3_1		= 310,
	VTC_AVC_LEVEL_3_2		= 320,
	VTC_AVC_LEVEL_4			= 400,
	VTC_AVC_LEVEL_4_1		= 410,
	VTC_AVC_LEVEL_4_2		= 420,
	VTC_AVC_LEVEL_5			= 500,
	VTC_AVC_LEVEL_5_1		= 510,
	VTC_AVC_LEVEL_5_2		= 520,
	VTC_AVC_LEVEL_6			= 600,
	VTC_AVC_LEVEL_6_1		= 610,
	VTC_AVC_LEVEL_6_2		= 620,

	//
	// For HEVC/H.265 Video Level
	//
	VTC_HEVC_LEVEL_UNKNOWN	= -1,
	VTC_HEVC_LEVEL_1		= 30,			// 1
	VTC_HEVC_LEVEL_2		= 60,			// 2
	VTC_HEVC_LEVEL_2_1		= 63,			// 2.1
	VTC_HEVC_LEVEL_3		= 90,			// 3
	VTC_HEVC_LEVEL_3_1		= 93,			// 3.1
	VTC_HEVC_LEVEL_4		= 120,			// 4
	VTC_HEVC_LEVEL_4_1		= 123,			// 4.1
	VTC_HEVC_LEVEL_5		= 150,			// 5
	VTC_HEVC_LEVEL_5_1		= 153,			// 5.1
	VTC_HEVC_LEVEL_5_2		= 156,			// 5.2
	VTC_HEVC_LEVEL_6		= 180,			// 6
	VTC_HEVC_LEVEL_6_1		= 183,			// 6.1
	VTC_HEVC_LEVEL_6_2		= 186,			// 6.2
} VTC_V_LEVEL;

typedef enum _VTC_STATE
{
	VTC_STATE_INVALID = -1,					// VTC is just created, and does not open
	VTC_STATE_RUNNING,						// VTC is running
	VTC_STATE_HALT,							// Halt the VTC regularly
	VTC_STATE_HALT_BY_ERROR,				// Halt the VTC by unrecoverable error
	VTC_STATE_CLOSE							// Released VTC resource
} VTC_STATE;

typedef enum _VTC_COLOUR_PRIMARIES
{
	VTC_CP_UNKNOWN = -1,
	VTC_CP_BT_709 = 1,						// BT.709
	VTC_CP_UNSPECIFIED = 2,					// Unspecified
	VTC_CP_BT_470_B_G = 5,					// Rec. ITU-R BT.470-6 System B, G (historical)
											// Rec. ITU-R BT.601-7 625
											// Rec. ITU-R BT.1358-0 625 (historical)
											// Rec. ITU-R BT.1700-0 625 PAL and 625 SECAM
	VTC_CP_BT_601 = 6,						// Rec. ITU-R BT.601-7 525
											// Rec. ITU-R BT.1358-1 525 or 625 (historical)
											// Rec. ITU-R BT.1700-0 NTSC
											// SMPTE ST 170 (2004)
	VTC_CP_BT_2020 = 9,						// BT.2020, BT.2100
	VTC_CP_XYZ = 10,						// SMPTE 428 (CIE 1921 XYZ)
} VTC_COLOUR_PRIMARIES;

typedef enum _VTC_TRANSFER_CHARACTERISTICS
{
	VTC_TC_UNKNOWN = -1,
	VTC_TC_BT_709 = 1,						// BT.709
	VTC_TC_UNSPECIFIED = 2,					// Unspecified
	VTC_TC_BT_470_B_G = 5,					// Rec. ITU-R BT.470-6 System B, G (historical)
											// Rec. ITU-R BT.601-7 625
											// Rec. ITU-R BT.1358-0 625 (historical)
											// Rec. ITU-R BT.1700-0 625 PAL and 625 SECAM
											// The previous PAL SD video, for example, PAL DVD-Video
	VTC_TC_BT_601 = 6,						// Rec. ITU-R BT.601-7 525
											// Rec. ITU-R BT.1358-1 525 or 625 (historical)
											// Rec. ITU-R BT.1700-0 NTSC
											// SMPTE ST 170 (2004)
											// The legacy NTSC SD-Video, for example, NTSC DVD-Video
	VTC_TC_SRGB = 13,						// sRGB or sYCC
	VTC_TC_BT_2020_10_BIT = 14,				// BT.2020 10-bit systems
	VTC_TC_BT_2020_12_BIT = 15,				// BT.2020 12-bit systems
	VTC_TC_SMPTE_2084 = 16,					// SMPTE ST 2084, ITU BT.2100 PQ and BDMV-HDR
	VTC_TC_HLG = 18							// ARIB STD-B67 (2015)
											// Rec. ITU-R BT.2100-2 hybrid log-gamma (HLG) system
} VTC_TRANSFER_CHARACTERISTICS;

typedef enum _VTC_MATRIX_COEFFICIENTS
{
	VTC_MC_UNKNOWN = -1,
	VTC_MC_BT_709 = 1,						// BT.709
	VTC_MC_UNSPECIFIED = 2,					// Unspecified
	VTC_MC_BT_470_B_G = 5,					// BT.470 System B, G (historical)
	VTC_MC_BT_601 = 6,						// BT.601
	VTC_MC_BT_2020_NCL = 9,					// BT.2020 non-constant luminance, BT.2100 YCbCr
	VTC_MC_BT_2020_CL = 10,					// BT.2020 constant luminance
} VTC_MATRIX_COEFFICIENTS;

typedef enum _VTC_PIC_STRUCT
{
	VTC_PIC_STRUCT_UNKNOWN = -1,
	VTC_PIC_STRUCT_PROGRESSIVE = 0,			// (progressive) Frame
	VTC_PIC_STRUCT_TOP,						// Top field
	VTC_PIC_STRUCT_BOTTOM,					// Bottom field
	VTC_PIC_STRUCT_TOP_BOTTOM,				// Top field, bottom field, in that order
	VTC_PIC_STRUCT_BOTTOM_TOP,				// Bottom field, top field, in that order
	VTC_PIC_STRUCT_TOP_BOTTOM_TOP,			// Top field, bottom field, top field repeated, in that order
	VTC_PIC_STRUCT_BOTTOM_TOP_BOTTOM,		// Bottom field, top field, bottom field repeated, in that order
	VTC_PIC_STRUCT_FRAME_DOUBLING,			// Frame doubling
	VTC_PIC_STRUCT_FRAME_TRIPLING,			// Frame tripling
	VTC_PIC_STRUCT_TOP_PAIRD_WITH_PREV_BOTTOM,							
											// Top field paired with previous bottom field in output order
	VTC_PIC_STRUCT_BOTTOM_PAIRED_WITH_PREV_TOP,
											// Bottom field paired with previous top field in output order
	VTC_PIC_STRUCT_TOP_PAIRED_WITH_NEXT_BOTTOM,	
											// Top field paired with next bottom field in output order
	VTC_PIC_STRUCT_BOTTOM_PAIRED_WITH_NEXT_TOP,				
											// Bottom field paired with next top field in output order
} VTC_PIC_STRUCT;

/*
	Codec FourCC List
	mp2v		mpeg2 video
	wvc1		VC-1 video
	hvc1		HEVC/H.265 which is provided in ITU-T Recommendation H.265(which includes VPS, SPS and PPS separated with video stream or video sample entry)
				ISOBMFF normally used it
	hev1		HEVC/H.265 which is provided in ITU-T Recommendation H.265(which includes VPS, SPS and PPS in video stream or video sample entry)
				MPEG-TS and MMT normally used it, recommended it
	avc1/avc2	AVC/H.264 which is provided in ITU-T Recommendation H.264(Which includes SPS, PPS separated with video stream or video sample entry)
				ISOBMFF normally used it
	avc3/avc4	AVC/H.264 which is provided in ITU-T Recommendation H.264(Which includes SPS, PPS in video stream or video sample entry)
				MPEG-TS and MMT normally used it, recommended it
	av01		AV1 video
*/
typedef struct vtc_param_t
{
	int32_t				src_stream_fourcc;	// specify the input video stream type
	int32_t				dst_stream_fourcc;	// specify the output video stream type
	int32_t				width;				// specify the input video width
											// If a value is not greater than 0 is assigned, will use the width of decoded picture
											// Otherwise, the specified width will be used, 
											// if the decoded video frame is not equal to it, the video scale will be done in transcoder side
	int32_t				height;				// specify the output video width
											// If a value is not greater than 0 is assigned, will use the height of decoded picture
											// Otherwise, the specified height will be used, 
											// if the decoded video frame is not equal to it, the video scale will be done in transcoder side
	VTC_BOOL			keep_DAR;			// when video scale happened, whether to keep the display aspect-ratio or not

	//
	// Frames per second/frame-rate
	//
	// fps_num/fps_den are all zeros, it means the fps is not specified
	// When the specified fps is different with the original fps, frame-rate conversion will 
	// be triggered in the transcode side
	uint32_t			fps_num;			// fps numerator
	uint32_t			fps_den;			// fps denominator

	//
	// Sample-Aspect-Ratio
	//
	// sar_num/sar_den are all zeros, it means the SAR is not specified;
	uint32_t			sar_num;			// Sample-Aspect-Ratio numerator
	uint32_t			sar_den;			// Sample-Aspect-Ratio denominator

	//
	// signal transfer information
	//
	// If the decoder can parse the below values, the underlying decoded value will be applied
	// If the decoder can not parse the below values, these values will be used
	VTC_3STATE_SWITCH	src_overscan;
	int32_t				src_video_format;
	VTC_3STATE_SWITCH	src_video_full_range;
	VTC_COLOUR_PRIMARIES
						src_colour_primaries;
	VTC_TRANSFER_CHARACTERISTICS
						src_transfer_characteristics;
	VTC_MATRIX_COEFFICIENTS
						src_matrix_coefficients;

	VTC_3STATE_SWITCH	src_pulldown;		// for example, 3:2 pulldown, 1 field may be repeat
											// Auto(-1): decoder will detect the source content pull-down mode
											// OFF ( 0): no pull-down
											// ON  ( 1): pull-down

	// if the bitrate is zero, it means the target bitrate is not specified
	uint32_t			target_bitrate;		// in unit of bps, 0 means unknown
	uint32_t			target_max_bitrate;	// In unit of bps, 0 means unknown
	uint32_t			target_cpb_buffer_size;
											// In unit of bits, 0 means unknown

	// target picture struct
	VTC_3STATE_SWITCH	target_interlace;	// Auto(-1): decoder will detect the target content;
											// OFF ( 0): progressive; 
											// If client side turns off interlace, and the source content is interlace
											// Video transcoder should do de-interlace, and convert interlace content to progressive content
											// ON  ( 1): interlaced

	union {
		VTC_V_PROFILE	output_profile;
		int				profile_value;
	};

	union {
		VTC_V_TIER		output_tier;
		int				tier_value;
	};

	union {
		VTC_V_LEVEL		output_level;
		int				level_value;
	};
} vtc_param_t;

//
// in one picture elementary stream, more than Access-Unit may be included
//
typedef struct vtc_picture_es_t
{
	int64_t				ctx;				// internal state which is used by vtc_out_picture_es_init/vtc_out_picture_es_cleanup,
											// don't modify it explicitly; If client don't use the internal method, this field has no meaning, 
	
	int64_t				pts;				// [in/out] the picture pts
	int64_t				dts;				// [in/out] the picture dts

	// For the transcoder output:
	// 1. If es_buf is not null, and es_buf_size is not 0, 
	//      it means that caller has already allocated the buffer
	//      to hold the transcoded ES, if the es_buf_size is not enough, 
	//      the error `VTC_RET_BUFFER_TOO_SMALL` will be returned, and 
	//      the required size will be set in es_buf_size
	// 2. If es_buf is null && es_buf_size is 0,
	//      It means that transcode engine will allocate the memory for 
	//      the ES output, and the allocated size is also assigned to 
	//      es_buf_size, and the caller need release it by self
	uint8_t*			es_buf;				// [in/out] the ES buffer
	int32_t				es_buf_size;		// [in/out] the ES buffer size

	int32_t				pic_type;			// [out] for example, I, P and B frame
	VTC_PIC_STRUCT		pic_struct;			// [in/out] the picture structure, progressive, top/bottom field and so on
	int32_t				key_frame;			// [out] key frame

	// When inputting the ES, can place this flag together with or without ES buffer 
	// to notify it is the last video picture. After that, can call `video_transcoder_input` 
	// to drain all transcoded picture ES data.
	uint8_t				EOS;				// [in/out] End of Stream
} vtc_picture_es_t;

/*!	@brief initialize the parameters */
VTC_API void vtc_params_init(vtc_param_t* params);

/*!	@brief parse the name/value property pair, and update the parameters */
VTC_API int vtc_params_parse(vtc_param_t* params, const char* name, const char* value);

/*!	@brief clone the video transcoder parameters from the current */
VTC_API int vtc_params_clone(const vtc_param_t* params, vtc_param_t* clone_params);

/*!	@brief According to the fields of current video transcoder parameter, select profile, tier and level */
VTC_API int vtc_params_autoselect_profile_tier_level(vtc_param_t* params);

/*!	@brief clean the resource hold in the params */
VTC_API void vtc_params_cleanup(vtc_param_t* params);

/*!	@brief initialize picture es structure */
VTC_API void vtc_picture_es_init(vtc_picture_es_t* picture_es);

/*!	@brief free the resource allocated in the video transcoder*/
VTC_API void vtc_picture_es_cleanup(vtc_picture_es_t* picture_es);

/*!	@brief Create a video transcoder */
VTC_API vtc_handle video_transcoder_open(vtc_param_t* vtc_params);

/*!	@brief Feed the picture elementary stream payload */
VTC_API int video_transcoder_input(vtc_handle vtc_handle, vtc_picture_es_t pic_es);

/*!	@brief Get the re-encoded video picture elementary stream payload */
VTC_API int video_transcoder_output(vtc_handle vtc_handle, vtc_picture_es_t* pic_es, uint32_t wait_ms);

/*!	@brief Get the video transcoder state */
VTC_API VTC_STATE video_transcoder_get_state(vtc_handle vtc_handle);

/*!	@brief Get the error code which caused video transcoder to enter into halt-by-error 'state'*/
VTC_API int video_transcoder_get_halt_error(vtc_handle vtc_handle, int* code_of_halt_by_error);

/*!	@brief close the specified video transcoder, and release the related resource. */
VTC_API	void video_transcoder_close(vtc_handle vtc_handle);

//
// For loading and export I/F dynamically
//
typedef void					(*PFUNC_VTC_PARAMS_INIT)(vtc_param_t*);
typedef int						(*PFUNC_VTC_PARAMS_PARSE)(vtc_param_t*, const char*, const char*);
typedef int						(*PFUNC_VTC_PARAMS_CLONE)(const vtc_param_t*, vtc_param_t*);
typedef int						(*PFUNC_VTC_PARAMS_AUTOSELECT_PROFILE_TIER_LEVEL)(vtc_param_t*);
typedef void					(*PFUNC_VTC_PARAMS_CLEANUP)(vtc_param_t*);
typedef void					(*PFUNC_VTC_PICTURE_ES_INIT)(vtc_picture_es_t*);
typedef void					(*PFUNC_VTC_PICTURE_ES_CLEANUP)(vtc_picture_es_t*);
typedef vtc_handle				(*PFUNC_VTC_OPEN)(vtc_param_t*);
typedef int						(*PFUNC_VTC_INPUT)(vtc_handle, vtc_picture_es_t);
typedef int						(*PFUNC_VTC_OUTPUT)(vtc_handle, vtc_picture_es_t*, uint32_t);
typedef VTC_STATE				(*PFUNC_VTC_GET_STATE)(vtc_handle);
typedef int						(*PFUNC_VTC_GET_HALT_ERROR)(vtc_handle, int*);
typedef void					(*PFUNC_VTC_CLOSE)(vtc_handle);

typedef	struct VTC_EXPORT
{
	void*						hModule;
	PFUNC_VTC_PARAMS_INIT		fn_params_init;
	PFUNC_VTC_PARAMS_PARSE		fn_params_parse;
	PFUNC_VTC_PARAMS_CLONE		fn_params_clone;
	PFUNC_VTC_PARAMS_AUTOSELECT_PROFILE_TIER_LEVEL
								fn_param_autoselect_profile_tier_level;
	PFUNC_VTC_PARAMS_CLEANUP	fn_params_cleanup;
	PFUNC_VTC_PICTURE_ES_INIT	fn_picture_es_init;
	PFUNC_VTC_PICTURE_ES_CLEANUP
								fn_picture_es_cleanup;
	PFUNC_VTC_OPEN				fn_open;
	PFUNC_VTC_INPUT				fn_input;
	PFUNC_VTC_OUTPUT			fn_output;
	PFUNC_VTC_GET_STATE			fn_get_state;
	PFUNC_VTC_GET_HALT_ERROR	fn_get_halt_error;
	PFUNC_VTC_CLOSE				fn_close;
}VTC_EXPORT;

#ifdef __cplusplus
}
#endif

#endif
