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
#ifndef __MEDIA_OBJECT_PRINT_H__
#define __MEDIA_OBJECT_PRINT_H__

#include <memory>
#include "tinyxml2.h"
#include "combase.h"
#include "DataUtil.h"

extern const char* g_szRule;
extern bool	g_silent_output;

void PrintMPVSyntaxElement(IUnknown* pCtx, uint8_t* pMPVSE, size_t cbMPVSE, int indent);
void PrintNALUnitSyntaxElement(IUnknown* pCtx, uint8_t* pNALNUBuf, size_t cbNALNUBuf, int indent);
void PrintSEIMsgSyntaxElement(IUnknown* pCtx, uint8_t* pSEIMsgBuf, size_t cbSEIMsgBuf, int indent);
void PrintSEIPayloadSyntaxElement(IUnknown* pCtx, uint32_t payload_type, uint8_t* pSEIPayloadBuf, size_t cbSEIPayloadBuf, int indent);
void PrintOBUSyntaxElement(IUnknown* pCtx, uint8_t* pOBUBuf, size_t cbOBUBuf, int indent);

template<class T>
int PrintMediaObject(T* pNavFieldProp, bool bIgnoreBin = false, int indent = 0)
{
	char* szXmlOutput = NULL;
	int iRet = RET_CODE_SUCCESS;
	tinyxml2::XMLDocument xmlDoc;
	int xml_buffer_size = (int)pNavFieldProp->ProduceDesc(NULL, 0);
	if (xml_buffer_size <= 0)
	{
		printf("Failed to export Xml from the NAL Object.\n");
		goto done;
	}

	szXmlOutput = new char[(size_t)xml_buffer_size + 1];
	if ((xml_buffer_size = (int)pNavFieldProp->ProduceDesc(szXmlOutput, (size_t)xml_buffer_size + 1)) <= 0)
	{
		AMP_SAFEDELA(szXmlOutput);
		printf("Failed to generate the Xml from the NAL Object.\n");
		goto done;
	}

	if (xmlDoc.Parse(szXmlOutput, xml_buffer_size) == tinyxml2::XML_SUCCESS)
	{
		int max_len_of_fixed_part = 0;
		// Get the max length
		{
			struct MaxLenTestUtil : tinyxml2::XMLVisitor
			{
				MaxLenTestUtil(bool bIgnoreBin, int indent) : level(0), szLongSpace{ 0 }, max_length(0), ignore_bin(bIgnoreBin), m_indent(indent){
					memset(szLongSpace, ' ', 240);
				}
				/// Visit an element.
				virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
					char szTmp[2048] = { 0 };

					const char* szDesc = element.Attribute("Desc");
					if (szDesc == nullptr || strcmp(szDesc, "") == 0)
					{
						level++;
						return true;
					}

					const char* szType = element.Attribute("Type");
					if (szType != nullptr && szType[0] == 'M' && szType[1] == '\0')
					{
						level++;
						// Skip it, the matrix is always print in a new line w/o any description
						return true;
					}

					const char* szValue = element.Attribute("Value");
					const char* szAlias = element.Attribute("Alias");
					if (!ignore_bin || (ignore_bin && (szType == nullptr || !(szType[0] == 'B' && szType[1]== '\0'))))
					{
						int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s  ", level * 4 + m_indent, szLongSpace,
														  szAlias ? szAlias : element.Name(), szValue ? szValue : "");
						
						if (ccWritten > max_length)
							max_length = ccWritten;
					}

					level++;
					return true;
				}
				/// Visit an element.
				virtual bool VisitExit(const tinyxml2::XMLElement& element) {
					level--;
					return true;
				}

				int level;
				char szLongSpace[241];
				int max_length;
				bool ignore_bin = false;
				int m_indent;

			} max_length_tester(bIgnoreBin, indent);

			xmlDoc.Accept(&max_length_tester);
			max_len_of_fixed_part = max_length_tester.max_length;
		}

		struct TestUtil : tinyxml2::XMLVisitor
		{
			TestUtil(int max_length, bool bIgnoreBin, int indent) 
				: level(0), szLongSpace{ 0 }
				, max_fixed_part_length(max_length), ignore_bin(bIgnoreBin), m_indent(indent){
				memset(szLongSpace, ' ', 240);
			}
			/// Visit an element.
			virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
				char szTmp[2048] = { 0 };
				const char* szValue = element.Attribute("Value");
				const char* szAlias = element.Attribute("Alias");
				const char* szDesc = element.Attribute("Desc");
				const char* szType = element.Attribute("Type");
				if (!ignore_bin || (ignore_bin && (szType == nullptr || !(szType[0] == 'B' && szType[1] == '\0'))))
				{
					int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s", level * 4 + m_indent, szLongSpace,
							szAlias ? szAlias : element.Name(),
							szType != nullptr && szType[0] == 'M' && szType[1] == '\0'? "":(szValue ? szValue : ""));
					
					if (!g_silent_output)
						printf("%s%.*s%s%s\n", szTmp,
							max_fixed_part_length > ccWritten ? (max_fixed_part_length - ccWritten) : 0, szLongSpace,
							szDesc && strcmp(szDesc, "") != 0 ? "// " : "",
							szDesc ? GetFixedWidthStrWithEllipsis(szDesc, 70).c_str() : "");

					if (szType != nullptr && szType[0] == 'M' && szType[1] == '\0' && szValue != nullptr)
					{
						int value_line_width = 0;
						const char* szLineStart = szValue;
						const char* szLineEnd = nullptr;
						size_t ccValue = strlen(szValue);
						do
						{
							szLineEnd = strchr(szLineStart, '\n');
							if (szLineEnd == nullptr)
								value_line_width = (int)(szValue + ccValue - szLineStart);
							else
								value_line_width = (int)(szLineEnd - szLineStart);

							if (!g_silent_output)
								printf("%*.*s\n", m_indent + level*4 + 4 + value_line_width, value_line_width, szLineStart);

							szLineStart = szLineEnd + 1;
						} while (szLineEnd != nullptr);
					}
				}

				level++;
				return true;
			}
			/// Visit an element.
			virtual bool VisitExit(const tinyxml2::XMLElement& element) {
				level--;
				return true;
			}

			int level;
			char szLongSpace[241];
			int max_fixed_part_length;
			bool ignore_bin = false;
			int m_indent = 0;

		} tester(max_len_of_fixed_part, bIgnoreBin, indent);

		xmlDoc.Accept(&tester);
	}
	else
		printf("The generated XML is invalid.\n");

done:
	AMP_SAFEDELA(szXmlOutput);

	return iRet;
}

#endif
