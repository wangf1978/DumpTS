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
#include "ISO14496_3.h"
#include "AMBitStream.h"

const char* ADTS_profile_ObjectType_names[2][4] = {
	{ "AAC Main", "AAC LC", "AAC SSR", "AAC LTP" },
	{ "Main profile", "Low Complexity profile (LC)", "Scalable Sampling Rate profile (SSR)", "reserved" }
};

namespace BST {

	namespace AACAudio {

		class LOASBitstreamCtx : public CComUnknown, public IMP4AACContext
		{
		public:
			DECLARE_IUNKNOWN

			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_IMP4AACContext)
					return GetCOMInterface((IMP4AACContext*)this, ppvObj);

				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

			MP4AMuxStreamConfig GetMuxStreamConfig()
			{
				return m_currMuxStreamConfig;
			}

			RET_CODE UpdateMuxStreamConfig(MP4AMuxStreamConfig mux_stream_config)
			{
				// try to merge with the previous Mux Stream Config
				if (mux_stream_config == nullptr)
					return RET_CODE_INVALID_PARAMETER;

				if (mux_stream_config->audioMuxVersionA == 0)
				{
					for (uint16_t prog = 0; prog < mux_stream_config->numProgram; prog++)
					{
						for (uint16_t lay = 0; lay < mux_stream_config->numLayer[prog]; lay++)
						{
							if (mux_stream_config->useSameConfig[prog][lay])
							{
								// copy the previous shared pointer into the new stream config
								mux_stream_config->AudioSpecificConfig[prog][lay] =
									m_currMuxStreamConfig->AudioSpecificConfig[prog][lay];
							}
						}
					}
				}

				m_currMuxStreamConfig = mux_stream_config;
				return RET_CODE_SUCCESS;
			}

			MP4AMuxElement	CreateAudioMuxElement(bool muxConfigPresent)
			{
				auto ptr_audio_mux_element = new BST::AACAudio::CAudioMuxElement(muxConfigPresent);
				ptr_audio_mux_element->UpdateCtx(this);
				return std::shared_ptr<CAudioMuxElement>(ptr_audio_mux_element);
			}

			void Reset()
			{
				m_currMuxStreamConfig = nullptr;
			}

		protected:
			MP4AMuxStreamConfig		m_currMuxStreamConfig;
		};

		RET_CODE CreateMP2AACContext(IMP2AACContext** ppMP2AACCtx)
		{
			return RET_CODE_ERROR_NOTIMPL;
		}

		RET_CODE CreateMP4AACContext(IMP4AACContext** ppMP4AACCtx)
		{
			if (ppMP4AACCtx == NULL)
				return RET_CODE_INVALID_PARAMETER;

			auto pCtx = new BST::AACAudio::LOASBitstreamCtx();
			pCtx->AddRef();
			*ppMP4AACCtx = (IMP4AACContext*)pCtx;
			return RET_CODE_SUCCESS;
		}

		int RAW_DATA_BLOCK::Unpack(CBitstream& bs, int start_bit_offset)
		{
			uint8_t id = 0;
			int iRet = RET_CODE_SUCCESS;
			DATA_ELEMENT* ptr_data_element = nullptr;
			uint64_t bit_pos = bs.Tell();
			while ((id = (uint8_t)bs.GetBits(3)) != ID_END)
			{
				AMP_NEWT(ptr_data_element, DATA_ELEMENT, id, ctx_audio_stream);
				if (ptr_data_element == nullptr)
					return RET_CODE_OUTOFMEMORY;

				ptr_data_element->start_bitpos = bit_pos + start_bit_offset;
				data_elements.push_back(ptr_data_element);
				if ((iRet = ptr_data_element->Unpack(bs)) < 0)
					return iRet;
				
				bit_pos = bs.Tell();
			}

			if (id == ID_END)
			{
				AMP_NEWT(ptr_data_element, DATA_ELEMENT, id, ctx_audio_stream);
				if (ptr_data_element == nullptr)
					return RET_CODE_OUTOFMEMORY;

				ptr_data_element->start_bitpos = bit_pos + start_bit_offset;
				data_elements.push_back(ptr_data_element);
			}

			bs.Realign();
			bit_pos = bs.Tell();

			return RET_CODE_SUCCESS;
		}

		int RAW_DATA_BLOCK::Unpack(CBitstream& bs)
		{
			return Unpack(bs, 0);
		}

		int RAW_DATA_BLOCK::Unpack(AMBst in_bst)
		{
			// Convert in_bst to CBitstream
			int nBitOffsetFromCurPtr = 0;
			int cbLeftBytes = 0;
			uint8_t* p = AMBst_LockCurPtr(in_bst, &nBitOffsetFromCurPtr, &cbLeftBytes);

			if (nBitOffsetFromCurPtr % 8 != 0)
			{
				printf("[AAC] the current bitstream is NOT started with byte-aligned position.\n");
				return RET_CODE_BUFFER_NOT_COMPATIBLE;
			}

			p += nBitOffsetFromCurPtr / 8;
			cbLeftBytes -= nBitOffsetFromCurPtr / 8;

			if (cbLeftBytes <= 0)
				return RET_CODE_NOTHING_TODO;

			CBitstream bs(p, (size_t)cbLeftBytes << 3);

			int nRet = Unpack(bs, AMBst_Tell(in_bst));

			uint64_t cbUpdated = bs.Tell();

			if (cbUpdated % 8 != 0)
			{
				printf("[AAC] The processed bits of AAC raw_data_block should be byte-aligned.\n");
			}

			AMBst_UnlockCurPtr(in_bst, (int)((cbUpdated + 7) / 8));

			return nRet;
		}

		bool ParseADTSBuffer(AMLinearRingBuffer es_ring_buffer, unsigned long& Syncword_Pos)
		{
			int cbSize = 0;
			unsigned char* pBuf = AM_LRB_GetReadPtr(es_ring_buffer, &cbSize);

			unsigned char* pStartBuf = pBuf;
			unsigned long  cbBufSize = cbSize;
			bool bFirstSyncWordFound = false, bSecondSyncWordFound = false;

			while (cbSize >= ADTS_HEADER_SIZE)
			{
				uint16_t history = (*pBuf) << 8;
				// Find ADTS syncword 0xFFF
				while (cbSize >= 2 && ((history = (history << 8 | (*(pBuf + 1)))) & 0xFFF0) != 0xFFF0)
				{
					cbSize--;
					pBuf++;
				}

				if (cbSize < ADTS_HEADER_SIZE)
					return false;

				// Check whether it is a real compatible ADTS header
				uint64_t u32Val = *((uint32_t*)pBuf);
				ULONG_FIELD_ENDIAN(u32Val);

				uint32_t ID = (u32Val >> 19) & 0x1;
				uint32_t layer = (u32Val >> 17) & 03;
				uint32_t protection_absent = (u32Val >> 16) & 0x1;
				uint32_t profile_ObjectType = (u32Val >> 14) & 0x03;
				uint32_t sample_frequency_index = (u32Val >> 10) & 0x0F;
				uint32_t channel_configuration = (u32Val >> 6) & 0x07;
				if (layer != 0 // Should be set to '00'
					|| sample_frequency_index == 0x0F	// escape value
					|| (ID == 1 && profile_ObjectType == 3)	// it is reserved for MPEG-2 AAC
					|| sample_frequency_index == 0x0D || sample_frequency_index == 0x0E // reserved
					)
				{
					// It is not a real ADTS header, continue finding it
					cbSize++;
					pBuf++;
					continue;
				}

				unsigned short min_adts_frame_size = ADTS_HEADER_SIZE;
				unsigned short aac_frame_length = ((pBuf[3] & 0x03) << 11) | (pBuf[4] << 3) | ((pBuf[5] >> 5) & 0x7);
				unsigned short number_of_raw_data_blocks_in_frame = pBuf[6] & 0x03;
				if (protection_absent == 0)
				{
					min_adts_frame_size += number_of_raw_data_blocks_in_frame * sizeof(uint16_t)/* raw data block position */ + sizeof(uint16_t)/*crc_check*/;
					if (number_of_raw_data_blocks_in_frame > 0)
						min_adts_frame_size += sizeof(uint16_t);
				}

				if (aac_frame_length < min_adts_frame_size)
				{
					// It is not a real ADTS header, continue finding it
					cbSize++;
					pBuf++;
					continue;
				}

				if (cbSize < aac_frame_length)
				{
					// Skip the previous data before syncword
					AM_LRB_SkipReadPtr(es_ring_buffer, (unsigned int)(pBuf - pStartBuf));
					return false;
				}

				// Do CRC error correction
				if (protection_absent == 0)
				{

				}


			}

			return false;
		}

		int ADTSBitstream::Process(unsigned char* pBuf, unsigned long cbSize, int* pcbLend, int* pcbLeft)
		{
			unsigned char* pNewBuffer = pBuf;
			unsigned long cbNewSize = cbSize;

			int write_buf_len = 0;
			if (ring_buffer_not_processed == NULL)
				ring_buffer_not_processed = AM_LRB_Create(0x10000);	// 64KB

			unsigned char* write_buf = AM_LRB_GetWritePtr(ring_buffer_not_processed, &write_buf_len);
			if (write_buf == NULL || write_buf_len < (int)cbSize) {
				// Reform the linear ring buffer
				AM_LRB_Reform(ring_buffer_not_processed);
				write_buf = AM_LRB_GetWritePtr(ring_buffer_not_processed, &write_buf_len);

				if (write_buf == NULL || write_buf_len < (int)cbSize) {
					int lrb_size = AM_LRB_GetSize(ring_buffer_not_processed);
					if (lrb_size <= 0 || (long long)lrb_size * 2 > 0x7FFFFFFF || AMP_FAILED(AM_LRB_Resize(ring_buffer_not_processed, 2 * lrb_size))) {
						printf("The current payload size(%lld) seems to be so big or unexpected, can't support it.\n", (long long)lrb_size * 2);
						return RET_CODE_OUTOFMEMORY;
					}

					write_buf = AM_LRB_GetWritePtr(ring_buffer_not_processed, &write_buf_len);
				}

				if (write_buf == NULL || write_buf_len < (int)cbSize)
					return RET_CODE_OUT_OF_RANGE;
			}

			int read_buf_len = 0;
			unsigned char* pPayloadBuf = AM_LRB_GetReadPtr(ring_buffer_not_processed, pcbLend);

			// Write the current data to ring buffer
			memcpy(write_buf, pBuf, cbSize);
			if ((long long)AM_LRB_SkipWritePtr(ring_buffer_not_processed, cbSize) != (long long)cbSize){
				printf("Unexpected to write data with %lu bytes into the current payload ring buffer.\n", cbSize);
				return RET_CODE_MISMATCH;
			}
				
			// Update new buffer and new size
			pNewBuffer = pPayloadBuf = AM_LRB_GetReadPtr(ring_buffer_not_processed, &read_buf_len);
			cbNewSize = (unsigned long)read_buf_len;

			// If the syncword is not found, we need find the real sync-word
			// TODO...

			int cbUnProcessedBytes = 0;
			Map(pNewBuffer, cbNewSize, true, &cbUnProcessedBytes);

			AMP_SAFEASSIGN(pcbLeft, cbUnProcessedBytes);

			read_buf_len = 0;
			pPayloadBuf = AM_LRB_GetReadPtr(ring_buffer_not_processed, &read_buf_len);

			if (read_buf_len > cbUnProcessedBytes)
				AM_LRB_SkipReadPtr(ring_buffer_not_processed, (unsigned int)(read_buf_len - cbUnProcessedBytes));

			return cbUnProcessedBytes != 0 ? RET_CODE_NEEDMOREINPUT : RET_CODE_SUCCESS;
		}

		int ADTSBitstream::Map(unsigned char* pBuf, unsigned long cbSize, bool bOptOnlyMapCompleteFrame, int* pcbLeft)
		{
			int iRet = RET_CODE_ERROR;
			unsigned char* pStartBuf = pBuf;
			unsigned long  cbBufSize = cbSize;

			while (cbSize >= ADTS_HEADER_SIZE)
			{
				uint16_t history = (*pBuf);
				// Find ADTS syncword 0xFFF
				while (cbSize >= 2 && ((history = (history << 8 | (*(pBuf + 1)))) & 0xFFF0) != 0xFFF0)
				{
					cbSize--;
					pBuf++;
				}

				if (cbSize < ADTS_HEADER_SIZE)
					goto done;

				// Check whether it is a real compatible ADTS header
				uint64_t u32Val = *((uint32_t*)pBuf);
				ULONG_FIELD_ENDIAN(u32Val);

				uint32_t layer = (u32Val >> 17) & 03;
				uint32_t sample_frequency_index = (u32Val >> 10) & 0x0F;
				if (layer != 0 || sample_frequency_index == 0x0F)
				{
					// It is not a real ADTS header, continue finding it
					cbSize++;
					pBuf++;
					continue;
				}

				// Find the ADTS frame length
				unsigned short aac_frame_length = ((pBuf[3] & 0x03) << 11) | (pBuf[4] << 3) | ((pBuf[5] >> 5) & 0x7);

				if (bOptOnlyMapCompleteFrame && cbSize < aac_frame_length)
					goto done;

				unsigned long frame_buf_size = AMP_MIN(cbSize, aac_frame_length);
				AMBst bst = AMBst_CreateFromBuffer(pStartBuf, frame_buf_size);
				 
				AMP_NEWT1(ptr_adts_frame, ADTSFrame);
				AMBst_Seek(bst, (cbBufSize - cbSize) * 8);
				if (AMP_FAILED(iRet = ptr_adts_frame->Map(bst)))
					printf("[AAC] Failed to map adts_frame {retcode: %d}.\n", iRet);

				adts_frames.push_back(ptr_adts_frame);
				ptr_adts_frame = NULL;

				AMBst_Destroy(bst);

				cbSize -= frame_buf_size;
				pBuf += frame_buf_size;

				pStartBuf = pBuf;
				cbBufSize = cbSize;
			}

		done:
			AMP_SAFEASSIGN(pcbLeft, cbBufSize);

			return RET_CODE_SUCCESS;
		}

		int ADTSBitstream::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_ERROR;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);

			ADTSFrame* ptr_adts_frame = NULL;

			MAP_BST_BEGIN(0);

			// Try to find the syncword
			try
			{
				bool bContinue = false;
				do
				{
					while (AMBst_PeekBits(in_bst, 12) != 0xFFF)
						AMBst_SkipBits(in_bst, 8);

					// Check whether it is a real compatible ADTS header
					uint32_t u32Val = (uint32_t)AMBst_PeekBits(in_bst, 32);

					uint32_t layer = (u32Val >> 17) & 03;
					uint32_t sample_frequency_index = (u32Val >> 10) & 0x0F;
					if (layer != 0 || sample_frequency_index == 0x0F)
					{
						AMBst_SkipBits(in_bst, 8);
						bContinue = true;
						continue;
					}

					bContinue = false;
				} while (bContinue);
			}
			catch (...)
			{
				return RET_CODE_NEEDMOREINPUT;
			}

			do {
				AMP_NEWT1(ptr_adts_frame, ADTSFrame);
				if (ptr_adts_frame == nullptr)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					break;
				}

				if (AMP_FAILED(iRet = ptr_adts_frame->Map(in_bst)))
				{
					if (ptr_adts_frame->map_status.number_of_fields == 0)
					{
						if (iRet == RET_CODE_NO_MORE_DATA)
							iRet = RET_CODE_SUCCESS;

						AMP_SAFEDEL(ptr_adts_frame);
						break;
					}
					printf("[AAC] Failed to map ADTS frame {retcode: %d}.\n", iRet);
				}

				adts_frames.push_back(ptr_adts_frame);
				ptr_adts_frame = NULL;

				if (iRet == RET_CODE_NO_MORE_DATA)
				{
					iRet = RET_CODE_SUCCESS;
					break;
				}
			} while (AMP_SUCCEEDED(iRet));

			MAP_BST_END();
			return iRet;
		}

		int ADTSBitstream::ADTSFrame::Map(AMBst in_bst)
		{
			int iRet = RET_CODE_SUCCESS;
			SYNTAX_BITSTREAM_MAP::Map(in_bst);
			try
			{
				MAP_BST_BEGIN(0);
				if (AMP_FAILED(iRet = adts_fixed_header.Map(in_bst)))
					return iRet;

				map_status.number_of_fields++;
				if (AMP_FAILED(iRet = adts_variable_header.Map(in_bst)))
					return iRet;

				map_status.number_of_fields++;

				audio_ctx.sampling_frequency_index = adts_fixed_header.sampling_frequency_index;

				if (adts_variable_header.number_of_raw_data_blocks_in_frame == 0)
				{
					if (adts_fixed_header.protection_absent == 0)
					{
						adts_error_check = new ADTSErrorCheck();
						adts_error_check->crc_check = AMBst_GetWord(in_bst);
						map_status.number_of_fields++;
					}
					else
						adts_error_check = nullptr;

					raw_data_block = new RAW_DATA_BLOCK(&audio_ctx);
					raw_data_block->bit_pos = AMBst_Tell(in_bst);
					if (AMP_FAILED(iRet = raw_data_block->Unpack(in_bst)))
						printf("[AAC] Failed to map raw_data_block {retcode: %d}.\n", iRet);
				}
				else
				{
					if (adts_fixed_header.protection_absent == 0)
					{
						adts_header_error_check = new ADTSHeaderErrorCheck();
						adts_header_error_check->raw_data_block_position[0] = 0;
						for (uint32_t i = 1; i <= adts_variable_header.number_of_raw_data_blocks_in_frame; i++)
						{
							adts_header_error_check->raw_data_block_position[i] = AMBst_GetWord(in_bst);
							map_status.number_of_fields++;
						}
						adts_header_error_check->crc_check = AMBst_GetWord(in_bst);
						map_status.number_of_fields++;
					}
					else
						adts_header_error_check = NULL;

					memset(raw_data_blocks, 0, sizeof(raw_data_blocks));
					memset(adts_raw_data_block_error_checks, 0, sizeof(adts_raw_data_block_error_checks));
					for (uint32_t i = 0; i <= adts_variable_header.number_of_raw_data_blocks_in_frame; i++)
					{
						if (i > 0 && adts_fixed_header.protection_absent == 0)
						{
							int cur_bit_pos = AMBst_Tell(in_bst);
							if (((uint64_t)adts_header_error_check->raw_data_block_position[i] << 3) > (uint64_t)(cur_bit_pos - raw_data_blocks[0]->bit_pos))
								AMBst_SkipBits(in_bst, (int)(((uint64_t)adts_header_error_check->raw_data_block_position[i] << 3) - (cur_bit_pos - raw_data_blocks[0]->bit_pos)));
						}

						raw_data_blocks[i] = new RAW_DATA_BLOCK(&audio_ctx);
						raw_data_block->bit_pos = AMBst_Tell(in_bst);
						if (AMP_FAILED(iRet = raw_data_blocks[i]->Unpack(in_bst)))
						{
							printf("[AAC] Failed to map raw_data_block {retcode: %d}.\n", iRet);
							if (iRet == RET_CODE_NO_MORE_DATA) goto done;
						}

						map_status.number_of_fields++;

						if (adts_fixed_header.protection_absent == 0)
						{
							adts_raw_data_block_error_checks[i] = new ADTSRawDataBlockErrorCheck();
							adts_raw_data_block_error_checks[i]->crc_check = AMBst_GetWord(in_bst);
							map_status.number_of_fields++;
						}
					}
				}

				int cur_bit_pos = AMBst_Tell(in_bst);
				AMBst_SkipBits(in_bst, (adts_variable_header.aac_frame_length << 3) - (cur_bit_pos - bit_pos));
			}
			catch (AMException e)
			{
				return e.RetCode();
			}
			catch (std::out_of_range&)
			{
				map_status.number_of_fields = 0;
				return RET_CODE_NO_MORE_DATA;
			}
			catch (std::exception& e)
			{
				printf("exception: %s\n", e.what());
				return RET_CODE_ERROR;
			}

		done:
			MAP_BST_END();
			SYNTAX_BITSTREAM_MAP::EndMap(in_bst);

			return iRet;
		}

		CLOASParser::CLOASParser(RET_CODE* pRetCode)
			: m_loas_enum(nullptr)
			, m_loas_enum_options(UINT32_MAX)
		{
			RET_CODE retCode = CreateMP4AACContext(&m_pCtxMP4AAC);

			m_rbRawBuf = AM_LRB_Create(1024 * 128);

			AMP_SAFEASSIGN(pRetCode, retCode);
		}

		CLOASParser::~CLOASParser()
		{
			AMP_SAFERELEASE(m_loas_enum);
			AMP_SAFERELEASE(m_pCtxMP4AAC);
			AM_LRB_Destroy(m_rbRawBuf);
		}

		RET_CODE CLOASParser::SetEnumerator(IUnknown* pEnumerator, uint32_t options)
		{
			ILOASEnumerator* pLOASEumerator = nullptr;
			if (pEnumerator != nullptr)
			{
				if (FAILED(pEnumerator->QueryInterface(IID_ILOASEnumerator, (void**)&pLOASEumerator)))
				{
					return RET_CODE_ERROR;
				}
			}

			if (m_loas_enum)
				m_loas_enum->Release();

			m_loas_enum = pLOASEumerator;

			m_loas_enum_options = options;
			return RET_CODE_SUCCESS;
		}

		RET_CODE CLOASParser::ProcessInput(uint8_t* pInput, size_t cbInput)
		{
			int read_size = 0;
			RET_CODE iRet = RET_CODE_SUCCESS;

			if (pInput == NULL || cbInput == 0)
				return RET_CODE_INVALID_PARAMETER;

			uint8_t* pBuf = AM_LRB_GetWritePtr(m_rbRawBuf, &read_size);
			if (pBuf == NULL || read_size < 0 || (size_t)read_size < cbInput)
			{
				// Try to reform linear ring buffer
				AM_LRB_Reform(m_rbRawBuf);
				if ((pBuf = AM_LRB_GetWritePtr(m_rbRawBuf, &read_size)) == NULL || read_size < 0 || (size_t)read_size < cbInput)
				{
					printf("[NALParser] Failed to get the write buffer(%p), or the write buffer size(%d) is not enough.\n", pBuf, read_size);
					return RET_CODE_BUFFER_TOO_SMALL;
				}
			}

			memcpy(pBuf, pInput, cbInput);

			AM_LRB_SkipWritePtr(m_rbRawBuf, (unsigned int)cbInput);

			return RET_CODE_SUCCESS;
		}

		RET_CODE CLOASParser::ProcessOutput(bool bDrain)
		{
			int cbSize = 0;
			RET_CODE iRet = RET_CODE_SUCCESS;
			uint8_t* pStartBuf = NULL, *pBuf = NULL, *pCurParseStartBuf = NULL;

			if ((pStartBuf = AM_LRB_GetReadPtr(m_rbRawBuf, &cbSize)) == NULL || cbSize <= 0)
			{
				return RET_CODE_NEEDMOREINPUT;
			}

			int minimum_loas_parse_buffer_size = 3;
			pCurParseStartBuf = pBuf = pStartBuf;
			while (cbSize >= minimum_loas_parse_buffer_size)
			{
				// Find "syncword"
				while (cbSize >= minimum_loas_parse_buffer_size && (((*pBuf)<<3) | ((*(pBuf + 1)>>5)&0x7)) != 0x2B7)
				{
					cbSize--; pBuf++;
				}

				if (cbSize >= minimum_loas_parse_buffer_size)
				{
					uint16_t audioMuxLengthBytes = ((*(pBuf + 1) & 0x1F) << 8) | *(pBuf + 2);
					if (cbSize >= audioMuxLengthBytes + 3)
					{
						if (m_bSynced == false)
						{
							if (VerifyAudioMuxElement(1, pBuf + 3, audioMuxLengthBytes))
							{
								m_bSynced = true;
								printf("[MP4AAC] Successfully located the syncword in LOAS AudioMuxElement.\n");
							}
							else
							{
								cbSize--; pBuf++;
								continue;
							}
						}

						if ((*(pBuf + 3) & 0x80) == 0)	// useSameStreamMux = 0
						{
							AMBst in_bst = AMBst_CreateFromBuffer(pBuf + 3, audioMuxLengthBytes);
							if (in_bst != nullptr)
							{
								try
								{
									CStreamMuxConfig* pStreamMuxConfig = new CStreamMuxConfig();
									AMBst_SkipBits(in_bst, 1);
									if (AMP_SUCCEEDED(pStreamMuxConfig->Map(in_bst)))
									{
										MP4AMuxStreamConfig spStreamConfig = std::shared_ptr<CStreamMuxConfig>(pStreamMuxConfig);

										// generate the SHA1 for each AudioSpecificConfig
										for (uint16_t prog = 0; prog < 16; prog++)
										{
											for (uint16_t lay = 0; lay < 8; lay++)
											{
												if (pStreamMuxConfig->AudioSpecificConfig[prog][lay] == nullptr)
													continue;

												assert(pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos > 0 &&
													pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_end_pos > 0);

												AMSHA1 handleSHA1 = AM_SHA1_Init();

												int left_bits = pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_end_pos -
													pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos;

												AMBst_Seek(in_bst, pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos);
												while (left_bits >= 64)
												{
													uint64_t u64Val = AMBst_GetBits(in_bst, 64);
													AM_SHA1_Input(handleSHA1, (uint8_t*)(&u64Val), 8);
													left_bits -= 64;
												}

												if (left_bits > 0)
												{
													uint64_t u64Val = AMBst_GetBits(in_bst, left_bits);
													AM_SHA1_Input(handleSHA1, (uint8_t*)(&u64Val), 8);
													left_bits = 0;
												}

												AM_SHA1_Finalize(handleSHA1);
												AM_SHA1_GetHash(handleSHA1, pStreamMuxConfig->AudioSpecificConfig[prog][lay]->sha1_value);
												AM_SHA1_Uninit(handleSHA1);

												//auto h = pStreamMuxConfig->AudioSpecificConfig[prog][lay]->sha1_value;
												//printf("start_bitpos: %d, end_bitpos: %d, SHA1 value: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
												//	pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos,
												//	pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_end_pos, 
												//	h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7], h[8], h[9], h[10], h[11], h[12], h[13], h[14], h[15]);
											}
										}

										m_pCtxMP4AAC->UpdateMuxStreamConfig(spStreamConfig);
									}
									else
									{
										delete pStreamMuxConfig;
									}
								}
								catch(...)
								{ }
								AMBst_Destroy(in_bst);
							}
						}

						if (m_loas_enum)
						{
							int iEnumRet = RET_CODE_SUCCESS;
							if ((iEnumRet = m_loas_enum->EnumLATMAUBegin(m_pCtxMP4AAC, pBuf + 3, audioMuxLengthBytes)) == RET_CODE_ABORT)
							{
								iRet = iEnumRet;
								goto done;
							}

							if ((iEnumRet = m_loas_enum->EnumLATMAUEnd(m_pCtxMP4AAC, pBuf + 3, audioMuxLengthBytes)) == RET_CODE_ABORT)
							{
								iRet = iEnumRet;
								goto done;
							}
						}

						pBuf += (ptrdiff_t)audioMuxLengthBytes + 3;
						cbSize -= (ptrdiff_t)audioMuxLengthBytes + 3;
					}
					else
					{
						// the buffer is not enough
						iRet = RET_CODE_NEEDMOREINPUT;
						break;
					}
				}
			}

			AM_LRB_SkipReadPtr(m_rbRawBuf, (unsigned int)(pBuf - pCurParseStartBuf));

			if (bDrain)
			{
				AM_LRB_Reset(m_rbRawBuf);
				iRet = RET_CODE_SUCCESS;
			}
			else
			{
				iRet = RET_CODE_NEEDMOREINPUT;
			}

		done:
			return iRet;
		}

		RET_CODE CLOASParser::ProcessAU(uint8_t* pBuf, size_t cbBuf)
		{
			int iRet = RET_CODE_SUCCESS;
			if (pBuf == nullptr)
				return RET_CODE_NOTHING_TODO;
			
			// the ES payload which is an access unit is passed
			if (cbBuf >= 2)
			{
				uint16_t sync_word = (((*pBuf) << 3) | ((*(pBuf + 1) >> 5) & 0x7));
				if (sync_word == 0x2B7)
				{
					if (cbBuf >= 3)
					{
						uint16_t audioMuxLengthBytes = ((*(pBuf + 1) & 0x1F) << 8) | *(pBuf + 2);
						if (cbBuf >= (size_t)audioMuxLengthBytes + 3)
						{
							pBuf += 3;
							cbBuf -= 3;
						}
						else
							return RET_CODE_NEEDMOREINPUT;
					}
				}
			}

			if (cbBuf <= 1)
				return RET_CODE_NOTHING_TODO;

			if ((pBuf[0] & 0x80) == 0)	// useSameStreamMux = 0
			{
				AMBst in_bst = AMBst_CreateFromBuffer(pBuf, (int)cbBuf);
				if (in_bst != nullptr)
				{
					try
					{
						CStreamMuxConfig* pStreamMuxConfig = new CStreamMuxConfig();
						AMBst_SkipBits(in_bst, 1);
						if (AMP_SUCCEEDED(pStreamMuxConfig->Map(in_bst)))
						{
							MP4AMuxStreamConfig spStreamConfig = std::shared_ptr<CStreamMuxConfig>(pStreamMuxConfig);

							// generate the SHA1 for each AudioSpecificConfig
							for (uint16_t prog = 0; prog < 16; prog++)
							{
								for (uint16_t lay = 0; lay < 8; lay++)
								{
									if (pStreamMuxConfig->AudioSpecificConfig[prog][lay] == nullptr)
										continue;

									assert(pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos > 0 &&
										pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_end_pos > 0);

									AMSHA1 handleSHA1 = AM_SHA1_Init();

									int left_bits = pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_end_pos -
										pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos;

									AMBst_Seek(in_bst, pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos);
									while (left_bits >= 64)
									{
										uint64_t u64Val = AMBst_GetBits(in_bst, 64);
										AM_SHA1_Input(handleSHA1, (uint8_t*)(&u64Val), 8);
										left_bits -= 64;
									}

									if (left_bits > 0)
									{
										uint64_t u64Val = AMBst_GetBits(in_bst, left_bits);
										AM_SHA1_Input(handleSHA1, (uint8_t*)(&u64Val), 8);
										left_bits = 0;
									}

									AM_SHA1_Finalize(handleSHA1);
									AM_SHA1_GetHash(handleSHA1, pStreamMuxConfig->AudioSpecificConfig[prog][lay]->sha1_value);
									AM_SHA1_Uninit(handleSHA1);

									//auto h = pStreamMuxConfig->AudioSpecificConfig[prog][lay]->sha1_value;
									//printf("start_bitpos: %d, end_bitpos: %d, SHA1 value: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
									//	pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_pos,
									//	pStreamMuxConfig->AudioSpecificConfig[prog][lay]->bit_end_pos, 
									//	h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7], h[8], h[9], h[10], h[11], h[12], h[13], h[14], h[15]);
								}
							}

							m_pCtxMP4AAC->UpdateMuxStreamConfig(spStreamConfig);
						}
						else
						{
							delete pStreamMuxConfig;
						}
					}
					catch (...)
					{
					}
					AMBst_Destroy(in_bst);
				}
			}

			if (m_loas_enum)
			{
				int iEnumRet = RET_CODE_SUCCESS;
				if ((iEnumRet = m_loas_enum->EnumLATMAUBegin(m_pCtxMP4AAC, pBuf + 3, cbBuf)) == RET_CODE_ABORT)
				{
					iRet = iEnumRet;
					goto done;
				}

				if ((iEnumRet = m_loas_enum->EnumLATMAUEnd(m_pCtxMP4AAC, pBuf + 3, cbBuf)) == RET_CODE_ABORT)
				{
					iRet = iEnumRet;
					goto done;
				}
			}

		done:
			return iRet;
		}

		RET_CODE CLOASParser::GetContext(IUnknown** ppCtx)
		{
			if (ppCtx == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			m_pCtxMP4AAC->AddRef();
			*ppCtx = (IUnknown*)m_pCtxMP4AAC;
			return RET_CODE_SUCCESS;
		}

		RET_CODE CLOASParser::Reset()
		{
			AM_LRB_Reset(m_rbRawBuf);

			m_pCtxMP4AAC->Reset();

			m_bSynced = false;

			return RET_CODE_SUCCESS;
		}

		bool CLOASParser::VerifyAudioMuxElement(bool muxConfigPresent, uint8_t* pAudioMuxElement, int cbAudioMuxElement)
		{
			AMBst in_bst = AMBst_CreateFromBuffer(pAudioMuxElement, cbAudioMuxElement);
			if (in_bst == nullptr)
				return false;

			bool bRet = true;
			CStreamMuxConfig* pStreamMuxConfig = nullptr;
			try
			{
				if (muxConfigPresent)
				{
					if (!AMBst_GetBits(in_bst, 1)) {
						pStreamMuxConfig = new CStreamMuxConfig();
						int iMSMRet = pStreamMuxConfig->Map(in_bst);
						if (AMP_FAILED(iMSMRet))
						{
							bRet = false;
							goto done;
						}
					}
				}
			}
			catch (...)
			{
				bRet = false;
			}

		done:
			AMP_SAFEDEL(pStreamMuxConfig);
			AMBst_Destroy(in_bst);
			return bRet;
		}


		const std::tuple<const char*, int> audioProfileLevelIndication_Descs[256] = {
			/* 0x00 */		{ "Reserved for ISO use", -1 },
			/* 0x01 */		{ "Main Audio Profile", 1 },
			/* 0x02 */		{ "Main Audio Profile", 2 },
			/* 0x03 */		{ "Main Audio Profile", 3 },
			/* 0x04 */		{ "Main Audio Profile", 4 },
			/* 0x05 */		{ "Scalable Audio Profile", 1 },
			/* 0x06 */		{ "Scalable Audio Profile", 2 },
			/* 0x07 */		{ "Scalable Audio Profile", 3 },
			/* 0x08 */		{ "Scalable Audio Profile", 4 },
			/* 0x09 */		{ "Speech Audio Profile", 1 },
			/* 0x0A */		{ "Speech Audio Profile", 2 },
			/* 0x0B */		{ "Synthetic Audio Profile", 1 },
			/* 0x0C */		{ "Synthetic Audio Profile", 2 },
			/* 0x0D */		{ "Synthetic Audio Profile", 3 },
			/* 0x0E */		{ "High Quality Audio Profile", 1 },
			/* 0x0F */		{ "High Quality Audio Profile", 2 },
			/* 0x10 */		{ "High Quality Audio Profile", 3 },
			/* 0x11 */		{ "High Quality Audio Profile", 4 },
			/* 0x12 */		{ "High Quality Audio Profile", 5 },
			/* 0x13 */		{ "High Quality Audio Profile", 6 },
			/* 0x14 */		{ "High Quality Audio Profile", 7 },
			/* 0x15 */		{ "High Quality Audio Profile", 8 },
			/* 0x16 */		{ "Low Delay Audio Profile", 1 },
			/* 0x17 */		{ "Low Delay Audio Profile", 2 },
			/* 0x18 */		{ "Low Delay Audio Profile", 3 },
			/* 0x19 */		{ "Low Delay Audio Profile", 4 },
			/* 0x1A */		{ "Low Delay Audio Profile", 5 },
			/* 0x1B */		{ "Low Delay Audio Profile", 6 },
			/* 0x1C */		{ "Low Delay Audio Profile", 7 },
			/* 0x1D */		{ "Low Delay Audio Profile", 8 },
			/* 0x1E */		{ "Natural Audio Profile", 1 },
			/* 0x1F */		{ "Natural Audio Profile", 2 },
			/* 0x20 */		{ "Natural Audio Profile", 3 },
			/* 0x21 */		{ "Natural Audio Profile", 4 },
			/* 0x22 */		{ "Mobile Audio Internetworking Profile", 1 },
			/* 0x23 */		{ "Mobile Audio Internetworking Profile", 2 },
			/* 0x24 */		{ "Mobile Audio Internetworking Profile", 3 },
			/* 0x25 */		{ "Mobile Audio Internetworking Profile", 4 },
			/* 0x26 */		{ "Mobile Audio Internetworking Profile", 5 },
			/* 0x27 */		{ "Mobile Audio Internetworking Profile", 6 },
			/* 0x28 */		{ "AAC Profile", 1 },
			/* 0x29 */		{ "AAC Profile", 2 },
			/* 0x2A */		{ "AAC Profile", 4 },
			/* 0x2B */		{ "AAC Profile", 5 },
			/* 0x2C */		{ "High Efficiency AAC Profile", 2 },
			/* 0x2D */		{ "High Efficiency AAC Profile", 3 },
			/* 0x2E */		{ "High Efficiency AAC Profile", 4 },
			/* 0x2F */		{ "High Efficiency AAC Profile", 5 },
			/* 0x30 */		{ "High Efficiency AAC v2 Profile", 2 },
			/* 0x31 */		{ "High Efficiency AAC v2 Profile", 3 },
			/* 0x32 */		{ "High Efficiency AAC v2 Profile", 4 },
			/* 0x33 */		{ "High Efficiency AAC v2 Profile", 5 },
			/* 0x34 */		{ "Low Delay AAC Profile", 1 },
			/* 0x35 */		{ "Baseline MPEG Surround Profile (see ISO/IEC23003-1)", 1 },
			/* 0x36 */		{ "Baseline MPEG Surround Profile (see ISO/IEC23003-1)", 2 },
			/* 0x37 */		{ "Baseline MPEG Surround Profile (see ISO/IEC23003-1)", 3 },
			/* 0x38 */		{ "Baseline MPEG Surround Profile (see ISO/IEC23003-1)", 4 },
			/* 0c39 */		{ "Baseline MPEG Surround Profile (see ISO/IEC23003-1)", 5 },
			/* 0x3A */		{ "Baseline MPEG Surround Profile (see ISO/IEC23003-1)", 6 },
			/* 0x3B-0x3F */	{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },
			/* 0x40-0x4F */	{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },
			/* 0x50-0x5F */	{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },
			/* 0x60-0x6F */	{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },
			/* 0x70-0x7F */	{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },{ "reserved for ISO use", -1 },
			/* 0x80-0x8F */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0x90-0x9F */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xA0-0xAF */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xB0-0xBF */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xC0-0xCF */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xD0-0xDF */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xE0-0xEF */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xF0-0xFD */	{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },{ "user private", -1 },
			/* 0xFE */		{ "no audio profile specified", -1 },
			/* 0xFF */		{ "no audio capability required", -1 },
		};

		const char* Audio_Object_Type_Names[42] = {
			/*0  */ "NULL",
			/*1  */	"AAC MAIN",
			/*2  */	"AAC LC",
			/*3  */	"AAC SSR",
			/*4  */	"AAC LTP",
			/*5  */	"SBR",
			/*6  */	"AAC scalable",
			/*7  */	"TwinVQ",
			/*8  */	"CELP",
			/*9  */	"HVXC",
			/*10 */	"reserved",
			/*11 */	"reserved",
			/*12 */	"TTSI",
			/*13 */	"Main synthetic",
			/*14 */	"Wavetable synthesis",
			/*15 */	"General MIDI",
			/*16 */	"Algorithmic Synthesis and Audio FX",
			/*17 */	"ER AAC LC",
			/*18 */	"reserved",
			/*19 */	"ER AAC LTP",
			/*20 */	"ER AAC scalable",
			/*21 */	"ER Twin VQ",
			/*22 */	"ER BSAC",
			/*23 */	"ER AAC LD",
			/*24 */	"ER CELP",
			/*25 */	"ER HVXC",
			/*26 */	"ER HILN",
			/*27 */	"ER Parametric",
			/*28 */	"SSC",
			/*29 */	"PS",
			/*30 */	"MPEG Surround",
			/*31 */	"escape",
			/*32 */	"Layer-1",
			/*33 */	"Layer-2",
			/*34 */	"Layer-3",
			/*35 */	"DST",
			/*36 */	"ALS",
			/*37 */	"SLS",
			/*38 */	"SLS non-core",
			/*39 */	"ER AAC ELD",
			/*40 */	"SMR Simple",
			/*41 */	"SMR Main",
		};

		const std::tuple<int, const char*> samplingFrequencyIndex_names[16] = {
			/*0x0*/	{ 96000, "96000HZ"},
			/*0x1*/	{ 88200, "88200HZ"},
			/*0x2*/	{ 64000, "64000HZ"},
			/*0x3*/	{ 48000, "48000HZ"},
			/*0x4*/	{ 44100, "44100HZ"},
			/*0x5*/	{ 32000, "32000HZ"},
			/*0x6*/	{ 24000, "24000HZ"},
			/*0x7*/	{ 22050, "22050HZ"},
			/*0x8*/	{ 16000, "16000HZ"},
			/*0x9*/	{ 12000, "12000HZ"},
			/*0xa*/	{ 11025, "11025HZ"},
			/*0xb*/	{ 8000,  "8000HZ"},
			/*0xc*/	{-7350,  "7350HZ"},
			/*0xd*/	{-1, "reserved"},
			/*0xe*/	{-1, "reserved"},
			/*0xf*/	{-1, "escape value"}
		};

		const std::tuple<int, const char*, const char*> channelConfiguration_names[16] = {
			/*0	*/	{0, "", "defined in AOT related SpecificConfig"},
			/*1	*/	{1, "single_channel_element", "center front speaker"},
			/*2	*/	{2, "channel_pair_element", "left, right front speakers"},
			/*3	*/	{3, "single_channel_element, channel_pair_element", "center front speaker, left, right front speakers"},
			/*4	*/	{4, "single_channel_element, channel_pair_element, single_channel_element", "center front speaker, left, right center front speakers,rear surround speakers"},
			/*5	*/	{5, "single_channel_element, channel_pair_element, channel_pair_element", "center front speaker, left, right front speakers,left surround, right surround rear speakers"},
			/*6	*/	{6, "single_channel_element, channel_pair_element, channel_pair_element,lfe _element", "center front speaker, left, right front speakers,left surround, right surround rear speakers,front low frequency effects speaker"},
			/*7	*/	{8, "single_channel_element, channel_pair_element, channel_pair_element,channel_pair_element,lfe_element", "center front speaker left, right center front speakers,left, right outside front speakers,left surround, right surround rear speakers,front low frequency effects speaker"},
			/*8	*/	{0, "", "reserved"},
			/*9	*/	{0, "", "reserved"},
			/*10*/	{0, "", "reserved"},
			/*11*/	{0, "", "reserved"},
			/*12*/	{0, "", "reserved"},
			/*13*/	{0, "", "reserved"},
			/*14*/	{0, "", "reserved"},
			/*15*/	{0, "", "reserved"}
		};

		const uint8_t PRED_SFB_MAX[13] = {
			33, 33, 38, 40, 40, 40, 41, 41, 37, 37, 37, 34, 34
		};

		/*
		Table 4.129 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 44.1 and 48 kHz
		*/
		const uint16_t table_4_129[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160, 176, 196,
			216, 240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 1024
		};

		/*
		Table 4.130 scalefactor bands for a window length of 256 and 240 (values for 240 in brackets) for
		SHORT_WINDOW at 32, 44.1 and 48 kHz
		*/
		const uint16_t table_4_130[] = {
			0, 4, 8, 12, 16, 20, 28, 36, 44, 56, 68, 80, 96, 112, 128
		};

		/*
		Table 4.131 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 32 kHz
		*/
		const uint16_t table_4_131[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 80, 88, 96, 108, 120, 132, 144, 160, 176, 196, 216,
			240, 264, 292, 320, 352, 384, 416, 448, 480, 512, 544, 576, 608, 640, 672, 704, 736, 768, 800, 832, 864, 896, 928, 960, 992, 1024
		};

		/*
		Table 4.132 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 8 kHz
		*/
		const uint16_t table_4_132[] = {
			0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 172, 188, 204, 220, 236, 252, 268,
			288, 308, 328, 348, 372, 396, 420, 448, 476, 508, 544, 580, 620, 664, 712, 764, 820, 880, 944, 1024
		};

		/*
		Table 4.133 scalefactor bands for a window length of 256 and 240 (values for 240 in brackets) for
		SHORT_WINDOW at 8 kHz
		*/
		const uint16_t table_4_133[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 60, 72, 88, 108, 128
		};

		/*
		Table 4.134 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 11.025, 12 and 16 kHz
		*/
		const uint16_t table_4_134[] = {
			0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 100, 112, 124, 136, 148, 160, 172, 184, 196, 212,
			228, 244, 260, 280, 300, 320, 344, 368, 396, 424, 456, 492, 532, 572, 616, 664, 716, 772, 832, 896, 960, 1024
		};

		/*
		Table 4.135 scalefactor bands for a window length of 256 and 240 (values for 240 in brackets) for
		SHORT_WINDOW at 11.025, 12 and 16 kHz
		*/
		const uint16_t table_4_135[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 40, 48, 60, 72, 88, 108, 128
		};

		/*
		Table 4.136 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 22.05 and 24 kHz
		*/
		const uint16_t table_4_136[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 76, 84, 92, 100, 108, 116, 124, 136, 148,
			160, 172, 188, 204, 220, 240, 260, 284, 308, 336, 364, 396, 432, 468, 508, 552, 600, 652, 704, 768, 832, 896, 960, 1024
		};

		/*
		Table 4.137 scalefactor bands for a window length of 256 and 240 (values for 240 in brackets) for
		SHORT_WINDOW at 22.05 and 24 kHz
		*/
		const uint16_t table_4_137[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 36, 44, 52, 64, 76, 92, 108, 128
		};

		/*
		Table 4.138 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 64 kHz
		*/
		const uint16_t table_4_138[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 100, 112, 124, 140, 156,
			172, 192, 216, 240, 268, 304, 344, 384, 424, 464, 504, 544, 584, 624, 664, 704, 744, 784, 824, 864, 904, 944, 984, 1024
		};

		/*
		Table 4.139 scalefactor bands for a window length of 256 and 240 (values for 240 in brackets) for
		SHORT_WINDOW at 64 kHz
		*/
		const uint16_t table_4_139[] = {
			0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128
		};

		/*
		Table 4.140 scalefactor bands for a window length of 2048 and 1920 (values for 1920 in brackets) for
		LONG_WINDOW, LONG_START_WINDOW, LONG_STOP_WINDOW at 88.2 and 96 kHz
		*/
		const uint16_t table_4_140[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88, 96, 108,
			120, 132, 144, 156, 172, 188, 212, 240, 276, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024
		};

		/*
		Table 4.141 scalefactor bands for a window length of 256 and 240 (values for 240 in brackets) for
		SHORT_WINDOW at 88.2 and 96 kHz
		*/
		const uint16_t table_4_141[] = {
			0, 4, 8, 12, 16, 20, 24, 32, 40, 48, 64, 92, 128
		};

		/*
		Table 4.142 scalefactor bands for a window length of 960 at 44.1 and 48 kHz
		*/
		const uint16_t table_4_142[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80,
			88, 96, 108, 120, 132, 144, 156, 172, 188, 212, 240, 272, 304, 336, 368, 400, 432, 480
		};

		/*
		Table 4.143 scalefactor bands for a window length of 1024 at 44.1 and 48 kHz
		*/
		const uint16_t table_4_143[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 68, 76, 84,
			92, 100, 112, 124, 136, 148, 164, 184, 208, 236, 268, 300, 332, 364, 396, 428, 460, 512
		};

		/*
		Table 4.144 scalefactor bands for a window length of 960 at 32 kHz	
		*/
		const uint16_t table_4_144[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 72, 80,
			88, 96, 104, 112, 124, 136, 148, 164, 180, 200, 224, 256, 288, 320, 352, 384, 416, 448, 480
		};

		/*
		Table 4.145 scalefactor bands for a window length of 1024 at 32 kHz
		*/
		const uint16_t table_4_145[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64, 72, 80, 88,
			96, 108, 120, 132, 144, 160, 176, 192, 212, 236, 260, 288, 320, 352, 384, 416, 448, 480, 512
		};

		/*
		Table 4.146 scalefactor bands for a window length of 960 at 22.05 and 24 kHz
		*/
		const uint16_t table_4_146[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 80,
			92, 104, 120, 140, 164, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480
		};

		/*
		Table 4.147 scalefactor bands for a window length of 1024 at 22.05 and 24 kHz
		*/
		const uint16_t table_4_147[] = {
			0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 52, 60, 68, 80,
			92, 104, 120, 140, 164, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512
		};

		#define ARRAY_PAIR(arr)	{arr, _countof(arr)}
		#define ARRAY_NULL		{nullptr, 0}

		const std::tuple<const uint16_t*, size_t> swb_offset_window[3][12][4] = {
			// Window Length: 960
			{
				/* 96KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 88.2KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 64KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 48KHZ	 */{ ARRAY_PAIR(table_4_143),ARRAY_PAIR(table_4_143),ARRAY_PAIR(table_4_143),ARRAY_PAIR(table_4_143), },
				/* 44.1KHZ	 */{ ARRAY_PAIR(table_4_143),ARRAY_PAIR(table_4_143),ARRAY_PAIR(table_4_143),ARRAY_PAIR(table_4_143), },
				/* 32KHZ	 */{ ARRAY_PAIR(table_4_145),ARRAY_PAIR(table_4_145),ARRAY_PAIR(table_4_145),ARRAY_PAIR(table_4_145), },
				/* 24KHZ	 */{ ARRAY_PAIR(table_4_147),ARRAY_PAIR(table_4_147),ARRAY_PAIR(table_4_147),ARRAY_PAIR(table_4_147), },
				/* 22.05KHZ	 */{ ARRAY_PAIR(table_4_147),ARRAY_PAIR(table_4_147),ARRAY_PAIR(table_4_147),ARRAY_PAIR(table_4_147), },
				/* 16KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 12kHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 11.025KHZ */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 8KHZ		 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
			}
			// Window Length: 1024
			,
			{
				/* 96KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 88.2KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 64KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 48KHZ	 */{ ARRAY_PAIR(table_4_142),ARRAY_PAIR(table_4_142),ARRAY_PAIR(table_4_142),ARRAY_PAIR(table_4_142), },
				/* 44.1KHZ	 */{ ARRAY_PAIR(table_4_142),ARRAY_PAIR(table_4_142),ARRAY_PAIR(table_4_142),ARRAY_PAIR(table_4_142), },
				/* 32KHZ	 */{ ARRAY_PAIR(table_4_144),ARRAY_PAIR(table_4_144),ARRAY_PAIR(table_4_144),ARRAY_PAIR(table_4_144), },
				/* 24KHZ	 */{ ARRAY_PAIR(table_4_146),ARRAY_PAIR(table_4_146),ARRAY_PAIR(table_4_146),ARRAY_PAIR(table_4_146), },
				/* 22.05KHZ	 */{ ARRAY_PAIR(table_4_146),ARRAY_PAIR(table_4_146),ARRAY_PAIR(table_4_146),ARRAY_PAIR(table_4_146), },
				/* 16KHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 12kHZ	 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 11.025KHZ */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
				/* 8KHZ		 */{ ARRAY_NULL,ARRAY_NULL,ARRAY_NULL,ARRAY_NULL, },
			}
			,
			// Window Length: 2048/1920
			{
				/* 96KHZ	 */{ ARRAY_PAIR(table_4_140),ARRAY_PAIR(table_4_140),ARRAY_PAIR(table_4_141),ARRAY_PAIR(table_4_140), },
				/* 88.2KHZ	 */{ ARRAY_PAIR(table_4_140),ARRAY_PAIR(table_4_140),ARRAY_PAIR(table_4_141),ARRAY_PAIR(table_4_140), },
				/* 64KHZ	 */{ ARRAY_PAIR(table_4_138),ARRAY_PAIR(table_4_138),ARRAY_PAIR(table_4_139),ARRAY_PAIR(table_4_138), },
				/* 48KHZ	 */{ ARRAY_PAIR(table_4_129),ARRAY_PAIR(table_4_129),ARRAY_PAIR(table_4_130),ARRAY_PAIR(table_4_129), },
				/* 44.1KHZ	 */{ ARRAY_PAIR(table_4_129),ARRAY_PAIR(table_4_129),ARRAY_PAIR(table_4_130),ARRAY_PAIR(table_4_129), },
				/* 32KHZ	 */{ ARRAY_PAIR(table_4_131),ARRAY_PAIR(table_4_131),ARRAY_PAIR(table_4_130),ARRAY_PAIR(table_4_131), },
				/* 24KHZ	 */{ ARRAY_PAIR(table_4_136),ARRAY_PAIR(table_4_136),ARRAY_PAIR(table_4_137),ARRAY_PAIR(table_4_136), },
				/* 22.05KHZ	 */{ ARRAY_PAIR(table_4_136),ARRAY_PAIR(table_4_136),ARRAY_PAIR(table_4_137),ARRAY_PAIR(table_4_136), },
				/* 16KHZ	 */{ ARRAY_PAIR(table_4_134),ARRAY_PAIR(table_4_134),ARRAY_PAIR(table_4_135),ARRAY_PAIR(table_4_134), },
				/* 12kHZ	 */{ ARRAY_PAIR(table_4_134),ARRAY_PAIR(table_4_134),ARRAY_PAIR(table_4_135),ARRAY_PAIR(table_4_134), },
				/* 11.025KHZ */{ ARRAY_PAIR(table_4_134),ARRAY_PAIR(table_4_134),ARRAY_PAIR(table_4_135),ARRAY_PAIR(table_4_134), },
				/* 8KHZ		 */{ ARRAY_PAIR(table_4_132),ARRAY_PAIR(table_4_132),ARRAY_PAIR(table_4_133),ARRAY_PAIR(table_4_132), },
			}
		};

		uint16_t CELP_Layer0_frameLen4[64] = {
			154,170,186,147,156,165,114,120,126,132,138,142,146,154,166,174,182,190,198,206,210,214,110,114,118,120,122,186,
			218,230,242,254,266,278,286,294,318,342,358,374,390,406,422,136,142,148,154,160,166,170,174,186,198,206,214,222,230,238,216,160,280,338,0,0,};

		uint16_t CELP_Layer1_5_frameLen4[20] = {
			80,60,40,20,368,416,464,496,284,320,356,380,200,224,248,264,116,128,140,148,};

		uint16_t CELP_Layer0_frameLen5[64][4] = {
			{156, 23, 8, 2},
			{172, 23, 8, 2},
			{188, 23, 8, 2},
			{149, 23, 8, 2},
			{158, 23, 8, 2},
			{167, 23, 8, 2},
			{116, 23, 8, 2},
			{122, 23, 8, 2},
			{128, 23, 8, 2},
			{134, 23, 8, 2},
			{140, 23, 8, 2},
			{144, 23, 8, 2},
			{148, 23, 8, 2},
			{156, 23, 8, 2},
			{168, 23, 8, 2},
			{176, 23, 8, 2},
			{184, 23, 8, 2},
			{192, 23, 8, 2},
			{200, 23, 8, 2},
			{208, 23, 8, 2},
			{212, 23, 8, 2},
			{216, 23, 8, 2},
			{112, 23, 8, 2},
			{116, 23, 8, 2},
			{120, 23, 8, 2},
			{122, 23, 8, 2},
			{124, 23, 8, 2},
			{188, 23, 8, 2},
			{220, 40, 8, 2},
			{232, 40, 8, 2},
			{244, 40, 8, 2},
			{256, 40, 8, 2},
			{268, 40, 8, 2},
			{280, 40, 8, 2},
			{288, 40, 8, 2},
			{296, 40, 8, 2},
			{320, 40, 8, 2},
			{344, 40, 8, 2},
			{360, 40, 8, 2},
			{376, 40, 8, 2},
			{392, 40, 8, 2},
			{408, 40, 8, 2},
			{424, 40, 8, 2},
			{138, 40, 8, 2},
			{144, 40, 8, 2},
			{150, 40, 8, 2},
			{156, 40, 8, 2},
			{162, 40, 8, 2},
			{168, 40, 8, 2},
			{172, 40, 8, 2},
			{176, 40, 8, 2},
			{188, 40, 8, 2},
			{200, 40, 8, 2},
			{208, 40, 8, 2},
			{216, 40, 8, 2},
			{224, 40, 8, 2},
			{232, 40, 8, 2},
			{240, 40, 8, 2},
			{218, 40, 8, 2},
			{162, 40, 8, 2},
			{282, 40, 8, 2},
			{340, 40, 8, 2},
			{0, 0, 0, 0},
			{0, 0, 0, 0}};

		uint16_t CELP_Layer1_5_frameLen5[20][4] = {
			{ 80,  0, 0, 0},
			{ 60,  0, 0, 0},
			{ 40,  0, 0, 0},
			{ 20,  0, 0, 0},
			{368, 21, 0, 0},
			{416, 21, 0, 0},
			{464, 21, 0, 0},
			{496, 21, 0, 0},
			{284, 21, 0, 0},
			{320, 21, 0, 0},
			{356, 21, 0, 0},
			{380, 21, 0, 0},
			{200, 21, 0, 0},
			{224, 21, 0, 0},
			{248, 21, 0, 0},
			{264, 21, 0, 0},
			{116, 21, 0, 0},
			{128, 21, 0, 0},
			{140, 21, 0, 0},
			{148, 21, 0, 0},
		};

		uint16_t CELP_Layer0_frameLen3[64][2] = {
			{156, 134},
			{172, 150},
			{188, 166},
			{149, 127},
			{158, 136},
			{167, 145},
			{116, 94 },
			{122, 100},
			{128, 106},
			{134, 112},
			{140, 118},
			{144, 122},
			{148, 126},
			{156, 134},
			{168, 146},
			{176, 154},
			{184, 162},
			{192, 170},
			{200, 178},
			{208, 186},
			{212, 190},
			{216, 194},
			{112, 90 },
			{116, 94 },
			{120, 98 },
			{122, 100},
			{124, 102},
			{188, 166},
			{220, 174},
			{232, 186},
			{244, 198},
			{256, 210},
			{268, 222},
			{280, 234},
			{288, 242},
			{296, 250},
			{320, 276},
			{344, 298},
			{360, 314},
			{376, 330},
			{392, 346},
			{408, 362},
			{424, 378},
			{138, 92 },
			{144, 98 },
			{150, 104},
			{156, 110},
			{162, 116},
			{168, 122},
			{172, 126},
			{176, 130},
			{188, 142},
			{200, 154},
			{208, 162},
			{216, 170},
			{224, 178},
			{232, 186},
			{240, 194},
			{218, 172},
			{162, 116},
			{282, 238},
			{340, 296},
			{  0,   0},
			{  0,   0},
		};

		const Huffman_Codebook scalefactor_hcb = {
			{ 0   ,18 ,0x3ffe8 },	{ 1   ,18 ,0x3ffe6 },	{ 2   ,18 ,0x3ffe7 },	{ 3   ,18 ,0x3ffe5 },	{ 4   ,19 ,0x7fff5 },	{ 5   ,19 ,0x7fff1 },
			{ 6   ,19 ,0x7ffed },	{ 7   ,19 ,0x7fff6 },	{ 8   ,19 ,0x7ffee },	{ 9   ,19 ,0x7ffef },	{ 10  ,19 ,0x7fff0 },	{ 11  ,19 ,0x7fffc },
			{ 12  ,19 ,0x7fffd },	{ 13  ,19 ,0x7ffff },	{ 14  ,19 ,0x7fffe },	{ 15  ,19 ,0x7fff7 },	{ 16  ,19 ,0x7fff8 },	{ 17  ,19 ,0x7fffb },
			{ 18  ,19 ,0x7fff9 },	{ 19  ,18 ,0x3ffe4 },	{ 20  ,19 ,0x7fffa },	{ 21  ,18 ,0x3ffe3 },	{ 22  ,17 ,0x1ffef },	{ 23  ,17 ,0x1fff0 },
			{ 24  ,16 ,0xfff5 },	{ 25  ,17 ,0x1ffee },	{ 26  ,16 ,0xfff2 },	{ 27  ,16 ,0xfff3 },	{ 28  ,16 ,0xfff4 },	{ 29  ,16 ,0xfff1 },
			{ 30  ,15 ,0x7ff6 },	{ 31  ,15 ,0x7ff7 },	{ 32  ,14 ,0x3ff9 },	{ 33  ,14 ,0x3ff5 },	{ 34  ,14 ,0x3ff7 },	{ 35  ,14 ,0x3ff3 },
			{ 36  ,14 ,0x3ff6 },	{ 37  ,14 ,0x3ff2 },	{ 38  ,13 ,0x1ff7 },	{ 39  ,13 ,0x1ff5 },	{ 40  ,12 ,0xff9 },		{ 41  ,12 ,0xff7 },
			{ 42  ,12 ,0xff6 },		{ 43  ,11 ,0x7f9 },		{ 44  ,12 ,0xff4 },		{ 45  ,11 ,0x7f8 },		{ 46  ,10 ,0x3f9 },		{ 47  ,10 ,0x3f7 },
			{ 48  ,10 ,0x3f5 },		{ 49  ,9  ,0x1f8 },		{ 50  ,9  ,0x1f7 },		{ 51  ,8  ,0xfa },		{ 52  ,8  ,0xf8 },		{ 53  ,8  ,0xf6 },
			{ 54  ,7  ,0x79 },		{ 55  ,6  ,0x3a },		{ 56  ,6  ,0x38 },		{ 57  ,5  ,0x1a },		{ 58  ,4  ,0xb },   	{ 59  ,3  ,0x4 },
			{ 60  ,1  ,0x0 },		{ 61  ,4  ,0xa },		{ 62  ,4  ,0xc },		{ 63  ,5  ,0x1b },		{ 64  ,6  ,0x39 },		{ 65  ,6  ,0x3b },
			{ 66  ,7  ,0x78 },		{ 67  ,7  ,0x7a },		{ 68  ,8  ,0xf7 },		{ 69  ,8  ,0xf9 },		{ 70  ,9  ,0x1f6 },		{ 71  ,9  ,0x1f9 },
			{ 72  ,10 ,0x3f4 },		{ 73  ,10 ,0x3f6 },		{ 74  ,10 ,0x3f8 },		{ 75  ,11 ,0x7f5 },		{ 76  ,11 ,0x7f4 },		{ 77  ,11 ,0x7f6 },
			{ 78  ,11 ,0x7f7 },		{ 79  ,12 ,0xff5 },		{ 80  ,12 ,0xff8 },		{ 81  ,13 ,0x1ff4 },	{ 82  ,13 ,0x1ff6 },	{ 83  ,13 ,0x1ff8 },
			{ 84  ,14 ,0x3ff8 },	{ 85  ,14 ,0x3ff4 },	{ 86  ,16 ,0xfff0 },	{ 87  ,15 ,0x7ff4 },	{ 88  ,16 ,0xfff6 },	{ 89  ,15 ,0x7ff5 },
			{ 90  ,18 ,0x3ffe2 },	{ 91  ,19 ,0x7ffd9 },	{ 92  ,19 ,0x7ffda },	{ 93  ,19 ,0x7ffdb },	{ 94  ,19 ,0x7ffdc },	{ 95  ,19 ,0x7ffdd },
			{ 96  ,19 ,0x7ffde },	{ 97  ,19 ,0x7ffd8 },	{ 98  ,19 ,0x7ffd2 },	{ 99  ,19 ,0x7ffd3 },	{ 100 ,19 ,0x7ffd4 },	{ 101 ,19 ,0x7ffd5 },
			{ 102 ,19 ,0x7ffd6 },	{ 103 ,19 ,0x7fff2 },	{ 104 ,19 ,0x7ffdf },	{ 105 ,19 ,0x7ffe7 },	{ 106 ,19 ,0x7ffe8 },	{ 107 ,19 ,0x7ffe9 },
			{ 108 ,19 ,0x7ffea },	{ 109 ,19 ,0x7ffeb },	{ 110 ,19 ,0x7ffe6 },	{ 111 ,19 ,0x7ffe0 },	{ 112 ,19 ,0x7ffe1 },	{ 113 ,19 ,0x7ffe2 },
			{ 114 ,19 ,0x7ffe3 },	{ 115 ,19 ,0x7ffe4 },	{ 116 ,19 ,0x7ffe5 },	{ 117 ,19 ,0x7ffd7 },	{ 118 ,19 ,0x7ffec },	{ 119 ,19 ,0x7fff4 },
			{ 120 ,19 ,0x7fff3 },
		};

		const int scalefactor_hcb_bst[241][2] = {
			{ 1, 2 },
			{ 60, 0 },                            // leaf node (V:60, L:1 C:0X0)
			{ 1, 2 },
			{ 2, 3 },
			{ 3, 4 },
			{ 59, 0 },                            // leaf node (V:59, L:3 C:0X4)
			{ 3, 4 },
			{ 4, 5 },
			{ 5, 6 },
			{ 61, 0 },                            // leaf node (V:61, L:4 C:0XA)
			{ 58, 0 },                            // leaf node (V:58, L:4 C:0XB)
			{ 62, 0 },                            // leaf node (V:62, L:4 C:0XC)
			{ 3, 4 },
			{ 4, 5 },
			{ 5, 6 },
			{ 57, 0 },                            // leaf node (V:57, L:5 C:0X1A)
			{ 63, 0 },                            // leaf node (V:63, L:5 C:0X1B)
			{ 4, 5 },
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 8 },
			{ 56, 0 },                            // leaf node (V:56, L:6 C:0X38)
			{ 64, 0 },                            // leaf node (V:64, L:6 C:0X39)
			{ 55, 0 },                            // leaf node (V:55, L:6 C:0X3A)
			{ 65, 0 },                            // leaf node (V:65, L:6 C:0X3B)
			{ 4, 5 },
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 8 },
			{ 66, 0 },                            // leaf node (V:66, L:7 C:0X78)
			{ 54, 0 },                            // leaf node (V:54, L:7 C:0X79)
			{ 67, 0 },                            // leaf node (V:67, L:7 C:0X7A)
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 53, 0 },                            // leaf node (V:53, L:8 C:0XF6)
			{ 68, 0 },                            // leaf node (V:68, L:8 C:0XF7)
			{ 52, 0 },                            // leaf node (V:52, L:8 C:0XF8)
			{ 69, 0 },                            // leaf node (V:69, L:8 C:0XF9)
			{ 51, 0 },                            // leaf node (V:51, L:8 C:0XFA)
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 70, 0 },                            // leaf node (V:70, L:9 C:0X1F6)
			{ 50, 0 },                            // leaf node (V:50, L:9 C:0X1F7)
			{ 49, 0 },                            // leaf node (V:49, L:9 C:0X1F8)
			{ 71, 0 },                            // leaf node (V:71, L:9 C:0X1F9)
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 72, 0 },                            // leaf node (V:72, L:10 C:0X3F4)
			{ 48, 0 },                            // leaf node (V:48, L:10 C:0X3F5)
			{ 73, 0 },                            // leaf node (V:73, L:10 C:0X3F6)
			{ 47, 0 },                            // leaf node (V:47, L:10 C:0X3F7)
			{ 74, 0 },                            // leaf node (V:74, L:10 C:0X3F8)
			{ 46, 0 },                            // leaf node (V:46, L:10 C:0X3F9)
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 76, 0 },                            // leaf node (V:76, L:11 C:0X7F4)
			{ 75, 0 },                            // leaf node (V:75, L:11 C:0X7F5)
			{ 77, 0 },                            // leaf node (V:77, L:11 C:0X7F6)
			{ 78, 0 },                            // leaf node (V:78, L:11 C:0X7F7)
			{ 45, 0 },                            // leaf node (V:45, L:11 C:0X7F8)
			{ 43, 0 },                            // leaf node (V:43, L:11 C:0X7F9)
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 44, 0 },                            // leaf node (V:44, L:12 C:0XFF4)
			{ 79, 0 },                            // leaf node (V:79, L:12 C:0XFF5)
			{ 42, 0 },                            // leaf node (V:42, L:12 C:0XFF6)
			{ 41, 0 },                            // leaf node (V:41, L:12 C:0XFF7)
			{ 80, 0 },                            // leaf node (V:80, L:12 C:0XFF8)
			{ 40, 0 },                            // leaf node (V:40, L:12 C:0XFF9)
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 81, 0 },                            // leaf node (V:81, L:13 C:0X1FF4)
			{ 39, 0 },                            // leaf node (V:39, L:13 C:0X1FF5)
			{ 82, 0 },                            // leaf node (V:82, L:13 C:0X1FF6)
			{ 38, 0 },                            // leaf node (V:38, L:13 C:0X1FF7)
			{ 83, 0 },                            // leaf node (V:83, L:13 C:0X1FF8)
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 12, 13 },
			{ 13, 14 },
			{ 37, 0 },                            // leaf node (V:37, L:14 C:0X3FF2)
			{ 35, 0 },                            // leaf node (V:35, L:14 C:0X3FF3)
			{ 85, 0 },                            // leaf node (V:85, L:14 C:0X3FF4)
			{ 33, 0 },                            // leaf node (V:33, L:14 C:0X3FF5)
			{ 36, 0 },                            // leaf node (V:36, L:14 C:0X3FF6)
			{ 34, 0 },                            // leaf node (V:34, L:14 C:0X3FF7)
			{ 84, 0 },                            // leaf node (V:84, L:14 C:0X3FF8)
			{ 32, 0 },                            // leaf node (V:32, L:14 C:0X3FF9)
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 87, 0 },                            // leaf node (V:87, L:15 C:0X7FF4)
			{ 89, 0 },                            // leaf node (V:89, L:15 C:0X7FF5)
			{ 30, 0 },                            // leaf node (V:30, L:15 C:0X7FF6)
			{ 31, 0 },                            // leaf node (V:31, L:15 C:0X7FF7)
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 12, 13 },
			{ 13, 14 },
			{ 14, 15 },
			{ 15, 16 },
			{ 86, 0 },                            // leaf node (V:86, L:16 C:0XFFF0)
			{ 29, 0 },                            // leaf node (V:29, L:16 C:0XFFF1)
			{ 26, 0 },                            // leaf node (V:26, L:16 C:0XFFF2)
			{ 27, 0 },                            // leaf node (V:27, L:16 C:0XFFF3)
			{ 28, 0 },                            // leaf node (V:28, L:16 C:0XFFF4)
			{ 24, 0 },                            // leaf node (V:24, L:16 C:0XFFF5)
			{ 88, 0 },                            // leaf node (V:88, L:16 C:0XFFF6)
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 12, 13 },
			{ 13, 14 },
			{ 14, 15 },
			{ 15, 16 },
			{ 16, 17 },
			{ 17, 18 },
			{ 25, 0 },                            // leaf node (V:25, L:17 C:0X1FFEE)
			{ 22, 0 },                            // leaf node (V:22, L:17 C:0X1FFEF)
			{ 23, 0 },                            // leaf node (V:23, L:17 C:0X1FFF0)
			{ 15, 16 },
			{ 16, 17 },
			{ 17, 18 },
			{ 18, 19 },
			{ 19, 20 },
			{ 20, 21 },
			{ 21, 22 },
			{ 22, 23 },
			{ 23, 24 },
			{ 24, 25 },
			{ 25, 26 },
			{ 26, 27 },
			{ 27, 28 },
			{ 28, 29 },
			{ 29, 30 },
			{ 90, 0 },                            // leaf node (V:90, L:18 C:0X3FFE2)
			{ 21, 0 },                            // leaf node (V:21, L:18 C:0X3FFE3)
			{ 19, 0 },                            // leaf node (V:19, L:18 C:0X3FFE4)
			{ 3, 0 },                             // leaf node (V:3, L:18 C:0X3FFE5)
			{ 1, 0 },                             // leaf node (V:1, L:18 C:0X3FFE6)
			{ 2, 0 },                             // leaf node (V:2, L:18 C:0X3FFE7)
			{ 0, 0 },                             // leaf node (V:0, L:18 C:0X3FFE8)
			{ 23, 24 },
			{ 24, 25 },
			{ 25, 26 },
			{ 26, 27 },
			{ 27, 28 },
			{ 28, 29 },
			{ 29, 30 },
			{ 30, 31 },
			{ 31, 32 },
			{ 32, 33 },
			{ 33, 34 },
			{ 34, 35 },
			{ 35, 36 },
			{ 36, 37 },
			{ 37, 38 },
			{ 38, 39 },
			{ 39, 40 },
			{ 40, 41 },
			{ 41, 42 },
			{ 42, 43 },
			{ 43, 44 },
			{ 44, 45 },
			{ 45, 46 },
			{ 98, 0 },                            // leaf node (V:98, L:19 C:0X7FFD2)
			{ 99, 0 },                            // leaf node (V:99, L:19 C:0X7FFD3)
			{ 100, 0 },                           // leaf node (V:100, L:19 C:0X7FFD4)
			{ 101, 0 },                           // leaf node (V:101, L:19 C:0X7FFD5)
			{ 102, 0 },                           // leaf node (V:102, L:19 C:0X7FFD6)
			{ 117, 0 },                           // leaf node (V:117, L:19 C:0X7FFD7)
			{ 97, 0 },                            // leaf node (V:97, L:19 C:0X7FFD8)
			{ 91, 0 },                            // leaf node (V:91, L:19 C:0X7FFD9)
			{ 92, 0 },                            // leaf node (V:92, L:19 C:0X7FFDA)
			{ 93, 0 },                            // leaf node (V:93, L:19 C:0X7FFDB)
			{ 94, 0 },                            // leaf node (V:94, L:19 C:0X7FFDC)
			{ 95, 0 },                            // leaf node (V:95, L:19 C:0X7FFDD)
			{ 96, 0 },                            // leaf node (V:96, L:19 C:0X7FFDE)
			{ 104, 0 },                           // leaf node (V:104, L:19 C:0X7FFDF)
			{ 111, 0 },                           // leaf node (V:111, L:19 C:0X7FFE0)
			{ 112, 0 },                           // leaf node (V:112, L:19 C:0X7FFE1)
			{ 113, 0 },                           // leaf node (V:113, L:19 C:0X7FFE2)
			{ 114, 0 },                           // leaf node (V:114, L:19 C:0X7FFE3)
			{ 115, 0 },                           // leaf node (V:115, L:19 C:0X7FFE4)
			{ 116, 0 },                           // leaf node (V:116, L:19 C:0X7FFE5)
			{ 110, 0 },                           // leaf node (V:110, L:19 C:0X7FFE6)
			{ 105, 0 },                           // leaf node (V:105, L:19 C:0X7FFE7)
			{ 106, 0 },                           // leaf node (V:106, L:19 C:0X7FFE8)
			{ 107, 0 },                           // leaf node (V:107, L:19 C:0X7FFE9)
			{ 108, 0 },                           // leaf node (V:108, L:19 C:0X7FFEA)
			{ 109, 0 },                           // leaf node (V:109, L:19 C:0X7FFEB)
			{ 118, 0 },                           // leaf node (V:118, L:19 C:0X7FFEC)
			{ 6, 0 },                             // leaf node (V:6, L:19 C:0X7FFED)
			{ 8, 0 },                             // leaf node (V:8, L:19 C:0X7FFEE)
			{ 9, 0 },                             // leaf node (V:9, L:19 C:0X7FFEF)
			{ 10, 0 },                            // leaf node (V:10, L:19 C:0X7FFF0)
			{ 5, 0 },                             // leaf node (V:5, L:19 C:0X7FFF1)
			{ 103, 0 },                           // leaf node (V:103, L:19 C:0X7FFF2)
			{ 120, 0 },                           // leaf node (V:120, L:19 C:0X7FFF3)
			{ 119, 0 },                           // leaf node (V:119, L:19 C:0X7FFF4)
			{ 4, 0 },                             // leaf node (V:4, L:19 C:0X7FFF5)
			{ 7, 0 },                             // leaf node (V:7, L:19 C:0X7FFF6)
			{ 15, 0 },                            // leaf node (V:15, L:19 C:0X7FFF7)
			{ 16, 0 },                            // leaf node (V:16, L:19 C:0X7FFF8)
			{ 18, 0 },                            // leaf node (V:18, L:19 C:0X7FFF9)
			{ 20, 0 },                            // leaf node (V:20, L:19 C:0X7FFFA)
			{ 17, 0 },                            // leaf node (V:17, L:19 C:0X7FFFB)
			{ 11, 0 },                            // leaf node (V:11, L:19 C:0X7FFFC)
			{ 12, 0 },                            // leaf node (V:12, L:19 C:0X7FFFD)
			{ 14, 0 },                            // leaf node (V:14, L:19 C:0X7FFFE)
			{ 13, 0 },                            // leaf node (V:13, L:19 C:0X7FFFF)
		};

		const Huffman_Codebook rvlc_hcb = {
			{-7, 7, 65 },
			{-6, 9, 257},
			{-5, 8, 129},
			{-4, 6, 33 },
			{-3, 5, 17 },
			{-2, 4, 9  },
			{-1, 3, 5  },
			{ 0, 1, 0  },
			{ 1, 3, 7  },
			{ 2, 5, 27 },
			{ 3, 6, 51 },
			{ 4, 7, 107},
			{ 5, 8, 195},
			{ 6, 9, 427},
			{ 7, 7, 99 },
		};

		const int rvlc_hcb_bst[37][2] = {
			{ 1, 2 },
			{ 0, 0 },                             // leaf node (V:0, L:1 C:0X0)
			{ 1, 2 },
			{ 2, 3 },
			{ 3, 4 },
			{ 4, 5 },
			{ -1, 0 },                            // leaf node (V:-1, L:3 C:0X5)
			{ 4, 5 },
			{ 1, 0 },                             // leaf node (V:1, L:3 C:0X7)
			{ 4, 5 },
			{ -2, 0 },                            // leaf node (V:-2, L:4 C:0X9)
			{ 4, 5 },
			{ 5, 6 },
			{ 6, 7 },
			{ -3, 0 },                            // leaf node (V:-3, L:5 C:0X11)
			{ 6, 7 },
			{ 7, 8 },
			{ 7, 8 },
			{ 2, 0 },                             // leaf node (V:2, L:5 C:0X1B)
			{ 6, 7 },
			{ -4, 0 },                            // leaf node (V:-4, L:6 C:0X21)
			{ 6, 7 },
			{ 6, 7 },
			{ 3, 0 },                             // leaf node (V:3, L:6 C:0X33)
			{ 5, 6 },
			{ 6, 7 },
			{ -7, 0 },                            // leaf node (V:-7, L:7 C:0X41)
			{ 6, 7 },
			{ 7, 0 },                             // leaf node (V:7, L:7 C:0X63)
			{ 5, 6 },
			{ 4, 0 },                             // leaf node (V:4, L:7 C:0X6B)
			{ 4, 5 },
			{ -5, 0 },                            // leaf node (V:-5, L:8 C:0X81)
			{ 5, 0 },                             // leaf node (V:5, L:8 C:0XC3)
			{ 2, 3 },
			{ -6, 0 },                            // leaf node (V:-6, L:9 C:0X101)
			{ 6, 0 },                             // leaf node (V:6, L:9 C:0X1AB)
		};

		const Huffman_Codebook rvlc_esc_hcb = {
			{ 0,  2, 0x2    }, { 1,  2, 0x0    }, { 2,  3, 0x6    }, { 3,  3, 0x2    }, { 4,  4, 0xe    },
			{ 5,  5, 0x1f   }, { 6,  5, 0xf    }, { 7,  5, 0xd    }, { 8,  6, 0x3d   }, { 9,  6, 0x1d   },
			{10,  6, 0x19   }, {11,  6, 0x18   }, {12,  7, 0x78   }, {13,  7, 0x38   }, {14,  8, 0xf2   },
			{15,  8, 0x72   }, {16,  9, 0x1e6  }, {17,  9, 0xe6   }, {18, 10, 0x3ce  }, {19, 10, 0x1cf  },
			{20, 11, 0x79e  }, {21, 11, 0x79f  }, {22, 11, 0x39d  }, {23, 12, 0x738  }, {24, 14, 0x1ce7 },
			{25, 13, 0xe72  }, {26, 15, 0x39cd }, {27, 20, 0x7398a}, {28, 20, 0x7398b}, {29, 20, 0x7398c},
			{30, 20, 0x7398d}, {31, 20, 0x7398e}, {32, 20, 0x7398f}, {33, 20, 0x73990}, {34, 20, 0x73991},
			{35, 20, 0x73992}, {36, 20, 0x73993}, {37, 20, 0x73994}, {38, 20, 0x73995}, {39, 20, 0x73996},
			{40, 20, 0x73997}, {41, 20, 0x73998}, {42, 20, 0x73999}, {43, 20, 0x7399a}, {44, 20, 0x7399b},
			{45, 20, 0x7399c}, {46, 20, 0x7399d}, {47, 20, 0x7399e}, {48, 20, 0x7399f}, {49, 19, 0x39cc0},
			{50, 19, 0x39cc1}, {51, 19, 0x39cc2}, {52, 19, 0x39cc3}, {53, 19, 0x39cc4},
		};

		const int rvlc_esc_hcb_bst[107][2] = {
			{ 1, 2 },
			{ 2, 3 },
			{ 3, 4 },
			{ 1, 0 },                             // leaf node (V:1, L:2 C:0X0)
			{ 3, 4 },
			{ 0, 0 },                             // leaf node (V:0, L:2 C:0X2)
			{ 3, 4 },
			{ 3, 0 },                             // leaf node (V:3, L:3 C:0X2)
			{ 3, 4 },
			{ 2, 0 },                             // leaf node (V:2, L:3 C:0X6)
			{ 3, 4 },
			{ 4, 5 },
			{ 5, 6 },
			{ 4, 0 },                             // leaf node (V:4, L:4 C:0XE)
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 0 },                             // leaf node (V:7, L:5 C:0XD)
			{ 6, 7 },
			{ 6, 0 },                             // leaf node (V:6, L:5 C:0XF)
			{ 6, 7 },
			{ 5, 0 },                             // leaf node (V:5, L:5 C:0X1F)
			{ 11, 0 },                            // leaf node (V:11, L:6 C:0X18)
			{ 10, 0 },                            // leaf node (V:10, L:6 C:0X19)
			{ 4, 5 },
			{ 9, 0 },                             // leaf node (V:9, L:6 C:0X1D)
			{ 4, 5 },
			{ 8, 0 },                             // leaf node (V:8, L:6 C:0X3D)
			{ 13, 0 },                            // leaf node (V:13, L:7 C:0X38)
			{ 3, 4 },
			{ 12, 0 },                            // leaf node (V:12, L:7 C:0X78)
			{ 3, 4 },
			{ 15, 0 },                            // leaf node (V:15, L:8 C:0X72)
			{ 3, 4 },
			{ 14, 0 },                            // leaf node (V:14, L:8 C:0XF2)
			{ 3, 4 },
			{ 17, 0 },                            // leaf node (V:17, L:9 C:0XE6)
			{ 3, 4 },
			{ 16, 0 },                            // leaf node (V:16, L:9 C:0X1E6)
			{ 3, 4 },
			{ 4, 5 },
			{ 19, 0 },                            // leaf node (V:19, L:10 C:0X1CF)
			{ 18, 0 },                            // leaf node (V:18, L:10 C:0X3CE)
			{ 3, 4 },
			{ 4, 5 },
			{ 22, 0 },                            // leaf node (V:22, L:11 C:0X39D)
			{ 20, 0 },                            // leaf node (V:20, L:11 C:0X79E)
			{ 21, 0 },                            // leaf node (V:21, L:11 C:0X79F)
			{ 23, 0 },                            // leaf node (V:23, L:12 C:0X738)
			{ 1, 2 },
			{ 25, 0 },                            // leaf node (V:25, L:13 C:0XE72)
			{ 1, 2 },
			{ 2, 3 },
			{ 24, 0 },                            // leaf node (V:24, L:14 C:0X1CE7)
			{ 2, 3 },
			{ 26, 0 },                            // leaf node (V:26, L:15 C:0X39CD)
			{ 2, 3 },
			{ 3, 4 },
			{ 4, 5 },
			{ 5, 6 },
			{ 6, 7 },
			{ 7, 8 },
			{ 8, 9 },
			{ 9, 10 },
			{ 10, 11 },
			{ 11, 12 },
			{ 12, 13 },
			{ 13, 14 },
			{ 14, 15 },
			{ 15, 16 },
			{ 49, 0 },                            // leaf node (V:49, L:19 C:0X39CC0)
			{ 50, 0 },                            // leaf node (V:50, L:19 C:0X39CC1)
			{ 51, 0 },                            // leaf node (V:51, L:19 C:0X39CC2)
			{ 52, 0 },                            // leaf node (V:52, L:19 C:0X39CC3)
			{ 53, 0 },                            // leaf node (V:53, L:19 C:0X39CC4)
			{ 11, 12 },
			{ 12, 13 },
			{ 13, 14 },
			{ 14, 15 },
			{ 15, 16 },
			{ 16, 17 },
			{ 17, 18 },
			{ 18, 19 },
			{ 19, 20 },
			{ 20, 21 },
			{ 21, 22 },
			{ 27, 0 },                            // leaf node (V:27, L:20 C:0X7398A)
			{ 28, 0 },                            // leaf node (V:28, L:20 C:0X7398B)
			{ 29, 0 },                            // leaf node (V:29, L:20 C:0X7398C)
			{ 30, 0 },                            // leaf node (V:30, L:20 C:0X7398D)
			{ 31, 0 },                            // leaf node (V:31, L:20 C:0X7398E)
			{ 32, 0 },                            // leaf node (V:32, L:20 C:0X7398F)
			{ 33, 0 },                            // leaf node (V:33, L:20 C:0X73990)
			{ 34, 0 },                            // leaf node (V:34, L:20 C:0X73991)
			{ 35, 0 },                            // leaf node (V:35, L:20 C:0X73992)
			{ 36, 0 },                            // leaf node (V:36, L:20 C:0X73993)
			{ 37, 0 },                            // leaf node (V:37, L:20 C:0X73994)
			{ 38, 0 },                            // leaf node (V:38, L:20 C:0X73995)
			{ 39, 0 },                            // leaf node (V:39, L:20 C:0X73996)
			{ 40, 0 },                            // leaf node (V:40, L:20 C:0X73997)
			{ 41, 0 },                            // leaf node (V:41, L:20 C:0X73998)
			{ 42, 0 },                            // leaf node (V:42, L:20 C:0X73999)
			{ 43, 0 },                            // leaf node (V:43, L:20 C:0X7399A)
			{ 44, 0 },                            // leaf node (V:44, L:20 C:0X7399B)
			{ 45, 0 },                            // leaf node (V:45, L:20 C:0X7399C)
			{ 46, 0 },                            // leaf node (V:46, L:20 C:0X7399D)
			{ 47, 0 },                            // leaf node (V:47, L:20 C:0X7399E)
			{ 48, 0 },                            // leaf node (V:48, L:20 C:0X7399F)
		};

		const Spectrum_Huffman_Codebook_Quad shcb1 = {
			{ { 0, -1, -1, -1, -1}, 11, 0x7f8}, { { 1, -1, -1, -1,  0},  9, 0x1f1},
			{ { 2, -1, -1, -1,  1}, 11, 0x7fd}, { { 3, -1, -1,  0, -1}, 10, 0x3f5},
			{ { 4, -1, -1,  0,  0},  7, 0x68 }, { { 5, -1, -1,  0,  1}, 10, 0x3f0},
			{ { 6, -1, -1,  1, -1}, 11, 0x7f7}, { { 7, -1, -1,  1,  0},  9, 0x1ec},
			{ { 8, -1, -1,  1,  1}, 11, 0x7f5}, { { 9, -1,  0, -1, -1}, 10, 0x3f1},
			{ {10, -1,  0, -1,  0},  7, 0x72 }, { {11, -1,  0, -1,  1}, 10, 0x3f4},
			{ {12, -1,  0,  0, -1},  7, 0x74 }, { {13, -1,  0,  0,  0},  5, 0x11 },
			{ {14, -1,  0,  0,  1},  7, 0x76 }, { {15, -1,  0,  1, -1},  9, 0x1eb},
			{ {16, -1,  0,  1,  0},  7, 0x6c }, { {17, -1,  0,  1,  1}, 10, 0x3f6},
			{ {18, -1,  1, -1, -1}, 11, 0x7fc}, { {19, -1,  1, -1,  0},  9, 0x1e1},
			{ {20, -1,  1, -1,  1}, 11, 0x7f1}, { {21, -1,  1,  0, -1},  9, 0x1f0},
			{ {22, -1,  1,  0,  0},  7, 0x61 }, { {23, -1,  1,  0,  1},  9, 0x1f6},
			{ {24, -1,  1,  1, -1}, 11, 0x7f2}, { {25, -1,  1,  1,  0},  9, 0x1ea},
			{ {26, -1,  1,  1,  1}, 11, 0x7fb}, { {27,  0, -1, -1, -1},  9, 0x1f2},
			{ {28,  0, -1, -1,  0},  7, 0x69 }, { {29,  0, -1, -1,  1},  9, 0x1ed},
			{ {30,  0, -1,  0, -1},  7, 0x77 }, { {31,  0, -1,  0,  0},  5, 0x17 },
			{ {32,  0, -1,  0,  1},  7, 0x6f }, { {33,  0, -1,  1, -1},  9, 0x1e6},
			{ {34,  0, -1,  1,  0},  7, 0x64 }, { {35,  0, -1,  1,  1},  9, 0x1e5},
			{ {36,  0,  0, -1, -1},  7, 0x67 }, { {37,  0,  0, -1,  0},  5, 0x15 },
			{ {38,  0,  0, -1,  1},  7, 0x62 }, { {39,  0,  0,  0, -1},  5, 0x12 },
			{ {40,  0,  0,  0,  0},  1, 0x0  }, { {41,  0,  0,  0,  1},  5, 0x14 },
			{ {42,  0,  0,  1, -1},  7, 0x65 }, { {43,  0,  0,  1,  0},  5, 0x16 },
			{ {44,  0,  0,  1,  1},  7, 0x6d }, { {45,  0,  1, -1, -1},  9, 0x1e9},
			{ {46,  0,  1, -1,  0},  7, 0x63 }, { {47,  0,  1, -1,  1},  9, 0x1e4},
			{ {48,  0,  1,  0, -1},  7, 0x6b }, { {49,  0,  1,  0,  0},  5, 0x13 },
			{ {50,  0,  1,  0,  1},  7, 0x71 }, { {51,  0,  1,  1, -1},  9, 0x1e3},
			{ {52,  0,  1,  1,  0},  7, 0x70 }, { {53,  0,  1,  1,  1},  9, 0x1f3},
			{ {54,  1, -1, -1, -1}, 11, 0x7fe}, { {55,  1, -1, -1,  0},  9, 0x1e7},
			{ {56,  1, -1, -1,  1}, 11, 0x7f3}, { {57,  1, -1,  0, -1},  9, 0x1ef},
			{ {58,  1, -1,  0,  0},  7, 0x60 }, { {59,  1, -1,  0,  1},  9, 0x1ee},
			{ {60,  1, -1,  1, -1}, 11, 0x7f0}, { {61,  1, -1,  1,  0},  9, 0x1e2},
			{ {62,  1, -1,  1,  1}, 11, 0x7fa}, { {63,  1,  0, -1, -1}, 10, 0x3f3},
			{ {64,  1,  0, -1,  0},  7, 0x6a }, { {65,  1,  0, -1,  1},  9, 0x1e8},
			{ {66,  1,  0,  0, -1},  7, 0x75 }, { {67,  1,  0,  0,  0},  5, 0x10 },
			{ {68,  1,  0,  0,  1},  7, 0x73 }, { {69,  1,  0,  1, -1},  9, 0x1f4},
			{ {70,  1,  0,  1,  0},  7, 0x6e }, { {71,  1,  0,  1,  1}, 10, 0x3f7},
			{ {72,  1,  1, -1, -1}, 11, 0x7f6}, { {73,  1,  1, -1,  0},  9, 0x1e0},
			{ {74,  1,  1, -1,  1}, 11, 0x7f9}, { {75,  1,  1,  0, -1}, 10, 0x3f2},
			{ {76,  1,  1,  0,  0},  7, 0x66 }, { {77,  1,  1,  0,  1},  9, 0x1f5},
			{ {78,  1,  1,  1, -1}, 11, 0x7ff}, { {79,  1,  1,  1,  0},  9, 0x1f7},
			{ {80,  1,  1,  1,  1}, 11, 0x7f4},
		};

		const int shcb1_bst[161][2] = {
			{1, 2},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:1 C:0X0)
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{67, 0},                            // leaf node (V-index:67(V: 67) L:5 C:0X10)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:5 C:0X11)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:5 C:0X12)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:5 C:0X13)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:5 C:0X14)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:5 C:0X15)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:5 C:0X16)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:5 C:0X17)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{58, 0},                            // leaf node (V-index:58(V: 58) L:7 C:0X60)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:7 C:0X61)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:7 C:0X62)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:7 C:0X63)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:7 C:0X64)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:7 C:0X65)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:7 C:0X66)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:7 C:0X67)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:7 C:0X68)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:7 C:0X69)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:7 C:0X6A)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:7 C:0X6B)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:7 C:0X6C)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:7 C:0X6D)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:7 C:0X6E)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:7 C:0X6F)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:7 C:0X70)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:7 C:0X71)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:7 C:0X72)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:7 C:0X73)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:7 C:0X74)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:7 C:0X75)
			{14, 0},                            // leaf node (V-index:14(V: 14) L:7 C:0X76)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:7 C:0X77)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{73, 0},                            // leaf node (V-index:73(V: 73) L:9 C:0X1E0)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:9 C:0X1E1)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:9 C:0X1E2)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:9 C:0X1E3)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:9 C:0X1E4)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:9 C:0X1E5)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:9 C:0X1E6)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:9 C:0X1E7)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:9 C:0X1E8)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:9 C:0X1E9)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:9 C:0X1EA)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:9 C:0X1EB)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:9 C:0X1EC)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:9 C:0X1ED)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:9 C:0X1EE)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:9 C:0X1EF)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:9 C:0X1F0)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:9 C:0X1F1)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:9 C:0X1F2)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:9 C:0X1F3)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:9 C:0X1F4)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:9 C:0X1F5)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:9 C:0X1F6)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:9 C:0X1F7)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{5, 0},                             // leaf node (V-index:5(V: 5) L:10 C:0X3F0)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:10 C:0X3F1)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:10 C:0X3F2)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:10 C:0X3F3)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:10 C:0X3F4)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:10 C:0X3F5)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:10 C:0X3F6)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:10 C:0X3F7)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{60, 0},                            // leaf node (V-index:60(V: 60) L:11 C:0X7F0)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:11 C:0X7F1)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:11 C:0X7F2)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:11 C:0X7F3)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:11 C:0X7F4)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:11 C:0X7F5)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:11 C:0X7F6)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:11 C:0X7F7)
			{0, 0},                             // leaf node (V-index:0(V: 0) L:11 C:0X7F8)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:11 C:0X7F9)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:11 C:0X7FA)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:11 C:0X7FB)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:11 C:0X7FC)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:11 C:0X7FD)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:11 C:0X7FE)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:11 C:0X7FF)
		};

		const Spectrum_Huffman_Codebook_Quad shcb2 = {
			{ { 0, -1, -1, -1, -1}, 9, 0x1f3}, { { 1, -1, -1, -1,  0}, 7, 0x6f },
			{ { 2, -1, -1, -1,  1}, 9, 0x1fd}, { { 3, -1, -1,  0, -1}, 8, 0xeb },
			{ { 4, -1, -1,  0,  0}, 6, 0x23 }, { { 5, -1, -1,  0,  1}, 8, 0xea },
			{ { 6, -1, -1,  1, -1}, 9, 0x1f7}, { { 7, -1, -1,  1,  0}, 8, 0xe8 },
			{ { 8, -1, -1,  1,  1}, 9, 0x1fa}, { { 9, -1,  0, -1, -1}, 8, 0xf2 },
			{ {10, -1,  0, -1,  0}, 6, 0x2d }, { {11, -1,  0, -1,  1}, 7, 0x70 },
			{ {12, -1,  0,  0, -1}, 6, 0x20 }, { {13, -1,  0,  0,  0}, 5, 0x6  },
			{ {14, -1,  0,  0,  1}, 6, 0x2b }, { {15, -1,  0,  1, -1}, 7, 0x6e },
			{ {16, -1,  0,  1,  0}, 6, 0x28 }, { {17, -1,  0,  1,  1}, 8, 0xe9 },
			{ {18, -1,  1, -1, -1}, 9, 0x1f9}, { {19, -1,  1, -1,  0}, 7, 0x66 },
			{ {20, -1,  1, -1,  1}, 8, 0xf8 }, { {21, -1,  1,  0, -1}, 8, 0xe7 },
			{ {22, -1,  1,  0,  0}, 6, 0x1b }, { {23, -1,  1,  0,  1}, 8, 0xf1 },
			{ {24, -1,  1,  1, -1}, 9, 0x1f4}, { {25, -1,  1,  1,  0}, 7, 0x6b },
			{ {26, -1,  1,  1,  1}, 9, 0x1f5}, { {27,  0, -1, -1, -1}, 8, 0xec },
			{ {28,  0, -1, -1,  0}, 6, 0x2a }, { {29,  0, -1, -1,  1}, 7, 0x6c },
			{ {30,  0, -1,  0, -1}, 6, 0x2c }, { {31,  0, -1,  0,  0}, 5, 0xa  },
			{ {32,  0, -1,  0,  1}, 6, 0x27 }, { {33,  0, -1,  1, -1}, 7, 0x67 },
			{ {34,  0, -1,  1,  0}, 6, 0x1a }, { {35,  0, -1,  1,  1}, 8, 0xf5 },
			{ {36,  0,  0, -1, -1}, 6, 0x24 }, { {37,  0,  0, -1,  0}, 5, 0x8  },
			{ {38,  0,  0, -1,  1}, 6, 0x1f }, { {39,  0,  0,  0, -1}, 5, 0x9  },
			{ {40,  0,  0,  0,  0}, 3, 0x0  }, { {41,  0,  0,  0,  1}, 5, 0x7  },
			{ {42,  0,  0,  1, -1}, 6, 0x1d }, { {43,  0,  0,  1,  0}, 5, 0xb  },
			{ {44,  0,  0,  1,  1}, 6, 0x30 }, { {45,  0,  1, -1, -1}, 8, 0xef },
			{ {46,  0,  1, -1,  0}, 6, 0x1c }, { {47,  0,  1, -1,  1}, 7, 0x64 },
			{ {48,  0,  1,  0, -1}, 6, 0x1e }, { {49,  0,  1,  0,  0}, 5, 0xc  },
			{ {50,  0,  1,  0,  1}, 6, 0x29 }, { {51,  0,  1,  1, -1}, 8, 0xf3 },
			{ {52,  0,  1,  1,  0}, 6, 0x2f }, { {53,  0,  1,  1,  1}, 8, 0xf0 },
			{ {54,  1, -1, -1, -1}, 9, 0x1fc}, { {55,  1, -1, -1,  0}, 7, 0x71 },
			{ {56,  1, -1, -1,  1}, 9, 0x1f2}, { {57,  1, -1,  0, -1}, 8, 0xf4 },
			{ {58,  1, -1,  0,  0}, 6, 0x21 }, { {59,  1, -1,  0,  1}, 8, 0xe6 },
			{ {60,  1, -1,  1, -1}, 8, 0xf7 }, { {61,  1, -1,  1,  0}, 7, 0x68 },
			{ {62,  1, -1,  1,  1}, 9, 0x1f8}, { {63,  1,  0, -1, -1}, 8, 0xee },
			{ {64,  1,  0, -1,  0}, 6, 0x22 }, { {65,  1,  0, -1,  1}, 7, 0x65 },
			{ {66,  1,  0,  0, -1}, 6, 0x31 }, { {67,  1,  0,  0,  0}, 4, 0x2  },
			{ {68,  1,  0,  0,  1}, 6, 0x26 }, { {69,  1,  0,  1, -1}, 8, 0xed },
			{ {70,  1,  0,  1,  0}, 6, 0x25 }, { {71,  1,  0,  1,  1}, 7, 0x6a },
			{ {72,  1,  1, -1, -1}, 9, 0x1fb}, { {73,  1,  1, -1,  0}, 7, 0x72 },
			{ {74,  1,  1, -1,  1}, 9, 0x1fe}, { {75,  1,  1,  0, -1}, 7, 0x69 },
			{ {76,  1,  1,  0,  0}, 6, 0x2e }, { {77,  1,  1,  0,  1}, 8, 0xf6 },
			{ {78,  1,  1,  1, -1}, 9, 0x1ff}, { {79,  1,  1,  1,  0}, 7, 0x6d },
			{ {80,  1,  1,  1,  1}, 9, 0x1f6},
		};

		const int shcb2_bst[161][2] = {
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:3 C:0X0)
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{67, 0},                            // leaf node (V-index:67(V: 67) L:4 C:0X2)
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{13, 0},                            // leaf node (V-index:13(V: 13) L:5 C:0X6)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:5 C:0X7)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:5 C:0X8)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:5 C:0X9)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:5 C:0XA)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:5 C:0XB)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:5 C:0XC)
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{34, 0},                            // leaf node (V-index:34(V: 34) L:6 C:0X1A)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:6 C:0X1B)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:6 C:0X1C)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:6 C:0X1D)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:6 C:0X1E)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:6 C:0X1F)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:6 C:0X20)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:6 C:0X21)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:6 C:0X22)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:6 C:0X23)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:6 C:0X24)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:6 C:0X25)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:6 C:0X26)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:6 C:0X27)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:6 C:0X28)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:6 C:0X29)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:6 C:0X2A)
			{14, 0},                            // leaf node (V-index:14(V: 14) L:6 C:0X2B)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:6 C:0X2C)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:6 C:0X2D)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:6 C:0X2E)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:6 C:0X2F)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:6 C:0X30)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:6 C:0X31)
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{47, 0},                            // leaf node (V-index:47(V: 47) L:7 C:0X64)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:7 C:0X65)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:7 C:0X66)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:7 C:0X67)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:7 C:0X68)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:7 C:0X69)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:7 C:0X6A)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:7 C:0X6B)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:7 C:0X6C)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:7 C:0X6D)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:7 C:0X6E)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:7 C:0X6F)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:7 C:0X70)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:7 C:0X71)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:7 C:0X72)
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{59, 0},                            // leaf node (V-index:59(V: 59) L:8 C:0XE6)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:8 C:0XE7)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:8 C:0XE8)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:8 C:0XE9)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:8 C:0XEA)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:8 C:0XEB)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:8 C:0XEC)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:8 C:0XED)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:8 C:0XEE)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:8 C:0XEF)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:8 C:0XF0)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:8 C:0XF1)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:8 C:0XF2)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:8 C:0XF3)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:8 C:0XF4)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:8 C:0XF5)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:8 C:0XF6)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:8 C:0XF7)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:8 C:0XF8)
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{56, 0},                            // leaf node (V-index:56(V: 56) L:9 C:0X1F2)
			{0, 0},                             // leaf node (V-index:0(V: 0) L:9 C:0X1F3)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:9 C:0X1F4)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:9 C:0X1F5)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:9 C:0X1F6)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1F7)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:9 C:0X1F8)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:9 C:0X1F9)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:9 C:0X1FA)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:9 C:0X1FB)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:9 C:0X1FC)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:9 C:0X1FD)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:9 C:0X1FE)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:9 C:0X1FF)
		};

		const Spectrum_Huffman_Codebook_Quad shcb3 = {
			{ { 0, 0, 0, 0, 0},  1, 0x0   }, { { 1, 0, 0, 0, 1},  4, 0x9   },
			{ { 2, 0, 0, 0, 2},  8, 0xef  }, { { 3, 0, 0, 1, 0},  4, 0xb   },
			{ { 4, 0, 0, 1, 1},  5, 0x19  }, { { 5, 0, 0, 1, 2},  8, 0xf0  },
			{ { 6, 0, 0, 2, 0},  9, 0x1eb }, { { 7, 0, 0, 2, 1},  9, 0x1e6 },
			{ { 8, 0, 0, 2, 2}, 10, 0x3f2 }, { { 9, 0, 1, 0, 0},  4, 0xa   },
			{ {10, 0, 1, 0, 1},  6, 0x35  }, { {11, 0, 1, 0, 2},  9, 0x1ef },
			{ {12, 0, 1, 1, 0},  6, 0x34  }, { {13, 0, 1, 1, 1},  6, 0x37  },
			{ {14, 0, 1, 1, 2},  9, 0x1e9 }, { {15, 0, 1, 2, 0},  9, 0x1ed },
			{ {16, 0, 1, 2, 1},  9, 0x1e7 }, { {17, 0, 1, 2, 2}, 10, 0x3f3 },
			{ {18, 0, 2, 0, 0},  9, 0x1ee }, { {19, 0, 2, 0, 1}, 10, 0x3ed },
			{ {20, 0, 2, 0, 2}, 13, 0x1ffa}, { {21, 0, 2, 1, 0},  9, 0x1ec },
			{ {22, 0, 2, 1, 1},  9, 0x1f2 }, { {23, 0, 2, 1, 2}, 11, 0x7f9 },
			{ {24, 0, 2, 2, 0}, 11, 0x7f8 }, { {25, 0, 2, 2, 1}, 10, 0x3f8 },
			{ {26, 0, 2, 2, 2}, 12, 0xff8 }, { {27, 1, 0, 0, 0},  4, 0x8   },
			{ {28, 1, 0, 0, 1},  6, 0x38  }, { {29, 1, 0, 0, 2}, 10, 0x3f6 },
			{ {30, 1, 0, 1, 0},  6, 0x36  }, { {31, 1, 0, 1, 1},  7, 0x75  },
			{ {32, 1, 0, 1, 2}, 10, 0x3f1 }, { {33, 1, 0, 2, 0}, 10, 0x3eb },
			{ {34, 1, 0, 2, 1}, 10, 0x3ec }, { {35, 1, 0, 2, 2}, 12, 0xff4 },
			{ {36, 1, 1, 0, 0},  5, 0x18  }, { {37, 1, 1, 0, 1},  7, 0x76  },
			{ {38, 1, 1, 0, 2}, 11, 0x7f4 }, { {39, 1, 1, 1, 0},  6, 0x39  },
			{ {40, 1, 1, 1, 1},  7, 0x74  }, { {41, 1, 1, 1, 2}, 10, 0x3ef },
			{ {42, 1, 1, 2, 0},  9, 0x1f3 }, { {43, 1, 1, 2, 1},  9, 0x1f4 },
			{ {44, 1, 1, 2, 2}, 11, 0x7f6 }, { {45, 1, 2, 0, 0},  9, 0x1e8 },
			{ {46, 1, 2, 0, 1}, 10, 0x3ea }, { {47, 1, 2, 0, 2}, 13, 0x1ffc},
			{ {48, 1, 2, 1, 0},  8, 0xf2  }, { {49, 1, 2, 1, 1},  9, 0x1f1 },
			{ {50, 1, 2, 1, 2}, 12, 0xffb }, { {51, 1, 2, 2, 0}, 10, 0x3f5 },
			{ {52, 1, 2, 2, 1}, 11, 0x7f3 }, { {53, 1, 2, 2, 2}, 12, 0xffc },
			{ {54, 2, 0, 0, 0},  8, 0xee  }, { {55, 2, 0, 0, 1}, 10, 0x3f7 },
			{ {56, 2, 0, 0, 2}, 15, 0x7ffe}, { {57, 2, 0, 1, 0},  9, 0x1f0 },
			{ {58, 2, 0, 1, 1}, 11, 0x7f5 }, { {59, 2, 0, 1, 2}, 15, 0x7ffd},
			{ {60, 2, 0, 2, 0}, 13, 0x1ffb}, { {61, 2, 0, 2, 1}, 14, 0x3ffa},
			{ {62, 2, 0, 2, 2}, 16, 0xffff}, { {63, 2, 1, 0, 0},  8, 0xf1  },
			{ {64, 2, 1, 0, 1}, 10, 0x3f0 }, { {65, 2, 1, 0, 2}, 14, 0x3ffc},
			{ {66, 2, 1, 1, 0},  9, 0x1ea }, { {67, 2, 1, 1, 1}, 10, 0x3ee },
			{ {68, 2, 1, 1, 2}, 14, 0x3ffb}, { {69, 2, 1, 2, 0}, 12, 0xff6 },
			{ {70, 2, 1, 2, 1}, 12, 0xffa }, { {71, 2, 1, 2, 2}, 15, 0x7ffc},
			{ {72, 2, 2, 0, 0}, 11, 0x7f2 }, { {73, 2, 2, 0, 1}, 12, 0xff5 },
			{ {74, 2, 2, 0, 2}, 16, 0xfffe}, { {75, 2, 2, 1, 0}, 10, 0x3f4 },
			{ {76, 2, 2, 1, 1}, 11, 0x7f7 }, { {77, 2, 2, 1, 2}, 15, 0x7ffb},
			{ {78, 2, 2, 2, 0}, 12, 0xff7 }, { {79, 2, 2, 2, 1}, 12, 0xff9 },
			{ {80, 2, 2, 2, 2}, 15, 0x7ffa},
		};

		const int shcb3_bst[161][2] = {
			{1, 2},
			{0, 0},                             // leaf node (V-index:0(V: 0) L:1 C:0X0)
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{27, 0},                            // leaf node (V-index:27(V: 27) L:4 C:0X8)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:4 C:0X9)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:4 C:0XA)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:4 C:0XB)
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{36, 0},                            // leaf node (V-index:36(V: 36) L:5 C:0X18)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:5 C:0X19)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 0},                            // leaf node (V-index:12(V: 12) L:6 C:0X34)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:6 C:0X35)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:6 C:0X36)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:6 C:0X37)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:6 C:0X38)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:6 C:0X39)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:7 C:0X74)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:7 C:0X75)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:7 C:0X76)
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{54, 0},                            // leaf node (V-index:54(V: 54) L:8 C:0XEE)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:8 C:0XEF)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:8 C:0XF0)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:8 C:0XF1)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:8 C:0XF2)
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{7, 0},                             // leaf node (V-index:7(V: 7) L:9 C:0X1E6)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:9 C:0X1E7)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:9 C:0X1E8)
			{14, 0},                            // leaf node (V-index:14(V: 14) L:9 C:0X1E9)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:9 C:0X1EA)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1EB)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:9 C:0X1EC)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:9 C:0X1ED)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:9 C:0X1EE)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:9 C:0X1EF)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:9 C:0X1F0)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:9 C:0X1F1)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:9 C:0X1F2)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:9 C:0X1F3)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:9 C:0X1F4)
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{46, 0},                            // leaf node (V-index:46(V: 46) L:10 C:0X3EA)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:10 C:0X3EB)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:10 C:0X3EC)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:10 C:0X3ED)
			{67, 0},                            // leaf node (V-index:67(V: 67) L:10 C:0X3EE)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:10 C:0X3EF)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:10 C:0X3F0)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:10 C:0X3F1)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:10 C:0X3F2)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:10 C:0X3F3)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:10 C:0X3F4)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:10 C:0X3F5)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:10 C:0X3F6)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:10 C:0X3F7)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:10 C:0X3F8)
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{72, 0},                            // leaf node (V-index:72(V: 72) L:11 C:0X7F2)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:11 C:0X7F3)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:11 C:0X7F4)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:11 C:0X7F5)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:11 C:0X7F6)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:11 C:0X7F7)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:11 C:0X7F8)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:11 C:0X7F9)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{35, 0},                            // leaf node (V-index:35(V: 35) L:12 C:0XFF4)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:12 C:0XFF5)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:12 C:0XFF6)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:12 C:0XFF7)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:12 C:0XFF8)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:12 C:0XFF9)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:12 C:0XFFA)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:12 C:0XFFB)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:12 C:0XFFC)
			{3, 4},
			{4, 5},
			{5, 6},
			{20, 0},                            // leaf node (V-index:20(V: 20) L:13 C:0X1FFA)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:13 C:0X1FFB)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:13 C:0X1FFC)
			{3, 4},
			{4, 5},
			{5, 6},
			{61, 0},                            // leaf node (V-index:61(V: 61) L:14 C:0X3FFA)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:14 C:0X3FFB)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:14 C:0X3FFC)
			{3, 4},
			{4, 5},
			{5, 6},
			{80, 0},                            // leaf node (V-index:80(V: 80) L:15 C:0X7FFA)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:15 C:0X7FFB)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:15 C:0X7FFC)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:15 C:0X7FFD)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:15 C:0X7FFE)
			{1, 2},
			{74, 0},                            // leaf node (V-index:74(V: 74) L:16 C:0XFFFE)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:16 C:0XFFFF)
		};

		const Spectrum_Huffman_Codebook_Quad shcb4 = {
			{ { 0, 0, 0, 0, 0},  4, 0x7  }, { { 1, 0, 0, 0, 1},  5, 0x16 },
			{ { 2, 0, 0, 0, 2},  8, 0xf6 }, { { 3, 0, 0, 1, 0},  5, 0x18 },
			{ { 4, 0, 0, 1, 1},  4, 0x8  }, { { 5, 0, 0, 1, 2},  8, 0xef },
			{ { 6, 0, 0, 2, 0},  9, 0x1ef}, { { 7, 0, 0, 2, 1},  8, 0xf3 },
			{ { 8, 0, 0, 2, 2}, 11, 0x7f8}, { { 9, 0, 1, 0, 0},  5, 0x19 },
			{ {10, 0, 1, 0, 1},  5, 0x17 }, { {11, 0, 1, 0, 2},  8, 0xed },
			{ {12, 0, 1, 1, 0},  5, 0x15 }, { {13, 0, 1, 1, 1},  4, 0x1  },
			{ {14, 0, 1, 1, 2},  8, 0xe2 }, { {15, 0, 1, 2, 0},  8, 0xf0 },
			{ {16, 0, 1, 2, 1},  7, 0x70 }, { {17, 0, 1, 2, 2}, 10, 0x3f0},
			{ {18, 0, 2, 0, 0},  9, 0x1ee}, { {19, 0, 2, 0, 1},  8, 0xf1 },
			{ {20, 0, 2, 0, 2}, 11, 0x7fa}, { {21, 0, 2, 1, 0},  8, 0xee },
			{ {22, 0, 2, 1, 1},  8, 0xe4 }, { {23, 0, 2, 1, 2}, 10, 0x3f2},
			{ {24, 0, 2, 2, 0}, 11, 0x7f6}, { {25, 0, 2, 2, 1}, 10, 0x3ef},
			{ {26, 0, 2, 2, 2}, 11, 0x7fd}, { {27, 1, 0, 0, 0},  4, 0x5  },
			{ {28, 1, 0, 0, 1},  5, 0x14 }, { {29, 1, 0, 0, 2},  8, 0xf2 },
			{ {30, 1, 0, 1, 0},  4, 0x9  }, { {31, 1, 0, 1, 1},  4, 0x4  },
			{ {32, 1, 0, 1, 2},  8, 0xe5 }, { {33, 1, 0, 2, 0},  8, 0xf4 },
			{ {34, 1, 0, 2, 1},  8, 0xe8 }, { {35, 1, 0, 2, 2}, 10, 0x3f4},
			{ {36, 1, 1, 0, 0},  4, 0x6  }, { {37, 1, 1, 0, 1},  4, 0x2  },
			{ {38, 1, 1, 0, 2},  8, 0xe7 }, { {39, 1, 1, 1, 0},  4, 0x3  },
			{ {40, 1, 1, 1, 1},  4, 0x0  }, { {41, 1, 1, 1, 2},  7, 0x6b },
			{ {42, 1, 1, 2, 0},  8, 0xe3 }, { {43, 1, 1, 2, 1},  7, 0x69 },
			{ {44, 1, 1, 2, 2},  9, 0x1f3}, { {45, 1, 2, 0, 0},  8, 0xeb },
			{ {46, 1, 2, 0, 1},  8, 0xe6 }, { {47, 1, 2, 0, 2}, 10, 0x3f6},
			{ {48, 1, 2, 1, 0},  7, 0x6e }, { {49, 1, 2, 1, 1},  7, 0x6a },
			{ {50, 1, 2, 1, 2},  9, 0x1f4}, { {51, 1, 2, 2, 0}, 10, 0x3ec},
			{ {52, 1, 2, 2, 1},  9, 0x1f0}, { {53, 1, 2, 2, 2}, 10, 0x3f9},
			{ {54, 2, 0, 0, 0},  8, 0xf5 }, { {55, 2, 0, 0, 1},  8, 0xec },
			{ {56, 2, 0, 0, 2}, 11, 0x7fb}, { {57, 2, 0, 1, 0},  8, 0xea },
			{ {58, 2, 0, 1, 1},  7, 0x6f }, { {59, 2, 0, 1, 2}, 10, 0x3f7},
			{ {60, 2, 0, 2, 0}, 11, 0x7f9}, { {61, 2, 0, 2, 1}, 10, 0x3f3},
			{ {62, 2, 0, 2, 2}, 12, 0xfff}, { {63, 2, 1, 0, 0},  8, 0xe9 },
			{ {64, 2, 1, 0, 1},  7, 0x6d }, { {65, 2, 1, 0, 2}, 10, 0x3f8},
			{ {66, 2, 1, 1, 0},  7, 0x6c }, { {67, 2, 1, 1, 1},  7, 0x68 },
			{ {68, 2, 1, 1, 2},  9, 0x1f5}, { {69, 2, 1, 2, 0}, 10, 0x3ee},
			{ {70, 2, 1, 2, 1},  9, 0x1f2}, { {71, 2, 1, 2, 2}, 11, 0x7f4},
			{ {72, 2, 2, 0, 0}, 11, 0x7f7}, { {73, 2, 2, 0, 1}, 10, 0x3f1},
			{ {74, 2, 2, 0, 2}, 12, 0xffe}, { {75, 2, 2, 1, 0}, 10, 0x3ed},
			{ {76, 2, 2, 1, 1},  9, 0x1f1}, { {77, 2, 2, 1, 2}, 11, 0x7f5},
			{ {78, 2, 2, 2, 0}, 11, 0x7fe}, { {79, 2, 2, 2, 1}, 10, 0x3f5},
			{ {80, 2, 2, 2, 2}, 11, 0x7fc},
		};

		const int shcb4_bst[161][2] = {
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:4 C:0X0)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:4 C:0X1)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:4 C:0X2)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:4 C:0X3)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:4 C:0X4)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:4 C:0X5)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:4 C:0X6)
			{0, 0},                             // leaf node (V-index:0(V: 0) L:4 C:0X7)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:4 C:0X8)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:4 C:0X9)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{28, 0},                            // leaf node (V-index:28(V: 28) L:5 C:0X14)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:5 C:0X15)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:5 C:0X16)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:5 C:0X17)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:5 C:0X18)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:5 C:0X19)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{67, 0},                            // leaf node (V-index:67(V: 67) L:7 C:0X68)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:7 C:0X69)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:7 C:0X6A)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:7 C:0X6B)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:7 C:0X6C)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:7 C:0X6D)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:7 C:0X6E)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:7 C:0X6F)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:7 C:0X70)
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{14, 0},                            // leaf node (V-index:14(V: 14) L:8 C:0XE2)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:8 C:0XE3)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:8 C:0XE4)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:8 C:0XE5)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:8 C:0XE6)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:8 C:0XE7)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:8 C:0XE8)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:8 C:0XE9)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:8 C:0XEA)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:8 C:0XEB)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:8 C:0XEC)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:8 C:0XED)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:8 C:0XEE)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:8 C:0XEF)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:8 C:0XF0)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:8 C:0XF1)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:8 C:0XF2)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:8 C:0XF3)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:8 C:0XF4)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:8 C:0XF5)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:8 C:0XF6)
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 0},                            // leaf node (V-index:18(V: 18) L:9 C:0X1EE)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1EF)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:9 C:0X1F0)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:9 C:0X1F1)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:9 C:0X1F2)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:9 C:0X1F3)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:9 C:0X1F4)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:9 C:0X1F5)
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{51, 0},                            // leaf node (V-index:51(V: 51) L:10 C:0X3EC)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:10 C:0X3ED)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:10 C:0X3EE)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:10 C:0X3EF)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:10 C:0X3F0)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:10 C:0X3F1)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:10 C:0X3F2)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:10 C:0X3F3)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:10 C:0X3F4)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:10 C:0X3F5)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:10 C:0X3F6)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:10 C:0X3F7)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:10 C:0X3F8)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:10 C:0X3F9)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{71, 0},                            // leaf node (V-index:71(V: 71) L:11 C:0X7F4)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:11 C:0X7F5)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:11 C:0X7F6)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:11 C:0X7F7)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:11 C:0X7F8)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:11 C:0X7F9)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:11 C:0X7FA)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:11 C:0X7FB)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:11 C:0X7FC)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:11 C:0X7FD)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:11 C:0X7FE)
			{1, 2},
			{74, 0},                            // leaf node (V-index:74(V: 74) L:12 C:0XFFE)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:12 C:0XFFF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb5 = {
			{ { 0, -4, -4}, 13, 0x1fff}, { { 1, -4, -3}, 12, 0xff7 }, { { 2, -4, -2}, 11, 0x7f4 },
			{ { 3, -4, -1}, 11, 0x7e8 }, { { 4, -4,  0}, 10, 0x3f1 }, { { 5, -4,  1}, 11, 0x7ee },
			{ { 6, -4,  2}, 11, 0x7f9 }, { { 7, -4,  3}, 12, 0xff8 }, { { 8, -4,  4}, 13, 0x1ffd},
			{ { 9, -3, -4}, 12, 0xffd }, { {10, -3, -3}, 11, 0x7f1 }, { {11, -3, -2}, 10, 0x3e8 },
			{ {12, -3, -1},  9, 0x1e8 }, { {13, -3,  0},  8, 0xf0  }, { {14, -3,  1},  9, 0x1ec },
			{ {15, -3,  2}, 10, 0x3ee }, { {16, -3,  3}, 11, 0x7f2 }, { {17, -3,  4}, 12, 0xffa },
			{ {18, -2, -4}, 12, 0xff4 }, { {19, -2, -3}, 10, 0x3ef }, { {20, -2, -2},  9, 0x1f2 },
			{ {21, -2, -1},  8, 0xe8  }, { {22, -2,  0},  7, 0x70  }, { {23, -2,  1},  8, 0xec  },
			{ {24, -2,  2},  9, 0x1f0 }, { {25, -2,  3}, 10, 0x3ea }, { {26, -2,  4}, 11, 0x7f3 },
			{ {27, -1, -4}, 11, 0x7eb }, { {28, -1, -3},  9, 0x1eb }, { {29, -1, -2},  8, 0xea  },
			{ {30, -1, -1},  5, 0x1a  }, { {31, -1,  0},  4, 0x8   }, { {32, -1,  1},  5, 0x19  },
			{ {33, -1,  2},  8, 0xee  }, { {34, -1,  3},  9, 0x1ef }, { {35, -1,  4}, 11, 0x7ed },
			{ {36,  0, -4}, 10, 0x3f0 }, { {37,  0, -3},  8, 0xf2  }, { {38,  0, -2},  7, 0x73  },
			{ {39,  0, -1},  4, 0xb   }, { {40,  0,  0},  1, 0x0   }, { {41,  0,  1},  4, 0xa   },
			{ {42,  0,  2},  7, 0x71  }, { {43,  0,  3},  8, 0xf3  }, { {44,  0,  4}, 11, 0x7e9 },
			{ {45,  1, -4}, 11, 0x7ef }, { {46,  1, -3},  9, 0x1ee }, { {47,  1, -2},  8, 0xef  },
			{ {48,  1, -1},  5, 0x18  }, { {49,  1,  0},  4, 0x9   }, { {50,  1,  1},  5, 0x1b  },
			{ {51,  1,  2},  8, 0xeb  }, { {52,  1,  3},  9, 0x1e9 }, { {53,  1,  4}, 11, 0x7ec },
			{ {54,  2, -4}, 11, 0x7f6 }, { {55,  2, -3}, 10, 0x3eb }, { {56,  2, -2},  9, 0x1f3 },
			{ {57,  2, -1},  8, 0xed  }, { {58,  2,  0},  7, 0x72  }, { {59,  2,  1},  8, 0xe9  },
			{ {60,  2,  2},  9, 0x1f1 }, { {61,  2,  3}, 10, 0x3ed }, { {62,  2,  4}, 11, 0x7f7 },
			{ {63,  3, -4}, 12, 0xff6 }, { {64,  3, -3}, 11, 0x7f0 }, { {65,  3, -2}, 10, 0x3e9 },
			{ {66,  3, -1},  9, 0x1ed }, { {67,  3,  0},  8, 0xf1  }, { {68,  3,  1},  9, 0x1ea },
			{ {69,  3,  2}, 10, 0x3ec }, { {70,  3,  3}, 11, 0x7f8 }, { {71,  3,  4}, 12, 0xff9 },
			{ {72,  4, -4}, 13, 0x1ffc}, { {73,  4, -3}, 12, 0xffc }, { {74,  4, -2}, 12, 0xff5 },
			{ {75,  4, -1}, 11, 0x7ea }, { {76,  4,  0}, 10, 0x3f3 }, { {77,  4,  1}, 10, 0x3f2 },
			{ {78,  4,  2}, 11, 0x7f5 }, { {79,  4,  3}, 12, 0xffb }, { {80,  4,  4}, 13, 0x1ffe},
		};

		const int shcb5_bst[161][2] = {
			{1, 2},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:1 C:0X0)
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{31, 0},                            // leaf node (V-index:31(V: 31) L:4 C:0X8)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:4 C:0X9)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:4 C:0XA)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:4 C:0XB)
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{48, 0},                            // leaf node (V-index:48(V: 48) L:5 C:0X18)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:5 C:0X19)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:5 C:0X1A)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:5 C:0X1B)
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{22, 0},                            // leaf node (V-index:22(V: 22) L:7 C:0X70)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:7 C:0X71)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:7 C:0X72)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:7 C:0X73)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{21, 0},                            // leaf node (V-index:21(V: 21) L:8 C:0XE8)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:8 C:0XE9)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:8 C:0XEA)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:8 C:0XEB)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:8 C:0XEC)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:8 C:0XED)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:8 C:0XEE)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:8 C:0XEF)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:8 C:0XF0)
			{67, 0},                            // leaf node (V-index:67(V: 67) L:8 C:0XF1)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:8 C:0XF2)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:8 C:0XF3)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{12, 0},                            // leaf node (V-index:12(V: 12) L:9 C:0X1E8)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:9 C:0X1E9)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:9 C:0X1EA)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:9 C:0X1EB)
			{14, 0},                            // leaf node (V-index:14(V: 14) L:9 C:0X1EC)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:9 C:0X1ED)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:9 C:0X1EE)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:9 C:0X1EF)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:9 C:0X1F0)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:9 C:0X1F1)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:9 C:0X1F2)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:9 C:0X1F3)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{11, 0},                            // leaf node (V-index:11(V: 11) L:10 C:0X3E8)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:10 C:0X3E9)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:10 C:0X3EA)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:10 C:0X3EB)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:10 C:0X3EC)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:10 C:0X3ED)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:10 C:0X3EE)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:10 C:0X3EF)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:10 C:0X3F0)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:10 C:0X3F1)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:10 C:0X3F2)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:10 C:0X3F3)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{3, 0},                             // leaf node (V-index:3(V: 3) L:11 C:0X7E8)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:11 C:0X7E9)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:11 C:0X7EA)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:11 C:0X7EB)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:11 C:0X7EC)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:11 C:0X7ED)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:11 C:0X7EE)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:11 C:0X7EF)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:11 C:0X7F0)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:11 C:0X7F1)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:11 C:0X7F2)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:11 C:0X7F3)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:11 C:0X7F4)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:11 C:0X7F5)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:11 C:0X7F6)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:11 C:0X7F7)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:11 C:0X7F8)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:11 C:0X7F9)
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{18, 0},                            // leaf node (V-index:18(V: 18) L:12 C:0XFF4)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:12 C:0XFF5)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:12 C:0XFF6)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:12 C:0XFF7)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:12 C:0XFF8)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:12 C:0XFF9)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:12 C:0XFFA)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:12 C:0XFFB)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:12 C:0XFFC)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:12 C:0XFFD)
			{2, 3},
			{3, 4},
			{72, 0},                            // leaf node (V-index:72(V: 72) L:13 C:0X1FFC)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:13 C:0X1FFD)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:13 C:0X1FFE)
			{0, 0},                             // leaf node (V-index:0(V: 0) L:13 C:0X1FFF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb6 = {
			{ { 0, -4, -4}, 11, 0x7fe}, { { 1, -4, -3}, 10, 0x3fd}, { { 2, -4, -2},  9, 0x1f1},
			{ { 3, -4, -1},  9, 0x1eb}, { { 4, -4,  0},  9, 0x1f4}, { { 5, -4,  1},  9, 0x1ea},
			{ { 6, -4,  2},  9, 0x1f0}, { { 7, -4,  3}, 10, 0x3fc}, { { 8, -4,  4}, 11, 0x7fd},
			{ { 9, -3, -4}, 10, 0x3f6}, { {10, -3, -3},  9, 0x1e5}, { {11, -3, -2},  8, 0xea },
			{ {12, -3, -1},  7, 0x6c }, { {13, -3,  0},  7, 0x71 }, { {14, -3,  1},  7, 0x68 },
			{ {15, -3,  2},  8, 0xf0 }, { {16, -3,  3},  9, 0x1e6}, { {17, -3,  4}, 10, 0x3f7},
			{ {18, -2, -4},  9, 0x1f3}, { {19, -2, -3},  8, 0xef }, { {20, -2, -2},  6, 0x32 },
			{ {21, -2, -1},  6, 0x27 }, { {22, -2,  0},  6, 0x28 }, { {23, -2,  1},  6, 0x26 },
			{ {24, -2,  2},  6, 0x31 }, { {25, -2,  3},  8, 0xeb }, { {26, -2,  4},  9, 0x1f7},
			{ {27, -1, -4},  9, 0x1e8}, { {28, -1, -3},  7, 0x6f }, { {29, -1, -2},  6, 0x2e },
			{ {30, -1, -1},  4, 0x8  }, { {31, -1,  0},  4, 0x4  }, { {32, -1,  1},  4, 0x6  },
			{ {33, -1,  2},  6, 0x29 }, { {34, -1,  3},  7, 0x6b }, { {35, -1,  4},  9, 0x1ee},
			{ {36,  0, -4},  9, 0x1ef}, { {37,  0, -3},  7, 0x72 }, { {38,  0, -2},  6, 0x2d },
			{ {39,  0, -1},  4, 0x2  }, { {40,  0,  0},  4, 0x0  }, { {41,  0,  1},  4, 0x3  },
			{ {42,  0,  2},  6, 0x2f }, { {43,  0,  3},  7, 0x73 }, { {44,  0,  4},  9, 0x1fa},
			{ {45,  1, -4},  9, 0x1e7}, { {46,  1, -3},  7, 0x6e }, { {47,  1, -2},  6, 0x2b },
			{ {48,  1, -1},  4, 0x7  }, { {49,  1,  0},  4, 0x1  }, { {50,  1,  1},  4, 0x5  },
			{ {51,  1,  2},  6, 0x2c }, { {52,  1,  3},  7, 0x6d }, { {53,  1,  4},  9, 0x1ec},
			{ {54,  2, -4},  9, 0x1f9}, { {55,  2, -3},  8, 0xee }, { {56,  2, -2},  6, 0x30 },
			{ {57,  2, -1},  6, 0x24 }, { {58,  2,  0},  6, 0x2a }, { {59,  2,  1},  6, 0x25 },
			{ {60,  2,  2},  6, 0x33 }, { {61,  2,  3},  8, 0xec }, { {62,  2,  4},  9, 0x1f2},
			{ {63,  3, -4}, 10, 0x3f8}, { {64,  3, -3},  9, 0x1e4}, { {65,  3, -2},  8, 0xed },
			{ {66,  3, -1},  7, 0x6a }, { {67,  3,  0},  7, 0x70 }, { {68,  3,  1},  7, 0x69 },
			{ {69,  3,  2},  7, 0x74 }, { {70,  3,  3},  8, 0xf1 }, { {71,  3,  4}, 10, 0x3fa},
			{ {72,  4, -4}, 11, 0x7ff}, { {73,  4, -3}, 10, 0x3f9}, { {74,  4, -2},  9, 0x1f6},
			{ {75,  4, -1},  9, 0x1ed}, { {76,  4,  0},  9, 0x1f8}, { {77,  4,  1},  9, 0x1e9},
			{ {78,  4,  2},  9, 0x1f5}, { {79,  4,  3}, 10, 0x3fb}, { {80,  4,  4}, 11, 0x7fc},
		};

		const int shcb6_bst[161][2] = {
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:4 C:0X0)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:4 C:0X1)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:4 C:0X2)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:4 C:0X3)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:4 C:0X4)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:4 C:0X5)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:4 C:0X6)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:4 C:0X7)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:4 C:0X8)
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{57, 0},                            // leaf node (V-index:57(V: 57) L:6 C:0X24)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:6 C:0X25)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:6 C:0X26)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:6 C:0X27)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:6 C:0X28)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:6 C:0X29)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:6 C:0X2A)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:6 C:0X2B)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:6 C:0X2C)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:6 C:0X2D)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:6 C:0X2E)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:6 C:0X2F)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:6 C:0X30)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:6 C:0X31)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:6 C:0X32)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:6 C:0X33)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{14, 0},                            // leaf node (V-index:14(V: 14) L:7 C:0X68)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:7 C:0X69)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:7 C:0X6A)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:7 C:0X6B)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:7 C:0X6C)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:7 C:0X6D)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:7 C:0X6E)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:7 C:0X6F)
			{67, 0},                            // leaf node (V-index:67(V: 67) L:7 C:0X70)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:7 C:0X71)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:7 C:0X72)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:7 C:0X73)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:7 C:0X74)
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{11, 0},                            // leaf node (V-index:11(V: 11) L:8 C:0XEA)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:8 C:0XEB)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:8 C:0XEC)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:8 C:0XED)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:8 C:0XEE)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:8 C:0XEF)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:8 C:0XF0)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:8 C:0XF1)
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{64, 0},                            // leaf node (V-index:64(V: 64) L:9 C:0X1E4)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:9 C:0X1E5)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:9 C:0X1E6)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:9 C:0X1E7)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:9 C:0X1E8)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:9 C:0X1E9)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:9 C:0X1EA)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:9 C:0X1EB)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:9 C:0X1EC)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:9 C:0X1ED)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:9 C:0X1EE)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:9 C:0X1EF)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1F0)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:9 C:0X1F1)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:9 C:0X1F2)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:9 C:0X1F3)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:9 C:0X1F4)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:9 C:0X1F5)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:9 C:0X1F6)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:9 C:0X1F7)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:9 C:0X1F8)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:9 C:0X1F9)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:9 C:0X1FA)
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{9, 0},                             // leaf node (V-index:9(V: 9) L:10 C:0X3F6)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:10 C:0X3F7)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:10 C:0X3F8)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:10 C:0X3F9)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:10 C:0X3FA)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:10 C:0X3FB)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:10 C:0X3FC)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:10 C:0X3FD)
			{2, 3},
			{3, 4},
			{80, 0},                            // leaf node (V-index:80(V: 80) L:11 C:0X7FC)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:11 C:0X7FD)
			{0, 0},                             // leaf node (V-index:0(V: 0) L:11 C:0X7FE)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:11 C:0X7FF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb7 = {
			{ { 0, 0, 0},  1, 0x0  }, { { 1, 0, 1},  3, 0x5  }, { { 2, 0, 2},  6, 0x37 },
			{ { 3, 0, 3},  7, 0x74 }, { { 4, 0, 4},  8, 0xf2 }, { { 5, 0, 5},  9, 0x1eb},
			{ { 6, 0, 6}, 10, 0x3ed}, { { 7, 0, 7}, 11, 0x7f7}, { { 8, 1, 0},  3, 0x4  },
			{ { 9, 1, 1},  4, 0xc  }, { {10, 1, 2},  6, 0x35 }, { {11, 1, 3},  7, 0x71 },
			{ {12, 1, 4},  8, 0xec }, { {13, 1, 5},  8, 0xee }, { {14, 1, 6},  9, 0x1ee},
			{ {15, 1, 7},  9, 0x1f5}, { {16, 2, 0},  6, 0x36 }, { {17, 2, 1},  6, 0x34 },
			{ {18, 2, 2},  7, 0x72 }, { {19, 2, 3},  8, 0xea }, { {20, 2, 4},  8, 0xf1 },
			{ {21, 2, 5},  9, 0x1e9}, { {22, 2, 6},  9, 0x1f3}, { {23, 2, 7}, 10, 0x3f5},
			{ {24, 3, 0},  7, 0x73 }, { {25, 3, 1},  7, 0x70 }, { {26, 3, 2},  8, 0xeb },
			{ {27, 3, 3},  8, 0xf0 }, { {28, 3, 4},  9, 0x1f1}, { {29, 3, 5},  9, 0x1f0},
			{ {30, 3, 6}, 10, 0x3ec}, { {31, 3, 7}, 10, 0x3fa}, { {32, 4, 0},  8, 0xf3 },
			{ {33, 4, 1},  8, 0xed }, { {34, 4, 2},  9, 0x1e8}, { {35, 4, 3},  9, 0x1ef},
			{ {36, 4, 4}, 10, 0x3ef}, { {37, 4, 5}, 10, 0x3f1}, { {38, 4, 6}, 10, 0x3f9},
			{ {39, 4, 7}, 11, 0x7fb}, { {40, 5, 0},  9, 0x1ed}, { {41, 5, 1},  8, 0xef },
			{ {42, 5, 2},  9, 0x1ea}, { {43, 5, 3},  9, 0x1f2}, { {44, 5, 4}, 10, 0x3f3},
			{ {45, 5, 5}, 10, 0x3f8}, { {46, 5, 6}, 11, 0x7f9}, { {47, 5, 7}, 11, 0x7fc},
			{ {48, 6, 0}, 10, 0x3ee}, { {49, 6, 1},  9, 0x1ec}, { {50, 6, 2},  9, 0x1f4},
			{ {51, 6, 3}, 10, 0x3f4}, { {52, 6, 4}, 10, 0x3f7}, { {53, 6, 5}, 11, 0x7f8},
			{ {54, 6, 6}, 12, 0xffd}, { {55, 6, 7}, 12, 0xffe}, { {56, 7, 0}, 11, 0x7f6},
			{ {57, 7, 1}, 10, 0x3f0}, { {58, 7, 2}, 10, 0x3f2}, { {59, 7, 3}, 10, 0x3f6},
			{ {60, 7, 4}, 11, 0x7fa}, { {61, 7, 5}, 11, 0x7fd}, { {62, 7, 6}, 12, 0xffc},
			{ {63, 7, 7}, 12, 0xfff},
		};

		const int shcb7_bst[127][2] = {
			{1, 2},
			{0, 0},                             // leaf node (V-index:0(V: 0) L:1 C:0X0)
			{1, 2},
			{2, 3},
			{3, 4},
			{8, 0},                             // leaf node (V-index:8(V: 8) L:3 C:0X4)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:3 C:0X5)
			{2, 3},
			{3, 4},
			{9, 0},                             // leaf node (V-index:9(V: 9) L:4 C:0XC)
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{17, 0},                            // leaf node (V-index:17(V: 17) L:6 C:0X34)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:6 C:0X35)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:6 C:0X36)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:6 C:0X37)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{25, 0},                            // leaf node (V-index:25(V: 25) L:7 C:0X70)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:7 C:0X71)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:7 C:0X72)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:7 C:0X73)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:7 C:0X74)
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{19, 0},                            // leaf node (V-index:19(V: 19) L:8 C:0XEA)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:8 C:0XEB)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:8 C:0XEC)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:8 C:0XED)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:8 C:0XEE)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:8 C:0XEF)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:8 C:0XF0)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:8 C:0XF1)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:8 C:0XF2)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:8 C:0XF3)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{34, 0},                            // leaf node (V-index:34(V: 34) L:9 C:0X1E8)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:9 C:0X1E9)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:9 C:0X1EA)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:9 C:0X1EB)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:9 C:0X1EC)
			{40, 0},                            // leaf node (V-index:40(V: 40) L:9 C:0X1ED)
			{14, 0},                            // leaf node (V-index:14(V: 14) L:9 C:0X1EE)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:9 C:0X1EF)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:9 C:0X1F0)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:9 C:0X1F1)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:9 C:0X1F2)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:9 C:0X1F3)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:9 C:0X1F4)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:9 C:0X1F5)
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{30, 0},                            // leaf node (V-index:30(V: 30) L:10 C:0X3EC)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:10 C:0X3ED)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:10 C:0X3EE)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:10 C:0X3EF)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:10 C:0X3F0)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:10 C:0X3F1)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:10 C:0X3F2)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:10 C:0X3F3)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:10 C:0X3F4)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:10 C:0X3F5)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:10 C:0X3F6)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:10 C:0X3F7)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:10 C:0X3F8)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:10 C:0X3F9)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:10 C:0X3FA)
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{56, 0},                            // leaf node (V-index:56(V: 56) L:11 C:0X7F6)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:11 C:0X7F7)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:11 C:0X7F8)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:11 C:0X7F9)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:11 C:0X7FA)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:11 C:0X7FB)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:11 C:0X7FC)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:11 C:0X7FD)
			{2, 3},
			{3, 4},
			{62, 0},                            // leaf node (V-index:62(V: 62) L:12 C:0XFFC)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:12 C:0XFFD)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:12 C:0XFFE)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:12 C:0XFFF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb8 = {
			{ { 0, 0, 0},  5, 0xe  }, { { 1, 0, 1},  4, 0x5  }, { { 2, 0, 2},  5, 0x10 },
			{ { 3, 0, 3},  6, 0x30 }, { { 4, 0, 4},  7, 0x6f }, { { 5, 0, 5},  8, 0xf1 },
			{ { 6, 0, 6},  9, 0x1fa}, { { 7, 0, 7}, 10, 0x3fe}, { { 8, 1, 0},  4, 0x3  },
			{ { 9, 1, 1},  3, 0x0  }, { {10, 1, 2},  4, 0x4  }, { {11, 1, 3},  5, 0x12 },
			{ {12, 1, 4},  6, 0x2c }, { {13, 1, 5},  7, 0x6a }, { {14, 1, 6},  7, 0x75 },
			{ {15, 1, 7},  8, 0xf8 }, { {16, 2, 0},  5, 0xf  }, { {17, 2, 1},  4, 0x2  },
			{ {18, 2, 2},  4, 0x6  }, { {19, 2, 3},  5, 0x14 }, { {20, 2, 4},  6, 0x2e },
			{ {21, 2, 5},  7, 0x69 }, { {22, 2, 6},  7, 0x72 }, { {23, 2, 7},  8, 0xf5 },
			{ {24, 3, 0},  6, 0x2f }, { {25, 3, 1},  5, 0x11 }, { {26, 3, 2},  5, 0x13 },
			{ {27, 3, 3},  6, 0x2a }, { {28, 3, 4},  6, 0x32 }, { {29, 3, 5},  7, 0x6c },
			{ {30, 3, 6},  8, 0xec }, { {31, 3, 7},  8, 0xfa }, { {32, 4, 0},  7, 0x71 },
			{ {33, 4, 1},  6, 0x2b }, { {34, 4, 2},  6, 0x2d }, { {35, 4, 3},  6, 0x31 },
			{ {36, 4, 4},  7, 0x6d }, { {37, 4, 5},  7, 0x70 }, { {38, 4, 6},  8, 0xf2 },
			{ {39, 4, 7},  9, 0x1f9}, { {40, 5, 0},  8, 0xef }, { {41, 5, 1},  7, 0x68 },
			{ {42, 5, 2},  6, 0x33 }, { {43, 5, 3},  7, 0x6b }, { {44, 5, 4},  7, 0x6e },
			{ {45, 5, 5},  8, 0xee }, { {46, 5, 6},  8, 0xf9 }, { {47, 5, 7}, 10, 0x3fc},
			{ {48, 6, 0},  9, 0x1f8}, { {49, 6, 1},  7, 0x74 }, { {50, 6, 2},  7, 0x73 },
			{ {51, 6, 3},  8, 0xed }, { {52, 6, 4},  8, 0xf0 }, { {53, 6, 5},  8, 0xf6 },
			{ {54, 6, 6},  9, 0x1f6}, { {55, 6, 7},  9, 0x1fd}, { {56, 7, 0}, 10, 0x3fd},
			{ {57, 7, 1},  8, 0xf3 }, { {58, 7, 2},  8, 0xf4 }, { {59, 7, 3},  8, 0xf7 },
			{ {60, 7, 4},  9, 0x1f7}, { {61, 7, 5},  9, 0x1fb}, { {62, 7, 6},  9, 0x1fc},
			{ {63, 7, 7}, 10, 0x3ff},
		};

		const int shcb8_bst[127][2] = {
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{9, 0},                             // leaf node (V-index:9(V: 9) L:3 C:0X0)
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{17, 0},                            // leaf node (V-index:17(V: 17) L:4 C:0X2)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:4 C:0X3)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:4 C:0X4)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:4 C:0X5)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:4 C:0X6)
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{0, 0},                             // leaf node (V-index:0(V: 0) L:5 C:0XE)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:5 C:0XF)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:5 C:0X10)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:5 C:0X11)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:5 C:0X12)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:5 C:0X13)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:5 C:0X14)
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{27, 0},                            // leaf node (V-index:27(V: 27) L:6 C:0X2A)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:6 C:0X2B)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:6 C:0X2C)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:6 C:0X2D)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:6 C:0X2E)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:6 C:0X2F)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:6 C:0X30)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:6 C:0X31)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:6 C:0X32)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:6 C:0X33)
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{41, 0},                            // leaf node (V-index:41(V: 41) L:7 C:0X68)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:7 C:0X69)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:7 C:0X6A)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:7 C:0X6B)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:7 C:0X6C)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:7 C:0X6D)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:7 C:0X6E)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:7 C:0X6F)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:7 C:0X70)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:7 C:0X71)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:7 C:0X72)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:7 C:0X73)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:7 C:0X74)
			{14, 0},                            // leaf node (V-index:14(V: 14) L:7 C:0X75)
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{30, 0},                            // leaf node (V-index:30(V: 30) L:8 C:0XEC)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:8 C:0XED)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:8 C:0XEE)
			{40, 0},                            // leaf node (V-index:40(V: 40) L:8 C:0XEF)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:8 C:0XF0)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:8 C:0XF1)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:8 C:0XF2)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:8 C:0XF3)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:8 C:0XF4)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:8 C:0XF5)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:8 C:0XF6)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:8 C:0XF7)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:8 C:0XF8)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:8 C:0XF9)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:8 C:0XFA)
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{54, 0},                            // leaf node (V-index:54(V: 54) L:9 C:0X1F6)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:9 C:0X1F7)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:9 C:0X1F8)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:9 C:0X1F9)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1FA)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:9 C:0X1FB)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:9 C:0X1FC)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:9 C:0X1FD)
			{2, 3},
			{3, 4},
			{47, 0},                            // leaf node (V-index:47(V: 47) L:10 C:0X3FC)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:10 C:0X3FD)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:10 C:0X3FE)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:10 C:0X3FF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb9 = {
			{ {  0,  0,  0},  1, 0x0   }, { {  1,  0,  1},  3, 0x5   }, { {  2,  0,  2},  6, 0x37  },
			{ {  3,  0,  3},  8, 0xe7  }, { {  4,  0,  4},  9, 0x1de }, { {  5,  0,  5}, 10, 0x3ce },
			{ {  6,  0,  6}, 10, 0x3d9 }, { {  7,  0,  7}, 11, 0x7c8 }, { {  8,  0,  8}, 11, 0x7cd },
			{ {  9,  0,  9}, 12, 0xfc8 }, { { 10,  0, 10}, 12, 0xfdd }, { { 11,  0, 11}, 13, 0x1fe4},
			{ { 12,  0, 12}, 13, 0x1fec}, { { 13,  1,  0},  3, 0x4   }, { { 14,  1,  1},  4, 0xc   },
			{ { 15,  1,  2},  6, 0x35  }, { { 16,  1,  3},  7, 0x72  }, { { 17,  1,  4},  8, 0xea  },
			{ { 18,  1,  5},  8, 0xed  }, { { 19,  1,  6},  9, 0x1e2 }, { { 20,  1,  7}, 10, 0x3d1 },
			{ { 21,  1,  8}, 10, 0x3d3 }, { { 22,  1,  9}, 10, 0x3e0 }, { { 23,  1, 10}, 11, 0x7d8 },
			{ { 24,  1, 11}, 12, 0xfcf }, { { 25,  1, 12}, 12, 0xfd5 }, { { 26,  2,  0},  6, 0x36  },
			{ { 27,  2,  1},  6, 0x34  }, { { 28,  2,  2},  7, 0x71  }, { { 29,  2,  3},  8, 0xe8  },
			{ { 30,  2,  4},  8, 0xec  }, { { 31,  2,  5},  9, 0x1e1 }, { { 32,  2,  6}, 10, 0x3cf },
			{ { 33,  2,  7}, 10, 0x3dd }, { { 34,  2,  8}, 10, 0x3db }, { { 35,  2,  9}, 11, 0x7d0 },
			{ { 36,  2, 10}, 12, 0xfc7 }, { { 37,  2, 11}, 12, 0xfd4 }, { { 38,  2, 12}, 12, 0xfe4 },
			{ { 39,  3,  0},  8, 0xe6  }, { { 40,  3,  1},  7, 0x70  }, { { 41,  3,  2},  8, 0xe9  },
			{ { 42,  3,  3},  9, 0x1dd }, { { 43,  3,  4},  9, 0x1e3 }, { { 44,  3,  5}, 10, 0x3d2 },
			{ { 45,  3,  6}, 10, 0x3dc }, { { 46,  3,  7}, 11, 0x7cc }, { { 47,  3,  8}, 11, 0x7ca },
			{ { 48,  3,  9}, 11, 0x7de }, { { 49,  3, 10}, 12, 0xfd8 }, { { 50,  3, 11}, 12, 0xfea },
			{ { 51,  3, 12}, 13, 0x1fdb}, { { 52,  4,  0},  9, 0x1df }, { { 53,  4,  1},  8, 0xeb  },
			{ { 54,  4,  2},  9, 0x1dc }, { { 55,  4,  3},  9, 0x1e6 }, { { 56,  4,  4}, 10, 0x3d5 },
			{ { 57,  4,  5}, 10, 0x3de }, { { 58,  4,  6}, 11, 0x7cb }, { { 59,  4,  7}, 11, 0x7dd },
			{ { 60,  4,  8}, 11, 0x7dc }, { { 61,  4,  9}, 12, 0xfcd }, { { 62,  4, 10}, 12, 0xfe2 },
			{ { 63,  4, 11}, 12, 0xfe7 }, { { 64,  4, 12}, 13, 0x1fe1}, { { 65,  5,  0}, 10, 0x3d0 },
			{ { 66,  5,  1},  9, 0x1e0 }, { { 67,  5,  2},  9, 0x1e4 }, { { 68,  5,  3}, 10, 0x3d6 },
			{ { 69,  5,  4}, 11, 0x7c5 }, { { 70,  5,  5}, 11, 0x7d1 }, { { 71,  5,  6}, 11, 0x7db },
			{ { 72,  5,  7}, 12, 0xfd2 }, { { 73,  5,  8}, 11, 0x7e0 }, { { 74,  5,  9}, 12, 0xfd9 },
			{ { 75,  5, 10}, 12, 0xfeb }, { { 76,  5, 11}, 13, 0x1fe3}, { { 77,  5, 12}, 13, 0x1fe9},
			{ { 78,  6,  0}, 11, 0x7c4 }, { { 79,  6,  1},  9, 0x1e5 }, { { 80,  6,  2}, 10, 0x3d7 },
			{ { 81,  6,  3}, 11, 0x7c6 }, { { 82,  6,  4}, 11, 0x7cf }, { { 83,  6,  5}, 11, 0x7da },
			{ { 84,  6,  6}, 12, 0xfcb }, { { 85,  6,  7}, 12, 0xfda }, { { 86,  6,  8}, 12, 0xfe3 },
			{ { 87,  6,  9}, 12, 0xfe9 }, { { 88,  6, 10}, 13, 0x1fe6}, { { 89,  6, 11}, 13, 0x1ff3},
			{ { 90,  6, 12}, 13, 0x1ff7}, { { 91,  7,  0}, 11, 0x7d3 }, { { 92,  7,  1}, 10, 0x3d8 },
			{ { 93,  7,  2}, 10, 0x3e1 }, { { 94,  7,  3}, 11, 0x7d4 }, { { 95,  7,  4}, 11, 0x7d9 },
			{ { 96,  7,  5}, 12, 0xfd3 }, { { 97,  7,  6}, 12, 0xfde }, { { 98,  7,  7}, 13, 0x1fdd},
			{ { 99,  7,  8}, 13, 0x1fd9}, { {100,  7,  9}, 13, 0x1fe2}, { {101,  7, 10}, 13, 0x1fea},
			{ {102,  7, 11}, 13, 0x1ff1}, { {103,  7, 12}, 13, 0x1ff6}, { {104,  8,  0}, 11, 0x7d2 },
			{ {105,  8,  1}, 10, 0x3d4 }, { {106,  8,  2}, 10, 0x3da }, { {107,  8,  3}, 11, 0x7c7 },
			{ {108,  8,  4}, 11, 0x7d7 }, { {109,  8,  5}, 11, 0x7e2 }, { {110,  8,  6}, 12, 0xfce },
			{ {111,  8,  7}, 12, 0xfdb }, { {112,  8,  8}, 13, 0x1fd8}, { {113,  8,  9}, 13, 0x1fee},
			{ {114,  8, 10}, 14, 0x3ff0}, { {115,  8, 11}, 13, 0x1ff4}, { {116,  8, 12}, 14, 0x3ff2},
			{ {117,  9,  0}, 11, 0x7e1 }, { {118,  9,  1}, 10, 0x3df }, { {119,  9,  2}, 11, 0x7c9 },
			{ {120,  9,  3}, 11, 0x7d6 }, { {121,  9,  4}, 12, 0xfca }, { {122,  9,  5}, 12, 0xfd0 },
			{ {123,  9,  6}, 12, 0xfe5 }, { {124,  9,  7}, 12, 0xfe6 }, { {125,  9,  8}, 13, 0x1feb},
			{ {126,  9,  9}, 13, 0x1fef}, { {127,  9, 10}, 14, 0x3ff3}, { {128,  9, 11}, 14, 0x3ff4},
			{ {129,  9, 12}, 14, 0x3ff5}, { {130, 10,  0}, 12, 0xfe0 }, { {131, 10,  1}, 11, 0x7ce },
			{ {132, 10,  2}, 11, 0x7d5 }, { {133, 10,  3}, 12, 0xfc6 }, { {134, 10,  4}, 12, 0xfd1 },
			{ {135, 10,  5}, 12, 0xfe1 }, { {136, 10,  6}, 13, 0x1fe0}, { {137, 10,  7}, 13, 0x1fe8},
			{ {138, 10,  8}, 13, 0x1ff0}, { {139, 10,  9}, 14, 0x3ff1}, { {140, 10, 10}, 14, 0x3ff8},
			{ {141, 10, 11}, 14, 0x3ff6}, { {142, 10, 12}, 15, 0x7ffc}, { {143, 11,  0}, 12, 0xfe8 },
			{ {144, 11,  1}, 11, 0x7df }, { {145, 11,  2}, 12, 0xfc9 }, { {146, 11,  3}, 12, 0xfd7 },
			{ {147, 11,  4}, 12, 0xfdc }, { {148, 11,  5}, 13, 0x1fdc}, { {149, 11,  6}, 13, 0x1fdf},
			{ {150, 11,  7}, 13, 0x1fed}, { {151, 11,  8}, 13, 0x1ff5}, { {152, 11,  9}, 14, 0x3ff9},
			{ {153, 11, 10}, 14, 0x3ffb}, { {154, 11, 11}, 15, 0x7ffd}, { {155, 11, 12}, 15, 0x7ffe},
			{ {156, 12,  0}, 13, 0x1fe7}, { {157, 12,  1}, 12, 0xfcc }, { {158, 12,  2}, 12, 0xfd6 },
			{ {159, 12,  3}, 12, 0xfdf }, { {160, 12,  4}, 13, 0x1fde}, { {161, 12,  5}, 13, 0x1fda},
			{ {162, 12,  6}, 13, 0x1fe5}, { {163, 12,  7}, 13, 0x1ff2}, { {164, 12,  8}, 14, 0x3ffa},
			{ {165, 12,  9}, 14, 0x3ff7}, { {166, 12, 10}, 14, 0x3ffc}, { {167, 12, 11}, 14, 0x3ffd},
			{ {168, 12, 12}, 15, 0x7fff},
		};

		const int shcb9_bst[337][2] = {
			{1, 2},
			{0, 0},                             // leaf node (V-index:0(V: 0) L:1 C:0X0)
			{1, 2},
			{2, 3},
			{3, 4},
			{13, 0},                            // leaf node (V-index:13(V: 13) L:3 C:0X4)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:3 C:0X5)
			{2, 3},
			{3, 4},
			{14, 0},                            // leaf node (V-index:14(V: 14) L:4 C:0XC)
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{27, 0},                            // leaf node (V-index:27(V: 27) L:6 C:0X34)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:6 C:0X35)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:6 C:0X36)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:6 C:0X37)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{40, 0},                            // leaf node (V-index:40(V: 40) L:7 C:0X70)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:7 C:0X71)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:7 C:0X72)
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{39, 0},                            // leaf node (V-index:39(V: 39) L:8 C:0XE6)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:8 C:0XE7)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:8 C:0XE8)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:8 C:0XE9)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:8 C:0XEA)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:8 C:0XEB)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:8 C:0XEC)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:8 C:0XED)
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{54, 0},                            // leaf node (V-index:54(V: 54) L:9 C:0X1DC)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:9 C:0X1DD)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:9 C:0X1DE)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:9 C:0X1DF)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:9 C:0X1E0)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:9 C:0X1E1)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:9 C:0X1E2)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:9 C:0X1E3)
			{67, 0},                            // leaf node (V-index:67(V: 67) L:9 C:0X1E4)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:9 C:0X1E5)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:9 C:0X1E6)
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{5, 0},                             // leaf node (V-index:5(V: 5) L:10 C:0X3CE)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:10 C:0X3CF)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:10 C:0X3D0)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:10 C:0X3D1)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:10 C:0X3D2)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:10 C:0X3D3)
			{105, 0},                           // leaf node (V-index:105(V: 105) L:10 C:0X3D4)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:10 C:0X3D5)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:10 C:0X3D6)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:10 C:0X3D7)
			{92, 0},                            // leaf node (V-index:92(V: 92) L:10 C:0X3D8)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:10 C:0X3D9)
			{106, 0},                           // leaf node (V-index:106(V: 106) L:10 C:0X3DA)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:10 C:0X3DB)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:10 C:0X3DC)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:10 C:0X3DD)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:10 C:0X3DE)
			{118, 0},                           // leaf node (V-index:118(V: 118) L:10 C:0X3DF)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:10 C:0X3E0)
			{93, 0},                            // leaf node (V-index:93(V: 93) L:10 C:0X3E1)
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{50, 51},
			{51, 52},
			{52, 53},
			{53, 54},
			{54, 55},
			{55, 56},
			{56, 57},
			{57, 58},
			{58, 59},
			{59, 60},
			{78, 0},                            // leaf node (V-index:78(V: 78) L:11 C:0X7C4)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:11 C:0X7C5)
			{81, 0},                            // leaf node (V-index:81(V: 81) L:11 C:0X7C6)
			{107, 0},                           // leaf node (V-index:107(V: 107) L:11 C:0X7C7)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:11 C:0X7C8)
			{119, 0},                           // leaf node (V-index:119(V: 119) L:11 C:0X7C9)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:11 C:0X7CA)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:11 C:0X7CB)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:11 C:0X7CC)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:11 C:0X7CD)
			{131, 0},                           // leaf node (V-index:131(V: 131) L:11 C:0X7CE)
			{82, 0},                            // leaf node (V-index:82(V: 82) L:11 C:0X7CF)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:11 C:0X7D0)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:11 C:0X7D1)
			{104, 0},                           // leaf node (V-index:104(V: 104) L:11 C:0X7D2)
			{91, 0},                            // leaf node (V-index:91(V: 91) L:11 C:0X7D3)
			{94, 0},                            // leaf node (V-index:94(V: 94) L:11 C:0X7D4)
			{132, 0},                           // leaf node (V-index:132(V: 132) L:11 C:0X7D5)
			{120, 0},                           // leaf node (V-index:120(V: 120) L:11 C:0X7D6)
			{108, 0},                           // leaf node (V-index:108(V: 108) L:11 C:0X7D7)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:11 C:0X7D8)
			{95, 0},                            // leaf node (V-index:95(V: 95) L:11 C:0X7D9)
			{83, 0},                            // leaf node (V-index:83(V: 83) L:11 C:0X7DA)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:11 C:0X7DB)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:11 C:0X7DC)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:11 C:0X7DD)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:11 C:0X7DE)
			{144, 0},                           // leaf node (V-index:144(V: 144) L:11 C:0X7DF)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:11 C:0X7E0)
			{117, 0},                           // leaf node (V-index:117(V: 117) L:11 C:0X7E1)
			{109, 0},                           // leaf node (V-index:109(V: 109) L:11 C:0X7E2)
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{50, 51},
			{51, 52},
			{52, 53},
			{53, 54},
			{54, 55},
			{55, 56},
			{56, 57},
			{57, 58},
			{133, 0},                           // leaf node (V-index:133(V: 133) L:12 C:0XFC6)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:12 C:0XFC7)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:12 C:0XFC8)
			{145, 0},                           // leaf node (V-index:145(V: 145) L:12 C:0XFC9)
			{121, 0},                           // leaf node (V-index:121(V: 121) L:12 C:0XFCA)
			{84, 0},                            // leaf node (V-index:84(V: 84) L:12 C:0XFCB)
			{157, 0},                           // leaf node (V-index:157(V: 157) L:12 C:0XFCC)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:12 C:0XFCD)
			{110, 0},                           // leaf node (V-index:110(V: 110) L:12 C:0XFCE)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:12 C:0XFCF)
			{122, 0},                           // leaf node (V-index:122(V: 122) L:12 C:0XFD0)
			{134, 0},                           // leaf node (V-index:134(V: 134) L:12 C:0XFD1)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:12 C:0XFD2)
			{96, 0},                            // leaf node (V-index:96(V: 96) L:12 C:0XFD3)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:12 C:0XFD4)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:12 C:0XFD5)
			{158, 0},                           // leaf node (V-index:158(V: 158) L:12 C:0XFD6)
			{146, 0},                           // leaf node (V-index:146(V: 146) L:12 C:0XFD7)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:12 C:0XFD8)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:12 C:0XFD9)
			{85, 0},                            // leaf node (V-index:85(V: 85) L:12 C:0XFDA)
			{111, 0},                           // leaf node (V-index:111(V: 111) L:12 C:0XFDB)
			{147, 0},                           // leaf node (V-index:147(V: 147) L:12 C:0XFDC)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:12 C:0XFDD)
			{97, 0},                            // leaf node (V-index:97(V: 97) L:12 C:0XFDE)
			{159, 0},                           // leaf node (V-index:159(V: 159) L:12 C:0XFDF)
			{130, 0},                           // leaf node (V-index:130(V: 130) L:12 C:0XFE0)
			{135, 0},                           // leaf node (V-index:135(V: 135) L:12 C:0XFE1)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:12 C:0XFE2)
			{86, 0},                            // leaf node (V-index:86(V: 86) L:12 C:0XFE3)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:12 C:0XFE4)
			{123, 0},                           // leaf node (V-index:123(V: 123) L:12 C:0XFE5)
			{124, 0},                           // leaf node (V-index:124(V: 124) L:12 C:0XFE6)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:12 C:0XFE7)
			{143, 0},                           // leaf node (V-index:143(V: 143) L:12 C:0XFE8)
			{87, 0},                            // leaf node (V-index:87(V: 87) L:12 C:0XFE9)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:12 C:0XFEA)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:12 C:0XFEB)
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{112, 0},                           // leaf node (V-index:112(V: 112) L:13 C:0X1FD8)
			{99, 0},                            // leaf node (V-index:99(V: 99) L:13 C:0X1FD9)
			{161, 0},                           // leaf node (V-index:161(V: 161) L:13 C:0X1FDA)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:13 C:0X1FDB)
			{148, 0},                           // leaf node (V-index:148(V: 148) L:13 C:0X1FDC)
			{98, 0},                            // leaf node (V-index:98(V: 98) L:13 C:0X1FDD)
			{160, 0},                           // leaf node (V-index:160(V: 160) L:13 C:0X1FDE)
			{149, 0},                           // leaf node (V-index:149(V: 149) L:13 C:0X1FDF)
			{136, 0},                           // leaf node (V-index:136(V: 136) L:13 C:0X1FE0)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:13 C:0X1FE1)
			{100, 0},                           // leaf node (V-index:100(V: 100) L:13 C:0X1FE2)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:13 C:0X1FE3)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:13 C:0X1FE4)
			{162, 0},                           // leaf node (V-index:162(V: 162) L:13 C:0X1FE5)
			{88, 0},                            // leaf node (V-index:88(V: 88) L:13 C:0X1FE6)
			{156, 0},                           // leaf node (V-index:156(V: 156) L:13 C:0X1FE7)
			{137, 0},                           // leaf node (V-index:137(V: 137) L:13 C:0X1FE8)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:13 C:0X1FE9)
			{101, 0},                           // leaf node (V-index:101(V: 101) L:13 C:0X1FEA)
			{125, 0},                           // leaf node (V-index:125(V: 125) L:13 C:0X1FEB)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:13 C:0X1FEC)
			{150, 0},                           // leaf node (V-index:150(V: 150) L:13 C:0X1FED)
			{113, 0},                           // leaf node (V-index:113(V: 113) L:13 C:0X1FEE)
			{126, 0},                           // leaf node (V-index:126(V: 126) L:13 C:0X1FEF)
			{138, 0},                           // leaf node (V-index:138(V: 138) L:13 C:0X1FF0)
			{102, 0},                           // leaf node (V-index:102(V: 102) L:13 C:0X1FF1)
			{163, 0},                           // leaf node (V-index:163(V: 163) L:13 C:0X1FF2)
			{89, 0},                            // leaf node (V-index:89(V: 89) L:13 C:0X1FF3)
			{115, 0},                           // leaf node (V-index:115(V: 115) L:13 C:0X1FF4)
			{151, 0},                           // leaf node (V-index:151(V: 151) L:13 C:0X1FF5)
			{103, 0},                           // leaf node (V-index:103(V: 103) L:13 C:0X1FF6)
			{90, 0},                            // leaf node (V-index:90(V: 90) L:13 C:0X1FF7)
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{114, 0},                           // leaf node (V-index:114(V: 114) L:14 C:0X3FF0)
			{139, 0},                           // leaf node (V-index:139(V: 139) L:14 C:0X3FF1)
			{116, 0},                           // leaf node (V-index:116(V: 116) L:14 C:0X3FF2)
			{127, 0},                           // leaf node (V-index:127(V: 127) L:14 C:0X3FF3)
			{128, 0},                           // leaf node (V-index:128(V: 128) L:14 C:0X3FF4)
			{129, 0},                           // leaf node (V-index:129(V: 129) L:14 C:0X3FF5)
			{141, 0},                           // leaf node (V-index:141(V: 141) L:14 C:0X3FF6)
			{165, 0},                           // leaf node (V-index:165(V: 165) L:14 C:0X3FF7)
			{140, 0},                           // leaf node (V-index:140(V: 140) L:14 C:0X3FF8)
			{152, 0},                           // leaf node (V-index:152(V: 152) L:14 C:0X3FF9)
			{164, 0},                           // leaf node (V-index:164(V: 164) L:14 C:0X3FFA)
			{153, 0},                           // leaf node (V-index:153(V: 153) L:14 C:0X3FFB)
			{166, 0},                           // leaf node (V-index:166(V: 166) L:14 C:0X3FFC)
			{167, 0},                           // leaf node (V-index:167(V: 167) L:14 C:0X3FFD)
			{2, 3},
			{3, 4},
			{142, 0},                           // leaf node (V-index:142(V: 142) L:15 C:0X7FFC)
			{154, 0},                           // leaf node (V-index:154(V: 154) L:15 C:0X7FFD)
			{155, 0},                           // leaf node (V-index:155(V: 155) L:15 C:0X7FFE)
			{168, 0},                           // leaf node (V-index:168(V: 168) L:15 C:0X7FFF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb10 = {
			{ {  0,  0,  0},  6, 0x22 }, { {  1,  0,  1},  5, 0x8  }, { {  2,  0,  2},  6, 0x1d },
			{ {  3,  0,  3},  6, 0x26 }, { {  4,  0,  4},  7, 0x5f }, { {  5,  0,  5},  8, 0xd3 },
			{ {  6,  0,  6},  9, 0x1cf}, { {  7,  0,  7}, 10, 0x3d0}, { {  8,  0,  8}, 10, 0x3d7},
			{ {  9,  0,  9}, 10, 0x3ed}, { { 10,  0, 10}, 11, 0x7f0}, { { 11,  0, 11}, 11, 0x7f6},
			{ { 12,  0, 12}, 12, 0xffd}, { { 13,  1,  0},  5, 0x7  }, { { 14,  1,  1},  4, 0x0  },
			{ { 15,  1,  2},  4, 0x1  }, { { 16,  1,  3},  5, 0x9  }, { { 17,  1,  4},  6, 0x20 },
			{ { 18,  1,  5},  7, 0x54 }, { { 19,  1,  6},  7, 0x60 }, { { 20,  1,  7},  8, 0xd5 },
			{ { 21,  1,  8},  8, 0xdc }, { { 22,  1,  9},  9, 0x1d4}, { { 23,  1, 10}, 10, 0x3cd},
			{ { 24,  1, 11}, 10, 0x3de}, { { 25,  1, 12}, 11, 0x7e7}, { { 26,  2,  0},  6, 0x1c },
			{ { 27,  2,  1},  4, 0x2  }, { { 28,  2,  2},  5, 0x6  }, { { 29,  2,  3},  5, 0xc  },
			{ { 30,  2,  4},  6, 0x1e }, { { 31,  2,  5},  6, 0x28 }, { { 32,  2,  6},  7, 0x5b },
			{ { 33,  2,  7},  8, 0xcd }, { { 34,  2,  8},  8, 0xd9 }, { { 35,  2,  9},  9, 0x1ce},
			{ { 36,  2, 10},  9, 0x1dc}, { { 37,  2, 11}, 10, 0x3d9}, { { 38,  2, 12}, 10, 0x3f1},
			{ { 39,  3,  0},  6, 0x25 }, { { 40,  3,  1},  5, 0xb  }, { { 41,  3,  2},  5, 0xa  },
			{ { 42,  3,  3},  5, 0xd  }, { { 43,  3,  4},  6, 0x24 }, { { 44,  3,  5},  7, 0x57 },
			{ { 45,  3,  6},  7, 0x61 }, { { 46,  3,  7},  8, 0xcc }, { { 47,  3,  8},  8, 0xdd },
			{ { 48,  3,  9},  9, 0x1cc}, { { 49,  3, 10},  9, 0x1de}, { { 50,  3, 11}, 10, 0x3d3},
			{ { 51,  3, 12}, 10, 0x3e7}, { { 52,  4,  0},  7, 0x5d }, { { 53,  4,  1},  6, 0x21 },
			{ { 54,  4,  2},  6, 0x1f }, { { 55,  4,  3},  6, 0x23 }, { { 56,  4,  4},  6, 0x27 },
			{ { 57,  4,  5},  7, 0x59 }, { { 58,  4,  6},  7, 0x64 }, { { 59,  4,  7},  8, 0xd8 },
			{ { 60,  4,  8},  8, 0xdf }, { { 61,  4,  9},  9, 0x1d2}, { { 62,  4, 10},  9, 0x1e2},
			{ { 63,  4, 11}, 10, 0x3dd}, { { 64,  4, 12}, 10, 0x3ee}, { { 65,  5,  0},  8, 0xd1 },
			{ { 66,  5,  1},  7, 0x55 }, { { 67,  5,  2},  6, 0x29 }, { { 68,  5,  3},  7, 0x56 },
			{ { 69,  5,  4},  7, 0x58 }, { { 70,  5,  5},  7, 0x62 }, { { 71,  5,  6},  8, 0xce },
			{ { 72,  5,  7},  8, 0xe0 }, { { 73,  5,  8},  8, 0xe2 }, { { 74,  5,  9},  9, 0x1da},
			{ { 75,  5, 10}, 10, 0x3d4}, { { 76,  5, 11}, 10, 0x3e3}, { { 77,  5, 12}, 11, 0x7eb},
			{ { 78,  6,  0},  9, 0x1c9}, { { 79,  6,  1},  7, 0x5e }, { { 80,  6,  2},  7, 0x5a },
			{ { 81,  6,  3},  7, 0x5c }, { { 82,  6,  4},  7, 0x63 }, { { 83,  6,  5},  8, 0xca },
			{ { 84,  6,  6},  8, 0xda }, { { 85,  6,  7},  9, 0x1c7}, { { 86,  6,  8},  9, 0x1ca},
			{ { 87,  6,  9},  9, 0x1e0}, { { 88,  6, 10}, 10, 0x3db}, { { 89,  6, 11}, 10, 0x3e8},
			{ { 90,  6, 12}, 11, 0x7ec}, { { 91,  7,  0},  9, 0x1e3}, { { 92,  7,  1},  8, 0xd2 },
			{ { 93,  7,  2},  8, 0xcb }, { { 94,  7,  3},  8, 0xd0 }, { { 95,  7,  4},  8, 0xd7 },
			{ { 96,  7,  5},  8, 0xdb }, { { 97,  7,  6},  9, 0x1c6}, { { 98,  7,  7},  9, 0x1d5},
			{ { 99,  7,  8},  9, 0x1d8}, { {100,  7,  9}, 10, 0x3ca}, { {101,  7, 10}, 10, 0x3da},
			{ {102,  7, 11}, 11, 0x7ea}, { {103,  7, 12}, 11, 0x7f1}, { {104,  8,  0},  9, 0x1e1},
			{ {105,  8,  1},  8, 0xd4 }, { {106,  8,  2},  8, 0xcf }, { {107,  8,  3},  8, 0xd6 },
			{ {108,  8,  4},  8, 0xde }, { {109,  8,  5},  8, 0xe1 }, { {110,  8,  6},  9, 0x1d0},
			{ {111,  8,  7},  9, 0x1d6}, { {112,  8,  8}, 10, 0x3d1}, { {113,  8,  9}, 10, 0x3d5},
			{ {114,  8, 10}, 10, 0x3f2}, { {115,  8, 11}, 11, 0x7ee}, { {116,  8, 12}, 11, 0x7fb},
			{ {117,  9,  0}, 10, 0x3e9}, { {118,  9,  1},  9, 0x1cd}, { {119,  9,  2},  9, 0x1c8},
			{ {120,  9,  3},  9, 0x1cb}, { {121,  9,  4},  9, 0x1d1}, { {122,  9,  5},  9, 0x1d7},
			{ {123,  9,  6},  9, 0x1df}, { {124,  9,  7}, 10, 0x3cf}, { {125,  9,  8}, 10, 0x3e0},
			{ {126,  9,  9}, 10, 0x3ef}, { {127,  9, 10}, 11, 0x7e6}, { {128,  9, 11}, 11, 0x7f8},
			{ {129,  9, 12}, 12, 0xffa}, { {130, 10,  0}, 10, 0x3eb}, { {131, 10,  1},  9, 0x1dd},
			{ {132, 10,  2},  9, 0x1d3}, { {133, 10,  3},  9, 0x1d9}, { {134, 10,  4},  9, 0x1db},
			{ {135, 10,  5}, 10, 0x3d2}, { {136, 10,  6}, 10, 0x3cc}, { {137, 10,  7}, 10, 0x3dc},
			{ {138, 10,  8}, 10, 0x3ea}, { {139, 10,  9}, 11, 0x7ed}, { {140, 10, 10}, 11, 0x7f3},
			{ {141, 10, 11}, 11, 0x7f9}, { {142, 10, 12}, 12, 0xff9}, { {143, 11,  0}, 11, 0x7f2},
			{ {144, 11,  1}, 10, 0x3ce}, { {145, 11,  2},  9, 0x1e4}, { {146, 11,  3}, 10, 0x3cb},
			{ {147, 11,  4}, 10, 0x3d8}, { {148, 11,  5}, 10, 0x3d6}, { {149, 11,  6}, 10, 0x3e2},
			{ {150, 11,  7}, 10, 0x3e5}, { {151, 11,  8}, 11, 0x7e8}, { {152, 11,  9}, 11, 0x7f4},
			{ {153, 11, 10}, 11, 0x7f5}, { {154, 11, 11}, 11, 0x7f7}, { {155, 11, 12}, 12, 0xffb},
			{ {156, 12,  0}, 11, 0x7fa}, { {157, 12,  1}, 10, 0x3ec}, { {158, 12,  2}, 10, 0x3df},
			{ {159, 12,  3}, 10, 0x3e1}, { {160, 12,  4}, 10, 0x3e4}, { {161, 12,  5}, 10, 0x3e6},
			{ {162, 12,  6}, 10, 0x3f0}, { {163, 12,  7}, 11, 0x7e9}, { {164, 12,  8}, 11, 0x7ef},
			{ {165, 12,  9}, 12, 0xff8}, { {166, 12, 10}, 12, 0xffe}, { {167, 12, 11}, 12, 0xffc},
			{ {168, 12, 12}, 12, 0xfff},
		};

		const int shcb10_bst[337][2] = {
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{14, 0},                            // leaf node (V-index:14(V: 14) L:4 C:0X0)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:4 C:0X1)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:4 C:0X2)
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{28, 0},                            // leaf node (V-index:28(V: 28) L:5 C:0X6)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:5 C:0X7)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:5 C:0X8)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:5 C:0X9)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:5 C:0XA)
			{40, 0},                            // leaf node (V-index:40(V: 40) L:5 C:0XB)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:5 C:0XC)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:5 C:0XD)
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{26, 0},                            // leaf node (V-index:26(V: 26) L:6 C:0X1C)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:6 C:0X1D)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:6 C:0X1E)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:6 C:0X1F)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:6 C:0X20)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:6 C:0X21)
			{0, 0},                             // leaf node (V-index:0(V: 0) L:6 C:0X22)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:6 C:0X23)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:6 C:0X24)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:6 C:0X25)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:6 C:0X26)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:6 C:0X27)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:6 C:0X28)
			{67, 0},                            // leaf node (V-index:67(V: 67) L:6 C:0X29)
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{18, 0},                            // leaf node (V-index:18(V: 18) L:7 C:0X54)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:7 C:0X55)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:7 C:0X56)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:7 C:0X57)
			{69, 0},                            // leaf node (V-index:69(V: 69) L:7 C:0X58)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:7 C:0X59)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:7 C:0X5A)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:7 C:0X5B)
			{81, 0},                            // leaf node (V-index:81(V: 81) L:7 C:0X5C)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:7 C:0X5D)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:7 C:0X5E)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:7 C:0X5F)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:7 C:0X60)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:7 C:0X61)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:7 C:0X62)
			{82, 0},                            // leaf node (V-index:82(V: 82) L:7 C:0X63)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:7 C:0X64)
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{50, 51},
			{51, 52},
			{52, 53},
			{53, 54},
			{83, 0},                            // leaf node (V-index:83(V: 83) L:8 C:0XCA)
			{93, 0},                            // leaf node (V-index:93(V: 93) L:8 C:0XCB)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:8 C:0XCC)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:8 C:0XCD)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:8 C:0XCE)
			{106, 0},                           // leaf node (V-index:106(V: 106) L:8 C:0XCF)
			{94, 0},                            // leaf node (V-index:94(V: 94) L:8 C:0XD0)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:8 C:0XD1)
			{92, 0},                            // leaf node (V-index:92(V: 92) L:8 C:0XD2)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:8 C:0XD3)
			{105, 0},                           // leaf node (V-index:105(V: 105) L:8 C:0XD4)
			{20, 0},                            // leaf node (V-index:20(V: 20) L:8 C:0XD5)
			{107, 0},                           // leaf node (V-index:107(V: 107) L:8 C:0XD6)
			{95, 0},                            // leaf node (V-index:95(V: 95) L:8 C:0XD7)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:8 C:0XD8)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:8 C:0XD9)
			{84, 0},                            // leaf node (V-index:84(V: 84) L:8 C:0XDA)
			{96, 0},                            // leaf node (V-index:96(V: 96) L:8 C:0XDB)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:8 C:0XDC)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:8 C:0XDD)
			{108, 0},                           // leaf node (V-index:108(V: 108) L:8 C:0XDE)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:8 C:0XDF)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:8 C:0XE0)
			{109, 0},                           // leaf node (V-index:109(V: 109) L:8 C:0XE1)
			{73, 0},                            // leaf node (V-index:73(V: 73) L:8 C:0XE2)
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{50, 51},
			{51, 52},
			{52, 53},
			{53, 54},
			{54, 55},
			{55, 56},
			{56, 57},
			{57, 58},
			{97, 0},                            // leaf node (V-index:97(V: 97) L:9 C:0X1C6)
			{85, 0},                            // leaf node (V-index:85(V: 85) L:9 C:0X1C7)
			{119, 0},                           // leaf node (V-index:119(V: 119) L:9 C:0X1C8)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:9 C:0X1C9)
			{86, 0},                            // leaf node (V-index:86(V: 86) L:9 C:0X1CA)
			{120, 0},                           // leaf node (V-index:120(V: 120) L:9 C:0X1CB)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:9 C:0X1CC)
			{118, 0},                           // leaf node (V-index:118(V: 118) L:9 C:0X1CD)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:9 C:0X1CE)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1CF)
			{110, 0},                           // leaf node (V-index:110(V: 110) L:9 C:0X1D0)
			{121, 0},                           // leaf node (V-index:121(V: 121) L:9 C:0X1D1)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:9 C:0X1D2)
			{132, 0},                           // leaf node (V-index:132(V: 132) L:9 C:0X1D3)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:9 C:0X1D4)
			{98, 0},                            // leaf node (V-index:98(V: 98) L:9 C:0X1D5)
			{111, 0},                           // leaf node (V-index:111(V: 111) L:9 C:0X1D6)
			{122, 0},                           // leaf node (V-index:122(V: 122) L:9 C:0X1D7)
			{99, 0},                            // leaf node (V-index:99(V: 99) L:9 C:0X1D8)
			{133, 0},                           // leaf node (V-index:133(V: 133) L:9 C:0X1D9)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:9 C:0X1DA)
			{134, 0},                           // leaf node (V-index:134(V: 134) L:9 C:0X1DB)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:9 C:0X1DC)
			{131, 0},                           // leaf node (V-index:131(V: 131) L:9 C:0X1DD)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:9 C:0X1DE)
			{123, 0},                           // leaf node (V-index:123(V: 123) L:9 C:0X1DF)
			{87, 0},                            // leaf node (V-index:87(V: 87) L:9 C:0X1E0)
			{104, 0},                           // leaf node (V-index:104(V: 104) L:9 C:0X1E1)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:9 C:0X1E2)
			{91, 0},                            // leaf node (V-index:91(V: 91) L:9 C:0X1E3)
			{145, 0},                           // leaf node (V-index:145(V: 145) L:9 C:0X1E4)
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{50, 51},
			{51, 52},
			{52, 53},
			{53, 54},
			{100, 0},                           // leaf node (V-index:100(V: 100) L:10 C:0X3CA)
			{146, 0},                           // leaf node (V-index:146(V: 146) L:10 C:0X3CB)
			{136, 0},                           // leaf node (V-index:136(V: 136) L:10 C:0X3CC)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:10 C:0X3CD)
			{144, 0},                           // leaf node (V-index:144(V: 144) L:10 C:0X3CE)
			{124, 0},                           // leaf node (V-index:124(V: 124) L:10 C:0X3CF)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:10 C:0X3D0)
			{112, 0},                           // leaf node (V-index:112(V: 112) L:10 C:0X3D1)
			{135, 0},                           // leaf node (V-index:135(V: 135) L:10 C:0X3D2)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:10 C:0X3D3)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:10 C:0X3D4)
			{113, 0},                           // leaf node (V-index:113(V: 113) L:10 C:0X3D5)
			{148, 0},                           // leaf node (V-index:148(V: 148) L:10 C:0X3D6)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:10 C:0X3D7)
			{147, 0},                           // leaf node (V-index:147(V: 147) L:10 C:0X3D8)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:10 C:0X3D9)
			{101, 0},                           // leaf node (V-index:101(V: 101) L:10 C:0X3DA)
			{88, 0},                            // leaf node (V-index:88(V: 88) L:10 C:0X3DB)
			{137, 0},                           // leaf node (V-index:137(V: 137) L:10 C:0X3DC)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:10 C:0X3DD)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:10 C:0X3DE)
			{158, 0},                           // leaf node (V-index:158(V: 158) L:10 C:0X3DF)
			{125, 0},                           // leaf node (V-index:125(V: 125) L:10 C:0X3E0)
			{159, 0},                           // leaf node (V-index:159(V: 159) L:10 C:0X3E1)
			{149, 0},                           // leaf node (V-index:149(V: 149) L:10 C:0X3E2)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:10 C:0X3E3)
			{160, 0},                           // leaf node (V-index:160(V: 160) L:10 C:0X3E4)
			{150, 0},                           // leaf node (V-index:150(V: 150) L:10 C:0X3E5)
			{161, 0},                           // leaf node (V-index:161(V: 161) L:10 C:0X3E6)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:10 C:0X3E7)
			{89, 0},                            // leaf node (V-index:89(V: 89) L:10 C:0X3E8)
			{117, 0},                           // leaf node (V-index:117(V: 117) L:10 C:0X3E9)
			{138, 0},                           // leaf node (V-index:138(V: 138) L:10 C:0X3EA)
			{130, 0},                           // leaf node (V-index:130(V: 130) L:10 C:0X3EB)
			{157, 0},                           // leaf node (V-index:157(V: 157) L:10 C:0X3EC)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:10 C:0X3ED)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:10 C:0X3EE)
			{126, 0},                           // leaf node (V-index:126(V: 126) L:10 C:0X3EF)
			{162, 0},                           // leaf node (V-index:162(V: 162) L:10 C:0X3F0)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:10 C:0X3F1)
			{114, 0},                           // leaf node (V-index:114(V: 114) L:10 C:0X3F2)
			{13, 14},
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{127, 0},                           // leaf node (V-index:127(V: 127) L:11 C:0X7E6)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:11 C:0X7E7)
			{151, 0},                           // leaf node (V-index:151(V: 151) L:11 C:0X7E8)
			{163, 0},                           // leaf node (V-index:163(V: 163) L:11 C:0X7E9)
			{102, 0},                           // leaf node (V-index:102(V: 102) L:11 C:0X7EA)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:11 C:0X7EB)
			{90, 0},                            // leaf node (V-index:90(V: 90) L:11 C:0X7EC)
			{139, 0},                           // leaf node (V-index:139(V: 139) L:11 C:0X7ED)
			{115, 0},                           // leaf node (V-index:115(V: 115) L:11 C:0X7EE)
			{164, 0},                           // leaf node (V-index:164(V: 164) L:11 C:0X7EF)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:11 C:0X7F0)
			{103, 0},                           // leaf node (V-index:103(V: 103) L:11 C:0X7F1)
			{143, 0},                           // leaf node (V-index:143(V: 143) L:11 C:0X7F2)
			{140, 0},                           // leaf node (V-index:140(V: 140) L:11 C:0X7F3)
			{152, 0},                           // leaf node (V-index:152(V: 152) L:11 C:0X7F4)
			{153, 0},                           // leaf node (V-index:153(V: 153) L:11 C:0X7F5)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:11 C:0X7F6)
			{154, 0},                           // leaf node (V-index:154(V: 154) L:11 C:0X7F7)
			{128, 0},                           // leaf node (V-index:128(V: 128) L:11 C:0X7F8)
			{141, 0},                           // leaf node (V-index:141(V: 141) L:11 C:0X7F9)
			{156, 0},                           // leaf node (V-index:156(V: 156) L:11 C:0X7FA)
			{116, 0},                           // leaf node (V-index:116(V: 116) L:11 C:0X7FB)
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{165, 0},                           // leaf node (V-index:165(V: 165) L:12 C:0XFF8)
			{142, 0},                           // leaf node (V-index:142(V: 142) L:12 C:0XFF9)
			{129, 0},                           // leaf node (V-index:129(V: 129) L:12 C:0XFFA)
			{155, 0},                           // leaf node (V-index:155(V: 155) L:12 C:0XFFB)
			{167, 0},                           // leaf node (V-index:167(V: 167) L:12 C:0XFFC)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:12 C:0XFFD)
			{166, 0},                           // leaf node (V-index:166(V: 166) L:12 C:0XFFE)
			{168, 0},                           // leaf node (V-index:168(V: 168) L:12 C:0XFFF)
		};

		const Spectrum_Huffman_Codebook_Pair shcb11 = {
			{ {  0,  0,  0},  4, 0x0  }, { {  1,  0,  1},  5, 0x6  }, { {  2,  0,  2},  6, 0x19 },
			{ {  3,  0,  3},  7, 0x3d }, { {  4,  0,  4},  8, 0x9c }, { {  5,  0,  5},  8, 0xc6 },
			{ {  6,  0,  6},  9, 0x1a7}, { {  7,  0,  7}, 10, 0x390}, { {  8,  0,  8}, 10, 0x3c2},
			{ {  9,  0,  9}, 10, 0x3df}, { { 10,  0, 10}, 11, 0x7e6}, { { 11,  0, 11}, 11, 0x7f3},
			{ { 12,  0, 12}, 12, 0xffb}, { { 13,  0, 13}, 11, 0x7ec}, { { 14,  0, 14}, 12, 0xffa},
			{ { 15,  0, 15}, 12, 0xffe}, { { 16,  0, 16}, 10, 0x38e}, { { 17,  1,  0},  5, 0x5  },
			{ { 18,  1,  1},  4, 0x1  }, { { 19,  1,  2},  5, 0x8  }, { { 20,  1,  3},  6, 0x14 },
			{ { 21,  1,  4},  7, 0x37 }, { { 22,  1,  5},  7, 0x42 }, { { 23,  1,  6},  8, 0x92 },
			{ { 24,  1,  7},  8, 0xaf }, { { 25,  1,  8},  9, 0x191}, { { 26,  1,  9},  9, 0x1a5},
			{ { 27,  1, 10},  9, 0x1b5}, { { 28,  1, 11}, 10, 0x39e}, { { 29,  1, 12}, 10, 0x3c0},
			{ { 30,  1, 13}, 10, 0x3a2}, { { 31,  1, 14}, 10, 0x3cd}, { { 32,  1, 15}, 11, 0x7d6},
			{ { 33,  1, 16},  8, 0xae }, { { 34,  2,  0},  6, 0x17 }, { { 35,  2,  1},  5, 0x7  },
			{ { 36,  2,  2},  5, 0x9  }, { { 37,  2,  3},  6, 0x18 }, { { 38,  2,  4},  7, 0x39 },
			{ { 39,  2,  5},  7, 0x40 }, { { 40,  2,  6},  8, 0x8e }, { { 41,  2,  7},  8, 0xa3 },
			{ { 42,  2,  8},  8, 0xb8 }, { { 43,  2,  9},  9, 0x199}, { { 44,  2, 10},  9, 0x1ac},
			{ { 45,  2, 11},  9, 0x1c1}, { { 46,  2, 12}, 10, 0x3b1}, { { 47,  2, 13}, 10, 0x396},
			{ { 48,  2, 14}, 10, 0x3be}, { { 49,  2, 15}, 10, 0x3ca}, { { 50,  2, 16},  8, 0x9d },
			{ { 51,  3,  0},  7, 0x3c }, { { 52,  3,  1},  6, 0x15 }, { { 53,  3,  2},  6, 0x16 },
			{ { 54,  3,  3},  6, 0x1a }, { { 55,  3,  4},  7, 0x3b }, { { 56,  3,  5},  7, 0x44 },
			{ { 57,  3,  6},  8, 0x91 }, { { 58,  3,  7},  8, 0xa5 }, { { 59,  3,  8},  8, 0xbe },
			{ { 60,  3,  9},  9, 0x196}, { { 61,  3, 10},  9, 0x1ae}, { { 62,  3, 11},  9, 0x1b9},
			{ { 63,  3, 12}, 10, 0x3a1}, { { 64,  3, 13}, 10, 0x391}, { { 65,  3, 14}, 10, 0x3a5},
			{ { 66,  3, 15}, 10, 0x3d5}, { { 67,  3, 16},  8, 0x94 }, { { 68,  4,  0},  8, 0x9a },
			{ { 69,  4,  1},  7, 0x36 }, { { 70,  4,  2},  7, 0x38 }, { { 71,  4,  3},  7, 0x3a },
			{ { 72,  4,  4},  7, 0x41 }, { { 73,  4,  5},  8, 0x8c }, { { 74,  4,  6},  8, 0x9b },
			{ { 75,  4,  7},  8, 0xb0 }, { { 76,  4,  8},  8, 0xc3 }, { { 77,  4,  9},  9, 0x19e},
			{ { 78,  4, 10},  9, 0x1ab}, { { 79,  4, 11},  9, 0x1bc}, { { 80,  4, 12}, 10, 0x39f},
			{ { 81,  4, 13}, 10, 0x38f}, { { 82,  4, 14}, 10, 0x3a9}, { { 83,  4, 15}, 10, 0x3cf},
			{ { 84,  4, 16},  8, 0x93 }, { { 85,  5,  0},  8, 0xbf }, { { 86,  5,  1},  7, 0x3e },
			{ { 87,  5,  2},  7, 0x3f }, { { 88,  5,  3},  7, 0x43 }, { { 89,  5,  4},  7, 0x45 },
			{ { 90,  5,  5},  8, 0x9e }, { { 91,  5,  6},  8, 0xa7 }, { { 92,  5,  7},  8, 0xb9 },
			{ { 93,  5,  8},  9, 0x194}, { { 94,  5,  9},  9, 0x1a2}, { { 95,  5, 10},  9, 0x1ba},
			{ { 96,  5, 11},  9, 0x1c3}, { { 97,  5, 12}, 10, 0x3a6}, { { 98,  5, 13}, 10, 0x3a7},
			{ { 99,  5, 14}, 10, 0x3bb}, { {100,  5, 15}, 10, 0x3d4}, { {101,  5, 16},  8, 0x9f },
			{ {102,  6,  0},  9, 0x1a0}, { {103,  6,  1},  8, 0x8f }, { {104,  6,  2},  8, 0x8d },
			{ {105,  6,  3},  8, 0x90 }, { {106,  6,  4},  8, 0x98 }, { {107,  6,  5},  8, 0xa6 },
			{ {108,  6,  6},  8, 0xb6 }, { {109,  6,  7},  8, 0xc4 }, { {110,  6,  8},  9, 0x19f},
			{ {111,  6,  9},  9, 0x1af}, { {112,  6, 10},  9, 0x1bf}, { {113,  6, 11}, 10, 0x399},
			{ {114,  6, 12}, 10, 0x3bf}, { {115,  6, 13}, 10, 0x3b4}, { {116,  6, 14}, 10, 0x3c9},
			{ {117,  6, 15}, 10, 0x3e7}, { {118,  6, 16},  8, 0xa8 }, { {119,  7,  0},  9, 0x1b6},
			{ {120,  7,  1},  8, 0xab }, { {121,  7,  2},  8, 0xa4 }, { {122,  7,  3},  8, 0xaa },
			{ {123,  7,  4},  8, 0xb2 }, { {124,  7,  5},  8, 0xc2 }, { {125,  7,  6},  8, 0xc5 },
			{ {126,  7,  7},  9, 0x198}, { {127,  7,  8},  9, 0x1a4}, { {128,  7,  9},  9, 0x1b8},
			{ {129,  7, 10}, 10, 0x38c}, { {130,  7, 11}, 10, 0x3a4}, { {131,  7, 12}, 10, 0x3c4},
			{ {132,  7, 13}, 10, 0x3c6}, { {133,  7, 14}, 10, 0x3dd}, { {134,  7, 15}, 10, 0x3e8},
			{ {135,  7, 16},  8, 0xad }, { {136,  8,  0}, 10, 0x3af}, { {137,  8,  1},  9, 0x192},
			{ {138,  8,  2},  8, 0xbd }, { {139,  8,  3},  8, 0xbc }, { {140,  8,  4},  9, 0x18e},
			{ {141,  8,  5},  9, 0x197}, { {142,  8,  6},  9, 0x19a}, { {143,  8,  7},  9, 0x1a3},
			{ {144,  8,  8},  9, 0x1b1}, { {145,  8,  9}, 10, 0x38d}, { {146,  8, 10}, 10, 0x398},
			{ {147,  8, 11}, 10, 0x3b7}, { {148,  8, 12}, 10, 0x3d3}, { {149,  8, 13}, 10, 0x3d1},
			{ {150,  8, 14}, 10, 0x3db}, { {151,  8, 15}, 11, 0x7dd}, { {152,  8, 16},  8, 0xb4 },
			{ {153,  9,  0}, 10, 0x3de}, { {154,  9,  1},  9, 0x1a9}, { {155,  9,  2},  9, 0x19b},
			{ {156,  9,  3},  9, 0x19c}, { {157,  9,  4},  9, 0x1a1}, { {158,  9,  5},  9, 0x1aa},
			{ {159,  9,  6},  9, 0x1ad}, { {160,  9,  7},  9, 0x1b3}, { {161,  9,  8}, 10, 0x38b},
			{ {162,  9,  9}, 10, 0x3b2}, { {163,  9, 10}, 10, 0x3b8}, { {164,  9, 11}, 10, 0x3ce},
			{ {165,  9, 12}, 10, 0x3e1}, { {166,  9, 13}, 10, 0x3e0}, { {167,  9, 14}, 11, 0x7d2},
			{ {168,  9, 15}, 11, 0x7e5}, { {169,  9, 16},  8, 0xb7 }, { {170, 10,  0}, 11, 0x7e3},
			{ {171, 10,  1},  9, 0x1bb}, { {172, 10,  2},  9, 0x1a8}, { {173, 10,  3},  9, 0x1a6},
			{ {174, 10,  4},  9, 0x1b0}, { {175, 10,  5},  9, 0x1b2}, { {176, 10,  6},  9, 0x1b7},
			{ {177, 10,  7}, 10, 0x39b}, { {178, 10,  8}, 10, 0x39a}, { {179, 10,  9}, 10, 0x3ba},
			{ {180, 10, 10}, 10, 0x3b5}, { {181, 10, 11}, 10, 0x3d6}, { {182, 10, 12}, 11, 0x7d7},
			{ {183, 10, 13}, 10, 0x3e4}, { {184, 10, 14}, 11, 0x7d8}, { {185, 10, 15}, 11, 0x7ea},
			{ {186, 10, 16},  8, 0xba }, { {187, 11,  0}, 11, 0x7e8}, { {188, 11,  1}, 10, 0x3a0},
			{ {189, 11,  2},  9, 0x1bd}, { {190, 11,  3},  9, 0x1b4}, { {191, 11,  4}, 10, 0x38a},
			{ {192, 11,  5},  9, 0x1c4}, { {193, 11,  6}, 10, 0x392}, { {194, 11,  7}, 10, 0x3aa},
			{ {195, 11,  8}, 10, 0x3b0}, { {196, 11,  9}, 10, 0x3bc}, { {197, 11, 10}, 10, 0x3d7},
			{ {198, 11, 11}, 11, 0x7d4}, { {199, 11, 12}, 11, 0x7dc}, { {200, 11, 13}, 11, 0x7db},
			{ {201, 11, 14}, 11, 0x7d5}, { {202, 11, 15}, 11, 0x7f0}, { {203, 11, 16},  8, 0xc1 },
			{ {204, 12,  0}, 11, 0x7fb}, { {205, 12,  1}, 10, 0x3c8}, { {206, 12,  2}, 10, 0x3a3},
			{ {207, 12,  3}, 10, 0x395}, { {208, 12,  4}, 10, 0x39d}, { {209, 12,  5}, 10, 0x3ac},
			{ {210, 12,  6}, 10, 0x3ae}, { {211, 12,  7}, 10, 0x3c5}, { {212, 12,  8}, 10, 0x3d8},
			{ {213, 12,  9}, 10, 0x3e2}, { {214, 12, 10}, 10, 0x3e6}, { {215, 12, 11}, 11, 0x7e4},
			{ {216, 12, 12}, 11, 0x7e7}, { {217, 12, 13}, 11, 0x7e0}, { {218, 12, 14}, 11, 0x7e9},
			{ {219, 12, 15}, 11, 0x7f7}, { {220, 12, 16},  9, 0x190}, { {221, 13,  0}, 11, 0x7f2},
			{ {222, 13,  1}, 10, 0x393}, { {223, 13,  2},  9, 0x1be}, { {224, 13,  3},  9, 0x1c0},
			{ {225, 13,  4}, 10, 0x394}, { {226, 13,  5}, 10, 0x397}, { {227, 13,  6}, 10, 0x3ad},
			{ {228, 13,  7}, 10, 0x3c3}, { {229, 13,  8}, 10, 0x3c1}, { {230, 13,  9}, 10, 0x3d2},
			{ {231, 13, 10}, 11, 0x7da}, { {232, 13, 11}, 11, 0x7d9}, { {233, 13, 12}, 11, 0x7df},
			{ {234, 13, 13}, 11, 0x7eb}, { {235, 13, 14}, 11, 0x7f4}, { {236, 13, 15}, 11, 0x7fa},
			{ {237, 13, 16},  9, 0x195}, { {238, 14,  0}, 11, 0x7f8}, { {239, 14,  1}, 10, 0x3bd},
			{ {240, 14,  2}, 10, 0x39c}, { {241, 14,  3}, 10, 0x3ab}, { {242, 14,  4}, 10, 0x3a8},
			{ {243, 14,  5}, 10, 0x3b3}, { {244, 14,  6}, 10, 0x3b9}, { {245, 14,  7}, 10, 0x3d0},
			{ {246, 14,  8}, 10, 0x3e3}, { {247, 14,  9}, 10, 0x3e5}, { {248, 14, 10}, 11, 0x7e2},
			{ {249, 14, 11}, 11, 0x7de}, { {250, 14, 12}, 11, 0x7ed}, { {251, 14, 13}, 11, 0x7f1},
			{ {252, 14, 14}, 11, 0x7f9}, { {253, 14, 15}, 11, 0x7fc}, { {254, 14, 16},  9, 0x193},
			{ {255, 15,  0}, 12, 0xffd}, { {256, 15,  1}, 10, 0x3dc}, { {257, 15,  2}, 10, 0x3b6},
			{ {258, 15,  3}, 10, 0x3c7}, { {259, 15,  4}, 10, 0x3cc}, { {260, 15,  5}, 10, 0x3cb},
			{ {261, 15,  6}, 10, 0x3d9}, { {262, 15,  7}, 10, 0x3da}, { {263, 15,  8}, 11, 0x7d3},
			{ {264, 15,  9}, 11, 0x7e1}, { {265, 15, 10}, 11, 0x7ee}, { {266, 15, 11}, 11, 0x7ef},
			{ {267, 15, 12}, 11, 0x7f5}, { {268, 15, 13}, 11, 0x7f6}, { {269, 15, 14}, 12, 0xffc},
			{ {270, 15, 15}, 12, 0xfff}, { {271, 15, 16},  9, 0x19d}, { {272, 16,  0},  9, 0x1c2},
			{ {273, 16,  1},  8, 0xb5 }, { {274, 16,  2},  8, 0xa1 }, { {275, 16,  3},  8, 0x96 },
			{ {276, 16,  4},  8, 0x97 }, { {277, 16,  5},  8, 0x95 }, { {278, 16,  6},  8, 0x99 },
			{ {279, 16,  7},  8, 0xa0 }, { {280, 16,  8},  8, 0xa2 }, { {281, 16,  9},  8, 0xac },
			{ {282, 16, 10},  8, 0xa9 }, { {283, 16, 11},  8, 0xb1 }, { {284, 16, 12},  8, 0xb3 },
			{ {285, 16, 13},  8, 0xbb }, { {286, 16, 14},  8, 0xc0 }, { {287, 16, 15},  9, 0x18f},
			{ {288, 16, 16},  5, 0x4  },
		};

		const int shcb11_bst[577][2] = {
			{1, 2},
			{2, 3},
			{3, 4},
			{4, 5},
			{5, 6},
			{6, 7},
			{7, 8},
			{8, 9},
			{9, 10},
			{10, 11},
			{11, 12},
			{12, 13},
			{13, 14},
			{14, 15},
			{15, 16},
			{0, 0},                             // leaf node (V-index:0(V: 0) L:4 C:0X0)
			{18, 0},                            // leaf node (V-index:18(V: 18) L:4 C:0X1)
			{14, 15},
			{15, 16},
			{16, 17},
			{17, 18},
			{18, 19},
			{19, 20},
			{20, 21},
			{21, 22},
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{288, 0},                           // leaf node (V-index:288(V: 288) L:5 C:0X4)
			{17, 0},                            // leaf node (V-index:17(V: 17) L:5 C:0X5)
			{1, 0},                             // leaf node (V-index:1(V: 1) L:5 C:0X6)
			{35, 0},                            // leaf node (V-index:35(V: 35) L:5 C:0X7)
			{19, 0},                            // leaf node (V-index:19(V: 19) L:5 C:0X8)
			{36, 0},                            // leaf node (V-index:36(V: 36) L:5 C:0X9)
			{22, 23},
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{20, 0},                            // leaf node (V-index:20(V: 20) L:6 C:0X14)
			{52, 0},                            // leaf node (V-index:52(V: 52) L:6 C:0X15)
			{53, 0},                            // leaf node (V-index:53(V: 53) L:6 C:0X16)
			{34, 0},                            // leaf node (V-index:34(V: 34) L:6 C:0X17)
			{37, 0},                            // leaf node (V-index:37(V: 37) L:6 C:0X18)
			{2, 0},                             // leaf node (V-index:2(V: 2) L:6 C:0X19)
			{54, 0},                            // leaf node (V-index:54(V: 54) L:6 C:0X1A)
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{46, 47},
			{47, 48},
			{48, 49},
			{49, 50},
			{50, 51},
			{51, 52},
			{52, 53},
			{53, 54},
			{54, 55},
			{55, 56},
			{56, 57},
			{57, 58},
			{58, 59},
			{59, 60},
			{60, 61},
			{61, 62},
			{62, 63},
			{63, 64},
			{64, 65},
			{65, 66},
			{66, 67},
			{67, 68},
			{68, 69},
			{69, 70},
			{70, 71},
			{71, 72},
			{72, 73},
			{73, 74},
			{69, 0},                            // leaf node (V-index:69(V: 69) L:7 C:0X36)
			{21, 0},                            // leaf node (V-index:21(V: 21) L:7 C:0X37)
			{70, 0},                            // leaf node (V-index:70(V: 70) L:7 C:0X38)
			{38, 0},                            // leaf node (V-index:38(V: 38) L:7 C:0X39)
			{71, 0},                            // leaf node (V-index:71(V: 71) L:7 C:0X3A)
			{55, 0},                            // leaf node (V-index:55(V: 55) L:7 C:0X3B)
			{51, 0},                            // leaf node (V-index:51(V: 51) L:7 C:0X3C)
			{3, 0},                             // leaf node (V-index:3(V: 3) L:7 C:0X3D)
			{86, 0},                            // leaf node (V-index:86(V: 86) L:7 C:0X3E)
			{87, 0},                            // leaf node (V-index:87(V: 87) L:7 C:0X3F)
			{39, 0},                            // leaf node (V-index:39(V: 39) L:7 C:0X40)
			{72, 0},                            // leaf node (V-index:72(V: 72) L:7 C:0X41)
			{22, 0},                            // leaf node (V-index:22(V: 22) L:7 C:0X42)
			{88, 0},                            // leaf node (V-index:88(V: 88) L:7 C:0X43)
			{56, 0},                            // leaf node (V-index:56(V: 56) L:7 C:0X44)
			{89, 0},                            // leaf node (V-index:89(V: 89) L:7 C:0X45)
			{58, 59},
			{59, 60},
			{60, 61},
			{61, 62},
			{62, 63},
			{63, 64},
			{64, 65},
			{65, 66},
			{66, 67},
			{67, 68},
			{68, 69},
			{69, 70},
			{70, 71},
			{71, 72},
			{72, 73},
			{73, 74},
			{74, 75},
			{75, 76},
			{76, 77},
			{77, 78},
			{78, 79},
			{79, 80},
			{80, 81},
			{81, 82},
			{82, 83},
			{83, 84},
			{84, 85},
			{85, 86},
			{86, 87},
			{87, 88},
			{88, 89},
			{89, 90},
			{90, 91},
			{91, 92},
			{92, 93},
			{93, 94},
			{94, 95},
			{95, 96},
			{96, 97},
			{97, 98},
			{98, 99},
			{99, 100},
			{100, 101},
			{101, 102},
			{102, 103},
			{103, 104},
			{104, 105},
			{105, 106},
			{106, 107},
			{107, 108},
			{108, 109},
			{109, 110},
			{110, 111},
			{111, 112},
			{112, 113},
			{113, 114},
			{114, 115},
			{115, 116},
			{73, 0},                            // leaf node (V-index:73(V: 73) L:8 C:0X8C)
			{104, 0},                           // leaf node (V-index:104(V: 104) L:8 C:0X8D)
			{40, 0},                            // leaf node (V-index:40(V: 40) L:8 C:0X8E)
			{103, 0},                           // leaf node (V-index:103(V: 103) L:8 C:0X8F)
			{105, 0},                           // leaf node (V-index:105(V: 105) L:8 C:0X90)
			{57, 0},                            // leaf node (V-index:57(V: 57) L:8 C:0X91)
			{23, 0},                            // leaf node (V-index:23(V: 23) L:8 C:0X92)
			{84, 0},                            // leaf node (V-index:84(V: 84) L:8 C:0X93)
			{67, 0},                            // leaf node (V-index:67(V: 67) L:8 C:0X94)
			{277, 0},                           // leaf node (V-index:277(V: 277) L:8 C:0X95)
			{275, 0},                           // leaf node (V-index:275(V: 275) L:8 C:0X96)
			{276, 0},                           // leaf node (V-index:276(V: 276) L:8 C:0X97)
			{106, 0},                           // leaf node (V-index:106(V: 106) L:8 C:0X98)
			{278, 0},                           // leaf node (V-index:278(V: 278) L:8 C:0X99)
			{68, 0},                            // leaf node (V-index:68(V: 68) L:8 C:0X9A)
			{74, 0},                            // leaf node (V-index:74(V: 74) L:8 C:0X9B)
			{4, 0},                             // leaf node (V-index:4(V: 4) L:8 C:0X9C)
			{50, 0},                            // leaf node (V-index:50(V: 50) L:8 C:0X9D)
			{90, 0},                            // leaf node (V-index:90(V: 90) L:8 C:0X9E)
			{101, 0},                           // leaf node (V-index:101(V: 101) L:8 C:0X9F)
			{279, 0},                           // leaf node (V-index:279(V: 279) L:8 C:0XA0)
			{274, 0},                           // leaf node (V-index:274(V: 274) L:8 C:0XA1)
			{280, 0},                           // leaf node (V-index:280(V: 280) L:8 C:0XA2)
			{41, 0},                            // leaf node (V-index:41(V: 41) L:8 C:0XA3)
			{121, 0},                           // leaf node (V-index:121(V: 121) L:8 C:0XA4)
			{58, 0},                            // leaf node (V-index:58(V: 58) L:8 C:0XA5)
			{107, 0},                           // leaf node (V-index:107(V: 107) L:8 C:0XA6)
			{91, 0},                            // leaf node (V-index:91(V: 91) L:8 C:0XA7)
			{118, 0},                           // leaf node (V-index:118(V: 118) L:8 C:0XA8)
			{282, 0},                           // leaf node (V-index:282(V: 282) L:8 C:0XA9)
			{122, 0},                           // leaf node (V-index:122(V: 122) L:8 C:0XAA)
			{120, 0},                           // leaf node (V-index:120(V: 120) L:8 C:0XAB)
			{281, 0},                           // leaf node (V-index:281(V: 281) L:8 C:0XAC)
			{135, 0},                           // leaf node (V-index:135(V: 135) L:8 C:0XAD)
			{33, 0},                            // leaf node (V-index:33(V: 33) L:8 C:0XAE)
			{24, 0},                            // leaf node (V-index:24(V: 24) L:8 C:0XAF)
			{75, 0},                            // leaf node (V-index:75(V: 75) L:8 C:0XB0)
			{283, 0},                           // leaf node (V-index:283(V: 283) L:8 C:0XB1)
			{123, 0},                           // leaf node (V-index:123(V: 123) L:8 C:0XB2)
			{284, 0},                           // leaf node (V-index:284(V: 284) L:8 C:0XB3)
			{152, 0},                           // leaf node (V-index:152(V: 152) L:8 C:0XB4)
			{273, 0},                           // leaf node (V-index:273(V: 273) L:8 C:0XB5)
			{108, 0},                           // leaf node (V-index:108(V: 108) L:8 C:0XB6)
			{169, 0},                           // leaf node (V-index:169(V: 169) L:8 C:0XB7)
			{42, 0},                            // leaf node (V-index:42(V: 42) L:8 C:0XB8)
			{92, 0},                            // leaf node (V-index:92(V: 92) L:8 C:0XB9)
			{186, 0},                           // leaf node (V-index:186(V: 186) L:8 C:0XBA)
			{285, 0},                           // leaf node (V-index:285(V: 285) L:8 C:0XBB)
			{139, 0},                           // leaf node (V-index:139(V: 139) L:8 C:0XBC)
			{138, 0},                           // leaf node (V-index:138(V: 138) L:8 C:0XBD)
			{59, 0},                            // leaf node (V-index:59(V: 59) L:8 C:0XBE)
			{85, 0},                            // leaf node (V-index:85(V: 85) L:8 C:0XBF)
			{286, 0},                           // leaf node (V-index:286(V: 286) L:8 C:0XC0)
			{203, 0},                           // leaf node (V-index:203(V: 203) L:8 C:0XC1)
			{124, 0},                           // leaf node (V-index:124(V: 124) L:8 C:0XC2)
			{76, 0},                            // leaf node (V-index:76(V: 76) L:8 C:0XC3)
			{109, 0},                           // leaf node (V-index:109(V: 109) L:8 C:0XC4)
			{125, 0},                           // leaf node (V-index:125(V: 125) L:8 C:0XC5)
			{5, 0},                             // leaf node (V-index:5(V: 5) L:8 C:0XC6)
			{57, 58},
			{58, 59},
			{59, 60},
			{60, 61},
			{61, 62},
			{62, 63},
			{63, 64},
			{64, 65},
			{65, 66},
			{66, 67},
			{67, 68},
			{68, 69},
			{69, 70},
			{70, 71},
			{71, 72},
			{72, 73},
			{73, 74},
			{74, 75},
			{75, 76},
			{76, 77},
			{77, 78},
			{78, 79},
			{79, 80},
			{80, 81},
			{81, 82},
			{82, 83},
			{83, 84},
			{84, 85},
			{85, 86},
			{86, 87},
			{87, 88},
			{88, 89},
			{89, 90},
			{90, 91},
			{91, 92},
			{92, 93},
			{93, 94},
			{94, 95},
			{95, 96},
			{96, 97},
			{97, 98},
			{98, 99},
			{99, 100},
			{100, 101},
			{101, 102},
			{102, 103},
			{103, 104},
			{104, 105},
			{105, 106},
			{106, 107},
			{107, 108},
			{108, 109},
			{109, 110},
			{110, 111},
			{111, 112},
			{112, 113},
			{113, 114},
			{140, 0},                           // leaf node (V-index:140(V: 140) L:9 C:0X18E)
			{287, 0},                           // leaf node (V-index:287(V: 287) L:9 C:0X18F)
			{220, 0},                           // leaf node (V-index:220(V: 220) L:9 C:0X190)
			{25, 0},                            // leaf node (V-index:25(V: 25) L:9 C:0X191)
			{137, 0},                           // leaf node (V-index:137(V: 137) L:9 C:0X192)
			{254, 0},                           // leaf node (V-index:254(V: 254) L:9 C:0X193)
			{93, 0},                            // leaf node (V-index:93(V: 93) L:9 C:0X194)
			{237, 0},                           // leaf node (V-index:237(V: 237) L:9 C:0X195)
			{60, 0},                            // leaf node (V-index:60(V: 60) L:9 C:0X196)
			{141, 0},                           // leaf node (V-index:141(V: 141) L:9 C:0X197)
			{126, 0},                           // leaf node (V-index:126(V: 126) L:9 C:0X198)
			{43, 0},                            // leaf node (V-index:43(V: 43) L:9 C:0X199)
			{142, 0},                           // leaf node (V-index:142(V: 142) L:9 C:0X19A)
			{155, 0},                           // leaf node (V-index:155(V: 155) L:9 C:0X19B)
			{156, 0},                           // leaf node (V-index:156(V: 156) L:9 C:0X19C)
			{271, 0},                           // leaf node (V-index:271(V: 271) L:9 C:0X19D)
			{77, 0},                            // leaf node (V-index:77(V: 77) L:9 C:0X19E)
			{110, 0},                           // leaf node (V-index:110(V: 110) L:9 C:0X19F)
			{102, 0},                           // leaf node (V-index:102(V: 102) L:9 C:0X1A0)
			{157, 0},                           // leaf node (V-index:157(V: 157) L:9 C:0X1A1)
			{94, 0},                            // leaf node (V-index:94(V: 94) L:9 C:0X1A2)
			{143, 0},                           // leaf node (V-index:143(V: 143) L:9 C:0X1A3)
			{127, 0},                           // leaf node (V-index:127(V: 127) L:9 C:0X1A4)
			{26, 0},                            // leaf node (V-index:26(V: 26) L:9 C:0X1A5)
			{173, 0},                           // leaf node (V-index:173(V: 173) L:9 C:0X1A6)
			{6, 0},                             // leaf node (V-index:6(V: 6) L:9 C:0X1A7)
			{172, 0},                           // leaf node (V-index:172(V: 172) L:9 C:0X1A8)
			{154, 0},                           // leaf node (V-index:154(V: 154) L:9 C:0X1A9)
			{158, 0},                           // leaf node (V-index:158(V: 158) L:9 C:0X1AA)
			{78, 0},                            // leaf node (V-index:78(V: 78) L:9 C:0X1AB)
			{44, 0},                            // leaf node (V-index:44(V: 44) L:9 C:0X1AC)
			{159, 0},                           // leaf node (V-index:159(V: 159) L:9 C:0X1AD)
			{61, 0},                            // leaf node (V-index:61(V: 61) L:9 C:0X1AE)
			{111, 0},                           // leaf node (V-index:111(V: 111) L:9 C:0X1AF)
			{174, 0},                           // leaf node (V-index:174(V: 174) L:9 C:0X1B0)
			{144, 0},                           // leaf node (V-index:144(V: 144) L:9 C:0X1B1)
			{175, 0},                           // leaf node (V-index:175(V: 175) L:9 C:0X1B2)
			{160, 0},                           // leaf node (V-index:160(V: 160) L:9 C:0X1B3)
			{190, 0},                           // leaf node (V-index:190(V: 190) L:9 C:0X1B4)
			{27, 0},                            // leaf node (V-index:27(V: 27) L:9 C:0X1B5)
			{119, 0},                           // leaf node (V-index:119(V: 119) L:9 C:0X1B6)
			{176, 0},                           // leaf node (V-index:176(V: 176) L:9 C:0X1B7)
			{128, 0},                           // leaf node (V-index:128(V: 128) L:9 C:0X1B8)
			{62, 0},                            // leaf node (V-index:62(V: 62) L:9 C:0X1B9)
			{95, 0},                            // leaf node (V-index:95(V: 95) L:9 C:0X1BA)
			{171, 0},                           // leaf node (V-index:171(V: 171) L:9 C:0X1BB)
			{79, 0},                            // leaf node (V-index:79(V: 79) L:9 C:0X1BC)
			{189, 0},                           // leaf node (V-index:189(V: 189) L:9 C:0X1BD)
			{223, 0},                           // leaf node (V-index:223(V: 223) L:9 C:0X1BE)
			{112, 0},                           // leaf node (V-index:112(V: 112) L:9 C:0X1BF)
			{224, 0},                           // leaf node (V-index:224(V: 224) L:9 C:0X1C0)
			{45, 0},                            // leaf node (V-index:45(V: 45) L:9 C:0X1C1)
			{272, 0},                           // leaf node (V-index:272(V: 272) L:9 C:0X1C2)
			{96, 0},                            // leaf node (V-index:96(V: 96) L:9 C:0X1C3)
			{192, 0},                           // leaf node (V-index:192(V: 192) L:9 C:0X1C4)
			{59, 60},
			{60, 61},
			{61, 62},
			{62, 63},
			{63, 64},
			{64, 65},
			{65, 66},
			{66, 67},
			{67, 68},
			{68, 69},
			{69, 70},
			{70, 71},
			{71, 72},
			{72, 73},
			{73, 74},
			{74, 75},
			{75, 76},
			{76, 77},
			{77, 78},
			{78, 79},
			{79, 80},
			{80, 81},
			{81, 82},
			{82, 83},
			{83, 84},
			{84, 85},
			{85, 86},
			{86, 87},
			{87, 88},
			{88, 89},
			{89, 90},
			{90, 91},
			{91, 92},
			{92, 93},
			{93, 94},
			{94, 95},
			{95, 96},
			{96, 97},
			{97, 98},
			{98, 99},
			{99, 100},
			{100, 101},
			{101, 102},
			{102, 103},
			{103, 104},
			{104, 105},
			{105, 106},
			{106, 107},
			{107, 108},
			{108, 109},
			{109, 110},
			{110, 111},
			{111, 112},
			{112, 113},
			{113, 114},
			{114, 115},
			{115, 116},
			{116, 117},
			{117, 118},
			{191, 0},                           // leaf node (V-index:191(V: 191) L:10 C:0X38A)
			{161, 0},                           // leaf node (V-index:161(V: 161) L:10 C:0X38B)
			{129, 0},                           // leaf node (V-index:129(V: 129) L:10 C:0X38C)
			{145, 0},                           // leaf node (V-index:145(V: 145) L:10 C:0X38D)
			{16, 0},                            // leaf node (V-index:16(V: 16) L:10 C:0X38E)
			{81, 0},                            // leaf node (V-index:81(V: 81) L:10 C:0X38F)
			{7, 0},                             // leaf node (V-index:7(V: 7) L:10 C:0X390)
			{64, 0},                            // leaf node (V-index:64(V: 64) L:10 C:0X391)
			{193, 0},                           // leaf node (V-index:193(V: 193) L:10 C:0X392)
			{222, 0},                           // leaf node (V-index:222(V: 222) L:10 C:0X393)
			{225, 0},                           // leaf node (V-index:225(V: 225) L:10 C:0X394)
			{207, 0},                           // leaf node (V-index:207(V: 207) L:10 C:0X395)
			{47, 0},                            // leaf node (V-index:47(V: 47) L:10 C:0X396)
			{226, 0},                           // leaf node (V-index:226(V: 226) L:10 C:0X397)
			{146, 0},                           // leaf node (V-index:146(V: 146) L:10 C:0X398)
			{113, 0},                           // leaf node (V-index:113(V: 113) L:10 C:0X399)
			{178, 0},                           // leaf node (V-index:178(V: 178) L:10 C:0X39A)
			{177, 0},                           // leaf node (V-index:177(V: 177) L:10 C:0X39B)
			{240, 0},                           // leaf node (V-index:240(V: 240) L:10 C:0X39C)
			{208, 0},                           // leaf node (V-index:208(V: 208) L:10 C:0X39D)
			{28, 0},                            // leaf node (V-index:28(V: 28) L:10 C:0X39E)
			{80, 0},                            // leaf node (V-index:80(V: 80) L:10 C:0X39F)
			{188, 0},                           // leaf node (V-index:188(V: 188) L:10 C:0X3A0)
			{63, 0},                            // leaf node (V-index:63(V: 63) L:10 C:0X3A1)
			{30, 0},                            // leaf node (V-index:30(V: 30) L:10 C:0X3A2)
			{206, 0},                           // leaf node (V-index:206(V: 206) L:10 C:0X3A3)
			{130, 0},                           // leaf node (V-index:130(V: 130) L:10 C:0X3A4)
			{65, 0},                            // leaf node (V-index:65(V: 65) L:10 C:0X3A5)
			{97, 0},                            // leaf node (V-index:97(V: 97) L:10 C:0X3A6)
			{98, 0},                            // leaf node (V-index:98(V: 98) L:10 C:0X3A7)
			{242, 0},                           // leaf node (V-index:242(V: 242) L:10 C:0X3A8)
			{82, 0},                            // leaf node (V-index:82(V: 82) L:10 C:0X3A9)
			{194, 0},                           // leaf node (V-index:194(V: 194) L:10 C:0X3AA)
			{241, 0},                           // leaf node (V-index:241(V: 241) L:10 C:0X3AB)
			{209, 0},                           // leaf node (V-index:209(V: 209) L:10 C:0X3AC)
			{227, 0},                           // leaf node (V-index:227(V: 227) L:10 C:0X3AD)
			{210, 0},                           // leaf node (V-index:210(V: 210) L:10 C:0X3AE)
			{136, 0},                           // leaf node (V-index:136(V: 136) L:10 C:0X3AF)
			{195, 0},                           // leaf node (V-index:195(V: 195) L:10 C:0X3B0)
			{46, 0},                            // leaf node (V-index:46(V: 46) L:10 C:0X3B1)
			{162, 0},                           // leaf node (V-index:162(V: 162) L:10 C:0X3B2)
			{243, 0},                           // leaf node (V-index:243(V: 243) L:10 C:0X3B3)
			{115, 0},                           // leaf node (V-index:115(V: 115) L:10 C:0X3B4)
			{180, 0},                           // leaf node (V-index:180(V: 180) L:10 C:0X3B5)
			{257, 0},                           // leaf node (V-index:257(V: 257) L:10 C:0X3B6)
			{147, 0},                           // leaf node (V-index:147(V: 147) L:10 C:0X3B7)
			{163, 0},                           // leaf node (V-index:163(V: 163) L:10 C:0X3B8)
			{244, 0},                           // leaf node (V-index:244(V: 244) L:10 C:0X3B9)
			{179, 0},                           // leaf node (V-index:179(V: 179) L:10 C:0X3BA)
			{99, 0},                            // leaf node (V-index:99(V: 99) L:10 C:0X3BB)
			{196, 0},                           // leaf node (V-index:196(V: 196) L:10 C:0X3BC)
			{239, 0},                           // leaf node (V-index:239(V: 239) L:10 C:0X3BD)
			{48, 0},                            // leaf node (V-index:48(V: 48) L:10 C:0X3BE)
			{114, 0},                           // leaf node (V-index:114(V: 114) L:10 C:0X3BF)
			{29, 0},                            // leaf node (V-index:29(V: 29) L:10 C:0X3C0)
			{229, 0},                           // leaf node (V-index:229(V: 229) L:10 C:0X3C1)
			{8, 0},                             // leaf node (V-index:8(V: 8) L:10 C:0X3C2)
			{228, 0},                           // leaf node (V-index:228(V: 228) L:10 C:0X3C3)
			{131, 0},                           // leaf node (V-index:131(V: 131) L:10 C:0X3C4)
			{211, 0},                           // leaf node (V-index:211(V: 211) L:10 C:0X3C5)
			{132, 0},                           // leaf node (V-index:132(V: 132) L:10 C:0X3C6)
			{258, 0},                           // leaf node (V-index:258(V: 258) L:10 C:0X3C7)
			{205, 0},                           // leaf node (V-index:205(V: 205) L:10 C:0X3C8)
			{116, 0},                           // leaf node (V-index:116(V: 116) L:10 C:0X3C9)
			{49, 0},                            // leaf node (V-index:49(V: 49) L:10 C:0X3CA)
			{260, 0},                           // leaf node (V-index:260(V: 260) L:10 C:0X3CB)
			{259, 0},                           // leaf node (V-index:259(V: 259) L:10 C:0X3CC)
			{31, 0},                            // leaf node (V-index:31(V: 31) L:10 C:0X3CD)
			{164, 0},                           // leaf node (V-index:164(V: 164) L:10 C:0X3CE)
			{83, 0},                            // leaf node (V-index:83(V: 83) L:10 C:0X3CF)
			{245, 0},                           // leaf node (V-index:245(V: 245) L:10 C:0X3D0)
			{149, 0},                           // leaf node (V-index:149(V: 149) L:10 C:0X3D1)
			{230, 0},                           // leaf node (V-index:230(V: 230) L:10 C:0X3D2)
			{148, 0},                           // leaf node (V-index:148(V: 148) L:10 C:0X3D3)
			{100, 0},                           // leaf node (V-index:100(V: 100) L:10 C:0X3D4)
			{66, 0},                            // leaf node (V-index:66(V: 66) L:10 C:0X3D5)
			{181, 0},                           // leaf node (V-index:181(V: 181) L:10 C:0X3D6)
			{197, 0},                           // leaf node (V-index:197(V: 197) L:10 C:0X3D7)
			{212, 0},                           // leaf node (V-index:212(V: 212) L:10 C:0X3D8)
			{261, 0},                           // leaf node (V-index:261(V: 261) L:10 C:0X3D9)
			{262, 0},                           // leaf node (V-index:262(V: 262) L:10 C:0X3DA)
			{150, 0},                           // leaf node (V-index:150(V: 150) L:10 C:0X3DB)
			{256, 0},                           // leaf node (V-index:256(V: 256) L:10 C:0X3DC)
			{133, 0},                           // leaf node (V-index:133(V: 133) L:10 C:0X3DD)
			{153, 0},                           // leaf node (V-index:153(V: 153) L:10 C:0X3DE)
			{9, 0},                             // leaf node (V-index:9(V: 9) L:10 C:0X3DF)
			{166, 0},                           // leaf node (V-index:166(V: 166) L:10 C:0X3E0)
			{165, 0},                           // leaf node (V-index:165(V: 165) L:10 C:0X3E1)
			{213, 0},                           // leaf node (V-index:213(V: 213) L:10 C:0X3E2)
			{246, 0},                           // leaf node (V-index:246(V: 246) L:10 C:0X3E3)
			{183, 0},                           // leaf node (V-index:183(V: 183) L:10 C:0X3E4)
			{247, 0},                           // leaf node (V-index:247(V: 247) L:10 C:0X3E5)
			{214, 0},                           // leaf node (V-index:214(V: 214) L:10 C:0X3E6)
			{117, 0},                           // leaf node (V-index:117(V: 117) L:10 C:0X3E7)
			{134, 0},                           // leaf node (V-index:134(V: 134) L:10 C:0X3E8)
			{23, 24},
			{24, 25},
			{25, 26},
			{26, 27},
			{27, 28},
			{28, 29},
			{29, 30},
			{30, 31},
			{31, 32},
			{32, 33},
			{33, 34},
			{34, 35},
			{35, 36},
			{36, 37},
			{37, 38},
			{38, 39},
			{39, 40},
			{40, 41},
			{41, 42},
			{42, 43},
			{43, 44},
			{44, 45},
			{45, 46},
			{167, 0},                           // leaf node (V-index:167(V: 167) L:11 C:0X7D2)
			{263, 0},                           // leaf node (V-index:263(V: 263) L:11 C:0X7D3)
			{198, 0},                           // leaf node (V-index:198(V: 198) L:11 C:0X7D4)
			{201, 0},                           // leaf node (V-index:201(V: 201) L:11 C:0X7D5)
			{32, 0},                            // leaf node (V-index:32(V: 32) L:11 C:0X7D6)
			{182, 0},                           // leaf node (V-index:182(V: 182) L:11 C:0X7D7)
			{184, 0},                           // leaf node (V-index:184(V: 184) L:11 C:0X7D8)
			{232, 0},                           // leaf node (V-index:232(V: 232) L:11 C:0X7D9)
			{231, 0},                           // leaf node (V-index:231(V: 231) L:11 C:0X7DA)
			{200, 0},                           // leaf node (V-index:200(V: 200) L:11 C:0X7DB)
			{199, 0},                           // leaf node (V-index:199(V: 199) L:11 C:0X7DC)
			{151, 0},                           // leaf node (V-index:151(V: 151) L:11 C:0X7DD)
			{249, 0},                           // leaf node (V-index:249(V: 249) L:11 C:0X7DE)
			{233, 0},                           // leaf node (V-index:233(V: 233) L:11 C:0X7DF)
			{217, 0},                           // leaf node (V-index:217(V: 217) L:11 C:0X7E0)
			{264, 0},                           // leaf node (V-index:264(V: 264) L:11 C:0X7E1)
			{248, 0},                           // leaf node (V-index:248(V: 248) L:11 C:0X7E2)
			{170, 0},                           // leaf node (V-index:170(V: 170) L:11 C:0X7E3)
			{215, 0},                           // leaf node (V-index:215(V: 215) L:11 C:0X7E4)
			{168, 0},                           // leaf node (V-index:168(V: 168) L:11 C:0X7E5)
			{10, 0},                            // leaf node (V-index:10(V: 10) L:11 C:0X7E6)
			{216, 0},                           // leaf node (V-index:216(V: 216) L:11 C:0X7E7)
			{187, 0},                           // leaf node (V-index:187(V: 187) L:11 C:0X7E8)
			{218, 0},                           // leaf node (V-index:218(V: 218) L:11 C:0X7E9)
			{185, 0},                           // leaf node (V-index:185(V: 185) L:11 C:0X7EA)
			{234, 0},                           // leaf node (V-index:234(V: 234) L:11 C:0X7EB)
			{13, 0},                            // leaf node (V-index:13(V: 13) L:11 C:0X7EC)
			{250, 0},                           // leaf node (V-index:250(V: 250) L:11 C:0X7ED)
			{265, 0},                           // leaf node (V-index:265(V: 265) L:11 C:0X7EE)
			{266, 0},                           // leaf node (V-index:266(V: 266) L:11 C:0X7EF)
			{202, 0},                           // leaf node (V-index:202(V: 202) L:11 C:0X7F0)
			{251, 0},                           // leaf node (V-index:251(V: 251) L:11 C:0X7F1)
			{221, 0},                           // leaf node (V-index:221(V: 221) L:11 C:0X7F2)
			{11, 0},                            // leaf node (V-index:11(V: 11) L:11 C:0X7F3)
			{235, 0},                           // leaf node (V-index:235(V: 235) L:11 C:0X7F4)
			{267, 0},                           // leaf node (V-index:267(V: 267) L:11 C:0X7F5)
			{268, 0},                           // leaf node (V-index:268(V: 268) L:11 C:0X7F6)
			{219, 0},                           // leaf node (V-index:219(V: 219) L:11 C:0X7F7)
			{238, 0},                           // leaf node (V-index:238(V: 238) L:11 C:0X7F8)
			{252, 0},                           // leaf node (V-index:252(V: 252) L:11 C:0X7F9)
			{236, 0},                           // leaf node (V-index:236(V: 236) L:11 C:0X7FA)
			{204, 0},                           // leaf node (V-index:204(V: 204) L:11 C:0X7FB)
			{253, 0},                           // leaf node (V-index:253(V: 253) L:11 C:0X7FC)
			{3, 4},
			{4, 5},
			{5, 6},
			{14, 0},                            // leaf node (V-index:14(V: 14) L:12 C:0XFFA)
			{12, 0},                            // leaf node (V-index:12(V: 12) L:12 C:0XFFB)
			{269, 0},                           // leaf node (V-index:269(V: 269) L:12 C:0XFFC)
			{255, 0},                           // leaf node (V-index:255(V: 255) L:12 C:0XFFD)
			{15, 0},                            // leaf node (V-index:15(V: 15) L:12 C:0XFFE)
			{270, 0},                           // leaf node (V-index:270(V: 270) L:12 C:0XFFF)
		};

		/*
			AAC spec
			Table 4.151 Spectrum Huffman codebooks parameters
			Codebook number, i	unsigned_cb[i]	Dimension of codebook	LAV for codebook	Codebook listed in Table
					0				-					-						0						  -
					1				0					4						1					Table 4.A.2
					2				0					4						1					Table 4.A.3
					3				1					4						2					Table 4.A.4
					4				1					4						2					Table 4.A.5
					5				0					2						4					Table 4.A.6
					6				0					2						4					Table 4.A.7
					7				1					2						7					Table 4.A.8
					8				1					2						7					Table 4.A.9
					9				1					2						12					Table 4.A.10
					10				1					2						12					Table 4.A.11
					11				1					2			    16 (with ESC 8191)			Table 4.A.12
					12				-					-					(reserved)					  -
					13				-					-			perceptual noise substitution		  -
					14				-					-				intensity out-of-phase			  -
					15				-					-				intensity in-phase				  -
					16				1					2					16 (w/o ESC 15)			Table 4.A.12
					17				1					2					16 (with ESC 31)		Table 4.A.12
					18				1					2					16 (with ESC 47)		Table 4.A.12
					19				1					2					16 (with ESC 63)		Table 4.A.12
					20				1					2					16 (with ESC 95)		Table 4.A.12
					21				1					2					16 (with ESC 127)		Table 4.A.12
					22				1					2					16 (with ESC 159)		Table 4.A.12
					23				1					2					16 (with ESC 191)		Table 4.A.12
					24				1					2					16 (with ESC 223)		Table 4.A.12
					25				1					2					16 (with ESC 255)		Table 4.A.12
					26				1					2					16 (with ESC 319)		Table 4.A.12
					27				1					2					16 (with ESC 383)		Table 4.A.12
					28				1					2					16 (with ESC 511)		Table 4.A.12
					29				1					2					16 (with ESC 767)		Table 4.A.12
					30				1					2					16 (with ESC 1023)		Table 4.A.12
					31				1					2					16 (with ESC 2047)		Table 4.A.12
		*/
		std::tuple<bool, int8_t, uint8_t, HCB_BST_NODE*, size_t, const char*> spectrum_hcb_params[32] = {
			{ false, -1, 0, nullptr, 0, "ZRO_HCB" },
			{ false,  4, 1, shcb1_bst,  sizeof(shcb1_bst) / sizeof(HCB_BST_NODE), "HCB#1" },
			{ false,  4, 1, shcb2_bst,  sizeof(shcb2_bst) / sizeof(HCB_BST_NODE), "HCB#2" },
			{ true,   4, 1, shcb3_bst,  sizeof(shcb3_bst) / sizeof(HCB_BST_NODE), "HCB#3" },
			{ true,   4, 2, shcb4_bst,  sizeof(shcb4_bst) / sizeof(HCB_BST_NODE), "HCB#4" },
			{ false,  2, 4, shcb5_bst,  sizeof(shcb5_bst) / sizeof(HCB_BST_NODE), "HCB#5" },
			{ false,  2, 4, shcb6_bst,  sizeof(shcb6_bst) / sizeof(HCB_BST_NODE), "HCB#6" },
			{ true,   2, 7, shcb7_bst,  sizeof(shcb7_bst) / sizeof(HCB_BST_NODE), "HCB#7" },
			{ true,   2, 7, shcb8_bst,  sizeof(shcb8_bst) / sizeof(HCB_BST_NODE), "HCB#8" },
			{ true,   2, 12,shcb9_bst,  sizeof(shcb9_bst) / sizeof(HCB_BST_NODE), "HCB#9" },
			{ true,   2, 12,shcb10_bst, sizeof(shcb10_bst) / sizeof(HCB_BST_NODE), "HCB#10" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 8191)" },
			{ false, -1, 0, nullptr, 0, "RESERVE_HCB" },
			{ false, -1, 0, nullptr, 0, "NOISE_HCB"},
			{ false, -1, 0, nullptr, 0, "INTENSITY_HCB2" },
			{ false, -1, 0, nullptr, 0, "INTENSITY_HCB" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(w/o ESC 15)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 31)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 47)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 63)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 95)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 127)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 159)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 191)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 223)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 255)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 319)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 383)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 511)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 767)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 1023)" },
			{ true,   2, 16,shcb11_bst, sizeof(shcb11_bst) / sizeof(HCB_BST_NODE), "HCB_ESC(with ESC 2047)" },
		};

		/*!
		@brief read the VLC value, code length and code
		@param read_bits how many bits to be read for this VLC code and value
		@param code VLC code value
		@return Return the VLC code's corresponding value, if error happen, exception will be throw
		*/
		int hcb_read_value(CBitstream& bs, const int hcb[][2], int array_size, int* read_bits, uint64_t* code)
		{
			int nReadBits = 0;
			uint64_t nCode = 0;
			uint16_t offset = 0;

			while (hcb[offset][1])
			{
				uint8_t b = (uint8_t)bs.GetBits(1);
				offset += hcb[offset][b];
				nReadBits++;
				nCode = (nCode << 1) | (b ? 1ULL : 0ULL);

				if (offset >= array_size)
				{
					throw AMException(RET_CODE_OUT_OF_RANGE, _T("VLC coding is not expected, and exceed the Huffman codebook definition"));
				}
			}

			AMP_SAFEASSIGN(read_bits, nReadBits);
			AMP_SAFEASSIGN(code, nCode);
			return hcb[offset][0];
		}

		/*!
		@brief Find the code with the specified value in Huffman codebook
		*/
		bool hcb_vlc(const int hcb[][2], int array_size, int value, uint64_t& code, int start_idx)
		{
			if (start_idx == 0)
				code = 0;

			if (hcb[start_idx][1] == 0)	// It is a leaf node
			{
				// Check whether it is the searched value or not
				if (hcb[start_idx][0] == value)
					return true;
			}
			else
			{
				uint64_t new_code = (code << 1);
				// Continue finding the next value in its child trees
				if (hcb_vlc(hcb, array_size, value, code, start_idx + hcb[start_idx][0]) == true)
				{
					code = new_code;
					return true;
				}

				new_code = (code << 1) | 1;
				if (hcb_vlc(hcb, array_size, value, code, start_idx + hcb[start_idx][1]) == true)
				{
					code = new_code;
					return true;
				}
			}

			return false;
		}

		/*!
			@brief Read the VLC table index after decoding it
			@remarks As for the hcod bits and codeword, don't need return in this function, because it can be got from returned index value, and find it in codebook table
		*/
		int shcb_read_value(CBitstream& bs, uint8_t spectrum_huffman_codebook_idx)
		{
			int nReadBits = 0;
			uint64_t nCode = 0;
			uint16_t offset = 0;

			if (spectrum_huffman_codebook_idx >= (uint8_t)_countof(spectrum_hcb_params))
				return -1;

			auto hcb_bst = std::get<3>(spectrum_hcb_params[spectrum_huffman_codebook_idx]);
			uint16_t hcb_bst_size = (uint16_t)std::get<4>(spectrum_hcb_params[spectrum_huffman_codebook_idx]);

			uint64_t bit_pos = bs.Tell();
			while (hcb_bst[offset][1])
			{
				uint8_t b = (uint8_t)bs.GetBits(1);
				offset += hcb_bst[offset][b];
				nReadBits++;
				nCode = (nCode << 1) | (b ? 1ULL : 0ULL);

				if (offset >= hcb_bst_size)
				{
					throw AMException(RET_CODE_OUT_OF_RANGE, _T("VLC coding is not expected, and exceed the Spectrum Huffman codebook definition"));
				}
			}

			return hcb_bst[offset][0];
		}

		/*!
			@remark how to get the bits and codword of hcod_esc?
			step#1: calculate the number of bits, nValBits = floor(log2(value)) + 1
			step#2: the pre-bits number: nPreBits = nValBits - 4
			step#3: the pre-bit string: (1<<nPreBits) - 2
			step#4:
				codword: ((1<<nPreBits) -2)<<(nValBits - 1) | value
				codbits: nValBits -1 + nPreBits
		*/
		int16_t huffman_getescape(CBitstream& bs, int16_t sp)
		{
			uint8_t neg, i;
			int16_t j;
			int16_t off;

			if (sp < 0)
			{
				if (sp != -16)
					return sp;
				neg = 1;
			}
			else {
				if (sp != 16)
					return sp;
				neg = 0;
			}

			for (i = 4; bs.GetBits(1); i++);

			if (i > 16)
				throw AMException(RET_CODE_BUFFER_NOT_COMPATIBLE, _T(""));

			bs.GetBits(i, off);

			j = off | (1 << i);
			if (neg)
				j = -j;

			return j;
		}

	}	// namespace AACAudio
}	// namespace BST
