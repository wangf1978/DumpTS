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
#ifndef __NAL_PARSER_BUF__
#define __NAL_PARSER_BUF__

#include "dump_data_type.h"
#include "NAL.h"
#include "nal_com.h"
#include "AMRingBuffer.h"
#include "AMSHA1.h"
#include "MSE.h"

enum NAL_BYTESTREAM_FORMAT
{
	NAL_BYTESTREAM_RAW = 0,			// (NAL_Unit)+
	NAL_BYTESTREAM_ANNEX_B,			// ([leading_zero_8bits][zero_byte] start_code_prefix_one_3bytes NAL_Unit [trailing_zero_8bits])+
	NAL_BYTESTREAM_ISO,				// ( NAL_Length + NAL_Unit)+
};

struct NAL_SEQUENCE
{
	uint32_t					presentation_start_time;
	uint64_t					start_byte_pos;
	uint16_t					pic_width_in_luma_samples;
	uint16_t					pic_height_in_luma_samples;

	uint8_t						aspect_ratio_info_present_flag : 1;
	uint8_t						colour_description_present_flag : 1;
	uint8_t						timing_info_present_flag : 1;
	uint8_t						fixed_frame_rate_flag : 1;
	uint8_t						units_field_based_flag : 1;	// for H264, it is the equivalent with nuit_field_based_flag
	uint8_t						reserved_for_future_use_0 : 3;

	uint8_t						aspect_ratio_idc;
	uint16_t					sar_width;
	uint16_t					sar_height;

	uint8_t						colour_primaries;
	uint8_t						transfer_characteristics;
	uint8_t						matrix_coeffs;

	uint32_t					num_units_in_tick;
	uint32_t					time_scale;

	bool						pic_parameter_set_id_sel[256];

	int16_t GetLayoutSize()
	{
		int16_t layout_size = 21;	// length ... reserved_for_future_use:4
		if (aspect_ratio_info_present_flag)
		{
			layout_size += 1;
			if (aspect_ratio_idc == 0xFF)
				layout_size += 4;
		}

		if (colour_description_present_flag)
			layout_size += 3;

		if (timing_info_present_flag)
			layout_size += 8;

		uint8_t num_of_ref_pps = 0;
		for (size_t idxPPS = 0; idxPPS < _countof(pic_parameter_set_id_sel); idxPPS++)
		{
			if (pic_parameter_set_id_sel[idxPPS])
				num_of_ref_pps++;
		}

		layout_size += 1 + num_of_ref_pps * sizeof(uint8_t);

		return layout_size;
	}

	// 100 nano-second
	int64_t GetFrameDuration() {
		return time_scale == 0 ? 0 : (int32_t)((long long)num_units_in_tick * 10000000LL * ((long long)units_field_based_flag + 1) / time_scale);
	}
};

class CNALParser : public CComUnknown, public IMSEParser
{
public:
	CNALParser(NAL_CODING coding, NAL_BYTESTREAM_FORMAT fmt = NAL_BYTESTREAM_ANNEX_B, uint8_t NULenDelimiterSize = 4 , RET_CODE* pRetCode=nullptr);
	virtual ~CNALParser();

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_IMSEParser)
			return GetCOMInterface((IMSEParser*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	MEDIA_SCHEME_TYPE		GetSchemeType() { return MEDIA_SCHEME_NAL; }
	RET_CODE				SetEnumerator(IUnknown* pEnumerator, uint32_t options);
	RET_CODE				ProcessInput(uint8_t* pBuf, size_t cbBuf);
	RET_CODE				ProcessOutput(bool bDrain = false);
	RET_CODE				GetContext(IUnknown** ppCtx);
	RET_CODE				Reset();

protected:
	int						PushESBP(uint8_t* pStart, uint8_t* pEnd);
	int						LoadVVCParameterSet(uint8_t* pNUBuf, int cbNUBuf, uint64_t cur_submit_pos);
	int						LoadHEVCParameterSet(uint8_t* pNUBuf, int cbNUBuf, uint64_t cur_submit_pos);
	int						LoadAVCParameterSet(uint8_t* pNUBuf, int cbNUBuf, uint64_t cur_submit_pos);
	int						PickupLastSliceHeaderInfo(uint8_t* pNUBuf, int cbNUBuf);

	//
	// slice_type: -1, used the previous slice
	// slice_type: -2, pps is missed, can't detect its slice type
	int						PickupVVCSliceHeaderInfo(NAL_UNIT_ENTRY& nu_entry, uint8_t* pNUBuf, int cbNUBuf);
	int						PickupHEVCSliceHeaderInfo(NAL_UNIT_ENTRY& nu_entry, uint8_t* pNUBuf, int cbNUBuf);
	int						PickupAVCSliceHeaderInfo(NAL_UNIT_ENTRY& nu_entry, uint8_t* pNUBuf, int cbNUBuf);

	/*!	@brief Commit a NAL unit to the ring buffer.
		@remarks If current processed byte-stream is Annex-B format, number_of_leading_bytes should be 3 or 4
		it only covers "start_code_prefix_one_3bytes" or "zero_byte + start_code_prefix_one_3bytes"
	*/
	int						CommitNALUnit(uint8_t number_of_leading_bytes = 0);
	int						CommitSliceInfo(bool bDrain);
	int						CommitVVCPicture(std::vector<NAL_UNIT_ENTRY>::const_iterator pic_start,
											  std::vector<NAL_UNIT_ENTRY>::const_iterator pic_end);
	int						CommitHEVCPicture(std::vector<NAL_UNIT_ENTRY>::const_iterator pic_start,
											  std::vector<NAL_UNIT_ENTRY>::const_iterator pic_end);
	int						CommitAVCPicture(std::vector<NAL_UNIT_ENTRY>::const_iterator pic_start,
											 std::vector<NAL_UNIT_ENTRY>::const_iterator pic_end);


	int						ParseNALUnit(uint8_t* pNUBuf, int cbNUBuf);
	int						ParseSEINU(uint8_t* pNUBuf, int cbNUBuf);

	RET_CODE				ProcessAnnexBOutput(bool bDrain);

protected:
	NAL_CODING				m_nal_coding;
	NAL_BYTESTREAM_FORMAT	m_nal_bytestream_format;
	uint8_t					m_nal_length_delimiter_size;
	INALContext*			m_pCtx;
	union
	{
		INALAVCContext*		m_pNALAVCCtx;
		INALHEVCContext*	m_pNALHEVCCtx;
		INALVVCContext*		m_pNALVVCCtx;
	};
	INALEnumerator*			m_nal_enum = nullptr;
	uint32_t				m_nal_enum_options = NAL_ENUM_OPTION_ALL;

	const int				read_unit_size = 2048;
	const int				minimum_nal_parse_buffer_size = 5;	// 3 prefix start code + 2 NAL unit header

	AMLinearRingBuffer		m_rbRawBuf = nullptr;
	/*!	@brief It is used to store Byte stream NAL unit including:
			   zero_byte + start_code_prefix_one_3bytes + nal_unit(NumBytesInNALunit ) + trailing_zero_8bits*/
	AMLinearRingBuffer		m_rbNALUnitEBSP = nullptr;

	/*! @brief It indicates how many bytes are already parsed from the first bytes into the parser since 
			   the parser is created or reset */
	uint64_t				m_cur_scan_pos = 0;
	/*! @brief It indicates the position of the last complete NAL unit including the leading bytes before NAL Unit 
			   Leading_zero_8bits + zero_byte + start_code_prefix_one_3bytes */
	uint64_t				m_cur_submit_pos = 0;
	uint8_t					m_cur_nal_unit_prefix_start_code_length = 0;
	uint32_t				m_cur_submit_nal_unit_len = 0;
	uint64_t				m_count_nal_unit_scanned = 0;

	std::vector<NAL_UNIT_ENTRY>
							m_nu_entries;
	NAL_UNIT_ENTRY			m_last_first_slice_nu_entry;
	int8_t					m_cur_nal_unit_type = -1;
	bool					m_hit_PH_NU_in_one_AU = false;

	union
	{
		struct  // For AVC
		{
			int64_t			avc_presentation_time_code;	// in the unit of 100-nano seconds
			uint32_t		avc_num_units_in_tick;
			uint32_t		avc_time_scale;
			uint8_t			avc_nuit_field_based_flag;
		};
		struct // For HEVC
		{
			int64_t			hevc_presentation_time_code;// in the unit of 100-nano seconds
			uint32_t		hevc_num_units_in_tick;
			uint32_t		hevc_time_scale;
			uint8_t			hevc_units_field_based_flag;
		};
		struct // For VVC
		{
			int64_t			vvc_presentation_time_code;	// in the unit of 100-nano seconds
			uint32_t		vvc_num_units_in_tick;
			uint32_t		vvc_time_scale;
			uint8_t			vui_progressive_source_flag : 1;
			uint8_t			vui_interlaced_source_flag : 1;
			uint8_t			byte_align0 : 6;
		};
		uint8_t				state_buf[512];
	};

	std::vector<NAL_SEQUENCE>
							nal_sequences;
};

#endif
