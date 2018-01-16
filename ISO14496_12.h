#pragma once

#include <stdint.h>
#include "Bitstream.h"
#include "combase.h"

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
					return -1;

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

						uint32_t	pad : 1;
						uint32_t	language : 15;
						uint32_t	pre_defined : 16;
					}PACKED;

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

						struct SampleDescriptionBox : public FullBox
						{
							uint32_t	handler_type;
							uint32_t	entry_count;

							std::vector<Box*>	SampleEntries;

							virtual int Unpack(CBitstream& bs)
							{
								int iRet = 0;

								if ((iRet = FullBox::Unpack(bs)) < 0)
									return iRet;

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

								SkipLeftBits(bs);
								return 0;
							}
						}PACKED;

					}PACKED;

				}PACKED;
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

			return 0;
		}

	}PACKED;
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED



