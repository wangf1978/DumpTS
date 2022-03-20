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
#ifndef __MEDIA_SYNTAX_ELEMENT_H__
#define __MEDIA_SYNTAX_ELEMENT_H__

#include "dump_data_type.h"
#include "combase.h"

class IMSEParser : public IUnknown
{
public:
	virtual RET_CODE	SetEnumerator(IUnknown* pEnumerator, uint32_t options) = 0;
	virtual RET_CODE	ProcessInput(uint8_t* pBuf, size_t cbBuf) = 0;
	virtual RET_CODE	ProcessOutput(bool bDrain = false) = 0;
	virtual RET_CODE	GetContext(IUnknown** ppCtx) = 0;
	virtual RET_CODE	Reset() = 0;

public:
	IMSEParser(){}
	virtual ~IMSEParser() {}
};

#endif
