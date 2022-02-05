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
#include "mpeg2video.h"
#include "mpeg2videoparser.h"

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

int next_start_code(AMBst in_bst) {

	int iRet = RET_CODE_SUCCESS;
	try
	{
		if (AMP_FAILED(iRet = AMBst_Realign(in_bst)))
		{
			printf(_T("error happened or data is insufficient in bitstream {retcode: %d}.\n"), iRet);
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
		RET_CODE VideoBitstreamCtx::SetStartCodeFilters(std::initializer_list<uint8_t> start_code_filters)
		{
			m_start_code_filters = start_code_filters;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetStartCodeFilters(std::vector<uint8_t>& start_code_filters)
		{
			start_code_filters = m_start_code_filters;
			return RET_CODE_SUCCESS;
		}

		bool VideoBitstreamCtx::IsStartCodeFiltered(uint8_t start_code)
		{
			return m_start_code_filters.size() == 0 ||
				std::find(m_start_code_filters.begin(), m_start_code_filters.end(), start_code) == m_start_code_filters.end() ? false : true;
		}

		void VideoBitstreamCtx::UpdateStartCode(uint8_t start_code)
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

		void VideoBitstreamCtx::Reset()
		{
			m_curr_level = -1;
			m_start_codes.clear();
			m_seq_hdr = nullptr;
			m_seq_ext = nullptr;
		}
	}
}
