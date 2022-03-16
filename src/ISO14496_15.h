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
#pragma once

#include <stdint.h>
#include "Bitstream.h"
#include "combase.h"
#include <algorithm>
#include "ISO14496_12.h"
#include "ISO14496_1.h"
#include "dump_data_type.h"
#include "AMRingBuffer.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4101)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4127)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

#define USE_AVCC_CONFIG(box_type)	(\
	(box_type) == 'avcC' ||\
	(box_type) == 'avc1' ||\
	(box_type) == 'avc2' ||\
	(box_type) == 'avc3' ||\
	(box_type) == 'avc4' ||\
	(box_type) == 'avcp')

#define USE_HVCC_CONFIG(box_type)	(\
	(box_type) == 'hvcC' ||\
	(box_type) == 'hvc1' ||\
	(box_type) == 'hev1' ||\
	(box_type) == 'hvc2' ||\
	(box_type) == 'hev2')

#define IS_HEVC_STREAM(box_type)	(\
	(box_type) == 'hvcC' ||\
	(box_type) == 'hvc1' ||\
	(box_type) == 'hev1' ||\
	(box_type) == 'hvc2' ||\
	(box_type) == 'hev2' ||\
	(box_type) == 'lhv1' ||\
	(box_type) == 'lhe1')

#define IS_AVC_STREAM(box_type)	(\
	(box_type) == 'avcC' ||\
	(box_type) == 'avc1' ||\
	(box_type) == 'avc2' ||\
	(box_type) == 'avc3' ||\
	(box_type) == 'avc4' ||\
	(box_type) == 'avcp' ||\
	(box_type) == 'svc1' ||\
	(box_type) == 'svc2' ||\
	(box_type) == 'svcC' ||\
	(box_type) == 'mvc1' ||\
	(box_type) == 'mvc2' ||\
	(box_type) == 'mvc3' ||\
	(box_type) == 'mvc4' ||\
	(box_type) == 'mvd1' ||\
	(box_type) == 'mvd2' ||\
	(box_type) == 'mvd3' ||\
	(box_type) == 'mvd4' ||\
	(box_type) == 'a3d1' ||\
	(box_type) == 'a3d2' ||\
	(box_type) == 'a3d3' ||\
	(box_type) == 'a3d4')

class IMMTESDataOutputAgent;

namespace BST
{
	namespace ISOBMFF
	{
		struct NALUnitSegment
		{
			uint16_t				nalUnitLength;
			std::vector<uint8_t>	nalUnit;

			NALUnitSegment(CBitstream& bs) : nalUnitLength(bs.GetWord()) {
				try {
					nalUnit.resize(nalUnitLength);
					bs.Read(&nalUnit[0], nalUnitLength);
				}
				catch (...)
				{
				}
			}
		};

		struct MPEG4ExtensionDescriptorsBox : public Box
		{
			std::vector<MPEG4System::BaseDescriptor*>	Descrs;

			virtual ~MPEG4ExtensionDescriptorsBox() {
				for (auto v : Descrs)
					delete v;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				try
				{
					MPEG4System::BaseDescriptor* ptr_descr = nullptr;
					while (left_bytes >= 2 && MPEG4System::BaseDescriptor::LoadDescriptor(bs, &ptr_descr) >= 0)
					{
						Descrs.push_back(ptr_descr);

						if (left_bytes < (uint64_t)ptr_descr->header_size + ptr_descr->sizeOfInstance)
							break;

						left_bytes -= (uint64_t)ptr_descr->header_size + ptr_descr->sizeOfInstance;
					}
				}
				catch (...)
				{
				}

				SkipLeftBits(bs);
				return 0;
			}

		};

		struct AVCDecoderConfigurationRecord
		{
			uint8_t			configurationVersion = 1;
			uint8_t			AVCProfileIndication;
			uint8_t			profile_compatibility;
			uint8_t			AVCLevelIndication;
			uint8_t			reserved_0 : 6;
			uint8_t			lengthSizeMinusOne : 2;
			uint8_t			reserved_1 : 3;
			uint8_t			numOfSequenceParameterSets : 5;
			std::vector<NALUnitSegment*>
				sequenceParameterSetNALUnits;
			uint8_t			numOfPictureParameterSets = 0;
			std::vector<NALUnitSegment*>
				pictureParameterSetNALUnits;
			uint8_t			reserved_2 : 6;
			uint8_t			chroma_format : 2;
			uint8_t			reserved_3 : 5;
			uint8_t			bit_depth_luma_minus8 : 3;
			uint8_t			reserved_4 : 5;
			uint8_t			bit_depth_chroma_minus8 : 3;
			uint8_t			numOfSequenceParameterSetExt = 0;
			std::vector<NALUnitSegment*>
				sequenceParameterSetExtNALUnits;

			AVCDecoderConfigurationRecord()
				: reserved_0(0x3F), reserved_1(0x7), reserved_2(0x3F), reserved_3(0x1F), reserved_4(0x1F) {}

			~AVCDecoderConfigurationRecord() {
				for (auto v : sequenceParameterSetNALUnits)
					delete v;
				for (auto v : pictureParameterSetNALUnits)
					delete v;
				for (auto v : sequenceParameterSetExtNALUnits)
					delete v;
			}

			int Unpack(CBitstream& bs, uint64_t left_bytes)
			{
				try
				{
					if (left_bytes < 6)
						return RET_CODE_ERROR;

					configurationVersion = bs.GetByte();
					AVCProfileIndication = bs.GetByte();
					profile_compatibility = bs.GetByte();
					AVCLevelIndication = bs.GetByte();
					reserved_0 = (uint8_t)bs.GetBits(6);
					lengthSizeMinusOne = (uint8_t)bs.GetBits(2);
					reserved_1 = (uint8_t)bs.GetBits(3);
					numOfSequenceParameterSets = (uint8_t)bs.GetBits(5);
					left_bytes -= 6;

					sequenceParameterSetNALUnits.reserve(numOfSequenceParameterSets);
					for (uint8_t i = 0; i < numOfSequenceParameterSets; i++)
					{
						if (left_bytes < 2 || left_bytes < 2 + bs.PeekBits(16))
							return RET_CODE_BOX_TOO_SMALL;

						sequenceParameterSetNALUnits.push_back(new NALUnitSegment(bs));
						left_bytes -= (uint64_t)sequenceParameterSetNALUnits.back()->nalUnitLength + 2;
					}

					if (left_bytes < 1)
						return RET_CODE_BOX_TOO_SMALL;

					numOfPictureParameterSets = bs.GetByte();
					left_bytes--;

					pictureParameterSetNALUnits.reserve(numOfPictureParameterSets);
					for (uint8_t i = 0; i < numOfPictureParameterSets; i++)
					{
						if (left_bytes < 2 || left_bytes < 2 + bs.PeekBits(16))
							return RET_CODE_BOX_TOO_SMALL;

						pictureParameterSetNALUnits.push_back(new NALUnitSegment(bs));
						left_bytes -= (uint64_t)pictureParameterSetNALUnits.back()->nalUnitLength + 2;
					}

					if (left_bytes > 0 && (AVCProfileIndication == 100 || AVCProfileIndication == 110 || AVCProfileIndication == 122 || AVCProfileIndication == 144))
					{
						if (left_bytes < 4)
							return RET_CODE_BOX_TOO_SMALL;

						reserved_2 = (uint8_t)bs.GetBits(6);
						chroma_format = (uint8_t)bs.GetBits(2);
						reserved_3 = (uint8_t)bs.GetBits(5);
						bit_depth_luma_minus8 = (uint8_t)bs.GetBits(3);
						reserved_4 = (uint8_t)bs.GetBits(5);
						bit_depth_chroma_minus8 = (uint8_t)bs.GetBits(3);
						numOfSequenceParameterSetExt = bs.GetByte();

						left_bytes -= 4;

						sequenceParameterSetExtNALUnits.reserve(numOfSequenceParameterSetExt);
						for (uint8_t i = 0; i < numOfSequenceParameterSetExt; i++)
						{
							if (left_bytes < 2 || left_bytes < 2 + bs.PeekBits(16))
								return RET_CODE_BOX_TOO_SMALL;

							sequenceParameterSetExtNALUnits.push_back(new NALUnitSegment(bs));
							left_bytes -= (uint64_t)sequenceParameterSetExtNALUnits.back()->nalUnitLength + 2;
						}
					}
				}
				catch (...)
				{
					return -1;
				}

				return 0;
			}

		};

		struct AVCConfigurationBox : public Box
		{
			AVCDecoderConfigurationRecord*	AVCConfig = nullptr;

			virtual ~AVCConfigurationBox() {
				if (AVCConfig)
					delete AVCConfig;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < 7)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				AVCConfig = new AVCDecoderConfigurationRecord();
				iRet = AVCConfig->Unpack(bs, left_bytes);

			done:
				SkipLeftBits(bs);
				return iRet;
			}
		}PACKED;

		struct AVCSampleEntry : public MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
		{
			AVCConfigurationBox*			config = nullptr;
			MPEG4ExtensionDescriptorsBox*	descr = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::Unpack(bs);

				for (auto ptr_box = first_child; ptr_box != nullptr; ptr_box = ptr_box->next_sibling)
				{
					if (ptr_box->type == 'm4ds')
						descr = (MPEG4ExtensionDescriptorsBox*)ptr_box;
					else if (ptr_box->type == 'avcC')
						config = (AVCConfigurationBox*)ptr_box;
				}

				return iRet;
			}
		}PACKED;

		struct AVC2SampleEntry : public MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
		{
			AVCConfigurationBox*			avcconfig = nullptr;
			MPEG4ExtensionDescriptorsBox*	descr = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::Unpack(bs);

				for (auto ptr_box = first_child; ptr_box != nullptr; ptr_box = ptr_box->next_sibling)
				{
					if (ptr_box->type == 'm4ds')
						descr = (MPEG4ExtensionDescriptorsBox*)ptr_box;
					else if (ptr_box->type == 'avcC')
						avcconfig = (AVCConfigurationBox*)ptr_box;
				}

				return iRet;
			}

		}PACKED;

		struct AVCParameterSampleEntry : public MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
		{
			AVCConfigurationBox*			config = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::Unpack(bs);

				for (auto ptr_box = first_child; ptr_box != nullptr; ptr_box = ptr_box->next_sibling)
				{
					if (ptr_box->type == 'avcC')
						config = (AVCConfigurationBox*)ptr_box;
				}

				return iRet;
			}

		}PACKED;

		struct HEVCDecoderConfigurationRecord
		{
			struct NALUnits
			{
				uint8_t							array_completeness : 1;
				uint8_t							reserved : 1;
				uint8_t							NAL_unit_type : 6;
				uint16_t						numNalus;
				std::vector<NALUnitSegment*>	Nalus;

				NALUnits(CBitstream& bs) :
					array_completeness((uint8_t)bs.GetBits(1)),
					reserved((uint8_t)bs.GetBits(1)),
					NAL_unit_type((uint8_t)bs.GetBits(6)),
					numNalus(bs.GetWord()) 
				{
					try {
						Nalus.resize(numNalus, nullptr);
						for (size_t i = 0; i < Nalus.size(); i++)
							Nalus[i] = new NALUnitSegment(bs);
					}
					catch (...) {}
				}

				virtual ~NALUnits() {
					for (auto v : Nalus)
						delete v;
				}

			};

			uint8_t					configurationVersion;
			uint8_t					general_profile_space : 2;
			uint8_t					general_tier_flag : 1;
			uint8_t					general_profile_idc : 5;
			uint32_t				general_profile_compatibility_flags;
			uint64_t				general_constraint_indicator_flags;
			uint8_t					general_level_idc;
			uint16_t				reserved_0 : 4;
			uint16_t				min_spatial_segmentation_idc : 12;
			uint8_t					reserved_1 : 6;
			uint8_t					parallelismType : 2;
			uint8_t					reserved_2 : 6;
			uint8_t					chroma_format_idc : 2;
			uint8_t					reserved_3 : 5;
			uint8_t					bit_depth_luma_minus8 : 3;
			uint8_t					reserved_4 : 5;
			uint8_t					bit_depth_chroma_minus8 : 5;
			uint16_t				avgFrameRate;
			uint8_t					constantFrameRate : 2;
			uint8_t					numTemporalLayers : 3;
			uint8_t					temporalIdNested : 1;
			uint8_t					lengthSizeMinusOne : 2;
			uint8_t					numOfArrays;
			std::vector<NALUnits*>	nalArray;

			HEVCDecoderConfigurationRecord()
				: reserved_0(0xF), reserved_1(0x3F), reserved_2(0x3F), reserved_3(0x1F), reserved_4(0x1F) {}

			~HEVCDecoderConfigurationRecord() {
				for (auto v : nalArray)
					delete v;
			}

			int Unpack(CBitstream& bs)
			{
				try
				{
					configurationVersion = bs.GetByte();
					general_profile_space = (uint8_t)bs.GetBits(2);
					general_tier_flag = (uint8_t)bs.GetBits(1);
					general_profile_idc = (uint8_t)bs.GetBits(5);
					general_profile_compatibility_flags = bs.GetDWord();
					general_constraint_indicator_flags = bs.GetBits(48);
					general_level_idc = bs.GetByte();
					reserved_0 = (uint8_t)bs.GetBits(4);
					min_spatial_segmentation_idc = (uint16_t)bs.GetBits(12);
					reserved_1 = (uint8_t)bs.GetBits(6);
					parallelismType = (uint8_t)bs.GetBits(2);
					reserved_2 = (uint8_t)bs.GetBits(6);
					chroma_format_idc = (uint8_t)bs.GetBits(2);
					reserved_3 = (uint8_t)bs.GetBits(5);
					bit_depth_luma_minus8 = (uint8_t)bs.GetBits(3);
					reserved_4 = (uint8_t)bs.GetBits(5);
					bit_depth_chroma_minus8 = (uint8_t)bs.GetBits(3);
					avgFrameRate = bs.GetWord();
					constantFrameRate = (uint8_t)bs.GetBits(2);
					numTemporalLayers = (uint8_t)bs.GetBits(3);
					temporalIdNested = (uint8_t)bs.GetBits(1);
					lengthSizeMinusOne = (uint8_t)bs.GetBits(2);
					numOfArrays = bs.GetByte();
					nalArray.resize(numOfArrays, nullptr);
					for (uint8_t i = 0; i < numOfArrays; i++)
						nalArray[i] = new NALUnits(bs);
				}
				catch (...)
				{
					return -1;
				}

				return 0;
			}

		};

		struct LHEVCDecoderConfigurationRecord
		{
			uint8_t					configurationVersion;
			uint16_t				reserved_0 : 4;
			uint16_t				min_spatial_segmentation_idc : 12;
			uint8_t					reserved_1 : 6;
			uint8_t					parallelismType : 2;
			uint8_t					reserved_2 : 2;
			uint8_t					numTemporalLayers : 3;
			uint8_t					temporalIdNested : 1;
			uint8_t					lengthSizeMinusOne : 2;
			uint8_t					numOfArrays;
			std::vector<HEVCDecoderConfigurationRecord::NALUnits*>
				nalArray;

			LHEVCDecoderConfigurationRecord()
				: reserved_0(0xF), reserved_1(0x3F), reserved_2(0x3) {}

			~LHEVCDecoderConfigurationRecord() {
				for (auto v : nalArray)
					delete v;
			}

			int Unpack(CBitstream& bs)
			{
				try
				{
					configurationVersion = bs.GetByte();
					reserved_0 = (uint8_t)bs.GetBits(4);
					min_spatial_segmentation_idc = (uint16_t)bs.GetBits(12);
					reserved_1 = (uint8_t)bs.GetBits(6);
					parallelismType = (uint8_t)bs.GetBits(2);
					reserved_2 = (uint8_t)bs.GetBits(2);
					numTemporalLayers = (uint8_t)bs.GetBits(3);
					temporalIdNested = (uint8_t)bs.GetBits(1);
					lengthSizeMinusOne = (uint8_t)bs.GetBits(2);
					numOfArrays = bs.GetByte();
					nalArray.resize(numOfArrays, nullptr);
					for (uint8_t i = 0; i < numOfArrays; i++)
						nalArray[i] = new HEVCDecoderConfigurationRecord::NALUnits(bs);
				}
				catch (...)
				{
					return -1;
				}

				return 0;
			}

		};

		/*
		Sample Entry and Box Types: 'hvc1', 'hev1', 'hvcC'
		Container: Sample Table Box ('stbl')
		Mandatory: An 'hvc1' or 'hev1' sample entry is mandatory
		Quantity: One or more sample entries may be present
		*/
		struct HEVCConfigurationBox : public Box
		{
			HEVCDecoderConfigurationRecord*	HEVCConfig = nullptr;

			virtual ~HEVCConfigurationBox() {
				if (HEVCConfig != nullptr)
					delete HEVCConfig;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < 23)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				HEVCConfig = new HEVCDecoderConfigurationRecord();
				iRet = HEVCConfig->Unpack(bs);

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		struct HEVCSampleEntry : public MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
		{
			HEVCConfigurationBox*			config = nullptr;
			MPEG4ExtensionDescriptorsBox*	descr = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::Unpack(bs);

				for (auto ptr_box = first_child; ptr_box != nullptr; ptr_box = ptr_box->next_sibling)
				{
					if (ptr_box->type == 'm4ds')
						descr = (MPEG4ExtensionDescriptorsBox*)ptr_box;
					else if (ptr_box->type == 'hvcC')
						config = (HEVCConfigurationBox*)ptr_box;
				}

				return iRet;
			}
		}PACKED;

		struct LHEVCConfigurationBox : public Box
		{
			LHEVCDecoderConfigurationRecord*	LHEVCConfig = nullptr;

			virtual ~LHEVCConfigurationBox() {
				if (LHEVCConfig != nullptr)
					delete LHEVCConfig;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < 6)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				LHEVCConfig = new LHEVCDecoderConfigurationRecord();
				iRet = LHEVCConfig->Unpack(bs);

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		struct HEVCLHVCSampleEntry : public HEVCSampleEntry
		{
			LHEVCConfigurationBox*			lhvcconfig = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = HEVCSampleEntry::Unpack(bs);

				for (auto ptr_box = first_child; ptr_box != nullptr; ptr_box = ptr_box->next_sibling)
				{
					if (ptr_box->type == 'lhvC')
						lhvcconfig = (LHEVCConfigurationBox*)ptr_box;
				}

				return iRet;
			}
		}PACKED;

		struct LHEVCSampleEntry : public MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
		{
			LHEVCConfigurationBox*			lhvcconfig = nullptr;
			MPEG4ExtensionDescriptorsBox*	descr = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::Unpack(bs);

				for (auto ptr_box = first_child; ptr_box != nullptr; ptr_box = ptr_box->next_sibling)
				{
					if (ptr_box->type == 'm4ds')
						descr = (MPEG4ExtensionDescriptorsBox*)ptr_box;
					else if (ptr_box->type == 'lhvC')
						lhvcconfig = (LHEVCConfigurationBox*)ptr_box;
				}

				return iRet;
			}

		}PACKED;

		struct OperatingPointsRecord
		{
			struct PROFILE_TIER_LEVEL
			{
				uint64_t	general_profile_space : 2;
				uint64_t	general_tier_flag : 1;
				uint64_t	general_profile_idc : 5;
				uint64_t	general_constraint_indicator_flags : 48;
				uint64_t	general_level_idc : 8;

				uint32_t	general_profile_compatibility_flags;
			}PACKED;

			struct OPERATING_POINT
			{
				struct LAYER
				{
					uint8_t		ptl_idx;
					uint8_t		layer_id : 6;
					uint8_t		is_outputlayer : 1;
					uint8_t		is_alternate_outputlayer : 1;
				}PACKED;

				uint32_t	output_layer_set_idx : 16;
				uint32_t	max_temporal_id : 8;
				uint32_t	layer_count : 8;

				std::vector<LAYER>
					layers;

				uint16_t	minPicWidth;
				uint16_t	minPicHeight;
				uint16_t	maxPicWidth;
				uint16_t	maxPicHeight;

				uint8_t		maxChromaFormat : 2;
				uint8_t		maxBitDepthMinus8 : 3;
				uint8_t		reserved : 1;
				uint8_t		frame_rate_info_flag : 1;
				uint8_t		bit_rate_info_flag : 1;

				uint16_t	avgFrameRate;
				uint8_t		reserved_1 : 6;
				uint8_t		constantFrameRate : 2;
				uint32_t	maxBitrate;
				uint32_t	avgBitrate;

			};

			struct LAYER
			{
				uint8_t		layerID = 0;
				uint8_t		num_direct_ref_layers = 0;
				std::vector<uint8_t>
					direct_ref_layerIDs;
				uint8_t		dimension_identifier[16] = { 0 };
			};

			uint16_t		scalability_mask;
			uint8_t			reserved_0 : 2;
			uint8_t			num_profile_tier_level : 6;
			std::vector<PROFILE_TIER_LEVEL>
				profile_tier_levels;

			uint16_t		num_operating_points;
			std::vector<OPERATING_POINT>
				operating_points;

			uint8_t			max_layer_count;
			std::vector<LAYER>
				layers;

			int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				scalability_mask = bs.GetWord();
				reserved_0 = bs.GetBits(2);
				num_profile_tier_level = bs.GetBits(6);

				profile_tier_levels.resize(num_profile_tier_level);

				for (auto& v : profile_tier_levels)
				{
					v.general_profile_space = bs.GetBits(2);
					v.general_tier_flag = bs.GetBits(1);
					v.general_profile_idc = bs.GetBits(5);
					v.general_profile_compatibility_flags = (uint32_t)bs.GetBits(32);
					v.general_constraint_indicator_flags = bs.GetBits(48);
					v.general_level_idc = bs.GetBits(8);
				}

				num_operating_points = bs.GetWord();
				operating_points.resize(num_operating_points);

				for (auto& v : operating_points)
				{
					v.output_layer_set_idx = (uint32_t)bs.GetBits(16);
					v.max_temporal_id = (uint32_t)bs.GetBits(8);
					v.layer_count = (uint32_t)bs.GetBits(8);

					v.layers.resize(v.layer_count);

					for (auto& layer : v.layers)
					{
						layer.ptl_idx = bs.GetByte();
						layer.layer_id = (uint8_t)bs.GetBits(6);
						layer.is_outputlayer = (uint8_t)bs.GetBits(1);
						layer.is_alternate_outputlayer = (uint8_t)bs.GetBits(1);
					}

					v.minPicWidth = bs.GetWord();
					v.minPicHeight = bs.GetWord();
					v.maxPicWidth = bs.GetWord();
					v.maxPicHeight = bs.GetWord();

					v.maxChromaFormat = (uint8_t)bs.GetBits(2);
					v.maxBitDepthMinus8 = (uint8_t)bs.GetBits(3);
					v.reserved = (uint8_t)bs.GetBits(1);
					v.frame_rate_info_flag = (uint8_t)bs.GetBits(1);
					v.bit_rate_info_flag = (uint8_t)bs.GetBits(1);

					if (v.frame_rate_info_flag)
					{
						v.avgFrameRate = bs.GetWord();
						v.reserved_1 = bs.GetBits(6);
						v.constantFrameRate = bs.GetBits(2);
					}

					if (v.bit_rate_info_flag)
					{
						v.maxBitrate = bs.GetWord();
						v.avgBitrate = bs.GetWord();
					}
				}

				max_layer_count = bs.GetByte();
				layers.resize(max_layer_count);

				for (auto& v : layers)
				{
					v.layerID = bs.GetByte();
					v.num_direct_ref_layers = bs.GetByte();
					v.direct_ref_layerIDs.resize(v.num_direct_ref_layers);
					for (int i = 0; i < v.num_direct_ref_layers; i++)
						v.direct_ref_layerIDs[i] = bs.GetByte();

					for (int j = 0; j < 16; j++)
					{
						if (scalability_mask&(1 << j))
							v.dimension_identifier[j] = bs.GetByte();
					}
				}

				return iRet;
			}

		};

		struct OperatingPointsInformation : public VisualSampleGroupEntry
		{
			OperatingPointsRecord	oinf;

			OperatingPointsInformation(uint32_t nSize) : VisualSampleGroupEntry('oinf', nSize) {
			}

			int Unpack(CBitstream& bs)
			{
				return oinf.Unpack(bs);
			}
		};

		struct TemporalLayerEntry : public VisualSampleGroupEntry
		{
			uint8_t					temporalLayerId = 0;
			uint8_t					tlprofile_space : 2;
			uint8_t					tltier_flag : 1;
			uint8_t					tlprofile_idc : 5;
			uint32_t				tlprofile_compatibility_flags = 0;
			uint64_t				tlconstraint_indicator_flags : 48;
			uint64_t				tllevel_idc : 8;
			uint64_t				tlConstantFrameRate : 8;
			uint16_t				tlMaxBitRate = 0;
			uint16_t				tlAvgBitRate = 0;
			uint16_t				tlAvgFrameRate = 0;

			TemporalLayerEntry(uint32_t nGroupingType, uint32_t nSize)
				: VisualSampleGroupEntry(nGroupingType, nSize)
				, tlprofile_space(0), tltier_flag(0), tlprofile_idc(0), tlprofile_compatibility_flags(0)
				, tlconstraint_indicator_flags(0), tllevel_idc(0), tlConstantFrameRate(0) {
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				uint32_t left_bytes = description_length;
				if (left_bytes < 20)
					goto done;

				temporalLayerId = bs.GetByte();
				tlprofile_space = (uint8_t)bs.GetBits(2);
				tltier_flag = (uint8_t)bs.GetBits(1);
				tlprofile_idc = (uint8_t)bs.GetBits(5);
				tlprofile_compatibility_flags = bs.GetDWord();
				tlconstraint_indicator_flags = bs.GetBits(48);
				tllevel_idc = bs.GetBits(8);
				tlMaxBitRate = bs.GetWord();
				tlAvgBitRate = bs.GetWord();
				tlConstantFrameRate = bs.GetBits(8);
				tlAvgFrameRate = bs.GetWord();
				left_bytes -= 20;

			done:
				if (left_bytes > 0)
					bs.SkipBits((int64_t)left_bytes << 3);
				return iRet;
			}

		}PACKED;

		struct INALAUSampleRepacker
		{
			virtual	int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe) = 0;
			/*!	@brief Repack NAL Unit to byte stream mentioned in H.264 and H.265 spec Annex-B
				@param pNalUnitBuf the NAL Unit buffer
				@param NumBytesInNalUnit the number of bytes of NAL unit buffer
				@param bAUCommitted An Access Unit hit or not */
			virtual int RepackNALUnitToAnnexBByteStream(uint8_t* pNalUnitBuf, int NumBytesInNalUnit, const PROCESS_DATA_INFO* NAL_Unit_DataInfo = nullptr, bool* bAUCommitted = nullptr) = 0;
			/*!	@brief Discard all data*/
			virtual int Flush() = 0;
			/*!	@brief Forcedly commit the data to the downstream*/
			virtual int Drain() = 0;
			virtual int SetNextAUPTSDTS(TM_90KHZ pts, TM_90KHZ dts) = 0;
			virtual int SetAUStartPointCallback(CB_AU_STARTPOINT cbAUStartPoint, void* ptr_context) = 0;

			virtual ~INALAUSampleRepacker() {}
		};

		struct NALAUSampleRepackerBase : public INALAUSampleRepacker
		{
			// INALAUSampleRepacker
			virtual int Seek(uint64_t src_sample_file_offset);
			virtual	int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe);
			virtual int SetNextAUPTSDTS(TM_90KHZ pts, TM_90KHZ dts);
			virtual int SetAUStartPointCallback(CB_AU_STARTPOINT cbAUStartPoint, void* ptr_context);

			// Other command functions
			virtual int WriteAUData(uint8_t* pBuf, int cbBuf, const PROCESS_DATA_INFO* NAL_Unit_DataInfo = nullptr);
			virtual int CommitAU();

			FILE*		m_fpSrc;
			FILE*		m_fpDst;
			TM_90KHZ	m_next_AU_PTS;
			TM_90KHZ	m_next_AU_DTS;

			CB_AU_STARTPOINT
				m_callback_au_startpoint;
			IMMTESDataOutputAgent*
				m_pOutputAgent;
			void*		m_context_au_startpoint;
			AMLinearRingBuffer
				m_lrb_AU;
			PROCESS_DATA_INFO
				m_data_info;

			NALAUSampleRepackerBase(FILE* fp, FILE* fw, IMMTESDataOutputAgent* pOutputAgent)
				: m_fpSrc(fp), m_fpDst(fw), m_next_AU_PTS(INVALID_TM_90KHZ_VALUE), m_next_AU_DTS(INVALID_TM_90KHZ_VALUE)
				, m_callback_au_startpoint(nullptr), m_pOutputAgent(pOutputAgent), m_context_au_startpoint(nullptr), m_lrb_AU(nullptr) {
				if (m_pOutputAgent != nullptr)
					m_lrb_AU = AM_LRB_Create(1024 * 1024);

				memset(&m_data_info, 0, sizeof(m_data_info));
			}

			~NALAUSampleRepackerBase() {
				AM_LRB_Destroy(m_lrb_AU);
			}
		};

		struct AVCSampleRepacker : public NALAUSampleRepackerBase
		{
			ISOBMFF::AVCDecoderConfigurationRecord*
				m_AVCConfigRecord;

			AVCSampleRepacker(FILE* fp, FILE* fw, IMMTESDataOutputAgent* pOutputAgent, ISOBMFF::AVCDecoderConfigurationRecord* pAVCConfigRecord)
				: NALAUSampleRepackerBase(fp, fw, pOutputAgent), m_AVCConfigRecord(pAVCConfigRecord) {}

			int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe);
			int RepackNALUnitToAnnexBByteStream(uint8_t* pNalUnitBuf, int NumBytesInNalUnit, const PROCESS_DATA_INFO* NAL_Unit_DataInfo = nullptr, bool* bAUCommitted = nullptr);
			int Flush();
			int Drain();
		};

		struct HEVCSampleRepacker : public NALAUSampleRepackerBase
		{
			ISOBMFF::HEVCDecoderConfigurationRecord*
				m_HEVCConfigRecord;
			std::vector<std::tuple<uint8_t* /*pNalUnitBuf*/, int /*NumBytesInNalUnit*/, PROCESS_DATA_INFO>>
				m_vNonVCLNUs;

			HEVCSampleRepacker(FILE* fp, FILE* fw, IMMTESDataOutputAgent* pOutputAgent, ISOBMFF::HEVCDecoderConfigurationRecord* pHEVCConfigRecord)
				: NALAUSampleRepackerBase(fp, fw, pOutputAgent), m_HEVCConfigRecord(pHEVCConfigRecord) {}

			int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe);
			int RepackNALUnitToAnnexBByteStream(uint8_t* pNalUnitBuf, int NumBytesInNalUnit, const PROCESS_DATA_INFO* NAL_Unit_DataInfo = nullptr, bool* bAUCommitted = nullptr);
			int Flush();
			int Drain();
		};

	} // namespace  ISOBMFF
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

