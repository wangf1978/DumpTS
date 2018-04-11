#include "StdAfx.h"
#include <vector>
#include <unordered_map>
#include <map>
#include "Matroska.h"
#include "DumpTS.h"
#include "PayloadBuf.h"

using namespace std;

extern const char *dump_msg[];
extern unordered_map<std::string, std::string> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;

void PrintTree(Matroska::EBMLElement* ptr_element, int level)
{
	if (ptr_element == nullptr)
		return;

	size_t line_chars = level * 5 + 128;
	char* szLine = new char[line_chars];
	memset(szLine, ' ', line_chars);

	const int indent = 2;
	const int level_span = 5;

	char* szText = nullptr;
	if (level >= 1)
	{
		Matroska::EBMLElement* ptr_parent = ptr_element->container;
		memcpy(szLine + indent + (level - 1)*level_span, "|--", 3);
		for (int i = level - 2; i >= 0 && ptr_parent != nullptr; i--)
		{
			if (ptr_parent->next_sibling != nullptr)
				memcpy(szLine + indent + i*level_span, "|", 1);
			ptr_parent = ptr_parent->container;
		}
		szText = szLine + indent + 3 + (level - 1)*level_span;
	}
	else
		szText = szLine + indent;

	if (ptr_element->container == nullptr)
		sprintf_s(szText, line_chars - (szText - szLine), ".\r\n");
	else if (ptr_element->desc_idx < 0 || ptr_element->desc_idx >= _countof(Matroska::EBML_element_descriptors))
		sprintf_s(szText, line_chars - (szText - szLine), "Unknown Element(0X%X)\r\n", ptr_element->ID);
	else
	{
		int cbWritten = 0;
		if (Matroska::EBML_element_descriptors[ptr_element->desc_idx].data_type == Matroska::EBML_DT_MASTER)
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", Matroska::EBML_element_descriptors[ptr_element->desc_idx].Element_Name);
		else
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s: ", Matroska::EBML_element_descriptors[ptr_element->desc_idx].Element_Name);

		if (cbWritten > 0)
			szText += cbWritten;

		cbWritten = -1;
		switch (Matroska::EBML_element_descriptors[ptr_element->desc_idx].data_type)
		{
		case Matroska::EBML_DT_UNSIGNED_INTEGER:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%llu", ((Matroska::UnsignedIntegerElement*)ptr_element)->uVal);
			break;
		case Matroska::EBML_DT_SIGNED_INTEGER:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%lld", ((Matroska::SignedIntegerElement*)ptr_element)->iVal);
			break;
		case Matroska::EBML_DT_DATE:
			break;
		case Matroska::EBML_DT_ASCII_STRING:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", ((Matroska::ASCIIStringElement*)ptr_element)->szVal);
			break;
		case Matroska::EBML_DT_UTF8_STRING:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", ((Matroska::UTF8StringElement*)ptr_element)->szUTF8);
			break;
		case Matroska::EBML_DT_FLOAT:
			break;
		}

		if (cbWritten > 0)
			szText += cbWritten;

		sprintf_s(szText, line_chars - (szText - szLine), "\r\n");
	}

	printf(szLine);

	delete[] szLine;

	auto ptr_child = ptr_element->first_child;
	while (ptr_child != nullptr)
	{
		PrintTree(ptr_child, level + 1);
		ptr_child = ptr_child->next_sibling;
	}

	return;
}

int DumpMKV()
{
	int iRet = -1;

	CFileBitstream bs(g_params["input"].c_str(), 4096, &iRet);

	Matroska::RootElement* root = Matroska::EBMLElement::Root();
	root->Unpack(bs);

	PrintTree(root, 0);

	return iRet;
}
