#include "stdafx.h"
#include "ISO14496_12.h"
#include "ISO14496_15.h"
#include "QTFF.h"

#define IS_PRINT_BOXTYPE(x)	(isprint((int)(((x)>>24)&0xFF)) && isprint((int)(((x)>>16)&0xFF)) && isprint((int)(((x)>>8)&0xFF)) && isprint((int)((x)&0xFF)))

namespace ISOBMFF
{
	std::unordered_map<uint32_t, const char*> box_desces = {
		{ 'ftyp', "file type and compatibility" },
		{ 'pdin', "progressive download information" },
		{ 'moov', "container for all the metadata" },
		{ 'mvhd', "movie header, overall declarations" },
		{ 'trak', "container for an individual track or stream" },
		{ 'tkhd', "track header, overall information about the track" },
		{ 'tref', "track reference container" },
		{ 'trgr', "track grouping indication" },
		{ 'edts', "edit list container" },
		{ 'elst', "an edit list" },
		{ 'mdia', "container for the media information in a track" },
		{ 'mdhd', "media header, overall information about the media" },
		{ 'hdlr', "handler, declares the media (handler) type" },
		{ 'minf', "media information container" },
		{ 'vmhd', "video media header, overall information (video track only)" },
		{ 'smhd', "sound media header, overall information (sound track only)" },
		{ 'hmhd', "hint media header, overall information (hint track only)" },
		{ 'nmhd', "Null media header, overall information (some tracks only)" },
		{ 'dinf', "data information box, container" },
		{ 'dref', "data reference box, declares source(s) of media data in track" },
		{ 'stbl', "sample table box, container for the time/space map" },
		{ 'stsd', "sample descriptions (codec types, initialization etc.)" },
		{ 'stts', "(decoding) time-to-sample" },
		{ 'ctts', "(composition) time to sample" },
		{ 'cslg', "composition to decode timeline mapping" },
		{ 'stsc', "sample-to-chunk, partial data-offset information" },
		{ 'stsz', "sample sizes (framing)" },
		{ 'stz2', "compact sample sizes (framing)" },
		{ 'stco', "chunk offset, partial data-offset information" },
		{ 'co64', "64-bit chunk offset" },
		{ 'stss', "sync sample table" },
		{ 'stsh', "shadow sync sample table" },
		{ 'padb', "sample padding bits" },
		{ 'stdp', "sample degradation priority" },
		{ 'sdtp', "independent and disposable samples" },
		{ 'sbgp', "sample-to-group" },
		{ 'sgpd', "sample group description" },
		{ 'subs', "sub-sample information" },
		{ 'saiz', "sample auxiliary information sizes" },
		{ 'saio', "sample auxiliary information offsets" },
		{ 'udta', "user-data" },
		{ 'mvex', "movie extends box" },
		{ 'mehd', "movie extends header box" },
		{ 'trex', "track extends defaults" },
		{ 'leva', "level assignment" },
		{ 'moof', "movie fragment" },
		{ 'mfhd', "movie fragment header" },
		{ 'traf', "track fragment" },
		{ 'tfhd', "track fragment header" },
		{ 'trun', "track fragment run" },
		//{ 'sbgp', "sample-to-group" },	//Duplicated
		//{ 'sgpd', "sample group description" },	//Duplicated
		//{ 'subs', "sub-sample information" },	//Duplicated
		//{ 'saiz', "sample auxiliary information sizes" },	//Duplicated
		//{ 'saio', "sample auxiliary information offsets" },	//Duplicated
		{ 'tfdt', "track fragment decode time" },
		{ 'mfra', "movie fragment random access" },
		{ 'tfra', "track fragment random access" },
		{ 'mfro', "movie fragment random access offset" },
		{ 'mdat', "media data container" },
		{ 'free', "free space" },
		{ 'skip', "free space" },
		//{ 'udta', "user-data" },	//Duplicated
		{ 'cprt', "copyright etc." },
		{ 'tsel', "track selection box" },
		{ 'strk', "sub track box" },
		{ 'stri', "sub track information box" },
		{ 'strd', "sub track definition box" },
		{ 'meta', "metadata" },
		//{ 'hdlr', "handler, declares the metadata (handler) type" },	//Duplicated
		//{ 'dinf', "data information box, container" },	//Duplicated
		//{ 'dref', "data reference box, declares source(s) of metadata items" },	//Duplicated
		{ 'iloc', "item location" },
		{ 'ipro', "item protection" },
		{ 'sinf', "protection scheme information box" },
		{ 'frma', "original format box" },
		{ 'schm', "scheme type box" },
		{ 'schi', "scheme information box" },
		{ 'iinf', "item information" },
		{ 'xml ', "XML container" },
		{ 'bxml', "binary XML container" },
		{ 'pitm', "primary item reference" },
		{ 'fiin', "file delivery item information" },
		{ 'paen', "partition entry" },
		{ 'fire', "file reservoir" },
		{ 'fpar', "file partition" },
		{ 'fecr', "FEC reservoir" },
		{ 'segr', "file delivery session group" },
		{ 'gitn', "group id to name" },
		{ 'idat', "item data" },
		{ 'iref', "item reference" },
		{ 'meco', "additional metadata container" },
		{ 'mere', "metabox relation" },
		{ 'styp', "segment type" },
		{ 'sidx', "segment index" },
		{ 'ssix', "subsegment index" },
		{ 'prft', "producer reference time" },
		{ 'rinf', "Restricted Scheme Information" }
	};

	std::unordered_map<uint32_t, const char*> handle_type_names = {
		{ 'soun', "Audio" },
		{ 'vide', "Video" },
		{ 'hint', "Hint" },
		{ 'meta', "Metadata" },
		{ 'avc1', "MPEG4-AVC" },
		{ 'avc2', "MPEG4-AVC" },
		{ 'avc3', "MPEG4-AVC" },
		{ 'avc4', "MPEG4-AVC" },
		{ 'hvc1', "HEVC" },
		{ 'hev1', "HEVC" },
		{ 'alis', "file alias" }
	};

	BOX_DESCRIPTOR box_descriptors[] = {
		{ 'ftyp', 1, 0 },
		{ 'pdin', 0, 0 },
		{ 'moov', 1, 0 },
		{ 'mvhd', 1, 1 },
		{ 'trak', 1, 1 },
		{ 'tkhd', 1, 2 },
		{ 'tref', 0, 2 },
		{ 'trgr', 0, 2 },
		{ 'edts', 0, 2 },
		{ 'elst', 0, 3 },
		{ 'mdia', 1, 2 },
		{ 'mdhd', 1, 3 },
		{ 'hdlr', 1, 3 },
		{ 'minf', 1, 3 },
		{ 'vmhd', 0, 4 },
		{ 'smhd', 0, 4 },
		{ 'hmhd', 0, 4 },
		{ 'nmhd', 0, 4 },
		{ 'dinf', 1, 4 },
		{ 'dref', 1, 5 },
		{ 'stbl', 1, 4 },
		{ 'stsd', 1, 5 },
		{ 'stts', 1, 5 },
		{ 'ctts', 0, 5 },
		{ 'cslg', 0, 5 },
		{ 'stsc', 1, 5 },
		{ 'stsz', 0, 5 },
		{ 'stz2', 0, 5 },
		{ 'stco', 1, 5 },
		{ 'co64', 0, 5 },
		{ 'stss', 0, 5 },
		{ 'stsh', 0, 5 },
		{ 'padb', 0, 5 },
		{ 'stdp', 0, 5 },
		{ 'sdtp', 0, 5 },
		{ 'sbgp', 0, 5 },
		{ 'sgpd', 0, 5 },
		{ 'subs', 0, 5 },
		{ 'saiz', 0, 5 },
		{ 'saio', 0, 5 },
		{ 'udta', 0, 2 },
		{ 'mvex', 0, 1 },
		{ 'mehd', 0, 2 },
		{ 'trex', 1, 2 },
		{ 'leva', 0, 2 },
		{ 'moof', 0, 0 },
		{ 'mfhd', 1, 1 },
		{ 'traf', 0, 1 },
		{ 'tfhd', 1, 2 },
		{ 'trun', 0, 2 },
		{ 'sbgp', 0, 2 },
		{ 'sgpd', 0, 2 },
		{ 'subs', 0, 2 },
		{ 'saiz', 0, 2 },
		{ 'saio', 0, 2 },
		{ 'tfdt', 0, 2 },
		{ 'mfra', 0, 0 },
		{ 'tfra', 0, 1 },
		{ 'mfro', 1, 1 },
		{ 'mdat', 0, 0 },
		{ 'free', 0, 0 },
		{ 'skip', 0, 0 },
		{ 'udta', 0, 1 },
		{ 'cprt', 0, 2 },
		{ 'tsel', 0, 2 },
		{ 'strk', 0, 2 },
		{ 'stri', 0, 3 },
		{ 'strd', 0, 3 },
		{ 'meta', 0, 0 },
		{ 'hdlr', 1, 1 },
		{ 'dinf', 0, 1 },
		{ 'dref', 0, 2 },
		{ 'iloc', 0, 1 },
		{ 'ipro', 0, 1 },
		{ 'sinf', 0, 2 },
		{ 'frma', 0, 3 },
		{ 'schm', 0, 3 },
		{ 'schi', 0, 3 },
		{ 'iinf', 0, 1 },
		{ 'xml ', 0, 1 },
		{ 'bxml', 0, 1 },
		{ 'pitm', 0, 1 },
		{ 'fiin', 0, 1 },
		{ 'paen', 0, 2 },
		{ 'fire', 0, 3 },
		{ 'fpar', 0, 3 },
		{ 'fecr', 0, 3 },
		{ 'segr', 0, 2 },
		{ 'gitn', 0, 2 },
		{ 'idat', 0, 1 },
		{ 'iref', 0, 1 },
		{ 'meco', 0, 0 },
		{ 'mere', 0, 1 },
		{ 'styp', 0, 0 },
		{ 'sidx', 0, 0 },
		{ 'ssix', 0, 0 },
		{ 'prft', 0, 0 },
	};

	struct BoxTree
	{
		int64_t			level : 8;
		int64_t			mandatory : 1;
		int64_t			reserved : 23;
		int64_t			type : 32;

		BoxTree*		parent = nullptr;
		BoxTree*		first_child = nullptr;
		BoxTree*		next_sibling = nullptr;

		BoxTree():BoxTree(UINT32_MAX, -1, 1) {}
		BoxTree(uint32_t box_type, int8_t box_level, bool bMandatory) 
			:type(box_type), level(box_level), mandatory(bMandatory?1:0) {}

		~BoxTree()
		{
			// delete all its children
			BoxTree* ptr_child = first_child;
			while (ptr_child)
			{
				BoxTree* ptr_front = ptr_child;
				ptr_child = ptr_child->next_sibling;
				delete ptr_front;
			}

			first_child = nullptr;
		}

		void AppendChild(BoxTree* child) noexcept
		{
			// Before appending the child box into the current box, the client must ensure
			// it is has already detached from the original box tree
			assert(child->parent == nullptr && child->next_sibling == nullptr);

			child->parent = this;

			// append the child to the last child
			if (first_child == NULL)
				first_child = child;
			else
			{
				// find the last child
				BoxTree* ptr_child = first_child;
				while (ptr_child->next_sibling)
					ptr_child = ptr_child->next_sibling;

				ptr_child->next_sibling = child;
			}
		}

		BoxTree* FindDescendant(uint32_t box_type)
		{
			if (first_child == nullptr)
				return nullptr;

			BoxTree* ptr_ret = nullptr;
			BoxTree* ptr_child = first_child;
			do
			{
				if (ptr_child->type == (int64_t)box_type)
					return ptr_child;

				if ((ptr_ret = FindDescendant(box_type)) != nullptr)
					return ptr_ret;

				ptr_child = ptr_child->next_sibling;
			} while (ptr_child != nullptr);

			return nullptr;
		}

		int GetMaxTreeWidth(int level)
		{
			if (first_child == nullptr)
			{
				const int indent = 2;
				const int level_span = 5;

				int cur_char_width = indent;

				if (level >= 1)
					cur_char_width += 3 + (level - 1)*level_span;

				if (IS_PRINT_BOXTYPE(type))
					cur_char_width += 4;	//xxxx
				else
					cur_char_width += 9;	//xxxxxxxxh

				return cur_char_width;
			}

			int max_tree_char_width = 0;
			BoxTree* ptr_child = first_child;
			do
			{
				int width = ptr_child->GetMaxTreeWidth(level + 1);
				if (max_tree_char_width < width)
					max_tree_char_width = width;
				ptr_child = ptr_child->next_sibling;
			} while (ptr_child != nullptr);

			return max_tree_char_width;
		}
	};

	void PrintBoxTree(BoxTree* ptr_element, int print_level, int column_widths[])
	{
		size_t line_chars = print_level * 5 + 160;
		char* szLine = new char[line_chars];
		memset(szLine, ' ', line_chars);

		const int indent = 2;
		const int level_span = 5;

		char* szText = nullptr;
		if (print_level >= 1)
		{
			BoxTree* ptr_parent = ptr_element->parent;
			memcpy(szLine + indent + (print_level - 1)*level_span, "|--", 3);
			for (int i = print_level - 2; i >= 0 && ptr_parent != nullptr; i--)
			{
				if (ptr_parent->next_sibling != nullptr)
					memcpy(szLine + indent + i*level_span, "|", 1);
				ptr_parent = ptr_parent->parent;
			}
			szText = szLine + indent + 3 + (print_level - 1)*level_span;
		}
		else
			szText = szLine + indent;

		if (ptr_element->parent == nullptr)
			sprintf_s(szText, line_chars - (szText - szLine), ".\r\n");
		else
		{
			char szTemp[256];
			int nWritePos = 0, cbWritten = -1;
			for (int i = 0; i < 3; i++)
			{
				cbWritten = -1;
				switch (i)
				{
				case 0:
					if (IS_PRINT_BOXTYPE(ptr_element->type))
						cbWritten = sprintf_s(szTemp, _countof(szTemp), "%c%c%c%c",
						(int)((ptr_element->type >> 24) & 0xFF), (int)((ptr_element->type >> 16) & 0xFF), (int)((ptr_element->type >> 8) & 0xFF), (int)((ptr_element->type) & 0xFF));
					else
						cbWritten = sprintf_s(szTemp, _countof(szTemp), "%Xh", (uint32_t)ptr_element->type);
					break;
				case 1:
					cbWritten = sprintf_s(szTemp, _countof(szTemp), "%s", ptr_element->mandatory ? "  *" : "");
					break;
				case 2:
					cbWritten = sprintf_s(szTemp, _countof(szTemp), "%s", box_desces.find(ptr_element->type)==box_desces.end()?"":box_desces[ptr_element->type]);
					break;
				}

				if (cbWritten > 0)
					memcpy(szText, szTemp, strlen(szTemp));

				nWritePos += column_widths[i] + 1;
				szText = szLine + nWritePos;
			}

			sprintf_s(szLine + nWritePos, line_chars - nWritePos, "\r\n");
		}

		printf(szLine);

		delete[] szLine;

		auto ptr_child = ptr_element->first_child;
		while (ptr_child != nullptr)
		{
			PrintBoxTree(ptr_child, print_level + 1, column_widths);
			ptr_child = ptr_child->next_sibling;
		}

		return;
	}

	void PrintISOBMFFBox(int64_t box_type)
	{
		// Construct the tree for print
		BoxTree root;
		BoxTree* pParent = &root;
		int32_t level = 0, max_desc_width = 0;
		for (auto iter = box_desces.cbegin(); iter != box_desces.cend(); iter++)
		{
			int32_t iter_len = (int32_t)strlen(iter->second);
			if (max_desc_width < iter_len)
				max_desc_width = iter_len;
		}

		for (size_t i = 0; i < _countof(box_descriptors); i++)
		{
			int32_t cur_level = box_descriptors[i].level < 0 ? 0 : box_descriptors[i].level;

			// Level is from lower to bigger
			if (cur_level < level)
			{
				for (int32_t idxLevel = cur_level; idxLevel < level; idxLevel++)
					pParent = pParent->parent;
			}

			BoxTree* pCurrent = new BoxTree(box_descriptors[i].box_type, box_descriptors[i].level, box_descriptors[i].mandatory?true:false);
			pParent->AppendChild(pCurrent);

			// Check whether there is a child under it or not
			if ((i + 1) < _countof(box_descriptors) && box_descriptors[i + 1].level > box_descriptors[i].level)
				pParent = pCurrent;

			level = cur_level;
		}

		BoxTree* pShowItem = box_type != -1 ? root.FindDescendant((uint32_t)box_type) : &root;
		if (pShowItem == nullptr)
			return;

		int max_tree_width = pShowItem->GetMaxTreeWidth(0);
		const char* szColumns[] = { "Box Type", "Manda", "Description" };
		int column_widths[] = { AMP_MAX(max_tree_width, (int)strlen(szColumns[0])), 5, AMP_MAX(max_desc_width, (int32_t)strlen(szColumns[2]))};

		// Print Header
		int table_width = 0;
		for (size_t i = 0; i < _countof(column_widths); i++)
			table_width += column_widths[i] + 1;

		char* szLine = new char[table_width + 2];
		memset(szLine, '-', table_width);
		szLine[0] = szLine[1] = ' ';
		szLine[table_width] = '\n';
		szLine[table_width + 1] = '\0';

		int nWritePos = 0;
		for (size_t i = 0; i < _countof(column_widths); i++)
		{
			memcpy(szLine + nWritePos + (column_widths[i] - strlen(szColumns[i])) / 2, szColumns[i], strlen(szColumns[i]));
			nWritePos += column_widths[i] + 1;
		}
		printf(szLine);

		delete[] szLine;

		PrintBoxTree(pShowItem, 0, column_widths);

		return;
	}

	Box* Box::RootBox()
	{
		static Box rootbox(8, 0);
		return &rootbox;
	}

	int Box::LoadBoxes(Box* pContainer, CBitstream& bs, Box** ppBox)
	{
		int iRet = 0;
		Box* ptr_box = NULL;

		try
		{
			uint64_t box_header = bs.PeekBits(64);
			switch ((box_header&UINT32_MAX))
			{
			case 'ftyp':
				ptr_box = new FileTypeBox();
				break;
			case 'pdin':
				ptr_box = new ProgressiveDownloadInfoBox();
				break;
			case 'moov':
				ptr_box = new MovieBox();
				break;
			case 'mvhd':
				ptr_box = new MovieBox::MovieHeaderBox();
				break;
			case 'trak':
				ptr_box = new MovieBox::TrackBox();
				break;
			case 'tkhd':
				ptr_box = new MovieBox::TrackBox::TrackHeaderBox();
				break;
			case 'tref':
				ptr_box = new MovieBox::TrackBox::TrackReferenceBox();
				break;
			case 'trgr':	// Track Group Box
				ptr_box = new MovieBox::TrackBox::TrackGroupBox();
				break;
			case 'edts':	// Edit Box
				ptr_box = new MovieBox::TrackBox::EditBox();
				break;
			case 'elst':
				ptr_box = new MovieBox::TrackBox::EditBox::EditListBox();
				break;
			case 'mdia':
				ptr_box = new MovieBox::TrackBox::MediaBox();
				break;
			case 'mdhd':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaHeaderBox();
				break;
			case 'hdlr':
				ptr_box = new HandlerBox();
				break;
			case 'minf':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox();
				break;
			case 'vmhd':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::VideoMediaHeaderBox();
				break;
			case 'smhd':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SoundMediaHeaderBox();
				break;
			case 'hmhd':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::HintMediaHeaderBox();
				break;
			case 'nmhd':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::NullMediaHeaderBox();
				break;
			case 'dinf':
				ptr_box = new DataInformationBox();
				break;
			case 'dref':
				ptr_box = new DataInformationBox::DataReferenceBox();
				break;
			case 'stbl':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox();
				break;
			case 'stsd':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox();
				break;
			case 'stts':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::TimeToSampleBox();
				break;
			case 'ctts':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompositionOffsetBox();
				break;
			case 'cslg':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompositionToDecodeBox();
				break;
			case 'stsc':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleToChunkBox();
				break;
			case 'stsz':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleSizeBox();
				break;
			case 'stz2':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::CompactSampleSizeBox();
				break;
			case 'stco':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkOffsetBox();
				break;
			case 'co64':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ChunkLargeOffsetBox();
				break;
			case 'stss':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SyncSampleBox();
				break;
			case 'stsh':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::ShadowSyncSampleBox();
				break;
			case 'padb':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::PaddingBitsBox();
				break;
			case 'stdp':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::DegradationPriorityBox();
				break;
			case 'sdtp':
				ptr_box = new MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDependencyTypeBox();
				break;
			case 'sbgp':	// SampleToGroupBox
				ptr_box = new SampleToGroupBox();
				break;
			case 'sgpd':
				ptr_box = new SampleGroupDescriptionBox();
				break;
			case 'subs':
				ptr_box = new SubSampleInformationBox();
				break;
			case 'saiz':
				ptr_box = new SampleAuxiliaryInformationSizesBox();
				break;
			case 'saio':
				ptr_box = new SampleAuxiliaryInformationOffsetsBox();
			case 'udta':
				ptr_box = new UserDataBox();
				break;
			case 'mvex':
				ptr_box = new MovieBox::MovieExtendsBox();
				break;
			case 'mehd':
				ptr_box = new MovieBox::MovieExtendsBox::MovieExtendsHeaderBox();
				break;
			case 'trex':
				ptr_box = new MovieBox::MovieExtendsBox::TrackExtendsBox();
				break;
			case 'leva':
				ptr_box = new MovieBox::MovieExtendsBox::LevelAssignmentBox();
				break;
			case 'moof':
				ptr_box = new MovieFragmentBox();
				break;
			case 'mfhd':
				ptr_box = new MovieFragmentBox::MovieFragmentHeaderBox();
				break;
			case 'traf':
				ptr_box = new MovieFragmentBox::TrackFragmentBox();
				break;
			case 'tfhd':
				ptr_box = new MovieFragmentBox::TrackFragmentBox::TrackFragmentHeaderBox();
				break;
			case 'trun':
				ptr_box = new MovieFragmentBox::TrackFragmentBox::TrackRunBox();
				break;
			case 'tfdt':
				ptr_box = new MovieFragmentBox::TrackFragmentBox::TrackFragmentBaseMediaDecodeTimeBox();
				break;
			case 'mfra':
				ptr_box = new MovieFragmentRandomAccessBox();
				break;
			case 'tfra':
				ptr_box = new MovieFragmentRandomAccessBox::TrackFragmentRandomAccessBox();
				break;
			case 'mfro':
				ptr_box = new MovieFragmentRandomAccessBox::MovieFragmentRandomAccessOffsetBox();
				break;
			case 'mdat':
				ptr_box = new MediaDataBox();
				break;
			case 'free':
				ptr_box = new FreeSpaceBox();
				break;
			case 'skip':
				ptr_box = new FreeSpaceBox();
			case 'cprt':
				ptr_box = new UserDataBox::CopyrightBox();
				break;
			case 'tsel':
				ptr_box = new UserDataBox::TrackSelectionBox();
				break;
			case 'strk':
				ptr_box = new UserDataBox::SubTrack();
				break;
			case 'stri':
				ptr_box = new UserDataBox::SubTrack::SubTrackInformation();
				break;
			case 'strd':
				ptr_box = new UserDataBox::SubTrack::SubTrackDefinition();
				break;
			case 'meta':
				// QT file metadata is different with ISO 14496-12 definition
			{
				Box* ptr_child = RootBox()->first_child;
				while (ptr_child != nullptr && ptr_child->type != 'ftyp')
					ptr_child = ptr_child->next_sibling;

				if (ptr_child != nullptr && ((FileTypeBox*)ptr_child)->major_brand == 'qt  ')
					ptr_box = new QTFF::MetaBox();
				else
					ptr_box = new MetaBox();
			}
			break;
			case 'iloc':
				ptr_box = new MetaBox::ItemLocationBox();
				break;
			case 'ipro':
				ptr_box = new MetaBox::ItemProtectionBox();
				break;
			case 'sinf':
				ptr_box = new ProtectionSchemeInfoBox();
				break;
			case 'frma':
				ptr_box = new OriginalFormatBox();
				break;
			case 'schm':
				ptr_box = new SchemeTypeBox();
				break;
			case 'schi':
				ptr_box = new SchemeInformationBox();
				break;
			case 'iinf':
				ptr_box = new MetaBox::ItemInfoBox();
				break;
			case 'xml ':
				ptr_box = new MetaBox::XMLBox();
				break;
			case 'bxml':
				ptr_box = new MetaBox::BinaryXMLBox();
				break;
			case 'pitm':
				ptr_box = new MetaBox::PrimaryItemBox();
				break;
			case 'fiin':
				ptr_box = new MetaBox::FDItemInformationBox();
				break;
			case 'paen':
				ptr_box = new MetaBox::FDItemInformationBox::PartitionEntry();
				break;
			case 'fire':
				ptr_box = new MetaBox::FDItemInformationBox::PartitionEntry::FileReservoirBox();
				break;
			case 'fpar':
				ptr_box = new MetaBox::FDItemInformationBox::PartitionEntry::FilePartitionBox();
				break;
			case 'fecr':
				ptr_box = new MetaBox::FDItemInformationBox::PartitionEntry::FECReservoirBox();
				break;
			case 'segr':
				ptr_box = new MetaBox::FDItemInformationBox::FDSessionGroupBox();
				break;
			case 'gitn':
				ptr_box = new MetaBox::FDItemInformationBox::GroupIdToNameBox();
				break;
			case 'idat':
				ptr_box = new MetaBox::ItemDataBox();
				break;
			case 'iref':
				ptr_box = new MetaBox::ItemReferenceBox();
				break;
			case 'meco':
				ptr_box = new AdditionalMetadataContainerBox();
				break;
			case 'mere':
				ptr_box = new AdditionalMetadataContainerBox::MetaboxRelationBox();
				break;
			case 'styp':
				/*
				Box Type: 'styp'
				Container: File
				Mandatory: No
				Quantity: Zero or more
				If segments are stored in separate files (e.g. on a standard HTTP server) it is recommended that these
				'segment files' contain a segment-type box, which must be first if present, to enable identification of those files,
				and declaration of the specifications with which they are compliant.

				A segment type has the same format as an 'ftyp' box [4.3], except that it takes the box type 'styp'. The
				brands within it may include the same brands that were included in the 'ftyp' box that preceded the
				'moov' box, and may also include additional brands to indicate the compatibility of this segment with various
				specification(s)
				*/
				ptr_box = new FileTypeBox();
				break;
			case 'sidx':
				ptr_box = new SegmentIndexBox();
				break;
			case 'ssix':
				ptr_box = new SubsegmentIndexBox();
				break;
			case 'prft':
				ptr_box = new ProducerReferenceTimeBox();
				break;
			case 'rinf':
				ptr_box = new RestrictedSchemeInfoBox();
				break;
			default:
				ptr_box = new UnknownBox();
			}

			pContainer->AppendChildBox(ptr_box);

			if ((iRet = ptr_box->Unpack(bs)) < 0)
			{
				printf("Failed to load a box and its children from the current bitstream.\n");

				AMP_SAFEASSIGN(ppBox, nullptr);
				pContainer->RemoveChildBox(ptr_box);

				return iRet;
			}

			AMP_SAFEASSIGN(ppBox, ptr_box);
		}
		catch (std::exception& e)
		{
			printf("exception: %s\r\n", e.what());
			return -1;
		}

		return 0;
	}

	void Box::UnloadBoxes(Box* pBox)
	{

	}

	int MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleDescriptionBox::Unpack(CBitstream& bs)
	{
		int iRet = 0;

		if ((iRet = FullBox::Unpack(bs)) < 0)
			return iRet;

		uint64_t left_bytes = LeftBytes(bs);
		if (left_bytes < sizeof(entry_count))
		{
			SkipLeftBits(bs);
			return RET_CODE_BOX_TOO_SMALL;
		}

		entry_count = bs.GetDWord();
		left_bytes -= sizeof(entry_count);

		// Try to get handler_type from its container
		if (container == nullptr)
		{
			printf("The current 'stsd' box has no container box unexpectedly.\n");
			return RET_CODE_ERROR;
		}

		if (container->type != 'stbl')
		{
			printf("The current 'stsd' box does NOT lie at a 'stbl' box container.\n");
			return RET_CODE_ERROR;
		}

		if (container->container == nullptr || container->container->type != 'minf' ||
			container->container->container == nullptr || container->container->container->type != 'mdia')
		{
			printf("Can't find the 'mdia' ancestor of the current 'stsd' box.\n");
			return RET_CODE_ERROR;
		}

		auto ptr_mdia_container = container->container->container;

		// find hdlr box
		Box* ptr_mdia_child = ptr_mdia_container->first_child;
		while (ptr_mdia_child != nullptr)
		{
			if (ptr_mdia_child->type == 'hdlr')
				break;
			ptr_mdia_child = ptr_mdia_child->next_sibling;
		}

		if (ptr_mdia_child == nullptr)
		{
			printf("Can't find 'hdlr' box for the current 'stsd' box.\n");
			return RET_CODE_ERROR;
		}

		handler_type = dynamic_cast<HandlerBox*>(ptr_mdia_child)->handler_type;

		for (uint32_t i = 0; i < entry_count; i++)
		{
			if (left_bytes < MIN_BOX_SIZE)
				break;

			uint64_t box_header = bs.PeekBits(64);
			
			Box* pBox = nullptr;
			switch (handler_type)
			{
			case 'soun':	// for audio tracks
				pBox = new AudioSampleEntry();
				break;
			case 'vide':	// for video tracks
				switch ((box_header&UINT32_MAX))
				{
				case 'hvc1':
				case 'hev1':
					pBox = new HEVCSampleEntry();
					break;
				case 'avc1':
				case 'avc3':
					pBox = new AVCSampleEntry();
					break;
				case 'avc2':
				case 'avc4':
					pBox = new AVC2SampleEntry();
					break;
				default:
					pBox = new VisualSampleEntry();
				}
				break;
			case 'hint':	// Hint track
				pBox = new HintSampleEntry();
				break;
			case 'meta':	// Metadata track
				pBox = new MetaDataSampleEntry();
				break;

			default:
				pBox = new UnknownBox();
			}

			pBox->container = this;

			pBox->Unpack(bs);

			SampleEntries.push_back(pBox);

			left_bytes -= pBox->size;
		}

		SkipLeftBits(bs);
		return 0;
	}
}
