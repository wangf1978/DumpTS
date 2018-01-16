#include "StdAfx.h"
#include <vector>
#include <unordered_map>
#include <map>
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include <assert.h>
#include <filesystem>

using namespace std;

extern const char *dump_msg[];
extern unordered_map<std::string, std::string> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;

using MP4_Boxes_Layout = std::vector<std::tuple<uint32_t/*box type*/, int64_t/*start pos*/, int64_t/*end_pos*/>>;

int ReadFullBoxExceptBaseBox(FILE* fp, uint8_t& version, uint32_t flags)
{
	uint8_t version_flags[4];
	if (fread_s(version_flags, sizeof(version_flags), 1, sizeof(version_flags), fp) < sizeof(version_flags))
		return -1;

	version = version_flags[0];
	flags = (version_flags[1] << 16) | (version_flags[2] << 8) | version_flags[3];

	return 0;
}

int extract_hdlr(FILE* fp)
{
	uint8_t hdlr_buf[7];
	if (fread_s(hdlr_buf, sizeof(hdlr_buf), 1, sizeof(hdlr_buf), fp) < sizeof(hdlr_buf))
		return -1;

	return 0;
}

int ListBoxes(FILE* fp, int level, int64_t start_pos, int64_t end_pos, MP4_Boxes_Layout* box_layouts=NULL, int32_t verbose=0)
{
	int iRet = 0;
	uint8_t box_header[8];
	uint8_t uuid_str[16];
	uint16_t used_size = 0;
	memset(uuid_str, 0, sizeof(uuid_str));

	if (_fseeki64(fp, start_pos, SEEK_SET) != 0)
		return 0;

	if (fread_s(box_header, sizeof(box_header), 1, sizeof(box_header), fp) < sizeof(box_header))
		return -1;

	 int64_t box_size = ENDIANULONG(*(uint32_t*)(&box_header[0]));
	uint32_t box_type = ENDIANULONG(*(uint32_t*)(&box_header[4]));
	used_size += sizeof(box_header);

	if (box_size == 1)
	{
		if (fread_s(box_header, sizeof(box_header), 1, sizeof(box_header), fp) < sizeof(box_header))
			return -1;

		box_size = ENDIANUINT64(*(uint64_t*)(&box_header[0]));
		used_size += sizeof(box_header);
	}

	if (box_type == 'uuid')
	{
		if (fread_s(uuid_str, sizeof(uuid_str), 1, sizeof(uuid_str), fp) < sizeof(uuid_str))
			return -1;

		used_size += sizeof(uuid_str);
	}

	if (box_layouts != NULL)
		box_layouts->push_back({box_type, start_pos, box_size ==0?end_pos:(start_pos + box_size)});

	// print the current box information
	if (verbose > 0)
	{
		for (int i = 0; i < level; i++)
			printf("\t");
		if (box_type == 'uuid')
			printf("uuid[%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X], size: %lld\r\n",
				uuid_str[0x0], uuid_str[0x1], uuid_str[0x2], uuid_str[0x3], uuid_str[0x4], uuid_str[0x5], uuid_str[0x6], uuid_str[0x7],
				uuid_str[0x8], uuid_str[0x9], uuid_str[0xa], uuid_str[0xb], uuid_str[0xc], uuid_str[0xd], uuid_str[0xe], uuid_str[0xf], box_size);
		else
			printf("%c%c%c%c, size: %lld\r\n", (box_type >> 24) & 0xFF, (box_type >> 16) & 0xFF, (box_type >> 8) & 0xFF, box_type & 0xFF, box_size);
	}

	uint8_t version = 0;
	uint32_t flags = 0;
	switch (box_type)
	{
	case 'moov':
	case 'trak':
	case 'mdia':
	case 'minf':
	case 'stbl':
	case 'edts':
	case 'dinf':
		if ((iRet = ListBoxes(fp, level+ 1, start_pos + used_size, box_size==0?INT64_MAX:(start_pos + box_size), box_layouts, verbose)) < 0)
			return iRet;
		break;
	case 'mvhd':
		{
			if ((iRet = ReadFullBoxExceptBaseBox(fp, version, flags)) < 0)
				return iRet;
		}
		break;
	case 'hdlr':
		{
			if ((iRet = ReadFullBoxExceptBaseBox(fp, version, flags)) < 0)
				return iRet;

			uint8_t hdlr_buf[20];
			if (fread_s(hdlr_buf, sizeof(hdlr_buf), 1, sizeof(hdlr_buf), fp) < sizeof(hdlr_buf))
				return -1;
		}
		break;
	}

	if (box_size == 0)
		return 0;

	start_pos += box_size;

	if (start_pos >= end_pos)
		return 0;

	return ListBoxes(fp, level, start_pos, end_pos, box_layouts, verbose);
}

int RefineMP4File(const std::string& src_filename, const std::string& dst_filename, std::map<uint32_t, bool>& removed_box_types)
{
	using FILE_SLICE = std::tuple<int64_t/*start_pos*/, int64_t/*end_pos*/>;

	int iRet = -1;
	uint8_t* buf = NULL;
	std::string write_pathname;
	MP4_Boxes_Layout mp4_boxes_layout;
	std::vector<FILE_SLICE> holes;
	std::vector<FILE_SLICE> merged_holes;
	std::tuple<int64_t, int64_t> prev_hole = { 0, 0 };
	std::vector<FILE_SLICE> file_copy_slices;
	// At first visit all boxes in MP4 file, and generated a table of boxes

	FILE* fp = NULL, *fw = NULL;
	errno_t errn = fopen_s(&fp, src_filename.c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", src_filename.c_str(), errn);
		goto done;
	}

	// Get file size
	_fseeki64(fp, -1, SEEK_END);
	int64_t file_size = _ftelli64(fp);
	_fseeki64(fp, 0, SEEK_SET);

	if (src_filename == dst_filename)
	{
		std::hash<std::string> hasher;
		size_t hash_val = hasher(src_filename);
		write_pathname += src_filename;
		write_pathname += "~";
		write_pathname += std::to_string(hash_val);

		if (_access(write_pathname.c_str(), 0) == 0)
		{
			printf("Can't create a temporary file path to refine the original MP4 file.\r\n");
			goto done;
		}			
	}
	else
		write_pathname = dst_filename;

	if ((iRet = ListBoxes(fp, 0, 0, file_size, &mp4_boxes_layout, 0)) == -1)
	{
		goto done;
	}

	if (mp4_boxes_layout.size() <= 0)
	{
		printf("No box can be found.\r\n");
		goto done;
	}

	// Find the holes in the original file
	for (auto& box : mp4_boxes_layout)
	{
		uint32_t box_type = std::get<0>(box);
		if (removed_box_types.find(box_type) != removed_box_types.end())
			holes.push_back({std::get<1>(box), std::get<2>(box)});
	}

	/* Merge the holes in the original file */
	for (auto& hole : holes)
	{
		if (std::get<1>(prev_hole) == 0)
		{
			prev_hole = hole;
			continue;
		}

		int64_t s1 = std::get<0>(prev_hole), e1 = std::get<1>(prev_hole);
		int64_t s2 = std::get<0>(hole), e2 = std::get<1>(hole);

		assert(s1 < s2);

		// check overlap or not
		if (s2 > e1)	// no overlap
		{
			merged_holes.push_back(prev_hole);
			prev_hole = hole;
		}
		else
		{
			// Overlapped, merge them together
			prev_hole = { s1, std::max(e1, e2) };
		}
	}

	if (std::get<1>(prev_hole) > 0)
		merged_holes.push_back(prev_hole);

	if (merged_holes.size() <= 0)
	{
		iRet = 0;
		goto done;
	}

	errn = fopen_s(&fw, write_pathname.c_str(), "wb");
	if (errn != 0 || fw == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", write_pathname.c_str(), errn);
		goto done;
	}

	// generate the copy slice
	int64_t s = 0, e = 0;
	for (auto hole : merged_holes)
	{
		if (s != std::get<0>(hole))
			file_copy_slices.push_back({ s, std::get<0>(hole) });
	
		s = std::get<1>(hole);
	}

	if (s < file_size)
		file_copy_slices.push_back({s, file_size});
	
	const size_t cache_size = 4096;
	buf = new uint8_t[cache_size];
	// begin to copy the data byte-2-byte
	for (auto slice : file_copy_slices)
	{
		_fseeki64(fp, std::get<0>(slice), SEEK_SET);
		size_t cbLeft = (size_t)(std::get<1>(slice) - std::get<0>(slice));
		size_t cbRead = 0;
		while (cbLeft > 0) {
			if ((cbRead = fread_s(buf, cache_size, 1, std::min(cache_size, cbLeft), fp)) <= 0)
			{
				iRet = -1;
				break;
			}

			if (fwrite(buf, 1, cbRead, fw) < cbRead)
			{
				iRet = -1;
				break;
			}

			cbLeft -= cbRead;
		}

		if (cbLeft > 0)
			break;
	}

	if (fp != NULL)
	{
		fclose(fp); fp = NULL;
	}

	if (fw != NULL)
	{
		fclose(fw); fw = NULL;
	}

	if (write_pathname != dst_filename)
	{
		if (_access(dst_filename.c_str(), 0) == 0)
			_unlink(dst_filename.c_str());

		if (rename(write_pathname.c_str(), dst_filename.c_str()) != 0)
		{
			printf("Failed to rename the file: %s to the file: %s.\r\n", write_pathname.c_str(), dst_filename.c_str());
			goto done;
		}
	}

	iRet = 0;

done:
	if (fp != NULL)
		fclose(fp);

	if (fw != NULL)
		fclose(fw);

	if (buf != NULL)
		delete[] buf;

	return iRet;
}

int ShowBoxInfo()
{
	int iRet = -1;
	FILE* fp = NULL;
	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\r\n", g_params["input"].c_str(), errn);
		goto done;
	}

	// Get file size
	_fseeki64(fp, -1, SEEK_END);
	int64_t file_size = _ftelli64(fp);
	_fseeki64(fp, 0, SEEK_SET);

	iRet = ListBoxes(fp, 0, 0, file_size, NULL, 1);

done:
	if (fp != NULL)
		fclose(fp);

	return iRet;
}

int DumpMP4()
{
	int iRet = -1;

	if (g_params.find("showinfo") != g_params.end())
	{
		iRet = ShowBoxInfo();
	}

	if (g_params.find("removebox") != g_params.end())
	{
		std::map<uint32_t, bool> removed_box_types;
		std::string& str_remove_boxtypes = g_params["removebox"];

		if (str_remove_boxtypes.length() == 0 ||
			str_remove_boxtypes.compare("''") == 0 ||
			str_remove_boxtypes.compare("\"\"") == 0)
		{
			removed_box_types[0] = true;
		}
		else
		{
			if (str_remove_boxtypes.length() < 4)
			{
				printf("The 'removebox' parameter is invalid.\r\n");
				return -1;
			}

			if (str_remove_boxtypes[0] == '\'')
			{
				// Remove the prefix and suffix of ' or "
				if (str_remove_boxtypes[str_remove_boxtypes.length() - 1] == '\'')
					str_remove_boxtypes = str_remove_boxtypes.substr(1, str_remove_boxtypes.length() - 2);
			}
			else if (str_remove_boxtypes[0] == '"')
			{
				// Remove the prefix and suffix of ' or "
				if (str_remove_boxtypes[str_remove_boxtypes.length() - 1] == '"')
					str_remove_boxtypes = str_remove_boxtypes.substr(1, str_remove_boxtypes.length() - 2);
			}

			uint32_t box_type = 0;
			uint32_t passed_char_count = 0;

			// parse the str_remove_boxtypes, and split into four byte
			for (size_t i = 0; i < str_remove_boxtypes.length(); i++)
			{
				if (passed_char_count == 4)
				{
					removed_box_types[box_type] = true;
					passed_char_count = 0;
				}

				if (passed_char_count == 0 && i > 0)
				{
					if (str_remove_boxtypes[i] == ',')
						continue;
				}

				box_type = (box_type << 8) | str_remove_boxtypes[i];
				passed_char_count++;
			}

			if (passed_char_count == 4)
				removed_box_types[box_type] = true;
		}

		if ((iRet = RefineMP4File(g_params["input"], g_params.find("output") == g_params.end() ? g_params["input"] : g_params["output"], removed_box_types), g_verbose_level) < 0)
			return iRet;
	}

	return iRet;
}
