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
#include "platcomm.h"
#include "av1.h"
#include "av1parser.h"

CAV1Parser::CAV1Parser(bool bAnnexB, bool bSingleOBUParse, RET_CODE* pRetCode)
{
	RET_CODE ret_code = RET_CODE_SUCCESS;
	if (AMP_FAILED(CreateAV1Context(&m_pCtx, bAnnexB, bSingleOBUParse)))
	{
		printf("Failed to create the AV1 Video context.\n");
		ret_code = RET_CODE_ERROR;
	}

	m_rbRawBuf = AM_LRB_Create(read_unit_size * 128);

	AMP_SAFEASSIGN(pRetCode, ret_code);
}

CAV1Parser::~CAV1Parser()
{
	AMP_SAFERELEASE(m_pCtx);
	AM_LRB_Destroy(m_rbRawBuf);
}

RET_CODE CAV1Parser::SetEnumerator(IAV1Enumerator* pEnumerator, uint32_t options)
{
	m_av1_enum = pEnumerator;
	m_av1_enum_options = options;

	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::ProcessInput(uint8_t* pBuf, size_t cbBuf)
{
	return RET_CODE_ERROR_NOTIMPL;
}

RET_CODE CAV1Parser::ProcessOutput(bool bDrain)
{
	return RET_CODE_ERROR_NOTIMPL;
}

RET_CODE CAV1Parser::ParseFrameBuf(uint8_t* pAUBuf, size_t cbAUBuf)
{
	RET_CODE iRet = RET_CODE_SUCCESS;

	uint8_t* p = pAUBuf;
	int64_t cbLeft = (int64_t)cbAUBuf;
	while (cbLeft > 0)
	{
		int64_t cbSubmit = AMP_MIN(cbLeft, 2048);
		if (AMP_SUCCEEDED(iRet = ProcessInput(p, (size_t)cbSubmit)))
		{
			if (ProcessOutput(cbLeft <= 2048 ? true : false) == RET_CODE_ABORT)
			{
				iRet = RET_CODE_ABORT;
				break;
			}
			cbLeft -= cbSubmit;
			p += cbSubmit;
		}
		else
			break;
	}

	return iRet;
}

RET_CODE CAV1Parser::GetAV1Context(IAV1Context** ppCtx)
{
	if (ppCtx == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	m_pCtx->AddRef();
	*ppCtx = m_pCtx;
	return RET_CODE_SUCCESS;
}

RET_CODE CAV1Parser::Reset()
{
	AM_LRB_Reset(m_rbRawBuf);

	m_pCtx->Reset();

	return RET_CODE_SUCCESS;
}


