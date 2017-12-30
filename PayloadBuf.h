#pragma once

#include <unordered_map>

using namespace std;

#define AUDIO_STREAM_ID						0xE0
#define FILTER_PID							0x1400//0x1011//0x1011//0x1400
#define TS_PACKET_SIZE						192

#define DUMP_RAW_OUTPUT						(1<<0)
#define DUMP_BD_M2TS						(1<<1)
#define DUMP_PES_OUTPUT						(1<<2)
#define DUMP_PTS_VIEW						(1<<3)
#define DUMP_PCM							(1<<4)		// Raw LPCM data
#define DUMP_WAV							(1<<5)		// MSFT wave file
#define DUMP_MEDIA_INFO_VIEW				(1<<6)

#define PID_PROGRAM_ASSOCIATION_TABLE		0x0000

#define IS_PES_PAYLOAD(p)					((p)[0] == 0 && (p)[1] == 0 && (p)[2] == 1 && (p)[3] >= 0xBC)

#define MPEG2_VIDEO_STREAM					0x02
#define MPEG4_AVC_VIDEO_STREAM				0x1B
#define SMPTE_VC1_VIDEO_STREAM				0xEA
#define MPEG4_MVC_VIDEO_STREAM				0x20
#define HEVC_VIDEO_STREAM					0x24

#define MPEG1_AUDIO_STREAM					0x03
#define MPEG2_AUDIO_STREAM					0x04
#define AAC_AUDIO_STREAM					0x0F	// ISO/IEC 13818-7 Audio with ADTS transport syntax
#define MPEG4_AAC_AUDIO_STREAM				0x11	// ISO/IEC 14496-3 Audio with the LATM transport syntax as defined in ISO/IEC 14496-3

#define HDMV_LPCM_AUDIO_STREAM				0x80
#define DOLBY_AC3_AUDIO_STREAM				0x81
#define DTS_AUDIO_STREAM					0x82
#define DOLBY_LOSSLESS_AUDIO_STREAM			0x83
#define DD_PLUS_AUDIO_STREAM				0x84

#define DTS_HD_EXCEPT_XLL_AUDIO_STREAM		0x85
#define DTS_HD_XLL_AUDIO_STREAM				0x86
#define DRA_AUDIO_STREAM					0x87
#define DRA_EXTENSION_AUDIO_STREAM			0x88
#define DD_PLUS_SECONDARY_AUDIO_STREAM		0xA1
#define DTS_HD_SECONDARY_AUDIO_STREAM		0xA2

#define STREAM_TYPE_NAMEA(st)	(\
	(st) == MPEG2_VIDEO_STREAM?"MPEG2 Video":(\
	(st) == MPEG4_AVC_VIDEO_STREAM?"MPEG4 AVC Video":(\
	(st) == SMPTE_VC1_VIDEO_STREAM?"VC1 Video":(\
	(st) == MPEG4_MVC_VIDEO_STREAM?"MVC Video":(\
	(st) == HEVC_VIDEO_STREAM?"HEVC Video":(\
	(st) == MPEG1_AUDIO_STREAM?"MPEG1 Audio":(\
	(st) == MPEG2_AUDIO_STREAM?"MPEG2 Audio":(\
	(st) == AAC_AUDIO_STREAM?"AAC Audio":(\
	(st) == MPEG4_AAC_AUDIO_STREAM?"MPEG4 AAC Audio":(\
	(st) == HDMV_LPCM_AUDIO_STREAM?"HDMV LPCM Audio":(\
	(st) == DOLBY_AC3_AUDIO_STREAM?"AC3 Audio":(\
	(st) == DTS_AUDIO_STREAM?"DTS Audio":(\
	(st) == DOLBY_LOSSLESS_AUDIO_STREAM?"Dolby Lossless Audio":(\
	(st) == DD_PLUS_AUDIO_STREAM?"DD+ Audio":(\
	(st) == DTS_HD_EXCEPT_XLL_AUDIO_STREAM?"DTS-HD audio":(\
	(st) == DTS_HD_XLL_AUDIO_STREAM?"DTS-HD Lossless Audio":(\
	(st) == DRA_AUDIO_STREAM?"DRA Audio":(\
	(st) == DRA_EXTENSION_AUDIO_STREAM?"DRA Extension Audio":(\
	(st) == DD_PLUS_SECONDARY_AUDIO_STREAM?"DD+ Secondary Audio":(\
	(st) == DTS_HD_SECONDARY_AUDIO_STREAM?"DTS LBR Audio":"Unknown"))))))))))))))))))))

enum MPEG_SYSTEM_TYPE
{
	MPEG_SYSTEM_UNKNOWN = -1,
	MPEG_SYSTEM_PS = 0,				// MPEG-2 program stream
	MPEG_SYSTEM_TS = 1,				// MPEG-2 (188-byte packet) transport stream
	MPEG_SYSTEM_TTS = 2,			// 192-byte packet transport stream
	MPEG_SYSTEM_TS204 = 3,			// 204-byte packet transport stream
	MPEG_SYSTEM_TS208 = 4,			// 204-byte packet transport stream
	MPEG_SYSTEM_DVD_VIDEO = 5,		// DVD-ROM VOB
	MPEG_SYSTEM_DVD_VR = 6,			// DVD-VR VOB
	MPEG_SYSTEM_BDMV = 7,			// BDMV transport stream
	MPEG_SYSTEM_BDAV = 8,			// BDAV transport stream
	MPEG_SYSTEM_MAX
};

struct TS_FORMAT_INFO
{
	MPEG_SYSTEM_TYPE			eMpegSys;
	unsigned short				packet_size;
	unsigned short				num_of_prefix_bytes;
	unsigned short				num_of_suffix_bytes;
	bool						encrypted;
};

enum PSI_TABLE_ID {
	TID_program_association_section = 0x00,
	TID_conditional_access_section,
	TID_TS_program_map_section,
	TID_TS_description_section,
	TID_ISO_IEC_14496_scene_description_section,
	TID_ISO_IEC_14496_object_descriptor_section,
	TID_Metadata_section,
	TID_IPMP_Control_Information_section,
	TID_Forbidden = 0xFF
};

struct PayloadBufSlice
{
	unsigned long ts_packet_idx;		// The TS packet index of current PES buf slice
	unsigned char start_off;			// The start byte position in TS packet of current PES buf slice
	unsigned char end_off;				// The end byte position in TS packet of current PES buf slice

	PayloadBufSlice(unsigned long idxTSPack, unsigned char offStart, unsigned char offEnd)
		: ts_packet_idx(idxTSPack), start_off(offStart), end_off(offEnd) {}
};

class CPayloadBuf
{
	std::vector<PayloadBufSlice>	slices;
	unsigned char*					buffer;
	unsigned long					buffer_len;
	unsigned long					buffer_alloc_size;

	FILE*							m_fw;
	unsigned char					m_ts_pack_size;
	unsigned short					m_PID;

	// PMT rough information
	unsigned short					m_PCR_PID;
	unordered_map<unsigned short, unsigned char>
									m_stream_types;

public:
	CPayloadBuf(FILE* fw, unsigned char nTSPackSize);
	CPayloadBuf(unsigned short PID);

	~CPayloadBuf();

	int PushTSBuf(unsigned long idxTSPack, unsigned char* pBuf, unsigned char offStart, unsigned char offEnd);
	/*!	@brief Replace the PID with specified PID in the current TS packs */
	int Process(std::unordered_map<int, int>& pid_maps);

	/*!	@breif Process PAT and PMT to get the stream information.
	@retval -1 incompatible buffer
	@retval -2 CRC verification failure
	@retval -3 Unsupported or unimplemented */
	int ProcessPMT(unordered_map<unsigned short, CPayloadBuf*>& pPMTPayloads);

	int GetPMTInfo(unsigned short ES_PID, unsigned char& stream_type);

	void Reset();

	int WriteBack(unsigned long off, unsigned char* pBuf, unsigned long cbSize);
};

inline long long ConvertToLongLong(std::string& str)
{
	size_t idx = 0;
	long long ret = -1LL;
	if (str.length() > 0)
	{
		// Check whether it is a valid PID value or not
		if (str.compare(0, 2, "0x") == 0)	// hex value
		{
			ret = std::stoll(str.substr(2), &idx, 16);
		}
		else if (str.compare(0, 1, "0") == 0)	// oct value
		{
			ret = std::stoll(str.substr(1), &idx, 8);
		}
		else
		{
			ret = std::stoll(str, &idx, 10);
		}
	}

	return idx > 0 ? ret : -1LL;
}

inline long long GetPTSValue(unsigned char* pkt_data)
{
	long long ts;
	ts = ((long long)(pkt_data[0] & 0x0e)) << 29;	//requires 33 bits
	ts |= ((long long)pkt_data[1] << 22);
	ts |= ((long long)pkt_data[2] & 0xfe) << 14;
	ts |= ((long long)pkt_data[3] << 7);
	ts |= ((long long)pkt_data[4] >> 1);
	return ts;
}


