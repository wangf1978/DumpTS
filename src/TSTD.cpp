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
#include <stdio.h>
#include "PayloadBuf.h"
#include "AMRingBuffer.h"
#include <algorithm>
#include "Syncer.h"
#include "DataUtil.h"
#include "system_13818_1.h"

extern map<std::string, std::string, CaseInsensitiveComparator> g_params;

// option, 1: only PCR; 2: PCR + video; 3: PCR + audio; 4: PCR + all elementary streams
int ShowPCR(int option)
{
	int iRet = -1;
	uint8_t header_offset = 0;
	unsigned long ts_pack_idx = 0;
	unsigned char ts_pack_size = TS_PACKET_SIZE;
	unsigned char buf[TS_PACKET_SIZE];
	FILE *fp = NULL;
	size_t nRead = 0;
	uint8_t packet_start_code_prefix[3] = { 0 };
	uint8_t stream_id = 0;
	std::map<unsigned short, AMLinearRingBuffer> ring_buffers;
	std::map<unsigned short, std::tuple<uint32_t/*start ATC time*/, uint32_t/*AU size(how many TS packs)*/, uint32_t/*AU idx*/>> filter_stat;
	int64_t prev_PCR_byte_position = INT64_MAX;
	uint64_t prev_PCR = UINT64_MAX;
	uint32_t prev_ATC = UINT32_MAX;
	uint64_t max_diff_diff_PCR_ATC = 0;
	int64_t max_transport_rate = 0;
	int64_t avg_transport_rate = 0;
	std::map<uint16_t, uint64_t> first_PCR_values;
	uint64_t min_ts = UINT64_MAX;
	uint64_t atc_sum = 0;
	uint32_t atc_tm = UINT32_MAX;
	uint64_t total_processed_bytes = 0;

	std::vector<std::tuple<uint32_t, uint64_t>> SPN_PCR;
	std::vector<std::tuple<uint32_t, uint32_t>> SPN_ATC;
	std::map<uint16_t, std::vector<std::tuple<uint32_t, uint64_t, uint64_t>>> STM_SPN_PTS_DTS;
	std::map<uint16_t, 
		std::tuple<uint32_t/*start_TS_pack_index with payload_unit_start_indicator*/, 
				   uint32_t/*current TS pack_index*/,
				   uint32_t/*total counts of TS packs*/,
				   uint64_t/*PTS*/,
				   uint64_t/*DTS*/,
				   bool/*is elementary stream or not*/>> stream_pack_info;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	nRead = fread(buf, 1, TS_PACKET_SIZE - 4, fp);
	if (nRead < TS_PACKET_SIZE - 4)
	{
		printf("The TS stream file is too small.\n");
		goto done;
	}

	if (buf[0] == 0x47)
	{
		header_offset = 0;
		ts_pack_size = TS_PACKET_SIZE - 4;
	}
	else if (buf[4] == 0x47)
	{
		header_offset = 4;
		ts_pack_size = TS_PACKET_SIZE;
	}
	else
	{
		printf("Unsupported transport stream.\n");
		goto done;
	}

	// reset the file read pointer
	FSEEK64(fp, 0, SEEK_SET);
	while (true)
	{
		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		total_processed_bytes += nRead;

		if (buf[header_offset] != 0x47)
		{
			printf("Seems to hit an wrong TS packet.\n");
			break;
		}

		uint16_t PID = ((buf[header_offset + 1] & 0x1F) << 8) | (buf[header_offset + 2]);
		uint16_t adaptation_field_control = (buf[header_offset + 3] >> 4) & 0x3;
		uint8_t  payload_unit_start_indicator = (buf[header_offset + 1] >> 6) & 0x1;
		uint8_t  continuity_counter = (buf[header_offset + 4] & 0xF);

		if (header_offset == 4)
		{
			uint32_t diff = 0;
			uint32_t arrive_time = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
			if (atc_tm != UINT32_MAX)
			{
				if (arrive_time > atc_tm)
					diff = (int32_t)(arrive_time - atc_tm);
				else
					diff = 0x40000000 + arrive_time - atc_tm;
			}

			atc_tm = arrive_time;
			atc_sum += (uint64_t)diff;

			if (option == 5)
				SPN_ATC.emplace_back(ts_pack_idx, atc_tm);
		}

		if (PID >= 0x10 && PID <= 0x1FFE)
		{
			AMLinearRingBuffer lrb = NULL;
			auto iter = ring_buffers.find(PID);
			if (payload_unit_start_indicator == 1)
			{
				auto iterStmPackInfo = stream_pack_info.find(PID);
				
				// submit the previous pts and dts information
				if (iterStmPackInfo != stream_pack_info.end())
				{
					if (std::get<5>(iterStmPackInfo->second))
					{
						auto& curr_stm_spn_pts_dts = STM_SPN_PTS_DTS[PID];
						if (std::get<3>(iterStmPackInfo->second) != UINT64_MAX)
						{
							curr_stm_spn_pts_dts.emplace_back(
								std::get<0>(iterStmPackInfo->second),
								std::get<3>(iterStmPackInfo->second),
								std::get<4>(iterStmPackInfo->second)
							);

							if (std::get<0>(iterStmPackInfo->second) != std::get<1>(iterStmPackInfo->second))
							{
								curr_stm_spn_pts_dts.emplace_back(
									std::get<1>(iterStmPackInfo->second),
									std::get<3>(iterStmPackInfo->second),
									std::get<4>(iterStmPackInfo->second)
								);
							}
						}
					}

					std::get<0>(iterStmPackInfo->second) = ts_pack_idx;	// the start TS pack index
					std::get<1>(iterStmPackInfo->second) = ts_pack_idx;	// the current TS pack index
					std::get<2>(iterStmPackInfo->second) = 1;			// the total count of TS packs for this AU
					std::get<3>(iterStmPackInfo->second) = UINT64_MAX;
					std::get<4>(iterStmPackInfo->second) = UINT64_MAX;
				}
				else
				{
					iterStmPackInfo = stream_pack_info.insert(
						std::make_pair(PID, std::make_tuple(ts_pack_idx, ts_pack_idx, 1, UINT64_MAX, UINT64_MAX, false))).first;
				}

				if (iter != ring_buffers.end())
				{
					int read_buf_len = 0;
					lrb = iter->second;
					uint8_t* pes_buf = AM_LRB_GetReadPtr(lrb, &read_buf_len);

					if (read_buf_len > 9 && pes_buf[0] == 0 && pes_buf[1] == 0 && pes_buf[2] == 0x01)
					{
						stream_id = pes_buf[3];
						uint16_t PES_packet_length = (pes_buf[4] << 8) | pes_buf[5];

						if (stream_id != SID_PROGRAM_STREAM_MAP
							&& stream_id != SID_PADDING_STREAM
							&& stream_id != SID_PRIVATE_STREAM_2
							&& stream_id != SID_ECM
							&& stream_id != SID_EMM
							&& stream_id != SID_PROGRAM_STREAM_DIRECTORY
							&& stream_id != SID_DSMCC_STREAM
							&& stream_id != SID_H222_1_TYPE_E)
						{
							uint8_t reserved_0 = (pes_buf[6] >> 6) & 0x3;
							uint8_t PES_scrambling_control = (pes_buf[6] >> 4) & 0x3;

							uint8_t PTS_DTS_flags = (pes_buf[7] >> 6) & 0x3;
							uint8_t PES_header_data_length = pes_buf[8];

							uint64_t pts = UINT64_MAX, dts = UINT64_MAX, start_atc_tm = UINT64_MAX;

							if ((PTS_DTS_flags == 0x2 && ((pes_buf[9] >> 4) & 0xF) == 0x2 && read_buf_len > 14 && PES_header_data_length >= 5) ||
								(PTS_DTS_flags == 0x3 && ((pes_buf[9] >> 4) & 0xF) == 0x3 && read_buf_len > 19 && PES_header_data_length >= 10))
							{
								pts = 0;
								pts = (pes_buf[9] >> 1) & 0x7;
								pts = pts << 15;
								pts |= (pes_buf[10] << 7) | (pes_buf[11] >> 1);
								pts = pts << 15;
								pts |= (pes_buf[12] << 7) | (pes_buf[13] >> 1);

								std::get<3>(iterStmPackInfo->second) = pts;

								if (PTS_DTS_flags == 0x3)
								{
									dts = 0;
									dts = (pes_buf[14] >> 1) & 0x7;
									dts = dts << 15;
									dts |= (pes_buf[15] << 7) | (pes_buf[16] >> 1);
									dts = dts << 15;
									dts |= (pes_buf[17] << 7) | (pes_buf[18] >> 1);

									std::get<4>(iterStmPackInfo->second) = dts;
								}

								std::get<5>(iterStmPackInfo->second) = true;

								start_atc_tm = std::get<0>(filter_stat[PID]);

								if (option == 4 || (option == 2 && (stream_id & 0xF0) == 0xE0) || (option == 3 && (stream_id & 0xC0) == 0xC0))
								{
									if (PTS_DTS_flags == 0x3)
									{
										printf("%sPES_PID: 0X%04X PTS(base: %10" PRIu64 "(90KHZ),           %13" PRIu64 "(27MHZ), %10" PRIu64 ".%03" PRIu64 "(ms)),\n"
											 "                    DTS(base: %10" PRIu64 "(90KHZ)            %13" PRIu64 "(27MHZ), %10" PRIu64 ".%03" PRIu64 "(ms)), ATC interval %" PRIu32 ", %" PRIu64 ".%03" PRIu64 "(ms).\n",
											(stream_id & 0xF0) == 0xE0 ? "[<V]" : (
											(stream_id & 0xC0) == 0xC0 ? "[<A]" : "    "),
											PID, pts, pts * 300, pts / 90, pts * 1000 / 90 % 1000, dts, dts * 300, dts / 90, dts * 1000 / 90 % 1000,
											std::get<1>(filter_stat[PID]), (atc_tm - start_atc_tm) / 27000, (atc_tm - start_atc_tm) / 27 % 1000);
									}
									else
									{
										printf("%sPES_PID: 0X%04X PTS(base: %10" PRIu64 "(90KHZ),           %13" PRIu64 "(27MHZ), %10" PRIu64 ".%03" PRIu64 "(ms)), ATC interval %" PRIu32 ", %" PRIu64 ".%03" PRIu64 "(ms).\n",
											(stream_id & 0xF0) == 0xE0 ? "[<V]" : (
											(stream_id & 0xC0) == 0xC0 ? "[<A]" : "    "),
											PID, pts, pts * 300, pts / 90, pts * 1000 / 90 % 1000,
											std::get<1>(filter_stat[PID]), (atc_tm - start_atc_tm) / 27000, (atc_tm - start_atc_tm) / 27 % 1000);
									}
								}
							}
						}
					}

					AM_LRB_Reset(lrb);
				}

				uint8_t* p = &buf[header_offset + 4];
				if (adaptation_field_control == 0x2 || adaptation_field_control == 0x3)
				{
					uint8_t adaptation_field_length = *p++;
					p += adaptation_field_length;
				}


				if (ts_pack_size - (unsigned char)(p - &buf[0]) >= 4 && p[0] == 0 && p[1] == 0 && p[2] == 1)
					stream_id = p[3];
				else
					stream_id = 0;

				if (option == 4 || (option == 2 && (stream_id & 0xF0) == 0xE0) || (option == 3 && (stream_id & 0xC0) == 0xC0))
				{
					printf("%sPES_PID: 0X%04X, AU_idx: %10" PRIu32 ", ATC_ts: %" PRIu32 "\n",
						(stream_id & 0xF0) == 0xE0 ? "[>V]" : (
						(stream_id & 0xC0) == 0xC0 ? "[>A]" : "    "),
						PID, iter == ring_buffers.end() ? 0 : std::get<2>(filter_stat[PID]), atc_tm);
				}

				if (iter == ring_buffers.end())
				{
					lrb = ring_buffers[PID] = AM_LRB_Create(1024);
					std::get<2>(filter_stat[PID]) = 0;
				}
				else
				{
					std::get<2>(filter_stat[PID])++;
				}

				if (header_offset == 4)
					std::get<0>(filter_stat[PID]) = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
			}
			else
			{
				auto iterStmPackInfo = stream_pack_info.find(PID);
				if (iterStmPackInfo != stream_pack_info.end())
				{
					std::get<2>(iterStmPackInfo->second)++;
				}
			}

			std::get<1>(filter_stat[PID])++;

			if (lrb != NULL)
			{
				uint8_t* p = &buf[header_offset + 4];
				if (adaptation_field_control == 0x2 || adaptation_field_control == 0x3)
				{
					uint8_t adaptation_field_length = *p++;
					p += adaptation_field_length;
				}

				// begin push the data into the linear buffer, only save the beginning of 1024 bytes
				int write_buf_len = 0;
				uint8_t* pWriteBuf = AM_LRB_GetWritePtr(lrb, &write_buf_len);
				if (pWriteBuf != NULL && write_buf_len > 0)
				{
					if (ts_pack_size - (unsigned char)(p - &buf[0]) < write_buf_len)
						write_buf_len = ts_pack_size - (unsigned char)(p - &buf[0]);

					memcpy(pWriteBuf, p, write_buf_len);
					AM_LRB_SkipWritePtr(lrb, write_buf_len);
				}
			}
		}

		ts_pack_idx++;
		if (!(adaptation_field_control & 0x2))
			continue;

		uint8_t* p = &buf[header_offset + 4];
		uint8_t adaptation_field_length = *p++;

		if (adaptation_field_length == 0)
			continue;

		uint8_t PCR_flag = ((*p) >> 4) & 0x1;
		uint8_t OPCR_flag = ((*p) >> 3) & 0x1;

		p++;

		if (PCR_flag == 0)
			continue;

		int64_t byte_position = FTELL64(fp) - ts_pack_size;

		uint64_t program_clock_reference_base = 0;
		for (int i = 0; i < 4; i++)
			program_clock_reference_base = (program_clock_reference_base << 8) | *(p++);

		program_clock_reference_base = (program_clock_reference_base << 1) | (((*p) >> 7) & 1);

		uint16_t program_clock_reference_extension = (*p) & 1;
		p++;
		program_clock_reference_extension = (program_clock_reference_extension << 8) | *(p++);

		uint64_t original_program_clock_reference_base = UINT64_MAX;
		uint16_t original_program_clock_reference_extension = UINT16_MAX;

		char szATC[256] = { 0 };
		if (header_offset == 4)
		{
			MBCSPRINTF_S(szATC, 256, ", ATC:%" PRIu32 "(27MHZ)", atc_tm);
		}

		int64_t transport_rate = 
			prev_PCR_byte_position == INT64_MAX ? 0 : (byte_position - prev_PCR_byte_position) * 27000000 * 8 / (program_clock_reference_base * 300 + program_clock_reference_extension - prev_PCR);

		if (max_transport_rate < transport_rate)
			max_transport_rate = transport_rate;

		if (option == 5)
			SPN_PCR.emplace_back((uint32_t)(byte_position / ts_pack_size), program_clock_reference_base * 300 + program_clock_reference_extension);

		if (first_PCR_values.find(PID) == first_PCR_values.end())
			first_PCR_values[PID] = program_clock_reference_base * 300 + program_clock_reference_extension;

		if (option != 5)
		{
			if (OPCR_flag)
			{
				original_program_clock_reference_base = 0;
				for (int i = 0; i < 4; i++)
					original_program_clock_reference_base = (original_program_clock_reference_base << 8) | *(p++);

				original_program_clock_reference_base = (original_program_clock_reference_base << 1) | (((*p) >> 7) & 1);

				original_program_clock_reference_extension = (*p) & 1;
				p++;
				original_program_clock_reference_extension = (original_program_clock_reference_extension << 8) | *(p++);

				printf(" -> PCR_PID: 0X%04X PCR(base: %10" PRIu64 "(90KHZ), ext: %3" PRIu16 ", %13" PRIu64 "(27MHZ), %10" PRIu64 ".%03" PRIu64 "(ms)), OPCR(base: %10" PRIu64 ", ext: %3" PRIu16 ", %" PRIu64 "(27MHZ), %" PRIu64 ".%03" PRIu64 "(ms)), "
					"transport_rate: %sbps%s\n",
					PID,
					program_clock_reference_base, program_clock_reference_extension,
					program_clock_reference_base * 300 + program_clock_reference_extension,
					(program_clock_reference_base * 300 + program_clock_reference_extension) / 27000,
					(program_clock_reference_base * 300 + program_clock_reference_extension) / 27 % 1000,
					original_program_clock_reference_base, original_program_clock_reference_extension,
					original_program_clock_reference_base * 300 + original_program_clock_reference_extension,
					(original_program_clock_reference_base * 300 + original_program_clock_reference_extension) / 27000,
					(original_program_clock_reference_base * 300 + original_program_clock_reference_extension) / 27 % 1000,
					GetHumanReadNumber(transport_rate, false, 2, 3, true).c_str(),
					szATC
				);
			}
			else
			{
				printf(" -> PCR_PID: 0X%04X PCR(base: %10" PRIu64 "(90KHZ), ext: %3" PRIu16 ", %13" PRIu64 "(27MHZ), %10" PRIu64 ".%03" PRIu64 "(ms)), "
					"transport_rate: %sbps%s\n",
					PID,
					program_clock_reference_base, program_clock_reference_extension,
					program_clock_reference_base * 300 + program_clock_reference_extension,
					(program_clock_reference_base * 300 + program_clock_reference_extension) / 27000,
					(program_clock_reference_base * 300 + program_clock_reference_extension) / 27 % 1000,
					GetHumanReadNumber(transport_rate, false, 2, 3, true).c_str(),
					szATC
				);
			}
		}

		uint32_t diff_ATC = atc_tm >= prev_ATC ? (atc_tm - prev_ATC) : (0x40000000 + atc_tm - prev_ATC);

		uint64_t base_diff = 0;
		uint64_t prev_PCR_base = prev_PCR / 300;
		uint64_t prev_PCR_ext = prev_PCR % 300;

		if (program_clock_reference_base >= prev_PCR_base)
			base_diff = program_clock_reference_base - prev_PCR_base;
		else
			base_diff = 0x200000000ULL + program_clock_reference_base - prev_PCR_base;

		base_diff *= 300ULL;

		if (program_clock_reference_extension >= prev_PCR_ext)
			base_diff += program_clock_reference_extension - prev_PCR_ext;
		else
			base_diff += 300ULL + program_clock_reference_extension - prev_PCR_ext;

		if (prev_ATC != UINT32_MAX && prev_PCR != UINT64_MAX)
		{
			if (max_diff_diff_PCR_ATC < (base_diff > diff_ATC ? (base_diff - diff_ATC) : (diff_ATC - base_diff)))
				max_diff_diff_PCR_ATC = (base_diff > diff_ATC ? (base_diff - diff_ATC) : (diff_ATC - base_diff));

			if ((base_diff > diff_ATC?(base_diff - diff_ATC):(diff_ATC - base_diff)) > 300)	// tolerance 0.1ms = 100 us
			{
				printf("!!!!! Diff ATC(%" PRIu32 " is not equal to the diff of PCR(%" PRIu64 ").\n", diff_ATC, base_diff);
			}
		}
		
		prev_PCR_byte_position = byte_position;
		prev_PCR = program_clock_reference_base * 300ULL + program_clock_reference_extension;
		prev_ATC = atc_tm;
	}

	avg_transport_rate = total_processed_bytes * 8 * 27000000 / atc_sum;

	printf("The max diff between diff ATC and diff PCR: %" PRIu64 "(270MHZ), %" PRIu64".%03" PRIu64 "(ms).\n",
		max_diff_diff_PCR_ATC, max_diff_diff_PCR_ATC / 27000, max_diff_diff_PCR_ATC / 27 % 1000);
	printf("The max transport rate: %" PRId64 "bps(%sbps)\n", max_transport_rate, GetHumanReadNumber(max_transport_rate, false, 2).c_str());
	printf("The average transport rate: %" PRId64 "bps(%sbps)\n", avg_transport_rate, GetHumanReadNumber(avg_transport_rate, false, 2).c_str());

	printf("              The first pts(27MHZ)    The first dts(27MHZ)\n");
	printf("----------------------------------------------------------\n");
	for (auto& stm : STM_SPN_PTS_DTS)
	{
		if (stm.second.size() == 0)
			continue;

		uint64_t pts = std::get<1>(stm.second[0]);
		uint64_t dts = std::get<2>(stm.second[0]);
		if (pts != UINT64_MAX && dts != UINT64_MAX)
		{
			printf("PID:0X%04X    %20" PRIu64 "    %20" PRIu64 "\n", stm.first, pts * 300, dts * 300);
			if (min_ts > dts * 300ULL)
				min_ts = dts * 300ULL;
		}
		else if (pts != UINT64_MAX)
		{
			printf("PID:0X%04X    %20" PRIu64 "\n", stm.first, pts * 300);
			if (min_ts > pts * 300ULL)
				min_ts = pts * 300ULL;
		}
	}
	for(auto iter: first_PCR_values)
		printf("PCR_PID: 0X%04X, The initial PCR value: %" PRIu64 "(27MHZ), diff with minimum dts: %" PRId64 " (27MHZ)/%" PRId64 ".%04" PRId64 "(ms)\n", 
			iter.first, iter.second, (int64_t)(min_ts - iter.second), (int64_t)(min_ts - iter.second)/27000, (int64_t)(min_ts - iter.second)*1000 / 27000%1000);

done:
	for (auto lrb : ring_buffers)
	{
		if (lrb.second != NULL)
			AM_LRB_Destroy(lrb.second);
	}

	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}

	if (option == 5)
	{
		int dataset_option = 0;		// 0: simple data set;1: full data set including every transport packet
		FILE* fp_csv = NULL;
		auto iterShowDiagram = g_params.find("showPCRDiagram");
		auto iterOutput = g_params.find("output");

		if (iterOutput != g_params.end())
		{
			int retOutput = -1;
			if (_stricmp(iterOutput->second.c_str(), "stdout") == 0)
				fp_csv = stdout;

			if ((retOutput = _access(iterOutput->second.c_str(), W_OK)) != 0)
			{
				if (errno != ENOENT)
				{
					printf("The output file: %s can't be written {error code: %d}.\n", iterOutput->second.c_str(), errno);
					return -1;
				}
			}
		}
		else
		{
			printf("Please specify a .csv file to save the PCR diagram dataset.\n");
			return -1;
		}

		if (iterShowDiagram != g_params.end() && _stricmp(iterShowDiagram->second.c_str(), "full") == 0)
		{
			dataset_option = 1;
		}

		if (fp_csv == nullptr) {
			FOPEN(fp_csv, iterOutput->second.c_str(), "wb");
		}

		if (fp_csv != NULL)
		{
			int ccWritten = 0;
			int ccWrittenOnce = 0;

			char szRow[1024] = { 0 };
			size_t max_number_of_rows = 0;
			if (max_number_of_rows < SPN_ATC.size())
				max_number_of_rows = SPN_ATC.size();

			if (max_number_of_rows < SPN_PCR.size())
				max_number_of_rows = SPN_PCR.size();

			ccWrittenOnce = MBCSPRINTF_S(szRow, sizeof(szRow) / sizeof(szRow[0]), "SPN,ATC(27MHZ),PCR(27MHZ)");
			if (ccWrittenOnce > 0)
				ccWritten += ccWrittenOnce;

			size_t next_PCR_idx = SIZE_MAX;
			size_t next_SPN_for_PCR = SIZE_MAX;
			std::vector<
				std::tuple<
				size_t/*next_stm_pts_dts_entry_idx*/,
				size_t/*next_stm_SPN*/,
				std::vector<std::tuple<uint32_t, uint64_t, uint64_t>>&>
			> next_stm_spn_entry;

			for (auto& stm : STM_SPN_PTS_DTS)
			{
				if (stm.second.size() > 0)
				{
					if (max_number_of_rows < stm.second.size())
						max_number_of_rows = stm.second.size();

					next_stm_spn_entry.emplace_back(0, std::get<0>(stm.second[0]), stm.second);
				}
				else
					next_stm_spn_entry.emplace_back(SIZE_MAX, SIZE_MAX, stm.second);

				ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten,
					",PTS(27MHZ)(0x%04x),DTS(27MHZ)(0x%04x)", stm.first, stm.first);
				if (ccWrittenOnce > 0)
					ccWritten += ccWrittenOnce;
			}
			if ((size_t)ccWritten + 1 < _countof(szRow))
				szRow[ccWritten++] = '\n';
			else
				szRow[_countof(szRow) - 1] = '\n';

			fwrite(szRow, 1, ccWritten, fp_csv);

			if (SPN_PCR.size() > 0)
			{
				next_PCR_idx = 0;
				next_SPN_for_PCR = std::get<0>(SPN_PCR[next_PCR_idx]);
			}

			for (size_t i = 0; i < max_number_of_rows; i++)
			{
				bool bFirstTSPackPayload = dataset_option ? true : false;
				ccWritten = 0;
				ccWrittenOnce = 0;
				if (i >= SPN_ATC.size())
					ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, ",,");
				else
					ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, "%" PRIu32 ",%" PRIu32 ",",
						std::get<0>(SPN_ATC[i]), std::get<1>(SPN_ATC[i]));

				if (ccWrittenOnce > 0)
					ccWritten += ccWrittenOnce;

				if (std::get<0>(SPN_ATC[i]) != next_SPN_for_PCR)
					ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, ",");
				else
				{
					ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, "%" PRIu64 ",",
						std::get<1>(SPN_PCR[next_PCR_idx]));
					if (next_PCR_idx + 1 >= SPN_PCR.size())
					{
						next_PCR_idx = SIZE_MAX;
						next_SPN_for_PCR = SIZE_MAX;
					}
					else
					{
						next_PCR_idx++;
						next_SPN_for_PCR = std::get<0>(SPN_PCR[next_PCR_idx]);
					}
					bFirstTSPackPayload = true;
				}

				if (ccWrittenOnce > 0)
					ccWritten += ccWrittenOnce;

				// For stream SPN and PTS/DTS(27MHZ)
				for (size_t j = 0; j < next_stm_spn_entry.size(); j++)
				{
					size_t next_stm_entry_idx = std::get<0>(next_stm_spn_entry[j]);
					size_t next_SPN_for_stm = std::get<1>(next_stm_spn_entry[j]);
					auto pts_dts_entry = std::get<2>(next_stm_spn_entry[j]);

					if (std::get<0>(SPN_ATC[i]) != next_SPN_for_stm)
					{
						ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, ",,");
					}
					else
					{
						uint64_t pts = std::get<1>(std::get<2>(next_stm_spn_entry[j])[next_stm_entry_idx]);
						uint64_t dts = std::get<2>(std::get<2>(next_stm_spn_entry[j])[next_stm_entry_idx]);

						if (pts != UINT64_MAX && dts != UINT64_MAX)
							ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, "%" PRIu64 ",%" PRIu64 ",", pts * 300, dts * 300);
						else if (pts != UINT64_MAX)
							ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, "%" PRIu64 ",,", pts * 300);
						else
							ccWrittenOnce = MBCSPRINTF_S(szRow + ccWritten, sizeof(szRow) / sizeof(szRow[0]) - ccWritten, ",,");

						if (next_stm_entry_idx + 1 >= pts_dts_entry.size())
						{
							std::get<0>(next_stm_spn_entry[j]) = SIZE_MAX;
							std::get<1>(next_stm_spn_entry[j]) = SIZE_MAX;
						}
						else
						{
							next_stm_entry_idx++;
							std::get<0>(next_stm_spn_entry[j]) = next_stm_entry_idx;
							next_SPN_for_stm = std::get<0>(pts_dts_entry[next_stm_entry_idx]);
							std::get<1>(next_stm_spn_entry[j]) = next_SPN_for_stm;
						}

						bFirstTSPackPayload = true;
					}

					if (ccWrittenOnce > 0)
						ccWritten += ccWrittenOnce;
				}

				szRow[ccWritten++] = '\n';

				if (bFirstTSPackPayload == false)
					continue;

				fwrite(szRow, 1, ccWritten, fp_csv);
			}

			if (fp_csv != stdout)
				fclose(fp_csv);
		}
	}

	return iRet;
}

int DiffTSATC()
{
	int iRet = -1;
	long long diff_threshold = -1;
	unsigned long ts_pack_idx = 0;
	unsigned char ts_pack_size = TS_PACKET_SIZE;
	unsigned char buf[TS_PACKET_SIZE];
	FILE *fp = NULL;
	long long start_pkt_idx = -1LL;
	long long end_pkt_idx = -1LL;
	long long pkt_idx = 0;
	uint32_t previous_arrive_time = UINT32_MAX;
	uint16_t pid_filter = UINT16_MAX;
	uint16_t previous_PID = UINT16_MAX;
	bool diff_AU_first_last = false;
	int64_t sum_duration = 0;
	int64_t pure_sum_duration = 0;
	int payload_unit_length = 0;
	long long payload_unit_count = 0;
	long long first_pkt_idx = -1LL, last_pkt_idx = -1LL;
	int64_t max_diff = INT64_MIN, min_diff = INT64_MAX;
	int64_t max_pure_diff = INT64_MIN, min_pure_diff = INT64_MAX;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	{
		auto iterDiffATC = g_params.find("diffATC");
		if (iterDiffATC != g_params.end())
		{
			diff_threshold = ConvertToLongLong(iterDiffATC->second);
		}

		auto iterStart = g_params.find("start");
		if (iterStart != g_params.end())
			start_pkt_idx = ConvertToLongLong(iterStart->second);

		auto iterEnd = g_params.find("end");
		if (iterEnd != g_params.end())
			end_pkt_idx = ConvertToLongLong(iterEnd->second);
	}

	if (g_params.find("pid") != g_params.end())
	{
		long long pid = ConvertToLongLong(g_params["pid"]);
		if (pid >= 0 && pid <= 0x1FFF)
		{
			pid_filter = (uint16_t)pid;

			if (g_params.find("payload_first_last") != g_params.end())
			{
				diff_AU_first_last = true;
			}
		}
	}

	while (true)
	{
		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		if (buf[4] != 0x47)
		{
			printf("It seems not to be a supported m2ts or tts stream file.\n");
			break;
		}

		uint32_t arrive_time = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]) & 0x3FFFFFFF;
		int32_t diff = INT32_MAX;

		uint16_t PID = ((buf[5] & 0x1F) << 8) | (buf[6]);
		uint8_t payload_unit_start_indicator = (buf[5] & 0x40) >> 6;
		uint8_t cc = buf[7] & 0xF;

		if (previous_arrive_time != UINT32_MAX)
		{
			if (arrive_time > previous_arrive_time)
				diff = (int32_t)(arrive_time - previous_arrive_time);
			else
				diff = 0x40000000 + arrive_time - previous_arrive_time;

			sum_duration += diff;
			if (pid_filter == previous_PID)
				pure_sum_duration += diff;

			if (pid_filter == PID)
				payload_unit_length += 192;

			bool bFiltered = false;
			if ((start_pkt_idx == -1LL || (start_pkt_idx >= 0 && pkt_idx >= start_pkt_idx)) &&
				(end_pkt_idx == -1LL || (end_pkt_idx >= 0 && pkt_idx < end_pkt_idx)))
			{
				if (diff_threshold == -1LL)
					bFiltered = true;
				else if (diff_threshold > 0 && (long long)diff > diff_threshold)
					bFiltered = true;
			}
			
			if (pid_filter == UINT16_MAX || pid_filter == PID)
			{
				if (diff_AU_first_last == false)
				{
					if (bFiltered)
					{
						printf("pkt_idx: %10lld [PID: 0X%04X][header 4bytes: %02X %02X %02X %02X] CC:%02d ATC: 0x%08" PRIX32 "(%10" PRIu32 "), diff: %" PRId64 "(%fms)\n",
							pkt_idx, PID, buf[0], buf[1], buf[2], buf[3], cc,
							arrive_time, arrive_time, sum_duration, sum_duration*1000.0f / 27000000.f);
					}

					if (min_diff > sum_duration)
						min_diff = sum_duration;

					if (max_diff < sum_duration)
						max_diff = sum_duration;

					sum_duration = 0;
					pure_sum_duration = 0;
				}
				else if (bFiltered)
				{
					if (payload_unit_start_indicator)
					{
						if (diff_AU_first_last == true && payload_unit_count > 0)
						{
							printf("payload_idx: %8lld [PID: 0X%04X] len: %8d(B) diff[packet first:%8lld ~ last:%8lld]: %10" PRId64 "(%3d.%04dms), pure duration:%10" PRId64 "(%3d.%04dms)\n",
								payload_unit_count - 1, PID, payload_unit_length, first_pkt_idx, last_pkt_idx,
								sum_duration, (int)(sum_duration / 27000), (int)(sum_duration * 10000 / 27000 % 10000),
								pure_sum_duration, (int)(pure_sum_duration / 27000), (int)(pure_sum_duration * 10000 / 27000 % 10000));

							if (min_diff > sum_duration)
								min_diff = sum_duration;

							if (max_diff < sum_duration)
								max_diff = sum_duration;

							if (min_pure_diff > pure_sum_duration)
								min_pure_diff = pure_sum_duration;

							if (max_pure_diff < pure_sum_duration)
								max_pure_diff = pure_sum_duration;

							sum_duration = 0;
							pure_sum_duration = 0;
							payload_unit_length = 0;
						}
						payload_unit_count++;
						first_pkt_idx = pkt_idx;
						last_pkt_idx = first_pkt_idx;
					}
					else
						last_pkt_idx = pkt_idx;
				}
			}
		}
		else
		{
			if (pid_filter == UINT16_MAX || pid_filter == PID)
				printf("pkt_idx: %10lld [PID: 0X%04X][header 4bytes: %02X %02X %02X %02X] CC:%02d ATC: 0x%08" PRIX32 "(%10" PRIu32 "), diff: \n",
					pkt_idx, PID, buf[0], buf[1], buf[2], buf[3], cc, arrive_time, arrive_time);
		}

		previous_arrive_time = arrive_time;
		pkt_idx++;
		previous_PID = PID;
	}

	if (sum_duration > 0)
	{
		if (diff_AU_first_last == true)
		{
			printf("payload_idx: %8lld [PID: 0X%04X] len: %8d(B) diff[packet first:%8lld ~ last:%8lld]: %10" PRId64 "(%3d.%04dms), pure duration:%10" PRId64 "(%3d.%04dms)\n",
				payload_unit_count - 1, pid_filter, payload_unit_length, first_pkt_idx, last_pkt_idx,
				sum_duration, (int)(sum_duration / 27000), (int)(sum_duration * 10000 / 27000 % 10000),
				pure_sum_duration, (int)(pure_sum_duration / 27000), (int)(pure_sum_duration * 10000 / 27000 % 10000));
			sum_duration = 0;
			pure_sum_duration = 0;
			payload_unit_length = 0;
		}
	}

	printf("\n");
	printf("The maximum diff of ATC between transport packets: %" PRId64 "(%" PRId64 ".%04" PRId64 "ms).\n",
		max_diff, max_diff/27000, max_diff*10000/27000%10000);
	printf("The minimum diff of ATC between transport packets: %" PRId64 "(%" PRId64 ".%04" PRId64 "ms).\n",
		min_diff, min_diff / 27000, min_diff * 10000 / 27000 % 10000);

	if (diff_AU_first_last == true && max_pure_diff != INT64_MIN && min_pure_diff != INT64_MAX)
	{
		printf("The maximum diff sum of PID:0x%04X of ATC between transport packets: %" PRId64 "(%" PRId64 ".%04" PRId64 "ms).\n",
			pid_filter, max_pure_diff, max_pure_diff / 27000, max_pure_diff * 10000 / 27000 % 10000);
		printf("The minimum diff sum of PID:0x%04X of ATC between transport packets: %" PRId64 "(%" PRId64 ".%04" PRId64 "ms).\n",
			pid_filter, min_pure_diff, min_pure_diff / 27000, min_pure_diff * 10000 / 27000 % 10000);
	}

	iRet = 0;

done:
	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
	return iRet;
}

int g_debug_sync = 0;

int BenchRead(int option)
{
	int iRet = -1;
	uint8_t header_offset = 0;
	uint32_t ts_pack_idx = 0;
	uint8_t ts_pack_size = TS_PACKET_SIZE;
	uint8_t buf[TS_PACKET_SIZE];
	FILE *fp = NULL;
	size_t nRead = 0;
	uint8_t packet_start_code_prefix[3] = { 0 };
	uint8_t stream_id = 0;
	std::map<unsigned short, AMLinearRingBuffer> ring_buffers;
	std::map<unsigned short, std::tuple<uint32_t/*start ATC time*/, uint32_t/*AU size(how many TS packs)*/, uint32_t/*AU idx*/>> filter_stat;
	int64_t prev_PCR_byte_position = INT64_MAX;
	uint64_t prev_PCR = UINT64_MAX;

	std::chrono::high_resolution_clock::time_point htp_start, htp_end;

	uint32_t arrive_timestamp;
	STDClockSyncer syncer(CLOCK_VALUE_30BIT | CLOCK_VALUE_27MHZ);

	uint64_t total_duration_27MHZ = 0ULL;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	nRead = fread(buf, 1, TS_PACKET_SIZE - 4, fp);
	if (nRead < TS_PACKET_SIZE - 4)
	{
		printf("The TS stream file is too small.\n");
		goto done;
	}

	if (buf[4] == 0x47)
	{
		header_offset = 4;
		ts_pack_size = TS_PACKET_SIZE;

		arrive_timestamp = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]) & 0x3FFFFFFF;
		if (syncer.Start(arrive_timestamp) < 0)
		{
			printf("Failed to start the T-STD syncer.\n");
			goto done;
		}
	}
	else
	{
		printf("Unsupported transport stream, please specify the m2ts or tts file.\n");
		goto done;
	}

	// reset the file read pointer
	htp_start = std::chrono::high_resolution_clock::now();

	FSEEK64(fp, 0, SEEK_SET);
	while (!feof(fp))
	{
		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		if (buf[header_offset] != 0x47)
		{
			printf("Seems to hit an wrong TTS/M2TS packet.\n");
			break;
		}

		uint32_t new_arrive_timestamp = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]) & 0x3FFFFFFF;

		total_duration_27MHZ += new_arrive_timestamp >= arrive_timestamp ?
			(new_arrive_timestamp - arrive_timestamp) :
			(0x40000000UL + new_arrive_timestamp - arrive_timestamp);

		arrive_timestamp = new_arrive_timestamp;

		//printf("arrive_timestamp: %lu at TS-Pack#%" PRIu32 "\n", arrive_timestamp, ts_pack_idx);

		int iSyncRet = syncer.Sync(arrive_timestamp);
		if (iSyncRet == RET_CODE_TIME_OUT)
			printf("Hit ATC sync time-out at TS-Pack#%" PRIu32 ", arrive_timestamp: %" PRIu32 "!\n", ts_pack_idx, arrive_timestamp);
		else if (iSyncRet == RET_CODE_CLOCK_DISCONTINUITY)
			printf("Hit ATC discontinuity at TS-Pack#%" PRIu32 "!\n", ts_pack_idx);
		else if (iSyncRet < 0)
			printf("Hit ATC sync error (%d) at TS-Pack#%" PRIu32 ".\n", iSyncRet, ts_pack_idx);

		ts_pack_idx++;
	}

	htp_end = std::chrono::high_resolution_clock::now();

	{
		int64_t num_of_bits_processed = ts_pack_idx * 192LL * 8LL;
		int64_t elapse_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(htp_end - htp_start).count();
		int64_t read_bitrate = num_of_bits_processed * 1000 / elapse_time_ms;
		int64_t transport_rate = num_of_bits_processed * 27000000 / total_duration_27MHZ;
		printf("\n");
		printf("It took %" PRId64 " ms to process the data read.\n", elapse_time_ms);
		printf("The average read rate: %" PRId64 " (bps)/%" PRId64 ".%03d (Mbps), the target transport_rate: %" PRId64 " (bps)/%" PRId64 ".%03d (Mbps)\n",
			read_bitrate, read_bitrate / 1000 / 1000, (uint16_t)(read_bitrate / 1000 % 1000),
			transport_rate, transport_rate / 1000 / 1000, (uint16_t)(transport_rate / 1000 % 1000));
	}
done:
	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
	return iRet;
}

int	LayoutTSPacket()
{
	int iRet = RET_CODE_SUCCESS;

	return iRet;
}

int DiffATCDTS()
{
	int iRet = -1;
	unsigned long ts_pack_idx = 0;
	unsigned char ts_pack_size = TS_PACKET_SIZE;
	unsigned char buf[TS_PACKET_SIZE];
	FILE *fp = NULL;
	long long start_pkt_idx = -1LL;
	long long end_pkt_idx = -1LL;
	long long pkt_idx = 0;
	uint32_t previous_arrive_time = UINT32_MAX;
	bool diff_AU_first_last = false;
	uint64_t sum_duration = 0;
	long long payload_unit_count = 0;
	std::vector<uint16_t> PIDs;
	std::vector<uint64_t> payload_unit_packets;
	std::vector<uint64_t> the_latest_stream_atc_sum;
	std::vector<uint64_t> the_payload_start_atc_sum;
	std::vector<uint64_t> the_latest_stream_dts;
	std::vector<std::vector<uint8_t>> pes_hdr_bufs;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	{
		auto iterStart = g_params.find("start");
		if (iterStart != g_params.end())
			start_pkt_idx = ConvertToLongLong(iterStart->second);

		auto iterEnd = g_params.find("end");
		if (iterEnd != g_params.end())
			end_pkt_idx = ConvertToLongLong(iterEnd->second);
	}

	if (g_params.find("pid") != g_params.end())
	{
		std::vector<std::string> strPIDs;
		splitstr(g_params["pid"].c_str(), ",;.:", strPIDs);

		if (strPIDs.size() < 2)
		{
			printf("Please specify 2 PIDs to diff!\n");
			goto done;
		}

		for (auto& strPID : strPIDs)
		{
			long long pid = ConvertToLongLong(strPID);
			if (pid >= 0 && pid <= 0x1FFF)
			{
				PIDs.push_back((uint16_t)pid);
			}
		}
	}

	if (PIDs.size() < 2)
	{
		printf("Please specify 2 valid PIDs to diff.\n");
		goto done;
	}

	if (PIDs.size() > 2)
		printf("Only compare the ATC and DTS of payload unit start for the PIDs: 0X%04X vs. 0X%04X\n", PIDs[0], PIDs[1]);

	payload_unit_packets.resize(PIDs.size());
	the_latest_stream_atc_sum.resize(PIDs.size());
	the_latest_stream_dts.resize(PIDs.size());
	the_payload_start_atc_sum.resize(PIDs.size());
	pes_hdr_bufs.resize(PIDs.size());

	for (size_t i = 0; i < the_latest_stream_dts.size(); i++)
		the_latest_stream_dts[i] = UINT64_MAX;

	for (size_t i = 0; i < the_latest_stream_atc_sum.size(); i++)
		the_latest_stream_atc_sum[i] = UINT64_MAX;

	while (true)
	{
		size_t nRead = fread(buf, 1, ts_pack_size, fp);
		if (nRead < ts_pack_size)
			break;

		if (buf[4] != 0x47)
		{
			printf("It seems not to be a supported m2ts or tts stream file.\n");
			break;
		}

		uint32_t arrive_time = ((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]) & 0x3FFFFFFF;
		int32_t diff = INT32_MAX;

		uint16_t PID = ((buf[5] & 0x1F) << 8) | (buf[6]);
		uint8_t payload_unit_start_indicator = (buf[5] & 0x40) >> 6;
		uint8_t adaptation_field_control = (buf[7] >> 4) & 0x3;
		uint8_t cc = buf[7] & 0xF;

		uint8_t* data_start = buf + 8;
		uint8_t adaptation_field_length = 0;
		if (adaptation_field_control == 2 || adaptation_field_control == 3)
		{
			adaptation_field_length = buf[8];
			data_start += (ptrdiff_t)adaptation_field_length + 1;
		}

		if (previous_arrive_time != UINT32_MAX)
		{
			if (arrive_time > previous_arrive_time)
				diff = (int32_t)(arrive_time - previous_arrive_time);
			else
				diff = 0x40000000 + arrive_time - previous_arrive_time;

			sum_duration += diff;

			for (size_t idxPID = 0; idxPID < PIDs.size(); idxPID++)
			{
				if (PID != PIDs[idxPID])
					continue;

				bool bNeedParseDTS = 0;
				if (payload_unit_start_indicator)
				{
					payload_unit_packets[idxPID] = 1;
					the_payload_start_atc_sum[idxPID] = sum_duration;

					bNeedParseDTS = true;
					pes_hdr_bufs[idxPID].clear();
				}
				else
					payload_unit_packets[idxPID]++;

				if (pes_hdr_bufs[idxPID].size() > 0 || bNeedParseDTS)
				{
					// need continue fill the buffer, and analyze the dts
					if (buf + 192 > data_start)
						pes_hdr_bufs[idxPID].insert(pes_hdr_bufs[idxPID].end(), data_start, buf + 192);

					const uint8_t* pes_buf = pes_hdr_bufs[idxPID].data();
					size_t pes_buf_size = pes_hdr_bufs[idxPID].size();

					// check the dts
					if (pes_buf_size > 9 && pes_buf[0] == 0 && pes_buf[1] == 0 && pes_buf[2] == 0x01)
					{
						uint8_t stream_id = pes_buf[3];
						uint16_t PES_packet_length = (pes_buf[4] << 8) | pes_buf[5];

						if (stream_id != SID_PROGRAM_STREAM_MAP
							&& stream_id != SID_PADDING_STREAM
							&& stream_id != SID_PRIVATE_STREAM_2
							&& stream_id != SID_ECM
							&& stream_id != SID_EMM
							&& stream_id != SID_PROGRAM_STREAM_DIRECTORY
							&& stream_id != SID_DSMCC_STREAM
							&& stream_id != SID_H222_1_TYPE_E)
						{
							uint8_t reserved_0 = (pes_buf[6] >> 6) & 0x3;
							uint8_t PES_scrambling_control = (pes_buf[6] >> 4) & 0x3;

							uint8_t PTS_DTS_flags = (pes_buf[7] >> 6) & 0x3;
							uint8_t PES_header_data_length = pes_buf[8];

							uint64_t pts = UINT64_MAX, dts = UINT64_MAX;

							if ((PTS_DTS_flags == 0x2 && ((pes_buf[9] >> 4) & 0xF) == 0x2 && pes_buf_size > 14 && PES_header_data_length >= 5) ||
								(PTS_DTS_flags == 0x3 && ((pes_buf[9] >> 4) & 0xF) == 0x3 && pes_buf_size > 19 && PES_header_data_length >= 10))
							{
								pts = 0;
								pts = (pes_buf[9] >> 1) & 0x7;
								pts = pts << 15;
								pts |= (pes_buf[10] << 7) | (pes_buf[11] >> 1);
								pts = pts << 15;
								pts |= (pes_buf[12] << 7) | (pes_buf[13] >> 1);

								if (PTS_DTS_flags == 0x3)
								{
									dts = 0;
									dts = (pes_buf[14] >> 1) & 0x7;
									dts = dts << 15;
									dts |= (pes_buf[15] << 7) | (pes_buf[16] >> 1);
									dts = dts << 15;
									dts |= (pes_buf[17] << 7) | (pes_buf[18] >> 1);

									the_latest_stream_dts[idxPID] = dts;
								}
								else
									the_latest_stream_dts[idxPID] = pts;

								the_latest_stream_atc_sum[idxPID] = the_payload_start_atc_sum[idxPID];

								if (the_latest_stream_dts[0] != UINT64_MAX && the_latest_stream_dts[1] != UINT64_MAX &&
									the_latest_stream_atc_sum[0] != UINT64_MAX && the_latest_stream_atc_sum[1] != UINT64_MAX)
								{
									int64_t delta_atc_sum = (int64_t)(the_latest_stream_atc_sum[0] - the_latest_stream_atc_sum[1]);
									int64_t delta_dts = (int64_t)(the_latest_stream_dts[0] - the_latest_stream_dts[1]) * 300LL;

									if (delta_atc_sum != 0 || delta_dts != 0)
									{
										printf("pkt#%10lld PID: 0X%04X delta_atc_sum: %12" PRId64 "(27MHZ)/%4" PRId64 ".%03" PRId64 "(ms) -- delta_dts: %12" PRId64 "(27MHZ)/%4" PRId64 ".%03" PRId64 "(ms) | delta: %14" PRId64 "(27MHZ)/%4" PRId64 ".%03" PRId64 "(ms)\n",
											pkt_idx, PID,
											delta_atc_sum, delta_atc_sum / 27000, abs(delta_atc_sum) / 27 % 1000,
											delta_dts, delta_dts / 27000, abs(delta_dts) / 27 % 1000,
											delta_dts - delta_atc_sum, (delta_dts - delta_atc_sum) / 27000, abs(delta_dts - delta_atc_sum) / 27 % 1000);
									}
								}

								pes_hdr_bufs[idxPID].clear();
							}
							else if (PTS_DTS_flags != 0x2 && PTS_DTS_flags != 0x3)
							{
								pes_hdr_bufs[idxPID].clear();
								the_latest_stream_dts[idxPID] = UINT64_MAX;
							}
						}
						else
						{
							pes_hdr_bufs[idxPID].clear();
							the_latest_stream_dts[idxPID] = UINT64_MAX;
						}
					}
				}
			}
		}
		else
		{
		}

		previous_arrive_time = arrive_time;
		pkt_idx++;
	}

	if (sum_duration > 0)
	{
		if (diff_AU_first_last == true)
		{
			sum_duration = 0;
		}
	}

	iRet = 0;

done:
	if (fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
	return iRet;
}
