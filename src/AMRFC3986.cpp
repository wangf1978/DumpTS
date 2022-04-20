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
#include "AMRFC3986.h"
#include <stdlib.h>

#ifdef _MSC_VER
  #define INLINE __forceinline /* use __forceinline (VC++ specific) */
#else
  #define INLINE inline        /* use standard inline */
#endif

enum URI_TYPE
{
	URI_REF,
	URI_BASE,
	URI_TARGET,
};

struct AMURI_REF{
	BOOL				bAvail;						// Available or not
	char				szURIRef[MAX_URI_LENGTH];	// only 8K
	URI_Components		components;					// (R.scheme, R.authority, R.path, R.query, R.fragment)

	AMURI_REF():bAvail(FALSE){
		memset(szURIRef, 0, sizeof(szURIRef));
	}

	void Reset(){
		bAvail = FALSE;
		components.reset();
	}
};

struct AMURI_NAVIGATION{
	AMURI_REF R;		// URI reference
	AMURI_REF Base;		// Base URI
	AMURI_REF T;		// target URI
};

INLINE BOOL IsAlpha(char c){
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

INLINE BOOL IsDigit(char c){
	return c >= '0' && c <= '9';
}

INLINE BOOL IsHexDigit(char c){
	return IsDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

INLINE BOOL IsUnreserved(char c){
	return IsAlpha(c) || IsDigit(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

INLINE BOOL IsGenDelims(char c){
	return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@';
}

INLINE BOOL IsSubDelims(char c){
	return c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || 
		   c == '*' || c == '+' || c == ',' || c == ';'  || c == '=';
}

INLINE BOOL IsRFC2396Reserved(char c){
	return c == ';' || c == '/' || c == '?' || c == ':' || c == '@' || 
		   c == '&' || c == '=' || c == '+' || c == '$' || c == ',';
}

INLINE BOOL IsRFC2396Mark(char c){
	return c == '-' || c == '_' || c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '(' || c == ')';
}

INLINE BOOL IsRFC2396Unreserved(char c)
{
	return IsAlpha(c) || IsDigit(c) || IsRFC2396Mark(c);
}

INLINE BOOL IsDelim(char c, const char* delims){
	while(*delims != '\0' && *delims != c)delims++;
	return *delims?TRUE:FALSE;
}

INLINE BOOL IsReserved(char c){
	return IsGenDelims(c) || IsSubDelims(c);
}

INLINE BOOL IsPctEncoded(_In_z_ char* p){
	return p[0] == '%' && IsHexDigit(p[1]) && IsHexDigit(p[2]);
}

/*!	@retval  0 Current char is not pchar
	@retval <0 Current buffer has error
	@retval >0 Current buffer is pchar. */
INLINE int IsPchar(_In_z_ char* p){
	if (p[0] == '%')
		return IsPctEncoded(p)?3:-1;

	if (IsUnreserved(p[0]) || IsSubDelims(p[0]) || p[0] == ':' || p[0] == '@')
		return 1;
	
	return 0;
}

INLINE int IsQueryChar(_In_z_ char* p){
	int iRet = IsPchar(p);
	return iRet != 0?iRet:((p[0] == '/' || p[0] == '?')?1:0);
}

INLINE int IsFragmentChar(_In_z_ char* p){
	int iRet = IsPchar(p);
	return iRet != 0?iRet:((p[0] == '/' || p[0] == '?')?1:0);
}

//	scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
INLINE int ExtractScheme(_In_z_ const char* szURI, inplace_str_range& range){
	const char* p = szURI;
	if (!IsAlpha(*p++))
		return RET_CODE_MISMATCH;

	while(IsAlpha(*p) || IsDigit(*p) || *p == '+' || *p == '-' || *p == '.')p++;

	if (*p == ':'){
		range.start = 0;
		range.length = (int)(p - szURI);
		range.type = URI_PART_SCHEME;
		return RET_CODE_SUCCESS;
	}

	return RET_CODE_MISMATCH;
}

//	hier-part   = "//" authority path-abempty
//                  / path-absolute
//                  / path-rootless
//                  / path-empty
INLINE int ExtractHierPart(_In_z_ const char* szURI, inplace_str_range& range, int start_pos, bool relative_ref){
	UNREFERENCED_PARAMETER(relative_ref);
	const char* p = szURI + start_pos;
	while(*p != '?' && *p != '#' && *p != '\0')p++;

	range.start = start_pos;
	range.length = (int)(p - szURI - start_pos);
	range.type = URI_PART_HIERPART;

	return RET_CODE_SUCCESS;
}

INLINE int ExtractQueryPart(_In_z_ const char* szURI, inplace_str_range& range, int start_pos)
{
	const char* p = szURI + start_pos;
	while(*p != '#' && *p != '\0')p++;

	range.start = start_pos;
	range.length = (int)(p - szURI - start_pos);
	range.type = URI_PART_QUERY;

	return RET_CODE_SUCCESS;
}

INLINE int ExtractFragmentPart(_In_z_ const char* szURI, inplace_str_range& range, int start_pos)
{
	const char* p = szURI + start_pos;
	while(*p != '\0')p++;

	range.start = start_pos;
	range.length = (int)(p - szURI - start_pos);
	range.type = URI_PART_FRAGMENT;

	return RET_CODE_SUCCESS;
}

INLINE int ExtractAuthorityPart(_In_z_ const char* szURI, inplace_str_range& range, const char* ps, const char* pe)
{
	const char* p = ps;
	while(p != pe && *p != '/')p++;

	range.start = (int)(ps - szURI);
	range.length = (int)(p - ps);
	range.type = URI_PART_AUTHORITY;

	return RET_CODE_SUCCESS;
}

INLINE int ExtractPathAbempty(_In_z_ const char* szURI, inplace_str_range& range, const char* ps, const char* pe)
{
	const char* p = ps;
	if (p < pe){
		if (*p != '/')
			return RET_CODE_MISMATCH;

		while(p != pe)p++;
	}

	range.start = (int)(ps - szURI);
	range.length = (int)(p - ps);
	range.type = URI_PART_PATH_ABEMPTY;

	return RET_CODE_SUCCESS;
}

INLINE int ExtractPathRootLessOrNoScheme(_In_z_ const char* szURI, inplace_str_range& range, const char* ps, const char* pe, BOOL relative)
{
	const char* p = ps;
	while(p != pe)p++;

	range.start = (int)(ps - szURI);
	range.length = (int)(p - ps);
	range.type = relative?URI_PART_PATH_NOSCHEME:URI_PART_PATH_ROOTLESS;

	return RET_CODE_SUCCESS;
}

INLINE int ExtractPathAbsolute(_In_z_ const char* szURI, inplace_str_range& range, const char* ps, const char* pe)
{
	const char* p = ps;
	while(p != pe)p++;

	range.start = (int)(ps - szURI);
	range.length = (int)(p - ps);
	range.type = URI_PART_PATH_ABSOLUTE;

	return RET_CODE_SUCCESS;
}

INLINE int SplitHierPart(_In_z_ const char* szURI, inplace_str_range hier_part_range, URI_Components& URI_heir_parts, BOOL relative)
{
	const char* ps = szURI + hier_part_range.start;
	const char* pe = szURI + hier_part_range.start + hier_part_range.length;
	const char* p  = ps;
	int iRet = RET_CODE_SUCCESS;

	if (ps == pe)
	{
		URI_heir_parts.Ranges[URI_COMPONENT_PATH].start = (int)(ps - szURI);
		URI_heir_parts.Ranges[URI_COMPONENT_PATH].length = 0;
		URI_heir_parts.Ranges[URI_COMPONENT_PATH].type = URI_PART_PATH_EMPTY;
		return RET_CODE_SUCCESS;
	}

	if (*p == '/'){
		if (p+1 != pe && *(p+1) == '/'){
			// "//" authority path-abempty
			p += 2;
			iRet = ExtractAuthorityPart(szURI, URI_heir_parts.Ranges[URI_COMPONENT_AUTHORITY], p, pe);
			if (AMP_FAILED(iRet))
				return iRet;

			p = szURI + URI_heir_parts.Ranges[URI_COMPONENT_AUTHORITY].start + URI_heir_parts.Ranges[URI_COMPONENT_AUTHORITY].length;

			iRet = ExtractPathAbempty(szURI, URI_heir_parts.Ranges[URI_COMPONENT_PATH], p, pe);

			if (AMP_FAILED(iRet))
				return iRet;
		}
		else
		{
			// path-absolute
			iRet = ExtractPathAbsolute(szURI, URI_heir_parts.Ranges[URI_COMPONENT_PATH], p, pe);
			if (AMP_FAILED(iRet))
				return iRet;
		}
	}
	else
	{
		iRet = ExtractPathRootLessOrNoScheme(szURI, URI_heir_parts.Ranges[URI_COMPONENT_PATH], p, pe, relative);
		if (AMP_FAILED(iRet))
			return iRet;
	}

	return iRet;
}

int AMURI_Split(_In_z_ const char* szURIRef, URI_Components& URI_Components)
{
	const char* p = szURIRef;
	if (p == NULL)
		return RET_CODE_INVALID_PARAMETER;

	int iRet = ExtractScheme(p, URI_Components.Ranges[URI_COMPONENT_SCHEME]);

	bool bRelativeRef = true;
	int start_pos = 0;
	if (AMP_SUCCEEDED(iRet))
	{
		bRelativeRef = false;
		start_pos = URI_Components.Ranges[URI_COMPONENT_SCHEME].start + URI_Components.Ranges[URI_COMPONENT_SCHEME].length + 1;
	}

	inplace_str_range range;
	iRet = ExtractHierPart(szURIRef, range, start_pos, bRelativeRef);
	if (AMP_FAILED(iRet))
		return iRet;

	iRet = SplitHierPart(szURIRef,range, URI_Components, bRelativeRef);
	if (AMP_FAILED(iRet))
		return iRet;
	
	start_pos = range.start + range.length + 1;
	if (szURIRef[start_pos-1] == '?' && AMP_SUCCEEDED(iRet = ExtractQueryPart(szURIRef, URI_Components.Ranges[URI_COMPONENT_QUERY], start_pos)))
		start_pos = URI_Components.Ranges[URI_COMPONENT_QUERY].start + URI_Components.Ranges[URI_COMPONENT_QUERY].length + 1;

	if (AMP_SUCCEEDED(iRet) && szURIRef[start_pos-1] == '#')
		iRet = ExtractFragmentPart(szURIRef, URI_Components.Ranges[URI_COMPONENT_FRAGMENT], start_pos);

	return iRet;
}

int AMURI_SplitAuthority(_In_z_ const char* szURI, inplace_str_range authority_component_range, URI_Authority_Components& Authority_Components)
{
	if (szURI == NULL)
		return RET_CODE_INVALID_PARAMETER;

	const char* ps = szURI + authority_component_range.start;
	const char* pe = szURI + authority_component_range.start + authority_component_range.length;
	const char* p  = ps;
	int iRet = RET_CODE_SUCCESS;

	if (ps == pe){
		Authority_Components.Ranges[URI_AUTHORITY_HOST].start = authority_component_range.start;
		Authority_Components.Ranges[URI_AUTHORITY_HOST].length = 0;
		return RET_CODE_SUCCESS;
	}

	// try to find '@'
	while(p != pe && *p != '@')p++;

	if (p != pe){
		Authority_Components.Ranges[URI_AUTHORITY_USERINFO].start = authority_component_range.start;
		Authority_Components.Ranges[URI_AUTHORITY_USERINFO].length = (int)(p - ps);
		p++;
	}
	else
		p = ps;

	Authority_Components.Ranges[URI_AUTHORITY_HOST].start = (int)(p - szURI);
	if (p != pe){
		if (*p == '['){
			// IP-literal routine, find the paired ']'
			p++;
			while(p != pe && *p != ']')p++;

			if (p != pe)
				p++;
		}

		while(p != pe && *p != ':' && *p != '/')p++;

		if (p != pe && *p == ':'){
			Authority_Components.Ranges[URI_AUTHORITY_HOST].length = (int)(p - szURI - Authority_Components.Ranges[URI_AUTHORITY_HOST].start);
			// Skip ':' and find the port segment
			p++;
			Authority_Components.Ranges[URI_AUTHORITY_PORT].start = (int)(p - szURI);
			Authority_Components.Ranges[URI_AUTHORITY_PORT].length = (int)(pe - p);
		}
		else
			Authority_Components.Ranges[URI_AUTHORITY_HOST].length = (int)(p - szURI - Authority_Components.Ranges[URI_AUTHORITY_HOST].start);
	}

	return iRet;
}

int	_SplitComponent(_In_reads_(ccComponentLength) const char* pComponent, int ccComponentLength, 
					std::vector<URI_Segment>& component_segments, const char* delims, int offset=0)
{
	const char* ps = pComponent;
	const char* pe = ccComponentLength == -1?(char*)((intptr_t)-1):(pComponent + ccComponentLength);
	const char* p  = ps;
	URI_Segment range;

	const char* psegstart = p;
	bool bTerminated;
	do
	{
		bTerminated = (p == pe || *p == '\0');
		if (bTerminated || IsDelim(*p, delims)){
			range.start = (int)(psegstart - pComponent + offset);
			range.length = (int)(p - psegstart);
			component_segments.push_back(range);
			if (p < pe)
				psegstart = p + 1;
		}

		p++;
	}while(!bTerminated);

	return RET_CODE_SUCCESS;
}

// Pair pattern:
// nv_segment = [name ":"]value
// nv_pari_segments = nv_segment *("&" nv_segment)
int	_SplitComponent2(_In_reads_(ccComponentLength) const char* pComponent, int ccComponentLength, 
					 std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims, const char* pair_delims, int offset=0)
{
	const char* ps = pComponent;
	const char* pe = ccComponentLength == -1?(char*)((intptr_t)-1):(pComponent + ccComponentLength);
	const char* p  = ps;
	URI_NameValue_Pair nv_range;

	const char* psegstart = p;
	const char* psubdelims = NULL;
	bool bTerminated;
	do
	{
		bTerminated = (p == pe || *p == '\0');
		if (bTerminated || IsDelim(*p, delims)){
			if (psubdelims != NULL){
				nv_range.name.start = (int)(psegstart - pComponent + offset);
				nv_range.name.length = (int)(psubdelims - psegstart);
			}
			else
			{
				nv_range.name.start = nv_range.name.length = -1;
			}

			nv_range.value.start = (int)((psubdelims?(psubdelims + 1):psegstart) - pComponent + offset);
			nv_range.value.length = (int)(p - (psubdelims?(psubdelims + 1):psegstart));

			namevalue_pairs.push_back(nv_range);
			if (p < pe){
				psegstart = p + 1;
				psubdelims = NULL;
			}
		}
		else if(psubdelims == NULL && IsDelim(*p, pair_delims))
		{
			psubdelims = p;
		}

		p++;
	}while(!bTerminated);

	return RET_CODE_SUCCESS;
}

int	AMURI_SplitComponent(_In_z_ const char* szURI, inplace_str_range component_range, std::vector<URI_Segment>& component_segments, const char* delims)
{
	if (szURI == NULL || component_range.start < 0 || component_range.length < 0 || delims == NULL)
		return RET_CODE_INVALID_PARAMETER;

	return _SplitComponent(szURI + component_range.start, component_range.length, component_segments, delims, component_range.start);
}

int	AMURI_SplitComponent(_In_z_ const char* pComponent, int ccComponentLength, std::vector<URI_Segment>& component_segments, const char* delims)
{
	if (pComponent == NULL || ccComponentLength < 0 || delims == NULL)
		return RET_CODE_INVALID_PARAMETER;

	return _SplitComponent(pComponent, ccComponentLength, component_segments, delims);
}

int	AMURI_SplitComponent(_In_z_ const char* szComponent, std::vector<URI_Segment>& component_segments, const char* delims)
{
	if (szComponent == NULL || delims == NULL)
		return RET_CODE_INVALID_PARAMETER;

	return _SplitComponent(szComponent, -1, component_segments, delims);
}

int	AMURI_SplitComponent2(_In_z_ const char* szURI, inplace_str_range component_range, 
						  std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims, const char* pair_delims)
{
	if (szURI == NULL || component_range.start < 0 || component_range.length < 0 || delims == NULL || pair_delims == NULL)
		return RET_CODE_INVALID_PARAMETER;

	return _SplitComponent2(szURI + component_range.start, component_range.length, namevalue_pairs, delims, pair_delims, component_range.start);
}

int	AMURI_SplitComponent2(_In_z_ const char* pComponent, int ccComponentLength, 
						  std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims, const char* pair_delims)
{
	if (pComponent == NULL || ccComponentLength < 0 || delims == NULL || pair_delims == NULL)
		return RET_CODE_INVALID_PARAMETER;

	return _SplitComponent2(pComponent, ccComponentLength, namevalue_pairs, delims, pair_delims);
}

int	AMURI_SplitComponent2(_In_z_ const char* szComponent, std::vector<URI_NameValue_Pair>& namevalue_pairs, const char* delims, const char* pair_delims)
{
	if (szComponent == NULL || delims == NULL || pair_delims == NULL)
		return RET_CODE_INVALID_PARAMETER;

	return _SplitComponent2(szComponent, -1, namevalue_pairs, delims, pair_delims);
}

int AMURI_EncodeFilePath(_In_z_ const char* szFilePath, char* szURI, int cbURI)
{
	return -1;
}

int AMURI_DecodeFilePath(_In_z_ const char* szURI, char* szFilePath, int cbFilePath)
{
	return -1;
}

int AMURI_DecodeSegment(_In_z_ const char* szSegment, int ccSegment, std::string& strDecoded)
{
	if (szSegment == nullptr || ccSegment <= 0)
		return RET_CODE_INVALID_PARAMETER;

	strDecoded.reserve((size_t)((uint64_t)ccSegment + 1));
	strDecoded.clear();

	const char* p = szSegment;
	int ccLeft = ccSegment;
	while (ccLeft > 0)
	{
		if (*p == '%')
		{
			if (ccLeft < 3)
				return RET_CODE_ERROR;

			uint8_t h[2] = { 0 };
			for (int i = 0; i < 2; i++)
			{
				uint8_t c = *(p + 1 + i);
				if (c >= '0' && c <= '9')
					h[i] = c - '0';
				else if (c >= 'a' && c <= 'f')
					h[i] = c - 'a' + 10;
				else if (c >= 'A' && c <= 'F')
					h[i] = c - 'A' + 10;
				else
					return RET_CODE_ERROR;
			}

			strDecoded.push_back((char)(((h[0] << 4) & 0xF) | (h[1] & 0xF)));
			p += 3; ccLeft -= 3;
		}
		else
		{
			strDecoded.push_back(*p);
			p++; ccLeft--;
		}
	}

	return RET_CODE_SUCCESS;
}

/*!	
	@return If the function succeeds, the return value is the length, in TCHARs, of the string copied to pOutPath, not including the terminating null character. 
			If the return value is greater than ccOutPath, the return value is the length, in TCHARs, of the buffer required to hold the path.
			if the return value is less than 0, at least one error happen 
*/
int AMURI_RemoveDotSegments(_In_z_ char* pInPath, int ccInPath, _Inout_z_ char* pOutPath, int ccOutPath)
{
	if (pInPath == 0 || ccInPath == 0)
		return RET_CODE_INVALID_PARAMETER;

	if (ccOutPath <= 0)
		pOutPath = NULL;

	int ccRequired = 1;
	std::vector<int> po_slash_pos;
	char* pi = pInPath,  *pie = pInPath + ccInPath;
	char* po = pOutPath, *poe = pOutPath == NULL?NULL:(pOutPath + ccOutPath);

	do
	{
		if (pie - pi >= 3 && pi[0] == '.' && pi[1] == '.' && pi[2] == '/')
			pi += 3;	// Skip it
		else if(pie - pi >= 2 && pi[0] == '.' && pi[1] == '/')
			pi += 2;	// Skip it
		else if(pie - pi >= 3 && pi[0] == '/' && pi[1] == '.' && pi[2] == '/')
			pi += 2;
		else if(pie - pi == 2 && pi[0] == '/' && pi[1] == '.'){
			pi += 1;
			*pi = '/';
		}
		else if(pie - pi >= 4 && pi[0] == '/' && pi[1] == '.' && pi[2] == '.' && pi[3] == '/'){
			pi += 3;
			if (pOutPath != NULL){
				if (po_slash_pos.size() > 0){
					po = pOutPath + po_slash_pos.back();
					po_slash_pos.pop_back();
				}
				else
					po = pOutPath;
			}
		}
		else if(pie - pi == 3 && pi[0] == '/' && pi[1] == '.' && pi[2] == '.'){
			pi += 2;
			*pi = '/';
			if (pOutPath != NULL){
				if (po_slash_pos.size() > 0){
					po = pOutPath + po_slash_pos.back();
					po_slash_pos.pop_back();
				}
				else
					po = pOutPath;
			}
		}
		else if((pie - pi == 1 && pi[0] == '.') || (pie - pi == 2 && pi[0] == '.' && pi[1] == '.'))
			pi = pie;
		else
		{
			// Find the next '/' or pie
			char* psegstart = pi;
			if (pi != pie)
				pi++;

			while(pi != pie && *pi != '/')pi++;
			// move the current segment to output path
			if (pi - psegstart > 0){
				if (pOutPath != NULL){
					// Need reserve one char for zero-terminator.
					if (poe - po > pi - psegstart){
						if (*psegstart == '/'){
							int slash_pos = (int)(po -pOutPath);
							po_slash_pos.push_back(slash_pos);
						}
						STRNCPY(po, poe - po, psegstart, pi - psegstart);
						po += pi - psegstart;
					}
					else
						po = pOutPath = NULL;	// output buffer is too small
				}
				ccRequired += (int)(pi - psegstart);
			}
		}
	}while(pi != pie);

	AMP_SAFEASSIGN(po, '\0');

	return pOutPath == NULL?ccRequired:(int)(po - pOutPath);
}

int AMURI_MergeRBPath(AMURI_NAVIGATION* URINav, int nTWritePos)
{
	AMURI_REF& Base = URINav->Base;
	AMURI_REF& T = URINav->T;
	AMURI_REF& R = URINav->R;

	if (Base.components.Ranges[URI_COMPONENT_AUTHORITY].length >= 0 && Base.components.Ranges[URI_COMPONENT_PATH].length == 0){
		// decide the path len
		int nMergedPathLen = 1 + R.components.Ranges[URI_COMPONENT_PATH].length;	// '/'
		if (nMergedPathLen + 1 > MAX_URI_LENGTH - nTWritePos)
			return RET_CODE_BUFFER_TOO_SMALL;

		T.components.Ranges[URI_COMPONENT_PATH].start = nTWritePos;
		T.components.Ranges[URI_COMPONENT_PATH].length = nMergedPathLen;
		T.components.Ranges[URI_COMPONENT_PATH].type = URI_PART_PATH_ABEMPTY;

		T.szURIRef[nTWritePos++] = '/';
		STRNCPY(T.szURIRef + nTWritePos, (ptrdiff_t)MAX_URI_LENGTH - nTWritePos, 
			R.szURIRef + R.components.Ranges[URI_COMPONENT_PATH].start, R.components.Ranges[URI_COMPONENT_PATH].length);
	}
	else
	{
		int last_splash_pos_in_base = Base.components.Ranges[URI_COMPONENT_PATH].start + Base.components.Ranges[URI_COMPONENT_PATH].length - 1;
		while(last_splash_pos_in_base >= Base.components.Ranges[URI_COMPONENT_PATH].start && 
			  Base.szURIRef[last_splash_pos_in_base] != '/')
			last_splash_pos_in_base--;

		if (last_splash_pos_in_base < Base.components.Ranges[URI_COMPONENT_PATH].start)
		{
			int nMergedPathLen = R.components.Ranges[URI_COMPONENT_PATH].length;
			if (nMergedPathLen + 1 > MAX_URI_LENGTH - nTWritePos)
				return RET_CODE_BUFFER_TOO_SMALL;

			T.components.Ranges[URI_COMPONENT_PATH].start = nTWritePos;
			T.components.Ranges[URI_COMPONENT_PATH].length = nMergedPathLen;
			T.components.Ranges[URI_COMPONENT_PATH].type = R.components.Ranges[URI_COMPONENT_PATH].type;

			STRNCPY(T.szURIRef + nTWritePos, (ptrdiff_t)MAX_URI_LENGTH - nTWritePos, 
				R.szURIRef + R.components.Ranges[URI_COMPONENT_PATH].start, R.components.Ranges[URI_COMPONENT_PATH].length);
		}
		else
		{
			int nMergedPathLen = last_splash_pos_in_base - Base.components.Ranges[URI_COMPONENT_PATH].start + 1 + R.components.Ranges[URI_COMPONENT_PATH].length;
			if (nMergedPathLen + 1 > MAX_URI_LENGTH - nTWritePos)
				return RET_CODE_BUFFER_TOO_SMALL;

			T.components.Ranges[URI_COMPONENT_PATH].start = nTWritePos;
			T.components.Ranges[URI_COMPONENT_PATH].length = nMergedPathLen;
			T.components.Ranges[URI_COMPONENT_PATH].type = Base.components.Ranges[URI_COMPONENT_PATH].type;

			STRNCPY(T.szURIRef + nTWritePos, (ptrdiff_t)MAX_URI_LENGTH - nTWritePos,
				Base.szURIRef + Base.components.Ranges[URI_COMPONENT_PATH].start, 
				(ptrdiff_t)last_splash_pos_in_base - Base.components.Ranges[URI_COMPONENT_PATH].start + 1);
			nTWritePos += last_splash_pos_in_base - Base.components.Ranges[URI_COMPONENT_PATH].start + 1;
			STRNCPY(T.szURIRef + nTWritePos, (ptrdiff_t)MAX_URI_LENGTH - nTWritePos, 
				R.szURIRef + R.components.Ranges[URI_COMPONENT_PATH].start, R.components.Ranges[URI_COMPONENT_PATH].length);
		}
	}

	return RET_CODE_SUCCESS;
}

INLINE int AMURI_CopyRBT2T(AMURI_NAVIGATION* URINav, int idxComponent, int& nTWritePos, URI_TYPE SourceURIType=URI_REF)
{
	int iRet = RET_CODE_SUCCESS;
	AMURI_REF& S = SourceURIType==URI_REF?URINav->R:(SourceURIType==URI_BASE?URINav->Base:URINav->T);
	AMURI_REF& T = URINav->T;

	if (S.bAvail == FALSE)
		return RET_CODE_ERROR;

	if (idxComponent == URI_COMPONENT_PATH && S.components.Ranges[URI_COMPONENT_PATH].length > 0)
	{
		// Input buffer may be changed
		char* pInPath = NULL;
		AMP_NEW0(pInPath, char, MAX_URI_LENGTH);
		if (pInPath == NULL)
			return RET_CODE_OUTOFMEMORY;

		STRNCPY(pInPath, MAX_URI_LENGTH, S.szURIRef + S.components.Ranges[URI_COMPONENT_PATH].start, S.components.Ranges[URI_COMPONENT_PATH].length);
		iRet = AMURI_RemoveDotSegments(pInPath, S.components.Ranges[URI_COMPONENT_PATH].length, 
			T.szURIRef + nTWritePos, _countof(T.szURIRef) - nTWritePos);
		AMP_SAFEDELA(pInPath);

		if (AMP_FAILED(iRet))
			return iRet;

		T.components.Ranges[URI_COMPONENT_PATH].start = nTWritePos;
		T.components.Ranges[URI_COMPONENT_PATH].length = iRet;
		// Need re-decide what kind of detailed path type
		T.components.Ranges[URI_COMPONENT_PATH].type = S.components.Ranges[URI_COMPONENT_PATH].type;

		nTWritePos += T.components.Ranges[URI_COMPONENT_PATH].length;
	}
	else
	{
		T.components.Ranges[idxComponent].start = nTWritePos;
		T.components.Ranges[idxComponent].length = S.components.Ranges[idxComponent].length;
		T.components.Ranges[idxComponent].type = S.components.Ranges[idxComponent].type;
		if (S.components.Ranges[idxComponent].length > 0){
			STRNCPY(T.szURIRef + nTWritePos, (ptrdiff_t)MAX_URI_LENGTH - nTWritePos,
				S.szURIRef + S.components.Ranges[idxComponent].start, S.components.Ranges[idxComponent].length);

			nTWritePos += S.components.Ranges[idxComponent].length;
		}
	}

	return iRet;
}

int AMURI_Transform_References(AMURI_NAVIGATION* URINav)
{
	int iRet = RET_CODE_SUCCESS;
	AMURI_REF& R = URINav->R;
	if (!R.bAvail)
		return RET_CODE_ERROR;

	URINav->T.Reset();
	URINav->T.bAvail = TRUE;

	int nTWritePos = 0;
	if (R.components.Ranges[URI_COMPONENT_SCHEME].length >= 0)
	{
		AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_SCHEME, nTWritePos)));
		AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_AUTHORITY, nTWritePos)));
		AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_PATH, nTWritePos)));
		AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_QUERY, nTWritePos)));
	}
	else
	{
		AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_SCHEME, nTWritePos, URI_BASE)));
		
		if (R.components.Ranges[URI_COMPONENT_AUTHORITY].length >= 0)
		{
			AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_AUTHORITY, nTWritePos)));
			AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_PATH, nTWritePos)));
			AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_QUERY, nTWritePos)));
		}
		else
		{
			AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_AUTHORITY, nTWritePos, URI_BASE)));
			AMP_Assert(R.components.Ranges[URI_COMPONENT_PATH].length >= 0);
			if (R.components.Ranges[URI_COMPONENT_PATH].length == 0)
			{
				AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_PATH, nTWritePos, URI_BASE)));
				AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_QUERY,nTWritePos, R.components.Ranges[URI_COMPONENT_QUERY].length >= 0?URI_REF:URI_BASE)));
			}
			else
			{
				if (R.szURIRef[R.components.Ranges[URI_COMPONENT_PATH].start] == '/'){
					AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_PATH, nTWritePos)));
				}
				else
				{
					AMP_CHKRET((iRet = AMURI_MergeRBPath(URINav, nTWritePos)));
					AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_PATH, nTWritePos, URI_TARGET)));
				}
				AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_QUERY, nTWritePos, URI_REF)));
			}
		}
	}

	AMP_CHKRET((iRet = AMURI_CopyRBT2T(URINav, URI_COMPONENT_FRAGMENT, nTWritePos)));

done:
	return iRet;
}

int AMURI_Component_Recomposition(AMURI_NAVIGATION* URINav, _In_reads_z_(ccURI) char* szURI, int ccURI)
{
	// Need reserve one char to hold the terminator '\0'
	int ccRequired = 1, nPos = 0;
	AMURI_REF& T = URINav->T;

	if (T.bAvail == FALSE)
		return RET_CODE_ERROR;

	if (T.components.Ranges[URI_COMPONENT_SCHEME].length > 0)
	{
		ccRequired += T.components.Ranges[URI_COMPONENT_SCHEME].length;
		ccRequired += 1;	// Appending ":"

		if (szURI != NULL && ccURI >= ccRequired){
			STRNCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, 
				T.szURIRef + T.components.Ranges[URI_COMPONENT_SCHEME].start, T.components.Ranges[URI_COMPONENT_SCHEME].length);
			nPos += T.components.Ranges[URI_COMPONENT_SCHEME].length;
			STRCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, ":");
			nPos++;
		}
	}

	if (T.components.Ranges[URI_COMPONENT_AUTHORITY].length >= 0)
	{
		ccRequired += 2;	// Appending "//"
		ccRequired += T.components.Ranges[URI_COMPONENT_AUTHORITY].length;
		if (szURI != NULL && ccURI >= ccRequired){
			STRCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, "//");
			nPos += 2;

			if (T.components.Ranges[URI_COMPONENT_AUTHORITY].length > 0){
				STRNCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, 
					T.szURIRef + T.components.Ranges[URI_COMPONENT_AUTHORITY].start, T.components.Ranges[URI_COMPONENT_AUTHORITY].length);
				nPos += T.components.Ranges[URI_COMPONENT_AUTHORITY].length;
			}
		}
	}

	AMP_Assert(T.components.Ranges[URI_COMPONENT_PATH].length >= 0);

	if (T.components.Ranges[URI_COMPONENT_PATH].length > 0){
		ccRequired += T.components.Ranges[URI_COMPONENT_PATH].length;
		if (szURI != NULL && ccURI >= ccRequired){
			STRNCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, 
				T.szURIRef + T.components.Ranges[URI_COMPONENT_PATH].start, T.components.Ranges[URI_COMPONENT_PATH].length);
			nPos += T.components.Ranges[URI_COMPONENT_PATH].length;
		}
	}

	if (T.components.Ranges[URI_COMPONENT_QUERY].length >= 0)
	{
		ccRequired += 1;	// Appending "?"
		ccRequired += T.components.Ranges[URI_COMPONENT_QUERY].length;

		if (szURI != NULL && ccURI >= ccRequired){
			STRCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, "?");
			nPos += 1;

			if (T.components.Ranges[URI_COMPONENT_QUERY].length > 0){
				STRNCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, T.szURIRef + T.components.Ranges[URI_COMPONENT_QUERY].start, T.components.Ranges[URI_COMPONENT_QUERY].length);
				nPos += T.components.Ranges[URI_COMPONENT_QUERY].length;
			}
		}
	}

	if (T.components.Ranges[URI_COMPONENT_FRAGMENT].length >= 0)
	{
		ccRequired += 1;	// Appending "#"
		ccRequired += T.components.Ranges[URI_COMPONENT_FRAGMENT].length;

		if (szURI != NULL && ccURI >= ccRequired){
			STRCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, _T("#"));
			nPos += 1;

			if (T.components.Ranges[URI_COMPONENT_FRAGMENT].length > 0){
				STRNCPY(szURI + nPos, (ptrdiff_t)ccURI - nPos, 
					T.szURIRef + T.components.Ranges[URI_COMPONENT_FRAGMENT].start, T.components.Ranges[URI_COMPONENT_FRAGMENT].length);
				nPos += T.components.Ranges[URI_COMPONENT_FRAGMENT].length;
			}
		}
	}

	AMP_Assert(nPos + 1 <= ccRequired);

	return nPos + 1 == ccRequired?nPos:ccRequired;
}

int AMURI_UpdateURIRefComponents(AMURI_NAVIGATION* URINav, URI_TYPE URIType, _In_z_ const char* szURI)
{
	AMURI_REF& S = URIType==URI_REF?URINav->R:(URIType==URI_BASE?URINav->Base:URINav->T);

	S.Reset();

	if (AMP_FAILED(AMURI_Split(szURI, S.components)))
		return RET_CODE_INVALID_PARAMETER;

	S.bAvail = TRUE;
	STRCPY(S.szURIRef, _countof(S.szURIRef), szURI);

	return RET_CODE_SUCCESS;
}

AMURI AMURI_Open(_In_z_ const char* szURI)
{
	int iRet = RET_CODE_SUCCESS;
	char *szNewTargetURI = NULL;

	AMP_NEWT1(URINav, AMURI_NAVIGATION);
	if (URINav == NULL)
		return NULL;

	if (AMP_FAILED((iRet = AMURI_UpdateURIRefComponents(URINav, URI_REF, szURI)))){
		printf("[URIParser] Failed to split into separated URI components.\n");
		goto done;
	}

	if (URINav->R.components.Ranges[URI_COMPONENT_SCHEME].length <= 0){
		iRet = RET_CODE_INVALID_PARAMETER;
		printf("[URIParser] The scheme part should be not empty.\n");
		goto done;
	}

	if (URINav->R.components.Ranges[URI_COMPONENT_PATH].length < 0){
		iRet = RET_CODE_INVALID_PARAMETER;
		printf("[URIParser] The path part should be defined.\n");
		goto done;
	}

	// It is not necessary to used base URI in the phase	
	if (AMP_FAILED((iRet = AMURI_Transform_References(URINav)))){
		AMP_SAFEDEL(URINav);
		return NULL;
	}

	// Check and update the base URI
	AMP_NEW0(szNewTargetURI, char, MAX_URI_LENGTH);
	if (szNewTargetURI == NULL){
		iRet = RET_CODE_OUTOFMEMORY;
		goto done;
	}

	if (AMP_FAILED((iRet = AMURI_Component_Recomposition(URINav, szNewTargetURI, MAX_URI_LENGTH))) || iRet > MAX_URI_LENGTH){
		iRet = RET_CODE_INVALID_PARAMETER;
		printf("[URIParser] There exist at least one error in URI.\n"); 
		goto done;
	}

	// Update the base RUI components
	if (AMP_FAILED((iRet = AMURI_UpdateURIRefComponents(URINav, URI_BASE, szNewTargetURI)))){
		printf("[URIParser] Failed to update URI components of Base URI.\n");
		goto done;
	}

done:
	if (AMP_FAILED(iRet)){
		AMP_SAFEDEL(URINav);
	}

	AMP_SAFEDELA(szNewTargetURI);

	return URINav;
}

int AMURI_Navigate(AMURI hURI, _In_z_ const char* szNewURIRef)
{
	int iRet = RET_CODE_SUCCESS;
	if (IS_INVALID_HANDLE(hURI))
		return RET_CODE_INVALID_PARAMETER;

	AMURI_NAVIGATION* URINav = (AMURI_NAVIGATION*)hURI;
	if (AMP_FAILED((iRet = AMURI_UpdateURIRefComponents(URINav, URI_REF, szNewURIRef)))){
		printf("[URIParser] Failed to parse URI components of input URI-reference: %s.\n", szNewURIRef);
		return iRet;
	}

	// Update the target reference
	if (AMP_FAILED((iRet = AMURI_Transform_References(URINav)))){
		printf("[URIParser] Failed to transfer the URI reference.\n");
		URINav->T.bAvail = FALSE;
		return iRet;
	}

	// Update the base URI
	char *szNewTargetURI = NULL;
	AMP_NEW0(szNewTargetURI, char, MAX_URI_LENGTH);
	if (szNewTargetURI == NULL)
		return RET_CODE_OUTOFMEMORY;

	if (AMP_FAILED((iRet = AMURI_Component_Recomposition(URINav, szNewTargetURI, MAX_URI_LENGTH))) || iRet > MAX_URI_LENGTH){
		AMP_SAFEDELA(szNewTargetURI);
		printf("[URIParser] Failed to do component recomposition.\n");
		return RET_CODE_ERROR;
	}

	if (AMP_FAILED((iRet = AMURI_UpdateURIRefComponents(URINav, URI_BASE, szNewTargetURI)))){
		printf("[URIParser] Failed to parse URI components of recomposed URI: %s.\n", szNewTargetURI);
		AMP_SAFEDELA(szNewTargetURI);
		return RET_CODE_ERROR;
	}

	AMP_SAFEDELA(szNewTargetURI);

	return RET_CODE_SUCCESS;
}

int AMURI_GetCompositeURI(AMURI hURI, _In_reads_z_(ccURI) char* szURI, int ccURI)
{
	if (IS_INVALID_HANDLE(hURI))
		return RET_CODE_INVALID_PARAMETER;

	AMURI_NAVIGATION* URINav = (AMURI_NAVIGATION*)hURI;
	return AMURI_Component_Recomposition(URINav, szURI, ccURI);
}

void AMURI_Close(AMURI& hURI)
{
	if (IS_INVALID_HANDLE(hURI))
		return;

	AMURI_NAVIGATION* URINav = (AMURI_NAVIGATION*)hURI;
	AMP_SAFEDEL(URINav);
	return;
}
