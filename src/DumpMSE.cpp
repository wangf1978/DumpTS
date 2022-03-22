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
#include "AMRFC3986.h"
#include "DataUtil.h"
#include "systemdef.h"
#include "mpeg2_video.h"
#include "MSE.h"
#include "h264_video.h"
#include "h265_video.h"
#include "h266_video.h"
#include "nal_parser.h"
#include "av1.h"
#include "mpeg2_video.h"
#include "ISO14496_3.h"

#define MSE_UNSPECIFIED				INT64_MAX
#define MSE_UNSELECTED				-1

using MSEID = int64_t;

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern MEDIA_SCHEME_TYPE g_media_scheme_type;
extern int AV1_PreparseStream(const char* szAV1FileName, bool& bIsAnnexB);

const char* g_szRule = "                                                                                                                        ";

struct MSENav
{
	MEDIA_SCHEME_TYPE
					scheme_type;
	union
	{
		struct
		{
			MSEID		sei_pl;						// SEI payload index
			MSEID		sei_msg;					// SEI message index
			MSEID		nu;							// NAL unit index
			MSEID		au;							// Access unit index
			MSEID		cvs;						// Codec video sequence index
		}NAL;
		struct
		{
			MSEID		au;
		}AUDIO;
		struct
		{
			MSEID		obu;
			MSEID		fu;
			MSEID		tu;
		}AV1;
		struct
		{
			MSEID		mb;
			MSEID		slice;
			MSEID		se;
			MSEID		au;
			MSEID		gop;
			MSEID		vseq;
		}MPV;
		uint8_t		bytes[128];
	};

	std::vector<std::tuple<uint32_t/*box type*/, MSEID/*the box index with the specified box-type of same parent box*/>>
					isobmff_boxes;

	std::vector<std::string>
					insidepath;

	MSENav(MEDIA_SCHEME_TYPE schemeType)
		: scheme_type(schemeType) {
		memset(bytes, 0xFF, sizeof(bytes));
	}

	int				Load(uint32_t enum_options);
	uint32_t		GetEnumOptions();

protected:
	void			InitAsUnspecified();
	int				LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments, 
										std::vector<std::string>& supported_authority_components, MSEID** muids);

	int				LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments);
	int				LoadMP4AuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments);

	int				CheckNALAuthorityComponent(size_t idxComponent);
	int				CheckAudioLAuthorityComponent(size_t idxComponent);
	int				CheckAV1AuthorityComponent(size_t idxComponent);
	int				CheckMPVAuthorityComponent(size_t idxComponent);

	static std::vector<std::string>
					nal_supported_authority_components;
	static std::vector<std::string>
					audio_supported_authority_components;
	static std::vector<std::string>
					av1_supported_authority_components;
	static std::vector<std::string>
					mpv_supported_authority_components;
};

std::vector<std::string> MSENav::nal_supported_authority_components 
					= { "cvs", "au", "nu", "seimsg", "seipl" };
std::vector<std::string> MSENav::audio_supported_authority_components 
					= { "au" };
std::vector<std::string> MSENav::av1_supported_authority_components 
					= { "tu", "fu", "obu" };
std::vector<std::string> MSENav::mpv_supported_authority_components 
					= { "vseq", "gop", "au", "slice", "mb", "se" };

void MSENav::InitAsUnspecified()
{
	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		NAL.sei_pl = NAL.sei_msg = NAL.nu = NAL.au = NAL.cvs = MSE_UNSPECIFIED;
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		AV1.obu = AV1.fu = AV1.tu = MSE_UNSPECIFIED;
	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		MPV.mb = MPV.slice = MPV.se = MPV.au = MPV.gop = MPV.vseq = MSE_UNSPECIFIED;
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{
		AUDIO.au = MSE_UNSPECIFIED;
	}
}

uint32_t MSENav::GetEnumOptions()
{
	uint32_t enum_options = 0;
	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		if (NAL.sei_pl != MSE_UNSELECTED)
			enum_options |= NAL_ENUM_OPTION_SEI_PAYLOAD;

		if (NAL.sei_msg != MSE_UNSELECTED)
			enum_options |= NAL_ENUM_OPTION_SEI_MSG;

		if (NAL.nu != MSE_UNSELECTED)
			enum_options |= NAL_ENUM_OPTION_NU;

		if (NAL.au != MSE_UNSELECTED)
			enum_options |= NAL_ENUM_OPTION_AU;
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		if (AV1.obu != MSE_UNSELECTED)
			enum_options |= AV1_ENUM_OPTION_OBU;

		if (AV1.fu != MSE_UNSELECTED)
			enum_options |= AV1_ENUM_OPTION_FU;

		if (AV1.tu != MSE_UNSELECTED)
			enum_options |= AV1_ENUM_OPTION_TU;
	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		if (MPV.gop != MSE_UNSELECTED)
			enum_options |= MPV_ENUM_OPTION_GOP;

		if (MPV.au != MSE_UNSELECTED)
			enum_options |= MPV_ENUM_OPTION_AU;

		if (MPV.slice != MSE_UNSELECTED)
			enum_options |= MPV_ENUM_OPTION_SE;

		if (MPV.mb != MSE_UNSELECTED)
			enum_options |= MPV_ENUM_OPTION_MB;

		if (MPV.se != MSE_UNSELECTED)
			enum_options |= MPV_ENUM_OPTION_SE;
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{
		if (AUDIO.au != MSE_UNSELECTED)
			enum_options |= GENERAL_ENUM_AU;
	}
	else if (scheme_type == MEDIA_SCHEME_ISOBMFF)
	{

	}

	return enum_options;
}

int MSENav::Load(uint32_t enum_options)
{
	int iRet = RET_CODE_SUCCESS;

	if (!(enum_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW)))
		return RET_CODE_INVALID_PARAMETER;

	auto iterShowMSE = g_params.find((enum_options&MSE_ENUM_LIST_VIEW)?"listMSE":"showMSE");
	if (iterShowMSE == g_params.end())
	{
		if (enum_options&MSE_ENUM_LIST_VIEW)
		{
			InitAsUnspecified();
			return RET_CODE_SUCCESS;
		}

		return RET_CODE_ERROR;
	}

	std::string strMSEURI = iterShowMSE->second;

	const char* szMSEURI = strMSEURI.c_str();
	if (szMSEURI == nullptr)
		return RET_CODE_SUCCESS;

	URI_Components MSE_uri_components;
	if (AMP_FAILED(AMURI_Split(szMSEURI, MSE_uri_components)))
	{
		printf("Please specify a valid URI for the option 'showMSE'\n");
		return RET_CODE_ERROR;
	}

	if (MSE_uri_components.Ranges[URI_PART_SCHEME].length <= 0 && MSE_uri_components.Ranges[URI_PART_AUTHORITY].length <= 0)
	{
		// Try to put MSE::// at the beginning of URI, otherwise
		// --listMSE=AU
		// 'AU' will be deemed as a relative path, but we want it as MSE://AU
		strMSEURI = "MSE://" + strMSEURI;
		szMSEURI = strMSEURI.c_str();

		MSE_uri_components.reset();

		if (AMP_FAILED(AMURI_Split(szMSEURI, MSE_uri_components)))
		{
			printf("Please specify a valid URI for the option 'showMSE'\n");
			return RET_CODE_ERROR;
		}
	}

	// Check the schema part, it should be 'MSE'
	std::string support_schemas[] = { "MSE"};
	if (MSE_uri_components.Ranges[URI_COMPONENT_SCHEME].length > 0)
	{
		size_t i = 0;
		for (; i < _countof(support_schemas); i++)
		{
			if (support_schemas[i].length() == (size_t)MSE_uri_components.Ranges[URI_COMPONENT_SCHEME].length &&
				MBCSNICMP(szMSEURI + MSE_uri_components.Ranges[URI_COMPONENT_SCHEME].start, support_schemas[i].c_str(), MSE_uri_components.Ranges[URI_COMPONENT_SCHEME].length) == 0)
				break;
		}
		if (i >= _countof(support_schemas))
		{
			printf("Please specify a valid schema, or don't specify schema.\n");
			return RET_CODE_ERROR;
		}
	}

	// Authority part is normally used to locate the minimum general media syntax element
	if (MSE_uri_components.Ranges[URI_COMPONENT_AUTHORITY].length > 0)
	{
		URI_Authority_Components MSE_uri_authority_components;
		if (AMP_FAILED(AMURI_SplitAuthority(szMSEURI, MSE_uri_components.Ranges[URI_COMPONENT_AUTHORITY], MSE_uri_authority_components)))
		{
			printf("The URI seems to contain invalid authority part, please confirm it!\n");
			return RET_CODE_ERROR;
		}

		// Only check host part, and ignore other part
		if (MSE_uri_authority_components.Ranges[URI_AUTHORITY_HOST].length > 0)
		{
			std::vector<URI_Segment> uri_segments;
			if (AMP_FAILED(AMURI_SplitComponent(szMSEURI + MSE_uri_authority_components.Ranges[URI_AUTHORITY_HOST].start, 
				MSE_uri_authority_components.Ranges[URI_AUTHORITY_HOST].length, uri_segments, ".")))
			{
				printf("The URI authority part seems to be invalid, please confirm it!\n");
				return RET_CODE_ERROR;
			}

			if (AMP_FAILED(LoadAuthorityPart(szMSEURI + MSE_uri_authority_components.Ranges[URI_AUTHORITY_HOST].start, uri_segments)))
			{
				printf("Sorry, unable to locate the MSE!\n");
				return RET_CODE_ERROR;
			}
		}
	}

	// Parse the path part
	if (MSE_uri_components.Ranges[URI_COMPONENT_PATH].length > 0)
	{
		std::vector<URI_Segment> uri_segments;
		if (AMP_FAILED(AMURI_SplitComponent(szMSEURI, MSE_uri_components.Ranges[URI_COMPONENT_PATH], uri_segments)))
		{
			printf("Seems to hit the invalid path part in MSE URI.\n");
			return RET_CODE_ERROR;
		}

		std::string seg_path;
		for (auto& seg : uri_segments)
		{
			if (AMP_FAILED(AMURI_DecodeSegment(szMSEURI + seg.start, seg.length, seg_path)))
			{
				printf("Failed to decode one segment in the path part of URI.\n");
				return RET_CODE_ERROR;
			}

			insidepath.emplace_back(seg_path);
		}

		// Ignore other part at present

		iRet = RET_CODE_SUCCESS;
	}

	return iRet;
}

int	MSENav::LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments,
	std::vector<std::string>& supported_authority_components,
	MSEID** muids)
{
	std::string strSeg;
	int64_t i64Val = -1LL;
	int iRet = RET_CODE_SUCCESS;

	for (auto& seg : uri_authority_segments)
	{
		if (AMP_FAILED(AMURI_DecodeSegment(szURI + seg.start, seg.length, strSeg)))
		{
			printf("Failed to decode one segment in a part of URI authority part.\n");
			return RET_CODE_ERROR;
		}

		size_t idx = 0;
		for (; idx < supported_authority_components.size(); idx++)
		{
			if (MBCSNICMP(strSeg.c_str(), supported_authority_components[idx].c_str(), supported_authority_components[idx].length()) == 0)
			{
				if (strSeg.length() == supported_authority_components[idx].length())
					*muids[idx] = MSE_UNSPECIFIED;
				else if (ConvertToInt((char*)strSeg.c_str() + 3, (char*)strSeg.c_str() + strSeg.length(), i64Val) && i64Val >= 0)
					*muids[idx] = i64Val;
				else
				{
					printf("Invalid %s in MSE URI.\n", supported_authority_components[idx].c_str());
					return RET_CODE_ERROR;
				}

				switch (scheme_type)
				{
				case MEDIA_SCHEME_NAL:
					iRet = CheckNALAuthorityComponent(idx);
					break;
				case MEDIA_SCHEME_ADTS:
				case MEDIA_SCHEME_LOAS_LATM:
					iRet = CheckAudioLAuthorityComponent(idx);
					break;
				case MEDIA_SCHEME_AV1:
					iRet = CheckAV1AuthorityComponent(idx);
					break;
				case MEDIA_SCHEME_MPV:
					iRet = CheckMPVAuthorityComponent(idx);
					break;
				}

				if (AMP_FAILED(iRet))
					return iRet;

				break;
			}
		}

		if (idx >= supported_authority_components.size())
		{
			printf("Found the unrecognized component(s) in the authority part of URI.\n");
			return RET_CODE_ERROR_NOTIMPL;
		}
	}

	return iRet;
}

int MSENav::CheckNALAuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence
	if ((idxComponent == 1 && (NAL.cvs != MSE_UNSELECTED)) ||
		(idxComponent == 2 && (NAL.cvs != MSE_UNSELECTED || NAL.au != MSE_UNSELECTED)) ||
		(idxComponent == 3 && (NAL.cvs != MSE_UNSELECTED || NAL.au != MSE_UNSELECTED || NAL.nu != MSE_UNSELECTED)) ||
		(idxComponent == 4 && (NAL.cvs != MSE_UNSELECTED || NAL.au != MSE_UNSELECTED || NAL.nu != MSE_UNSELECTED || NAL.sei_msg != MSE_UNSELECTED)))
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}

	return RET_CODE_SUCCESS;
}

int MSENav::CheckAudioLAuthorityComponent(size_t idxComponent)
{
	return RET_CODE_SUCCESS;
}

int MSENav::CheckAV1AuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence
	if ((idxComponent == 1 && (AV1.tu != MSE_UNSELECTED)) ||
		(idxComponent == 2 && (AV1.tu != MSE_UNSELECTED || AV1.fu != MSE_UNSELECTED)))
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}

	return RET_CODE_SUCCESS;
}

int MSENav::CheckMPVAuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence
	if ((idxComponent == 1 && (MPV.vseq != MSE_UNSELECTED)) ||	// gop
		(idxComponent == 2 && (MPV.vseq != MSE_UNSELECTED || MPV.gop != MSE_UNSELECTED)) || // au happen, vseq and gop should NOT happen
		(idxComponent == 3 && (MPV.vseq != MSE_UNSELECTED || MPV.gop != MSE_UNSELECTED || MPV.au != MSE_UNSELECTED)) ||
		(idxComponent == 4 && (MPV.vseq != MSE_UNSELECTED || MPV.gop != MSE_UNSELECTED || MPV.au != MSE_UNSELECTED || (MPV.se != MSE_UNSELECTED && MPV.slice != MSE_UNSELECTED))) ||
		(idxComponent == 5 && (MPV.vseq != MSE_UNSELECTED || MPV.gop != MSE_UNSELECTED || MPV.au != MSE_UNSELECTED || (MPV.se != MSE_UNSELECTED && MPV.slice != MSE_UNSELECTED) || MPV.mb != MSE_UNSELECTED)))
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}

	return RET_CODE_SUCCESS;
}

int	MSENav::LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments)
{
	int iRet = RET_CODE_ERROR;
	switch (scheme_type)
	{
	case MEDIA_SCHEME_TRANSPORT_STREAM:
		iRet = RET_CODE_ERROR_NOTIMPL;
		break;
	case MEDIA_SCHEME_PROGRAM_STREAM:
		iRet = RET_CODE_ERROR_NOTIMPL;
		break;
	case MEDIA_SCHEME_ISOBMFF:
		iRet = LoadMP4AuthorityPart(szURI, uri_authority_segments);
		break;
	case MEDIA_SCHEME_MATROSKA:
		iRet = RET_CODE_ERROR_NOTIMPL;
		break;
	case MEDIA_SCHEME_MMT:
		iRet = RET_CODE_ERROR_NOTIMPL;
		break;
	case MEDIA_SCHEME_AIFF:
		iRet = RET_CODE_ERROR_NOTIMPL;
		break;
	case MEDIA_SCHEME_NAL:
	{
		MSEID* muids[] = { &NAL.cvs, &NAL.au, &NAL.nu, &NAL.sei_msg, &NAL.sei_pl };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, nal_supported_authority_components, muids);
		break;
	}
	case MEDIA_SCHEME_ADTS:
	case MEDIA_SCHEME_LOAS_LATM:
	{
		MSEID* muids[] = { &AUDIO.au };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, audio_supported_authority_components, muids);
		break;
	}
	case MEDIA_SCHEME_AV1:
	{
		MSEID* muids[] = { &AV1.tu, &AV1.fu, &AV1.obu };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, av1_supported_authority_components, muids);
		break;
	}
	case MEDIA_SCHEME_MPV:
	{
		MSEID* muids[] = { &MPV.vseq, &MPV.gop, &MPV.au, &MPV.slice, &MPV.mb, &MPV.se };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, mpv_supported_authority_components, muids);
		break;
	}
	default:
		iRet = RET_CODE_ERROR_NOTIMPL;
	}

	if (iRet == RET_CODE_ERROR_NOTIMPL)
		return iRet;

	return iRet;
}

int	MSENav::LoadMP4AuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments)
{
	std::string strSeg;
	int64_t i64Val = -1LL;
	int iRet = RET_CODE_SUCCESS;

	for (auto& seg : uri_authority_segments)
	{
		if (AMP_FAILED(AMURI_DecodeSegment(szURI + seg.start, seg.length, strSeg)))
		{
			printf("Failed to decode one segment in a part of URI authority part.\n");
			return RET_CODE_ERROR;
		}

		if (strSeg.length() < 4)
		{
			printf("The box type should be Four-CC style.\n");
			return RET_CODE_ERROR;
		}

		uint32_t box_type = (((uint32_t)strSeg[0]) << 24) | (((uint32_t)strSeg[1]) << 16) | (((uint32_t)strSeg[2]) << 8) | (uint32_t)strSeg[3];

		char* s = (char*)strSeg.c_str() + 4;
		char* e = (char*)strSeg.c_str() + strSeg.length();

		if (strSeg.length() > 4)
		{
			if (ConvertToInt(s, e, i64Val) == false || i64Val < 0)
			{
				printf("Unable to parse the ISOBMFF Box URI for '%s'.\n", strSeg.c_str());
				return RET_CODE_ERROR;
			}
		}
		else
			i64Val = MSE_UNSPECIFIED;

		isobmff_boxes.push_back({ box_type, i64Val });
	}

	return RET_CODE_SUCCESS;
}

class CMPVSEEnumerator : public CComUnknown, public IMPVEnumerator
{
public:
	CMPVSEEnumerator(IMPVContext* pCtx)
		: m_pMPVContext(pCtx) {
		if (m_pMPVContext != nullptr)
			m_pMPVContext->AddRef();
	}

	virtual ~CMPVSEEnumerator() {
		AMP_SAFERELEASE(m_pMPVContext);
	}

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_IMPVEnumerator)
			return GetCOMInterface((IMPVEnumerator*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	RET_CODE EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%*.sAU#%" PRId64 " (%s)", 
			(m_level[MPV_LEVEL_AU] - 1) * 4, g_szRule, m_unit_count[m_level[MPV_LEVEL_AU] - 1],
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

		std::string strFormattedURI = GetURI(MPV_LEVEL_AU);
		if (strFormattedURI.length() < column_width_URI)
		{
			memset(szItem + ccWritten, ' ', column_width_URI - strFormattedURI.length());
			ccWritten += column_width_URI - strFormattedURI.length();
		}

		memcpy(szItem + ccWritten, strFormattedURI.c_str(), strFormattedURI.length());
		ccWritten += strFormattedURI.length();
		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		return RET_CODE_SUCCESS; 
	}

	RET_CODE EnumSliceStart(IUnknown* pCtx, uint8_t* pSliceBuf, size_t cbSliceBuf) { return RET_CODE_SUCCESS; }
	RET_CODE EnumSliceEnd(IUnknown* pCtx, uint8_t* pSliceBuf, size_t cbSliceBuf) { return RET_CODE_SUCCESS; }
	RET_CODE EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		m_unit_count[m_level[MPV_LEVEL_AU] - 1]++;
		if (m_level[MPV_LEVEL_SE] > 0)
			m_unit_count[m_level[MPV_LEVEL_SE] > 0] = 0;
		return RET_CODE_SUCCESS; 
	}

	RET_CODE EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
	{
		if (cbBufWithStartCode < 4 || cbBufWithStartCode > INT32_MAX)
			return RET_CODE_NOTHING_TODO;

		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%*.sSE#%" PRId64 " %s",
			(m_level[MPV_LEVEL_SE] - 1) * 4, g_szRule, m_unit_count[m_level[MPV_LEVEL_SE] - 1],
			GetSEName(pBufWithStartCode, cbBufWithStartCode));

		if (ccWrittenOnce <= 0)
			return RET_CODE_NOTHING_TODO;

		if ((size_t)ccWrittenOnce < column_width_name)
			memset(szItem + ccWritten + ccWrittenOnce, ' ', column_width_name - ccWrittenOnce);

		szItem[column_width_name] = '|';
		ccWritten = column_width_name + 1;

		if((ccWrittenOnce = MBCSPRINTF_S(szItem + ccWritten, _countof(szItem) - ccWritten, "%10s B |", GetReadableNum(cbBufWithStartCode).c_str())) <= 0)
			return RET_CODE_NOTHING_TODO;

		ccWritten += ccWrittenOnce;

		std::string szFormattedURI = GetURI(MPV_LEVEL_SE);
		if (szFormattedURI.length() < column_width_URI)
		{
			memset(szItem + ccWritten, ' ', column_width_URI - szFormattedURI.length());
			ccWritten += column_width_URI - szFormattedURI.length();
		}

		memcpy(szItem + ccWritten, szFormattedURI.c_str(), szFormattedURI.length());
		ccWritten += szFormattedURI.length();
		szItem[ccWritten < _countof(szItem) ? ccWritten : _countof(szItem) - 1] = '\0';

		printf("%s\n", szItem);

		m_unit_count[m_level[MPV_LEVEL_SE] - 1]++;
		if (m_level[MPV_LEVEL_MB] > 0)
			m_unit_count[m_level[MPV_LEVEL_SE] > 0] = 0;

		return RET_CODE_SUCCESS;
	}
	RET_CODE EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) { return RET_CODE_SUCCESS; }

public:
	int						m_level[16] = { 0 };	// 0 is an invalid level
	int64_t					m_unit_count[16] = { 0 };

	std::string	GetURI(int level_id)
	{
		std::string strURI;
		strURI.reserve(128);
		for (int i = level_id; i >= 0; i--)
		{
			if (m_level[i] > 0)
			{
				if (strURI.length() > 0)
					strURI += ".";
				strURI += MPV_LEVEL_NAME(i);
				strURI += std::to_string(m_unit_count[m_level[i] - 1]);
			}
		}

		return strURI;
	}

	const char* GetSEName(uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
	{
		uint8_t* p = pBufWithStartCode;
		size_t cbLeft = cbBufWithStartCode;

		uint8_t start_code = p[3];
		if (start_code == EXTENSION_START_CODE || start_code == USER_DATA_START_CODE)
		{
			uint8_t extension_start_code_identifier = (p[4] >> 4) & 0xF;
			return mpv_extension_syntax_element_names[extension_start_code_identifier];
		}

		return mpv_syntax_element_names[start_code];
	}

protected:
	IMPVContext*			m_pMPVContext;
	const size_t			column_width_name = 47;
	const size_t			column_width_len  = 13;
	const size_t			column_width_URI  = 27;

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

	if (g_media_scheme_type == MEDIA_SCHEME_UNKNOWN)
	{
		printf("Unsupported media scheme!\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (g_media_scheme_type == MEDIA_SCHEME_NAL)
	{
		NAL_CODING nal_coding = NAL_CODING_UNKNOWN;
		if (_stricmp(iterSrcFmt->second.c_str(), "h264") == 0)
			nal_coding = NAL_CODING_AVC;
		else if (_stricmp(iterSrcFmt->second.c_str(), "h265") == 0)
			nal_coding = NAL_CODING_HEVC;
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
	else if (g_media_scheme_type == MEDIA_SCHEME_AV1)
	{
		bool bAnnexBStream = false;
		if (AMP_FAILED(AV1_PreparseStream(g_params["input"].c_str(), bAnnexBStream)))
		{
			printf("Failed to detect it is a low-overhead bit-stream or length-delimited AV1 bit-stream.\n");
			return RET_CODE_ERROR_NOTIMPL;
		}

		printf("%s AV1 bitstream...\n", bAnnexBStream ? "Length-Delimited" : "Low-Overhead");

		CAV1Parser* pAV1Parser = new CAV1Parser(bAnnexBStream, false, nullptr);
		if (pAV1Parser != nullptr)
			iRet = pAV1Parser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
		else
			iRet = RET_CODE_OUTOFMEMORY;
	}
	else if (g_media_scheme_type == MEDIA_SCHEME_MPV)
	{
		CMPEG2VideoParser* pMPVParser = new CMPEG2VideoParser();
		if (pMPVParser != nullptr)
			iRet = pMPVParser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
		else
			iRet = RET_CODE_OUTOFMEMORY;
	}
	else if (g_media_scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{
		BST::AACAudio::CLOASParser* pLOASParser = new BST::AACAudio::CLOASParser();
		if (pLOASParser != nullptr)
			iRet = pLOASParser->QueryInterface(IID_IMSEParser, (void**)ppMSEParser);
		else
			iRet = RET_CODE_OUTOFMEMORY;
	}
	else if (g_media_scheme_type == MEDIA_SCHEME_ISOBMFF)
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

	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{

	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		IMPVContext* pMPVCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			CMPVSEEnumerator* pMPVSEEnumerator = new CMPVSEEnumerator(pMPVCtx);
			if (SUCCEEDED(pMPVSEEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				uint32_t options = mse_nav.GetEnumOptions();
				iRet = pMSEParser->SetEnumerator(pMPVSEEnumerator, enum_options | options);

				int next_level = 1;
				for (size_t i = 0; i < _countof(pMPVSEEnumerator->m_level); i++)
				{
					if (options&(1ULL << i))
					{
						pMPVSEEnumerator->m_level[i] = next_level;
						next_level++;
					}
				}

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
	if (enum_options&MSE_ENUM_LIST_VIEW)
	{
		MEDIA_SCHEME_TYPE scheme_type = pMSEParser->GetSchemeType();
		if (scheme_type == MEDIA_SCHEME_MPV)
			printf("------------Name-------------------------------|-----len-----|------------URI-------------\n");
	}
	else if (enum_options&MSE_ENUM_SYNTAX_VIEW)
	{

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

int	ShowMSE()
{
	uint32_t enum_options = MSE_ENUM_SYNTAX_VIEW;
	if (g_params.find("hexview") != g_params.end())
		enum_options = MSE_ENUM_HEX_VIEW;

	return MSEParse(enum_options);
}

int	ListMSE()
{
	return MSEParse(MSE_ENUM_LIST_VIEW);
}
