#pragma once

#include <stdint.h>
#include "Bitstream.h"
#include "combase.h"
#include <algorithm>

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

extern std::unordered_map<uint32_t, const char*> box_desces;
extern std::unordered_map<uint32_t, const char*> handle_type_names;

#define MIN_BOX_SIZE			8
#define MIN_FULLBOX_SIZE		12

namespace ISOMediaFile
{
	class IBox: public IUnknown
	{
	public:
		virtual int Unpack(CBitstream& bs) = 0;
		virtual int Pack(CBitstream& bs) = 0;
		virtual int Clean() = 0;
	};

	struct Box: public CComUnknown, public IBox
	{
		uint64_t	size = MIN_BOX_SIZE;
		uint32_t	type = 0;

		uint8_t		usertype[16] = { 0 };
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

		Box(){}

		Box(Box* pContainerBox): container(pContainerBox){
		}

		Box(uint64_t box_size, uint32_t box_type) : size(box_size), type(box_type) {
		}

		virtual ~Box() { Clean(); }

		virtual int Unpack(CBitstream& bs)
		{
			start_bitpos = bs.Tell();
			AMP_Assert(start_bitpos % 8 == 0);

			size = bs.GetDWord();
			type = bs.GetDWord();

			if (size == 1)
				size = bs.GetQWord();
			else if (size == 0)
			{
				// box extends to end of file
			}
			else if (size < MIN_BOX_SIZE)
			{
				// Ignore the current box
				return RET_CODE_ERROR;
			}

			if (type == 'uuid')
			{
				if (LeftBytes(bs) < sizeof(usertype))
				{
					// Ignore the current box
					return -1;
				}

				for (int i = 0; i < 16; i++)
					usertype[i] = bs.GetByte();
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

		static Box*	RootBox();
		static int	LoadBoxes(Box* pContainer, CBitstream& bs, Box** ppBox = nullptr);
		static void UnloadBoxes(Box* pBox);
	}PACKED;

	struct FullBox : public Box
	{
		uint32_t	version : 8;
		uint32_t	flags : 24;

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
			while (child_sum_size < left_box_size && LoadBoxes(this, bs, &ptr_box) >= 0)
			{
				printf("box-type: %c%c%c%c(0X%X), size: 0X%llX\r\n", 
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
			while (child_sum_size < left_box_size && LoadBoxes(this, bs, &ptr_box) >= 0)
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
		uint32_t				major_brand;
		uint32_t				minor_version;
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
	}PACKED;

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
	}PACKED;

	/*
	Box Type: 'hdlr'
	Container: Media Box ('mdia') or Meta Box ('meta')
	Mandatory: Yes
	Quantity: Exactly one
	*/
	struct HandlerBox : public FullBox
	{
		uint32_t		pre_defined;
		uint32_t		handler_type;
		uint32_t		reserved[3];
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
			for (int i = 0; i < _countof(reserved); i++)
				reserved[i] = bs.GetDWord();
			left_bytes -= unpack_size;

			ReadString(bs, name);

		done:
			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;

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
		}PACKED;

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
						ptr_box = &unknBox;	// It's unexpected

					if ((iRet = ptr_box->Unpack(bs)) < 0)
						break;

					left_bytes -= ptr_box->size;

					if (box_type == 'url ' || box_type == 'urn ')
						data_entries.push_back(ptr_box);
				}

			done:
				SkipLeftBits(bs);
				return 0;
			};

		}PACKED;

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

		uint32_t	grouping_type;
		struct
		{
			uint32_t	grouping_type_parameter;
		}PACKED v1;
		uint32_t			entry_count;
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

			if (left_bytes < sizeof(Entry)*entry_count)
				printf("The 'sbgp' box size is too small {size: %llu, entry_count: %lu}.\n", size, entry_count);

			while (left_bytes >= sizeof(Entry) && entries.size() < entry_count)
			{
				entries.emplace_back(bs);
				left_bytes -= sizeof(Entry);
			}

		done:
			SkipLeftBits(bs);
			return 0;
		};
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
		}PACKED;

		uint32_t			grouping_type;
		uint32_t			default_length;
		uint32_t			entry_count;
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
			left_bytes -= sizeof(uint32_t);

			if (version == 1)
			{
				if (left_bytes < sizeof(default_length))
				{
					iRet = RET_CODE_BOX_TOO_SMALL;
					goto done;
				}

				default_length = bs.GetDWord();
				left_bytes -= sizeof(uint32_t);
			}

			if (left_bytes < sizeof(entry_count))
			{
				iRet = RET_CODE_BOX_TOO_SMALL;
				goto done;
			}

			entry_count = bs.GetDWord();
			left_bytes -= sizeof(uint32_t);

			for (uint32_t i = 0; i < entry_count; i++)
			{
				Entry entry;
				if (version == 1 && default_length == 0)
				{
					if (left_bytes < sizeof(entry_count))
						break;

					entry.description_length = bs.GetDWord();
					left_bytes -= sizeof(uint32_t);
				}
				else
					entry.description_length = default_length;

				if (left_bytes < entry.description_length)
					break;

				bs.SkipBits((uint64_t)entry.description_length << 3);
				left_bytes -= (uint64_t)entry.description_length << 3;
			}

		done:
			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;

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
				uint32_t	subsample_size;
				uint8_t		subsample_priority;
				uint8_t		discardable;
				uint32_t	reserved;
			}PACKED;

			uint32_t	sample_delta;
			uint32_t	subsample_count;

			std::vector<SubSample>	subsamples;
		}PACKED;

		uint32_t			entry_count;
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
						if (left_bytes < (version == 1 ? sizeof(uint32_t) : sizeof(uint16_t)) + 6)
							break;

						Entry::SubSample subsample;
						subsample.subsample_size = version==1?bs.GetDWord():bs.GetWord();
						subsample.subsample_priority = bs.GetByte();
						subsample.discardable = bs.GetByte();
						subsample.reserved = bs.GetDWord();

						left_bytes -= (version == 1 ? sizeof(uint32_t) : sizeof(uint16_t)) + 6;
					}
					if (j < entry.subsample_count)
						break;
				}
			}

			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;

	/*
	Box Type: 'saiz'
	Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
	Mandatory: No
	Quantity: Zero or More
	*/
	struct SampleAuxiliaryInformationSizesBox : public FullBox
	{
		uint32_t				aux_info_type;
		uint32_t				aux_info_type_parameter;

		uint8_t					default_sample_info_size;
		uint32_t				sample_count;

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
	}PACKED;

	/*
	Box Type: 'saio'
	Container: Sample Table Box ('stbl') or Track Fragment Box ('traf')
	Mandatory: No
	Quantity: Zero or More
	*/
	struct SampleAuxiliaryInformationOffsetsBox : public FullBox
	{
		uint32_t				aux_info_type;
		uint32_t				aux_info_type_parameter;

		uint32_t				entry_count;
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
	}PACKED;

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
		}PACKED;

		/*
		Box Type: 'tsel'
		Container: User Data Box ('udta')
		Mandatory: No
		Quantity: Zero or One
		*/
		struct TrackSelectionBox : public FullBox
		{
			uint32_t				switch_group;
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
		}PACKED;

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
				int16_t					switch_group;
				int16_t					alternate_group;
				uint32_t				sub_track_ID;
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
			}PACKED;

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
				}PACKED;

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
	}PACKED;

	/*
	Box Types: 'frma'
	Container: Protection Scheme Information Box ('sinf') or Restricted Scheme Information Box ('rinf')
	Mandatory: Yes when used in a protected sample entry or in a restricted sample entry
	Quantity: Exactly one
	*/
	struct OriginalFormatBox : public Box
	{
		uint32_t	data_format;

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
	Container: Protection Scheme Information Box (‘sinf?, Restricted Scheme Information Box ('rinf'),
	or SRTP Process box ('srpp')
	Mandatory: No
	Quantity: Zero or one in 'sinf', depending on the protection structure; Exactly one in 'rinf' and 'srpp'
	*/
	struct SchemeTypeBox : public FullBox
	{
		uint32_t		scheme_type;	// 4CC identifying the scheme
		uint32_t		scheme_version;	// scheme version
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
	}PACKED;

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
				if (LoadBoxes(this, bs, &ptr_box) < 0)
					break;

				scheme_specific_data.push_back(ptr_box);
			}

			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;

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
					LoadBoxes(this, bs, (Box**)&info);
			}

			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;

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
					LoadBoxes(this, bs, (Box**)&scheme_type_box);
			}

			if (LeftBytes(bs) >= MIN_BOX_SIZE)
			{
				if ((bs.PeekBits(64)&UINT32_MAX) == 'schi')
					LoadBoxes(this, bs, (Box**)&info);
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
		}PACKED;

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
		}PACKED;

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
				uint16_t				item_ID;
				uint16_t				reserved : 12;
				uint16_t				construction_method : 4;
				uint16_t				data_reference_index;
				std::vector<uint8_t>	base_offset;
				uint16_t				extent_count;
				std::vector<uint8_t>	extent_index;
				std::vector<uint8_t>	extent_offset;
				std::vector<uint8_t>	extent_length;
			}PACKED;

			uint8_t		offset_size : 4;
			uint8_t		length_size : 4;
			uint8_t		base_offset_size : 4;
			uint8_t		index_size : 4;

			uint16_t	item_count;
			std::vector<Item>	items;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				uint64_t left_bytes = LeftBytes(bs);
				if (left_bytes < 4)
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				offset_size = (uint8_t)bs.GetBits(4);
				length_size = (uint8_t)bs.GetBits(4);
				base_offset_size = (uint8_t)bs.GetBits(4);
				offset_size = (uint8_t)bs.GetBits(4);
				item_count = bs.GetWord();
				left_bytes -= 4;

				items.reserve(item_count);

				int item_fix_size = sizeof(Item::item_ID) + sizeof(Item::data_reference_index) + base_offset_size + sizeof(Item::extent_count);
				if (version == 1)
					item_fix_size += sizeof(uint16_t);

				for (uint32_t i = 0; i < item_count; i++)
				{
					if (left_bytes < item_fix_size)
						break;

					items.emplace_back();
					auto curitem = items.back();
					curitem.item_ID = bs.GetWord();
					if (version == 1)
					{
						curitem.reserved = (uint16_t)bs.GetBits(12);
						curitem.construction_method = (uint16_t)bs.GetBits(4);
					}

					curitem.data_reference_index = bs.GetWord();
					for (uint16_t j = 0; j < base_offset_size; j++)
						curitem.base_offset.push_back(bs.GetByte());

					curitem.extent_count = bs.GetWord();
					left_bytes -= item_fix_size;

					uint16_t j = 0;
					for (; j < curitem.extent_count; j++)
					{
						uint16_t extent_size = (version == 1 && index_size > 0) ? (index_size + offset_size + length_size) : (offset_size + length_size);
						if (left_bytes < extent_size)
							break;

						for (uint8_t k = 0; k < index_size; k++)
							curitem.extent_index.push_back(bs.GetByte());

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
		}PACKED;

		/*
		Box Type: 'pitm'
		Container: Meta box ('meta')
		Mandatory: No
		Quantity: Zero or one
		*/
		struct PrimaryItemBox : public FullBox
		{
			uint16_t	item_ID;

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
			uint16_t		protection_count; 
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
					if (LoadBoxes(this, bs, (Box**)&ptr_box) < 0)
						break;

					protection_informations.push_back(ptr_box);
				}

				SkipLeftBits(bs);
				return 0;
			}

		}PACKED;

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
				uint64_t				content_length;
				uint64_t				transfer_length;
				uint8_t					entry_count;
				std::vector<uint32_t>	group_ids;
			}PACKED;

			struct ItemInfoEntryv0 : public FullBox
			{
				uint16_t				item_ID;
				uint16_t				item_protection_index;
				std::string				item_name;
				std::string				content_type;
				std::string				content_encoding;	// optional
			}PACKED;

			struct ItemInfoEntryv1 : public ItemInfoEntryv0
			{
				uint32_t				extension_type;
				ItemInfoExtension*		item_info_extension;

				ItemInfoEntryv1(): extension_type(0), item_info_extension(nullptr){}
				virtual ~ItemInfoEntryv1() {
					AMP_SAFEDEL(item_info_extension);
				}
			}PACKED;

			struct ItemInfoEntryv2 : public FullBox
			{
				uint16_t				item_ID;
				uint16_t				item_protection_index;
				uint32_t				item_type;

				std::string				item_name;

				// item_type == 'mime'
				std::string				content_type;
				std::string				content_encoding;	// optional

				// item_type == 'uri '
				std::string				item_uri_type;
			}PACKED;

			uint16_t		entry_count;
			uint16_t		last_entry_idx;
			union
			{
				ItemInfoEntryv0*	item_info_entries_v0;
				ItemInfoEntryv1*	item_info_entries_v1;
				ItemInfoEntryv2*	item_info_entries_v2;
			}PACKED;

			virtual ~ItemInfoBox()
			{
				if (version == 0) {
					AMP_SAFEDELA(item_info_entries_v0);
				}
				else if (version == 1) {
					AMP_SAFEDELA(item_info_entries_v1);
				}
				else if (version == 2) {
					AMP_SAFEDELA(item_info_entries_v2);
				}
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
				left_bytes -= sizeof(entry_count);

				if (version == 0)
					item_info_entries_v0 = new ItemInfoEntryv0[entry_count];
				else if (version == 1)
					item_info_entries_v1 = new ItemInfoEntryv1[entry_count];
				else if (version == 2)
					item_info_entries_v2 = new ItemInfoEntryv2[entry_count];

				last_entry_idx = 0;
				for (uint16_t i = 0; i < entry_count; i++)
				{
					if (version == 0 || version == 1)
					{
						if (left_bytes < sizeof(uint16_t) + sizeof(uint16_t) + 3/* 3 null terminator */)
						{
							printf("Not enough to fill one ItemInfoEntry v0/v1, last_entry_idx: %d, left_bytes: %llu.\r\n", last_entry_idx, left_bytes);
							goto done;
						}

						ItemInfoEntryv0* entry = version == 0 
							? &item_info_entries_v0[last_entry_idx] 
							: (ItemInfoEntryv0*)(&item_info_entries_v1[last_entry_idx]);

						entry->item_ID = bs.GetWord();
						entry->item_protection_index = bs.GetWord();
						ReadString(bs, entry->item_name);
						ReadString(bs, entry->content_type);
						ReadString(bs, entry->content_encoding);

						left_bytes -= sizeof(entry->item_ID) + sizeof(entry->item_protection_index) + 
							entry->item_name.length() + entry->content_type.length() + entry->content_encoding.length();
					}

					if (version == 1)
					{
						if (left_bytes < sizeof(uint32_t))
						{
							printf("Not enough to fill one ItemInfoEntry v1, last_entry_idx: %d, left_bytes: %llu.\r\n", last_entry_idx, left_bytes);
							goto done;
						}

						item_info_entries_v1[last_entry_idx].extension_type = bs.GetDWord();
						left_bytes -= sizeof(uint32_t);

						if (item_info_entries_v1[last_entry_idx].extension_type == 'fdel')
						{
							if (left_bytes < 2/*2 null terminator*/)
							{
								printf("Not enough to fill FDItemInfoExtension of one ItemInfoEntry v1, last_entry_idx: %d, left_bytes: %llu.\r\n", last_entry_idx, left_bytes);
								goto done;
							}

							FDItemInfoExtension* fd_item_info_extension = new FDItemInfoExtension();
							item_info_entries_v1[last_entry_idx].item_info_extension = fd_item_info_extension;

							ReadString(bs, fd_item_info_extension->content_location);
							ReadString(bs, fd_item_info_extension->content_MD5);

							left_bytes -= fd_item_info_extension->content_location.length();
							left_bytes -= fd_item_info_extension->content_MD5.length();

							if (left_bytes < sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint8_t))
							{
								printf("Not enough to fill FDItemInfoExtension of one ItemInfoEntry v1, last_entry_idx: %d, left_bytes: %llu.\r\n", last_entry_idx, left_bytes);
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
							printf("Unsupported extension_type: 0X%08X.\r\n", item_info_entries_v1[last_entry_idx].extension_type);
							goto done;
						}
					}

					if (version == 2)
					{
						if (left_bytes < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + 1/*item_name null terminator*/)
						{
							printf("Not enough to fill one ItemInfoEntry v2, last_entry_idx: %d, left_bytes: %llu.\r\n", last_entry_idx, left_bytes);
							goto done;
						}

						item_info_entries_v2[last_entry_idx].item_ID = bs.GetWord();
						item_info_entries_v2[last_entry_idx].item_protection_index = bs.GetWord();
						item_info_entries_v2[last_entry_idx].item_type = bs.GetDWord();

						ReadString(bs, item_info_entries_v2[last_entry_idx].item_name);

						left_bytes -= sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint32_t) + item_info_entries_v2[last_entry_idx].item_name.length();

						if (item_info_entries_v2[last_entry_idx].item_type == 'mime')
						{
							ReadString(bs, item_info_entries_v2[last_entry_idx].content_type);
							ReadString(bs, item_info_entries_v2[last_entry_idx].content_encoding);

							left_bytes -= item_info_entries_v2[last_entry_idx].content_type.length() +
								item_info_entries_v2[last_entry_idx].content_encoding.length();
						}
						else if (item_info_entries_v2[last_entry_idx].item_type == 'uri ')
						{
							ReadString(bs, item_info_entries_v2[last_entry_idx].item_uri_type);

							left_bytes -= item_info_entries_v2[last_entry_idx].item_uri_type.length();
						}
					}

					last_entry_idx++;
				}

			done:
				SkipLeftBits(bs);
				return 0;
			}

		}PACKED;

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
					uint16_t		item_ID;
					uint16_t		packet_payload_size;
					uint8_t			reserved = 0;
					uint8_t			FEC_encoding_ID;
					uint16_t		FEC_instance_ID;
					uint16_t		max_source_block_length;
					uint16_t		encoding_symbol_length;
					uint16_t		max_number_of_encoding_symbols;
					std::string		scheme_specific_info;
					uint16_t		entry_count;
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

				}PACKED;

				/*
				Box Type: 'fecr'
				Container: Partition Entry ('paen')
				Mandatory: No
				Quantity: Zero or One
				*/
				struct FECReservoirBox : public FullBox
				{
					uint16_t		entry_count;
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
				}PACKED;

				/*
				Box Type: 'fire'
				Container: Partition Entry ('paen')
				Mandatory: No
				Quantity: Zero or One
				*/
				struct FileReservoirBox : public FullBox
				{
					uint32_t		entry_count;
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

				}PACKED;

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

			}PACKED; // PartionEntry

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
					uint8_t					entry_count;
					std::vector<uint32_t>	group_IDs;
					uint16_t				num_channels_in_session_group;
					std::vector<uint32_t>	hint_track_ids;
				}PACKED;

				uint16_t					num_session_groups;
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

						if (left_bytes < entry_count * sizeof(uint32_t))
							break;

						session_groups.emplace_back();
						auto back = session_groups.back();
						for (uint8_t i = 0; i < entry_count; i++)
							back.group_IDs.push_back(bs.GetDWord());

						left_bytes -= entry_count * sizeof(uint32_t);

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

			}PACKED; // FDSessionGroupBox

			/*
			Box Type: 'gitn'
			Container: FD Information Box ('fiin')
			Mandatory: No
			Quantity: Zero or One
			*/
			struct GroupIdToNameBox : public FullBox
			{
				uint16_t		entry_count;
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

			}PACKED; // GroupIdToNameBox

			uint16_t						entry_count;
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
					auto pPE = partition_entries.back() = new PartitionEntry();

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

		}PACKED; // FDItemInformationBox

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
		}PACKED;

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
				uint16_t				from_item_ID;
				uint16_t				reference_count;
				std::vector<uint16_t>	to_item_IDs;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

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

					while (left_bytes >= sizeof(uint16_t) && reference_count < to_item_IDs.size())
					{
						to_item_IDs.push_back(bs.GetWord());
						left_bytes -= sizeof(uint16_t);
					}

				done:
					SkipLeftBits(bs);
					return iRet;
				}
			}PACKED;

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
				}

				SkipLeftBits(bs);
				return iRet;
			}
		}PACKED; // ItemReferenceBox

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
			uint32_t	first_metabox_handler_type;
			uint32_t	second_metabox_handler_type;
			uint8_t		metabox_relation;

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

	}PACKED;

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
			uint32_t	starts_with_SAPL : 1;
			uint32_t	SAP_type : 3;
			uint32_t	SAP_delta_time : 28;
		}PACKED;

		uint32_t				reference_ID;
		uint32_t				timescale;

		uint64_t				earliest_presentation_time;
		uint64_t				first_offset;

		uint16_t				reserved = 0;
		uint16_t				reference_count;
		std::vector<Reference>	references;

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = 0;

			if ((iRet = Box::Unpack(bs)) < 0)
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
				auto back = references.back();
				back.reference_type = (uint32_t)bs.GetBits(1);
				back.referenced_size = (uint32_t)bs.GetBits(31);
				back.subsegment_duration = bs.GetDWord();
				back.starts_with_SAPL = (uint32_t)bs.GetBits(1);
				back.SAP_type = (uint32_t)bs.GetBits(3);
				back.SAP_delta_time = (uint32_t)bs.GetBits(28);
				left_bytes -= sizeof(Reference);
			}

		done:
			SkipLeftBits(bs);
			return iRet;
		}
	}PACKED;

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

			uint32_t			ranges_count;
			std::vector<Range>	ranges;
		}PACKED;

		uint32_t				segment_count;
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
				auto back = segments.back();

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
		uint32_t		reference_track_ID;
		uint64_t		ntp_timestamp;
		uint64_t		media_time;

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
					uint64_t	create_time;
					uint64_t	modification_time;
					uint32_t	timescale;
					uint64_t	duration;
				}PACKED v1;
				struct {
					uint32_t	create_time;
					uint32_t	modification_time;
					uint32_t	timescale;
					uint32_t	duration;
				}PACKED v0;
			}PACKED;

			int32_t		rate;
			int16_t		volume;
			uint16_t	reserved_0;
			uint32_t	reserved_1[2];
			int32_t		matrix[9];
			uint32_t	pre_defined[6];
			uint32_t	next_track_ID;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = FullBox::Unpack(bs)) < 0)
					return iRet;

				if (version == 1)
				{
					v1.create_time = bs.GetQWord();
					v1.modification_time = bs.GetQWord();
					v1.timescale = bs.GetDWord();
					v1.duration = bs.GetQWord();
				}
				else
				{
					v0.create_time = bs.GetDWord();
					v0.modification_time = bs.GetDWord();
					v0.timescale = bs.GetDWord();
					v0.duration = bs.GetDWord();
				}

				rate = bs.GetLong();
				volume = bs.GetShort();
				reserved_0 = bs.GetWord();
				reserved_1[0] = bs.GetDWord();
				reserved_1[1] = bs.GetDWord();

				for (int i = 0; i < _countof(matrix); i++)
					matrix[i] = bs.GetLong();

				for (int i = 0; i < _countof(pre_defined); i++)
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

				uint32_t	reserved_0[2];
				int16_t		layer;
				int16_t		alternate_group;
				int16_t		volume;
				uint16_t	reserved_1;
				int32_t		matrix[9];
				int32_t		width;
				int32_t		height;
			
				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					uint64_t start_bitpos = bs.Tell();
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
					
					for (int i = 0; i < _countof(matrix); i++)
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
						if (left_bytes > sizeof(uint32_t))
						{
							for (uint64_t i = 0; i < left_bytes / sizeof(uint32_t); i++)
							{
								track_IDs.push_back(bs.GetDWord());
							}
						}

						SkipLeftBits(bs);

						return 0;
					}
				}PACKED;

				std::vector<TrackReferenceTypeBox> TypeBoxes;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					uint64_t start_bitpos = bs.Tell();
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
			}PACKED;

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
					uint32_t	track_group_id;

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

					uint64_t start_bitpos = bs.Tell();
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

			}PACKED;

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

					uint32_t			entry_count;
					std::vector<Entry>	entries;

					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;

						uint64_t start_bitpos = bs.Tell();
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
							int actual_entry_size = (version == 1 ? 16 : 8) + 4;
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
				}PACKED;

				EditListBox* edit_list_box;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;

					if ((iRet = Box::Unpack(bs)) < 0)
						return iRet;

					LoadBoxes(this, bs, (Box**)&edit_list_box);

					SkipLeftBits(bs);
					return 0;
				}

			}PACKED;

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
					
					virtual int Unpack(CBitstream& bs)
					{
						int iRet = 0;
						if ((iRet = FullBox::Unpack(bs)) < 0)
							return iRet;

						auto left_bytes = LeftBytes(bs);
						if (version == 1 && left_bytes < sizeof(v1) + 4 ||
							version != 1 && left_bytes < sizeof(v0) + 4)
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
						uint16_t	graphicsmode;
						uint16_t	opcolor[3];

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
						uint16_t	maxPDUsize;
						uint16_t	avgPDUsize;
						uint32_t	maxbitrate;
						uint32_t	avgbitrate;
						uint32_t	reserved;

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
						}PACKED;

						struct BitRateBox : public Box
						{
							uint32_t	bufferSizeDB;
							uint32_t	maxBitrate;
							uint32_t	avgBitrate;

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
						}PACKED;

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
						}PACKED;

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
						}PACKED;

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
							URIInitBox*			ptr_init;				// optional
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
						}PACKED;

						// Visual Sequences
						struct PixelAspectRatioBox : public Box
						{
							uint32_t	hSpacing;
							uint32_t	vSpacing;

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
							uint32_t	cleanApertureWidthN;
							uint32_t	cleanApertureWidthD;
							uint32_t	cleanApertureHeightN;
							uint32_t	cleanApertureHeightD;
							uint32_t	horizOffN;
							uint32_t	horizOffD;
							uint32_t	vertOffN;
							uint32_t	vertOffD;

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
						}PACKED;

						struct ColourInformationBox : public Box
						{
							uint32_t	colour_type;

							// On-Screen colours
							uint16_t	colour_primaries;
							uint16_t	transfer_characteristics;
							uint16_t	matrix_coefficients;
							uint8_t		full_range_flag:1;
							uint8_t		reserved:7;
							
							// restricted ICC profile

							// unrestricted ICC profile

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
								if (colour_type == 'nclx')
								{
									if (LeftBytes(bs) < 7)
									{
										SkipLeftBits(bs);
										return RET_CODE_BOX_TOO_SMALL;
									}

									colour_primaries = bs.GetWord();
									transfer_characteristics = bs.GetWord();
									matrix_coefficients = bs.GetWord();
									full_range_flag = (uint8_t)bs.GetBits(1);
									reserved = (uint8_t)bs.GetBits(7);
								}
								else if (colour_type == 'rICC')
								{
									// TODO...
								}
								else if (colour_type == 'prof')
								{
									// TODO...
								}

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						struct VisualSampleEntry : public SampleEntry
						{
							uint16_t				pre_defined_0;
							uint16_t				reserved_0;
							uint32_t				pre_defined_1[3];
							uint16_t				width;
							uint16_t				height;
							uint32_t				horizresolution;
							uint32_t				vertresolution;
							uint32_t				reserved_1;
							uint16_t				frame_count;
							uint8_t					compressorname_size;
							uint8_t					compressorname[31];
							uint16_t				depth = 0x0018;	
							int16_t					pre_defined_2 = -1;
							// other boxes from derived specifications
							CleanApertureBox*		ptr_clap = nullptr;	// optional
							PixelAspectRatioBox*	ptr_pasp = nullptr;	// optional

							virtual ~VisualSampleEntry()
							{
								if (ptr_clap != nullptr)
									delete ptr_clap;

								if (ptr_pasp != nullptr)
									delete ptr_pasp;
							}

							int _Unpack(CBitstream& bs)
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

								left_bytes = LeftBytes(bs);
								try
								{
									while (left_bytes >= MIN_BOX_SIZE)
									{
										uint64_t box_header = bs.PeekBits(64);
										uint32_t box_type = (box_header&UINT32_MAX);
										if (box_type == 'clap')
										{
											if (ptr_clap == nullptr)
												ptr_clap = new CleanApertureBox();
											if (ptr_clap->Unpack(bs) < 0)
												break;
										}
										else if (box_type == 'pasp')
										{
											if (ptr_pasp == nullptr)
												ptr_pasp = new PixelAspectRatioBox();
											if (ptr_pasp->Unpack(bs) < 0)
												break;
										}
										else
											break;

										left_bytes = LeftBytes(bs);
									}
								}
								catch (std::exception& e)
								{
									// continue since these 2 fields are optional
								}

								return 0;
							}

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = _Unpack(bs);
								SkipLeftBits(bs);
								return iRet;
							}
						}PACKED;

						// Audio Sequences
						struct AudioSampleEntry : public SampleEntry
						{
							uint32_t				reserved_0[2];
							uint16_t				channelcount = 2;
							uint16_t				samplesize = 16;
							uint16_t				pre_defined = 0;
							uint16_t				reserved_1 = 0;
							uint32_t				samplerate;

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = SampleEntry::Unpack(bs)) < 0)
									return iRet;

								if (LeftBytes(bs) < (uint64_t)(&samplerate - &reserved_0[0]) + sizeof(samplerate))
								{
									SkipLeftBits(bs);
									return RET_CODE_BOX_TOO_SMALL;
								}

								reserved_0[0] = bs.GetDWord();
								reserved_0[1] = bs.GetDWord();
								channelcount = bs.GetWord();
								samplerate = bs.GetWord();
								pre_defined = bs.GetWord();
								reserved_1 = bs.GetWord();
								samplerate = bs.GetDWord();

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						/*
						Box Types: 'stsd'
						Container: Sample Table Box ('stbl')
						Mandatory: Yes
						Quantity: Exactly one
						*/
						struct SampleDescriptionBox : public FullBox
						{
							uint32_t	handler_type;
							uint32_t	entry_count;

							std::vector<Box*>	SampleEntries;

							virtual ~SampleDescriptionBox()
							{
								for (size_t i = 0; i < SampleEntries.size(); i++)
								{
									if (SampleEntries[i] != nullptr)
										delete SampleEntries[i];
								}
							}

							virtual int Unpack(CBitstream& bs);
						}PACKED;

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

							uint32_t			entry_count;
							std::vector<Entry>	entries;

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
						}PACKED;

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

							uint32_t			entry_count;
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

									left_bytes -= sizeof(Entry);
								}

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						/*
						Box Type: 'cslg'
						Container: Sample Table Box ('stbl')
						Mandatory: No
						Quantity: Zero or one
						*/
						struct CompositionToDecodeBox : public FullBox
						{
							int32_t		compositionToDTSShift;
							int32_t		leastDecodeToDisplayDelta;
							int32_t		greatestDecodeToDisplayDelta;
							int32_t		compositionStartTime;
							int32_t		compositionEndTime;

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
							uint32_t				entry_count;
							std::vector<uint32_t>	sample_numbers;

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
						}PACKED;

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

							uint32_t			entry_count;
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
						}PACKED;

						/*
						Box Type: 'stsz', 'stz2'
						Container: Sample Table Box ('stbl')
						Mandatory: Yes
						Quantity: Exactly one variant must be present
						*/
						struct SampleSizeBox : public FullBox
						{
							uint32_t				sample_size;
							uint32_t				sample_count;

							std::vector<uint32_t>	entry_size;

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

								if (LeftBytes(bs) < sizeof(sample_size) + sizeof(sample_count))
								{
									SkipLeftBits(bs);
									return RET_CODE_BOX_TOO_SMALL;
								}

								sample_size = bs.GetDWord();
								sample_count = bs.GetDWord();

								if (sample_size == 0)
								{
									if (LeftBytes(bs) < sample_count * sizeof(uint32_t))
									{
										for (uint32_t i = 0; i < sample_count; i++)
											entry_size.push_back(bs.GetDWord());
									}
								}

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						struct CompactSampleSizeBox : public FullBox
						{
							uint32_t				reserved : 24;
							uint32_t				field_size : 8;
							uint32_t				sample_count;

							std::vector<uint16_t>	entry_size;

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
								if (LeftBytes(bs) < sample_count * field_size)
								{
									for (uint32_t i = 0; i < sample_count; i++)
										entry_size.push_back((uint16_t)bs.GetBits(field_size));
								}

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

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

							uint32_t				entry_count;
							std::vector<EntryInfo>	entry_infos;

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
						}PACKED;

						/*
						Box Type: 'stco', 'co64'
						Container: Sample Table Box ('stbl')
						Mandatory: Yes
						Quantity: Exactly one variant must be present
						*/
						struct ChunkOffsetBox : public FullBox
						{
							uint32_t				entry_count;
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

								if (left_bytes < sizeof(uint32_t)*entry_count)
									printf("The 'stco' box size is too small {size: %llu, entry_count: %lu}.\n", size, entry_count);

								uint32_t actual_entry_count = left_bytes / sizeof(uint32_t) < (uint64_t)entry_count ? (uint32_t)(left_bytes / sizeof(uint32_t)) : entry_count;
								for (uint32_t i = 0; i < actual_entry_count; i++)
									chunk_offset.push_back(bs.GetDWord());

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

						struct ChunkLargeOffsetBox : public FullBox
						{
							uint32_t				entry_count;
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

								if (left_bytes < sizeof(uint32_t)*entry_count)
									printf("The 'stco' box size is too small {size: %llu, entry_count: %lu}.\n", size, entry_count);

								uint32_t actual_entry_count = left_bytes / sizeof(uint32_t) < (uint64_t)entry_count ? (uint32_t)(left_bytes / sizeof(uint32_t)) : entry_count;
								for (uint32_t i = 0; i < actual_entry_count; i++)
									chunk_offset.push_back(bs.GetQWord());

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

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

							uint32_t					sample_count;
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
									printf("The 'padb' box size is too small {size: %llu, sample_count: %lu}.\n", size, sample_count);

								for (uint32_t i = 0; i < AMP_MIN(left_bytes, padding_byte_count); i++)
								{
									pads.emplace_back();
									auto pad_entry = pads.back();
									pad_entry.reserved_1 = (uint8_t)bs.GetBits(1);
									pad_entry.pad1 = (uint8_t)bs.GetBits(3);
									pad_entry.reserved_2 = (uint8_t)bs.GetBits(1);
									pad_entry.pad2 = (uint8_t)bs.GetBits(3);
								}

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

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

								Box* pChild = container->first_child;
								while (pChild != nullptr && pChild->type != 'stsz' && pChild->type != 'stz2')
									pChild = pChild->next_sibling;

								if (pChild == nullptr)
								{
									printf("Can't find 'stsz' or 'stz2' box to get sample count.\n");
									goto done;
								}

								uint32_t sample_count = 0;
								if (pChild->type == 'stsz')
									sample_count = ((SampleSizeBox*)pChild)->sample_count;
								else if (pChild->type == 'stz2')
									sample_count = ((CompactSampleSizeBox*)pChild)->sample_count;

								uint64_t left_bytes = LeftBytes(bs);
								for (uint32_t i = 0; i < (uint32_t)AMP_MIN((uint64_t)sample_count, left_bytes >> 1); i++)
									priorities.push_back(bs.GetWord());

							done:
								SkipLeftBits(bs);
								return 0;
							}

						}PACKED;

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

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

								// Find 'stsz' or 'stz2', and then get sample count
								if (container == nullptr)
								{
									printf("No container box for the current 'sdtp' box.\n");
									goto done;
								}

								if (container->type != 'stbl')
								{
									printf("The 'sdtp' box container box is not in 'stbl' box.\n");
									goto done;
								}

								Box* pChild = container->first_child;
								while (pChild != nullptr && pChild->type != 'stsz' && pChild->type != 'stz2')
									pChild = pChild->next_sibling;

								if (pChild == nullptr)
								{
									printf("Can't find 'stsz' or 'stz2' box to get sample count.\n");
									goto done;
								}

								uint32_t sample_count = 0;
								if (pChild->type == 'stsz')
									sample_count = ((SampleSizeBox*)pChild)->sample_count;
								else if (pChild->type == 'stz2')
									sample_count = ((CompactSampleSizeBox*)pChild)->sample_count;

								uint64_t left_bytes = LeftBytes(bs);
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
						}PACKED;

						SampleDescriptionBox*		sample_description_box;
						TimeToSampleBox*			time_to_sample_box;
						CompositionOffsetBox*		composition_offset_box;
						CompositionToDecodeBox*		composition_to_decode_box;
						SampleToChunkBox*			sample_to_chunk_box;
						SampleSizeBox*				sample_size_box;
						CompactSampleSizeBox*		compact_sample_size_box;
						ChunkOffsetBox*				chunk_offset_box;
						ChunkLargeOffsetBox*		chunk_large_offset_box;
						SyncSampleBox*				sync_sample_box;
						ShadowSyncSampleBox*		shadow_sync_sample_box;
						PaddingBitsBox*				padding_bits_box;
						DegradationPriorityBox*		degradation_priority_box;
						SampleDependencyTypeBox*	sample_dependency_type_box;
						std::vector<SampleToGroupBox*>
													sample_to_group_boxes;
						std::vector<SampleGroupDescriptionBox*>
													sample_group_description_boxes;
						SubSampleInformationBox*	subsample_information_box;
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

					}PACKED;

					VideoMediaHeaderBox*		video_media_header_box;
					SoundMediaHeaderBox*		sound_media_header_box;
					HintMediaHeaderBox*			hint_media_header_box;
					NullMediaHeaderBox*			null_media_header_box;
					DataInformationBox*			data_information_box;
					SampleTableBox*				sample_table_box;

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

				MediaHeaderBox*		media_header_box;
				HandlerBox*			handler_box;
				MediaInformationBox*
									media_information_box;

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

			TrackHeaderBox*			track_header_box;
			TrackReferenceBox*		track_reference_box;
			TrackGroupBox*			track_group_box;
			EditBox*				edit_box;
			MediaBox*				media_box;
			UserDataBox*			user_data_box;

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
				uint64_t		fragment_duration;

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
				uint32_t	track_ID;
				uint32_t	default_sample_description_index;
				uint32_t	default_sample_duration;
				uint32_t	default_sample_size;
				uint32_t	default_sample_flags;

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

				uint32_t	level_count;

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

						if (left_bytes < level.assignment_type == 1 ? 8 : 4)
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
					}

					SkipLeftBits(bs);
					return 0;
				}

			}PACKED;

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
		}PACKED;

		MovieHeaderBox*			movie_header_box = nullptr;					// Movie header
		std::vector<TrackBox*>	track_boxes;								// Track boxes
		MovieExtendsBox*		movie_extends_box = nullptr;				// Movie extends Box
		UserDataBox*			user_data_box = nullptr;					// User data box
		MetaBox*				meta_box = nullptr;							// Meta box
		AdditionalMetadataContainerBox*
								additional_metadata_container_box = nullptr;// Additional metadata container box

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

	}PACKED;

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
			uint32_t	sequence_number;

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
				uint32_t	track_ID;
				uint64_t	base_data_offset;
				uint32_t	sample_description_index;
				uint32_t	default_sample_duration;
				uint32_t	default_sample_size;
				uint32_t	default_sample_flags;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					if (LeftBytes(bs) < 28)
					{
						SkipLeftBits(bs);
						return RET_CODE_BOX_TOO_SMALL;
					}

					track_ID = bs.GetDWord();
					base_data_offset = bs.GetQWord();
					base_data_offset = bs.GetDWord();
					sample_description_index = bs.GetDWord();
					default_sample_duration = bs.GetDWord();
					default_sample_size = bs.GetDWord();
					default_sample_flags = bs.GetDWord();

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

				uint32_t	sample_count;
				int32_t		data_offset;
				uint32_t	first_sample_flags;

				std::vector<Sample>	samples;

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

					sample_count = bs.GetDWord();
					data_offset = bs.GetLong();
					first_sample_flags = bs.GetDWord();
					left_bytes -= 12;

					for (uint32_t i = 0; i < sample_count; i++)
					{
						if (left_bytes < sizeof(Sample))
							break;

						Sample sample;
						sample.sample_duration = bs.GetDWord();
						sample.sample_size = bs.GetDWord();
						sample.sample_flags = bs.GetDWord();
						if (version == 0)
							sample.v0.sample_composition_time_offset = bs.GetDWord();
						else
							sample.v1.sample_composition_time_offset = bs.GetLong();

						samples.push_back(sample);
						left_bytes -= sizeof(sample);
					}

					SkipLeftBits(bs);
					return 0;
				}
			}PACKED;

			/*
			Box Type: 'tfdt'
			Container: Track Fragment box (‘traf?
			Mandatory: No
			Quantity: Zero or one
			*/
			struct TrackFragmentBaseMediaDecodeTimeBox : public FullBox
			{
				uint64_t	baseMediaDecodeTime;

				virtual int Unpack(CBitstream& bs)
				{
					int iRet = 0;
					if ((iRet = FullBox::Unpack(bs)) < 0)
						return iRet;

					uint64_t left_bytes = LeftBytes(bs);
					if (left_bytes < version==1?sizeof(uint64_t):sizeof(uint32_t))
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
		}PACKED;

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


				ptr_child = ptr_child->next_sibling;
			}

			return iRet;
		}
	}PACKED;

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

			uint32_t	track_ID;
			uint32_t	reserved : 26;
			uint32_t	length_size_of_traf_num : 2;
			uint32_t	length_size_of_trun_num : 2;
			uint32_t	length_size_of_sample_num : 2;
			uint32_t	number_of_entry;

			std::vector<Entry>	entries;

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

					left_bytes -= entry_size;
				}

				SkipLeftBits(bs);
				return 0;
			}
		}PACKED;

		/*
		Box Type: 'mfro'
		Container: Movie Fragment Random Access Box ('mfra')
		Mandatory: Yes
		Quantity: Exactly one
		*/
		struct MovieFragmentRandomAccessOffsetBox : public FullBox
		{
			uint32_t	size;

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
	}PACKED;
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED



