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

#endif
