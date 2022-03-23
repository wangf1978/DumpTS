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
#include "platcomm.h"
#include "mpeg2_video.h"
#include "mpeg2_video_parser.h"

const char* picture_coding_type_name[8] = {
/* 000 */ "Forbidden",
/* 001 */ "intra-coded (I)", 
/* 010 */ "predictive-coded (P)", 
/* 011 */ "bidirectionally-predictive-coded (B)", 
/* 100 */ "Shall not be used(dc intra-coded (D) in ISO/IEC11172-2)",
/* 101 */ "Reserved",
/* 110 */ "Reserved",
/* 111 */ "Reserved"
};

const char* aspect_ratio_infomation_desc[16] = {
	"Forbidden",
	"Square sample",
	"4:3",
	"16:9",
	"2.21:1",
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

const char* frame_rate_value_desc[16] = {
	"Forbidden",
	"24 000/1001 (23.976...)",
	"24",
	"25",
	"30 000/1001 (29.97...)",
	"30",
	"50",
	"60 000/1001 (59.94...)",
	"60",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

/*
const char* chroma_format_names[4] = {
	"Reserved",
	"4:2:0",
	"4:2:2",
	"4:4:4"
};
*/

const char* mpeg2_video_format_names[8] = {
	"Component",
	"PAL",
	"NTSC",
	"SECAM",
	"MAC",
	"Unspecified video format",
	"Reserved",
	"Reserved"
};

const char* mpeg2_video_scalable_mode[4] = {
	"Data partitioning",
	"Spatial scalability",
	"SNR scalability",
	"Temporal scalability"
};

const char* mpeg2_video_timecode_type_names[4] = {
	"one timestamp for the frame",
	"one timestamp for the first or only field",
	"one timestamp for the second field",
	"two timestamps, one for each of two fields"
};

const char* mpeg2_video_counting_type_names[8] = {
	"nframes parameter not used",
	"no dropping of nframes count values",
	"dropping of individual zero values of nframes count",
	"dropping of individual max_nframes values of nframes count",
	"dropping of the two lowest (values 0 and 1) nframes counts when units_of_seconds and tens_of_seconds are zero and units_of_minutes is not zero",
	"dropping of unspecified individual nframes count values",
	"dropping of unspecified numbers of unspecified nframes count values",
	"reserved"
};

const char* profile_and_level_identification_names[8][16] = {
	{"reserved","reserved",	"@HighP",		"reserved",	"@High",		"reserved", "@High 1440",		"reserved", "@Main",		"reserved", "@Low",			"reserved", "reserved", "reserved", "reserved", "reserved"},
	{"High",	"High",		"High@HighP",	"High",		"High@High",	"High",		"High@High 1440",	"High",		"High@Main",	"High",		"High@Low",		"High",		"High",		"High",		"High",		"High"},
	{"Spatial",	"Spatial",	"Spatial@HighP","Spatial",	"Spatial@High",	"Spatial",	"Spatial@High 1440","Spatial",	"Spatial@Main",	"Spatial",	"Spatial@Low",	"Spatial",	"Spatial",	"Spatial",	"Spatial",	"Spatial"},
	{"SNR",		"SNR",		"SNR@HighP",	"SNR",		"SNR@High",		"SNR",		"SNR@High 1440",	"SNR",		"SNR@Main",		"SNR",		"SNR@Low",		"SNR",		"SNR",		"SNR",		"SNR",		"SNR"},
	{"Main",	"Main",		"Main@HighP",	"Main",		"Main@High",	"Main",		"Main@High 1440",	"Main",		"Main@Main",	"Main",		"Main@Low",		"Main",		"Main",		"Main",		"Main",		"Main"},
	{"Simple",	"Simple",	"Simple@HighP","Simple",	"Simple@High",	"Simple",	"Simple@High 1440",	"Simple",	"Simple@Main",	"Simple",	"Simple@Low",	"Simple",	"Simple",	"Simple",	"Simple",	"Simple"},
	{"reserved","reserved",	"@HighP",		"reserved",	"@High",		"reserved", "@High 1440",		"reserved", "@Main",		"reserved", "@Low",			"reserved", "reserved", "reserved", "reserved", "reserved"},
	{"reserved","reserved",	"@HighP",		"reserved",	"@High",		"reserved", "@High 1440",		"reserved", "@Main",		"reserved", "@Low",			"reserved", "reserved", "reserved", "reserved", "reserved"},
};

const char* mpeg2_profile_names[8] = {
	"", "High", "Spatially Scalable", "SNR Scalable", "Main", "Simple", "4:2:2", "Multi-view"
};

const char* mpeg2_level_names[16] = {
	"", "", "HighP", "", "High", "", "High 1440", "", "Main", "Low", "", "", "", "", "", ""
};

const char* mpv_syntax_element_names[256] =
{
	"picture_header",
	"slice1",   "slice2", "slice3", "slice4", "slice5", "slice6", "slice7", "slice8", "slice9", "slice10",
	"slice11",  "slice12", "slice13", "slice14", "slice15", "slice16", "slice17", "slice18", "slice19", "slice20",
	"slice21",  "slice22", "slice23", "slice24", "slice25", "slice26", "slice27", "slice28", "slice29", "slice30",
	"slice31",  "slice32", "slice33", "slice34", "slice35", "slice36", "slice37", "slice38", "slice39", "slice40",
	"slice41",  "slice42", "slice43", "slice44", "slice45", "slice46", "slice47", "slice48", "slice49", "slice50",
	"slice51",  "slice52", "slice53", "slice54", "slice55", "slice56", "slice57", "slice58", "slice59", "slice60",
	"slice61",  "slice62", "slice63", "slice64", "slice65", "slice66", "slice67", "slice68", "slice69", "slice70",
	"slice71",  "slice72", "slice73", "slice74", "slice75", "slice76", "slice77", "slice78", "slice79", "slice80",
	"slice81",  "slice82", "slice83", "slice84", "slice85", "slice86", "slice87", "slice88", "slice89", "slice90",
	"slice91",  "slice92", "slice93", "slice94", "slice95", "slice96", "slice97", "slice98", "slice99", "slice100",
	"slice101", "slice102", "slice103", "slice104", "slice105", "slice106", "slice107", "slice108", "slice109", "slice110",
	"slice111", "slice112", "slice113", "slice114", "slice115", "slice116", "slice117", "slice118", "slice119", "slice120",
	"slice121", "slice122", "slice123", "slice124", "slice125", "slice126", "slice127", "slice128", "slice129", "slice130",
	"slice131", "slice132", "slice133", "slice134", "slice135", "slice136", "slice137", "slice138", "slice139", "slice140",
	"slice141", "slice142", "slice143", "slice144", "slice145", "slice146", "slice147", "slice148", "slice149", "slice150",
	"slice151", "slice152", "slice153", "slice154", "slice155", "slice156", "slice157", "slice158", "slice159", "slice160",
	"slice161", "slice162", "slice163", "slice164", "slice165", "slice166", "slice167", "slice168", "slice169", "slice170",
	"slice171", "slice172", "slice173", "slice174", "slice175", "", "", "user_data", "sequence_header", "sequence_error",
	"extension_data", "", "sequence_end", "group_of_pictures_header", "MPEG_program_end", "pack_header", "system_header", 
	"program_stream_map", "private_stream_1", "padding_stream", "private_stream_2", 
	"MPEG_audio_stream_0", "MPEG_audio_stream_1","MPEG_audio_stream_2","MPEG_audio_stream_3","MPEG_audio_stream_4","MPEG_audio_stream_5","MPEG_audio_stream_6","MPEG_audio_stream_7",
	"MPEG_audio_stream_8", "MPEG_audio_stream_9","MPEG_audio_stream_10","MPEG_audio_stream_11","MPEG_audio_stream_12","MPEG_audio_stream_13","MPEG_audio_stream_14","MPEG_audio_stream_15",
	"MPEG_audio_stream_16", "MPEG_audio_stream_17","MPEG_audio_stream_18","MPEG_audio_stream_19","MPEG_audio_stream_20","MPEG_audio_stream_21","MPEG_audio_stream_22","MPEG_audio_stream_23",
	"MPEG_audio_stream_24", "MPEG_audio_stream_25","MPEG_audio_stream_26","MPEG_audio_stream_27","MPEG_audio_stream_28","MPEG_audio_stream_29","MPEG_audio_stream_30","MPEG_audio_stream_31",
	"video_stream_0","video_stream_1","video_stream_2","video_stream_3","video_stream_4","video_stream_5","video_stream_6","video_stream_7",
	"video_stream_8","video_stream_9","video_stream_10","video_stream_11","video_stream_12","video_stream_13","video_stream_14","video_stream_15",
	"ECM_stream", "EMM_stream", "DSMCC_stream", "ISO_13522_stream", 
	"H.222.1 type A", "H.222.1 type B", "H.222.1 type C", "H.222.1 type D", "H.222.1 type E",
	"ancillary_stream", "ISO-14496-1_SL-packetized_stream", "ISO-14496-1_FlexMux_stream", "metadata stream", "extended_stream", "reserved data stream", "program_stream_directory"
};

const char* mpv_extension_syntax_element_names[16] = 
{
	"reserved",
	"sequence_extension",
	"sequence_display_extension",
	"quant_matrix_extension",
	"copyright_extension",
	"sequence_scalable_extension",
	"reserved",
	"picture_display_extension",
	"picture_coding_extension",
	"picture_spatial_scalable_extension",
	"picture_temporal_scalable_extension",
	"camera_parameters_extension",
	"ITU-T_extension",
	"reserved",
	"reserved",
	"reserved"
};

int next_start_code(AMBst in_bst) {

	int iRet = RET_CODE_SUCCESS;
	try
	{
		if (AMP_FAILED(iRet = AMBst_Realign(in_bst)))
		{
			printf("error happened or data is insufficient in bitstream {retcode: %d}.\n", iRet);
			return iRet;
		}

		int left_bits = 0;
		AMBst_Tell(in_bst, &left_bits);
		while (left_bits >= 24 && AMBst_PeekBits(in_bst, 24) != 0x000001)
		{
			AMBst_SkipBits(in_bst, 8);
			left_bits -= 8;
		}

		if (left_bits < 24)
			iRet = RET_CODE_NO_MORE_DATA;
	}
	catch (AMException e)
	{
		iRet = e.RetCode();
	}

	return iRet;
}

const char* get_profile_and_level_indication_names(int profile_and_level_indication) {
	unsigned char Escape_bit = (profile_and_level_indication >> 7) & 0X01;
	unsigned char Profile_identification = (profile_and_level_indication >> 4) & 0x07;
	unsigned char Level_identification = profile_and_level_indication & 0xF;

	if (Escape_bit) {
		if (Profile_identification == 0) {
			switch (Level_identification) {
			case 0xE:
				return "Multi-view profile@Low level";
			case 0XB:
				return "Multi-view profile@High1440 level";
			case 0xA:
				return "Multi-view profile@High level";
			case 0x5:
				return "4:2:2 profile@Main level";
			case 0x2:
				return "4:2:2 profile@High level";
			default:
				return "reserved";
			}
		}
		return "reserved";
	}

	return profile_and_level_identification_names[Profile_identification][Level_identification];
}

uint8_t* FindNextStartCode(uint8_t* pBuf, unsigned long cbSize)
{
	// Find the start_code_prefix_one_3bytes
	uint8_t* pStart = pBuf;
	while (cbSize >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
	{
		cbSize--;
		pBuf++;
	}

	if (cbSize < 3)
		return NULL;

	return pBuf;
}

RET_CODE CreateMPVContext(IMPVContext** ppMPVCtx)
{
	if (ppMPVCtx == NULL)
		return RET_CODE_INVALID_PARAMETER;

	auto pCtx = new BST::MPEG2Video::VideoBitstreamCtx();
	pCtx->AddRef();
	*ppMPVCtx = (IMPVContext*)pCtx;
	return RET_CODE_SUCCESS;
}

namespace BST
{
	namespace MPEG2Video
	{
		RET_CODE VideoBitstreamCtx::SetStartCodeFilters(std::initializer_list<uint16_t> start_code_filters)
		{
			m_start_code_filters = start_code_filters;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetStartCodeFilters(std::vector<uint16_t>& start_code_filters)
		{
			start_code_filters = m_start_code_filters;
			return RET_CODE_SUCCESS;
		}

		bool VideoBitstreamCtx::IsStartCodeFiltered(uint16_t start_code)
		{
			if (m_start_code_filters.size() == 0)
				return true;

			return std::find(m_start_code_filters.begin(), m_start_code_filters.end(), start_code) == m_start_code_filters.end() ? false : true;
		}

		void VideoBitstreamCtx::UpdateStartCode(uint16_t start_code)
		{
			// If a new sequence header is started, clear the start code history
			if (start_code == SEQUENCE_HEADER_CODE || start_code == GROUP_START_CODE || start_code == PICTURE_START_CODE)
			{
				m_curr_level = -1;
				m_start_codes.clear();
			}

			m_start_codes.push_back(start_code);

			// Update the current level
			for (auto riter = m_start_codes.rbegin(); riter != m_start_codes.rend(); riter++)
			{
				if (*riter == PICTURE_START_CODE)
				{
					m_curr_level = 2;
					break;
				}
				else if (*riter == GROUP_START_CODE)
				{
					m_curr_level = 1;
					break;
				}
				else if (*riter == SEQUENCE_HEADER_CODE)
				{
					m_curr_level = 0;
					break;
				}
			}
		}

		int VideoBitstreamCtx::GetCurrentLevel()
		{
			return m_curr_level;
		}

		SEQHDR VideoBitstreamCtx::GetSeqHdr()
		{
			return m_seq_hdr;
		}

		RET_CODE VideoBitstreamCtx::UpdateSeqHdr(SEQHDR seqHdr)
		{
			if (!seqHdr)
				return RET_CODE_INVALID_PARAMETER;

			m_seq_hdr = seqHdr;
			return RET_CODE_SUCCESS;
		}

		SEQEXT VideoBitstreamCtx::GetSeqExt()
		{
			return m_seq_ext;
		}

		RET_CODE VideoBitstreamCtx::UpdateSeqExt(SEQEXT seqExt)
		{
			if (!seqExt)
				return RET_CODE_INVALID_PARAMETER;

			m_seq_ext = seqExt;
			return RET_CODE_SUCCESS;
		}

		SEQSCAEXT VideoBitstreamCtx::GetSeqScalableExt()
		{
			return m_seq_scalable_ext;
		}

		RET_CODE VideoBitstreamCtx::UpdateSeqScalableExt(SEQSCAEXT seqScaExt)
		{
			if (!seqScaExt)
				return RET_CODE_INVALID_PARAMETER;

			m_seq_scalable_ext = seqScaExt;
			return RET_CODE_SUCCESS;
		}

		void VideoBitstreamCtx::Reset()
		{
			m_curr_level = -1;
			m_start_codes.clear();
			m_seq_hdr = nullptr;
			m_seq_ext = nullptr;
		}
	}
}

