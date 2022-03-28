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
#include "mediaobjprint.h"
#include "DumpTS.h"
#include <unordered_map>

using namespace std;

extern map<std::string, std::string, CaseInsensitiveComparator> g_params;

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

	int top = GetTopRecordCount();

	CNALParser NALParser(coding);
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(NALParser.GetContext(&pMSECtx)) || 
		FAILED(pMSECtx->QueryInterface(IID_INALContext, (void**)&pNALContext)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the %s NAL context.\n", NAL_CODING_NAME(coding));
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	class CNALEnumerator : public CComUnknown, public INALEnumerator
	{
	public:
		CNALEnumerator(INALContext* pNALCtx) : m_pNALContext(pNALCtx) {
			if (m_pNALContext)
				m_pNALContext->AddRef();
			m_coding = m_pNALContext->GetNALCoding();
		}

		virtual ~CNALEnumerator() {
			AMP_SAFERELEASE(m_pNALContext);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_INALEnumerator)
				return GetCOMInterface((INALEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumNewVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type){return RET_CODE_SUCCESS;}

		RET_CODE EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type)
		{
			printf("Access-Unit#%" PRIu64 "\n", m_AUCount);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			uint8_t nal_unit_type = m_coding == NAL_CODING_AVC ? (pEBSPNUBuf[0] & 0x1F):(m_coding == NAL_CODING_HEVC? ((pEBSPNUBuf[0] >> 1) & 0x3F):0);
			printf("\tNAL Unit %s -- %s, len: %zu\n", 
				m_coding == NAL_CODING_AVC? avc_nal_unit_type_names[nal_unit_type]:(m_coding == NAL_CODING_HEVC? hevc_nal_unit_type_names[nal_unit_type]:"Unknown"),
				m_coding == NAL_CODING_AVC? avc_nal_unit_type_descs[nal_unit_type]:(m_coding == NAL_CODING_HEVC? hevc_nal_unit_type_descs[nal_unit_type]:"Unknown"), cbEBSPNUBuf);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf)
		{
			printf("\t\tSEI message\n");
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
		{
			printf("\t\t\tSEI payload %s, length: %zu\n", sei_payload_type_names[payload_type], cbRBSPPayloadBuf);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf){return RET_CODE_SUCCESS;}

		RET_CODE EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			m_NUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf)
		{
			m_AUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
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

	NALEnumerator.AddRef();
	NALParser.SetEnumerator((IUnknown*)(&NALEnumerator), options);

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
			BitRates[SchedSelIdx] = ((uint64_t)hrd_parameters->bit_rate_value_minus1[SchedSelIdx] + 1) * (uint64_t)1ULL<<(6 + hrd_parameters->bit_rate_scale);
			CpbSize[SchedSelIdx] = ((uint64_t)hrd_parameters->cpb_size_value_minus1[SchedSelIdx] + 1)*(uint64_t)1ULL << (4 + hrd_parameters->cpb_size_scale);
			printf("\t\tthe maximum input bit rate for the CPB: %" PRIu64 "bps/%sbps \n", 
				BitRates[SchedSelIdx], GetHumanReadNumber(BitRates[SchedSelIdx]).c_str());
			printf("\t\tthe CPB size: %" PRIu64 "b/%sb \n",
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
			BitRates[SchedSelIdx] = ((uint64_t)hrd_parameters->bit_rate_value_minus1[SchedSelIdx] + 1) * (uint64_t)1ULL << (6 + hrd_parameters->bit_rate_scale);
			CpbSize[SchedSelIdx] = ((uint64_t)hrd_parameters->cpb_size_value_minus1[SchedSelIdx] + 1)*(uint64_t)1ULL << (4 + hrd_parameters->cpb_size_scale);
			printf("\t\tthe maximum input bit rate for the CPB: %" PRIu64 "bps/%sbps \n",
				BitRates[SchedSelIdx], GetHumanReadNumber(BitRates[SchedSelIdx]).c_str());
			printf("\t\tthe CPB size: %" PRIu64 "b/%sb \n",
				CpbSize[SchedSelIdx], GetHumanReadNumber(CpbSize[SchedSelIdx]).c_str());
			printf("\t\t%s mode\n", hrd_parameters->cbr_flag[SchedSelIdx] ? "CBR" : "VBR");
		}
	}

	return RET_CODE_SUCCESS;
}

int PrintHRDFromHEVCSPS(H265_NU sps_nu)
{
	if (!sps_nu || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	uint64_t BitRates[32] = { 0 };
	uint64_t CpbSize[32] = { 0 };
	bool bUseConcludedValues[2] = { true, true };
	if (sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->vui_hrd_parameters_present_flag &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->m_commonInfPresentFlag &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->vcl_hrd_parameters_present_flag)
	{
		printf("Type-I HRD:\n");
		auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters;
		for (uint8_t i = 0; i <= hrd_parameters->m_maxNumSubLayersMinus1; i++)
		{
			printf("\tSublayer#%d:\n", i);
			auto sub_layer_hrd_parameters = hrd_parameters->sub_layer_infos[i]->ptr_vcl_sub_layer_hrd_parameters;
			for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->sub_layer_infos[i]->cpb_cnt_minus1; SchedSelIdx++)
			{
				printf("\t\tSchedSelIdx#%d:\n", SchedSelIdx);
				BitRates[SchedSelIdx] = ((uint64_t)sub_layer_hrd_parameters->sub_layer_hrd_parameters[SchedSelIdx]->bit_rate_value_minus1 + 1) << (6 + hrd_parameters->bit_rate_scale);
				CpbSize[SchedSelIdx] = ((uint64_t)sub_layer_hrd_parameters->sub_layer_hrd_parameters[SchedSelIdx]->cpb_size_value_minus1 + 1) << (4 + hrd_parameters->cpb_size_scale);
				printf("\t\t\tthe maximum input bit rate for the CPB: %" PRIu64 "bps/%sbps \n",
					BitRates[SchedSelIdx], GetHumanReadNumber(BitRates[SchedSelIdx]).c_str());
				printf("\t\t\tthe CPB size: %" PRIu64 "b/%sb \n",
					CpbSize[SchedSelIdx], GetHumanReadNumber(CpbSize[SchedSelIdx]).c_str());
				printf("\t\t\t%s mode\n", sub_layer_hrd_parameters->sub_layer_hrd_parameters[SchedSelIdx]->cbr_flag ? "CBR" : "VBR");
			}
		}
	}

	if (sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->vui_hrd_parameters_present_flag &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->m_commonInfPresentFlag &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->nal_hrd_parameters_present_flag)
	{
		printf("Type-II HRD:\n");
		auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters;
		for (uint8_t i = 0; i <= hrd_parameters->m_maxNumSubLayersMinus1; i++)
		{
			printf("\tSublayer#%d:\n", i);
			auto sub_layer_hrd_parameters = hrd_parameters->sub_layer_infos[i]->ptr_nal_sub_layer_hrd_parameters;
			for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->sub_layer_infos[i]->cpb_cnt_minus1; SchedSelIdx++)
			{
				printf("\t\tSchedSelIdx#%d:\n", SchedSelIdx);
				BitRates[SchedSelIdx] = ((uint64_t)sub_layer_hrd_parameters->sub_layer_hrd_parameters[SchedSelIdx]->bit_rate_value_minus1 + 1) << (6 + hrd_parameters->bit_rate_scale);
				CpbSize[SchedSelIdx] = ((uint64_t)sub_layer_hrd_parameters->sub_layer_hrd_parameters[SchedSelIdx]->cpb_size_value_minus1 + 1) << (4 + hrd_parameters->cpb_size_scale);
				printf("\t\t\tthe maximum input bit rate for the CPB: %" PRIu64 "bps/%sbps \n",
					BitRates[SchedSelIdx], GetHumanReadNumber(BitRates[SchedSelIdx]).c_str());
				printf("\t\t\tthe CPB size: %" PRIu64 "b/%sb \n",
					CpbSize[SchedSelIdx], GetHumanReadNumber(CpbSize[SchedSelIdx]).c_str());
				printf("\t\t\t%s mode\n", sub_layer_hrd_parameters->sub_layer_hrd_parameters[SchedSelIdx]->cbr_flag ? "CBR" : "VBR");
			}
		}
	}

	return RET_CODE_SUCCESS;
}

int PrintHRDFromSEIPayloadBufferingPeriod(IUnknown* pCtx, uint8_t* pSEIPayload, size_t cbSEIPayload)
{
	int iRet = RET_CODE_SUCCESS;
	if (pSEIPayload == nullptr || cbSEIPayload == 0 || pCtx == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	INALContext* pNALCtx = nullptr;
	if (FAILED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
		return RET_CODE_INVALID_PARAMETER;

	AMBst bst = nullptr;
	BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD* pBufPeriod = new
		BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD((int)cbSEIPayload, pNALCtx);
	NAL_CODING coding = pNALCtx->GetNALCoding();
	
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
		H265_NU sps_nu;
		INALHEVCContext* pNALHEVCCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_INALHEVCContext, (void**)&pNALHEVCCtx)))
		{
			sps_nu = pNALHEVCCtx->GetHEVCSPS(pBufPeriod->bp_seq_parameter_set_id);

			pNALHEVCCtx->Release();
			pNALHEVCCtx = nullptr;
		}

		if (pBufPeriod->irap_cpb_params_present_flag)
		{
			printf("\tcpb_delay_offset: %" PRIu32 "\n", pBufPeriod->cpb_delay_offset);
			printf("\tdpb_delay_offset: %" PRIu32 "\n", pBufPeriod->dpb_delay_offset);
		}

		printf("\tconcatenation_flag: %d\n", (int)pBufPeriod->concatenation_flag);
		printf("\tau_cpb_removal_delay_delta: %" PRId64"\n", (int64_t)pBufPeriod->au_cpb_removal_delay_delta_minus1 + 1);

		bool sub_pic_hrd_params_present_flag = false;
		bool NalHrdBpPresentFlag = false;
		bool VclHrdBpPresentFlag = false;
		uint8_t au_cpb_removal_delay_length_minus1 = 0;
		uint8_t dpb_output_delay_length_minus1 = 0;
		uint8_t initial_cpb_removal_delay_length_minus1 = 0;
		if (sps_nu &&
			sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters &&
			sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters)
		{
			sub_pic_hrd_params_present_flag = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_pic_hrd_params_present_flag;
			au_cpb_removal_delay_length_minus1 = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->au_cpb_removal_delay_length_minus1;
			dpb_output_delay_length_minus1 = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->dpb_output_delay_length_minus1;
			initial_cpb_removal_delay_length_minus1 = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->initial_cpb_removal_delay_length_minus1;
			NalHrdBpPresentFlag = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->nal_hrd_parameters_present_flag ? true : false;
			VclHrdBpPresentFlag = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->vcl_hrd_parameters_present_flag ? true : false;
		}

		if (NalHrdBpPresentFlag)
		{
			auto pSubLayer0Info = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
			size_t CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
			for (size_t i = 0; i <= CpbCnt; i++)
			{
				printf("\tNAL CPB#%zu\n", i);
				printf("\t\tinitial_cpb_removal_delay: %" PRIu32 "\n", pBufPeriod->nal_initial_cpb_removal_info[i].initial_cpb_removal_delay);
				printf("\t\tinitial_cpb_removal_offset: %" PRIu32 "\n", pBufPeriod->nal_initial_cpb_removal_info[i].initial_cpb_removal_offset);
				if (sub_pic_hrd_params_present_flag || pBufPeriod->irap_cpb_params_present_flag)
				{
					printf("\t\tinitial_alt_cpb_removal_delay: %" PRIu32 "\n", pBufPeriod->nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay);
					printf("\t\tinitial_alt_cpb_removal_offset: %" PRIu32 "\n", pBufPeriod->nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset);
				}
			}
		}

		if (VclHrdBpPresentFlag)
		{
			auto pSubLayer0Info = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
			size_t CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
			for (size_t i = 0; i <= CpbCnt; i++)
			{
				printf("\tNAL CPB#%zu\n", i);
				printf("\t\tinitial_cpb_removal_delay: %" PRIu32 "\n", pBufPeriod->vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay);
				printf("\t\tinitial_cpb_removal_offset: %" PRIu32 "\n", pBufPeriod->vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset);
				if (sub_pic_hrd_params_present_flag || pBufPeriod->irap_cpb_params_present_flag)
				{
					printf("\t\tinitial_alt_cpb_removal_delay: %" PRIu32 "\n", pBufPeriod->vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay);
					printf("\t\tinitial_alt_cpb_removal_offset: %" PRIu32 "\n", pBufPeriod->vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset);
				}
			}
		}
	}
	else if (coding == NAL_CODING_VVC)
	{

	}

done:
	if (bst)
		AMBst_Destroy(bst);

	AMP_SAFEDEL(pBufPeriod);
	AMP_SAFERELEASE(pNALCtx);

	return iRet;
}

int PrintHRDFromSEIPayloadPicTiming(IUnknown* pCtx, uint8_t* pSEIPayload, size_t cbSEIPayload)
{
	int iRet = RET_CODE_SUCCESS;

	if (pSEIPayload == nullptr || cbSEIPayload == 0 || pCtx == nullptr)
		return RET_CODE_INVALID_PARAMETER;

	INALContext* pNALCtx = nullptr;
	if (FAILED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
		return RET_CODE_INVALID_PARAMETER;

	NAL_CODING coding = pNALCtx->GetNALCoding();

	AMBst bst = nullptr;
	bst = AMBst_CreateFromBuffer(pSEIPayload, (int)cbSEIPayload);

	BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264* pPicTiming = nullptr;
	BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H265* pPicTimingH265 = nullptr;

	if (bst == nullptr)
	{
		iRet = RET_CODE_OUTOFMEMORY;
		goto done;
	}

	printf("Picture Timing(payloadLength: %zu):\n", cbSEIPayload);
	//print_mem(pSEIPayload, (int)cbSEIPayload, 4);

	if (coding == NAL_CODING_AVC)
	{
		pPicTiming = new BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264((int)cbSEIPayload, pNALCtx);

		if (AMP_FAILED(iRet = pPicTiming->Map(bst)))
		{
			printf("Failed to unpack the SEI payload: pic_timing.\n");
			goto done;
		}

		printf("\tcpb_removal_delay: %" PRIu32 "\n", pPicTiming->cpb_removal_delay);
		printf("\tdpb_output_delay: %" PRIu32 "\n", pPicTiming->dpb_output_delay);

		if (pPicTiming->pic_struct_present_flag)
			printf("\tpic_struct: %d (%s)\n", pPicTiming->pic_struct, PIC_STRUCT_MEANING(pPicTiming->pic_struct));

		int NumClockTS = 0;
		if (pPicTiming->pic_struct >= 0 && pPicTiming->pic_struct <= 2)
			NumClockTS = 1;
		else if (pPicTiming->pic_struct == 3 || pPicTiming->pic_struct == 4 || pPicTiming->pic_struct == 7)
			NumClockTS = 2;
		else if (pPicTiming->pic_struct == 5 || pPicTiming->pic_struct == 6 || pPicTiming->pic_struct == 8)
			NumClockTS = 3;

		for (int i = 0; i < NumClockTS; i++)
		{
			printf("\tClockTS#%d:\n", i);
			printf("\t\tclock_timestamp_flag: %d\n", (int)pPicTiming->ClockTS[i].clock_timestamp_flag);
			if (pPicTiming->ClockTS[i].clock_timestamp_flag)
			{
				printf("\t\tct_type: %d(%s)\n", (int)pPicTiming->ClockTS[i].ct_type, 
					pPicTiming->ClockTS[i].ct_type == 0?"progressive":(
					pPicTiming->ClockTS[i].ct_type == 1?"interlaced":(
					pPicTiming->ClockTS[i].ct_type == 2?"unknown":"")));
				printf("\t\tnuit_field_based_flag: %d\n", (int)pPicTiming->ClockTS[i].nuit_field_based_flag);
				printf("\t\tcounting_type: %d\n", (int)pPicTiming->ClockTS[i].counting_type);
				printf("\t\tfull_timestamp_flag: %d\n", (int)pPicTiming->ClockTS[i].full_timestamp_flag);
				printf("\t\tdiscontinuity_flag: %d\n", (int)pPicTiming->ClockTS[i].discontinuity_flag);
				printf("\t\tcnt_dropped_flag: %d\n", (int)pPicTiming->ClockTS[i].cnt_dropped_flag);
				printf("\t\ttime code: %s\n", pPicTiming->ClockTS[i].GetTimeCode().c_str());
				if (pPicTiming->ClockTS[i].time_offset_length > 0)
					printf("\t\ttime offset: %" PRId64 "\n", pPicTiming->ClockTS[i].time_offset);
			}
		}
	}
	else if (coding == NAL_CODING_HEVC)
	{
		pPicTimingH265 = new BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H265((int)cbSEIPayload, pNALCtx);

		if (AMP_FAILED(iRet = pPicTimingH265->Map(bst)))
		{
			printf("Failed to unpack the SEI payload: pic_timing.\n");
			goto done;
		}

		if (pPicTimingH265->frame_field_info_present_flag)
		{
			printf("\tpic_struct: %d (%s)\n", pPicTimingH265->pic_struct, pic_struct_names[pPicTimingH265->pic_struct]);
			printf("\tsource_scan_type: %d (%s)\n", pPicTimingH265->source_scan_type, 
				pPicTimingH265->source_scan_type == 0 ? "Interlaced":(
				pPicTimingH265->source_scan_type == 1 ? "Progressive" : (
				pPicTimingH265->source_scan_type == 2 ? "Unspecified" : "")));
			printf("\tduplicate_flag: %d\n", pPicTimingH265->duplicate_flag);
		}

		if (pPicTimingH265->CpbDpbDelaysPresentFlag)
		{
			printf("\tau_cpb_removal_delay: %" PRIu64 "\n", pPicTimingH265->au_cpb_removal_delay_minus1 + 1);
			printf("\tpic_dpb_output_delay: %" PRIu64 "\n", pPicTimingH265->pic_dpb_output_delay);
			if (pPicTimingH265->sub_pic_hrd_params_present_flag)
			{
				printf("\tpic_dpb_output_du_delay: %" PRIu64 "\n", pPicTimingH265->pic_dpb_output_du_delay);
				if (pPicTimingH265->sub_pic_cpb_params_in_pic_timing_sei_flag)
				{
					printf("\tnum_decoding_units: %" PRIu64 "\n", pPicTimingH265->num_decoding_units_minus1 + 1);
					printf("\tdu_common_cpb_removal_delay_flag: %d\n", (int)pPicTimingH265->du_common_cpb_removal_delay_flag);

					if (pPicTimingH265->du_common_cpb_removal_delay_flag)
					{
						printf("\tdu_common_cpb_removal_delay_increment: %" PRIu64 "\n", pPicTimingH265->du_common_cpb_removal_delay_increment_minus1 + 1);
					}

					for (size_t i = 0; i <= (size_t)pPicTimingH265->num_decoding_units_minus1; i++)
					{
						printf("\tdu#%zu:\n", i);
						printf("\t\tnum_nalus_in_du: %" PRIu64 "\n", pPicTimingH265->dus[i].num_nalus_in_du_minus1 + 1);
						if (!pPicTimingH265->du_common_cpb_removal_delay_flag && i < pPicTimingH265->num_decoding_units_minus1)
						{
							printf("\t\tdu_cpb_removal_delay_increment: %" PRIu64 "\n", pPicTimingH265->dus[i].du_cpb_removal_delay_increment_minus1 + 1);
						}
					}
				}
			}
		}
	}
	else
	{
		// TODO...
	}

done:
	if (bst)
		AMBst_Destroy(bst);

	AMP_SAFEDEL(pPicTiming);
	AMP_SAFEDEL(pPicTimingH265);
	AMP_SAFERELEASE(pNALCtx);

	return iRet;
}

void PrintAVCSPSRoughInfo(H264_NU sps_nu)
{
	if (!sps_nu || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
		return;

	auto& sps_seq = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data;

	printf("A new H.264 sequence(seq_parameter_set_id:%d):\n", sps_seq.seq_parameter_set_id);
	printf("\tAVC Profile: %s\n", get_h264_profile_name(sps_seq.GetH264Profile()));
	printf("\tAVC Level: %s\n", get_h264_level_name(sps_seq.GetH264Level()));
	printf("\tChroma: %s\n", chroma_format_idc_names[sps_seq.chroma_format_idc]);

	printf("\tScan type: %s\n", sps_seq.frame_mbs_only_flag?"Progressive":"Interlaced");

	uint8_t SubWidthC = (sps_seq.chroma_format_idc == 1 || sps_seq.chroma_format_idc == 2) ? 2 : (sps_seq.chroma_format_idc == 3 && sps_seq.separate_colour_plane_flag == 0 ? 1 : 0);
	uint8_t SubHeightC = (sps_seq.chroma_format_idc == 2 || (sps_seq.chroma_format_idc == 3 && sps_seq.separate_colour_plane_flag == 0)) ? 1 : (sps_seq.chroma_format_idc == 1 ? 2 : 0);

	uint16_t PicWidthInMbs = sps_seq.pic_width_in_mbs_minus1 + 1;
	uint32_t PicWidthInSamplesL = PicWidthInMbs * 16;
	//uint32_t PicWidthInSamplesC = PicWidthInMbs * MbWidthC;
	uint16_t PicHeightInMapUnits = sps_seq.pic_height_in_map_units_minus1 + 1;
	uint32_t PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits;
	uint16_t FrameHeightInMbs = (2 - sps_seq.frame_mbs_only_flag) * PicHeightInMapUnits;
	uint8_t ChromaArrayType = sps_seq.separate_colour_plane_flag == 0 ? sps_seq.chroma_format_idc : 0;
	uint8_t CropUnitX = ChromaArrayType == 0 ? 1 : SubWidthC;
	uint8_t CropUnitY = ChromaArrayType == 0 ? (2 - sps_seq.frame_mbs_only_flag) : SubHeightC * (2 - sps_seq.frame_mbs_only_flag);

	uint32_t frame_buffer_width = PicWidthInSamplesL, frame_buffer_height = FrameHeightInMbs * 16;
	uint32_t display_width = frame_buffer_width, display_height = frame_buffer_height;

	if (sps_seq.frame_cropping_flag)
	{
		uint32_t crop_unit_x = 0, crop_unit_y = 0;
		if (0 == sps_seq.chroma_format_idc)	// monochrome
		{
			crop_unit_x = 1;
			crop_unit_y = 2 - sps_seq.frame_mbs_only_flag;
		}
		else if (1 == sps_seq.chroma_format_idc)	// 4:2:0
		{
			crop_unit_x = 2;
			crop_unit_y = 2 * (2 - sps_seq.frame_mbs_only_flag);
		}
		else if (2 == sps_seq.chroma_format_idc)	// 4:2:2
		{
			crop_unit_x = 2;
			crop_unit_y = 2 - sps_seq.frame_mbs_only_flag;
		}
		else if (3 == sps_seq.chroma_format_idc)
		{
			crop_unit_x = 1;
			crop_unit_y = 2 - sps_seq.frame_mbs_only_flag;
		}

		display_width -= crop_unit_x * (sps_seq.frame_crop_left_offset + sps_seq.frame_crop_right_offset);
		display_height -= crop_unit_y * (sps_seq.frame_crop_top_offset + sps_seq.frame_crop_bottom_offset);
	}

	printf("\tCoded Frame resolution: %" PRIu32 "x%" PRIu32 "\n", frame_buffer_width, frame_buffer_height);
	printf("\tDisplay resolution: %" PRIu32 "x%" PRIu32 "\n", display_width, display_height);

	if (sps_seq.vui_parameters_present_flag && sps_seq.vui_parameters)
	{
		auto vui_parameters = sps_seq.vui_parameters;

		if (vui_parameters->aspect_ratio_info_present_flag)
		{
			if (vui_parameters->aspect_ratio_idc == 0xFF)
				printf("\tSample Aspect-Ratio: %" PRIu16 "x%" PRIu16 "\n", vui_parameters->sar_width, vui_parameters->sar_height);
			else
				printf("\tSample Aspect-Ratio: %s\n", sample_aspect_ratio_descs[vui_parameters->aspect_ratio_idc]);
		}

		if (vui_parameters->video_signal_type_present_flag && vui_parameters->colour_description_present_flag)
		{
			printf("\tColour Primaries: %d(%s)\n", vui_parameters->colour_primaries, vui_colour_primaries_names[vui_parameters->colour_primaries]);
			printf("\tTransfer Characteristics: %d(%s)\n", vui_parameters->transfer_characteristics, vui_transfer_characteristics_names[vui_parameters->transfer_characteristics]);
			printf("\tMatrix Coeffs: %d(%s)\n", vui_parameters->matrix_coeffs, vui_matrix_coeffs_descs[vui_parameters->matrix_coeffs]);
		}

		uint32_t units_field_based_flag = 1;		// For H264, default value is 1
		if (vui_parameters->timing_info_present_flag)
		{
			if (vui_parameters->time_scale > 0)
			{
				float frame_rate = (float)vui_parameters->time_scale / (vui_parameters->num_units_in_tick * (units_field_based_flag + 1));
				printf("\tFrame-Rate: %f fps\n", frame_rate);
			}
		}
	}

	return;
}

void PrintHEVCSPSRoughInfo(H265_NU sps_nu)
{
	if (!sps_nu || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
		return;

	auto sps_seq = sps_nu->ptr_seq_parameter_set_rbsp;

	if (sps_seq == nullptr ||
		sps_seq->profile_tier_level == nullptr ||
		!sps_seq->profile_tier_level->general_profile_level.profile_present_flag)
		return;

	const char* szProfileName = get_hevc_profile_name(sps_seq->profile_tier_level->GetHEVCProfile());
	printf("A new H.265 sequence(seq_parameter_set_id:%d):\n", sps_seq->sps_seq_parameter_set_id);
	printf("\tHEVC Profile: %s\n", szProfileName);
	printf("\tTiger: %s\n", sps_seq->profile_tier_level->general_profile_level.tier_flag?"High":"Main");

	if (sps_seq->profile_tier_level->general_profile_level.level_present_flag)
		printf("\tLevel %d.%d\n", (int)(sps_seq->profile_tier_level->general_profile_level.level_idc / 30), (int)(sps_seq->profile_tier_level->general_profile_level.level_idc % 30 / 3));

	printf("\tChroma: %s\n", chroma_format_idc_names[sps_seq->chroma_format_idc]);

	if (sps_seq->vui_parameters_present_flag &&
		sps_seq->vui_parameters)
		printf("\tScan type: %s\n", sps_seq->vui_parameters->field_seq_flag ? "Interlaced" : "Progressive");

	uint32_t display_width = sps_seq->pic_width_in_luma_samples, display_height = sps_seq->pic_height_in_luma_samples;
	if (sps_seq->conformance_window_flag)
	{
		uint32_t sub_width_c = ((1 == sps_seq->chroma_format_idc) || (2 == sps_seq->chroma_format_idc)) && (0 == sps_seq->separate_colour_plane_flag) ? 2 : 1;
		uint32_t sub_height_c = (1 == sps_seq->chroma_format_idc) && (0 == sps_seq->separate_colour_plane_flag) ? 2 : 1;
		display_width -= sub_width_c * (sps_seq->conf_win_left_offset + sps_seq->conf_win_right_offset);
		display_height = sub_height_c * (sps_seq->conf_win_top_offset + sps_seq->conf_win_bottom_offset);
	}

	printf("\tCoded Frame resolution: %" PRIu32 "x%" PRIu32 "\n", sps_seq->pic_width_in_luma_samples, sps_seq->pic_height_in_luma_samples);
	printf("\tDisplay resolution: %" PRIu32 "x%" PRIu32 "\n", display_width, display_height);

	if (sps_seq->vui_parameters_present_flag && sps_seq->vui_parameters)
	{
		auto vui_parameters = sps_seq->vui_parameters;

		if (vui_parameters->aspect_ratio_info_present_flag)
		{
			if (vui_parameters->aspect_ratio_idc == 0xFF)
				printf("\tSample Aspect-Ratio: %" PRIu16 "x%" PRIu16 "\n", vui_parameters->sar_width, vui_parameters->sar_height);
			else
				printf("\tSample Aspect-Ratio: %s\n", sample_aspect_ratio_descs[vui_parameters->aspect_ratio_idc]);
		}

		if (vui_parameters->video_signal_type_present_flag && vui_parameters->colour_description_present_flag)
		{
			printf("\tColour Primaries: %d(%s)\n", vui_parameters->colour_primaries, vui_colour_primaries_names[vui_parameters->colour_primaries]);
			printf("\tTransfer Characteristics: %d(%s)\n", vui_parameters->transfer_characteristics, vui_transfer_characteristics_names[vui_parameters->transfer_characteristics]);
			printf("\tMatrix Coeffs: %d(%s)\n", vui_parameters->matrix_coeffs, vui_matrix_coeffs_descs[vui_parameters->matrix_coeffs]);
		}

		uint32_t units_field_based_flag = 1;		// For H264, default value is 1
		if (vui_parameters->vui_timing_info_present_flag)
		{
			if (vui_parameters->vui_time_scale > 0)
			{
				float frame_rate = (float)vui_parameters->vui_time_scale / (vui_parameters->vui_num_units_in_tick * (vui_parameters->field_seq_flag + 1));
				printf("\tFrame-Rate: %f fps\n", frame_rate);
			}
		}
	}

	return;
}

int GetStreamInfoFromSPS(NAL_CODING coding, uint8_t* pAnnexBBuf, size_t cbAnnexBBuf, STREAM_INFO& stm_info)
{
	INALContext* pNALContext = nullptr;
	int iRet = RET_CODE_ERROR;

	if (cbAnnexBBuf >= INT64_MAX)
		return RET_CODE_BUFFER_OVERFLOW;

	CNALParser NALParser(coding);
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(NALParser.GetContext(&pMSECtx)) || 
		FAILED(pMSECtx->QueryInterface(IID_INALContext, (void**)&pNALContext)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the %s NAL context.\n", NAL_CODING_NAME(coding));
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	if (coding == NAL_CODING_AVC)
		pNALContext->SetNUFilters({ BST::H264Video::SPS_NUT });
	else if (coding == NAL_CODING_HEVC)
		pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT, BST::H265Video::SPS_NUT });
	else if (coding == NAL_CODING_VVC)
		pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT, BST::H266Video::SPS_NUT });

	class CNALEnumerator : public CComUnknown, public INALEnumerator
	{
	public:
		CNALEnumerator(INALContext* pNALCtx) : m_pNALContext(pNALCtx) {
			if (m_pNALContext)
				m_pNALContext->AddRef();
			m_coding = m_pNALContext->GetNALCoding();
			if (m_coding == NAL_CODING_AVC)
				m_pNALContext->QueryInterface(IID_INALAVCContext, (void**)&m_pNALAVCContext);
			else if (m_coding == NAL_CODING_HEVC)
				m_pNALContext->QueryInterface(IID_INALHEVCContext, (void**)&m_pNALHEVCContext);
		}

		virtual ~CNALEnumerator() {
			AMP_SAFERELEASE(m_pNALAVCContext);
			AMP_SAFERELEASE(m_pNALHEVCContext);
			AMP_SAFERELEASE(m_pNALContext);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_INALEnumerator)
				return GetCOMInterface((INALEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumNewVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type) { return RET_CODE_SUCCESS; }

		RET_CODE EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
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

						m_spsid = nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.seq_parameter_set_id;

						m_pNALAVCContext->UpdateAVCSPS(nu);
					}
					else if (nal_unit_type == BST::H264Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALAVCContext->UpdateAVCPPS(nu);
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
					}
					else if (nal_unit_type == BST::H265Video::SPS_NUT)
					{
						if (memcmp(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_spsid = nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id;

						m_pNALHEVCContext->UpdateHEVCSPS(nu);
					}
					else if (nal_unit_type == BST::H265Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCPPS(nu);
					}
				}
			}

		done:
			if (bst)
				AMBst_Destroy(bst);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			m_NUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
		{
			printf("Hitting error {error_code: %d}.\n", error_code);
			return RET_CODE_SUCCESS;
		}

		int16_t GetSPSID() { return m_spsid; }

		INALContext* m_pNALContext = nullptr;
		INALAVCContext* m_pNALAVCContext = nullptr;
		INALHEVCContext* m_pNALHEVCContext = nullptr;
		uint64_t m_NUCount = 0;
		NAL_CODING m_coding = NAL_CODING_UNKNOWN;
		AMSHA1_RET prev_sps_sha1_ret[32] = { {0} };
		AMSHA1_RET prev_vps_sha1_ret[32] = { {0} };
		// H.264, there may be 256 pps at maximum; H.265/266: there may be 64 pps at maximum
		AMSHA1_RET prev_pps_sha1_ret[256] = { {0} };
		int16_t m_spsid = -1;
	}NALEnumerator(pNALContext);

	NALEnumerator.AddRef();
	NALParser.SetEnumerator((IUnknown*)(&NALEnumerator), NAL_ENUM_OPTION_NU);

	uint8_t* p = pAnnexBBuf;
	int64_t cbLeft = (int64_t)cbAnnexBBuf;
	INALAVCContext* pNALAVCContext = nullptr;
	INALHEVCContext* pNALHEVCContext = nullptr;

	if (coding == NAL_CODING_AVC)
	{
		if (FAILED(pNALContext->QueryInterface(IID_INALAVCContext, (void**)&pNALAVCContext)))
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}
	}
	else if (coding == NAL_CODING_HEVC)
	{
		if (FAILED(pNALContext->QueryInterface(IID_INALHEVCContext, (void**)&pNALHEVCContext)))
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}
	}

	while (cbLeft > 0)
	{
		int64_t cbSubmit = AMP_MIN(cbLeft, 2048);
		if (AMP_SUCCEEDED(NALParser.ProcessInput(p, (size_t)cbSubmit)))
		{
			NALParser.ProcessOutput(cbLeft <= 2048 ? true : false);
			if (coding == NAL_CODING_AVC)
			{
				int16_t sps_id = NALEnumerator.GetSPSID();
				if (sps_id >= 0 && sps_id <= UINT8_MAX)
				{
					auto sps_nu = pNALAVCContext->GetAVCSPS((uint8_t)sps_id);
					if (sps_nu &&
						sps_nu->ptr_seq_parameter_set_rbsp)
					{
						auto& sps_seq = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data;

						stm_info.video_info.profile = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.GetH264Profile();
						stm_info.video_info.tier = -1;
						stm_info.video_info.level = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.GetH264Level();

						uint8_t SubWidthC = (sps_seq.chroma_format_idc == 1 || sps_seq.chroma_format_idc == 2) ? 2 : (sps_seq.chroma_format_idc == 3 && sps_seq.separate_colour_plane_flag == 0 ? 1 : 0);
						uint8_t SubHeightC = (sps_seq.chroma_format_idc == 2 || (sps_seq.chroma_format_idc == 3 && sps_seq.separate_colour_plane_flag == 0)) ? 1 : (sps_seq.chroma_format_idc == 1 ? 2 : 0);

						uint16_t PicWidthInMbs = sps_seq.pic_width_in_mbs_minus1 + 1;
						uint32_t PicWidthInSamplesL = PicWidthInMbs * 16;
						//uint32_t PicWidthInSamplesC = PicWidthInMbs * MbWidthC;
						uint16_t PicHeightInMapUnits = sps_seq.pic_height_in_map_units_minus1 + 1;
						uint32_t PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits;
						uint16_t FrameHeightInMbs = (2 - sps_seq.frame_mbs_only_flag) * PicHeightInMapUnits;
						uint8_t ChromaArrayType = sps_seq.separate_colour_plane_flag == 0 ? sps_seq.chroma_format_idc : 0;
						uint8_t CropUnitX = ChromaArrayType == 0 ? 1 : SubWidthC;
						uint8_t CropUnitY = ChromaArrayType == 0 ? (2 - sps_seq.frame_mbs_only_flag) : SubHeightC * (2 - sps_seq.frame_mbs_only_flag);

						uint32_t frame_buffer_width = PicWidthInSamplesL, frame_buffer_height = FrameHeightInMbs * 16;
						uint32_t display_width = frame_buffer_width, display_height = frame_buffer_height;

						if (sps_seq.frame_cropping_flag)
						{
							uint32_t crop_unit_x = 0, crop_unit_y = 0;
							if (0 == sps_seq.chroma_format_idc)	// monochrome
							{
								crop_unit_x = 1;
								crop_unit_y = 2 - sps_seq.frame_mbs_only_flag;
							}
							else if (1 == sps_seq.chroma_format_idc)	// 4:2:0
							{
								crop_unit_x = 2;
								crop_unit_y = 2 * (2 - sps_seq.frame_mbs_only_flag);
							}
							else if (2 == sps_seq.chroma_format_idc)	// 4:2:2
							{
								crop_unit_x = 2;
								crop_unit_y = 2 - sps_seq.frame_mbs_only_flag;
							}
							else if (3 == sps_seq.chroma_format_idc)
							{
								crop_unit_x = 1;
								crop_unit_y = 2 - sps_seq.frame_mbs_only_flag;
							}

							display_width -= crop_unit_x * (sps_seq.frame_crop_left_offset + sps_seq.frame_crop_right_offset);
							display_height -= crop_unit_y * (sps_seq.frame_crop_top_offset + sps_seq.frame_crop_bottom_offset);
						}

						stm_info.video_info.video_width = display_width;
						stm_info.video_info.video_height = display_height;

						if (sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters_present_flag &&
							sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters)
						{
							auto vui_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters;
							if (vui_parameters->nal_hrd_parameters_present_flag &&
								vui_parameters->nal_hrd_parameters)
							{
								stm_info.video_info.bitrate = (vui_parameters->nal_hrd_parameters->bit_rate_value_minus1[0] + 1) << (vui_parameters->nal_hrd_parameters->bit_rate_scale + 6);
							}
							else if (vui_parameters->vcl_hrd_parameters_present_flag &&
								vui_parameters->vcl_hrd_parameters)
							{
								stm_info.video_info.bitrate = (vui_parameters->vcl_hrd_parameters->bit_rate_value_minus1[0] + 1) << (vui_parameters->vcl_hrd_parameters->bit_rate_scale + 6);
							}

							if (vui_parameters->aspect_ratio_info_present_flag)
							{
								uint16_t SAR_N = 0, SAR_D = 0;
								if (vui_parameters->aspect_ratio_idc == 0xFF)
								{
									SAR_N = vui_parameters->sar_width;
									SAR_D = vui_parameters->sar_height;
								}
								else if (vui_parameters->aspect_ratio_idc >= 0 && vui_parameters->aspect_ratio_idc < sizeof(sample_aspect_ratios) / sizeof(sample_aspect_ratios[0]))
								{
									SAR_N = std::get<0>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
									SAR_D = std::get<1>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
								}

								if (SAR_N != 0 && SAR_D != 0)
								{
									float diff_4_3 = fabs(((float)SAR_N*display_width) / ((float)SAR_D*display_height) - 4.0f / 3.0f);
									float diff_16_9 = fabs(((float)SAR_N*display_width) / ((float)SAR_D*display_height) - 16.0f / 9.0f);

									stm_info.video_info.aspect_ratio_numerator = diff_4_3 < diff_16_9 ? 4 : 16;
									stm_info.video_info.aspect_ratio_denominator = diff_4_3 < diff_16_9 ? 3 : 9;
								}
							}

							if (vui_parameters->video_signal_type_present_flag && vui_parameters->colour_description_present_flag)
							{
								stm_info.video_info.transfer_characteristics = vui_parameters->transfer_characteristics;
								stm_info.video_info.colour_primaries = vui_parameters->colour_primaries;
							}

							stm_info.video_info.chroma_format_idc = sps_seq.chroma_format_idc;

							if (vui_parameters->timing_info_present_flag && vui_parameters->fixed_frame_rate_flag)
							{
								uint64_t GCD = gcd((uint64_t)vui_parameters->num_units_in_tick * 2, vui_parameters->time_scale);
								stm_info.video_info.framerate_numerator = (uint32_t)(vui_parameters->time_scale / GCD);
								stm_info.video_info.framerate_denominator = (uint32_t)((uint64_t)vui_parameters->num_units_in_tick * 2 / GCD);
							}
						}

						iRet = RET_CODE_SUCCESS;
						break;
					}
				}
			}
			else if (coding == NAL_CODING_HEVC)
			{
				int16_t sps_id = NALEnumerator.GetSPSID();
				if (sps_id >= 0 && sps_id <= UINT8_MAX)
				{
					auto sps_nu = pNALHEVCContext->GetHEVCSPS((uint8_t)sps_id);
					if (sps_nu &&
						sps_nu->ptr_seq_parameter_set_rbsp)
					{
						auto sps_seq = sps_nu->ptr_seq_parameter_set_rbsp;

						stm_info.video_info.profile = BST::H265Video::HEVC_PROFILE_UNKNOWN;
						stm_info.video_info.tier = BST::H265Video::HEVC_TIER_UNKNOWN;
						if (sps_seq->profile_tier_level)
						{
							if (sps_seq->profile_tier_level->general_profile_level.profile_present_flag)
							{
								stm_info.video_info.profile = sps_seq->profile_tier_level->GetHEVCProfile();
								stm_info.video_info.tier = sps_seq->profile_tier_level->general_profile_level.tier_flag;
							}

							stm_info.video_info.level = sps_seq->profile_tier_level->general_profile_level.level_idc;
						}

						uint32_t display_width = sps_seq->pic_width_in_luma_samples, display_height = sps_seq->pic_height_in_luma_samples;
						if (sps_seq->conformance_window_flag)
						{
							uint32_t sub_width_c = ((1 == sps_seq->chroma_format_idc) || (2 == sps_seq->chroma_format_idc)) && (0 == sps_seq->separate_colour_plane_flag) ? 2 : 1;
							uint32_t sub_height_c = (1 == sps_seq->chroma_format_idc) && (0 == sps_seq->separate_colour_plane_flag) ? 2 : 1;
							display_width -= sub_width_c * (sps_seq->conf_win_left_offset + sps_seq->conf_win_right_offset);
							display_height = sub_height_c * (sps_seq->conf_win_top_offset + sps_seq->conf_win_bottom_offset);
						}

						stm_info.video_info.video_width = display_width;
						stm_info.video_info.video_height = display_height;

						if (sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters_present_flag &&
							sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters)
						{
							auto vui_parameters = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters;
							if (vui_parameters->vui_hrd_parameters_present_flag &&
								vui_parameters->hrd_parameters &&
								vui_parameters->hrd_parameters->m_commonInfPresentFlag &&
								vui_parameters->hrd_parameters->sub_layer_infos)
							{
								if (vui_parameters->hrd_parameters->nal_hrd_parameters_present_flag)
								{
									auto ptr_nal_sub_layer_hrd_parameters = vui_parameters->hrd_parameters->sub_layer_infos[0]->ptr_nal_sub_layer_hrd_parameters;
									stm_info.video_info.bitrate = 
										(ptr_nal_sub_layer_hrd_parameters->sub_layer_hrd_parameters[0]->bit_rate_value_minus1 + 1) << (vui_parameters->hrd_parameters->bit_rate_scale + 6);
								}
								else if (vui_parameters->hrd_parameters->vcl_hrd_parameters_present_flag)
								{
									auto ptr_vcl_sub_layer_hrd_parameters = vui_parameters->hrd_parameters->sub_layer_infos[0]->ptr_vcl_sub_layer_hrd_parameters;
									stm_info.video_info.bitrate = 
										(ptr_vcl_sub_layer_hrd_parameters->sub_layer_hrd_parameters[0]->bit_rate_value_minus1 + 1) << (vui_parameters->hrd_parameters->bit_rate_scale + 6);
								}
							}

							if (vui_parameters->aspect_ratio_info_present_flag)
							{
								uint16_t SAR_N = 0, SAR_D = 0;
								if (vui_parameters->aspect_ratio_idc == 0xFF)
								{
									SAR_N = vui_parameters->sar_width;
									SAR_D = vui_parameters->sar_height;
								}
								else if (vui_parameters->aspect_ratio_idc >= 0 && vui_parameters->aspect_ratio_idc < sizeof(sample_aspect_ratios) / sizeof(sample_aspect_ratios[0]))
								{
									SAR_N = std::get<0>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
									SAR_D = std::get<1>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
								}

								if (SAR_N != 0 && SAR_D != 0)
								{
									float diff_4_3 = fabs(((float)SAR_N*display_width) / ((float)SAR_D*display_height) - 4.0f / 3.0f);
									float diff_16_9 = fabs(((float)SAR_N*display_width) / ((float)SAR_D*display_height) - 16.0f / 9.0f);

									stm_info.video_info.aspect_ratio_numerator = diff_4_3 < diff_16_9 ? 4 : 16;
									stm_info.video_info.aspect_ratio_denominator = diff_4_3 < diff_16_9 ? 3 : 9;
								}
							}

							if (vui_parameters->video_signal_type_present_flag && vui_parameters->colour_description_present_flag)
							{
								stm_info.video_info.transfer_characteristics = vui_parameters->transfer_characteristics;
								stm_info.video_info.colour_primaries = vui_parameters->colour_primaries;
							}

							stm_info.video_info.chroma_format_idc = sps_seq->chroma_format_idc;

							if (vui_parameters->vui_timing_info_present_flag)
							{
								uint64_t GCD = gcd((uint64_t)vui_parameters->vui_num_units_in_tick * (1ULL + vui_parameters->field_seq_flag), vui_parameters->vui_time_scale);
								stm_info.video_info.framerate_numerator = (uint32_t)(vui_parameters->vui_time_scale / GCD);
								stm_info.video_info.framerate_denominator = (uint32_t)((uint64_t)vui_parameters->vui_num_units_in_tick * (1ULL + vui_parameters->field_seq_flag) / GCD);
							}
						}

						iRet = RET_CODE_SUCCESS;
						break;
					}
				}
			}

			cbLeft -= cbSubmit;
			p += cbSubmit;
		}
		else
			break;
	}

done:
	if (pNALAVCContext)
	{
		pNALAVCContext->Release();
		pNALAVCContext = nullptr;
	}

	if (pNALHEVCContext)
	{
		pNALHEVCContext->Release();
		pNALHEVCContext = nullptr;
	}

	if (pNALContext)
	{
		pNALContext->Release();
		pNALContext = nullptr;
	}

	return iRet;
}

/*
	 1: VPS
	 2: SPS
	 3: PPS
	12: HRD
	81: rough VPS
	82: rough SPS
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

	int top = GetTopRecordCount();

	CNALParser NALParser(coding);
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(NALParser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_INALContext, (void**)&pNALContext)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the %s NAL context.\n", NAL_CODING_NAME(coding));
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	if (coding == NAL_CODING_AVC)
	{
		if (object_type == 1)
		{
			printf("No VPS object in H.264 stream.\n");
			return RET_CODE_INVALID_PARAMETER;
		}
		else if (object_type == 2 || object_type == 82)
			pNALContext->SetNUFilters({ BST::H264Video::SPS_NUT });
		else if (object_type == 3)
			pNALContext->SetNUFilters({ BST::H264Video::SPS_NUT, BST::H264Video::PPS_NUT });
	}
	else if (coding == NAL_CODING_HEVC)
	{
		if (object_type == 1 || object_type == 81)
			pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT });
		else if (object_type == 2 || object_type == 82)
			pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT, BST::H265Video::SPS_NUT });
		else if (object_type == 3)
			pNALContext->SetNUFilters({ BST::H265Video::VPS_NUT, BST::H265Video::SPS_NUT, BST::H265Video::PPS_NUT });
	}
	else if (coding == NAL_CODING_VVC)
	{
		if (object_type == 1 || object_type == 81)
			pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT });
		else if (object_type == 2 || object_type == 82)
			pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT, BST::H266Video::SPS_NUT });
		else if (object_type == 3)
			pNALContext->SetNUFilters({ BST::H266Video::VPS_NUT, BST::H266Video::SPS_NUT, BST::H266Video::PPS_NUT });
	}

	class CNALEnumerator : public CComUnknown, public INALEnumerator
	{
	public:
		CNALEnumerator(INALContext* pNALCtx, int objType) : m_pNALContext(pNALCtx), object_type(objType) {
			m_coding = m_pNALContext->GetNALCoding();
			if (m_pNALContext)
				m_pNALContext->AddRef();
			if (m_coding == NAL_CODING_AVC)
				m_pNALContext->QueryInterface(IID_INALAVCContext, (void**)&m_pNALAVCContext);
			else if (m_coding == NAL_CODING_HEVC)
				m_pNALContext->QueryInterface(IID_INALHEVCContext, (void**)&m_pNALHEVCContext);
		}

		virtual ~CNALEnumerator() {
			AMP_SAFERELEASE(m_pNALAVCContext);
			AMP_SAFERELEASE(m_pNALHEVCContext);
			AMP_SAFERELEASE(m_pNALContext);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_INALEnumerator)
				return GetCOMInterface((INALEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}
		RET_CODE EnumNewVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }

		RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type) { return RET_CODE_SUCCESS; }

		RET_CODE EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type){return RET_CODE_SUCCESS;}

		RET_CODE EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
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
							PrintMediaObject(nu.get());
						else if (object_type == 82)
							PrintAVCSPSRoughInfo(nu);
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
							PrintMediaObject(nu.get());
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
							PrintMediaObject(nu.get());
					}
					else if (nal_unit_type == BST::H265Video::SPS_NUT)
					{
						if (memcmp(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCSPS(nu);

						if (object_type == 2)
							PrintMediaObject(nu.get());
						else if (object_type == 82)
							PrintHEVCSPSRoughInfo(nu);
						else if (object_type == 12)
							PrintHRDFromHEVCSPS(nu);
					}
					else if (nal_unit_type == BST::H265Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCPPS(nu);

						if (object_type == 3)
							PrintMediaObject(nu.get());
					}
				}
			}

		done:
			if (bst)
				AMBst_Destroy(bst);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
		{
			if (payload_type == SEI_PAYLOAD_BUFFERING_PERIOD)
			{
				if (object_type == 12)
				{
					// Try to activate SPS
					if (cbRBSPPayloadBuf > 0)
					{
						AMBst bst = AMBst_CreateFromBuffer(pRBSPSEIPayloadBuf, (int)cbRBSPPayloadBuf);
						try
						{
							uint64_t bp_seq_parameter_set_id = AMBst_Get_ue(bst);
							if (bp_seq_parameter_set_id >= 0 && bp_seq_parameter_set_id <= 15)
							{
								if (m_coding == NAL_CODING_HEVC && m_pNALHEVCContext)
									m_pNALHEVCContext->ActivateSPS((int8_t)bp_seq_parameter_set_id);
								else if (m_coding == NAL_CODING_AVC && m_pNALAVCContext)
									m_pNALAVCContext->ActivateSPS((int8_t)bp_seq_parameter_set_id);
							}
						}
						catch (...)
						{

						}
					}

					PrintHRDFromSEIPayloadBufferingPeriod(pCtx, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
				}
			}
			else if (payload_type == SEI_PAYLOAD_PIC_TIMING)
			{
				if (object_type == 12)
					PrintHRDFromSEIPayloadPicTiming(pCtx, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
			}
			else if (payload_type == SEI_PAYLOAD_ACTIVE_PARAMETER_SETS && m_coding == NAL_CODING_HEVC && m_pNALHEVCContext)
			{
				if (object_type == 12 && cbRBSPPayloadBuf > 0)
				{
					AMBst bst = AMBst_CreateFromBuffer(pRBSPSEIPayloadBuf, (int)cbRBSPPayloadBuf);
					try
					{
						AMBst_SkipBits(bst, 6);
						uint64_t num_sps_ids_minus1 = AMBst_Get_ue(bst);
						uint64_t active_seq_parameter_set_id0 = AMBst_Get_ue(bst);

						if (active_seq_parameter_set_id0 >= 0 && active_seq_parameter_set_id0 <= 15)
						{
							m_pNALHEVCContext->ActivateSPS((int8_t)active_seq_parameter_set_id0);
						}
					}
					catch (...)
					{

					}
					
					AMBst_Destroy(bst);
				}
			}
			return RET_CODE_SUCCESS;
		}
		RET_CODE EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			m_NUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf){return RET_CODE_SUCCESS;}
		RET_CODE EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
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

	NALEnumerator.AddRef();
	NALParser.SetEnumerator((IUnknown*)(&NALEnumerator), object_type == 12?NAL_ENUM_OPTION_ALL:NAL_ENUM_OPTION_NU);

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

int ShowNALInfo()
{
	return ShowNALObj(82);
}

int ShowPPS()
{
	return ShowNALObj(3);
}

int	ShowHRD()
{
	return ShowNALObj(12);
}

// At present, only support Type-II HRD
int RunH264HRD()
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

	int top = GetTopRecordCount();

	CNALParser NALParser(coding);
	IUnknown* pMSECtx = nullptr;
	if (AMP_FAILED(NALParser.GetContext(&pMSECtx)) ||
		FAILED(pMSECtx->QueryInterface(IID_INALContext, (void**)&pNALContext)))
	{
		AMP_SAFERELEASE(pMSECtx);
		printf("Failed to get the %s NAL context.\n", NAL_CODING_NAME(coding));
		return RET_CODE_ERROR_NOTIMPL;
	}
	AMP_SAFERELEASE(pMSECtx);

	class CHRDRunner : public CComUnknown, public INALEnumerator
	{
	public:
		CHRDRunner(INALContext* pNALCtx) : m_pNALContext(pNALCtx) {
			if (m_pNALContext)
				m_pNALContext->AddRef();
			m_coding = m_pNALContext->GetNALCoding();
			if (m_coding == NAL_CODING_AVC)
				m_pNALContext->QueryInterface(IID_INALAVCContext, (void**)&m_pNALAVCContext);
			else if (m_coding == NAL_CODING_HEVC)
				m_pNALContext->QueryInterface(IID_INALHEVCContext, (void**)&m_pNALHEVCContext);
		}

		virtual ~CHRDRunner() {
			AMP_SAFERELEASE(m_pNALAVCContext);
			AMP_SAFERELEASE(m_pNALHEVCContext);
			AMP_SAFERELEASE(m_pNALContext);
		}

		DECLARE_IUNKNOWN
		HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
		{
			if (ppvObj == NULL)
				return E_POINTER;

			if (uuid == IID_INALEnumerator)
				return GetCOMInterface((INALEnumerator*)this, ppvObj);

			return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
		}

	public:
		RET_CODE EnumNewVSEQ(IUnknown* pCtx) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNewCVS(IUnknown* pCtx, int8_t represent_nal_unit_type) { return RET_CODE_SUCCESS; }

		RET_CODE EnumNALAUBegin(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf, int picture_slice_type)
		{ 
			bHaveSEIBufferingPeriod = false;
			return RET_CODE_SUCCESS; 
		}

		RET_CODE EnumNALUnitBegin(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
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
					}
					else if (nal_unit_type == BST::H264Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALAVCContext->UpdateAVCPPS(nu);
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
					}
					else if (nal_unit_type == BST::H265Video::SPS_NUT)
					{
						if (memcmp(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_sps_sha1_ret[nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCSPS(nu);
					}
					else if (nal_unit_type == BST::H265Video::PPS_NUT)
					{
						if (memcmp(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET)) == 0)
							goto done;
						else
							memcpy(prev_pps_sha1_ret[nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id], sha1_ret, sizeof(AMSHA1_RET));

						m_pNALHEVCContext->UpdateHEVCPPS(nu);
					}
				}
			}

		done:
			if (bst)
				AMBst_Destroy(bst);
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALSEIMessageBegin(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALSEIPayloadBegin(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf)
		{
			if (payload_type == SEI_PAYLOAD_BUFFERING_PERIOD)
			{
				BeginBufferPeriod(pCtx, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
			}
			else if (payload_type == SEI_PAYLOAD_PIC_TIMING)
			{
				BeginPicTiming(pCtx, pRBSPSEIPayloadBuf, cbRBSPPayloadBuf);
			}
			return RET_CODE_SUCCESS;
		}
		RET_CODE EnumNALSEIPayloadEnd(IUnknown* pCtx, uint32_t payload_type, uint8_t* pRBSPSEIPayloadBuf, size_t cbRBSPPayloadBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALSEIMessageEnd(IUnknown* pCtx, uint8_t* pRBSPSEIMsgRBSPBuf, size_t cbRBSPSEIMsgBuf) { return RET_CODE_SUCCESS; }
		RET_CODE EnumNALUnitEnd(IUnknown* pCtx, uint8_t* pEBSPNUBuf, size_t cbEBSPNUBuf)
		{
			m_NUCount++;
			return RET_CODE_SUCCESS;
		}

		RET_CODE EnumNALAUEnd(IUnknown* pCtx, uint8_t* pEBSPAUBuf, size_t cbEBSPAUBuf)
		{
			uint64_t t_ai_earliest = UINT64_MAX;

			/*
				Update the t_a_f
				The final arrival time for access unit n is derived by
			*/
			uint64_t t_a_f = UINT64_MAX;
			// Assume going into the CPB buffer now
			if (HRD_AU_n == 0)
			{
				HRD_AU_t_ai.push_back(0);
				HRD_AU_t_r_n.push_back(active_initial_cpb_removal_delay);
			}
			else if (HRD_AU_n != UINT32_MAX)
			{
				if (active_cbr_flag == 1)
				{
					/*
						If cbr_flag[ SchedSelIdx ] is equal to 1, the initial arrival time for access unit n, 
						is equal to the final arrival time (which is derived below) of access unit n-1, i.e.,	
					*/
					HRD_AU_t_ai.push_back(HRD_AU_t_af.back());
				}
				else
				{
					/*
						Otherwise (cbr_flag[ SchedSelIdx ] is equal to 0), the initial arrival time for access unit n is derived by	
					*/
					assert(HRD_AU_t_r.size() == (uint64_t)HRD_AU_n + 1);
					if (HRD_AU_n_of_buffering_period == 0)
					{
						
						if (HRD_AU_t_r.back() > active_initial_cpb_removal_delay)
							t_ai_earliest = HRD_AU_t_r.back() - active_initial_cpb_removal_delay;
						else
							t_ai_earliest = 0;
					}
					else
					{
						if (HRD_AU_t_r.back() > ((uint64_t)active_initial_cpb_removal_delay + active_initial_cpb_removal_delay_offset))
							t_ai_earliest = HRD_AU_t_r.back() - ((uint64_t)active_initial_cpb_removal_delay + active_initial_cpb_removal_delay_offset);
						else
							t_ai_earliest = 0;
					}

					uint64_t t_ai = AMP_MAX(HRD_AU_t_af.back(), t_ai_earliest);
					HRD_AU_t_ai.push_back(t_ai);
				}
			}

			assert(HRD_AU_n == HRD_AU_t_af.size());
			t_a_f = HRD_AU_t_ai.back() + cbEBSPAUBuf * 8ULL * 90000 / active_bitrate;
			HRD_AU_t_af.push_back(t_a_f);

			int64_t delta_t_g_90 = -1LL;
			if (HRD_AU_t_r.back() < HRD_AU_t_af.back())
			{
				printf("\t!!!!!!!!CPB will underflow.\n");
			}
			else if (HRD_AU_n > 0)
			{
				delta_t_g_90 = HRD_AU_t_r.back() - HRD_AU_t_af[HRD_AU_n - 1];

				//if (!active_cbr_flag)
				//{
				//	if (active_initial_cpb_removal_delay > delta_t_g_90)
				//		printf("assert happened!!!\n");
				//}
				//else
				//{
				//	//
				//}
			}

			printf("[AU#%05" PRIu32 "] -- initial arrive time: %" PRIu64 "(%" PRIu64 ".%03" PRIu64 "ms), final arrive time: %" PRIu64 "(%" PRIu64 ".%03" PRIu64 "ms), CPB removal time: %" PRIu64 "(%" PRIu64 ".%03" PRIu64 "ms), delta_t_g_90: %" PRId64 "\n", 
				HRD_AU_n,
				HRD_AU_t_ai.back(), HRD_AU_t_ai.back()/90, HRD_AU_t_ai.back() * 100 / 9 % 1000,
				HRD_AU_t_af.back(), HRD_AU_t_af.back()/90, HRD_AU_t_af.back() * 100 / 9 % 1000,
				 HRD_AU_t_r.back(),  HRD_AU_t_r.back()/90 , HRD_AU_t_r.back() * 100 / 9 % 1000,
				delta_t_g_90);

			HRD_AU_n++;
			HRD_AU_n_of_buffering_period++;

			return RET_CODE_SUCCESS; 
		}
		RET_CODE EnumNALError(IUnknown* pCtx, uint64_t stream_offset, int error_code)
		{
			printf("Hitting error {error_code: %d}.\n", error_code);
			return RET_CODE_SUCCESS;
		}

		int BeginBufferPeriod(IUnknown* pCtx, uint8_t* pSEIPayload, size_t cbSEIPayload)
		{
			int iRet = RET_CODE_SUCCESS;
			uint32_t sel_initial_cpb_removal_delay = UINT32_MAX;
			uint32_t sel_initial_cpb_removal_delay_offset = UINT32_MAX;

			if (pSEIPayload == nullptr || cbSEIPayload == 0 || pCtx == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			INALContext* pNALCtx = nullptr;
			if (FAILED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
				return RET_CODE_INVALID_PARAMETER;

			AMBst bst = nullptr;
			BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD* pBufPeriod = new
				BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD((int)cbSEIPayload, pNALCtx);
			NAL_CODING coding = pNALCtx->GetNALCoding();

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
				if (SUCCEEDED(pNALCtx->QueryInterface(IID_INALAVCContext, (void**)&pNALAVCCtx)))
				{
					sps_nu = pNALAVCCtx->GetAVCSPS(pBufPeriod->bp_seq_parameter_set_id);

					active_sps = sps_nu;

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
						auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters;
						for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->cpb_cnt_minus1; SchedSelIdx++)
						{
							if (active_HRD_type == 1 && active_SchedSelIdx == SchedSelIdx)
							{
								sel_initial_cpb_removal_delay = pBufPeriod->vcl_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_delay;
								sel_initial_cpb_removal_delay_offset = pBufPeriod->vcl_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_offset;
								active_cbr_flag = hrd_parameters->cbr_flag[SchedSelIdx];
								active_bitrate = ((uint64_t)hrd_parameters->bit_rate_value_minus1[SchedSelIdx] + 1) * (uint64_t)1ULL << (6 + hrd_parameters->bit_rate_scale);
								active_cpb_size = ((uint64_t)hrd_parameters->cpb_size_value_minus1[SchedSelIdx] + 1)*(uint64_t)1ULL << (4 + hrd_parameters->cpb_size_scale);
								break;
							}
						}
					}

					if (vui_parameters->nal_hrd_parameters_present_flag)
					{
						auto hrd_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters;
						for (uint8_t SchedSelIdx = 0; SchedSelIdx <= hrd_parameters->cpb_cnt_minus1; SchedSelIdx++)
						{
							if (active_HRD_type == 2 && active_SchedSelIdx == SchedSelIdx)
							{
								sel_initial_cpb_removal_delay = pBufPeriod->nal_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_delay;
								sel_initial_cpb_removal_delay_offset = pBufPeriod->nal_initial_cpb_removal_info[SchedSelIdx].initial_cpb_removal_offset;
								active_cbr_flag = hrd_parameters->cbr_flag[SchedSelIdx];
								active_bitrate = ((uint64_t)hrd_parameters->bit_rate_value_minus1[SchedSelIdx] + 1) * (uint64_t)1ULL << (6 + hrd_parameters->bit_rate_scale);
								active_cpb_size = ((uint64_t)hrd_parameters->cpb_size_value_minus1[SchedSelIdx] + 1)*(uint64_t)1ULL << (4 + hrd_parameters->cpb_size_scale);
								break;
							}
						}
					}

					if (vui_parameters->timing_info_present_flag)
						HRD_t_c = vui_parameters->num_units_in_tick * 90000ULL / vui_parameters->time_scale;

					active_low_delay_hrd_flag = vui_parameters->low_delay_hrd_flag;

					printf("New buffering period(initial_cpb_removal_delay:%" PRIu32 " initial_cpb_removal_delay_offset:%" PRIu32 ")\n", 
						sel_initial_cpb_removal_delay, sel_initial_cpb_removal_delay_offset);
				}
			}
			else if (coding == NAL_CODING_HEVC)
			{

			}
			else if (coding == NAL_CODING_VVC)
			{

			}

			// Initialize the HRD
			if (bHRDInited == false)
			{
				HRD_AU_n = 0;
				HRD_AU_t_ai.clear();
				HRD_AU_t_r_n.clear();
				active_initial_cpb_removal_delay = sel_initial_cpb_removal_delay;
				active_initial_cpb_removal_delay_offset = sel_initial_cpb_removal_delay_offset;

				bHRDInited = true;
			}

			HRD_AU_n_of_buffering_period = 0;
			bHaveSEIBufferingPeriod = true;

		done:
			if (bst)
				AMBst_Destroy(bst);

			AMP_SAFEDEL(pBufPeriod);
			AMP_SAFERELEASE(pNALCtx);

			return iRet;
		}

		int BeginPicTiming(IUnknown* pCtx, uint8_t* pSEIPayload, size_t cbSEIPayload)
		{
			int iRet = RET_CODE_SUCCESS;

			if (pSEIPayload == nullptr || cbSEIPayload == 0 || pCtx == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			INALContext* pNALCtx = nullptr;
			if (FAILED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
				return RET_CODE_INVALID_PARAMETER;

			NAL_CODING coding = pNALCtx->GetNALCoding();

			AMBst bst = nullptr;
			bst = AMBst_CreateFromBuffer(pSEIPayload, (int)cbSEIPayload);

			BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264* pPicTiming = nullptr;

			if (bst == nullptr)
			{
				iRet = RET_CODE_OUTOFMEMORY;
				goto done;
			}

			if (coding == NAL_CODING_AVC)
			{
				pPicTiming = new BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264((int)cbSEIPayload, pNALCtx);
				if (AMP_FAILED(iRet = pPicTiming->Map(bst)))
				{
					printf("Failed to unpack the SEI payload: buffering period.\n");
					goto done;
				}

				uint64_t t_r_n = UINT64_MAX;
				if (HRD_AU_n > 0)
				{
					if (bHaveSEIBufferingPeriod)
					{
						/*
							When an access unit n is the first access unit of a buffering period that does not initialize the HRD,
							the nominal removal time of the access unit from the CPB is specified by
						*/
						if (HRD_AU_previous_t_r_n != UINT64_MAX && HRD_t_c != UINT64_MAX)
						{
							t_r_n = HRD_AU_previous_t_r_n + HRD_t_c * pPicTiming->cpb_removal_delay;
							HRD_AU_t_r_n.push_back(t_r_n);

							HRD_AU_previous_t_r_n = t_r_n;
						}
					}
					else
					{
						/*
							The nominal removal time tr,n(n) of an access unit n that is not the first access unit of a buffering period is given by
						*/
						t_r_n = HRD_AU_previous_t_r_n + HRD_t_c * pPicTiming->cpb_removal_delay;
						HRD_AU_t_r_n.push_back(t_r_n);
					}
				}
				else
				{
					t_r_n = active_initial_cpb_removal_delay;
					HRD_AU_previous_t_r_n = t_r_n;
				}

				if (active_low_delay_hrd_flag == 0 || t_r_n >= HRD_AU_t_af[HRD_AU_n])
				{
					/*
						If low_delay_hrd_flag is equal to 0 or tr,n( n ) >= taf( n ), the removal time of access unit n is specified by
					*/
					HRD_AU_t_r.push_back(t_r_n);
				}
				else
				{
					uint64_t t_r = t_r_n + (HRD_AU_t_af[HRD_AU_n] - t_r_n + HRD_t_c - 1) / HRD_t_c * HRD_t_c;
					HRD_AU_t_r.push_back(t_r);
				}
			}

		done:
			if (bst)
				AMBst_Destroy(bst);

			AMP_SAFEDEL(pPicTiming);
			AMP_SAFERELEASE(pNALCtx);

			return iRet;
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

		H264_NU active_sps;
		uint32_t active_HRD_type = 2;	// At present, only support HDR type-II
		uint32_t active_SchedSelIdx = 0;	// At present, always using the first one
		uint32_t active_initial_cpb_removal_delay = UINT32_MAX;
		uint32_t active_initial_cpb_removal_delay_offset = UINT32_MAX;
		uint32_t active_cbr_flag = UINT32_MAX;
		uint32_t active_low_delay_hrd_flag = UINT32_MAX;
		uint64_t active_bitrate = UINT64_MAX;
		uint64_t active_cpb_size = 0;

		// For HRD analyzing
		bool bHRDInited = false;
		uint32_t HRD_AU_n = UINT32_MAX;
		uint32_t HRD_AU_n_of_buffering_period = UINT32_MAX;
		std::vector<uint64_t> HRD_AU_t_ai;
		std::vector<uint64_t> HRD_AU_t_af;
		std::vector<uint64_t> HRD_AU_t_r_n;
		std::vector<uint64_t> HRD_AU_t_r;
		bool bHaveSEIBufferingPeriod = false;
		uint64_t HRD_AU_previous_t_r_n = UINT64_MAX;
		uint64_t HRD_t_c = UINT64_MAX;

	}HRDRunner(pNALContext);

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

	HRDRunner.AddRef();
	NALParser.SetEnumerator((IUnknown*)(&HRDRunner), NAL_ENUM_OPTION_ALL);

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

