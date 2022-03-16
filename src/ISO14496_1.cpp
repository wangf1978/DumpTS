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
#include "ISO14496_1.h"

namespace BST
{
	namespace MPEG4System
	{
		int BaseDescriptor::LoadDescriptor(CBitstream& bs, BaseDescriptor** ppDescr)
		{
			int iRet = 0;
			BaseDescriptor* ptr_descr = nullptr;

			try
			{
				uint8_t descr_tag = (uint8_t)bs.PeekBits(8);
				switch (descr_tag)
				{
				case ObjectDescrTag:
				case InitialObjectDescrTag:
				case ES_DescrTag:
				case DecoderConfigDescrTag:
				case DecSpecificInfoTag:
				case SLConfigDescrTag:
				case ContentIdentDescrTag:
					ptr_descr = new UnimplDescriptor();
					break;
				case SupplContentIdentDescrTag:
					ptr_descr = new SupplementaryContentIdentificationDescriptor();
					break;
				case IPI_DescrPointerTag:
					ptr_descr = new IPI_DescrPointer();
					break;
				case IPMP_DescrPointerTag:
				case IPMP_DescrTag:
				case QoS_DescrTag:
				case RegistrationDescrTag:
				case ES_ID_IncTag:
				case ES_ID_RefTag:
				case MP4_IOD_Tag:
				case MP4_OD_Tag:
				case IPL_DescrPointerRefTag:
				case ExtensionProfileLevelDescrTag:
				case profileLevelIndicationIndexDescrTag:
				case ContentClassificationDescrTag:
				case KeyWordDescrTag:
				case RatingDescrTag:
					ptr_descr = new UnimplDescriptor();
					break;
				case LanguageDescrTag:
					ptr_descr = new LanguageDescriptor();
					break;
				case ShortTextualDescrTag:
				case ExpandedTextualDescrTag:
				case ContentCreatorNameDescrTag:
				case ContentCreationDateDescrTag:
				case OCICreatorNameDescrTag:
				case OCICreationDateDescrTag:
				case SmpteCameraPositionDescrTag:
				case SegmentDescrTag:
				case MediaTimeDescrTag:
				case IPMP_ToolsListDescrTag:
				case IPMP_ToolTag:
				case M4MuxTimingDescrTag:
				case M4MuxCodeTableDescrTag:
				case ExtSLConfigDescrTag:
				case M4MuxBufferSizeDescrTag:
				case M4MuxIdentDescrTag:
				case DependencyPointerTag:
				case DependencyMarkerTag:
				case M4MuxChannelDescrTag:
				default:
					ptr_descr = new UnimplDescriptor();
					break;
				}

				if ((iRet = ptr_descr->Unpack(bs)) < 0)
				{
					printf("Failed to load a descriptor from the current bitstream.\n");
					AMP_SAFEASSIGN(ppDescr, nullptr);
					return iRet;
				}

				AMP_SAFEASSIGN(ppDescr, ptr_descr);
			}
			catch (...)
			{
				return -1;
			}

			return 0;
		}

		const char* ObjectTypeIndication_Names[256] = {
			/* 0x00 */		"Forbidden",
			/* 0x01 */		"Systems ISO/IEC 14496-1",
			/* 0x02 */		"Systems ISO/IEC 14496-1",
			/* 0x03 */		"Interaction Stream",
			/* 0x04 */		"Systems ISO/IEC 14496-1 Extended BIFS Configuration",
			/* 0x05 */		"Systems ISO/IEC 14496-1 AFX",
			/* 0x06 */		"Font Data Stream",
			/* 0x07 */		"Synthesized Texture Stream",
			/* 0x08 */		"Streaming Text Stream",
			/* 0x09-0x1F */ "reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use",
			/* 0x20 */		"Visual ISO/IEC 14496-2",
			/* 0x21 */		"Visual ITU-T Recommendation H.264 | ISO/IEC 14496-10",
			/* 0x22 */		"Parameter Sets for ITU-T Recommendation H.264 | ISO/IEC 14496-10",
			/* 0x23-0x3F*/	"reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use",
			/* 0x40 */		"Audio ISO/IEC 14496-3",
			/* 0x41-0x5F*/	"reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use",
			/* 0x60 */		"Visual ISO/IEC 13818-2 Simple Profile",
			/* 0x61 */		"Visual ISO/IEC 13818-2 Main Profile",
			/* 0x62 */		"Visual ISO/IEC 13818-2 SNR Profile",
			/* 0x63 */		"Visual ISO/IEC 13818-2 Spatial Profile",
			/* 0x64 */		"Visual ISO/IEC 13818-2 High Profile",
			/* 0x65 */		"Visual ISO/IEC 13818-2 422 Profile",
			/* 0x66 */		"Audio ISO/IEC 13818-7 Main Profile",
			/* 0x67 */		"Audio ISO/IEC 13818-7 LowComplexity Profile",
			/* 0x68 */		"Audio ISO/IEC 13818-7 Scaleable Sampling Rate Profile",
			/* 0x69 */		"Audio ISO/IEC 13818-3",
			/* 0x6A */		"Visual ISO/IEC 11172-2",
			/* 0x6B */		"Audio ISO/IEC 11172-3",
			/* 0x6C */		"Visual ISO/IEC 10918-1",
			/* 0x6D */		"reserved for registration authority",
			/* 0x6E */		"Visual ISO/IEC 15444-1",
			/* 0x6F-0x9F */	"reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use",
			/* 0xA0-0xBF */	"reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority","reserved for registration authority",
			/* 0xC0-0xE0 */	"user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private",
			/* 0xE1 */		"reserved for registration authority",
			/* 0xE2-0xFE */	"user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private",
			/* 0xFF */		"no object type specified",
		};

		const char* DecoderConfig_StreamType_Names[64] = {
			/* 0x00 */		"Forbidden",
			/* 0x01 */		"ObjectDescriptorStream",
			/* 0x02 */		"ClockReferenceStream",
			/* 0x03 */		"SceneDescriptionStream",
			/* 0x04 */		"VisualStream",
			/* 0x05 */		"AudioStream",
			/* 0x06 */		"MPEG7Stream",
			/* 0x07 */		"IPMPStream",
			/* 0x08 */		"ObjectContentInfoStream",
			/* 0x09 */		"MPEGJStream",
			/* 0x0A */		"Interaction Stream",
			/* 0x0B */		"IPMPToolStream",
			/* 0x0C-0x1F */	"reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use","reserved for ISO use",
			/* 0x20-0x3F */	"user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private","user private",
		};

		std::tuple<const char*, const char*> MP4_descriptor_descs[256] =
		{
			/* 0x00 */	{"Forbidden", ""},
			/* 0x01 */	{"ObjectDescrTag", "" },
			/* 0x02 */	{"InitialObjectDescrTag", "" },
			/* 0x03 */	{"ES_DescrTag", "" },
			/* 0x04 */	{"DecoderConfigDescrTag", "" },
			/* 0x05 */	{"DecSpecificInfoTag", ""},
			/* 0x06 */	{"SLConfigDescrTag", "" },
			/* 0x07 */	{"ContentIdentDescrTag", "" },
			/* 0x08 */	{"SupplContentIdentDescrTag", "" },
			/* 0x09 */	{"IPI_DescrPointerTag", "" },
			/* 0x0A */	{"IPMP_DescrPointerTag", "" },
			/* 0x0B */	{"IPMP_DescrTag", "" },
			/* 0x0C */	{"QoS_DescrTag", "" },
			/* 0x0D */	{"RegistrationDescrTag", "" },
			/* 0x0E */	{"ES_ID_IncTag", "" },
			/* 0x0F */	{"ES_ID_RefTag", "" },
			/* 0x10 */	{"MP4_IOD_Tag", "" },
			/* 0x11 */	{"MP4_OD_Tag", "" },
			/* 0x12 */	{"IPL_DescrPointerRefTag", "" },
			/* 0x13 */	{"ExtensionProfileLevelDescrTag", "" },
			/* 0x14 */	{"profileLevelIndicationIndexDescrTag", "" },
			/* 0x15 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x16 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x17 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x18 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x19 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x1A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x1B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x1C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x1D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x1E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x1F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x20 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x21 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x22 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x23 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x24 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x25 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x26 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x27 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x28 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x29 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x2A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x2B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x2C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x2D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x2E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x2F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x30 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x31 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x32 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x33 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x34 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x35 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x36 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x37 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x38 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x39 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x3A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x3B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x3C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x3D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x3E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x3F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x40 */	{"ContentClassificationDescrTag", "" },
			/* 0x41 */	{"KeyWordDescrTag", "" },
			/* 0x42 */	{"RatingDescrTag", "" },
			/* 0x43 */	{"LanguageDescrTag", "" },
			/* 0x44 */	{"ShortTextualDescrTag", "" },
			/* 0x45 */	{"ExpandedTextualDescrTag", "" },
			/* 0x46 */	{"ContentCreatorNameDescrTag", "" },
			/* 0x47 */	{"ContentCreationDateDescrTag", "" },
			/* 0x48 */	{"OCICreatorNameDescrTag", "" },
			/* 0x49 */	{"OCICreationDateDescrTag", "" },
			/* 0x4A */	{"SmpteCameraPositionDescrTag", "" },
			/* 0x4B */	{"SegmentDescrTag", "" },
			/* 0x4C */	{"MediaTimeDescrTag", "" },
			/* 0x4D */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x4E */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x4F */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x50 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x51 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x52 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x53 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x54 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x55 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x56 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x57 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x58 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x59 */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x5A */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x5B */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x5C */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x5D */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x5E */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x5F */	{"Reserved_for_ISO_use_OCI_extensions", "reserved for ISO use(OCI extensions)"},
			/* 0x60 */	{"IPMP_ToolsListDescrTag", "" },
			/* 0x61 */	{"IPMP_ToolTag", "" },
			/* 0x62 */	{"M4MuxTimingDescrTag", "" },
			/* 0x63 */	{"M4MuxCodeTableDescrTag", "" },
			/* 0x64 */	{"ExtSLConfigDescrTag", "" },
			/* 0x65 */	{"M4MuxBufferSizeDescrTag", "" },
			/* 0x66 */	{"M4MuxIdentDescrTag", "" },
			/* 0x67 */	{"DependencyPointerTag", "" },
			/* 0x68 */	{"DependencyMarkerTag", "" },
			/* 0x69 */	{"M4MuxChannelDescrTag", "" },
			/* 0x6A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x6B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x6C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x6D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x6E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x6F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x70 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x71 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x72 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x73 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x74 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x75 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x76 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x77 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x78 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x79 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x7A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x7B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x7C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x7D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x7E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x7F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x80 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x81 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x82 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x83 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x84 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x85 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x86 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x87 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x88 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x89 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x8A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x8B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x8C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x8D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x8E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x8F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x90 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x91 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x92 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x93 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x94 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x95 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x96 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x97 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x98 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x99 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x9A */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x9B */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x9C */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x9D */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x9E */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0x9F */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA0 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA1 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA2 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA3 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA4 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA5 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA6 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA7 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA8 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xA9 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xAA */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xAB */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xAC */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xAD */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xAE */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xAF */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB0 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB1 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB2 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB3 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB4 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB5 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB6 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB7 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB8 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xB9 */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xBA */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xBB */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xBC */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xBD */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xBE */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xBF */	{"Reserved_for_ISO_use", "reserved for ISO use" },
			/* 0xC0 */	{"User_private", "User private" },
			/* 0xC1 */	{"User_private", "User private" },
			/* 0xC2 */	{"User_private", "User private" },
			/* 0xC3 */	{"User_private", "User private" },
			/* 0xC4 */	{"User_private", "User private" },
			/* 0xC5 */	{"User_private", "User private" },
			/* 0xC6 */	{"User_private", "User private" },
			/* 0xC7 */	{"User_private", "User private" },
			/* 0xC8 */	{"User_private", "User private" },
			/* 0xC9 */	{"User_private", "User private" },
			/* 0xCA */	{"User_private", "User private" },
			/* 0xCB */	{"User_private", "User private" },
			/* 0xCC */	{"User_private", "User private" },
			/* 0xCD */	{"User_private", "User private" },
			/* 0xCE */	{"User_private", "User private" },
			/* 0xCF */	{"User_private", "User private" },
			/* 0xD0 */	{"User_private", "User private" },
			/* 0xD1 */	{"User_private", "User private" },
			/* 0xD2 */	{"User_private", "User private" },
			/* 0xD3 */	{"User_private", "User private" },
			/* 0xD4 */	{"User_private", "User private" },
			/* 0xD5 */	{"User_private", "User private" },
			/* 0xD6 */	{"User_private", "User private" },
			/* 0xD7 */	{"User_private", "User private" },
			/* 0xD8 */	{"User_private", "User private" },
			/* 0xD9 */	{"User_private", "User private" },
			/* 0xDA */	{"User_private", "User private" },
			/* 0xDB */	{"User_private", "User private" },
			/* 0xDC */	{"User_private", "User private" },
			/* 0xDD */	{"User_private", "User private" },
			/* 0xDE */	{"User_private", "User private" },
			/* 0xDF */	{"User_private", "User private" },
			/* 0xE0 */	{"User_private", "User private" },
			/* 0xE1 */	{"User_private", "User private" },
			/* 0xE2 */	{"User_private", "User private" },
			/* 0xE3 */	{"User_private", "User private" },
			/* 0xE4 */	{"User_private", "User private" },
			/* 0xE5 */	{"User_private", "User private" },
			/* 0xE6 */	{"User_private", "User private" },
			/* 0xE7 */	{"User_private", "User private" },
			/* 0xE8 */	{"User_private", "User private" },
			/* 0xE9 */	{"User_private", "User private" },
			/* 0xEA */	{"User_private", "User private" },
			/* 0xEB */	{"User_private", "User private" },
			/* 0xEC */	{"User_private", "User private" },
			/* 0xED */	{"User_private", "User private" },
			/* 0xEE */	{"User_private", "User private" },
			/* 0xEF */	{"User_private", "User private" },
			/* 0xF0 */	{"User_private", "User private" },
			/* 0xF1 */	{"User_private", "User private" },
			/* 0xF2 */	{"User_private", "User private" },
			/* 0xF3 */	{"User_private", "User private" },
			/* 0xF4 */	{"User_private", "User private" },
			/* 0xF5 */	{"User_private", "User private" },
			/* 0xF6 */	{"User_private", "User private" },
			/* 0xF7 */	{"User_private", "User private" },
			/* 0xF8 */	{"User_private", "User private" },
			/* 0xF9 */	{"User_private", "User private" },
			/* 0xFA */	{"User_private", "User private" },
			/* 0xFB */	{"User_private", "User private" },
			/* 0xFC */	{"User_private", "User private" },
			/* 0xFD */	{"User_private", "User private" },
			/* 0xFE */	{"User_private", "User private" },
			/* 0xBF */	{"Forbidden", "" },
		};
	}
}

