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

#define MIN_BOX_SIZE			8
#define MIN_FULLBOX_SIZE		12

namespace ISOMediaFile
{
	class IBox: public IUnknown
	{
	public:
		virtual int Unpack(CBitstream& bs) = 0;
		virtual int Pack(CBitstream& bs) = 0;
	};

	struct Box: public CComUnknown, public IBox
	{
		uint64_t	size = MIN_BOX_SIZE;
		uint32_t	type = 0;

		uint8_t		usertype[16] = { 0 };
		uint64_t	start_bitpos = 0;

		Box*		next_sibling = nullptr;
		Box*		first_children = nullptr;
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

		virtual ~Box()
		{
			// delete all its children
			Box* ptr_child = first_children;
			while (ptr_child)
			{
				Box* ptr_front = ptr_child;
				ptr_child = ptr_child->next_sibling;
				delete ptr_front;
			}
		}

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
			for (uint64_t i = 0; i < left_bytes; i++)
			{
				char c = bs.GetChar();
				if (c == '\0')
					break;

				str.push_back(c);
			}
		}

		void AppendChildBox(Box* child) noexcept
		{
			// Before appending the child box into the current box, the client must ensure
			// it is has already detached from the original box tree
			assert(child->container == nullptr && child->next_sibling == nullptr);

			child->container = this;

			// append the child to the last child
			if (first_children == NULL)
				first_children = child;
			else
			{
				// find the last child
				Box* ptr_child = first_children;
				while (ptr_child->next_sibling)
					ptr_child = ptr_child->next_sibling;

				ptr_child->next_sibling = child;
			}
		}

		static Box*	RootBox();
		static int	LoadBoxes(Box* pContainer, CBitstream& bs, Box** ppBox);
		static void UnloadBoxes(Box* pBox);
	}PACKED;

	struct FullBox : public Box
	{
		uint32_t	version : 8;
		uint32_t	flags : 24;

		virtual int Unpack(CBitstream bs)
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
		virtual int Unpack(CBitstream bs)
		{
			int iRet = 0;
			if ((iRet = Box::Unpack(bs)) < 0)
				return iRet;

			SkipLeftBits(bs);
			return 0;
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
		uint32_t	major_brand;
		uint32_t	minor_version;
		uint32_t	compatible_brands[];

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = 0;

			if ((iRet = Box::Unpack(bs)) < 0)
				return iRet;

			if (LeftBytes(bs) < sizeof(major_brand) + sizeof(minor_version))
			{
				SkipLeftBits(bs);
				return RET_CODE_BOX_TOO_SMALL;
			}

			major_brand = bs.GetDWord();
			minor_version = bs.GetDWord();

			SkipLeftBits(bs);

			return 0;
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
				infos.resize((size_t)(left_bytes / sizeof(Info)));
				for (size_t i = 0; i < infos.size(); i++)
				{
					infos[i].rate = bs.GetDWord();
					infos[i].initial_delay = bs.GetDWord();
				}
			}

			SkipLeftBits(bs);

			return 0;
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

			if (LeftBytes(bs) < sizeof(pre_defined) + sizeof(handler_type) + sizeof(reserved))
			{
				SkipLeftBits(bs);
				return RET_CODE_BOX_TOO_SMALL;
			}

			pre_defined = bs.GetDWord();
			handler_type = bs.GetDWord();
			for (int i = 0; i < _countof(reserved); i++)
				reserved[i] = bs.GetDWord();

			uint64_t left_bytes = LeftBytes(bs);
			for (uint64_t i = 0; i < left_bytes; i++)
			{
				char c = bs.GetChar();
				if (c == '\0')
					break;
				name.push_back(c);
			}

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
	struct DataInformationBox : public Box
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

				uint64_t left_bytes = LeftBytes(bs);
				for (uint64_t i = 0; i < left_bytes; i++)
				{
					char c = bs.GetChar();
					if (c == '\0')
						break;

					location.push_back(c);
				}

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

				uint64_t left_bytes = LeftBytes(bs);
				for (uint64_t i = 0; i < left_bytes; i++)
				{
					char c = bs.GetChar();
					if (c == '\0')
						break;

					name.push_back(c);
				}

				left_bytes = LeftBytes(bs);
				for (uint64_t i = 0; i < left_bytes; i++)
				{
					char c = bs.GetChar();
					if (c == '\0')
						break;

					location.push_back(c);
				}

				SkipLeftBits(bs);
				return 0;
			}
		}PACKED;

		struct DataReferenceBox : public FullBox
		{
			uint32_t			entry_count;
			std::vector<Box*>	data_entries;

			DataReferenceBox() : entry_count(0) {}
			~DataReferenceBox() {
				for (size_t i = 0; i < data_entries.size(); i++)
					if (data_entries[i] != NULL)
						delete data_entries[i];
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

				left_bytes -= 4;

				UnknownBox unknBox;
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

					ptr_box->Unpack(bs);
					left_bytes = LeftBytes(bs);

					if (box_type == 'url ' || box_type == 'urn ')
						data_entries.push_back(ptr_box);
				}

				SkipLeftBits(bs);
				return 0;
			};

		}PACKED;

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
				SkipLeftBits(bs);
				return RET_CODE_BOX_TOO_SMALL;
			}

			grouping_type = bs.GetDWord();
			if (version == 1)
			{
				left_bytes = LeftBytes(bs);
				if (left_bytes < sizeof(v1))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				v1.grouping_type_parameter = bs.GetDWord();
			}

			if ((left_bytes = LeftBytes(bs)) < sizeof(entry_count))
			{
				SkipLeftBits(bs);
				return RET_CODE_BOX_TOO_SMALL;
			}
			entry_count = bs.GetDWord();

			if ((left_bytes = LeftBytes(bs)) < sizeof(Entry)*entry_count)
				printf("The 'sbgp' box size is too small {size: %llu, entry_count: %lu}.\n", size, entry_count);

			uint32_t actual_entry_count = (uint32_t)AMP_MIN(left_bytes / sizeof(Entry), (uint64_t)entry_count);
			for (uint32_t i = 0; i < actual_entry_count; i++)
			{
				Entry entry;
				entry.sample_count = bs.GetDWord();
				entry.group_description_index = bs.GetDWord();
				entries.push_back(entry);
			}

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
				SkipLeftBits(bs);
				return RET_CODE_BOX_TOO_SMALL;
			}

			grouping_type = bs.GetDWord();

			if (version == 1)
			{
				if ((left_bytes = LeftBytes(bs)) < sizeof(default_length))
				{
					SkipLeftBits(bs);
					return RET_CODE_BOX_TOO_SMALL;
				}

				default_length = bs.GetDWord();
			}

			if ((left_bytes = LeftBytes(bs)) < sizeof(entry_count))
			{
				SkipLeftBits(bs);
				return RET_CODE_BOX_TOO_SMALL;
			}

			entry_count = bs.GetDWord();
			for (uint32_t i = 0; i < entry_count; i++)
			{
				Entry entry;
				if (version == 1 && default_length == 0)
				{
					if ((left_bytes = LeftBytes(bs)) < sizeof(entry_count))
						break;

					entry.description_length = bs.GetDWord();
				}
				else
					entry.description_length = default_length;

				if ((left_bytes = LeftBytes(bs)) < entry.description_length)
					break;

				bs.SkipBits((uint64_t)entry.description_length << 3);
			}

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
	struct UserDataBox : public FullBox
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
		struct SubTrack : public Box
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

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;

				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				SkipLeftBits(bs);
				return 0;
			}
		}PACKED;

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
	Container: Protection Scheme Information Box (‘sinf’), Restricted Scheme Information Box ('rinf'),
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
	struct MetaBox : public FullBox
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

	}PACKED;


	/*
	Box Type: 'moov'
	Container: File
	Mandatory: Yes
	Quantity: Exactly one
	*/
	struct MovieBox : public Box
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
		struct TrackBox : public Box
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

					uint64_t start_bitpos = bs.Tell();
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
			struct MediaBox : public Box
			{
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
				struct MediaInformationBox : public Box
				{
					/*
					Box Types: 'mhd' 'mhd' 'mhd' 'mhd'
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
						uint16_t	balance;
						uint16_t	reserved[3];

						virtual int Unpack(CBitstream& bs)
						{
							int iRet = 0;

							if ((iRet = FullBox::Unpack(bs)) < 0)
								return iRet;

							if (LeftBytes(bs) < sizeof(balance) + sizeof(reserved))
							{
								SkipLeftBits(bs);
								return RET_CODE_BOX_TOO_SMALL;
							}

							balance = bs.GetWord();
							for (size_t i = 0; i < _countof(reserved); i++)
								reserved[i] = bs.GetWord();

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
					struct SampleTableBox : public Box
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

								SkipLeftBits(bs);
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

							virtual int Unpack(CBitstream& bs)
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
										{
											UnknownBox unknBox;
											unknBox.Unpack(bs);
										}

										left_bytes = LeftBytes(bs);
									}
								}
								catch (std::exception& e)
								{
									// continue since these 2 fields are optional
								}

								SkipLeftBits(bs);
								return 0;
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

								// Try to get handler_type from its container
								if (container == nullptr)
								{
									printf("The current 'stsd' box has no container box unexpectedly.\n");
									return RET_CODE_ERROR;
								}

								if (container->type != 'stbl')
								{
									printf("The current 'stsd' box does NOT lie at a 'stbl' box container.\n");
									return RET_CODE_ERROR;
								}

								if (container->container == nullptr || container->container->type != 'minf' ||
									container->container->container == nullptr || container->container->container->type != 'mdia')
								{
									printf("Can't find the 'mdia' ancestor of the current 'stsd' box.\n");
									return RET_CODE_ERROR;
								}

								auto ptr_mdia_container = container->container->container;

								// find hdlr box
								Box* ptr_mdia_child = ptr_mdia_container->first_children;
								while (ptr_mdia_child != nullptr)
								{
									if (ptr_mdia_child->type == 'hdlr')
										break;
									ptr_mdia_child = ptr_mdia_child->next_sibling;
								}

								if (ptr_mdia_child == nullptr)
								{
									printf("Can't find 'hdlr' box for the current 'stsd' box.\n");
									return RET_CODE_ERROR;
								}

								handler_type = dynamic_cast<HandlerBox*>(ptr_mdia_child)->handler_type;

								for (uint32_t i = 0; i < entry_count; i++)
								{
									Box* pBox = nullptr;
									switch (handler_type)
									{
									case 'soun':	// for audio tracks
										pBox = new AudioSampleEntry();
										break;
									case 'vide':	// for video tracks
										pBox = new VisualSampleEntry();
										break;
									case 'hint':	// Hint track
										pBox = new HintSampleEntry();
										break;
									case 'meta':	// Metadata track
										pBox = new MetaDataSampleEntry();
										break;
									default:
										pBox = new UnknownBox();
									}

									pBox->container = this;

									pBox->Unpack(bs);

									SampleEntries.push_back(pBox);
								}

								SkipLeftBits(bs);
								return 0;
							}
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

								Box* pChild = container->first_children;
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

								Box* pChild = container->first_children;
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

					}PACKED;

				}PACKED;
			}PACKED;

		}PACKED;

		/*
		Box Type: 'mvex'
		Container: Movie Box ('moov')
		Mandatory: No
		Quantity: Zero or one
		*/
		struct MovieExtendsBox : public Box
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

		}PACKED;

		MovieHeaderBox* ptr_movie_header_box;
		std::vector<TrackBox> track_boxes;

		MovieBox()
			: ptr_movie_header_box(NULL)
		{
		}

		virtual int Unpack(CBitstream bs)
		{
			int iRet = 0;
			if ((iRet = Box::Unpack(bs)) < 0)
				return iRet;

			SkipLeftBits(bs);
			return 0;
		}

	}PACKED;

	/*
	Box Type: 'moof'
	Container: File
	Mandatory: No
	Quantity: Zero or more
	*/
	struct MovieFragmentBox : public Box
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

			virtual int Unpack(CBitstream bs)
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
		struct TrackFragmentBox : public Box
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

				virtual int Unpack(CBitstream bs)
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

				virtual int Unpack(CBitstream bs)
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
			Container: Track Fragment box (‘traf’)
			Mandatory: No
			Quantity: Zero or one
			*/
			struct TrackFragmentBaseMediaDecodeTimeBox : public FullBox
			{
				uint64_t	baseMediaDecodeTime;

				virtual int Unpack(CBitstream bs)
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

			virtual int Unpack(CBitstream bs)
			{
				int iRet = 0;
				if ((iRet = Box::Unpack(bs)) < 0)
					return iRet;

				SkipLeftBits(bs);
				return 0;
			}
		}PACKED;

		virtual int Unpack(CBitstream bs)
		{
			int iRet = 0;
			if ((iRet = Box::Unpack(bs)) < 0)
				return iRet;

			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;

	/*
	Box Type: 'mfra'
	Container: File
	Mandatory: No
	Quantity: Zero or one
	*/
	struct MovieFragmentRandomAccessBox : public Box
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

			virtual int Unpack(CBitstream bs)
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

			virtual int Unpack(CBitstream bs)
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

		virtual int Unpack(CBitstream bs)
		{
			int iRet = 0;
			if ((iRet = Box::Unpack(bs)) < 0)
				return iRet;

			SkipLeftBits(bs);
			return 0;
		}
	}PACKED;
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED



