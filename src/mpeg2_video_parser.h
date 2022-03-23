/*

MIT License

Copyright (c) 2022 Ravin.Wang(wangf1978@hotmail.com)

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
#ifndef __MPEG2_VIDEO_PARSER_H__
#define __MPEG2_VIDEO_PARSER_H__

#include "dump_data_type.h"
#include "AMRingBuffer.h"
#include "AMSHA1.h"

#include <assert.h>
#include <memory.h>
#include <time.h>
#include <sys/timeb.h>
#include "combase.h"
#include "AMArray.h"
#include "AMBitStream.h"
#include "DumpUtil.h"
#include "MSE.h"

namespace BST {
	namespace MPEG2Video {
		struct CSequenceHeader;
		struct CSequenceExtension;
		struct CGroupPicturesHeader;
		struct CPictureHeader;
		struct CPictureCodingExtension;
		struct CSequenceScalableExtension;
	}
}

using SEQHDR = std::shared_ptr<BST::MPEG2Video::CSequenceHeader>;
using SEQEXT = std::shared_ptr<BST::MPEG2Video::CSequenceExtension>;
using GOPHDR = std::shared_ptr<BST::MPEG2Video::CGroupPicturesHeader>;
using PICHDR = std::shared_ptr<BST::MPEG2Video::CPictureHeader>;
using PICEXT = std::shared_ptr<BST::MPEG2Video::CPictureCodingExtension>;
using SEQSCAEXT = std::shared_ptr<BST::MPEG2Video::CSequenceScalableExtension>;

class IMPVContext : public IUnknown
{
public:
	virtual RET_CODE		SetStartCodeFilters(std::initializer_list<uint16_t> start_code_filters) = 0;
	virtual RET_CODE		GetStartCodeFilters(std::vector<uint16_t>& start_code_filters) = 0;
	virtual bool			IsStartCodeFiltered(uint16_t start_code) = 0;
	virtual void			UpdateStartCode(uint16_t start_code) = 0;
	// level: 0, sequence_header/sequence_extension/extension_and_user_data(0)
	// level: 1, group_of_pictures_header/extension_and_user_data(1)
	// level: 2, picture_header/picture_coding_extension/extension_and_user_data(2)/picture_data
	virtual int				GetCurrentLevel()=0;
	virtual SEQHDR			GetSeqHdr() = 0;
	virtual RET_CODE		UpdateSeqHdr(SEQHDR seqHdr) = 0;
	virtual SEQEXT			GetSeqExt() = 0;
	virtual RET_CODE		UpdateSeqExt(SEQEXT seqHdr) = 0;
	virtual SEQSCAEXT		GetSeqScalableExt() = 0;
	virtual RET_CODE		UpdateSeqScalableExt(SEQSCAEXT seqScaExt) = 0;
	virtual void			Reset() = 0;

public:
	IMPVContext() {}
	virtual ~IMPVContext() {}
};

class CMPEG2VideoParser: public CComUnknown, public IMSEParser
{
public:
	CMPEG2VideoParser(RET_CODE* pRetCode=nullptr);
	virtual ~CMPEG2VideoParser();

	DECLARE_IUNKNOWN
	HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
	{
		if (ppvObj == NULL)
			return E_POINTER;

		if (uuid == IID_IMSEParser)
			return GetCOMInterface((IMSEParser*)this, ppvObj);

		return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
	}

public:
	MEDIA_SCHEME_TYPE		GetSchemeType() { return MEDIA_SCHEME_MPV; }
	RET_CODE				SetEnumerator(IUnknown* pEnumerator, uint32_t options);
	RET_CODE				ProcessInput(uint8_t* pBuf, size_t cbBuf);
	RET_CODE				ProcessOutput(bool bDrain = false);
	RET_CODE				ParseAUBuf(uint8_t* pAUBuf, size_t cbAUBuf);
	RET_CODE				GetContext(IUnknown** ppCtx);
	RET_CODE				Reset();

protected:
	int						PushMPVUnitBuf(uint8_t* pStart, uint8_t* pEnd);
	int						CommitMPVUnit(bool bDrain=false);
	RET_CODE				CommitAU();

protected:
	IMPVContext*			m_pCtx;
	IMPVEnumerator*			m_mpv_enum = nullptr;
	uint32_t				m_mpv_enum_options = MPV_ENUM_OPTION_ALL;

	const int				read_unit_size = 2048;
	const int				minimum_mpv_parse_buffer_size = 4;	//00 00 01 start_code_byte, 4 bytes

	AMLinearRingBuffer		m_rbRawBuf = nullptr;
	/*!	@brief It is used to store Byte stream MPEG unit including:
			   start_code_prefix_one_3bytes + start_code + MPEG2 object */
	AMLinearRingBuffer		m_rbMPVUnit = nullptr;

	/*! @brief It indicates how many bytes are already parsed from the first bytes into the parser since 
			   the parser is created or reset */
	uint64_t				m_cur_scan_pos = 0;
	/*! @brief It indicates the position of the last complete MPV unit including the leading bytes before MPV Unit */
	uint64_t				m_cur_submit_pos = 0;
	int16_t					m_cur_mpv_start_code = -1;
	int32_t					m_last_commit_buf_len = 0;
	int32_t					m_picture_coding_type = -1;
	int32_t					m_gop_start = 0;
	bool					m_closed_gop = false;
	bool					m_broken_link = false;
	bool					m_wait_for_new_vseq = true;

	std::vector<std::tuple<int32_t, uint32_t>>	
							m_se_ranges_in_au;
};

#endif
