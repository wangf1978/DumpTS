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
#pragma once

#include <unordered_map>
#include "systemdef.h"

using namespace std;

struct TS_FORMAT_INFO
{
	MPEG_SYSTEM_TYPE			eMpegSys;
	unsigned short				packet_size;
	unsigned short				num_of_prefix_bytes;
	unsigned short				num_of_suffix_bytes;
	bool						encrypted;
};

// Please see Table 2-28 - Program-specific information
enum PSI_TYPE
{
	PSI_PAT = 0,
	PSI_PMT,
	PSI_NIT,
	PSI_CAT,
	PSI_TSDT,
	PSI_IPMP_CIT,
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
	TID_DIT = 0x7E,
	TID_SIT = 0x7F,
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
	unsigned short GetPCRPID() { return m_PCR_PID; }
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
	int ProcessPSI(int dumpOptions);
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


