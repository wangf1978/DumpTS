#include "StdAfx.h"
#include "PayloadBuf.h"

using namespace std;

extern const char *dump_msg[];
extern unordered_map<std::string, std::string> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;

int CheckRawBufferMediaInfo(unsigned short PID, int stream_type, unsigned char* pBuf, int cbSize)
{
	// At first, check whether the stream type is already decided or not for the current dumped stream.
	if (stream_type == DOLBY_LOSSLESS_AUDIO_STREAM)
	{
		// Try to analyze the MLP audio information.
	}

	return -1;
}

int FlushPSIBuffer(FILE* fw, unsigned char* psi_buffer, int psi_buffer_len, int dumpopt, int &raw_data_len)
{
	int iret = 0;
	// For PSI, store whole payload with pointer field
	raw_data_len = psi_buffer_len;
	if (fw != NULL)
		fwrite(psi_buffer, 1, raw_data_len, fw);

	return iret;
}

int FlushPESBuffer(FILE* fw, unsigned short PID, int stream_type, unsigned char* pes_buffer, int pes_buffer_len, int dumpopt, int &raw_data_len, int stream_id = -1, int stream_id_extension = -1)
{
	int iret = 0;
	raw_data_len = 0;
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
			int pes_hdr_len = pes_buffer[8];
			unsigned char PTS_DTS_flags = (pes_buffer[7] >> 6) & 0x3;
			unsigned char ESCR_flag = (pes_buffer[7] >> 5) & 0x1;
			unsigned char ES_rate_flag = (pes_buffer[7] >> 4) & 0x1;
			unsigned char DSM_trick_mode_flag = (pes_buffer[7] >> 3) & 0x1;
			unsigned char additional_copy_info_flag = (pes_buffer[7] >> 2) & 0x1;
			unsigned char PES_CRC_flag = (pes_buffer[7] >> 1) & 0x1;
			unsigned char pes_hdr_extension_flag = pes_buffer[7] & 0x1;

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
					off += 2;

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

			// Check the media information of current elementary stream
			if (g_params.find("showinfo") != g_params.end())
			{
				if (pes_len == 0)
				{
					raw_data_len = pes_buffer_len - pes_hdr_len - 9;
					CheckRawBufferMediaInfo(PID, stream_type, pes_buffer + pes_hdr_len + 9, raw_data_len);
				}
				else
				{
					raw_data_len = pes_len - 3 - pes_hdr_len;
					CheckRawBufferMediaInfo(PID, stream_type, pes_buffer + pes_hdr_len + 9, raw_data_len);
				}
			}

			if (dumpopt&DUMP_RAW_OUTPUT)
			{
				if (pes_buffer_len < pes_len + 6 || pes_buffer_len < pes_hdr_len + 9)
				{
					iret = -1;
				}
				else if (pes_len == 0)
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
					__int64 pts = GetPTSValue(pes_buffer + 9);
					printf("PktIndex:%d, PTS value is %lld(0X%llX).\r\n", PktIndex++, pts, pts);
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

	return iret;
}

// Dump a stream with specified PID to a pes/es stream
int DumpOneStream()
{
	int iRet = -1;
	FILE *fp = NULL, *fw = NULL;
	int sPID = FILTER_PID;
	int dumpopt = 0;
	unsigned char* pes_buffer = new (std::nothrow) unsigned char[20 * 1024 * 1024];

	unordered_map<unsigned short, CPayloadBuf*> pPSIBufs;
	unordered_map<unsigned short, unsigned char> stream_types;

	pPSIBufs[0] = new CPayloadBuf(PID_PROGRAM_ASSOCIATION_TABLE);

	unsigned char buf[1024];

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

	if (g_params.find("output") != g_params.end())
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
	unsigned long pat_buffer_len = 0;
	int buf_head_offset = g_ts_fmtinfo.num_of_prefix_bytes;
	int ts_pack_size = g_ts_fmtinfo.packet_size;

	if (g_params.find("srcfmt") != g_params.end() && g_params["srcfmt"].compare("m2ts") == 0)
		dumpopt |= DUMP_BD_M2TS;

	int stream_id_extension = -1;
	if (g_params.find("stream_id_extension") != g_params.end())
	{
		stream_id_extension = (int)ConvertToLongLong(g_params["stream_id_extension"]);
	}

	int stream_id = -1;
	if (g_params.find("stream_id") != g_params.end())
	{
		stream_id = (int)ConvertToLongLong(g_params["stream_id"]);
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

	unsigned long ts_pack_idx = 0;
	while (true)
	{
		int nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		unsigned short PID = (buf[buf_head_offset + 1] & 0x1f) << 8 | buf[buf_head_offset + 2];

		bool bPSI = pPSIBufs.find(PID) != pPSIBufs.end() ? true : false;

		// Continue the parsing when PAT/PMT is hit
		if (PID != sPID && !bPSI)
		{
			ts_pack_idx++;
			continue;
		}

		int index = buf_head_offset + 4;
		unsigned char payload_unit_start = buf[buf_head_offset + 1] & 0x40;
		unsigned char adaptation_field_control = (buf[buf_head_offset + 3] >> 4) & 0x03;
		unsigned char discontinuity_counter = buf[buf_head_offset + 3] & 0x0f;

		if (adaptation_field_control & 0x02)
			index += buf[buf_head_offset + 4] + 1;

		if (payload_unit_start)
		{
			if (bPSI)
			{
				if (pPSIBufs.find(PID) != pPSIBufs.end())
				{
					if (pPSIBufs[PID]->ProcessPMT(pPSIBufs) == 0)
					{
						// Update each PID stream type
						unsigned char stream_type = 0xFF;
						if (pPSIBufs[PID]->GetPMTInfo(sPID, stream_type) == 0)
							stream_types[sPID] = stream_type;
					}

					pPSIBufs[PID]->Reset();
				}
			}

			if (PID == sPID)
			{
				if (bPSI)
				{
					if ((dump_ret = FlushPSIBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
						printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
				}
				else
				{
					if ((dump_ret = FlushPESBuffer(fw, 
						PID, stream_types.find(PID) == stream_types.end()?-1: stream_types[PID], 
						pes_buffer, pes_buffer_len, dumpopt, raw_data_len, stream_id, stream_id_extension)) < 0)
						printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
				}

				pes_buffer_len = 0;
				pes_hdr_location = ftell(fp) - nRead;
			}
		}

		if (bPSI)
		{
			if (pPSIBufs[PID]->PushTSBuf(ts_pack_idx, buf, index, g_ts_fmtinfo.packet_size + g_ts_fmtinfo.num_of_prefix_bytes) >= 0)
			{
				// Try to process PSI buffer
				int nProcessPMTRet = pPSIBufs[PID]->ProcessPMT(pPSIBufs);
				// If process PMT result is -1, it means the buffer is too small, don't reset the buffer.
				if (nProcessPMTRet != -1)
					pPSIBufs[PID]->Reset();

				if (nProcessPMTRet == 0)
				{
					// Update each PID stream type
					unsigned char stream_type = 0xFF;
					if (pPSIBufs[PID]->GetPMTInfo(sPID, stream_type) == 0)
						stream_types[sPID] = stream_type;
				}
			}
		}

		if (PID == sPID && (payload_unit_start || !payload_unit_start && pes_buffer_len > 0))
		{
			memcpy(pes_buffer + pes_buffer_len, buf + index, g_ts_fmtinfo.packet_size + g_ts_fmtinfo.num_of_prefix_bytes - index);
			pes_buffer_len += g_ts_fmtinfo.packet_size + g_ts_fmtinfo.num_of_prefix_bytes - index;
		}

		ts_pack_idx++;
	}

	if (sPID == PID_PROGRAM_ASSOCIATION_TABLE || pPSIBufs.find(sPID) != pPSIBufs.end())
	{
		if ((dump_ret = FlushPSIBuffer(fw, pes_buffer, pes_buffer_len, dumpopt, raw_data_len)) < 0)
			printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
	}
	else
	{
		if ((dump_ret = FlushPESBuffer(fw, 
			sPID, stream_types.find(sPID) == stream_types.end() ? -1 : stream_types[sPID],
			pes_buffer, pes_buffer_len, dumpopt, raw_data_len, stream_id, stream_id_extension)) < 0)
			printf(dump_msg[-dump_ret - 1], ftell(fp), ftell(fw));
	}

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

	for (std::unordered_map<unsigned short, CPayloadBuf*>::iterator iter = pPSIBufs.begin(); iter != pPSIBufs.end(); iter++)
	{
		delete iter->second;
		iter->second = NULL;
	}

	return iRet;
}