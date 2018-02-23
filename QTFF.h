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
	struct MetaBox : public ISOMediaFile::Box
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
		struct MetaDataItemKeysAtom : public ISOMediaFile::FullBox
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

		struct MetaDataItemListAtom : public ISOMediaFile::Box
		{
			struct MetadataItemAtom: public ISOMediaFile::Box
			{
				struct ValueAtom : public ISOMediaFile::Box
				{
					uint32_t		Type_Indicator;
					uint32_t		Locale_Indicator;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = RET_CODE_SUCCESS;

						iRet = ISOMediaFile::Box::Unpack(bs);
						if (iRet < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < sizeof(Type_Indicator) + sizeof(Locale_Indicator))
						{
							iRet = RET_CODE_BOX_TOO_SMALL;
							goto done;
						}

						Type_Indicator = bs.GetDWord();
						Locale_Indicator = bs.GetDWord();

						left_bytes -= sizeof(Type_Indicator) + sizeof(Locale_Indicator);

					done:
						SkipLeftBits(bs);
						return iRet;
					}
				}PACKED;

				struct DataAtom : public ValueAtom
				{
					std::vector<uint8_t>	Value;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = RET_CODE_SUCCESS;

						iRet = ValueAtom::Unpack(bs);
						if (iRet < 0)
							return iRet;

						uint64_t box_left_bytes = LeftBytes(bs);
						if (box_left_bytes > (size_t)-1)
							goto done;

						size_t left_bytes = (size_t)LeftBytes(bs);
						if (left_bytes > 0)
						{
							Value.resize(left_bytes);
							bs.Read(&Value[0], left_bytes);
						}

					done:
						SkipLeftBits(bs);
						return iRet;
					}
				}PACKED;

				struct ItemInformationAtom : public ISOMediaFile::FullBox
				{
					uint32_t		Item_ID;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = RET_CODE_SUCCESS;

						iRet = ISOMediaFile::FullBox::Unpack(bs);
						if (iRet < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < sizeof(Item_ID))
						{
							iRet = RET_CODE_BOX_TOO_SMALL;
							goto done;
						}

						Item_ID = bs.GetDWord();
						left_bytes -= sizeof(Item_ID);

					done:
						SkipLeftBits(bs);
						return iRet;
					}
				}PACKED;

				struct NameAtom : public ISOMediaFile::FullBox
				{
					std::string		Name;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = RET_CODE_SUCCESS;

						iRet = ISOMediaFile::FullBox::Unpack(bs);
						if (iRet < 0)
							return iRet;

						ReadString(bs, Name);
						return iRet;
					}
				}PACKED;

				ItemInformationAtom*		item_information_atom;
				NameAtom*					name_atom;
				std::vector<ValueAtom*>		value_atoms;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;

					iRet = ISOMediaFile::Box::Unpack(bs);
					if (iRet < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					while(left_bytes >= MIN_BOX_SIZE)
					{
						Box* ptr_box = NULL;
						uint64_t box_header = bs.PeekBits(64);
						switch ((box_header&UINT32_MAX))
						{
						case 'itif':
							ptr_box = new ItemInformationAtom();
							break;
						case 'name':
							ptr_box = new NameAtom();
							break;
						case 'data':
							ptr_box = new DataAtom();
							break;
						default:
							ptr_box = new ISOMediaFile::UnknownBox();
							break;
						}

						AppendChildBox(ptr_box);

						if ((iRet = ptr_box->Unpack(bs)) < 0)
						{
							printf("Failed to load a QT atom from the current QT bitstream.\n");

							RemoveChildBox(ptr_box);
							goto done;
						}

						switch (ptr_box->type)
						{
						case 'itif':
							item_information_atom = (ItemInformationAtom*)ptr_box;
							break;
						case 'name':
							name_atom = (NameAtom*)ptr_box;
							break;
						case 'data':
							value_atoms.push_back((ValueAtom*)ptr_box);
							break;
						}

						if (left_bytes < ptr_box->size)
							break;

						left_bytes -= ptr_box->size;
					}

				done:
					SkipLeftBits(bs);
					return iRet;
				}
			}PACKED;

			std::vector<MetadataItemAtom*>	metadata_items;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				iRet = ISOMediaFile::Box::Unpack(bs);
				if (iRet < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				while(left_bytes >= MIN_BOX_SIZE)
				{
					MetadataItemAtom* ptr_box = new MetadataItemAtom();
					AppendChildBox(ptr_box);

					if ((iRet = ptr_box->Unpack(bs)) < 0)
					{
						printf("Failed to load a QT metadata item atom from the current QT bitstream.\n");

						RemoveChildBox(ptr_box);
						goto done;
					}

					metadata_items.push_back(ptr_box);

					if (left_bytes < ptr_box->size)
						break;

					left_bytes -= ptr_box->size;
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}
		}PACKED;

		struct CountryListAtom : public ISOMediaFile::FullBox
		{
			struct Entry
			{
				uint16_t				Country_count;
				std::vector<uint16_t>	Countries;

				Entry(uint16_t CountryCount) : Country_count(CountryCount) {
					if (CountryCount > 0)
						Countries.reserve(CountryCount);
				}
			}PACKED;

			uint32_t			Entry_count;
			std::vector<Entry>	Entries;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ISOMediaFile::FullBox::Unpack(bs);
				if (iRet < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(Entry_count))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				Entry_count = bs.GetDWord();
				left_bytes -= sizeof(Entry_count);

				while (left_bytes >= sizeof(uint16_t) && Entries.size() < Entry_count)
				{
					Entries.emplace_back(bs.GetWord());
					left_bytes -= sizeof(uint16_t);

					auto back = Entries.back();

					while (left_bytes >= sizeof(uint16_t) && back.Countries.size() < back.Country_count)
					{
						back.Countries.push_back(bs.GetWord());
						left_bytes -= sizeof(uint16_t);
					}
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		struct LanguageListAtom : public ISOMediaFile::FullBox
		{
			struct Entry
			{
				uint16_t				Language_count;
				std::vector<uint16_t>	Languages;

				Entry(uint16_t CountryCount) : Language_count(CountryCount) {
					if (CountryCount > 0)
						Languages.reserve(CountryCount);
				}
			}PACKED;

			uint32_t			Entry_count;
			std::vector<Entry>	Entries;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ISOMediaFile::FullBox::Unpack(bs);
				if (iRet < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(Entry_count))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				Entry_count = bs.GetDWord();
				left_bytes -= sizeof(Entry_count);

				while (left_bytes >= sizeof(uint16_t) && Entries.size() < Entry_count)
				{
					Entries.emplace_back(bs.GetWord());
					left_bytes -= sizeof(uint16_t);

					auto back = Entries.back();

					while (left_bytes >= sizeof(uint16_t) && back.Languages.size() < back.Language_count)
					{
						back.Languages.push_back(bs.GetWord());
						left_bytes -= sizeof(uint16_t);
					}
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		ISOMediaFile::HandlerBox*	handler_atom = nullptr;
		MetaDataHeaderAtom*			header_atom = nullptr;
		MetaDataItemKeysAtom*		item_keys_atom = nullptr;
		MetaDataItemListAtom*		item_list_atom = nullptr;
		CountryListAtom*			country_list_atom = nullptr;
		LanguageListAtom*			language_list_atom = nullptr;

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = ISOMediaFile::Box::Unpack(bs);
			if (iRet < 0)
				return iRet;

			uint64_t left_bytes = LeftBytes(bs);
			while (left_bytes >= MIN_BOX_SIZE)
			{
				Box* ptr_box = NULL;
				uint64_t box_header = bs.PeekBits(64);
				switch ((box_header&UINT32_MAX))
				{
				case 'mhdr':
					ptr_box = new MetaDataHeaderAtom();
					break;
				case 'keys':
					ptr_box = new MetaDataItemKeysAtom();
					break;
				case 'ilst':
					ptr_box = new MetaDataItemListAtom();
					break;
				case 'ctry':
					ptr_box = new CountryListAtom();
					break;
				case 'lang':
					ptr_box = new LanguageListAtom();
					break;
				}

				if (ptr_box != nullptr)
				{
					AppendChildBox(ptr_box);

					if ((iRet = ptr_box->Unpack(bs)) < 0)
					{
						printf("Failed to load a QT atom from the current QT bitstream.\n");

						RemoveChildBox(ptr_box);
						goto done;
					}
				}
				else
					if ((iRet = LoadBoxes(this, bs, &ptr_box)) < 0)
						goto done;

				if (ptr_box->type == 'hdlr')
					handler_atom = (ISOMediaFile::HandlerBox*)ptr_box;
				else if (ptr_box->type == 'mhdr')
					header_atom = (MetaDataHeaderAtom*)ptr_box;
				else if (ptr_box->type == 'keys')
					item_keys_atom = (MetaDataItemKeysAtom*)ptr_box;
				else if (ptr_box->type == 'ilst')
					item_list_atom = (MetaDataItemListAtom*)ptr_box;
				else if (ptr_box->type == 'ctry')
					country_list_atom = (CountryListAtom*)ptr_box;
				else if (ptr_box->type == 'lang')
					language_list_atom = (LanguageListAtom*)ptr_box;

				if (left_bytes < ptr_box->size)
					break;

				left_bytes -= ptr_box->size;
			}

		done:
			SkipLeftBits(bs);
			return iRet;
		}

	}PACKED;

} // namespace  ISOMediaFile

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

