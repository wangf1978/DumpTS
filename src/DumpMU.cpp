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

#define MU_UNSPECIFIED				INT64_MAX
#define MU_UNSELECTED				-1

using MUID = int64_t;

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

struct MediaUnitNav
{
	MEDIA_SCHEME_TYPE
					scheme_type;
	union
	{
		struct
		{
			MUID		sei_pl;						// SEI payload index
			MUID		sei_msg;					// SEI message index
			MUID		nu;							// NAL unit index
			MUID		au;							// Access unit index
			MUID		cvs;						// Codec video sequence index
		}NAL;
		struct
		{
			MUID		au;
		}AUDIO;
		struct
		{
			MUID		obu;
			MUID		fu;
			MUID		tu;
		}AV1;
		struct
		{
			MUID		se;
			MUID		mb;
			MUID		slice;
			MUID		au;
			MUID		gop;
			MUID		vseq;
		}MPV;
		uint8_t		bytes[128];
	};

	std::vector<std::tuple<uint32_t/*box type*/, MUID/*the box index with the specified box-type of same parent box*/>>
					isobmff_boxes;

	std::vector<std::string>
					insidepath;

	MediaUnitNav(MEDIA_SCHEME_TYPE schemeType)
		: scheme_type(schemeType) {
		memset(bytes, 0xFF, sizeof(bytes));
	}

	int Load();

protected:
	int	LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments, 
		std::vector<std::string>& supported_authority_components, MUID** muids);

	int	LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments);
	int	LoadMP4AuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments);

	int CheckNALAuthorityComponent(size_t idxComponent);
	int CheckAudioLAuthorityComponent(size_t idxComponent);
	int CheckAV1AuthorityComponent(size_t idxComponent);
	int CheckMPVAuthorityComponent(size_t idxComponent);

	static std::vector<std::string>		nal_supported_authority_components;
	static std::vector<std::string>		audio_supported_authority_components;
	static std::vector<std::string>		av1_supported_authority_components;
	static std::vector<std::string>		mpv_supported_authority_components;
};

std::vector<std::string> MediaUnitNav::nal_supported_authority_components = { "cvs", "au", "nu", "seimsg", "seipl" };
std::vector<std::string> MediaUnitNav::audio_supported_authority_components = { "au" };
std::vector<std::string> MediaUnitNav::av1_supported_authority_components = { "tu", "fu", "obu" };
std::vector<std::string> MediaUnitNav::mpv_supported_authority_components = { "vseq", "gop", "au", "slice", "mb", "se" };

int MediaUnitNav::Load()
{
	int iRet = RET_CODE_SUCCESS;

	auto iterShowMU = g_params.find("showMU");
	if (iterShowMU == g_params.end())
		return RET_CODE_ERROR;

	const char* szMUURI = iterShowMU->second.c_str();
	if (szMUURI == nullptr)
		return RET_CODE_SUCCESS;

	URI_Components MU_uri_components;
	if (AMP_FAILED(AMURI_Split(szMUURI, MU_uri_components)))
	{
		printf("Please specify a valid URI for the option 'showMU'\n");
		return RET_CODE_ERROR;
	}

	// Check the schema part, it should be 'syntax' or 'MU'
	std::string support_schemas[] = { "syntax", "MU" };
	if (MU_uri_components.Ranges[URI_COMPONENT_SCHEME].length > 0)
	{
		size_t i = 0;
		for (; i < _countof(support_schemas); i++)
		{
			if (support_schemas[i].length() == (size_t)MU_uri_components.Ranges[URI_COMPONENT_SCHEME].length &&
				MBCSNICMP(szMUURI + MU_uri_components.Ranges[URI_COMPONENT_SCHEME].start, support_schemas[i].c_str(), MU_uri_components.Ranges[URI_COMPONENT_SCHEME].length) == 0)
				break;
		}
		if (i >= _countof(support_schemas))
		{
			printf("Please specify a valid schema, or don't specify schema.\n");
			return RET_CODE_ERROR;
		}
	}

	// Authority part is normally used to locate the minimum general media unit
	if (MU_uri_components.Ranges[URI_COMPONENT_AUTHORITY].length > 0)
	{
		URI_Authority_Components MU_uri_authority_components;
		if (AMP_FAILED(AMURI_SplitAuthority(szMUURI, MU_uri_components.Ranges[URI_COMPONENT_AUTHORITY], MU_uri_authority_components)))
		{
			printf("The URI seems to contain invalid authority part, please confirm it!\n");
			return RET_CODE_ERROR;
		}

		// Only check host part, and ignore other part
		if (MU_uri_authority_components.Ranges[URI_AUTHORITY_HOST].length > 0)
		{
			std::vector<URI_Segment> uri_segments;
			if (AMP_FAILED(AMURI_SplitComponent(szMUURI + MU_uri_authority_components.Ranges[URI_AUTHORITY_HOST].start, 
				MU_uri_authority_components.Ranges[URI_AUTHORITY_HOST].length, uri_segments, ".")))
			{
				printf("The URI authority part seems to be invalid, please confirm it!\n");
				return RET_CODE_ERROR;
			}

			if (AMP_FAILED(LoadAuthorityPart(szMUURI, uri_segments)))
			{
				printf("Sorry, unable to locate the media unit!\n");
				return RET_CODE_ERROR;
			}
		}
	}

	// Parse the path part
	if (MU_uri_components.Ranges[URI_COMPONENT_PATH].length > 0)
	{
		std::vector<URI_Segment> uri_segments;
		if (AMP_FAILED(AMURI_SplitComponent(szMUURI, MU_uri_components.Ranges[URI_COMPONENT_PATH], uri_segments)))
		{
			printf("Seems to hit the invalid path part in MU URI.\n");
			return RET_CODE_ERROR;
		}

		std::string seg_path;
		for (auto& seg : uri_segments)
		{
			if (AMP_FAILED(AMURI_DecodeSegment(szMUURI + seg.start, seg.length, seg_path)))
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

int	MediaUnitNav::LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments,
	std::vector<std::string>& supported_authority_components,
	MUID** muids)
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
					*muids[idx] = MU_UNSPECIFIED;
				else if (ConvertToInt((char*)strSeg.c_str() + 3, (char*)strSeg.c_str() + strSeg.length(), i64Val) && i64Val >= 0)
					*muids[idx] = i64Val;
				else
				{
					printf("Invalid %s in Media Unit URI.\n", supported_authority_components[idx].c_str());
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

int MediaUnitNav::CheckNALAuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence
	if ((idxComponent == 1 && (NAL.cvs != MU_UNSELECTED)) ||
		(idxComponent == 2 && (NAL.cvs != MU_UNSELECTED || NAL.au != MU_UNSELECTED)) ||
		(idxComponent == 3 && (NAL.cvs != MU_UNSELECTED || NAL.au != MU_UNSELECTED || NAL.nu != MU_UNSELECTED)) ||
		(idxComponent == 4 && (NAL.cvs != MU_UNSELECTED || NAL.au != MU_UNSELECTED || NAL.nu != MU_UNSELECTED || NAL.sei_msg != MU_UNSELECTED)))
	{
		printf("Please specify the Media Unit URI from small media unit to bigger media unit.\n");
		return RET_CODE_ERROR;
	}

	return RET_CODE_SUCCESS;
}

int MediaUnitNav::CheckAudioLAuthorityComponent(size_t idxComponent)
{
	return RET_CODE_SUCCESS;
}

int MediaUnitNav::CheckAV1AuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence
	if ((idxComponent == 1 && (AV1.tu != MU_UNSELECTED)) ||
		(idxComponent == 2 && (AV1.tu != MU_UNSELECTED || AV1.fu != MU_UNSELECTED)))
	{
		printf("Please specify the Media Unit URI from small media unit to bigger media unit.\n");
		return RET_CODE_ERROR;
	}

	return RET_CODE_SUCCESS;
}

int MediaUnitNav::CheckMPVAuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence
	if ((idxComponent == 1 && (MPV.vseq != MU_UNSELECTED)) ||
		(idxComponent == 2 && (MPV.vseq != MU_UNSELECTED || MPV.gop != MU_UNSELECTED)) ||
		(idxComponent == 3 && (MPV.vseq != MU_UNSELECTED || MPV.gop != MU_UNSELECTED || MPV.au != MU_UNSELECTED)) ||
		(idxComponent == 4 && (MPV.vseq != MU_UNSELECTED || MPV.gop != MU_UNSELECTED || MPV.au != MU_UNSELECTED || MPV.slice != MU_UNSELECTED)) ||
		(idxComponent == 5 && (MPV.vseq != MU_UNSELECTED || MPV.gop != MU_UNSELECTED || MPV.au != MU_UNSELECTED || MPV.slice != MU_UNSELECTED || MPV.mb != MU_UNSELECTED)))
	{
		printf("Please specify the Media Unit URI from small media unit to bigger media unit.\n");
		return RET_CODE_ERROR;
	}

	return RET_CODE_SUCCESS;
}

int	MediaUnitNav::LoadAuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments)
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
		MUID* muids[] = { &NAL.cvs, &NAL.au, &NAL.nu, &NAL.sei_msg, &NAL.sei_pl };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, nal_supported_authority_components, muids);
		break;
	}
	case MEDIA_SCHEME_ADTS:
	case MEDIA_SCHEME_LOAS_LATM:
	{
		MUID* muids[] = { &AUDIO.au };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, audio_supported_authority_components, muids);
		break;
	}
	case MEDIA_SCHEME_AV1:
	{
		MUID* muids[] = { &AV1.tu, &AV1.fu, &AV1.obu };
		iRet = LoadAuthorityPart(szURI, uri_authority_segments, av1_supported_authority_components, muids);
		break;
	}
	case MEDIA_SCHEME_MPV:
	{
		MUID* muids[] = { &MPV.vseq, &MPV.gop, &MPV.au, &MPV.slice, &MPV.mb, &MPV.se };
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

int	MediaUnitNav::LoadMP4AuthorityPart(const char* szURI, std::vector<URI_Segment>& uri_authority_segments)
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
			i64Val = MU_UNSPECIFIED;

		isobmff_boxes.push_back({ box_type, i64Val });
	}

	return RET_CODE_SUCCESS;
}

int	ShowMU()
{
	return -1;
}

int	ListMU()
{
	return -1;
}
