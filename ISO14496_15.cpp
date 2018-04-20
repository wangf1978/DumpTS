#include "StdAfx.h"
#include "ISO14496_15.h"

namespace ISOBMFF
{
	int NALAUSampleRepackerBase::Seek(uint64_t src_sample_file_offset)
	{
		if (_fseeki64(m_fpSrc, src_sample_file_offset, SEEK_SET) != 0)
		{
			printf("Failed to seek to the file position: %lld.\n", src_sample_file_offset);
			return -1;
		}

		return 0;
	}

	int	NALAUSampleRepackerBase::RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe)
	{
		return -1;
	}

	int	AVCSampleRepacker::RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe)
	{
		uint8_t buf[2048];
		int64_t cbLeft = sample_size;
		int iRet = RET_CODE_SUCCESS;
		int8_t next_nal_unit_type = (int8_t)AVC_NAL_UNIT_TYPE::SPS_NUT;
		bool bFirstNALUnit = true;

		uint8_t four_bytes_start_prefixes[4] = { 0, 0, 0, 1 };
		uint8_t three_bytes_start_prefixes[3] = { 0, 0, 1 };

		auto write_nu_array = [&](int8_t nal_unit_type) {
			if (nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::SPS_NUT)
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
			else if (nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::PPS_NUT)
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
			else if (nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::SPS_EXT_NUT)
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
			fread(buf, 1, m_AVCConfigRecord->lengthSizeMinusOne + 1, m_fpSrc);
			for (int i = 0; i < m_AVCConfigRecord->lengthSizeMinusOne + 1; i++)
				NALUnitLength = (NALUnitLength << 8) | buf[i];

			bool bLastNALUnit = cbLeft <= (m_AVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength) ? true : false;

			uint8_t first_leading_read_pos = 0;
			if (keyframe == FLAG_SET && next_nal_unit_type != -1)
			{
				// read the nal_unit_type
				if (fread(buf, 1, 1, m_fpSrc) != 1)
					break;

				first_leading_read_pos = 1;

				int8_t nal_ref_idc = (buf[0] >> 5) & 0x03;
				int8_t nal_unit_type = buf[0] & 0x1F;
				if (nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::SPS_NUT || nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::PPS_NUT)
				{
					for (int8_t i = next_nal_unit_type; i < nal_unit_type; i++)
						write_nu_array(i);

					next_nal_unit_type = nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::PPS_NUT ? (int8_t)AVC_NAL_UNIT_TYPE::SPS_EXT_NUT : (nal_unit_type + 1);
				}
				else if (nal_unit_type >= 1 && nal_unit_type <= 5)
				{
					for (int8_t i = next_nal_unit_type; i <= (int8_t)AVC_NAL_UNIT_TYPE::SPS_EXT_NUT; i++)
						write_nu_array(i);
					next_nal_unit_type = -1;
				}
				else if (nal_unit_type == (int8_t)AVC_NAL_UNIT_TYPE::SEI_NUT)
				{
					for (int8_t i = next_nal_unit_type; i <= (int8_t)AVC_NAL_UNIT_TYPE::PPS_NUT; i++)
						write_nu_array(i);

					next_nal_unit_type = (int8_t)AVC_NAL_UNIT_TYPE::SPS_EXT_NUT;
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

				if ((nCpyCnt = fread(&buf[first_leading_read_pos], 1, nCpyCnt, m_fpSrc)) == 0)
					break;

				if (m_fpDst != NULL)
					fwrite(buf, 1, nCpyCnt + first_leading_read_pos, m_fpDst);

				cbLeftNALUnit -= nCpyCnt;
				first_leading_read_pos = 0;
			}

			cbLeft -= m_AVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength;
			bFirstNALUnit = false;
		}

		if (cbLeft != 0)
			printf("[AVCSampleRepacker] There are %lld bytes of NAL unit data is not re-packed.\n", cbLeft);

		return iRet;
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
		int8_t next_nal_unit_type = (int8_t)HEVC_NAL_UNIT_TYPE::VPS_NUT;
		bool bFirstNALUnit = true;

		auto write_nu_array = [&](int8_t nal_unit_type) {
			if (nu_array_info[nal_unit_type].nu_array_idx == -1)
				return false;

			auto nuArray = m_HEVCConfigRecord->nalArray[nu_array_info[nal_unit_type].nu_array_idx];
			for (size_t i = 0; i < nuArray->numNalus; i++)
			{
				if (m_fpDst != NULL)
				{
					if (nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::VPS_NUT || nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::SPS_NUT || nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::PPS_NUT)
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
			fread(buf, 1, m_HEVCConfigRecord->lengthSizeMinusOne + 1, m_fpSrc);
			for (int i = 0; i < m_HEVCConfigRecord->lengthSizeMinusOne + 1; i++)
				NALUnitLength = (NALUnitLength << 8) | buf[i];

			bool bLastNALUnit = cbLeft <= (m_HEVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength) ? true : false;

			uint8_t first_leading_read_pos = 0;
			if (keyframe == FLAG_SET && next_nal_unit_type != -1)
			{
				// read the nal_unit_type
				if (fread(buf, 1, 2, m_fpSrc) != 2)
					break;

				first_leading_read_pos = 2;

				int8_t nal_unit_type = (buf[0] >> 1) & 0x3F;
				int8_t nuh_layer_id = ((buf[0] & 0x01) << 5) | ((buf[1] >> 3) & 0x1F);
				int8_t nuh_temporal_id_plus1 = buf[1] & 0x07;
				if (nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::VPS_NUT ||
					nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::SPS_NUT ||
					nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::PPS_NUT ||
					nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::PREFIX_SEI_NUT)
				{
					for (int8_t i = next_nal_unit_type; i < nal_unit_type; i++)
						write_nu_array(i);

					next_nal_unit_type = (nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::PPS_NUT) ? (int8_t)HEVC_NAL_UNIT_TYPE::PREFIX_SEI_NUT : (
						nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::PREFIX_SEI_NUT ? (int8_t)HEVC_NAL_UNIT_TYPE::SUFFIX_SEI_NUT : (nal_unit_type + 1));
				}
				else if (nal_unit_type >= (int8_t)HEVC_NAL_UNIT_TYPE::TRAIL_N && nal_unit_type <= (int8_t)HEVC_NAL_UNIT_TYPE::RSV_VCL31)
				{
					for (int8_t i = next_nal_unit_type; i <= (int8_t)HEVC_NAL_UNIT_TYPE::PREFIX_SEI_NUT; i++)
						write_nu_array(i);

					next_nal_unit_type = (int8_t)HEVC_NAL_UNIT_TYPE::SUFFIX_SEI_NUT;
				}
				else
				{
					if (nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::SUFFIX_SEI_NUT)
						next_nal_unit_type = -1;

					if (next_nal_unit_type == (int8_t)HEVC_NAL_UNIT_TYPE::SUFFIX_SEI_NUT && bLastNALUnit)
						write_nu_array((int8_t)HEVC_NAL_UNIT_TYPE::SUFFIX_SEI_NUT);
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

				if ((nCpyCnt = fread(&buf[first_leading_read_pos], 1, nCpyCnt, m_fpSrc)) == 0)
					break;

				if (m_fpDst != NULL)
					fwrite(buf, 1, nCpyCnt + first_leading_read_pos, m_fpDst);

				cbLeftNALUnit -= nCpyCnt;
				first_leading_read_pos = 0;
			}

			cbLeft -= m_HEVCConfigRecord->lengthSizeMinusOne + 1 + NALUnitLength;
			bFirstNALUnit = false;
		}

		if (cbLeft != 0)
			printf("[HEVCSampleRepacker] There are %lld bytes of NAL unit data is not re-packed.\n", cbLeft);

		return iRet;
	}

}
