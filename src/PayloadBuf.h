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

#define SESF_TELETEXT_STREAM				0x06
#define TTML_STREAM							0x06

/*
	0x0A						Multi-protocol Encapsulation
	0x0B						DSM-CC U-N Messages
	0x0C						DSM-CC Stream Descriptors
	0x0D						DSM-CC Sections (any type, including private data)
*/
#define DSMCC_TYPE_A						0x0A
#define DSMCC_TYPE_B						0x0B
#define DSMCC_TYPE_C						0x0C
#define DSMCC_TYPE_D						0x0D

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
	(st) == DTS_HD_SECONDARY_AUDIO_STREAM?"DTS LBR Audio":(\
	(st) == SESF_TELETEXT_STREAM?"Teletext, ARIB subtitle or TTML":(\
	(st) == DSMCC_TYPE_A?"DSM-CC Multi-protocol Encapsulation":(\
	(st) == DSMCC_TYPE_B?"DSM-CC DSM-CC U-N Messages":(\
	(st) == DSMCC_TYPE_C?"DSM-CC DSM-CC Stream Descriptors":(\
	(st) == DSMCC_TYPE_D?"DSM-CC SM-CC Sections":"Unknown")))))))))))))))))))))))))

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
	MPEG_SYSTEM_MP4 = 9,			// ISO MP4
	MPEG_SYSTEM_MATROSKA = 10,		// Matroska based file-format
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
protected:
	std::vector<PayloadBufSlice>	slices;
	uint8_t*						buffer;
	uint32_t						buffer_len;
	uint32_t						buffer_alloc_size;

	FILE*							m_fw;
	uint8_t							m_ts_pack_size;
	uint16_t						m_PID;

	// PMT rough information
	uint16_t						m_PCR_PID;
	unordered_map<uint16_t, uint8_t>
									m_stream_types;

public:
	CPayloadBuf(FILE* fw, uint8_t nTSPackSize);
	CPayloadBuf(uint16_t PID);

	~CPayloadBuf();

	int PushTSBuf(uint32_t idxTSPack, uint8_t* pBuf, uint8_t offStart, uint8_t offEnd);
	/*!	@brief Replace the PID with specified PID in the current TS packs */
	int Process(std::unordered_map<int, int>& pid_maps);

	void Reset();

	int WriteBack(unsigned int off, unsigned char* pBuf, unsigned long cbSize);

	unsigned short GetPID() { return m_PID; }
};

class CPSIBuf;

struct PSI_PROCESS_CONTEXT
{
	unordered_map<unsigned short, CPSIBuf*>* pPSIPayloads;
	bool bChanged;		// there are PSI changes since the previous process
};

class CPSIBuf: public CPayloadBuf
{
public:
	uint8_t							table_id;
	uint8_t							version_number;
	uint8_t							section_number;
	uint8_t							last_section_number;

	PSI_PROCESS_CONTEXT*			ctx_psi_process;

public:
	CPSIBuf(PSI_PROCESS_CONTEXT* CtxPSIProcess, unsigned short PID);

	/*!	@breif Process PAT and PMT to get the stream information.
		@retval -1 incompatible buffer
		@retval -2 CRC verification failure
		@retval -3 Unsupported or unimplemented */
	int ProcessPSI();
};

class CPMTBuf : public CPSIBuf
{
public:
	uint16_t						program_number;

public:
	CPMTBuf(PSI_PROCESS_CONTEXT* CtxPSIProcess, unsigned short PID, unsigned short nProgramNumber);

	int GetPMTInfo(unsigned short ES_PID, unsigned char& stream_type);

	unordered_map<unsigned short, unsigned char>& GetStreamTypes();
};

inline long long ConvertToLongLong(std::string& str)
{
	size_t idx = 0;
	long long ret = -1LL;
	try
	{
		if (str.length() > 0)
		{
			// Check whether it is a valid PID value or not
			if (str.compare(0, 2, "0x") == 0)	// hex value
			{
				ret = std::stoll(str.substr(2), &idx, 16);
			}
			else if (str.length() > 1 && str.compare(0, 1, "0") == 0)	// oct value
			{
				ret = std::stoll(str.substr(1), &idx, 8);
			}
			else
			{
				ret = std::stoll(str, &idx, 10);
			}
		}
	}
	catch (...)
	{
	}

	return idx > 0 ? ret : -1LL;
}

inline int64_t GetPTSValue(unsigned char* pkt_data)
{
	int64_t ts;
	ts  = ((int64_t)(pkt_data[0] & 0x0e)) << 29;	//requires 33 bits
	ts |= ((int64_t)pkt_data[1] << 22);
	ts |= ((int64_t)pkt_data[2] & 0xfe) << 14;
	ts |= ((int64_t)pkt_data[3] << 7);
	ts |= ((int64_t)pkt_data[4] >> 1);
	return ts;
}


