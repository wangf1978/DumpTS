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
#include "mpeg2_video.h"
#include "h264_video.h"
#include "h265_video.h"
#include "h266_video.h"
#include "NAL.h"
#include "av1.h"

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

	int enum_cmd = enum_options & MSE_ENUM_CMD_MASK;
	if (!(enum_cmd == MSE_ENUM_LIST_VIEW || enum_cmd == MSE_ENUM_SYNTAX_VIEW || enum_cmd == MSE_ENUM_HEX_VIEW || enum_cmd == MSE_ENUM_AU_RANGE))
		return RET_CODE_INVALID_PARAMETER;

	const char* szCmdOption = (enum_cmd == MSE_ENUM_LIST_VIEW)   ? "listMSE" : (
							  (enum_cmd == MSE_ENUM_SYNTAX_VIEW) ? "showMSE":(
							  (enum_cmd == MSE_ENUM_HEX_VIEW)    ? "showMSEHex" : (
							  (enum_cmd == MSE_ENUM_AU_RANGE)    ? "AU": nullptr)));

	if (szCmdOption == nullptr)
		return RET_CODE_ERROR_NOTIMPL;

	auto iterMSEOption = g_params.find(szCmdOption);
	if (iterMSEOption == g_params.end() || iterMSEOption->second.length() == 0)
	{
		if (enum_cmd == MSE_ENUM_LIST_VIEW)
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
		else if (enum_cmd == MSE_ENUM_AU_RANGE)
		{
			InitAs(MSE_UNSPECIFIED);
			if (scheme_type == MEDIA_SCHEME_MPV)
			{
				MPV.slice.Reset(MSE_UNSELECTED);
				MPV.mb.Reset(MSE_UNSELECTED);
				MPV.se.Reset(MSE_UNSELECTED);
			}
			else if (scheme_type == MEDIA_SCHEME_NAL)
			{
				NAL.nu.Reset(MSE_UNSELECTED);
				NAL.vcl_nu.Reset(MSE_UNSELECTED);
				NAL.sei_nu.Reset(MSE_UNSELECTED);
				NAL.aud_nu.Reset(MSE_UNSELECTED);
				NAL.vps_nu.Reset(MSE_UNSELECTED);
				NAL.sps_nu.Reset(MSE_UNSELECTED);
				NAL.pps_nu.Reset(MSE_UNSELECTED);
				NAL.IDR_nu.Reset(MSE_UNSELECTED);
				NAL.FIL_nu.Reset(MSE_UNSELECTED);
				NAL.sei_msg.Reset(MSE_UNSELECTED);
				NAL.sei_pl.Reset(MSE_UNSELECTED);
			}
			else if (scheme_type == MEDIA_SCHEME_AV1)
			{
				AV1.obu.Reset(MSE_UNSELECTED);
				AV1.obu_frame.Reset(MSE_UNSELECTED);
				AV1.obu.Reset(MSE_UNSELECTED);
			}
			return RET_CODE_SUCCESS;
		}

		return RET_CODE_ERROR;
	}

	std::string strMSEURI = iterMSEOption->second;

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

//
// MPEG2 Video navigation enumerator
//
CMPVNavEnumerator::CMPVNavEnumerator(IMPVContext* pCtx, uint32_t options, MSENav* pMSENav)
	: m_pMPVContext(pCtx), m_pMSENav(pMSENav) 
{
	if (m_pMPVContext != nullptr)
		m_pMPVContext->AddRef();

	int next_level = 0;
	for (size_t i = 0; i < _countof(m_level); i++) {
		if (options&(1ULL << i)) {
			m_level[i] = next_level;

			// GOP and VSEQ are the point, not a range
			if (i == MPV_LEVEL_GOP || i == MPV_LEVEL_VSEQ)
				m_unit_index[next_level] = -1;
			else
				m_unit_index[next_level] = 0;

			m_nLastLevel = (int)i;
			next_level++;
		}
	}

	m_options = options;
}

CMPVNavEnumerator::~CMPVNavEnumerator()
{
	AMP_SAFERELEASE(m_pMPVContext);
}

HRESULT CMPVNavEnumerator::NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
{
	if (ppvObj == NULL)
		return E_POINTER;

	if (uuid == IID_IMPVEnumerator)
		return GetCOMInterface((IMPVEnumerator*)this, ppvObj);

	return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
}

RET_CODE CMPVNavEnumerator::EnumVSEQStart(IUnknown* pCtx)
{
	m_unit_index[m_level[MPV_LEVEL_VSEQ]]++;

	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(MPV_LEVEL_VSEQ)) != RET_CODE_SUCCESS)
		return iRet;

	return OnProcessVSEQ(pCtx);
}

RET_CODE CMPVNavEnumerator::EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
{
	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(MPV_LEVEL_AU)) != RET_CODE_SUCCESS)
		return iRet;

	return OnProcessAU(pCtx, pAUBuf, cbAUBuf, picCodingType);
}

RET_CODE CMPVNavEnumerator::EnumNewGOP(IUnknown* pCtx, bool closed_gop, bool broken_link)
{
	m_unit_index[m_level[MPV_LEVEL_GOP]]++;
	if (m_level[MPV_LEVEL_AU] > 0)
		m_unit_index[m_level[MPV_LEVEL_AU]] = 0;
	if (m_level[MPV_LEVEL_SE] > 0)
		m_unit_index[m_level[MPV_LEVEL_SE]] = 0;
	m_curr_slice_count = 0;
	if (m_level[MPV_LEVEL_MB] > 0)
		m_unit_index[m_level[MPV_LEVEL_MB]] = 0;

	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(MPV_LEVEL_GOP)) != RET_CODE_SUCCESS)
		return iRet;

	OnProcessGOP(pCtx, closed_gop, broken_link);

	return RET_CODE_SUCCESS;
}

RET_CODE CMPVNavEnumerator::EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
{
	if (cbBufWithStartCode < 4 || cbBufWithStartCode > INT32_MAX)
		return RET_CODE_NOTHING_TODO;

	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(MPV_LEVEL_SE, pBufWithStartCode[3])) == RET_CODE_SUCCESS)
	{
		OnProcessObject(pCtx, pBufWithStartCode, cbBufWithStartCode);
	}

	m_unit_index[m_level[MPV_LEVEL_SE]]++;
	if (m_level[MPV_LEVEL_MB] > 0)
		m_unit_index[m_level[MPV_LEVEL_MB]] = 0;

	// This syntax element is a slice
	if (pBufWithStartCode[3] >= 0x01 && pBufWithStartCode[3] <= 0xAF)
		m_curr_slice_count++;

	return iRet;
}

RET_CODE CMPVNavEnumerator::EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
{
	m_unit_index[m_level[MPV_LEVEL_AU]]++;
	if (m_level[MPV_LEVEL_SE] > 0)
		m_unit_index[m_level[MPV_LEVEL_SE]] = 0;
	m_curr_slice_count = 0;
	if (m_level[MPV_LEVEL_MB] > 0)
		m_unit_index[m_level[MPV_LEVEL_MB]] = 0;
	return RET_CODE_SUCCESS;
}

RET_CODE CMPVNavEnumerator::EnumVSEQEnd(IUnknown* pCtx)
{
	if (m_level[MPV_LEVEL_GOP] > 0)
		m_unit_index[m_level[MPV_LEVEL_GOP]] = 0;
	if (m_level[MPV_LEVEL_AU] > 0)
		m_unit_index[m_level[MPV_LEVEL_AU]] = 0;
	if (m_level[MPV_LEVEL_SE] > 0)
		m_unit_index[m_level[MPV_LEVEL_SE]] = 0;
	m_curr_slice_count = 0;
	if (m_level[MPV_LEVEL_MB] > 0)
		m_unit_index[m_level[MPV_LEVEL_MB]] = 0;

	return RET_CODE_SUCCESS;
}

RET_CODE CMPVNavEnumerator::EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
{
	return RET_CODE_SUCCESS;
}

std::string CMPVNavEnumerator::GetURI(int level_id)
{
	std::string strURI;
	strURI.reserve(128);
	for (int i = level_id; i >= 0; i--)
	{
		if (m_level[i] >= 0)
		{
			if (strURI.length() > 0)
				strURI += ".";
			if (i == MPV_LEVEL_SE && OnlySliceSE())
			{
				strURI += "SLICE" + std::to_string(m_curr_slice_count);
			}
			else
			{
				strURI += MPV_LEVEL_NAME(i);
				strURI += std::to_string(m_unit_index[m_level[i]]);
			}
		}
	}

	return strURI;
}

RET_CODE CMPVNavEnumerator::CheckFilter(int level_id, uint8_t start_code)
{
	if (m_pMSENav == nullptr)
		return RET_CODE_SUCCESS;

	MSEID_RANGE filter[] = { MSEID_RANGE(), MSEID_RANGE(), m_pMSENav->MPV.vseq, m_pMSENav->MPV.gop, m_pMSENav->MPV.au, m_pMSENav->MPV.se, m_pMSENav->MPV.mb,
		MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE(), MSEID_RANGE() };

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
	if (level_id == MPV_LEVEL_SE && m_pMSENav->MPV.se.IsAllUnspecfied())
	{
		if (start_code >= 0x1 && start_code <= 0xAF)
		{
			if (!m_pMSENav->MPV.slice.IsNull() && !m_pMSENav->MPV.slice.Contain(m_curr_slice_count))
			{
				if (m_pMSENav->MPV.slice.Behind(m_curr_slice_count))
					return RET_CODE_ABORT;
				return RET_CODE_NOTHING_TODO;
			}
		}
		else
		{
			if (!m_pMSENav->MPV.slice.IsAllExcluded() && !m_pMSENav->MPV.slice.IsNull() && !m_pMSENav->MPV.slice.IsNaR())
				return RET_CODE_NOTHING_TODO;
		}
	}

	return RET_CODE_SUCCESS;
}

const char* CMPVNavEnumerator::GetSEName(uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
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

//
// Implementation for CNALNavEnumerator
//
CNALNavEnumerator::CNALNavEnumerator(INALContext* pCtx, uint32_t options, MSENav* pMSENav)
	: m_pNALContext(pCtx), m_pMSENav(pMSENav) 
{
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

CNALNavEnumerator::~CNALNavEnumerator() 
{
	AMP_SAFERELEASE(m_pNALContext);
}

HRESULT CNALNavEnumerator::NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
{
	if (ppvObj == NULL)
		return E_POINTER;

	if (uuid == IID_INALEnumerator)
		return GetCOMInterface((INALEnumerator*)this, ppvObj);

	return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
}

RET_CODE CNALNavEnumerator::EnumNewVSEQ(IUnknown* pCtx)
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

	return OnProcessVSEQ(pCtx);
}

RET_CODE CNALNavEnumerator::EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type)
{
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

	return OnProcessCVS(pCtx, represent_nal_unit_type);
}

RET_CODE CNALNavEnumerator::EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type)
{
	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(NAL_LEVEL_AU)) != RET_CODE_SUCCESS)
		return iRet;

	return OnProcessAU(pCtx, pEBSPAUBuf, cbEBSPAUBuf, picture_slice_type);
}

RET_CODE CNALNavEnumerator::EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
{
	uint8_t cbNALUnitHdr = 0, nal_unit_type = 0;
	int iRet = CheckNALUnitEBSP(pEBSPNUBuf, cbEBSPNUBuf, cbNALUnitHdr, nal_unit_type);
	if (AMP_FAILED(iRet))
		return iRet;

	if ((iRet = CheckFilter(NAL_LEVEL_NU, nal_unit_type)) == RET_CODE_SUCCESS)
	{
		m_curr_nu_type = nal_unit_type;

		iRet = OnProcessNU(pCtx, pEBSPNUBuf, cbEBSPNUBuf);
	}

	return iRet;
}

RET_CODE CNALNavEnumerator::EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
{
	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(NAL_LEVEL_SEI_MSG)) == RET_CODE_SUCCESS)
	{
		iRet = OnProcessSEIMessage(pCtx, pRBSPSEIMsgRBSPBuf, cbRBSPSEIMsgBuf);
	}

	return iRet;
}

RET_CODE CNALNavEnumerator::EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
{
	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(NAL_LEVEL_SEI_PAYLOAD)) == RET_CODE_SUCCESS)
	{
		iRet = OnProcessSEIPayload(pCtx, payload_type, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
	}

	return iRet;
}

RET_CODE CNALNavEnumerator::EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
{
	m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]]++;
	return RET_CODE_SUCCESS;
}

RET_CODE CNALNavEnumerator::EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
{
	m_unit_index[m_level[NAL_LEVEL_SEI_MSG]]++;
	if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
		m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;
	return RET_CODE_SUCCESS;
}

RET_CODE CNALNavEnumerator::EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
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

RET_CODE CNALNavEnumerator::EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf)
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

RET_CODE CNALNavEnumerator::EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
{
	return RET_CODE_SUCCESS;
}

std::string	CNALNavEnumerator::GetURI(int level_id)
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

RET_CODE CNALNavEnumerator::CheckFilter(int level_id, uint8_t nal_unit_type)
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

const char* CNALNavEnumerator::GetNUName(uint8_t nal_unit_type)
{
	if (m_curr_nal_coding == NAL_CODING_AVC)
		return nal_unit_type >= 0 && nal_unit_type < _countof(avc_nal_unit_type_short_names) ? avc_nal_unit_type_short_names[nal_unit_type] : "";
	else if (m_curr_nal_coding == NAL_CODING_HEVC)
		return nal_unit_type >= 0 && nal_unit_type < _countof(hevc_nal_unit_type_short_names) ? hevc_nal_unit_type_short_names[nal_unit_type] : "";
	else if (m_curr_nal_coding == NAL_CODING_VVC)
		return nal_unit_type >= 0 && nal_unit_type < _countof(vvc_nal_unit_type_short_names) ? vvc_nal_unit_type_short_names[nal_unit_type] : "";
	return "";
}

const char* CNALNavEnumerator::GetSEIPayoadTypeName(uint32_t payload_type)
{
	return payload_type < _countof(sei_payload_type_names) ? sei_payload_type_names[payload_type] : "";
}

//
// AV1 video navigation enumerator
//
CAV1NavEnumerator::CAV1NavEnumerator(IAV1Context* pCtx, uint32_t options, MSENav* pMSENav)
	: m_pAV1Context(pCtx), m_pMSENav(pMSENav) 
{
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

CAV1NavEnumerator::~CAV1NavEnumerator() {
	AMP_SAFERELEASE(m_pAV1Context);
}

HRESULT CAV1NavEnumerator::NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
{
	if (ppvObj == NULL)
		return E_POINTER;

	if (uuid == IID_IAV1Enumerator)
		return GetCOMInterface((IAV1Enumerator*)this, ppvObj);

	return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
}

RET_CODE CAV1NavEnumerator::EnumNewVSEQ(IUnknown* pCtx)
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

	return OnProcessVSEQ(pCtx);
}

RET_CODE CAV1NavEnumerator::EnumNewCVS(IUnknown* pCtx, int8_t reserved)
{
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

	return OnProcessCVS(pCtx, reserved);
}

/*
	For the Annex-B byte-stream:
	The first OBU in the first frame_unit of each temporal_unit must be a temporal delimiter OBU
	(and this is the only place temporal delimiter OBUs can appear).
*/
RET_CODE CAV1NavEnumerator::EnumTemporalUnitStart(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type)
{
	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(AV1_LEVEL_TU)) != RET_CODE_SUCCESS)
		return iRet;

	return OnProcessTU(pCtx, ptr_TU_buf, TU_size, frame_type);
}

RET_CODE CAV1NavEnumerator::EnumFrameUnitStart(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf, int frame_type)
{
	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(AV1_LEVEL_FU)) != RET_CODE_SUCCESS)
		return iRet;

	return OnProcessFU(pCtx, pFrameUnitBuf, cbFrameUnitBuf, frame_type);
}

RET_CODE CAV1NavEnumerator::EnumOBU(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, uint8_t obu_type, uint32_t obu_size)
{
	char szItem[256] = { 0 };
	size_t ccWritten = 0;
	int ccWrittenOnce = 0;

	int iRet = RET_CODE_SUCCESS;
	if ((iRet = CheckFilter(AV1_LEVEL_OBU, obu_type)) == RET_CODE_SUCCESS)
	{
		iRet = OnProcessOBU(pCtx, pOBUBuf, cbOBUBuf, obu_type, obu_size);
	}

	m_unit_index[m_level[AV1_LEVEL_OBU]]++;
	// This syntax element is a slice
	if (IS_OBU_FRAME(obu_type))
		m_curr_frameobu_count++;

	return iRet;
}

RET_CODE CAV1NavEnumerator::EnumFrameUnitEnd(IUnknown* pCtx, uint8_t* pFrameUnitBuf, uint32_t cbFrameUnitBuf)
{
	m_unit_index[m_level[AV1_LEVEL_FU]]++;
	if (m_level[AV1_LEVEL_OBU] > 0)
		m_unit_index[m_level[AV1_LEVEL_OBU]] = 0;

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1NavEnumerator::EnumTemporalUnitEnd(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size)
{
	m_unit_index[m_level[AV1_LEVEL_TU]]++;
	if (m_level[AV1_LEVEL_FU] > 0)
		m_unit_index[m_level[AV1_LEVEL_FU]] = 0;
	if (m_level[AV1_LEVEL_OBU] > 0)
		m_unit_index[m_level[AV1_LEVEL_OBU]] = 0;
	m_curr_frameobu_count = 0;

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1NavEnumerator::EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
{
	return RET_CODE_SUCCESS;
}

std::string	CAV1NavEnumerator::GetURI(int level_id)
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

const char* CAV1NavEnumerator::GetOBUName(uint8_t obu_type)
{
	return (obu_type >= 0 && obu_type < _countof(obu_type_short_names)) ? obu_type_short_names[obu_type] : "";
}

RET_CODE CAV1NavEnumerator::CheckFilter(int level_id, uint8_t obu_type)
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

