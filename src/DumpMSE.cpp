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
			
		if (enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW)
		{
			if (m_nLastLevel == MPV_LEVEL_SE)
			{
				int indent = 4 * m_level[MPV_LEVEL_SE];
				int right_part_len = int(column_width_name + 1 + column_width_len + 1 + column_width_URI + 1);

				printf("%.*s%.*s\n", indent, g_szRule, right_part_len - indent, g_szHorizon);
				if (enum_cmd == MSE_ENUM_SYNTAX_VIEW)
					PrintMPVSyntaxElement(pCtx, pBufWithStartCode, cbBufWithStartCode, 4 * m_level[MPV_LEVEL_SE]);
				else if (enum_cmd == MSE_ENUM_HEX_VIEW)
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
class CNALSEEnumerator : public CNALNavEnumerator
{
public:
	CNALSEEnumerator(INALContext* pCtx, uint32_t options, MSENav* pMSENav)
		: CNALNavEnumerator(pCtx, options, pMSENav) {}

public:
	RET_CODE OnProcessVSEQ(IUnknown* pCtx)
	{
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

	RET_CODE OnProcessCVS(IUnknown* pCtx, int8_t represent_nal_unit_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

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

	RET_CODE OnProcessAU(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

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

	RET_CODE OnProcessNU(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

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
						(m_level[NAL_LEVEL_NU]) * 4, g_szRule, NU_FILTER_NAME(i), m_filtered_nu_count[i], GetNUName(m_curr_nu_type));
					break;
				}
			}

			if (!bSpecifiedNUFilter)
			{
				ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s::%s",
					(m_level[NAL_LEVEL_NU]) * 4, g_szRule, "NU", m_unit_index[m_level[NAL_LEVEL_NU]],
					m_curr_nal_coding == NAL_CODING_AVC ?  (IS_AVC_VCL_NAL(m_curr_nu_type)  ? "VCL" : "non-VCL") : (
					m_curr_nal_coding == NAL_CODING_HEVC ? (IS_HEVC_VCL_NAL(m_curr_nu_type) ? "VCL" : "non-VCL") : (
					m_curr_nal_coding == NAL_CODING_VVC ?  (IS_VVC_VCL_NAL(m_curr_nu_type)  ? "VCL" : "non-VCL") : "")),
					GetNUName(m_curr_nu_type));
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
				if (enum_cmd == MSE_ENUM_SYNTAX_VIEW)
					PrintNALUnitSyntaxElement(pCtx, pEBSPNUBuf, cbEBSPNUBuf, 4 * m_level[NAL_LEVEL_NU]);
				else if (enum_cmd == MSE_ENUM_HEX_VIEW)
					print_mem(pEBSPNUBuf, (int)cbEBSPNUBuf, 4 * m_level[NAL_LEVEL_NU]);
				printf("\n");
			}
		}

		return RET_CODE_SUCCESS;
	}

	RET_CODE OnProcessSEIMessage(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

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
				if (enum_cmd == MSE_ENUM_SYNTAX_VIEW)
					PrintSEIMsgSyntaxElement(pCtx, pRBSPSEIMsgRBSPBuf, cbRBSPSEIMsgBuf, 4 * m_level[NAL_LEVEL_SEI_MSG]);
				else if (enum_cmd == MSE_ENUM_HEX_VIEW)
					print_mem(pRBSPSEIMsgRBSPBuf, (int)cbRBSPSEIMsgBuf, 4 * m_level[NAL_LEVEL_SEI_MSG]);
				printf("\n");
			}
		}

		return RET_CODE_SUCCESS;
	}

	RET_CODE OnProcessSEIPayload(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

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
				if (enum_cmd == MSE_ENUM_SYNTAX_VIEW)
					PrintSEIPayloadSyntaxElement(pCtx, payload_type, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf, 4 * m_level[NAL_LEVEL_SEI_PAYLOAD]);
				else if (enum_cmd == MSE_ENUM_HEX_VIEW)
					print_mem(pRBSPSEIPayloadBuf, (int)cbRBSPPayloadBuf, 4 * m_level[NAL_LEVEL_SEI_PAYLOAD]);
				printf("\n");
			}
		}

		return RET_CODE_SUCCESS;
	}
};

//
// AV1 video enumerator for MSE operation
//
class CAV1SEEnumerator : public CAV1NavEnumerator
{
public:
	CAV1SEEnumerator(IAV1Context* pCtx, uint32_t options, MSENav* pMSENav)
		: CAV1NavEnumerator(pCtx, options, pMSENav){}

public:
	RET_CODE OnProcessVSEQ(IUnknown* pCtx)
	{
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

	RET_CODE OnProcessCVS(IUnknown* pCtx, int8_t reserved)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

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
	RET_CODE OnProcessTU(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

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

	RET_CODE OnProcessFU(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

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

	RET_CODE OnProcessOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
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

				if (enum_cmd == MSE_ENUM_SYNTAX_VIEW)
					PrintOBUSyntaxElement(pCtx, pOBUBuf, cbOBUBuf, 4 * m_level[AV1_LEVEL_OBU]);
				else if (enum_cmd == MSE_ENUM_HEX_VIEW)
				{
					if (!g_silent_output)
						print_mem(pOBUBuf, (int)cbOBUBuf, 4 * m_level[AV1_LEVEL_OBU]);
				}

				if (!g_silent_output)
					printf("\n");
			}
		}

		return iRet;
	}
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

int BindMSEEnumerator(IMSEParser* pMSEParser, IUnknown* pCtx, uint32_t enum_options, MSENav& mse_nav)
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

	if (AMP_FAILED(iRet = BindMSEEnumerator(pMediaParser, pCtx, enum_options, mse_nav)))
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
