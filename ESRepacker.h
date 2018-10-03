/*
MIT License

Copyright (c) 2017 Ravin.Wang(wangf1978@hotmail.com)

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

/*
	The elementary stream data in a media file container may be re-organized.
	If directly dumping it to a file, there may be no player to play it, or 
	even it is ill-formated.

	For example, MPEG4-AVC/HEVC picture data in ISOBMFF/Matroska, in general, 
	one picture may consist of multiple NAL unit, they are stored like as

	|----------------------------------->   NAL_UNIT Payload  <--------------------------------------|
	|-- NAL_unit size--|-------------------------------NAL Unit data---------------------------------|

	|------------------------------->   H.264/HEVC(MPEG4/MKV frame)   <------------------------------|
	|--- NAL_UNIT payload --|--- NAL_UNIT payload ---|--- NAL_UNIT payload ---|..........

	This kind of payload can't be recognized by external players, we need repack it to 
	Annex B Byte stream format defined in MPEG4-AVC and HEVC spec
*/

#include "ISO14496_15.h"
#include "CodecID.h"
#include "AMRingBuffer.h"

#define USE_NAL(codec_id)				(codec_id == CODEC_ID_V_MPEG4_AVC || codec_id == CODEC_ID_V_MPEGH_HEVC)

enum ES_BYTE_STREAM_FORMAT
{
	ES_BYTE_STREAM_RAW = 0,
	ES_BYTE_STREAM_AVC_ANNEXB,			// (Leading_zero_8bits) + (zero_byte) + start_code_prefix_one_3bytes + nal_unit + (trailing_zero_8bits)
	ES_BYTE_STREAM_HEVC_ANNEXB,			// (Leading_zero_8bits) + (zero_byte) + start_code_prefix_one_3bytes + nal_unit + (trailing_zero_8bits)
	ES_BYTE_STREAM_ISO_NALAU_SAMPLE,	// NALUnitLength + nal_unit + NALUnitLength + .....
	ES_BYTE_STREAM_NALUNIT_WITH_LEN,	// NALUnitLength + nal_unit
};

enum ES_SEEK_POINT_TYPE
{
	ES_SEEK_RAW,
	ES_SEEK_ISOBMFF_CHUNK = 1,
	ES_SEEK_MATROSKA_SIMPLE_BLOCK,
	ES_SEEK_MATROSKA_BLOCK_GROUP,
	ES_SEEK_SKIP,
};

enum FRAGMENTATION_INDICATOR
{
	FRAG_INDICATOR_COMPLETE = 0,
	FRAG_INDICATOR_FIRST,
	FRAG_INDICATOR_MIDDLE,
	FRAG_INDICATOR_LAST
};

struct SEEK_POINT_INFO
{
	ES_SEEK_POINT_TYPE
					seek_point_type;
	uint32_t		track_number;
	uint64_t		time_code;
	FLAG_VALUE		key_frame;
	FLAG_VALUE		invisible;
	FLAG_VALUE		discardable;
	uint8_t			lacing;
	uint8_t			reserved[3];
	std::vector<uint32_t>
					frame_sizes;
};

struct ES_REPACK_CONFIG
{
	CODEC_ID		codec_id;
	union
	{
		ISOBMFF::AVCDecoderConfigurationRecord*
					pAVCConfigRecord;
		ISOBMFF::HEVCDecoderConfigurationRecord*
					pHEVCConfigRecord;
		void*		pCodecPrivObj;
	};
	char			es_output_file_path[MAX_PATH];
	// Used for converting AnnexB byte-stream to ISO NAL Access Unit Sample format.
	int				NALUnit_Length_Size;
};

class CESRepacker
{
public:
	CESRepacker(ES_BYTE_STREAM_FORMAT srcESFmt, ES_BYTE_STREAM_FORMAT dstESFmt);

	virtual ~CESRepacker();

	/*!	@brief Configure the ES re-packer */
	virtual int Config(ES_REPACK_CONFIG es_repack_config);

	/*!	@brief Open the media file with complete and continuous ES sample 
		@remark For the file which splits a continuous ES sample into multiple packs, for example, 
		transport stream or program stream, this method can't be used.*/
	virtual int Open(const char* szSrcFile);

	/*!	@brief Seek the specified file position in the specified source file */
	virtual int Seek(int64_t src_sample_file_offset, ES_SEEK_POINT_TYPE seek_point_type=ES_SEEK_RAW);

	/*!	@brief Update the seek point information at the current file position. */
	virtual int UpdateSeekPointInfo();

	/*!	@brief Get the ES chunk or block information at the current seek point. */
	virtual int GetSeekPointInfo(SEEK_POINT_INFO& seek_point_info);
	
	/*!	@brief Begin the repack process in the specified seek position */
	virtual	int	Repack(uint32_t sample_size, FLAG_VALUE keyframe);

	/*!	@brief Process the buffer with the specified buffer size */
	virtual int Process(uint8_t* pBuf, int cbSize, FRAGMENTATION_INDICATOR indicator);

	/*!	@brief Flush the cache buffer in the ES packer. */
	virtual int Flush();
	
	/*!	@brief Close ES re-packer and release the resource. */
	virtual int Close();

protected:
	/* 
		Pre-process the block header from the current seek point, 
		For Simple Block, only parsing the block header, and locate the file position to the first byte of ES packet data
		For BlockGroup, try to parsing the data till to Block element
	*/
	int PreProcessMatroskaBlock();
	int PostProcessMatroSkaBlock();

private:
	ES_BYTE_STREAM_FORMAT	m_srcESFmt;
	ES_BYTE_STREAM_FORMAT	m_dstESFmt;
	ES_REPACK_CONFIG		m_config;
	char					m_szSrcFilepath[MAX_PATH];

	FILE*					m_fpSrc;
	FILE*					m_fpDst;

	SEEK_POINT_INFO			m_cur_seek_point_info;
	AMLinearRingBuffer		m_lrb_NAL;

	union
	{
		void* m_external_repacker;
		ISOBMFF::INALAUSampleRepacker* m_NALAURepacker;
	};
};

