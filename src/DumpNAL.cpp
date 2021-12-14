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
#include "h266_video.h"
#include "tinyxml2.h"
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

	class CNALEnumerator : public INALEnumerator
	{
	public:
		CNALEnumerator(INALContext* pNALCtx) : m_pNALContext(pNALCtx) {
			m_coding = m_pNALContext->GetNALCoding();
		}

		~CNALEnumerator() {}

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

template<class T>
int PrintNALObject(std::shared_ptr<T> pNavFieldProp)
{
	char* szXmlOutput = NULL;
	int iRet = RET_CODE_SUCCESS;
	tinyxml2::XMLDocument xmlDoc;
	int xml_buffer_size = (int)pNavFieldProp->ProduceDesc(NULL, 0);
	if (xml_buffer_size <= 0)
	{
		printf("Failed to export Xml from the NAL Object.\n");
		goto done;
	}

	szXmlOutput = new char[xml_buffer_size + 1];
	if ((xml_buffer_size = (int)pNavFieldProp->ProduceDesc(szXmlOutput, xml_buffer_size + 1)) <= 0)
	{
		AMP_SAFEDELA(szXmlOutput);
		printf("Failed to generate the Xml from the NAL Object.\n");
		goto done;
	}

	if (xmlDoc.Parse(szXmlOutput, xml_buffer_size) == tinyxml2::XML_SUCCESS)
	{
		int max_len_of_fixed_part = 0;
		// Get the max length
		{
			struct MaxLenTestUtil : tinyxml2::XMLVisitor
			{
				MaxLenTestUtil() : level(0), szLongSpace{ 0 }, max_length(0){
					memset(szLongSpace, ' ', 240);
				}
				/// Visit an element.
				virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
					char szTmp[2048] = { 0 };
					const char* szValue = element.Attribute("Value");
					const char* szAlias = element.Attribute("Alias");
					const char* szDesc = element.Attribute("Desc");
					int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s", level * 4, szLongSpace,
						szAlias ? szAlias : element.Name(),
						szValue ? szValue : "");

					if (ccWritten > max_length && szDesc != NULL && strcmp(szDesc, "") != 0)
						max_length = ccWritten;

					level++;
					return true;
				}
				/// Visit an element.
				virtual bool VisitExit(const tinyxml2::XMLElement& element) {
					level--;
					return true;
				}

				int level;
				char szLongSpace[241];
				int max_length;

			} max_length_tester;

			xmlDoc.Accept(&max_length_tester);
			max_len_of_fixed_part = max_length_tester.max_length;
		}

		struct TestUtil : tinyxml2::XMLVisitor
		{
			TestUtil(int max_length) : level(0), szLongSpace{ 0 }, max_fixed_part_length(max_length){
				memset(szLongSpace, ' ', 240);
			}
			/// Visit an element.
			virtual bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* firstAttribute) {
				char szTmp[2048] = { 0 };
				const char* szValue = element.Attribute("Value");
				const char* szAlias = element.Attribute("Alias");
				const char* szDesc = element.Attribute("Desc");
				int ccWritten = (int)MBCSPRINTF_S(szTmp, sizeof(szTmp) / sizeof(szTmp[0]), "%.*s%s: %s", level * 4, szLongSpace,
					szAlias ? szAlias : element.Name(),
					szValue ? szValue : "");

				printf("%s%.*s%s%s\n", szTmp,
					max_fixed_part_length > ccWritten?(max_fixed_part_length - ccWritten):0, szLongSpace,
					szDesc && strcmp(szDesc, "") != 0 ? "// " : "",
					szDesc ? GetFixedWidthStrWithEllipsis(szDesc, 70).c_str() : "");
				level++;
				return true;
			}
			/// Visit an element.
			virtual bool VisitExit(const tinyxml2::XMLElement& element) {
				level--;
				return true;
			}

			int level;
			char szLongSpace[241];
			int max_fixed_part_length;

		} tester(max_len_of_fixed_part);

		xmlDoc.Accept(&tester);
	}
	else
		printf("The generated XML is invalid.\n");

done:
	AMP_SAFEDELA(szXmlOutput);

	return iRet;
}

int PrintHRDFromAVCSPS(H264_NU sps_nu)
{
	if (!sps_nu || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	uint64_t BitRates[32] = { 0 };
	uint64_t CpbSize[32] = { 0 };
	bool bUseConcludedValues[2] = { true, true };
	if (sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters &&
		sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters_present_flag &&
		sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters)
	{
		printf("Type-I HRD:\n");
		auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters;
		for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->cpb_cnt_minus1; SchedSelIdx++)
		{
			printf("\tSchedSelIdx#%d:\n", SchedSelIdx);
			BitRates[SchedSelIdx] = (hrd_parameters->bit_rate_value_minus1[SchedSelIdx] + 1) * (uint64_t)1ULL<<(6 + hrd_parameters->bit_rate_scale);
			CpbSize[SchedSelIdx] = (hrd_parameters->cpb_size_value_minus1[SchedSelIdx] + 1)*(uint64_t)1ULL << (4 + hrd_parameters->cpb_size_scale);
			printf("\t\tthe maximum input bit rate for the CPB: % " PRIu64"bps/%sbps \n", 
				BitRates[SchedSelIdx], GetHumanReadNumber(BitRates[SchedSelIdx]).c_str());
			printf("\t\tthe CPB size: % " PRIu64"b/%sb \n",
				CpbSize[SchedSelIdx], GetHumanReadNumber(CpbSize[SchedSelIdx]).c_str());
			printf("\t\t%s mode\n", hrd_parameters->cbr_flag[SchedSelIdx]?"CBR":"VBR");
		}
	}

	if (sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters &&
		sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters_present_flag &&
		sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters)
	{
		printf("Type-II HRD:\n");
		auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters;
		for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->cpb_cnt_minus1; SchedSelIdx++)
		{
			printf("\tSchedSelIdx#%d:\n", SchedSelIdx);
			BitRates[SchedSelIdx] = (hrd_parameters->bit_rate_value_minus1[SchedSelIdx] + 1) * (uint64_t)1ULL << (6 + hrd_parameters->bit_rate_scale);
			CpbSize[SchedSelIdx] = (hrd_parameters->cpb_size_value_minus1[SchedSelIdx] + 1)*(uint64_t)1ULL << (4 + hrd_parameters->cpb_size_scale);
			printf("\t\tthe maximum input bit rate for the CPB: % " PRIu64"bps/%sbps \n",
				BitRates[SchedSelIdx], GetHumanReadNumber(BitRates[SchedSelIdx]).c_str());
			printf("\t\tthe CPB size: % " PRIu64"b/%sb \n",
				CpbSize[SchedSelIdx], GetHumanReadNumber(CpbSize[SchedSelIdx]).c_str());
			printf("\t\t%s mode\n", hrd_parameters->cbr_flag[SchedSelIdx] ? "CBR" : "VBR");
		}
	}


	return RET_CODE_SUCCESS;
}

int PrintHRDFromSEIPayloadBufferingPeriod(INALContext* pCtx, uint8_t* pSEIPayload, size_t cbSEIPayload)
{
	int iRet = RET_CODE_SUCCESS;
	if (pSEIPayload == nullptr || cbSEIPayload == 0)
		return RET_CODE_INVALID_PARAMETER;

	AMBst bst = nullptr;
	BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD* pBufPeriod = new
		BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD((int)cbSEIPayload, pCtx);
	NAL_CODING coding = pCtx->GetNALCoding();
	
	if (pBufPeriod != nullptr)
		bst = AMBst_CreateFromBuffer(pSEIPayload, (int)cbSEIPayload);

	if (pBufPeriod == nullptr || bst == nullptr)
	{
		iRet = RET_CODE_OUTOFMEMORY;
		goto done;
	}

	if (AMP_FAILED(iRet = pBufPeriod->Map(bst)))
	{
		printf("Failed to unpack the SEI payload: buffering period.\n");
		goto done;
	}

	if (coding == NAL_CODING_AVC)
	{
		H264_NU sps_nu;
		INALAVCContext* pNALAVCCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_INALAVCContext, (void**)&pNALAVCCtx)))
		{
			sps_nu = pNALAVCCtx->GetAVCSPS(pBufPeriod->bp_seq_parameter_set_id);

			pNALAVCCtx->Release();
			pNALAVCCtx = nullptr;
		}

		if (sps_nu &&
			sps_nu->ptr_seq_parameter_set_rbsp != nullptr &&
			sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters)
		{
			auto vui_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters;

			if (vui_parameters->vcl_hrd_parameters_present_flag)
			{
				printf("Type-I HRD:\n");
				auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters;
				for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->cpb_cnt_minus1; SchedSelIdx++)
				{
					printf("\tSchedSelIdx#%d:\n", SchedSelIdx);
					printf("\t\tinitial_cpb_removal_delay: %" PRIu32 "\n", pBufPeriod->vcl_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_delay);
					printf("\t\tinitial_cpb_removal_delay_offset: %" PRIu32 "\n", pBufPeriod->vcl_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_offset);
				}
			}

			if (vui_parameters->nal_hrd_parameters_present_flag)
			{
				printf("Type-II HRD:\n");
				auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters;
				for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->cpb_cnt_minus1; SchedSelIdx++)
				{
					printf("\tSchedSelIdx#%d:\n", SchedSelIdx);
					printf("\t\tinitial_cpb_removal_delay: %" PRIu32 "\n", pBufPeriod->nal_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_delay);
					printf("\t\tinitial_cpb_removal_delay_offset: %" PRIu32 "\n", pBufPeriod->nal_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_offset);
				}
			}
		}
	}
	else if (coding == NAL_CODING_HEVC)
	{
		
	}
	else if (coding == NAL_CODING_VVC)
	{

	}


done:
	if (bst)
		AMBst_Destroy(bst);

	AMP_SAFEDEL(pBufPeriod);

	return iRet;
}

int PrintHRDFromSEIPayloadPicTiming(INALContext* pCtx, uint8_t* pSEIPayload, size_t cbSEIPayload)
{
	int iRet = RET_CODE_SUCCESS;

	if (pSEIPayload == nullptr || cbSEIPayload == 0)
		return RET_CODE_INVALID_PARAMETER;

	NAL_CODING coding = pCtx->GetNALCoding();

	AMBst bst = nullptr;
	bst = AMBst_CreateFromBuffer(pSEIPayload, (int)cbSEIPayload);

	if (bst == nullptr)
	{
		iRet = RET_CODE_OUTOFMEMORY;
		goto done;
	}

	if (coding == NAL_CODING_AVC)
	{
		BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264* pPicTiming = new
			BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264((int)cbSEIPayload, pCtx);
		NAL_CODING coding = pCtx->GetNALCoding();

		if (AMP_FAILED(iRet = pPicTiming->Map(bst)))
		{
			printf("Failed to unpack the SEI payload: buffering period.\n");
			goto done;
		}

		printf("Picture Timing:\n");
		printf("\tcpb_removal_delay: %" PRIu32 "\n", pPicTiming->cpb_removal_delay);
		printf("\tdpb_output_delay: %" PRIu32 "\n", pPicTiming->dpb_output_delay);
	}
	else
	{
		// TODO...
	}

done:
	if (bst)
		AMBst_Destroy(bst);

	return iRet;
}

/*
	 1: VPS
	 2: SPS
	 3: PPS
	12: HRD
*/
int ShowNALObj(int object_type)
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
	{
		printf("Only support H.264/265/266 at present.\n");
		return RET_CODE_ERROR_NOTIMPL;
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

	if (coding == NAL_CODING_AVC)
	{
		if (object_type == 1)
		{
			printf("No VPS object in H.264 stream.\n");
			return RET_CODE_INVALID_PARAMETER;
		}
		else if (object_type == 2)
			pNALContext->SetNUFilters({ BST::H264Video::SPS_NUT });
		else if (object_type == 3)
			pNALContext->SetNUFilters({ BST::H264Video::SPS_NUT, BST::H264Video::PPS_NUT });
	}
	else if (coding == NAL_CODING_HEVC)
	{
		if (object_type == 1)
			pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT });
		else if (object_type == 2)
			pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT, BST::H265Video::SPS_NUT });
		else if (object_type == 3)
			pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT, BST::H265Video::SPS_NUT, BST::H265Video::PPS_NUT });
	}
	else if (coding == NAL_CODING_VVC)
	{
		if (object_type == 1)
			pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT });
		else if (object_type == 2)
			pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT, BST::H266Video::SPS_NUT });
		else if (object_type == 3)
			pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT, BST::H266Video::SPS_NUT, BST::H266Video::PPS_NUT });
	}

	class CNALEnumerator : public INALEnumerator
	{
	public:
		CNALEnumerator(INALContext* pNALCtx, int objType) : m_pNALContext(pNALCtx), object_type(objType) {
			m_coding = m_pNALContext->GetNALCoding();
			if (m_coding == NAL_CODING_AVC)
				m_pNALContext->QueryInterface(IID_INALAVCContext, (void**)&m_pNALAVCContext);
			else if (m_coding == NAL_CODING_HEVC)
				m_pNALContext->QueryInterface(IID_INALHEVCContext, (void**)&m_pNALHEVCContext);
		}

		~CNALEnumerator() {}

		RET_CODE EnumNALAUBegin(INALContext* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf){return RET_CODE_SUCCESS;}

		RET_CODE EnumNALUnitBegin(INALContext* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			int iRet = RET_CODE_SUCCESS;
			uint8_t nal_unit_type = 0xFF;

			AMBst bst = AMBst_CreateFromBuffer(pEBSPNUBuf, (int)cbEBSPNUBuf);
			if (m_coding == NAL_CODING_AVC)
			{
				nal_unit_type = (pEBSPNUBuf[0] & 0x1F);
				if (nal_unit_type == BST::H264Video::SPS_NUT || nal_unit_type == BST::H264Video::PPS_NUT)
				{
					H264_NU nu = m_pNALAVCContext->CreateAVCNU();
					if (AMP_FAILED(iRet = nu->Map(bst)))
					{
						printf("Failed to unpack %s parameter set {error: %d}.\n", avc_nal_unit_type_descs[nal_unit_type], iRet);
						goto done;
					}

					// Check whether the buffer is the same with previous one or not
					AMSHA1_RET sha1_ret = { 0 };
					AMSHA1 sha1_handle = AM_SHA1_Init(pEBSPNUBuf, (int)cbEBSPNUBuf);
					AM_SHA1_Finalize(sha1_handle);
					AM_SHA1_GetHash(sha1_handle, sha1_ret);
					AM_SHA1_Uninit(sha1_handle);

					if (nal_unit_type == BST::H264Video::SPS_NUT)
					{
						if (memcmp(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALAVCContext->UpdateAVCSPS(nu);

						if (object_type == 2)
							PrintNALObject(nu);
						else if (object_type == 12)
							PrintHRDFromAVCSPS(nu);
					}
					else if (nal_unit_type == BST::H264Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALAVCContext->UpdateAVCPPS(nu);

						if (object_type == 3)
							PrintNALObject(nu);
					}
				}
			}
			else if (m_coding == NAL_CODING_HEVC)
			{
				nal_unit_type = ((pEBSPNUBuf[0] >> 1) & 0x3F);

				if (nal_unit_type == BST::H265Video::VPS_NUT || nal_unit_type == BST::H265Video::SPS_NUT || nal_unit_type == BST::H265Video::PPS_NUT)
				{
					H265_NU nu = m_pNALHEVCContext->CreateHEVCNU();
					if (AMP_FAILED(iRet = nu->Map(bst)))
					{
						printf("Failed to unpack %s.\n", hevc_nal_unit_type_names[nal_unit_type]);
						goto done;
					}

					// Check whether the buffer is the same with previous one or not
					AMSHA1_RET sha1_ret = { 0 };
					AMSHA1 sha1_handle = AM_SHA1_Init(pEBSPNUBuf, (int)cbEBSPNUBuf);
					AM_SHA1_Finalize(sha1_handle);
					AM_SHA1_GetHash(sha1_handle, sha1_ret);
					AM_SHA1_Uninit(sha1_handle);

					if (nal_unit_type == BST::H265Video::VPS_NUT)
					{
						if (memcmp(prev_vps_sha1_ret[nu->ptr_video_parameter_set_rbsp->vps_video_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_vps_sha1_ret[nu->ptr_video_parameter_set_rbsp->vps_video_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCVPS(nu);

						if (object_type == 1)
							PrintNALObject(nu);
					}
					else if (nal_unit_type == BST::H265Video::SPS_NUT)
					{
						if (memcmp(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCSPS(nu);

						if (object_type == 2)
							PrintNALObject(nu);
					}
					else if (nal_unit_type == BST::H265Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCPPS(nu);

						if (object_type == 3)
							PrintNALObject(nu);
					}
				}
			}

		done:
			if (bst)
				AMBst_Destroy(bst);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageBegin(INALContext* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALSEIPayloadBegin(INALContext* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
		{
			if (payload_type == SEI_PAYLOAD_BUFFERING_PERIOD)
			{
				if (object_type == 12)
					PrintHRDFromSEIPayloadBufferingPeriod(pCtx, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
			}
			else if (payload_type == SEI_PAYLOAD_PIC_TIMING)
			{
				if (object_type == 12)
					PrintHRDFromSEIPayloadPicTiming(pCtx, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
			}
			return RET_CODE_SUCCESS;
		}
		RET_CODE EnumNALSEIPayloadEnd(INALContext* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALSEIMessageEnd(INALContext* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALUnitEnd(INALContext* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			m_NUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALAUEnd(INALContext* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALError(INALContext* pCtx, uint64_t stream_offset, int error_code)
		{
			printf("Hitting error {error_code: %d}.\n", error_code);
			return RET_CODE_SUCCESS;
		}

		INALContext* m_pNALContext = nullptr;
		INALAVCContext* m_pNALAVCContext = nullptr;
		INALHEVCContext* m_pNALHEVCContext = nullptr;
		uint64_t m_NUCount = 0;
		NAL_CODING m_coding = NAL_CODING_UNKNOWN;
		AMSHA1_RET prev_sps_sha1_ret[32] = { {0} };
		AMSHA1_RET prev_vps_sha1_ret[32] = { {0} };
		// H.264, there may be 256 pps at maximum; H.265/266: there may be 64 pps at maximum
		AMSHA1_RET prev_pps_sha1_ret[256] = { {0} };
		int object_type = -1;
	}NALEnumerator(pNALContext, object_type);

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

	NALParser.SetEnumerator((INALEnumerator*)(&NALEnumerator), object_type == 12?NAL_ENUM_OPTION_ALL:NAL_ENUM_OPTION_NU);

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

int	ShowVPS()
{
	return ShowNALObj(1);
}

int	ShowSPS()
{
	return ShowNALObj(2);
}

int ShowPPS()
{
	return ShowNALObj(3);
}

int	ShowHRD()
{
	return ShowNALObj(12);
}

