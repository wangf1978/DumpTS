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
#include "nal_com.h"
#include "NAL.h"
#include "nal_parser.h"
#include "DataUtil.h"
#include "h264_video.h"
#include "h265_video.h"
#include <unordered_map>

using namespace std;

extern unordered_map<std::string, std::string> g_params;

int	ShowNUs()
{
	INALContext* pNALContext = nullptr;
	uint8_t pBuf[2048] = { 0 };

	const int read_unit_size = 2048;

	FILE* rfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;

	auto iter_srcfmt = g_params.find("srcfmt");
	if (iter_srcfmt == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	NAL_CODING coding = NAL_CODING_UNKNOWN;
	if (iter_srcfmt->second.compare("h264") == 0)
		coding = NAL_CODING_AVC;
	else if (iter_srcfmt->second.compare("h265") == 0)
		coding = NAL_CODING_HEVC;
	else if (iter_srcfmt->second.compare("h266") == 0)
		coding = NAL_CODING_VVC;
	else
		return RET_CODE_ERROR_NOTIMPL;

	int options = 0;
	std::string& strShowNU = g_params["showNU"];
	std::vector<std::string> strShowNUOptions;
	splitstr(strShowNU.c_str(), ",;.:", strShowNUOptions);
	if (strShowNUOptions.size() == 0)
		options = NAL_ENUM_OPTION_ALL;
	else
	{
		for (auto& sopt : strShowNUOptions)
		{
			if (MBCSICMP(sopt.c_str(), "au") == 0)
				options |= NAL_ENUM_OPTION_AU;

			if (MBCSICMP(sopt.c_str(), "nu") == 0)
				options |= NAL_ENUM_OPTION_NU;

			if (MBCSICMP(sopt.c_str(), "seimsg") == 0 || MBCSICMP(sopt.c_str(), "seimessage") == 0)
				options |= NAL_ENUM_OPTION_SEI_MSG;

			if (MBCSICMP(sopt.c_str(), "seipayload") == 0)
				options |= NAL_ENUM_OPTION_SEI_PAYLOAD;
		}
	}

	int top = -1;
	auto iterTop = g_params.find("top");
	if (iterTop != g_params.end())
	{
		int64_t top_records = -1;
		ConvertToInt(iterTop->second, top_records);
		if (top_records < 0 || top_records > INT32_MAX)
			top = -1;
	}

	CNALParser NALParser(coding);
	if (AMP_FAILED(NALParser.GetNALContext(&pNALContext)))
	{
		printf("Failed to get the %s NAL context.\n", NAL_CODING_NAME(coding));
		return RET_CODE_ERROR_NOTIMPL;
	}

	class CHEVCNALEnumerator : public INALEnumerator
	{
	public:
		CHEVCNALEnumerator(INALContext* pNALCtx) : m_pNALContext(pNALCtx) {
			m_coding = m_pNALContext->GetNALCoding();
		}

		~CHEVCNALEnumerator() {}

		RET_CODE EnumNALAUBegin(INALContext* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf)
		{
			printf("Access-Unit#%" PRIu64 "\n", m_AUCount);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALUnitBegin(INALContext* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			uint8_t nal_unit_type = m_coding == NAL_CODING_AVC ? (pEBSPNUBuf[0] & 0x1F):(m_coding == NAL_CODING_HEVC? ((pEBSPNUBuf[0] >> 1) & 0x3F):0);
			printf("\tNAL Unit %s -- %s, len: %zu\n", 
				m_coding == NAL_CODING_AVC? avc_nal_unit_type_names[nal_unit_type]:(m_coding == NAL_CODING_HEVC? hevc_nal_unit_type_names[nal_unit_type]:"Unknown"),
				m_coding == NAL_CODING_AVC? avc_nal_unit_type_descs[nal_unit_type]:(m_coding == NAL_CODING_HEVC? hevc_nal_unit_type_descs[nal_unit_type]:"Unknown"), cbEBSPNUBuf);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageBegin(INALContext* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
		{
			printf("\t\tSEI message\n");
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIPayloadBegin(INALContext* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
		{
			printf("\t\t\tSEI payload %s, length: %zu\n", sei_payload_type_names[payload_type], cbRBSPPayloadBuf);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIPayloadEnd(INALContext* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
		{
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageEnd(INALContext* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
		{
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALUnitEnd(INALContext* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			m_NUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALAUEnd(INALContext* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf)
		{
			m_AUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALError(INALContext* pCtx, uint64_t stream_offset, int error_code)
		{
			printf("Hitting error {error_code: %d}.\n", error_code);
			return RET_CODE_SUCCESS;
		}

		INALContext* m_pNALContext = nullptr;
		uint64_t m_AUCount = 0;
		uint64_t m_NUCount = 0;
		NAL_CODING m_coding = NAL_CODING_UNKNOWN;
	}NALEnumerator(pNALContext);

	errno_t errn = fopen_s(&rfp, g_params["input"].c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", g_params["input"].c_str(), errn);
		goto done;
	}

	// Get file size
	_fseeki64(rfp, 0, SEEK_END);
	file_size = _ftelli64(rfp);
	_fseeki64(rfp, 0, SEEK_SET);

	NALParser.SetEnumerator((INALEnumerator*)(&NALEnumerator), options);

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = NALParser.ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = NALParser.ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	if (feof(rfp))
		iRet = NALParser.ProcessOutput(true);

done:
	if (rfp != nullptr)
		fclose(rfp);

	if (pNALContext)
	{
		pNALContext->Release();
		pNALContext = nullptr;
	}

	return iRet;
}

