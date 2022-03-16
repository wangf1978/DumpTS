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

#include <stdint.h>
#include "Bitstream.h"
#include "combase.h"
#include "DataUtil.h"
#include <algorithm>
#include <list>

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

#define MIN_BOX_SIZE			8
#define MIN_FULLBOX_SIZE		12
#define MAX_DISPLAY_COUNT		4096

//#ifdef _MSC_VER
//#define INLINE __forceinline /* use __forceinline (VC++ specific) */
//#else
//#define INLINE inline        /* use standard inline */
//#endif

#define PRINT_AC(c)	(isprint(c)?(c):'.')
#define PRINT_WC(c)	(iswprint(c)?(c):L'.')
#ifdef _UNICODE
#define PRINT_TC(c)	PRINT_WC(c)
#else
#define PRINT_TC(c)	PRINT_AC(c)
#endif

extern int g_verbose_level;

namespace BST
{
	namespace ISOBMFF
	{
		struct BOX_DESCRIPTOR
		{
			uint32_t	box_type;
			uint8_t		mandatory;
			uint8_t		level;
		};

		extern std::unordered_map<uint32_t, const char*> box_desces;
		extern std::unordered_map<uint32_t, const char*> handle_type_names;
		extern BOX_DESCRIPTOR box_descriptors[97];

		void PrintISOBMFFBox(int64_t box_type = -1LL);

		class IBox: public IUnknown
		{
		public:
			virtual int Unpack(CBitstream& bs) = 0;
			virtual int Pack(CBitstream& bs) = 0;
			virtual int Clean() = 0;
				virtual std::string GetTypeName() = 0;
		};

		struct Box: public CComUnknown, public IBox
		{
			/*
			If the most significant bit of size is 1, it means 
				the size value in Box object is 1, 
				the large-size value is the left bits.
			*/
			struct
			{
				uint64_t	largesize_used : 1;
				uint64_t	size : 63;
			}PACKED;
			uint32_t	type = 0;

			uint64_t	start_bitpos = 0;

			Box*		next_sibling = nullptr;
			Box*		first_child = nullptr;
			Box*		container = nullptr;

			DECLARE_IUNKNOWN

			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_IBox)
					return GetCOMInterface((IBox*)this, ppvObj);
			
				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

			Box(){ largesize_used = 0;  size = MIN_BOX_SIZE; }

			Box(Box* pContainerBox): container(pContainerBox){
				largesize_used = 0;  size = MIN_BOX_SIZE;
			}

			Box(uint64_t box_size, uint32_t box_type) : size(box_size), type(box_type) {
				largesize_used = 0;  size = MIN_BOX_SIZE;
			}

			virtual ~Box() { Clean(); }

			virtual int Unpack(CBitstream& bs)
			{
				start_bitpos = bs.Tell();
				AMP_Assert(start_bitpos % 8 == 0);

				size = bs.GetDWord();
				type = bs.GetDWord();

				if (size == 1)
				{
					largesize_used = 1;
					size = bs.GetQWord();
				}
				else if (size == 0)
				{
					// box extends to end of file
				}
				else if (size < MIN_BOX_SIZE)
				{
					// Ignore the current box
					return RET_CODE_ERROR;
				}

				return 0;
			}

			virtual int Pack(CBitstream& bs)
			{
				return -1;
			}

			virtual int Clean()
			{
				// delete all its children
				Box* ptr_child = first_child;
				while (ptr_child)
				{
					Box* ptr_front = ptr_child;
					ptr_child = ptr_child->next_sibling;
					delete ptr_front;
				}

				first_child = nullptr;

				return 0;
			}

			virtual std::string GetTypeName()
			{
				std::string str_ret;
				str_ret.reserve(16);

				str_ret.push_back(PRINT_AC((type >> 24) & 0xFF));
				str_ret.push_back(PRINT_AC((type >> 16) & 0xFF));
				str_ret.push_back(PRINT_AC((type >> 8) & 0xFF));
				str_ret.push_back(PRINT_AC(type & 0xFF));

				return str_ret;
			}

			virtual int Load(CBitstream& bs)
			{
				return Box::Load(this, bs);
			}

			void SkipLeftBits(CBitstream& bs)
			{
				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes > 0)
					bs.SkipBits(left_bytes << 3);
			}

			uint64_t LeftBytes(CBitstream& bs)
			{
				if (size == 0)
					return UINT64_MAX;

				uint64_t unpack_bits = bs.Tell() - start_bitpos;
				AMP_Assert(unpack_bits % 8 == 0);

				if (size > ((unpack_bits) >> 3))
					return (size - ((unpack_bits) >> 3));

				return 0;
			}

			inline void ReadString(CBitstream& bs, std::string& str)
			{
				uint64_t left_bytes = LeftBytes(bs);

				try
				{
					for (uint64_t i = 0; i < left_bytes; i++)
					{
						char c = bs.GetChar();
						str.push_back(c);

						if (c == '\0')
							break;
					}
				}
				catch (...)
				{
					return;
				}
			}

			void AppendChildBox(Box* child) noexcept
			{
				// Before appending the child box into the current box, the client must ensure
				// it is has already detached from the original box tree
				assert(child->container == nullptr && child->next_sibling == nullptr);

				child->container = this;

				// append the child to the last child
				if (first_child == NULL)
					first_child = child;
				else
				{
					// find the last child
					Box* ptr_child = first_child;
					while (ptr_child->next_sibling)
						ptr_child = ptr_child->next_sibling;

					ptr_child->next_sibling = child;
				}
			}

			/*!	@brief Find the box by the specified pattern, for example, /meta/iloc
				@param pattern the find pattern contains FOURCC tags and '/'
				@remarks as for the patter, if it starts with /, it means a absolute pattern;
				If it starts with ., start find the box from the current pattern;
				otherwise it is relative pattern
			*/
			std::vector<Box*> FindBox(const char* pattern);

			void RemoveChildBox(Box* child) noexcept
			{
				// Find this child from tree
				if (first_child == nullptr)
					return;

				Box* ptr_child = first_child;
				while (ptr_child->next_sibling != child && ptr_child->next_sibling != nullptr)
					ptr_child = ptr_child->next_sibling;

				Box* ptr_cur_box = ptr_child->next_sibling;
				if (ptr_cur_box != nullptr)
				{
					ptr_child->next_sibling = ptr_cur_box->next_sibling;
					ptr_cur_box->container = nullptr;

					delete ptr_cur_box;
				}
			}

			Box* RootBox();
			bool IsQT();
			bool IsHEIC();

			// bool: in use or not
			static std::list<Box> root_box_list;

			static Box& CreateRootBox();
			static void DestroyRootBox(Box &root_box);

			/*!	@brief Load a child box and all descendant boxes of this child box from the current bitstream position */
			static int LoadOneBoxBranch(Box* pContainer, CBitstream& bs, Box** ppBox = nullptr);

			static int Load(Box* pContainer, CBitstream& bs);

			static uint32_t GetMediaTimeScale(Box& root_box, uint32_t track_ID);
			static uint32_t GetMovieTimeScale(Box& root_box);

		}PACKED;

		struct UUIDBox : public Box
		{
			uint8_t		usertype[16] = { 0 };

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				if (LeftBytes(bs) < sizeof(usertype))
					return RET_CODE_ERROR;

				for (size_t i = 0; i < sizeof(usertype); i++)
					usertype[i] = bs.GetByte();

				SkipLeftBits(bs);

				return 0;
			}
		}PACKED;

		struct FullBox : public Box
		{
			uint32_t	version : 8;
			uint32_t	flags : 24;

			FullBox() : version(0), flags(0) {
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				if (LeftBytes(bs) < 4)
					return RET_CODE_ERROR;
			
				version = (uint8_t)bs.GetBits(8);
				flags = (uint32_t)bs.GetBits(24);

				return 0;
			}
		}PACKED;

		// Unknown box type which is not defined ISO spec
		struct UnknownBox : public Box
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				SkipLeftBits(bs);
				return 0;
			}
		}PACKED;

		struct ContainerBox : public Box
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_box_size = LeftBytes(bs);

				Box* ptr_box = nullptr;
				uint64_t child_sum_size = 0;
				while (child_sum_size < left_box_size && LoadOneBoxBranch(this, bs, &ptr_box) >= 0)
				{
					if (g_verbose_level > 0)
						printf("box-type: %c%c%c%c(0X%X), size: 0X%" PRIX64 "\n", 
							(ptr_box->type>>24)&0xFF, (ptr_box->type >> 16) & 0xFF, (ptr_box->type >> 8) & 0xFF, ptr_box->type& 0xFF, ptr_box->type, ptr_box->size);
					child_sum_size += ptr_box->size;
				}

				SkipLeftBits(bs);

				return iRet;
			}
		}PACKED;

		struct ContainerFullBox : public FullBox
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_box_size = LeftBytes(bs);

				Box* ptr_box = nullptr;
				uint64_t child_sum_size = 0;
				while (child_sum_size < left_box_size && LoadOneBoxBranch(this, bs, &ptr_box) >= 0)
				{
					child_sum_size += ptr_box->size;
				}

				SkipLeftBits(bs);

				return iRet;
			}
		}PACKED;

		/*
		Box Type: 'ftyp'
		Container: File
		Mandatory: Yes
		Quantity: Exactly one (but see below)
		*/
		struct FileTypeBox : public Box
		{
			uint32_t				major_brand = 0;
			uint32_t				minor_version  = 0;
			std::vector<uint32_t>	compatible_brands;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(major_brand) + sizeof(minor_version))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				major_brand = bs.GetDWord();
				minor_version = bs.GetDWord();

				left_bytes -= sizeof(major_brand) + sizeof(minor_version);

				try
				{
					while (left_bytes >= sizeof(uint32_t))
					{
						compatible_brands.push_back(bs.GetDWord());
						left_bytes -= sizeof(uint32_t);
					}
				}
				catch (...)
				{
					iRet = RET_CODE_SUCCESS;
					goto done;
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}
		};

		/*
		Box Type: 'mdat'
		Container: File
		Mandatory: No
		Quantity: Zero or more
		*/
		struct MediaDataBox : public Box
		{
			uint8_t		data[];

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				SkipLeftBits(bs);

				return 0;
			}
		}PACKED;

		/*
		Box Types: 'free' 'skip'
		Container: File or other box
		Mandatory: No
		Quantity: Zero or more
		*/
		struct FreeSpaceBox : public Box
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				SkipLeftBits(bs);

				return 0;
			}
		}PACKED;

		/*
		Box Types: 'pdin'
		Container: File
		Mandatory: No
		Quantity: Zero or One
		*/
		struct ProgressiveDownloadInfoBox : public FullBox
		{
			struct Info
			{
				uint32_t	rate;
				uint32_t	initial_delay;
			}PACKED;

			std::vector<Info>	infos;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes >= sizeof(Info))
				{
					try
					{
						infos.resize((size_t)(left_bytes / sizeof(Info)));
						for (size_t i = 0; i < infos.size(); i++)
						{
							infos[i].rate = bs.GetDWord();
							infos[i].initial_delay = bs.GetDWord();
						}
					}
					catch (...)
					{
						iRet = RET_CODE_SUCCESS;
						goto done;
					}
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}
		};

		/*
		Box Type: 'hdlr'
		Container: Media Box ('mdia') or Meta Box ('meta')
		Mandatory: Yes
		Quantity: Exactly one
		*/
		struct HandlerBox : public FullBox
		{
			uint32_t		pre_defined = 0;
			uint32_t		handler_type = 0;
			uint32_t		reserved[3] = { 0 };
			std::string		name;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				size_t unpack_size = sizeof(pre_defined) + sizeof(handler_type) + sizeof(reserved);
				if (left_bytes < unpack_size)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				pre_defined = bs.GetDWord();
				handler_type = bs.GetDWord();
				for (size_t i = 0; i < _countof(reserved); i++)
					reserved[i] = bs.GetDWord();
				left_bytes -= unpack_size;

				ReadString(bs, name);

			done:
				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Type: 'dinf'
		Container: Media Information Box ('minf') or Meta Box ('meta')
		Mandatory: Yes (required within 'minf' box) and No (optional within 'meta' box)
		Quantity: Exactly one
		*/
		struct DataInformationBox : public ContainerBox
		{
			/*
			Box Types: 'url ', 'urn ',  'dref'
			Container: Data Information Box ('dinf')
			Mandatory: Yes
			Quantity: Exactly one
			*/
			struct DataEntryUrlBox : public FullBox
			{
				std::string location;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					ReadString(bs, location);

					SkipLeftBits(bs);
					return 0;
				}
			};

			struct DataEntryUrnBox : public FullBox
			{
				std::string name;
				std::string location;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					ReadString(bs, name);
					ReadString(bs, location);

					SkipLeftBits(bs);
					return 0;
				}
			};

			struct DataReferenceEntry : public FullBox
			{
				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
				
					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					SkipLeftBits(bs);
					return iRet;
				}
			}PACKED;

			struct DataReferenceBox : public FullBox
			{
				uint32_t			entry_count = 0;
				std::vector<Box*>	data_entries;

				~DataReferenceBox() {
					for (size_t i = 0; i < data_entries.size(); i++)
						if (data_entries[i] != NULL)
							delete data_entries[i];
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					UnknownBox unknBox;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(entry_count))
					{
						iRet = RET_CODE_BOX_TOO_SMALL;
						goto done;
					}

					entry_count = bs.GetDWord();
					left_bytes -= 4;

					while (left_bytes >= MIN_BOX_SIZE)
					{
						Box* ptr_box = NULL;
						uint64_t box_header = bs.PeekBits(64);
						uint32_t box_type = (box_header&UINT32_MAX);
 						if (box_type == 'url ')
							ptr_box = new DataEntryUrlBox();
						else if (box_type == 'urn ')
							ptr_box = new DataEntryUrnBox();
						else
							ptr_box = new DataReferenceEntry();

						if ((iRet = ptr_box->Unpack(bs)) < 0)
						{
							delete ptr_box;
							break;
						}

						left_bytes -= ptr_box->size;

						data_entries.push_back(ptr_box);
					}

				done:
					SkipLeftBits(bs);
					return 0;
				};

			};

			DataReferenceBox*	data_reference_box = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ContainerBox::Unpack(bs);
			
				if (iRet < 0)
					return iRet;

				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					if (ptr_child->type == 'dref')
						data_reference_box = (DataReferenceBox*)ptr_child;

					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}
		}PACKED;

		/*
		Box Type: 'sbgp'
		Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
		Mandatory: No
		Quantity: Zero or more.
		*/
		struct SampleToGroupBox : public FullBox
		{
			union Entry
			{
				struct
				{
					uint32_t	sample_count;
					uint32_t	group_description_index;
				}PACKED;
				uint64_t	uint64_val;

				Entry(CBitstream& bs)
					:sample_count(bs.GetDWord()), group_description_index(bs.GetDWord()) {
				}
			}PACKED;

			uint32_t	grouping_type = 0;
			struct
			{
				uint32_t	grouping_type_parameter = 0;
			}PACKED v1;
			uint32_t			entry_count = 0;
			std::vector<Entry>	entries;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(grouping_type))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				grouping_type = bs.GetDWord();
				left_bytes -= sizeof(grouping_type);

				if (version == 1)
				{
					if (left_bytes < sizeof(v1))
					{
						iRet = RET_CODE_BOX_TOO_SMALL;
						goto done;
					}

					v1.grouping_type_parameter = bs.GetDWord();
					left_bytes -= sizeof(uint32_t);
				}

				if (left_bytes < sizeof(entry_count))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				entry_count = bs.GetDWord();
				left_bytes -= sizeof(uint32_t);

				if (left_bytes < (uint64_t)sizeof(Entry)*entry_count)
					printf("The 'sbgp' box size is too small {size: %" PRIu64 ", entry_count: %" PRIu32 "}.\n", size, entry_count);

				while (left_bytes >= sizeof(Entry) && entries.size() < entry_count)
				{
					entries.emplace_back(bs);
					left_bytes -= sizeof(Entry);
				}

			done:
				SkipLeftBits(bs);
				return 0;
			};
		};

		struct SampleGroupDescriptionEntry
		{
			uint32_t	grouping_type;
			uint32_t	description_length;

			SampleGroupDescriptionEntry(uint32_t nGroupingType, uint32_t nSize) : 
				grouping_type(nGroupingType), description_length(nSize) {
			}

			virtual ~SampleGroupDescriptionEntry(){}

			virtual int Unpack(CBitstream& bs)
			{
				if (description_length > 0)
					bs.SkipBits((int64_t)description_length << 3);

				return RET_CODE_SUCCESS;
			}
		}PACKED;

		struct VisualSampleGroupEntry : public SampleGroupDescriptionEntry
		{
			VisualSampleGroupEntry(uint32_t nGroupingType, uint32_t nSize) : SampleGroupDescriptionEntry(nGroupingType, nSize) {
			}
		}PACKED;

		struct AudioSampleGroupEntry : public SampleGroupDescriptionEntry
		{
			AudioSampleGroupEntry(uint32_t nGroupingType, uint32_t nSize) : SampleGroupDescriptionEntry(nGroupingType, nSize) {
			}
		}PACKED;

		struct HintSampleGroupEntry : public SampleGroupDescriptionEntry
		{
			HintSampleGroupEntry(uint32_t nGroupingType, uint32_t nSize) : SampleGroupDescriptionEntry(nGroupingType, nSize) {
			}
		}PACKED;

		struct VisualRollRecoveryEntry : public VisualSampleGroupEntry
		{
			int16_t				roll_distance = 0;

			VisualRollRecoveryEntry(uint32_t nGroupingType, uint32_t nSize) : VisualSampleGroupEntry(nGroupingType, nSize) {
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				uint32_t left_bytes = description_length;

				if (left_bytes < sizeof(roll_distance))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				roll_distance = bs.GetShort();

				left_bytes -= sizeof(roll_distance);

			done:
				if (left_bytes > 0)
					bs.SkipBits((int64_t)left_bytes << 3);
				return iRet;
			}
		}PACKED;

		struct AudioRollRecoveryEntry : public AudioSampleGroupEntry
		{
			int16_t				roll_distance = 0;

			AudioRollRecoveryEntry(uint32_t nGroupingType, uint32_t nSize) : AudioSampleGroupEntry(nGroupingType, nSize) {
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				uint32_t left_bytes = description_length;

				if (left_bytes < sizeof(roll_distance))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				roll_distance = bs.GetShort();

				left_bytes -= sizeof(roll_distance);

			done:
				if (left_bytes > 0)
					bs.SkipBits((int64_t)left_bytes << 3);
				return iRet;
			}
		}PACKED;

		struct AlternativeStartupEntry : public VisualSampleGroupEntry
		{
			struct Entry
			{
				uint16_t			num_output_samples = 0;
				uint16_t			num_total_samples = 0;
			}PACKED;

			uint16_t			roll_count = 0;
			uint16_t			first_output_sample = 0;
			std::vector<uint32_t>
								sample_offsets;
			std::vector<Entry>	entries;

			AlternativeStartupEntry(uint32_t nGroupingType, uint32_t nSize) : VisualSampleGroupEntry(nGroupingType, nSize) {
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				uint32_t left_bytes = description_length;

				if (left_bytes < sizeof(roll_count) + sizeof(first_output_sample))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				roll_count = bs.GetWord();
				first_output_sample = bs.GetWord();
				left_bytes -= sizeof(roll_count) + sizeof(first_output_sample);

				sample_offsets.reserve(roll_count);
				for (size_t i = 0; i < roll_count && left_bytes >= 4; i++) {
					sample_offsets.push_back(bs.GetDWord());
					left_bytes -= 4;
				}

				if (left_bytes >= 4)
				{
					entries.reserve(left_bytes / 4);
					while (left_bytes >= 4)
					{
						Entry entry;
						entry.num_output_samples = bs.GetWord();
						entry.num_total_samples = bs.GetWord();
						entries.push_back(entry);
						left_bytes -= 4;
					}
				}

			done:
				if (left_bytes > 0)
					bs.SkipBits(((int64_t)left_bytes << 3));
				return iRet;
			}
		};

		struct VisualRandomAccessEntry : public VisualSampleGroupEntry
		{
			uint8_t				num_leading_samples_known : 1;
			uint8_t				num_leading_samples : 7;

			VisualRandomAccessEntry(uint32_t nGroupingType, uint32_t nSize) 
				: VisualSampleGroupEntry(nGroupingType, nSize)
				, num_leading_samples_known(0)
				, num_leading_samples(0){
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				uint32_t left_bytes = description_length;

				if (left_bytes < sizeof(uint8_t))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				num_leading_samples_known = (uint8_t)bs.GetBits(1);
				num_leading_samples = (uint8_t)bs.GetBits(7);

				left_bytes -= sizeof(uint8_t);

			done:
				if (left_bytes > 0)
					bs.SkipBits(((int64_t)left_bytes << 3));
				return iRet;
			}
		}PACKED;

		struct TemporalLevelEntry : public VisualSampleGroupEntry
		{
			uint8_t				level_independently_decodable : 1;
			uint8_t				reserved : 7;

			TemporalLevelEntry(uint32_t nGroupingType, uint32_t nSize) 
				: VisualSampleGroupEntry(nGroupingType, nSize)
				, level_independently_decodable(0)
				, reserved(0){
			}

			int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				uint32_t left_bytes = description_length;

				if (left_bytes < sizeof(uint8_t))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				level_independently_decodable = (uint8_t)bs.GetBits(1);
				reserved = (uint8_t)bs.GetBits(7);

				left_bytes -= sizeof(uint8_t);

			done:
				if (left_bytes > 0)
					bs.SkipBits(((int64_t)left_bytes << 3));
				return iRet;
			}
		}PACKED;

		/*
		Box Type: 'sgpd'
		Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
		Mandatory: No
		Quantity: Zero or more, with one for each Sample to Group Box.
		*/
		struct SampleGroupDescriptionBox : public FullBox
		{
			struct Entry
			{
				uint32_t	description_length;
				SampleGroupDescriptionEntry*
							sample_group_description_entry;
			}PACKED;

			uint32_t			grouping_type = 0;
			uint32_t			default_length = 0;
			uint32_t			entry_count = 0;
			std::vector<Entry>	entries;

			uint32_t			handler_type = 0;

			virtual ~SampleGroupDescriptionBox()
			{
				for (auto& v : entries)
				{
					AMP_SAFEDEL2(v.sample_group_description_entry);
				}
			}

			virtual int Unpack(CBitstream& bs);
		};

		/*
		Box Type: 'subs'
		Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
		Mandatory: No
		Quantity: Zero or one
		*/
		struct SubSampleInformationBox : public FullBox
		{
			struct Entry
			{
				struct SubSample
				{
					uint32_t	subsample_size = 0;
					uint8_t		subsample_priority = 0;
					uint8_t		discardable = 0;
					uint32_t	reserved = 0;
				}PACKED;

				uint32_t	sample_delta = 0;
				uint32_t	subsample_count = 0;

				std::vector<SubSample>	subsamples;
			};

			uint32_t			entry_count = 0;
			std::vector<Entry>	entries;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(entry_count))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				entry_count = bs.GetDWord();
				left_bytes -= sizeof(entry_count);
				for (uint32_t i = 0; i < entry_count; i++)
				{
					Entry entry;
					if (left_bytes < sizeof(entry.sample_delta) + sizeof(entry.subsample_count))
						break;

					entry.sample_delta = bs.GetDWord();
					entry.subsample_count = bs.GetWord();
					left_bytes -= sizeof(entry.sample_delta) + sizeof(entry.subsample_count);

					if (entry.subsample_count > 0)
					{
						uint16_t j = 0;
						for (; j < entry.subsample_count; j++)
						{
							if (left_bytes < (version == 1 ? sizeof(uint32_t) : (uint64_t)sizeof(uint16_t)) + 6)
								break;

							Entry::SubSample subsample;
							subsample.subsample_size = version==1?bs.GetDWord():bs.GetWord();
							subsample.subsample_priority = bs.GetByte();
							subsample.discardable = bs.GetByte();
							subsample.reserved = bs.GetDWord();

							entry.subsamples.push_back(subsample);

							left_bytes -= (version == 1 ? sizeof(uint32_t) : (uint64_t)sizeof(uint16_t)) + 6;
						}
						if (j < entry.subsample_count)
							break;
					}
				}

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Type: 'saiz'
		Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
		Mandatory: No
		Quantity: Zero or More
		*/
		struct SampleAuxiliaryInformationSizesBox : public FullBox
		{
			uint32_t				aux_info_type = 0;
			uint32_t				aux_info_type_parameter = 0;

			uint8_t					default_sample_info_size = 0;
			uint32_t				sample_count = 0;

			std::vector<uint8_t>	sample_info_sizes;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (flags & 1)
				{
					if (left_bytes < sizeof(aux_info_type) + sizeof(aux_info_type_parameter))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					aux_info_type = bs.GetDWord();
					aux_info_type_parameter = bs.GetDWord();

					left_bytes -= sizeof(aux_info_type) + sizeof(aux_info_type_parameter);
				}

				if (left_bytes < sizeof(default_sample_info_size) + sizeof(sample_count))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				default_sample_info_size = bs.GetByte();
				sample_count = bs.GetDWord();
				left_bytes -= sizeof(default_sample_info_size) + sizeof(sample_count);

				if (default_sample_info_size == 0)
				{
					for (uint32_t i = 0; i < (uint32_t)AMP_MIN(left_bytes, (uint64_t)sample_count); i++)
						sample_info_sizes.push_back(bs.GetByte());
				}

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Type: 'saio'
		Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
		Mandatory: No
		Quantity: Zero or More
		*/
		struct SampleAuxiliaryInformationOffsetsBox : public FullBox
		{
			uint32_t				aux_info_type = 0;
			uint32_t				aux_info_type_parameter = 0;

			uint32_t				entry_count = 0;
			std::vector<uint64_t>	offsets;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (flags & 1)
				{
					if (left_bytes < sizeof(aux_info_type) + sizeof(aux_info_type_parameter))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					aux_info_type = bs.GetDWord();
					aux_info_type_parameter = bs.GetDWord();

					left_bytes -= sizeof(aux_info_type) + sizeof(aux_info_type_parameter);
				}

				if (left_bytes < sizeof(entry_count))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				entry_count = bs.GetDWord();
				left_bytes -= sizeof(entry_count);

				for (uint32_t i = 0; i < entry_count; i++)
				{
					if (left_bytes < (version == 1 ? sizeof(uint64_t) : sizeof(uint32_t)))
						break;

					offsets.push_back(version==1?bs.GetQWord(): bs.GetDWord());
					left_bytes -= version == 1 ? sizeof(uint64_t) : sizeof(uint32_t);
				}

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Type: 'udta'
		Container: Movie Box ('moov') or Track Box ('trak')
		Mandatory: No
		Quantity: Zero or one
		*/
		struct UserDataBox : public ContainerBox
		{
			/*
			Box Type: 'cprt'
			Container: User data box ('udta')
			Mandatory: No
			Quantity: Zero or more
			*/
			struct CopyrightBox : public FullBox
			{
				uint16_t	pad : 1;
				uint16_t	language0 : 5;
				uint16_t	language1 : 5;
				uint16_t	language2 : 5;

				std::string	notice;

				CopyrightBox()
					: pad(0), language0(0), language1(0), language2(0) {
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(uint16_t))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					pad = (uint16_t)bs.GetBits(1);
					language0 = (uint16_t)bs.GetBits(5);
					language1 = (uint16_t)bs.GetBits(5);
					language2 = (uint16_t)bs.GetBits(5);

					ReadString(bs, notice);

					SkipLeftBits(bs);
					return 0;
				}
			};

			/*
			Box Type: 'tsel'
			Container: User Data Box ('udta')
			Mandatory: No
			Quantity: Zero or One
			*/
			struct TrackSelectionBox : public FullBox
			{
				uint32_t				switch_group = 0;
				std::vector<uint32_t>	attribute_list;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(uint32_t))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					switch_group = bs.GetDWord();
					left_bytes -= 4;

					while (left_bytes >= 4)
					{
						attribute_list.push_back(bs.GetDWord());
						left_bytes -= 4;
					}

					SkipLeftBits(bs);
					return 0;
				}
			};

			/*
			Box Type: 'strk'
			Container: User Data box ('udta') of the corresponding Track box ('trak')
			Mandatory: No
			Quantity: Zero or more
			*/
			struct SubTrack : public ContainerBox
			{
				/*
				Box Type: 'stri'
				Container: Sub Track box ('strk')
				Mandatory: Yes
				Quantity: One
				*/
				struct SubTrackInformation : public FullBox
				{
					int16_t					switch_group = 0;
					int16_t					alternate_group = 0;
					uint32_t				sub_track_ID = 0;
					std::vector<int32_t>	attribute_list;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < 8)
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						switch_group = bs.GetWord();
						alternate_group = bs.GetWord();
						sub_track_ID = bs.GetDWord();
						left_bytes -= 8;

						while (left_bytes >= 4)
						{
							attribute_list.push_back(bs.GetDWord());
							left_bytes -= 4;
						}

						SkipLeftBits(bs);
						return 0;
					}
				};

				/*
				Box Type: 'strd'
				Container: Sub Track box ('strk')
				Mandatory: Yes
				Quantity: One
				*/
				struct SubTrackDefinition : public Box
				{
					/*
					Box Type: 'stsg'
					Container: Sub Track Definition box ('strd')
					Mandatory: No
					Quantity: Zero or more
					*/
					struct SubTrackSampleGroupBox : public FullBox
					{
						uint32_t				grouping_type;
						uint16_t				item_count;
						std::vector<uint32_t>	group_description_index;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							if (left_bytes < 6)
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}
							grouping_type = bs.GetDWord();
							item_count = bs.GetWord();
							left_bytes -= 6;

							while (left_bytes >= 4)
							{
								group_description_index.push_back(bs.GetDWord());
								left_bytes -= 4;
							}

							SkipLeftBits(bs);
							return 0;
						}
					};

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						SkipLeftBits(bs);
						return 0;
					}

				}PACKED;

				SubTrackInformation*		subtrack_information = nullptr;
				SubTrackDefinition*			subtrack_definition = nullptr;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = ContainerBox::Unpack(bs)) < 0)
						return iRet;

					Box* ptr_child = first_child;
					while (ptr_child != nullptr)
					{
						switch (ptr_child->type)
						{
						case 'stri':
							subtrack_information = (SubTrackInformation*)ptr_child;
							break;
						case 'strd':
							subtrack_definition = (SubTrackDefinition*)ptr_child;
							break;
						}

						ptr_child = ptr_child->next_sibling;
					}

					return iRet;
				}
			}PACKED;

			std::vector<CopyrightBox*>		copyright_boxes;
			TrackSelectionBox*				track_selection_box = nullptr;
			std::vector<SubTrack*>			subtracks;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = ContainerBox::Unpack(bs)) < 0)
					return iRet;

				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					switch (ptr_child->type)
					{
					case 'cprt':
						copyright_boxes.push_back((CopyrightBox*)ptr_child);
						break;
					case 'tsel':
						track_selection_box = (TrackSelectionBox*)ptr_child;
						break;
					case 'strk':
						subtracks.push_back((SubTrack*)ptr_child);
						break;
					}

					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}
		};

		/*
		Box Types: 'frma'
		Container: Protection Scheme Information Box ('sinf') or Restricted Scheme Information Box ('rinf')
		Mandatory: Yes when used in a protected sample entry or in a restricted sample entry
		Quantity: Exactly one
		*/
		struct OriginalFormatBox : public Box
		{
			uint32_t	data_format = 0;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				if (LeftBytes(bs) < sizeof(data_format))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				data_format = bs.GetDWord();

				SkipLeftBits(bs);
				return 0;
			}
		}PACKED;

		/*
		Box Types: 'schm'
		Container: Protection Scheme Information Box ('sinf', Restricted Scheme Information Box ('rinf'),
		or SRTP Process box ('srpp')
		Mandatory: No
		Quantity: Zero or one in 'sinf', depending on the protection structure; Exactly one in 'rinf' and 'srpp'
		*/
		struct SchemeTypeBox : public FullBox
		{
			uint32_t		scheme_type = 0;	// 4CC identifying the scheme
			uint32_t		scheme_version = 0;	// scheme version
			std::string		scheme_uri;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				if (LeftBytes(bs) < sizeof(scheme_type) + sizeof(scheme_version))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				scheme_type = bs.GetDWord();
				scheme_version = bs.GetDWord();

				ReadString(bs, scheme_uri);

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Types: 'schi'
		Container: Protection Scheme Information Box ('sinf'), Restricted Scheme Information Box ('rinf'),
		or SRTP Process box ('srpp')
		Mandatory: No
		Quantity: Zero or one
		*/
		struct SchemeInformationBox : public Box
		{
			std::vector<Box*>	scheme_specific_data;

			virtual ~SchemeInformationBox()
			{
				for (auto v : scheme_specific_data)
					delete v;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				while (LeftBytes(bs) > 0)
				{
					Box* ptr_box = NULL;
					if (LoadOneBoxBranch(this, bs, &ptr_box) < 0)
						break;

					scheme_specific_data.push_back(ptr_box);
				}

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Types: 'rinf'
		Container: Restricted Sample Entry or Sample Entry
		Mandatory: Yes
		Quantity: Exactly one
		*/
		struct RestrictedSchemeInfoBox : public Box
		{
			OriginalFormatBox		original_format;
			SchemeTypeBox			scheme_type_box;
			SchemeInformationBox*	info = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				if ((bs.PeekBits(64)&UINT32_MAX) != 'frma')
					return RET_CODE_BOX_INCOMPATIBLE;

				if ((iRet = original_format.Unpack(bs)) < 0)
					return iRet;

				original_format.container = this;

				if ((bs.PeekBits(64)&UINT32_MAX) != 'schm')
					return RET_CODE_BOX_INCOMPATIBLE;

				if ((iRet = scheme_type_box.Unpack(bs)) < 0)
					return iRet;

				scheme_type_box.container = this;

				if (LeftBytes(bs) >= MIN_BOX_SIZE)
				{
					if ((bs.PeekBits(64)&UINT32_MAX) == 'schi')
						LoadOneBoxBranch(this, bs, (Box**)&info);
				}

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Types: 'sinf'
		Container: Protected Sample Entry, or Item Protection Box ('ipro')
		Mandatory: Yes
		Quantity: Exactly oneOne or More
		*/
		struct ProtectionSchemeInfoBox: public Box
		{
			OriginalFormatBox		original_format;
			SchemeTypeBox*			scheme_type_box = nullptr;
			SchemeInformationBox*	info = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				if ((bs.PeekBits(64)&UINT32_MAX) != 'frma')
					return RET_CODE_BOX_INCOMPATIBLE;

				if ((iRet = original_format.Unpack(bs)) < 0)
					return iRet;

				original_format.container = this;

				if (LeftBytes(bs) >= MIN_BOX_SIZE)
				{
					if ((bs.PeekBits(64)&UINT32_MAX) == 'schm')
						LoadOneBoxBranch(this, bs, (Box**)&scheme_type_box);
				}

				if (LeftBytes(bs) >= MIN_BOX_SIZE)
				{
					if ((bs.PeekBits(64)&UINT32_MAX) == 'schi')
						LoadOneBoxBranch(this, bs, (Box**)&info);
				}

				SkipLeftBits(bs);
				return 0;
			}
		};

		/*
		Box Type: 'meta'
		Container: File, Movie Box ('moov'), Track Box ('trak'), or Additional Metadata Container Box ('meco')
		Mandatory: No
		Quantity: Zero or one (in File, 'moov', and 'trak'), One or more (in 'meco')
		*/
		struct MetaBox : public ContainerFullBox
		{
			/*
			Box Type: 'xml ' or 'bxml'
			Container: Meta box ('meta')
			Mandatory: No
			Quantity: Zero or one	
			*/
			struct XMLBox: public FullBox
			{
				std::string xml;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					xml.reserve((size_t)LeftBytes(bs));

					ReadString(bs, xml);

					SkipLeftBits(bs);
					return 0;
				}
			};

			struct BinaryXMLBox : public FullBox
			{
				std::vector<uint8_t>	data;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					data.reserve((size_t)left_bytes);

					while (left_bytes > 0)
					{
						data.push_back(bs.GetByte());
						left_bytes--;
					}

					return 0;
				}
			};

			/*
			Box Type: 'iloc'
			Container: Meta box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct ItemLocationBox : public FullBox
			{
				struct Item
				{
					uint32_t				item_ID;
					uint16_t				reserved : 12;
					uint16_t				construction_method : 4;
					uint16_t				data_reference_index;
					std::vector<uint8_t>	base_offset;
					uint16_t				extent_count;
					std::vector<uint8_t>	extent_index;
					std::vector<uint8_t>	extent_offset;
					std::vector<uint8_t>	extent_length;
				};

				uint8_t		offset_size : 4;
				uint8_t		length_size : 4;
				uint8_t		base_offset_size : 4;
				uint8_t		index_size : 4;

				uint32_t	item_count;
				std::vector<Item>	items;

				uint64_t GetMaxItemSize()
				{
					uint64_t max_item_size = 0;
					for (size_t i = 0; i < items.size(); i++)
					{
						uint64_t total_item_extent_size = 0;
						for (size_t j = 0; j < items[i].extent_count; j++)
						{
							uint64_t extent_size = 0;
							for (uint8_t k = 0; k < length_size; k++)
								extent_size = (extent_size << 8) | items[i].extent_length[j*length_size + k];

							total_item_extent_size += extent_size;
						}

						if (max_item_size < total_item_extent_size)
							max_item_size = total_item_extent_size;
					}

					return max_item_size;
				}

				uint64_t GetItemSize(uint32_t item_ID)
				{
					for (size_t i = 0; i < items.size(); i++)
					{
						if (items[i].item_ID == item_ID)
						{
							uint64_t total_item_extent_size = 0;
							for (size_t j = 0; j < items[i].extent_count; j++)
							{
								uint64_t extent_size = 0;
								for (uint8_t k = 0; k < length_size; k++)
									extent_size = (extent_size << 8) | items[i].extent_length[j*length_size + k];

								total_item_extent_size += extent_size;
							}

							return total_item_extent_size;
						}
					}

					return 0;
				}

				/* return (construction_method, data_reference_index, [(extent_index, extent_offset, extent_length), (extent_index, extent_offset, extent_length), ...]*/
				using ExtentResult = std::tuple<int32_t, uint16_t, std::vector<std::tuple<uint64_t, uint64_t, uint64_t>>>;
				ExtentResult GetExtents(uint32_t item_ID)
				{
					int32_t ret_construction_method = -1;
					uint16_t ret_data_reference_index = 0;
					std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> result;
					for (size_t i = 0; i < items.size(); i++)
					{
						if (items[i].item_ID == item_ID)
						{
							uint64_t base_offset = 0;
							for (uint8_t k = 0; k < base_offset_size; k++)
								base_offset = (base_offset << 8) | items[i].base_offset[k];

							for (size_t j = 0; j < items[i].extent_count; j++)
							{
								uint64_t extent_index = 0;
								if (((version == 1) || (version == 2)) && (index_size > 0))
								{
									for (uint8_t k = 0; k < index_size; k++)
										extent_index = (extent_index << 8) | items[i].extent_index[j*index_size + k];
								}

								uint64_t extent_offset = 0;
								for (uint8_t k = 0; k < offset_size; k++)
									extent_offset = (extent_offset << 8) | items[i].extent_offset[j*offset_size + k];

								extent_offset += base_offset;

								uint64_t extent_size = 0;
								for (uint8_t k = 0; k < length_size; k++)
									extent_size = (extent_size << 8) | items[i].extent_length[j*length_size + k];

								result.push_back(std::make_tuple(extent_index, extent_offset, extent_size));
							}

							ret_construction_method = version == 0 ? 0 : items[i].construction_method;
							ret_data_reference_index = items[i].data_reference_index;
							break;
						}
					}

					return std::make_tuple(ret_construction_method, ret_data_reference_index, result);
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < (version>=2?6:4))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					offset_size = (uint8_t)bs.GetBits(4);
					length_size = (uint8_t)bs.GetBits(4);
					base_offset_size = (uint8_t)bs.GetBits(4);
					index_size = (uint8_t)bs.GetBits(4);
					if (version < 2)
						item_count = bs.GetWord();
					else if(version == 2)
						item_count = bs.GetDWord();
					
					left_bytes -= (version == 2 ? 6 : 4);

					items.reserve(item_count);

					size_t item_fix_size = (version < 2 ? 2 : 4) + sizeof(Item::data_reference_index) + base_offset_size + sizeof(Item::extent_count);
					if (version == 1 || version == 2)
						item_fix_size += sizeof(uint16_t);	// for reserved + construction_method

					for (uint32_t i = 0; i < item_count; i++)
					{
						if (left_bytes < item_fix_size)
							break;

						items.emplace_back();
						auto& curitem = items.back();
						if (version < 2)
							curitem.item_ID = bs.GetWord();
						else if(version == 2)
							curitem.item_ID = bs.GetDWord();

						if (version == 1 || version == 2)
						{
							curitem.reserved = (uint16_t)bs.GetBits(12);
							curitem.construction_method = (uint16_t)bs.GetBits(4);
						}
						else
							curitem.construction_method = 0;

						curitem.data_reference_index = bs.GetWord();
						for (uint16_t j = 0; j < base_offset_size; j++)
							curitem.base_offset.push_back(bs.GetByte());

						curitem.extent_count = bs.GetWord();
						left_bytes -= item_fix_size;

						uint16_t j = 0;
						for (; j < curitem.extent_count; j++)
						{
							uint16_t extent_size = ((version == 1 || version == 2) && index_size > 0) ? (index_size + offset_size + length_size) : (offset_size + length_size);
							if (left_bytes < extent_size)
								break;

							if (version == 1 || version == 2)
							{
								for (uint8_t k = 0; k < index_size; k++)
									curitem.extent_index.push_back(bs.GetByte());
							}

							for (uint8_t k = 0; k < offset_size; k++)
								curitem.extent_offset.push_back(bs.GetByte());

							for (uint8_t k = 0; k < length_size; k++)
								curitem.extent_length.push_back(bs.GetByte());

							left_bytes -= extent_size;
						}

						if (j < curitem.extent_count)
							break;
					}

					SkipLeftBits(bs);
					return 0;
				}
			};

			/*
			Box Type: 'pitm'
			Container: Meta box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct PrimaryItemBox : public FullBox
			{
				uint16_t	item_ID = 0;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					if (LeftBytes(bs) < sizeof(item_ID))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					item_ID = bs.GetWord();

					SkipLeftBits(bs);
					return 0;
				}
			}PACKED;

			/*
			Box Type: 'ipro'
			Container: Meta box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct ItemProtectionBox : public FullBox
			{
				uint16_t		protection_count = 0; 
				std::vector<ProtectionSchemeInfoBox*>
								protection_informations;

				~ItemProtectionBox()
				{
					for (auto v : protection_informations)
						delete v;
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(protection_count))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					protection_count = bs.GetWord();
					left_bytes -= 2;

					while (LeftBytes(bs) >= MIN_BOX_SIZE)
					{
						if ((bs.PeekBits(64)&UINT32_MAX) != 'sinf')
							break;

						ProtectionSchemeInfoBox* ptr_box;
						if (LoadOneBoxBranch(this, bs, (Box**)&ptr_box) < 0)
							break;

						protection_informations.push_back(ptr_box);
					}

					SkipLeftBits(bs);
					return 0;
				}

			};

			/*
			Box Type: 'iinf'
			Container: Meta Box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct ItemInfoBox : public FullBox
			{
				struct ItemInfoExtension
				{
					virtual ~ItemInfoExtension(){}
				}PACKED;

				struct FDItemInfoExtension : public ItemInfoExtension
				{
					std::string				content_location;
					std::string				content_MD5;
					uint64_t				content_length = 0;
					uint64_t				transfer_length = 0;
					uint8_t					entry_count = 0;
					std::vector<uint32_t>	group_ids;
				};

				struct ItemInfoEntryv0
				{
					uint16_t				item_ID = 0;
					uint16_t				item_protection_index = 0;
					std::string				item_name;
					std::string				content_type;
					std::string				content_encoding;	// optional
				};

				struct ItemInfoEntryv1 : public ItemInfoEntryv0
				{
					uint32_t				extension_type;
					ItemInfoExtension*		item_info_extension;

					ItemInfoEntryv1(): extension_type(0), item_info_extension(nullptr){}
					virtual ~ItemInfoEntryv1() {
						AMP_SAFEDEL(item_info_extension);
					}
				}PACKED;

				struct ItemInfoEntryv2
				{
					uint32_t				item_ID = 0;
					uint16_t				item_protection_index = 0;
					uint32_t				item_type = 0;

					std::string				item_name;

					// item_type == 'mime'
					std::string				content_type;
					std::string				content_encoding;	// optional

					// item_type == 'uri '
					std::string				item_uri_type;
				};

				struct ItemInfoEntry : public FullBox
				{
					union
					{
						void*				item_info_data = nullptr;
						ItemInfoEntryv0*	item_info_entry_v0;
						ItemInfoEntryv1*	item_info_entry_v1;
						ItemInfoEntryv2*	item_info_entry_v2;
					}PACKED;

					ItemInfoEntry(): item_info_data(0){
					}

					virtual ~ItemInfoEntry()
					{
						if (version == 0) {
							AMP_SAFEDEL2(item_info_entry_v0);
						}
						else if (version == 1) {
							AMP_SAFEDEL2(item_info_entry_v1);
						}
						else if (version == 2) {
							AMP_SAFEDEL2(item_info_entry_v2);
						}
					}

					INLINE uint32_t GetItemID()
					{
						if (version == 0)
							return item_info_entry_v0->item_ID;
						else if (version == 1)
							return item_info_entry_v1->item_ID;
					
						return item_info_entry_v2->item_ID;
					}

					INLINE uint32_t GetItemType()
					{
						if (version >= 2)
							return item_info_entry_v2->item_type;

						return 0;
					}

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);

						if (version == 0)
							item_info_entry_v0 = new ItemInfoEntryv0;
						else if (version == 1)
							item_info_entry_v1 = new ItemInfoEntryv1;
						else if (version >= 2)
							item_info_entry_v2 = new ItemInfoEntryv2;

						if (version == 0 || version == 1)
						{
							if (left_bytes < sizeof(uint16_t) + sizeof(uint16_t) + 3/* 3 null terminator */)
							{
								printf("Not enough to fill one ItemInfoEntry v0/v1, left_bytes: %" PRIu64 ".\n", left_bytes);
								goto done;
							}

							ItemInfoEntryv0* entry = version == 0
								? item_info_entry_v0	: (ItemInfoEntryv0*)(item_info_entry_v1);

							entry->item_ID = bs.GetWord();
							entry->item_protection_index = bs.GetWord();
							ReadString(bs, entry->item_name);
							ReadString(bs, entry->content_type);
							ReadString(bs, entry->content_encoding);

							left_bytes -= (uint64_t)sizeof(entry->item_ID) + sizeof(entry->item_protection_index) +
								entry->item_name.length() + entry->content_type.length() + entry->content_encoding.length();
						}

						if (version == 1)
						{
							if (left_bytes < sizeof(uint32_t))
							{
								_tprintf(_T("[ISOBMFF] Not enough to fill one ItemInfoEntry v1, left_bytes: %" PRIu64 ".\n"), left_bytes);
								goto done;
							}

							item_info_entry_v1->extension_type = bs.GetDWord();
							left_bytes -= sizeof(uint32_t);

							if (item_info_entry_v1->extension_type == 'fdel')
							{
								if (left_bytes < 2/*2 null terminator*/)
								{
									_tprintf(_T("[ISOBMFF] Not enough to fill FDItemInfoExtension of one ItemInfoEntry v1, left_bytes: %" PRIu64 ".\n"), left_bytes);
									goto done;
								}

								FDItemInfoExtension* fd_item_info_extension = new FDItemInfoExtension();
								item_info_entry_v1->item_info_extension = fd_item_info_extension;

								ReadString(bs, fd_item_info_extension->content_location);
								ReadString(bs, fd_item_info_extension->content_MD5);

								left_bytes -= fd_item_info_extension->content_location.length();
								left_bytes -= fd_item_info_extension->content_MD5.length();

								if (left_bytes < sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint8_t))
								{
									_tprintf(_T("[ISOBMFF] Not enough to fill FDItemInfoExtension of one ItemInfoEntry v1, left_bytes: %" PRIu64 ".\n"), left_bytes);
									goto done;
								}

								fd_item_info_extension->content_length = bs.GetQWord();
								fd_item_info_extension->transfer_length = bs.GetQWord();
								fd_item_info_extension->entry_count = bs.GetByte();

								left_bytes -= sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint8_t);

								while (left_bytes >= sizeof(uint32_t) && fd_item_info_extension->group_ids.size() < fd_item_info_extension->entry_count)
								{
									fd_item_info_extension->group_ids.push_back(bs.GetDWord());
									left_bytes -= sizeof(uint32_t);
								}

								if (fd_item_info_extension->group_ids.size() < fd_item_info_extension->entry_count)
									goto done;
							}
							else
							{
								_tprintf(_T("[ISOBMFF] Unsupported extension_type: 0X%08X.\n"), item_info_entry_v1->extension_type);
								goto done;
							}
						}

						if (version == 2)
						{
							if (left_bytes < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + 1/*item_name null terminator*/)
							{
								_tprintf(_T("[ISOBMFF] Not enough to fill one ItemInfoEntry v2, left_bytes: %" PRIu64 ".\n"), left_bytes);
								goto done;
							}

							if (version >= 3)
								item_info_entry_v2->item_ID = bs.GetDWord();
							else
								item_info_entry_v2->item_ID = bs.GetWord();
							item_info_entry_v2->item_protection_index = bs.GetWord();
							item_info_entry_v2->item_type = bs.GetDWord();

							ReadString(bs, item_info_entry_v2->item_name);

							left_bytes -= (version >= 3?sizeof(uint32_t):(uint64_t)sizeof(uint16_t)) + sizeof(uint16_t) + sizeof(uint32_t) + item_info_entry_v2->item_name.length();

							if (item_info_entry_v2->item_type == 'mime')
							{
								ReadString(bs, item_info_entry_v2->content_type);
								ReadString(bs, item_info_entry_v2->content_encoding);

								left_bytes -= (uint64_t)item_info_entry_v2->content_type.length() +
									item_info_entry_v2->content_encoding.length();
							}
							else if (item_info_entry_v2->item_type == 'uri ')
							{
								ReadString(bs, item_info_entry_v2->item_uri_type);

								left_bytes -= item_info_entry_v2->item_uri_type.length();
							}
						}

					done:
						SkipLeftBits(bs);
						return 0;
					}
				}PACKED;

				uint16_t		entry_count = 0;
				uint32_t		last_entry_idx = 0;
				std::vector<ItemInfoEntry>
								item_info_entries;

				uint32_t GetItemType(uint32_t item_ID)
				{
					for (size_t i = 0; i < item_info_entries.size(); i++)
					{
						if (item_info_entries[i].GetItemID() == item_ID)
							return item_info_entries[i].GetItemType();
					}

					return 0;
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(entry_count))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					if (version == 0)
						entry_count = bs.GetWord();
					else
						entry_count = bs.GetDWord();

					left_bytes -= version == 0 ? 2 : 4;

					if (entry_count > 0)
					{
						item_info_entries.resize(entry_count);

						last_entry_idx = 0;
						while (left_bytes >= MIN_FULLBOX_SIZE)
						{
							if ((iRet = item_info_entries[last_entry_idx].Unpack(bs)) < 0)
								goto done;

							left_bytes -= item_info_entries[last_entry_idx].size;
							last_entry_idx++;
						}
					}

				done:
					SkipLeftBits(bs);
					return iRet;
				}

			};

			/*
			Box Type: 'fiin'
			Container: Meta Box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct FDItemInformationBox : public FullBox
			{
				struct PartitionEntry : public Box
				{
					/*
					Box Type: 'fpar'
					Container: Partition Entry ('paen')
					Mandatory: Yes
					Quantity: Exactly one
					*/
					struct FilePartitionBox : public FullBox
					{
						uint16_t		item_ID = 0;
						uint16_t		packet_payload_size = 0;
						uint8_t			reserved = 0;
						uint8_t			FEC_encoding_ID = 0;
						uint16_t		FEC_instance_ID = 0;
						uint16_t		max_source_block_length = 0;
						uint16_t		encoding_symbol_length = 0;
						uint16_t		max_number_of_encoding_symbols = 0;
						std::string		scheme_specific_info;
						uint16_t		entry_count = 0;
						std::vector<std::tuple<uint16_t/*block_count*/, uint32_t/*block_size*/>>
										entries;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							auto unpack_size = sizeof(item_ID) + sizeof(packet_payload_size) + sizeof(reserved) + sizeof(FEC_encoding_ID) +
								sizeof(FEC_instance_ID) + sizeof(max_source_block_length) + sizeof(encoding_symbol_length) + sizeof(max_number_of_encoding_symbols);
							if (left_bytes < unpack_size)
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}

							item_ID = bs.GetWord();
							packet_payload_size = bs.GetWord();
							reserved = bs.GetByte();
							FEC_encoding_ID = bs.GetByte();
							FEC_instance_ID = bs.GetWord();;
							max_source_block_length = bs.GetWord();
							encoding_symbol_length = bs.GetWord();
							max_number_of_encoding_symbols = bs.GetWord();

							left_bytes -= unpack_size;

							ReadString(bs, scheme_specific_info);

							left_bytes -= scheme_specific_info.length();

							if (left_bytes < sizeof(entry_count))
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}

							unpack_size = sizeof(uint16_t) + sizeof(uint32_t);
							while (left_bytes >= unpack_size && entries.size() < entry_count)
							{
								entries.push_back({ bs.GetWord(), bs.GetDWord() });
								left_bytes -= unpack_size;
							}

							SkipLeftBits(bs);
							return 0;
						}

					};

					/*
					Box Type: 'fecr'
					Container: Partition Entry ('paen')
					Mandatory: No
					Quantity: Zero or One
					*/
					struct FECReservoirBox : public FullBox
					{
						uint16_t		entry_count = 0;
						std::vector<std::tuple<uint16_t/*item_ID*/, uint32_t/*symbol_count*/>>
										entries;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							if (left_bytes < sizeof(entry_count))
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}

							entry_count = bs.GetWord();
							left_bytes -= sizeof(entry_count);

							auto unpack_size = sizeof(uint16_t) + sizeof(uint32_t);
							while (left_bytes >= unpack_size && entries.size() < entry_count)
							{
								entries.push_back({ bs.GetWord(), bs.GetDWord() });
								left_bytes -= unpack_size;
							}

							SkipLeftBits(bs);
							return 0;
						}
					};

					/*
					Box Type: 'fire'
					Container: Partition Entry ('paen')
					Mandatory: No
					Quantity: Zero or One
					*/
					struct FileReservoirBox : public FullBox
					{
						uint32_t		entry_count = 0;
						std::unordered_map<uint16_t, uint32_t>
										entries;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							if (left_bytes < sizeof(entry_count))
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}

							entry_count = bs.GetWord();
							left_bytes -= sizeof(entry_count);

							auto unpack_size = sizeof(uint16_t) + sizeof(uint32_t);
							while (left_bytes >= unpack_size && entries.size() < entry_count)
							{
								entries[bs.GetWord()] = bs.GetDWord();
								left_bytes -= unpack_size;
							}

							SkipLeftBits(bs);
							return 0;
						}

					};

					FilePartitionBox	blocks_and_symbols;
					FECReservoirBox*	FEC_symbol_locations; //optional
					FileReservoirBox*	File_symbol_locations; //optional

					PartitionEntry()
						: FEC_symbol_locations(nullptr)
						, File_symbol_locations(nullptr){
					}

					virtual ~PartitionEntry()
					{
						AMP_SAFEDEL(FEC_symbol_locations);
						AMP_SAFEDEL(File_symbol_locations);
					}

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						if ((iRet = blocks_and_symbols.Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < MIN_FULLBOX_SIZE)
							goto done;

						FEC_symbol_locations = new FECReservoirBox();
						FEC_symbol_locations->container = this;
						if (FEC_symbol_locations->Unpack(bs) < 0)
						{
							delete FEC_symbol_locations;
							FEC_symbol_locations = NULL;
							goto done;
						}

						if ((left_bytes-=FEC_symbol_locations->size) < MIN_FULLBOX_SIZE)
							goto done;

						File_symbol_locations = new FileReservoirBox();
						File_symbol_locations->container = this;
						if (File_symbol_locations->Unpack(bs) < 0)
						{
							delete File_symbol_locations;
							File_symbol_locations = NULL;
							goto done;
						}

					done:
						SkipLeftBits(bs);
						return iRet;
					}

				}; // PartionEntry

				/*
				Box Type: 'segr'
				Container: FD Information Box ('fiin')
				Mandatory: No
				Quantity: Zero or One
				*/
				struct FDSessionGroupBox : public Box
				{
					struct SessionGroup
					{
						uint8_t					entry_count = 0;
						std::vector<uint32_t>	group_IDs;
						uint16_t				num_channels_in_session_group = 0;
						std::vector<uint32_t>	hint_track_ids;
					};

					uint16_t					num_session_groups = 0;
					std::vector<SessionGroup>	session_groups;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < sizeof(num_session_groups))
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						num_session_groups = bs.GetWord();
						left_bytes -= sizeof(num_session_groups);

						for (uint16_t session_group_id = 0; session_group_id < num_session_groups; session_group_id++)
						{
							uint8_t entry_count = 0;
							if (left_bytes < sizeof(uint8_t))
								break;

							entry_count = bs.GetByte();
							left_bytes -= sizeof(entry_count);

							if (left_bytes < (uint64_t)entry_count * sizeof(uint32_t))
								break;

							session_groups.emplace_back();
							auto& back = session_groups.back();
							for (uint8_t i = 0; i < entry_count; i++)
								back.group_IDs.push_back(bs.GetDWord());

							left_bytes -= (uint64_t)entry_count * sizeof(uint32_t);

							if (left_bytes < sizeof(uint16_t))
							{
								session_groups.pop_back();
								break;
							}

							back.num_channels_in_session_group = bs.GetWord();
							left_bytes -= sizeof(uint16_t);

							while (left_bytes >= sizeof(uint32_t) && back.hint_track_ids.size() < back.num_channels_in_session_group)
							{
								back.hint_track_ids.push_back(bs.GetDWord());
								left_bytes -= sizeof(uint32_t);
							}
						}

						SkipLeftBits(bs);
						return 0;
					}

				}; // FDSessionGroupBox

				/*
				Box Type: 'gitn'
				Container: FD Information Box ('fiin')
				Mandatory: No
				Quantity: Zero or One
				*/
				struct GroupIdToNameBox : public FullBox
				{
					uint16_t		entry_count = 0;
					std::unordered_map<uint32_t, std::string>	
									entries;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < sizeof(entry_count))
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						entry_count = bs.GetWord();
						left_bytes -= sizeof(entry_count);

						while (LeftBytes(bs) >= sizeof(uint32_t) && entries.size() < entry_count)
							ReadString(bs, entries[bs.GetDWord()]);

						SkipLeftBits(bs);
						return 0;
					}

				}; // GroupIdToNameBox

				uint16_t						entry_count = 0;
				std::vector<PartitionEntry*>	partition_entries;
				FDSessionGroupBox*				session_info; //optional
				GroupIdToNameBox*				group_id_to_name; //optional

				FDItemInformationBox()
					: session_info(nullptr)
					, group_id_to_name(nullptr){
				}

				virtual ~FDItemInformationBox()
				{
					for (auto v : partition_entries)
						delete v;
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(entry_count))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					entry_count = bs.GetWord();
					left_bytes -= sizeof(uint16_t);

					while (left_bytes >= MIN_BOX_SIZE && partition_entries.size() < entry_count)
					{
						partition_entries.emplace_back();
						auto& pPE = partition_entries.back();
						pPE = new PartitionEntry();

						if (pPE->Unpack(bs) < 0)
						{
							delete pPE;
							partition_entries.pop_back();
							break;
						}

						left_bytes -= pPE->size;
					}

					if (partition_entries.size() < entry_count)
						goto done;

					if (left_bytes < MIN_BOX_SIZE)
						goto done;

					session_info = new FDSessionGroupBox();
					session_info->container = this;
					if (session_info->Unpack(bs) < 0)
					{
						delete session_info;
						session_info = NULL;
						goto done;
					}

					if ((left_bytes -= session_info->size) < MIN_FULLBOX_SIZE)
						goto done;

					group_id_to_name = new GroupIdToNameBox();
					group_id_to_name->container = this;
					if (group_id_to_name->Unpack(bs) < 0)
					{
						delete group_id_to_name;
						group_id_to_name = NULL;
						goto done;
					}

				done:
					SkipLeftBits(bs);
					return iRet;
				}

			}; // FDItemInformationBox

			/*
			Box Type: 'idat'
			Container: Metadata box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct ItemDataBox : public Box
			{
				std::vector<uint8_t>	data;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = Box::Unpack(bs)) < 0)
						return iRet;

					// Don't read the bitstream to data to save memory

					SkipLeftBits(bs);
					return iRet;
				}
			};

			/*
			Box Type: 'iref'
			Container: Metadata box ('meta')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct ItemReferenceBox : public FullBox
			{
				struct SingleItemTypeReferenceBox : public Box
				{
					uint16_t				from_item_ID = 0;
					uint16_t				reference_count = 0;
					std::vector<uint16_t>	to_item_IDs;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;
						uint16_t last_idx = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < sizeof(uint16_t) + sizeof(uint16_t))
						{
							iRet = RET_CODE_BOX_TOO_SMALL;
							goto done;
						}

						from_item_ID = bs.GetWord();
						reference_count = bs.GetWord();
						left_bytes -= sizeof(uint16_t) + sizeof(uint16_t);

						while (left_bytes >= sizeof(uint16_t) && last_idx < reference_count)
						{
							to_item_IDs.push_back(bs.GetWord());
							
							if (left_bytes < sizeof(uint16_t))
								break;

							left_bytes -= sizeof(uint16_t);
							last_idx++;
						}

					done:
						SkipLeftBits(bs);
						return iRet;
					}
				};

				std::vector<SingleItemTypeReferenceBox*>	references;

				virtual ~ItemReferenceBox()
				{
					for (auto v : references)
						delete v;
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					while (left_bytes >= MIN_BOX_SIZE)
					{
						auto ref = new SingleItemTypeReferenceBox();
						ref->container = this;
						if (ref->Unpack(bs) < 0)
						{
							delete ref;
							break;
						}

						references.push_back(ref);
							left_bytes -= ref->size;
					}

					SkipLeftBits(bs);
					return iRet;
				}
			}; // ItemReferenceBox

			HandlerBox*			handler_box = nullptr;
			DataInformationBox*	data_inforamtion_box = nullptr;
			XMLBox*				xml_box = nullptr;
			BinaryXMLBox*		binary_xml_box = nullptr;
			ItemLocationBox*	item_location_box = nullptr;
			PrimaryItemBox*		primary_item_box = nullptr;
			ItemProtectionBox*	item_protection_box = nullptr;
			ItemInfoBox*		item_info_box = nullptr;
			FDItemInformationBox*
								FD_item_information_box = nullptr;
			ItemDataBox*		item_data_box = nullptr;
			ItemReferenceBox*	item_reference_box = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ContainerFullBox::Unpack(bs);
				if (iRet < 0)
					return iRet;

				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					switch (ptr_child->type)
					{
					case 'hdlr':
						handler_box = (HandlerBox*)ptr_child;
						break;
					case 'dinf':
						data_inforamtion_box = (DataInformationBox*)ptr_child;
						break;
					case 'iloc':
						item_location_box = (ItemLocationBox*)ptr_child;
						break;
					case 'ipro':
						item_protection_box = (ItemProtectionBox*)ptr_child;
						break;
					case 'iinf':
						item_info_box = (ItemInfoBox*)ptr_child;
						break;
					case 'xml ':
						xml_box = (XMLBox*)ptr_child;
						break;
					case 'bxml':
						binary_xml_box = (BinaryXMLBox*)ptr_child;
						break;
					case 'pitm':
						primary_item_box = (PrimaryItemBox*)ptr_child;
						break;
					case 'fiin':
						FD_item_information_box = (FDItemInformationBox*)ptr_child;
						break;
					case 'idat':
						item_data_box = (ItemDataBox*)ptr_child;
						break;
					case 'iref':
						item_reference_box = (ItemReferenceBox*)ptr_child;
						break;
					}
					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}

		}PACKED;

		/*
		Box Type: 'meco'
		Container: File, Movie Box ('moov'), or Track Box ('trak')
		Mandatory: No
		Quantity: Zero or one
		*/
		struct AdditionalMetadataContainerBox : public ContainerBox
		{
			/*
			Box Type: 'mere'
			Container: Additional Metadata Container Box ('meco')
			Mandatory: No
			Quantity: Zero or more
			*/
			struct MetaboxRelationBox : public FullBox
			{
				uint32_t	first_metabox_handler_type = 0;
				uint32_t	second_metabox_handler_type = 0;
				uint8_t		metabox_relation = 0;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < sizeof(first_metabox_handler_type) + sizeof(second_metabox_handler_type) + sizeof(metabox_relation))
					{
						iRet = RET_CODE_BOX_TOO_SMALL;
						goto done;
					}

					first_metabox_handler_type = bs.GetDWord();
					second_metabox_handler_type = bs.GetDWord();
					metabox_relation = bs.GetDWord();

				done:
					SkipLeftBits(bs);
					return iRet;
				}

			}PACKED;

			std::vector<MetaboxRelationBox*>	metabox_relation_boxes;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ContainerBox::Unpack(bs);

				if (iRet < 0)
					return iRet;

				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					switch (ptr_child->type)
					{
					case 'mere':
						metabox_relation_boxes.push_back((MetaboxRelationBox*)ptr_child);
						break;
					}

					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}

		};

		/*
		Box Type: 'sidx'
		Container: File
		Mandatory: No
		Quantity: Zero or more
		*/
		struct SegmentIndexBox : public FullBox
		{
			struct Reference
			{
				uint32_t	reference_type : 1;
				uint32_t	referenced_size : 31;
				uint32_t	subsegment_duration;
				uint32_t	starts_with_SAP : 1;
				uint32_t	SAP_type : 3;
				uint32_t	SAP_delta_time : 28;

				std::string Meaning()
				{
					if (starts_with_SAP == 0 && SAP_type == 0)
						return "No information of SAPs is provided";
					else if (starts_with_SAP == 0 && SAP_type >= 1 && SAP_type <= 6 && reference_type == 0)
						return "The subsegment contains (but may not start with) a SAP of the given SAP_type and the first SAP of the given SAP_type corresponds to SAP_delta_time";
					else if (starts_with_SAP == 0 && SAP_type >= 1 && SAP_type <= 6 && reference_type == 1)
						return "All the referenced subsegments contain a SAP of at most the given SAP_type and none of these SAPs is of an unknown type";
					else if (starts_with_SAP == 1 && SAP_type == 0 && reference_type == 0)
						return "The subsegment starts with a SAP of an unknown type";
					else if (starts_with_SAP == 1 && SAP_type == 0 && reference_type == 1)
						return "All the referenced subsegments start with a SAP which may be of an unknown type";
					else if (starts_with_SAP == 1 && SAP_type >= 1 && SAP_type <= 6 && reference_type == 0)
						return "The referenced subsegment starts with a SAP of the given SAP_type";
					else if (starts_with_SAP == 1 && SAP_type >= 1 && SAP_type <= 6 && reference_type == 1)
						return "All the referenced subsegments start with a SAP of at most the given SAP_type and none of these SAPs is of an unknown type";

					return "";
				}
			}PACKED;

			uint32_t				reference_ID = 0;
			uint32_t				timescale = 0;

			uint64_t				earliest_presentation_time = 0;
			uint64_t				first_offset = 0;

			uint16_t				reserved = 0;
			uint16_t				reference_count = 0;
			std::vector<Reference>	references;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				auto unpack_size = sizeof(reference_ID) + sizeof(timescale) + (version == 1 ? (sizeof(uint64_t) * 2) : (sizeof(uint32_t) * 2)) + sizeof(reserved) + sizeof(reference_count);
				if (left_bytes < unpack_size)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				reference_ID = bs.GetDWord();
				timescale = bs.GetDWord();

				earliest_presentation_time = version == 1 ? bs.GetQWord() : bs.GetDWord();
				first_offset = version == 1 ? bs.GetQWord() : bs.GetDWord();

				reserved = bs.GetWord();
				reference_count = bs.GetWord();

				left_bytes -= unpack_size;

				while (left_bytes >= sizeof(Reference) && references.size() < reference_count)
				{
					references.emplace_back();
					auto& back = references.back();
					back.reference_type = (uint32_t)bs.GetBits(1);
					back.referenced_size = (uint32_t)bs.GetBits(31);
					back.subsegment_duration = bs.GetDWord();
					back.starts_with_SAP = (uint32_t)bs.GetBits(1);
					back.SAP_type = (uint32_t)bs.GetBits(3);
					back.SAP_delta_time = (uint32_t)bs.GetBits(28);
					left_bytes -= sizeof(Reference);
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}
		};

		/*
		Box Type: 'ssix'
		Container: File
		Mandatory: No
		Quantity: Zero or more
		*/
		struct SubsegmentIndexBox : public FullBox
		{
			struct Segment
			{
				struct Range
				{
					uint32_t		level : 8;
					uint32_t		range_size : 24;
				}PACKED;

				uint32_t			ranges_count = 0;
				std::vector<Range>	ranges;
			};

			uint32_t				segment_count = 0;
			std::vector<Segment>	segments;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(segment_count))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				segment_count = bs.GetDWord();
				left_bytes -= sizeof(segment_count);

				while (left_bytes >= sizeof(uint32_t) && segments.size() < segment_count)
				{
					segments.emplace_back();
					auto& back = segments.back();

					back.ranges_count = bs.GetDWord();
					left_bytes -= sizeof(uint32_t);

					while (left_bytes >= sizeof(Segment::Range) && back.ranges.size() < back.ranges_count)
					{
						back.ranges.emplace_back();
						back.ranges.back().level = bs.GetByte();
						back.ranges.back().range_size = (uint32_t)bs.GetBits(24);
						left_bytes -= sizeof(Segment::Range);
					}

					if (back.ranges.size() < back.ranges_count)
						break;
				}

			done:
				SkipLeftBits(bs);
				return iRet;
			}
		};

		/*
		Box Type: 'prft'
		Container: File
		Mandatory: No
		Quantity: Zero or more
		*/
		struct ProducerReferenceTimeBox: public FullBox
		{
			uint32_t		reference_track_ID = 0;
			uint64_t		ntp_timestamp = 0;
			uint64_t		media_time = 0;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				auto unpack_size = sizeof(reference_track_ID) + sizeof(ntp_timestamp) + version == 1 ? sizeof(uint64_t) : sizeof(uint32_t);
				if (left_bytes < unpack_size)
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				reference_track_ID = bs.GetDWord();
				ntp_timestamp = bs.GetQWord();
				media_time = (version == 1 ? bs.GetQWord() : bs.GetDWord());

			done:
				SkipLeftBits(bs);
				return iRet;
			}

		}PACKED;

		/*
		Box Type: 'moov'
		Container: File
		Mandatory: Yes
		Quantity: Exactly one
		*/
		struct MovieBox : public ContainerBox
		{
			/*
			Box Type: 'mvhd'
			Container: Movie Box ('moov')
			Mandatory: Yes
			Quantity: Exactly one
			*/
			struct MovieHeaderBox : public FullBox
			{
				union {
					struct {
						uint64_t	creation_time;
						uint64_t	modification_time;
						uint32_t	timescale;
						uint64_t	duration;
					}PACKED v1;
					struct {
						uint32_t	creation_time;
						uint32_t	modification_time;
						uint32_t	timescale;
						uint32_t	duration;
					}PACKED v0;
				}PACKED;

				int32_t		rate = 0x00010000;
				int16_t		volume = 0x0100;
				uint16_t	reserved_0 = 0;
				uint32_t	reserved_1[2] = { 0 };
				int32_t		matrix[9] = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
				uint32_t	pre_defined[6] = { 0 };
				uint32_t	next_track_ID = 0;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					if (version == 1)
					{
						v1.creation_time = bs.GetQWord();
						v1.modification_time = bs.GetQWord();
						v1.timescale = bs.GetDWord();
						v1.duration = bs.GetQWord();
					}
					else
					{
						v0.creation_time = bs.GetDWord();
						v0.modification_time = bs.GetDWord();
						v0.timescale = bs.GetDWord();
						v0.duration = bs.GetDWord();
					}

					rate = bs.GetLong();
					volume = bs.GetShort();
					reserved_0 = bs.GetWord();
					reserved_1[0] = bs.GetDWord();
					reserved_1[1] = bs.GetDWord();

					for (size_t i = 0; i < _countof(matrix); i++)
						matrix[i] = bs.GetLong();

					for (size_t i = 0; i < _countof(pre_defined); i++)
						pre_defined[i] = bs.GetDWord();

					next_track_ID = bs.GetDWord();

					SkipLeftBits(bs);

					return 0;
				}
			}PACKED;

			/*
			Box Type: 'trak'
			Container: Movie Box ('moov')
			Mandatory: Yes
			Quantity: One or more
			*/
			struct TrackBox : public ContainerBox
			{
				/*
				Box Type: 'tkhd'
				Container: Track Box ('trak')
				Mandatory: Yes
				Quantity: Exactly one
				*/
				struct TrackHeaderBox : public FullBox
				{
					union
					{
						struct
						{
							uint64_t	creation_time;
							uint64_t	modification_time;
							uint32_t	track_ID;
							uint32_t	reserved;
							uint64_t	duration;
						}PACKED v1;
						struct
						{
							uint32_t	creation_time;
							uint32_t	modification_time;
							uint32_t	track_ID;
							uint32_t	reserved;
							uint32_t	duration;
						}PACKED v0;
					}PACKED;

					uint32_t	reserved_0[2] = { 0 };
					int16_t		layer = 0;
					int16_t		alternate_group = 0;
					int16_t		volume = 0;
					uint16_t	reserved_1 = 0;
					int32_t		matrix[9] = { 0 };
					int32_t		width = 0;
					int32_t		height = 0;
			
					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						if (version == 1)
						{
							v1.creation_time = bs.GetQWord();
							v1.modification_time = bs.GetQWord();
							v1.track_ID = bs.GetDWord();
							v1.reserved = bs.GetDWord();
							v1.duration = bs.GetQWord();
						}
						else
						{
							v0.creation_time = bs.GetDWord();
							v0.modification_time = bs.GetDWord();
							v0.track_ID = bs.GetDWord();
							v0.reserved = bs.GetDWord();
							v0.duration = bs.GetDWord();
						}

						reserved_0[0] = bs.GetDWord();
						reserved_0[1] = bs.GetDWord();

						layer = bs.GetShort();
						alternate_group = bs.GetShort();
						volume = bs.GetShort();
						reserved_1 = bs.GetWord();
					
						for (size_t i = 0; i < _countof(matrix); i++)
							matrix[i] = bs.GetLong();

						width = bs.GetLong();
						height = bs.GetLong();

						SkipLeftBits(bs);

						return 0;
					}
			
				}PACKED;

				/*
				Box Type: `tref'
				Container: Track Box ('trak')
				Mandatory: No
				Quantity: Zero or one
				*/
				struct TrackReferenceBox : public Box
				{
					struct TrackReferenceTypeBox : public Box
					{
						std::vector<uint32_t>	track_IDs;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = Box::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							if (left_bytes >= sizeof(uint32_t))
							{
								for (uint64_t i = 0; i < left_bytes / sizeof(uint32_t); i++)
								{
									track_IDs.push_back(bs.GetDWord());
								}
							}

							SkipLeftBits(bs);

							return 0;
						}
					};

					std::vector<TrackReferenceTypeBox> TypeBoxes;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						while (LeftBytes(bs) >= MIN_BOX_SIZE)
						{
							TypeBoxes.emplace_back();
							if ((iRet = TypeBoxes.back().Unpack(bs)) < 0)
								return iRet;
						}

						SkipLeftBits(bs);

						return 0;
					}
				};

				/*
				Box Type: 'trgr'
				Container: Track Box ('trak')
				Mandatory: No
				Quantity: Zero or one
				*/
				struct TrackGroupBox : public Box
				{
					struct TrackGroupTypeBox : public FullBox
					{
						uint32_t	track_group_id = 0;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							if (left_bytes >= sizeof(uint32_t))
								track_group_id = bs.GetDWord();

							SkipLeftBits(bs);

							return 0;
						}
					}PACKED;

					std::vector<TrackGroupTypeBox> TypeBoxes;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						while (LeftBytes(bs) >= MIN_FULLBOX_SIZE)
						{
							TypeBoxes.emplace_back();
							if ((iRet = TypeBoxes.back().Unpack(bs)) < 0)
								return iRet;
						}

						SkipLeftBits(bs);
						return 0;
					}

				};

				/*
				Box Type: 'edts'
				Container: Track Box ('trak')
				Mandatory: No
				Quantity: Zero or one
				*/
				struct EditBox : public Box
				{
					/*
					Box Type: 'elst'
					Container: Edit Box ('edts')
					Mandatory: No
					Quantity: Zero or one
					*/
					struct EditListBox : public FullBox
					{
						struct Entry
						{
							uint64_t	segment_duration;
							int64_t		media_time;
							int16_t		media_rate_integer;
							int16_t		media_rate_fraction;
						}PACKED;

						uint32_t			entry_count = 0;
						std::vector<Entry>	entries;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							uint64_t left_bytes = LeftBytes(bs);
							if (left_bytes < sizeof(entry_count))
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}

							entry_count = bs.GetDWord();
							left_bytes -= sizeof(entry_count);

							for (uint64_t i = 0; i < entry_count; i++)
							{
								uint64_t actual_entry_size = (version == 1 ? 16ULL : 8ULL) + 4ULL;
								if (left_bytes < actual_entry_size)
									break;

								Entry entry;
								entry.segment_duration = version==1?bs.GetQWord(): bs.GetDWord();
								entry.media_time = version == 1 ? bs.GetLongLong() : bs.GetLong();
								entry.media_rate_integer = bs.GetShort();
								entry.media_rate_fraction = bs.GetShort();

								entries.push_back(entry);
							}

							SkipLeftBits(bs);
							return 0;
						}
					};

					EditListBox* edit_list_box = nullptr;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = Box::Unpack(bs)) < 0)
							return iRet;

						LoadOneBoxBranch(this, bs, (Box**)&edit_list_box);

						SkipLeftBits(bs);
						return 0;
					}

				};

				/*
				Box Type: 'mdia'
				Container: Track Box ('trak')
				Mandatory: Yes
				Quantity: Exactly one
				*/
				struct MediaBox : public ContainerBox
				{
					/*
					Box Type: 'mdhd'
					Container: Media Box ('mdia')
					Mandatory: Yes
					Quantity: Exactly one
					*/
					struct MediaHeaderBox : public FullBox
					{
						union
						{
							uint8_t		bytes[28] = { 0 };
							struct {
								uint64_t creation_time;
								uint64_t modification_time;
								uint32_t timescale;
								uint64_t duration;
							}PACKED v1;

							struct {
								uint32_t creation_time;
								uint32_t modification_time;
								uint32_t timescale;
								uint32_t duration;
							}PACKED v0;
						}PACKED;

						uint32_t	pad : 1;
						uint32_t	language : 15;
						uint32_t	pre_defined : 16;

						MediaHeaderBox()
							: pad(0), language(0), pre_defined(0) {
						}
					
						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;
							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							auto left_bytes = LeftBytes(bs);
							if ((version == 1 && left_bytes < sizeof(v1) + 4) ||
								(version != 1 && left_bytes < sizeof(v0) + 4))
								return -1;

							if (version == 1)
							{
								v1.creation_time = bs.GetQWord();
								v1.modification_time = bs.GetQWord();
								v1.timescale = bs.GetDWord();
								v1.duration = bs.GetQWord();
							}
							else
							{
								v0.creation_time = bs.GetDWord();
								v0.modification_time = bs.GetDWord();
								v0.timescale = bs.GetDWord();
								v0.duration = bs.GetDWord();
							}

							pad = (uint32_t)bs.GetBits(1);
							language = (uint32_t)bs.GetBits(15);
							pre_defined = (uint32_t)bs.GetWord();

							SkipLeftBits(bs);
							return 0;
						}

					}PACKED;

					/*
					Box Type: minf
					Container: Media Box (mdia)
					Mandatory: Yes
					Quantity: Exactly one
					*/
					struct MediaInformationBox : public ContainerBox
					{
						/*
						Box Types: 'vmhd', 'smhd', 'hmhd', 'nmhd'
						Container: Media Information Box ('minf')
						Mandatory: Yes
						Quantity: Exactly one specific media header shall be present
						*/
						struct VideoMediaHeaderBox : public FullBox
						{
							uint16_t	graphicsmode = 0;
							uint16_t	opcolor[3] = { 0 };

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

								if (LeftBytes(bs) < sizeof(graphicsmode) + sizeof(opcolor))
								{
									SkipLeftBits(bs);
									return RET_CODE_BOX_TOO_SMALL;
								}

								graphicsmode = bs.GetWord();
								for (size_t i = 0; i < _countof(opcolor); i++)
									opcolor[i] = bs.GetWord();

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						struct SoundMediaHeaderBox : public FullBox
						{
							uint16_t	balance = 0;
							uint16_t	reserved = 0;

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

								uint64_t left_bytes = LeftBytes(bs);
								if (left_bytes < sizeof(balance) + sizeof(reserved))
								{
									SkipLeftBits(bs);
									return RET_CODE_BOX_TOO_SMALL;
								}

								balance = bs.GetWord();
								reserved = bs.GetWord();

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						struct HintMediaHeaderBox : public FullBox
						{
							uint16_t	maxPDUsize = 0;
							uint16_t	avgPDUsize = 0;
							uint32_t	maxbitrate = 0;
							uint32_t	avgbitrate = 0;
							uint32_t	reserved = 0;

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

								if (LeftBytes(bs) < sizeof(maxPDUsize) + sizeof(avgPDUsize) + sizeof(maxbitrate) + sizeof(avgbitrate) + sizeof(reserved))
								{
									SkipLeftBits(bs);
									return RET_CODE_BOX_TOO_SMALL;
								}

								maxPDUsize = bs.GetWord();
								avgPDUsize = bs.GetWord();
								maxbitrate = bs.GetDWord();
								avgbitrate = bs.GetDWord();
								reserved = bs.GetDWord();

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						struct NullMediaHeaderBox : public FullBox
						{
							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						/*
						Box Type: 'stbl'
						Container: Media Information Box ('minf')
						Mandatory: Yes
						Quantity: Exactly one
						*/
						struct SampleTableBox : public ContainerBox
						{
							/*
							Box Types: 'stsd'
							Container: Sample Table Box ('stbl')
							Mandatory: Yes
							Quantity: Exactly one
							*/
							struct SampleEntry : public Box
							{
								uint8_t		reserved[6] = { 0 };
								uint16_t	data_reference_index;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = Box::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(reserved) + sizeof(data_reference_index))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									for (size_t i = 0; i < _countof(reserved); i++)
										reserved[i] = bs.GetByte();

									data_reference_index = bs.GetWord();

									// This is a base class, can't skip left bits
									//SkipLeftBits(bs);
									return 0;
								}
							}PACKED;

							struct HintSampleEntry : public SampleEntry
							{
								std::vector<uint8_t>	data;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = SampleEntry::Unpack(bs)) < 0)
										return iRet;

									// TODO...

									SkipLeftBits(bs);
									return 0;
								}
							};

							struct BitRateBox : public Box
							{
								uint32_t	bufferSizeDB = 0;
								uint32_t	maxBitrate = 0;
								uint32_t	avgBitrate = 0;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = Box::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(bufferSizeDB) + sizeof(maxBitrate) + sizeof(avgBitrate))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									bufferSizeDB = bs.GetDWord();
									maxBitrate = bs.GetDWord();
									avgBitrate = bs.GetDWord();

									SkipLeftBits(bs);
									return 0;
								}
							}PACKED;

							struct MetaDataSampleEntry : public SampleEntry
							{
								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = SampleEntry::Unpack(bs)) < 0)
										return iRet;

									SkipLeftBits(bs);
									return 0;
								}
							}PACKED;

							struct XMLMetaDataSampleEntry : public MetaDataSampleEntry
							{
								std::string		content_encoding;			// optional
								std::string		str_namespace;
								std::string		schema_location;			// optional
								BitRateBox*		ptr_BitRateBox = nullptr;	// optional

								virtual ~XMLMetaDataSampleEntry()
								{
									if (ptr_BitRateBox)
										delete ptr_BitRateBox;
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = MetaDataSampleEntry::Unpack(bs)) < 0)
										return iRet;

									ReadString(bs, content_encoding);
									ReadString(bs, str_namespace);
									ReadString(bs, schema_location);

									if (LeftBytes(bs) >= sizeof(BitRateBox::bufferSizeDB) + sizeof(BitRateBox::maxBitrate) + sizeof(BitRateBox::avgBitrate))
									{
										ptr_BitRateBox = new BitRateBox();
										if ((iRet = ptr_BitRateBox->Unpack(bs)) < 0)
										{
											SkipLeftBits(bs);
											return RET_CODE_BOX_TOO_SMALL;
										}
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							struct TextMetaDataSampleEntry : public MetaDataSampleEntry
							{
								std::string		content_encoding;	// optional
								std::string		mime_format;
								BitRateBox*		ptr_BitRateBox;		// optional

								virtual ~TextMetaDataSampleEntry()
								{
									if (ptr_BitRateBox)
										delete ptr_BitRateBox;
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = MetaDataSampleEntry::Unpack(bs)) < 0)
										return iRet;

									ReadString(bs, content_encoding);
									ReadString(bs, mime_format);

									if (LeftBytes(bs) >= sizeof(BitRateBox::bufferSizeDB) + sizeof(BitRateBox::maxBitrate) + sizeof(BitRateBox::avgBitrate))
									{
										ptr_BitRateBox = new BitRateBox();
										if ((iRet = ptr_BitRateBox->Unpack(bs)) < 0)
										{
											SkipLeftBits(bs);
											return RET_CODE_BOX_TOO_SMALL;
										}
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							struct URIBox : public FullBox
							{
								std::string		theURI;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									ReadString(bs, theURI);

									SkipLeftBits(bs);
									return 0;
								}
							};

							struct URIInitBox : public FullBox
							{
								uint8_t		uri_initialization_data[];

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									SkipLeftBits(bs);
									return 0;
								}
							}PACKED;

							struct URIMetaSampleEntry : public MetaDataSampleEntry
							{
								URIBox				the_label;
								URIInitBox*			ptr_init = nullptr;		// optional
								//MPEG4BitRateBox*	ptr_BitRateBox;			// optional

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = MetaDataSampleEntry::Unpack(bs)) < 0)
										return iRet;

									if ((iRet = the_label.Unpack(bs)) < 0)
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							// Visual Sequences
							struct PixelAspectRatioBox : public Box
							{
								uint32_t	hSpacing = 0;
								uint32_t	vSpacing = 0;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = Box::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(hSpacing) + sizeof(vSpacing))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									hSpacing = bs.GetDWord();
									vSpacing = bs.GetDWord();

									SkipLeftBits(bs);
									return 0;
								}
							}PACKED;

							struct CleanApertureBox : public Box
							{
								uint32_t	cleanApertureWidthN = 0;
								uint32_t	cleanApertureWidthD = 0;
								uint32_t	cleanApertureHeightN = 0;
								uint32_t	cleanApertureHeightD = 0;
								uint32_t	horizOffN = 0;
								uint32_t	horizOffD = 0;
								uint32_t	vertOffN = 0;
								uint32_t	vertOffD = 0;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = Box::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < (uint64_t)(&vertOffD - &cleanApertureWidthN) + sizeof(vertOffD))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									cleanApertureWidthN = bs.GetDWord();
									cleanApertureWidthD = bs.GetDWord();
									cleanApertureHeightN = bs.GetDWord();
									cleanApertureHeightD = bs.GetDWord();
									horizOffN = bs.GetDWord();
									horizOffD = bs.GetDWord();
									vertOffN = bs.GetDWord();
									vertOffD = bs.GetDWord();

									SkipLeftBits(bs);
									return 0;
								}
							};

							struct ColourInformationBox : public Box
							{
								uint32_t	colour_type = 0;

								union
								{
									uint8_t			bytes[7] = { 0 };
									struct
									{
										// On-Screen colours
										uint16_t	colour_primaries;
										uint16_t	transfer_characteristics;
										uint16_t	matrix_coefficients;
										uint8_t		full_range_flag:1;
										uint8_t		reserved:7;
									}PACKED;

									// It is the same definition in H.265/H.264 VUI color_primaries, transfer_chatacters and matrix_coeffs
									struct
									{
										uint16_t	Primaries_index;
										uint16_t	Transfer_function_index;
										uint16_t	Matrix_index;
									}PACKED;
							
									// restricted ICC profile

									// unrestricted ICC profile
								}PACKED;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = Box::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(colour_type))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									colour_type = bs.GetDWord();
										left_bytes -= 4;

									if (colour_type == 'nclx')
									{
										if (left_bytes < 7)
										{
											iRet =  RET_CODE_BOX_TOO_SMALL;
											goto done;
										}

										colour_primaries = bs.GetWord();
										transfer_characteristics = bs.GetWord();
										matrix_coefficients = bs.GetWord();
										full_range_flag = (uint8_t)bs.GetBits(1);
										reserved = (uint8_t)bs.GetBits(7);
									}
									else if (colour_type == 'nclc')
									{
										if (left_bytes < 6)
										{
											iRet = RET_CODE_BOX_TOO_SMALL;
											goto done;
										}

										Primaries_index = bs.GetWord();
										Transfer_function_index = bs.GetWord();
										Matrix_index = bs.GetWord();
									}
									else if (colour_type == 'rICC')
									{
										// TODO...
									}
									else if (colour_type == 'prof')
									{
										// TODO...
									}

								done:
									SkipLeftBits(bs);
									return iRet;
								}

							}PACKED;

							struct RtpHintSampleEntry : public SampleEntry
							{
								struct TimeScaleEntry : public Box
								{
									uint32_t				timescale = 0;

									virtual int Unpack(CBitstream& bs)
									{
										int iRet = RET_CODE_SUCCESS;
										if ((iRet = Box::Unpack(bs)) < 0)
											return iRet;

										uint64_t left_bytes = LeftBytes(bs);
										if (left_bytes < sizeof(timescale))
										{
											iRet = RET_CODE_BOX_TOO_SMALL;
											goto done;
										}

										timescale = bs.GetDWord();

									done:
										SkipLeftBits(bs);
										return iRet;
									}

								}PACKED;

								struct TimeOffset : public Box
								{
									int32_t				offset = 0;

									virtual int Unpack(CBitstream& bs)
									{
										int iRet = RET_CODE_SUCCESS;
										if ((iRet = Box::Unpack(bs)) < 0)
											return iRet;

										uint64_t left_bytes = LeftBytes(bs);
										if (left_bytes < sizeof(offset))
										{
											iRet = RET_CODE_BOX_TOO_SMALL;
											goto done;
										}

										offset = bs.GetLong();

									done:
										SkipLeftBits(bs);
										return iRet;
									}

								}PACKED;

								struct SequenceOffset : public Box
								{
									int32_t				offset = 0;

									virtual int Unpack(CBitstream& bs)
									{
										int iRet = RET_CODE_SUCCESS;
										if ((iRet = Box::Unpack(bs)) < 0)
											return iRet;

										uint64_t left_bytes = LeftBytes(bs);
										if (left_bytes < sizeof(offset))
										{
											iRet = RET_CODE_BOX_TOO_SMALL;
											goto done;
										}

										offset = bs.GetLong();

									done:
										SkipLeftBits(bs);
										return iRet;
									}

								}PACKED;

								uint16_t				hinttrackversion = 0;
								uint16_t				highestcompatibleversion = 0;
								uint32_t				maxpacketsize = 0;

								TimeScaleEntry*			ptr_tims = nullptr;
								TimeOffset*				ptr_tsro = nullptr;
								SequenceOffset*			ptr_snro = nullptr;

								virtual int Unpack(CBitstream& bs)
								{
									Box* ptr_box = nullptr;
									int iRet = RET_CODE_SUCCESS;
									if ((iRet = SampleEntry::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(hinttrackversion) + sizeof(highestcompatibleversion) + sizeof(maxpacketsize))
									{
										iRet = RET_CODE_BOX_TOO_SMALL;
										goto done;
									}

									hinttrackversion = bs.GetWord();
									highestcompatibleversion = bs.GetWord();
									maxpacketsize = bs.GetDWord();

									left_bytes -= sizeof(hinttrackversion) + sizeof(highestcompatibleversion) + sizeof(maxpacketsize);
									while (left_bytes >= MIN_BOX_SIZE && LoadOneBoxBranch(this, bs, &ptr_box) >= 0)
									{
										switch (ptr_box->type)
										{
										case 'tims':
											ptr_tims = (TimeScaleEntry*)ptr_box;
											break;
										case 'tsro':
											ptr_tsro = (TimeOffset*)ptr_box;
											break;
										case 'snro':
											ptr_snro = (SequenceOffset*)ptr_box;
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

							struct VisualSampleEntry : public SampleEntry
							{
								uint16_t				pre_defined_0 = 0;
								uint16_t				reserved_0 = 0;
								uint32_t				pre_defined_1[3] = { 0 };
								uint16_t				width = 0;
								uint16_t				height = 0;
								uint32_t				horizresolution = 0;
								uint32_t				vertresolution = 0;
								uint32_t				reserved_1 = 0;
								uint16_t				frame_count = 0;
								uint8_t					compressorname_size = 0;
								uint8_t					compressorname[31] = { 0 };
								uint16_t				depth = 0x0018;	
								int16_t					pre_defined_2 = -1;
								// other boxes from derived specifications
								CleanApertureBox*		ptr_clap = nullptr;	// optional
								PixelAspectRatioBox*	ptr_pasp = nullptr;	// optional

								int _UnpackHeader(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = SampleEntry::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < (uint64_t)((uint8_t*)(&pre_defined_2) - (uint8_t*)(&pre_defined_0)) + sizeof(pre_defined_2))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									pre_defined_0 = bs.GetWord();
									reserved_0 = bs.GetWord();
									for (size_t i = 0; i < _countof(pre_defined_1); i++)
										pre_defined_1[i] = bs.GetDWord();
									width = bs.GetWord();
									height = bs.GetWord();
									horizresolution = bs.GetDWord();
									vertresolution = bs.GetDWord();
									reserved_1 = bs.GetDWord();
									frame_count = bs.GetWord();
									compressorname_size = bs.GetByte();
									for (size_t i = 0; i < _countof(compressorname); i++)
										compressorname[i] = bs.GetByte();
									depth = bs.GetWord();
									pre_defined_2 = bs.GetShort();

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
										switch (ptr_box->type)
										{
										case 'clap':
											ptr_clap = (CleanApertureBox*)ptr_box;
												break;
										case 'pasp':
											ptr_pasp = (PixelAspectRatioBox*)ptr_box;
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

							// Audio Sequences
							struct AudioSampleEntry : public SampleEntry
							{
								uint32_t				reserved_0[2] = { 0 };
								uint16_t				channelcount = 2;
								uint16_t				samplesize = 16;
								uint16_t				pre_defined = 0;
								uint16_t				reserved_1 = 0;
								uint32_t				samplerate = 0;

								int _UnpackHeader(CBitstream& bs)
								{
									int iRet = RET_CODE_SUCCESS;

									if ((iRet = SampleEntry::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < (uint64_t)(&samplerate - &reserved_0[0]) + sizeof(samplerate))
									{
										iRet = RET_CODE_BOX_TOO_SMALL;
										goto done;
									}

									reserved_0[0] = bs.GetDWord();
									reserved_0[1] = bs.GetDWord();
									channelcount = bs.GetWord();
									samplesize = bs.GetWord();
									pre_defined = bs.GetWord();
									reserved_1 = bs.GetWord();
									samplerate = bs.GetDWord();

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
							};

							/*
							Box Types: 'stsd'
							Container: Sample Table Box ('stbl')
							Mandatory: Yes
							Quantity: Exactly one
							*/
							struct SampleDescriptionBox : public FullBox
							{
								uint32_t	handler_type = 0;
								uint32_t	entry_count = 0;

								std::vector<Box*>	SampleEntries;

								virtual int Unpack(CBitstream& bs);
							};

							/*
							Box Type: 'stts'
							Container: Sample Table Box ('stbl')
							Mandatory: Yes
							Quantity: Exactly one
							*/
							struct TimeToSampleBox : public FullBox
							{
								struct Entry
								{
									uint32_t	sample_count;
									uint32_t	sample_delta;
								}PACKED;

								uint32_t			entry_count = 0;
								std::vector<Entry>	entries;

								bool FindSamplePos(uint32_t sample_number, size_t& idxEntry, uint32_t& posInEntry, int64_t& time_pos)
								{
									if (sample_number < 1)
										return false;

									int64_t sample_time = 0;
									uint32_t elapse_num_of_samples = 0;
									for (size_t i = 0; i < entries.size(); i++)
									{
										if (sample_number >= elapse_num_of_samples + 1 &&
											sample_number <= elapse_num_of_samples + entries[i].sample_count)
										{
											idxEntry = i;
											posInEntry = sample_number - elapse_num_of_samples  - 1;
											time_pos = sample_time + (int64_t)posInEntry*entries[i].sample_delta;
											return true;
										}

										elapse_num_of_samples += entries[i].sample_count;
										sample_time += (int64_t)entries[i].sample_count*entries[i].sample_delta;
									}

									return false;
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();

									uint64_t left_bytes = LeftBytes(bs);
									for (uint32_t i = 0; i < (uint32_t)AMP_MIN((uint64_t)entry_count, left_bytes/8); i++)
										entries.push_back({ bs.GetDWord(), bs.GetDWord() });

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'ctts'
							Container: Sample Table Box ('stbl')
							Mandatory: No
							Quantity: Zero or one
							*/
							struct CompositionOffsetBox : public FullBox
							{
								struct Entry
								{
									union
									{
										struct
										{
											uint32_t	sample_count;
											uint32_t	sample_offset;
										}v0;
										struct
										{
											uint32_t	sample_count;
											int32_t		sample_offset;
										}v1;
									}PACKED;
								}PACKED;

								uint32_t			entry_count = 0;
								std::vector<Entry>	entries;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();
									left_bytes -= sizeof(entry_count);

									for (uint32_t i = 0; i < entry_count; i++)
									{
										if (left_bytes < sizeof(Entry))
											break;

										Entry entry;
										if (version == 1)
										{
											entry.v1.sample_count = bs.GetDWord();
											entry.v1.sample_offset = bs.GetDWord();
										}
										else
										{
											entry.v0.sample_count = bs.GetDWord();
											entry.v0.sample_offset = bs.GetLong();
										}
										entries.push_back(entry);

										left_bytes -= sizeof(Entry);
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'cslg'
							Container: Sample Table Box ('stbl')
							Mandatory: No
							Quantity: Zero or one
							*/
							struct CompositionToDecodeBox : public FullBox
							{
								int32_t		compositionToDTSShift = 0;
								int32_t		leastDecodeToDisplayDelta = 0;
								int32_t		greatestDecodeToDisplayDelta = 0;
								int32_t		compositionStartTime = 0;
								int32_t		compositionEndTime = 0;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(int32_t) * 5)
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									compositionToDTSShift = bs.GetLong();
									leastDecodeToDisplayDelta = bs.GetLong();
									greatestDecodeToDisplayDelta = bs.GetLong();
									compositionStartTime = bs.GetLong();
									compositionEndTime = bs.GetLong();

									SkipLeftBits(bs);
									return 0;
								}
							}PACKED;

							/*
							Box Type: 'stss'
							Container: Sample Table Box ('stbl')
							Mandatory: No
							Quantity: Zero or one
							*/
							struct SyncSampleBox : public FullBox
							{
								uint32_t				entry_count = 0;
								std::vector<uint32_t>	sample_numbers;

								bool IsSyncFrame(uint32_t sample_number)
								{
									if (entry_count == 0 || sample_numbers.size() == 0)
										return false;
									return std::binary_search(sample_numbers.cbegin(), sample_numbers.cend(), sample_number);
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();
									left_bytes -= sizeof(entry_count);

									for (uint32_t i = 0; i < entry_count; i++)
									{
										if (left_bytes < 4)
											break;

										sample_numbers.push_back(bs.GetDWord());

										left_bytes -= 4;
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'stsh'
							Container: Sample Table Box ('stbl')
							Mandatory: No
							Quantity: Zero or one
							*/
							struct ShadowSyncSampleBox : public FullBox
							{
								struct Entry
								{
									uint32_t	shadowed_sample_number;
									uint32_t	sync_sample_number;
								}PACKED;

								uint32_t			entry_count = 0;
								std::vector<Entry>	entries;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();
									left_bytes -= sizeof(entry_count);

									for (uint32_t i = 0; i < entry_count; i++)
									{
										if (left_bytes < sizeof(Entry))
											break;

										entries.push_back({bs.GetDWord(), bs.GetDWord()});

										left_bytes -= sizeof(Entry);
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'stsz', 'stz2'
							Container: Sample Table Box ('stbl')
							Mandatory: Yes
							Quantity: Exactly one variant must be present
							*/
							struct SampleSizeBox : public FullBox
							{
								uint32_t				sample_size = 0;
								uint32_t				sample_count = 0;

								std::vector<uint32_t>	entry_size;

								uint32_t GetMaxSampleSize()
								{
									uint32_t max_sample_size = 0UL;
									if (sample_size == 0)
									{
										for (size_t i = 0; i < entry_size.size(); i++)
											if (max_sample_size < entry_size[i])
												max_sample_size = entry_size[i];
									}
									else
										max_sample_size = sample_size;

									return max_sample_size;
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);
									if (left_bytes < sizeof(sample_size) + sizeof(sample_count))
									{
										iRet = RET_CODE_BOX_TOO_SMALL;
										goto done;
									}

									sample_size = bs.GetDWord();
									sample_count = bs.GetDWord();
									left_bytes -= sizeof(sample_size) + sizeof(sample_count);

									if (sample_size == 0)
									{
										uint32_t actual_sample_count = AMP_MIN((uint32_t)(left_bytes>>2), sample_count);
										entry_size.resize(actual_sample_count);
										for (uint32_t i = 0; i < sample_count; i++)
											entry_size[i] = bs.GetDWord();
									}

								done:
									SkipLeftBits(bs);
									return iRet;
								}
							};

							struct CompactSampleSizeBox : public FullBox
							{
								uint32_t				reserved : 24;
								uint32_t				field_size : 8;
								uint32_t				sample_count = 0;

								std::vector<uint16_t>	entry_size;

								CompactSampleSizeBox() : reserved(0), field_size(0) {
								}

								uint16_t GetMaxSampleSize()
								{
									uint16_t max_sample_size = 0;
									for (size_t i = 0; i < entry_size.size(); i++)
										if (max_sample_size < entry_size[i])
											max_sample_size = entry_size[i];
									return max_sample_size;
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(uint32_t) + sizeof(uint32_t))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									reserved = (uint32_t)bs.GetBits(24);
									field_size = (uint32_t)bs.GetBits(8);
									sample_count = bs.GetDWord();

									assert(field_size == 4 || field_size == 8 || field_size == 16);
									if (LeftBytes(bs) < (uint64_t)sample_count * field_size)
									{
										for (uint32_t i = 0; i < sample_count; i++)
											entry_size.push_back((uint16_t)bs.GetBits(field_size));
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'stsc'
							Container: Sample Table Box ('stbl')
							Mandatory: Yes
							Quantity: Exactly one
							*/
							struct SampleToChunkBox : public FullBox
							{
								struct EntryInfo
								{
									uint32_t	first_chunk;
									uint32_t	samples_per_chunk;
									uint32_t	sample_description_index;
								}PACKED;

								uint32_t				entry_count = 0;
								std::vector<EntryInfo>	entry_infos;

								bool FindChunk(uint32_t sample_number, uint32_t total_chunks, uint32_t& chunk_number, uint32_t& first_sample_number, uint32_t &sample_desc_number)
								{
									uint32_t total_numbers = 0;
									for (size_t i = 0; i < entry_infos.size(); i++)
									{
										uint32_t end_chunk = (i + 1 >= entry_infos.size()) ? (total_chunks + 1) : entry_infos[i+1].first_chunk;

										if (end_chunk <= entry_infos[i].first_chunk)
											end_chunk = entry_infos[i].first_chunk + 1;

										if (sample_number >= total_numbers + 1 &&
											sample_number <= total_numbers + (end_chunk - entry_infos[i].first_chunk)*entry_infos[i].samples_per_chunk)
										{
											chunk_number = entry_infos[i].first_chunk + (sample_number - total_numbers - 1) / entry_infos[i].samples_per_chunk;
											first_sample_number = total_numbers + (chunk_number - entry_infos[i].first_chunk)*entry_infos[i].samples_per_chunk + 1;
											sample_desc_number = entry_infos[i].sample_description_index;
											return true;
										}

										total_numbers += (end_chunk - entry_infos[i].first_chunk)*entry_infos[i].samples_per_chunk;
									}

									return false;
								}

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();

									for (uint32_t i = 0; i < entry_count; i++)
									{
										entry_infos.emplace_back();
										EntryInfo& entry_info = entry_infos.back();
										entry_info.first_chunk = bs.GetDWord();
										entry_info.samples_per_chunk = bs.GetDWord();
										entry_info.sample_description_index = bs.GetDWord();
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'stco', 'co64'
							Container: Sample Table Box ('stbl')
							Mandatory: Yes
							Quantity: Exactly one variant must be present
							*/
							struct ChunkOffsetBox : public FullBox
							{
								uint32_t				entry_count = 0;
								std::vector<uint32_t>	chunk_offset;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();
									uint64_t left_bytes = LeftBytes(bs);

									if (left_bytes < (uint64_t)sizeof(uint32_t)*entry_count)
										printf("The 'stco' box size is too small {size: %" PRIu64 ", entry_count: %" PRIu32 "}.\n", size, entry_count);

									uint32_t actual_entry_count = left_bytes / sizeof(uint32_t) < (uint64_t)entry_count ? (uint32_t)(left_bytes / sizeof(uint32_t)) : entry_count;
									for (uint32_t i = 0; i < actual_entry_count; i++)
										chunk_offset.push_back(bs.GetDWord());

									SkipLeftBits(bs);
									return 0;
								}
							};

							struct ChunkLargeOffsetBox : public FullBox
							{
								uint32_t				entry_count = 0;
								std::vector<uint64_t>	chunk_offset;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(entry_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									entry_count = bs.GetDWord();
									uint64_t left_bytes = LeftBytes(bs);

									if (left_bytes < (uint64_t)sizeof(uint32_t)*entry_count)
										printf("The 'stco' box size is too small {size: %" PRIu64 ", entry_count: %" PRIu32 "}.\n", size, entry_count);

									uint32_t actual_entry_count = left_bytes / sizeof(uint32_t) < (uint64_t)entry_count ? (uint32_t)(left_bytes / sizeof(uint32_t)) : entry_count;
									for (uint32_t i = 0; i < actual_entry_count; i++)
										chunk_offset.push_back(bs.GetQWord());

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'padb'
							Container: Sample Table ('stbl')
							Mandatory: No
							Quantity: Zero or one
							*/
							struct PaddingBitsBox : public FullBox
							{
								union PaddingEntry 
								{
									struct
									{
										uint8_t	reserved_1 : 1;
										uint8_t	pad1 : 3;
										uint8_t	reserved_2 : 1;
										uint8_t	pad2 : 3;
									}PACKED;
									uint8_t	padding_byte;
								}PACKED;

								uint32_t					sample_count = 0;
								std::vector<PaddingEntry>	pads;
							
								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									if (LeftBytes(bs) < sizeof(sample_count))
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									sample_count = bs.GetDWord();
									uint64_t left_bytes = LeftBytes(bs);

									uint64_t padding_byte_count = (sample_count + 1) >> 1;
									if (left_bytes < padding_byte_count)
										printf("The 'padb' box size is too small {size: %" PRIu64 ", sample_count: %" PRIu32 "}.\n", size, sample_count);

									for (uint32_t i = 0; i < AMP_MIN(left_bytes, padding_byte_count); i++)
									{
										pads.emplace_back();
										auto& pad_entry = pads.back();
										pad_entry.reserved_1 = (uint8_t)bs.GetBits(1);
										pad_entry.pad1 = (uint8_t)bs.GetBits(3);
										pad_entry.reserved_2 = (uint8_t)bs.GetBits(1);
										pad_entry.pad2 = (uint8_t)bs.GetBits(3);
									}

									SkipLeftBits(bs);
									return 0;
								}
							};

							/*
							Box Type: 'stdp'
							Container: Sample Table Box ('stbl').
							Mandatory: No.
							Quantity: Zero or one.
							*/
							struct DegradationPriorityBox : public FullBox
							{
								std::vector<uint16_t>		priorities;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;
									Box* pChild = nullptr;
									uint32_t sample_count = 0;
									uint64_t left_bytes = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									// Find 'stsz' or 'stz2', and then get sample count
									if (container == nullptr)
									{
										printf("No container box for the current 'stdp' box.\n");
										goto done;
									}

									if (container->type != 'stbl')
									{
										printf("The 'stdp' box container box is not in 'stbl' box.\n");
										goto done;
									}

									pChild = container->first_child;
									while (pChild != nullptr && pChild->type != 'stsz' && pChild->type != 'stz2')
										pChild = pChild->next_sibling;

									if (pChild == nullptr)
									{
										printf("Can't find 'stsz' or 'stz2' box to get sample count.\n");
										goto done;
									}

									if (pChild->type == 'stsz')
										sample_count = ((SampleSizeBox*)pChild)->sample_count;
									else if (pChild->type == 'stz2')
										sample_count = ((CompactSampleSizeBox*)pChild)->sample_count;

									left_bytes = LeftBytes(bs);
									for (uint32_t i = 0; i < (uint32_t)AMP_MIN((uint64_t)sample_count, left_bytes >> 1); i++)
										priorities.push_back(bs.GetWord());

								done:
									SkipLeftBits(bs);
									return 0;
								}

							};

							/*
							Box Types: 'sdtp'
							Container: Sample Table Box ('stbl')
							Mandatory: No
							Quantity: Zero or one
							*/
							struct SampleDependencyTypeBox : public FullBox
							{
								union Entry
								{
									struct
									{
										uint8_t		is_leading : 2;
										uint8_t		sample_depends_on : 2;
										uint8_t		sample_is_depended_on : 2;
										uint8_t		sample_has_redundancy : 2;
									}PACKED;
									uint8_t		uint8_value;
								}PACKED;

								std::vector<Entry>	entries;

								virtual int Unpack(CBitstream& bs)
								{
									int iRet = 0;
									Box* pChild = nullptr;
									uint32_t sample_count = 0;

									if ((iRet = FullBox::Unpack(bs)) < 0)
										return iRet;

									uint64_t left_bytes = LeftBytes(bs);

									// Find 'stsz' or 'stz2', and then get sample count
									if (container == nullptr)
									{
										printf("[Box][sdtp] No container box for the current 'sdtp' box.\n");
										goto done;
									}

									if (container->type != 'stbl')
									{
										printf("[Box][sdtp] The 'sdtp' box container box is not in 'stbl' box.\n");
										goto done;
									}

									pChild = container->first_child;
									while (pChild != nullptr && pChild->type != 'stsz' && pChild->type != 'stz2')
										pChild = pChild->next_sibling;

									if (pChild == nullptr)
									{
										printf("[Box][sdtp] Can't find 'stsz' or 'stz2' box to get sample count.\n");
										//goto done;
									}

									if (left_bytes > UINT32_MAX)
										goto done;

									sample_count = (uint32_t)left_bytes;
									if (pChild != nullptr)
									{
										if (pChild->type == 'stsz')
											sample_count = ((SampleSizeBox*)pChild)->sample_count;
										else if (pChild->type == 'stz2')
											sample_count = ((CompactSampleSizeBox*)pChild)->sample_count;
									}

									for (uint32_t i = 0; i < (uint32_t)AMP_MIN((uint64_t)sample_count, left_bytes); i++)
									{
										Entry entry;
										entry.is_leading = (uint8_t)bs.GetBits(2);
										entry.sample_depends_on = (uint8_t)bs.GetBits(2);
										entry.sample_is_depended_on = (uint8_t)bs.GetBits(2);
										entry.sample_has_redundancy = (uint8_t)bs.GetBits(2);
										entries.push_back(entry);
									}

								done:
									SkipLeftBits(bs);
									return 0;
								}
							};

							SampleDescriptionBox*		sample_description_box = nullptr;
							TimeToSampleBox*			time_to_sample_box = nullptr;
							CompositionOffsetBox*		composition_offset_box = nullptr;
							CompositionToDecodeBox*		composition_to_decode_box = nullptr;
							SampleToChunkBox*			sample_to_chunk_box = nullptr;
							SampleSizeBox*				sample_size_box = nullptr;
							CompactSampleSizeBox*		compact_sample_size_box = nullptr;
							ChunkOffsetBox*				chunk_offset_box = nullptr;
							ChunkLargeOffsetBox*		chunk_large_offset_box = nullptr;
							SyncSampleBox*				sync_sample_box = nullptr;
							ShadowSyncSampleBox*		shadow_sync_sample_box = nullptr;
							PaddingBitsBox*				padding_bits_box = nullptr;
							DegradationPriorityBox*		degradation_priority_box = nullptr;
							SampleDependencyTypeBox*	sample_dependency_type_box = nullptr;
							std::vector<SampleToGroupBox*>
														sample_to_group_boxes;
							std::vector<SampleGroupDescriptionBox*>
														sample_group_description_boxes;
							SubSampleInformationBox*	subsample_information_box = nullptr;
							std::vector<SampleAuxiliaryInformationSizesBox*>
														sample_aux_information_size_boxes;
							std::vector<SampleAuxiliaryInformationOffsetsBox*>
														sample_aux_information_offset_boxes;

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = ContainerBox::Unpack(bs);
								if (iRet < 0)
									return iRet;

								Box* ptr_child = first_child;
								while (ptr_child != nullptr)
								{
									switch (ptr_child->type)
									{
									case 'stsd':
										sample_description_box = (SampleDescriptionBox*)ptr_child;
										break;
									case 'stts':
										time_to_sample_box = (TimeToSampleBox*)ptr_child;
										break;
									case 'ctts':
										composition_offset_box = (CompositionOffsetBox*)ptr_child;
										break;
									case 'cslg':
										composition_to_decode_box = (CompositionToDecodeBox*)ptr_child;
										break;
									case 'stsc':
										sample_to_chunk_box = (SampleToChunkBox*)ptr_child;
										break;
									case 'stsz':
										sample_size_box = (SampleSizeBox*)ptr_child;
										break;
									case 'stz2':
										compact_sample_size_box = (CompactSampleSizeBox*)ptr_child;
										break;
									case 'stco':
										chunk_offset_box = (ChunkOffsetBox*)ptr_child;
										break;
									case 'co64':
										chunk_large_offset_box = (ChunkLargeOffsetBox*)ptr_child;
										break;
									case 'stss':
										sync_sample_box = (SyncSampleBox*)ptr_child;
										break;
									case 'stsh':
										shadow_sync_sample_box = (ShadowSyncSampleBox*)ptr_child;
										break;
									case 'padb':
										padding_bits_box = (PaddingBitsBox*)ptr_child;
										break;
									case 'stdp':
										degradation_priority_box = (DegradationPriorityBox*)ptr_child;
										break;
									case 'sdtp':
										sample_dependency_type_box = (SampleDependencyTypeBox*)ptr_child;
										break;
									case 'sbgp':
										sample_to_group_boxes.push_back((SampleToGroupBox*)ptr_child);
										break;
									case 'sgpd':
										sample_group_description_boxes.push_back((SampleGroupDescriptionBox*)ptr_child);
										break;
									case 'subs':
										subsample_information_box = (SubSampleInformationBox*)ptr_child;
										break;
									case 'saiz':
										sample_aux_information_size_boxes.push_back((SampleAuxiliaryInformationSizesBox*)ptr_child);
										break;
									case 'saio':
										sample_aux_information_offset_boxes.push_back((SampleAuxiliaryInformationOffsetsBox*)ptr_child);
										break;
									}

									ptr_child = ptr_child->next_sibling;
								}

								return iRet;
							}

						};

						VideoMediaHeaderBox*		video_media_header_box = nullptr;
						SoundMediaHeaderBox*		sound_media_header_box = nullptr;
						HintMediaHeaderBox*			hint_media_header_box = nullptr;
						NullMediaHeaderBox*			null_media_header_box = nullptr;
						DataInformationBox*			data_information_box = nullptr;
						SampleTableBox*				sample_table_box = nullptr;

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = ContainerBox::Unpack(bs);

							if (iRet < 0)
								return iRet;

							Box* ptr_child = first_child;
							while (ptr_child != nullptr)
							{
								switch (ptr_child->type)
								{
								case 'vmhd':
									video_media_header_box = (VideoMediaHeaderBox*)ptr_child;
									break;
								case 'smhd':
									sound_media_header_box = (SoundMediaHeaderBox*)ptr_child;
									break;
								case 'hmhd':
									hint_media_header_box = (HintMediaHeaderBox*)ptr_child;
									break;
								case 'nmhd':
									null_media_header_box = (NullMediaHeaderBox*)ptr_child;
									break;
								case 'dinf':
									data_information_box = (DataInformationBox*)ptr_child;
									break;
								case 'stbl':
									sample_table_box = (SampleTableBox*)ptr_child;
									break;
								}

								ptr_child = ptr_child->next_sibling;
							}

							SkipLeftBits(bs);

							return iRet;
						}

					}PACKED;

					MediaHeaderBox*		media_header_box = nullptr;
					HandlerBox*			handler_box = nullptr;
					MediaInformationBox*
										media_information_box = nullptr;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = ContainerBox::Unpack(bs);

						if (iRet < 0)
							return iRet;

						Box* ptr_child = first_child;
						while (ptr_child != nullptr)
						{
							switch (ptr_child->type)
							{
							case 'mdhd':
								media_header_box = (MediaHeaderBox*)ptr_child;
								break;
							case 'hdlr':
								handler_box = (HandlerBox*)ptr_child;
								break;
							case 'minf':
								media_information_box = (MediaInformationBox*)ptr_child;
								break;
							}

							ptr_child = ptr_child->next_sibling;
						}

						SkipLeftBits(bs);

						return iRet;
					}
				}PACKED;

				TrackHeaderBox*			track_header_box = nullptr;
				TrackReferenceBox*		track_reference_box = nullptr;
				TrackGroupBox*			track_group_box = nullptr;
				EditBox*				edit_box = nullptr;
				MediaBox*				media_box = nullptr;
				UserDataBox*			user_data_box = nullptr;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = ContainerBox::Unpack(bs);

					if (iRet < 0)
						return iRet;

					// Update the quick access references
					Box* ptr_child = first_child;
					while (ptr_child != nullptr)
					{
						switch (ptr_child->type)
						{
						case 'tkhd':
							track_header_box = (TrackHeaderBox*)ptr_child;
							break;
						case 'tref':
							track_reference_box = (TrackReferenceBox*)ptr_child;
							break;
						case 'trgr':
							track_group_box = (TrackGroupBox*)ptr_child;
							break;
						case 'edts':
							edit_box = (EditBox*)ptr_child;
							break;
						case 'mdia':
							media_box = (MediaBox*)ptr_child;
							break;
						case 'udta':
							user_data_box = (UserDataBox*)ptr_child;
							break;
						}

						ptr_child = ptr_child->next_sibling;
					}

					SkipLeftBits(bs);

					return iRet;
				}

				uint32_t GetTrackID()
				{
					if (track_header_box == nullptr)
						return 0;

					return track_header_box->version == 1 ? track_header_box->v1.track_ID : track_header_box->v0.track_ID;
				}

			}PACKED;

			/*
			Box Type: 'mvex'
			Container: Movie Box ('moov')
			Mandatory: No
			Quantity: Zero or one
			*/
			struct MovieExtendsBox : public ContainerBox
			{
				/*
				Box Type: 'mehd'
				Container: Movie Extends Box('mvex')
				Mandatory: No
				Quantity: Zero or one
				*/
				struct MovieExtendsHeaderBox : public FullBox
				{
					uint64_t		fragment_duration = 0;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						if (version == 1)
							fragment_duration = bs.GetQWord();
						else
							fragment_duration = bs.GetDWord();

						SkipLeftBits(bs);
						return 0;
					}
				}PACKED;

				/*
				Box Type: 'trex'
				Container: Movie Extends Box ('mvex')
				Mandatory: Yes
				Quantity: Exactly one for each track in the Movie Box
				*/
				struct TrackExtendsBox : public FullBox
				{
					uint32_t	track_ID = 0;
					uint32_t	default_sample_description_index = 0;
					uint32_t	default_sample_duration = 0;
					uint32_t	default_sample_size = 0;
					uint32_t	default_sample_flags = 0;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < 5 * sizeof(uint32_t))
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						track_ID = bs.GetDWord();
						default_sample_description_index = bs.GetDWord();
						default_sample_duration = bs.GetDWord();
						default_sample_size = bs.GetDWord();
						default_sample_flags = bs.GetDWord();

						SkipLeftBits(bs);
						return 0;
					}
				}PACKED;

				/*
				Box Type: 'leva'
				Container: Movie Extends Box ('mvex')
				Mandatory: No
				Quantity: Zero or one
				*/
				struct LevelAssignmentBox : public FullBox
				{
					struct Level
					{
						uint32_t	track_id;
						uint8_t		padding_flag : 1;
						uint8_t		assignment_type : 7;
						union
						{
							struct
							{
								uint32_t	grouping_type;
								uint32_t	grouping_type_parameter;
							}PACKED;
							uint32_t	sub_track_id;
						}PACKED;
					}PACKED;

					uint32_t			level_count = 0;
					std::vector<Level>	levels;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < sizeof(level_count))
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						level_count = bs.GetDWord();
						left_bytes -= sizeof(level_count);

						for (uint32_t i = 0; i < level_count; i++)
						{
							if (left_bytes < 5)
								break;

							Level level;
							level.track_id = bs.GetDWord();
							level.padding_flag = (uint8_t)bs.GetBits(1);
							level.assignment_type = (uint8_t)bs.GetBits(7);
							left_bytes -= 5;

							if (level.assignment_type != 0 && level.assignment_type != 1 && level.assignment_type != 4)
								continue;

							if (left_bytes < (level.assignment_type == 1 ? 8ULL : 4ULL))
								break;

							switch (level.assignment_type)
							{
							case 0:
								level.grouping_type = bs.GetDWord();
								left_bytes -= 4;
								break;
							case 1:
								level.grouping_type = bs.GetDWord();
								level.grouping_type_parameter = bs.GetDWord();
								left_bytes -= 4;
								break;
							case 4:
								level.sub_track_id = bs.GetDWord();
								left_bytes -= 4;
								break;
							}

							levels.push_back(level);
						}

						SkipLeftBits(bs);
						return 0;
					}
				};

				MovieExtendsHeaderBox*			movie_extends_header_box;
				std::vector<TrackExtendsBox*>	track_extends_boxes;
				LevelAssignmentBox*				level_assignment_box;

				MovieExtendsBox()
					: movie_extends_header_box(nullptr)
					, level_assignment_box(nullptr) {
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = ContainerBox::Unpack(bs);
					if (iRet < 0)
						return iRet;

					Box* ptr_child = first_child;
					while (ptr_child != nullptr)
					{
						switch (ptr_child->type)
						{
						case 'mehd':
							movie_extends_header_box = (MovieExtendsHeaderBox*)ptr_child;
							break;
						case 'trex':
							track_extends_boxes.push_back((TrackExtendsBox*)ptr_child);
							break;
						case 'leva':
							level_assignment_box = (LevelAssignmentBox*)ptr_child;
							break;
						}

						ptr_child = ptr_child->next_sibling;
					}

					return iRet;
				}
			};

			MovieHeaderBox*			movie_header_box = nullptr;					// Movie header
			std::vector<TrackBox*>	track_boxes;								// Track boxes
			MovieExtendsBox*		movie_extends_box = nullptr;				// Movie extends Box
			UserDataBox*			user_data_box = nullptr;					// User data box
			MetaBox*				meta_box = nullptr;							// Meta box
			AdditionalMetadataContainerBox*
									additional_metadata_container_box = nullptr;// Additional metadata container box

			uint32_t GetMediaTimeScale(uint32_t track_ID)
			{
				for (auto v : track_boxes)
				{
					if (v == nullptr || v->track_header_box == nullptr)
						continue;

					if (v->track_header_box != nullptr &&
						track_ID == (v->track_header_box->version == 0 ? v->track_header_box->v0.track_ID : v->track_header_box->v1.track_ID))
					{
						if (v->media_box != nullptr &&
							v->media_box->media_header_box != nullptr)
							return v->media_box->media_header_box->version == 0 ? 
								v->media_box->media_header_box->v0.timescale :
								v->media_box->media_header_box->v1.timescale;
						break;
					}
				}
				return 0;
			}

			uint32_t GetMovieTimeScale()
			{
				if (movie_header_box != nullptr)
					return movie_header_box->version == 0 ? movie_header_box->v0.timescale : movie_header_box->v1.timescale;

				return 0;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = ContainerBox::Unpack(bs);

				if (iRet < 0)
					return iRet;

				// Update the quick access references
				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					if (ptr_child->type == 'mvhd')
						movie_header_box = (MovieHeaderBox*)ptr_child;

					if (ptr_child->type == 'trak')
						track_boxes.push_back((TrackBox*)ptr_child);

					if (ptr_child->type == 'mvex')
						movie_extends_box = (MovieExtendsBox*)ptr_child;

					if (ptr_child->type == 'udta')
						user_data_box = (UserDataBox*)ptr_child;

					if (ptr_child->type == 'meta')
						meta_box = (MetaBox*)ptr_child;

					if (ptr_child->type == 'meco')
						additional_metadata_container_box = (AdditionalMetadataContainerBox*)ptr_child;

					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}

		};

		/*
		Box Type: 'moof'
		Container: File
		Mandatory: No
		Quantity: Zero or more
		*/
		struct MovieFragmentBox : public ContainerBox
		{
			/*
			Box Type: 'mfhd'
			Container: Movie Fragment Box ('moof')
			Mandatory: Yes
			Quantity: Exactly one
			*/
			struct MovieFragmentHeaderBox : public FullBox
			{
				uint32_t	sequence_number = 0;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					if (LeftBytes(bs) < sizeof(sequence_number))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					sequence_number = bs.GetDWord();

					SkipLeftBits(bs);
					return 0;
				}
			}PACKED;

			/*
			Box Type: 'traf'
			Container: Movie Fragment Box ('moof')
			Mandatory: No
			Quantity: Zero or more
			*/
			struct TrackFragmentBox : public ContainerBox
			{
				/*
				Box Type: 'tfhd'
				Container: Track Fragment Box ('traf')
				Mandatory: Yes
				Quantity: Exactly one
				*/
				struct TrackFragmentHeaderBox : public FullBox
				{
					uint32_t	track_ID = 0;
					uint64_t	base_data_offset = 0;
					uint32_t	sample_description_index = 0;
					uint32_t	default_sample_duration = 0;
					uint32_t	default_sample_size = 0;
					uint32_t	default_sample_flags = 0;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;
						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						auto cbLeftBytes = LeftBytes(bs);

						if (cbLeftBytes < 4)
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						track_ID = bs.GetDWord();
						cbLeftBytes -= 4;
					
						if (flags & 0x000001)
						{
							/*  0x000001 base-data-offset-present: indicates the presence of the base-data-offset field.This provides
									an explicit anchor for the data offsets in each track run(see below). If not provided, the base-data-offset
									for the first track in the movie fragment is the position of the first byte of the enclosing Movie
									Fragment Box, and for second and subsequent track fragments, the default is the end of the data
									defined by the preceding fragment. Fragments 'inheriting' their offset in this way must all use the same
									data - reference(i.e., the data for these tracks must be in the same file). */
							if (cbLeftBytes < 8)
								goto done;

							base_data_offset = bs.GetQWord();
							cbLeftBytes -= 8;
						}

						if (flags & 0x000002)
						{
							// 0x000002 sample-description-index-present: indicates the presence of this field, which over-rides, in this fragment, the default set up in the Track Extends Box.
							if (cbLeftBytes < 4)
								goto done;

							sample_description_index = bs.GetDWord();
							cbLeftBytes -= 4;
						}

						if (flags & 0x000008)
						{
							// 0x000008 default-sample-duration-present
							if (cbLeftBytes < 4)
								goto done;

							default_sample_duration = bs.GetDWord();
							cbLeftBytes -= 4;
						}

						if (flags & 0x000010)
						{
							// 0x000010 default-sample-size-present
							if (cbLeftBytes < 4)
								goto done;

							default_sample_size = bs.GetDWord();
							cbLeftBytes -= 4;
						}

						if (flags & 0x000020)
						{
							// 0x000020 default-sample-flags-present
							if (cbLeftBytes < 4)
								goto done;

							default_sample_flags = bs.GetDWord();
							cbLeftBytes -= 4;
						}

					done:

						SkipLeftBits(bs);
						return 0;
					}
				}PACKED;

				/*
				Box Type: 'trun'
				Container: Track Fragment Box ('traf')
				Mandatory: No
				Quantity: Zero or more
				*/
				struct TrackRunBox : public FullBox
				{
					struct Sample
					{
						uint32_t	sample_duration;
						uint32_t	sample_size;
						uint32_t	sample_flags;
						union
						{
							struct
							{
								uint32_t	sample_composition_time_offset;
							}v0;
							struct
							{
								int32_t		sample_composition_time_offset;
							}v1;
						};
					}PACKED;

					uint32_t	sample_count = 0;
					int32_t		data_offset = 0;
					uint32_t	first_sample_flags = 0;

					std::vector<Sample>	samples;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;
						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < 4)
						{
							iRet = RET_CODE_BOX_TOO_SMALL;
							goto done;
						}

						sample_count = bs.GetDWord();
						left_bytes -= 4;

						if (flags & 0x000001)
						{
							// 0x000001 data-offset-present.
							if (left_bytes < 4)
								goto done;

							data_offset = bs.GetLong();
							left_bytes -= 4;
						}

						if (flags & 0x000004)
						{
							/*
							0x000004 first-sample-flags-present; this over-rides the default flags for the first sample only. This
								makes it possible to record a group of frames where the first is a key and the rest are difference
								frames, without supplying explicit flags for every sample. If this flag and field are used, sample-flags
								shall not be present.
							*/
							if (left_bytes < 4)
								goto done;

							first_sample_flags = bs.GetDWord();
							left_bytes -= 4;
						}

						for (uint32_t i = 0; i < sample_count; i++)
						{
							Sample sample;

							if (flags & 0x000100)
							{
								if (left_bytes < 4)
									goto done;

								sample.sample_duration = bs.GetDWord();
								left_bytes -= 4;
							}

							if (flags & 0x000200)
							{
								if (left_bytes < 4)
									goto done;

								sample.sample_size = bs.GetDWord();
								left_bytes -= 4;
							}

							if (flags & 0x000400)
							{
								if (left_bytes < 4)
									goto done;

								sample.sample_flags = bs.GetDWord();
								left_bytes -= 4;
							}

							if (flags & 0x000800)
							{
								if (left_bytes < 4)
									goto done;

								if (version == 0)
									sample.v0.sample_composition_time_offset = bs.GetDWord();
								else
									sample.v1.sample_composition_time_offset = bs.GetLong();
								left_bytes -= 4;
							}

							samples.push_back(sample);
						}

					done:
						SkipLeftBits(bs);
						return iRet;
					}
				};

				/*
				Box Type: 'tfdt'
				Container: Track Fragment box ('traf')
				Mandatory: No
				Quantity: Zero or one
				*/
				struct TrackFragmentBaseMediaDecodeTimeBox : public FullBox
				{
					uint64_t	baseMediaDecodeTime = 0;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;
						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						uint64_t left_bytes = LeftBytes(bs);
						if (left_bytes < (version==1?sizeof(uint64_t):sizeof(uint32_t)))
						{
							SkipLeftBits(bs);
							return RET_CODE_BOX_TOO_SMALL;
						}

						baseMediaDecodeTime = version == 1 ? bs.GetQWord() : bs.GetDWord();

						SkipLeftBits(bs);
						return 0;
					}
				}PACKED;

				TrackFragmentHeaderBox*			track_fragment_header_box = nullptr;
				std::vector<TrackRunBox*>		track_run_boxes;
				TrackFragmentBaseMediaDecodeTimeBox*
												track_fragment_base_media_decode_time_box = nullptr;
				std::vector<SampleToGroupBox*>	sample_to_group_boxes;
				std::vector<SampleGroupDescriptionBox*>
												sample_group_description_boxes;
				SubSampleInformationBox*		subsample_information_box = nullptr;
				std::vector<SampleAuxiliaryInformationSizesBox*>
												sample_aux_informtion_sizes_boxes;
				std::vector<SampleAuxiliaryInformationOffsetsBox*>
												sample_aux_information_offsets_boxes;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					if ((iRet = ContainerBox::Unpack(bs)) < 0)
						return iRet;

					Box* ptr_child = first_child;
					while (ptr_child != nullptr)
					{
						switch (ptr_child->type)
						{
						case 'tfhd':
							track_fragment_header_box = (TrackFragmentHeaderBox*)ptr_child;
							break;
						case 'trun':
							track_run_boxes.push_back((TrackRunBox*)ptr_child);
							break;
						case 'tfdt':
							track_fragment_base_media_decode_time_box = (TrackFragmentBaseMediaDecodeTimeBox*)ptr_child;
							break;
						case 'sbgp':
							sample_to_group_boxes.push_back((SampleToGroupBox*)ptr_child);
							break;
						case 'sgpd':
							sample_group_description_boxes.push_back((SampleGroupDescriptionBox*)ptr_child);
							break;
						case 'subs':
							subsample_information_box = (SubSampleInformationBox*)ptr_child;
							break;
						case 'saiz':
							sample_aux_informtion_sizes_boxes.push_back((SampleAuxiliaryInformationSizesBox*)ptr_child);
							break;
						case 'saio':
							sample_aux_information_offsets_boxes.push_back((SampleAuxiliaryInformationOffsetsBox*)ptr_child);
							break;
						}

						ptr_child = ptr_child->next_sibling;
					}

					return iRet;
				}
			};

			MovieFragmentHeaderBox*			movie_fragment_header_box = nullptr;
			std::vector<TrackFragmentBox*>	track_fragment_boxes;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = ContainerBox::Unpack(bs)) < 0)
					return iRet;

				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					if (ptr_child->type == 'mfhd')
						movie_fragment_header_box = (MovieFragmentHeaderBox*)ptr_child;
					else if (ptr_child->type == 'traf')
						track_fragment_boxes.push_back((TrackFragmentBox*)ptr_child);

					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}
		};

		/*
		Box Type: 'mfra'
		Container: File
		Mandatory: No
		Quantity: Zero or one
		*/
		struct MovieFragmentRandomAccessBox : public ContainerBox
		{
			/*
			Box Type: 'tfra'
			Container: Movie Fragment Random Access Box ('mfra')
			Mandatory: No
			Quantity: Zero or one per track
			*/
			struct TrackFragmentRandomAccessBox : public FullBox
			{
				struct Entry
				{
					uint64_t	time;
					uint64_t	moof_offset;

					uint8_t		traf_number[4];
					uint8_t		trun_number[4];
					uint8_t		sample_number[4];
				}PACKED;

				uint32_t	track_ID = 0;
				uint32_t	reserved : 26;
				uint32_t	length_size_of_traf_num : 2;
				uint32_t	length_size_of_trun_num : 2;
				uint32_t	length_size_of_sample_num : 2;
				uint32_t	number_of_entry = 0;

				std::vector<Entry>	entries;

				TrackFragmentRandomAccessBox()
					: reserved(0), length_size_of_traf_num(0), length_size_of_trun_num(0), length_size_of_sample_num(0) {
				}

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < 12)
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					track_ID = bs.GetDWord();
					reserved = (uint32_t)bs.GetBits(26);
					length_size_of_traf_num = (uint32_t)bs.GetBits(2);
					length_size_of_trun_num = (uint32_t)bs.GetBits(2);
					length_size_of_sample_num = (uint32_t)bs.GetBits(2);
					number_of_entry = bs.GetDWord();
					left_bytes -= 12;

					for (uint32_t i = 0; i < number_of_entry; i++)
					{
						uint32_t entry_size = (version == 1 ? 16 : 8) + length_size_of_traf_num + 1
							+ length_size_of_trun_num + 1 + length_size_of_sample_num + 1;
						if (left_bytes < entry_size)
							break;

						Entry entry;
						entry.time = version == 1 ? bs.GetQWord() : bs.GetWord();
						entry.moof_offset = version == 1 ? bs.GetQWord() : bs.GetWord();

						for (uint32_t j = 0; j <= length_size_of_traf_num; j++)
							entry.traf_number[j] = bs.GetByte();

						for (uint32_t j = 0; j < length_size_of_trun_num; j++)
							entry.trun_number[j] = bs.GetByte();

						for (uint32_t j = 0; j < length_size_of_sample_num; j++)
							entry.sample_number[j] = bs.GetByte();

						entries.push_back(entry);

						left_bytes -= entry_size;
					}

					SkipLeftBits(bs);
					return 0;
				}
			};

			/*
			Box Type: 'mfro'
			Container: Movie Fragment Random Access Box ('mfra')
			Mandatory: Yes
			Quantity: Exactly one
			*/
			struct MovieFragmentRandomAccessOffsetBox : public FullBox
			{
				uint32_t	size = 0;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					if (LeftBytes(bs) < sizeof(size))
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					size = bs.GetDWord();

					SkipLeftBits(bs);
					return 0;
				}
			}PACKED;

			std::vector<TrackFragmentRandomAccessBox*>	track_fragment_random_access_boxes;
			MovieFragmentRandomAccessOffsetBox*			movie_fragment_random_access_offset_boxes = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = ContainerBox::Unpack(bs)) < 0)
					return iRet;

				Box* ptr_child = first_child;
				while (ptr_child != nullptr)
				{
					switch (ptr_child->type)
					{
					case 'tfra':
						track_fragment_random_access_boxes.push_back((TrackFragmentRandomAccessBox*)ptr_child);
						break;
					case 'mfro':
						movie_fragment_random_access_offset_boxes = (MovieFragmentRandomAccessOffsetBox*)ptr_child;
						break;
					}

					ptr_child = ptr_child->next_sibling;
				}

				return iRet;
			}
		};
	}
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED



