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
#include "CodecID.h"
#include "DataUtil.h"
#include "mpeg2_video.h"
#include "MSE.h"
#include "h264_video.h"
#include "h265_video.h"
#include "h266_video.h"
#include "nal_parser.h"
#include "av1.h"
#include "mpeg2_video.h"
#include "ISO14496_3.h"
#include "mediaobjprint.h"
#include "MSEnav.h"

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern MEDIA_SCHEME_TYPE g_source_media_scheme_type;
extern int AV1_PreparseStream(const char* szAV1FileName, bool& bIsAnnexB);

const char* g_szRule    = "                                                                                                                        ";
const char* g_szHorizon = "------------------------------------------------------------------------------------------------------------------------";

//
// MPEG2 video enumerator for MSE operation
//
class CMPVSEEnumerator : public CMPVNavEnumerator
{
public:
	CMPVSEEnumerator(IMPVContext* pCtx, uint32_t options, MSENav* pMSENav) 
		: CMPVNavEnumerator(pCtx, options, pMSENav) {}

public:
	RET_CODE OnProcessVSEQ(IUnknown* pCtx)
	{ 
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sVideo Sequence#%" PRId64 "",
			(m_level[MPV_LEVEL_VSEQ]) * 4, g_szRule, m_unit_index[m_level[MPV_LEVEL_VSEQ]]);

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%12s |", " ")) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, MPV_LEVEL_VSEQ);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS; 
	}

	RET_CODE OnProcessAU(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sAU#%" PRId64 " (%s)", 
			(m_level[MPV_LEVEL_AU]) * 4, g_szRule, m_unit_index[m_level[MPV_LEVEL_AU]],
			PICTURE_CODING_TYPE_SHORTNAME(picCodingType));

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbAUBuf).c_str())) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, MPV_LEVEL_AU);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS; 
	}

	RET_CODE OnProcessGOP(IUnknown* pCtx, bool closed_gop, bool broken_link)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sGOP#%" PRId64 " (%s%s)",
			(m_level[MPV_LEVEL_GOP]) * 4, g_szRule, m_unit_index[m_level[MPV_LEVEL_GOP]], closed_gop?"closed":"open", broken_link?",broken-link":"");

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%12s |"," ")) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, MPV_LEVEL_GOP);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE OnProcessObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
	{
		if (cbBufWithStartCode < 4 || cbBufWithStartCode > INT32_MAX)
			return RET_CODE_NOTHING_TODO;

		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;
		int enum_cmd = m_options & MSE_ENUM_CMD_MASK;

		if (enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
		{
			bool onlyShowSlice = OnlySliceSE();
			ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s",
				(m_level[MPV_LEVEL_SE]) * 4, g_szRule, onlyShowSlice?"Slice": "SE",
				onlyShowSlice?m_curr_slice_count:m_unit_index[m_level[MPV_LEVEL_SE]],
				GetSEName(pBufWithStartCode, cbBufWithStartCode));

			if (ccWrittenOnce <= 0)
				return RET_CODE_NOTHING_TODO;

			if ((size_t)ccWrittenOnce < column_width_name)
				memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

			szItem[column_width_name] = '|';
			ccWritten = column_width_name + 1;

			if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbBufWithStartCode).c_str())) <= 0)
				return RET_CODE_NOTHING_TODO;

			ccWritten += ccWrittenOnce;

			AppendURI(szItem, ccWritten, MPV_LEVEL_SE);

			szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

			printf("%s\n", szItem);
		}
			
		if (m_options&(MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
		{
			if (m_nLastLevel == MPV_LEVEL_SE)
			{
				int indent = 4 * m_level[MPV_LEVEL_SE];
				int right_part_len = int(column_width_name + 1 + column_width_len + 1 + column_width_URI + 1);

				printf("%.*s%.*s\n", indent, g_szRule, right_part_len - indent, g_szHorizon);
				if (m_options&MSE_ENUM_SYNTAX_VIEW)
					PrintMPVSyntaxElement(pCtx, pBufWithStartCode, cbBufWithStartCode, 4 * m_level[MPV_LEVEL_SE]);
				else if (m_options&MSE_ENUM_HEX_VIEW)
					print_mem(pBufWithStartCode, (int)cbBufWithStartCode, 4 * m_level[MPV_LEVEL_SE]);
				printf("\n");
			}
		}

		return RET_CODE_SUCCESS;
	}
};

//
// NAL video enumerator for MSE operation
//
class CNALSEEnumerator : public CComUnknown, public INALEnumerator
{
public:
	CNALSEEnumerator(INALContext* pCtx, uint32_t options, MSENav* pMSENav)
		: m_pNALContext(pCtx), m_pMSENav(pMSENav) {
		if (m_pNALContext != nullptr) {
			m_pNALContext->AddRef();
			m_curr_nal_coding = m_pNALContext->GetNALCoding();
		}

		int next_level = 0;
		for (size_t i = 0; i < _countof(m_level); i++) {
			if (options&(1ULL << i)) {
				m_level[i] = next_level;

				// CVS is the point, not a range
				if (i == NAL_LEVEL_VSEQ || i == NAL_LEVEL_CVS)
					m_unit_index[next_level] = -1;
				else
					m_unit_index[next_level] = 0;

				m_nLastLevel = (int)i;
				next_level++;
			}
		}

		m_options = options;
	}

	virtual ~CNALSEEnumerator() {
		AMP_SAFERELEASE(m_pNALContext);
	}

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_INALEnumerator)
			return GetCOMInterface((INALEnumerator*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	RET_CODE EnumNewVSEQ(IUnknown* pCtx)
	{
		m_unit_index[m_level[NAL_LEVEL_VSEQ]]++;
		if (m_level[NAL_LEVEL_CVS] > 0)
			m_unit_index[m_level[NAL_LEVEL_CVS]] = -1;
		if (m_level[NAL_LEVEL_AU] > 0)
			m_unit_index[m_level[NAL_LEVEL_AU]] = 0;
		if (m_level[NAL_LEVEL_NU] > 0)
			m_unit_index[m_level[NAL_LEVEL_NU]] = 0;
		memset(m_filtered_nu_count, 0, sizeof(m_filtered_nu_count));
		if (m_level[NAL_LEVEL_SEI_MSG] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_MSG]] = 0;
		if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_VSEQ)) != RET_CODE_SUCCESS)
			return iRet;

		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sVideo Sequence#%" PRId64 "",
			(m_level[NAL_LEVEL_VSEQ]) * 4, g_szRule, m_unit_index[m_level[NAL_LEVEL_VSEQ]]);

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%12s |", " ")) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, NAL_LEVEL_VSEQ);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		m_unit_index[m_level[NAL_LEVEL_CVS]]++;
		if (m_level[NAL_LEVEL_AU] > 0)
			m_unit_index[m_level[NAL_LEVEL_AU]] = 0;
		if (m_level[NAL_LEVEL_NU] > 0)
			m_unit_index[m_level[NAL_LEVEL_NU]] = 0;
		memset(m_filtered_nu_count, 0, sizeof(m_filtered_nu_count));
		if (m_level[NAL_LEVEL_SEI_MSG] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_MSG]] = 0;
		if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_CVS)) != RET_CODE_SUCCESS)
			return iRet;

		const char* szCVSType = "";
		if (m_curr_nal_coding == NAL_CODING_HEVC)
			szCVSType = IS_CRA(represent_nal_unit_type) ? "(CRA, open GOP)" 
														: (IS_BLA(represent_nal_unit_type) 
															? "(BLA)" 
															: (IS_IDR(represent_nal_unit_type) ? "(IDR, closed GOP)" : ""));
		else if (m_curr_nal_coding == NAL_CODING_AVC)
			szCVSType = represent_nal_unit_type == AVC_CS_IDR_PIC ? "(IDR, closed GOP)" : "(I, open GOP)";
		else if (m_curr_nal_coding == NAL_CODING_VVC)
			szCVSType = IS_VVC_CRA(represent_nal_unit_type) ? "(CRA, open GOP)" : (IS_VVC_IDR(represent_nal_unit_type) ? "(IDR, closed GOP)" : "");

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sCVS#%" PRId64 " %s",
			(m_level[NAL_LEVEL_CVS]) * 4, g_szRule, m_unit_index[m_level[NAL_LEVEL_CVS]], szCVSType);

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%12s |", " ")) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, NAL_LEVEL_CVS);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_AU)) != RET_CODE_SUCCESS)
			return iRet;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sAU#%" PRId64 " (%s)",
			(m_level[NAL_LEVEL_AU]) * 4, g_szRule, m_unit_index[m_level[NAL_LEVEL_AU]],
			m_curr_nal_coding == NAL_CODING_AVC?AVC_PIC_SLICE_TYPE_NAMEA(picture_slice_type):(
			m_curr_nal_coding == NAL_CODING_HEVC?HEVC_PIC_SLICE_TYPE_NAMEA(picture_slice_type):(
			m_curr_nal_coding == NAL_CODING_VVC? VVC_PIC_SLICE_TYPE_NAMEA(picture_slice_type) : "")));

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbEBSPAUBuf).c_str())) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, NAL_LEVEL_AU);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
	{
		uint8_t cbNALUnitHdr = 0, nal_unit_type = 0;
		int iRet = CheckNALUnitEBSP(pEBSPNUBuf, cbEBSPNUBuf, cbNALUnitHdr, nal_unit_type);
		if (AMP_FAILED(iRet))
			return iRet;

		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		if ((iRet = CheckFilter(NAL_LEVEL_NU, nal_unit_type)) == RET_CODE_SUCCESS)
		{
			m_curr_nu_type = nal_unit_type;

			int enum_cmd = m_options & MSE_ENUM_CMD_MASK;
			if (enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				bool bSpecifiedNUFilter = false;
				for (int i = 0; i < NU_FILTER_MAX; i++)
				{
					if (OnlyFilterNU((NU_FILTER_TYPE)i))
					{
						bSpecifiedNUFilter = true;
						ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s",
							(m_level[NAL_LEVEL_NU]) * 4, g_szRule, NU_FILTER_NAME(i), m_filtered_nu_count[i], GetNUName(nal_unit_type));
						break;
					}
				}

				if (!bSpecifiedNUFilter)
				{
					ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s::%s",
						(m_level[NAL_LEVEL_NU]) * 4, g_szRule, "NU", m_unit_index[m_level[NAL_LEVEL_NU]],
						m_curr_nal_coding == NAL_CODING_AVC ?  (IS_AVC_VCL_NAL(nal_unit_type)  ? "VCL" : "non-VCL") : (
						m_curr_nal_coding == NAL_CODING_HEVC ? (IS_HEVC_VCL_NAL(nal_unit_type) ? "VCL" : "non-VCL") : (
						m_curr_nal_coding == NAL_CODING_VVC ?  (IS_VVC_VCL_NAL(nal_unit_type)  ? "VCL" : "non-VCL") : "")),
						GetNUName(nal_unit_type));
				}

				if (ccWrittenOnce <= 0)
					return RET_CODE_NOTHING_TODO;

				if ((size_t)ccWrittenOnce < column_width_name)
					memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

				szItem[column_width_name] = '|';
				ccWritten = column_width_name + 1;

				if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbEBSPNUBuf).c_str())) <= 0)
					return RET_CODE_NOTHING_TODO;

				ccWritten += ccWrittenOnce;

				AppendURI(szItem, ccWritten, NAL_LEVEL_NU);

				szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

				printf("%s\n", szItem);
			}

			if (enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				if (m_nLastLevel == NAL_LEVEL_NU)
				{
					int indent = 4 * m_level[NAL_LEVEL_NU];
					int right_part_len = int(column_width_name + 1 + column_width_len + 1 + column_width_URI + 1);

					printf("%.*s%.*s\n", indent, g_szRule, right_part_len - indent, g_szHorizon);
					if (m_options&MSE_ENUM_SYNTAX_VIEW)
						PrintNALUnitSyntaxElement(pCtx, pEBSPNUBuf, cbEBSPNUBuf, 4 * m_level[NAL_LEVEL_NU]);
					else if (m_options&MSE_ENUM_HEX_VIEW)
						print_mem(pEBSPNUBuf, (int)cbEBSPNUBuf, 4 * m_level[NAL_LEVEL_NU]);
					printf("\n");
				}
			}
		}

		return iRet;
	}

	RET_CODE EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_SEI_MSG)) == RET_CODE_SUCCESS)
		{
			int enum_cmd = m_options & MSE_ENUM_CMD_MASK;
			if (enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " ",
					(m_level[NAL_LEVEL_SEI_MSG]) * 4, g_szRule, "SEI message",
					m_unit_index[m_level[NAL_LEVEL_SEI_MSG]]);

				if (ccWrittenOnce <= 0)
					return RET_CODE_NOTHING_TODO;

				if ((size_t)ccWrittenOnce < column_width_name)
					memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

				szItem[column_width_name] = '|';
				ccWritten = column_width_name + 1;

				if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbRBSPSEIMsgBuf).c_str())) <= 0)
					return RET_CODE_NOTHING_TODO;

				ccWritten += ccWrittenOnce;

				AppendURI(szItem, ccWritten, NAL_LEVEL_SEI_MSG);

				szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

				printf("%s\n", szItem);
			}

			if (enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				if (m_nLastLevel == NAL_LEVEL_SEI_MSG)
				{
					int indent = 4 * m_level[NAL_LEVEL_SEI_MSG];
					int right_part_len = int(column_width_name + 1 + column_width_len + 1 + column_width_URI + 1);

					printf("%.*s%.*s\n", indent, g_szRule, right_part_len - indent, g_szHorizon);
					if (m_options&MSE_ENUM_SYNTAX_VIEW)
						PrintSEIMsgSyntaxElement(pCtx, pRBSPSEIMsgRBSPBuf, cbRBSPSEIMsgBuf, 4 * m_level[NAL_LEVEL_SEI_MSG]);
					else if (m_options&MSE_ENUM_HEX_VIEW)
						print_mem(pRBSPSEIMsgRBSPBuf, (int)cbRBSPSEIMsgBuf, 4 * m_level[NAL_LEVEL_SEI_MSG]);
					printf("\n");
				}
			}
		}

		return iRet;
	}

	RET_CODE EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_SEI_PAYLOAD)) == RET_CODE_SUCCESS)
		{
			int enum_cmd = m_options & MSE_ENUM_CMD_MASK;
			if (enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s#%" PRId64 " %s ",
					(m_level[NAL_LEVEL_SEI_PAYLOAD]) * 4, g_szRule,
					m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]], GetSEIPayoadTypeName(payload_type));

				if (ccWrittenOnce <= 0)
					return RET_CODE_NOTHING_TODO;

				if ((size_t)ccWrittenOnce < column_width_name)
					memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

				szItem[column_width_name] = '|';
				ccWritten = column_width_name + 1;

				if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbRBSPPayloadBuf).c_str())) <= 0)
					return RET_CODE_NOTHING_TODO;

				ccWritten += ccWrittenOnce;

				AppendURI(szItem, ccWritten, NAL_LEVEL_SEI_PAYLOAD);

				szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

				printf("%s\n", szItem);
			}

			if (enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				if (m_nLastLevel == NAL_LEVEL_SEI_PAYLOAD)
				{
					int indent = 4 * m_level[NAL_LEVEL_SEI_PAYLOAD];
					int right_part_len = int(column_width_name + 1 + column_width_len + 1 + column_width_URI + 1);

					printf("%.*s%.*s\n", indent, g_szRule, right_part_len - indent, g_szHorizon);
					if (m_options&MSE_ENUM_SYNTAX_VIEW)
						PrintSEIPayloadSyntaxElement(pCtx, payload_type, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf, 4 * m_level[NAL_LEVEL_SEI_PAYLOAD]);
					else if (m_options&MSE_ENUM_HEX_VIEW)
						print_mem(pRBSPSEIPayloadBuf, (int)cbRBSPPayloadBuf, 4 * m_level[NAL_LEVEL_SEI_PAYLOAD]);
					printf("\n");
				}
			}
		}

		return iRet;
	}

	RET_CODE EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
	{
		m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]]++;
		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
	{
		m_unit_index[m_level[NAL_LEVEL_SEI_MSG]]++;
		if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;
		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
	{
		if (m_curr_nal_coding == NAL_CODING_AVC)
		{
			if (IS_AVC_VCL_NAL(m_curr_nu_type)){
				if (m_curr_nu_type == AVC_CS_IDR_PIC)
					m_filtered_nu_count[NU_FILTER_IDR]++;
				m_filtered_nu_count[NU_FILTER_VCL]++;
			}
			else if (m_curr_nu_type == AVC_SEI_NUT) m_filtered_nu_count[NU_FILTER_SEI]++;
			else if (m_curr_nu_type == AVC_AUD_NUT) m_filtered_nu_count[NU_FILTER_AUD]++;
			else if (m_curr_nu_type == AVC_SPS_NUT) m_filtered_nu_count[NU_FILTER_SPS]++;
			else if (m_curr_nu_type == AVC_PPS_NUT) m_filtered_nu_count[NU_FILTER_PPS]++;
			else if (m_curr_nu_type == AVC_FD_NUT)  m_filtered_nu_count[NU_FILTER_FIL]++;
		}
		else if (m_curr_nal_coding == NAL_CODING_HEVC)
		{
			if (IS_HEVC_VCL_NAL(m_curr_nu_type)){
				if (IS_IDR(m_curr_nu_type))
					m_filtered_nu_count[NU_FILTER_IDR]++;
				m_filtered_nu_count[NU_FILTER_VCL]++;
			}
			else if (m_curr_nu_type == HEVC_PREFIX_SEI_NUT || m_curr_nu_type == HEVC_SUFFIX_SEI_NUT)
				m_filtered_nu_count[NU_FILTER_SEI]++;
			else if (m_curr_nu_type == HEVC_AUD_NUT) m_filtered_nu_count[NU_FILTER_AUD]++;
			else if (m_curr_nu_type == HEVC_VPS_NUT) m_filtered_nu_count[NU_FILTER_VPS]++;
			else if (m_curr_nu_type == HEVC_SPS_NUT) m_filtered_nu_count[NU_FILTER_SPS]++;
			else if (m_curr_nu_type == HEVC_PPS_NUT) m_filtered_nu_count[NU_FILTER_PPS]++;
			else if (m_curr_nu_type == HEVC_FD_NUT)  m_filtered_nu_count[NU_FILTER_FIL]++;
		}
		else if (m_curr_nal_coding == NAL_CODING_VVC)
		{
			if (IS_VVC_VCL_NAL(m_curr_nu_type)) {
				if (IS_VVC_IDR(m_curr_nu_type))
					m_filtered_nu_count[NU_FILTER_IDR]++;
				m_filtered_nu_count[NU_FILTER_VCL]++;
			}
			else if (m_curr_nu_type == VVC_PREFIX_SEI_NUT || m_curr_nu_type == VVC_SUFFIX_SEI_NUT)
				m_filtered_nu_count[NU_FILTER_SEI]++;
			else if (m_curr_nu_type == VVC_AUD_NUT) m_filtered_nu_count[NU_FILTER_AUD]++;
			else if (m_curr_nu_type == VVC_VPS_NUT) m_filtered_nu_count[NU_FILTER_VPS]++;
			else if (m_curr_nu_type == VVC_SPS_NUT) m_filtered_nu_count[NU_FILTER_SPS]++;
			else if (m_curr_nu_type == VVC_PPS_NUT) m_filtered_nu_count[NU_FILTER_PPS]++;
			else if (m_curr_nu_type == VVC_FD_NUT)  m_filtered_nu_count[NU_FILTER_FIL]++;
		}

		m_unit_index[m_level[NAL_LEVEL_NU]]++;
		if (m_level[NAL_LEVEL_SEI_MSG] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_MSG]] = 0;
		if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;

		m_curr_nu_type = -1;

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf)
	{
		m_unit_index[m_level[NAL_LEVEL_AU]]++;
		if (m_level[NAL_LEVEL_NU] > 0)
			m_unit_index[m_level[NAL_LEVEL_NU]] = 0;
		memset(m_filtered_nu_count, 0, sizeof(m_filtered_nu_count));
		if (m_level[NAL_LEVEL_SEI_MSG] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_MSG]] = 0;
		if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code) { return RET_CODE_SUCCESS; }

protected:
	int						m_level[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int64_t					m_unit_index[16] = { 0 };
	int64_t					m_filtered_nu_count[NU_FILTER_MAX] = { 0 };
	int8_t					m_curr_nu_type = -1;
	NAL_CODING				m_curr_nal_coding = NAL_CODING_UNKNOWN;

	inline bool OnlyFilterNU(NU_FILTER_TYPE nu_filter_type)
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

	std::string	GetURI(int level_id)
	{
		std::string strURI;
		strURI.reserve(128);
		for (int i = level_id; i >= 0; i--)
		{
			if (m_level[i] >= 0)
			{
				if (strURI.length() > 0)
					strURI += ".";
		
				bool bSpecifiedNUFilter = false;
				if (i == NAL_LEVEL_NU)
				{
					for (int i = 0; i < NU_FILTER_MAX; i++)
					{
						if (OnlyFilterNU((NU_FILTER_TYPE)i))
						{
							bSpecifiedNUFilter = true;
							strURI += NU_FILTER_URI_NAME(i) + std::to_string(m_filtered_nu_count[i]);
							break;
						}
					}
				}
				if (!bSpecifiedNUFilter)
				{
					strURI += NAL_LEVEL_NAME(i);
					strURI += std::to_string(m_unit_index[m_level[i]]);
				}
			}
		}

		return strURI;
	}

	RET_CODE CheckFilter(int level_id, uint8_t nal_unit_type=0xFF)
	{
		if (m_pMSENav == nullptr)
			return RET_CODE_SUCCESS;

		if ((m_nLastLevel == NAL_LEVEL_SEI_MSG || m_nLastLevel == NAL_LEVEL_SEI_PAYLOAD) && level_id == NAL_LEVEL_NU && m_level[level_id] < 0)
		{
			if (nal_unit_type != 0xFF && !(
				(m_curr_nal_coding == NAL_CODING_AVC  && (nal_unit_type == BST::H264Video::SEI_NUT)) ||
				(m_curr_nal_coding == NAL_CODING_HEVC && (nal_unit_type >= BST::H265Video::PREFIX_SEI_NUT || nal_unit_type == BST::H265Video::SUFFIX_SEI_NUT)) ||
				(m_curr_nal_coding == NAL_CODING_VVC  && (nal_unit_type >= BST::H266Video::PREFIX_SEI_NUT || nal_unit_type == BST::H266Video::SUFFIX_SEI_NUT))))
			{
				return RET_CODE_NOTHING_TODO;
			}
		}

		MSEID_RANGE filter[] = { MSEID_RANGE(), MSEID_RANGE(), m_pMSENav->NAL.vseq, m_pMSENav->NAL.cvs, m_pMSENav->NAL.au, m_pMSENav->NAL.nu, m_pMSENav->NAL.sei_msg,
			m_pMSENav->NAL.sei_pl, MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE() };

		bool bNullParent = true;
		for (int i = 0; i <= level_id; i++)
		{
			if (m_level[i] < 0 || filter[i].IsAllUnspecfied())
				continue;

			if (filter[i].Ahead(m_unit_index[m_level[i]]))
				return RET_CODE_NOTHING_TODO;
			else if (filter[i].Behind(m_unit_index[m_level[i]]))
			{
				if (bNullParent)
					return RET_CODE_ABORT;
				else
					return RET_CODE_NOTHING_TODO;
			}
			/*
				In this case, although filter is not ahead of or behind the id, but it also does NOT include the id
				____________                      ______________________________
							\                    /
				|///////////f0xxxxxxxxxidxxxxxxxxxf1\\\\\\\\\\\\\\\\\\\\\\\\....
			*/
			else if (!filter[i].Contain(m_unit_index[m_level[i]]))
				return RET_CODE_NOTHING_TODO;

			if (!filter[i].IsNull() && !filter[i].IsNaR())
			{
				if (i == NAL_LEVEL_NU)
				{
					if ((m_nLastLevel == NAL_LEVEL_SEI_MSG || m_nLastLevel == NAL_LEVEL_SEI_PAYLOAD) && m_pMSENav->NAL.sei_nu.IsAllExcluded())
					{

					}
					else
						bNullParent = false;
				}
				else
				bNullParent = false;
			}
		}

		// Do some special processing since VCL_NU or SEI_NU is a subset of NU
		if (level_id == NAL_LEVEL_NU && m_pMSENav->NAL.nu.IsAllUnspecfied())
		{
			// Since NU has sub-set, and if it is all unspecified, it means one of sub-set is applied
			uint8_t bExcludeAllFilter[NU_FILTER_MAX] = { 0 };
			MSEID_RANGE* subset_nu[] = {
				&m_pMSENav->NAL.vcl_nu, &m_pMSENav->NAL.sei_nu, &m_pMSENav->NAL.aud_nu, &m_pMSENav->NAL.vps_nu,
				&m_pMSENav->NAL.sps_nu, &m_pMSENav->NAL.pps_nu, &m_pMSENav->NAL.IDR_nu, &m_pMSENav->NAL.FIL_nu };
			bool bExcludeAllFilters = true;
			bool bExcludeAllFiltersExcept[NU_FILTER_MAX];
			for (int ft = 0; ft < NU_FILTER_MAX; ft++)
				bExcludeAllFiltersExcept[ft] = true;

			for (int ft = 0; ft < NU_FILTER_MAX; ft++)
			{
				bExcludeAllFilter[ft] = subset_nu[ft]->IsAllExcluded() || subset_nu[ft]->IsNull() || subset_nu[ft]->IsNaR();
				if (!bExcludeAllFilter[ft])
				{
					bExcludeAllFilters = false;
					for (int other = 0; other < NU_FILTER_MAX; other++) {
						if (other != ft)
							bExcludeAllFiltersExcept[other] = false;
					}
				}
			}

			for (int ft = 0; ft < NU_FILTER_MAX; ft++)
			{
				bool bHit = false;
				if (subset_nu[ft]->IsNull() || subset_nu[ft]->IsNaR())
					continue;
				if (m_curr_nal_coding == NAL_CODING_AVC)
				{
					if (IS_AVC_VCL_NAL(nal_unit_type) && ft == NU_FILTER_VCL) bHit = true;
					else if (nal_unit_type == AVC_CS_IDR_PIC && ft == NU_FILTER_IDR) bHit = true;
					else if (nal_unit_type == AVC_SEI_NUT && ft == NU_FILTER_SEI) bHit = true;
					else if (nal_unit_type == AVC_AUD_NUT && ft == NU_FILTER_AUD) bHit = true;
					else if (nal_unit_type == AVC_SPS_NUT && ft == NU_FILTER_SPS) bHit = true;
					else if (nal_unit_type == AVC_PPS_NUT && ft == NU_FILTER_PPS) bHit = true;
					else if (nal_unit_type == AVC_FD_NUT  && ft == NU_FILTER_FIL) bHit = true;
				}
				else if (m_curr_nal_coding == NAL_CODING_HEVC)
				{
					if (IS_HEVC_VCL_NAL(nal_unit_type) && ft == NU_FILTER_VCL) bHit = true;
					else if (IS_IDR(nal_unit_type) && ft == NU_FILTER_IDR) bHit = true;
					else if ((nal_unit_type == HEVC_PREFIX_SEI_NUT || nal_unit_type == HEVC_SUFFIX_SEI_NUT) && ft == NU_FILTER_IDR) bHit = true;
					else if (nal_unit_type == HEVC_AUD_NUT && ft == NU_FILTER_AUD) bHit = true;
					else if (nal_unit_type == HEVC_VPS_NUT && ft == NU_FILTER_VPS) bHit = true;
					else if (nal_unit_type == HEVC_SPS_NUT && ft == NU_FILTER_SPS) bHit = true;
					else if (nal_unit_type == HEVC_PPS_NUT && ft == NU_FILTER_PPS) bHit = true;
					else if (nal_unit_type == HEVC_FD_NUT  && ft == NU_FILTER_FIL) bHit = true;
				}
				else if (m_curr_nal_coding == NAL_CODING_VVC)
				{
					if (IS_VVC_VCL_NAL(nal_unit_type) && ft == NU_FILTER_VCL) bHit = true;
					else if (IS_VVC_IDR(nal_unit_type) && ft == NU_FILTER_IDR) bHit = true;
					else if ((nal_unit_type == VVC_PREFIX_SEI_NUT || nal_unit_type == VVC_SUFFIX_SEI_NUT) && ft == NU_FILTER_IDR) bHit = true;
					else if (nal_unit_type == VVC_AUD_NUT && ft == NU_FILTER_AUD) bHit = true;
					else if (nal_unit_type == VVC_VPS_NUT && ft == NU_FILTER_VPS) bHit = true;
					else if (nal_unit_type == VVC_SPS_NUT && ft == NU_FILTER_SPS) bHit = true;
					else if (nal_unit_type == VVC_PPS_NUT && ft == NU_FILTER_PPS) bHit = true;
					else if (nal_unit_type == VVC_FD_NUT  && ft == NU_FILTER_FIL) bHit = true;
				}

				if (!bHit)
					continue;

				if (!subset_nu[ft]->IsNull() && !subset_nu[ft]->Contain(m_filtered_nu_count[ft]))
				{
					if (subset_nu[ft]->Behind(m_filtered_nu_count[NU_FILTER_VCL]))
						return RET_CODE_ABORT;
					return RET_CODE_NOTHING_TODO;
				}
				else if (subset_nu[ft]->IsNull() && !bExcludeAllFiltersExcept[ft])	// Other filter(s) is(are) available
					return RET_CODE_NOTHING_TODO;
				else
					return RET_CODE_SUCCESS;
			}

#if  0
			if ((m_curr_nal_coding == NAL_CODING_AVC && IS_AVC_VCL_NAL(nal_unit_type)) ||
				(m_curr_nal_coding == NAL_CODING_HEVC && IS_HEVC_VCL_NAL(nal_unit_type)))
			{
				if (!m_pMSENav->NAL.vcl_nu.IsNull() && !m_pMSENav->NAL.vcl_nu.Contain(m_filtered_nu_count[NU_FILTER_VCL]))
				{
					if (m_pMSENav->NAL.vcl_nu.Behind(m_filtered_nu_count[NU_FILTER_VCL]))
						return RET_CODE_ABORT;
					return RET_CODE_NOTHING_TODO;
				}
				else if(m_pMSENav->NAL.vcl_nu.IsNull() && (!bExcludeAllSEINU))
					return RET_CODE_NOTHING_TODO;
				else
					return RET_CODE_SUCCESS;
			}
			
			if ((m_curr_nal_coding == NAL_CODING_AVC && AVC_SEI_NUT == nal_unit_type) ||
				(m_curr_nal_coding == NAL_CODING_HEVC && (HEVC_PREFIX_SEI_NUT == nal_unit_type || HEVC_SUFFIX_SEI_NUT == nal_unit_type)))
			{
				if (!m_pMSENav->NAL.sei_nu.IsNull() && !m_pMSENav->NAL.sei_nu.Contain(m_filtered_nu_count[NU_FILTER_SEI]))
				{
					if (m_pMSENav->NAL.sei_nu.Behind(m_filtered_nu_count[NU_FILTER_SEI]))
						return RET_CODE_ABORT;
					return RET_CODE_NOTHING_TODO;
				}
				else if (m_pMSENav->NAL.sei_nu.IsNull() && (!bExcludeAllVCLNU))
					return RET_CODE_NOTHING_TODO;
				else
					return RET_CODE_SUCCESS;
			}
#endif

			// If VCL or SEI filter is active, other NAL unit will be skipped
			if (!bExcludeAllFilters)
				return RET_CODE_NOTHING_TODO;
		}
		else if (level_id > NAL_LEVEL_NU && (OnlyFilterNU(NU_FILTER_VCL) || OnlyFilterNU(NU_FILTER_AUD) 
			|| OnlyFilterNU(NU_FILTER_VPS) || OnlyFilterNU(NU_FILTER_SPS) || OnlyFilterNU(NU_FILTER_VCL) 
			|| OnlyFilterNU(NU_FILTER_IDR) || OnlyFilterNU(NU_FILTER_FIL) || m_pMSENav->NAL.sei_nu.IsAllExcluded()))
			return RET_CODE_NOTHING_TODO;

		return RET_CODE_SUCCESS;
	}

	const char* GetNUName(uint8_t nal_unit_type)
	{
		if (m_curr_nal_coding == NAL_CODING_AVC)
			return nal_unit_type >= 0 && nal_unit_type < _countof(avc_nal_unit_type_short_names) ? avc_nal_unit_type_short_names[nal_unit_type] : "";
		else if (m_curr_nal_coding == NAL_CODING_HEVC)
			return nal_unit_type >= 0 && nal_unit_type < _countof(hevc_nal_unit_type_short_names) ? hevc_nal_unit_type_short_names[nal_unit_type] : "";
		else if (m_curr_nal_coding == NAL_CODING_VVC)
			return nal_unit_type >= 0 && nal_unit_type < _countof(vvc_nal_unit_type_short_names) ? vvc_nal_unit_type_short_names[nal_unit_type] : "";
		return "";
	}

	const char* GetSEIPayoadTypeName(uint32_t payload_type)
	{
		return payload_type < _countof(sei_payload_type_names) ? sei_payload_type_names[payload_type] : "";
	}

	inline int AppendURI(char* szItem, size_t& ccWritten, int level_id)
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

	inline int CheckNALUnitEBSP(uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf, uint8_t& nalUnitHeaderBytes, uint8_t& nal_unit_type)
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

protected:
	INALContext*			m_pNALContext;
	MSENav*					m_pMSENav;
	int						m_nLastLevel = -1;
	uint32_t				m_options = 0;
	const size_t			column_width_name = 47;
	const size_t			column_width_len = 13;
	const size_t			column_width_URI = 36;
	const size_t			right_padding = 1;
};

//
// AV1 video enumerator for MSE operation
//
class CAV1SEEnumerator : public CComUnknown, public IAV1Enumerator
{
public:
	CAV1SEEnumerator(IAV1Context* pCtx, uint32_t options, MSENav* pMSENav)
		: m_pAV1Context(pCtx), m_pMSENav(pMSENav) {
		if (m_pAV1Context != nullptr)
		{
			m_bAnnexB = m_pAV1Context->GetByteStreamFormat() == AV1_BYTESTREAM_LENGTH_DELIMITED ? true : false;
			m_pAV1Context->AddRef();
		}

		int next_level = 0;
		for (size_t i = 0; i < _countof(m_level); i++) {
			if (options&(1ULL << i)) {
				m_level[i] = next_level;

				// CVS is the point, not a range
				if (i == AV1_LEVEL_VSEQ || i == AV1_LEVEL_CVS)
					m_unit_index[next_level] = -1;
				else
					m_unit_index[next_level] = 0;

				m_nLastLevel = (int)i;
				next_level++;
			}
		}

		m_options = options;
	}

	virtual ~CAV1SEEnumerator() {
		AMP_SAFERELEASE(m_pAV1Context);
	}

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_IAV1Enumerator)
			return GetCOMInterface((IAV1Enumerator*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	RET_CODE EnumNewVSEQ(IUnknown* pCtx)
	{
		m_unit_index[m_level[AV1_LEVEL_VSEQ]]++;
		if (m_level[AV1_LEVEL_CVS] > 0)
			m_unit_index[m_level[AV1_LEVEL_CVS]] = -1;
		if (m_level[AV1_LEVEL_TU] > 0)
			m_unit_index[m_level[AV1_LEVEL_TU]] = 0;
		if (m_level[AV1_LEVEL_FU] > 0)
			m_unit_index[m_level[AV1_LEVEL_FU]] = 0;
		m_curr_frameobu_count = 0;
		if (m_level[AV1_LEVEL_OBU] > 0)
			m_unit_index[m_level[AV1_LEVEL_OBU]] = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(AV1_LEVEL_VSEQ)) != RET_CODE_SUCCESS)
			return iRet;

		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sVideo Sequence#%" PRId64 "",
			(m_level[AV1_LEVEL_VSEQ]) * 4, g_szRule, m_unit_index[m_level[AV1_LEVEL_VSEQ]]);

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%12s |", " ")) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, AV1_LEVEL_VSEQ);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		if (!g_silent_output)
			printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t reserved)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		m_unit_index[m_level[AV1_LEVEL_CVS]]++;
		if (m_level[AV1_LEVEL_TU] > 0)
			m_unit_index[m_level[AV1_LEVEL_TU]] = 0;
		if (m_level[AV1_LEVEL_FU] > 0)
			m_unit_index[m_level[AV1_LEVEL_FU]] = 0;
		m_curr_frameobu_count = 0;
		if (m_level[AV1_LEVEL_OBU] > 0)
			m_unit_index[m_level[AV1_LEVEL_OBU]] = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(AV1_LEVEL_CVS)) != RET_CODE_SUCCESS)
			return iRet;

		const char* szGOPType = "";

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sCVS#%" PRId64 " %s",
			(m_level[AV1_LEVEL_CVS]) * 4, g_szRule, m_unit_index[m_level[AV1_LEVEL_CVS]], szGOPType);

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%12s |", " ")) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, AV1_LEVEL_CVS);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		if (!g_silent_output)
			printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	/*
		For the Annex-B byte-stream:
		The first OBU in the first frame_unit of each temporal_unit must be a temporal delimiter OBU 
		(and this is the only place temporal delimiter OBUs can appear).
	*/
	RET_CODE EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(AV1_LEVEL_TU)) != RET_CODE_SUCCESS)
			return iRet;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sTU#%" PRId64 " (%s)",
			(m_level[AV1_LEVEL_TU]) * 4, g_szRule, m_unit_index[m_level[AV1_LEVEL_TU]],
			AV1_FRAME_TYPE_NAMEA(frame_type));

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(TU_size).c_str())) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, AV1_LEVEL_TU);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		if (!g_silent_output)
			printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(AV1_LEVEL_FU)) != RET_CODE_SUCCESS)
			return iRet;

		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*sFU#%" PRId64 " (%s)",
			(m_level[AV1_LEVEL_FU]) * 4, g_szRule, m_unit_index[m_level[AV1_LEVEL_FU]],
			AV1_FRAME_TYPE_NAMEA(frame_type));

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbFrameUnitBuf).c_str())) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		AppendURI(szItem, ccWritten, AV1_LEVEL_FU);

		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		if (!g_silent_output)
			printf("%s\n", szItem);

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(AV1_LEVEL_OBU, obu_type)) == RET_CODE_SUCCESS)
		{
			int enum_cmd = m_options & MSE_ENUM_CMD_MASK;
			if (enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				bool bOnlyFrameOBU = OnlyFrameOBU();
				ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s",
					(m_level[AV1_LEVEL_OBU]) * 4, g_szRule, bOnlyFrameOBU?"Frame":"OBU",
					bOnlyFrameOBU?m_curr_frameobu_count:m_unit_index[m_level[AV1_LEVEL_OBU]], GetOBUName(obu_type));

				if (ccWrittenOnce <= 0)
					return RET_CODE_NOTHING_TODO;

				if ((size_t)ccWrittenOnce < column_width_name)
					memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

				szItem[column_width_name] = '|';
				ccWritten = column_width_name + 1;

				if ((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbOBUBuf).c_str())) <= 0)
					return RET_CODE_NOTHING_TODO;

				ccWritten += ccWrittenOnce;

				AppendURI(szItem, ccWritten, AV1_LEVEL_OBU);

				szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

				if (!g_silent_output)
					printf("%s\n", szItem);
			}

			if (enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
			{
				if (m_nLastLevel == AV1_LEVEL_OBU)
				{
					int indent = 4 * m_level[AV1_LEVEL_OBU];
					int right_part_len = int(column_width_name + 1 + column_width_len + 1 + column_width_URI + 1);

					if (!g_silent_output)
						printf("%.*s%.*s\n", indent, g_szRule, right_part_len - indent, g_szHorizon);

					if (m_options&MSE_ENUM_SYNTAX_VIEW)
						PrintOBUSyntaxElement(pCtx, pOBUBuf, cbOBUBuf, 4 * m_level[AV1_LEVEL_OBU]);
					else if (m_options&MSE_ENUM_HEX_VIEW)
					{
						if (!g_silent_output)
							print_mem(pOBUBuf, (int)cbOBUBuf, 4 * m_level[AV1_LEVEL_OBU]);
					}

					if (!g_silent_output)
						printf("\n");
				}
			}
		}

		m_unit_index[m_level[AV1_LEVEL_OBU]]++;
		// This syntax element is a slice
		if (IS_OBU_FRAME(obu_type))
			m_curr_frameobu_count++;

		return iRet;
	}

	RET_CODE EnumFrameUnitEnd(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf)
	{
		m_unit_index[m_level[AV1_LEVEL_FU]]++;
		if (m_level[AV1_LEVEL_OBU] > 0)
			m_unit_index[m_level[AV1_LEVEL_OBU]] = 0;

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumTemporalUnitEnd(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size)
	{
		m_unit_index[m_level[AV1_LEVEL_TU]]++;
		if (m_level[AV1_LEVEL_FU] > 0)
			m_unit_index[m_level[AV1_LEVEL_FU]] = 0;
		if (m_level[AV1_LEVEL_OBU] > 0)
			m_unit_index[m_level[AV1_LEVEL_OBU]] = 0;
		m_curr_frameobu_count = 0;

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) { return RET_CODE_SUCCESS; }

protected:
	int						m_level[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int64_t					m_unit_index[16] = { 0 };
	int64_t					m_curr_frameobu_count = 0;
	bool					m_bAnnexB = false;

	inline bool OnlyFrameOBU()
	{
		if (m_pMSENav == nullptr)
			return false;

		return (m_pMSENav->AV1.obu.IsAllUnspecfied()) &&
			(m_pMSENav->AV1.obu_frame.IsAllUnspecfied() || (!m_pMSENav->AV1.obu_frame.IsNull() && !m_pMSENav->AV1.obu_frame.IsNaR())) &&
			!(m_pMSENav->AV1.obu_frame.IsAllExcluded());
	}

	std::string	GetURI(int level_id)
	{
		std::string strURI;
		strURI.reserve(128);
		for (int i = level_id; i >= 0; i--)
		{
			if (m_level[i] >= 0)
			{
				if (strURI.length() > 0)
					strURI += ".";
				if (i == AV1_LEVEL_OBU && OnlyFrameOBU())
				{
					strURI += "FRAME" + std::to_string(m_curr_frameobu_count);
				}
				else
				{
					strURI += AV1_LEVEL_NAME(i);
					strURI += std::to_string(m_unit_index[m_level[i]]);
				}
			}
		}

		return strURI;
	}

	RET_CODE CheckFilter(int level_id, uint8_t obu_type = 0xFF)
	{
		if (m_pMSENav == nullptr)
			return RET_CODE_SUCCESS;

		MSEID_RANGE filter[] = { 
			MSEID_RANGE(), MSEID_RANGE(), m_pMSENav->AV1.vseq, m_pMSENav->AV1.cvs, m_pMSENav->AV1.tu, m_pMSENav->AV1.fu, m_pMSENav->AV1.obu,
			MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE() 
		};

		bool bNullParent = true;
		for (int i = 0; i <= level_id; i++)
		{
			if (m_level[i] < 0 || filter[i].IsAllUnspecfied())
				continue;

			if (filter[i].Ahead(m_unit_index[m_level[i]]))
				return RET_CODE_NOTHING_TODO;
			else if (filter[i].Behind(m_unit_index[m_level[i]]))
			{
				if (bNullParent)
					return RET_CODE_ABORT;
				else
					return RET_CODE_NOTHING_TODO;
			}
			/*
				In this case, although filter is not ahead of or behind the id, but it also does NOT include the id
				____________                      ______________________________
							\                    /
				|///////////f0xxxxxxxxxidxxxxxxxxxf1\\\\\\\\\\\\\\\\\\\\\\\\....
			*/
			else if (!filter[i].Contain(m_unit_index[m_level[i]]))
				return RET_CODE_NOTHING_TODO;

			if (!filter[i].IsNull() && !filter[i].IsNaR())
				bNullParent = false;
		}

		// Do some special processing since SLICE is a subset of SE
		if (level_id == AV1_LEVEL_OBU && m_pMSENav->AV1.obu.IsAllUnspecfied())
		{
			if (IS_OBU_FRAME(obu_type))
			{
				if (!m_pMSENav->AV1.obu_frame.IsNull() && !m_pMSENav->AV1.obu_frame.Contain(m_curr_frameobu_count))
				{
					if (m_pMSENav->AV1.obu_frame.Behind(m_curr_frameobu_count))
						return RET_CODE_ABORT;
					return RET_CODE_NOTHING_TODO;
				}
			}
			else
			{
				if (!m_pMSENav->AV1.obu_frame.IsAllExcluded() && !m_pMSENav->AV1.obu_frame.IsNull() && !m_pMSENav->AV1.obu_frame.IsNaR())
					return RET_CODE_NOTHING_TODO;
			}
		}
		else if (level_id > AV1_LEVEL_OBU && OnlyFrameOBU())
			return RET_CODE_NOTHING_TODO;

		return RET_CODE_SUCCESS;
	}

	const char* GetOBUName(uint8_t obu_type)
	{
		return (obu_type >= 0 && obu_type < _countof(obu_type_short_names)) ? obu_type_short_names[obu_type] : "";
	}

	inline int AppendURI(char* szItem, size_t& ccWritten, int level_id)
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

protected:
	IAV1Context*			m_pAV1Context;
	MSENav*					m_pMSENav;
	int						m_nLastLevel = -1;
	uint32_t				m_options = 0;
	const size_t			column_width_name = 47;
	const size_t			column_width_len = 13;
	const size_t			column_width_URI = 31;
	const size_t			right_padding = 1;
};

int CreateMSEParser(IMSEParser** ppMSEParser)
{
	int iRet = RET_CODE_SUCCESS;

	if (ppMSEParser == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	auto iterSrcFmt = g_params.find("srcfmt");
	if (iterSrcFmt == g_params.end())
	{
		printf("Unknown source format, can't create the parser for source file.\n");
		return RET_CODE_ERROR;
	}

	if (g_source_media_scheme_type == MEDIA_SCHEME_UNKNOWN)
	{
		printf("Unsupported media scheme!\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (g_source_media_scheme_type == MEDIA_SCHEME_NAL)
	{
		NAL_CODING nal_coding = NAL_CODING_UNKNOWN;
		if (_stricmp(iterSrcFmt->second.c_str(), "h264") == 0)
			nal_coding = NAL_CODING_AVC;
		else if (_stricmp(iterSrcFmt->second.c_str(), "h265") == 0)
			nal_coding = NAL_CODING_HEVC;
		else if (_stricmp(iterSrcFmt->second.c_str(), "h266") == 0)
			nal_coding = NAL_CODING_VVC;
		else
			printf("Oops! Don't support this kind of NAL bitstream at present!\n");

		if (nal_coding != NAL_CODING_UNKNOWN)
		{
			CNALParser* pNALParser = new CNALParser(nal_coding);
			if (pNALParser != nullptr)
				iRet = pNALParser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
			else
				iRet = RET_CODE_OUTOFMEMORY;
		}
	}
	else if (g_source_media_scheme_type == MEDIA_SCHEME_AV1)
	{
		bool bAnnexBStream = false, bIVF = false;
		if (AMP_FAILED(AV1_PreparseStream(g_params["input"].c_str(), bAnnexBStream)))
		{
			printf("Failed to detect it is a low-overhead bit-stream or length-delimited AV1 bit-stream.\n");
			return RET_CODE_ERROR_NOTIMPL;
		}

		auto iterContainer = g_params.find("container");
		if (iterContainer != g_params.end())
			bIVF = _stricmp(iterContainer->second.c_str(), "ivf") == 0 ? true : false;

		printf("%s AV1 bitstream...\n", bAnnexBStream ? "Length-Delimited" : "Low-Overhead");

		CAV1Parser* pAV1Parser = new CAV1Parser(bAnnexBStream, false, bIVF, nullptr);
		if (pAV1Parser != nullptr)
			iRet = pAV1Parser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
		else
			iRet = RET_CODE_OUTOFMEMORY;
	}
	else if (g_source_media_scheme_type == MEDIA_SCHEME_MPV)
	{
		CMPEG2VideoParser* pMPVParser = new CMPEG2VideoParser();
		if (pMPVParser != nullptr)
			iRet = pMPVParser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
		else
			iRet = RET_CODE_OUTOFMEMORY;
	}
	else if (g_source_media_scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{
		BST::AACAudio::CLOASParser* pLOASParser = new BST::AACAudio::CLOASParser();
		if (pLOASParser != nullptr)
			iRet = pLOASParser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
		else
			iRet = RET_CODE_OUTOFMEMORY;
	}
	else if (g_source_media_scheme_type == MEDIA_SCHEME_ISOBMFF)
	{
		printf("Oops! Not implement yet.\n");
		iRet = RET_CODE_ERROR_NOTIMPL;
	}
	else
	{
		printf("Oops! Don't support this kind of media at present!\n");
		iRet = RET_CODE_ERROR_NOTIMPL;
	}

	return iRet;
}

int BindMSEEnumerator(IMSEParser* pMSEParser, IUnknown* pCtx, uint32_t enum_options, MSENav& mse_nav, FILE* fp)
{
	if (pMSEParser == nullptr || pCtx == nullptr)
		return RET_CODE_ERROR_NOTIMPL;

	int iRet = RET_CODE_ERROR_NOTIMPL;
	MEDIA_SCHEME_TYPE scheme_type = pMSEParser->GetSchemeType();

	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		INALContext* pNALCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();
			CNALSEEnumerator* pNALSEEnumerator = new CNALSEEnumerator(pNALCtx, enum_options | options, &mse_nav);
			if (SUCCEEDED(iRet = pNALSEEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pNALCtx);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		IAV1Context* pAV1Ctx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IAV1Context, (void**)&pAV1Ctx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();
			CAV1SEEnumerator* pAV1SEEnumerator = new CAV1SEEnumerator(pAV1Ctx, enum_options | options, &mse_nav);
			if (SUCCEEDED(iRet = pAV1SEEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pAV1Ctx);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		IMPVContext* pMPVCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();
			CMPVSEEnumerator* pMPVSEEnumerator = new CMPVSEEnumerator(pMPVCtx, enum_options | options, &mse_nav);
			if (SUCCEEDED(iRet = pMPVSEEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pMPVCtx);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{

	}
	else if (scheme_type == MEDIA_SCHEME_ISOBMFF)
	{

	}

	return iRet;
}

#define MAX_MSE_LIST_VIEW_HEADER_WIDTH		100
#define MAX_MSE_SYNTAX_VIEW_HEADER_WIDTH	100
void PrintMSEHeader(IMSEParser* pMSEParser, IUnknown* pCtx, uint32_t enum_options, MSENav& mse_nav, FILE* fp)
{
	int enum_cmd = enum_options & MSE_ENUM_CMD_MASK;
	if (enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
	{
		MEDIA_SCHEME_TYPE scheme_type = pMSEParser->GetSchemeType();
		if (scheme_type == MEDIA_SCHEME_MPV)
			printf("------------Name-------------------------------|-----len-----|------------URI-------------\n");
		else if (scheme_type == MEDIA_SCHEME_NAL)
			printf("------------Name-------------------------------|-----len-----|------------URI----------------------\n");
		else if (scheme_type == MEDIA_SCHEME_AV1)
			printf("------------Name-------------------------------|-----len-----|------------URI-----------------\n");
	}
}

void LocateMSEParseStartPosition(IUnknown* pCtx, FILE* rfp)
{
	bool bIVF = false;
	uint8_t ivf_hdr[IVF_HDR_SIZE] = { 0 };

	// Update the header information to context?
	// TODO..


	auto iterContainer = g_params.find("container");
	if (iterContainer != g_params.end())
		bIVF = _stricmp(iterContainer->second.c_str(), "ivf") == 0 ? true : false;

	if (bIVF)
	{
		if (fread(ivf_hdr, 1, IVF_HDR_SIZE, rfp) != IVF_HDR_SIZE ||
			ivf_hdr[0] != 'D' || ivf_hdr[1] != 'K' || ivf_hdr[2] != 'I' || ivf_hdr[3] != 'F'
			|| ((ivf_hdr[5] << 8) | ivf_hdr[4]) != 0	// Version should be 0
			|| ((ivf_hdr[7] << 8) | ivf_hdr[6]) < 0x20	// header length should be NOT less than 32
			)
		{
			printf("[AV1] It is really a .ivf file?!\n");
			return;
		}

		uint16_t ivf_hdr_len = (ivf_hdr[7] << 8) | ivf_hdr[6];
		uint32_t codec_fourcc = ((uint32_t)ivf_hdr[8] << 24) | ((uint32_t)ivf_hdr[9] << 16) | ((uint32_t)ivf_hdr[10] << 8) | ivf_hdr[11];

		if (codec_fourcc != 'AV01')
		{
			// TODO...
		}

		if (_fseeki64(rfp, ivf_hdr_len, SEEK_SET) != 0)
		{
			printf("[AV1] Failed to jump to the frame part.\n");
			return;
		}
	}
}

int	MSEParse(uint32_t enum_options)
{
	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;
	uint8_t pBuf[2048] = { 0 };
	IMSEParser* pMediaParser = nullptr;
	IUnknown* pCtx = nullptr;

	const int read_unit_size = 2048;

	auto iter_srcInput = g_params.find("input");
	if (iter_srcInput == g_params.end())
	{
		printf("Please specify an input file to show its syntax element.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (AMP_FAILED(iRet = CreateMSEParser(&pMediaParser)))
	{
		printf("Failed to create the media syntax element parser {error: %d}\n", iRet);
		return RET_CODE_ERROR_NOTIMPL;
	}

	MSENav mse_nav(pMediaParser->GetSchemeType());
	if (AMP_FAILED(iRet = mse_nav.Load(enum_options)))
	{
		printf("Failed to load the Media Syntax Element URI.\n");
		return iRet;
	}

	errno_t errn = fopen_s(&rfp, iter_srcInput->second.c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", iter_srcInput->second.c_str(), errn);
		goto done;
	}

	if (AMP_FAILED(iRet = pMediaParser->GetContext(&pCtx)))
	{
		printf("Failed to get the media element syntax element context {error: %d}\n", iRet);
		goto done;
	}

	if (AMP_FAILED(iRet = BindMSEEnumerator(pMediaParser, pCtx, enum_options, mse_nav, rfp)))
	{
		printf("Failed to bind the media syntax element enumerator {error: %d}\n", iRet);
		goto done;
	}

	file_size = GetFileSizeByFP(rfp);

	PrintMSEHeader(pMediaParser, pCtx, enum_options, mse_nav, rfp);

	LocateMSEParseStartPosition(pCtx, rfp);

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = pMediaParser->ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = pMediaParser->ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	if (feof(rfp))
		iRet = pMediaParser->ProcessOutput(true);

done:
	if (rfp != nullptr)
		fclose(rfp);

	if (pMediaParser)
	{
		pMediaParser->Release();
		pMediaParser = nullptr;
	}

	if (pCtx)
	{
		pCtx->Release();
		pCtx = nullptr;
	}

	return iRet;
}

int ShowMSEHex()
{
	return MSEParse(MSE_ENUM_HEX_VIEW);
}

int	ShowMSE()
{
	return MSEParse(MSE_ENUM_SYNTAX_VIEW);
}

int	ListMSE()
{
	return MSEParse(MSE_ENUM_LIST_VIEW);
}
