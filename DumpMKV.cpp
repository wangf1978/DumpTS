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

	size_t line_chars = level * 5 + 160;
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

	int32_t desc_idx = ptr_element->GetDescIdx(ptr_element->ID);

	if (ptr_element->container == nullptr)
		sprintf_s(szText, line_chars - (szText - szLine), ".\r\n");
	else if (desc_idx < 0 || desc_idx >= _countof(Matroska::EBML_element_descriptors))
		sprintf_s(szText, line_chars - (szText - szLine), "Unknown Element(0X%X), size: %lld\r\n", ptr_element->ID, ptr_element->Size);
	else
	{
		int cbWritten = 0;
		if (Matroska::EBML_element_descriptors[desc_idx].data_type == Matroska::EBML_DT_MASTER)
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s (Size: %lld)",
				Matroska::EBML_element_descriptors[desc_idx].Element_Name, ptr_element->Size);
		else if (Matroska::EBML_element_descriptors[desc_idx].data_type == Matroska::EBML_DT_BINARY && ptr_element->ID == 0xA3)
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s (Size:% 8lld): ",
				Matroska::EBML_element_descriptors[desc_idx].Element_Name, ptr_element->Size);
		else
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s (Size: %lld): ", 
				Matroska::EBML_element_descriptors[desc_idx].Element_Name, ptr_element->Size);

		if (cbWritten > 0)
			szText += cbWritten;

		cbWritten = -1;
		switch (Matroska::EBML_element_descriptors[desc_idx].data_type)
		{
		case Matroska::EBML_DT_UNSIGNED_INTEGER:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%llu", ((Matroska::UnsignedIntegerElement*)ptr_element)->uVal);
			break;
		case Matroska::EBML_DT_SIGNED_INTEGER:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%lld", ((Matroska::SignedIntegerElement*)ptr_element)->iVal);
			break;
		case Matroska::EBML_DT_DATE:
			{
				struct tm tmVal;
				if (localtime_s(&tmVal, &((Matroska::DateElement*)ptr_element)->timeVal) == 0)
					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%d-%02d-%02d %02dh:%02dh:%02d", 
						tmVal.tm_year + 1900, tmVal.tm_mon + 1, tmVal.tm_mday, tmVal.tm_hour, tmVal.tm_min, tmVal.tm_sec);
				else
					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "Unknown date-time");
			}
			break;
		case Matroska::EBML_DT_ASCII_STRING:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", ((Matroska::ASCIIStringElement*)ptr_element)->szVal);
			break;
		case Matroska::EBML_DT_UTF8_STRING:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", ((Matroska::UTF8StringElement*)ptr_element)->szUTF8);
			break;
		case Matroska::EBML_DT_FLOAT:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%f", ((Matroska::FloatElement*)ptr_element)->fVal);
			break;
		case Matroska::EBML_DT_BINARY:
			switch (ptr_element->ID)
			{
			case 0xA3:
			{
				Matroska::SimpleBlockElement* pSimpleBlock = (Matroska::SimpleBlockElement*)ptr_element;
				cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "trackno: %d, rel-timecode:% 6d, keyframe: %d, invisible: %d, lacing: %d, discardable: %d",
					pSimpleBlock->simple_block_hdr.track_number,
					(int)((short)pSimpleBlock->simple_block_hdr.timecode),
					pSimpleBlock->simple_block_hdr.Keyframe,
					pSimpleBlock->simple_block_hdr.Invisible,
					pSimpleBlock->simple_block_hdr.Lacing,
					pSimpleBlock->simple_block_hdr.Discardable);
			}
				break;
			default:
				cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "Binary");
			}
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

int ShowMKVInfo(Matroska::EBMLElement* root, Matroska::EBMLElement* ptr_element)
{
	int iRet = RET_CODE_SUCCESS;
	if (ptr_element == nullptr)
	{
		PrintTree(root, 0);
	}
	else
	{
		PrintTree(ptr_element, 0);
	}

	return iRet;
}

// @retval -1, failed to find the element
// @retval 0, find one element meet the condition 
// @retval 1, cease the recursive find
int FindEBMLElementByElementID(Matroska::EBMLElement* ptr_element, uint32_t element_id, uint32_t track_id, std::vector<Matroska::EBMLElement*>& result)
{
	int iRet = -1;
	if (ptr_element == nullptr)
		return -1;

	Matroska::EBMLElement* ptr_child = ptr_element->first_child;

	while (ptr_child != nullptr)
	{
		if (element_id != UINT32_MAX && ptr_child->ID == element_id)
		{
			if (element_id == 0xA3)	// For SimpleBlock
			{
				// Need filter the track_id in advance
				auto pSimpleBlock = (Matroska::SimpleBlockElement*)ptr_child;
				if (pSimpleBlock->simple_block_hdr.track_number == track_id && track_id != UINT32_MAX || track_id == UINT32_MAX)
				{
					result.push_back(ptr_child);
					iRet = 0;
				}
			}
			else
			{
				result.push_back(ptr_child);
				// Check whether it is multiple or not
				int idx = Matroska::EBMLElement::GetDescIdx(element_id);
				if (idx == -1 ||
					Matroska::EBML_element_descriptors[idx].bMultiple == 0)
					return 1;
				else
					iRet = 0;
			}
		}

		if (FindEBMLElementByElementID(ptr_child, element_id, track_id, result) == 1)
			return 1;

		ptr_child = ptr_child->next_sibling;
	}

	return iRet;
}

// @retval -1, failed to find the element
// @retval 0, find one element meet the condition 
// @retval 1, cease the recursive find
int FindEBMLElementByTrackID(Matroska::EBMLElement* ptr_element, uint32_t track_id, uint32_t element_id, std::vector<Matroska::EBMLElement*>& result)
{
	int iRet = -1;
	if (ptr_element == nullptr)
		return -1;

	Matroska::EBMLElement* ptr_child = ptr_element->first_child;

	while (ptr_child != nullptr)
	{
		if (track_id != UINT32_MAX && ptr_child->ID == 0xD7 && ((Matroska::UnsignedIntegerElement*)ptr_child)->uVal == track_id)
		{
			if (element_id == UINT32_MAX)
				result.push_back(ptr_child->container);
			else
				// From the current Track to find the element_id
				FindEBMLElementByElementID(ptr_child->container, element_id, UINT32_MAX, result);

			return 1;
		}

		if (FindEBMLElementByTrackID(ptr_child, track_id, element_id, result) == 1)
			return 1;

		ptr_child = ptr_child->next_sibling;
	}

	return iRet;
}

int DumpMKVSimpleBlock(FILE* fp, FILE* fw, Matroska::SimpleBlockElement* ptr_simple_block)
{
	if (ptr_simple_block == NULL)
		return -1;

	if (fp == NULL)
		return -1;

	if (_fseeki64(fp, ptr_simple_block->start_offset, SEEK_SET) != 0)
	{
		printf("[Matroska] Failed to seek to the position: %lld(0X%llX) at the source file.\n", ptr_simple_block->start_offset, ptr_simple_block->start_offset);
		return -1;
	}

	if (ptr_simple_block->Size > UINT32_MAX)
	{
		printf("[Matroska] The simple block element size(%llu) is too big.\n", ptr_simple_block->Size);
		return -1;
	}

	uint8_t num_frames_minus1 = 0;
	uint32_t total_frame_size = 0;
	std::vector<uint32_t> frame_sizes;
	uint32_t cbLeftSize = (uint32_t)ptr_simple_block->Size - 4;
	if (ptr_simple_block->simple_block_hdr.Lacing != 0)
	{
		if (fread(&num_frames_minus1, 1, 1, fp) == 0)
		{
			printf("[Matroska] Failed to read the field \"Number of frames in the lace-1\".\n");
			return -1;
		}

		cbLeftSize--;

		uint8_t u8Byte;
		for (uint8_t i = 0; i < num_frames_minus1; i++)
		{
			if (ptr_simple_block->simple_block_hdr.Lacing == 1)
			{
				// Xiph lacing
				uint32_t lacing_code_size = 0;
				do
				{
					if (fread(&u8Byte, 1, 1, fp) == 0)
					{
						printf("[Matroska] Failed to read lace-coded size for Xiph lacing.\n");
						return -1;
					}
					cbLeftSize--;
					lacing_code_size += u8Byte;
				} while (u8Byte == 0xFF && cbLeftSize > 0);

				frame_sizes.push_back(lacing_code_size);
			}
			else if (ptr_simple_block->simple_block_hdr.Lacing == 3)
			{
				// EBML lacing
				if (fread(&u8Byte, 1, 1, fp) == 0)
				{
					printf("[Matroska] Failed to read lace-coded size for EBML lacing.\n");
					return -1;
				}

				uint8_t idx = 0;
				for (; idx < 7; idx++)
					if ((u8Byte&(1 << (7 - idx))) == 1)
						break;

				if (idx >= 7)
				{
					printf("[Matroska] Failed to read lace-coded size for EBML lacing.\n");
					return -1;
				}

				int32_t lacing_coded_size = u8Byte & ~(1 << (7 - idx));
				if (idx > 0)
				{
					uint8_t lacing_size_byte[8];
					if (fread(lacing_size_byte, 1, idx, fp) != idx)
					{
						printf("[Matroska] Failed to read lace-coded size for EBML lacing.\n");
						return -1;
					}

					for (uint8_t j = 0; j < idx; j++)
						lacing_coded_size = (lacing_coded_size << 8) | lacing_size_byte[j];
				}

				if (frame_sizes.size() == 0)
					frame_sizes.push_back((uint32_t)lacing_coded_size);
				else
				{
					lacing_coded_size -= (1LL << (((idx + 1) << 3) - (idx + 1) - 1)) - 1;
					frame_sizes.push_back(frame_sizes.back() + lacing_coded_size);
				}

				cbLeftSize -= idx + 1;
			}
			else if (ptr_simple_block->simple_block_hdr.Lacing == 2)
			{
				frame_sizes.push_back(cbLeftSize / (num_frames_minus1 + 1));
			}

			total_frame_size += frame_sizes.back();
		}
	}

	if (cbLeftSize < total_frame_size)
	{
		printf("[Matroska] The data in lacing header is invalid.\n");
		return -1;
	}

	frame_sizes.push_back(cbLeftSize - total_frame_size);

	// For H264/HEVC, need convert the payload buffer to annex B format

	uint8_t buf[2048];
	for (uint8_t i = 0; i <= num_frames_minus1; i++)
	{
		
		do
		{
			size_t cbRead = (size_t)AMP_MIN(cbLeftSize, 2048ULL);
			if ((cbRead = fread(buf, 1, cbRead, fp)) == 0)
				break;

			if (NULL != fw)
				fwrite(buf, 1, cbRead, fw);

			cbLeftSize -= cbRead;
		} while (cbLeftSize > 0);
	}

	return 0;
}

int DumpMKVBlockGroup(FILE* fp, FILE* fw, Matroska::EBMLElement* ptr_block_group)
{
	return -1;
}

int DumpMKVOneStream(Matroska::EBMLElement* root, Matroska::EBMLElement* track, uint32_t track_id)
{
	int iRet = RET_CODE_SUCCESS;
	FILE *fp = NULL, *fw = NULL;

	if (track == nullptr || track->ID != 0xAE)
	{
		printf("No available 'trak' entry.\n");
		return iRet;
	}

	std::vector<Matroska::EBMLElement*> clusters;
	FindEBMLElementByElementID(root, 0x1F43B675, track_id, clusters);

	if (clusters.size() == 0)
	{
		printf("[Matroska] No cluster data is available for track_id#%d.\n", track_id);
		return -1;
	}

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if (g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	for (auto cluster : clusters)
	{
		Matroska::EBMLElement* ptr_child = cluster->first_child;
		while (ptr_child != nullptr)
		{
			if (ptr_child->ID == 0xA3)	// SimpleBlock
			{
				DumpMKVSimpleBlock(fp, fw, (Matroska::SimpleBlockElement*)ptr_child);
			}
			else if (ptr_child->ID == 0xA0)	// BlockGroup
			{
				DumpMKVBlockGroup(fp, fw, ptr_child);
			}

			ptr_child = ptr_child->next_sibling;
		}
	}

done:
	if (fp != NULL)
		fclose(fp);
	if (fw != NULL)
		fclose(fp);

	return iRet;
}

int DumpMKVPartial(Matroska::EBMLElement* root)
{
	return -1;
}

int DumpMKV()
{
	int iRet = -1;

	CFileBitstream bs(g_params["input"].c_str(), 4096, &iRet);

	Matroska::RootElement* root = Matroska::EBMLElement::Root();
	root->Unpack(bs);

	long long track_id = -1LL;
	if (g_params.find("trackid") != g_params.end())
	{
		track_id = ConvertToLongLong(g_params["trackid"]);
		if (track_id <= 0 || track_id > UINT32_MAX)
		{
			printf("The specified track-id is out of range.\r\n");
			return -1;
		}
	}

	long long element_id = -1LL;
	if (g_params.find("boxtype") != g_params.end())
	{
		element_id = ConvertToLongLong(g_params["boxtype"]);
		if (element_id < 0 || element_id >= UINT32_MAX)
		{
			printf("The specified boxtype(element-ID) is out of range.\n");
			return -1;
		}

		if (Matroska::EBMLElement::GetDescIdx((uint32_t)element_id) == -1)
		{
			printf("The specified boxtype(element-ID: %Xh) is not defined in Matroska spec.\n", (uint32_t)element_id);
			return -1;
		}
	}

	std::vector<Matroska::EBMLElement*> result;
	if (track_id != -1LL || element_id != -1LL)
	{
		// Try to find the track element
		FindEBMLElementByTrackID(root, (uint32_t)track_id, (uint32_t)element_id, result);

		if (result.size() == 0)
			FindEBMLElementByElementID(root, (uint32_t)element_id, (uint32_t)track_id, result);
	}

	if (g_params.find("showinfo") != g_params.end())
	{
		if (track_id == -1LL && element_id == -1LL)
			iRet = ShowMKVInfo(root, nullptr);
		else
		{
			for (auto e : result)
				iRet = ShowMKVInfo(root, e);
		}
	}

	if (g_params.find("trackid") != g_params.end() && result.size() > 0)
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0))
		{
			iRet = DumpMKVOneStream(root, result[0], (uint32_t)track_id);
		}
		else if (str_output_fmt.compare("mkv") == 0)
		{
			iRet = DumpMKVPartial(root);
		}
	}

	return iRet;
}
