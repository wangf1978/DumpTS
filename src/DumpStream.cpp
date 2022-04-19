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
#include "PayloadBuf.h"
#include <memory.h>
#include <string.h>
#include "Bitstream.h"
#include "DumpTS.h"
#include "AudChMap.h"
#include "nal_com.h"
#include <limits>
#include <type_traits>
#include <iterator>
#include "DataUtil.h"
#include "mpeg2_video_parser.h"
#include "mpeg2_video.h"
#include "ISO14496_3.h"
#include "system_13818_1.h"

using namespace std;

extern const char *dump_msg[];
extern map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;

extern const char* vui_transfer_characteristics_names[256];
extern const char* vui_colour_primaries_names[256];
extern const char* chroma_format_idc_names[4];
extern const char* hevc_profile_name[36];
extern const char* get_h264_profile_name(int profile);
extern const char* get_h264_level_name(int level);

// For MLP audio
#define FBB_SYNC_CODE				0xF8726FBB
#define FBA_SYNC_CODE				0xF8726FBA
#define MLP_SIGNATURE				0xB752

#define AC3_SYNCWORD				0x0B77
#define EAC3_SYNCWORD				0x0B77

#define DTS_SYNCWORD_CORE			0x7ffe8001
#define DTS_SYNCWORD_XCH			0x5a5a5a5a
#define DTS_SYNCWORD_XXCH			0x47004a03
#define DTS_SYNCWORD_X96			0x1d95f262
#define DTS_SYNCWORD_XBR			0x655e315e
#define DTS_SYNCWORD_LBR			0x0a801921
#define DTS_SYNCWORD_XLL			0x41a29547
#define DTS_SYNCWORD_SUBSTREAM		0x64582025
#define DTS_SYNCWORD_SUBSTREAM_CORE 0x02b09261

#define AAC_SYNCWORD				0xFFF
#define MPEGA_FRAME_SYNC			0x7FF

#define ADTS_HEADER_SIZE			7

using PID_StramInfos = unordered_map<unsigned short, std::vector<STREAM_INFO>>;
using DDP_Program_StreamInfos = unordered_map<uint8_t /* Program */, std::vector<STREAM_INFO>>;

PID_StramInfos g_stream_infos;
DDP_Program_StreamInfos g_ddp_program_stream_infos;
uint8_t g_cur_ddp_program_id = 0XFF;
std::vector<STREAM_INFO> g_cur_dtshd_stream_infos;

struct FRAME_SIZE_CODE_TABLE
{
	short	nominal_bit_rate;
	short	words[3];
}PACKED;

const FRAME_SIZE_CODE_TABLE frame_size_code_table[38] = {
	{ 32,  {64,   69,     96} },
	{ 32,  {64,   70,     96} },
	{ 40,  {80,   87,    120} },
	{ 40,  {80,   88,    120} },
	{ 48,  {96,   104,   144} },
	{ 48,  {96,   105,   144} },
	{ 56,  {112,  121,   168} },
	{ 56,  {112,  122,   168} },
	{ 64,  {128,  139,   192} },
	{ 64,  {128,  140,   192} },
	{ 80,  {160,  174,   240} },
	{ 80,  {160,  175,   240} },
	{ 96,  {192,  208,   288} },
	{ 96,  {192,  209,   288} },
	{ 112, {224,  243,   336} },
	{ 112, {224,  244,   336} },
	{ 128, {256,  278,   384} },
	{ 128, {256,  279,   384} },
	{ 160, {320,  348,   480} },
	{ 160, {320,  349,   480} },
	{ 192, {384,  417,   576} },
	{ 192, {384,  418,   576} },
	{ 224, {448,  487,   672} },
	{ 224, {448,  488,   672} },
	{ 256, {512,  557,   768} },
	{ 256, {512,  558,   768} },
	{ 320, {640,  696,   960} },
	{ 320, {640,  697,   960} },
	{ 384, {768,  835,  1152} },
	{ 384, {768,  836,  1152} },
	{ 448, {896,  975,  1344} },
	{ 448, {896,  976,  1344} },
	{ 512, {1024, 1114, 1536} },
	{ 512, {1024, 1115, 1536} },
	{ 576, {1152, 1253, 1728} },
	{ 576, {1152, 1254, 1728} },
	{ 640, {1280, 1393, 1920} },
	{ 640, {1280, 1394, 1920} },
};

extern int GetStreamInfoFromSPS(NAL_CODING coding, uint8_t* pAnnexBBuf, size_t cbAnnexBBuf, STREAM_INFO& stm_info);
extern int GetStreamInfoSeqHdrAndExt(
	BST::MPEG2Video::CSequenceHeader* pSeqHdr,
	BST::MPEG2Video::CSequenceExtension* pSeqExt,
	BST::MPEG2Video::CSequenceDisplayExtension* pSeqDispExt,
	STREAM_INFO& stm_info);

int ParseAC3Frame(unsigned short PID, int stream_type, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	if (cbSize < 0)
		return -1;

	CBitstream bst(pBuf, (size_t)cbSize << 3);
	bst.SkipBits(32);

	uint8_t fscod = (uint8_t)bst.GetBits(2);
	uint8_t frmsizecod = (uint8_t)bst.GetBits(6);

	uint8_t bsid = (uint8_t)bst.GetBits(5);

	if (bsid != 0x08)
		return -1;

	uint8_t bsmod = (uint8_t)bst.GetBits(3);
	uint8_t acmod = (uint8_t)bst.GetBits(3);

	uint8_t cmixlev = 0;
	uint8_t surmixlev = 0;
	uint8_t dsurmod = 0;
	if ((acmod & 0x1) && (acmod != 1))
		cmixlev = (uint8_t)bst.GetBits(2);

	if ((acmod & 0x4))
		surmixlev = (uint8_t)bst.GetBits(2);

	if (acmod == 0x2)
		dsurmod = (uint8_t)bst.GetBits(2);

	uint8_t lfeon = (uint8_t)bst.GetBits(1);

	audio_info.audio_info.bits_per_sample = 16;
	audio_info.audio_info.sample_frequency = fscod == 0 ? 48000 : (fscod == 1 ? 44100 : (fscod == 2 ? 32000 : 0));
	audio_info.audio_info.channel_mapping = acmod_ch_assignments[acmod];

	if (lfeon)
		audio_info.audio_info.channel_mapping.LFE = 1;

	audio_info.stream_coding_type = stream_type;

	if (frmsizecod >= 0 && frmsizecod <= 37)
		audio_info.audio_info.bitrate = frame_size_code_table[frmsizecod].nominal_bit_rate*1000;

	return 0;
}

int ParseEAC3Frame(unsigned short PID, int stream_type, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info, unsigned char& ddp_progid)
{
	int iRet = -1;
	if (cbSize < 0)
		return -1;

	CBitstream bst(pBuf, (size_t)cbSize << 3);
	bst.SkipBits(16);

	uint32_t byte2to5 = (uint32_t)bst.PeekBits(32);
	uint8_t bsid = (byte2to5 >> 3) & 0x1F;

	if (bsid != 0x8 && bsid != 0x10)
		return -1;

	if (bsid == 0x8 || (bsid == 0x10 && ((byte2to5 >> 30) == 0 || (byte2to5 >> 30) == 2)))
	{
		unsigned char program_id = 0;
		if (bsid == 0x08)
			program_id = 0;
		else
		{
			uint8_t substreamid = (byte2to5 >> 27) & 0x7;
			program_id = substreamid;
		}

		if (g_ddp_program_stream_infos.size() > 0 && 
			g_ddp_program_stream_infos.find(program_id) != g_ddp_program_stream_infos.end() &&
			g_ddp_program_stream_infos[program_id].size() > 0)
		{
			auto ddp_prog_stminfo = &g_ddp_program_stream_infos[program_id][0];

			// Current it is an AC3 program
			audio_info.stream_coding_type = ddp_prog_stminfo->stream_coding_type;
			audio_info.audio_info.bits_per_sample = 16;
			audio_info.audio_info.sample_frequency = ddp_prog_stminfo->audio_info.sample_frequency;

			CH_MAPPING ddp_channel_mapping;
			for (size_t i = 0; i < g_ddp_program_stream_infos[program_id].size(); i++)
			{
				ddp_channel_mapping.u64Val |= g_ddp_program_stream_infos[program_id][i].audio_info.channel_mapping.u64Val;
			}
				
			audio_info.audio_info.channel_mapping = ddp_prog_stminfo->audio_info.channel_mapping = ddp_channel_mapping;

			g_ddp_program_stream_infos[program_id].clear();
			ddp_progid = program_id;
			iRet = 0;
		}
	}

	if (bsid == 0x8)	// AC3
	{
		STREAM_INFO ac3_info;
		if (ParseAC3Frame(PID, stream_type, pBuf, cbSize, ac3_info) < 0)
		{
			// Clear the current information
			g_ddp_program_stream_infos[0].clear();
			goto done;
		}

		g_cur_ddp_program_id = 0;
		g_ddp_program_stream_infos[0].resize(1);
		g_ddp_program_stream_infos[0][0] = ac3_info;
	}
	else if (bsid == 0x10)	// EAC3
	{
		uint8_t strmtyp = (uint8_t)bst.GetBits(2);
		uint8_t substreamid = (uint8_t)bst.GetBits(3);

		uint16_t frmsiz = (uint16_t)bst.GetBits(11);

		uint8_t fscod = (uint8_t)bst.GetBits(2);
		uint8_t fscod2 = 0xFF;
		uint8_t numblkscod = 0x3;
		if (fscod == 0x3)
			fscod2 = (uint8_t)bst.GetBits(2);
		else
			numblkscod = (uint8_t)bst.GetBits(2);

		uint8_t acmod = (uint8_t)bst.GetBits(3);
		uint8_t lfeon = (uint8_t)bst.GetBits(1);

		uint8_t bsid = (uint8_t)bst.GetBits(5);
		uint8_t dialnorm = (uint8_t)bst.GetBits(5);
		uint8_t compre = (uint8_t)bst.GetBits(1);
		if (compre)
			bst.SkipBits(8);

		if (acmod == 0x0)
		{
			bst.SkipBits(5);
			uint8_t compr2e = (uint8_t)bst.GetBits(1);
			if (compr2e)
				bst.SkipBits(8);
		}

		uint16_t chanmap = 0;
		uint8_t chanmape = 0;
		if (strmtyp == 0x01)
		{
			chanmape = (uint8_t)bst.GetBits(1);
			if (chanmape)
				chanmap = (uint16_t)bst.GetBits(16);
		}

		auto getfs = [](uint8_t fscod, uint8_t fscod2) {
			return fscod == 0 ? 48000 : (fscod == 1 ? 44100 : (fscod == 2 ? 32000 : (fscod == 3 ? (
				fscod2 == 0 ? 24000 : (fscod2 == 1 ? 22050 : (fscod2 == 2 ? 16000 : 0))) : 0)));
		};

		if (strmtyp == 0 || strmtyp == 2)
		{
			g_cur_ddp_program_id = substreamid;
			g_ddp_program_stream_infos[substreamid].resize(1);

			// analyze the audio info.
			auto ptr_stm_info = &g_ddp_program_stream_infos[substreamid][0];
			ptr_stm_info->stream_coding_type = stream_type;
			ptr_stm_info->audio_info.bits_per_sample = 16;
			ptr_stm_info->audio_info.sample_frequency = getfs(fscod, fscod2);
			ptr_stm_info->audio_info.channel_mapping = acmod_ch_assignments[acmod];
			
			if (lfeon)
				ptr_stm_info->audio_info.channel_mapping.LFE = 1;
		}
		else if (strmtyp == 1 && g_cur_ddp_program_id >= 0 && g_cur_ddp_program_id <= 7)
		{
			if (g_ddp_program_stream_infos[g_cur_ddp_program_id].size() != (size_t)substreamid + 1UL)
			{
				g_ddp_program_stream_infos[g_cur_ddp_program_id].clear();
				goto done;
			}

			g_ddp_program_stream_infos[g_cur_ddp_program_id].resize((size_t)substreamid + 2);

			// analyze the audio info
			auto ptr_stm_info = &g_ddp_program_stream_infos[g_cur_ddp_program_id][(size_t)substreamid + 1];
			ptr_stm_info->stream_coding_type = stream_type;
			ptr_stm_info->audio_info.bits_per_sample = 16;
			ptr_stm_info->audio_info.sample_frequency = getfs(fscod, fscod2);

			if (chanmape)
			{
				CH_MAPPING ddp_channel_mapping;
				for (size_t i = 0; i < _countof(ddp_ch_assignment); i++)
					if ((1 << (15 - i))&chanmap)
						ddp_channel_mapping.u64Val |= ddp_ch_assignment[i].u64Val;
				ptr_stm_info->audio_info.channel_mapping = ddp_channel_mapping;
			}
			else
				ptr_stm_info->audio_info.channel_mapping = acmod_ch_assignments[acmod];
			if (lfeon)
				ptr_stm_info->audio_info.channel_mapping.LFE = 1;
		}
		else
		{
			// Nothing to do
			goto done;
		}
	}

done:
	return iRet;
}

int ParseMLPAU(unsigned short PID, int stream_type, unsigned long sync_code, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	if (sync_code != FBA_SYNC_CODE && sync_code != FBB_SYNC_CODE)
		return -1;

	unsigned char check_nibble = (pBuf[0] & 0xF0) >> 4;
	unsigned short access_unit_length = ((pBuf[0] << 8) | pBuf[1]) & 0xFFF;

	audio_info.stream_coding_type = stream_type;

	unsigned char audio_sampling_frequency = 0;
	if (sync_code == FBA_SYNC_CODE)
	{
		audio_info.audio_info.bits_per_sample = 24;

		audio_sampling_frequency = (pBuf[8] >> 4) & 0xF;

		unsigned char channel_assignment_6ch_presentation = ((pBuf[9] & 0x0F) << 1) | ((pBuf[10] >> 7) & 0x01);
		unsigned char channel_assignment_8ch_presentation = ((pBuf[10] & 0x1F) << 8) | pBuf[11];

		audio_info.audio_info.channel_mapping.clear();
		audio_info.audio_info.channel_mapping.cat = CH_MAPPING_CAT_DCINEMA;
		unsigned short flags = (pBuf[14] << 8) | pBuf[15];
		if ((flags & 0x8000))
		{
			for (size_t i = 0; i < _countof(FBA_Channel_Loc_mapping_1); i++)
				if (channel_assignment_8ch_presentation&(1<<i))
					audio_info.audio_info.channel_mapping.u64Val |= FBA_Channel_Loc_mapping_1[i].u64Val;
		}
		else
		{
			for (size_t i = 0; i < _countof(FBA_Channel_Loc_mapping_0); i++)
				if (channel_assignment_8ch_presentation & (1 << i))
					audio_info.audio_info.channel_mapping.u64Val |= FBA_Channel_Loc_mapping_0[i].u64Val;
		}
	}
	else if (sync_code == FBB_SYNC_CODE)
	{
		unsigned char quantization_word_length_1 = (pBuf[8] >> 4);
		unsigned char audio_sampling_frequency_1 = (pBuf[9] >> 4) & 0xF;
		audio_sampling_frequency = audio_sampling_frequency_1;

		audio_info.audio_info.channel_mapping.clear();
		audio_info.audio_info.channel_mapping.cat = CH_MAPPING_CAT_DCINEMA;

		unsigned char channel_assignment = pBuf[11] & 0x1F;
		if (channel_assignment < sizeof(FBB_Channel_assignments) / sizeof(FBB_Channel_assignments[0]))
			audio_info.audio_info.channel_mapping = FBB_Channel_assignments[channel_assignment];
		else
			audio_info.audio_info.channel_mapping.clear();	// Not support
		
		audio_info.audio_info.bits_per_sample = quantization_word_length_1 == 0 ? 16 : (quantization_word_length_1 == 1 ? 20 : (quantization_word_length_1 == 2 ? 24 : 0));
	}

	switch (audio_sampling_frequency)
	{
	case 0x0: audio_info.audio_info.sample_frequency = 48000; break;
	case 0x1: audio_info.audio_info.sample_frequency = 96000; break;
	case 0x2: audio_info.audio_info.sample_frequency = 192000; break;
	case 0x8: audio_info.audio_info.sample_frequency = 44100; break;
	case 0x9: audio_info.audio_info.sample_frequency = 88200; break;
	case 0xA: audio_info.audio_info.sample_frequency = 176400; break;
	default:
		audio_info.audio_info.sample_frequency = (uint32_t)-1;
	}

	return 0;
}

int ParseCoreDTSAU(unsigned short PID, int stream_type, unsigned long sync_code, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	int iRet = -1;
	if (cbSize < 0)
		return -1;

	CBitstream bst(pBuf, (size_t)cbSize << 3);

	try
	{
		bst.SkipBits(32);
		uint8_t FTYPE = (uint8_t)bst.GetBits(1);

		if (FTYPE == 0)
			return -1;

		uint8_t SHORT = (uint8_t)bst.GetBits(5);
		uint8_t CPF = (uint8_t)bst.GetBits(1);
		uint8_t NBLKS = (uint8_t)bst.GetBits(7);
		if (NBLKS <= 4)
			return -1;

		uint16_t FSIZE = (uint16_t)bst.GetBits(14);
		if (FSIZE <= 94)
			return -1;

		uint8_t AMODE = (uint8_t)bst.GetBits(6);
		uint8_t SFREQ = (uint8_t)bst.GetBits(4);
		uint8_t RATE = (uint8_t)bst.GetBits(5);
		uint8_t FixedBit = (uint8_t)bst.GetBits(1);

		if (FixedBit != 0)
			return -1;

		uint8_t DYNF = (uint8_t)bst.GetBits(1);
		uint8_t TIMEF = (uint8_t)bst.GetBits(1);
		uint8_t AUXF = (uint8_t)bst.GetBits(1);
		uint8_t HDCD = (uint8_t)bst.GetBits(1);
		uint8_t EXT_AUDIO_ID = (uint8_t)bst.GetBits(3);
		uint8_t EXT_AUDIO = (uint8_t)bst.GetBits(1);
		uint8_t ASPF = (uint8_t)bst.GetBits(1);
		uint8_t LFF = (uint8_t)bst.GetBits(2);
		uint8_t HFLAG = (uint8_t)bst.GetBits(1);
		uint16_t HCRC;
		if (CPF == 1) // Present
			HCRC = (uint16_t)bst.GetBits(16);
		uint8_t FILTS = (uint8_t)bst.GetBits(1);
		uint8_t VERNUM = (uint8_t)bst.GetBits(4);
		uint8_t CHIST = (uint8_t)bst.GetBits(2);
		uint8_t PCMR = (uint8_t)bst.GetBits(3);
		uint8_t SUMF = (uint8_t)bst.GetBits(1);
		uint8_t SUMS = (uint8_t)bst.GetBits(1);

		static uint32_t SFREQs[] = {
			0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0, 12000, 24000, 48000, 0, 0 };

		static uint32_t RATEs[] = {
			32000, 56000, 64000, 96000, 112000, 128000, 192000, 224000, 256000, 320000, 384000, 448000, 512000, 576000, 640000,
			768000, 960000, 1024000, 1152000, 1280000, 1344000, 1408000, 1411200, 1472000, 1536000, 1920000, 2048000, 3072000, 3840000,
			(uint32_t)-3,	// Open
			(uint32_t)-2,	// Variant
			(uint32_t)-1	// Lossless
		};
		static uint8_t bpses[] = {
			16, 16, 20, 20, 24, 24, 0, 0
		};

		// Organize the information.
		audio_info.stream_coding_type = stream_type;
		audio_info.audio_info.channel_mapping = dts_audio_channel_arragements[AMODE];

		if (LFF)
			audio_info.audio_info.channel_mapping.DTS.LFE1 = 1;

		audio_info.audio_info.sample_frequency = SFREQs[SFREQ];
		audio_info.audio_info.bits_per_sample = bpses[PCMR];
		audio_info.audio_info.bitrate = RATEs[RATE];
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

enum DTS_EXTSUBSTREAM
{

	DTS_CORESUBSTREAM_CORE = 0x00000001,
	DTS_BCCORE_XXCH = 0x00000002,
	DTS_BCCORE_X96 = 0x00000004,
	DTS_BCCORE_XCH = 0x00000008,
	DTS_EXSUBSTREAM_CORE = 0x00000010,
	DTS_EXSUBSTREAM_XBR = 0x00000020,
	DTS_EXSUBSTREAM_XXCH = 0x00000040,
	DTS_EXSUBSTREAM_X96 = 0x00000080,
	DTS_EXSUBSTREAM_LBR = 0x00000100,
	DTS_EXSUBSTREAM_XLL = 0x00000200,
	DTS_EXSUBSTREAM_AUX1 = 0x00000400,
	DTS_EXSUBSTREAM_AUX2 = 0x00000800,
};

int ParseDTSExSSAU(unsigned short PID, int stream_type, unsigned long sync_code, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	auto NumSpkrTableLookUp = [&](uint32_t nSpkrMask)
	{
		unsigned int i, nTotalChs;

		if (nSpkrMask == 0)
			return 0;

		nTotalChs = 0;
		for (i = 0; i < _countof(dtshd_speaker_bitmask_table); i++)
		{
			if ((1 << i)& nSpkrMask)
				nTotalChs += std::get<3>(dtshd_speaker_bitmask_table[i]);
		}
		return (int)nTotalChs;
	};

	auto CountBitsSet_to_1 = [](uint8_t bitcount, uint32_t val)
	{
		uint8_t nRet = 0;
		for (uint8_t i = 0; i < bitcount; i++)
		{
			if (val & (1 << i))
				nRet++;
		}
		return nRet;
	};

	if (cbSize < 0)
		return -1;

	CBitstream bst(pBuf, (size_t)cbSize << 3);
	bst.SkipBits(32);

	uint8_t UserDefinedBits = (uint8_t)bst.GetBits(8); DBG_UNREFERENCED_LOCAL_VARIABLE(UserDefinedBits);
	uint8_t nExtSSIndex = (uint8_t)bst.GetBits(2);

	uint8_t	bHeaderSizeType = (uint8_t)bst.GetBits(1);
	uint8_t nuBits4Header = bHeaderSizeType ? 12 : 8;
	uint8_t nuBits4ExSSFsize = bHeaderSizeType ? 20 : 16;

	uint32_t nuExtSSHeaderSize, nuExtSSFsize;
	bst.GetBits(nuBits4Header, nuExtSSHeaderSize);
	bst.GetBits(nuBits4ExSSFsize, nuExtSSFsize);
	nuExtSSHeaderSize++; nuExtSSFsize++;

	uint8_t bStaticFieldsPresent;
	bst.GetBits(1, bStaticFieldsPresent);

	STREAM_INFO stm_info(stream_type);

	uint8_t nuNumAudioPresnt = 1, nuNumAssets = 1, bMixMetadataEnbl = 0;
	uint8_t nuNumMixOutConfigs = 0;
	uint8_t nuMixOutChMask[4];
	uint8_t nNumMixOutCh[4];
	uint16_t nuExSSFrameDurationCode;
	if (bStaticFieldsPresent)
	{
		uint8_t nuRefClockCode, bTimeStampFlag;

		bst.GetBits(2, nuRefClockCode);
		bst.GetBits(3, nuExSSFrameDurationCode);
		nuExSSFrameDurationCode = 512 * (nuExSSFrameDurationCode + 1);
		bst.GetBits(1, bTimeStampFlag);

		if (bTimeStampFlag)
			bst.SkipBits(36);

		bst.GetBits(3, nuNumAudioPresnt); nuNumAudioPresnt++;
		bst.GetBits(3, nuNumAssets); nuNumAssets++;

		uint8_t nuActiveExSSMask[8];
		uint8_t nuActiveAssetMask[8][4];
		for (uint8_t nAuPr = 0; nAuPr < nuNumAudioPresnt; nAuPr++)
			bst.GetBits(nExtSSIndex + 1, nuActiveExSSMask[nAuPr]);

		for (uint8_t nAuPr = 0; nAuPr < nuNumAudioPresnt; nAuPr++) {
			for (uint8_t nSS = 0; nSS < nExtSSIndex + 1; nSS++) {
				if (((nuActiveExSSMask[nAuPr] >> nSS) & 0x1) == 1)
					nuActiveAssetMask[nAuPr][nSS] = (uint8_t)bst.GetBits(8);
				else
					nuActiveAssetMask[nAuPr][nSS] = 0;
			}
		}

		bMixMetadataEnbl = (uint8_t)bst.GetBits(1);
		if (bMixMetadataEnbl)
		{
			uint8_t nuMixMetadataAdjLevel = (uint8_t)bst.GetBits(2);
			uint8_t nuBits4MixOutMask = (uint8_t)(bst.GetBits(2) + 1) << 2;
			nuNumMixOutConfigs = (uint8_t)bst.GetBits(2) + 1;
			// Output Mixing Configuration Loop
			for (uint8_t ns = 0; ns < nuNumMixOutConfigs; ns++) {
				nuMixOutChMask[ns] = (uint8_t)bst.GetBits(nuBits4MixOutMask);
				nNumMixOutCh[ns] = NumSpkrTableLookUp(nuMixOutChMask[ns]);
			}
		}
	}
	else
		return -1;	// Can't get audio information if bStaticFieldPresent is false;

	uint32_t nuAssetFsize[8] = { 0 };
	for (int nAst = 0; nAst < nuNumAssets; nAst++)
		nuAssetFsize[nAst] = (uint16_t)bst.GetBits(nuBits4ExSSFsize) + 1;

	for (int nAst = 0; nAst < nuNumAssets; nAst++)
	{
		uint8_t nuAssetIndex;
		uint16_t nuAssetDescriptFsize;
		uint8_t nuBitResolution = 0, nuMaxSampleRate = 0, nuTotalNumChs = 0, bOne2OneMapChannels2Speakers = 0;

		uint64_t asset_start_bitpos = bst.Tell();

		bst.GetBits(9, nuAssetDescriptFsize); nuAssetDescriptFsize++;
		bst.GetBits(3, nuAssetIndex);

		uint8_t bEmbeddedStereoFlag = 0, bEmbeddedSixChFlag = 0, nuRepresentationType = 0;
		uint8_t nuMainAudioScaleCode[4][32];

		if (bStaticFieldsPresent)
		{
			uint8_t bAssetTypeDescrPresent = (uint8_t)bst.GetBits(1);
			uint8_t nuAssetTypeDescriptor = 0;
			if (bAssetTypeDescrPresent)
				nuAssetTypeDescriptor = (uint8_t)bst.GetBits(4);
			uint8_t bLanguageDescrPresent = (uint8_t)bst.GetBits(1);
			uint32_t LanguageDescriptor;
			if (bLanguageDescrPresent)
				LanguageDescriptor = (uint32_t)bst.GetBits(24);
			uint8_t bInfoTextPresent = (uint8_t)bst.GetBits(1);
			uint16_t nuInfoTextByteSize;
			if (bInfoTextPresent)
				nuInfoTextByteSize = (uint16_t)bst.GetBits(10) + 1;

			if (bInfoTextPresent)
				bst.SkipBits((int64_t)nuInfoTextByteSize * 8);
			nuBitResolution = (uint8_t)bst.GetBits(5) + 1;
			nuMaxSampleRate = (uint8_t)bst.GetBits(4);
			nuTotalNumChs = (uint8_t)bst.GetBits(8) + 1;
			bOne2OneMapChannels2Speakers = (uint8_t)bst.GetBits(1);

			uint32_t nuSpkrActivityMask = 0;
			if (bOne2OneMapChannels2Speakers)
			{
				uint8_t bSpkrMaskEnabled = 0, nuNumBits4SAMask = 0;
				if (nuTotalNumChs>2)
					bEmbeddedStereoFlag = (uint8_t)bst.GetBits(1);

				if (nuTotalNumChs>6)
					bEmbeddedSixChFlag = (uint8_t)bst.GetBits(1);

				bSpkrMaskEnabled = (uint8_t)bst.GetBits(1);
				if (bSpkrMaskEnabled)
					nuNumBits4SAMask = (uint8_t)(bst.GetBits(2) + 1) << 2;

				if (bSpkrMaskEnabled)
					nuSpkrActivityMask = (uint32_t)bst.GetBits(nuNumBits4SAMask);
				uint8_t nuNumSpkrRemapSets = (uint8_t)bst.GetBits(3);
				uint32_t nuStndrSpkrLayoutMask[8];
				uint8_t nuNumDecCh4Remap[8];
				uint32_t nuRemapDecChMask[8][32];
				uint8_t nuSpkrRemapCodes[8][32][32];
				for (uint8_t ns = 0; ns<nuNumSpkrRemapSets; ns++)
					nuStndrSpkrLayoutMask[ns] = (uint8_t)bst.GetBits(nuNumBits4SAMask);
				for (uint8_t ns = 0; ns<nuNumSpkrRemapSets; ns++) {
					uint8_t nuNumSpeakers = NumSpkrTableLookUp(nuStndrSpkrLayoutMask[ns]);
					nuNumDecCh4Remap[ns] = (uint8_t)bst.GetBits(5) + 1;
					for (int nCh = 0; nCh<nuNumSpeakers; nCh++) { // Output channel loop
						nuRemapDecChMask[ns][nCh] = (uint32_t)bst.GetBits(nuNumDecCh4Remap[ns]);
						uint8_t nCoefs = CountBitsSet_to_1(nuNumDecCh4Remap[ns], nuRemapDecChMask[ns][nCh]);
						for (uint8_t nc = 0; nc<nCoefs; nc++)
							nuSpkrRemapCodes[ns][nCh][nc] = (uint8_t)bst.GetBits(5);
					} // End output channel loop
				} // End nuNumSpkrRemapSets loop
			}
			else
			{
				// No speaker feed case
				bEmbeddedStereoFlag = 0;
				bEmbeddedSixChFlag = 0;
				nuRepresentationType = (uint8_t)bst.GetBits(3);
			}


			//
			// Update stm_info of current audio asset
			// It is not so accurate, later we may get the audio information order by audio present defined in each extension stream header
			//
			if (stm_info.audio_info.bits_per_sample < nuBitResolution)
				stm_info.audio_info.bits_per_sample = nuBitResolution;

			static uint32_t ExSS_Asset_Max_SampleRates[] = {
				8000,16000,32000,64000,128000,
				22050,44100,88200,176400,352800,
				12000,24000,48000,96000,192000,384000
			};

			if (stm_info.audio_info.sample_frequency < ExSS_Asset_Max_SampleRates[nuMaxSampleRate])
				stm_info.audio_info.sample_frequency = ExSS_Asset_Max_SampleRates[nuMaxSampleRate];

			// Update the channel information
			if (bOne2OneMapChannels2Speakers)
			{
				for (size_t i = 0; i < _countof(dtshd_speaker_bitmask_table); i++)
				{
					if ((1<<i)&nuSpkrActivityMask)
						stm_info.audio_info.channel_mapping.u64Val |= std::get<2>(dtshd_speaker_bitmask_table[i]).u64Val;
				}
			}
			else
			{
				// Can't process it at present.
				// TODO...
			}
		} // End of if (bStaticFieldsPresent)

		uint64_t asset_cur_bitpos = bst.Tell();

		uint8_t nuDRCCode, nuDialNormCode, nuDRC2ChDmixCode, bMixMetadataPresent;
		uint8_t bDRCCoefPresent = (uint8_t)bst.GetBits(1);
		if (bDRCCoefPresent)
			nuDRCCode = (uint8_t)bst.GetBits(8);
		uint8_t bDialNormPresent = (uint8_t)bst.GetBits(1);
		if (bDialNormPresent)
			nuDialNormCode = (uint8_t)bst.GetBits(5);
		if (bDRCCoefPresent && bEmbeddedStereoFlag)
			nuDRC2ChDmixCode = (uint8_t)bst.GetBits(8);
		if (bMixMetadataEnbl)
			bMixMetadataPresent = (uint8_t)bst.GetBits(1);
		else
			bMixMetadataPresent = 0;

		if (bMixMetadataPresent) {
			uint8_t bExternalMixFlag = (uint8_t)bst.GetBits(1);
			uint8_t nuPostMixGainAdjCode = (uint8_t)bst.GetBits(6);
			uint8_t nuControlMixerDRC = (uint8_t)bst.GetBits(2);
			if (nuControlMixerDRC <3)
				uint8_t nuLimit4EmbeddedDRC = (uint8_t)bst.GetBits(3);
			if (nuControlMixerDRC == 3)
				uint8_t nuCustomDRCCode = (uint8_t)bst.GetBits(8);
			uint8_t bEnblPerChMainAudioScale = (uint8_t)bst.GetBits(1);
			for (uint8_t ns = 0; ns<nuNumMixOutConfigs; ns++) {
				if (bEnblPerChMainAudioScale) {
					for (uint8_t nCh = 0; nCh<nNumMixOutCh[ns]; nCh++)
						nuMainAudioScaleCode[ns][nCh] = (uint8_t)bst.GetBits(6);
				}
				else
					nuMainAudioScaleCode[ns][0] = (uint8_t)bst.GetBits(6);
			}
			uint8_t nEmDM = 1;
			uint8_t nDecCh[2];
			nDecCh[0] = nuTotalNumChs;
			if (bEmbeddedSixChFlag) {
				nDecCh[nEmDM] = 6;
				nEmDM = nEmDM + 1;
			}
			if (bEmbeddedStereoFlag) {
				nDecCh[nEmDM] = 2;
				nEmDM = nEmDM + 1;
			}
			uint8_t nuMixMapMask[4][2][6];
			uint8_t nuNumMixCoefs[4][2][6];
			uint8_t nuMixCoeffs[4][2][6][32];
			for (uint8_t ns = 0; ns<nuNumMixOutConfigs; ns++) { //Configuration Loop
				for (uint8_t nE = 0; nE<nEmDM; nE++) { // Embedded downmix loop
					for (uint8_t nCh = 0; nCh<nDecCh[nE]; nCh++) { //Supplemental Channel Loop
						nuMixMapMask[ns][nE][nCh] = (uint8_t)bst.GetBits(nNumMixOutCh[ns]);
						nuNumMixCoefs[ns][nE][nCh] = CountBitsSet_to_1(nNumMixOutCh[ns], nuMixMapMask[ns][nE][nCh]);
						for (uint8_t nC = 0; nC<nuNumMixCoefs[ns][nE][nCh]; nC++)
							nuMixCoeffs[ns][nE][nCh][nC] = (uint8_t)bst.GetBits(6);
					} // End supplemental channel loop
				} // End of Embedded downmix loop
			} // End configuration loop
		} // End if (bMixMetadataPresent)

		uint16_t nuCoreExtensionMask = 0;
		uint8_t nuCodingMode = (uint8_t)bst.GetBits(2);
		switch (nuCodingMode) {
		case 0:
		{
			nuCoreExtensionMask = (uint16_t)bst.GetBits(12);
			if (nuCoreExtensionMask & DTS_EXSUBSTREAM_CORE) {
				uint16_t nuExSSCoreFsize = (uint16_t)bst.GetBits(14) + 1;
				uint8_t bExSSCoreSyncPresent = (uint8_t)bst.GetBits(1);
				uint8_t nuExSSCoreSyncDistInFrames = 0;
				if (bExSSCoreSyncPresent)
					nuExSSCoreSyncDistInFrames = (uint8_t)(1 << (bst.GetBits(2)));
			}
			uint16_t nuExSSXBRFsize, nuExSSXXCHFsize, nuExSSX96Fsize, nuExSSLBRFsize;
			uint8_t bExSSLBRSyncPresent, nuExSSLBRSyncDistInFrames;
			if (nuCoreExtensionMask & DTS_EXSUBSTREAM_XBR)
				nuExSSXBRFsize = (uint16_t)bst.GetBits(14) + 1;
			if (nuCoreExtensionMask & DTS_EXSUBSTREAM_XXCH)
				nuExSSXXCHFsize = (uint16_t)bst.GetBits(14) + 1;
			if (nuCoreExtensionMask & DTS_EXSUBSTREAM_X96)
				nuExSSX96Fsize = (uint16_t)bst.GetBits(12) + 1;
			if (nuCoreExtensionMask & DTS_EXSUBSTREAM_LBR) {
				nuExSSLBRFsize = (uint16_t)bst.GetBits(14) + 1;
				bExSSLBRSyncPresent = (uint8_t)bst.GetBits(1);
				if (bExSSLBRSyncPresent)
					nuExSSLBRSyncDistInFrames = (uint8_t)(1 << (bst.GetBits(2)));
			}
			if (nuCoreExtensionMask & DTS_EXSUBSTREAM_XLL) {
				uint32_t nuExSSXLLFsize = (uint32_t)bst.GetBits(nuBits4ExSSFsize) + 1;
				uint8_t bExSSXLLSyncPresent = (uint8_t)bst.GetBits(1);
				if (bExSSXLLSyncPresent) {
					uint16_t nuPeakBRCntrlBuffSzkB = (uint16_t)bst.GetBits(4) << 4;
					uint8_t nuBitsInitDecDly = (uint8_t)bst.GetBits(5) + 1;
					uint32_t nuInitLLDecDlyFrames = (uint32_t)bst.GetBits(nuBitsInitDecDly);
					uint32_t nuExSSXLLSyncOffset = (uint32_t)bst.GetBits(nuBits4ExSSFsize);
				}
			}
		}
		break;
		case 1:
		{
			uint32_t nuExSSXLLFsize = (uint32_t)bst.GetBits(nuBits4ExSSFsize) + 1;
			uint8_t bExSSXLLSyncPresent = (uint8_t)bst.GetBits(1);
			if (bExSSXLLSyncPresent) {
				uint16_t nuPeakBRCntrlBuffSzkB = (uint16_t)(bst.GetBits(4) << 4);
				uint8_t nuBitsInitDecDly = (uint8_t)bst.GetBits(5) + 1;
				uint32_t nuInitLLDecDlyFrames = (uint32_t)bst.GetBits(nuBitsInitDecDly);
				uint32_t nuExSSXLLSyncOffset = (uint32_t)bst.GetBits(nuBits4ExSSFsize);
			}
		}
		break;
		case 2:
		{
			uint16_t nuExSSLBRFsize = (uint16_t)bst.GetBits(14) + 1;
			uint8_t bExSSLBRSyncPresent = (uint8_t)bst.GetBits(1);
			uint8_t nuExSSLBRSyncDistInFrames = 0;
			if (bExSSLBRSyncPresent)
				nuExSSLBRSyncDistInFrames = (uint8_t)(1 << (bst.GetBits(2)));
		}
		break;
		case 3:
		{
			uint16_t nuExSSAuxFsize = (uint16_t)bst.GetBits(14) + 1;
			uint8_t nuAuxCodecID = (uint8_t)bst.GetBits(8);
			uint8_t bExSSAuxSyncPresent = (uint8_t)bst.GetBits(1);
			uint8_t nuExSSAuxSyncDistInFrames = 0;
			if (bExSSAuxSyncPresent)
				nuExSSAuxSyncDistInFrames = (uint8_t)bst.GetBits(3) + 1;
		}
		break;
		default:
			break;
		}

		asset_cur_bitpos = bst.Tell();

		uint8_t nuDTSHDStreamID;
		if (((nuCodingMode == 0) && (nuCoreExtensionMask & DTS_EXSUBSTREAM_XLL)) || (nuCodingMode == 1)) {
			nuDTSHDStreamID = (uint8_t)bst.GetBits(3);
		}
		uint8_t bOnetoOneMixingFlag = 0, bEnblPerChMainAudioScale = 0;
		if (bOne2OneMapChannels2Speakers == 1 && bMixMetadataEnbl == 1 && bMixMetadataPresent == 0)
			bOnetoOneMixingFlag = (uint8_t)bst.GetBits(1);
		if (bOnetoOneMixingFlag) {
			bEnblPerChMainAudioScale = (uint8_t)bst.GetBits(1);
			for (uint8_t ns = 0; ns<nuNumMixOutConfigs; ns++) {
				if (bEnblPerChMainAudioScale) {
					for (uint8_t nCh = 0; nCh<nNumMixOutCh[ns]; nCh++)
						nuMainAudioScaleCode[ns][nCh] = (uint8_t)bst.GetBits(6);
				}
				else
					nuMainAudioScaleCode[ns][0] = (uint8_t)bst.GetBits(6);
			}
		} // End of bOnetoOneMixingFlag==true condition
		uint8_t bDecodeAssetInSecondaryDecoder = (uint8_t)bst.GetBits(1); DBG_UNREFERENCED_LOCAL_VARIABLE(bDecodeAssetInSecondaryDecoder);

		asset_cur_bitpos = bst.Tell();

		if (asset_cur_bitpos - asset_start_bitpos < ((uint64_t)nuAssetDescriptFsize << 3))
		{
			bool bDRCMetadataRev2Present = (bst.GetBits(1) == 1) ? true : false;
			if (bDRCMetadataRev2Present == true)
			{
				uint8_t DRCversion_Rev2 = (uint8_t)bst.GetBits(4);
				// one DRC value for each block of 256 samples
				uint8_t nRev2_DRCs = (uint8_t)(nuExSSFrameDurationCode / 256);
				// assumes DRCversion_Rev2 == 1:
				//uint8_t DRCCoeff_Rev2[16];
				for (uint8_t subSubFrame = 0; subSubFrame < nRev2_DRCs; subSubFrame++)
				{
					//DRCCoeff_Rev2[subSubFrame] = dts_dynrng_to_db(bst.GetBits(8));
					bst.SkipBits(8);
				}
			}

			asset_cur_bitpos = bst.Tell();
		}
	}	// End for (int nAst = 0; nAst < nuNumAssets; nAst++)

	if (stm_info.audio_info.bitrate == 0 &&
		stm_info.audio_info.bits_per_sample == 0 &&
		stm_info.audio_info.channel_mapping.is_invalid() &&
		stm_info.audio_info.sample_frequency == 0)
		return -1;

	audio_info = stm_info;

	return 0;
}

int ParseCoreDTSHDAU(unsigned short PID, int stream_type, unsigned long sync_code, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	int iRet = -1;

	if (sync_code == DTS_SYNCWORD_CORE)
	{
		// If Core stream is hit, generate the audio information
		if (g_cur_dtshd_stream_infos.size() > 0)
		{
			if (g_cur_dtshd_stream_infos.size() == 1)
				audio_info = g_cur_dtshd_stream_infos.front();
			else
				audio_info = g_cur_dtshd_stream_infos.back();

			g_cur_dtshd_stream_infos.clear();
			iRet = 0;
		}
	}


	try
	{
		int iRet2 = -1;
		if (sync_code == DTS_SYNCWORD_CORE)
		{
			g_cur_dtshd_stream_infos.emplace_back();
			iRet2 = ParseCoreDTSAU(PID, stream_type, sync_code, pBuf, cbSize, g_cur_dtshd_stream_infos.back());
		}
		else if (sync_code == DTS_SYNCWORD_SUBSTREAM)
		{
			g_cur_dtshd_stream_infos.emplace_back();
			iRet2 = ParseDTSExSSAU(PID, stream_type, sync_code, pBuf, cbSize, g_cur_dtshd_stream_infos.back());
		}

		// The DTS-HD full AU is not complete, reset the previous parse information
		// The first stm_info in g_cur_dtshd_stream_infos should be core DTS
		if (iRet2 < 0)
			g_cur_dtshd_stream_infos.clear();
	}
	catch (...)
	{
		return iRet;
	}

	return iRet;
}

#define ID_SCE 0x0
#define ID_CPE 0x1
#define ID_CCE 0x2
#define ID_LFE 0x3
#define ID_DSE 0x4
#define ID_PCE 0x5
#define ID_FIL 0x6
#define ID_END 0x7

int ParseADTSFrame(unsigned short PID, int stream_type, unsigned long sync_code, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	int iRet = -1;
	if (cbSize < 0)
		return -1;

	try
	{
		CBitstream bst(pBuf, (size_t)cbSize << 3);

		bst.SkipBits(12);

		/*
		adts_fixed_header
		*/
		uint8_t ID;
		uint8_t layer;
		uint8_t protection_absent;
		uint8_t profile;
		uint8_t sampling_frequency_index;
		uint8_t private_bit;
		uint8_t channel_configuration;
		uint8_t original_copy;
		uint8_t home;

		bst.GetBits(1, ID);
		bst.GetBits(2, layer);
		bst.GetBits(1, protection_absent);
		bst.GetBits(2, profile);
		bst.GetBits(4, sampling_frequency_index);
		bst.GetBits(1, private_bit);
		bst.GetBits(3, channel_configuration);
		bst.GetBits(1, original_copy);
		bst.GetBits(1, home);

		/*
		adts_variable_header
		*/
		uint8_t copyright_identification_bit;
		uint8_t copyright_identification_start;
		uint16_t frame_length;
		uint16_t adts_buffer_fullness;
		uint8_t number_of_raw_data_blocks_in_frame;

		bst.GetBits(1, copyright_identification_bit);
		bst.GetBits(1, copyright_identification_start);
		bst.GetBits(13, frame_length);
		bst.GetBits(11, adts_buffer_fullness);
		bst.GetBits(2, number_of_raw_data_blocks_in_frame);

		static uint32_t sampling_frequencies[] = {
			96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000
		};

		audio_info.stream_coding_type = stream_type;

		audio_info.audio_info.bits_per_sample = 16;
		audio_info.audio_info.sample_frequency = sampling_frequencies[sampling_frequency_index];
		audio_info.audio_info.bitrate = 0;

		if (channel_configuration == 0)
		{
			/*
			If channel_configuration is greater than 0, the channel configuration is given by the Default
			bitstream index number in Table 42, see subclause 8.5. If channel_configuration equals 0,
			the channel configuration is not specified in the header and must be given by a program_config_element()
			following as first syntactic element in the first raw_data_block() after the header, or by the implicit
			configuration (see subclause 8.5) or must be known in the application (Table 8).
			*/
			uint16_t crc_check;
			if (number_of_raw_data_blocks_in_frame == 0)
			{
				// Skip adts_error_check();
				if (!protection_absent)
					bst.GetBits(16, crc_check);
			}
			else
			{
				uint16_t raw_data_block_position[4];
				// Skip adts_header_error_check();
				if (!protection_absent)
				{
					for (int i = 1; i <= number_of_raw_data_blocks_in_frame; i++)
						bst.GetBits(16, raw_data_block_position[i]);
					bst.GetBits(16, crc_check);
				}
			}

			uint8_t id_sync_ele;
			bst.GetBits(3, id_sync_ele);
			if (id_sync_ele != ID_PCE)
				return -1;
			else
			{
				BST::AACAudio::PROGRAM_CONFIG_ELEMENT pce;
				if (AMP_SUCCEEDED(pce.Unpack(bst)))
				{
					audio_info.audio_info.channel_mapping = pce.GetChannelMapping();
					iRet = 0;
				}
			}
		}
		else
		{
			audio_info.audio_info.channel_mapping = aac_channel_configurations[channel_configuration];
			iRet = 0;
		}
	}
	catch (...)
	{
		iRet = -1;
	}

	return iRet;
}

int ParseMPEGAudioFrame(unsigned short PID, int stream_type, unsigned long sync_code, unsigned char* pBuf, int cbSize, STREAM_INFO& audio_info)
{
	int iRet = -1;
	uint8_t MPEG_Audio_VerID;
	uint8_t LayerID;
	uint8_t protection;
	uint8_t bitrate_index;
	uint8_t sampling_rate_frequency_index;
	uint8_t padding;
	uint8_t private_bit;
	uint8_t channel_mode;
	uint8_t mode_extension;
	uint8_t copyright;
	uint8_t original;
	uint8_t emphasis;

	static uint32_t bitrate_tab[16][2][3] = {
		{ { 0, 0, 0 },{ 0, 0, 0 } },
		{ { 32000, 32000, 32000 },{ 32000, 8000, 8000 } },
		{ { 64000, 48000, 40000 },{ 48000, 16000, 16000 } },
		{ { 96000, 56000, 48000 },{ 56000, 24000, 24000 } },
		{ { 128000, 64000, 56000 },{ 64000, 32000, 32000 } },
		{ { 160000, 80000, 64000 },{ 80000, 40000, 40000 } },
		{ { 192000, 96000, 80000 },{ 96000, 48000, 48000 } },
		{ { 224000, 112000, 96000 },{ 112000, 56000, 56000 } },
		{ { 256000, 128000, 112000 },{ 128000, 64000, 64000 } },
		{ { 288000, 160000, 128000 },{ 144000, 80000, 80000 } },
		{ { 320000, 192000, 160000 },{ 160000, 96000, 96000 } },
		{ { 352000, 224000, 192000 },{ 176000, 112000, 112000 } },
		{ { 384000, 256000, 224000 },{ 192000, 128000, 128000 } },
		{ { 416000, 320000, 256000 },{ 224000, 144000, 144000 } },
		{ { 448000, 384000, 320000 },{ 256000, 160000, 160000 } },
		{ { 0, 0, 0 },{ 0, 0, 0 } }
	};

	static uint32_t Sampling_rates[4][3] = {
		{44100, 22050, 11025},
		{48000, 24000, 12000},
		{32000, 16000, 8000},
		{0, 0, 0}
	};

	if (cbSize < 0)
		return -1;

	try
	{
		CBitstream bst(pBuf, (size_t)cbSize << 3);

		bst.SkipBits(11);
		bst.GetBits(2, MPEG_Audio_VerID);
		bst.GetBits(2, LayerID);
		bst.GetBits(1, protection);
		bst.GetBits(4, bitrate_index);
		bst.GetBits(2, sampling_rate_frequency_index);
		bst.GetBits(1, padding);
		bst.GetBits(1, private_bit);
		bst.GetBits(2, channel_mode);
		bst.GetBits(2, mode_extension);
		bst.GetBits(1, copyright);
		bst.GetBits(1, original);
		bst.GetBits(2, emphasis);

		audio_info.stream_coding_type = stream_type;
		audio_info.audio_info.bitrate = bitrate_tab[bitrate_index][MPEG_Audio_VerID == 3 ? 0 : 1][3 - LayerID];
		audio_info.audio_info.bits_per_sample = 16;
		audio_info.audio_info.sample_frequency = Sampling_rates[sampling_rate_frequency_index][MPEG_Audio_VerID == 3 ? 0 : (MPEG_Audio_VerID == 0 ? 2 : 1)];
		audio_info.audio_info.channel_mapping = mpega_channel_mode_layouts[channel_mode];
		iRet = 0;
	}
	catch (...)
	{
		iRet = -1;
	}

	return iRet;
}

int GetStreamInfoFromMPEG2AU(uint8_t* pAUBuf, size_t cbAUBuf, STREAM_INFO& stm_info)
{
	IMPVContext* pMPVContext = nullptr;
	int iRet = RET_CODE_ERROR;

	if (cbAUBuf >= INT64_MAX)
		return RET_CODE_BUFFER_OVERFLOW;

	CMPEG2VideoParser MPVParser;
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(MPVParser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_IMPVContext, (void**)&pMPVContext)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the MPV context.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	pMPVContext->SetStartCodeFilters({ SEQUENCE_HEADER_CODE, EXTENSION_START_CODE });

	class CMPVEnumerator : public CComUnknown, public IMPVEnumerator
	{
	public:
		CMPVEnumerator(IMPVContext* pCtx)
		: m_pMPVContext(pCtx){
			if (m_pMPVContext)
				m_pMPVContext->AddRef();
		}

		virtual ~CMPVEnumerator() {
			AMP_SAFERELEASE(m_pMPVContext);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_IMPVEnumerator)
				return GetCOMInterface((IMPVEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumVSEQStart(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNewGOP(IUnknown* pCtx, bool closed_gop, bool broken_link) { return RET_CODE_SUCCESS; }
		RET_CODE EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType){return RET_CODE_SUCCESS;}
		RET_CODE EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType){return RET_CODE_SUCCESS;}
		RET_CODE EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
		{
			if (cbBufWithStartCode < 4 || cbBufWithStartCode > INT32_MAX)
				return RET_CODE_NOTHING_TODO;

			AMBst bst = nullptr;
			RET_CODE ret_code = RET_CODE_SUCCESS;
			uint8_t mpv_start_code = pBufWithStartCode[3];
			if (mpv_start_code == EXTENSION_START_CODE && 
				m_pMPVContext->GetCurrentLevel() == 0 && 
				((pBufWithStartCode[4]>>4)&0xFF) == SEQUENCE_DISPLAY_EXTENSION_ID)
			{
				// Try to find sequence_display_extension()
				if ((bst = AMBst_CreateFromBuffer(pBufWithStartCode, (int)cbBufWithStartCode)) == nullptr)
				{
					printf("Failed to create a bitstream object.\n");
					return RET_CODE_ABORT;
				}

				try
				{
					AMBst_SkipBits(bst, 32);
					int left_bits = 0;

					BST::MPEG2Video::CSequenceDisplayExtension* pSeqDispExt =
						new (std::nothrow) BST::MPEG2Video::CSequenceDisplayExtension;
					if (pSeqDispExt == nullptr)
					{
						printf("Failed to create Sequence_Display_Extension.\n");
						ret_code = RET_CODE_ABORT;
						goto done;
					}
					if (AMP_FAILED(ret_code = pSeqDispExt->Map(bst)))
					{
						delete pSeqDispExt;
						printf("Failed to parse the sequence_display_extension() {error code: %d}.\n", ret_code);
						goto done;
					}
					m_sp_sequence_display_extension = std::shared_ptr<BST::MPEG2Video::CSequenceDisplayExtension>(pSeqDispExt);
				}
				catch (AMException& e)
				{
					ret_code = e.RetCode();
				}
			}

		done:
			if (bst != nullptr)
				AMBst_Destroy(bst);
			return ret_code;
		}
		RET_CODE EnumVSEQEnd(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) { return RET_CODE_SUCCESS; }

		IMPVContext*			m_pMPVContext;
		std::shared_ptr<BST::MPEG2Video::CSequenceDisplayExtension>
								m_sp_sequence_display_extension;

	} MPVEnumerator(pMPVContext);

	MPVEnumerator.AddRef();
	MPVParser.SetEnumerator(&MPVEnumerator, MPV_ENUM_OPTION_SE);

	if (AMP_SUCCEEDED(iRet = MPVParser.ParseAUBuf(pAUBuf, cbAUBuf)))
	{
		auto spSeqHdr = pMPVContext->GetSeqHdr();
		auto spSeqExt = pMPVContext->GetSeqExt();
		iRet = GetStreamInfoSeqHdrAndExt(spSeqHdr.get(), spSeqExt.get(), MPVEnumerator.m_sp_sequence_display_extension.get(), stm_info);
	}

	AMP_SAFERELEASE(pMPVContext);

	return iRet;
}

int GetStreamInfoFromMP4AAU(uint8_t* pAUBuf, size_t cbAUBuf, STREAM_INFO& stm_info)
{
	int iRet = RET_CODE_SUCCESS;
	BST::AACAudio::IMP4AACContext* pCtxMP4AAC = nullptr;
	BST::AACAudio::CLOASParser LOASParser;
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(LOASParser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_IMP4AACContext, (void**)&pCtxMP4AAC)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the MPEG4 AAC context.\n");
		return RET_CODE_ERROR;
	}
	AMP_SAFERELEASE(pMSECtx);

	uint32_t options = 0;
	class CLOASEnumerator : public CComUnknown, public ILOASEnumerator
	{
	public:
		CLOASEnumerator(BST::AACAudio::IMP4AACContext* pCtxMP4AAC)
			: m_pCtxMP4AAC(pCtxMP4AAC){
			if (m_pCtxMP4AAC)
				m_pCtxMP4AAC->AddRef();
			memset(audio_specific_config_sha1, 0, sizeof(audio_specific_config_sha1));
		}

		virtual ~CLOASEnumerator() {
			AMP_SAFERELEASE(m_pCtxMP4AAC);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_ILOASEnumerator)
				return GetCOMInterface((ILOASEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumLATMAUBegin(IUnknown* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf){
			//printf("Access-Unit#%" PRIu64 "\n", m_AUCount);
			return RET_CODE_SUCCESS;
		}
		RET_CODE EnumSubFrameBegin(IUnknown* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload){return RET_CODE_SUCCESS;}
		RET_CODE EnumSubFrameEnd(IUnknown* pCtx, uint8_t* pSubFramePayload, size_t cbSubFramePayload){return RET_CODE_SUCCESS;}
		RET_CODE EnumLATMAUEnd(IUnknown* pCtx, uint8_t* pLATMAUBuf, size_t cbLATMAUBuf)
		{
			m_AUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumError(IUnknown* pCtx, RET_CODE error_code)
		{
			printf("Hitting an error {code: %d}!\n", error_code);
			return RET_CODE_SUCCESS;
		}

		BST::AACAudio::IMP4AACContext* m_pCtxMP4AAC = nullptr;
		uint64_t m_AUCount = 0;
		AMSHA1_RET audio_specific_config_sha1[16][8];
	}LOASEnumerator(pCtxMP4AAC);

	LOASEnumerator.AddRef();
	LOASParser.SetEnumerator((IUnknown*)(&LOASEnumerator), options);

	iRet = LOASParser.ProcessAU(pAUBuf, cbAUBuf);

	auto mux_stream_config = pCtxMP4AAC->GetMuxStreamConfig();

	// check whether the SHA1, and judge whether AudioSpecificConfig is changed
	for (int prog = 0; prog < 16; prog++)
	{
		for (int lay = 0; lay < 8; lay++)
		{
			if (mux_stream_config->AudioSpecificConfig[prog][lay] == nullptr)
				continue;

			// Also show the audio frame duration
			auto audio_specific_config = mux_stream_config->AudioSpecificConfig[prog][lay];
			auto audio_object_type = audio_specific_config->GetAudioObjectType();

			int frameLength = 0;	// Unknown
			if (audio_object_type == BST::AACAudio::AAC_main ||
				audio_object_type == BST::AACAudio::AAC_LC ||
				audio_object_type == BST::AACAudio::AAC_SSR ||
				audio_object_type == BST::AACAudio::AAC_LTP ||
				audio_object_type == BST::AACAudio::AAC_Scalable ||
				audio_object_type == BST::AACAudio::TwinVQ ||
				audio_object_type == BST::AACAudio::ER_AAC_LC ||
				audio_object_type == BST::AACAudio::ER_AAC_LTP ||
				audio_object_type == BST::AACAudio::ER_AAC_scalable ||
				audio_object_type == BST::AACAudio::ER_TwinVQ ||
				audio_object_type == BST::AACAudio::ER_BSAC ||
				audio_object_type == BST::AACAudio::ER_AAC_LD)
			{
				int nSamplingRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, -1, -1, -1 };

				if (audio_specific_config->samplingFrequencyIndex == 0xF)
					stm_info.audio_info.sample_frequency = audio_specific_config->samplingFrequency;
				else
					stm_info.audio_info.sample_frequency = nSamplingRates[audio_specific_config->samplingFrequencyIndex];

				stm_info.audio_info.channel_mapping = aac_channel_configurations[audio_specific_config->channelConfiguration];
				stm_info.audio_info.bits_per_sample = 16;
			}

			break;
		}
	}

	AMP_SAFERELEASE(pCtxMP4AAC);
	return iRet;
}

int CheckRawBufferMediaInfo(unsigned short PID, int stream_type, unsigned char* pBuf, int cbSize, int dumpopt, int stream_id, int stream_id_extension)
{
	unsigned char* p = pBuf;
	int cbLeft = cbSize;
	int iParseRet = -1;
	unsigned char audio_program_id = 0;
	STREAM_INFO stm_info;

	if (dumpopt&DUMP_VOB)
	{
		unsigned char stream_id = (PID >> 8) & 0xFF;
		unsigned char sub_stream_id = PID & 0xFF;
		if ((stream_id & 0xF0) == 0xE0)	// Video
		{

		}
		else if (stream_id == 0xBD)
		{

		}
	}

	// At first, check whether the stream type is already decided or not for the current dumped stream.
	if (stream_type == DOLBY_LOSSLESS_AUDIO_STREAM && stream_id_extension != 0x76)
	{
		// Try to analyze the MLP audio information.
		// header size:
		// check_nibble/access_unit_length/input_timing: 4 bytes
		// major_sync_info: at least 28 bytes
		const int min_header_size = 32;
		if (cbSize < min_header_size)
			return -1;

		unsigned long sync_code = (p[4] << 16) | (p[5] << 8) | p[6];
		// For the first round, try to find FBA sync code
		while (cbLeft >= min_header_size)
		{
			sync_code = (sync_code << 8) | p[7];
			if (sync_code == FBA_SYNC_CODE || sync_code == FBB_SYNC_CODE)
			{
				int signature = (p[12] << 8) + p[13];

				if (signature == MLP_SIGNATURE)
				{
					/* found it */
					break;
				}
			}
			
			cbLeft--;
			p++;
		}

		if (cbLeft >= min_header_size && (sync_code == FBA_SYNC_CODE || sync_code == FBB_SYNC_CODE))
		{
			if (ParseMLPAU(PID, stream_type, sync_code, p, cbLeft, stm_info) == 0)
			{
				audio_program_id = 0;
				iParseRet = 0;
			}
		}
	}
	else if (DOLBY_AC3_AUDIO_STREAM == stream_type || DD_PLUS_AUDIO_STREAM == stream_type || (stream_type == DOLBY_LOSSLESS_AUDIO_STREAM && stream_id_extension == 0x76))
	{
		unsigned short sync_code = p[0];
		while (cbLeft >= 8)
		{
			sync_code = (sync_code << 8) | p[1];
			if (sync_code == AC3_SYNCWORD)
				break;

			cbLeft--;
			p++;
		}

		if (cbLeft >= 8)
		{
			if (DOLBY_AC3_AUDIO_STREAM == stream_type || (stream_type == DOLBY_LOSSLESS_AUDIO_STREAM && stream_id_extension == 0x76))
			{
				if (ParseAC3Frame(PID, stream_type, p, cbLeft, stm_info) == 0)
				{
					if (stream_type == DOLBY_LOSSLESS_AUDIO_STREAM && stream_id_extension == 0x76)
						audio_program_id = 1;
					else
						audio_program_id = 0;
					iParseRet = 0;
				}
			}
			else if (DD_PLUS_AUDIO_STREAM == stream_type)
			{
				if (ParseEAC3Frame(PID, stream_type, p, cbLeft, stm_info, audio_program_id) == 0)
				{
					iParseRet = 0;
				}
			}
		}
	}
	else if (HDMV_LPCM_AUDIO_STREAM == stream_type)
	{
		if (cbSize >= 4)
		{
			//uint16_t audio_data_payload_size = (pBuf[0] << 8) | pBuf[1];
			uint8_t channel_assignment = (pBuf[2] >> 4) & 0x0F;
			uint8_t sample_frequency = pBuf[2] & 0x0F;
			uint8_t bits_per_sample = (pBuf[3] >> 6) & 0x03;

			stm_info.stream_coding_type = stream_type;
			stm_info.audio_info.sample_frequency = sample_frequency == 1 ? 48000 : (sample_frequency == 4 ? 96000 : (sample_frequency == 5 ? 192000 : 0));
			stm_info.audio_info.bits_per_sample = bits_per_sample == 1 ? 16 : (bits_per_sample == 2 ? 20 : (bits_per_sample == 3 ? 24 : 0));
			stm_info.audio_info.channel_mapping = std::get<0>(hdmv_lpcm_ch_assignments[channel_assignment]);
			iParseRet = 0;
		}
	}
	else if (DTS_AUDIO_STREAM == stream_type)
	{
		uint32_t sync_code = p[0];
		while (cbLeft >= 13)
		{
			sync_code = (sync_code << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
			if (sync_code == DTS_SYNCWORD_CORE)	// Only Support Big-Endian 32-bit sync code at present.
				break;

			cbLeft--;
			p++;
		}

		if (ParseCoreDTSAU(PID, stream_type, sync_code, p, cbLeft, stm_info) == 0)
		{
			iParseRet = 0;
		}
	}
	else if (DTS_HD_EXCEPT_XLL_AUDIO_STREAM == stream_type || DTS_HD_XLL_AUDIO_STREAM == stream_type)
	{
		uint32_t sync_code = p[0];
		while (cbLeft >= 4)
		{
			sync_code = (sync_code << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
			if (sync_code == DTS_SYNCWORD_CORE || sync_code == DTS_SYNCWORD_SUBSTREAM)	// Only Support Big-Endian 32-bit sync code at present.
				break;

			cbLeft--;
			p++;
		}

		if (ParseCoreDTSHDAU(PID, stream_type, sync_code, p, cbLeft, stm_info) == 0)
		{
			iParseRet = 0;
		}
	}
	else if (AAC_AUDIO_STREAM == stream_type)
	{
		// In the PES payload, the first byte of the ES payload may not be the start byte an AAC frame
		unsigned short sync_code = p[0];
		unsigned char additional_header_bytes = 0;
		uint32_t crc_check = (uint32_t)-1;
		while (cbLeft >= ADTS_HEADER_SIZE)
		{
			while (cbLeft >= 2 && ((sync_code = (sync_code << 8) | (*(p + 1))) & 0xFFF0) != (AAC_SYNCWORD << 4))
			{
				cbLeft--;
				p++;
			}

			if (cbLeft < ADTS_HEADER_SIZE)
				break;

			// Check whether it is a real compatible ADTS header
			uint64_t u32Val = *((uint32_t*)p);
			ULONG_FIELD_ENDIAN(u32Val);

			uint32_t ID = (u32Val >> 19) & 0x1;
			uint32_t layer = (u32Val >> 17) & 03;
			uint32_t protection_absent = (u32Val >> 16) & 0x1;
			uint32_t profile_ObjectType = (u32Val >> 14) & 0x03;
			uint32_t sample_frequency_index = (u32Val >> 10) & 0x0F;
			//uint32_t channel_configuration = (u32Val >> 6) & 0x07;
			if (layer != 0 // Should be set to '00'
				|| sample_frequency_index == 0x0F	// escape value
				|| (ID == 1 && profile_ObjectType == 3)	// it is reserved for MPEG-2 AAC
				|| sample_frequency_index == 0x0D || sample_frequency_index == 0x0E // reserved
				)
			{
				crc_check = (uint32_t)-1;
				// It is not a real ADTS header, continue finding it
				cbLeft--;
				p++;
				continue;
			}

			//unsigned short aac_frame_length = ((p[3] & 0x03) << 11) | (p[4] << 3) | ((p[5] >> 5) & 0x7);
			unsigned char number_of_raw_data_blocks_in_frame = p[6] & 0x3;
			if (number_of_raw_data_blocks_in_frame == 0)
			{
				// adts_error_check()
				if (protection_absent == 0)
				{
					if (cbLeft >= ADTS_HEADER_SIZE + 2)
					{
						crc_check = (uint16_t)(((uint16_t)p[ADTS_HEADER_SIZE] << 8) | p[ADTS_HEADER_SIZE+1]);
						additional_header_bytes = 2;
					}
					else
					{
						cbLeft--;
						p++;
						continue;
					}
				}
			}
			else
			{
				if (protection_absent == 0)
				{
					if (cbLeft >= ADTS_HEADER_SIZE + 2 + number_of_raw_data_blocks_in_frame)
					{
						crc_check = (uint16_t)(((uint16_t)p[ADTS_HEADER_SIZE + number_of_raw_data_blocks_in_frame] << 8) |
							p[ADTS_HEADER_SIZE + number_of_raw_data_blocks_in_frame + 1]);
						additional_header_bytes += 2 + number_of_raw_data_blocks_in_frame;
					}
					else
					{
						cbLeft--;
						p++;
						continue;
					}
				}
			}

			if (cbLeft >= ADTS_HEADER_SIZE + additional_header_bytes + 1 && number_of_raw_data_blocks_in_frame == 0)
			{
				uint8_t id_syn_ele = (p[ADTS_HEADER_SIZE + additional_header_bytes] >> 5) & 0x7;
				if (id_syn_ele == 7)
				{
					crc_check = (uint32_t)-1;
					// It is not a real ADTS header, continue finding it
					cbLeft--;
					p++;
					continue;
				}
			}

			break;
		}

		if (cbLeft >= ADTS_HEADER_SIZE && ParseADTSFrame(PID, stream_type, sync_code, p, cbLeft, stm_info) == 0)
		{
			iParseRet = 0;
		}
	}
	else if (MPEG4_AAC_AUDIO_STREAM == stream_type)
	{
		stm_info.stream_coding_type = stream_type;
		// Check whether it is a LATM or LOAS packet
		if (AMP_SUCCEEDED(GetStreamInfoFromMP4AAU(p, cbLeft, stm_info)))
		{
			iParseRet = 0;
		}
	}
	else if (MPEG1_AUDIO_STREAM == stream_type || MPEG2_AUDIO_STREAM == stream_type)
	{
		unsigned short sync_code = p[0];
		while (cbLeft >= 4)
		{
			sync_code = (sync_code << 8) | p[1];
			if ((sync_code & 0xFFE0) == (MPEGA_FRAME_SYNC << 5))
				break;

			cbLeft--;
			p++;
		}

		if (ParseMPEGAudioFrame(PID, stream_type, sync_code, p, cbLeft, stm_info) == 0)
		{
			iParseRet = 0;
		}
	}
	else if (MPEG2_VIDEO_STREAM == stream_type)
	{
		stm_info.stream_coding_type = stream_type;
		if (AMP_SUCCEEDED(GetStreamInfoFromMPEG2AU(p, cbLeft, stm_info)))
		{
			iParseRet = 0;
		}
	}
	else if (MPEG4_AVC_VIDEO_STREAM == stream_type)
	{
		stm_info.stream_coding_type = stream_type;
		if (AMP_SUCCEEDED(GetStreamInfoFromSPS(NAL_CODING_AVC, p, cbLeft, stm_info)))
		{
			iParseRet = 0;
		}
	}
	else if (HEVC_VIDEO_STREAM == stream_type)
	{
		stm_info.stream_coding_type = stream_type;
		if (AMP_SUCCEEDED(GetStreamInfoFromSPS(NAL_CODING_HEVC, p, cbLeft, stm_info)))
		{
			iParseRet = 0;
		}
	}

	if (iParseRet == 0)
	{
		bool bChanged = false;

		stm_info.stream_id = stream_id;
		stm_info.stream_id_extension = stream_id_extension;

		// Compare with the previous audio information.
		if (g_stream_infos.find(PID) == g_stream_infos.end())
			bChanged = true;
		else if (IS_VIDEO_STREAM_TYPE(stream_type))
		{
			if (g_stream_infos[PID].size() > 0)
			{
				STREAM_INFO& cur_stm_info = g_stream_infos[PID][0];
				if (memcmp(&cur_stm_info, &stm_info, sizeof(stm_info)) != 0)
					bChanged = true;
			}
			else
				bChanged = true;
		}
		else
		{
			if (g_stream_infos[PID].size() > audio_program_id)
			{
				STREAM_INFO& cur_stm_info = g_stream_infos[PID][audio_program_id];
				if (memcmp(&cur_stm_info, &stm_info, sizeof(stm_info)) != 0)
					bChanged = true;
			}
			else
				bChanged = true;
		}

		if (bChanged)
		{
			if (IS_VIDEO_STREAM_TYPE(stream_type))
			{
				if (g_stream_infos[PID].size() == 0)
					g_stream_infos[PID].resize(1);
				g_stream_infos[PID][audio_program_id] = stm_info;
				printf("%s Stream information:\n", STREAM_TYPE_NAMEA(stm_info.stream_coding_type));

				printf("\tPID: 0X%X.\n", PID);

				if (stm_info.stream_id != -1)
					printf("\tStream ID: 0X%02X\n", stm_info.stream_id);

				printf("\tStream Type: %d(0X%02X).\n", stm_info.stream_coding_type, stm_info.stream_coding_type);

				if (stm_info.stream_id_extension != -1)
					printf("\tStream ID Extension: 0X%02X\n", stm_info.stream_id_extension);

				if (stream_type == HEVC_VIDEO_STREAM)
				{
					printf("\tHEVC Profile: %s\n", 
						stm_info.video_info.profile >= 0 && 
						stm_info.video_info.profile < (int32_t)(sizeof(hevc_profile_name)/sizeof(hevc_profile_name[0]))?hevc_profile_name[stm_info.video_info.profile]:"Unknown");
					printf("\tHEVC tier: %s\n", stm_info.video_info.tier == 0 ? "Main Tier" : (stm_info.video_info.tier == 1 ? "High Tier" : "Unknown"));
					printf("\tHEVC level: %d.%d\n", (int)(stm_info.video_info.level / 30), (int)(stm_info.video_info.level % 30 / 3));
				}
				else if (stream_type == MPEG4_AVC_VIDEO_STREAM)
				{
					printf("\tAVC Profile: %s\n", get_h264_profile_name(stm_info.video_info.profile));
					printf("\tAVC Level: %s\n", get_h264_level_name(stm_info.video_info.level));
				}
				else if (stream_type == MPEG2_VIDEO_STREAM)
				{
					printf("\tMPEG2 Video Profile: %s\n", mpeg2_profile_names[stm_info.video_info.profile]);
					printf("\tMPEG2 Video Level: %s\n", mpeg2_level_names[stm_info.video_info.level]);
				}

				if (stm_info.video_info.video_width != 0 && stm_info.video_info.video_height != 0)
					printf("\tVideo Resolution: %" PRIu32 "x%" PRIu32 "\n", stm_info.video_info.video_width, stm_info.video_info.video_height);

				if (stm_info.video_info.aspect_ratio_numerator != 0 && stm_info.video_info.aspect_ratio_denominator != 0)
					printf("\tVideo Aspect Ratio: %d:%d\n", stm_info.video_info.aspect_ratio_numerator, stm_info.video_info.aspect_ratio_denominator);

				if (stm_info.video_info.chroma_format_idc >= 0 && stm_info.video_info.chroma_format_idc <= 3)
					printf("\tVideo Chroma format: %s\n", chroma_format_idc_names[stm_info.video_info.chroma_format_idc]);

				if (stm_info.video_info.colour_primaries > 0)
					printf("\tColor Primaries: %s\n", vui_colour_primaries_names[stm_info.video_info.colour_primaries]);

				if (stm_info.video_info.transfer_characteristics > 0)
					printf("\tEOTF: %s\n", vui_transfer_characteristics_names[stm_info.video_info.transfer_characteristics]);

				if (stm_info.video_info.framerate_denominator != 0 && stm_info.video_info.framerate_numerator != 0)
					printf("\tFrame rate: %" PRIu32 "/%" PRIu32 " fps\n", stm_info.video_info.framerate_numerator, stm_info.video_info.framerate_denominator);

				if (stm_info.video_info.bitrate != 0 && stm_info.video_info.bitrate != UINT32_MAX)
				{
					uint32_t k = 1000;
					uint32_t m = k * k;
					if (stm_info.video_info.bitrate >= m)
						printf("\tBitrate: %" PRIu32 ".%03" PRIu32 " Mbps.\n", stm_info.video_info.bitrate / m, (uint32_t)(((uint64_t)stm_info.video_info.bitrate * 1000) / m % 1000));
					else if (stm_info.video_info.bitrate >= k)
						printf("\tBitrate: %" PRIu32 ".%03" PRIu32 " Kbps.\n", stm_info.video_info.bitrate / k, stm_info.video_info.bitrate * 1000 / k % 1000);
					else
						printf("\tBitrate: %" PRIu32 " bps.\n", stm_info.video_info.bitrate);
				}
			}
			else if(IS_AUDIO_STREAM_TYPE(stream_type))
			{
				if (g_stream_infos[PID].size() <= audio_program_id)
					g_stream_infos[PID].resize((size_t)audio_program_id + 1);
				g_stream_infos[PID][audio_program_id] = stm_info;
				if (g_stream_infos[PID].size() > 1)
				{
					if (stream_type == DOLBY_LOSSLESS_AUDIO_STREAM)
					{
						if (stream_id_extension == 0x76)
							printf("%s Stream information(AC3 part):\n", STREAM_TYPE_NAMEA(stm_info.stream_coding_type));
						else
							printf("%s Stream information(MLP part):\n", STREAM_TYPE_NAMEA(stm_info.stream_coding_type));

					}
					else
						printf("%s Stream information#%d:\n", STREAM_TYPE_NAMEA(stm_info.stream_coding_type), audio_program_id);
				}
				else
					printf("%s Stream information:\n", STREAM_TYPE_NAMEA(stm_info.stream_coding_type));
				printf("\tPID: 0X%X.\n", PID);

				if (stm_info.stream_id != -1)
					printf("\tStream ID: 0X%02X\n", stm_info.stream_id);

				printf("\tStream Type: %d(0X%02X).\n", stm_info.stream_coding_type, stm_info.stream_coding_type);

				if (stm_info.stream_id_extension != -1)
					printf("\tStream ID Extension: 0X%02X\n", stm_info.stream_id_extension);

				printf("\tSample Frequency: %d (HZ).\n", stm_info.audio_info.sample_frequency);
				printf("\tBits Per Sample: %d.\n", stm_info.audio_info.bits_per_sample);
				printf("\tChannel Layout: %s.\n", stm_info.audio_info.channel_mapping.get_desc().c_str());

				if (stm_info.audio_info.bitrate != 0 && stm_info.audio_info.bitrate != UINT32_MAX)
				{
					uint32_t k = 1000;
					uint32_t m = k * k;
					if (stm_info.audio_info.bitrate >= m)
						printf("\tBitrate: %" PRIu32 ".%03" PRIu32 " Mbps.\n", stm_info.video_info.bitrate / m, (uint32_t)(((uint64_t)stm_info.video_info.bitrate * 1000) / m % 1000));
					else if (stm_info.audio_info.bitrate >= k)
						printf("\tBitrate: %" PRIu32 ".%03" PRIu32 " kbps.\n", stm_info.audio_info.bitrate / k, stm_info.audio_info.bitrate * 1000 / k % 1000);
					else
						printf("\tBitrate: %" PRIu32 " bps.\n", stm_info.audio_info.bitrate);
				}
			}
			else
			{
				if (g_stream_infos[PID].size() == 0)
					g_stream_infos[PID].resize(1);
				g_stream_infos[PID][audio_program_id] = stm_info;
				printf("%s Stream information:\n", STREAM_TYPE_NAMEA(stm_info.stream_coding_type));

				printf("\tPID: 0X%X.\n", PID);

				if (stm_info.stream_id != -1)
					printf("\tStream ID: 0X%02X\n", stm_info.stream_id);

				printf("\tStream Type: %d(0X%02X).\n", stm_info.stream_coding_type, stm_info.stream_coding_type);

				if (stm_info.stream_id_extension != -1)
					printf("\tStream ID Extension: 0X%02X\n", stm_info.stream_id_extension);
			}
		}
	}

	return iParseRet;
}

int WriteWaveFileBuffer(FILE* fw, int stream_type, unsigned char* es_buffer, int es_buffer_len)
{
	if (fw == NULL)
		return -1;

	// The first LPCM packet, need write the wave header
	uint16_t audio_data_payload_size = (es_buffer[0] << 8) | es_buffer[1];
	uint8_t channel_assignment = (es_buffer[2] >> 4) & 0x0F;
	uint8_t sample_frequency = es_buffer[2] & 0x0F;
	uint8_t bits_per_sample = (es_buffer[3] >> 6) & 0x03;

	if (es_buffer_len < audio_data_payload_size + 4)
		return -1;

	uint32_t numch[] = { 0, 1, 0, 2, 4, 4,4, 4, 6, 6, 8, 8, 0, 0, 0, 0 };
	uint32_t fs[] = { 0, 48000, 0, 0, 96000, 192000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t bps[] = { 0, 16, 20, 24 };

	if (numch[channel_assignment] == 0)
	{
		if (g_verbose_level > 0)
			printf("Channel assignment value (%d) in PCM header is invalid.\n", channel_assignment);
		return -1;
	}

	if (fs[sample_frequency] == 0)
	{
		if (g_verbose_level > 0)
			printf("Sample frequency value (%d) in PCM header is invalid.\n", sample_frequency);
		return -1;
	}

	if (bps[bits_per_sample] == 0)
	{
		if (g_verbose_level > 0)
			printf("Bits_per_sample(%d) in PCM header is invalid.\n", bits_per_sample);
		return -1;
	}

	uint8_t* hdmv_lpcm_payload_buf = new uint8_t[audio_data_payload_size];
	memcpy(hdmv_lpcm_payload_buf, es_buffer + 4, audio_data_payload_size);

	bool bWaveFormatExtent = (numch[channel_assignment] > 2 || bps[bits_per_sample] > 16) ? true : false;
	uint32_t dwChannelMask = 0;
	for (size_t i = 0; i < _countof(DCINEMA_Channel_Speaker_Mapping); i++)
	{
		if (std::get<0>(hdmv_lpcm_ch_assignments[channel_assignment]).is_present((int)i))
		{
			if (DCINEMA_Channel_Speaker_Mapping[i] < SPEAKER_POS_MAX)
				dwChannelMask |= CHANNEL_BITMASK(DCINEMA_Channel_Speaker_Mapping[i]);
		}
	}

	if (g_dump_status.num_of_dumped_payloads == 0)
	{
		std::vector<uint8_t> riff_hdr_bytes;
		if (!bWaveFormatExtent)
		{
			// mono or stereo, old wave format file can support it.
			tuple<
				uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
				uint16_t, uint16_t, uint32_t, uint32_t, uint16_t, uint16_t,
				uint32_t, uint32_t> RIFFHdr = 
			{
				ENDIANULONG('RIFF'), 0, ENDIANULONG('WAVE'), ENDIANULONG('fmt '), 16,
				1, numch[channel_assignment], fs[sample_frequency], numch[channel_assignment] * fs[sample_frequency] * (bps[bits_per_sample] + 7) / 8, 
				numch[channel_assignment] * ((bps[bits_per_sample] + 7) / 8), (bps[bits_per_sample] + 7) & 0xFFF8,
				ENDIANULONG('data'), 0
			};
	
			
			fill_bytes_from_tuple(RIFFHdr, riff_hdr_bytes);
		}
		else
		{
			// multiple channels, or 20-bit LPCM
			tuple<
				uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
				uint16_t, uint16_t, uint32_t, uint32_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t,
				uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
				uint32_t, uint32_t, uint32_t,
				uint32_t, uint32_t> RIFFHdr = 
			{
				ENDIANULONG('RIFF'), 0, ENDIANULONG('WAVE'), ENDIANULONG('fmt '), 40,
				0xFFFE, numch[channel_assignment], fs[sample_frequency], numch[channel_assignment] * fs[sample_frequency] * (bps[bits_per_sample] + 7) / 8,
				numch[channel_assignment] * ((bps[bits_per_sample] + 7) / 8), (bps[bits_per_sample] + 7) & 0xFFF8,
				22, bps[bits_per_sample], dwChannelMask, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71,
				ENDIANULONG('fact'), 4, 0,
				ENDIANULONG('data'), 0
			};

			fill_bytes_from_tuple(RIFFHdr, riff_hdr_bytes);
		}

		if (fw != NULL)
			fwrite(riff_hdr_bytes.data(), 1, riff_hdr_bytes.size(), fw);
	}

	uint8_t* raw_data = es_buffer + 4;
	int raw_data_len = es_buffer_len - 4;

	uint8_t* pDstSample = raw_data;
	uint8_t* pSrcSample = hdmv_lpcm_payload_buf;
	int BPS = (bps[bits_per_sample] + 7) / 8;
	int group_size = numch[channel_assignment] * BPS;
	std::vector<uint8_t>& chmaps = std::get<1>(hdmv_lpcm_ch_assignments[channel_assignment]);

	// CHANNLE_LOC -> SPEAKER_LOC -> Channel Number
	int speaker_ch_num = 0;
	int Speaker_Channel_Numbers[32] = { -1 };
	for (int i = SPEAKER_POS_FRONT_LEFT; i < SPEAKER_POS_MAX; i++)
		if (dwChannelMask&CHANNEL_BITMASK(i))
			Speaker_Channel_Numbers[i] = speaker_ch_num++;

	for (int i = 0; i < audio_data_payload_size / group_size; i++)
	{
		for (size_t src_ch = 0; src_ch < chmaps.size(); src_ch++)
		{
			size_t dst_ch = Speaker_Channel_Numbers[DCINEMA_Channel_Speaker_Mapping[chmaps[src_ch]]];
			for (int B = 0; B < BPS; B++)
			{
				pDstSample[dst_ch*BPS + B] = pSrcSample[src_ch*BPS + BPS - B - 1];
			}
		}

		pDstSample += group_size;
		pSrcSample += group_size;
	}

	raw_data_len = audio_data_payload_size / group_size*group_size;

	// Write the data body
	if (fw != NULL && raw_data_len > 0)
		fwrite(raw_data, 1, raw_data_len, fw);

	delete[] hdmv_lpcm_payload_buf;

	return 0;
}

int FinalizeWaveFileBuffer(FILE* fw, unsigned PID, int stream_type)
{
	long long real_file_size = _ftelli64(fw);
	if (real_file_size == 0)
		return -1;

	uint32_t file_size = real_file_size > UINT32_MAX ? UINT32_MAX : (uint32_t)real_file_size;

	fseek(fw, 0, SEEK_SET);
	unsigned char wave_header1[36];
	if (fread(wave_header1, 1, sizeof(wave_header1), fw) <= 0)
		return -1;

	uint16_t audio_format = wave_header1[20] | (wave_header1[21] << 8);
	uint16_t num_of_chs = wave_header1[22] | (wave_header1[23] << 8);
	*((uint32_t*)(&wave_header1[4])) = file_size - 8;

	// write-back the header.
	fseek(fw, 0, SEEK_SET);
	fwrite(wave_header1, 1, sizeof(wave_header1), fw);

	uint32_t data_chunk_size = 0;
	if (audio_format == 1)
	{
		data_chunk_size = file_size - 36 - 8;
	}
	else if (audio_format == 0xFFFE)
	{
		fseek(fw, 24, SEEK_CUR);

		uint8_t fact_chunk_buf[12];
		if (fread(fact_chunk_buf, 1, sizeof(fact_chunk_buf), fw) <= 0)
			return -1;

		data_chunk_size = file_size - 36 - 24 - 12 - 4;
		uint32_t each_channel_size = data_chunk_size / num_of_chs;
		*((uint32_t*)(&fact_chunk_buf[8])) = (each_channel_size) & 0xFF;

		fseek(fw, -1 * (long)(sizeof(fact_chunk_buf)), SEEK_CUR);
		fwrite(fact_chunk_buf, 1, sizeof(fact_chunk_buf), fw);
	}

	if (data_chunk_size > 0)
	{
		uint8_t data_chunk_hdr[8];
		if (fread(data_chunk_hdr, 1, sizeof(data_chunk_hdr), fw) <= 0)
			return -1;

		*((uint32_t*)(&data_chunk_hdr[4])) = data_chunk_size;

		fseek(fw, -1 * (long)(sizeof(data_chunk_hdr)), SEEK_CUR);
		fwrite(data_chunk_hdr, 1, sizeof(data_chunk_hdr), fw);
	}

	return 0;
}

int FlushPSIBuffer(FILE* fw, unsigned char* psi_buffer, int psi_buffer_len, int dumpopt, int &raw_data_len)
{
	int iret = 0;
	// For PSI, store whole payload with pointer field

	uint8_t* p = psi_buffer;
	int cbLeft = psi_buffer_len;

	uint8_t pointer_field = *(p++); cbLeft--;

	if (cbLeft < pointer_field)
	{
		printf("invalid 'pointer_field' value.\n");
		return -1;
	}

	p += pointer_field;
	cbLeft -= pointer_field;

	if (cbLeft < 3)
	{
		printf("No enough data for PSI section.\n");
		return -1;
	}

	uint16_t section_length = ((p[1] & 0xF) << 8) | p[2];

	if (section_length + 4 > psi_buffer_len)
	{
		printf("The PSI section data seems not to be enough.\n");
		return -1;
	}

	raw_data_len = (int)section_length + 4;
	if (fw != NULL)
		fwrite(psi_buffer, 1, raw_data_len, fw);

	return iret;
}

int FlushPESBuffer(
	unsigned char* pes_buffer,
	int pes_buffer_len,
	int &raw_data_len,
	PES_FILTER_INFO& filter_info,
	FILE* fw,
	int dumpopt)
{
	int iret = 0;
	int stream_type = -1;
	int stream_id = -1;
	int stream_id_extension = -1;
	int64_t d64PTS = INT64_MIN, d64DTS = INT64_MIN;
	char szLog[512] = { 0 };
	raw_data_len = 0;

	if (!(dumpopt&DUMP_VOB))
	{
		stream_type = filter_info.TS.stream_type;
		stream_id = filter_info.TS.stream_id;
		stream_id_extension = filter_info.TS.stream_id_extension;
	}

	if (pes_buffer_len >= 9)
	{
		if (pes_buffer[0] == 0 &&
			pes_buffer[1] == 0 &&
			pes_buffer[2] == 1)
		{
			int off = 9;
			int pes_stream_id = pes_buffer[3];
			int pes_stream_id_extension = -1;
			int pes_len = pes_buffer[4] << 8 | pes_buffer[5];
			int pes_hdr_len = 0;
			unsigned char* raw_data = NULL;

			if (pes_stream_id != SID_PROGRAM_STREAM_MAP &&
				//pes_stream_id != SID_PRIVATE_STREAM_1 &&
				pes_stream_id != SID_PADDING_STREAM &&
				pes_stream_id != SID_PRIVATE_STREAM_2 &&
				pes_stream_id != SID_ECM &&
				pes_stream_id != SID_EMM &&
				pes_stream_id != SID_PROGRAM_STREAM_DIRECTORY &&
				pes_stream_id != SID_DSMCC_STREAM &&
				pes_stream_id != SID_H222_1_TYPE_E) {
				unsigned char PTS_DTS_flags = (pes_buffer[7] >> 6) & 0x3;
				unsigned char ESCR_flag = (pes_buffer[7] >> 5) & 0x1;
				unsigned char ES_rate_flag = (pes_buffer[7] >> 4) & 0x1;
				unsigned char DSM_trick_mode_flag = (pes_buffer[7] >> 3) & 0x1;
				unsigned char additional_copy_info_flag = (pes_buffer[7] >> 2) & 0x1;
				unsigned char PES_CRC_flag = (pes_buffer[7] >> 1) & 0x1;
				unsigned char pes_hdr_extension_flag = pes_buffer[7] & 0x1;

				int pes_hdr_len = pes_buffer[8];

				if (PTS_DTS_flags == 2)
					off += 5;
				else if (PTS_DTS_flags == 3)
					off += 10;

				if (ESCR_flag)
					off += 6;

				if (ES_rate_flag)
					off += 3;

				if (DSM_trick_mode_flag)
					off += 1;

				if (additional_copy_info_flag)
					off += 1;

				if (PES_CRC_flag)
					off += 2;

				if (off >= pes_buffer_len)
					return -1;

				if (pes_hdr_extension_flag)
				{
					unsigned char PES_private_data_flag = (pes_buffer[off] >> 7) & 0x1;
					unsigned char pack_header_field_flag = (pes_buffer[off] >> 6) & 0x1;
					unsigned char program_packet_sequence_counter_flag = (pes_buffer[off] >> 5) & 0x1;
					unsigned char PSTD_buffer_flag = (pes_buffer[off] >> 4) & 0x1;
					unsigned char PES_extension_flag_2 = pes_buffer[off] & 0x1;
					off += 1;

					if (PES_private_data_flag)
						off += 16;

					if (pack_header_field_flag)
					{
						off++;

						if (off >= pes_buffer_len)
							return -1;

						off += pes_buffer[off];
					}

					if (program_packet_sequence_counter_flag)
						off += 2;

					if (PSTD_buffer_flag)
					{
						if ((dumpopt&DUMP_VOB) && (pes_buffer[3] & 0xF0) == 0xE0)	// Adjust stream_type to 1 or 2 accurately for DVD-Video VOB
						{
							// it is a video stream
							int P_STD_buffer_scale = (pes_buffer[off] >> 5) & 0x1;
							if (P_STD_buffer_scale)
							{
								// According to DVD-Video Spec, P-STD buffer_size has below meaning
								// 232: Payload according to ISO/IEC 13818-2)
								//  46: Payload according to ISO/IEC 11172-2)
								int P_STD_buffer_size = ((pes_buffer[off] & 0x1F) << 8) | pes_buffer[off + 1];
								if (P_STD_buffer_size == 46)
									stream_type = 1;
								else if (P_STD_buffer_size == 232)
									stream_type = 2;
							}
						}

						off += 2;

					}

					if (off >= pes_buffer_len)
						return -1;

					if (PES_extension_flag_2)
					{
						unsigned char PES_extension_field_length = pes_buffer[off] & 0x7F;

						off++;

						if (off >= pes_buffer_len)
							return -1;

						if (PES_extension_field_length > 0)
						{
							unsigned char stream_id_extension_flag = (pes_buffer[off] >> 7) & 0x1;
							if (stream_id_extension_flag == 0)
							{
								pes_stream_id_extension = pes_buffer[off];
							}
						}
					}
				}

				// filter it by stream_id and stream_id_extension
				if (stream_id != -1 && stream_id != pes_stream_id)
					return 1;	// mis-match with stream_id filter

				if (stream_id_extension != -1 && pes_stream_id_extension != -1 && stream_id_extension != pes_stream_id_extension)
					return 1;	// mis-match with stream_id filter

				if (pes_len == 0)
					raw_data_len = pes_buffer_len - pes_hdr_len - 9;
				else
					raw_data_len = pes_len - 3 - pes_hdr_len;

				raw_data = pes_buffer + pes_hdr_len + 9;
				{
					uint8_t* pHdrStart = pes_buffer + 9;
					if (PTS_DTS_flags & 0x2)
					{
						if (pes_buffer_len >= pes_hdr_len + 9)
							d64PTS = GetPTSValue(pHdrStart);

						pHdrStart += 5;
					}

					if (PTS_DTS_flags & 0x01)
					{
						if (pes_buffer_len >= pes_hdr_len + 9)
							d64DTS = GetPTSValue(pHdrStart);

						pHdrStart += 5;
					}
				}
			}
			else
			{
				raw_data_len = pes_len;
				raw_data = pes_buffer + 6;
			}

			// Decide the actual filter information for DVD-Video VOB file
			if (dumpopt&DUMP_VOB)
			{
				if (filter_info.VOB.StreamIndex >= 0 && filter_info.VOB.StreamIndex <= 0x0F)
				{
					if (filter_info.VOB.Stream_CAT == STREAM_CAT_VIDEO)
					{
						stream_id = 0xe0 | filter_info.VOB.StreamIndex;
					}
					else if (filter_info.VOB.Stream_CAT == STREAM_CAT_AUDIO)
					{
						// If the current ES is MPEG audio
						if ((pes_buffer[3] & 0xF8) == 0xC0)
						{
							stream_id = 0xC8 | filter_info.VOB.StreamIndex;
						}
						else if (pes_buffer[3] == 0xbd && raw_data_len > 0)
						{
							stream_id = 0xbd;
							if ((raw_data[0] & 0xF8) == 0xA0)	// LPCM
							{

							}
							else if ((raw_data[0] & 0xF8) == 0x80)	// AC3
							{

							}
							else if ((raw_data[0] & 0xF8) == 0x88)	// DTS
							{

							}
						}
					}
					else if (filter_info.VOB.Stream_CAT == STREAM_CAT_SUBTITLE)
					{

					}
				}
				else
					printf("Unexpected stream index value(%d) specified.\n", filter_info.VOB.StreamIndex);
			}

			// Check the media information of current elementary stream
			if ((DUMP_MEDIA_INFO_VIEW&dumpopt)/* ||
				(!(DUMP_VOB&dumpopt) && IS_NAL_STREAM_TYPE(filter_info.TS.stream_type))*/)
			{
				if (raw_data_len > 0)
				{
					int StreamUniqueID = 1;
					// For DVD-Video VOB, PID = (STREAM_CAT << 8) | stream_number
					if (dumpopt&DUMP_VOB)
						StreamUniqueID = (filter_info.VOB.Stream_CAT << 8) | filter_info.VOB.StreamIndex;
					else
						StreamUniqueID = filter_info.TS.PID;

					CheckRawBufferMediaInfo((unsigned short)StreamUniqueID, stream_type, raw_data, raw_data_len, dumpopt, pes_stream_id, pes_stream_id_extension);
				}
			}

			if ((dumpopt&DUMP_RAW_OUTPUT) || (dumpopt&DUMP_PCM) || (dumpopt&DUMP_WAV))
			{
				if (raw_data_len < 0)
				{
					if (g_verbose_level >= 1)
						printf("The PES buffer length(%d) is too short, it is expected to be greater than %d.\n", pes_buffer_len, std::max(pes_len + 6, pes_hdr_len + 9));
					iret = -1;
					goto done;
				}

				if ((dumpopt&DUMP_PCM) || (dumpopt&DUMP_WAV))
				{
					// Check whether stream_type is the supported type or not
					if (stream_type != HDMV_LPCM_AUDIO_STREAM)
					{
						if (g_verbose_level >= 1)
							printf("At present, not sure ES is LPCM stream or not, drop this ES packet.\n");
						goto done;
					}

					if (dumpopt&DUMP_WAV)
					{
						iret = WriteWaveFileBuffer(fw, stream_type, raw_data, raw_data_len);
					}

					// strip the LPCM header (4 bytes)
					raw_data += 4;
					raw_data_len -= 4;
				}

				if (fw != NULL && raw_data_len > 0 && !(dumpopt&DUMP_WAV))
				{
					if (dumpopt&DUMP_VOB)
					{
						// TODO...
					}
					fwrite(raw_data, 1, raw_data_len, fw);
				}
			}
			else if (dumpopt&DUMP_PES_OUTPUT)
			{
				raw_data_len = pes_len == 0 ? pes_buffer_len : pes_len;
				if (fw != NULL)
					fwrite(pes_buffer, 1, raw_data_len, fw);
			}

			if (dumpopt&DUMP_PTS_VIEW)
			{
				if (pes_buffer_len < pes_len + 6 || pes_buffer_len < pes_hdr_len + 9)
				{
					iret = -1;
				}
				else
				{
					static int PktIndex = 0;
					size_t ccLog = sizeof(szLog) / sizeof(szLog[0]);
					int ccWritten = 0;
					int ccWrittenOnce = MBCSPRINTF_S(szLog, ccLog, "PktIndex:%10d", PktIndex++);
					if (ccWrittenOnce > 0)
						ccWritten += ccWrittenOnce;

					if (ccWrittenOnce > 0)
					{
						if (d64PTS == INT64_MIN)
							ccWrittenOnce = MBCSPRINTF_S(szLog + ccWritten, ccLog - ccWritten, ", PTS: %22c", ' ');
						else
							ccWrittenOnce = MBCSPRINTF_S(szLog + ccWritten, ccLog - ccWritten, ", PTS: %10" PRId64 "(%9" PRIX64 "h)", d64PTS, d64PTS);

						if (ccWrittenOnce > 0)
							ccWritten += ccWrittenOnce;
					}

					if (ccWrittenOnce > 0)
					{
						if (d64DTS == INT64_MIN)
							ccWrittenOnce = MBCSPRINTF_S(szLog + ccWritten, ccLog - ccWritten, ", DTS: %22c", ' ');
						else
							ccWrittenOnce = MBCSPRINTF_S(szLog + ccWritten, ccLog - ccWritten, ", DTS: %10" PRId64 "(%9" PRIX64 "h)", d64DTS, d64DTS);

						if (ccWrittenOnce > 0)
							ccWritten += ccWrittenOnce;
					}

					if (ccWrittenOnce > 0)
					{
						ccWrittenOnce = MBCSPRINTF_S(szLog + ccWritten, ccLog - ccWritten, ", PES pkt len: %10d, ES len: %10d", pes_buffer_len, raw_data_len);

						if (ccWrittenOnce > 0)
							ccWritten += ccWrittenOnce;
					}

					printf("%s.\n", szLog);
				}
			}
		}
		else
		{
			iret = -2;	// invalid ES
		}
	}
	else if (pes_buffer_len > 0)
	{
		iret = -3;	// too short PES raw data buffer
	}

done:
	return iret;
}

struct PID_STAT
{
	uint64_t			count = 0;
	std::set<uint8_t>	stream_types;
	uint32_t			pcr : 1;
	uint32_t			pmt : 1;
	uint32_t			reserved : 30;

	PID_STAT()
		: pcr(0), pmt(0), reserved(0) {
	}
};

std::map<uint16_t, const char*> PID_Allocation_ARIB = {
	{ 0x0000, "PAT"},
	{ 0x0001, "CAT"},
	{ 0x0010, "NIT"},
	{ 0x0011, "SDT/BAT"},
	{ 0x0012, "EIT"},
	{ 0x0013, "RST"},
	{ 0x0014, "TDT/TOT"},
	{ 0x0017, "DCT"},
	{ 0x001E, "DIT"},
	{ 0x001F, "SIT"},
	{ 0x0020, "LIT"},
	{ 0x0021, "ERT"},
	{ 0x0022, "PCAT"},
	{ 0x0023, "SDTT"},
	{ 0x0024, "BIT"},
	{ 0x0025, "NBIT/LDT"},
	{ 0x0026, "EIT"},
	{ 0x0027, "EIT"},
	{ 0x0028, "SDTT"},
	{ 0x0029, "CDT"},
	{ 0x002F, "Multiple frame header information"},
	{ 0x1FFF, "Null packet"}
};

// Dump a stream with specified PID to a PES/ES stream
int DumpOneStream()
{
	int iRet = -1;
	FILE *fp = NULL, *fw = NULL;
	int sPID = FILTER_PID;
	int dumpopt = 0;
	errno_t errn;
	unsigned char* pes_buffer = new (std::nothrow) unsigned char[20 * 1024 * 1024];

	std::map<uint16_t, PID_STAT> PID_stat;

	//size_t pes_hdr_location = 0;
	int raw_data_len = 0, dump_ret;
	unsigned long pes_buffer_len = 0;
	//unsigned long pat_buffer_len = 0;
	int buf_head_offset = g_ts_fmtinfo.num_of_prefix_bytes;
	uint16_t ts_pack_size = g_ts_fmtinfo.packet_size;
	int stream_id = -1;
	int stream_id_extension = -1;
	unsigned long ts_pack_idx = 0;
	int64_t start_tspck_pos = -1LL;
	int64_t end_tspck_pos = -1LL;
	int sProgSeqID = -1;
	int curProgSeqID = -1;
	bool bCopyOriginalStream = false;
	std::vector<uint8_t> unwritten_pmt_buf;
	std::vector<uint8_t> unwritten_pat_buf;

	unordered_map<unsigned short, CPSIBuf*> pPSIBufs;
	unordered_map<unsigned short, unsigned char> stream_types;
	unordered_map<unsigned short, unordered_map<unsigned short, unsigned char>> prev_PMT_stream_types;
	unordered_map<unsigned short, uint8_t> mapPSIVersionNumbers;

	PSI_PROCESS_CONTEXT psi_process_ctx;
	psi_process_ctx.pPSIPayloads = &pPSIBufs;
	psi_process_ctx.bChanged = false;

	pPSIBufs[0] = new CPSIBuf(&psi_process_ctx, PID_PROGRAM_ASSOCIATION_TABLE);
	pPSIBufs[0x1F] = new CPSIBuf(&psi_process_ctx, PID_SELECTION_INFORMATION_TABLE);

	unsigned char buf[1024];
	auto iterSrcFmt = g_params.find("srcfmt");
	auto iterDstFmt = g_params.find("outputfmt");

	if (g_params.find("pid") == g_params.end())
	{
		printf("No PID is specified, nothing to do.\n");
		goto done;
	}

	sPID = (int)ConvertToLongLong(g_params["pid"]);

	errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if (g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	if (iterSrcFmt != g_params.end() && iterSrcFmt->second.compare("m2ts") == 0)
		dumpopt |= DUMP_BD_M2TS;

	if (g_params.find("stream_id_extension") != g_params.end())
	{
		stream_id_extension = (int)ConvertToLongLong(g_params["stream_id_extension"]);
	}

	if (g_params.find("stream_id") != g_params.end())
	{
		stream_id = (int)ConvertToLongLong(g_params["stream_id"]);
	}

	// Make sure the dump option
	if (iterDstFmt != g_params.end())
	{
		// Don't support converting ts to tts or m2ts at present
		if (iterSrcFmt != g_params.end() &&
			_stricmp(iterSrcFmt->second.c_str(), "ts") == 0 && (
			_stricmp(iterDstFmt->second.c_str(), "tts") == 0 ||
			_stricmp(iterDstFmt->second.c_str(), "m2ts") == 0))
		{
			printf("Don't support convert source format: %s to destination format: %s!\n", iterSrcFmt->second.c_str(), iterDstFmt->second.c_str());
			goto done;
		}

		if (_stricmp(iterDstFmt->second.c_str(), "es") == 0)
			dumpopt |= DUMP_RAW_OUTPUT;
		else if (_stricmp(iterDstFmt->second.c_str(), "pes") == 0)
			dumpopt |= DUMP_PES_OUTPUT;
		else if (_stricmp(iterDstFmt->second.c_str(), "pcm") == 0)
			dumpopt |= DUMP_PCM;
		else if (_stricmp(iterDstFmt->second.c_str(), "wav") == 0)
			dumpopt |= DUMP_WAV;
	}

	if (g_params.find("showpts") != g_params.end())
	{
		dumpopt |= DUMP_PTS_VIEW;
	}

	if (g_params.find("showinfo") != g_params.end())
	{
		if (sPID > 0)
			dumpopt |= DUMP_STREAM_INFO_VIEW;

		dumpopt |= DUMP_MEDIA_INFO_VIEW;
	}

	if (g_params.find("showSIT") != g_params.end())
	{
		dumpopt |= DUMP_DTV_SIT;
	}

	if (g_params.find("showPMT") != g_params.end())
	{
		dumpopt |= DUMP_PMT;
	}

	if (g_params.find("showPAT") != g_params.end())
	{
		dumpopt |= DUMP_PAT;
	}

#if 0
	if (g_params.find("showVPS") != g_params.end())
	{
		dumpopt |= DUMP_NAL_VPS;
	}

	if (g_params.find("showSPS") != g_params.end())
	{
		dumpopt |= DUMP_NAL_SPS;
	}

	if (g_params.find("showPPS") != g_params.end())
	{
		dumpopt |= DUMP_NAL_PPS;
	}
#endif

	if (g_params.find("start") != g_params.end())
	{
		start_tspck_pos = ConvertToLongLong(g_params["start"]);

		// Check the start ts pack position
		if (start_tspck_pos < 0 || (unsigned long long)start_tspck_pos >= g_dump_status.num_of_packs)
		{
			iRet = RET_CODE_INVALID_PARAMETER;
			printf("The current start pack pos(%" PRId64 ") exceed the valid range [0, %" PRIu64 ").\n", start_tspck_pos, g_dump_status.num_of_packs);
			goto done;
		}

		if (FSEEK64(fp, start_tspck_pos*ts_pack_size, SEEK_SET) != 0)
		{
			iRet = RET_CODE_ERROR;
			printf("Failed to seek the specified position {err: %d}.\n", ferror(fp));
			goto done;
		}

		g_dump_status.cur_pack_idx = start_tspck_pos;
		ts_pack_idx = (unsigned long)start_tspck_pos;
	}
	else
		start_tspck_pos = 0;

	if (g_params.find("end") != g_params.end())
	{
		end_tspck_pos = ConvertToLongLong(g_params["end"]);

		// Check the end ts pack position
		if (end_tspck_pos <= 0 || (unsigned long long)end_tspck_pos > g_dump_status.num_of_packs)
		{
			iRet = RET_CODE_INVALID_PARAMETER;
			printf("The current end pack pos(%" PRId64 ") exceed the valid range (0, %" PRIu64 "].\n", end_tspck_pos, g_dump_status.num_of_packs);
			goto done;
		}
	}
	else
		end_tspck_pos = std::numeric_limits<decltype(end_tspck_pos)>::max();

	if (g_params.find("progseq") != g_params.end())
		sProgSeqID = (int)ConvertToLongLong(g_params["progseq"]);

	if (iterDstFmt != g_params.end())
	{
		bCopyOriginalStream = (_stricmp(iterDstFmt->second.c_str(), "copy") == 0 ||
							   _stricmp(iterDstFmt->second.c_str(), "m2ts") == 0 ||
							   _stricmp(iterDstFmt->second.c_str(), "tts") == 0 ||
							   _stricmp(iterDstFmt->second.c_str(), "ts") == 0) ? true : false;
		if (g_verbose_level > 0)
		{
			if (start_tspck_pos > 0 || (end_tspck_pos > 0 && end_tspck_pos != std::numeric_limits<decltype(end_tspck_pos)>::max()))
				printf("copy a part of the original stream file.\n");
			else if (curProgSeqID >= 0)
				printf("copy the #%d program sequence of the original stream file.\n", curProgSeqID);
			else
				printf("copy the original stream file.\n");
		}
	}

	if (sProgSeqID >= 0 && bCopyOriginalStream)
	{
		unwritten_pat_buf.reserve(6144);
		unwritten_pmt_buf.reserve(6144);	// 32 TS packets
	}

	g_dump_status.state = DUMP_STATE_RUNNING;

	while (true)
	{
		// Check whether to reach the stopping position or not
		if (g_dump_status.cur_pack_idx >= (unsigned long long)end_tspck_pos)
		{
			if (end_tspck_pos != std::numeric_limits<decltype(end_tspck_pos)>::max())
				printf("Reach the end position: %" PRId64 ", stop dumping the stream.\n", end_tspck_pos);
			break;
		}

		if (sProgSeqID >= 0 && curProgSeqID > sProgSeqID)
		{
			printf("Now reach to program sequence#%d, stop dumping the stream.\n", curProgSeqID);
			break;
		}

		uint16_t nRead = (uint16_t)fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		unsigned short PID = (buf[buf_head_offset + 1] & 0x1f) << 8 | buf[buf_head_offset + 2];
		auto iter_PID_state = PID_stat.find(PID);

		if (iter_PID_state == PID_stat.end())
		{
			auto insert_ret = PID_stat.insert({ PID, PID_STAT() });
			if (insert_ret.second == false)
			{
				printf("Failed to insert a stat record for PID: 0x%04X.\n", PID);
				break;
			}
			iter_PID_state = insert_ret.first;
		}

		iter_PID_state->second.count++;

		bool bPSI = pPSIBufs.find(PID) != pPSIBufs.end() ? true : false;

		if (bCopyOriginalStream)
		{
			if (sProgSeqID == -1 && fw != NULL)
				fwrite(buf, 1, ts_pack_size, fw);
			else
			{
				if (sProgSeqID >= 0 && curProgSeqID == sProgSeqID && !bPSI)
				{
					if (unwritten_pat_buf.size() > 0)
					{
						fwrite(unwritten_pat_buf.data(), 1, unwritten_pat_buf.size(), fw);
						unwritten_pat_buf.clear();
					}

					if (unwritten_pmt_buf.size() > 0)
					{
						fwrite(unwritten_pmt_buf.data(), 1, unwritten_pmt_buf.size(), fw);
						unwritten_pmt_buf.clear();
					}

					fwrite(buf, 1, ts_pack_size, fw);
				}
			}
		}

		// Continue the parsing when PAT/PMT is hit
		if (PID != sPID && !bPSI)
		{
			g_dump_status.cur_pack_idx++;
			ts_pack_idx++;
			continue;
		}

		int index = buf_head_offset + 4;
		unsigned char payload_unit_start = buf[buf_head_offset + 1] & 0x40;
		unsigned char adaptation_field_control = (buf[buf_head_offset + 3] >> 4) & 0x03;
		//unsigned char discontinuity_counter = buf[buf_head_offset + 3] & 0x0f;

		if (adaptation_field_control & 0x02)
			index += buf[buf_head_offset + 4] + 1;

		if (payload_unit_start)
		{
			if (bPSI)
			{
				auto iterPSIBuf = pPSIBufs.find(PID);
				if (iterPSIBuf != pPSIBufs.end())
				{
					if (iterPSIBuf->second->ProcessPSI(dumpopt) == 0)
					{
						// Update each PID stream type
						if (iterPSIBuf->second->table_id == TID_TS_program_map_section && sPID != 0)
						{
							unsigned char stream_type = 0xFF;
							if (((CPMTBuf*)iterPSIBuf->second)->GetPMTInfo(sPID, stream_type) == 0)
								stream_types[sPID] = stream_type;
						}
					}

					iterPSIBuf->second->Reset();
				}

				if (PID == PID_PROGRAM_ASSOCIATION_TABLE && bCopyOriginalStream)
					unwritten_pat_buf.clear();
			}

			if (PID == sPID && pes_buffer_len > 0 && (curProgSeqID == sProgSeqID || sProgSeqID == -1))
			{
				if (bPSI)
				{
					if ((dump_ret = FlushPSIBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
						printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
				}
				else
				{
					PES_FILTER_INFO pes_filter_info;
					pes_filter_info.TS.PID = sPID;
					pes_filter_info.TS.stream_id = stream_id;
					pes_filter_info.TS.stream_id_extension = stream_id_extension;
					pes_filter_info.TS.stream_type = stream_types.find(sPID) == stream_types.end() ? -1 : stream_types[sPID];

					if ((dump_ret = FlushPESBuffer(pes_buffer, pes_buffer_len, raw_data_len, pes_filter_info, fw, dumpopt)) < 0)
						printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
				}

				g_dump_status.num_of_processed_payloads++;
				if (dump_ret == 0)
					g_dump_status.num_of_dumped_payloads++;

				pes_buffer_len = 0;
				//pes_hdr_location = ftell(fp) - nRead;
			}
		}

		if (bPSI)
		{
			auto iterPSIBuf = pPSIBufs.find(PID);
			if (iterPSIBuf != pPSIBufs.end() && iterPSIBuf->second->PushTSBuf(ts_pack_idx, buf, index, (unsigned char)g_ts_fmtinfo.packet_size) >= 0)
			{
				// Try to process PSI buffer
				int nProcessPSIRet = iterPSIBuf->second->ProcessPSI(dumpopt);
				// If process PMT result is -1, it means the buffer is too small, don't reset the buffer.
				if (nProcessPSIRet != -1)
				{
					if (sProgSeqID >= 0)
					{
						if (iterPSIBuf->second->table_id == TID_TS_program_map_section)
						{
							bool bNewProgramSeq = false;
							if (mapPSIVersionNumbers.find(PID) != mapPSIVersionNumbers.end())
							{
								if (iterPSIBuf->second->version_number != mapPSIVersionNumbers[PID])
									bNewProgramSeq = true;
							}
							else
								bNewProgramSeq = true;

							if (bNewProgramSeq)
							{
								curProgSeqID++;
								if (curProgSeqID == sProgSeqID && bCopyOriginalStream)
								{
									if (fw != NULL)
									{
										if (unwritten_pat_buf.size() > 0)
										{
											fwrite(unwritten_pat_buf.data(), 1, unwritten_pat_buf.size(), fw);
											unwritten_pat_buf.clear();
										}

										if (unwritten_pmt_buf.size() > 0)
										{
											fwrite(unwritten_pmt_buf.data(), 1, unwritten_pmt_buf.size(), fw);
											unwritten_pmt_buf.clear();
										}

										fwrite(buf, 1, ts_pack_size, fw);
									}
								}

								mapPSIVersionNumbers[PID] = iterPSIBuf->second->version_number;
							}

							if (curProgSeqID == sProgSeqID)
							{
								std::copy(buf, buf + ts_pack_size, std::back_inserter(unwritten_pmt_buf));
							}
						}
						else if (PID == PID_PROGRAM_ASSOCIATION_TABLE)
						{
							int number_of_programs = 0;
							// Check how many program in the current PAT, if number of programs is greater than 1, stop the process
							for (auto iter = pPSIBufs.begin(); iter != pPSIBufs.end(); iter++)
							{
								if ((*iter).second->table_id == TID_TS_program_map_section)
									number_of_programs++;
							}

							if (number_of_programs > 1)
							{
								printf("Can only pick up the part of TS stream with only one program.\n");
								iRet = -1;
								goto done;
							}

							if (bCopyOriginalStream)
								std::copy(buf, buf + ts_pack_size, std::back_inserter(unwritten_pat_buf));
						}
					}

					iterPSIBuf->second->Reset();
				}
				else
				{
					if (sProgSeqID >= 0 && iterPSIBuf->second->table_id == TID_TS_program_map_section && bCopyOriginalStream)
						std::copy(buf, buf + ts_pack_size, std::back_inserter(unwritten_pmt_buf));

					if (sProgSeqID >= 0 && PID == PID_PROGRAM_ASSOCIATION_TABLE && bCopyOriginalStream)
						std::copy(buf, buf + ts_pack_size, std::back_inserter(unwritten_pat_buf));
				}

				if (nProcessPSIRet == 0)
				{
					if (dumpopt & DUMP_MEDIA_INFO_VIEW)
					{
						if (iterPSIBuf->second->table_id == TID_TS_program_map_section)
						{
							unordered_map<unsigned short, unsigned char>& stm_types = ((CPMTBuf*)iterPSIBuf->second)->GetStreamTypes();
							if (prev_PMT_stream_types.find(PID) == prev_PMT_stream_types.end() || stm_types != prev_PMT_stream_types[PID])
							{
								int idxStm = 0;
								size_t num_of_streams = stm_types.size();
								unsigned short cur_PMT_PCR_PID = ((CPMTBuf*)iterPSIBuf->second)->GetPCRPID();

								for (auto& iter : stm_types)
									PID_stat[iter.first].stream_types.insert(iter.second);

								if (cur_PMT_PCR_PID != 0x1FFF)
									PID_stat[cur_PMT_PCR_PID].pcr = 1;

								iter_PID_state->second.pmt = 1;

								// only show the below information only for media info case
								if (!(dumpopt & DUMP_STREAM_INFO_VIEW))
								{
									if (num_of_streams > 0)
									{
										printf("Program(PID:0X%04X)\n", PID);

										for (auto iter = stm_types.cbegin(); iter != stm_types.cend(); iter++, idxStm++)
										{
											if (num_of_streams < 10)
												printf("\tStream#%d, PID: 0X%04X, stm_type: 0X%02X (%s)\n", idxStm, iter->first, iter->second, STREAM_TYPE_NAMEA(iter->second));
											else if (num_of_streams < 100)
												printf("\tStream#%02d, PID: 0X%04X, stm_type: 0X%02X (%s)\n", idxStm, iter->first, iter->second, STREAM_TYPE_NAMEA(iter->second));
											else if (num_of_streams < 1000)
												printf("\tStream#%03d, PID: 0X%04X, stm_type: 0X%02X (%s)\n", idxStm, iter->first, iter->second, STREAM_TYPE_NAMEA(iter->second));
											else
												printf("\tStream#%d, PID: 0X%04X, stm_type: 0X%02X (%s)\n", idxStm, iter->first, iter->second, STREAM_TYPE_NAMEA(iter->second));
										}
										printf("\n");
									}
								}

								prev_PMT_stream_types[PID] = stm_types;
							}
						}

						// For PAT
						if (PID == PID_PROGRAM_ASSOCIATION_TABLE)
						{
							if (psi_process_ctx.bChanged)
							{
								if (!(dumpopt & DUMP_STREAM_INFO_VIEW))
								{
									for (auto iter = pPSIBufs.begin(); iter != pPSIBufs.end(); iter++)
									{
										if ((*iter).second->table_id == TID_TS_program_map_section)
										{
											CPMTBuf* pPMTBuf = (CPMTBuf*)((*iter).second);
											unsigned short nPID = pPMTBuf->GetPID();
											printf("Program Number: %d, %s: 0X%X(%d).\n", pPMTBuf->program_number, pPMTBuf->program_number == 0 ? "Network_PID" : "program_map_PID", nPID, nPID);
										}
									}
								}
							}
						}
					}

					if (iterPSIBuf->second->table_id == TID_TS_program_map_section)
					{
						// Update each PID stream type
						unsigned char stream_type = 0xFF;
						if (((CPMTBuf*)iterPSIBuf->second)->GetPMTInfo(sPID, stream_type) == 0 && sPID != 0)
						{
							stream_types[sPID] = stream_type;

							// Check whether it is a supported stream type or not
							if ((dumpopt&DUMP_PCM) || (dumpopt&DUMP_WAV))
							{
								if (stream_type != HDMV_LPCM_AUDIO_STREAM)
								{
									printf("At present only support dumping PCM/WAV data from LPCM audio stream.\n");
									iRet = -1;
									goto done;
								}
							}
						}
					}
				}
			}
		}

		if (PID == sPID && (payload_unit_start || (!payload_unit_start && pes_buffer_len > 0)) && (curProgSeqID == sProgSeqID || sProgSeqID == -1))
		{
			memcpy(pes_buffer + pes_buffer_len, buf + index, (size_t)g_ts_fmtinfo.packet_size - index);
			pes_buffer_len += g_ts_fmtinfo.packet_size - index;
		}

		ts_pack_idx++;
		g_dump_status.cur_pack_idx++;
	}

	if (pes_buffer_len > 0)
	{
		if (sPID == PID_PROGRAM_ASSOCIATION_TABLE || pPSIBufs.find(sPID) != pPSIBufs.end())
		{
			if ((dump_ret = FlushPSIBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
				printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
		}
		else
		{
			PES_FILTER_INFO pes_filter_info;
			pes_filter_info.TS.PID = sPID;
			pes_filter_info.TS.stream_id = stream_id;
			pes_filter_info.TS.stream_id_extension = stream_id_extension;
			pes_filter_info.TS.stream_type = stream_types.find(sPID) == stream_types.end() ? -1 : stream_types[sPID];
			if ((dump_ret = FlushPESBuffer(
				pes_buffer, pes_buffer_len, raw_data_len, pes_filter_info, fw, dumpopt)) < 0)
			{
				// At the last PES payload, don't show it except the verbose is set
				if (g_verbose_level > 0)
					printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
			}
		}

		g_dump_status.num_of_processed_payloads++;
		if (dump_ret == 0)
			g_dump_status.num_of_dumped_payloads++;
	}

	if (dumpopt&DUMP_WAV)
	{
		// Fill the correct value in WAVE file header
		FinalizeWaveFileBuffer(fw, sPID, stream_types.find(sPID) == stream_types.end() ? -1 : stream_types[sPID]);
	}

	// Finish the dumping w/o any error
	g_dump_status.completed = 1;

	iRet = 0;

	if (dumpopt & DUMP_MEDIA_INFO_VIEW)
	{
		printf("The number of transport packets: %" PRIu64 "\n", g_dump_status.num_of_packs);
		for (auto& iter : PID_stat)
		{
			std::string PID_desc;
			auto iterPIDTable = PID_Allocation_ARIB.find(iter.first);
			if (iterPIDTable != PID_Allocation_ARIB.end())
				PID_desc += iterPIDTable->second;

			if (iter.second.stream_types.size() > 0)
			{
				if (PID_desc.length() > 0)
					PID_desc += "/";

				size_t i = 0;
				for (auto stm_type: iter.second.stream_types)
				{
					if (i > 0)
						PID_desc += ",";
					PID_desc += STREAM_TYPE_NAMEA(stm_type);
				}
			}

			if (iter.second.pcr)
			{
				if (PID_desc.length() > 0)
					PID_desc += "/";

				PID_desc += "PCR";
			}

			if (iter.second.pmt)
			{
				if (PID_desc.length() > 0)
					PID_desc += "/";

				PID_desc += "PMT";
			}

			printf("\tPID: 0x%04X\t\ttransport packet count: %10s - %s\n", iter.first, GetReadableNum(iter.second.count).c_str(), PID_desc.c_str());
		}
	}

done:
	if (fp)
		fclose(fp);
	if (fw)
		fclose(fw);

	if (pes_buffer != NULL)
	{
		delete[] pes_buffer;
		pes_buffer = NULL;
	}

	for (auto iter = pPSIBufs.begin(); iter != pPSIBufs.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}

	return iRet;
}
