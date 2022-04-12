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
#include "mediaobjprint.h"

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

using MSEID = int64_t;

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern MEDIA_SCHEME_TYPE g_media_scheme_type;
extern int AV1_PreparseStream(const char* szAV1FileName, bool& bIsAnnexB);

const char* g_szRule    = "                                                                                                                        ";
const char* g_szHorizon = "------------------------------------------------------------------------------------------------------------------------";

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
					= { "vseq", "cvs", "au", "nu", "vcl", "seimsg", "seipl" };
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
				NAL.vcl_nu.Reset(MSE_UNSELECTED);
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

		size_t idx = 0;
		for (; idx < supported_authority_components.size(); idx++)
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

		if (idx >= supported_authority_components.size())
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
			if (!NAL.vcl_nu.IsNull())
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

int MSENav::CheckNALAuthorityComponent(size_t idxComponent)
{
	// Check the occur sequence, "vseq", "cvs", "au", "nu", "vcl", "seimsg", "seipl"
	if ((idxComponent > 0 && !NAL.vseq.IsNull()		&& !NAL.vseq.IsNaR()) ||// cvs or deeper
		(idxComponent > 1 && !NAL.cvs.IsNull()		&& !NAL.cvs.IsNaR()) ||	// au or deeper
		(idxComponent > 2 && !NAL.au.IsNull()		&& !NAL.au.IsNaR()) ||	// nu or deeper
		(idxComponent > 4 && !NAL.nu.IsNull()		&& !NAL.nu.IsNaR()) ||	// sei-message or sei_payload
		(idxComponent > 5 && !NAL.sei_msg.IsNull()	&& !NAL.sei_msg.IsNaR()))	// sei-payload
	{
		printf("Please specify the MSE URI from lower to higher.\n");
		return RET_CODE_ERROR;
	}
	else if (idxComponent == 3 || idxComponent == 4)
	{
		if ((idxComponent == 3 && !NAL.vcl_nu.IsNull() && !NAL.vcl_nu.IsNaR()) ||
			(idxComponent == 4 && !NAL.nu.IsNull() && !NAL.nu.IsNaR()))
		{
			printf("'nu' should NOT occur at the same time with 'vcl'.\n");
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
		MSEID_RANGE* ranges[] = {&NAL.vseq, &NAL.cvs, &NAL.au, &NAL.nu, &NAL.vcl_nu, &NAL.sei_msg, &NAL.sei_pl};
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
// MPEG2 video enumerator for MSE operation
//
class CMPVSEEnumerator : public CComUnknown, public IMPVEnumerator
{
public:
	CMPVSEEnumerator(IMPVContext* pCtx, uint32_t options, MSENav* pMSENav) 
		: m_pMPVContext(pCtx), m_pMSENav(pMSENav) {
		if (m_pMPVContext != nullptr)
			m_pMPVContext->AddRef();

		int next_level = 0;
		for (size_t i = 0; i < _countof(m_level); i++){
			if (options&(1ULL << i)){
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
	RET_CODE EnumVSEQStart(IUnknown* pCtx) 
	{ 
		m_unit_index[m_level[MPV_LEVEL_VSEQ]]++;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(MPV_LEVEL_VSEQ)) != RET_CODE_SUCCESS)
			return iRet;

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

	RET_CODE EnumAUStart(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(MPV_LEVEL_AU)) != RET_CODE_SUCCESS)
			return iRet;

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

	virtual RET_CODE EnumNewGOP(IUnknown* pCtx, bool closed_gop, bool broken_link)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;

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

	RET_CODE EnumObject(IUnknown* pCtx, uint8_t* pBufWithStartCode, size_t cbBufWithStartCode)
	{
		if (cbBufWithStartCode < 4 || cbBufWithStartCode > INT32_MAX)
			return RET_CODE_NOTHING_TODO;

		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(MPV_LEVEL_SE, pBufWithStartCode[3])) == RET_CODE_SUCCESS)
		{
			if (m_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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
		}

		m_unit_index[m_level[MPV_LEVEL_SE]]++;
		if (m_level[MPV_LEVEL_MB] > 0)
			m_unit_index[m_level[MPV_LEVEL_MB]] = 0;

		// This syntax element is a slice
		if (pBufWithStartCode[3] >= 0x01 && pBufWithStartCode[3] <= 0xAF)
			m_curr_slice_count++;

		return iRet;
	}

	RET_CODE EnumAUEnd(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		m_unit_index[m_level[MPV_LEVEL_AU]]++;
		if (m_level[MPV_LEVEL_SE] > 0)
			m_unit_index[m_level[MPV_LEVEL_SE]] = 0;
		m_curr_slice_count = 0;
		if (m_level[MPV_LEVEL_MB] > 0)
			m_unit_index[m_level[MPV_LEVEL_MB]] = 0;
		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumVSEQEnd(IUnknown* pCtx)
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

	RET_CODE EnumError(IUnknown* pCtx, uint64_t stream_offset, int error_code) { return RET_CODE_SUCCESS; }

protected:
	int						m_level[16] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int64_t					m_unit_index[16] = { 0 };
	int64_t					m_curr_slice_count = 0;

	inline bool OnlySliceSE()
	{
		if (m_pMSENav == nullptr)
			return false;

		return (m_pMSENav->MPV.se.IsAllUnspecfied()) && 
			   (m_pMSENav->MPV.slice.IsAllUnspecfied() || (!m_pMSENav->MPV.slice.IsNull() && !m_pMSENav->MPV.slice.IsNaR())) &&
			  !(m_pMSENav->MPV.slice.IsAllExcluded());;
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

	RET_CODE CheckFilter(int level_id, uint8_t start_code=0)
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
	IMPVContext*			m_pMPVContext;
	MSENav*					m_pMSENav;
	int						m_nLastLevel = -1;
	uint32_t				m_options = 0;
	const size_t			column_width_name = 47;
	const size_t			column_width_len  = 13;
	const size_t			column_width_URI  = 27;
	const size_t			right_padding = 1;
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
		m_curr_vcl_count = 0;
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
		m_curr_vcl_count = 0;
		if (m_level[NAL_LEVEL_SEI_MSG] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_MSG]] = 0;
		if (m_level[NAL_LEVEL_SEI_PAYLOAD] > 0)
			m_unit_index[m_level[NAL_LEVEL_SEI_PAYLOAD]] = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_CVS)) != RET_CODE_SUCCESS)
			return iRet;

		const char* szCVSType = "";
		if (m_curr_nal_coding == NAL_CODING_HEVC)
			szCVSType = IS_CRA(represent_nal_unit_type) ? "(CRA, open GOP)" : (IS_BLA(represent_nal_unit_type) ? "(BLA)" : (IS_IDR(represent_nal_unit_type) ? "(IDR, closed GOP)" : ""));
		else if (m_curr_nal_coding == NAL_CODING_AVC)
			szCVSType = represent_nal_unit_type == AVC_CS_IDR_PIC ? "(IDR, closed GOP)" : "(I, open GOP)";

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
			m_curr_nal_coding == NAL_CODING_HEVC?HEVC_PIC_SLICE_TYPE_NAMEA(picture_slice_type):""));

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
			if (m_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
			{
				bool onlyShowVCLNU = OnlyVCLNU();
				if (onlyShowVCLNU)
				{
					ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s",
						(m_level[NAL_LEVEL_NU]) * 4, g_szRule, "VCL-NU", m_curr_vcl_count, GetNUName(nal_unit_type));
				}
				else
				{
					ccWrittenOnce = MBCSPRINTF_S(szItem, _countof(szItem), "%.*s%s#%" PRId64 " %s::%s",
						(m_level[NAL_LEVEL_NU]) * 4, g_szRule, "NU", m_unit_index[m_level[NAL_LEVEL_NU]],
						m_curr_nal_coding == NAL_CODING_AVC ? (IS_AVC_VCL_NAL(nal_unit_type) ? "VCL" : "non-VCL") : (
						m_curr_nal_coding == NAL_CODING_HEVC ? (IS_HEVC_VCL_NAL(nal_unit_type) ? "VCL" : "non-VCL") : ""),
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

			if (m_options&(MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_SEI_MSG)) == RET_CODE_SUCCESS)
		{
			if (m_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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

			if (m_options&(MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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

		return RET_CODE_SUCCESS;
	}

	RET_CODE EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
	{
		char szItem[256] = { 0 };
		size_t ccWritten = 0;
		int ccWrittenOnce = 0;

		int iRet = RET_CODE_SUCCESS;
		if ((iRet = CheckFilter(NAL_LEVEL_SEI_PAYLOAD)) == RET_CODE_SUCCESS)
		{
			if (m_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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

			if (m_options&(MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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

		return RET_CODE_SUCCESS;
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
		if ((m_curr_nal_coding == NAL_CODING_AVC  && IS_AVC_VCL_NAL(m_curr_nu_type)) ||
			(m_curr_nal_coding == NAL_CODING_HEVC && IS_HEVC_VCL_NAL(m_curr_nu_type)))
				m_curr_vcl_count++;

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
		m_curr_vcl_count = 0;
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
	int64_t					m_curr_vcl_count = 0;
	int8_t					m_curr_nu_type = -1;
	NAL_CODING				m_curr_nal_coding = NAL_CODING_UNKNOWN;

	inline bool OnlyVCLNU()
	{
		if (m_pMSENav == nullptr)
			return false;

		return (m_pMSENav->NAL.nu.IsAllUnspecfied()) && 
			   (m_pMSENav->NAL.vcl_nu.IsAllUnspecfied() || (!m_pMSENav->NAL.vcl_nu.IsNull() && !m_pMSENav->NAL.vcl_nu.IsNaR())) &&
			  !(m_pMSENav->NAL.vcl_nu.IsAllExcluded());
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
				if (i == NAL_LEVEL_NU && OnlyVCLNU())
				{
					strURI += "VCL" + std::to_string(m_curr_vcl_count);
				}
				else
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

		if ((m_nLastLevel == NAL_LEVEL_SEI_MSG || m_nLastLevel == NAL_LEVEL_SEI_PAYLOAD) && level_id == NAL_LEVEL_NU)
		{
			if (nal_unit_type != 0xFF && !(
				(m_curr_nal_coding == NAL_CODING_AVC  && (nal_unit_type == BST::H264Video::SEI_NUT)) ||
				(m_curr_nal_coding == NAL_CODING_HEVC && (nal_unit_type >= BST::H265Video::PREFIX_SEI_NUT || nal_unit_type == BST::H265Video::SUFFIX_SEI_NUT))))
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
				bNullParent = false;
		}

		// Do some special processing since SLICE is a subset of SE
		if (level_id == NAL_LEVEL_NU && m_pMSENav->NAL.nu.IsAllUnspecfied())
		{
			if ((m_curr_nal_coding == NAL_CODING_AVC && IS_AVC_VCL_NAL(nal_unit_type)) ||
				(m_curr_nal_coding == NAL_CODING_HEVC && IS_HEVC_VCL_NAL(nal_unit_type)))
			{
				if (!m_pMSENav->NAL.vcl_nu.IsNull() && !m_pMSENav->NAL.vcl_nu.Contain(m_curr_vcl_count))
				{
					if (m_pMSENav->NAL.vcl_nu.Behind(m_curr_vcl_count))
						return RET_CODE_ABORT;
					return RET_CODE_NOTHING_TODO;
				}
			}
			else
			{
				if (!m_pMSENav->NAL.vcl_nu.IsAllExcluded() && !m_pMSENav->NAL.vcl_nu.IsNull() && !m_pMSENav->NAL.vcl_nu.IsNaR())
					return RET_CODE_NOTHING_TODO;
			}
		}
		else if (level_id > NAL_LEVEL_NU && OnlyVCLNU())
			return RET_CODE_NOTHING_TODO;

		return RET_CODE_SUCCESS;
	}

	const char* GetNUName(uint8_t nal_unit_type)
	{
		if (m_curr_nal_coding == NAL_CODING_AVC)
			return nal_unit_type >= 0 && nal_unit_type < _countof(avc_nal_unit_type_short_names) ? avc_nal_unit_type_short_names[nal_unit_type] : "";
		else if (m_curr_nal_coding == NAL_CODING_HEVC)
			return nal_unit_type >= 0 && nal_unit_type < _countof(hevc_nal_unit_type_short_names) ? hevc_nal_unit_type_short_names[nal_unit_type] : "";
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
			if (m_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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

			if (m_options&(MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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
	if (enum_options&(MSE_ENUM_LIST_VIEW | MSE_ENUM_SYNTAX_VIEW | MSE_ENUM_HEX_VIEW))
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
