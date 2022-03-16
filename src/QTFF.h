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

namespace BST
{
	namespace QTFF
	{
		/*
			QT Metadata structure is incompatible with ISO 14496-12
		*/
		struct MetaBox : public ISOBMFF::Box
		{
			struct MetaDataHeaderAtom : public ISOBMFF::FullBox
			{
				uint32_t		nextItemID = 0;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;

					iRet = ISOBMFF::FullBox::Unpack(bs);
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
			struct MetaDataItemKeysAtom : public ISOBMFF::FullBox
			{
				struct Entry
				{
					int32_t		Key_size;
					int32_t		Key_namespace;
					std::string	Key_value;

					Entry(CBitstream& bs)
						: Key_size(bs.GetLong()), Key_namespace(bs.GetLong()) {
						if (Key_size > 8)
							Key_value.resize((size_t)Key_size - 8);
					}
				};

				uint32_t			Entry_count = 0;
				std::vector<Entry>	Entries;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;
					const size_t entry_header_size = sizeof(int32_t) + sizeof(int32_t);

					iRet = ISOBMFF::FullBox::Unpack(bs);
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

					if (Entry_count <= 0)
						goto done;

					while (left_bytes >= entry_header_size && Entries.size() < (size_t)Entry_count)
					{
						Entries.emplace_back(bs);

						left_bytes -= entry_header_size;
						if (Entries.back().Key_size > 0 && left_bytes + entry_header_size >= (uint64_t)Entries.back().Key_size)
						{
							int key_value_size = (int)(Entries.back().Key_size - entry_header_size);
							bs.Read((uint8_t*)(&Entries.back().Key_value[0]), key_value_size);
							left_bytes -= key_value_size;
						}
					}

				done:
					SkipLeftBits(bs);
					return iRet;
				}
			};

			struct MetaDataItemListAtom : public ISOBMFF::Box
			{
				struct MetadataItemAtom : public ISOBMFF::Box
				{
					struct ValueAtom : public ISOBMFF::Box
					{
						uint32_t		Type_Indicator = 0;
						uint32_t		Locale_Indicator = 0;

						virtual int _Unpack(CBitstream& bs)
						{
							int iRet = RET_CODE_SUCCESS;

							iRet = ISOBMFF::Box::Unpack(bs);
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
							return iRet;
						}

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = _Unpack(bs);
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

							iRet = ValueAtom::_Unpack(bs);
							if (iRet < 0)
								return iRet;

							int left_bytes;
							uint64_t box_left_bytes = LeftBytes(bs);
							if (box_left_bytes > INT32_MAX)
								goto done;

							left_bytes = (int)box_left_bytes;
							if (left_bytes > 0)
							{
								Value.resize(left_bytes);
								bs.Read(&Value[0], left_bytes);
							}

						done:
							SkipLeftBits(bs);
							return iRet;
						}
					};

					struct ItemInformationAtom : public ISOBMFF::FullBox
					{
						uint32_t		Item_ID = 0;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = RET_CODE_SUCCESS;

							iRet = ISOBMFF::FullBox::Unpack(bs);
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

					struct NameAtom : public ISOBMFF::FullBox
					{
						std::string		Name;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = RET_CODE_SUCCESS;

							iRet = ISOBMFF::FullBox::Unpack(bs);
							if (iRet < 0)
								return iRet;

							ReadString(bs, Name);
							return iRet;
						}
					};

					ItemInformationAtom*		item_information_atom = nullptr;
					NameAtom*					name_atom = nullptr;
					std::vector<ValueAtom*>		value_atoms;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = RET_CODE_SUCCESS;

						iRet = ISOBMFF::Box::Unpack(bs);
						if (iRet < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						while (left_bytes >= MIN_BOX_SIZE)
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
								ptr_box = new ISOBMFF::UnknownBox();
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
				};

				std::vector<MetadataItemAtom*>	metadata_items;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = RET_CODE_SUCCESS;

					iRet = ISOBMFF::Box::Unpack(bs);
					if (iRet < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					while (left_bytes >= MIN_BOX_SIZE)
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
			};

			struct CountryListAtom : public ISOBMFF::FullBox
			{
				struct Entry
				{
					uint16_t				Country_count;
					std::vector<uint16_t>	Countries;

					Entry(uint16_t CountryCount) : Country_count(CountryCount) {
						if (CountryCount > 0)
							Countries.reserve(CountryCount);
					}
				};

				uint32_t			Entry_count = 0;
				std::vector<Entry>	Entries;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = ISOBMFF::FullBox::Unpack(bs);
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

						auto& back = Entries.back();

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

			};

			struct LanguageListAtom : public ISOBMFF::FullBox
			{
				struct Entry
				{
					uint16_t				Language_count;
					std::vector<uint16_t>	Languages;

					Entry(uint16_t CountryCount) : Language_count(CountryCount) {
						if (CountryCount > 0)
							Languages.reserve(CountryCount);
					}
				};

				uint32_t			Entry_count = 0;
				std::vector<Entry>	Entries;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = ISOBMFF::FullBox::Unpack(bs);
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

						auto& back = Entries.back();

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
			};

			ISOBMFF::HandlerBox*		handler_atom = nullptr;
			MetaDataHeaderAtom*			header_atom = nullptr;
			MetaDataItemKeysAtom*		item_keys_atom = nullptr;
			MetaDataItemListAtom*		item_list_atom = nullptr;
			CountryListAtom*			country_list_atom = nullptr;
			LanguageListAtom*			language_list_atom = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ISOBMFF::Box::Unpack(bs);
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
						if ((iRet = LoadOneBoxBranch(this, bs, &ptr_box)) < 0)
							goto done;

					if (ptr_box->type == 'hdlr')
						handler_atom = (ISOBMFF::HandlerBox*)ptr_box;
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

		struct SoundSampleDescription : public ISOBMFF::MovieBox::TrackBox::MediaBox::MediaInformationBox::SampleTableBox::SampleEntry
		{
			struct AudioChannelLayoutAtom : public ISOBMFF::FullBox
			{

			}PACKED;

			struct TerminatorAtom : public ISOBMFF::Box {
				int Unpack(CBitstream& bs)
				{
					int iRet = ISOBMFF::Box::Unpack(bs);
					SkipLeftBits(bs);
					return iRet;
				}
			}PACKED;

			struct siDecompressionParamAtom : public ISOBMFF::ContainerBox
			{
			}PACKED;

			// for version 0, 1 and 3 common parts
			uint16_t				Version = 0;
			uint16_t				Revision_level = 0;
			uint32_t				Vendor = 0;
			union
			{
				uint16_t				channelcount = 2;
				uint16_t				always3;
			}PACKED;

			union
			{
				uint16_t				samplesize = 16;
				uint16_t				always16;
			}PACKED;

			union
			{
				uint16_t				CompressionID = 0;
				int16_t					alwaysMinus2;
			}PACKED;

			union
			{
				uint16_t				PacketSize = 0;
				uint16_t				always0;
			}PACKED;

			union
			{
				uint32_t				SampleRate;
				uint32_t				always65536;
			}PACKED;

			union
			{
				struct
				{
					// for version 1 extension part
					uint32_t				Samples_per_packet;
					uint32_t				Bytes_per_packet;
					uint32_t				Bytes_per_frame;
					uint32_t				Bytes_per_sample;
				}PACKED;

				struct
				{
					// for version 2
					uint32_t				sizeOfStructOnly;
					uint64_t				audioSampleRate;
					uint32_t				numAudioChannels;
					uint32_t				always7F000000;
					uint32_t				constBitsPerChannel;
					uint32_t				formatSpecificFlags;
					uint32_t				constBytesPerAudioPacket;
					uint32_t				constLPCMFramesPerAudioPacket;
				}PACKED;
			}PACKED;

			int _UnpackHeader(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;

				if ((iRet = SampleEntry::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < 20)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				Version = bs.GetWord();
				Revision_level = bs.GetWord();
				Vendor = bs.GetDWord();
				channelcount = bs.GetWord();
				samplesize = bs.GetWord();
				CompressionID = bs.GetWord();
				PacketSize = bs.GetWord();
				SampleRate = bs.GetDWord();
				left_bytes -= 20;

				if (Version == 1)
				{
					if (left_bytes < 16)
					{
						iRet = RET_CODE_BOX_TOO_SMALL;
						goto done;
					}

					Samples_per_packet = bs.GetDWord();
					Bytes_per_packet = bs.GetDWord();
					Bytes_per_frame = bs.GetDWord();
					Bytes_per_sample = bs.GetDWord();
				}
				else if (Version == 2)
				{
					if (left_bytes < 36)
					{
						iRet = RET_CODE_BOX_TOO_SMALL;
						goto done;
					}

					sizeOfStructOnly = bs.GetDWord();
					audioSampleRate = bs.GetQWord();
					numAudioChannels = bs.GetDWord();
					always7F000000 = bs.GetDWord();
					constBitsPerChannel = bs.GetDWord();
					formatSpecificFlags = bs.GetDWord();
					constBytesPerAudioPacket = bs.GetDWord();
					constLPCMFramesPerAudioPacket = bs.GetDWord();
				}

			done:
				return iRet;
			}

			virtual int Unpack(CBitstream& bs)
			{
				uint64_t left_bytes;
				Box* ptr_box = nullptr;
				int iRet = _UnpackHeader(bs);

				if (iRet < 0)
					goto done;

				left_bytes = LeftBytes(bs);
				while (left_bytes >= MIN_BOX_SIZE && LoadOneBoxBranch(this, bs, &ptr_box) >= 0)
				{
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
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED

