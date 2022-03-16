/*

MIT License

Copyright (c) 2021 Ravin.Wang(wangf1978@hotmail.com)

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
#include <vector>
#include <unordered_map>
#include <map>
#include "Matroska.h"
#include "DumpTS.h"
#include "PayloadBuf.h"
#include "ESRepacker.h"

using namespace std;

extern const char *dump_msg[];
extern map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;

void PrintTree(BST::Matroska::EBMLElement* ptr_element, int level)
{
	if (ptr_element == nullptr || level < 0)
		return;

	size_t line_chars = (size_t)level * 5 + 160;
	char* szLine = new char[line_chars];
	memset(szLine, ' ', line_chars);

	const int indent = 2;
	const int level_span = 5;

	char* szText = nullptr;
	if (level >= 1)
	{
		BST::Matroska::EBMLElement* ptr_parent = ptr_element->container;
		memcpy(szLine + indent + ((ptrdiff_t)level - 1)*level_span, "|--", 3);
		for (int i = level - 2; i >= 0 && ptr_parent != nullptr; i--)
		{
			if (ptr_parent->next_sibling != nullptr)
				memcpy(szLine + indent + (ptrdiff_t)i*level_span, "|", 1);
			ptr_parent = ptr_parent->container;
		}
		szText = szLine + indent + 3 + ((ptrdiff_t)level - 1)*level_span;
	}
	else
		szText = szLine + indent;

	int32_t desc_idx = ptr_element->GetDescIdx(ptr_element->ID);

	if (ptr_element->container == nullptr)
		sprintf_s(szText, line_chars - (szText - szLine), ".\n");
	else if (desc_idx < 0 || desc_idx >= (int32_t)_countof(BST::Matroska::EBML_element_descriptors))
		sprintf_s(szText, line_chars - (szText - szLine), "Unknown Element(0X%X), size: %" PRIu64 "\n", ptr_element->ID, ptr_element->Size);
	else
	{
		int cbWritten = 0;
		if (BST::Matroska::EBML_element_descriptors[desc_idx].data_type == BST::Matroska::EBML_DT_MASTER)
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s (Size: %" PRIu64 ")",
				BST::Matroska::EBML_element_descriptors[desc_idx].Element_Name, ptr_element->Size);
		else if (BST::Matroska::EBML_element_descriptors[desc_idx].data_type == BST::Matroska::EBML_DT_BINARY && ptr_element->ID == 0xA3)
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s (Size:%8" PRIu64 "): ",
				BST::Matroska::EBML_element_descriptors[desc_idx].Element_Name, ptr_element->Size);
		else
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s (Size: %" PRIu64 "): ",
				BST::Matroska::EBML_element_descriptors[desc_idx].Element_Name, ptr_element->Size);

		if (cbWritten > 0)
			szText += cbWritten;

		cbWritten = -1;
		switch (BST::Matroska::EBML_element_descriptors[desc_idx].data_type)
		{
		case BST::Matroska::EBML_DT_UNSIGNED_INTEGER:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%" PRIu64 "", ((BST::Matroska::UnsignedIntegerElement*)ptr_element)->uVal);
			break;
		case BST::Matroska::EBML_DT_SIGNED_INTEGER:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%" PRIi64 "", ((BST::Matroska::SignedIntegerElement*)ptr_element)->iVal);
			break;
		case BST::Matroska::EBML_DT_DATE:
			{
				struct tm tmVal;
				if (localtime_s(&tmVal, &((BST::Matroska::DateElement*)ptr_element)->timeVal) == 0)
					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%d-%02d-%02d %02dh:%02dh:%02d", 
						tmVal.tm_year + 1900, tmVal.tm_mon + 1, tmVal.tm_mday, tmVal.tm_hour, tmVal.tm_min, tmVal.tm_sec);
				else
					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "Unknown date-time");
			}
			break;
		case BST::Matroska::EBML_DT_ASCII_STRING:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", ((BST::Matroska::ASCIIStringElement*)ptr_element)->szVal);
			break;
		case BST::Matroska::EBML_DT_UTF8_STRING:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%s", ((BST::Matroska::UTF8StringElement*)ptr_element)->szUTF8);
			break;
		case BST::Matroska::EBML_DT_FLOAT:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%f", ((BST::Matroska::FloatElement*)ptr_element)->fVal);
			break;
		case BST::Matroska::EBML_DT_BINARY:
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "Binary");
			break;
		}

		if (cbWritten > 0)
			szText += cbWritten;

		sprintf_s(szText, line_chars - (szText - szLine), "\n");
	}

	printf("%s", szLine);

	delete[] szLine;

	auto ptr_child = ptr_element->first_child;
	while (ptr_child != nullptr)
	{
		PrintTree(ptr_child, level + 1);
		ptr_child = ptr_child->next_sibling;
	}

	return;
}

int ShowMKVInfo(BST::Matroska::EBMLElement* root, BST::Matroska::EBMLElement* ptr_element)
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

int DumpMKVOneStream(BST::Matroska::EBMLElement* root, BST::Matroska::EBMLElement* track, uint32_t track_id)
{
	int iRet = RET_CODE_SUCCESS;
	FILE *fp = NULL, *fw = NULL;
	void* codec_priv_obj = nullptr;
	std::vector<BST::Matroska::EBMLElement*> results;
	const char* szCodecID = nullptr;
	CESRepacker* pESRepacker = nullptr;
	SEEK_POINT_INFO curr_seek_point_info;

	if (track == nullptr || track->ID != 0xAE)
	{
		printf("No available 'trak' entry.\n");
		return iRet;
	}

	std::vector<BST::Matroska::EBMLElement*> clusters;
	root->FindEBMLElementByElementID(MATROSKA_CLUSTER_ID, clusters);

	if (clusters.size() == 0)
	{
		printf("[Matroska] No cluster data is available for track_id#%d.\n", track_id);
		return -1;
	}

	track->FindEBMLElementByElementID(0x86, results);

	if (results.size() > 0)
	{
		BST::Matroska::ASCIIStringElement* codecIDElement = (BST::Matroska::ASCIIStringElement*)results[0];
		szCodecID = codecIDElement->szVal;
	}

	CODEC_ID codec_id = CODEC_ID_UNKNOWN;
	if (szCodecID != nullptr && _stricmp(szCodecID, "V_MPEG4/ISO/AVC") == 0)
	{
		codec_id = CODEC_ID_V_MPEG4_AVC;
		pESRepacker = new CNALRepacker(ES_BYTE_STREAM_ISO_NALAU_SAMPLE, ES_BYTE_STREAM_AVC_ANNEXB);
	}
	else if (szCodecID != nullptr && _stricmp(szCodecID, "V_MPEGH/ISO/HEVC") == 0)
	{
		codec_id = CODEC_ID_V_MPEGH_HEVC;
		pESRepacker = new CNALRepacker(ES_BYTE_STREAM_ISO_NALAU_SAMPLE, ES_BYTE_STREAM_HEVC_ANNEXB);
	}
	else if (szCodecID != nullptr && _stricmp(szCodecID, "V_AV1") == 0)
	{
		codec_id = CODEC_ID_V_AV1;
		pESRepacker = new CESRepacker(ES_BYTE_STREAM_RAW, ES_BYTE_STREAM_RAW);
	}
	else
		pESRepacker = new CESRepacker(ES_BYTE_STREAM_RAW, ES_BYTE_STREAM_RAW);

	ES_REPACK_CONFIG es_repack_config;
	es_repack_config.codec_id = codec_id;
	if (g_params.find("output") != g_params.end() && g_params["output"].length() > 0)
		strcpy_s(es_repack_config.es_output_file_path, MAX_PATH, g_params["output"].c_str());
	else
		memset(es_repack_config.es_output_file_path, 0, sizeof(es_repack_config.es_output_file_path));

	// Generate the codec_priv_obj
	results.clear();
	track->FindEBMLElementByElementID(0x63A2, results);
	if (results.size() > 0)
	{
		auto pCodecPrivElement = (BST::Matroska::CodecPrivateElement*)results[0];
		if (codec_id == CODEC_ID_V_MPEG4_AVC)
		{
			codec_priv_obj = new BST::ISOBMFF::AVCDecoderConfigurationRecord();
			pCodecPrivElement->UnpacksAsAVC((BST::ISOBMFF::AVCDecoderConfigurationRecord*)codec_priv_obj);
		}
		else if (codec_id == CODEC_ID_V_MPEGH_HEVC)
		{
			codec_priv_obj = new BST::ISOBMFF::HEVCDecoderConfigurationRecord();
			pCodecPrivElement->UnpackAsHEVC((BST::ISOBMFF::HEVCDecoderConfigurationRecord*)codec_priv_obj);
		}
		else
			codec_priv_obj = (void*)pCodecPrivElement;
	}

	es_repack_config.pCodecPrivObj = codec_priv_obj;

	if((iRet = pESRepacker->Config(es_repack_config)) < 0)
	{
		printf("[Matroska] Failed to config the ES re-packer {retcode: %d}.\n", iRet);
		goto done;
	}

	if((iRet = pESRepacker->Open(g_params["input"].c_str())) < 0)
	{
		printf("[Matroska] Failed to open the ES re-packer with the file: %s {retcode: %d}.\n", g_params["input"].c_str(), iRet);
		goto done;
	}

	for (auto cluster : clusters)
	{
		BST::Matroska::ClusterElement* ptr_cluster = (BST::Matroska::ClusterElement*)cluster;

		if (ptr_cluster->TrackBlockMaps.find(track_id) == ptr_cluster->TrackBlockMaps.end())
			continue;

		auto TrackBlockMap = ptr_cluster->TrackBlockMaps[track_id];

		for (size_t i = 0; i < TrackBlockMap->Block_fines.size(); i++)
		{
			if (pESRepacker->Seek(TrackBlockMap->GetBytePos(i), TrackBlockMap->Block_fines[i].is_simple_block ? ES_SEEK_MATROSKA_SIMPLE_BLOCK : ES_SEEK_MATROSKA_BLOCK_GROUP) < 0)
			{
				printf("[Matroska] Failed to seek to the %s#%" PRIu32 " in the cluster %p \n", 
					TrackBlockMap->Block_fines[i].is_simple_block ? "SimpleBlock":"BlockGroup", (uint32_t)i, cluster);
				iRet = RET_CODE_ERROR;
				goto done;
			}

			if (pESRepacker->GetSeekPointInfo(curr_seek_point_info) < 0)
			{
				printf("[Matroska] Failed to get the seek point information.\n");
				iRet = RET_CODE_ERROR;
				goto done;
			}

			for (size_t idxFrame = 0; idxFrame < curr_seek_point_info.frame_sizes.size(); idxFrame++)
			{
				if (pESRepacker->Repack(curr_seek_point_info.frame_sizes[idxFrame], (idxFrame == 0 && curr_seek_point_info.seek_point_type == ES_SEEK_MATROSKA_SIMPLE_BLOCK)
					? curr_seek_point_info.key_frame : FLAG_UNKNOWN) < 0)
				{
					printf("[Matroska] Failed to repack the frame data to target file.\n");
				}
			}
		}
	}

done:
	if (fp != NULL)
		fclose(fp);
	if (fw != NULL)
		fclose(fp);

	if (codec_priv_obj != nullptr)
	{
		if (codec_id == CODEC_ID_V_MPEG4_AVC)
			delete (BST::ISOBMFF::AVCDecoderConfigurationRecord*)codec_priv_obj;
		else if (codec_id == CODEC_ID_V_MPEGH_HEVC)
			delete (BST::ISOBMFF::HEVCDecoderConfigurationRecord*)codec_priv_obj;
	}

	if (pESRepacker != nullptr)
	{
		pESRepacker->Close();
		delete pESRepacker;
		pESRepacker = nullptr;
	}

	return iRet;
}

int DumpMKVPartial(BST::Matroska::EBMLElement* root)
{
	return -1;
}

int DumpMKV()
{
	int iRet = -1;

	CFileBitstream bs(g_params["input"].c_str(), 4096, &iRet);

	BST::Matroska::RootElement* root = BST::Matroska::EBMLElement::Root();
	root->Unpack(bs);

	long long track_id = -1LL;
	if (g_params.find("trackid") != g_params.end())
	{
		track_id = ConvertToLongLong(g_params["trackid"]);
		if (track_id <= 0 || track_id > UINT32_MAX)
		{
			printf("The specified track-id is out of range.\n");
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

		if (BST::Matroska::EBMLElement::GetDescIdx((uint32_t)element_id) == -1)
		{
			printf("The specified boxtype(element-ID: %Xh) is not defined in Matroska spec.\n", (uint32_t)element_id);
			return -1;
		}
	}

	std::vector<BST::Matroska::EBMLElement*> result;
	if (track_id != -1LL || element_id != -1LL)
	{
		// Try to find the track element
		root->FindEBMLElementByTrackID((uint32_t)track_id, (uint32_t)element_id, result);

		if (result.size() == 0)
			root->FindEBMLElementByElementID((uint32_t)element_id, result);
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
