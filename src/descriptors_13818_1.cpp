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
#include "platcomm.h"
#include "descriptors_13818_1.h"
#include "system_13818_1.h"

const char* descriptor_tag_names[256] = {
"reserved",
"forbidden",
"video_stream_descriptor",
"audio_stream_descriptor",
"hierarchy_descriptor",
"registration_descriptor",
"data_stream_alignment_descriptor",
"target_background_grid_descriptor",
"video_window_descriptor",
"CA_descriptor",
"ISO_639_language_descriptor",
"system_clock_descriptor",
"multiplex_buffer_utilization_",
"copyright_descriptor",
"maximum_bitrate_descriptor",
"private_data_indicator_descriptor",
"smoothing_buffer_descriptor",
"STD_descriptor",
"IBP_descriptor",
"",
"",
"",
"",
"",
"",
"",
"",
"MPEG-4_video_descriptor",
"MPEG-4_audio_descriptor",
"IOD_descriptor",
"SL_descriptor",
"FMC_descriptor",
"external_ES_ID_descriptor",
"MuxCode_descriptor",
"FmxBufferSize_descriptor",
"multiplexbuffer_descriptor",
"content_labeling_descriptor",
"metadata_pointer_descriptor",
"metadata_descriptor",
"metadata_STD_descriptor",
"AVC video descriptor",
"IPMP_descriptor (defined in ISO/IEC 13818-11, MPEG-2 IPMP)",
"AVC timing and HRD descriptor",
"MPEG-2_AAC_audio_descriptor",
"FlexMuxTiming_descriptor",
"MPEG-4_text_descriptor",
"MPEG-4_audio_extension_descriptor",
"auxiliary_video_stream_descriptor",
"SVC extension descriptor",
"MVC extension descriptor",
"J2K video descriptor",
"MVC operation point descriptor",
"MPEG2_stereoscopic_video_format_descriptor",
"Stereoscopic_program_info_descriptor",
"Stereoscopic_video_info_descriptor",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
};

const char* frame_rate_value_names[16] = {
"forbidden",
"24 000/1001 (23,976)",
"24",
"25",
"30 000/1001 (29,97)",
"30",
"50",
"60 000/1001 (59,94)",
"60",
""
""
""
""
""
"reserved",
"reserved",
};

const char* frame_rate_multiple_value_names[16] = {
"forbidden",
"24 000/1001 (23,976)",
"24, 23.976",
"25",
"30 000/1001 (29,97), 23.976",
"30, 23.976, 24.0, 29.97",
"50, 25.0",
"60 000/1001 (59,94), 23.976, 29.97",
"60, 23.976, 24.0, 29.97, 30.0, 59.94",
""
""
""
""
""
"reserved",
"reserved",
};

const char* chroma_format_names[4] = {"reserved", "4:2:0", "4:2:2", "4:4:4"};

const char* hierarchy_type_names[16] = {
	"Reserved",
	"Spatial Scalability",
	"SNR Scalability",
	"Temporal Scalability",
	"Data partitioning",
	"Extension bitstream",
	"Private Stream",
	"Multi-view Profile",
	"Combined Scalability",
	"MVC video sub-bitstream",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Base layer or MVC base view sub-bitstream or AVC video sub-bitstream of MVC"
};

const char* MPEG4_visual_profile_and_level_names[256] = {
	"Reserved", //00000000
	"Simple Profile/Level 1", //00000001
	"Simple Profile/Level 2", //00000010
	"Simple Profile/Level 3", //00000011
	"Reserved", //00000100 − 00010000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Simple Scalable Profile/Level 1", //00010001
	"Simple Scalable Profile/Level 2", //00010010
	"Reserved", //00010011 − 00100000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Core Profile/Level 1", //00100001
	"Core Profile/Level 2", //00100010
	"Reserved", //00100011 − 00110001
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Main Profile/Level 2", //00110010
	"Main Profile/Level 3", //00110011
	"Main Profile/Level 4", //00110100
	"Reserved", //00110101 − 01000001
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"N-bit Profile/Level 2", //01000010
	"Reserved", //01000011 − 01010000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Scalable Texture Profile/Level 1", //01010001
	"Reserved", //01010010 − 01100000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Simple Face Animation Profile/Level 1", //01100001
	"Simple Face Animation Profile/Level 2", //01100010
	"Simple FBA Profile/Level 1", //01100011
	"Simple FBA Profile/Level 2", //01100100
	"Reserved", //01100101 − 01110000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Basic Animated Texture Profile/Level 1", //01110001
	"Basic Animated Texture Profile/Level 2", //01110010
	"Reserved", //01110011 − 10000000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Hybrid Profile/Level 1", //10000001
	"Hybrid Profile/Level 2", //10000010
	"Reserved", //10000011 − 10010000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Advanced Real Time Simple Profile/Level 1", //10010001
	"Advanced Real Time Simple Profile/Level 2", //10010010
	"Advanced Real Time Simple Profile/Level 3", //10010011
	"Advanced Real Time Simple Profile/Level 4", //10010100
	"Reserved", //10010101 − 10100000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Core Scalable Profile/Level1", //10100001
	"Core Scalable Profile/Level2", //10100010
	"Core Scalable Profile/Level3", //10100011
	"Reserved", //10100100 − 10110000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Advanced Coding Efficiency Profile/Level 1", //10110001
	"Advanced Coding Efficiency Profile/Level 2", //10110010
	"Advanced Coding Efficiency Profile/Level 3", //10110011
	"Advanced Coding Efficiency Profile/Level 4", //10110100
	"Reserved", //10110101 – 11000000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Advanced Core Profile/Level 1", //11000001
	"Advanced Core Profile/Level 2", //11000010
	"Reserved", //11000011 – 11010000
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Advanced Scalable Texture/Level1", //11010001
	"Advanced Scalable Texture/Level2", //11010010
	"Advanced Scalable Texture/Level3", //11010011
	"Reserved", //11010100 − 11111111
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

const char* audioProfileLevelIndication_names[256] = {
/*0x00*/		"Reserved for ISO use",
/*0x01*/		"Main Audio Profile@L1",
/*0x02*/		"Main Audio Profile@L2",
/*0x03*/		"Main Audio Profile@L3",
/*0x04*/		"Main Audio Profile@L4",
/*0x05*/		"Scalable Audio Profile@L1",
/*0x06*/		"Scalable Audio Profile@L2",
/*0x07*/		"Scalable Audio Profile@L3",
/*0x08*/		"Scalable Audio Profile@L4",
/*0x09*/		"Speech Audio Profile@L1",
/*0x0A*/		"Speech Audio Profile@L2",
/*0x0B*/		"Synthetic Audio Profile@L1",
/*0x0C*/		"Synthetic Audio Profile@L2",
/*0x0D*/		"Synthetic Audio Profile@L3",
/*0x0E*/		"High Quality Audio Profile@L1",
/*0x0F*/		"High Quality Audio Profile@L2",
/*0x10*/		"High Quality Audio Profile@L3",
/*0x11*/		"High Quality Audio Profile@L4",
/*0x12*/		"High Quality Audio Profile@L5",
/*0x13*/		"High Quality Audio Profile@L6",
/*0x14*/		"High Quality Audio Profile@L7",
/*0x15*/		"High Quality Audio Profile@L8",
/*0x16*/		"Low Delay Audio Profile@L1",
/*0x17*/		"Low Delay Audio Profile@L2",
/*0x18*/		"Low Delay Audio Profile@L3",
/*0x19*/		"Low Delay Audio Profile@L4",
/*0x1A*/		"Low Delay Audio Profile@L5",
/*0x1B*/		"Low Delay Audio Profile@L6",
/*0x1C*/		"Low Delay Audio Profile@L7",
/*0x1D*/		"Low Delay Audio Profile@L8",
/*0x1E*/		"Natural Audio Profile@L1",
/*0x1F*/		"Natural Audio Profile@L2",
/*0x20*/		"Natural Audio Profile@L3",
/*0x21*/		"Natural Audio Profile@L4",
/*0x22*/		"Mobile Audio Internetworking Profile@L1",
/*0x23*/		"Mobile Audio Internetworking Profile@L2",
/*0x24*/		"Mobile Audio Internetworking Profile@L3",
/*0x25*/		"Mobile Audio Internetworking Profile@L4",
/*0x26*/		"Mobile Audio Internetworking Profile@L5",
/*0x27*/		"Mobile Audio Internetworking Profile@L6",
/*0x28*/		"AAC Profile@L1",
/*0x29*/		"AAC Profile@L2",
/*0x2A*/		"AAC Profile@L4",
/*0x2B*/		"AAC Profile@L5",
/*0x2C*/		"High Efficiency AAC Profile@L2",
/*0x2D*/		"High Efficiency AAC Profile@L3",
/*0x2E*/		"High Efficiency AAC Profile@L4",
/*0x2F*/		"High Efficiency AAC Profile@L5",
/*0x30*/		"reserved for ISO use",
/*0x31*/		"reserved for ISO use",
/*0x32*/		"reserved for ISO use",
/*0x33*/		"reserved for ISO use",
/*0x34*/		"reserved for ISO use",
/*0x35*/		"reserved for ISO use",
/*0x36*/		"reserved for ISO use",
/*0x37*/		"reserved for ISO use",
/*0x38*/		"reserved for ISO use",
/*0x39*/		"reserved for ISO use",
/*0x3a*/		"reserved for ISO use",
/*0x3b*/		"reserved for ISO use",
/*0x3c*/		"reserved for ISO use",
/*0x3d*/		"reserved for ISO use",
/*0x3e*/		"reserved for ISO use",
/*0x3f*/		"reserved for ISO use",
/*0x40*/		"reserved for ISO use",
/*0x41*/		"reserved for ISO use",
/*0x42*/		"reserved for ISO use",
/*0x43*/		"reserved for ISO use",
/*0x44*/		"reserved for ISO use",
/*0x45*/		"reserved for ISO use",
/*0x46*/		"reserved for ISO use",
/*0x47*/		"reserved for ISO use",
/*0x48*/		"reserved for ISO use",
/*0x49*/		"reserved for ISO use",
/*0x4a*/		"reserved for ISO use",
/*0x4b*/		"reserved for ISO use",
/*0x4c*/		"reserved for ISO use",
/*0x4d*/		"reserved for ISO use",
/*0x4e*/		"reserved for ISO use",
/*0x4f*/		"reserved for ISO use",
/*0x50*/		"reserved for ISO use",
/*0x51*/		"reserved for ISO use",
/*0x52*/		"reserved for ISO use",
/*0x53*/		"reserved for ISO use",
/*0x54*/		"reserved for ISO use",
/*0x55*/		"reserved for ISO use",
/*0x56*/		"reserved for ISO use",
/*0x57*/		"reserved for ISO use",
/*0x58*/		"reserved for ISO use",
/*0x59*/		"reserved for ISO use",
/*0x5a*/		"reserved for ISO use",
/*0x5b*/		"reserved for ISO use",
/*0x5c*/		"reserved for ISO use",
/*0x5d*/		"reserved for ISO use",
/*0x5e*/		"reserved for ISO use",
/*0x5f*/		"reserved for ISO use",
/*0x60*/		"reserved for ISO use",
/*0x61*/		"reserved for ISO use",
/*0x62*/		"reserved for ISO use",
/*0x63*/		"reserved for ISO use",
/*0x64*/		"reserved for ISO use",
/*0x65*/		"reserved for ISO use",
/*0x66*/		"reserved for ISO use",
/*0x67*/		"reserved for ISO use",
/*0x68*/		"reserved for ISO use",
/*0x69*/		"reserved for ISO use",
/*0x6a*/		"reserved for ISO use",
/*0x6b*/		"reserved for ISO use",
/*0x6c*/		"reserved for ISO use",
/*0x6d*/		"reserved for ISO use",
/*0x6e*/		"reserved for ISO use",
/*0x6f*/		"reserved for ISO use",
/*0x70*/		"reserved for ISO use",
/*0x71*/		"reserved for ISO use",
/*0x72*/		"reserved for ISO use",
/*0x73*/		"reserved for ISO use",
/*0x74*/		"reserved for ISO use",
/*0x75*/		"reserved for ISO use",
/*0x76*/		"reserved for ISO use",
/*0x77*/		"reserved for ISO use",
/*0x78*/		"reserved for ISO use",
/*0x79*/		"reserved for ISO use",
/*0x7a*/		"reserved for ISO use",
/*0x7b*/		"reserved for ISO use",
/*0x7c*/		"reserved for ISO use",
/*0x7d*/		"reserved for ISO use",
/*0x7e*/		"reserved for ISO use",
/*0x7f*/		"reserved for ISO use",
/*0x80*/		"user private",
/*0x81*/		"user private",
/*0x82*/		"user private",
/*0x83*/		"user private",
/*0x84*/		"user private",
/*0x85*/		"user private",
/*0x86*/		"user private",
/*0x87*/		"user private",
/*0x88*/		"user private",
/*0x89*/		"user private",
/*0x8a*/		"user private",
/*0x8b*/		"user private",
/*0x8c*/		"user private",
/*0x8d*/		"user private",
/*0x8e*/		"user private",
/*0x8f*/		"user private",
/*0x90*/		"user private",
/*0x91*/		"user private",
/*0x92*/		"user private",
/*0x93*/		"user private",
/*0x94*/		"user private",
/*0x95*/		"user private",
/*0x96*/		"user private",
/*0x97*/		"user private",
/*0x98*/		"user private",
/*0x99*/		"user private",
/*0x9a*/		"user private",
/*0x9b*/		"user private",
/*0x9c*/		"user private",
/*0x9d*/		"user private",
/*0x9e*/		"user private",
/*0x9f*/		"user private",
/*0xa0*/		"user private",
/*0xa1*/		"user private",
/*0xa2*/		"user private",
/*0xa3*/		"user private",
/*0xa4*/		"user private",
/*0xa5*/		"user private",
/*0xa6*/		"user private",
/*0xa7*/		"user private",
/*0xa8*/		"user private",
/*0xa9*/		"user private",
/*0xaa*/		"user private",
/*0xab*/		"user private",
/*0xac*/		"user private",
/*0xad*/		"user private",
/*0xae*/		"user private",
/*0xaf*/		"user private",
/*0xb0*/		"user private",
/*0xb1*/		"user private",
/*0xb2*/		"user private",
/*0xb3*/		"user private",
/*0xb4*/		"user private",
/*0xb5*/		"user private",
/*0xb6*/		"user private",
/*0xb7*/		"user private",
/*0xb8*/		"user private",
/*0xb9*/		"user private",
/*0xba*/		"user private",
/*0xbb*/		"user private",
/*0xbc*/		"user private",
/*0xbd*/		"user private",
/*0xbe*/		"user private",
/*0xbf*/		"user private",
/*0xc0*/		"user private",
/*0xc1*/		"user private",
/*0xc2*/		"user private",
/*0xc3*/		"user private",
/*0xc4*/		"user private",
/*0xc5*/		"user private",
/*0xc6*/		"user private",
/*0xc7*/		"user private",
/*0xc8*/		"user private",
/*0xc9*/		"user private",
/*0xca*/		"user private",
/*0xcb*/		"user private",
/*0xcc*/		"user private",
/*0xcd*/		"user private",
/*0xce*/		"user private",
/*0xcf*/		"user private",
/*0xd0*/		"user private",
/*0xd1*/		"user private",
/*0xd2*/		"user private",
/*0xd3*/		"user private",
/*0xd4*/		"user private",
/*0xd5*/		"user private",
/*0xd6*/		"user private",
/*0xd7*/		"user private",
/*0xd8*/		"user private",
/*0xd9*/		"user private",
/*0xda*/		"user private",
/*0xdb*/		"user private",
/*0xdc*/		"user private",
/*0xdd*/		"user private",
/*0xde*/		"user private",
/*0xdf*/		"user private",
/*0xe0*/		"user private",
/*0xe1*/		"user private",
/*0xe2*/		"user private",
/*0xe3*/		"user private",
/*0xe4*/		"user private",
/*0xe5*/		"user private",
/*0xe6*/		"user private",
/*0xe7*/		"user private",
/*0xe8*/		"user private",
/*0xe9*/		"user private",
/*0xea*/		"user private",
/*0xeb*/		"user private",
/*0xec*/		"user private",
/*0xed*/		"user private",
/*0xee*/		"user private",
/*0xef*/		"user private",
/*0xf0*/		"user private",
/*0xf1*/		"user private",
/*0xf2*/		"user private",
/*0xf3*/		"user private",
/*0xf4*/		"user private",
/*0xf5*/		"user private",
/*0xf6*/		"user private",
/*0xf7*/		"user private",
/*0xf8*/		"user private",
/*0xf9*/		"user private",
/*0xfa*/		"user private",
/*0xfb*/		"user private",
/*0xfc*/		"user private",
/*0xfd*/		"user private",
/*0xFE*/		"no audio profile specified",
/*0xFF*/		"no audio capability required"
};

namespace BST {

	int C13818_1_Descriptor::Map(unsigned char *pBuf, unsigned long cbSize, unsigned long *desired_size, unsigned long *stuffing_size)
	{
		unsigned long ulMappedSize = 0;

		if (pBuf == NULL)
			return RET_CODE_BUFFER_NOT_FOUND;

		if (cbSize < 2)
			return RET_CODE_BUFFER_TOO_SMALL;

		if((descriptor_tag = pBuf[0]) == 1){
			return RET_CODE_BUFFER_NOT_COMPATIBLE;
		}

		switch(descriptor_tag)
		{
		case DT_video_stream_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, video_stream_descriptor, CVideoStreamDescriptor); break;
		case DT_audio_stream_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, audio_stream_descriptor, CAudioStreamDescriptor); break;
		case DT_hierarchy_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, hierarchy_descriptor, CHierarchyDescriptor); break;
		case DT_registration_descriptor: 
			if (ptr_ctx != NULL && toupper(pBuf[2]) == 'H' && toupper(pBuf[3]) == 'D' &&toupper(pBuf[4]) == 'M' &&toupper(pBuf[5]) == 'V' &&
				ptr_ctx->ptr_section != NULL && ptr_ctx->ptr_section->table_id == TID_TS_program_map_section)
			{
				if (ptr_ctx->loop_idx == 0){
					MAP_MEM_TO_STRUCT_POINTER(1, HDMV_registration_descriptor, CHDMVRegistrationDescriptor);
					sub_tag = 1;
					break;
				}else if(ptr_ctx->loop_idx == 1){
					if (ptr_ctx->stream_type == 0x02 || ptr_ctx->stream_type == 0x1B || ptr_ctx->stream_type == 0x20 || ptr_ctx->stream_type == 0x24){
						MAP_MEM_TO_STRUCT_POINTER(1, HDMV_video_registration_descriptor, CHDMVVideoRegistrationDescriptor);
						sub_tag = 2;
						break;
					}else if(ptr_ctx->stream_type == 0xEA){
						MAP_MEM_TO_STRUCT_POINTER2(1, VC1_registration_descriptor, CVC1RegistrationDescriptor); 
						sub_tag = 3;
						break;
					}else if(ptr_ctx->stream_type == 0x80){
						MAP_MEM_TO_STRUCT_POINTER(1, HDMV_LPCM_audio_registration_descriptor, CHDMVLPCMAudioRegistrationDescriptor);
						sub_tag = 4;
						break;
					}else if(ptr_ctx->stream_type == 0x81){
						MAP_MEM_TO_STRUCT_POINTER(1, AC3_registration_descriptor, CAC3RegistrationDescriptor); 
						sub_tag = 5;
						break;
					}else if(ptr_ctx->stream_type == 0x87 || ptr_ctx->stream_type == 0x88){
						MAP_MEM_TO_STRUCT_POINTER(1, DRA_registration_descriptor, CDRARegistrationDescriptor);
						sub_tag = 6;
						break;
					}
				}
			}

			MAP_MEM_TO_STRUCT_POINTER(1, registration_descriptor, CRegistrationDescriptor);
			sub_tag = 0;
			break;
		case DT_data_stream_alignment_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, data_stream_alignment_descriptor, CDataStreamAlignmentDescriptor); break;
		case DT_target_background_grid_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, target_background_grid_descriptor, CTargetBackgroundGridDescriptor); break;
		case DT_video_window_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, video_window_descriptor, CVideoWindowDescriptor); break;
		case DT_CA_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, CA_descriptor, CCADescriptor); break;								
		case DT_ISO_639_language_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, ISO_639_language_descriptor, CISO639LanguageDescriptor); break;
		case DT_system_clock_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, system_clock_descriptor, CSystemClockDescriptor); break;
		case DT_multiplex_buffer_utilization_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, multiplex_buffer_utilization_descriptor, CMultiplexBufferUtilizationDescriptor); break;
		case DT_copyright_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, copyright_descriptor, CCopyrightDescriptor); break;
		case DT_maximum_bitrate_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, maximum_bitrate_descriptor, CMaximumBitrateDescriptor); break;
		case DT_private_data_indicator_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, private_data_indicator_descriptor, CPrivateDataIndicatorDescriptor); break;
		case DT_smoothing_buffer_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, smoothing_buffer_descriptor, CSmoothingBufferDescriptor); break;
		case DT_STD_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, STD_descriptor, CSTDDescriptor); break;
		case DT_IBP_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, IBP_descriptor, CIBPDescriptor); break;
		case DT_MPEG4_video_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, MPEG4_video_descriptor, CMPEG4VideoDescriptor); break;
		case DT_MPEG4_audio_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, MPEG4_audio_descriptor, CMPEG4AudioDescriptor); break;
		case DT_IOD_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, IOD_descriptor, CIODDescriptor); break;
		case DT_SL_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, SL_descriptor, CSLDescriptor); break;
		case DT_FMC_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, FMC_descriptor, CFMCDescriptor); break;
		case DT_external_ES_ID_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, external_ES_ID_descriptor, CExternalESIDDescriptor); break;
		case DT_MuxCode_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, MuxCode_descriptor, CMuxcodeDescriptor); break;	// Advance Endian Map
		case DT_FmxBufferSize_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, FmxBufferSize_descriptor, CFmxBufferSizeDescriptor); break;
		case DT_multiplexbuffer_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, multiplexbuffer_descriptor, CMultiplexBufferDescriptor); break;
		case DT_content_labeling_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, content_labeling_descriptor, CContentLabelingDescriptor); break;	// Advance Endian Map
		case DT_metadata_pointer_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, metadata_pointer_descriptor, CMetadataPointerDescriptor); break;
		case DT_metadata_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, metadata_descriptor, CMetadataDescriptor); break;	// Advance Endian Map
		case DT_metadata_STD_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, metadata_STD_descriptor, CMetadataSTDDescriptor); break;
		case DT_AVC_video_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, AVC_video_descriptor, CAVCVideoDescriptor); break;
		//case DT_IPMP_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, CA_descriptor, CCADescriptor); break;
		case DT_AVC_timing_and_HRD_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, AVC_timing_and_HRD_descriptor, CAVCTimingAndHRDDescriptor); break;
		case DT_MPEG2_AAC_audio_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, MPEG2_AAC_audio_descriptor, CMPEG2AACAudioDescriptor); break;
		case DT_FlexMuxTiming_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, FlexMuxTiming_descriptor, CFlexMuxTimingDescriptor); break;
		case DT_MPEG4_text_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, MPEG4_text_descriptor, CMPEG4TextDescriptor); break;	// Advance Endian Map
		case DT_MPEG4_audio_extension_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, MPEG4_audio_extension_descriptor, CMPEG4AudioExtensionDescriptor); break;	// Advance Endian Map
		case DT_auxiliary_video_stream_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, auxiliary_video_stream_descriptor, CAuxiliaryVideoStreamDescriptor); break;
		case DT_SVC_extension_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, SVC_extension_descriptor, CSVCExtensionDescriptor); break;
		case DT_MVC_extension_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, MVC_extension_descriptor, CMVCExtensionDescriptor); break;
		case DT_J2K_video_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, J2K_video_descriptor, CJ2KVideoDescriptor); break;
		case DT_MVC_operation_point_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, MVC_operation_point_descriptor, CMVCOperationPointDescriptor); break;	// Advance Endian Map
		case DT_MPEG2_stereoscopic_video_format_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, MPEG2_stereoscopic_video_format_descriptor, CMPEG2StereoscopicVideoFormatDescriptor); break;
		case DT_Stereoscopic_program_info_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, Stereoscopic_program_info_descriptor, CStereoscopicProgramInfoDescriptor); break;
		case DT_Stereoscopic_video_info_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, Stereoscopic_video_info_descriptor, CStereoscopicVideoInfoDescriptor); break;	// Advance Endian Map
		case DT_AC3_audio_descriptor: MAP_MEM_TO_STRUCT_POINTER2(1, AC3_audio_descriptor, CAC3AudioDescriptor); break;	// Advance Endian Map
		case DT_Caption_service_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, Caption_service_descriptor, CCaptionServiceDescriptor); break;
		case DT_HDMV_copy_control_descritpor: MAP_MEM_TO_STRUCT_POINTER(1, HDMV_copy_control_descriptor, CHDMVCopyControlDescriptor); break;
		case DT_Partial_transport_stream_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, Partial_transport_stream_descriptor, CPartialTransportStreamDescriptor); break;
		case DT_BD_system_use_descriptor: MAP_MEM_TO_STRUCT_POINTER(1, BD_system_use_descriptor, CBDSystemUseDescriptor); break;
		default:
			MAP_MEM_TO_STRUCT_POINTER(1, pDescriptor, CGeneralDescriptor)
		}

		AMP_SAFEASSIGN(desired_size, ulMappedSize);

		return RET_CODE_SUCCESS;
	}

	int C13818_1_Descriptor::Unmap(/* Out */ unsigned char* pBuf, /* In/Out */unsigned long* pcbSize)
	{
		if (pcbSize == NULL || pBuf == NULL)
		{
			switch(descriptor_tag)
			{
			case DT_forbidden: return RET_CODE_BUFFER_NOT_COMPATIBLE;	// Nothing to do, normally it mean the descriptor has already unmapped
			case DT_video_stream_descriptor: UNMAP_STRUCT_POINTER0_1(video_stream_descriptor, CVideoStreamDescriptor); break;
			case DT_audio_stream_descriptor: UNMAP_STRUCT_POINTER0_1(audio_stream_descriptor, CAudioStreamDescriptor); break;
			case DT_hierarchy_descriptor: UNMAP_STRUCT_POINTER0_1(hierarchy_descriptor, CHierarchyDescriptor); break;
			case DT_registration_descriptor: 
				switch(sub_tag)
				{
				case 0: UNMAP_STRUCT_POINTER0_1(registration_descriptor, CRegistrationDescriptor); break;
				case 1: UNMAP_STRUCT_POINTER0_1(HDMV_registration_descriptor, CHDMVRegistrationDescriptor); break;
				case 2: UNMAP_STRUCT_POINTER0_1(HDMV_video_registration_descriptor, CHDMVVideoRegistrationDescriptor); break;
				case 3: UNMAP_STRUCT_POINTER2_1(VC1_registration_descriptor); break;
				case 4: UNMAP_STRUCT_POINTER0_1(HDMV_LPCM_audio_registration_descriptor, CHDMVLPCMAudioRegistrationDescriptor); break;
				case 5: UNMAP_STRUCT_POINTER0_1(AC3_registration_descriptor, CAC3RegistrationDescriptor); break;
				case 6: UNMAP_STRUCT_POINTER0_1(DRA_registration_descriptor, CDRARegistrationDescriptor); break;
				}
				break;
			case DT_data_stream_alignment_descriptor: UNMAP_STRUCT_POINTER0_1(data_stream_alignment_descriptor, CDataStreamAlignmentDescriptor); break;
			case DT_target_background_grid_descriptor: UNMAP_STRUCT_POINTER0_1(target_background_grid_descriptor, CTargetBackgroundGridDescriptor); break;
			case DT_video_window_descriptor: UNMAP_STRUCT_POINTER0_1(video_window_descriptor, CVideoWindowDescriptor); break;
			case DT_CA_descriptor: UNMAP_STRUCT_POINTER0_1(CA_descriptor, CCADescriptor); break;								
			case DT_ISO_639_language_descriptor: UNMAP_STRUCT_POINTER0_1(ISO_639_language_descriptor, CISO639LanguageDescriptor); break;
			case DT_system_clock_descriptor: UNMAP_STRUCT_POINTER0_1(system_clock_descriptor, CSystemClockDescriptor); break;
			case DT_multiplex_buffer_utilization_descriptor: UNMAP_STRUCT_POINTER0_1(multiplex_buffer_utilization_descriptor, CMultiplexBufferUtilizationDescriptor); break;
			case DT_copyright_descriptor: UNMAP_STRUCT_POINTER0_1(copyright_descriptor, CCopyrightDescriptor); break;
			case DT_maximum_bitrate_descriptor: UNMAP_STRUCT_POINTER0_1(maximum_bitrate_descriptor, CMaximumBitrateDescriptor); break;
			case DT_private_data_indicator_descriptor: UNMAP_STRUCT_POINTER0_1(private_data_indicator_descriptor, CPrivateDataIndicatorDescriptor); break;
			case DT_smoothing_buffer_descriptor: UNMAP_STRUCT_POINTER0_1(smoothing_buffer_descriptor, CSmoothingBufferDescriptor); break;
			case DT_STD_descriptor: UNMAP_STRUCT_POINTER0_1(STD_descriptor, CSTDDescriptor); break;
			case DT_IBP_descriptor: UNMAP_STRUCT_POINTER0_1(IBP_descriptor, CIBPDescriptor); break;
			case DT_MPEG4_video_descriptor: UNMAP_STRUCT_POINTER0_1(MPEG4_video_descriptor, CMPEG4VideoDescriptor); break;
			case DT_MPEG4_audio_descriptor: UNMAP_STRUCT_POINTER0_1(MPEG4_audio_descriptor, CMPEG4AudioDescriptor); break;
			case DT_IOD_descriptor: UNMAP_STRUCT_POINTER0_1(IOD_descriptor, CIODDescriptor); break;
			case DT_SL_descriptor: UNMAP_STRUCT_POINTER0_1(SL_descriptor, CSLDescriptor); break;
			case DT_FMC_descriptor: UNMAP_STRUCT_POINTER0_1(FMC_descriptor, CFMCDescriptor); break;
			case DT_external_ES_ID_descriptor: UNMAP_STRUCT_POINTER0_1(external_ES_ID_descriptor, CExternalESIDDescriptor); break;
			case DT_MuxCode_descriptor: UNMAP_STRUCT_POINTER2_1(MuxCode_descriptor); break;	// Advance Endian Map
			case DT_FmxBufferSize_descriptor: UNMAP_STRUCT_POINTER0_1(FmxBufferSize_descriptor, CFmxBufferSizeDescriptor); break;
			case DT_multiplexbuffer_descriptor: UNMAP_STRUCT_POINTER0_1(multiplexbuffer_descriptor, CMultiplexBufferDescriptor); break;
			case DT_content_labeling_descriptor: UNMAP_STRUCT_POINTER2_1(content_labeling_descriptor); break;	// Advance Endian Map
			case DT_metadata_pointer_descriptor: UNMAP_STRUCT_POINTER2_1(metadata_pointer_descriptor); break;
			case DT_metadata_descriptor: UNMAP_STRUCT_POINTER2_1(metadata_descriptor); break;	// Advance Endian Map
			case DT_metadata_STD_descriptor: UNMAP_STRUCT_POINTER0_1(metadata_STD_descriptor, CMetadataSTDDescriptor); break;
			case DT_AVC_video_descriptor: UNMAP_STRUCT_POINTER0_1(AVC_video_descriptor, CAVCVideoDescriptor); break;
			//case DT_IPMP_descriptor: UNMAP_STRUCT_POINTER0_1( CA_descriptor); break;
			case DT_AVC_timing_and_HRD_descriptor: UNMAP_STRUCT_POINTER0_1(AVC_timing_and_HRD_descriptor, CAVCTimingAndHRDDescriptor); break;
			case DT_MPEG2_AAC_audio_descriptor: UNMAP_STRUCT_POINTER0_1(MPEG2_AAC_audio_descriptor, CMPEG2AACAudioDescriptor); break;
			case DT_FlexMuxTiming_descriptor: UNMAP_STRUCT_POINTER0_1(FlexMuxTiming_descriptor, CFlexMuxTimingDescriptor); break;
			case DT_MPEG4_text_descriptor: UNMAP_STRUCT_POINTER2_1(MPEG4_text_descriptor); break;	// Advance Endian Map
			case DT_MPEG4_audio_extension_descriptor: UNMAP_STRUCT_POINTER2_1(MPEG4_audio_extension_descriptor); break;	// Advance Endian Map
			case DT_auxiliary_video_stream_descriptor: UNMAP_STRUCT_POINTER0_1(auxiliary_video_stream_descriptor, CAuxiliaryVideoStreamDescriptor); break;
			case DT_SVC_extension_descriptor: UNMAP_STRUCT_POINTER0_1(SVC_extension_descriptor, CSVCExtensionDescriptor); break;
			case DT_MVC_extension_descriptor: UNMAP_STRUCT_POINTER0_1(MVC_extension_descriptor, CMVCExtensionDescriptor); break;
			case DT_J2K_video_descriptor: UNMAP_STRUCT_POINTER0_1(J2K_video_descriptor, CJ2KVideoDescriptor); break;
			case DT_MVC_operation_point_descriptor: UNMAP_STRUCT_POINTER2_1(MVC_operation_point_descriptor); break;	// Advance Endian Map
			case DT_MPEG2_stereoscopic_video_format_descriptor: UNMAP_STRUCT_POINTER0_1( MPEG2_stereoscopic_video_format_descriptor, CMPEG2StereoscopicVideoFormatDescriptor); break;
			case DT_Stereoscopic_program_info_descriptor: UNMAP_STRUCT_POINTER0_1(Stereoscopic_program_info_descriptor, CStereoscopicProgramInfoDescriptor); break;
			case DT_Stereoscopic_video_info_descriptor: UNMAP_STRUCT_POINTER2_1(Stereoscopic_video_info_descriptor); break;	// Advance Endian Map
			case DT_AC3_audio_descriptor: UNMAP_STRUCT_POINTER2_1(AC3_audio_descriptor); break;	// Advance Endian Map
			case DT_Caption_service_descriptor: UNMAP_STRUCT_POINTER0_1(Caption_service_descriptor, CCaptionServiceDescriptor); break;
			case DT_HDMV_copy_control_descritpor: UNMAP_STRUCT_POINTER0_1(HDMV_copy_control_descriptor, CHDMVCopyControlDescriptor); break;
			case DT_Partial_transport_stream_descriptor: UNMAP_STRUCT_POINTER0_1(Partial_transport_stream_descriptor, CPartialTransportStreamDescriptor); break;
			case DT_BD_system_use_descriptor: UNMAP_STRUCT_POINTER0_1(BD_system_use_descriptor, CBDSystemUseDescriptor); break;
			default:
				UNMAP_STRUCT_POINTER0_1( pDescriptor, CGeneralDescriptor)
			}
			if (pBuf == NULL && pcbSize == NULL)
				descriptor_tag = 1;
		}
		else
		{
			UNMAP_GENERAL_UTILITY_2()
		}
		return RET_CODE_SUCCESS;
	}

	int C13818_1_Descriptor::WriteToBs(AMBst bs)
	{
		switch(descriptor_tag)
		{
		case DT_video_stream_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(video_stream_descriptor, CVideoStreamDescriptor); break;
		case DT_audio_stream_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(audio_stream_descriptor, CAudioStreamDescriptor); break;
		case DT_hierarchy_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(hierarchy_descriptor, CHierarchyDescriptor); break;
		case DT_registration_descriptor:
			switch(sub_tag)
			{
			case 0: UNMAP_DUPLICATE_STRUCT_POINTER(registration_descriptor, CRegistrationDescriptor); break;
			case 1: UNMAP_DUPLICATE_STRUCT_POINTER(HDMV_registration_descriptor, CHDMVRegistrationDescriptor); break;
			case 2: UNMAP_DUPLICATE_STRUCT_POINTER(HDMV_video_registration_descriptor, CHDMVVideoRegistrationDescriptor); break;
			case 3: if(VC1_registration_descriptor)VC1_registration_descriptor->WriteToBs(bs); break;
			case 4: UNMAP_DUPLICATE_STRUCT_POINTER(HDMV_LPCM_audio_registration_descriptor, CHDMVLPCMAudioRegistrationDescriptor); break;
			case 5: UNMAP_DUPLICATE_STRUCT_POINTER(AC3_registration_descriptor, CAC3RegistrationDescriptor); break;
			case 6: UNMAP_DUPLICATE_STRUCT_POINTER(DRA_registration_descriptor, CDRARegistrationDescriptor); break;
			}
			break;
		case DT_data_stream_alignment_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(data_stream_alignment_descriptor, CDataStreamAlignmentDescriptor); break;
		case DT_target_background_grid_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(target_background_grid_descriptor, CTargetBackgroundGridDescriptor); break;
		case DT_video_window_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(video_window_descriptor, CVideoWindowDescriptor); break;
		case DT_CA_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(CA_descriptor, CCADescriptor); break;								
		case DT_ISO_639_language_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(ISO_639_language_descriptor, CISO639LanguageDescriptor); break;
		case DT_system_clock_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(system_clock_descriptor, CSystemClockDescriptor); break;
		case DT_multiplex_buffer_utilization_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(multiplex_buffer_utilization_descriptor, CMultiplexBufferUtilizationDescriptor); break;
		case DT_copyright_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(copyright_descriptor, CCopyrightDescriptor); break;
		case DT_maximum_bitrate_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(maximum_bitrate_descriptor, CMaximumBitrateDescriptor); break;
		case DT_private_data_indicator_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(private_data_indicator_descriptor, CPrivateDataIndicatorDescriptor); break;
		case DT_smoothing_buffer_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(smoothing_buffer_descriptor, CSmoothingBufferDescriptor); break;
		case DT_STD_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(STD_descriptor, CSTDDescriptor); break;
		case DT_IBP_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(IBP_descriptor, CIBPDescriptor); break;
		case DT_MPEG4_video_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(MPEG4_video_descriptor, CMPEG4VideoDescriptor); break;
		case DT_MPEG4_audio_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(MPEG4_audio_descriptor, CMPEG4AudioDescriptor); break;
		case DT_IOD_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(IOD_descriptor, CIODDescriptor); break;
		case DT_SL_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(SL_descriptor, CSLDescriptor); break;
		case DT_FMC_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(FMC_descriptor, CFMCDescriptor); break;
		case DT_external_ES_ID_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(external_ES_ID_descriptor, CExternalESIDDescriptor); break;
		case DT_MuxCode_descriptor: if(MuxCode_descriptor)MuxCode_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_FmxBufferSize_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(FmxBufferSize_descriptor, CFmxBufferSizeDescriptor); break;
		case DT_multiplexbuffer_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(multiplexbuffer_descriptor, CMultiplexBufferDescriptor); break;
		case DT_content_labeling_descriptor: if(content_labeling_descriptor)content_labeling_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_metadata_pointer_descriptor: if(metadata_pointer_descriptor)metadata_pointer_descriptor->WriteToBs(bs); break;
		case DT_metadata_descriptor: if(metadata_descriptor)metadata_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_metadata_STD_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(metadata_STD_descriptor, CMetadataSTDDescriptor); break;
		case DT_AVC_video_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(AVC_video_descriptor, CAVCVideoDescriptor); break;
		//case DT_IPMP_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER( CA_descriptor); break;
		case DT_AVC_timing_and_HRD_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(AVC_timing_and_HRD_descriptor, CAVCTimingAndHRDDescriptor); break;
		case DT_MPEG2_AAC_audio_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(MPEG2_AAC_audio_descriptor, CMPEG2AACAudioDescriptor); break;
		case DT_FlexMuxTiming_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(FlexMuxTiming_descriptor, CFlexMuxTimingDescriptor); break;
		case DT_MPEG4_text_descriptor: if(MPEG4_text_descriptor)MPEG4_text_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_MPEG4_audio_extension_descriptor: if(MPEG4_audio_extension_descriptor)MPEG4_audio_extension_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_auxiliary_video_stream_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(auxiliary_video_stream_descriptor, CAuxiliaryVideoStreamDescriptor); break;
		case DT_SVC_extension_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(SVC_extension_descriptor, CSVCExtensionDescriptor); break;
		case DT_MVC_extension_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(MVC_extension_descriptor, CMVCExtensionDescriptor); break;
		case DT_J2K_video_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(J2K_video_descriptor, CJ2KVideoDescriptor); break;
		case DT_MVC_operation_point_descriptor: if(MVC_operation_point_descriptor)MVC_operation_point_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_MPEG2_stereoscopic_video_format_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER( MPEG2_stereoscopic_video_format_descriptor, CMPEG2StereoscopicVideoFormatDescriptor); break;
		case DT_Stereoscopic_program_info_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(Stereoscopic_program_info_descriptor, CStereoscopicProgramInfoDescriptor); break;
		case DT_Stereoscopic_video_info_descriptor: if(Stereoscopic_video_info_descriptor)Stereoscopic_video_info_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_AC3_audio_descriptor: if(AC3_audio_descriptor)AC3_audio_descriptor->WriteToBs(bs); break;	// Advance Endian Map
		case DT_Caption_service_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(Caption_service_descriptor, CCaptionServiceDescriptor); break;
		case DT_HDMV_copy_control_descritpor: UNMAP_DUPLICATE_STRUCT_POINTER(HDMV_copy_control_descriptor, CHDMVCopyControlDescriptor); break;
		case DT_Partial_transport_stream_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(Partial_transport_stream_descriptor, CPartialTransportStreamDescriptor); break;
		case DT_BD_system_use_descriptor: UNMAP_DUPLICATE_STRUCT_POINTER(BD_system_use_descriptor, CBDSystemUseDescriptor); break;
		default:
			UNMAP_DUPLICATE_STRUCT_POINTER( pDescriptor, CGeneralDescriptor)
		}

		return RET_CODE_SUCCESS;
	}

	size_t C13818_1_Descriptor::ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint, long long* bit_offset)
	{
		size_t cbRequired = 0;
		char szTemp2[TEMP2_SIZE], szTemp3[TEMP3_SIZE], szTemp4[TEMP4_SIZE], szTagName[TAGNAME_SIZE];
		char* pTemp = NULL;
		memset(szTemp2, 0, sizeof(szTemp2));
		memset(szTemp3, 0, sizeof(szTemp3));
		memset(szTemp4, 0, sizeof(szTemp4));
		memset(szTagName, 0, sizeof(szTagName));
		if (szOutXml != 0 && cbLen > 0)
			memset(szOutXml, 0, cbLen);

		switch (descriptor_tag)
		{
		case DT_video_stream_descriptor: NAV_FIELD_PROP_REF1(video_stream_descriptor); break;
		case DT_audio_stream_descriptor: NAV_FIELD_PROP_REF1(audio_stream_descriptor); break;
		case DT_hierarchy_descriptor: NAV_FIELD_PROP_REF1(hierarchy_descriptor); break;
		case DT_registration_descriptor:
			switch (sub_tag)
			{
			case 0: NAV_FIELD_PROP_REF1(registration_descriptor); break;
			case 1: NAV_FIELD_PROP_REF1(HDMV_registration_descriptor); break;
			case 2: NAV_FIELD_PROP_REF1(HDMV_video_registration_descriptor); break;
			case 3: NAV_FIELD_PROP_REF1(VC1_registration_descriptor); break;
			case 4: NAV_FIELD_PROP_REF1(HDMV_LPCM_audio_registration_descriptor); break;
			case 5: NAV_FIELD_PROP_REF1(AC3_registration_descriptor); break;
			case 6: NAV_FIELD_PROP_REF1(DRA_registration_descriptor); break;
			}
			break;
		case DT_data_stream_alignment_descriptor: NAV_FIELD_PROP_REF1(data_stream_alignment_descriptor); break;
		case DT_target_background_grid_descriptor: NAV_FIELD_PROP_REF1(target_background_grid_descriptor); break;
		case DT_video_window_descriptor: NAV_FIELD_PROP_REF1(video_window_descriptor); break;
		case DT_CA_descriptor: NAV_FIELD_PROP_REF1(CA_descriptor); break;
		case DT_ISO_639_language_descriptor: NAV_FIELD_PROP_REF1(ISO_639_language_descriptor); break;
		case DT_system_clock_descriptor: NAV_FIELD_PROP_REF1(system_clock_descriptor); break;
		case DT_multiplex_buffer_utilization_descriptor: NAV_FIELD_PROP_REF1(multiplex_buffer_utilization_descriptor); break;
		case DT_copyright_descriptor: NAV_FIELD_PROP_REF1(copyright_descriptor); break;
		case DT_maximum_bitrate_descriptor: NAV_FIELD_PROP_REF1(maximum_bitrate_descriptor); break;
		case DT_private_data_indicator_descriptor: NAV_FIELD_PROP_REF1(private_data_indicator_descriptor); break;
		case DT_smoothing_buffer_descriptor: NAV_FIELD_PROP_REF1(smoothing_buffer_descriptor); break;
		case DT_STD_descriptor: NAV_FIELD_PROP_REF1(STD_descriptor); break;
		case DT_IBP_descriptor: NAV_FIELD_PROP_REF1(IBP_descriptor); break;
		case DT_MPEG4_video_descriptor: NAV_FIELD_PROP_REF1(MPEG4_video_descriptor); break;
		case DT_MPEG4_audio_descriptor: NAV_FIELD_PROP_REF1(MPEG4_audio_descriptor); break;
		case DT_IOD_descriptor: NAV_FIELD_PROP_REF1(IOD_descriptor); break;
		case DT_SL_descriptor: NAV_FIELD_PROP_REF1(SL_descriptor); break;
		case DT_FMC_descriptor: NAV_FIELD_PROP_REF1(FMC_descriptor); break;
		case DT_external_ES_ID_descriptor: NAV_FIELD_PROP_REF1(external_ES_ID_descriptor); break;
		case DT_MuxCode_descriptor: NAV_FIELD_PROP_REF1(MuxCode_descriptor); break;	// Advance Endian Map
		case DT_FmxBufferSize_descriptor: NAV_FIELD_PROP_REF1(FmxBufferSize_descriptor); break;
		case DT_multiplexbuffer_descriptor: NAV_FIELD_PROP_REF1(multiplexbuffer_descriptor); break;
		case DT_content_labeling_descriptor: NAV_FIELD_PROP_REF1(content_labeling_descriptor); break;	// Advance Endian Map
		case DT_metadata_pointer_descriptor: NAV_FIELD_PROP_REF1(metadata_pointer_descriptor); break;
		case DT_metadata_descriptor: NAV_FIELD_PROP_REF1(metadata_descriptor); break;	// Advance Endian Map
		case DT_metadata_STD_descriptor: NAV_FIELD_PROP_REF1(metadata_STD_descriptor); break;
		case DT_AVC_video_descriptor: NAV_FIELD_PROP_REF1(AVC_video_descriptor); break;
		//case DT_IPMP_descriptor: NAV_FIELD_PROP_REF1( CA_descriptor); break;
		case DT_AVC_timing_and_HRD_descriptor: NAV_FIELD_PROP_REF1(AVC_timing_and_HRD_descriptor); break;
		case DT_MPEG2_AAC_audio_descriptor: NAV_FIELD_PROP_REF1(MPEG2_AAC_audio_descriptor); break;
		case DT_FlexMuxTiming_descriptor: NAV_FIELD_PROP_REF1(FlexMuxTiming_descriptor); break;
		case DT_MPEG4_text_descriptor: NAV_FIELD_PROP_REF1(MPEG4_text_descriptor); break;	// Advance Endian Map
		case DT_MPEG4_audio_extension_descriptor: NAV_FIELD_PROP_REF1(MPEG4_audio_extension_descriptor); break;	// Advance Endian Map
		case DT_auxiliary_video_stream_descriptor: NAV_FIELD_PROP_REF1(auxiliary_video_stream_descriptor); break;
		case DT_SVC_extension_descriptor: NAV_FIELD_PROP_REF1(SVC_extension_descriptor); break;
		case DT_MVC_extension_descriptor: NAV_FIELD_PROP_REF1(MVC_extension_descriptor); break;
		case DT_J2K_video_descriptor: NAV_FIELD_PROP_REF1(J2K_video_descriptor); break;
		case DT_MVC_operation_point_descriptor: NAV_FIELD_PROP_REF1(MVC_operation_point_descriptor); break;	// Advance Endian Map
		case DT_MPEG2_stereoscopic_video_format_descriptor: NAV_FIELD_PROP_REF1(MPEG2_stereoscopic_video_format_descriptor); break;
		case DT_Stereoscopic_program_info_descriptor: NAV_FIELD_PROP_REF1(Stereoscopic_program_info_descriptor); break;
		case DT_Stereoscopic_video_info_descriptor: NAV_FIELD_PROP_REF1(Stereoscopic_video_info_descriptor); break;	// Advance Endian Map
		case DT_AC3_audio_descriptor: NAV_FIELD_PROP_REF1(AC3_audio_descriptor); break;	// Advance Endian Map
		case DT_Caption_service_descriptor: NAV_FIELD_PROP_REF1(Caption_service_descriptor); break;
		case DT_HDMV_copy_control_descritpor: NAV_FIELD_PROP_REF1(HDMV_copy_control_descriptor); break;
		case DT_Partial_transport_stream_descriptor: NAV_FIELD_PROP_REF1(Partial_transport_stream_descriptor); break;
		case DT_BD_system_use_descriptor: NAV_FIELD_PROP_REF1(BD_system_use_descriptor); break;
		default:
			NAV_FIELD_PROP_REF1(pDescriptor);
		}
		return cbRequired;
	}

}	//  namespace BST

