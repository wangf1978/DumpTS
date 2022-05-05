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
#ifndef __MSE_NAV_H__
#define __MSE_NAV_H__

#include "AMRFC3986.h"
#include "systemdef.h"
#include "MSE.h"
#include "nal_com.h"

#define MSE_UNSPECIFIED				INT64_MAX
#define MSE_UNSELECTED				INT64_MIN
#define MSE_MIN_MSEID				(0)
#define MSE_MAX_MSEID				(INT64_MAX - 1)
#define MSE_EXCLUDE(mseid)			(((mseid) >=0 && (mseid) <=  MSE_MAX_MSEID)?~(mseid):(mseid))
#define IS_INCLUSIVE_MSEID(mseid)	(((mseid) >=0 && (mseid) <=  MSE_MAX_MSEID))
#define IS_EXCLUSIVE_MSEID(mseid)	(((mseid) < 0 && (mseid) >= -MSE_MAX_MSEID - 1))
#define MSEID_EXCLUDED(mseid)		(((mseid) < 0 && (mseid) >= -MSE_MAX_MSEID - 1)?(~(mseid)):(mseid))

#define NAV_START					0
#define NAV_END						1

enum NU_FILTER_TYPE
{
	NU_FILTER_VCL	= 0,
	NU_FILTER_SEI,
	NU_FILTER_AUD,
	NU_FILTER_VPS,
	NU_FILTER_SPS,
	NU_FILTER_PPS,
	NU_FILTER_IDR,
	NU_FILTER_FIL,
	NU_FILTER_MAX
};

#define NU_FILTER_NAME(ft)	(\
	(ft) == NU_FILTER_VCL?"VCL-NU":(\
	(ft) == NU_FILTER_SEI?"SEI-NU":(\
	(ft) == NU_FILTER_AUD?"AUD-NU":(\
	(ft) == NU_FILTER_VPS?"VPS-NU":(\
	(ft) == NU_FILTER_SPS?"SPS-NU":(\
	(ft) == NU_FILTER_PPS?"PPS-NU":(\
	(ft) == NU_FILTER_IDR?"IDR-NU":(\
	(ft) == NU_FILTER_FIL?"FIL-NU":"NU"))))))))

#define NU_FILTER_URI_NAME(ft)	(\
	(ft) == NU_FILTER_VCL?"VCL":(\
	(ft) == NU_FILTER_SEI?"SEINU":(\
	(ft) == NU_FILTER_AUD?"AUD":(\
	(ft) == NU_FILTER_VPS?"VPS":(\
	(ft) == NU_FILTER_SPS?"SPS":(\
	(ft) == NU_FILTER_PPS?"PPS":(\
	(ft) == NU_FILTER_IDR?"IDR":(\
	(ft) == NU_FILTER_FIL?"FIL":"NU"))))))))

struct MSENav;

extern int	CreateMSEParser(IMSEParser** ppMSEParser);
extern void	PrintMSEHeader(IMSEParser* pMSEParser, IUnknown* pCtx, uint32_t enum_options, MSENav& mse_nav, FILE* fp);
extern void	LocateMSEParseStartPosition(IUnknown* pCtx, FILE* rfp);

using MSEID = int64_t;

struct MSEID_RANGE
{
	MSEID		id[2];

	MSEID_RANGE() :MSEID_RANGE(MSE_UNSELECTED, MSE_UNSELECTED) {}
	MSEID_RANGE(MSEID start, MSEID end) :id{ start, end } {}

	bool IsNull() const { return id[0] == MSE_UNSELECTED && id[1] == MSE_UNSELECTED; }
	bool IsAllExcluded() const { return id[0] == MSE_EXCLUDE(0) && id[1] == MSE_EXCLUDE(MSE_MAX_MSEID); }
	bool IsAllUnspecfied() const { return id[0] == MSE_UNSPECIFIED && id[1] == MSE_UNSPECIFIED; }
	bool IsNaR() const { return (id[0] == MSE_UNSELECTED && id[1] != MSE_UNSELECTED) || (id[1] == MSE_UNSELECTED && id[0] != MSE_UNSELECTED); }
	void Reset(int64_t mseid = MSE_UNSELECTED) { id[0] = id[1] = mseid; }
	bool Contain(MSEID it) {
		if ((IS_INCLUSIVE_MSEID(id[0]) && it >= id[0]) || id[0] == MSE_UNSPECIFIED)
		{
			if ((IS_INCLUSIVE_MSEID(id[1]) && it <= id[1]) || id[1] == MSE_UNSPECIFIED)
				return true;
			else if (IS_EXCLUSIVE_MSEID(id[1]) && it > MSEID_EXCLUDED(id[1]))
				return true;
		}
		else if ((IS_EXCLUSIVE_MSEID(id[0]) && it < MSEID_EXCLUDED(id[0])) || (IS_EXCLUSIVE_MSEID(id[1]) && it > MSEID_EXCLUDED(id[1])))
			return true;
		return false;
	}
	/*
	            ____ahead                        ____behind
	           /    _________________           /
	          /    /                 \         /
	    |----it----a0-----------------a1------it---------|
	*/
	bool Ahead(MSEID it)	// the current range is ahead the specified point or not
	{
		if ((IS_INCLUSIVE_MSEID(id[0]) && it < id[0]))
			return true;
		if ((id[0] == MSE_UNSPECIFIED || (IS_EXCLUSIVE_MSEID(id[0]) && MSEID_EXCLUDED(id[0])) == 0) && IS_EXCLUSIVE_MSEID(id[1]) && it <= MSEID_EXCLUDED(id[1]))
			return true;
		return false;
	}

	bool Behind(MSEID it)
	{
		if (IS_INCLUSIVE_MSEID(id[1]) && it > id[1])
			return true;
		return false;
	}
};

struct MSENav
{
	MEDIA_SCHEME_TYPE	scheme_type;
	union
	{
		MSEID_RANGE			mse_id_ranges[16];
		struct
		{
			MSEID_RANGE		sei_pl;						// SEI payload index
			MSEID_RANGE		sei_msg;					// SEI message index
			MSEID_RANGE		nu;							// NAL unit index
			MSEID_RANGE		vcl_nu;						// VCL NAL unit index
			MSEID_RANGE		sei_nu;						// SEI NAL unit index
			MSEID_RANGE		aud_nu;						// AUD NAL unit index
			MSEID_RANGE		vps_nu;						// VPS NAL unit index
			MSEID_RANGE		sps_nu;						// SPS NAL unit index
			MSEID_RANGE		pps_nu;						// PPS NAL unit index
			MSEID_RANGE		IDR_nu;						// IDR NAL unit index
			MSEID_RANGE		FIL_nu;						// FIL NAL unit index
			MSEID_RANGE		au;							// Access unit index
			MSEID_RANGE		cvs;						// Codec video sequence index
			MSEID_RANGE		vseq;
		}NAL;
		struct
		{
			MSEID_RANGE		au;
		}AUDIO;
		struct
		{
			MSEID_RANGE		obu_frame;					// Including FRAME_HEADER, FRAME and REDUNDANT_FRAME_HEADER
			MSEID_RANGE		obu;
			MSEID_RANGE		fu;
			MSEID_RANGE		tu;
			MSEID_RANGE		cvs;
			MSEID_RANGE		vseq;
		}AV1;
		struct
		{
			MSEID_RANGE		mb;
			// slice is a subset of mpeg2 video syntax element
			// if se is specified, slice should not occur in URI
			// if se is unselected or unspecified, but there is slice in URI, use slice se filter
			MSEID_RANGE		slice;
			MSEID_RANGE		se;
			MSEID_RANGE		au;
			MSEID_RANGE		gop;
			MSEID_RANGE		vseq;
		}MPV;
	};

	std::vector<std::tuple<uint32_t/*box type*/, MSEID/*the box index with the specified box-type of same parent box*/>>
					isobmff_boxes;

	std::vector<std::string>
					insidepath;

	MSENav(MEDIA_SCHEME_TYPE schemeType)
		: scheme_type(schemeType) {
		InitAs(MSE_UNSELECTED);
	}

	int				Load(uint32_t enum_options);
	uint32_t		GetEnumOptions();

protected:
	void			InitAs(MSEID mseid);
	int				LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments, 
									  std::vector<std::string>& supported_authority_components, MSEID_RANGE* ranges[]);

	int				LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments);
	int				LoadMP4AuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments);

	int				CheckNALAuthorityComponent(size_t idxComponent);
	int				CheckAudioLAuthorityComponent(size_t idxComponent);
	int				CheckAV1AuthorityComponent(size_t idxComponent);
	int				CheckMPVAuthorityComponent(size_t idxComponent);
	int				NormalizeAuthorityPart();
	bool			CheckNUFilterCoexist(size_t idxComponent);

	static std::vector<std::string>
					nal_supported_authority_components;
	static std::vector<std::string>
					audio_supported_authority_components;
	static std::vector<std::string>
					av1_supported_authority_components;
	static std::vector<std::string>
					mpv_supported_authority_components;
};


//
// MPEG2 video enumerator for MSE nav operation
//
class IMPVContext;
class CMPVNavEnumerator : public CComUnknown, public IMPVEnumerator
{
public:
	CMPVNavEnumerator(IMPVContext* pCtx, uint32_t options, MSENav* pMSENav);
	virtual ~CMPVNavEnumerator();

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj);

public:
	RET_CODE			EnumVSEQStart(IUnknown* pCtx);
	RET_CODE			EnumNewGOP(IUnknown* pCtx, bool closed_gop, bool broken_link);
	RET_CODE			EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType);
	RET_CODE			EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode);
	RET_CODE			EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType);
	RET_CODE			EnumVSEQEnd(IUnknown* pCtx);
	RET_CODE			EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code);

protected:
	virtual RET_CODE	OnProcessVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessGOP(IUnknown* pCtx, bool closed_gop, bool broken_link) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessAU(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode) { return RET_CODE_SUCCESS; }

protected:
	int					m_level[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int64_t				m_unit_index[16] = { 0 };
	int64_t				m_curr_slice_count = 0;

	std::string			GetURI(int level_id);
	RET_CODE			CheckFilter(int level_id, uint8_t start_code = 0);
	const char*			GetSEName(uint8_t* pBufWithStartCode, size_t cbBufWithStartCode);
	bool				OnlySliceSE();
	int					AppendURI(char* szItem, size_t& ccWritten, int level_id);

protected:
	IMPVContext*		m_pMPVContext;
	MSENav*				m_pMSENav;
	int					m_nLastLevel = -1;
	uint32_t			m_options = 0;
	const size_t		column_width_name = 47;
	const size_t		column_width_len = 13;
	const size_t		column_width_URI = 27;
	const size_t		right_padding = 1;
};

inline bool CMPVNavEnumerator::OnlySliceSE()
{
	if (m_pMSENav == nullptr)
		return false;

	return (m_pMSENav->MPV.se.IsAllUnspecfied()) &&
		(m_pMSENav->MPV.slice.IsAllUnspecfied() || (!m_pMSENav->MPV.slice.IsNull() && !m_pMSENav->MPV.slice.IsNaR())) &&
		!(m_pMSENav->MPV.slice.IsAllExcluded());;
}

inline int CMPVNavEnumerator::AppendURI(char* szItem, size_t& ccWritten, int level_id)
{
	std::string szFormattedURI = GetURI(level_id);
	if (szFormattedURI.length() < column_width_URI)
	{
		memset(szItem + ccWritten, ' ', column_width_URI - szFormattedURI.length());
		ccWritten += column_width_URI - szFormattedURI.length();
	}

	memcpy(szItem + ccWritten, szFormattedURI.c_str(), szFormattedURI.length());
	ccWritten += szFormattedURI.length();

	return RET_CODE_SUCCESS;
}

//
// NAL video enumerator for MSE operation
//
class INALContext;
class CNALNavEnumerator : public CComUnknown, public INALEnumerator
{
public:
	CNALNavEnumerator(INALContext* pCtx, uint32_t options, MSENav* pMSENav);
	virtual ~CNALNavEnumerator();

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj);

public:
	RET_CODE			EnumNewVSEQ(IUnknown* pCtx);
	RET_CODE			EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type);
	RET_CODE			EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type);
	RET_CODE			EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf);
	RET_CODE			EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf);
	RET_CODE			EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf);
	RET_CODE			EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf);
	RET_CODE			EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf);
	RET_CODE			EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf);
	RET_CODE			EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf);
	RET_CODE			EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code);

protected:
	virtual RET_CODE	OnProcessVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessCVS(IUnknown* pCtx, int8_t represent_nal_unit_type) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessAU(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessNU(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessSEIMessage(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessSEIPayload(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf) { return RET_CODE_SUCCESS; }

protected:
	int					m_level[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int64_t				m_unit_index[16] = { 0 };
	int64_t				m_filtered_nu_count[NU_FILTER_MAX] = { 0 };
	int8_t				m_curr_nu_type = -1;
	NAL_CODING			m_curr_nal_coding = NAL_CODING_UNKNOWN;

	inline bool			OnlyFilterNU(NU_FILTER_TYPE nu_filter_type);

	std::string			GetURI(int level_id);

	RET_CODE			CheckFilter(int level_id, uint8_t nal_unit_type = 0xFF);
	const char*			GetNUName(uint8_t nal_unit_type);
	const char*			GetSEIPayoadTypeName(uint32_t payload_type);

	inline int			AppendURI(char* szItem, size_t& ccWritten, int level_id);
	inline int			CheckNALUnitEBSP(uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf, uint8_t& nalUnitHeaderBytes, uint8_t& nal_unit_type);

protected:
	INALContext*		m_pNALContext;
	MSENav*				m_pMSENav;
	int					m_nLastLevel = -1;
	uint32_t			m_options = 0;
	const size_t		column_width_name = 47;
	const size_t		column_width_len = 13;
	const size_t		column_width_URI = 36;
	const size_t		right_padding = 1;
};

inline bool CNALNavEnumerator::OnlyFilterNU(NU_FILTER_TYPE nu_filter_type)
{
	if (m_pMSENav == nullptr)
		return false;

	MSEID_RANGE* subset_nu[] = {
		&m_pMSENav->NAL.vcl_nu, &m_pMSENav->NAL.sei_nu, &m_pMSENav->NAL.aud_nu, &m_pMSENav->NAL.vps_nu,
		&m_pMSENav->NAL.sps_nu, &m_pMSENav->NAL.pps_nu, &m_pMSENav->NAL.IDR_nu, &m_pMSENav->NAL.FIL_nu };

	return (m_pMSENav->NAL.nu.IsAllUnspecfied()) && 
			(subset_nu[nu_filter_type]->IsAllUnspecfied() || (!subset_nu[nu_filter_type]->IsNull() && !subset_nu[nu_filter_type]->IsNaR())) &&
			!(subset_nu[nu_filter_type]->IsAllExcluded());
}

inline int CNALNavEnumerator::AppendURI(char* szItem, size_t& ccWritten, int level_id)
{
	std::string szFormattedURI = GetURI(level_id);
	if (szFormattedURI.length() < column_width_URI)
	{
		memset(szItem + ccWritten, ' ', column_width_URI - szFormattedURI.length());
		ccWritten += column_width_URI - szFormattedURI.length();
	}

	memcpy(szItem + ccWritten, szFormattedURI.c_str(), szFormattedURI.length());
	ccWritten += szFormattedURI.length();

	return RET_CODE_SUCCESS;
}

inline int CNALNavEnumerator::CheckNALUnitEBSP(uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf, uint8_t& nalUnitHeaderBytes, uint8_t& nal_unit_type)
{
	if (pEBSPNUBuf == nullptr || cbEBSPNUBuf < 1)
		return RET_CODE_INVALID_PARAMETER;

	if (m_curr_nal_coding == NAL_CODING_AVC)
	{
		nalUnitHeaderBytes = 1;

		int8_t forbidden_zero_bit = (pEBSPNUBuf[0] >> 7) & 0x01;
		int8_t nal_ref_idc = (pEBSPNUBuf[0] >> 5) & 0x3;
		nal_unit_type = pEBSPNUBuf[0] & 0x1F;

		int8_t svc_extension_flag = 0;
		int8_t avc_3d_extension_flag = 0;

		if (nal_unit_type == 14 || nal_unit_type == 20 || nal_unit_type == 21)
		{
			if (nal_unit_type != 21)
				svc_extension_flag = (pEBSPNUBuf[1] >> 7) & 0x01;
			else
				avc_3d_extension_flag = (pEBSPNUBuf[1] >> 7) & 0x01;

			if (svc_extension_flag)
			{
				nalUnitHeaderBytes += 3;
				if (cbEBSPNUBuf < 5)
					return RET_CODE_NEEDMOREINPUT;
			}
			else if (avc_3d_extension_flag)
			{
				nalUnitHeaderBytes += 2;
				if (cbEBSPNUBuf < 4)
					return RET_CODE_NEEDMOREINPUT;
			}
			else
			{
				nalUnitHeaderBytes += 3;
				if (cbEBSPNUBuf < 5)
					return RET_CODE_NEEDMOREINPUT;
			}
		}
	}
	else if (m_curr_nal_coding == NAL_CODING_HEVC)
	{
		nalUnitHeaderBytes = 2;
		if (cbEBSPNUBuf < 2)
			return RET_CODE_NEEDMOREINPUT;

		nal_unit_type = (pEBSPNUBuf[0] >> 1) & 0x3F;;
	}
	else if (m_curr_nal_coding == NAL_CODING_VVC)
	{
		nalUnitHeaderBytes = 2;
		if (cbEBSPNUBuf < 2)
			return RET_CODE_NEEDMOREINPUT;

		nal_unit_type = (pEBSPNUBuf[1] >> 3) & 0x1F;
	}
	else
		return RET_CODE_ERROR_NOTIMPL;
		
	return RET_CODE_SUCCESS;
}

//
// AV1 video enumerator for MSE operation
//
class IAV1Context;
class CAV1NavEnumerator : public CComUnknown, public IAV1Enumerator
{
public:
	CAV1NavEnumerator(IAV1Context* pCtx, uint32_t options, MSENav* pMSENav);
	virtual ~CAV1NavEnumerator();

	DECLARE_IUNKNOWN
	HRESULT				NonDelegatingQueryInterface(REFIID uuid, void** ppvObj);

public:
	RET_CODE			EnumNewVSEQ(IUnknown* pCtx);
	RET_CODE			EnumNewCVS(IUnknown* pCtx, int8_t reserved);
	/*
		For the Annex-B byte-stream:
		The first OBU in the first frame_unit of each temporal_unit must be a temporal delimiter OBU 
		(and this is the only place temporal delimiter OBUs can appear).
	*/
	RET_CODE			EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type);
	RET_CODE			EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type);
	RET_CODE			EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size);
	RET_CODE			EnumFrameUnitEnd(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf);
	RET_CODE			EnumTemporalUnitEnd(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size);
	RET_CODE			EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code);

protected:
	virtual RET_CODE	OnProcessVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessCVS(IUnknown* pCtx, int8_t reserved) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessTU(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessFU(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type) { return RET_CODE_SUCCESS; }
	virtual RET_CODE	OnProcessOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size){ return RET_CODE_SUCCESS; }

protected:
	int					m_level[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int64_t				m_unit_index[16] = { 0 };
	int64_t				m_curr_frameobu_count = 0;
	bool				m_bAnnexB = false;

	bool				OnlyFrameOBU();
	std::string			GetURI(int level_id);
	RET_CODE			CheckFilter(int level_id, uint8_t obu_type = 0xFF);
	const char*			GetOBUName(uint8_t obu_type);
	int					AppendURI(char* szItem, size_t& ccWritten, int level_id);

protected:
	IAV1Context*		m_pAV1Context;
	MSENav*				m_pMSENav;
	int					m_nLastLevel = -1;
	uint32_t			m_options = 0;
	const size_t		column_width_name = 47;
	const size_t		column_width_len = 13;
	const size_t		column_width_URI = 31;
	const size_t		right_padding = 1;
};

inline bool CAV1NavEnumerator::OnlyFrameOBU()
{
	if (m_pMSENav == nullptr)
		return false;

	return (m_pMSENav->AV1.obu.IsAllUnspecfied()) &&
		(m_pMSENav->AV1.obu_frame.IsAllUnspecfied() || (!m_pMSENav->AV1.obu_frame.IsNull() && !m_pMSENav->AV1.obu_frame.IsNaR())) &&
		!(m_pMSENav->AV1.obu_frame.IsAllExcluded());
}

inline int CAV1NavEnumerator::AppendURI(char* szItem, size_t& ccWritten, int level_id)
{
	std::string szFormattedURI = GetURI(level_id);
	if (szFormattedURI.length() < column_width_URI)
	{
		memset(szItem + ccWritten, ' ', column_width_URI - szFormattedURI.length());
		ccWritten += column_width_URI - szFormattedURI.length();
	}

	memcpy(szItem + ccWritten, szFormattedURI.c_str(), szFormattedURI.length());
	ccWritten += szFormattedURI.length();

	return RET_CODE_SUCCESS;
}

#endif
