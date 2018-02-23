#include "stdafx.h"
#include "ISO14496_12.h"
#include "QTFF.h"

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

namespace ISOMediaFile
{
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
}
