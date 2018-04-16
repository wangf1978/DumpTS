#include "stdafx.h"
#include "Matroska.h"

namespace Matroska
{
	EBML_ELEMENT_DESCRIPTOR EBML_element_descriptors[] = {
		/* EBML Header */
		{
			"EBML", 0x1A45DFA3, 0,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"EBMLVersion", 0x4286, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"EBMLReadVersion", 0x42F7, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"EBMLMaxIDLength", 0x42F2, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"EBMLMaxSizeLength", 0x42F3, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"DocType", 0x4282, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,1,0,0
		},
		{
			"DocTypeVersion", 0x4287, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"DocTypeReadVersion", 0x4285, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		/* Global elements (used everywhere in the format) */
		{
			"Void", 0xEC, 0xFF,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_BINARY,0,1,0
		},
		{
			"CRC-32", 0xBF, 0xFF,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		/* Segment */
		{
			"Segment", 0x18538067, 0,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,0,0
		},
		/* Meta Seek Information */
		{
			"SeekHead", 0x114D9B74, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"Seek", 0x4DBB, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"SeekID", 0x53AB, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_BINARY,1,0,0
		},
		{
			"SeekPosition", 0x53AC, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		/* Segment Information */
		{
			"Info", 0x1549A966, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"SegmentUID", 0x73A4, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"SegmentFilename", 0x7384, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"PrevUID", 0x3CB923, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"PrevFilename", 0x3C83AB, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"NextUID", 0x3EB923, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"NextFilename", 0x3E83BB, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"SegmentFamily", 0x4444, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,1,0
		},
		{
			"ChapterTranslate", 0x6924, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,1,0
		},
		{
			"ChapterTranslateEditionUID", 0x69FC, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},
		{
			"ChapterTranslateCodec", 0x69BF, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapterTranslateID", 0x69A5, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,1,0,0
		},

		{
			"TimecodeScale", 0x2AD7B1, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"Duration", 0x4489, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"DateUTC", 0x4461, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_DATE,0,0,0
		},
		{
			"Title", 0x7BA9, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"MuxingApp", 0x4D80, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,1,0,0
		},
		{
			"WritingApp", 0x5741, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,1,0,0
		},

		/* Cluster */
		{
			"Cluster", 0x1F43B675, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"Timecode", 0xE7, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"SilentTracks", 0x5854, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"SilentTrackNumber", 0x58D7, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},
		{
			"Position", 0xA7, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"PrevSize", 0xAB, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"SimpleBlock", 0xA3, 2,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_BINARY,0,1,0
		},
		{
			"BlockGroup", 0xA0, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"Block", 0xA1, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_BINARY,1,0,0
		},
		{
			"BlockAdditions", 0x75A1, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"BlockMore", 0xA6, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,1,1,0
		},
		{
			"BlockAddID", 0xEE, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},

		{
			"BlockAdditional", 0xA5, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"BlockDuration", 0x9B, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ReferencePriority", 0xFA, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ReferenceBlock", 0xFB, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_SIGNED_INTEGER,0,1,0
		},
		{
			"CodecState", 0xA4, 3,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"DiscardPadding", 0x75A2, 3,
			EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_SIGNED_INTEGER,0,0,0
		},
		{
			"Slices", 0x8E, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,0,0
		},
		{
			"TimeSlice", 0xE8, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"LaceNumber", 0xCC, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},

		/* Track */
		{
			"Tracks", 0x1654AE6B, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"TrackEntry", 0xAE, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"TrackNumber", 0xD7, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackUID", 0x73C5, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackType", 0x83, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"FlagEnabled", 0xB9, 3,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"FlagDefault", 0x88, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"FlagForced", 0x55AA, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"FlagLacing", 0x9C, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"MinCache", 0x6DE7, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"MaxCache", 0x6DF8, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"DefaultDuration", 0x23E383, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"DefaultDecodedFieldDuration", 0x234E7A, 3,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"MaxBlockAdditionID", 0x55EE, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"Name", 0x536E, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"Language", 0x22B59C, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,0,0,0
		},
		{
			"CodecID", 0x86, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,1,0,0
		},
		{
			"CodecPrivate", 0x63A2, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_BINARY,0,0,0
		},

		{
			"CodecName", 0x258688, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"AttachmentLink", 0x7446, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"CodecDecodeAll", 0xAA, 3,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackOverlay", 0x6FAB, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},
		{
			"CodecDelay", 0x56AA, 3,
			EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"SeekPreRoll", 0x56BB, 3,
			EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackTranslate", 0x6624, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,1,0
		},
		{
			"TrackTranslateEditionUID", 0x66FC, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},

		{
			"TrackTranslateCodec", 0x66BF, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackTranslateTrackID", 0x66A5, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,1,0,0
		},
		{
			"Video", 0xE0, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,0,0
		},
		{
			"FlagInterlaced", 0x9A, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"FieldOrder", 0x9D, 4,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"StereoMode", 0x53B8, 4,
			EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"AlphaMode", 0x53C0, 4,
			EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"PixelWidth", 0xB0, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},

		{
			"PixelHeight", 0xBA, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"PixelCropBottom", 0x54AA, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"PixelCropTop", 0x54BB, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"PixelCropLeft", 0x54CC, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"PixelCropRight", 0x54DD, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"DisplayWidth", 0x54B0, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"DisplayHeight", 0x54BA, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"DisplayUnit", 0x54B2, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},

		{
			"AspectRatioType", 0x54B3, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ColourSpace", 0x2EB524, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"Colour", 0x55B0, 4,
			EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"MatrixCoefficients", 0x55B1, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"BitsPerChannel", 0x55B2, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChromaSubsamplingHorz", 0x55B3, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChromaSubsamplingVert", 0x55B4, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"CbSubsamplingHorz", 0x55B5, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},

		{
			"CbSubsamplingVert", 0x55B6, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChromaSitingHorz", 0x55B7, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChromaSitingVert", 0x55B8, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"Range", 0x55B9, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"TransferCharacteristics", 0x55BA, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"Primaries", 0x55BB, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"MaxCLL", 0x55BC, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"MaxFALL", 0x55BD, 5,
			EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},

		{
			"MasteringMetadata", 0x55D0, 5,
			EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"PrimaryRChromaticityX", 0x55D1, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"PrimaryRChromaticityY", 0x55D2, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"PrimaryGChromaticityX", 0x55D3, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"PrimaryGChromaticityY", 0x55D4, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"PrimaryBChromaticityX", 0x55D5, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"PrimaryBChromaticityY", 0x55D6, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"WhitePointChromaticityX", 0x55D7, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},


		{
			"WhitePointChromaticityY", 0x55D8, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"LuminanceMax", 0x55D9, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"LuminanceMin", 0x55DA, 6,
			EBML_VER_4,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"Audio", 0xE1, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,0,0
		},
		{
			"SamplingFrequency", 0xB5, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_FLOAT,1,0,0
		},
		{
			"OutputSamplingFrequency", 0x78B5, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_FLOAT,0,0,0
		},
		{
			"Channels", 0x9F, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"BitDepth", 0x6264, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},

		{
			"TrackOperation", 0xE2, 3,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"TrackCombinePlanes", 0xE3, 4,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"TrackPlane", 0xE4, 5,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,1,1,0
		},
		{
			"TrackPlaneUID", 0xE5, 6,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackPlaneType", 0xE6, 6,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TrackJoinBlocks", 0xE9, 4,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"TrackJoinUID", 0xED, 5,
			EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,1,0
		},
		{
			"ContentEncodings", 0x6D80, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},

		{
			"ContentEncoding", 0x6240, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,1,1,0
		},
		{
			"ContentEncodingOrder", 0x5031, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ContentEncodingScope", 0x5032, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ContentEncodingType", 0x5033, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ContentCompression", 0x5034, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"ContentCompAlgo", 0x4254, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ContentCompSettings", 0x4255, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"ContentEncryption", 0x5035, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},

		{
			"ContentEncAlgo", 0x47E1, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ContentEncKeyID", 0x47E2, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"ContentSignature", 0x47E3, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"ContentSigKeyID", 0x47E4, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"ContentSigAlgo", 0x47E5, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ContentSigHashAlgo", 0x47E6, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},

		/* Cueing Data */
		{
			"Cues", 0x1C53BB6B, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,0,0
		},
		{
			"CuePoint", 0xBB, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"CueTime", 0xB3, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"CueTrackPositions", 0xB7, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"CueTrack", 0xF7, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"CueClusterPosition", 0xF1, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"CueRelativePosition", 0xF0, 4,
			EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"CueDuration", 0xB2, 4,
			EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"CueBlockNumber", 0x5378, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"CueCodecState", 0xEA, 4,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"CueReference", 0xDB, 4,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,1,0
		},
		{
			"CueRefTime", 0x96, 5,
			EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},

		/* Attachment */
		{
			"Attachments", 0x1941A469, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"AttachedFile", 0x61A7, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,1,1,0
		},
		{
			"FileDescription", 0x467E, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"FileName", 0x466E, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UTF8_STRING,1,0,0
		},
		{
			"FileMimeType", 0x4660, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_ASCII_STRING,1,0,0
		},
		{
			"FileData", 0x465C, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,1,0,0
		},
		{
			"FileUID", 0x46AE, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},

		/* Chapters */
		{
			"Chapters", 0x1043A770, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,0,0
		},
		{
			"EditionEntry", 0x45B9, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"EditionUID", 0x45BC, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"EditionFlagHidden", 0x45BD, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"EditionFlagDefault", 0x45DB, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"EditionFlagOrdered", 0x45DD, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChapterAtom", 0xB6, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"ChapterUID", 0x73C4, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapterStringUID", 0x5654, 4,
			EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"ChapterTimeStart", 0x91, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapterTimeEnd", 0x92, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChapterFlagHidden", 0x98, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapterFlagEnabled", 0x4598, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapterSegmentUID", 0x6E67, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"ChapterSegmentEditionUID", 0x6EBC, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChapterPhysicalEquiv", 0x63C3, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"ChapterTrack", 0x8F, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,0,0
		},
		{
			"ChapterTrackNumber", 0x89, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,1,0
		},

		{
			"ChapterDisplay", 0x80, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"ChapString", 0x85, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,1,0,0
		},
		{
			"ChapLanguage", 0x437C, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,1,1,0
		},
		{
			"ChapCountry", 0x437E, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,0,1,0
		},
		{
			"ChapProcess", 0x6944, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,1,0
		},
		{
			"ChapProcessCodecID", 0x6955, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapProcessPrivate", 0x450D, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,0,0,0
		},
		{
			"ChapProcessCommand", 0x6911, 5,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_MASTER,0,1,0
		},

		{
			"ChapProcessTime", 0x6922, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"ChapProcessData", 0x6933, 6,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_BINARY,1,0,0
		},

		/* Tagging */
		{
			"Tags", 0x1254C367, 1,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,0,1,0
		},
		{
			"Tag", 0x7373, 2,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"Targets", 0x63C0, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,0,0
		},
		{
			"TargetTypeValue", 0x68CA, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,0,0
		},
		{
			"TargetType", 0x63CA, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,0,0,0
		},
		{
			"TagTrackUID", 0x63C5, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},

		{
			"TagEditionUID", 0x63C9, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},
		{
			"TagChapterUID", 0x63C4, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},
		{
			"TagAttachmentUID", 0x63C6, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4,
			EBML_DT_UNSIGNED_INTEGER,0,1,0
		},
		{
			"SimpleTag", 0x67C8, 3,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_MASTER,1,1,0
		},
		{
			"TagName", 0x45A3, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,0,0,0
		},
		{
			"TagLanguage", 0x447A, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_ASCII_STRING,1,0,0
		},
		{
			"TagDefault", 0x4484, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UNSIGNED_INTEGER,1,0,0
		},
		{
			"TagString", 0x4487, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_UTF8_STRING,1,0,0
		},

		{
			"TagBinary", 0x4485, 4,
			EBML_VER_1 | EBML_VER_2 | EBML_VER_3 | EBML_VER_4 | EBML_VER_WEBM,
			EBML_DT_BINARY,0,0,0
		},
	};

	uint32_t g_level_0_element_IDs[] = {
		0x1A45DFA3, 0x18538067
	};

	uint32_t g_global_element_IDs[] = {
		0xEC, 0xBF
	};

	std::unordered_map<uint32_t, size_t>	g_mapIDDesc;
	std::unordered_map<uint32_t, size_t>	g_mapLevel0IDDesc;
	std::unordered_map<uint32_t, size_t>	g_mapGlobalIDDesc;
	std::unordered_map<uint32_t, uint32_t>	g_mapParentIDs;

	RootElement* EBMLElement::Root()
	{
		static RootElement root;
		return &root;
	}

	int32_t EBMLElement::GetDescIdx(uint32_t ElementID)
	{
		if (g_mapIDDesc.find(ElementID) == g_mapIDDesc.end())
			return -1;

		return (int32_t)g_mapIDDesc[ElementID];
	}

	inline void InitializeElementIDAndDescriptorMap()
	{
		if (g_mapIDDesc.size() > 0)
			return;

		for (size_t i = 0; i < _countof(EBML_element_descriptors); i++)
			g_mapIDDesc[EBML_element_descriptors[i].EBML_ID] = i;

		for (size_t i = 0; i < _countof(g_level_0_element_IDs); i++)
			g_mapLevel0IDDesc[g_level_0_element_IDs[i]] = g_mapIDDesc[g_level_0_element_IDs[i]];

		for (size_t i = 0; i < _countof(g_global_element_IDs); i++)
			g_mapGlobalIDDesc[g_global_element_IDs[i]] = g_mapIDDesc[g_global_element_IDs[i]];

		int32_t level = 0;
		std::vector<uint32_t> parent_IDs = {INVALID_EBML_ID};
		for (size_t i = 0; i < _countof(EBML_element_descriptors); i++)
		{
			if (EBML_element_descriptors[i].Level == -1)
				continue;

			// Level is from lower to bigger
			if (EBML_element_descriptors[i].Level < level)
			{
				for (int32_t idxLevel = EBML_element_descriptors[i].Level; idxLevel < level; idxLevel++)
				{
					for (int l = 0; l < EBML_element_descriptors[g_mapIDDesc[parent_IDs.back()]].Level; l++)
						printf("  ");
					printf("<--%08Xh %s\n", EBML_element_descriptors[g_mapIDDesc[parent_IDs.back()]].EBML_ID, EBML_element_descriptors[g_mapIDDesc[parent_IDs.back()]].Element_Name);
					parent_IDs.pop_back();
				}
			}

			g_mapParentIDs[EBML_element_descriptors[i].EBML_ID] = parent_IDs.back();

			// Enter into the new master element
			if (EBML_element_descriptors[i].data_type == EBML_DT_MASTER)
			{
				for (int l = 0; l < EBML_element_descriptors[i].Level; l++)
					printf("  ");
				printf("-->%08Xh %s\n", EBML_element_descriptors[i].EBML_ID, EBML_element_descriptors[i].Element_Name);
				parent_IDs.push_back(EBML_element_descriptors[i].EBML_ID);
			}
			else
			{
				for (int l = 0; l < EBML_element_descriptors[i].Level; l++)
					printf("  ");
				printf("---%08Xh %s\n", EBML_element_descriptors[i].EBML_ID, EBML_element_descriptors[i].Element_Name);
			}

			level = EBML_element_descriptors[i].Level;
		}
	}

	int EBMLElement::LoadEBMLElements(EBMLElement* pContainer, CBitstream& bs, EBMLElement** ppElement)
	{
		int iRet = 0;
		EBMLElement* ptr_element = NULL;

		InitializeElementIDAndDescriptorMap();

		try
		{
			uint8_t nLeadingZeros = 0;

			uint32_t u32ID = (uint32_t)bs.PeekBits(8);
			for (uint8_t i = 0; i < 4; i++)
				if ((u32ID&(1 << (7 - i))) == 0)
					nLeadingZeros++;
				else
					break;

			if (nLeadingZeros >= 4)	// Unexpected
				return -1;

			u32ID = (uint32_t)bs.PeekBits((nLeadingZeros + 1) << 3);

			int32_t idxDesc = -1;

			// Find the element ID of global elements
			if (g_mapGlobalIDDesc.find(u32ID) != g_mapGlobalIDDesc.end())
				idxDesc = g_mapGlobalIDDesc[u32ID];
			else
			{
				if (pContainer->IsRoot())
				{
					// Find the element ID of level-0
					if (g_mapLevel0IDDesc.find(u32ID) != g_mapLevel0IDDesc.end())
						idxDesc = g_mapLevel0IDDesc[u32ID];
					else
						printf("[Matroska] Unexpected element_ID: 0X%X, it should be a level-0 or global element.\n", u32ID);
				}
				else
				{
					// Check the container's level
					int32_t desc_idx = pContainer->GetDescIdx(pContainer->ID);
					int32_t levelContainer = EBML_element_descriptors[desc_idx].Level;
					if (g_mapIDDesc.find(u32ID) != g_mapIDDesc.end())
					{
						idxDesc = g_mapIDDesc[u32ID];
						int32_t levelChild = EBML_element_descriptors[idxDesc].Level;
						if (levelChild != levelContainer + 1)
						{
							printf("[Matroska] Unexpected element_ID: 0X%X, its level: %d is not equal to its master element level: %d + 1.\n",
								u32ID, levelChild, levelContainer);
							idxDesc = -1;
						}
						else if (g_mapParentIDs.find(u32ID) == g_mapParentIDs.end())
						{
							printf("[Matroska] Unexpected element ID: 0X%X, it should NOT be sub-element of EBML ID 0X%X.\n", u32ID, pContainer->ID);
						}
						else if (g_mapParentIDs[u32ID] != pContainer->ID)
						{
							printf("[Matroska] Unexpected element ID: 0X%X, its parent EBML ID 0X%X is not 0X%X.\n", u32ID, g_mapParentIDs[u32ID], pContainer->ID);
						}
					}
					else
						printf("[Matroska] Unexpected element_ID: 0X%X.\n", u32ID);
				}
			}

			if (idxDesc >= 0)
			{
				switch (EBML_element_descriptors[idxDesc].data_type)
				{
				case EBML_DT_SIGNED_INTEGER:
					ptr_element = new SignedIntegerElement();
					break;
				case EBML_DT_UNSIGNED_INTEGER:
					ptr_element = new UnsignedIntegerElement();
					break;
				case EBML_DT_FLOAT:
					ptr_element = new FloatElement();
					break;
				case EBML_DT_ASCII_STRING:
					ptr_element = new ASCIIStringElement();
					break;
				case EBML_DT_UTF8_STRING:
					ptr_element = new UTF8StringElement();
					break;
				case EBML_DT_DATE:
					ptr_element = new DateElement();
					break;
				case EBML_DT_MASTER:
					ptr_element = new MasterElement();
					break;
				case EBML_DT_BINARY:
				{
					switch (u32ID)
					{
					case 0xA3:
						ptr_element = new SimpleBlockElement();
						break;
					case 0x63A2:
						ptr_element = new CodecPrivateElement();
						break;
					default:
						ptr_element = new BinaryElement();
					}
				}
					break;
				default:
					ptr_element = new UnknownEBMLElement();
				}
			}
			else
				ptr_element = new UnknownEBMLElement();

			pContainer->AppendChild(ptr_element);

			if ((iRet = ptr_element->Unpack(bs)) < 0)
			{
				printf("[Matroska] Failed to load a EBML element and its children from the current bitstream.\n");

				AMP_SAFEASSIGN(ppElement, nullptr);
				pContainer->RemoveChild(ptr_element);

				return iRet;
			}

			AMP_SAFEASSIGN(ppElement, ptr_element);

		}
		catch (std::exception& e)
		{
			printf("exception: %s\r\n", e.what());
			return -1;
		}

		return 0;
	}

	void EBMLElement::UnloadEBMLElements(EBMLElement* pElement)
	{

	}

	// @retval -1, failed to find the element
	// @retval 0, find one element meet the condition 
	// @retval 1, cease the recursive find
	int EBMLElement::FindEBMLElementByElementID(uint32_t element_id, uint32_t track_id, std::vector<EBMLElement*>& result)
	{
		int iRet = -1;
		Matroska::EBMLElement* ptr_child = first_child;

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

			if (ptr_child->FindEBMLElementByElementID(element_id, track_id, result) == 1)
				return 1;

			ptr_child = ptr_child->next_sibling;
		}

		return iRet;
	}

	// @retval -1, failed to find the element
	// @retval 0, find one element meet the condition 
	// @retval 1, cease the recursive find
	int EBMLElement::FindEBMLElementByTrackID(uint32_t track_id, uint32_t element_id, std::vector<EBMLElement*>& result)
	{
		int iRet = -1;
		Matroska::EBMLElement* ptr_child = first_child;

		while (ptr_child != nullptr)
		{
			if (track_id != UINT32_MAX && ptr_child->ID == 0xD7 && ((Matroska::UnsignedIntegerElement*)ptr_child)->uVal == track_id)
			{
				if (element_id == UINT32_MAX)
					result.push_back(ptr_child->container);
				else
					// From the current Track to find the element_id
					ptr_child->container->FindEBMLElementByElementID(element_id, UINT32_MAX, result);

				return 1;
			}

			if (ptr_child->FindEBMLElementByTrackID(track_id, element_id, result) == 1)
				return 1;

			ptr_child = ptr_child->next_sibling;
		}

		return iRet;
	}

} // namespace Matroska