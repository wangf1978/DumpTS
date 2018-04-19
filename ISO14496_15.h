#pragma once

#include <stdint.h>
#include "Bitstream.h"
#include "combase.h"
#include <algorithm>
#include "ISO14496_12.h"
#include "ISO14496_1.h"

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

enum class AVC_NAL_UNIT_TYPE
{
	CS_NON_IDR_PIC = 1,		// Coded slice of a non-IDR picture
	CSD_PARTITION_A = 2,	// Coded slice data partition A
	CSD_PARTITION_B = 3,	// Coded slice data partition B
	CSD_PARTITION_C = 4,	// Coded slice data partition C
	CS_IDR_PIC = 5,			// Coded slice of an IDR picture
	SEI_NUT = 6,			// Supplemental enhancement information
	SPS_NUT = 7,			// Sequence	parameter set
	PPS_NUT = 8,			// Picture parameter set
	AUD_NUT = 9,			// Access unit delimiter
	EOS_NUT = 10,			// End of Sequence
	EOB_NUT = 11,			// End of stream
	FD_NUT = 12,			// Filler data
	SPS_EXT_NUT = 13,		// Sequence parameter set extension
	PREFIX_NUT = 14,		// Prefix NAL unit
	SUB_SPS_NUT = 15,		// Subset sequence parameter set
	SL_WO_PARTITION = 19,	// Coded slice of an auxiliary coded picture without partitioning
	SL_EXT = 20,			// Coded slice extension
	SL_EXT_DVIEW = 21,		// Coded slice extension for depth view components
};

enum class HEVC_NAL_UNIT_TYPE
{
	TRAIL_N = 0,
	TRAIL_R = 1,
	TSA_N = 2,
	TSA_R = 3,
	STSA_N = 4,
	STSA_R = 5,
	RADL_N = 6,
	RADL_R = 7,
	RASL_N = 8,
	RASL_R = 9,
	RSV_VCL_N10 = 10,
	RSV_VCL_N12 = 12,
	RSV_VCL_N14 = 14,
	RSV_VCL_R11 = 11,
	RSV_VCL_R13 = 13,
	RSV_VCL_R15 = 15,
	BLA_W_LP = 16,
	BLA_W_RADL = 17,
	BLA_N_LP = 18,
	IDR_W_RADL = 19,
	IDR_N_LP = 20,
	CRA_NUT = 21,
	RSV_IRAP_VCL22 = 22,
	RSV_IRAP_VCL23 = 23,
	RSV_VCL24 = 24,
	RSV_VCL31 = 31,
	VPS_NUT = 32,
	SPS_NUT = 33,
	PPS_NUT = 34,
	AUD_NUT = 35,
	EOS_NUT = 36,
	EOB_NUT = 37,
	FD_NUT = 38,
	PREFIX_SEI_NUT = 39,
	SUFFIX_SEI_NUT = 40,
	RSV_NVCL41 = 41,
	RSV_NVCL47 = 47,
	UNSPEC48 = 48,
	UNSPEC63 = 63,
};

namespace ISOMediaFile
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
			catch(...)
			{ }
		}
	}PACKED;

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

					if (left_bytes < ptr_descr->header_size + ptr_descr->sizeOfInstance)
						break;

					left_bytes -= ptr_descr->header_size + ptr_descr->sizeOfInstance;
				}
			}
			catch (...)
			{
			}

			SkipLeftBits(bs);
			return 0;
		}

	}PACKED;

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
					left_bytes -= 2 + sequenceParameterSetNALUnits.back()->nalUnitLength;
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
					left_bytes -= 2 + pictureParameterSetNALUnits.back()->nalUnitLength;
				}

				if (AVCProfileIndication == 100 || AVCProfileIndication == 110 || AVCProfileIndication == 122 || AVCProfileIndication == 144)
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
						left_bytes -= 2 + sequenceParameterSetExtNALUnits.back()->nalUnitLength;
					}
				}
			}
			catch (...)
			{
				return -1;
			}

			return 0;
		}

	}PACKED;

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

	struct AVCSampleEntry : public ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
	{
		AVCConfigurationBox*			config = nullptr;
		MPEG4ExtensionDescriptorsBox*	descr = nullptr;

		virtual ~AVCSampleEntry()
		{
			if (config != nullptr)
				delete config;
			if (descr != nullptr)
				delete descr;
		}

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = RET_CODE_SUCCESS;
			if ((iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::_Unpack(bs)) < 0)
				return iRet;

			uint64_t left_bytes = LeftBytes(bs);
			if (left_bytes < MIN_BOX_SIZE)
			{
				iRet = RET_CODE_BOX_TOO_SMALL;
				goto done;
			}

			config = new AVCConfigurationBox();
			if ((iRet = config->Unpack(bs)) < 0)
				goto done;

			if (left_bytes > config->size + MIN_BOX_SIZE)
			{
				uint64_t box_header = bs.PeekBits(64);
				if ((box_header&UINT32_MAX) == 'm4ds')
				{
					descr = new MPEG4ExtensionDescriptorsBox();
					descr->Unpack(bs);
				}
			}

		done:
			SkipLeftBits(bs);
			return iRet;
		}
	}PACKED;

	struct AVC2SampleEntry : public ISOMediaFile::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry
	{
		AVCConfigurationBox*			avcconfig = nullptr;
		MPEG4ExtensionDescriptorsBox*	descr = nullptr;

		virtual ~AVC2SampleEntry()
		{
			if (avcconfig != nullptr)
				delete avcconfig;
			if (descr != nullptr)
				delete descr;
		}

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = RET_CODE_SUCCESS;
			if ((iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::_Unpack(bs)) < 0)
				return iRet;

			uint64_t left_bytes = LeftBytes(bs);
			if (left_bytes < MIN_BOX_SIZE)
			{
				iRet = RET_CODE_BOX_TOO_SMALL;
				goto done;
			}

			avcconfig = new AVCConfigurationBox();
			if ((iRet = avcconfig->Unpack(bs)) < 0)
				goto done;

			if (left_bytes > avcconfig->size + MIN_BOX_SIZE)
			{
				uint64_t box_header = bs.PeekBits(64);
				if ((box_header&UINT32_MAX) == 'm4ds')
				{
					descr = new MPEG4ExtensionDescriptorsBox();
					descr->Unpack(bs);
				}
			}

		done:
			SkipLeftBits(bs);
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
				numNalus(bs.GetWord()){
				try {
					Nalus.resize(numNalus, nullptr);
					for (size_t i = 0; i < Nalus.size(); i++)
						Nalus[i] = new NALUnitSegment(bs);
				}catch(...){}
			}

			virtual ~NALUnits() {
				for (auto v : Nalus)
					delete v;
			}

		}PACKED;

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
			: reserved_0(0xF), reserved_1(0x3F), reserved_2(0x3F), reserved_3(0x1F), reserved_4(0x1F){}

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

	}PACKED;

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

		virtual ~HEVCSampleEntry()
		{
			if (config != nullptr)
				delete config;
			if (descr != nullptr)
				delete descr;
		}

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = RET_CODE_SUCCESS;
			if ((iRet = MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::VisualSampleEntry::_Unpack(bs)) < 0)
				return iRet;

			uint64_t left_bytes = LeftBytes(bs);
			if (left_bytes < MIN_BOX_SIZE)
			{
				iRet = RET_CODE_BOX_TOO_SMALL;
				goto done;
			}

			config = new HEVCConfigurationBox();
			if ((iRet = config->Unpack(bs)) < 0)
				goto done;

			if (left_bytes > config->size + MIN_BOX_SIZE)
			{
				uint64_t box_header = bs.PeekBits(64);
				if ((box_header&UINT32_MAX) == 'm4ds')
				{
					descr = new MPEG4ExtensionDescriptorsBox();
					descr->Unpack(bs);
				}
			}			

		done:
			SkipLeftBits(bs);
			return iRet;
		}

	}PACKED;

	struct INALAUSampleRepacker
	{
		virtual	int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe) = 0;
	};

	struct NALAUSampleRepackerBase : public INALAUSampleRepacker
	{
		virtual int Seek(uint64_t src_sample_file_offset);
		virtual	int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe);

		FILE*		m_fpSrc;
		FILE*		m_fpDst;

		NALAUSampleRepackerBase(FILE* fp, FILE* fw) :m_fpSrc(fp), m_fpDst(fw) {
		}
	};

	struct AVCSampleRepacker: public NALAUSampleRepackerBase
	{
		ISOMediaFile::AVCDecoderConfigurationRecord*
					m_AVCConfigRecord;

		AVCSampleRepacker(FILE* fp, FILE* fw, ISOMediaFile::AVCDecoderConfigurationRecord* pAVCConfigRecord)
			: NALAUSampleRepackerBase(fp, fw), m_AVCConfigRecord(pAVCConfigRecord){}

		int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe);
	};

	struct HEVCSampleRepacker : public NALAUSampleRepackerBase
	{

		ISOMediaFile::HEVCDecoderConfigurationRecord*
					m_HEVCConfigRecord;

		HEVCSampleRepacker(FILE* fp, FILE* fw, ISOMediaFile::HEVCDecoderConfigurationRecord* pHEVCConfigRecord)
			: NALAUSampleRepackerBase(fp, fw), m_HEVCConfigRecord(pHEVCConfigRecord) {}

		int	RepackSamplePayloadToAnnexBByteStream(uint32_t sample_size, FLAG_VALUE keyframe);
	};

} // namespace  ISOMediaFile

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

