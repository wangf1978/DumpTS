#pragma once

#include "ISO14496_12.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#pragma warning(disable:4201)
#pragma warning(disable:4101)
#pragma warning(disable:4100)
#pragma warning(disable:4189)
#pragma warning(disable:4127)
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

namespace QTFF
{
	/*
		QT Metadata structure is incompatible with ISO 14496-12
	*/
	struct MetaBox : public ISOMediaFile::ContainerBox
	{
		struct MetaDataHeaderAtom : public ISOMediaFile::FullBox
		{
			uint32_t		nextItemID;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				iRet = ISOMediaFile::FullBox::Unpack(bs);
				if (iRet < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(nextItemID))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				nextItemID = bs.GetDWord();
				left_bytes -= sizeof(nextItemID);

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		/*
			https://developer.apple.com/library/content/documentation/QuickTime/QTFF/Metadata/Metadata.html#//apple_ref/doc/uid/TP40000939-CH1-SW1
		*/
		struct MetadataItemKeysAtom : public ISOMediaFile::FullBox
		{
			struct Entry
			{
				int32_t		Key_size;
				int32_t		Key_namespace;
				std::string	Key_value;

				Entry(int32_t keySize, int32_t keyNamespace)
					: Key_size(keySize), Key_namespace(keyNamespace) {
					if (Key_size > 8)
						Key_value.reserve(Key_size - 8);
				}
			}PACKED;

			int32_t				Entry_count;
			std::vector<Entry>	Entries;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				iRet = ISOMediaFile::FullBox::Unpack(bs);
				if (iRet < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(Entry_count))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				Entry_count = bs.GetShort();
				left_bytes -= sizeof(Entry_count);

				if (Entry_count <= 0)
					goto done;

				const size_t entry_header_size = sizeof(int32_t) + sizeof(int32_t);
				while (left_bytes >= entry_header_size && Entries.size() < (size_t)Entry_count)
				{
					Entries.emplace_back(bs.GetLong(), bs.GetLong());

					left_bytes -= entry_header_size;
					if (left_bytes + entry_header_size >= Entries.back().Key_size)
					{
						bs.Read((uint8_t*)(&Entries.back().Key_value[0]), Entries.back().Key_size - entry_header_size);
						left_bytes -= Entries.back().Key_size;
					}
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		struct MetadataItemListAtom : public ISOMediaFile::Box
		{
			struct MetadataItemAtom: public ISOMediaFile::Box
			{
				struct DataAtom : public ISOMediaFile::Box
				{
					uint32_t		Type_Indicator;
					uint32_t		Locale_Indicator;
					std::vector<uint8_t>
									Value;
				}PACKED;

				//struct Value

				struct ItemInformationAtom : public ISOMediaFile::FullBox
				{
					uint32_t		Item_ID;
				}PACKED;

				struct NameAtom : public ISOMediaFile::FullBox
				{
					std::string		Name;
				}PACKED;




			}PACKED;
		}PACKED;

		ISOMediaFile::HandlerBox*	handler_box = nullptr;


		virtual int Unpack(CBitstream& bs)
		{
			int iRet = ISOMediaFile::ContainerBox::Unpack(bs);
			if (iRet < 0)
				return iRet;


		}

	}PACKED;

} // namespace  ISOMediaFile

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

