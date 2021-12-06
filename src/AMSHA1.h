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

#ifndef _AM_ALGORITHM_SHA1_H_
#define _AM_ALGORITHM_SHA1_H_

typedef void*			AMSHA1;
typedef unsigned char	AMSHA1_RET[20];

/*!	@brief Initialize SHA1 algorithm. */
AMP_FOUNDATION_PROC	AMSHA1	AM_SHA1_Init	(unsigned char* pBuf=NULL, unsigned long cbBuf=0);
/*!	@brief Input a buffer for SHA1 hash, it can be called for multiple times. */
AMP_FOUNDATION_PROC int		AM_SHA1_Input	(AMSHA1 handle, unsigned char* pBuf, unsigned long cbBuf);
/*!	@brief Calculate the final SHA1 hash value. 
	@remarks Once it is called, AM_SHA1_Input can't be called any more except AM_SHA1_Reset is called. */
AMP_FOUNDATION_PROC int		AM_SHA1_Finalize(AMSHA1 handle);
/*!	@brief Get the calculated hash value. */
AMP_FOUNDATION_PROC int		AM_SHA1_GetHash	(AMSHA1 handle, AMSHA1_RET& Result);
/*!	@brief Reset any immediate calculation result, and be ready to calculate a new SHA1 value. */
AMP_FOUNDATION_PROC	void	AM_SHA1_Reset	(AMSHA1 handle);
/*!	@brief Release the resource for SHA1 hash process. */
AMP_FOUNDATION_PROC void	AM_SHA1_Uninit	(AMSHA1& handle);

#endif
