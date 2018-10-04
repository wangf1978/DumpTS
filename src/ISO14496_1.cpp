#include "StdAfx.h"
#include "ISO14496_1.h"

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

}
