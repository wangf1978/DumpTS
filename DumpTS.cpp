 // DumpTS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <memory.h>
#include <string>
#include <map>
#include <unordered_map>
#include <tuple>
#include <io.h>
#include <locale>
#include <algorithm>
#include "crc.h"

using namespace std;

#define AUDIO_STREAM_ID	0xE0
#define FILTER_PID		0x1400//0x1011//0x1011//0x1400
#define TS_PACKET_SIZE	192

#define DUMP_RAW_OUTPUT	(1<<0)
#define DUMP_BD_M2TS	(1<<1)
#define DUMP_PES_OUTPUT	(1<<2)
#define DUMP_PTS_VIEW	(1<<3)

const char *dump_msg[] = {
	"Warning: wrong PES_length field value, read:location, read pos[%d], write pos:[%d].\r\n",
	"Error: a PES packet with a invalid start code will be ignored, read pos[%d], write pos:[%d].\r\n",
	"Error: a PES packet with too small bytes will be ignored, read pos[%d], write pos:[%d].\r\n",
};

const char* dumpparam[] = {"raw", "m2ts", "pes", "ptsview"};

const int   dumpoption[] = {1<<0, 1<<1, 1<<2, 1<<3};

long long GetPTSValue(unsigned char* pkt_data)
{
	long long ts;
	ts  = ((long long)(pkt_data[0]&0x0e))<<29;	//requires 33 bits
	ts |= ((long long)pkt_data[1]<<22);
	ts |= ((long long)pkt_data[2]&0xfe)<<14;
	ts |= ((long long)pkt_data[3]<<7);
	ts |= ((long long)pkt_data[4]>>1);
	return ts;
}

int FlushPESBuffer(FILE* fw, unsigned char* pes_buffer, int pes_buffer_len, int dumpopt, int &raw_data_len)
{
	int iret=0;
	raw_data_len = 0;
	if(pes_buffer_len >= 9)
	{
		if( pes_buffer[0] == 0 &&
			pes_buffer[1] == 0 &&
			pes_buffer[2] == 1 )
		{
			int pes_len = pes_buffer[4]<<8 | pes_buffer[5];
			int pes_hdr_len = pes_buffer[8];

			if(dumpopt&DUMP_RAW_OUTPUT)
			{
				if(pes_buffer_len < pes_len + 6 || pes_buffer_len < pes_hdr_len + 9)
				{
					iret = -1;
				}
				else if(pes_len == 0)
				{
					raw_data_len = pes_buffer_len - pes_hdr_len - 9;
					if (fw != NULL)
						fwrite(pes_buffer + pes_hdr_len + 9, 1, raw_data_len, fw);
				}
				else
				{
					raw_data_len = pes_len - 3 - pes_hdr_len;
					if (fw != NULL)
						fwrite(pes_buffer + pes_hdr_len + 9, 1, raw_data_len, fw);
				}
			}
			else if(dumpopt&DUMP_PES_OUTPUT)
			{
				raw_data_len = pes_len==0?pes_buffer_len:pes_len;
				if (fw != NULL)
					fwrite(pes_buffer, 1, raw_data_len, fw);
			}

			if(dumpopt&DUMP_PTS_VIEW)
			{
				if(pes_buffer_len < pes_len + 6 || pes_buffer_len < pes_hdr_len + 9)
				{
					iret = -1;
				}
				else
				{
					static int PktIndex = 0;
					__int64 pts = GetPTSValue(pes_buffer + 9);
					printf("PktIndex:%d, PTS value is %lld(0X%llX).\r\n", PktIndex++, pts, pts);
				}
			}
		}
		else
		{
			iret = -2;
		}
	}
	else if(pes_buffer_len > 0)
	{
		iret = -3;
	}

	return iret;
}

//
// DumpTS TSSourceFileName DestFileName --pid=0x100 --destpid=0x1011 --srcfmt=m2ts --outputfmt=es/pes/m2ts/ts --showpts
//

unordered_map<std::string, std::string> g_params;

void ParseCommandLine(int argc, char* argv[])
{
	if (argc < 2)
		return;

	g_params.insert({ "input", argv[1] });

	std::string str_arg_prefixes[] = {
		"output", "pid", "destpid", "srcfmt", "outputfmt", "showpts"
	};
	
	for (int iarg = 2; iarg < argc; iarg++)
	{
		std::string strArg = argv[iarg];
		if (strArg.find("--") == 0)
			strArg = strArg.substr(2);

		for (int i = 0; i < sizeof(str_arg_prefixes) / sizeof(str_arg_prefixes[0]); i++)
		{
			if (strArg.find(str_arg_prefixes[i] + "=") == 0)
			{
				string strVal = strArg.substr(str_arg_prefixes[i].length() + 1);
				std::transform(strVal.begin(), strVal.end(), strVal.begin(), ::tolower);
				g_params.insert({ str_arg_prefixes[i],  strVal});
				break;
			}
		}
	}

	unordered_map<std::string, std::string>::const_iterator iter = g_params.cbegin();
	for (; iter != g_params.cend(); iter++)
	{
		printf("%s : %s.\r\n", iter->first.c_str(), iter->second.c_str());
	}

	// Set the default values

	return;
}

long long ConvertToLongLong(std::string& str)
{
	size_t idx = 0;
	long long ret = -1LL;
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

	return idx > 0 ? ret : -1LL;
}

bool VerifyCommandLine()
{
	int ret = 0;

	//
	// For input file
	//

	// Test the source file exists or not
	if (g_params.find("input") == g_params.end())
	{
		printf("Please specify the input ts file path.\r\n");
		return false;
	}

	if ((ret = _access(g_params["input"].c_str(), 4)) != 0)
	{
		printf("Failed to open the input file: %s {error code: %d}.\r\n", g_params["input"].c_str(), errno);
		return false;
	}

	//
	// For output file
	//
	if (g_params.find("output") != g_params.end() && (ret = _access(g_params["output"].c_str(), 2)) != 0)
	{
		if (errno != ENOENT)
		{
			printf("The output file: %s can't be written {error code: %d}.\r\n", g_params["output"].c_str(), errno);
			return false;
		}
	}

	//
	// For PID
	//
	if (g_params.find("pid") != g_params.end())
	{
		long long pid = ConvertToLongLong(g_params["pid"]);
		if (!(pid >= 0 && pid <= 0x1FFF))
		{
			printf("The specified PID: %s is invalid, please make sure it is between 0 to 0x1FFF inclusive.\r\n", g_params["pid"].c_str());
			return false;
		}
	}

	//
	// For Destination PID
	//
	if (g_params.find("destpid") != g_params.end())
	{
		long long pid = ConvertToLongLong(g_params["destpid"]);
		if (!(pid >= 0 && pid <= 0x1FFF))
		{
			printf("The specified Destination PID: %s is invalid, please make sure it is between 0 to 0x1FFF inclusive.\r\n", g_params["destpid"].c_str());
			return false;
		}
	}

	//
	// for source format
	//


	// TODO...

	return true;
}

void PrintHelp()
{
	printf("Usage: DumpTS.exe TSSourceFileName [OPTION]...\r\n");
	printf("\t--output\t\tThe output dumped file path\r\n");
	printf("\t--pid\t\t\tThe PID of dumped stream\r\n");
	printf("\t--destpid\t\tThe PID of source stream will be placed with this PID\r\n");
	printf("\t--srcfmt\t\tThe source TS format, including: ts, m2ts, if it is not specified, find the sync-word to decide it\r\n");
	printf("\t--outputfmt\t\tThe destination dumped format, including: ts, m2ts, pes and es\r\n");
	printf("\t--showpts\t\tPrint the pts of every elementary stream packet\n");

	printf("Examples:\r\n");
	printf("DumpTS c:\\00001.m2ts --output=c:\\00001.hevc --pid=0x1011 --srcfmt=m2ts --outputfmt=es --showpts\r\n");
	printf("DumpTS c:\\test.ts --output=c:\\00001.m2ts --pid=0x100 --destpid=0x1011 --srcfmt=ts --outputfmt=m2ts\r\n");

	return;
}

// Dump a stream with specified PID to a pes/es stream
int DumpOneStream()
{
	int iRet = -1;
	FILE *fp = NULL, *fw = NULL;
	int sPID = FILTER_PID;
	int dumpopt = 0;
	unsigned char* pes_buffer = new (std::nothrow) unsigned char[20 * 1024 * 1024];

	unsigned char buf[TS_PACKET_SIZE];

	if (g_params.find("pid") == g_params.end())
	{
		printf("No PID is specified, nothing to do.\r\n");
		goto done;
	}

	sPID = (int)ConvertToLongLong(g_params["pid"]);
	
	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if(g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	size_t pes_hdr_location;
	int raw_data_len, dump_ret;
	unsigned long pes_buffer_len = 0;
	int buf_head_offset = 0;	// dumpopt&DUMP_BD_M2TS ? 4 : 0;
	int ts_pack_size = TS_PACKET_SIZE - 4;	// dumpopt&DUMP_BD_M2TS ? TS_PACKET_SIZE : TS_PACKET_SIZE - 4;

	if (g_params.find("srcfmt") != g_params.end() && g_params["srcfmt"].compare("m2ts") == 0)
	{
		buf_head_offset = 4;
		ts_pack_size = TS_PACKET_SIZE;
		dumpopt |= DUMP_BD_M2TS;
	}

	// Make sure the dump option
	if (g_params.find("outputfmt") != g_params.end())
	{
		if (g_params["outputfmt"].compare("es") == 0)
			dumpopt |= DUMP_RAW_OUTPUT;
		else if (g_params["output"].compare("pes") == 0)
			dumpopt |= DUMP_PES_OUTPUT;
	}

	if (g_params.find("showpts") != g_params.end())
	{
		dumpopt |= DUMP_PTS_VIEW;
	}

	while (true)
	{
		int nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		unsigned short PID = (buf[buf_head_offset + 1] & 0x1f) << 8 | buf[buf_head_offset + 2];
		if (PID != sPID)
			continue;

		int index = buf_head_offset + 4;
		unsigned char payload_unit_start = buf[buf_head_offset + 1] & 0x40;
		unsigned char adaptation_field_control = (buf[buf_head_offset + 3] >> 4) & 0x03;
		unsigned char discontinuity_counter = buf[buf_head_offset + 3] & 0x0f;

		if (payload_unit_start)
		{
			if ((dump_ret = FlushPESBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
				printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
			pes_buffer_len = 0;
			pes_hdr_location = ftell(fp) - nRead;
		}

		if (adaptation_field_control & 0x02)
			index += buf[buf_head_offset + 4] + 1;

		if (payload_unit_start || !payload_unit_start && pes_buffer_len > 0)
		{
			memcpy(pes_buffer + pes_buffer_len, buf + index, TS_PACKET_SIZE - index);
			pes_buffer_len += TS_PACKET_SIZE - index;
		}
	}

	if ((dump_ret = FlushPESBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
		printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));

	iRet = 0;

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

	return iRet;
}

// Dump a partial TS
int DumpPartialTS()
{
	return -1;
}

struct PayloadBufSlice
{
	unsigned long ts_packet_idx;		// The TS packet index of current PES buf slice
	unsigned char start_off;			// The start byte position in TS packet of current PES buf slice
	unsigned char end_off;				// The end byte position in TS packet of current PES buf slice

	PayloadBufSlice(unsigned long idxTSPack, unsigned char offStart, unsigned char offEnd)
		: ts_packet_idx(idxTSPack), start_off(offStart), end_off(offEnd) {}
};

#define IS_PES_PAYLOAD(p)			((p)[0] == 0 && (p)[1] == 0 && (p)[2] == 1 && (p)[3] >= 0xBC)

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

struct PayloadBuf
{
	std::vector<PayloadBufSlice> slices;
	unsigned char* buffer;
	unsigned long buffer_len;

	FILE* m_fw;
	unsigned char m_ts_pack_size;

	PayloadBuf(FILE* fw, unsigned char nTSPackSize)
		: buffer_len(0)
		, m_fw(fw)
		, m_ts_pack_size(nTSPackSize)
	{
		buffer = new (std::nothrow) unsigned char[20 * 1024 * 1024];
	}

	~PayloadBuf()
	{
		if (buffer != NULL)
		{
			delete[] buffer;
			buffer = NULL;
		}
	}

	int PushTSBuf(unsigned long idxTSPack, unsigned char* pBuf, unsigned char offStart, unsigned char offEnd)
	{
		slices.emplace_back(idxTSPack, offStart, offEnd);

		if (buffer != NULL)
			memcpy(buffer + buffer_len, pBuf + offStart, offEnd - offStart);

		buffer_len += offEnd - offStart;

		return 0;
	}

	int Process(std::unordered_map<int, int>& pid_maps, FILE* fw)
	{
		unsigned long ulMappedSize = 0;
		unsigned char* pBuf = buffer;
		// Check the current payload buffer is a PSI buffer or not.
		if (buffer_len >= 4 && !IS_PES_PAYLOAD(buffer))
		{
			unsigned char pointer_field = *pBuf;
			unsigned char table_id;
			ulMappedSize++;
			ulMappedSize += pointer_field;

			if (ulMappedSize < buffer_len)
				table_id = pBuf[ulMappedSize];
			else
				return -1;

			if (ulMappedSize + 3 > buffer_len)
				return -1;

			unsigned char* pSectionStart = &pBuf[ulMappedSize];
			unsigned long ulSectionStart = ulMappedSize;

			unsigned short section_length = (pBuf[ulMappedSize + 1] << 8 | pBuf[ulMappedSize + 2]) & 0XFFF;

			// The maximum number of bytes in a section of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 defined PSI table is
			// 1024 bytes. The maximum number of bytes in a private_section is 4096 bytes.
			// The DSMCC section data is also 4096 (table_id from 0x38 to 0x3F)
			if (section_length > ((pBuf[ulMappedSize] >= 0x40 && pBuf[ulMappedSize] <= 0xFE ||
				pBuf[ulMappedSize] >= 0x38 && pBuf[ulMappedSize] <= 0x3F) ? 4093 : 1021))
				return -1;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

			if (ulMappedSize + 3 + section_length > buffer_len)
				return -1;	// RET_CODE_BUFFER_TOO_SMALL;

			unsigned char section_syntax_indicator = (pBuf[ulMappedSize + 1] >> 7) & 0x01;

			if ((table_id == TID_program_association_section || table_id == TID_TS_program_map_section) && !section_syntax_indicator)
				return -1;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

			if (ulMappedSize + 8 > buffer_len)
				return -1;

			ulMappedSize += 8;

			bool bChanged = false;

			// Check the PID in PAT and PMT
			if (table_id == TID_program_association_section)
			{
				// 4 bytes of CRC32, 4 bytes of PAT entry
				while(ulMappedSize + 4 + 4 <= 3 + section_length + ulSectionStart)
				{
					unsigned short PID = (pBuf[ulMappedSize + 2] & 0x1f) << 8 | pBuf[ulMappedSize + 3];

					if (pid_maps.find(PID) != pid_maps.end())
					{
						pBuf[ulMappedSize + 2] &= 0xE0;
						pBuf[ulMappedSize + 2] |= ((pid_maps[PID] >> 8) & 0x1F);
						pBuf[ulMappedSize + 3] = pid_maps[PID] & 0xFF;
						
						// Update the original TS pack according to buffer offset and value
						WriteBack(ulMappedSize + 2, &pBuf[ulMappedSize + 2], 2);

						bChanged = true;
					}

					ulMappedSize += 4;
				}
			}
			else if (table_id == TID_TS_program_map_section)
			{
				if (ulMappedSize + 4 > buffer_len)
					return -1;

				// Change PCR_PID
				unsigned short PID = (pBuf[ulMappedSize] & 0x1f) << 8 | pBuf[ulMappedSize + 1];
				if (pid_maps.find(PID) != pid_maps.end())
				{
					pBuf[ulMappedSize] &= 0xE0;
					pBuf[ulMappedSize] |= ((pid_maps[PID] >> 8) & 0x1F);
					pBuf[ulMappedSize + 1] = pid_maps[PID] & 0xFF;

					// Update the original TS pack according to buffer offset and value
					WriteBack(ulMappedSize, &pBuf[ulMappedSize], 2);
				}

				unsigned short program_info_length = (pBuf[ulMappedSize + 2] & 0xF) << 8 | pBuf[ulMappedSize + 3];
				ulMappedSize += 4;

				if (ulMappedSize + program_info_length > buffer_len)
					return -1;

				ulMappedSize += program_info_length;

				// Reserve 4 bytes of CRC32 and 5 bytes of basic ES info (stream_type, reserved, elementary_PID, reserved and ES_info_length)
				while (ulMappedSize + 5 + 4 <= 3 + section_length + ulSectionStart)
				{
					unsigned char stream_type = pBuf[ulMappedSize];
					PID = (pBuf[ulMappedSize + 1] & 0x1f) << 8 | pBuf[ulMappedSize + 2];
					unsigned short ES_info_length = (pBuf[ulMappedSize + 3] & 0xF) << 8 | pBuf[ulMappedSize + 4];

					if (pid_maps.find(PID) != pid_maps.end())
					{
						pBuf[ulMappedSize + 1] &= 0xE0;
						pBuf[ulMappedSize + 1] |= ((pid_maps[PID] >> 8) & 0x1F);
						pBuf[ulMappedSize + 2] = pid_maps[PID] & 0xFF;

						// Update the original TS pack according to buffer offset and value
						WriteBack(ulMappedSize + 1, &pBuf[ulMappedSize+ 1], 2);

						bChanged = true;
					}

					ulMappedSize += 5;
					ulMappedSize += ES_info_length;
				}
			}

			if (bChanged)
			{
				// Recalculate the CRC32 value
				F_CRC_InicializaTable();
				crc crc_val = F_CRC_CalculaCheckSum(&pBuf[ulSectionStart], (uint16_t)(ulMappedSize - ulSectionStart));
				pBuf[ulMappedSize] = (crc_val >> 24) & 0xFF;
				pBuf[ulMappedSize + 1] = (crc_val >> 16) & 0xFF;
				pBuf[ulMappedSize + 2] = (crc_val >> 8) & 0xFF;
				pBuf[ulMappedSize + 3] = (crc_val) & 0xFF;

				WriteBack(ulMappedSize, &pBuf[ulMappedSize], 4);
			}
		}

		return 0;
	}

	void Reset()
	{
		slices.clear();
		buffer_len = 0;
	}

	int WriteBack(unsigned long off, unsigned char* pBuf, unsigned long cbSize)
	{
		int iRet = -1;
		unsigned long cbRead = 0;
		// Find which TS pack the corresponding bytes are written into TS pack buffer
		for (std::vector<PayloadBufSlice>::iterator iter = slices.begin(); iter != slices.end(); iter++)
		{
			if (off >= cbRead && off < cbRead + iter->end_off - iter->start_off)
			{
				const unsigned long cbWritten = std::min(cbSize, cbRead + iter->end_off - iter->start_off - off);

				if (m_fw != NULL)
				{
					// Record the backup position of file
					long long backup_pos = _ftelli64(m_fw);

					_fseeki64(m_fw, iter->ts_packet_idx*m_ts_pack_size + iter->start_off + off - cbRead, SEEK_SET);
					if (fwrite(pBuf, 1, cbWritten, m_fw) != cbWritten)
					{
						printf("Failed to write back the bytes into destination file.\r\n");
						_fseeki64(m_fw, backup_pos, SEEK_SET);
						goto done;
					}

					// Restore the original file position
					_fseeki64(m_fw, backup_pos, SEEK_SET);
				}

				off += cbWritten;
				cbSize -= cbWritten;
				pBuf += cbWritten;
			}
			
			if (off + cbSize <= cbRead)
				break;

			cbRead += iter->end_off - iter->start_off;
		}

		iRet = 0;
		
	done:
		return iRet;
	}
};

// Refactor TS, for example, change PID, PTS, or changing from ts to m2ts and so on
int RefactorTS()
{
	// At present, only support the below operation
	// Src PID#1 -> Dest PID#1
	// Src PID#2 -> Dest PID#2
	// ...
	// Add 30 bits of ATC clock time at the beginning of 188 bytes

	int iRet = -1;
	unsigned long ulATCTime = 0x1951D2E;
	bool bAppendATCTime = false;
	unordered_map<int, int> pid_maps;
	unsigned char buf[TS_PACKET_SIZE];
	FILE *fp = NULL, *fw = NULL;
	unordered_map<unsigned short, PayloadBuf*> pPayloadBufs;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if (g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	if (g_params.find("srcfmt") == g_params.end())
	{
		// Check its extension name
		char szExt[_MAX_EXT];
		memset(szExt, 0, sizeof(szExt));
		_splitpath_s(g_params["input"].c_str(), NULL, 0, NULL, 0, NULL, 0, szExt, _MAX_EXT);

		if (_stricmp(szExt, ".ts") == 0)
			g_params["srcfmt"] = "ts";
		else if (_stricmp(szExt, ".m2ts") == 0)
			g_params["srcfmt"] = "m2ts";
		else
		{
			// Implement it later, need scan the current TS, and decide which kind of TS stream it is.
			goto done;
		}
	}

	if (g_params.find("outputfmt") == g_params.end())
	{
		// Assume the output format is the same with input format
		g_params["outputfmt"] = g_params["srcfmt"];
	}

	// Check whether to add 30bits of ATC time at the beginning of each 188 TS packet
	if (g_params["outputfmt"].compare("m2ts") == 0 && g_params["srcfmt"].compare("ts") == 0)
		bAppendATCTime = true;

	if (g_params.find("pid") != g_params.end() && g_params.find("destpid") != g_params.end())
	{
		// Set up the PID maps.
		// Assume every PIDs are separated with ',' or ';'
		const char* p = g_params["pid"].c_str(), *s = p;
		std::vector<int> src_pids;
		
		for(;;)
		{
			if (*p == ',' || *p == ';' || *p == '\0')
			{
				std::string strPID(s, p - s);
				long long llPID = ConvertToLongLong(strPID);
				if (llPID >= 0 && llPID <= 0x1FFFLL)
					src_pids.push_back((int)llPID);
				else
					src_pids.push_back(-1);

				if (*p == '\0')
					break;

				s = p + 1;
			}

			p++;
		}

		s = p = g_params["destpid"].c_str();
		std::vector<int> dest_pids;
		
		for (;;)
		{
			if (*p == ',' || *p == ';' || *p == '\0')
			{
				std::string strPID(s, p - s);
				long long llPID = ConvertToLongLong(strPID);
				if (llPID >= 0 && llPID <= 0x1FFFLL)
					dest_pids.push_back((int)llPID);
				else
					dest_pids.push_back(-1);

				if (*p == '\0')
					break;

				s = p + 1;
			}

			p++;
		}

		for (size_t i = 0; i < std::max(src_pids.size(), dest_pids.size()); i++)
		{
			if (src_pids[i] == -1)
				continue;

			if (dest_pids[i] == -1)
				continue;

			pid_maps[src_pids[i]] = dest_pids[i];
		}
	}

	unsigned long pes_buffer_len = 0;
	unsigned char buf_head_offset = 0;	// dumpopt&DUMP_BD_M2TS ? 4 : 0;
	unsigned char ts_pack_size = TS_PACKET_SIZE - 4;	// dumpopt&DUMP_BD_M2TS ? TS_PACKET_SIZE : TS_PACKET_SIZE - 4;

	if (g_params.find("srcfmt") != g_params.end() && g_params["srcfmt"].compare("m2ts") == 0)
		ts_pack_size = TS_PACKET_SIZE;

	unsigned char dest_ts_pack_size = TS_PACKET_SIZE - 4;
	if (g_params.find("outputfmt") != g_params.end() && g_params["outputfmt"].compare("m2ts") == 0)
	{
		buf_head_offset = 4;
		dest_ts_pack_size = TS_PACKET_SIZE;
	}

	unsigned long ts_pack_idx = 0;
	while (true)
	{
		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		// Try to append ATC time
		if (dest_ts_pack_size == 192 && ts_pack_size == 188)
		{
			memmove(buf + 4, buf, 188);

			if (bAppendATCTime)
			{
				buf[0] = (ulATCTime >> 24) & 0xFF;
				buf[1] = (ulATCTime >> 16) & 0xFF;
				buf[2] = (ulATCTime >> 8) & 0xFF;
				buf[3] = ulATCTime & 0xFF;

				ulATCTime += 0x1F2;			// Fixed value at present, later we may support the accurate ATC time
				ulATCTime &= 0x3FFFFFFF;
			}
			else
				*((unsigned long*)(&buf[0])) = 0;
		}
		else if (dest_ts_pack_size == 188 && ts_pack_size == 192)
		{
			memmove(buf, buf + 4, 188);
		}
		else
		{
			printf("Don't know how to process {%s(), %s: %d}.\n", __FUNCTION__, __FILE__, __LINE__);
			break;
		}

		// Try to change PID
		unsigned short PID = (buf[buf_head_offset + 1] & 0x1f) << 8 | buf[buf_head_offset + 2];
		if (pid_maps.find(PID) != pid_maps.end())
		{
			buf[buf_head_offset + 1] &= 0xE0;
			buf[buf_head_offset + 1] |= ((pid_maps[PID] >> 8) & 0x1F);
			buf[buf_head_offset + 2] = pid_maps[PID] & 0xFF;
		}

		if (fw != NULL)
		{
			if (fwrite(buf, 1, dest_ts_pack_size, fw) < dest_ts_pack_size)
			{
				printf("Failed to write %d bytes into output file.\n", dest_ts_pack_size);
				break;
			}
		}

		int index = buf_head_offset + 4;
		unsigned char payload_unit_start = buf[buf_head_offset + 1] & 0x40;
		unsigned char adaptation_field_control = (buf[buf_head_offset + 3] >> 4) & 0x03;
		unsigned char discontinuity_counter = buf[buf_head_offset + 3] & 0x0f;

		if (pPayloadBufs.find(PID) != pPayloadBufs.end())
		{
			if (payload_unit_start)
			{
				pPayloadBufs[PID]->Process(pid_maps, fw);
				pPayloadBufs[PID]->Reset();
			}
		}
		else
			pPayloadBufs[PID] = new PayloadBuf(fw, dest_ts_pack_size);

		if (adaptation_field_control & 0x02)
			index += buf[buf_head_offset + 4] + 1;

		if (payload_unit_start || !payload_unit_start && pes_buffer_len > 0)
		{
			pPayloadBufs[PID]->PushTSBuf(ts_pack_idx, buf, index, dest_ts_pack_size);
		}

		ts_pack_idx++;
	}

	if (feof(fp) && dest_ts_pack_size == 192 && ts_pack_size == 188 && (ts_pack_idx % 32) != 0)
	{
		// Fill the NULL packet to be aligned with 6K
		int padding_pack_count = 32 - (ts_pack_idx % 32);
		for (int i = 0; i < padding_pack_count; i++)
		{
			memset(buf + 4, 0xFF, 188);
			buf[4] = 0x47;
			buf[5] = 0x1F;
			buf[6] = 0xFF;
			buf[7] = 0x10;

			if (bAppendATCTime)
			{
				buf[0] = (ulATCTime >> 24) & 0xFF;
				buf[1] = (ulATCTime >> 16) & 0xFF;
				buf[2] = (ulATCTime >> 8) & 0xFF;
				buf[3] = ulATCTime & 0xFF;

				ulATCTime += 0x1F2;			// Fixed value at present, later we may support the accurate ATC time
				ulATCTime &= 0x3FFFFFFF;
			}
			else
				*((unsigned long*)(&buf[0])) = 0;

			if (fw != NULL)
			{
				if (fwrite(buf, 1, dest_ts_pack_size, fw) < dest_ts_pack_size)
				{
					printf("Failed to write %d bytes into output file.\n", dest_ts_pack_size);
					break;
				}
			}
		}
	}

	for (std::unordered_map<unsigned short, PayloadBuf*>::iterator iter = pPayloadBufs.begin(); iter != pPayloadBufs.end(); iter++)
	{
		if (iter->second != NULL)
			iter->second->Process(pid_maps, fw);
	}

	iRet = 0;

done:
	if (fp)
		fclose(fp);
	if (fw)
		fclose(fw);

	for (std::unordered_map<unsigned short, PayloadBuf*>::iterator iter = pPayloadBufs.begin(); iter != pPayloadBufs.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}

	return iRet;
}

int main(int argc, char* argv[])
{
	int nDumpRet = 0;

	if(argc < 2)
	{
		PrintHelp();
		return 0;
	}

	ParseCommandLine(argc, argv);

	if (VerifyCommandLine() == false)
	{
		PrintHelp();
		return 0;
	}

	//
	// Check what dump case should be gone through
	//

	// If the output format is ES/PES, and PID is also specified, go into the DumpOneStream mode
	if (g_params.find("pid") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];
		std::string& str_pid = g_params["pid"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0))
		{
			nDumpRet = DumpOneStream();
		}
		else if (str_output_fmt.compare("ts") == 0 || str_output_fmt.compare("m2ts") == 0)
		{
			if (g_params.find("destpid") == g_params.end())
				nDumpRet = DumpPartialTS();
			else
				nDumpRet = RefactorTS();
		}
	}
	else
	{
		nDumpRet = RefactorTS();
	}

	return 0;
}

