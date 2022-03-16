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
#include "ISO14496_15.h"
#include "NAL.h"
#include "ESRepacker.h"

namespace BST
{
	namespace ISOBMFF
	{
		int NALAUSampleRepackerBase::Seek(uint64_t src_sample_file_offset)
		{
			if (_fseeki64(m_fpSrc, src_sample_file_offset, SEEK_SET) != 0)
			{
				printf("Failed to seek to the file position: %" PRIu64 ".\n", src_sample_file_offset);
				return -1;
			}

			return 0;
		}

		int	NALAUSampleRepackerBase::RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe)
		{
			return -1;
		}

		int NALAUSampleRepackerBase::SetNextAUPTSDTS(TM_90KHZ pts, TM_90KHZ dts)
		{
			m_next_AU_PTS = pts;
			m_next_AU_DTS = dts;
			return RET_CODE_SUCCESS;
		}

		int NALAUSampleRepackerBase::SetAUStartPointCallback(CB_AU_STARTPOINT cbAUStartPoint, void* ptr_context)
		{
			m_callback_au_startpoint = cbAUStartPoint;
			m_context_au_startpoint = ptr_context;
			return RET_CODE_SUCCESS;
		}

		int NALAUSampleRepackerBase::WriteAUData(uint8_t* pBuf, int cbBuf, const PROCESS_DATA_INFO* NAL_Unit_DataInfo)
		{
			int iRet = RET_CODE_SUCCESS;
			if (pBuf == NULL || cbBuf <= 0)
				return RET_CODE_INVALID_PARAMETER;

			if (NAL_Unit_DataInfo && !m_data_info.valid)
				m_data_info = *NAL_Unit_DataInfo;

			if (m_fpDst != NULL)
			{
				fwrite((const void*)pBuf, 1, cbBuf, m_fpDst);
			}

			if (m_lrb_AU)
			{
				int nAUWriteBufLen = 0;
				uint8_t* pAUWriteBuf = AM_LRB_GetWritePtr(m_lrb_AU, &nAUWriteBufLen);
				if (nAUWriteBufLen <= 0 || pAUWriteBuf == nullptr || nAUWriteBufLen < cbBuf)
				{
					// Try to reform and enlarge the buffer.
					AM_LRB_Reform(m_lrb_AU);
					pAUWriteBuf = AM_LRB_GetWritePtr(m_lrb_AU, &nAUWriteBufLen);
					if (nAUWriteBufLen == 0 || pAUWriteBuf == nullptr || nAUWriteBufLen < cbBuf)
					{
						// Enlarge the ring buffer size
						int nCurSize = AM_LRB_GetSize(m_lrb_AU);
						if (nCurSize == INT32_MAX || nCurSize + cbBuf > INT32_MAX)
						{
							printf("[NALRepacker][WriteAUData] The input AE unit buffer is too huge, can't be supported now.\n");
							return RET_CODE_OUTOFMEMORY;
						}

						if (nCurSize + cbBuf >= INT32_MAX / 2)
							nCurSize = INT32_MAX;
						else
							nCurSize += cbBuf;

						printf("[NALRepacker][WriteAUData] Try to resize the linear ring buffer size to %d bytes.\n", nCurSize);
						if ((iRet = AM_LRB_Resize(m_lrb_AU, nCurSize)) < 0)
							return iRet;

						pAUWriteBuf = AM_LRB_GetWritePtr(m_lrb_AU, &nAUWriteBufLen);
						if (nAUWriteBufLen == 0 || pAUWriteBuf == nullptr)
							return RET_CODE_OUTOFMEMORY;
					}
				}

				memcpy(pAUWriteBuf, pBuf, cbBuf);
				AM_LRB_SkipWritePtr(m_lrb_AU, cbBuf);
			}

			return RET_CODE_SUCCESS;
		}

		int NALAUSampleRepackerBase::CommitAU()
		{
			int nAUBufLen = 0;
			if (m_lrb_AU)
			{
				uint8_t* pAUReadPtr = AM_LRB_GetReadPtr(m_lrb_AU, &nAUBufLen);
				if (pAUReadPtr != NULL && nAUBufLen > 0)
				{
					if (m_pOutputAgent != nullptr)
					{
						m_pOutputAgent->OutputES(m_data_info.valid ? m_data_info.CID : 0xFFFF,
							m_data_info.valid ? m_data_info.packet_id : 0xFFFF,
							pAUReadPtr, nAUBufLen, m_next_AU_PTS, m_next_AU_DTS);
					}

					AM_LRB_SkipReadPtr(m_lrb_AU, nAUBufLen);
				}
			}

			m_data_info.valid = 0;

			return RET_CODE_SUCCESS;
		}

		int	AVCSampleRepacker::RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe)
		{
			uint8_t buf[2048];
			int64_t cbLeft = sample_size;
			int iRet = RET_CODE_SUCCESS;
			int8_t next_nal_unit_type = (int8_t)AVC_SPS_NUT;
			bool bFirstNALUnit = true;

			uint8_t four_bytes_start_prefixes[4] = { 0, 0, 0, 1 };
			uint8_t three_bytes_start_prefixes[3] = { 0, 0, 1 };

			auto write_nu_array = [&](int8_t nal_unit_type) {
				if (nal_unit_type == (int8_t)AVC_SPS_NUT)
				{
					for (size_t i = 0; i < m_AVCConfigRecord->sequenceParameterSetNALUnits.size(); i++)
					{
						if (m_fpDst != NULL)
						{
							fwrite(four_bytes_start_prefixes, 1, 4, m_fpDst);
							fwrite(&m_AVCConfigRecord->sequenceParameterSetNALUnits[i]->nalUnit[0], 1, m_AVCConfigRecord->sequenceParameterSetNALUnits[i]->nalUnit.size(), m_fpDst);
						}
						bFirstNALUnit = false;
					}
					return true;
				}
				else if (nal_unit_type == (int8_t)AVC_PPS_NUT)
				{
					for (size_t i = 0; i < m_AVCConfigRecord->pictureParameterSetNALUnits.size(); i++)
					{
						if (m_fpDst != NULL)
						{
							fwrite(four_bytes_start_prefixes, 1, 4, m_fpDst);
							fwrite(&m_AVCConfigRecord->pictureParameterSetNALUnits[i]->nalUnit[0], 1, m_AVCConfigRecord->pictureParameterSetNALUnits[i]->nalUnit.size(), m_fpDst);
						}
						bFirstNALUnit = false;
					}
					return true;
				}
				else if (nal_unit_type == (int8_t)AVC_SPS_EXT_NUT)
				{
					for (size_t i = 0; i < m_AVCConfigRecord->sequenceParameterSetExtNALUnits.size(); i++)
					{
						if (m_fpDst != NULL)
						{
							fwrite(three_bytes_start_prefixes, 1, 3, m_fpDst);
							fwrite(&m_AVCConfigRecord->sequenceParameterSetExtNALUnits[i]->nalUnit[0], 1, m_AVCConfigRecord->sequenceParameterSetExtNALUnits[i]->nalUnit.size(), m_fpDst);
						}
						bFirstNALUnit = false;
					}
					return true;
				}

				return false;
			};

			while (cbLeft > 0)
			{
				uint32_t NALUnitLength = 0;
				if (fread(buf, 1, (size_t)m_AVCConfigRecord->lengthSizeMinusOne + 1, m_fpSrc) != (size_t)m_AVCConfigRecord->lengthSizeMinusOne + 1UL)
					break;

				for (int i = 0; i < m_AVCConfigRecord->lengthSizeMinusOne + 1; i++)
					NALUnitLength = (NALUnitLength << 8) | buf[i];

				//bool bLastNALUnit = cbLeft <= (m_AVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength) ? true : false;

				uint8_t first_leading_read_pos = 0;
				if (keyframe == FLAG_SET && next_nal_unit_type != -1)
				{
					// read the nal_unit_type
					if (fread(buf, 1, 1, m_fpSrc) != 1)
						break;

					first_leading_read_pos = 1;

					//int8_t nal_ref_idc = (buf[0] >> 5) & 0x03;
					int8_t nal_unit_type = buf[0] & 0x1F;
					if (nal_unit_type == (int8_t)AVC_SPS_NUT || nal_unit_type == (int8_t)AVC_PPS_NUT)
					{
						for (int8_t i = next_nal_unit_type; i < nal_unit_type; i++)
							write_nu_array(i);

						next_nal_unit_type = nal_unit_type == (int8_t)AVC_PPS_NUT ? (int8_t)AVC_SPS_EXT_NUT : (nal_unit_type + 1);
					}
					else if (nal_unit_type >= 1 && nal_unit_type <= 5)
					{
						for (int8_t i = next_nal_unit_type; i <= (int8_t)AVC_SPS_EXT_NUT; i++)
							write_nu_array(i);
						next_nal_unit_type = -1;
					}
					else if (nal_unit_type == (int8_t)AVC_SEI_NUT)
					{
						for (int8_t i = next_nal_unit_type; i <= (int8_t)AVC_PPS_NUT; i++)
							write_nu_array(i);

						next_nal_unit_type = (int8_t)AVC_SPS_EXT_NUT;
					}
				}

				if (m_fpDst != NULL)
				{
					if (bFirstNALUnit == true)
						fwrite(four_bytes_start_prefixes, 1, 4, m_fpDst);
					else
						fwrite(three_bytes_start_prefixes, 1, 3, m_fpDst);
				}

				uint32_t cbLeftNALUnit = NALUnitLength - first_leading_read_pos;
				while (cbLeftNALUnit > 0)
				{
					uint32_t nCpyCnt = AMP_MIN(2048UL - first_leading_read_pos, cbLeftNALUnit);
					if ((nCpyCnt = (uint32_t)fread(&buf[first_leading_read_pos], 1, nCpyCnt, m_fpSrc)) == 0)
						break;

					if (m_fpDst != NULL)
						fwrite(buf, 1, (size_t)nCpyCnt + first_leading_read_pos, m_fpDst);

					cbLeftNALUnit -= nCpyCnt;
					first_leading_read_pos = 0;
				}

				cbLeft -= (int64_t)m_AVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength;
				bFirstNALUnit = false;
			}

			if (cbLeft != 0)
				printf("[AVCSampleRepacker] There are %" PRIi64 " bytes of NAL unit data is not re-packed.\n", cbLeft);

			return iRet;
		}

		int AVCSampleRepacker::RepackNALUnitToAnnexBByteStream(uint8_t* pNalUnitBuf, int NumBytesInNalUnit, const PROCESS_DATA_INFO* NAL_Unit_DataInfo, bool* bAUCommitted)
		{
			return -1;
		}

		int AVCSampleRepacker::Flush()
		{
			return -1;
		}

		int AVCSampleRepacker::Drain()
		{
			return -1;
		}

		int	HEVCSampleRepacker::RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe)
		{
			int iRet = RET_CODE_SUCCESS;
			struct
			{
				int8_t nu_array_idx = -1;
				int8_t array_completeness = 0;
			}nu_array_info[64];

			uint8_t four_bytes_start_prefixes[4] = { 0, 0, 0, 1 };
			uint8_t three_bytes_start_prefixes[3] = { 0, 0, 1 };

			if (keyframe == FLAG_SET)
			{
				// Write SPS, PPS, VPS or declarative SEI NAL unit
				for (size_t i = 0; i < m_HEVCConfigRecord->nalArray.size(); i++)
				{
					auto nu_type = m_HEVCConfigRecord->nalArray[i]->NAL_unit_type;
					nu_array_info[nu_type].array_completeness = m_HEVCConfigRecord->nalArray[i]->array_completeness;
					nu_array_info[nu_type].nu_array_idx = (int8_t)i;
				}
			}

			uint8_t buf[2048];
			int64_t cbLeft = sample_size;
			int8_t next_nal_unit_type = (int8_t)HEVC_VPS_NUT;
			bool bFirstNALUnit = true;

			auto write_nu_array = [&](int8_t nal_unit_type) {
				if (nu_array_info[nal_unit_type].nu_array_idx == -1)
					return false;

				auto nuArray = m_HEVCConfigRecord->nalArray[nu_array_info[nal_unit_type].nu_array_idx];
				for (size_t i = 0; i < nuArray->numNalus; i++)
				{
					if (m_fpDst != NULL)
					{
						if (nal_unit_type == (int8_t)HEVC_VPS_NUT || nal_unit_type == (int8_t)HEVC_SPS_NUT || nal_unit_type == (int8_t)HEVC_PPS_NUT)
							fwrite(four_bytes_start_prefixes, 1, 4, m_fpDst);
						else
							fwrite(three_bytes_start_prefixes, 1, 3, m_fpDst);

						fwrite(&nuArray->Nalus[i]->nalUnit[0], 1, nuArray->Nalus[i]->nalUnitLength, m_fpDst);
					}
					bFirstNALUnit = false;
				}

				return nuArray->numNalus > 0 ? true : false;
			};

			while (cbLeft > 0)
			{
				uint32_t NALUnitLength = 0;
				if (fread(buf, 1, (size_t)m_HEVCConfigRecord->lengthSizeMinusOne + 1, m_fpSrc) != (size_t)m_HEVCConfigRecord->lengthSizeMinusOne + 1UL)
					break;

				for (int i = 0; i < m_HEVCConfigRecord->lengthSizeMinusOne + 1; i++)
					NALUnitLength = (NALUnitLength << 8) | buf[i];

				bool bLastNALUnit = cbLeft <= ((int64_t)m_HEVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength) ? true : false;

				uint8_t first_leading_read_pos = 0;
				if (keyframe == FLAG_SET && next_nal_unit_type != -1)
				{
					// read the nal_unit_type
					if (fread(buf, 1, 2, m_fpSrc) != 2)
						break;

					first_leading_read_pos = 2;

					int8_t nal_unit_type = (buf[0] >> 1) & 0x3F;
					//int8_t nuh_layer_id = ((buf[0] & 0x01) << 5) | ((buf[1] >> 3) & 0x1F);
					//int8_t nuh_temporal_id_plus1 = buf[1] & 0x07;
					if (nal_unit_type == (int8_t)HEVC_VPS_NUT ||
						nal_unit_type == (int8_t)HEVC_SPS_NUT ||
						nal_unit_type == (int8_t)HEVC_PPS_NUT ||
						nal_unit_type == (int8_t)HEVC_PREFIX_SEI_NUT)
					{
						for (int8_t i = next_nal_unit_type; i < nal_unit_type; i++)
							write_nu_array(i);

						next_nal_unit_type = (nal_unit_type == (int8_t)HEVC_PPS_NUT) ? (int8_t)HEVC_PREFIX_SEI_NUT : (
							nal_unit_type == (int8_t)HEVC_PREFIX_SEI_NUT ? (int8_t)HEVC_SUFFIX_SEI_NUT : (nal_unit_type + 1));
					}
					else if (nal_unit_type >= (int8_t)HEVC_TRAIL_N && nal_unit_type <= (int8_t)HEVC_RSV_VCL31)
					{
						for (int8_t i = next_nal_unit_type; i <= (int8_t)HEVC_PREFIX_SEI_NUT; i++)
							write_nu_array(i);

						next_nal_unit_type = (int8_t)HEVC_SUFFIX_SEI_NUT;
					}
					else
					{
						if (nal_unit_type == (int8_t)HEVC_SUFFIX_SEI_NUT)
							next_nal_unit_type = -1;

						if (next_nal_unit_type == (int8_t)HEVC_SUFFIX_SEI_NUT && bLastNALUnit)
							write_nu_array((int8_t)HEVC_SUFFIX_SEI_NUT);
					}
				}

				if (m_fpDst != NULL)
				{
					if (bFirstNALUnit == true)
						fwrite(four_bytes_start_prefixes, 1, 4, m_fpDst);
					else
						fwrite(three_bytes_start_prefixes, 1, 3, m_fpDst);
				}

				uint32_t cbLeftNALUnit = NALUnitLength - first_leading_read_pos;
				while (cbLeftNALUnit > 0)
				{
					uint32_t nCpyCnt = AMP_MIN(2048UL - first_leading_read_pos, cbLeftNALUnit);
					if ((nCpyCnt = (uint32_t)fread(&buf[first_leading_read_pos], 1, nCpyCnt, m_fpSrc)) == 0)
						break;

					if (m_fpDst != NULL)
						fwrite(buf, 1, (size_t)nCpyCnt + first_leading_read_pos, m_fpDst);

					cbLeftNALUnit -= nCpyCnt;
					first_leading_read_pos = 0;
				}

				cbLeft -= (int64_t)m_HEVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength;
				bFirstNALUnit = false;
			}

			if (cbLeft != 0)
				printf("[HEVCSampleRepacker] There are %" PRIi64 " bytes of NAL unit data is not re-packed.\n", cbLeft);

			return iRet;
		}

		int HEVCSampleRepacker::RepackNALUnitToAnnexBByteStream(uint8_t* pNalUnitBuf, int NumBytesInNalUnit, const PROCESS_DATA_INFO* NAL_Unit_DataInfo, bool* bAUCommitted)
		{
			uint8_t four_bytes_start_prefixes[4] = { 0, 0, 0, 1 };
			uint8_t three_bytes_start_prefixes[3] = { 0, 0, 1 };

			if (NumBytesInNalUnit < 2)
				return RET_CODE_BOX_TOO_SMALL;

			uint8_t picture_type = PIC_TYPE_UNKNOWN;
			//uint8_t forbidden_zero_bit = (pNalUnitBuf[0] >> 7) & 0x01;
			uint8_t nal_unit_type = (pNalUnitBuf[0] >> 1) & 0x3F;
			uint8_t nuh_layer_id = (uint8_t)(((pNalUnitBuf[0] & 0x1) << 5)&((pNalUnitBuf[1] >> 3) & 0x1F));
			//uint8_t nuh_temporal_id_plus1 = pNalUnitBuf[1] & 0x7;

			// For Non-VCL NAL unit, it will be cached until to hit the first VCL NAL unit of an access unit
			if (nal_unit_type >= 32 && nal_unit_type <= 63)
			{
				uint8_t* pNuBuf = new uint8_t[NumBytesInNalUnit];
				memcpy(pNuBuf, pNalUnitBuf, NumBytesInNalUnit);
				PROCESS_DATA_INFO data_info;
				memset(&data_info, 0, sizeof(data_info));
				if (NAL_Unit_DataInfo != nullptr)
					data_info = *NAL_Unit_DataInfo;
				m_vNonVCLNUs.push_back(std::make_tuple(pNuBuf, NumBytesInNalUnit, data_info));
				return RET_CODE_SUCCESS;
			}

			uint8_t first_slice_segment_in_pic_flag = 0;

			// For VCL NAL unit
			// Check whether it is the first slice of a coded picture in an access unit
			if ((nal_unit_type >= 0 && nal_unit_type <= 9) ||
				(nal_unit_type >= 16 && nal_unit_type <= 21))
			{
				if (NumBytesInNalUnit < 3)
					return RET_CODE_BOX_TOO_SMALL;

				// Judge the current picture type
				if (IS_IDR(nal_unit_type))
					picture_type = PIC_TYPE_IDR;
				else if (IS_CRA(nal_unit_type))
					picture_type = PIC_TYPE_CRA;
				else if (IS_IRAP(nal_unit_type))
					picture_type = PIC_TYPE_IRAP;
				else if (IS_LEADING(nal_unit_type))
					picture_type = PIC_TYPE_LEADING;
				else if (IS_TRAILING(nal_unit_type))
					picture_type = PIC_TYPE_TRAILING;

				first_slice_segment_in_pic_flag = (pNalUnitBuf[2] >> 7) & 0x1;
			}

			/*
			When one or more of the following conditions are true, the zero_byte syntax element shall be present:
			- The nal_unit_type within the nal_unit( ) syntax structure is equal to VPS_NUT, SPS_NUT or PPS_NUT.
			- The byte stream NAL unit syntax structure contains the first NAL unit of an access unit in decoding order, as specified in clause 7.4.2.4.4
			*/
			bool bStartNewAccessUnitFound = false;
			for (auto& v : m_vNonVCLNUs)
			{
				uint8_t* pNuBuf = std::get<0>(v);
				int cbNuBuf = std::get<1>(v);

				uint8_t nu_type = (pNuBuf[0] >> 1) & 0x3F;
				uint8_t nu_nuh_layer_id = ((pNuBuf[0] & 0x1) << 5)&((pNuBuf[1] >> 3) & 0x1F);

				if (first_slice_segment_in_pic_flag && nuh_layer_id == 0 &&
					bStartNewAccessUnitFound == false && (
					(nu_type == 35 && nu_nuh_layer_id == 0) || // access unit delimiter NAL unit with nuh_layer_id equal to 0 (when present)
						(nu_type == 32 && nu_nuh_layer_id == 0) || // VPS NAL unit with nuh_layer_id equal to 0 (when present)
						(nu_type == 33 && nu_nuh_layer_id == 0) || // SPS NAL unit with nuh_layer_id equal to 0 (when present)
						(nu_type == 34 && nu_nuh_layer_id == 0) || // PPS NAL unit with nuh_layer_id equal to 0 (when present)
						(nu_type == 39 && nu_nuh_layer_id == 0) || // Prefix SEI NAL unit with nuh_layer_id equal to 0 (when present)
						(nu_type >= 41 && nu_type <= 44 && nu_nuh_layer_id == 0) || // NAL units with nal_unit_type in the range of RSV_NVCL41..RSV_NVCL44 with nuh_layer_id equal to 0 (when present)
						(nu_type >= 48 && nu_type <= 55 && nu_nuh_layer_id == 0) // NAL units with nal_unit_type in the range of UNSPEC48..UNSPEC55 with nuh_layer_id equal to 0 (when present)
						))
				{
					bStartNewAccessUnitFound = true;

					/*if (fwrite(four_bytes_start_prefixes, 1, 4, m_fpDst) != 4)*/
					if (AMP_FAILED(WriteAUData(four_bytes_start_prefixes, 4, &std::get<2>(v))))
					{
						printf("[HEVCSampleRepacker] Failed to write 4 bytes of start code prefix to the output file.\n");
						return RET_CODE_ERROR;
					}

					if (m_callback_au_startpoint != nullptr)
					{
						ACCESS_UNIT_INFO au_info;
						memset(&au_info, 0, sizeof(au_info));

						au_info.picture_type = picture_type;
						m_callback_au_startpoint(m_next_AU_PTS, m_next_AU_DTS, &std::get<2>(v), &au_info, m_context_au_startpoint);
					}
				}
				else
				{
					bool is_zero_byte_present = (nu_type == 32 || nu_type == 33 || nu_type == 34) ? true : false;
					uint8_t cbStartCodePrefix = is_zero_byte_present ? 4 : 3;
					//if (fwrite(is_zero_byte_present ? four_bytes_start_prefixes : three_bytes_start_prefixes, 1, cbStartCodePrefix, m_fpDst) != cbStartCodePrefix)
					if (AMP_FAILED(WriteAUData(is_zero_byte_present ? four_bytes_start_prefixes : three_bytes_start_prefixes, cbStartCodePrefix, &std::get<2>(v))))
					{
						printf("[HEVCSampleRepacker] Failed to write %u bytes of start code prefix to the output file.\n", cbStartCodePrefix);
						return RET_CODE_ERROR;
					}
				}

				//if (fwrite(pNuBuf, 1, (size_t)cbNuBuf, m_fpDst) != (size_t)cbNuBuf)
				if (AMP_FAILED(WriteAUData(pNuBuf, cbNuBuf, &std::get<2>(v))))
				{
					printf("[HEVCSampleRepacker] Failed to write %u bytes of NAL Unit to the output file.\n", cbNuBuf);
					return RET_CODE_ERROR;
				}

				delete[] pNuBuf;
			}

			// Clear the cached Non-VCL NAL units
			m_vNonVCLNUs.clear();

			// Write the NAL unit to the output file
			bool is_zero_byte_present = (first_slice_segment_in_pic_flag && nuh_layer_id == 0 && !bStartNewAccessUnitFound) ? true : false;
			uint8_t cbStartCodePrefix = is_zero_byte_present ? 4 : 3;
			//if (fwrite(is_zero_byte_present ? four_bytes_start_prefixes : three_bytes_start_prefixes, 1, cbStartCodePrefix, m_fpDst) != cbStartCodePrefix)
			if (AMP_FAILED(WriteAUData(is_zero_byte_present ? four_bytes_start_prefixes : three_bytes_start_prefixes, cbStartCodePrefix, NAL_Unit_DataInfo)))
			{
				printf("[HEVCSampleRepacker] Failed to write %u bytes of start code prefix to the output file.\n", cbStartCodePrefix);
				return RET_CODE_ERROR;
			}

			//if (fwrite(pNalUnitBuf, 1, (size_t)NumBytesInNalUnit, m_fpDst) != (size_t)NumBytesInNalUnit)
			if (AMP_FAILED(WriteAUData(pNalUnitBuf, NumBytesInNalUnit, NAL_Unit_DataInfo)))
			{
				printf("[HEVCSampleRepacker] Failed to write %d bytes of NAL unit to the output file.\n", NumBytesInNalUnit);
				return RET_CODE_ERROR;
			}

			if (is_zero_byte_present)
			{
				bStartNewAccessUnitFound = true;

				if (m_callback_au_startpoint != nullptr)
				{
					ACCESS_UNIT_INFO au_info;
					memset(&au_info, 0, sizeof(au_info));

					au_info.picture_type = picture_type;
					m_callback_au_startpoint(m_next_AU_PTS, m_next_AU_DTS, NAL_Unit_DataInfo, &au_info, m_context_au_startpoint);
				}
			}

			if (bAUCommitted)
				*bAUCommitted = bStartNewAccessUnitFound;

			return RET_CODE_SUCCESS;
		}

		int HEVCSampleRepacker::Flush()
		{
			for (auto& v : m_vNonVCLNUs)
			{
				uint8_t* pNuBuf = std::get<0>(v);
				delete[] pNuBuf;
			}
			m_vNonVCLNUs.clear();

			AM_LRB_Reset(m_lrb_AU);
			m_data_info.valid = 0;

			return RET_CODE_SUCCESS;
		}

		int HEVCSampleRepacker::Drain()
		{
			int nRet = RET_CODE_SUCCESS;
			uint8_t four_bytes_start_prefixes[4] = { 0, 0, 0, 1 };
			uint8_t three_bytes_start_prefixes[3] = { 0, 0, 1 };

			for (auto& v : m_vNonVCLNUs)
			{
				uint8_t* pNuBuf = std::get<0>(v);
				int cbNuBuf = std::get<1>(v);

				uint8_t nu_type = (pNuBuf[0] >> 1) & 0x3F;
				//uint8_t nu_nuh_layer_id = ((pNuBuf[0] & 0x1) << 5)&((pNuBuf[1] >> 3) & 0x1F);

				bool is_zero_byte_present = (nu_type == 32 || nu_type == 33 || nu_type == 34) ? true : false;
				{
					uint8_t cbStartCodePrefix = is_zero_byte_present ? 4 : 3;
					//if (fwrite(is_zero_byte_present ? four_bytes_start_prefixes : three_bytes_start_prefixes, 1, cbStartCodePrefix, m_fpDst) != cbStartCodePrefix)
					if (AMP_FAILED(WriteAUData(is_zero_byte_present ? four_bytes_start_prefixes : three_bytes_start_prefixes, cbStartCodePrefix, &std::get<2>(v))))
					{
						printf("[HEVCSampleRepacker] Failed to flush %u bytes of start code prefix to the output file.\n", cbStartCodePrefix);
						nRet = RET_CODE_ERROR;
						goto done;
					}

					//if (fwrite(pNuBuf, 1, (size_t)cbNuBuf, m_fpDst) != (size_t)cbNuBuf)
					if (AMP_FAILED(WriteAUData(pNuBuf, cbNuBuf, &std::get<2>(v))))
					{
						printf("[HEVCSampleRepacker] Failed to flush %u bytes of NAL Unit to the output file.\n", cbNuBuf);
						nRet = RET_CODE_ERROR;
						goto done;
					}
				}
			}

			// Forcedly commit the AU data
			CommitAU();

		done:
			for (auto& v : m_vNonVCLNUs)
			{
				uint8_t* pNuBuf = std::get<0>(v);
				delete[] pNuBuf;
			}
			m_vNonVCLNUs.clear();

			return nRet;
		}

	}
}

