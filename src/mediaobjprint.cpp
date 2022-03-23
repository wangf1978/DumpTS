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
#include "mediaobjprint.h"
#include "mpeg2_video.h"

void PrintMPVSyntaxElement(IUnknown* pCtx, uint8_t* pMPVSE, size_t cbMPVSE, int indent)
{
	IMPVContext* pMPVCtx = nullptr;
	if (pCtx == nullptr)
		return;

	uint8_t* p = pMPVSE;
	size_t cbLeft = cbMPVSE;

	if (cbLeft < 4 || p == nullptr)
		return;

	if (p[0] != 0 || p[1] != 0 || p[2] != 1)
		return;

	uint8_t start_code = p[3];

	if (FAILED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx)))
		return;

	AMBst bst = AMBst_CreateFromBuffer(p, (int)cbLeft);

	try
	{
		if (start_code == SEQUENCE_HEADER_CODE)
		{
			BST::MPEG2Video::CSequenceHeader SeqHdr;
			if (AMP_FAILED(SeqHdr.Map(bst)))
			{
				printf("Can't parse the sequence header.\n");
				goto done;
			}

			PrintMediaObject(&SeqHdr, false, indent);
		}
		else if (start_code == EXTENSION_START_CODE)
		{
			uint8_t extension_start_code_identifier = (p[4] >> 4) & 0xF;
			if (extension_start_code_identifier == SEQUENCE_EXTENSION_ID)
			{
				BST::MPEG2Video::CSequenceExtension SeqExt;
				if (AMP_FAILED(SeqExt.Map(bst)))
				{
					printf("Can't parse the sequence extension.\n");
					goto done;
				}

				PrintMediaObject(&SeqExt, false, indent);
			}
			else if (extension_start_code_identifier == SEQUENCE_DISPLAY_EXTENSION_ID)
			{
				BST::MPEG2Video::CSequenceDisplayExtension SeqDispExt;
				if (AMP_FAILED(SeqDispExt.Map(bst)))
				{
					printf("Can't parse the sequence display extension.\n");
					goto done;
				}

				PrintMediaObject(&SeqDispExt, false, indent);
			}
			else if (extension_start_code_identifier == QUANT_MATRIX_EXTENSION_ID)
			{
				BST::MPEG2Video::CQuantMatrixExtension QuantMatrixExt;
				if (AMP_FAILED(QuantMatrixExt.Map(bst)))
				{
					printf("Can't parse the quantization matrix extension.\n");
					goto done;
				}

				PrintMediaObject(&QuantMatrixExt, false, indent);
			}
			else if (extension_start_code_identifier == COPYRIGHT_EXTENSION_ID)
			{
				BST::MPEG2Video::CCopyrightExtension CopyrightExt;
				if (AMP_FAILED(CopyrightExt.Map(bst)))
				{
					printf("Can't parse copyright extension.\n");
					goto done;
				}

				PrintMediaObject(&CopyrightExt, false, indent);
			}
			else if (extension_start_code_identifier == SEQUENCE_SCALABLE_EXTENSION_ID)
			{
				BST::MPEG2Video::CSequenceScalableExtension SeqScalableExt;
				if (AMP_FAILED(SeqScalableExt.Map(bst)))
				{
					printf("Can't parse sequence scalable extension.\n");
					goto done;
				}

				PrintMediaObject(&SeqScalableExt, false, indent);
			}
			else if (extension_start_code_identifier == PICTURE_DISPLAY_EXTENSION_ID)
			{
				BST::MPEG2Video::CPictureDisplayExtension PicDispExt;
				if (AMP_FAILED(PicDispExt.Map(bst)))
				{
					printf("Can't parse sequence scalable extension.\n");
					goto done;
				}

				PrintMediaObject(&PicDispExt, false, indent);
			}
			else if (extension_start_code_identifier == PICTURE_CODING_EXTENSION_ID)
			{
				BST::MPEG2Video::CPictureCodingExtension PicCodingExt;
				if (AMP_FAILED(PicCodingExt.Map(bst)))
				{
					printf("Can't parse picture coding extension.\n");
					goto done;
				}

				PrintMediaObject(&PicCodingExt, false, indent);
			}
			else if (extension_start_code_identifier == PICTURE_SPATIAL_SCALABLE_EXTENSION_ID)
			{
				BST::MPEG2Video::CPictureSpatialScalableExtension PicSpatialScalableExt;
				if (AMP_FAILED(PicSpatialScalableExt.Map(bst)))
				{
					printf("Can't parse the picture spatial scalable extension.\n");
					goto done;
				}

				PrintMediaObject(&PicSpatialScalableExt, false, indent);
			}
			else if (extension_start_code_identifier == PICTURE_TEMPORAL_SCALABLE_EXTENSION_ID)
			{
				BST::MPEG2Video::CTemporalScalableExtension PicTPScalableExt;
				if (AMP_FAILED(PicTPScalableExt.Map(bst)))
				{
					printf("Can't parse the picture temporal scalable extension.\n");
					goto done;
				}

				PrintMediaObject(&PicTPScalableExt, false, indent);
			}
			else if (extension_start_code_identifier == CAMERA_PARAMETERS_EXTENSION_ID)
			{
				BST::MPEG2Video::CCameraParametersExtension CameraParamExt;
				if (AMP_FAILED(CameraParamExt.Map(bst)))
				{
					printf("Can't parse the camera parameter extension.\n");
					goto done;
				}

				PrintMediaObject(&CameraParamExt, false, indent);
			}
			else if (extension_start_code_identifier == ITUT_EXTENSION_ID)
			{
				BST::MPEG2Video::CITUTExtension ITUTExt;
				if (AMP_FAILED(ITUTExt.Map(bst)))
				{
					printf("Can't parse the ITU-T extension.\n");
					goto done;
				}

				PrintMediaObject(&ITUTExt, false, indent);
			}
			else
			{
				printf("Don't support the extension with extension_start_code_identifier: %X\n", extension_start_code_identifier);
				goto done;
			}
		}
		else if (start_code == USER_DATA_START_CODE)
		{
			BST::MPEG2Video::CUserData UserData;
			if (AMP_FAILED(UserData.Map(bst)))
			{
				printf("Can't parse the user_data.\n");
				goto done;
			}

			PrintMediaObject(&UserData, false, indent);
		}
		else if (start_code == PICTURE_START_CODE)
		{
			BST::MPEG2Video::CPictureHeader PicHdr;
			if (AMP_FAILED(PicHdr.Map(bst)))
			{
				printf("Can't parse the picture header.\n");
				goto done;
			}

			PrintMediaObject(&PicHdr, false, indent);
		}
		else if (start_code == GROUP_START_CODE)
		{
			BST::MPEG2Video::CGroupPicturesHeader GOPHdr;
			if (AMP_FAILED(GOPHdr.Map(bst)))
			{
				printf("Can't parse the group of picture.\n");
				goto done;
			}

			PrintMediaObject(&GOPHdr, false, indent);
		}
		else if (start_code >= 1 && start_code <= 0xAF)
		{
			IMPVContext* pMPVCtx = nullptr;
			if (SUCCEEDED(pCtx->QueryInterface(IID_IMPVContext, (void**)&pMPVCtx)))
			{
				BST::MPEG2Video::CSlice PicSlice(pMPVCtx);
				if (AMP_FAILED(PicSlice.Map(bst)))
				{
					AMP_SAFERELEASE(pMPVCtx);
					printf("Can't parse the slice.\n");
					goto done;
				}

				PrintMediaObject(&PicSlice, false, indent);
				AMP_SAFERELEASE(pMPVCtx);
			}
		}
		else if (start_code == SEQUENCE_ERROR_CODE)
		{
			
		}
	}
	catch (...)
	{
		printf("Can't parse the bitstream.\n");
	}

done:
	AMBst_Destroy(bst);
	AMP_SAFERELEASE(pMPVCtx);
	return;
}

