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
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include <assert.h>
#include <functional>
#include "ISO14496_12.h"
#include "ISO14496_15.h"
#include "NAL.h"
#include "AMRFC3986.h"

using namespace std;
using namespace BST::ISOBMFF;

extern const char *dump_msg[];
extern map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;
extern const char* hevc_nal_unit_type_names[64];

using MP4_Boxes_Layout = std::vector<std::tuple<uint32_t/*box type*/, int64_t/*start pos*/, int64_t/*end_pos*/>>;

int ReadFullBoxExceptBaseBox(FILE* fp, uint8_t& version, uint32_t flags)
{
	uint8_t version_flags[4];
	if (fread_s(version_flags, sizeof(version_flags), 1U, sizeof(version_flags), fp) < sizeof(version_flags))
		return -1;

	version = version_flags[0];
	flags = (version_flags[1] << 16) | (version_flags[2] << 8) | version_flags[3];

	return 0;
}

int extract_hdlr(FILE* fp)
{
	uint8_t hdlr_buf[7];
	if (fread_s(hdlr_buf, sizeof(hdlr_buf), 1U, sizeof(hdlr_buf), fp) < sizeof(hdlr_buf))
		return -1;

	return 0;
}

int CopyBoxTypeName(char* szText, int ccSize, uint32_t box_type)
{
	if (szText == NULL)
		return -1;

	bool bNeedHex = false;
	int ccLeft = 4, ccSizeLeft = ccSize;
	while (ccSize > 0 && ccLeft > 0)
	{
		int c = (int)(box_type >> ((ccLeft - 1) << 3)) & 0xFF;

		if (!bNeedHex && !isprint(c))
			bNeedHex = true;

		*szText = isprint(c) ? (char)c : ' ';
		szText++;
		ccSizeLeft--;
		ccLeft--;
	}

	if (bNeedHex)
	{
		int cbWritten = sprintf_s(szText, ccSizeLeft, "(%08Xh)", box_type);
		if (cbWritten > 0)
		{
			szText += cbWritten;
			ccSizeLeft -= cbWritten;
		}
	}

	if (ccSizeLeft > 0)
		*szText = '\0';

	return ccSize - ccSizeLeft;
}

/*
using BoxSlice =  tuple<Box, header_part, box_part_offset, box_part_size>;
using BoxSlices = std::vector<BoxSlice>;
*/
using BoxSlice = tuple<Box*, bool, uint64_t, uint64_t>;
using BoxSlices = std::vector<BoxSlice>;

int FlushBoxSlice(BoxSlice& box_slice, FILE* fp, FILE* fw, uint64_t nMDATShift)
{
	uint8_t buf[2048];
	Box* ptr_cur_box = std::get<0>(box_slice);
	if (ptr_cur_box == nullptr)
		return RET_CODE_NOTHING_TODO;

	bool header_part = std::get<1>(box_slice);
	uint64_t box_part_offset = std::get<2>(box_slice);
	uint64_t box_part_size = std::get<3>(box_slice);

	assert(ptr_cur_box->start_bitpos % 8 == 0);
	assert(header_part == true || (ptr_cur_box->start_bitpos >> 3) < box_part_offset);

	uint64_t file_offset = header_part ? (ptr_cur_box->start_bitpos >> 3) : box_part_offset;
	uint32_t box_size = ptr_cur_box->largesize_used ? 1U : (uint32_t)ptr_cur_box->size;
	uint32_t box_type = ptr_cur_box->type;
	uint64_t box_large_size = ptr_cur_box->largesize_used ? ptr_cur_box->size : 0;

	if (g_verbose_level > 0)
	{
		printf("File offset: %" PRIu64 "(0X%" PRIX64 "), Header Part: %s, Part size: %" PRIu64 "\n", file_offset, file_offset, header_part?"Yes":"No", box_part_size);
		printf("    box-type: %c%c%c%c(0X%08X), box_size: %" PRIu32 ", large_box_size: %" PRIu64 "\n",
			isprint((box_type >> 24) & 0xFF) ? ((box_type >> 24) & 0xFF) : '.',
			isprint((box_type >> 16) & 0xFF) ? ((box_type >> 16) & 0xFF) : '.',
			isprint((box_type >>  8) & 0xFF) ? ((box_type >>  8) & 0xFF) : '.',
			isprint((box_type) & 0xFF) ? ((box_type) & 0xFF) : '.', box_type, box_size, box_large_size);
	}

	if (file_offset >= INT64_MAX)
		return RET_CODE_INVALID_PARAMETER;

	if (box_part_size == 0)
	{
		// If the box slice size is set 0 for the header part, it means the whole box will be written
		if (header_part == true)
			box_part_size = ptr_cur_box->size == 0 ? UINT64_MAX : ptr_cur_box->size;
		else
			return RET_CODE_INVALID_PARAMETER;
	}

	if (header_part)
	{
		buf[0] = (box_size >> 24) & 0xFF;
		buf[1] = (box_size >> 16) & 0xFF;
		buf[2] = (box_size >>  8) & 0xFF;
		buf[3] = (box_size) & 0xFF;
		buf[4] = (box_type >> 24) & 0xFF;
		buf[5] = (box_type >> 16) & 0xFF;
		buf[6] = (box_type >>  8) & 0xFF;
		buf[7] = (box_type) & 0xFF;
		if (fwrite(buf, 1, 8, fw) != 8)
			return RET_CODE_ERROR;

		if (box_size == 1)
		{
			buf[0] = (uint8_t)((box_large_size >> 56) & 0xFF);
			buf[1] = (uint8_t)((box_large_size >> 48) & 0xFF);
			buf[2] = (uint8_t)((box_large_size >> 40) & 0xFF);
			buf[3] = (uint8_t)((box_large_size >> 32) & 0xFF);
			buf[4] = (uint8_t)((box_large_size >> 24) & 0xFF);
			buf[5] = (uint8_t)((box_large_size >> 16) & 0xFF);
			buf[6] = (uint8_t)((box_large_size >>  8) & 0xFF);
			buf[7] = (uint8_t)(box_large_size & 0xFF);
			if (fwrite(buf, 1, 8, fw) != 8)
				return RET_CODE_ERROR;
		}

		uint8_t box_header_size = (uint8_t)(box_size == 1 ? 16 : 8);
		if (box_part_size < box_header_size)
			return RET_CODE_INVALID_PARAMETER;

		box_part_size -= box_header_size;
		file_offset += box_header_size;
	}

	// Only box header exist
	if (box_part_size == 0)
		return RET_CODE_SUCCESS;

	if (_fseeki64(fp, (int64_t)file_offset, SEEK_SET) != 0)
		return RET_CODE_ERROR;

	if (nMDATShift > 0 && (box_type == 'stco' || box_type == 'co64'))
	{
		size_t actual_read_size = 0UL;
		size_t read_size = 8;
		if ((actual_read_size = fread(buf, 1, read_size, fp)) != read_size)
		{
			if (!feof(fp))
				return RET_CODE_ERROR;
			else
				box_part_size = 0;	// exit the loop from the next round
		}

		if (fwrite(buf, 1, actual_read_size, fw) != actual_read_size)
			return RET_CODE_ERROR;

		if (box_part_size >= actual_read_size)
			box_part_size -= actual_read_size;
		else if (box_part_size != 0)
			return RET_CODE_ERROR;	// Should never happen

		uint32_t entry_size = box_type == 'stco' ? 4 : 8;
		uint32_t entry_count = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[5] << 16) | ((uint32_t)buf[6] << 8) | (uint32_t)buf[7];
		uint32_t left_entry_count = (uint32_t)(box_part_size / entry_size);

		entry_count = AMP_MIN(entry_count, left_entry_count);

		if (entry_count > 0)
		{
			uint8_t *chunk_offsets = new uint8_t[(size_t)entry_count*entry_size];
			if ((actual_read_size = fread(chunk_offsets, 1, (size_t)entry_count*entry_size, fp)) != (size_t)entry_count*entry_size)
			{
				delete[] chunk_offsets;
				printf("[MP4] Failed to read %" PRIu32 " bytes for 'stco' and 'co64' box.\n", entry_count*entry_size);
				return RET_CODE_ERROR;
			}

			for (uint32_t i = 0; i < entry_count; i++)
			{
				if (box_type == 'stco')
				{
					uint32_t chunk_offset = ((uint32_t)chunk_offsets[i*entry_size] << 24) | ((uint32_t)chunk_offsets[i*entry_size + 1] << 16) |
						((uint32_t)chunk_offsets[i*entry_size + 2] << 8) | (uint32_t)chunk_offsets[i*entry_size + 3];
					if ((uint64_t)chunk_offset < nMDATShift)
					{
						printf("[MP4] the original chunk offset: %" PRIu32 " seems not to be correct.\n", chunk_offset);
						chunk_offset = 0;
					}
					else
						chunk_offset -= (uint32_t)nMDATShift;

					chunk_offsets[i*entry_size] = (chunk_offset >> 24) & 0xFF;
					chunk_offsets[i*entry_size + 1] = (chunk_offset >> 16) & 0xFF;
					chunk_offsets[i*entry_size + 2] = (chunk_offset >>  8) & 0xFF;
					chunk_offsets[i*entry_size + 3] = (chunk_offset) & 0xFF;
				}
				else if (box_type == 'co64')
				{
					uint64_t chunk_offset = ((uint64_t)chunk_offsets[i*entry_size] << 56) | 
						((uint64_t)chunk_offsets[i*entry_size + 1] << 48) |
						((uint64_t)chunk_offsets[i*entry_size + 2] << 40) | 
						((uint64_t)chunk_offsets[i*entry_size + 3] << 32) |
						((uint64_t)chunk_offsets[i*entry_size + 4] << 24) |
						((uint64_t)chunk_offsets[i*entry_size + 5] << 16) |
						((uint64_t)chunk_offsets[i*entry_size + 6] <<  8) |
						((uint32_t)chunk_offsets[i*entry_size + 7]);
					if (chunk_offset < nMDATShift)
					{
						chunk_offset = 0;
						printf("[MP4] the original chunk offset: %" PRIu64 " seems not to be correct.\n", chunk_offset);
					}
					else
						chunk_offset -= nMDATShift;

					chunk_offsets[i*entry_size] = (chunk_offset >> 56) & 0xFF;
					chunk_offsets[i*entry_size + 1] = (chunk_offset >> 48) & 0xFF;
					chunk_offsets[i*entry_size + 2] = (chunk_offset >> 40) & 0xFF;
					chunk_offsets[i*entry_size + 3] = (chunk_offset >> 32) & 0xFF;
					chunk_offsets[i*entry_size + 4] = (chunk_offset >> 24) & 0xFF;
					chunk_offsets[i*entry_size + 5] = (chunk_offset >> 16) & 0xFF;
					chunk_offsets[i*entry_size + 6] = (chunk_offset >>  8) & 0xFF;
					chunk_offsets[i*entry_size + 7] = (chunk_offset) & 0xFF;
				}
			}

			if (fwrite(chunk_offsets, 1, (size_t)entry_count*entry_size, fw) != (size_t)entry_count*entry_size)
			{
				delete[] chunk_offsets;
				return RET_CODE_ERROR;
			}

			delete[] chunk_offsets;
		}

		box_part_size = (left_entry_count > entry_count) ? (left_entry_count - entry_count)*entry_size : 0;
	}

	while (box_part_size > 0)
	{
		size_t actual_read_size = 0UL;
		size_t read_size = (size_t)AMP_MIN(box_part_size, 2048);
		if ((actual_read_size = fread(buf, 1, read_size, fp)) != read_size)
		{
			if (!feof(fp))
				return RET_CODE_ERROR;
			else
				box_part_size = 0;	// exit the loop from the next round
		}

		if (fwrite(buf, 1, actual_read_size, fw) != actual_read_size)
			return RET_CODE_ERROR;

		if (box_part_size >= actual_read_size)
			box_part_size -= actual_read_size;
		else if (box_part_size != 0)
			return RET_CODE_ERROR;	// Should never happen
	}

	return RET_CODE_SUCCESS;
}

/*!	@brief Fill the box slices
	@retval 0 No slice is filled
	@retval 1 Available slice(s) are filled
	@retval 2 Available slice(s) are filled, and there are the son children to be a removed box
*/
int FillBoxSlices(Box* cur_box, BoxSlices& box_slices, const std::set<uint32_t>& remove_box_types)
{
	if (cur_box == nullptr)
		return 0;

	int iRet = 1;
	bool bHeaderPart = true;
	uint64_t src_start_pos = cur_box->start_bitpos >> 3;
	uint64_t real_box_size = cur_box->size == 0 ? UINT64_MAX : cur_box->size;

	Box* child = cur_box->first_child;
	if (child == nullptr)
	{
		// No child, copy this box directly
		box_slices.push_back({ cur_box, bHeaderPart, cur_box->start_bitpos>>3, real_box_size });
		return 1;
	}

	uint64_t last_box_end_pos = src_start_pos;
	for (; child != nullptr; child = child->next_sibling)
	{
		if (remove_box_types.find(child->type) != remove_box_types.end())
		{
			// Decrease its parents' size by its own size
			Box* pParent = child->container;
			while (pParent != nullptr)
			{
				if (pParent->container == nullptr)
					break;

				if (pParent->size > child->size)
					pParent->size -= child->size;

				pParent = pParent->container;
			}

			// This box is to be removed
			if ((child->start_bitpos >> 3) > last_box_end_pos)
			{
				box_slices.push_back({ cur_box, bHeaderPart, last_box_end_pos, (child->start_bitpos >> 3) - last_box_end_pos });
				bHeaderPart = false;
			}

			last_box_end_pos = child->size == 0 ? UINT64_MAX : ((child->start_bitpos >> 3) + child->size);
			iRet = 2;

			if (child->size == 0)	// The removed box extend to the file end
				break;

			continue;
		}

		if (last_box_end_pos != UINT64_MAX && last_box_end_pos < (child->start_bitpos >> 3))
		{
			box_slices.push_back({ cur_box, bHeaderPart, last_box_end_pos, (child->start_bitpos >> 3) - last_box_end_pos });
			last_box_end_pos = (child->start_bitpos >> 3);
			bHeaderPart = false;
		}

		last_box_end_pos = child->size == 0 ? UINT64_MAX : ((child->start_bitpos >> 3) + child->size);

		// child->size may be changed in this procedure
		if (FillBoxSlices(child, box_slices, remove_box_types) == 2)
			iRet = 2;

		if (child->size == 0)
			break;
	}
	
	if (last_box_end_pos != UINT64_MAX)
	{
		if (cur_box->size == 0)
			box_slices.push_back({ cur_box, bHeaderPart, last_box_end_pos, UINT64_MAX });
		else if (last_box_end_pos < src_start_pos + cur_box->size)
			box_slices.push_back({ cur_box, bHeaderPart, last_box_end_pos, (src_start_pos + cur_box->size) - last_box_end_pos });
	}

	return iRet;
}

int ListBoxes(FILE* fp, int level, int64_t start_pos, int64_t end_pos, MP4_Boxes_Layout* box_layouts=NULL, int32_t verbose=0)
{
	int iRet = 0;
	uint8_t box_header[8];
	uint8_t uuid_str[16];
	uint16_t used_size = 0;
	memset(uuid_str, 0, sizeof(uuid_str));

	if (_fseeki64(fp, start_pos, SEEK_SET) != 0 || feof(fp) || start_pos >= end_pos)
		return 0;

	if (fread_s(box_header, sizeof(box_header), 1U, sizeof(box_header), fp) < sizeof(box_header))
		return -1;

	 int64_t box_size = ENDIANULONG(*(uint32_t*)(&box_header[0]));
	uint32_t box_type = ENDIANULONG(*(uint32_t*)(&box_header[4]));
	used_size += sizeof(box_header);

	if (box_size == 1)
	{
		if (fread_s(box_header, sizeof(box_header), 1U, sizeof(box_header), fp) < sizeof(box_header))
			return -1;

		box_size = ENDIANUINT64(*(uint64_t*)(&box_header[0]));
		used_size += sizeof(box_header);
	}

	if (box_type == 'uuid')
	{
		if (fread_s(uuid_str, sizeof(uuid_str), 1U, sizeof(uuid_str), fp) < sizeof(uuid_str))
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
			printf("uuid[%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X], size: %" PRId64 "\n",
				uuid_str[0x0], uuid_str[0x1], uuid_str[0x2], uuid_str[0x3], uuid_str[0x4], uuid_str[0x5], uuid_str[0x6], uuid_str[0x7],
				uuid_str[0x8], uuid_str[0x9], uuid_str[0xa], uuid_str[0xb], uuid_str[0xc], uuid_str[0xd], uuid_str[0xe], uuid_str[0xf], box_size);
		else
			printf("%c%c%c%c, size: %" PRId64 "\n", (box_type >> 24) & 0xFF, (box_type >> 16) & 0xFF, (box_type >> 8) & 0xFF, box_type & 0xFF, box_size);
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
			if (fread_s(hdlr_buf, sizeof(hdlr_buf), 1U, sizeof(hdlr_buf), fp) < sizeof(hdlr_buf))
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

int RemoveBoxes(FILE* fp, FILE* fw, std::set<uint32_t>& removed_box_types)
{
	using FILE_SLICE = std::tuple<int64_t/*start_pos*/, int64_t/*end_pos*/>;

	int iRet = RET_CODE_ERROR;
	uint8_t* buf = NULL;
	size_t cache_size = 4096;
	int64_t s = 0, /*e = 0, */file_size;

	// Get file size
	_fseeki64(fp, 0, SEEK_END);
	file_size = _ftelli64(fp);
	_fseeki64(fp, 0, SEEK_SET);

	MP4_Boxes_Layout mp4_boxes_layout;
	std::vector<FILE_SLICE> holes;
	std::vector<FILE_SLICE> merged_holes;
	std::tuple<int64_t, int64_t> prev_hole = { 0, 0 };
	std::vector<FILE_SLICE> file_copy_slices;

	// At first visit all boxes in MP4 file, and generated a table of boxes
	if ((iRet = ListBoxes(fp, 0, 0, file_size, &mp4_boxes_layout, 0)) == -1)
	{
		goto done;
	}

	if (mp4_boxes_layout.size() <= 0)
	{
		printf("No box can be found.\n");
		goto done;
	}

	// Find the holes in the original file
	for (auto& box : mp4_boxes_layout)
	{
		uint32_t box_type = std::get<0>(box);
		if (removed_box_types.find(box_type) != removed_box_types.end())
			holes.push_back({ std::get<1>(box), std::get<2>(box) });
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
			prev_hole = { s1, AMP_MAX(e1, e2) };
		}
	}

	if (std::get<1>(prev_hole) > 0)
		merged_holes.push_back(prev_hole);

	if (merged_holes.size() <= 0)
	{
		iRet = 0;
		goto done;
	}

	// generate the copy slice
	for (auto hole : merged_holes)
	{
		if (s != std::get<0>(hole))
			file_copy_slices.push_back({ s, std::get<0>(hole) });

		s = std::get<1>(hole);
	}

	if (s < file_size)
		file_copy_slices.push_back({ s, file_size });

	buf = new uint8_t[cache_size];
	// begin to copy the data byte-2-byte
	for (auto slice : file_copy_slices)
	{
		_fseeki64(fp, std::get<0>(slice), SEEK_SET);
		size_t cbLeft = (size_t)(std::get<1>(slice) - std::get<0>(slice));
		size_t cbRead = 0;
		while (cbLeft > 0) {
			if ((cbRead = fread_s(buf, cache_size, 1U, AMP_MIN(cache_size, cbLeft), fp)) <= 0)
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

	iRet = RET_CODE_SUCCESS;

done:
	if (buf != NULL)
		delete[] buf;

	return iRet;
}

int RefineMP4File(Box* root_box, const std::string& src_filename, const std::string& dst_filename, std::set<uint32_t>& removed_box_types)
{
	int iRet = -1;
	errno_t errn;
	std::string write_pathname;
	FILE *fp = nullptr, *fw = nullptr;
	if (src_filename == dst_filename)
	{
		std::hash<std::string> hasher;
		size_t hash_val = hasher(src_filename);
		write_pathname += src_filename;
		write_pathname += "_";
		write_pathname += std::to_string(hash_val);

		if (_access(write_pathname.c_str(), F_OK) == 0)
		{
			// Try to find another temporary swap file
			unsigned int itries = 0;
			while (itries < INT8_MAX)
			{
				std::string new_write_pathnmae = write_pathname + "_" + std::to_string(itries);
				if (_access(new_write_pathnmae.c_str(), F_OK) != 0)
				{
					write_pathname = new_write_pathnmae;
					break;
				}

				itries++;
			}
			
			if (itries >= INT8_MAX)
			{
				printf("Can't find a temporary file to refine the original MP4 file.\n");
				goto done;
			}
		}			
	}
	else
		write_pathname = dst_filename;

	errn = fopen_s(&fp, src_filename.c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", src_filename.c_str(), errn);
		goto done;
	}

	errn = fopen_s(&fw, write_pathname.c_str(), "wb");
	if (errn != 0 || fw == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", write_pathname.c_str(), errn);
		goto done;
	}

	if (root_box != nullptr)
	{
		BoxSlices box_slices;
		bool bBoxRemoved = false;
		uint64_t nMDATChunkShiftAhead = 0;
		Box* pChild = root_box->first_child;
		while (pChild != nullptr)
		{
			if (removed_box_types.find(pChild->type) != removed_box_types.end())
			{
				bBoxRemoved = true;
				pChild = pChild->next_sibling;
				continue;
			}
			
			if (FillBoxSlices(pChild, box_slices, removed_box_types) == 2)
				bBoxRemoved = true;

			pChild = pChild->next_sibling;
		}

		// Find 'mdat' chunk is shifted or not
		uint64_t nPostMDATChunkOffset = 0;
		for (auto& slice : box_slices)
		{
			Box* ptr_cur_box = std::get<0>(slice);
			if (ptr_cur_box == nullptr)
				continue;

			bool header_part = std::get<1>(slice);
			uint64_t box_part_offset = std::get<2>(slice);
			uint64_t box_part_size = std::get<3>(slice);

			if (g_verbose_level >= 99)
			{
				printf("[MP4] '%c%c%c%c', header_part: %d, offset: %" PRIu64 ", size: %" PRIu64 ".\n",
					(ptr_cur_box->type >> 24) & 0xFF, (ptr_cur_box->type >> 16) & 0xFF, (ptr_cur_box->type >> 8) & 0xFF, ptr_cur_box->type & 0xFF,
					header_part, box_part_offset, box_part_size);
			}

			if (ptr_cur_box->type == 'mdat' && header_part)
			{
				if ((ptr_cur_box->start_bitpos >> 3) > nPostMDATChunkOffset)
				{
					nMDATChunkShiftAhead = (ptr_cur_box->start_bitpos >> 3) - nPostMDATChunkOffset;
					printf("[MP4] The 'mdat' chunk is shift ahead %" PRIu64 " bytes.\n", nMDATChunkShiftAhead);
				}
				break;
			}

			nPostMDATChunkOffset += box_part_size;
		}

		if (!bBoxRemoved)
			printf("The specified box-type(s) are not found in the current ISOBMFF file.\n");
		else
		{
			for (auto& slice : box_slices)
			{
				if ((iRet = FlushBoxSlice(slice, fp, fw, nMDATChunkShiftAhead)) < 0)
					break;
			}
		}
	}
	else
		iRet = RemoveBoxes(fp, fw, removed_box_types);

	if (fp != NULL) {
		fclose(fp); fp = nullptr;
	}

	if (fw != NULL) {
		fclose(fw); fw = nullptr;
	}

	if (iRet >= 0)
	{
		if (write_pathname != dst_filename)
		{
			if (_access(dst_filename.c_str(), F_OK) == 0)
			{
				if (_unlink(dst_filename.c_str()) == 0)
				{
					if (rename(write_pathname.c_str(), dst_filename.c_str()) != 0)
					{
						printf("Failed to rename the file: %s to the file: %s.\n", write_pathname.c_str(), dst_filename.c_str());
						goto done;
					}
				}
				else
					printf("Failed to delete the destination file \"%s\" to rename the intermediate file to the destination file.\n", dst_filename.c_str());
			}
		}
	}
	else
	{
		// Remove the temporary file
		printf("Failed to remove the boxes, delete the temporary file: %s.\n", write_pathname.c_str());
		if (_unlink(write_pathname.c_str()) != 0)
			printf("Failed to remove the temporary immediate filename: %s.\n", write_pathname.c_str());
	}

done:
	if (fp != NULL)
		fclose(fp);

	if (fw != NULL)
		fclose(fw);

	return iRet;
}

void PrintTree(Box* ptr_box, int level)
{
	if (ptr_box == nullptr || level < 0)
		return;

	size_t line_chars = (size_t)level * 5 + 128;
	char* szLine = new char[line_chars];
	memset(szLine, ' ', line_chars);
	
	const int indent = 2;
	const int level_span = 5;

	char* szText = nullptr;
	if (level >= 1)
	{
		Box* ptr_parent = ptr_box->container;
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

	if (ptr_box->container == nullptr)
		sprintf_s(szText, line_chars - (szText - szLine), ".\n");
	else
	{
		int cbWritten = 0;
		if (ptr_box->type == 'uuid')
		{
			auto ptr_uuid_box = (UUIDBox*)ptr_box;
			cbWritten += sprintf_s(szText, line_chars - (szText - szLine), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X (size: %" PRId64 ")\n",
				ptr_uuid_box->usertype[0x0], ptr_uuid_box->usertype[0x1], ptr_uuid_box->usertype[0x2], ptr_uuid_box->usertype[0x3],
				ptr_uuid_box->usertype[0x4], ptr_uuid_box->usertype[0x5], ptr_uuid_box->usertype[0x6], ptr_uuid_box->usertype[0x7],
				ptr_uuid_box->usertype[0x8], ptr_uuid_box->usertype[0x9], ptr_uuid_box->usertype[0xa], ptr_uuid_box->usertype[0xb],
				ptr_uuid_box->usertype[0xc], ptr_uuid_box->usertype[0xd], ptr_uuid_box->usertype[0xe], ptr_uuid_box->usertype[0xf],
				ptr_uuid_box->size);
		}
		else
		{
			cbWritten += CopyBoxTypeName(szText, (int)(line_chars - (szText - szLine)), ptr_box->type);
		}
		szText += cbWritten;

		if (ptr_box->type == 'trak')
		{
			MovieBox::TrackBox* ptr_trackbox = (MovieBox::TrackBox*)ptr_box;
			if (ptr_trackbox->track_header_box != nullptr)
			{
				MovieBox* ptr_moviebox = (MovieBox*)ptr_box->container;

				uint32_t track_ID = ptr_trackbox->track_header_box->version == 1 ? ptr_trackbox->track_header_box->v1.track_ID : ptr_trackbox->track_header_box->v0.track_ID;
				uint64_t duration = ptr_trackbox->track_header_box->version == 1 ? ptr_trackbox->track_header_box->v1.duration : ptr_trackbox->track_header_box->v0.duration;

				if (ptr_moviebox != nullptr && ptr_moviebox->container != nullptr)
				{
					MovieBox::MovieHeaderBox* ptr_moviehdr = ptr_moviebox->movie_header_box;
					uint32_t timescale = ptr_moviehdr->version == 1 ? ptr_moviehdr->v1.timescale : ptr_moviehdr->v0.timescale;

					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- track_ID: %d, duration: %" PRIu64 ".%03" PRIu64 "s",
						track_ID, duration / timescale, duration*1000/timescale%1000);
				}
				else
				{
					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- track_ID: %d", track_ID);
				}
				szText += cbWritten;
			}
		}

		if (ptr_box->type == 'hdlr' && ptr_box->container && ptr_box->container->type == 'mdia')
		{
			HandlerBox* ptr_handler_box = (HandlerBox*)ptr_box;
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- %s track",
				handle_type_names.find(ptr_handler_box->handler_type) != handle_type_names.end() ? handle_type_names[ptr_handler_box->handler_type] : "Unknown");
			szText += cbWritten;
		}

		if (ptr_box->type == 'tfhd')
		{
			auto ptr_tfhd_box = (MovieFragmentBox::TrackFragmentBox::TrackFragmentHeaderBox*)ptr_box;
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- track %d", ptr_tfhd_box->track_ID);
			szText += cbWritten;
		}

		if (ptr_box->type == 'tfdt')
		{
			auto ptr_tfdt_box = (MovieFragmentBox::TrackFragmentBox::TrackFragmentBaseMediaDecodeTimeBox*)ptr_box;
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- baseMediaDecodeTime: %" PRIu64 "", ptr_tfdt_box->baseMediaDecodeTime);
			szText += cbWritten;
		}

		if (ptr_box->type == 'trun')
		{
			auto ptr_trun_box = (MovieFragmentBox::TrackFragmentBox::TrackRunBox*)ptr_box;
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- SampleCount: %" PRIu32 "", ptr_trun_box->sample_count);
			szText += cbWritten;
			if (ptr_trun_box->flags & 0x000001)
			{
				cbWritten = sprintf_s(szText, line_chars - (szText - szLine), ", data offset: %" PRId32 "", ptr_trun_box->data_offset);
				szText += cbWritten;
			}
		}

		if (ptr_box->type == 'stsd')
		{
			if (ptr_box->container && ptr_box->container->container && ptr_box->container->container->container &&
				ptr_box->container->type == 'stbl' && 
				ptr_box->container->container->type == 'minf' &&
				ptr_box->container->container->container->type == 'mdia')
			{
				auto ptr_mdia_container = ptr_box->container->container->container;

				// find hdlr box
				Box* ptr_mdia_child = ptr_mdia_container->first_child;
				while (ptr_mdia_child != nullptr)
				{
					if (ptr_mdia_child->type == 'hdlr')
						break;
					ptr_mdia_child = ptr_mdia_child->next_sibling;
				}

				if (ptr_mdia_child != nullptr)
				{
					uint32_t handler_type = dynamic_cast<HandlerBox*>(ptr_mdia_child)->handler_type;

					auto ptr_sd = (MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox*)ptr_box;
					for (uint32_t i = 0; i < ptr_sd->SampleEntries.size(); i++)
					{
						if (handler_type == 'vide')
						{
							auto ptrVisualSampleEntry = (MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry*)ptr_sd->SampleEntries[i];
							if (i > 0)
								*(szText++) = ',';
							else
							{
								cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- ");
								szText += cbWritten;
							}
							
							if (ptrVisualSampleEntry->compressorname_size > 0)
							{
								memcpy(szText, (const char*)ptrVisualSampleEntry->compressorname, ptrVisualSampleEntry->compressorname_size);
								szText += ptrVisualSampleEntry->compressorname_size;
							}
							else
							{
								szText += CopyBoxTypeName(szText, (int)(line_chars - (szText - szLine)), ptrVisualSampleEntry->type);
							}

							cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "@%dx%d", ptrVisualSampleEntry->width, ptrVisualSampleEntry->height);
							szText += cbWritten;

						}
						else if (handler_type == 'soun')
						{
							auto ptrAudioSampleEntry = (MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::AudioSampleEntry*)ptr_sd->SampleEntries[i];
							if (i > 0)
								*(szText++) = ',';
							else
							{
								cbWritten = sprintf_s(szText, (int)(line_chars - (szText - szLine)), " -- ");
								szText += cbWritten;
							}
							szText += CopyBoxTypeName(szText, (int)(line_chars - (szText - szLine)), ptrAudioSampleEntry->type);
							szText += sprintf_s(szText, line_chars - (szText - szLine), "@%dHZ", ptrAudioSampleEntry->samplerate >> 16);
						}
					}
				}
			}
		}

		sprintf_s(szText, line_chars - (szText - szLine), "\n");
	}

	printf("%s", szLine);

	delete[] szLine;

	auto ptr_child = ptr_box->first_child;
	while(ptr_child != nullptr)
	{
		PrintTree(ptr_child, level + 1);
		ptr_child = ptr_child->next_sibling;
	}

	return;
}

Box* FindBox(Box* ptr_box, std::string strBoxPath)
{
	URI_Components box_uri;
	int nRet = AMURI_Split(strBoxPath.c_str(), box_uri);

	if (AMP_FAILED(nRet))
		return nullptr;

	if (box_uri.Ranges[URI_COMPONENT_PATH].IsEmpty())
		return nullptr;

	std::vector<URI_Segment> component_segments;
	if (AMP_FAILED(AMURI_SplitComponent(strBoxPath.c_str() + box_uri.Ranges[URI_COMPONENT_PATH].start, box_uri.Ranges[URI_COMPONENT_PATH].length, component_segments)))
		return nullptr;

	for (auto& seg : component_segments)
	{
		int64_t idx = INT64_MAX;
		std::string strBoxSeg;
		if (AMP_FAILED(AMURI_DecodeSegment(strBoxPath.c_str() + seg.start, seg.length, strBoxSeg)))
			return nullptr;

		const char* p = strBoxSeg.c_str();
		if (strBoxSeg.length() > 4)
			ConvertToInt((char*)p + 4, (char*)p + strBoxSeg.length(), idx);
		else if (strBoxSeg.length() < 4)
			return nullptr;

		int found_idx = -1;
		uint32_t seg_box_type = ((uint32_t)(*p) << 24) | ((uint32_t)(*(p + 1)) << 16) | ((uint32_t)(*(p + 2)) << 8) | *(p + 3);
		for (auto child = ptr_box->first_child; child != nullptr; child = child->next_sibling)
		{
			if (child->type == seg_box_type)
			{
				found_idx++;
				if (idx == found_idx || idx == INT64_MAX)
				{
					ptr_box = child;
					break;
				}
			}
		}

		if (found_idx <= -1 || (idx != INT64_MAX && idx != found_idx))
			return nullptr;
	}

	return ptr_box;
}

Box* FindBox(Box* ptr_box, uint32_t track_id, uint32_t box_type)
{
	if (ptr_box == nullptr)
		return nullptr;
	Box* ptr_ret_box = nullptr;
	Box* ptr_child_box = ptr_box->first_child;
	while (ptr_child_box != nullptr)
	{
		if (track_id != UINT32_MAX && ptr_child_box->type == 'tkhd' && ptr_child_box->container != NULL && ptr_child_box->container->type == 'trak')
		{
			// check its track-id
			MovieBox::TrackBox::TrackHeaderBox* ptr_trackhdr_box = (MovieBox::TrackBox::TrackHeaderBox*)ptr_child_box;
			if((ptr_trackhdr_box->version == 1 && ptr_trackhdr_box->v1.track_ID == track_id) ||
			   (ptr_trackhdr_box->version == 0 && ptr_trackhdr_box->v0.track_ID == track_id))
			{
				if (box_type != UINT32_MAX)
					return FindBox(ptr_trackhdr_box->container, UINT32_MAX, box_type);

				return ptr_trackhdr_box->container;
			}

			ptr_child_box = ptr_child_box->next_sibling;
			continue;
		}

		if (track_id == UINT32_MAX && box_type == ptr_child_box->type)
			return ptr_child_box;

		if ((ptr_ret_box = FindBox(ptr_child_box, track_id, box_type)) != nullptr)
			return ptr_ret_box;

		ptr_child_box = ptr_child_box->next_sibling;
	}

	return nullptr;
};

int ShowBoxInfo(Box* root_box, Box* ptr_box)
{
	int iRet = RET_CODE_SUCCESS;
#if 0
	FILE* fp = NULL;
	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	// Get file size
	_fseeki64(fp, 0, SEEK_END);
	int64_t file_size = _ftelli64(fp);
	_fseeki64(fp, 0, SEEK_SET);

	iRet = ListBoxes(fp, 0, 0, file_size, NULL, 1);

done:
	if (fp != NULL)
		fclose(fp);
#else
	if (g_params.find("trackid") == g_params.end() || ptr_box == nullptr)
	{
		if (ptr_box != nullptr)
			PrintTree(ptr_box, 0);
		else
			PrintTree(root_box, 0);

		return iRet;
	}

	uint32_t box_type = ptr_box->type;
	if (box_type == 'trak')
		PrintTree(ptr_box, 0);
	else
	{
		printf("=========================='%s' box information======================\n", g_params["boxtype"].c_str());
		if (box_type == 'stsc')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleToChunkBox* pSampleToChunkBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleToChunkBox*)ptr_box;
				
			printf("Entry Count: %d.\n", pSampleToChunkBox->entry_count);
			printf("--Entry_ID-------First Chunk------Samples Per Chunk----Sample Description Index--\n");
			for (uint32_t i = 0; i < pSampleToChunkBox->entry_infos.size(); i++)
			{
				printf("  #%06" PRIu32 "     % 10d       % 10d      %10d\n", i+1,
					pSampleToChunkBox->entry_infos[i].first_chunk, 
					pSampleToChunkBox->entry_infos[i].samples_per_chunk, 
					pSampleToChunkBox->entry_infos[i].sample_description_index);
			}
		}
		else if (box_type == 'stco')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkOffsetBox* pChunkOffsetBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkOffsetBox*)ptr_box;

			printf("Entry Count: %d.\n", pChunkOffsetBox->entry_count);
			printf("--- Chunk ID ------------- Chunk Offset --\n");
			for (size_t i = 0; i < pChunkOffsetBox->chunk_offset.size(); i++)
			{
				printf(" #%-10zu             %10d\n", i+1, pChunkOffsetBox->chunk_offset[i]);
			}
		}
		else if (box_type == 'co64')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkLargeOffsetBox* pLargeChunkOffsetBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkLargeOffsetBox*)ptr_box;

			printf("Entry Count: %d.\n", pLargeChunkOffsetBox->entry_count);
			printf("--- Chunk ID ------------- Chunk Offset --\n");
			for (size_t i = 0; i < pLargeChunkOffsetBox->chunk_offset.size(); i++)
			{
				printf(" #%-10zu             %10" PRIu64 "\n", i+1, pLargeChunkOffsetBox->chunk_offset[i]);
			}
		}
		else if (box_type == 'stsz')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleSizeBox* pSampleSizeBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleSizeBox*)ptr_box;

			printf("Default sample size: %" PRIu32 "\n", pSampleSizeBox->sample_size);
			printf("The number of samples in the current track: %" PRIu32 "\n", pSampleSizeBox->sample_count);
			if (pSampleSizeBox->sample_size == 0)
			{
				printf("-- Sample ID ------------- Sample Size --\n");
				for (size_t i = 0; i < pSampleSizeBox->entry_size.size(); i++)
					printf("  #%-10zu            % 10d\n", i+1, pSampleSizeBox->entry_size[i]);
			}
		}
		else if (box_type == 'stsd')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox* pSampleDescBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox*)ptr_box;

			auto PrintBinaryBuf = [](uint8_t* pBuf, size_t cbBuf) {
				printf("\t\t      00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F\n");
				printf("\t\t      ----------------------------------------------------------------\n");
				for (size_t idx = 0; idx < cbBuf; idx++)
				{
					if (idx % 16 == 0)
						printf("\t\t %03zu  ", idx);

					printf("%02X  ", pBuf[idx]);
					if ((idx + 1) % 8 == 0)
						printf("  ");

					if ((idx + 1) % 16 == 0)
						printf("\n");
				}

				printf("\n\n");
			};

			printf("entry count: %d\n", pSampleDescBox->entry_count);
			for (size_t i = 0; i < pSampleDescBox->SampleEntries.size(); i++)
			{
				if (pSampleDescBox->handler_type == 'vide')
				{
					MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry* pVisualSampleEntry =
						(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry*)pSampleDescBox->SampleEntries[i];
					printf("Resolution: %dx%d\n", pVisualSampleEntry->width, pVisualSampleEntry->height);
					printf("Compressorname: %s\n", (char*)pVisualSampleEntry->compressorname);
					printf("Depth: %d bits/pixel\n", pVisualSampleEntry->depth);

					if (pVisualSampleEntry->type == 'hvc1' || pVisualSampleEntry->type == 'hev1' || pVisualSampleEntry->type == 'hvcC')
					{
						HEVCSampleEntry* pHEVCSampleEntry = (HEVCSampleEntry*)pVisualSampleEntry;
						printf("Coding name: '%c%c%c%c'\n", pVisualSampleEntry->type >> 24, (pVisualSampleEntry->type >> 16) & 0xFF, (pVisualSampleEntry->type >> 8) & 0xFF, pVisualSampleEntry->type & 0xFF);
							
						auto config = pHEVCSampleEntry->config;
						printf("configurationVersion: %d\n", config->HEVCConfig->configurationVersion);
						printf("general_profile_space: %d\n", config->HEVCConfig->general_profile_space);
						printf("general_tier_flag: %d\n", config->HEVCConfig->general_tier_flag);
						printf("general_profile_idc: %d\n", config->HEVCConfig->general_profile_idc);
						printf("general_profile_compatibility_flags: 0X%X\n", config->HEVCConfig->general_profile_compatibility_flags);
						printf("general_constraint_indicator_flags: 0X%" PRIX64 "\n", config->HEVCConfig->general_constraint_indicator_flags);
						printf("general_level_idc: %d\n", config->HEVCConfig->general_level_idc);
						printf("min_spatial_segmentation_idc: %d\n", config->HEVCConfig->min_spatial_segmentation_idc);
						printf("parallelismType: %d\n", config->HEVCConfig->parallelismType);
						printf("chroma_format_idc: %d\n", config->HEVCConfig->chroma_format_idc);
						printf("bit_depth_luma_minus8: %d\n", config->HEVCConfig->bit_depth_luma_minus8);
						printf("bit_depth_chroma_minus8: %d\n", config->HEVCConfig->bit_depth_chroma_minus8);
						printf("avgFrameRate: %d\n", config->HEVCConfig->avgFrameRate);
						printf("constantFrameRate: %d\n", config->HEVCConfig->constantFrameRate);
						printf("numTemporalLayers: %d\n", config->HEVCConfig->numTemporalLayers);
						printf("temporalIdNested: %d\n", config->HEVCConfig->temporalIdNested);
						printf("lengthSizeMinusOne: %d\n", config->HEVCConfig->lengthSizeMinusOne);
						printf("numOfArrays: %d\n", config->HEVCConfig->numOfArrays);
						for (int j = 0; j < config->HEVCConfig->numOfArrays; j++)
						{
							printf("Array#%d\n", j);
							printf("\tarray_completeness[%d]: %d\n", j, config->HEVCConfig->nalArray[j]->array_completeness);
							printf("\tNAL_unit_type[%d]: %d (%s)\n", j, config->HEVCConfig->nalArray[j]->NAL_unit_type, hevc_nal_unit_type_names[config->HEVCConfig->nalArray[j]->NAL_unit_type]);
							printf("\tnumNalus[%d]: %d\n", j, config->HEVCConfig->nalArray[j]->numNalus);
							for (int k = 0; k < config->HEVCConfig->nalArray[j]->numNalus; k++)
							{
								printf("\t\tnalUnitLength[%d][%d]: %d\n", j, k, config->HEVCConfig->nalArray[j]->Nalus[k]->nalUnitLength);
								PrintBinaryBuf(&config->HEVCConfig->nalArray[j]->Nalus[k]->nalUnit[0], config->HEVCConfig->nalArray[j]->Nalus[k]->nalUnit.size());
							}
						}
					}
					else if (pVisualSampleEntry->type == 'avc1' || pVisualSampleEntry->type == 'avc2' || pVisualSampleEntry->type == 'avc3' || pVisualSampleEntry->type == 'avc4')
					{
						AVCSampleEntry* pAVCSampleEntry = (AVCSampleEntry*)pVisualSampleEntry;
						printf("Coding name: '%c%c%c%c'\n", pVisualSampleEntry->type >> 24, (pVisualSampleEntry->type >> 16) & 0xFF, (pVisualSampleEntry->type >> 8) & 0xFF, pVisualSampleEntry->type & 0xFF);

						auto config = pAVCSampleEntry->config;

						printf("configurationVersion: %d\n", config->AVCConfig->configurationVersion);
						printf("AVCProfileIndication: %d\n", config->AVCConfig->AVCProfileIndication);
						printf("profile_compatibility: 0X%X\n", config->AVCConfig->profile_compatibility);
						printf("AVCLevelIndication: %d\n", config->AVCConfig->AVCLevelIndication);
						printf("lengthSizeMinusOne: %d\n", config->AVCConfig->lengthSizeMinusOne);
						printf("numOfSequenceParameterSets: %d\n", config->AVCConfig->numOfSequenceParameterSets);

						for (size_t i = 0; i < config->AVCConfig->numOfSequenceParameterSets; i++)
						{
							printf("\tSequenceParameterSet#%zu (nalUnitLength: %d)\n", i, config->AVCConfig->sequenceParameterSetNALUnits[i]->nalUnitLength);
							PrintBinaryBuf(&config->AVCConfig->sequenceParameterSetNALUnits[i]->nalUnit[0], config->AVCConfig->sequenceParameterSetNALUnits[i]->nalUnit.size());
						}

						printf("numOfPictureParameterSets: %d\n", config->AVCConfig->numOfPictureParameterSets);

						for (size_t i = 0; i < config->AVCConfig->numOfPictureParameterSets; i++)
						{
							printf("\tPictureParameterSet#%zu (nalUnitLength: %d)\n", i, config->AVCConfig->pictureParameterSetNALUnits[i]->nalUnitLength);
							PrintBinaryBuf(&config->AVCConfig->pictureParameterSetNALUnits[i]->nalUnit[0], config->AVCConfig->pictureParameterSetNALUnits[i]->nalUnit.size());
						}

						if (config->AVCConfig->AVCProfileIndication == 100 || config->AVCConfig->AVCProfileIndication == 110 || config->AVCConfig->AVCProfileIndication == 122 || config->AVCConfig->AVCProfileIndication == 144)
						{
							printf("chroma_format: %d\n", config->AVCConfig->chroma_format);
							printf("bit_depth_luma_minus8: %d\n", config->AVCConfig->bit_depth_luma_minus8);
							printf("bit_depth_chroma_minus8: %d\n", config->AVCConfig->bit_depth_chroma_minus8);

							printf("numOfSequenceParameterSetExt: %d\n", config->AVCConfig->numOfSequenceParameterSetExt);

							for (size_t i = 0; i < config->AVCConfig->numOfSequenceParameterSetExt; i++)
							{
								printf("\tSequenceParameterSetExt#%zu (nalUnitLength: %d)\n", i, config->AVCConfig->sequenceParameterSetExtNALUnits[i]->nalUnitLength);
								PrintBinaryBuf(&config->AVCConfig->sequenceParameterSetExtNALUnits[i]->nalUnit[0], config->AVCConfig->sequenceParameterSetExtNALUnits[i]->nalUnit.size());
							}
						}
					}
				}
			}
		}
		else if (box_type == 'sdtp')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDependencyTypeBox* pSampleDepTypeBox
				= (MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDependencyTypeBox*)ptr_box;
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox* pSampleTableBox
				= (MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox*)ptr_box->container;

			if (pSampleTableBox != nullptr && pSampleTableBox->sample_size_box != nullptr)
			{
				uint32_t sample_count = pSampleTableBox->sample_size_box->sample_count;
				printf("== Sample ID === Leading ===== Depends On ==== Depended On ==== Redundancy ==\n");
				for (uint32_t i = 0; i < sample_count; i++)
				{
					printf("   #%-10" PRIu32 "      %d         %s        %s         %s\n",
						i+1,
						pSampleDepTypeBox->entries[i].is_leading,
						pSampleDepTypeBox->entries[i].sample_depends_on == 1 ? "Non-I-Frame" : (pSampleDepTypeBox->entries[i].sample_depends_on == 2 ? "    I-Frame" : "    Unknown"),
						pSampleDepTypeBox->entries[i].sample_is_depended_on == 1 ? "    Yes" : (pSampleDepTypeBox->entries[i].sample_is_depended_on == 2 ? "     No" : "Unknown"),
						pSampleDepTypeBox->entries[i].sample_has_redundancy == 1 ? "    Yes" : (pSampleDepTypeBox->entries[i].sample_has_redundancy == 2 ? "     No" : "Unknown"));
				}
			}
		}
		else if (box_type == 'ctts')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompositionOffsetBox* pCompositionOffsetBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompositionOffsetBox*)ptr_box;
			printf("entry count: %" PRIu32 "\n", pCompositionOffsetBox->entry_count);
			printf("== Entry ID ===== Sample Count ========= Sample Offset ==\n");
			for (size_t i = 0; i < pCompositionOffsetBox->entries.size(); i++)
			{
				if (pCompositionOffsetBox->version == 1)
					printf("   #%-10zu      %-10" PRIu32 "            %-10" PRId32 "\n", i + 1UL, pCompositionOffsetBox->entries[i].v1.sample_count, pCompositionOffsetBox->entries[i].v1.sample_offset);
				else if (pCompositionOffsetBox->version == 0)
					printf("   #%-10zu      %-10" PRIu32 "            %-10" PRId32 "\n", i + 1UL, pCompositionOffsetBox->entries[i].v0.sample_count, pCompositionOffsetBox->entries[i].v0.sample_offset);
			}
		}
		else if (box_type == 'stts')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::TimeToSampleBox* pTimeToSampleBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::TimeToSampleBox*)ptr_box;
			MovieBox::TrackBox::MediaBox* pMediaBox =
				(MovieBox::TrackBox::MediaBox*)ptr_box->container->container->container;
			uint32_t timescale = 0;
			if (pMediaBox && pMediaBox->media_header_box)
				timescale = pMediaBox->media_header_box->version == 0 ? pMediaBox->media_header_box->v0.timescale : (
					pMediaBox->media_header_box->version == 1 ? pMediaBox->media_header_box->v1.timescale : 0);

			printf("entry count: %d\n", pTimeToSampleBox->entry_count);
			printf("== Entry ID ===== Sample Count ========= Sample Delta ==\n");
			for (size_t i = 0; i < pTimeToSampleBox->entries.size(); i++)
				if (timescale == 0)
					printf("  #%-10zu      %-10" PRIu32 "          %10" PRIu32 "\n", i + 1UL, pTimeToSampleBox->entries[i].sample_count, pTimeToSampleBox->entries[i].sample_delta);
				else
					printf("  #%-10zu      %-10" PRIu32 "          %10" PRIu32 "(%" PRIu32 ".%03" PRIu32 "s)\n", i + 1UL, pTimeToSampleBox->entries[i].sample_count, pTimeToSampleBox->entries[i].sample_delta,
						pTimeToSampleBox->entries[i].sample_delta / timescale, pTimeToSampleBox->entries[i].sample_delta * 1000 / timescale % 1000);
		}
		else if (box_type == 'cslg')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompositionToDecodeBox* pCompositionToDecodeBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompositionToDecodeBox*)ptr_box;
			printf("compositionToDTSShift: %d\n", pCompositionToDecodeBox->compositionToDTSShift);
			printf("leastDecodeToDisplayDelta: %d\n", pCompositionToDecodeBox->leastDecodeToDisplayDelta);
			printf("greatestDecodeToDisplayDelta: %d\n", pCompositionToDecodeBox->greatestDecodeToDisplayDelta);
			printf("compositionStartTime: %d\n", pCompositionToDecodeBox->compositionStartTime);
			printf("compositionEndTime: %d\n", pCompositionToDecodeBox->compositionEndTime);
		}
		else if (box_type == 'stss')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SyncSampleBox* pSyncSampleBox =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SyncSampleBox*)ptr_box;

			printf("entry_count: %" PRIu32 "\n", pSyncSampleBox->entry_count);
			printf("== Entry ID ====== sample_number ===\n");
			for (size_t i = 0; i < pSyncSampleBox->sample_numbers.size(); i++)
				printf("   #%-10zu      %10" PRIu32 "\n", i, pSyncSampleBox->sample_numbers[i]);
		}
	}
#endif
	return iRet;
}

/*
@param key_sample 0: non-key sample; 1: key sample; 2: unknown
*/
int DumpMP4Sample(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox* pSampleDescBox, FILE* fp, FILE* fw, uint32_t sample_id, uint64_t sample_offset, uint32_t sample_size, int key_sample)
{
	int iRet = RET_CODE_ERROR;

	static uint8_t four_bytes_start_prefixes[4] = { 0, 0, 0, 1 };
	static uint8_t three_bytes_start_prefixes[3] = { 0, 0, 1 };
	
	HEVCConfigurationBox* pHEVCConfigurationBox = nullptr;
	AVCConfigurationBox* pAVCConfigurationBox = nullptr;
	for (size_t i = 0; pSampleDescBox != nullptr && i < pSampleDescBox->SampleEntries.size(); i++)
	{
		if (pSampleDescBox->handler_type == 'vide')
		{
			MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry* pVisualSampleEntry =
				(MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry*)pSampleDescBox->SampleEntries[i];

			if (pVisualSampleEntry == nullptr)
				continue;

			if (pVisualSampleEntry->type == 'av1C')
			{

			}
			else if (pVisualSampleEntry->type == 'hvc1' || pVisualSampleEntry->type == 'hev1' || pVisualSampleEntry->type == 'hvcC')
			{
				auto pHEVCSampleEntry = (HEVCSampleEntry*)pVisualSampleEntry;
				if (pHEVCSampleEntry != nullptr && pHEVCSampleEntry->config != nullptr)
				{
					pHEVCConfigurationBox = pHEVCSampleEntry->config;
					break;
				}
			}
			else if (pVisualSampleEntry->type == 'avc1' || pVisualSampleEntry->type == 'avc3')
			{
				auto pAVCSampleEntry = (AVCSampleEntry*)pVisualSampleEntry;
				if (pAVCSampleEntry != nullptr && pAVCSampleEntry->config != nullptr)
				{
					pAVCConfigurationBox = pAVCSampleEntry->config;
					break;
				}
			}
			else if (pVisualSampleEntry->type == 'avc2' || pVisualSampleEntry->type == 'avc4')
			{
				auto pAVC2SampleEntry = (AVC2SampleEntry*)pVisualSampleEntry;
				if (pAVC2SampleEntry != nullptr && pAVC2SampleEntry->avcconfig != nullptr)
				{
					pAVCConfigurationBox = pAVC2SampleEntry->avcconfig;
					break;
				}
			}
		}
		else if(pSampleDescBox->handler_type == 'soun')
		{

		}
	}

	if (g_verbose_level >= 1)
		printf("Sample#%-8" PRIu32 ", sample offset: %10" PRIu64 ", sample size: %10" PRIu32 ", key sample: %d\n", sample_id, sample_offset, sample_size, key_sample);

	if (pHEVCConfigurationBox != nullptr)
	{
		struct
		{
			int8_t nu_array_idx = -1;
			int8_t array_completeness = 0;
		}nu_array_info[64];

		if (key_sample)
		{
			// Write SPS, PPS, VPS or declarative SEI NAL unit
			for (size_t i = 0; i < pHEVCConfigurationBox->HEVCConfig->nalArray.size(); i++)
			{
				auto nu_type = pHEVCConfigurationBox->HEVCConfig->nalArray[i]->NAL_unit_type;
				nu_array_info[nu_type].array_completeness = pHEVCConfigurationBox->HEVCConfig->nalArray[i]->array_completeness;
				nu_array_info[nu_type].nu_array_idx = (int8_t)i;
			}
		}

		uint8_t buf[2048];
		int64_t cbLeft = sample_size;
		int8_t next_nal_unit_type = (int8_t)HEVC_VPS_NUT;
		bool bFirstNALUnit = true;

		auto write_nu_array = [&](int8_t nal_unit_type) {
			if (nu_array_info[nal_unit_type].nu_array_idx == -1)
				return false;

			auto nuArray = pHEVCConfigurationBox->HEVCConfig->nalArray[nu_array_info[nal_unit_type].nu_array_idx];
			for (size_t i = 0; i < nuArray->numNalus; i++)
			{
				if (fw != NULL)
				{
					if (nal_unit_type == (int8_t)HEVC_VPS_NUT || nal_unit_type == (int8_t)HEVC_SPS_NUT || nal_unit_type == (int8_t)HEVC_PPS_NUT)
						fwrite(four_bytes_start_prefixes, 1, 4, fw);
					else
						fwrite(three_bytes_start_prefixes, 1, 3, fw);

					fwrite(&nuArray->Nalus[i]->nalUnit[0], 1, nuArray->Nalus[i]->nalUnitLength, fw);
				}
				bFirstNALUnit = false;
			}

			return nuArray->numNalus > 0 ? true : false;
		};
		
		uint8_t lengthSizeMinusOne = 3;
		if (pHEVCConfigurationBox && pHEVCConfigurationBox->HEVCConfig)
			lengthSizeMinusOne = pHEVCConfigurationBox->HEVCConfig->lengthSizeMinusOne;

		while (cbLeft > 0)
		{
			uint32_t NALUnitLength = 0;
			if (fread(buf, 1, (size_t)lengthSizeMinusOne + 1UL, fp) != (size_t)lengthSizeMinusOne + 1UL)
				break;

			for (int i = 0; i < lengthSizeMinusOne + 1; i++)
				NALUnitLength = (NALUnitLength << 8) | buf[i];

			bool bLastNALUnit = cbLeft <= ((int64_t)lengthSizeMinusOne + 1 + NALUnitLength) ? true : false;

			uint8_t first_leading_read_pos = 0;
			if (key_sample && next_nal_unit_type != -1)
			{
				// read the nal_unit_type
				if (fread(buf, 1, 2, fp) != 2)
					break;

				first_leading_read_pos = 2;

				int8_t nal_unit_type = (buf[0] >> 1) & 0x3F;
				int8_t nuh_layer_id = ((buf[0] & 0x01) << 5) | ((buf[1] >> 3) & 0x1F);
				int8_t nuh_temporal_id_plus1 = buf[1] & 0x07;
				if (nal_unit_type == (int8_t)HEVC_VPS_NUT || 
					nal_unit_type == (int8_t)HEVC_SPS_NUT || 
					nal_unit_type == (int8_t)HEVC_PPS_NUT || 
					nal_unit_type == (int8_t)HEVC_PREFIX_SEI_NUT)
				{
					for (int8_t i = next_nal_unit_type; i < nal_unit_type; i++)
						write_nu_array(i);

					next_nal_unit_type = (nal_unit_type == (int8_t)HEVC_PPS_NUT) ? (int8_t)HEVC_PREFIX_SEI_NUT : (
										  nal_unit_type == (int8_t)HEVC_PREFIX_SEI_NUT? (int8_t)HEVC_SUFFIX_SEI_NUT:(nal_unit_type + 1));
				}
				else if (nal_unit_type >= (int8_t)HEVC_TRAIL_N && nal_unit_type <= (int8_t)HEVC_RSV_VCL31)
				{
					for (int8_t i = next_nal_unit_type; i <= (int8_t)HEVC_PREFIX_SEI_NUT; i++)
						write_nu_array(i);

					next_nal_unit_type = (int8_t)HEVC_SUFFIX_SEI_NUT;
				}
				else
				{
					if (nal_unit_type == (int8_t)HEVC_SUFFIX_SEI_NUT)
						next_nal_unit_type = -1;

					if (next_nal_unit_type == (int8_t)HEVC_SUFFIX_SEI_NUT && bLastNALUnit)
						write_nu_array((int8_t)HEVC_SUFFIX_SEI_NUT);
				}
			}

			if (fw != NULL)
			{
				if (bFirstNALUnit == true)
					fwrite(four_bytes_start_prefixes, 1, 4, fw);
				else
					fwrite(three_bytes_start_prefixes, 1, 3, fw);
			}

			uint32_t cbLeftNALUnit = NALUnitLength - first_leading_read_pos;
			while (cbLeftNALUnit > 0)
			{
				uint32_t nCpyCnt = AMP_MIN(2048UL - first_leading_read_pos, cbLeftNALUnit);

				if ((nCpyCnt = (uint32_t)fread(&buf[first_leading_read_pos], 1, nCpyCnt, fp)) == 0)
					break;

				if (fw != NULL)
					fwrite(buf, 1, (size_t)nCpyCnt + first_leading_read_pos, fw);

				cbLeftNALUnit -= nCpyCnt;
				first_leading_read_pos = 0;
			}

			cbLeft -= (int64_t)lengthSizeMinusOne + 1 + NALUnitLength;
			bFirstNALUnit = false;
		}

		if (cbLeft == 0)
			iRet = RET_CODE_SUCCESS;
	}
	else if (pAVCConfigurationBox != nullptr)
	{
		uint8_t buf[2048];
		int64_t cbLeft = sample_size;
		int8_t next_nal_unit_type = (int8_t)AVC_SPS_NUT;
		bool bFirstNALUnit = true;

		auto write_nu_array = [&](int8_t nal_unit_type) {
			if (nal_unit_type == (int8_t)AVC_SPS_NUT)
			{
				for (size_t i = 0; i < pAVCConfigurationBox->AVCConfig->sequenceParameterSetNALUnits.size(); i++)
				{
					if (fw != NULL)
					{
						fwrite(four_bytes_start_prefixes, 1, 4, fw);
						fwrite(&pAVCConfigurationBox->AVCConfig->sequenceParameterSetNALUnits[i]->nalUnit[0], 1, pAVCConfigurationBox->AVCConfig->sequenceParameterSetNALUnits[i]->nalUnit.size(), fw);
					}
					bFirstNALUnit = false;
				}
				return true;
			}
			else if(nal_unit_type == (int8_t)AVC_PPS_NUT)
			{
				for (size_t i = 0; i < pAVCConfigurationBox->AVCConfig->pictureParameterSetNALUnits.size(); i++)
				{
					if (fw != NULL)
					{
						fwrite(four_bytes_start_prefixes, 1, 4, fw);
						fwrite(&pAVCConfigurationBox->AVCConfig->pictureParameterSetNALUnits[i]->nalUnit[0], 1, pAVCConfigurationBox->AVCConfig->pictureParameterSetNALUnits[i]->nalUnit.size(), fw);
					}
					bFirstNALUnit = false;
				}
				return true;
			}
			else if (nal_unit_type == (int8_t)AVC_SPS_EXT_NUT)
			{
				for (size_t i = 0; i < pAVCConfigurationBox->AVCConfig->sequenceParameterSetExtNALUnits.size(); i++)
				{
					if (fw != NULL)
					{
						fwrite(three_bytes_start_prefixes, 1, 3, fw);
						fwrite(&pAVCConfigurationBox->AVCConfig->sequenceParameterSetExtNALUnits[i]->nalUnit[0], 1, pAVCConfigurationBox->AVCConfig->sequenceParameterSetExtNALUnits[i]->nalUnit.size(), fw);
					}
					bFirstNALUnit = false;
				}
				return true;
			}

			return false;
		};

		uint8_t lengthSizeMinusOne = 3;
		if (pAVCConfigurationBox && pAVCConfigurationBox->AVCConfig)
			lengthSizeMinusOne = pAVCConfigurationBox->AVCConfig->lengthSizeMinusOne;

		while (cbLeft > 0)
		{
			uint32_t NALUnitLength = 0;
			if (fread(buf, 1, (size_t)lengthSizeMinusOne + 1UL, fp) != (size_t)lengthSizeMinusOne + 1ULL)
				break;

			for (int i = 0; i < lengthSizeMinusOne + 1; i++)
				NALUnitLength = (NALUnitLength << 8) | buf[i];

			bool bLastNALUnit = cbLeft <= ((int64_t)lengthSizeMinusOne + 1 + NALUnitLength) ? true : false;
			DBG_UNREFERENCED_LOCAL_VARIABLE(bLastNALUnit);

			uint8_t first_leading_read_pos = 0;
			if (key_sample && next_nal_unit_type != -1)
			{
				// read the nal_unit_type
				if (fread(buf, 1, 1, fp) != 1)
					break;

				first_leading_read_pos = 1;

				int8_t nal_ref_idc = (buf[0] >> 5) & 0x03; DBG_UNREFERENCED_LOCAL_VARIABLE(nal_ref_idc);
				int8_t nal_unit_type = buf[0] & 0x1F;
				if (nal_unit_type == (int8_t)AVC_SPS_NUT || nal_unit_type == (int8_t)AVC_PPS_NUT)
				{
					for (int8_t i = next_nal_unit_type; i < nal_unit_type; i++)
						write_nu_array(i);

					next_nal_unit_type = nal_unit_type == (int8_t)AVC_PPS_NUT ? (int8_t)AVC_SPS_EXT_NUT : (nal_unit_type + 1);
				}
				else if (nal_unit_type >= 1 && nal_unit_type <= 5)
				{
					for (int8_t i = next_nal_unit_type; i <= (int8_t)AVC_SPS_EXT_NUT; i++)
						write_nu_array(i);
					next_nal_unit_type = -1;
				}
				else if (nal_unit_type == (int8_t)AVC_SEI_NUT)
				{
					for (int8_t i = next_nal_unit_type; i <= (int8_t)AVC_PPS_NUT; i++)
						write_nu_array(i);

					next_nal_unit_type = (int8_t)AVC_SPS_EXT_NUT;
				}
			}

			if (fw != NULL)
			{
				if (bFirstNALUnit == true)
					fwrite(four_bytes_start_prefixes, 1, 4, fw);
				else
					fwrite(three_bytes_start_prefixes, 1, 3, fw);
			}

			uint32_t cbLeftNALUnit = NALUnitLength - first_leading_read_pos;
			while (cbLeftNALUnit > 0)
			{
				uint32_t nCpyCnt = AMP_MIN(2048UL - first_leading_read_pos, cbLeftNALUnit);

				if ((nCpyCnt = (uint32_t)fread(&buf[first_leading_read_pos], 1, nCpyCnt, fp)) == 0)
					break;

				if (fw != NULL)
					fwrite(buf, 1, (size_t)nCpyCnt + first_leading_read_pos, fw);

				cbLeftNALUnit -= nCpyCnt;
				first_leading_read_pos = 0;
			}

			cbLeft -= (int64_t)lengthSizeMinusOne + 1 + NALUnitLength;
			bFirstNALUnit = false;
		}

		if (cbLeft == 0)
			iRet = RET_CODE_SUCCESS;
	}
	else
	{
		uint8_t buf[2048];
		int64_t cbLeft = sample_size;

		while (cbLeft > 0)
		{
			size_t nCpyCnt = (size_t)AMP_MIN(2048, cbLeft);
			if ((nCpyCnt = fread(buf, 1, nCpyCnt, fp)) == 0)
				break;

			if (fw != nullptr)
				fwrite(buf, 1, nCpyCnt, fw);

			cbLeft -= nCpyCnt;
		}

		if (cbLeft == 0)
			iRet = RET_CODE_SUCCESS;
	}

	return iRet;
}

int DumpMP4OneStreamFromMovieFragments(Box* root_box, uint32_t track_id, FILE* fp, FILE* fw, MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox* pSampleDescBox=nullptr)
{
	int iRet = RET_CODE_SUCCESS;

	if (pSampleDescBox == nullptr)
	{
		// If there is no 'stsd' box, try to find it from the init_mp4
		auto iter = g_params.find("dashinitmp4");
		if (iter != g_params.cend())
		{
			int iBSRet = RET_CODE_SUCCESS;
			CFileBitstream bs(iter->second.c_str(), 4096, &iBSRet);
			if (iBSRet >= 0)
			{
				auto& dash_init_mp4_box = Box::CreateRootBox();
				dash_init_mp4_box.Load(bs);

				auto result = dash_init_mp4_box.FindBox(".../trak");
				for (auto& v : result)
				{
					if (v == nullptr)
						continue;

					BST::ISOBMFF::MovieBox::TrackBox* ptr_trak_box = (BST::ISOBMFF::MovieBox::TrackBox*)result[0];
					if (ptr_trak_box->GetTrackID() == track_id)
					{
						// Try to find the 'stsd' box, and update the current null one
						auto stsd_result = ptr_trak_box->FindBox(".../stsd");
						if (stsd_result.size() > 0)
							pSampleDescBox = (MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox*)stsd_result[0];

						break;
					}
				}
			}
		}

		if (pSampleDescBox == nullptr)
			printf("Can't find the track box with the specified track-id: %" PRIu32 ".\n", track_id);
	}

	int sample_id = 0;
	try
	{
		// Enumerate the 'moof' box
		Box* ptr_child = root_box->first_child;
		while (ptr_child != nullptr)
		{
			if (ptr_child->type != 'moof')
			{
				ptr_child = ptr_child->next_sibling;
				continue;
			}

			MovieFragmentBox* ptr_moof_box = (MovieFragmentBox*)ptr_child;
			uint64_t moof_file_offset = (ptr_moof_box->start_bitpos >> 3);

			// find the following 'mdat' box
			auto ptr_mdat_child = ptr_child->next_sibling;
			while (ptr_mdat_child != nullptr)
			{
				if (ptr_mdat_child->type == 'mdat' || ptr_mdat_child->type == 'moof')
				{
					if (ptr_mdat_child->type == 'moof')
						ptr_mdat_child = nullptr;
					break;
				}

				ptr_mdat_child = ptr_mdat_child->next_sibling;
			}

			// Begin to track the track elementary stream
			/*uint32_t sequence_number = ptr_moof_box->movie_fragment_header_box->sequence_number;*/

			auto GetTrunFileOffset = [&](MovieFragmentBox::TrackFragmentBox* ptr_traf_box,
				MovieFragmentBox::TrackFragmentBox::TrackRunBox* ptr_trun_box,
				uint64_t prev_data_end_offset) {
				uint64_t file_offset = UINT64_MAX;

				if (ptr_traf_box == nullptr || ptr_trun_box == nullptr)
					return file_offset;

				auto ptr_tfhd_box = ptr_traf_box->track_fragment_header_box;
				bool traf_data_offset_exist = ptr_tfhd_box && (ptr_tfhd_box->flags & 0x000001);
				bool trun_data_offset_exist = ptr_trun_box->flags & 0x000001;

				// Make sure the file offset is decided
				if (!traf_data_offset_exist &&		// No base_data_offset
					!trun_data_offset_exist)		// No data_offset in 'trun' box
				{
					// If no 'mdat' is found
					if (ptr_mdat_child == nullptr)
						goto done;

					// first track and first trun
					if (ptr_traf_box == ptr_moof_box->track_fragment_boxes[0] &&
						ptr_trun_box == ptr_traf_box->track_run_boxes[0])
						return ptr_mdat_child->start_bitpos >> 3;

					// return the byte position of the preceding trun
					file_offset = prev_data_end_offset;
				}
				else if (!trun_data_offset_exist)	// No data_offset in 'trun' box
				{
					if (ptr_trun_box == ptr_traf_box->track_run_boxes[0])	// the first 'trun' in a 'traf'
						file_offset = moof_file_offset + ptr_tfhd_box->base_data_offset;
					else
						file_offset = prev_data_end_offset;
				}
				else if(ptr_tfhd_box != nullptr)
					file_offset = moof_file_offset + ptr_tfhd_box->base_data_offset + ptr_trun_box->data_offset;

			done:
				return file_offset;
			};

			//auto GetTrafFileOffset = [&](MovieFragmentBox::TrackFragmentBox* ptr_traf_box, uint64_t prev_data_end_offset) {
			//	return GetTrunFileOffset(ptr_traf_box, ptr_traf_box->track_run_boxes.size() > 0 ? ptr_traf_box->track_run_boxes[0] : nullptr, prev_data_end_offset);
			//};

			//uint64_t base_data_offset = 0ULL;
			uint64_t prev_data_end_offset = (uint64_t)-1LL;
			for (auto ptr_traf_box = ptr_moof_box->track_fragment_boxes.cbegin(); ptr_traf_box != ptr_moof_box->track_fragment_boxes.cend(); ptr_traf_box++)
			{
				auto ptr_tfhd_box = (*ptr_traf_box)->track_fragment_header_box;
				for (auto ptr_trun_box = (*ptr_traf_box)->track_run_boxes.cbegin(); ptr_trun_box != (*ptr_traf_box)->track_run_boxes.cend(); ptr_trun_box++)
				{
					uint64_t file_offset = GetTrunFileOffset(*ptr_traf_box, *ptr_trun_box, prev_data_end_offset);

					prev_data_end_offset = file_offset;

					if (file_offset == (uint64_t)-1LL)
						continue;

					bool file_seek_success = _fseeki64(fp, (long long)file_offset, SEEK_SET) == 0 ? true : false;

					bool bHaveSampleSize = ((*ptr_trun_box)->flags & 0x000200) ? true : false;
					if (bHaveSampleSize)
					{
						// Extract the sample one-by-one
						for (size_t i = 0; i < (*ptr_trun_box)->samples.size(); i++)
						{
							if (ptr_tfhd_box != nullptr && track_id == ptr_tfhd_box->track_ID && file_seek_success)
							{
								// Extract elementary stream
								DumpMP4Sample(pSampleDescBox, fp, fw, sample_id, prev_data_end_offset, (*ptr_trun_box)->samples[i].sample_size, false);
								sample_id++;
							}

							prev_data_end_offset += (*ptr_trun_box)->samples[i].sample_size;
						}

						if ((*ptr_trun_box)->samples.size() != (*ptr_trun_box)->sample_count)
						{
							prev_data_end_offset = UINT64_MAX;
						}
					}
					else
					{
						if (ptr_tfhd_box == nullptr || !(ptr_tfhd_box->flags & 0x000010))
						{
							// No default sample size is available
							prev_data_end_offset = UINT64_MAX;
							continue;
						}

						uint32_t default_sample_size = ptr_tfhd_box->default_sample_size;

						for (uint32_t i = 0; i < (*ptr_trun_box)->sample_count; i++)
						{
							if (track_id == ptr_tfhd_box->track_ID && file_seek_success)
							{
								// Extract elementary stream
								DumpMP4Sample(pSampleDescBox, fp, fw, sample_id, prev_data_end_offset, (*ptr_trun_box)->samples[i].sample_size, false);
								sample_id++;
							}

							prev_data_end_offset += default_sample_size;
						}
					}  // if (bHaveSampleSize)
				}  // for (auto ptr_trun_box...
			}  // for (auto ptr_traf_box =

			ptr_child = ptr_child->next_sibling;
		}  // while (ptr_child != nullptr)
	}
	catch (...)
	{
		iRet = RET_CODE_ERROR;
	}

	return iRet;
}

int DumpMP4OneStreamFromMovieFragments(Box* root_box, uint32_t track_id)
{
	int iRet = RET_CODE_SUCCESS;
	FILE *fp = NULL, *fw = NULL;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if (g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	DumpMP4OneStreamFromMovieFragments(root_box, track_id, fp, fw);

done:
	if (fp != NULL)
		fclose(fp);
	if (fw != NULL)
		fclose(fw);
	return iRet;
}

int DumpMP4OneStream(Box* root_box, Box* track_box)
{
	uint32_t sample_id = 0;
	int iRet = RET_CODE_SUCCESS;
	FILE *fp = NULL, *fw = NULL;

	if (track_box == nullptr || track_box->type != 'trak')
	{
		printf("No available 'trak' box.\n");
		return iRet;
	}

	MovieBox::TrackBox* pTrackBox = (MovieBox::TrackBox*)track_box;

	if (pTrackBox->media_box == nullptr ||
		pTrackBox->media_box->media_information_box == nullptr ||
		pTrackBox->media_box->media_information_box->sample_table_box == nullptr)
	{
		printf("No 'stbl' box.\n");
		return iRet;
	}

	auto pSampleTableBox = pTrackBox->media_box->media_information_box->sample_table_box;
	auto pSampleToChunkBox = pSampleTableBox->sample_to_chunk_box;
	auto pCompactSampleSizeBox = pSampleTableBox->compact_sample_size_box;
	auto pSampleSizeBox = pSampleTableBox->sample_size_box;
	auto pChunkLargeOffsetBox = pSampleTableBox->chunk_large_offset_box;
	auto pChunkOffsetBox = pSampleTableBox->chunk_offset_box;
	auto pSyncSampleBox = pSampleTableBox->sync_sample_box;
	auto pSampleDependencyTypeBox = pSampleTableBox->sample_dependency_type_box;
	auto pSampleDescBox = pSampleTableBox->sample_description_box;

	errno_t errn = fopen_s(&fp, g_params["input"].c_str(), "rb");
	if (errn != 0 || fp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	if (g_params.find("output") != g_params.end())
	{
		errn = fopen_s(&fw, g_params["output"].c_str(), "wb+");
		if (errn != 0 || fw == NULL)
		{
			printf("Failed to open the file: %s {errno: %d}.\n", g_params["output"].c_str(), errn);
			goto done;
		}
	}

	try
	{
		size_t nTotalSampleSize = pSampleSizeBox->entry_size.size();
		for (size_t entry_id = 0; entry_id < pSampleToChunkBox->entry_infos.size(); entry_id++)
		{
			uint32_t last_chunk = 0;
			if (entry_id == pSampleToChunkBox->entry_infos.size() - 1)
				last_chunk = pSampleToChunkBox->entry_infos[entry_id].first_chunk + (uint32_t)(nTotalSampleSize - sample_id) / pSampleToChunkBox->entry_infos[entry_id].samples_per_chunk;
			else
				last_chunk = pSampleToChunkBox->entry_infos[entry_id + 1].first_chunk;

			for (uint32_t chunkid = pSampleToChunkBox->entry_infos[entry_id].first_chunk; chunkid < last_chunk;chunkid++)
			{
				// Calculate the sample offset and sample size
				uint64_t sample_offset = pChunkLargeOffsetBox == nullptr ? pChunkOffsetBox->chunk_offset[chunkid-1] : pChunkLargeOffsetBox->chunk_offset[chunkid-1];
				if (_fseeki64(fp, (long long)sample_offset, SEEK_SET) != 0)
					continue;

				for (uint32_t idx = 0; idx < pSampleToChunkBox->entry_infos[entry_id].samples_per_chunk; idx++)
				{
					uint32_t sample_size = pCompactSampleSizeBox ? pCompactSampleSizeBox->entry_size[sample_id] : pSampleSizeBox->entry_size[sample_id];

					int key_frame = 2;
					// Check whether it is a key sample or not
					if (pSyncSampleBox != nullptr)
					{
						auto iter = std::find(pSyncSampleBox->sample_numbers.cbegin(), pSyncSampleBox->sample_numbers.cend(), sample_id + 1);
						if (iter != pSyncSampleBox->sample_numbers.cend())
							key_frame = 1;
						else
							key_frame = 0;
					}
					else if (pSampleDependencyTypeBox != nullptr)
					{
						key_frame = pSampleDependencyTypeBox->entries[sample_id].sample_depends_on == 2 ? 1 : (
							pSampleDependencyTypeBox->entries[sample_id].sample_depends_on == 1 ? 0 : 2);
					}

					DumpMP4Sample(pSampleDescBox, fp, fw, sample_id, sample_offset, sample_size, key_frame);

					sample_id++;
					sample_offset += sample_size;
				}
			}
		}

		DumpMP4OneStreamFromMovieFragments(root_box, pTrackBox->GetTrackID(), fp, fw, pSampleDescBox);

		iRet = RET_CODE_SUCCESS;
	}
	catch (...)
	{
		iRet = RET_CODE_ERROR;
	}
	
done:
	if (fp != NULL)
		fclose(fp);
	if (fw != NULL)
		fclose(fw);
	return iRet;
}

int DumpMP4Partial(Box* root_box)
{
	return -1;
}

int DumpMP4()
{
	int iRet = -1;

	auto& root_box = Box::CreateRootBox();

	{
		CFileBitstream bs(g_params["input"].c_str(), 4096, &iRet);
		root_box.Load(bs);
	}

	Box* ptr_box = nullptr;

	std::string strBoxType;
	auto iterBoxType = g_params.find("boxtype");
	if (iterBoxType != g_params.end())
	{
		strBoxType = iterBoxType->second;
		ptr_box = FindBox(&root_box, strBoxType);
	}

	bool bMovieTrackAbsent = false;
	uint32_t select_track_id = UINT32_MAX;
	if (g_params.find("trackid") != g_params.end())
	{
		// Try to filter the box with the specified track-id and box-type under the track container box
		long long track_id = ConvertToLongLong(g_params["trackid"]);
		if (track_id <= 0 || track_id > UINT32_MAX)
		{
			printf("The specified track-id is out of range.\n");
			return -1;
		}

		select_track_id = (uint32_t)track_id;

		// Convert box-type to integer characters
		uint32_t box_type = UINT32_MAX;	// invalid box type
		if (g_params.find("boxtype") != g_params.end() && g_params["boxtype"].length() > 0)
		{
			std::string& strBoxType = g_params["boxtype"];
			if (strBoxType.length() < 4)
			{
				printf("The specified box-type is not a FOURCC string.\n");
				return -1;
			}

			box_type = strBoxType[0] << 24 | strBoxType[1] << 16 | strBoxType[2] << 8 | strBoxType[3];
		}

		// Locate the track box
		if ((ptr_box = FindBox(&root_box, (uint32_t)track_id, box_type)) == nullptr)
		{
			if (box_type == UINT32_MAX)
			{
				//printf("Can't find the track box with the specified track-id: %lld.\n", track_id);
				// There may be still movie fragments
				bMovieTrackAbsent = true;
			}
			else
			{
				printf("Can't find the box with box-type: %s in the track-box with track-id: %lld.\n", g_params["boxtype"].c_str(), track_id);
				return -1;
			}
		}
	}

	if (g_params.find("showinfo") != g_params.end())
	{
		iRet = ShowBoxInfo(&root_box, ptr_box);
	}

	if (g_params.find("removebox") != g_params.end())
	{
		std::set<uint32_t> removed_box_types;
		std::string& str_remove_boxtypes = g_params["removebox"];

		if (str_remove_boxtypes.length() == 0 ||
			str_remove_boxtypes.compare("''") == 0 ||
			str_remove_boxtypes.compare("\"\"") == 0)
		{
			removed_box_types.insert(0);
		}
		else
		{
			if (str_remove_boxtypes.length() < 4)
			{
				printf("The 'removebox' parameter is invalid.\n");
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
					removed_box_types.insert(box_type);
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
				removed_box_types.insert(box_type);
		}

		if ((iRet = RefineMP4File(&root_box, g_params["input"], g_params.find("output") == g_params.end() ? g_params["input"] : g_params["output"], removed_box_types)) < 0)
			return iRet;
	}
	else if (g_params.find("trackid") != g_params.end() && g_params.find("output") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0))
		{
			if (bMovieTrackAbsent)
				iRet = DumpMP4OneStreamFromMovieFragments(&root_box, select_track_id);
			else
				iRet = DumpMP4OneStream(&root_box, ptr_box);
		}
		else if (str_output_fmt.compare("mp4") == 0)
		{
			iRet = DumpMP4Partial(&root_box);
		}
	}

	return iRet;
}
