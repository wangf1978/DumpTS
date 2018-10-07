#include "StdAfx.h"
#include "PayloadBuf.h"
#include "crc.h"
#include <algorithm>

CPayloadBuf::CPayloadBuf(FILE* fw, uint8_t nTSPackSize)
	: buffer_len(0)
	, m_fw(fw)
	, m_ts_pack_size(nTSPackSize)
{
	buffer_alloc_size = 20 * 1024 * 1024;
	buffer = new (std::nothrow) unsigned char[buffer_alloc_size];
}

CPayloadBuf::CPayloadBuf(uint16_t PID)
	: buffer(NULL)
	, buffer_len(0)
	, buffer_alloc_size(0)
	, m_fw(NULL)
	, m_ts_pack_size(0)
	, m_PID(PID)
{
}

CPayloadBuf::~CPayloadBuf()
{
	if (buffer != NULL)
	{
		delete[] buffer;
		buffer = NULL;
	}
}

int CPayloadBuf::PushTSBuf(uint32_t idxTSPack, uint8_t* pBuf, uint8_t offStart, uint8_t offEnd)
{
	if (idxTSPack != UINT32_MAX)
		slices.emplace_back(idxTSPack, offStart, offEnd);

	if (buffer == NULL)
	{
		buffer_alloc_size = 64 * 1024;
		buffer = new (std::nothrow) unsigned char[buffer_alloc_size];
		if (buffer == NULL)
		{
			printf("Failed to allocate the buffer with the size: %" PRIu32 " (bytes).\n", buffer_alloc_size);
			return -1;
		}
	}

	// check whether the buffer exceeds the next buffer length
	if (buffer_alloc_size < buffer_len + (offEnd - offStart))
	{
		unsigned char* new_buffer = new (std::nothrow) unsigned char[buffer_alloc_size * 2];
		if (new_buffer == NULL)
		{
			printf("Failed to allocate the buffer with the size: %" PRIu32 " (bytes).\n", buffer_alloc_size << 1);
			return -1;
		}

		memcpy(new_buffer, buffer, buffer_len);
		delete[] buffer;

		buffer_alloc_size *= 2;
		buffer = new_buffer;
	}

	memcpy(buffer + buffer_len, pBuf + offStart, offEnd - offStart);

	buffer_len += offEnd - offStart;

	return 0;
}

int CPayloadBuf::Process(std::unordered_map<int, int>& pid_maps)
{
	unsigned long ulMappedSize = 0;
	unsigned char* pBuf = buffer;
	// Check the current payload buffer is a PSI buffer or not.
	if (buffer_len >= 4 && !IS_PES_PAYLOAD(buffer))
	{
		unsigned char pointer_field = *pBuf;
		unsigned char table_id;
		ulMappedSize++;
		ulMappedSize += pointer_field;

		if (ulMappedSize < buffer_len)
			table_id = pBuf[ulMappedSize];
		else
			return -1;

		if (ulMappedSize + 3 > buffer_len)
			return -1;

		//unsigned char* pSectionStart = &pBuf[ulMappedSize];
		unsigned long ulSectionStart = ulMappedSize;

		unsigned short section_length = (pBuf[ulMappedSize + 1] << 8 | pBuf[ulMappedSize + 2]) & 0XFFF;

		// The maximum number of bytes in a section of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 defined PSI table is
		// 1024 bytes. The maximum number of bytes in a private_section is 4096 bytes.
		// The DSMCC section data is also 4096 (table_id from 0x38 to 0x3F)
		if (section_length > ((pBuf[ulMappedSize] >= 0x40 && pBuf[ulMappedSize] <= 0xFE) ||
							  (pBuf[ulMappedSize] >= 0x38 && pBuf[ulMappedSize] <= 0x3F) ? 4093 : 1021))
			return -1;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

		if (ulMappedSize + 3 + section_length > buffer_len)
			return -1;	// RET_CODE_BUFFER_TOO_SMALL;

		unsigned char section_syntax_indicator = (pBuf[ulMappedSize + 1] >> 7) & 0x01;

		if ((table_id == TID_program_association_section || table_id == TID_TS_program_map_section) && !section_syntax_indicator)
			return -1;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

		if (ulMappedSize + 8 > buffer_len)
			return -1;

		ulMappedSize += 8;

		bool bChanged = false;

		// Check the PID in PAT and PMT
		if (table_id == TID_program_association_section)
		{
			// 4 bytes of CRC32, 4 bytes of PAT entry
			while (ulMappedSize + 4 + 4 <= 3 + section_length + ulSectionStart)
			{
				unsigned short PID = (pBuf[ulMappedSize + 2] & 0x1f) << 8 | pBuf[ulMappedSize + 3];

				if (pid_maps.find(PID) != pid_maps.end())
				{
					pBuf[ulMappedSize + 2] &= 0xE0;
					pBuf[ulMappedSize + 2] |= ((pid_maps[PID] >> 8) & 0x1F);
					pBuf[ulMappedSize + 3] = pid_maps[PID] & 0xFF;

					// Update the original TS pack according to buffer offset and value
					WriteBack(ulMappedSize + 2, &pBuf[ulMappedSize + 2], 2);

					bChanged = true;
				}

				ulMappedSize += 4;
			}
		}
		else if (table_id == TID_TS_program_map_section)
		{
			if (ulMappedSize + 4 > buffer_len)
				return -1;

			// Change PCR_PID
			unsigned short PID = (pBuf[ulMappedSize] & 0x1f) << 8 | pBuf[ulMappedSize + 1];
			if (pid_maps.find(PID) != pid_maps.end())
			{
				pBuf[ulMappedSize] &= 0xE0;
				pBuf[ulMappedSize] |= ((pid_maps[PID] >> 8) & 0x1F);
				pBuf[ulMappedSize + 1] = pid_maps[PID] & 0xFF;

				// Update the original TS pack according to buffer offset and value
				WriteBack(ulMappedSize, &pBuf[ulMappedSize], 2);
			}

			unsigned short program_info_length = (pBuf[ulMappedSize + 2] & 0xF) << 8 | pBuf[ulMappedSize + 3];
			ulMappedSize += 4;

			if (ulMappedSize + program_info_length > buffer_len)
				return -1;

			ulMappedSize += program_info_length;

			// Reserve 4 bytes of CRC32 and 5 bytes of basic ES info (stream_type, reserved, elementary_PID, reserved and ES_info_length)
			while (ulMappedSize + 5 + 4 <= 3 + section_length + ulSectionStart)
			{
				//unsigned char stream_type = pBuf[ulMappedSize];
				PID = (pBuf[ulMappedSize + 1] & 0x1f) << 8 | pBuf[ulMappedSize + 2];
				unsigned short ES_info_length = (pBuf[ulMappedSize + 3] & 0xF) << 8 | pBuf[ulMappedSize + 4];

				if (pid_maps.find(PID) != pid_maps.end())
				{
					pBuf[ulMappedSize + 1] &= 0xE0;
					pBuf[ulMappedSize + 1] |= ((pid_maps[PID] >> 8) & 0x1F);
					pBuf[ulMappedSize + 2] = pid_maps[PID] & 0xFF;

					// Update the original TS pack according to buffer offset and value
					WriteBack(ulMappedSize + 1, &pBuf[ulMappedSize + 1], 2);

					bChanged = true;
				}

				ulMappedSize += 5;
				ulMappedSize += ES_info_length;
			}
		}

		if (bChanged)
		{
			// Recalculate the CRC32 value
			F_CRC_InicializaTable();
			crc crc_val = F_CRC_CalculaCheckSum(&pBuf[ulSectionStart], (uint16_t)(ulMappedSize - ulSectionStart));
			pBuf[ulMappedSize] = (crc_val >> 24) & 0xFF;
			pBuf[ulMappedSize + 1] = (crc_val >> 16) & 0xFF;
			pBuf[ulMappedSize + 2] = (crc_val >> 8) & 0xFF;
			pBuf[ulMappedSize + 3] = (crc_val) & 0xFF;

			WriteBack(ulMappedSize, &pBuf[ulMappedSize], 4);
		}
	}

	return 0;
}

void CPayloadBuf::Reset()
{
	slices.clear();
	buffer_len = 0;
}

int CPayloadBuf::WriteBack(unsigned int off, unsigned char* pBuf, unsigned long cbSize)
{
	int iRet = -1;
	unsigned long cbRead = 0;
	// Find which TS pack the corresponding bytes are written into TS pack buffer
	for (std::vector<PayloadBufSlice>::iterator iter = slices.begin(); iter != slices.end(); iter++)
	{
		if (off >= cbRead && off < cbRead + iter->end_off - iter->start_off)
		{
			const unsigned long cbWritten = std::min(cbSize, cbRead + iter->end_off - iter->start_off - off);

			if (m_fw != NULL)
			{
				// Record the backup position of file
				long long backup_pos = _ftelli64(m_fw);

				_fseeki64(m_fw, iter->ts_packet_idx*m_ts_pack_size + iter->start_off + off - cbRead, SEEK_SET);
				if (fwrite(pBuf, 1, cbWritten, m_fw) != cbWritten)
				{
					printf("Failed to write back the bytes into destination file.\n");
					_fseeki64(m_fw, backup_pos, SEEK_SET);
					goto done;
				}

				// Restore the original file position
				_fseeki64(m_fw, backup_pos, SEEK_SET);
			}

			off += cbWritten;
			cbSize -= cbWritten;
			pBuf += cbWritten;
		}

		if (off + cbSize <= cbRead)
			break;

		cbRead += iter->end_off - iter->start_off;
	}

	iRet = 0;

done:
	return iRet;
}

CPSIBuf::CPSIBuf(PSI_PROCESS_CONTEXT* CtxPSIProcess, unsigned short PID)
	: CPayloadBuf(PID)
	, version_number(0xFF)
	, ctx_psi_process(CtxPSIProcess)
{

}

/*!	@breif Process PAT and PMT to get the stream information.
@retval -1 buffer is too small
@retval -2 CRC verification failure
@retval -3 Unsupported or unimplemented */
int CPSIBuf::ProcessPSI()
{
	int iRet = -1;
	unsigned long ulMappedSize = 0;
	unsigned char* pBuf = buffer;

	if (buffer_len < 4)
		return -1;

	unordered_map<unsigned short, CPSIBuf*>& pPMTPayloads = *ctx_psi_process->pPSIPayloads;

	ctx_psi_process->bChanged = false;

	unsigned char pointer_field = *pBuf;
	ulMappedSize++;
	ulMappedSize += pointer_field;

	if (ulMappedSize < buffer_len)
		table_id = pBuf[ulMappedSize];
	else
		return -1;

	if (ulMappedSize + 3 > buffer_len)
		return -1;

	//unsigned char* pSectionStart = &pBuf[ulMappedSize];
	unsigned long ulSectionStart = ulMappedSize;

	unsigned short section_length = (pBuf[ulMappedSize + 1] << 8 | pBuf[ulMappedSize + 2]) & 0XFFF;

	// The maximum number of bytes in a section of a Rec. ITU-T H.222.0 | ISO/IEC 13818-1 defined PSI table is
	// 1024 bytes. The maximum number of bytes in a private_section is 4096 bytes.
	// The DSMCC section data is also 4096 (table_id from 0x38 to 0x3F)
	if (section_length > ((pBuf[ulMappedSize] >= 0x40 && pBuf[ulMappedSize] <= 0xFE) ||
						  (pBuf[ulMappedSize] >= 0x38 && pBuf[ulMappedSize] <= 0x3F) ? 4093 : 1021))
		return -3;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

	if (ulMappedSize + 3 + section_length > buffer_len)
		return -1;	// RET_CODE_BUFFER_TOO_SMALL;

	unsigned char section_syntax_indicator = (pBuf[ulMappedSize + 1] >> 7) & 0x01;

	if (table_id != TID_program_association_section && table_id != TID_TS_program_map_section)
		return -3;	// Only support PAT or PMT

	if ((table_id == TID_program_association_section || table_id == TID_TS_program_map_section) && !section_syntax_indicator)
		return -3;	// RET_CODE_BUFFER_NOT_COMPATIBLE;

	if (section_syntax_indicator) {
		F_CRC_InicializaTable();
		if (F_CRC_CalculaCheckSum(pBuf + ulMappedSize, 3 + section_length) != 0) {
			printf("[13818-1] current PSI section failed do check-sum.\n");
			return -2;
		}
	}

	if (ulMappedSize + 8 > buffer_len)
		return -1;

	uint8_t current_next_indicator = pBuf[ulMappedSize + 5] & 0x1;
	if (current_next_indicator == 0)
		return 0;

	uint8_t current_version_number = (pBuf[ulMappedSize + 5] >> 1) & 0x1F;
	if (current_version_number != version_number)
	{
		ctx_psi_process->bChanged = true;
		version_number = current_version_number;
		section_number = pBuf[ulMappedSize + 6];
		last_section_number = pBuf[ulMappedSize + 7];
	}

	ulMappedSize += 8;

	if (m_PID == PID_PROGRAM_ASSOCIATION_TABLE && table_id == TID_program_association_section)
	{
		// check how many PMT entry exist in the current PAT
		int num_of_PMTs = (section_length + ulSectionStart + 3 - ulMappedSize - 4) >> 2;
		std::vector<std::tuple<unsigned short/* program number*/, unsigned short/*PMT PID*/>> PMT_PIDs;
		PMT_PIDs.reserve(num_of_PMTs);

		// 4 bytes of CRC32, 4 bytes of PAT entry
		while (ulMappedSize + 4 + 4 <= 3 + section_length + ulSectionStart)
		{
			unsigned short program_number = (pBuf[ulMappedSize] << 8) | pBuf[ulMappedSize + 1];
			unsigned short PMT_PID = (pBuf[ulMappedSize + 2] & 0x1f) << 8 | pBuf[ulMappedSize + 3];

			if (program_number != 0)
				PMT_PIDs.push_back({ program_number, PMT_PID });

			ulMappedSize += 4;
		}

		// Refresh the current PMT payloads
		// If the current PID in PMT payloads does not belong to PAT, delete it
		for (auto iter = pPMTPayloads.cbegin(); iter != pPMTPayloads.cend(); iter++)
		{
			auto PMT_iter = PMT_PIDs.cbegin();
			for (; PMT_iter != PMT_PIDs.cend() && iter->first != std::get<1>(*PMT_iter); PMT_iter++);

			if (PMT_iter == PMT_PIDs.cend() && iter->first != PID_PROGRAM_ASSOCIATION_TABLE)
			{
				delete iter->second;
				pPMTPayloads.erase(iter);
			}
		}

		// If the PMT ID does not exist in pPMTPayloads, create it.
		for (auto PMT_iter = PMT_PIDs.cbegin(); PMT_iter != PMT_PIDs.cend(); PMT_iter++)
		{
			if (pPMTPayloads.find(std::get<1>(*PMT_iter)) == pPMTPayloads.end())
				pPMTPayloads[std::get<1>(*PMT_iter)] = new CPMTBuf(ctx_psi_process, std::get<1>(*PMT_iter), std::get<0>(*PMT_iter));
		}

		iRet = 0;
	}
	else if (table_id == TID_TS_program_map_section)
	{
		// Process the current PMT, and get the related information.
		if (ulMappedSize + 4 > buffer_len)
			return -1;

		// Change PCR_PID
		m_PCR_PID = (pBuf[ulMappedSize] & 0x1f) << 8 | pBuf[ulMappedSize + 1];

		unsigned short program_info_length = (pBuf[ulMappedSize + 2] & 0xF) << 8 | pBuf[ulMappedSize + 3];
		ulMappedSize += 4;

		if (ulMappedSize + program_info_length > buffer_len)
			return -1;

		ulMappedSize += program_info_length;

		// Reserve 4 bytes of CRC32 and 5 bytes of basic ES info (stream_type, reserved, elementary_PID, reserved and ES_info_length)
		while (ulMappedSize + 5 + 4 <= 3 + section_length + ulSectionStart)
		{
			unsigned char stream_type = pBuf[ulMappedSize];
			unsigned short ES_PID = (pBuf[ulMappedSize + 1] & 0x1f) << 8 | pBuf[ulMappedSize + 2];
			m_stream_types[ES_PID] = stream_type;
			unsigned short ES_info_length = (pBuf[ulMappedSize + 3] & 0xF) << 8 | pBuf[ulMappedSize + 4];

			ulMappedSize += 5;
			ulMappedSize += ES_info_length;
		}

		iRet = 0;
	}
	else
		iRet = -3;

	return iRet;
}

CPMTBuf::CPMTBuf(PSI_PROCESS_CONTEXT* CtxPSIProcess, unsigned short PID, unsigned short nProgramNumber)
	: CPSIBuf(CtxPSIProcess, PID)
	, program_number(nProgramNumber)
{
	table_id = TID_TS_program_map_section;
}

int CPMTBuf::GetPMTInfo(unsigned short ES_PID, unsigned char& stream_type)
{
	if (m_stream_types.find(ES_PID) == m_stream_types.end())
		return -1;

	stream_type = m_stream_types[ES_PID];
	return 0;
}

unordered_map<unsigned short, unsigned char>& CPMTBuf::GetStreamTypes()
{
	return m_stream_types;
}
