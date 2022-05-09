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
#include <stdio.h>
#include <map>
#include "video_transcoder.h"
#include "systemdef.h"
#include "DataUtil.h"
#include "MSE.h"
#include "nal_com.h"
#include "mpeg2_video.h"
#include "h264_video.h"
#include "h265_video.h"
#include "h266_video.h"
#include "av1.h"
#include "MSEnav.h"

#define INITIAL_PTS_VALUE			(10LL*90000LL)

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;

extern MEDIA_SCHEME_TYPE			g_source_media_scheme_type;
extern MEDIA_SCHEME_TYPE			g_output_media_scheme_type;

extern MEDIA_SCHEME_TYPE			CheckAndUpdateFileFormat(std::string& filepath, const char* param_name);
extern const std::map<int, AVC_LEVEL_LIMIT> 
									avc_level_limits;
extern const std::map<uint8_t, GENERAL_TIER_AND_LEVEL_LIMIT> 
									general_tier_and_level_limits;
extern const HEVC_PROFILE_FACTOR	hevc_profile_factors[36];

extern int LoadVTCExport(VTC_EXPORT& vtc_exports);
extern int UnloadVTCExport(VTC_EXPORT& vtc_exports);

int32_t get_video_stream_fourcc(const char* srcfmt)
{
	if (_stricmp(srcfmt, "h264") == 0)
		return 'avc3';
	else if (_stricmp(srcfmt, "h265") == 0)
		return 'hev1';
	else if (_stricmp(srcfmt, "mpv") == 0)
		return 'mp2v';
	else if (_stricmp(srcfmt, "av1") == 0)
		return 'av01';

	return 0;
}

int InitVideoTranscodeParams(VTC_EXPORT& vtc_export, vtc_param_t& vtc_params)
{
	int iRet = RET_CODE_SUCCESS;
	int64_t width = 0, height = 0;
	auto iter_dstfmt	= g_params.find("outputfmt");
	auto iter_srcfmt	= g_params.find("srcfmt");
	auto iter_bitrate	= g_params.find("bitrate");
	auto iter_width		= g_params.find("width");
	auto iter_height	= g_params.find("height");
	auto iter_fps		= g_params.find("fps");
	auto iter_sar		= g_params.find("SAR");
	auto iter_profile	= g_params.find("profile");
	auto iter_tier		= g_params.find("tier");
	auto iter_level		= g_params.find("level");

	if (iter_srcfmt == g_params.end())
	{
		printf("Can't create video encoder since no source format is specified.\n");
		return RET_CODE_ERROR;
	}

	if (iter_dstfmt == g_params.end())
	{
		printf("Can't create video encoder since no target format is specified.\n");
		return RET_CODE_ERROR;
	}

	vtc_export.fn_params_init(&vtc_params);

	vtc_params.src_stream_fourcc = get_video_stream_fourcc(iter_srcfmt->second.c_str());
	vtc_params.dst_stream_fourcc = get_video_stream_fourcc(iter_dstfmt->second.c_str());

	if (vtc_params.src_stream_fourcc == 0) {
		printf("Can't get the FOURCC for the input source format, and abort the transcode.\n");
		iRet = RET_CODE_INVALID_PARAMETER;
		goto done;
	}
	else if (vtc_params.dst_stream_fourcc == 0) 
	{
		vtc_params.dst_stream_fourcc = vtc_params.src_stream_fourcc;
	}

	// Check the width and height
	if (iter_width != g_params.end())
	{
		if (ConvertToInt(iter_width->second, width) == false || width < 0 || width > INT32_MAX)
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}

		vtc_params.width = (int32_t)width;
	}

	if (iter_height != g_params.end())
	{
		if (ConvertToInt(iter_height->second, height) == false || height < 0 || height > INT32_MAX)
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}

		vtc_params.height = (int32_t)height;
	}

	// check and parse fps
	if (iter_fps != g_params.end())
	{
		int64_t fps_num = -1, fps_den = -1;
		if (ConvertToRationalNumber(iter_fps->second, fps_num, fps_den) == false ||
			fps_num < 0 || fps_num > UINT32_MAX ||
			fps_den < 0 || fps_den > UINT32_MAX)
		{
			printf("The 'fps' parameter \"--fps=%s\" can't be parsed.\n", iter_fps->second.c_str());
			iRet = RET_CODE_ERROR;
			goto done;
		}
		else
		{
			vtc_params.fps_num = (uint32_t)fps_num;
			vtc_params.fps_den = (uint32_t)fps_den;
		}
	}

	// check and parse SAR
	if (iter_sar != g_params.end())
	{
		int64_t sar_num = -1, sar_den = -1;
		if (ConvertToRationalNumber(iter_sar->second, sar_num, sar_den) == false ||
			sar_num < 0 || sar_num > UINT32_MAX ||
			sar_den < 0 || sar_den > UINT32_MAX)
		{
			printf("The 'SAR' parameter \"--SAR=%s\" can't be parsed.\n", iter_sar->second.c_str());
			iRet = RET_CODE_ERROR;
			goto done;
		}
		else
		{
			vtc_params.sar_num = (uint32_t)sar_num;
			vtc_params.sar_den = (uint32_t)sar_den;
		}
	}

	// Check and parse the bitrate
	if (iter_bitrate != g_params.end())
	{
		int64_t target_bitrate = -1;
		if (ConvertToInt(iter_bitrate->second, target_bitrate) == false || target_bitrate < 0 || target_bitrate > UINT32_MAX)
		{
			iRet = RET_CODE_ERROR_NOTIMPL;
			goto done;
		}

		vtc_params.target_bitrate = (uint32_t)target_bitrate;
	}

	if (iter_profile != g_params.end())
		vtc_export.fn_params_parse(&vtc_params, "profile", iter_profile->second.c_str());

	if (iter_tier != g_params.end())
		vtc_export.fn_params_parse(&vtc_params, "tier", iter_tier->second.c_str());

	if (iter_level != g_params.end())
		vtc_export.fn_params_parse(&vtc_params, "level", iter_level->second.c_str());

	iRet = RET_CODE_SUCCESS;

done:
	if (AMP_FAILED(iRet))
		vtc_export.fn_params_cleanup(&vtc_params);
	return iRet;
}

class CAUESTranscoder
{
public:
	CAUESTranscoder(FILE* wfp, VTC_EXPORT& vtc_export, int* pRet)
		: m_vtc_export(&vtc_export)
		, m_vtc_handle(NULL)
		, m_cur_pts(-1LL)
		, m_fpw(wfp)
		, m_bInputEOS(false) 
		, m_bOutputEOS(false)
	{
		int iRet = InitVideoTranscodeParams(vtc_export, m_vtc_params);
		if (pRet)
			*pRet = iRet;
	}

	virtual ~CAUESTranscoder() {
		if (m_vtc_export)
		{
			if (m_vtc_export->fn_close && m_vtc_handle)
			{
				m_vtc_export->fn_close(m_vtc_handle);
				m_vtc_handle = nullptr;
			}

			if (m_vtc_export->fn_params_cleanup)
				m_vtc_export->fn_params_cleanup(&m_vtc_params);
		}
	}

	RET_CODE TranscodeAUESBuffer(uint8_t* pAUBuf, size_t cbAUBuf, bool bEOS)
	{
		return _InternalTranscoeProcess(pAUBuf, cbAUBuf, bEOS, 0);
	}

	RET_CODE TranscodeDrain()
	{
		return _InternalTranscoeProcess(NULL, 0, TRUE, VTC_INFINITE);
	}

	RET_CODE ValiddateAndUpdateParams(vtc_param_t& vtc_params)
	{
		if (IS_AVC_FOURCC(vtc_params.dst_stream_fourcc))
		{
			auto iter_limits = avc_level_limits.find(vtc_params.output_level);
			if (iter_limits != avc_level_limits.end())
			{
				uint32_t cpbBrNalFactor = 1000;
				//uint32_t cpbBrNalFactor = 1200;
				//if (vtc_params.output_profile == VTC_AVC_PROFILE_HIGH ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_PROGRESSIVE_HIGH ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_CONSTRAINED_HIGH)
				//	cpbBrNalFactor = 1500;
				//else if (vtc_params.output_profile == VTC_AVC_PROFILE_HIGH_10 ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_PROGRESSIVE_HIGH_10 ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_HIGH_10_INTRA)
				//	cpbBrNalFactor = 3600;
				//else if(vtc_params.output_profile == VTC_AVC_PROFILE_HIGH_422 ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_HIGH_422_INTRA)
				//	cpbBrNalFactor = 4800;
				//else if (vtc_params.output_profile == VTC_AVC_PROFILE_HIGH_444_PREDICTIVE ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_HIGH_444_INTRA ||
				//	vtc_params.output_profile == VTC_AVC_PROFILE_CAVLC_444_INTRA_PROFIILE)
				//	cpbBrNalFactor = 4800;

				vtc_params.target_max_bitrate = cpbBrNalFactor * iter_limits->second.MaxBR;			// NAL HRD
				vtc_params.target_cpb_buffer_size = cpbBrNalFactor * iter_limits->second.MaxCPB;	// NAL CPB
			}
		}

		return RET_CODE_SUCCESS;
	}

protected:
	virtual RET_CODE _InternalTranscoeProcess(uint8_t* pAUBuf, size_t cbAUBuf, bool bEOS, uint32_t msTimeOut)
	{
		int iRetVTC = VTC_RET_OK;
		int iRet = RET_CODE_SUCCESS;
		vtc_picture_es_t input_es, output_es;
		bool bTranscodePipelineFull = false;
		
		if (m_vtc_handle == nullptr || m_vtc_export == nullptr)
			return VTC_RET_NOT_INITIALIZED;

		// If already send the EOS, directly drain the left encoded ES
		if (m_bInputEOS && pAUBuf == nullptr && cbAUBuf == 0 && bEOS == true)
			goto repick;

		m_vtc_export->fn_picture_es_init(&input_es);

		if (m_cur_pts == -1LL) {
			input_es.pts = INITIAL_PTS_VALUE;
			m_cur_pts = INITIAL_PTS_VALUE;
		}

		input_es.es_buf = pAUBuf;
		input_es.es_buf_size = (int32_t)cbAUBuf;
		input_es.EOS = bEOS;

	reinput:
		iRetVTC = m_vtc_export->fn_input(m_vtc_handle, input_es);
		if (iRetVTC == VTC_RET_PIPEFULL)
			bTranscodePipelineFull = true;
		else
		{
			bTranscodePipelineFull = false;
			if (VTC_FAILED(iRetVTC)) {
				printf("Failed to input the video ES data to the video transcoder.\n");
				return RET_CODE_ERROR;
			}
			else
			{
				// the EOS has already sent out
				if (bEOS)
					m_bInputEOS = true;
			}
		}

	repick:
		m_vtc_export->fn_picture_es_init(&output_es);

		iRetVTC = m_vtc_export->fn_output(m_vtc_handle, &output_es, msTimeOut);

		if (VTC_SUCCEEDED(iRetVTC))
		{
			if (output_es.es_buf != nullptr && output_es.es_buf_size > 0)
			{
				if (m_fpw != NULL)
					fwrite(output_es.es_buf, 1, output_es.es_buf_size, m_fpw);

				m_vtc_export->fn_picture_es_cleanup(&output_es);
			}

			if (output_es.EOS)
			{
				m_bOutputEOS = true;
				iRet = RET_CODE_SUCCESS;
			}
			else
			{
				if (bTranscodePipelineFull)
					goto reinput;
				else
					// There may be still transcoded ES in the transcoder, pick up all
					goto repick;
			}
		}
		else if (iRetVTC == VTC_RET_NEEDMOREINPUT || iRetVTC == VTC_RET_TIME_OUT)
			iRet = RET_CODE_NOTHING_TODO;
		else
		{
			iRet = RET_CODE_ERROR;
			bTranscodePipelineFull = false;
		}

		if (bTranscodePipelineFull)
			goto reinput;

		return iRet;
	}

protected:
	vtc_param_t			m_vtc_params;
	VTC_EXPORT*			m_vtc_export;
	vtc_handle			m_vtc_handle;
	int64_t				m_cur_pts;
	FILE*				m_fpw;
	bool				m_bInputEOS;
	bool				m_bOutputEOS;
};

class CMPVAUEnumerator : public CMPVNavEnumerator, public CAUESTranscoder
{
public:
	CMPVAUEnumerator(IMPVContext* pCtx, uint32_t options, MSENav* pMSENav, FILE* wfp, VTC_EXPORT& vtc_export, int* pRet)
		: CMPVNavEnumerator(pCtx, options, pMSENav)
		, CAUESTranscoder(wfp, vtc_export, pRet){}

	~CMPVAUEnumerator() {}

public:
	RET_CODE OnProcessAU(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		if (cbAUBuf > INT32_MAX || cbAUBuf == 0)
			return RET_CODE_NOTHING_TODO;

		// Check whether the sequence header is ready or not
		if (m_vtc_handle == nullptr)
		{
			IMPVContext* pMPVCtx = nullptr;
			if (FAILED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx))) {
				return RET_CODE_NOTHING_TODO;
			}

			SEQHDR seq_hdr = pMPVCtx->GetSeqHdr();
			SEQEXT seq_ext = pMPVCtx->GetSeqExt();

			AMP_SAFERELEASE(pMPVCtx);

			if (seq_hdr == nullptr)
				return RET_CODE_NOTHING_TODO;

			vtc_param_t params;
			m_vtc_export->fn_params_clone(&m_vtc_params, &params);

			FinalizeVTCParamsForM2V(seq_hdr, seq_ext, params);

			if ((m_vtc_handle = m_vtc_export->fn_open(&params)) == nullptr)
			{
				printf("Failed to open the video transcoder.\n");
				return RET_CODE_ABORT;
			}
		}

		return TranscodeAUESBuffer(pAUBuf, cbAUBuf, false);
	}

	RET_CODE FinalizeVTCParamsForM2V(SEQHDR seq_hdr, SEQEXT seq_ext, vtc_param_t& params);
};

class CNALAUEnumerator : public CNALNavEnumerator, public CAUESTranscoder
{
public:
	CNALAUEnumerator(INALContext* pCtx, uint32_t options, MSENav* pMSENav, FILE* wfp, VTC_EXPORT& vtc_export, int* pRet)
		: CNALNavEnumerator(pCtx, options, pMSENav)
		, CAUESTranscoder(wfp, vtc_export, pRet){}

	~CNALAUEnumerator() {}

public:
	RET_CODE OnProcessAU(IUnknown* pCtx, uint8_t* pAUBuf, size_t cbAUBuf, int picCodingType)
	{
		if (cbAUBuf > INT32_MAX || cbAUBuf == 0)
			return RET_CODE_NOTHING_TODO;

		// Check whether the sequence header is ready or not
		if (m_vtc_handle == nullptr)
		{
			vtc_param_t params;
			m_vtc_export->fn_params_clone(&m_vtc_params, &params);

			if (m_curr_nal_coding == NAL_CODING_AVC)
			{
				INALAVCContext* pAVCCtx = nullptr;
				if (FAILED(pCtx->QueryInterface(IID_INALAVCContext, (void**)&pAVCCtx))) {
					return RET_CODE_NOTHING_TODO;
				}

				int8_t sps_id = pAVCCtx->GetActiveSPSID();
				if (sps_id < 0) {
					AMP_SAFERELEASE(pAVCCtx);
					return RET_CODE_NOTHING_TODO;
				}

				H264_NU sps_nu = pAVCCtx->GetAVCSPS(sps_id);
				if (sps_nu == nullptr) {
					AMP_SAFERELEASE(pAVCCtx);
					return RET_CODE_NOTHING_TODO;
				}

				FinalizeVTCParamsForAVCSource(pAVCCtx, sps_nu, params);
				AMP_SAFERELEASE(pAVCCtx);
			}
			else if (m_curr_nal_coding == NAL_CODING_HEVC)
			{
				INALHEVCContext* pHEVCCtx = nullptr;
				if (FAILED(pCtx->QueryInterface(IID_INALHEVCContext, (void**)&pHEVCCtx))) {
					return RET_CODE_NOTHING_TODO;
				}

				int8_t sps_id = pHEVCCtx->GetActiveSPSID();
				if (sps_id < 0) {
					AMP_SAFERELEASE(pHEVCCtx);
					return RET_CODE_NOTHING_TODO;
				}

				H265_NU sps_nu = pHEVCCtx->GetHEVCSPS(sps_id);
				if (sps_nu == nullptr) {
					AMP_SAFERELEASE(pHEVCCtx);
					return RET_CODE_NOTHING_TODO;
				}

				FinalizeVTCParamsForHEVCSource(pHEVCCtx, sps_nu, params);
				AMP_SAFERELEASE(pHEVCCtx);
			}
			else if (m_curr_nal_coding == NAL_CODING_VVC)
			{
				INALVVCContext* pVVCCtx = nullptr;
				if (FAILED(pCtx->QueryInterface(IID_INALVVCContext, (void**)&pVVCCtx))) {
					return RET_CODE_NOTHING_TODO;
				}

				int8_t sps_id = pVVCCtx->GetActiveSPSID();
				if (sps_id < 0) {
					AMP_SAFERELEASE(pVVCCtx);
					return RET_CODE_NOTHING_TODO;
				}

				H266_NU sps_nu = pVVCCtx->GetVVCSPS(sps_id);
				if (sps_nu == nullptr) {
					AMP_SAFERELEASE(pVVCCtx);
					return RET_CODE_NOTHING_TODO;
				}

				FinalizeVTCParamsForVVCSource(pVVCCtx, sps_nu, params);
				AMP_SAFERELEASE(pVVCCtx);
			}
			else
				return RET_CODE_ERROR_NOTIMPL;

			if ((m_vtc_handle = m_vtc_export->fn_open(&params)) == nullptr)
			{
				printf("Failed to open the video transcoder.\n");
				return RET_CODE_ABORT;
			}
		}

		return TranscodeAUESBuffer(pAUBuf, cbAUBuf, false);
	}

	RET_CODE FinalizeVTCParamsForAVCSource(INALAVCContext* pAVCCtx, H264_NU sps_nu, vtc_param_t& params);
	RET_CODE FinalizeVTCParamsForHEVCSource(INALHEVCContext* pHEVCCtx, H265_NU sps_nu, vtc_param_t& params);
	RET_CODE FinalizeVTCParamsForVVCSource(INALVVCContext* pVVCCtx, H266_NU sps_nu, vtc_param_t& params);
};

class CAV1AUEnumerator : public CAV1NavEnumerator, public CAUESTranscoder
{
public:
	CAV1AUEnumerator(IAV1Context* pCtx, uint32_t options, MSENav* pMSENav, FILE* wfp, VTC_EXPORT& vtc_export, int* pRet)
		: CAV1NavEnumerator(pCtx, options, pMSENav)
		, CAUESTranscoder(wfp, vtc_export, pRet){}

	~CAV1AUEnumerator() {}

public:
	RET_CODE OnProcessTU(IUnknown* pCtx, uint8_t* ptr_TU_buf, uint32_t TU_size, int frame_type)
	{
		if (TU_size > INT32_MAX || TU_size == 0)
			return RET_CODE_NOTHING_TODO;

		// Check whether the sequence header is ready or not
		if (m_vtc_handle == nullptr)
		{
			IAV1Context* pAV1Ctx = nullptr;
			if (FAILED(pCtx->QueryInterface(IID_IAV1Context, (void**)&pAV1Ctx))) {
				return RET_CODE_NOTHING_TODO;
			}

			AV1_OBU seq_hdr_obu = pAV1Ctx->GetSeqHdrOBU();

			if (seq_hdr_obu == nullptr)
				return RET_CODE_NOTHING_TODO;

			vtc_param_t params;
			m_vtc_export->fn_params_clone(&m_vtc_params, &params);

			FinalizeVTCParamsForAV1(seq_hdr_obu, params);

			if ((m_vtc_handle = m_vtc_export->fn_open(&params)) == nullptr)
			{
				printf("Failed to open the video transcoder.\n");
				return RET_CODE_ABORT;
			}
		}

		return TranscodeAUESBuffer(ptr_TU_buf, TU_size, false);
	}

	RET_CODE FinalizeVTCParamsForAV1(AV1_OBU seq_hdr_obu, vtc_param_t& params);
};

int BindAUTranscodeEnumerator(IMSEParser* pMSEParser, IUnknown* pCtx, uint32_t enum_options, MSENav& mse_nav, FILE* wfp, VTC_EXPORT& vtc_export, CAUESTranscoder** ppAUTranscoder)
{
	if (pMSEParser == nullptr || pCtx == nullptr)
		return RET_CODE_ERROR_NOTIMPL;

	int iRet = RET_CODE_ERROR_NOTIMPL;
	MEDIA_SCHEME_TYPE scheme_type = pMSEParser->GetSchemeType();

	if (scheme_type == MEDIA_SCHEME_NAL)
	{
		INALContext* pNALCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_INALContext, (void**)&pNALCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();

			CNALAUEnumerator* pNALAUEnumerator = new CNALAUEnumerator(pNALCtx, enum_options | options, &mse_nav, wfp, vtc_export, &iRet);

			if (AMP_FAILED(iRet)) {
				AMP_SAFERELEASE(pNALCtx);
				AMP_SAFEDEL(pNALAUEnumerator);
				goto done;
			}

			if (SUCCEEDED(iRet = pNALAUEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pNALCtx);

			if (ppAUTranscoder)
				*ppAUTranscoder = (CAUESTranscoder*)pNALAUEnumerator;
		}
	}
	else if (scheme_type == MEDIA_SCHEME_AV1)
	{
		IAV1Context* pAV1Ctx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IAV1Context, (void**)&pAV1Ctx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();
			CAV1AUEnumerator* pAV1AUEnumerator = new CAV1AUEnumerator(pAV1Ctx, enum_options | options, &mse_nav, wfp, vtc_export, &iRet);

			if (AMP_FAILED(iRet)) {
				AMP_SAFERELEASE(pAV1Ctx);
				AMP_SAFEDEL(pAV1AUEnumerator);
				goto done;
			}

			if (SUCCEEDED(iRet = pAV1AUEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pAV1Ctx);

			if (ppAUTranscoder)
				*ppAUTranscoder = (CAUESTranscoder*)pAV1AUEnumerator;
		}
	}
	else if (scheme_type == MEDIA_SCHEME_MPV)
	{
		IMPVContext* pMPVCtx = nullptr;
		if (SUCCEEDED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx)))
		{
			IUnknown* pMSEEnumerator = nullptr;
			uint32_t options = mse_nav.GetEnumOptions();

			CMPVAUEnumerator* pMPVAUEnumerator = new CMPVAUEnumerator(pMPVCtx, enum_options | options, &mse_nav, wfp, vtc_export, &iRet);

			if (AMP_FAILED(iRet)) {
				AMP_SAFERELEASE(pMPVCtx);
				AMP_SAFEDEL(pMPVAUEnumerator);
				goto done;
			}

			if (SUCCEEDED(iRet = pMPVAUEnumerator->QueryInterface(__uuidof(IUnknown), (void**)&pMSEEnumerator)))
			{
				iRet = pMSEParser->SetEnumerator(pMSEEnumerator, enum_options | options);
				AMP_SAFERELEASE(pMSEEnumerator);
			}

			AMP_SAFERELEASE(pMPVCtx);

			if (ppAUTranscoder)
				*ppAUTranscoder = (CAUESTranscoder*)pMPVAUEnumerator;
		}
	}
	else if (scheme_type == MEDIA_SCHEME_LOAS_LATM)
	{

	}
	else if (scheme_type == MEDIA_SCHEME_ISOBMFF)
	{

	}

done:
	return iRet;
}

int TranscodeFromESToES()
{
	FILE* rfp = NULL;
	FILE* wfp = NULL;
	int iRet = RET_CODE_SUCCESS;
	int64_t file_size = 0;
	uint8_t pBuf[2048] = { 0 };
	IMSEParser* pMediaParser = nullptr;
	IUnknown* pCtx = nullptr;
	VTC_EXPORT vtc_export;
	CAUESTranscoder* pAUTranscoder = nullptr;

	auto iter_dstfmt = g_params.find("outputfmt");
	auto iter_srcfmt = g_params.find("srcfmt");
	auto iter_bitrate = g_params.find("bitrate");
	if (iter_bitrate == g_params.end())
		return RET_CODE_ERROR_NOTIMPL;

	int64_t target_bitrate = -1;
	if (ConvertToInt(iter_bitrate->second, target_bitrate) == false)
		return RET_CODE_ERROR_NOTIMPL;

	const int read_unit_size = 2048;

	auto iter_dstOutput = g_params.find("output");
	auto iter_srcInput = g_params.find("input");
	if (iter_srcInput == g_params.end())
	{
		printf("Please specify an input ES file to transcode.\n");
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (iter_dstfmt == g_params.end())
	{
		// check the file extension
		if (g_params.find("output") != g_params.end())
		{
			std::string& str_output_file_name = g_params["output"];
			// "outputfmt" is used by container to es, don't modify it
			// If want to know its detailed output format, need use g_output_media_sheme_type
			CheckAndUpdateFileFormat(str_output_file_name, "outputfmt");
			iter_dstfmt = g_params.find("outputfmt");
		}

		if (iter_dstfmt == g_params.end() && g_params.find("copy") != g_params.end() && iter_srcfmt != g_params.end())
		{
			g_params["outputfmt"] = iter_srcfmt->second;
			iter_dstfmt = g_params.find("outputfmt");
		}
	}

	if (AMP_FAILED(iRet = CreateMSEParser(&pMediaParser)))
	{
		printf("Failed to create the media syntax element parser {error: %d}\n", iRet);
		return RET_CODE_ERROR_NOTIMPL;
	}

	int enum_cmd_option = MSE_ENUM_AU_RANGE;
	MSENav mse_nav(pMediaParser->GetSchemeType());
	if (AMP_FAILED(iRet = mse_nav.Load(enum_cmd_option)))
	{
		printf("Failed to load the Media Syntax Element URI.\n");
		return iRet;
	}

	errno_t errn = fopen_s(&rfp, iter_srcInput->second.c_str(), "rb");
	if (errn != 0 || rfp == NULL)
	{
		printf("Failed to open the file: %s {errno: %d}.\n", iter_srcInput->second.c_str(), errn);
		goto done;
	}

	if (iter_dstOutput != g_params.end())
	{
		errn = fopen_s(&wfp, iter_dstOutput->second.c_str(), "wb");
		if (errn != 0 || wfp == NULL)
		{
			printf("Failed to open the file: %s for writing {errno: %d}.\n", iter_dstOutput->second.c_str(), errn);
			goto done;
		}
	}

	if (AMP_FAILED(iRet = pMediaParser->GetContext(&pCtx)))
	{
		printf("Failed to get the media element syntax element context {error: %d}\n", iRet);
		goto done;
	}

	if (AMP_FAILED(LoadVTCExport(vtc_export)))
	{
		printf("Failed to initialize the video transcoder.\n");
		goto done;
	}

	if (AMP_FAILED(iRet = BindAUTranscodeEnumerator(pMediaParser, pCtx, enum_cmd_option, mse_nav, wfp, vtc_export, &pAUTranscoder)))
	{
		printf("Failed to bind the media syntax element enumerator {error: %d}\n", iRet);
		goto done;
	}

	file_size = GetFileSizeByFP(rfp);

	PrintMSEHeader(pMediaParser, pCtx, enum_cmd_option, mse_nav, rfp);

	LocateMSEParseStartPosition(pCtx, rfp);

	do
	{
		int read_size = read_unit_size;
		if ((read_size = (int)fread(pBuf, 1, read_unit_size, rfp)) <= 0)
		{
			iRet = RET_CODE_IO_READ_ERROR;
			break;
		}

		iRet = pMediaParser->ProcessInput(pBuf, read_size);
		if (AMP_FAILED(iRet))
			break;

		iRet = pMediaParser->ProcessOutput();
		if (iRet == RET_CODE_ABORT)
			break;

	} while (!feof(rfp));

	// Drain the left data in the parser, if --AU is specified, feof(rfp) will not equal to true 
	// because the parser is abort in the middle of parsing
	if (feof(rfp))
		iRet = pMediaParser->ProcessOutput(true);

	if (pAUTranscoder) {
		pAUTranscoder->TranscodeDrain();
	}

done:
	if (rfp != nullptr)
		fclose(rfp);

	if (pMediaParser)
	{
		pMediaParser->Release();
		pMediaParser = nullptr;
	}

	if (pCtx)
	{
		pCtx->Release();
		pCtx = nullptr;
	}

	UnloadVTCExport(vtc_export);

	return iRet;
}

RET_CODE CMPVAUEnumerator::FinalizeVTCParamsForM2V(SEQHDR seq_hdr, SEQEXT seq_ext, vtc_param_t& params)
{
	if (params.width <= 0) {
		params.width = seq_ext == nullptr ? seq_hdr->horizontal_size_value
			: (seq_hdr->horizontal_size_value | (seq_ext->horizontal_size_extension << 12));
	}

	if (params.height <= 0) {
		params.height = seq_ext == nullptr ? seq_hdr->vertical_size_value
			: (seq_hdr->vertical_size_value | (seq_ext->vertical_size_extension << 12));
	}

	if (params.width <= 0 || params.height <= 0)
	{
		printf("The specified width(%d) or height(%d) is out of range.\n", params.width, params.height);
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (params.src_pulldown == VTC_SWITCH_AUTO && seq_ext && seq_ext->progressive_sequence)
		params.src_pulldown = VTC_SWITCH_OFF;

	if (params.target_interlace == VTC_SWITCH_AUTO && seq_ext && seq_ext->progressive_sequence)
		params.target_interlace = VTC_SWITCH_OFF;

	if (params.fps_den == 0 && params.fps_num == 0)
	{
		uint8_t frame_rate_extension_d = 0, frame_rate_extension_n = 0;
		if (seq_ext != nullptr)
		{
			frame_rate_extension_d = seq_ext->frame_rate_extension_d;
			frame_rate_extension_n = seq_ext->frame_rate_extension_n;
		}

		switch (seq_hdr->frame_rate_code)
		{
		case 1: params.fps_num = 24000; params.fps_den = 1001; break;
		case 2: params.fps_num = 24; params.fps_den = 1; break;
		case 3: params.fps_num = 25; params.fps_den = 1; break;
		case 4: params.fps_num = 30000; params.fps_den = 1001; break;
		case 5: params.fps_num = 30; params.fps_den = 1; break;
		case 6: params.fps_num = 50; params.fps_den = 1; break;
		case 7: params.fps_num = 60000; params.fps_den = 1001; break;
		case 8: params.fps_num = 60; params.fps_den = 1; break;
		default:
			return RET_CODE_ERROR_NOTIMPL;
		}
	}

	if (params.sar_den == 0 && params.sar_num == 0)
	{
		// Check the display aspect ratio
		auto iter_dar = g_params.find("DAR");
		if (iter_dar != g_params.end())
		{
			int64_t dar_num = -1, dar_den = -1;
			if (ConvertToRationalNumber(iter_dar->second, dar_num, dar_den) == false ||
				dar_num <= 0 || dar_num > UINT32_MAX ||
				dar_den <= 0 || dar_den > UINT32_MAX)
			{
				printf("The 'DAR' parameter \"--DAR=%s\" can't be parsed, ignore it.\n", iter_dar->second.c_str());
			}
			else
			{
				uint64_t common_divisor = gcd((uint64_t)(dar_num*params.height), (uint64_t)(dar_den*params.width));
				params.sar_num = (uint32_t)(dar_num*params.height / common_divisor);
				params.sar_den = (uint32_t)(dar_den*params.width  / common_divisor);
			}
		}

		if (params.sar_den == 0 && params.sar_num == 0)
		{
			uint64_t sar_num = 0, sar_den = 0;
			switch (seq_hdr->aspect_ratio_information)
			{
			case 1: sar_num = 1; sar_den = 1; break;
			case 2: sar_num = (uint64_t)params.height*4;	sar_den = (uint64_t)params.width * 3; break;
			case 3: sar_num = (uint64_t)params.height*16 ;	sar_den = (uint64_t)params.width * 9; break;
			case 4: sar_num = (uint64_t)params.height*221;	sar_den = (uint64_t)params.width * 1; break;
			default:
				return RET_CODE_ERROR_NOTIMPL;
			}

			uint64_t common_divisor = gcd(sar_num, sar_den);
			params.sar_num = (uint32_t)(sar_num / common_divisor);
			params.sar_den = (uint32_t)(sar_den / common_divisor);
		}
	}

	m_vtc_export->fn_param_autoselect_profile_tier_level(&params);

	return ValiddateAndUpdateParams(params);
}

RET_CODE CNALAUEnumerator::FinalizeVTCParamsForAVCSource(INALAVCContext* pAVCCtx, H264_NU sps_nu, vtc_param_t& params)
{
	if (params.width <= 0)
		params.width = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.display_width;;

	if (params.height <= 0)
		params.height = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.display_height;

	if (params.width <= 0 || params.height <= 0)
	{
		printf("The specified width(%d) or height(%d) is out of range.\n", params.width, params.height);
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (params.sar_den == 0 && params.sar_num == 0)
	{
		// Check the display aspect ratio
		auto iter_dar = g_params.find("DAR");
		if (iter_dar != g_params.end())
		{
			int64_t dar_num = -1, dar_den = -1;
			if (ConvertToRationalNumber(iter_dar->second, dar_num, dar_den) == false ||
				dar_num <= 0 || dar_num > UINT32_MAX ||
				dar_den <= 0 || dar_den > UINT32_MAX)
			{
				printf("The 'DAR' parameter \"--DAR=%s\" can't be parsed, ignore it.\n", iter_dar->second.c_str());
			}
			else
			{
				uint64_t common_divisor = gcd((uint64_t)(dar_num*params.height), (uint64_t)(dar_den*params.width));
				params.sar_num = (uint32_t)(dar_num*params.height / common_divisor);
				params.sar_den = (uint32_t)(dar_den*params.width / common_divisor);
			}
		}
	}

	if (params.src_pulldown == VTC_SWITCH_AUTO && sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.frame_mbs_only_flag)
		params.src_pulldown = VTC_SWITCH_OFF;

	if (params.target_interlace == VTC_SWITCH_AUTO && sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.frame_mbs_only_flag)
		params.target_interlace = VTC_SWITCH_OFF;

	if (sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters_present_flag &&
		sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters)
	{
		auto vui_parameters = sps_nu->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters;

		if (vui_parameters->overscan_info_present_flag)
			params.src_overscan = (VTC_3STATE_SWITCH)vui_parameters->overscan_appropriate_flag;

		if (vui_parameters->video_signal_type_present_flag)
		{
			params.src_video_format = vui_parameters->video_format;
			params.src_video_full_range = (VTC_3STATE_SWITCH)vui_parameters->video_full_range_flag;
			if (vui_parameters->colour_description_present_flag)
			{
				params.src_colour_primaries = (VTC_COLOUR_PRIMARIES)vui_parameters->colour_primaries;
				params.src_transfer_characteristics = (VTC_TRANSFER_CHARACTERISTICS)vui_parameters->transfer_characteristics;
				params.src_matrix_coefficients = (VTC_MATRIX_COEFFICIENTS)vui_parameters->matrix_coeffs;
			}
		}

		if (params.fps_den == 0 && params.fps_num == 0 && vui_parameters->timing_info_present_flag)
		{
			params.fps_num = vui_parameters->time_scale;
			params.fps_den = vui_parameters->num_units_in_tick * 2;

			uint64_t common_divisor = gcd(params.fps_num, params.fps_den);
			params.fps_num = (uint32_t)((uint64_t)params.fps_num / common_divisor);
			params.fps_den = (uint32_t)((uint64_t)params.fps_den / common_divisor);
		}

		if (params.sar_den == 0 && params.sar_num == 0 && vui_parameters->aspect_ratio_info_present_flag)
		{
			if (vui_parameters->aspect_ratio_idc == 0xFF)
			{
				params.sar_num = vui_parameters->sar_width;
				params.sar_den = vui_parameters->sar_height;
			}
			else if (vui_parameters->aspect_ratio_idc >= 0 && vui_parameters->aspect_ratio_idc < _countof(sample_aspect_ratios))
			{
				params.sar_num = std::get<0>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
				params.sar_den = std::get<1>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
			}

			if (params.sar_den > 0 && params.sar_num > 0)
			{
				uint64_t common_divisor = gcd(params.sar_num, params.sar_den);
				params.sar_num = (uint32_t)(params.sar_num / common_divisor);
				params.sar_den = (uint32_t)(params.sar_den / common_divisor);
			}
			else
			{
				printf("Unexpected SAR is found from AVC VUI information.\n");
			}
		}
	}

	m_vtc_export->fn_param_autoselect_profile_tier_level(&params);

	return ValiddateAndUpdateParams(params);
}

RET_CODE CNALAUEnumerator::FinalizeVTCParamsForHEVCSource(INALHEVCContext* pHEVCCtx, H265_NU sps_nu, vtc_param_t& params)
{
	if (params.width <= 0)
		params.width = sps_nu->ptr_seq_parameter_set_rbsp->display_width;;

	if (params.height <= 0)
		params.height = sps_nu->ptr_seq_parameter_set_rbsp->display_height;

	if (params.width <= 0 || params.height <= 0)
	{
		printf("The specified width(%d) or height(%d) is out of range.\n", params.width, params.height);
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (params.sar_den == 0 && params.sar_num == 0)
	{
		// Check the display aspect ratio
		auto iter_dar = g_params.find("DAR");
		if (iter_dar != g_params.end())
		{
			int64_t dar_num = -1, dar_den = -1;
			if (ConvertToRationalNumber(iter_dar->second, dar_num, dar_den) == false ||
				dar_num <= 0 || dar_num > UINT32_MAX ||
				dar_den <= 0 || dar_den > UINT32_MAX)
			{
				printf("The 'DAR' parameter \"--DAR=%s\" can't be parsed, ignore it.\n", iter_dar->second.c_str());
			}
			else
			{
				uint64_t common_divisor = gcd((uint64_t)(dar_num*params.height), (uint64_t)(dar_den*params.width));
				params.sar_num = (uint32_t)(dar_num*params.height / common_divisor);
				params.sar_den = (uint32_t)(dar_den*params.width / common_divisor);
			}
		}
	}

	if (params.src_pulldown == VTC_SWITCH_AUTO &&
		sps_nu->ptr_seq_parameter_set_rbsp->profile_tier_level &&
		sps_nu->ptr_seq_parameter_set_rbsp->profile_tier_level->general_profile_level.progressive_source_flag == 1 &&
		sps_nu->ptr_seq_parameter_set_rbsp->profile_tier_level->general_profile_level.interlaced_source_flag == 0)
	{
		params.src_pulldown = VTC_SWITCH_OFF;
	}

	if (params.target_interlace == VTC_SWITCH_AUTO &&
		sps_nu->ptr_seq_parameter_set_rbsp->profile_tier_level &&
		sps_nu->ptr_seq_parameter_set_rbsp->profile_tier_level->general_profile_level.progressive_source_flag == 1 &&
		sps_nu->ptr_seq_parameter_set_rbsp->profile_tier_level->general_profile_level.interlaced_source_flag == 0)
	{
		params.target_interlace = VTC_SWITCH_OFF;
	}

	if (sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters_present_flag &&
		sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters)
	{
		auto vui_parameters = sps_nu->ptr_seq_parameter_set_rbsp->vui_parameters;

		if (vui_parameters->overscan_info_present_flag)
			params.src_overscan = (VTC_3STATE_SWITCH)vui_parameters->overscan_appropriate_flag;

		if (vui_parameters->video_signal_type_present_flag)
		{
			params.src_video_format = vui_parameters->video_format;
			params.src_video_full_range = (VTC_3STATE_SWITCH)vui_parameters->video_full_range_flag;
			if (vui_parameters->colour_description_present_flag)
			{
				params.src_colour_primaries = (VTC_COLOUR_PRIMARIES)vui_parameters->colour_primaries;
				params.src_transfer_characteristics = (VTC_TRANSFER_CHARACTERISTICS)vui_parameters->transfer_characteristics;
				params.src_matrix_coefficients = (VTC_MATRIX_COEFFICIENTS)vui_parameters->matrix_coeffs;
			}
		}

		if (params.fps_den == 0 && params.fps_num == 0 && vui_parameters->vui_timing_info_present_flag)
		{
			params.fps_num = vui_parameters->vui_time_scale;
			params.fps_den = vui_parameters->vui_num_units_in_tick * (vui_parameters->field_seq_flag + 1);

			uint64_t common_divisor = gcd(params.fps_num, params.fps_den);
			params.fps_num = (uint32_t)((uint64_t)params.fps_num / common_divisor);
			params.fps_den = (uint32_t)((uint64_t)params.fps_den / common_divisor);
		}

		if (params.sar_den == 0 && params.sar_num == 0 && vui_parameters->aspect_ratio_info_present_flag)
		{
			if (vui_parameters->aspect_ratio_idc == 0xFF)
			{
				params.sar_num = vui_parameters->sar_width;
				params.sar_den = vui_parameters->sar_height;
			}
			else if (vui_parameters->aspect_ratio_idc >= 0 && vui_parameters->aspect_ratio_idc < _countof(sample_aspect_ratios))
			{
				params.sar_num = std::get<0>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
				params.sar_den = std::get<1>(sample_aspect_ratios[vui_parameters->aspect_ratio_idc]);
			}

			if (params.sar_den > 0 && params.sar_num > 0)
			{
				uint64_t common_divisor = gcd(params.sar_num, params.sar_den);
				params.sar_num = (uint32_t)(params.sar_num / common_divisor);
				params.sar_den = (uint32_t)(params.sar_den / common_divisor);
			}
			else
			{
				printf("Unexpected SAR is found from AVC VUI information.\n");
			}
		}

		if (params.src_colour_primaries == VTC_CP_UNSPECIFIED || params.src_colour_primaries == VTC_CP_UNKNOWN)
		{
			if (vui_parameters->video_signal_type_present_flag && vui_parameters->colour_description_present_flag)
			{
				switch (vui_parameters->colour_primaries)
				{
				case 1: params.src_colour_primaries = VTC_CP_BT_709; break;
				case 5: params.src_colour_primaries = VTC_CP_BT_470_B_G; break;
				case 6: params.src_colour_primaries = VTC_CP_BT_601; break;
				case 9: params.src_colour_primaries = VTC_CP_BT_2020; break;
				case 10: params.src_colour_primaries = VTC_CP_XYZ; break;
				}

				switch (vui_parameters->transfer_characteristics)
				{
				case 1: params.src_transfer_characteristics = VTC_TC_BT_709; break;
				case 5: params.src_transfer_characteristics = VTC_TC_BT_470_B_G; break;
				case 6: params.src_transfer_characteristics = VTC_TC_BT_601; break;
				case 13: params.src_transfer_characteristics = VTC_TC_SRGB; break;
				case 14: params.src_transfer_characteristics = VTC_TC_BT_2020_10_BIT; break;
				case 15: params.src_transfer_characteristics = VTC_TC_BT_2020_12_BIT; break;
				case 16: params.src_transfer_characteristics = VTC_TC_SMPTE_2084; break;
				case 18: params.src_transfer_characteristics = VTC_TC_HLG; break;
				}

				switch (vui_parameters->matrix_coeffs)
				{
				case 1: params.src_matrix_coefficients = VTC_MC_BT_709; break;
				case 5: params.src_matrix_coefficients = VTC_MC_BT_470_B_G; break;
				case 6: params.src_matrix_coefficients = VTC_MC_BT_601; break;
				case 9: params.src_matrix_coefficients = VTC_MC_BT_2020_NCL; break;
				case 10: params.src_matrix_coefficients = VTC_MC_BT_2020_CL; break;
				}
			}
		}
	}

	m_vtc_export->fn_param_autoselect_profile_tier_level(&params);

	return ValiddateAndUpdateParams(params);
}

RET_CODE CNALAUEnumerator::FinalizeVTCParamsForVVCSource(INALVVCContext* pVVCCtx, H266_NU sps_nu, vtc_param_t& params)
{
	return RET_CODE_ERROR_NOTIMPL;
}

RET_CODE CAV1AUEnumerator::FinalizeVTCParamsForAV1(AV1_OBU seq_hdr_obu, vtc_param_t& params)
{
	if (params.width <= 0)
		params.width = seq_hdr_obu->ptr_sequence_header_obu->max_frame_width_minus_1 + 1;

	if (params.height <= 0)
		params.height = seq_hdr_obu->ptr_sequence_header_obu->max_frame_height_minus_1 + 1;

	if (params.width <= 0 || params.height <= 0)
	{
		printf("The specified width(%d) or height(%d) is out of range.\n", params.width, params.height);
		return RET_CODE_ERROR_NOTIMPL;
	}

	if (params.sar_den == 0 && params.sar_num == 0)
	{
		// Check the display aspect ratio
		auto iter_dar = g_params.find("DAR");
		if (iter_dar != g_params.end())
		{
			int64_t dar_num = -1, dar_den = -1;
			if (ConvertToRationalNumber(iter_dar->second, dar_num, dar_den) == false ||
				dar_num <= 0 || dar_num > UINT32_MAX ||
				dar_den <= 0 || dar_den > UINT32_MAX)
			{
				printf("The 'DAR' parameter \"--DAR=%s\" can't be parsed, ignore it.\n", iter_dar->second.c_str());
			}
			else
			{
				uint64_t common_divisor = gcd((uint64_t)(dar_num*params.height), (uint64_t)(dar_den*params.width));
				params.sar_num = (uint32_t)(dar_num*params.height / common_divisor);
				params.sar_den = (uint32_t)(dar_den*params.width / common_divisor);
			}
		}
	}

	if (seq_hdr_obu->ptr_sequence_header_obu->timing_info_present_flag && seq_hdr_obu->ptr_sequence_header_obu->ptr_timing_info != nullptr)
	{
		if (seq_hdr_obu->ptr_sequence_header_obu->ptr_timing_info->equal_picture_interval)
		{
			if (params.fps_den == 0 && params.fps_num == 0 && 
				seq_hdr_obu->ptr_sequence_header_obu->ptr_timing_info->time_scale > 0 &&
				seq_hdr_obu->ptr_sequence_header_obu->ptr_timing_info->num_units_in_display_tick > 0)
			{
				params.fps_num = seq_hdr_obu->ptr_sequence_header_obu->ptr_timing_info->time_scale;
				params.fps_den = seq_hdr_obu->ptr_sequence_header_obu->ptr_timing_info->num_units_in_display_tick;

				uint64_t common_divisor = gcd(params.fps_num, params.fps_den);
				params.fps_num = (uint32_t)((uint64_t)params.fps_num / common_divisor);
				params.fps_den = (uint32_t)((uint64_t)params.fps_den / common_divisor);
			}
		}
		else
			printf("Not an AV1 video stream with fixed frame-rate.\n");
	}

	// Process SAR...
	// TODO...

	if (params.src_colour_primaries == VTC_CP_UNSPECIFIED || params.src_colour_primaries == VTC_CP_UNKNOWN)
	{
		if (seq_hdr_obu->ptr_sequence_header_obu->color_config &&
			seq_hdr_obu->ptr_sequence_header_obu->color_config->color_description_present_flag)
		{
			switch (seq_hdr_obu->ptr_sequence_header_obu->color_config->color_primaries)
			{
			case 1: params.src_colour_primaries = VTC_CP_BT_709; break;
			case 5: params.src_colour_primaries = VTC_CP_BT_470_B_G; break;
			case 6: params.src_colour_primaries = VTC_CP_BT_601; break;
			case 9: params.src_colour_primaries = VTC_CP_BT_2020; break;
			case 10: params.src_colour_primaries = VTC_CP_XYZ; break;
			}

			switch (seq_hdr_obu->ptr_sequence_header_obu->color_config->transfer_characteristics)
			{
			case 1: params.src_transfer_characteristics = VTC_TC_BT_709; break;
			case 5: params.src_transfer_characteristics = VTC_TC_BT_470_B_G; break;
			case 6: params.src_transfer_characteristics = VTC_TC_BT_601; break;
			case 13: params.src_transfer_characteristics = VTC_TC_SRGB; break;
			case 14: params.src_transfer_characteristics = VTC_TC_BT_2020_10_BIT; break;
			case 15: params.src_transfer_characteristics = VTC_TC_BT_2020_12_BIT; break;
			case 16: params.src_transfer_characteristics = VTC_TC_SMPTE_2084; break;
			case 18: params.src_transfer_characteristics = VTC_TC_HLG; break;
			}

			switch (seq_hdr_obu->ptr_sequence_header_obu->color_config->matrix_coefficients)
			{
			case 1: params.src_matrix_coefficients = VTC_MC_BT_709; break;
			case 5: params.src_matrix_coefficients = VTC_MC_BT_470_B_G; break;
			case 6: params.src_matrix_coefficients = VTC_MC_BT_601; break;
			case 9: params.src_matrix_coefficients = VTC_MC_BT_2020_NCL; break;
			case 10: params.src_matrix_coefficients = VTC_MC_BT_2020_CL; break;
			}
		}
	}

	// AV1 only support progressive
	params.src_pulldown = VTC_SWITCH_OFF;
	params.target_interlace = VTC_SWITCH_OFF;

	m_vtc_export->fn_param_autoselect_profile_tier_level(&params);

	return ValiddateAndUpdateParams(params);
}


