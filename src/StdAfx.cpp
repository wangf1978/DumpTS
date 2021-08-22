// stdafx.cpp : source file that includes just the standard includes
//	DumpTS.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "StdAfx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void print_mem(uint8_t* pBuf, int cbSize, int indent)
{
	// At first calculate the total size of dumped memory size.
	size_t ccBufLen = 80 * 2 + ((size_t)cbSize + 15) / 16 * (80 + 20 + indent * 4);
	char szIndent[256] = { 0 };
	memset(szIndent, ' ', indent > 255 ? 255 : indent);

	char* szBuffer = new char[ccBufLen + 1];
	char* szWriteBuf = szBuffer;

	int ccWritten = 0;
	ccWritten = snprintf(szWriteBuf, ccBufLen, "%s         00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F\n", szIndent);
	ccBufLen -= ccWritten; szWriteBuf += ccWritten;

	ccWritten = snprintf(szWriteBuf, ccBufLen, "%s         ----------------------------------------------------------------\n", szIndent);
	ccBufLen -= ccWritten; szWriteBuf += ccWritten;

	for (int idx = 0; idx < (int)cbSize; idx++)
	{
		if (idx % 16 == 0)
		{
			ccWritten = snprintf(szWriteBuf, ccBufLen, "%s %06X  ", szIndent, idx); assert(ccWritten > 0);
			ccBufLen -= ccWritten; szWriteBuf += ccWritten; if (ccBufLen < 0)break;
		}

		ccWritten = snprintf(szWriteBuf, ccBufLen, "%02X%s", pBuf[idx], (idx+1)%16 == 0?"":"  "); assert(ccWritten > 0);
		ccBufLen -= ccWritten; szWriteBuf += ccWritten; if (ccBufLen < 0)break;

		if ((idx + 1) % 16 == 0 || idx + 1 == cbSize)
		{
			char szSpace[128] = { 0 };
			if ((idx + 1) % 16 != 0)
			{
				int cbSpace = 0;
				if ((idx + 1) % 16 <= 8)
					cbSpace += 2;

				cbSpace += (16 - (idx + 1) % 16) * 4 - 2;
				memset(szSpace, ' ', cbSpace);
			}

			// print ASCII
			ccWritten = snprintf(szWriteBuf, ccBufLen, "%s | ", szSpace); assert(ccWritten > 0);
			ccBufLen -= ccWritten; szWriteBuf += ccWritten; if (ccBufLen < 0)break;

			int line_idx = idx / 16 * 16;

			for (int ci = line_idx; ci <= idx; ci++)
				szWriteBuf[ci - line_idx] = isprint(pBuf[ci]) ? pBuf[ci] : '.';
			ccWritten = idx - line_idx + 1;

			ccBufLen -= ccWritten; szWriteBuf += ccWritten; if (ccBufLen < 0)break;

			ccWritten = snprintf(szWriteBuf, ccBufLen, "\n"); assert(ccWritten > 0);
			ccBufLen -= ccWritten; szWriteBuf += ccWritten; if (ccBufLen < 0)break;
		}
		else if ((idx + 1) % 8 == 0)
		{
			ccWritten = snprintf(szWriteBuf, ccBufLen, "  "); assert(ccWritten > 0);
			ccBufLen -= ccWritten; szWriteBuf += ccWritten; if (ccBufLen < 0)break;
		}
	}

	ccWritten = snprintf(szWriteBuf, ccBufLen, "\n"); assert(ccWritten > 0);
	ccBufLen -= ccWritten; szWriteBuf += ccWritten;

	printf("%s", szBuffer);

	delete[] szBuffer;
}
