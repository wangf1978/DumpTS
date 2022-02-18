#ifndef __MEDIA_OBJECT_PRINT_H__
#define __MEDIA_OBJECT_PRINT_H__

#include "tinyxml2.h"

template<class T>
int PrintMediaObject(std::shared_ptr<T> pNavFieldProp)
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
				MaxLenTestUtil() : level(0), szLongSpace{ 0 }, max_length(0){
					memset(szLongSpace, ' ', 240);
				}
				/// Visit an element.
				virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
					char szTmp[2048] = { 0 };
					const char* szValue = element.Attribute("Value");
					const char* szAlias = element.Attribute("Alias");
					const char* szDesc = element.Attribute("Desc");
					int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s", level * 4, szLongSpace,
						szAlias ? szAlias : element.Name(),
						szValue ? szValue : "");

					if (ccWritten > max_length && szDesc != NULL && strcmp(szDesc, "") != 0)
						max_length = ccWritten;

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

			} max_length_tester;

			xmlDoc.Accept(&max_length_tester);
			max_len_of_fixed_part = max_length_tester.max_length;
		}

		struct TestUtil : tinyxml2::XMLVisitor
		{
			TestUtil(int max_length) : level(0), szLongSpace{ 0 }, max_fixed_part_length(max_length){
				memset(szLongSpace, ' ', 240);
			}
			/// Visit an element.
			virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
				char szTmp[2048] = { 0 };
				const char* szValue = element.Attribute("Value");
				const char* szAlias = element.Attribute("Alias");
				const char* szDesc = element.Attribute("Desc");
				int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s", level * 4, szLongSpace,
					szAlias ? szAlias : element.Name(),
					szValue ? szValue : "");

				printf("%s%.*s%s%s\n", szTmp,
					max_fixed_part_length > ccWritten ? (max_fixed_part_length - ccWritten) : 0, szLongSpace,
					szDesc && strcmp(szDesc, "") != 0 ? "// " : "",
					szDesc ? GetFixedWidthStrWithEllipsis(szDesc, 70).c_str() : "");
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

		} tester(max_len_of_fixed_part);

		xmlDoc.Accept(&tester);
	}
	else
		printf("The generated XML is invalid.\n");

done:
	AMP_SAFEDELA(szXmlOutput);

	return iRet;
}

#endif
