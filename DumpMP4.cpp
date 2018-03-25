#include "StdAfx.h"
#include <vector>
#include <unordered_map>
#include <map>
#include "PayloadBuf.h"
#include "DumpTS.h"
#include "Bitstream.h"
#include <assert.h>
#include <filesystem>
#include <functional>
#include "ISO14496_12.h"
#include "ISO14496_15.h"

using namespace std;

extern const char *dump_msg[];
extern unordered_map<std::string, std::string> g_params;
extern TS_FORMAT_INFO g_ts_fmtinfo;
extern int g_verbose_level;
extern DUMP_STATUS g_dump_status;

const char* h265_nal_unit_type_names[64] = {
	/*00*/ "VCL::TRAIL_N",
	/*01*/ "VCL::TRAIL_R",
	/*02*/ "VCL::TSA_N",
	/*03*/ "VCL::TSA_R",
	/*04*/ "VCL::STSA_N",
	/*05*/ "VCL::STSA_R",
	/*06*/ "VCL::RADL_N",
	/*07*/ "VCL::RADL_R",
	/*08*/ "VCL::RASL_N",
	/*09*/ "VCL::RASL_R",
	/*10*/ "VCL::RSV_VCL_N10",
	/*11*/ "VCL::RSV_VCL_R11",
	/*12*/ "VCL::RSV_VCL_N12",
	/*13*/ "VCL::RSV_VCL_R13",
	/*14*/ "VCL::RSV_VCL_N14",
	/*15*/ "VCL::RSV_VCL_R15",
	/*16*/ "VCL::BLA_W_LP Coded",
	/*17*/ "VCL::BLA_W_RADL",
	/*18*/ "VCL::BLA_N_LP",
	/*19*/ "VCL::IDR_W_RADL",
	/*20*/ "VCL::IDR_N_LP",
	/*21*/ "VCL::CRA_NUT",
	/*22*/ "VCL::RSV_IRAP_VCL22",
	/*23*/ "VCL::RSV_IRAP_VCL23",
	/*24*/ "VCL::RSV_VCL24",
	/*25*/ "VCL::RSV_VCL25",
	/*26*/ "VCL::RSV_VCL26",
	/*27*/ "VCL::RSV_VCL27",
	/*28*/ "VCL::RSV_VCL28",
	/*29*/ "VCL::RSV_VCL29",
	/*30*/ "VCL::RSV_VCL30",
	/*31*/ "VCL::RSV_VCL31",
	/*32*/ "non-VCL::VPS_NUT",
	/*33*/ "non-VCL::SPS_NUT",
	/*34*/ "non-VCL::PPS_NUT",
	/*35*/ "non-VCL::AUD_NUT",
	/*36*/ "non-VCL::EOS_NUT",
	/*37*/ "non-VCL::EOB_NUT",
	/*38*/ "non-VCL::FD_NUT",
	/*39*/ "non-VCL::PREFIX_SEI_NUT",
	/*40*/ "non-VCL::SUFFIX_SEI_NUT",
	/*41*/ "non-VCL::RSV_NVCL41",
	/*42*/ "non-VCL::RSV_NVCL42",
	/*43*/ "non-VCL::RSV_NVCL43",
	/*44*/ "non-VCL::RSV_NVCL44",
	/*45*/ "non-VCL::RSV_NVCL45",
	/*46*/ "non-VCL::RSV_NVCL46",
	/*47*/ "non-VCL::RSV_NVCL47",
	/*48*/ "non-VCL::UNSPEC48",
	/*49*/ "non-VCL::UNSPEC49",
	/*50*/ "non-VCL::UNSPEC50",
	/*51*/ "non-VCL::UNSPEC51",
	/*52*/ "non-VCL::UNSPEC52",
	/*53*/ "non-VCL::UNSPEC53",
	/*54*/ "non-VCL::UNSPEC54",
	/*55*/ "non-VCL::UNSPEC55",
	/*56*/ "non-VCL::UNSPEC56",
	/*57*/ "non-VCL::UNSPEC57",
	/*58*/ "non-VCL::UNSPEC58",
	/*59*/ "non-VCL::UNSPEC59",
	/*60*/ "non-VCL::UNSPEC60",
	/*61*/ "non-VCL::UNSPEC61",
	/*62*/ "non-VCL::UNSPEC62",
	/*63*/ "non-VCL::UNSPEC63",
};

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
			if ((cbRead = fread_s(buf, cache_size, 1, AMP_MIN(cache_size, cbLeft), fp)) <= 0)
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

void PrintTree(ISOMediaFile::Box* ptr_box, int level)
{
	if (ptr_box == nullptr)
		return;

	size_t line_chars = level * 5 + 128;
	char* szLine = new char[line_chars];
	memset(szLine, ' ', line_chars);
	
	const int indent = 2;
	const int level_span = 5;

	char* szText = nullptr;
	if (level >= 1)
	{
		ISOMediaFile::Box* ptr_parent = ptr_box->container;
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

	if (ptr_box->container == nullptr)
		sprintf_s(szText, line_chars - (szText - szLine), ".\r\n");
	else if (ptr_box->type != 'uuid')
	{
		char c0 = (ptr_box->type >> 24) & 0xFF;
		char c1 = (ptr_box->type >> 16) & 0xFF;
		char c2 = (ptr_box->type >> 8) & 0xFF;
		char c3 = (ptr_box->type & 0xFF);
		int cbWritten = 0;
		if (isprint(c0) && isprint(c1) && isprint(c2) && isprint(c3))
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "'%c%c%c%c' (size: %lld)", c0, c1, c2, c3, ptr_box->size);
		else
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "'%c%c%c%c'/%08Xh (size: %lld)",
				isprint(c0) ? c0 : ' ', isprint(c1) ? c1 : ' ', isprint(c2) ? c2 : ' ', isprint(c3) ? c3 : ' ',
				ptr_box->type, ptr_box->size);

		szText += cbWritten;

		if (ptr_box->type == 'trak')
		{
			ISOMediaFile::MovieBox::TrackBox* ptr_trackbox = (ISOMediaFile::MovieBox::TrackBox*)ptr_box;
			if (ptr_trackbox->track_header_box != nullptr)
			{
				ISOMediaFile::MovieBox* ptr_moviebox = (ISOMediaFile::MovieBox*)ptr_box->container;

				uint32_t track_ID = ptr_trackbox->track_header_box->version == 1 ? ptr_trackbox->track_header_box->v1.track_ID : ptr_trackbox->track_header_box->v0.track_ID;
				uint64_t duration = ptr_trackbox->track_header_box->version == 1 ? ptr_trackbox->track_header_box->v1.duration : ptr_trackbox->track_header_box->v0.duration;

				if (ptr_moviebox != nullptr && ptr_moviebox->container != nullptr)
				{
					ISOMediaFile::MovieBox::MovieHeaderBox* ptr_moviehdr = ptr_moviebox->movie_header_box;
					uint32_t timescale = ptr_moviehdr->version == 1 ? ptr_moviehdr->v1.timescale : ptr_moviehdr->v0.timescale;

					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- track_ID: %d, duration: %llu.%03llus",
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
			ISOMediaFile::HandlerBox* ptr_handler_box = (ISOMediaFile::HandlerBox*)ptr_box;
			cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- %s track",
				handle_type_names.find(ptr_handler_box->handler_type) != handle_type_names.end() ? handle_type_names[ptr_handler_box->handler_type] : "Unknown");
			szText += cbWritten;
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
				ISOMediaFile::Box* ptr_mdia_child = ptr_mdia_container->first_child;
				while (ptr_mdia_child != nullptr)
				{
					if (ptr_mdia_child->type == 'hdlr')
						break;
					ptr_mdia_child = ptr_mdia_child->next_sibling;
				}

				if (ptr_mdia_child != nullptr)
				{
					uint32_t handler_type = dynamic_cast<ISOMediaFile::HandlerBox*>(ptr_mdia_child)->handler_type;

					auto ptr_sd = (ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox*)ptr_box;
					for (uint32_t i = 0; i < ptr_sd->SampleEntries.size(); i++)
					{
						if (handler_type == 'vide')
						{
							auto ptrVisualSampleEntry = (ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry*)ptr_sd->SampleEntries[i];
							if (i > 0)
								*(szText++) = ',';
							else
							{
								cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- ");
								szText += cbWritten;
							}
							memcpy(szText, (const char*)ptrVisualSampleEntry->compressorname, ptrVisualSampleEntry->compressorname_size);
							szText += ptrVisualSampleEntry->compressorname_size;

							cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "@%dx%d", ptrVisualSampleEntry->width, ptrVisualSampleEntry->height);
							szText += cbWritten;

						}
						else if (handler_type == 'soun')
						{
							auto ptrAudioSampleEntry = (ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::AudioSampleEntry*)ptr_sd->SampleEntries[i];
							if (i > 0)
								*(szText++) = ',';
							else
							{
								cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " -- ");
								szText += cbWritten;
							}
							char c0 = (ptrAudioSampleEntry->type >> 24) & 0xFF;
							char c1 = (ptrAudioSampleEntry->type >> 16) & 0xFF;
							char c2 = (ptrAudioSampleEntry->type >> 8) & 0xFF;
							char c3 = (ptrAudioSampleEntry->type & 0xFF);

							*szText++ = isprint(c0) ? c0 : '.';
							*szText++ = isprint(c1) ? c1 : '.';
							*szText++ = isprint(c2) ? c2 : '.';
							*szText++ = isprint(c3) ? c3 : '.';

							cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "@%dHZ", ptrAudioSampleEntry->samplerate>>16);
							szText += cbWritten;
						}
					}
				}
			}
		}

		sprintf_s(szText, line_chars - (szText - szLine), "\r\n");
	}
	else
	{
		sprintf_s(szText, line_chars - (szText - szLine), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X (size: %lld)\r\n",
			ptr_box->usertype[0x0], ptr_box->usertype[0x1], ptr_box->usertype[0x2], ptr_box->usertype[0x3], 
			ptr_box->usertype[0x4], ptr_box->usertype[0x5], ptr_box->usertype[0x6], ptr_box->usertype[0x7],
			ptr_box->usertype[0x8], ptr_box->usertype[0x9], ptr_box->usertype[0xa], ptr_box->usertype[0xb], 
			ptr_box->usertype[0xc], ptr_box->usertype[0xd], ptr_box->usertype[0xe], ptr_box->usertype[0xf], 
			ptr_box->size);
	}

	printf(szLine);

	delete[] szLine;

	auto ptr_child = ptr_box->first_child;
	while(ptr_child != nullptr)
	{
		PrintTree(ptr_child, level + 1);
		ptr_child = ptr_child->next_sibling;
	}

	return;
}

int ShowBoxInfo(ISOMediaFile::Box* root_box)
{
	int iRet = -1;
#if 0
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
#else
	if (g_params.find("trackid") == g_params.end())
		PrintTree(root_box, 0);
	else
	{
		// Try to filter the box with the specified track-id and box-type under the track container box
		long long track_id = ConvertToLongLong(g_params["trackid"]);
		if (track_id <= 0 || track_id > UINT32_MAX)
		{
			printf("The specified track-id is out of range.\r\n");
			return -1;
		}

		// Convert box-type to integer characters
		uint32_t box_type = UINT32_MAX;	// invalid box type
		if (g_params.find("boxtype") != g_params.end())
		{
			std::string& strBoxType = g_params["boxtype"];
			if (strBoxType.length() < 4)
			{
				printf("The specified box-type is not a FOURCC string.\r\n");
				return -1;
			}

			box_type = strBoxType[0] << 24 | strBoxType[1] << 16 | strBoxType[2] << 8 | strBoxType[3];
		}

		// Locate the track box
		std::function<ISOMediaFile::Box*(ISOMediaFile::Box*, uint32_t, uint32_t)> FindBox;
		FindBox = [&FindBox](ISOMediaFile::Box* ptr_box, uint32_t track_id, uint32_t box_type) -> ISOMediaFile::Box* {
			if (ptr_box == nullptr)
				return nullptr;
			ISOMediaFile::Box* ptr_ret_box = nullptr;
			ISOMediaFile::Box* ptr_child_box = ptr_box->first_child;
			while (ptr_child_box != nullptr)
			{
				if (track_id != UINT32_MAX && ptr_child_box->type == 'tkhd' && ptr_child_box->container != NULL && ptr_child_box->container->type == 'trak')
				{
					// check its track-id
					ISOMediaFile::MovieBox::TrackBox::TrackHeaderBox* ptr_trackhdr_box = (ISOMediaFile::MovieBox::TrackBox::TrackHeaderBox*)ptr_child_box;
					if (ptr_trackhdr_box->version == 1 && ptr_trackhdr_box->v1.track_ID == track_id ||
						ptr_trackhdr_box->version == 0 && ptr_trackhdr_box->v0.track_ID == track_id)
					{
						if (box_type != UINT32_MAX)
							return FindBox(ptr_trackhdr_box->container, UINT32_MAX, box_type);

						return ptr_trackhdr_box->container;
					}

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

		auto ptr_box = FindBox(root_box, (uint32_t)track_id, box_type);
		if (ptr_box == nullptr)
		{
			if (box_type == UINT32_MAX)
				printf("Can't find the track box with the specified track-id: %lld.\r\n", track_id);
			else
				printf("Can't find the box with box-type: %s in the track-box with track-id: %lld.\r\n", g_params["boxtype"].c_str(), track_id);
			return -1;
		}

		if (box_type == UINT32_MAX)
			PrintTree(ptr_box, 0);
		else
		{
			printf("=========================='%s' box information======================\n", g_params["boxtype"].c_str());
			if (box_type == 'stsc')
			{
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleToChunkBox* pSampleToChunkBox =
					(ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleToChunkBox*)ptr_box;
				
				printf("Entry Count: %d.\n", pSampleToChunkBox->entry_count);
				printf("--Entry_ID-------First Chunk------Samples Per Chunk----Sample Description Index--\n");
				for (uint32_t i = 0; i < pSampleToChunkBox->entry_infos.size(); i++)
				{
					printf("  #%06d     % 10d       % 10d      %10d\n", i+1, 
						pSampleToChunkBox->entry_infos[i].first_chunk, 
						pSampleToChunkBox->entry_infos[i].samples_per_chunk, 
						pSampleToChunkBox->entry_infos[i].sample_description_index);
				}
			}
			else if (box_type == 'stco')
			{
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkOffsetBox* pChunkOffsetBox =
					(ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkOffsetBox*)ptr_box;

				printf("Entry Count: %d.\n", pChunkOffsetBox->entry_count);
				printf("--- Chunk ID ------------- Chunk Offset --\n");
				for (size_t i = 0; i < pChunkOffsetBox->chunk_offset.size(); i++)
				{
					printf(" #% -10d             % 10d\n", i+1, pChunkOffsetBox->chunk_offset[i]);
				}
			}
			else if (box_type == 'co64')
			{
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkLargeOffsetBox* pLargeChunkOffsetBox =
					(ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkLargeOffsetBox*)ptr_box;

				printf("Entry Count: %d.\n", pLargeChunkOffsetBox->entry_count);
				printf("--- Chunk ID ------------- Chunk Offset --\n");
				for (size_t i = 0; i < pLargeChunkOffsetBox->chunk_offset.size(); i++)
				{
					printf(" #% -10d             % 10lld\n", i+1, pLargeChunkOffsetBox->chunk_offset[i]);
				}
			}
			else if (box_type == 'stsz')
			{
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleSizeBox* pSampleSizeBox =
					(ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleSizeBox*)ptr_box;

				printf("Default sample size: %lu\n", pSampleSizeBox->sample_size);
				printf("The number of samples in the current track: %lu\n", pSampleSizeBox->sample_count);
				if (pSampleSizeBox->sample_size == 0)
				{
					printf("-- Sample ID ------------- Sample Size --\n");
					for (size_t i = 0; i < pSampleSizeBox->entry_size.size(); i++)
						printf("  #% -10d            % 10d\n", i+1, pSampleSizeBox->entry_size[i]);
				}
			}
			else if (box_type == 'stsd')
			{
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox* pSampleDescBox =
					(ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox*)ptr_box;

				printf("entry count: %d\n", pSampleDescBox->entry_count);
				for (size_t i = 0; i < pSampleDescBox->SampleEntries.size(); i++)
				{
					if (pSampleDescBox->handler_type == 'vide')
					{
						ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry* pVisualSampleEntry =
							(ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry*)pSampleDescBox->SampleEntries[i];
						printf("Resolution: %dx%d\n", pVisualSampleEntry->width, pVisualSampleEntry->height);
						printf("Compressorname: %s\n", (char*)pVisualSampleEntry->compressorname);
						printf("Depth: %d bits/pixel\n", pVisualSampleEntry->depth);

						if (pVisualSampleEntry->type == 'hvc1' || pVisualSampleEntry->type == 'hev1' || pVisualSampleEntry->type == 'hvcC')
						{
							ISOMediaFile::HEVCSampleEntry* pHEVCSampleEntry = (ISOMediaFile::HEVCSampleEntry*)pVisualSampleEntry;
							printf("Coding name: '%c%c%c%c'\n", pVisualSampleEntry->type >> 24, (pVisualSampleEntry->type >> 16) & 0xFF, (pVisualSampleEntry->type >> 8) & 0xFF, pVisualSampleEntry->type & 0xFF);
							
							auto config = pHEVCSampleEntry->config;
							printf("configurationVersion: %d\n", config->HEVCConfig->configurationVersion);
							printf("general_profile_space: %d\n", config->HEVCConfig->general_profile_space);
							printf("general_tier_flag: %d\n", config->HEVCConfig->general_tier_flag);
							printf("general_profile_idc: %d\n", config->HEVCConfig->general_profile_idc);
							printf("general_profile_compatibility_flags: 0X%X\n", config->HEVCConfig->general_profile_compatibility_flags);
							printf("general_constraint_indicator_flags: 0X%llX\n", config->HEVCConfig->general_constraint_indicator_flags);
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
								printf("\tNAL_unit_type[%d]: %d (%s)\n", j, config->HEVCConfig->nalArray[j]->NAL_unit_type, h265_nal_unit_type_names[config->HEVCConfig->nalArray[j]->NAL_unit_type]);
								printf("\tnumNalus[%d]: %d\n", j, config->HEVCConfig->nalArray[j]->numNalus);
								for (int k = 0; k < config->HEVCConfig->nalArray[j]->numNalus; k++)
								{
									printf("\t\tnalUnitLength[%d][%d]: %d\n", j, k, config->HEVCConfig->nalArray[j]->Nalus[k]->nalUnitLength);
									printf("\t\t      00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F\n");
									printf("\t\t      ----------------------------------------------------------------\n");
									for (int idx = 0; idx < config->HEVCConfig->nalArray[j]->Nalus[k]->nalUnitLength; idx++)
									{
										if (idx % 16 == 0)
											printf("\t\t %03X  ", idx);

										printf("%02X  ", config->HEVCConfig->nalArray[j]->Nalus[k]->nalUnit[idx]);
										if ((idx + 1) % 8 == 0)
											printf("  ");

										if ((idx + 1) % 16 == 0)
											printf("\n");
									}

									printf("\n\n");
								}
							}
						}
					}
				}
			}
			else if (box_type == 'sdtp')
			{
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDependencyTypeBox* pSampleDepTypeBox
					= (ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDependencyTypeBox*)ptr_box;
				ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox* pSampleTableBox
					= (ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox*)ptr_box->container;

				if (pSampleTableBox != nullptr && pSampleTableBox->sample_size_box != nullptr)
				{
					uint32_t sample_count = pSampleTableBox->sample_size_box->sample_count;
					printf("== Sample ID === Leading ===== Depends On ==== Depended On ==== Redundancy ==\n");
					for (uint32_t i = 0; i < sample_count; i++)
					{
						printf("   #% -10d      %d         %s        %s         %s\n", 
							i+1,
							pSampleDepTypeBox->entries[i].is_leading,
							pSampleDepTypeBox->entries[i].sample_depends_on == 1 ? "Non-I-Frame" : (pSampleDepTypeBox->entries[i].sample_depends_on == 2 ? "    I-Frame" : "    Unknown"),
							pSampleDepTypeBox->entries[i].sample_is_depended_on == 1 ? "    Yes" : (pSampleDepTypeBox->entries[i].sample_is_depended_on == 2 ? "     No" : "Unknown"),
							pSampleDepTypeBox->entries[i].sample_has_redundancy == 1 ? "    Yes" : (pSampleDepTypeBox->entries[i].sample_has_redundancy == 2 ? "     No" : "Unknown"));
					}
				}
			}
		}
	}
#endif
	return iRet;
}

int DumpMP4OneStream(ISOMediaFile::Box* root_box)
{
	int iRet = -1;

	return iRet;
}

int DumpMP4Partial(ISOMediaFile::Box* root_box)
{
	return -1;
}

int DumpMP4()
{
	int iRet = -1;

	CFileBitstream bs(g_params["input"].c_str(), 4096, &iRet);

	ISOMediaFile::Box* root_box = ISOMediaFile::Box::RootBox();
	while (ISOMediaFile::Box::LoadBoxes(root_box, bs) >= 0);

	if (g_params.find("showinfo") != g_params.end())
	{
		iRet = ShowBoxInfo(root_box);
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
	else if (g_params.find("trackid") != g_params.end())
	{
		if (g_params.find("outputfmt") == g_params.end())
			g_params["outputfmt"] = "es";

		std::string& str_output_fmt = g_params["outputfmt"];
		std::string& str_trackid = g_params["trackid"];

		if ((str_output_fmt.compare("es") == 0 || str_output_fmt.compare("pes") == 0 || str_output_fmt.compare("wav") == 0 || str_output_fmt.compare("pcm") == 0))
		{
			iRet = DumpMP4OneStream(root_box);
		}
		else if (str_output_fmt.compare("ts") == 0 || str_output_fmt.compare("m2ts") == 0)
		{
			iRet = DumpMP4Partial(root_box);
		}
	}

	return iRet;
}
