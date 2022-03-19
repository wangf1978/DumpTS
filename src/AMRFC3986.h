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
#ifndef _AM_RFC_3986_H_
#define _AM_RFC_3986_H_

#include <vector>
#include <string>

typedef void* AMURI;

#define MAX_URI_LENGTH					8192

#define URI_COMPONENT_SCHEME			0
#define URI_COMPONENT_AUTHORITY			1
#define URI_COMPONENT_PATH				2
#define URI_COMPONENT_QUERY				3
#define URI_COMPONENT_FRAGMENT			4

#define MAX_URI_COMPONENTS				5

#define URI_AUTHORITY_USERINFO			0
#define	URI_AUTHORITY_HOST				1
#define URI_AUTHORITY_PORT				2

#define MAX_URI_AUTHORITY_COMPONENTS	3

enum URI_RFC_VER {
	URI_RFC_2396,
	URI_RFC_3986
};

enum URI_PART_TYPE
{
	URI_PART_UNKNOWN,

	URI_PART_SCHEME,
	URI_PART_HIERPART,
	URI_PART_AUTHORITY,
	URI_PART_PATH_ABEMPTY,
	URI_PART_PATH_ABSOLUTE,
	URI_PART_PATH_NOSCHEME,
	URI_PART_PATH_ROOTLESS,
	URI_PART_PATH_EMPTY,
	URI_PART_QUERY,
	URI_PART_FRAGMENT,

	// Authority part
	URI_PART_USERINFO,
	URI_PART_HOST,
	URI_PART_PORT,

	// PATH segments
	URI_PART_SEGMENT,
	URI_PART_SEGMENT_NZ,
	URI_PART_SEGMENT_NZ_NC,
};

struct inplace_str_range{
	int					start;
	int					length;
	URI_PART_TYPE		type;

	inplace_str_range():start(-1), length(-1), type(URI_PART_UNKNOWN){}
	void reset(){start = length = -1; type = URI_PART_UNKNOWN;}
	BOOL IsNULL(){return (start == -1 && length == -1)?TRUE:FALSE;}
	BOOL IsEmpty(){return (start >= 0 && length == 0)?TRUE:FALSE;}
};

struct URI_Components{
	inplace_str_range	Ranges[MAX_URI_COMPONENTS];	// (R.scheme, R.authority, R.path, R.query, R.fragment)
	void reset(){for(int i=0;i<MAX_URI_COMPONENTS;i++)Ranges[i].reset();}
};

struct URI_Segment{
	int					start;
	int					length;

	URI_Segment():start(-1), length(-1){}
	void reset(){start = length = -1;}
	BOOL IsNULL(){return (start == -1 && length == -1)?TRUE:FALSE;}
	BOOL IsEmpty(){return (start >= 0 && length == 0)?TRUE:FALSE;}
};

struct URI_Authority_Components{
	URI_Segment			Ranges[MAX_URI_AUTHORITY_COMPONENTS];
};

struct URI_NameValue_Pair{
	URI_Segment			name;
	URI_Segment			value;
};

////////////////////////////////////////////////////////////////////////////////
// The utility function for URI
////////////////////////////////////////////////////////////////////////////////
AMP_FOUNDATION_PROC int		AMURI_Split(_In_z_ const char* szURI, URI_Components& URI_Components);
/*!	@brief This function is only available for RFC 3986, for RFC 2396, please don't call it. */
AMP_FOUNDATION_PROC int		AMURI_SplitAuthority(_In_z_ const char* szURI, inplace_str_range authority_component_range, URI_Authority_Components& Authority_Components);
AMP_FOUNDATION_PROC int		AMURI_SplitComponent(_In_z_ const char* szURI, inplace_str_range component_range, std::vector<URI_Segment>& component_segments, const char* delims="/");
AMP_FOUNDATION_PROC int		AMURI_SplitComponent(_In_z_ const char* pComponent, int nComponentLength, std::vector<URI_Segment>& component_segments, const char* delims="/");
AMP_FOUNDATION_PROC int		AMURI_SplitComponent(_In_z_ const char* szComponent, std::vector<URI_Segment>& component_segments, const char* delims="/");
AMP_FOUNDATION_PROC int		AMURI_SplitComponent2(_In_z_ const char* szURI, inplace_str_range component_range, std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims="&", const char* pair_delims="=");
AMP_FOUNDATION_PROC int		AMURI_SplitComponent2(_In_z_ const char* pComponent, int nComponentLength, std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims="&", const char* pair_delims="=");
AMP_FOUNDATION_PROC int		AMURI_SplitComponent2(_In_z_ const char* szComponent, std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims="&", const char* pair_delims="=");

AMP_FOUNDATION_PROC int		AMURI_EncodeFilePath(_In_z_ const char* szFilePath, char* szURI, int cbURI);
AMP_FOUNDATION_PROC int		AMURI_DecodeFilePath(_In_z_ const char* szURI, char* szFilePath, int cbFilePath);
AMP_FOUNDATION_PROC int		AMURI_DecodeSegment(_In_z_ const char* szSegment, int ccSegment, std::string& strDecoded);

////////////////////////////////////////////////////////////////////////////////
// Support relate-ref and absolute URI
////////////////////////////////////////////////////////////////////////////////
AMP_FOUNDATION_PROC AMURI	AMURI_Open(_In_z_ const char* szURI);
AMP_FOUNDATION_PROC int		AMURI_Navigate(AMURI hURI, _In_z_ const char* szNewURI);
AMP_FOUNDATION_PROC int		AMURI_GetCompositeURI(AMURI hURI, _In_reads_z_(ccURI) char* szURI, int ccURI);
AMP_FOUNDATION_PROC void	AMURI_Close(AMURI& hURI);

#endif
