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
#include "MSEnav.h"
#include "DataUtil.h"

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

std::vector<std::string> MSENav::nal_supported_authority_components 
					= { "vseq", "cvs", "au", "nu", "vcl", "seinu", "aud", "vps", "sps", "pps", "IDR", "FIL", "seimsg", "seipl" };
std::vector<std::string> MSENav::audio_supported_authority_components 
					= { "au" };
std::vector<std::string> MSENav::av1_supported_authority_components 
					= { "vseq", "cvs", "tu", "fu", "obu", "frame" };
std::vector<std::string> MSENav::mpv_supported_authority_components 
					= { "vseq", "gop", "au", "se", "slice", "mb"};

void MSENav::InitAs(MSEID mseid)
{
	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		NAL.sei_pl.Reset(mseid);
		NAL.sei_msg.Reset(mseid);
		NAL.nu.Reset(mseid);
		NAL.vcl_nu.Reset(mseid);
		NAL.sei_nu.Reset(mseid);
		NAL.aud_nu.Reset(mseid);
		NAL.vps_nu.Reset(mseid);
		NAL.sps_nu.Reset(mseid);
		NAL.pps_nu.Reset(mseid);
		NAL.IDR_nu.Reset(mseid);
		NAL.FIL_nu.Reset(mseid);
		NAL.au.Reset(mseid);
		NAL.cvs.Reset(mseid);
		NAL.vseq.Reset(mseid);
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		AV1.obu_frame.Reset(mseid);
		AV1.obu.Reset(mseid);
		AV1.fu.Reset(mseid);
		AV1.tu.Reset(mseid);
		AV1.cvs.Reset(mseid);
		AV1.vseq.Reset(mseid);
	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		MPV.mb.Reset(mseid);
		MPV.slice.Reset(mseid);
		MPV.se.Reset(mseid);
		MPV.au.Reset(mseid);
		MPV.gop.Reset(mseid);
		MPV.vseq.Reset(mseid);
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{
		AUDIO.au.Reset(mseid);
	}
}

uint32_t MSENav::GetEnumOptions()
{
	uint32_t enum_options = 0;
	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		if (!NAL.sei_pl.IsNull() && !NAL.sei_pl.IsNaR())
			enum_options |= NAL_ENUM_OPTION_SEI_PAYLOAD;

		if (!NAL.sei_msg.IsNull() && !NAL.sei_msg.IsNaR())
			enum_options |= NAL_ENUM_OPTION_SEI_MSG;

		if (!NAL.nu.IsNull() && !NAL.nu.IsNaR())
			enum_options |= NAL_ENUM_OPTION_NU;

		if (!NAL.au.IsNull() && !NAL.au.IsNaR())
			enum_options |= NAL_ENUM_OPTION_AU;

		if (!NAL.cvs.IsNull() && !NAL.cvs.IsNaR())
			enum_options |= NAL_ENUM_OPTION_CVS;

		if (!NAL.vseq.IsNull() && !NAL.vseq.IsNaR())
			enum_options |= NAL_ENUM_OPTION_VSEQ;
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		if (!AV1.obu.IsNull() && !AV1.obu.IsNaR())
			enum_options |= AV1_ENUM_OPTION_OBU;

		if (!AV1.fu.IsNull() && !AV1.fu.IsNaR())
			enum_options |= AV1_ENUM_OPTION_FU;

		if (!AV1.tu.IsNull() && !AV1.tu.IsNaR())
			enum_options |= AV1_ENUM_OPTION_TU;

		if (!AV1.cvs.IsNull() && !AV1.cvs.IsNaR())
			enum_options |= AV1_ENUM_OPTION_CVS;

		if (!AV1.vseq.IsNull() && !AV1.vseq.IsNaR())
			enum_options |= AV1_ENUM_OPTION_VSEQ;
	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		if (!MPV.vseq.IsNull() && !MPV.vseq.IsNaR())
			enum_options |= MPV_ENUM_OPTION_VSEQ;

		if (!MPV.gop.IsNull() && !MPV.gop.IsNaR())
			enum_options |= MPV_ENUM_OPTION_GOP;

		if (!MPV.au.IsNull() && !MPV.au.IsNaR())
			enum_options |= MPV_ENUM_OPTION_AU;

		if (!MPV.se.IsNull() && !MPV.se.IsNaR())
			enum_options |= MPV_ENUM_OPTION_SE;

		if (!MPV.slice.IsNull() && !MPV.slice.IsNaR())
			enum_options |= MPV_ENUM_OPTION_SE;

		if (!MPV.mb.IsNull() && !MPV.mb.IsNaR())
			enum_options |= MPV_ENUM_OPTION_MB;
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{
		if (!AUDIO.au.IsNull() && !AUDIO.au.IsNaR())
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

	if (!(enum_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW)))
		return RET_CODE_INVALID_PARAMETER;

	const char* szCmdOption = (enum_options&MSE_ENUM_LIST_VIEW) ? "listMSE" : (
							  (enum_options&MSE_ENUM_SYNTAX_VIEW) ? "showMSE":(
							  (enum_options&MSE_ENUM_HEX_VIEW) ? "showMSEHex" : nullptr));

	if (szCmdOption == nullptr)
		return RET_CODE_ERROR_NOTIMPL;

	auto iterShowMSE = g_params.find(szCmdOption);
	if (iterShowMSE == g_params.end() || iterShowMSE->second.length() == 0)
	{
		if (enum_options&MSE_ENUM_LIST_VIEW)
		{
			InitAs(MSE_UNSPECIFIED);
			if (scheme_type == MEDIA_SCHEME_MPV)
				MPV.slice.Reset(MSE_UNSELECTED);
			else if (scheme_type == MEDIA_SCHEME_NAL)
			{
				NAL.vcl_nu.Reset(MSE_UNSELECTED);
				NAL.sei_nu.Reset(MSE_UNSELECTED);
				NAL.aud_nu.Reset(MSE_UNSELECTED);
				NAL.vps_nu.Reset(MSE_UNSELECTED);
				NAL.sps_nu.Reset(MSE_UNSELECTED);
				NAL.pps_nu.Reset(MSE_UNSELECTED);
				NAL.IDR_nu.Reset(MSE_UNSELECTED);
				NAL.FIL_nu.Reset(MSE_UNSELECTED);
			}
			else if (scheme_type == MEDIA_SCHEME_AV1)
				AV1.obu_frame.Reset(MSE_UNSELECTED);
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
		printf("Please specify a valid URI for the option '%s'\n", szCmdOption);
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
			printf("Please specify a valid URI for the option '%s'\n", szCmdOption);
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
				MBCSNICMP(szMSEURI + MSE_uri_components.Ranges[URI_COMPONENT_SCHEME].start, 
					support_schemas[i].c_str(), MSE_uri_components.Ranges[URI_COMPONENT_SCHEME].length) == 0)
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
	std::vector<std::string>& supported_authority_components, MSEID_RANGE* ranges[])
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

		int idx = (int)(supported_authority_components.size() - 1);
		for (; idx >= 0; idx--)
		{
			// Parse the '~'exclusion prefix part of syntax-element filter
			bool bExclude = false;
			const char* szSeg = strSeg.c_str();
			size_t ccSeg = strSeg.length();
			while (ccSeg > 0 && *szSeg == '~')
			{
				bExclude = !bExclude;
				ccSeg--;
				szSeg++;
			}

			const char* sz_authority_comp = supported_authority_components[idx].c_str();
			size_t authority_comp_len = supported_authority_components[idx].length();
			if (ccSeg > 0 && MBCSNICMP(szSeg, sz_authority_comp, authority_comp_len) == 0)
			{
				if (ccSeg == authority_comp_len)
				{
					ranges[idx]->id[0] = bExclude ? MSE_EXCLUDE(0) : 0;
					ranges[idx]->id[1] = bExclude ? MSE_EXCLUDE(MSE_MAX_MSEID) : MSE_MAX_MSEID;
				}
				else if (ccSeg > authority_comp_len)
				{
					int64_t i64StartVal, i64EndVal;
					if (ConvertToInclusiveNNRange(szSeg + authority_comp_len, szSeg + ccSeg, i64StartVal, i64EndVal, MSE_MAX_MSEID))
					{
						if (IS_INCLUSIVE_MSEID(i64StartVal) && IS_INCLUSIVE_MSEID(i64EndVal))
						{
							ranges[idx]->id[0] = bExclude ? MSE_EXCLUDE(i64StartVal) : i64StartVal;
							ranges[idx]->id[1] = bExclude ? MSE_EXCLUDE(i64EndVal) : i64EndVal;
						}
						else
						{
							printf("The MSE index should be between %" PRId64 " to %" PRId64 " inclusively.\n", (int64_t)MSE_MIN_MSEID, MSE_MAX_MSEID);
							return RET_CODE_ERROR;
						}
					}
					else
					{
						printf("Invalid %s in MSE URI.\n", sz_authority_comp);
						return RET_CODE_ERROR;
					}
				}
				else
				{
					printf("Invalid %s in MSE URI.\n", sz_authority_comp);
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

		if (idx < 0)
		{
			printf("Found the unrecognized component(s) in the authority part of URI.\n");
			return RET_CODE_ERROR_NOTIMPL;
		}
	}

	// Adjust some values
	iRet = NormalizeAuthorityPart();

	return iRet;
}

int MSENav::NormalizeAuthorityPart()
{
	// Process some subset scenarios
	// If a syntax element or its subset exist, should mark its superset syntax element as "unspecified"
	if (scheme_type == MEDIA_SCHEME_MPV)
	{
		if (MPV.se.IsNull())
		{
			if(!MPV.slice.IsNull())
				MPV.se.Reset(MSE_UNSPECIFIED);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_NAL)
	{
		if (NAL.nu.IsNull())
		{
			if (!NAL.vcl_nu.IsNull() || !NAL.sei_nu.IsNull() || !NAL.aud_nu.IsNull() || !NAL.vps_nu.IsNull() ||
				!NAL.sps_nu.IsNull() || !NAL.pps_nu.IsNull() || !NAL.IDR_nu.IsNull() || !NAL.FIL_nu.IsNull())
				NAL.nu.Reset(MSE_UNSPECIFIED);
		}
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		if (AV1.obu.IsNull())
		{
			if (!AV1.obu_frame.IsNull())
				AV1.obu.Reset(MSE_UNSPECIFIED);
		}
	}

	return RET_CODE_SUCCESS;
}

bool MSENav::CheckNUFilterCoexist(size_t idxComponent)
{
	size_t subset_filter_count = 0;
	if (!NAL.nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 3)
		subset_filter_count++;
	if (!NAL.vcl_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 4)
		subset_filter_count++;
	if (!NAL.sei_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 5)
		subset_filter_count++;
	if (!NAL.aud_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 6)
		subset_filter_count++;
	if (!NAL.vps_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 7)
		subset_filter_count++;
	if (!NAL.sps_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 8)
		subset_filter_count++;
	if (!NAL.pps_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 9)
		subset_filter_count++;
	if (!NAL.IDR_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 10)
		subset_filter_count++;
	if (!NAL.FIL_nu.IsNull() && !NAL.nu.IsNaR() && idxComponent != 11)
		subset_filter_count++;

	return subset_filter_count > 0 ? true : false;
}

int MSENav::CheckNALAuthorityComponent(size_t idxComponent)
{
	//                              0      1     2     3      4       5       6      7      8      9     10     11      12         13
	// Check the occur sequence, "vseq", "cvs", "au", "nu", "vcl", "seinu", "aud", "vps", "sps", "pps", "IDR", "FIL", "seimsg", "seipl"
	if ((idxComponent > 0  && !NAL.vseq.IsNull()	&& !NAL.vseq.IsNaR()) ||// cvs or deeper
		(idxComponent > 1  && !NAL.cvs.IsNull()		&& !NAL.cvs.IsNaR()) ||	// au or deeper
		(idxComponent > 2  && !NAL.au.IsNull()		&& !NAL.au.IsNaR()) ||	// nu or deeper
		(idxComponent > 11 && !NAL.nu.IsNull()		&& !NAL.nu.IsNaR()) ||	// sei-message or sei_payload
		(idxComponent > 12 && !NAL.sei_msg.IsNull()	&& !NAL.sei_msg.IsNaR()))	// sei-payload
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}
	else if (idxComponent >= 3 && idxComponent <= 11)
	{
		if (CheckNUFilterCoexist(idxComponent))
		{
			printf("'nu', 'vcl', 'seinu', 'aud', 'vps', 'sps', 'pps', 'IDR' or 'FIL' should NOT occur at the same time.\n");
			return RET_CODE_ERROR;
		}
	}

	return RET_CODE_SUCCESS;
}

int MSENav::CheckAudioLAuthorityComponent(size_t idxComponent)
{
	return RET_CODE_SUCCESS;
}

int MSENav::CheckAV1AuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence according to "vseq", "cvs", "tu", "fu", "obu", "frame" 
	if ((idxComponent > 0 && !AV1.vseq.IsNull() && !AV1.vseq.IsNaR()) || // CVS/GOP or deeper
		(idxComponent > 1 && !AV1.cvs.IsNull() && !AV1.cvs.IsNaR()) ||	 // TU or deeper happen, vseq and gop should NOT happen
		(idxComponent > 2 && !AV1.tu.IsNull() && !AV1.tu.IsNaR()) ||
		(idxComponent > 3 && !AV1.fu.IsNull() && !AV1.fu.IsNaR()))
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}
	else if (idxComponent == 4 || idxComponent == 5)
	{
		if ((idxComponent == 4 && !AV1.obu_frame.IsNull() && !AV1.obu_frame.IsNaR()) ||
			(idxComponent == 5 && !AV1.obu.IsNull()       && !AV1.obu.IsNaR()))
		{
			printf("'obu' should NOT occur at the same time with 'frame'.\n");
			return RET_CODE_ERROR;
		}
	}

	return RET_CODE_SUCCESS;
}

int MSENav::CheckMPVAuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence according to "vseq", "gop", "au", "se", "slice", "mb"
	if ((idxComponent > 0 && !MPV.vseq.IsNull() && !MPV.vseq.IsNaR()) || // gop or deeper
		(idxComponent > 1 && !MPV.gop.IsNull()  && !MPV.gop.IsNaR()) || // au or deeper happen, vseq and gop should NOT happen
		(idxComponent > 2 && !MPV.au.IsNull()   && !MPV.au.IsNaR()) || // se or deeper happen
		(idxComponent > 4 && !MPV.se.IsNull()   && !MPV.se.IsNaR() && !MPV.slice.IsNull() && !MPV.slice.IsNaR()))
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}
	else if (idxComponent == 3 || idxComponent == 4)
	{
		if ((idxComponent == 3 && !MPV.slice.IsNull() && !MPV.slice.IsNaR()) ||
			(idxComponent == 4 && !MPV.se.IsNull() && !MPV.se.IsNaR()))
		{
			printf("'se' should NOT occur at the same time with 'slice'.\n");
			return RET_CODE_ERROR;
		}
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
		MSEID_RANGE* ranges[] = {&NAL.vseq, &NAL.cvs, &NAL.au, 
			&NAL.nu, &NAL.vcl_nu, &NAL.sei_nu, &NAL.aud_nu, &NAL.vps_nu, &NAL.sps_nu, &NAL.pps_nu, &NAL.IDR_nu, &NAL.FIL_nu,
			&NAL.sei_msg, &NAL.sei_pl};
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, nal_supported_authority_components, ranges);
		break;
	}
	case MEDIA_SCHEME_ADTS:
	case MEDIA_SCHEME_LOAS_LATM:
	{
		MSEID_RANGE* ranges[] = {&AUDIO.au};
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, audio_supported_authority_components, ranges);
		break;
	}
	case MEDIA_SCHEME_AV1:
	{
		MSEID_RANGE* ranges[] = { &AV1.vseq, &AV1.cvs, &AV1.tu, &AV1.fu, &AV1.obu, &AV1.obu_frame };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, av1_supported_authority_components, ranges);
		break;
	}
	case MEDIA_SCHEME_MPV:
	{
		MSEID_RANGE* ranges[] = {&MPV.vseq, &MPV.gop, &MPV.au, &MPV.se, &MPV.slice, &MPV.mb};
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, mpv_supported_authority_components, ranges);
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

