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
#ifndef _NAL_COM_H_
#define _NAL_COM_H_

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "combase.h"
#include "AMArray.h"
#include "AMBitStream.h"
#include "DumpUtil.h"
#include "dump_data_type.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

enum NAL_CODING
{
	NAL_CODING_UNKNOWN = -1,
	NAL_CODING_AVC = 1,	// H.264, Advanced video coding
	NAL_CODING_HEVC,	// H.265, High efficiency video coding
	NAL_CODING_VVC,		// H.266, Versatile video coding
};

#define NAL_CODING_NAME(c)	((c) == NAL_CODING_AVC?"AVC":((c) == NAL_CODING_HEVC?"HEVC":((c) == NAL_CODING_VVC?"VVC":"Unknown")))

namespace BST {
	namespace H264Video {
		struct NAL_UNIT;
	}

	namespace H265Video {
		struct NAL_UNIT;
	}

	namespace H266Video {
		struct NAL_UNIT;
	}
}

using H264_NU = std::shared_ptr<BST::H264Video::NAL_UNIT>;
using H265_NU = std::shared_ptr<BST::H265Video::NAL_UNIT>;
using H266_NU = std::shared_ptr<BST::H266Video::NAL_UNIT>;

class INALContext: public IUnknown
{
public:
	virtual NAL_CODING		GetNALCoding() = 0;
	virtual RET_CODE		SetNUFilters(std::initializer_list<uint8_t> NU_type_filters) = 0;
	virtual RET_CODE		GetNUFilters(std::vector<uint8_t>& NU_type_filters) = 0;
	virtual bool			IsNUFiltered(uint8_t nal_unit_type) = 0;
	virtual RET_CODE		SetActiveNUType(int8_t nu_type) = 0;
	virtual int8_t			GetActiveNUType() = 0;
	virtual void			Reset() = 0;

public:
	INALContext() {}
	virtual ~INALContext() {}
};

class INALAVCContext : public INALContext
{
public:
	virtual H264_NU			GetAVCSPS(uint8_t sps_id) = 0;
	virtual H264_NU			GetAVCPPS(uint8_t pps_id) = 0;
	virtual RET_CODE		UpdateAVCSPS(H264_NU sps_nu) = 0;
	virtual RET_CODE		UpdateAVCPPS(H264_NU pps_nu) = 0;
	virtual H264_NU			CreateAVCNU() = 0;

	virtual int8_t			GetActiveSPSID() = 0;
	/*
		When a sequence parameter set RBSP (with a particular value of seq_parameter_set_id) is not already active 
		and it is referred to by activation of a picture parameter set RBSP (using that value of seq_parameter_set_id) 
		or is referred to by an SEI NAL unit containing a buffering period SEI message (using that value of seq_parameter_set_id), 
		it is activated. This sequence parameter set RBSP is called the active sequence parameter set RBSP until it is deactivated 
		by the activation of another sequence parameter set RBSP. A sequence parameter set RBSP, with that particular value of 
		seq_parameter_set_id, shall be available to the decoding process prior to its activation. An activated sequence parameter 
		set RBSP shall remain active for the entire coded video sequence.
	*/
	virtual	RET_CODE		ActivateSPS(int8_t sps_id) = 0;
	virtual RET_CODE		DetactivateSPS() = 0;
};

class INALHEVCContext : public INALContext
{
public:
	virtual H265_NU			GetHEVCVPS(uint8_t vps_id) = 0;
	virtual H265_NU			GetHEVCSPS(uint8_t sps_id) = 0;
	virtual H265_NU			GetHEVCPPS(uint8_t pps_id) = 0;
	virtual RET_CODE		UpdateHEVCVPS(H265_NU vps_nu) = 0;
	virtual RET_CODE		UpdateHEVCSPS(H265_NU sps_nu) = 0;
	virtual RET_CODE		UpdateHEVCPPS(H265_NU pps_nu) = 0;
	virtual H265_NU			CreateHEVCNU() = 0;

	virtual int8_t			GetActiveSPSID() = 0;
	/*
		When an SPS RBSP (with a particular value of sps_seq_parameter_set_id) is not already active for the base layer 
		and it is referred to by activation of a PPS RBSP (in which pps_seq_parameter_set_id is equal to the sps_seq_parameter_set_id value) 
		for the base layer or, when vps_base_layer_internal_flag is equal to 1 and vps_base_layer_available_flag is equal to 1, 
		is referred to by an SEI NAL unit containing an active parameter sets SEI message (in which active_seq_parameter_set_id[0] is equal 
		to the sps_seq_parameter_set_id value), it is activated for the base layer. 
		This SPS RBSP is called the active SPS RBSP for the base layer until it is deactivated by the activation of another SPS RBSP for the base layer. 
		An SPS RBSP, with that particular value of sps_seq_parameter_set_id, shall be available to the decoding process prior to its activation, 
		included in at least one access unit with TemporalId equal to 0 or provided through external means, and the SPS NAL unit containing 
		the SPS RBSP shall have nuh_layer_id equal to 0. An activated SPS RBSP for the base layer shall remain active for the entire coded video sequence (CVS).
	*/
	virtual	RET_CODE		ActivateSPS(int8_t sps_id) = 0;
	virtual RET_CODE		DetactivateSPS() = 0;
};

class INALVVCContext : public INALContext
{
public:
	virtual H266_NU			GetVVCDCI() = 0;
	virtual H266_NU			GetVVCOPI() = 0;
	virtual H266_NU			GetVVCVPS(uint8_t vps_id) = 0;
	virtual H266_NU			GetVVCSPS(uint8_t sps_id) = 0;
	virtual H266_NU			GetVVCPPS(uint8_t pps_id) = 0;
	virtual H266_NU			GetVVCPH() = 0;
	virtual RET_CODE		UpdateVVCDCI(H266_NU dci_nu) = 0;
	virtual RET_CODE		UpdateVVCOPI(H266_NU opi_nu) = 0;
	virtual RET_CODE		UpdateVVCVPS(H266_NU vps_nu) = 0;
	virtual RET_CODE		UpdateVVCSPS(H266_NU sps_nu) = 0;
	virtual RET_CODE		UpdateVVCPPS(H266_NU pps_nu) = 0;
	virtual RET_CODE		UpdateVVCPH(H266_NU ph_nu) = 0;
	virtual H266_NU			CreateVVCNU() = 0;

	virtual RET_CODE		GetNumRefIdxActive(int8_t num_ref_idx_active[2]) = 0;
	virtual RET_CODE		GetRplsIdx(int8_t rpls_idx[2]) = 0;
	virtual int8_t			GetActiveSPSID() = 0;
	virtual	RET_CODE		ActivateSPS(int8_t sps_id) = 0;
	virtual RET_CODE		DetactivateSPS() = 0;
};

extern RET_CODE CreateAVCNALContext(INALAVCContext** ppNALCtx);
extern RET_CODE CreateHEVCNALContext(INALHEVCContext** ppNALCtx);
extern RET_CODE CreateVVCNALContext(INALVVCContext** ppNALCtx);

inline uint8_t* FindNALStartCodePrefixOne3Bytes(uint8_t* pBuf, unsigned long cbSize)
{
	// Find the start_code_prefix_one_3bytes
	while (cbSize >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
	{
		cbSize--;
		pBuf++;
	}

	if (cbSize < 3)
		return NULL;

	return pBuf;
}

inline uint8_t* NextNALStart(uint8_t* pBuf, unsigned long cbSize)
{
	// Find the start_code_prefix_one_3bytes
	uint8_t* pStart = pBuf;
	while (cbSize >= 3 && !(pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1))
	{
		cbSize--;
		pBuf++;
	}

	if (cbSize < 3)
		return NULL;

	if (pBuf > pStart && *(pBuf - 1) == 0)
	{
		pBuf--;
		cbSize++;
	}

	return pBuf;
}

#define Clip3(x, y, z)	(z)<(x)?(x):((z)>(y)?(y):(z))

extern const char* itu_t_t35_country_code_names[256];
extern const char* cc_type_names[4];
extern const char* pic_struct_names[16];

namespace BST {

	extern bool more_rbsp_data(AMBst in_bst);

	extern float get_hevc_sar(uint8_t aspect_ratio_idc, uint16_t sar_width, uint16_t sar_height);

	struct RBSP_TRAILING_BITS : public SYNTAX_BITSTREAM_MAP
	{
		CAMBitArray	rbsp_trailing_bits;

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				do {
					nal_read_bitarray(in_bst, rbsp_trailing_bits, map_status.number_of_fields);
				} while (!AMBst_IsAligned(in_bst));
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
			NAV_WRITE_TAG_BEGIN2("rbsp_trailing_bits");
			BST_FIELD_PROP_NUMBER("rbsp_stop_one_bit", 1, rbsp_trailing_bits[i], "Should be 1");
			BST_FIELD_PROP_NUMBER_BITARRAY("rbsp_alignment_zero_bit", rbsp_trailing_bits, "Should be all 0", 1);
			NAV_WRITE_TAG_END2("rbsp_trailing_bits");
		DECLARE_FIELDPROP_END()
	};

	struct RBSP_SLICE_TRAILING_BITS : public SYNTAX_BITSTREAM_MAP
	{
		RBSP_TRAILING_BITS		rbsp_trailing_bits;
		std::vector<uint16_t>	cabac_zero_words;

		bool entropy_coding_mode_flag;

		RBSP_SLICE_TRAILING_BITS(bool bEntropyCodingModeFlag): entropy_coding_mode_flag(bEntropyCodingModeFlag){}

		int Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			try
			{
				MAP_BST_BEGIN(0);
				nal_read_obj(in_bst, rbsp_trailing_bits);
				if (entropy_coding_mode_flag)
				{
					while (AMBst_NAL_more_rbsp_data(in_bst))
					{
						uint16_t cabac_zero_word;
						nal_read_u(in_bst, cabac_zero_word, 16, uint16_t);
						cabac_zero_words.push_back(cabac_zero_word);
					}
				}
				MAP_BST_END();
			}
			catch (AMException e)
			{
				return e.RetCode();
			}

			return RET_CODE_SUCCESS;
		}

		int Unmap(AMBst out_bst)
		{
			UNREFERENCED_PARAMETER(out_bst);
			return RET_CODE_ERROR_NOTIMPL;
		}

		DECLARE_FIELDPROP_BEGIN()
		NAV_WRITE_TAG_BEGIN2("rbsp_slice_trailing_bits");
		BST_FIELD_PROP_OBJECT(rbsp_trailing_bits)
		if (entropy_coding_mode_flag)
		{
			for (i = 0; i < (int)cabac_zero_words.size(); i++)
			{
				BST_ARRAY_FIELD_PROP_NUMBER("cabac_zero_word", i, 16, cabac_zero_words[i], "Should be 00 00");
			}
		}
		NAV_WRITE_TAG_END2("rbsp_slice_trailing_bits");
		DECLARE_FIELDPROP_END()

	};

}

#ifdef _WIN32
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

#endif
