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
#include "ISO14496_15.h"
#include <algorithm>
#include <new>
#include <chrono>
#include <time.h>
#include <sys/timeb.h>

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

#define INVALID_EBML_ID					UINT32_MAX

#define MATROSKA_EBML_ID				0x1A45DFA3
#define MATROSKA_SEGMENT_ID				0x18538067
#define MATROSKA_SEEKHEAD_ID			0x114D9B74
#define MATROSKA_INFO_ID				0x1549A966
#define MATROSKA_CLUSTER_ID				0x1F43B675
#define MATROSKA_TRACKS_ID				0x1654AE6B
#define MATROSKA_CUES_ID				0x1C53BB6B
#define MATROSKA_ATTACHMENTS_ID			0x1941A469
#define MATROSKA_CHAPTERS_ID			0x1043A770
#define MATROSKA_TAGS_ID				0x1254C367

namespace BST
{
	namespace Matroska
	{
		void PrintEBMLElements(uint32_t EBMLID);

		enum EBML_DATA_TYPE
		{
			EBML_DT_UNKNOWN = -1,
			EBML_DT_SIGNED_INTEGER = 0,
			EBML_DT_UNSIGNED_INTEGER,
			EBML_DT_FLOAT,
			EBML_DT_ASCII_STRING,
			EBML_DT_UTF8_STRING,
			EBML_DT_DATE,
			EBML_DT_MASTER,
			EBML_DT_BINARY,
		};

		#define EBML_DATA_TYPE_NAMEA(data_type)	(\
			(data_type) == BST::Matroska::EBML_DT_SIGNED_INTEGER ? "i" : (\
			(data_type) == BST::Matroska::EBML_DT_UNSIGNED_INTEGER ? "u" : (\
			(data_type) == BST::Matroska::EBML_DT_FLOAT ? "f" : (\
			(data_type) == BST::Matroska::EBML_DT_ASCII_STRING ? "s" : (\
			(data_type) == BST::Matroska::EBML_DT_UTF8_STRING ? "8" : (\
			(data_type) == BST::Matroska::EBML_DT_DATE ? "d" : (\
			(data_type) == BST::Matroska::EBML_DT_MASTER ? "m" : (\
			(data_type) == BST::Matroska::EBML_DT_BINARY ? "b" : " "))))))))

		enum EBML_VERSION_CAPS
		{
			EBML_VER_1			= 0x01,		// The element is contained in Matroska version 1
			EBML_VER_2			= 0x02,		// The element is contained in Matroska version 2
			EBML_VER_3			= 0x04,		// The element is contained in Matroska version 3
			EBML_VER_4			= 0x08,		// The element is contained in Matroska version 4
			EBML_VER_WEBM		= 0x80,		// The element is contained in WEBM
		};

		struct EBML_ELEMENT_DESCRIPTOR
		{
			const char*		Element_Name;
			uint32_t		EBML_ID;
			int32_t			Level : 8;		// -1: global
			int32_t			VersionCaps : 8;
			int32_t			data_type : 8;
			int32_t			bMandatory : 1;	// 0: No; 1: Yes
			int32_t			bMultiple : 1;
			int32_t			reserved : 6;
		}PACKED;

		extern EBML_ELEMENT_DESCRIPTOR EBML_element_descriptors[212];

		class IEBMLElement : public IUnknown
		{
		public:
			virtual int Unpack(CBitstream& bs) = 0;
			virtual int Pack(CBitstream& bs) = 0;
			virtual int Clean() = 0;
		};

		struct RootElement;

		struct EBMLElement : public CComUnknown, public IEBMLElement
		{
			uint32_t		ID = 0;
			uint64_t		Size = 0;

			EBMLElement*	next_sibling = nullptr;
			EBMLElement*	first_child = nullptr;
			EBMLElement*	container = nullptr;

			DECLARE_IUNKNOWN

			HRESULT NonDelegatingQueryInterface(REFIID uuid, void** ppvObj)
			{
				if (ppvObj == NULL)
					return E_POINTER;

				if (uuid == IID_IEBMLElement)
					return GetCOMInterface((EBMLElement*)this, ppvObj);

				return CComUnknown::NonDelegatingQueryInterface(uuid, ppvObj);
			}

			EBMLElement(): EBMLElement(0, 0) {}

			EBMLElement(EBMLElement* pContainerBox) : container(pContainerBox) {
			}

			EBMLElement(uint32_t Element_ID, uint64_t Element_Size) : ID(Element_ID), Size(Element_Size) {
			}

			virtual ~EBMLElement() { Clean(); }

			static int32_t GetDescIdx(uint32_t ElementID);

			static uint64_t UnpackUnsignedIntVal(CBitstream&bs, uint8_t max_octs = 8, bool unPackVal=true, uint8_t* pcbValLen=nullptr)
			{
				uint8_t nLeadingZeros = 0;
				uint64_t u64Val = bs.GetByte();
				for (uint8_t i = 0; i < max_octs; i++)
					if ((u64Val&(1ULL << (7 - i))) == 0)
						nLeadingZeros++;
					else
						break;

				if (nLeadingZeros >= max_octs)	// Unexpected
					return UINT64_MAX;

				if (unPackVal)
					u64Val &= ~(1 << (7 - nLeadingZeros));

				for (uint8_t i = 0; i<nLeadingZeros; i++)
					u64Val = (((uint64_t)u64Val) << 8) | (uint8_t)bs.GetBits(8);

				if (pcbValLen != nullptr)
					*pcbValLen = nLeadingZeros + 1;

				return u64Val;
			}

			static uint64_t UnpackUnsignedIntVal(FILE* fp, uint8_t max_octs = 8, bool unPackVal = true)
			{
				uint8_t nLeadingZeros = 0;
				uint8_t u8Byte;
				if (fread(&u8Byte, 1, 1, fp) != 1)
					return UINT64_MAX;

				uint64_t u64Val = u8Byte;
				for (uint8_t i = 0; i < max_octs; i++)
					if ((u64Val&(1ULL << (7 - i))) == 0)
						nLeadingZeros++;
					else
						break;

				if (nLeadingZeros >= max_octs)	// Unexpected
					return UINT64_MAX;

				if (unPackVal)
					u64Val &= ~(1 << (7 - nLeadingZeros));

				for (uint8_t i = 0; i < nLeadingZeros; i++)
				{
					if (fread(&u8Byte, 1, 1, fp) != 1)
						return UINT64_MAX;

					u64Val = (((uint64_t)u64Val) << 8) | u8Byte;
				}

				return u64Val;
			}

			virtual int Unpack(CBitstream& bs)
			{
				// Read the element ID
				//uint64_t start_bitpos = bs.Tell();
				uint64_t u64Val = UnpackUnsignedIntVal(bs, 4, false);
				if (u64Val == UINT64_MAX)
					return -1;

				ID = (uint32_t)u64Val;

				if ((u64Val = UnpackUnsignedIntVal(bs)) == UINT64_MAX)
					return -1;

				Size = u64Val;

				//printf("ID: 0X%X, Size: %lld(0X%" PRIX64 "), start_bitpos: %lld\n", ID, Size, Size, start_bitpos);

				return 0;
			}

			virtual int Pack(CBitstream& bs)
			{
				return -1;
			}

			virtual int Clean()
			{
				// delete all its children
				EBMLElement* ptr_child = first_child;
				while (ptr_child)
				{
					EBMLElement* ptr_front = ptr_child;
					ptr_child = ptr_child->next_sibling;
					delete ptr_front;
				}

				first_child = nullptr;

				return 0;
			}

			void AppendChild(EBMLElement* child) noexcept
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
					EBMLElement* ptr_child = first_child;
					while (ptr_child->next_sibling)
						ptr_child = ptr_child->next_sibling;

					ptr_child->next_sibling = child;
				}
			}

			void RemoveChild(EBMLElement* child) noexcept
			{
				// Find this child from tree
				if (first_child == nullptr)
					return;

				EBMLElement* ptr_child = first_child;
				while (ptr_child->next_sibling != child && ptr_child->next_sibling != nullptr)
					ptr_child = ptr_child->next_sibling;

				EBMLElement* ptr_cur_box = ptr_child->next_sibling;
				if (ptr_cur_box != nullptr)
				{
					ptr_child->next_sibling = ptr_cur_box->next_sibling;
					ptr_cur_box->container = nullptr;

					delete ptr_cur_box;
				}
			}

			bool IsRoot() {
				return ID == 0 && Size == 0 && this->container == NULL;
			}

			EBML_DATA_TYPE DateType() {
				int32_t desc_idx = GetDescIdx(ID);
				if (desc_idx < 0 || desc_idx >= (int32_t)_countof(EBML_element_descriptors))
					return EBML_DT_UNKNOWN;

				return (EBML_DATA_TYPE)EBML_element_descriptors[desc_idx].data_type;
			}

			int FindEBMLElementByElementID(uint32_t element_id, std::vector<EBMLElement*>& result);
			int FindEBMLElementByTrackID(uint32_t track_id, uint32_t element_id, std::vector<EBMLElement*>& result);

			static RootElement*	Root();
			static int LoadEBMLElements(EBMLElement* pContainer, CBitstream& bs, EBMLElement** ppElement = nullptr);
			static void UnloadEBMLElements(EBMLElement* pElement);
		}PACKED;

		struct UnknownEBMLElement : public EBMLElement
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				if (Size > 0)
					bs.SkipBits(Size << 3);

				return 0;
			}
		};

		struct SignedIntegerElement : public EBMLElement
		{
			int64_t			iVal = 0;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;
				if (Size > 8 || Size <= 0)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					_tprintf(_T("Unexpected!! Signed Integer - Big-endian, the size should be from 1 to 8 octets\n"));
					goto done;
				}
			
				iVal = (int64_t)bs.GetBits((uint8_t)Size << 3);
				if (Size < 8)
				{
					uint64_t signMask = (1ULL << ((Size << 3) - 1));
					if (iVal&signMask)
						iVal = iVal - (signMask << 1);
				}
				cbLeft = 0;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return iRet;
			}
		};

		struct UnsignedIntegerElement : public EBMLElement
		{
			uint64_t			uVal = 0;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;
				if (Size > 8 || Size <= 0)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					_tprintf(_T("[Matroska] Unexpected!! Unsigned Integer - Big-endian, the size should be from 1 to 8 octets\n"));
					goto done;
				}

				uVal = bs.GetBits((uint8_t)Size << 3);
				cbLeft = 0;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return iRet;
			}
		};

		struct FloatElement : public EBMLElement
		{
			double			fVal = NAN;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;
				if (Size != 4 && Size != 8)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					_tprintf(_T("Unexpected!! Float - Big-endian, defined for 4 and 8 octets (32, 64 bits)\n"));
					goto done;
				}

				if (Size == 4)
				{
					union {
						float fVal;
						uint32_t u32Val;
					}PACKED Val;

					Val.u32Val = bs.GetDWord();
					fVal = Val.fVal;
					cbLeft -= 4;
				}
				else if (Size == 8)
				{
					union {
						double fVal;
						uint64_t u64Val;
					}PACKED Val;

					Val.u64Val = bs.GetQWord();
					fVal = Val.fVal;
					cbLeft -= 8;
				}

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return iRet;
			}
		};

		struct ASCIIStringElement : public EBMLElement 
		{
			char*	szVal = nullptr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;
				if (Size >= UINT32_MAX)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					_tprintf(_T("The EBML element size: %" PRIu64 " is too big.\n"), Size);
					goto done;
				}

				szVal = new(std::nothrow) char[(uint32_t)(Size + 1)];
				if (szVal == NULL)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					_tprintf(_T("Failed to allocate the memory with the size: %" PRIu64 ".\n"), Size);
					goto done;
				}

				memset(szVal, 0, (uint32_t)(Size + 1));
				bs.Read((uint8_t*)szVal, (uint32_t)Size);
				cbLeft = 0;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return iRet;
			}
		};

		struct UTF8StringElement : public EBMLElement
		{
			char*	szUTF8 = nullptr;

			~UTF8StringElement() {
				AMP_SAFEDELA(szUTF8);
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;
				if (Size >= UINT32_MAX)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					_tprintf(_T("The EBML element size: %" PRIu64 " is too big.\n"), Size);
					goto done;
				}

				szUTF8 = new(std::nothrow) char[(uint32_t)(Size + 1)];
				if (szUTF8 == NULL)
				{
					iRet = RET_CODE_OUTOFMEMORY;
					_tprintf(_T("Failed to allocate the memory with the size: %" PRIu64 ".\n"), Size);
					goto done;
				}

				memset(szUTF8, 0, (uint32_t)(Size + 1));
				bs.Read((uint8_t*)szUTF8, (uint32_t)Size);
				cbLeft = 0;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return iRet;
			}
		};

		struct DateElement : public EBMLElement
		{
			int64_t ns_since_20010101_midnight = 0;	// Original value with high resolution
			time_t	timeVal = 0;					// The standard C/C++ time type

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;

				if (Size < 8)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					_tprintf(_T("The EBML element size: %" PRIu64 " is too small for Date type.\n"), Size);
					goto done;
				}

				ns_since_20010101_midnight = (int64_t)bs.GetQWord();
				cbLeft -= 8;

				{
					// Convert it to standard c/c++ time_t
					struct tm UTC20010101Midnight;
					memset(&UTC20010101Midnight, 0, sizeof(UTC20010101Midnight));
					UTC20010101Midnight.tm_year = 2001 - 1900;
					UTC20010101Midnight.tm_mday = 1;

					time_t timeUTC20010101Midnight = -1;
	#ifdef _WIN32
					if ((timeUTC20010101Midnight = _mkgmtime(&UTC20010101Midnight)) == -1)
	#else
					if ((timeUTC20010101Midnight = timegm(&UTC20010101Midnight)) <= 0)
	#endif
					{
						printf("Failed to generate UTC time value for 2001-01-01T00:00:00,000000000 UTC\n");
						timeVal = -1;
					}

					timeVal = timeUTC20010101Midnight + ns_since_20010101_midnight / 1000000000LL;
				}

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return iRet;
			}
		};

		struct BinaryElement : public EBMLElement
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				if (Size > 0)
					bs.SkipBits(Size << 3);
				return 0;
			}
		};

		struct MasterElement : public EBMLElement
		{
			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t start_bitpos = bs.Tell();
				AMP_Assert(start_bitpos % 8 == 0);

				// Try to load all its children elements
				uint64_t cbLeft = Size;
				while (cbLeft >= 2)
				{
					if (LoadEBMLElements(this, bs) < 0)
					{
						printf("[Matroska] Failed to load its sub-elements of the next lower level.\n");
						break;
					}

					uint64_t cbParsed = (bs.Tell() - start_bitpos) >> 3;
					cbLeft = Size > cbParsed ? (Size - cbParsed) : 0;
				}

				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return 0;
			}
		};

		struct RootElement : public EBMLElement
		{

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				while ((iRet = LoadEBMLElements(this, bs)) >= 0);

				return 0;
			}
		};

		// Save memory for BlockGroup or SimpleBlock index
		struct ClusterBlockMapForOneTrack
		{
			struct BlockCoarse
			{
				uint32_t	ref_to_Block_fine_id;
				uint32_t	Byte_Pos_Block_Coarse;

				BlockCoarse(uint64_t file_pos, uint32_t fine_id) {
					Byte_Pos_Block_Coarse = (uint32_t)((file_pos >> 25)&UINT32_MAX);
					ref_to_Block_fine_id = fine_id;
				}
			}PACKED;

			struct BlockFine
			{
				uint32_t	is_simple_block : 1;
				uint32_t	is_keyframe : 1;
				uint32_t	is_invisible : 1;
				uint32_t	is_discardable : 1;
				uint32_t	reserved : 3;
				uint32_t	Byte_Pos_Block_fine : 25;

				BlockFine(uint64_t file_pos, bool bSimpleBlock, bool bKeyframe, bool bInvisible, bool bDiscardable)
					: is_simple_block(bSimpleBlock ? 1 : 0)
					, is_keyframe(bKeyframe ? 1 : 0)
					, is_invisible(bInvisible ? 1 : 0)
					, is_discardable(bDiscardable ? 1 : 0)
					, reserved(0)
					, Byte_Pos_Block_fine(file_pos & 0x1FFFFFF){
				}
			}PACKED;

			std::vector<BlockCoarse>	Block_coarses;
			std::vector<BlockFine>		Block_fines;

			int64_t GetBytePos(size_t Block_id)
			{
				if (Block_id >= Block_fines.size())
					return -1LL;

				size_t  BC_id = (size_t)-1;
				for (size_t i = 0; i < Block_coarses.size(); i++)
				{
					if (Block_id < Block_coarses[i].ref_to_Block_fine_id)
					{
						BC_id = i < 1 ? 0 : (i - 1);
						break;
					}
				}
				if (BC_id == (size_t)-1)
					BC_id = Block_coarses.size() - 1;

				return (((int64_t)Block_coarses[BC_id].Byte_Pos_Block_Coarse)<<25) | Block_fines[Block_id].Byte_Pos_Block_fine;
			}

			int AppendBlock(uint64_t file_pos, bool bSimpleBlock, bool bKeyframe, bool bInvisible, bool bDiscardable)
			{
				Block_fines.emplace_back(file_pos, bSimpleBlock, bKeyframe, bInvisible, bDiscardable);

				if (Block_coarses.size() == 0 || Block_coarses.back().Byte_Pos_Block_Coarse != (uint32_t)((file_pos >> 25)&UINT32_MAX))
					Block_coarses.emplace_back(file_pos, (uint32_t)(Block_fines.size() - 1));

				return RET_CODE_SUCCESS;
			}
		};

		struct ClusterElement : public MasterElement
		{
			std::unordered_map<uint8_t, ClusterBlockMapForOneTrack*> TrackBlockMaps;

			virtual ~ClusterElement(){
				auto iter = TrackBlockMaps.cbegin();
				for (; iter != TrackBlockMaps.cend(); iter++)
					delete iter->second;
				TrackBlockMaps.clear();
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t start_bitpos = bs.Tell();
				AMP_Assert(start_bitpos % 8 == 0);

				// Try to load all its children elements
				uint64_t cbLeft = Size;
				while (cbLeft >= 2)
				{
					if (LoadEBMLElements(this, bs) < 0)
					{
						printf("[Matroska] Failed to load its sub-elements of the next lower level.\n");
						break;
					}

					uint64_t cbParsed = (bs.Tell() - start_bitpos) >> 3;
					cbLeft = Size > cbParsed ? (Size - cbParsed) : 0;
				}

				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);
				return 0;
			}

			int LoadSimpleBlock(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t block_start_pos = bs.Tell();
				assert(block_start_pos % 8 == 0);

				//printf("-- SimpleBlock file offset: 0X%" PRIX64 ".\n", block_start_pos / 8);

				uint32_t ID = (uint32_t)UnpackUnsignedIntVal(bs, 4, false);
				if (ID != 0xA3)
					return RET_CODE_BOX_INCOMPATIBLE;

				uint64_t Size = UnpackUnsignedIntVal(bs);
				if (Size == UINT64_MAX)
					return RET_CODE_BOX_INCOMPATIBLE;

				uint64_t cbLeft = Size;

				uint64_t u64Val = UnpackUnsignedIntVal(bs);
				if (u64Val == UINT64_MAX)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					goto done;
				}

				if (u64Val >= 0x80)
				{
					printf("[Matroska] At present, only support 127 tracks at maximum.\n");
					iRet = RET_CODE_ERROR_NOTIMPL;
					goto done;
				}

				if (TrackBlockMaps.find((uint8_t)u64Val) == TrackBlockMaps.end())
					TrackBlockMaps[(uint8_t)u64Val] = new ClusterBlockMapForOneTrack();

				{
					auto BlockMap = TrackBlockMaps[(uint8_t)u64Val];

					bs.SkipBits(16);
					bool bKeyframe = bs.GetBits(1) ? true : false;
					bs.SkipBits(3);
					bool bInvisible = (uint8_t)bs.GetBits(1) ? true : false;
					bs.SkipBits(2);
					bool bDiscardable = (uint8_t)bs.GetBits(1) ? true : false;

					BlockMap->AppendBlock(block_start_pos / 8, true, bKeyframe, bInvisible, bDiscardable);
				}

				cbLeft -= 4;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);

				return iRet;
			}

			int LoadBlockGroup(CBitstream& bs)
			{
				int iRet = RET_CODE_SUCCESS;
				uint64_t u64Val = 0;

				uint64_t block_start_pos = bs.Tell();
				assert(block_start_pos % 8 == 0);

				uint32_t ID = (uint32_t)UnpackUnsignedIntVal(bs, 4, false);
				if (ID != 0xA0)
					return RET_CODE_BOX_INCOMPATIBLE;

				uint64_t Size = UnpackUnsignedIntVal(bs);
				if (Size == UINT64_MAX)
					return RET_CODE_BOX_INCOMPATIBLE;

				int64_t cbLeft = Size;

				uint32_t Child_ID = UINT32_MAX;
				uint64_t Child_Size = UINT64_MAX;
				do
				{
					uint8_t cbValLen = 0;
					if ((Child_ID = (uint32_t)EBMLElement::UnpackUnsignedIntVal(bs, 4, false, &cbValLen)) == UINT32_MAX)
						break;

					cbLeft -= cbValLen;

					if ((Child_Size = EBMLElement::UnpackUnsignedIntVal(bs, 8, true, &cbValLen)) == UINT64_MAX)
						break;

					cbLeft -= cbValLen;

					if (Child_ID != 0xA1)
					{
						int64_t nActualSkipbits = bs.SkipBits(Child_Size << 3);
						if (nActualSkipbits < 0 || (uint64_t)nActualSkipbits != (Child_Size << 3))
							break;

						cbLeft -= Child_Size;
					}
				} while (Child_ID != 0xA1 && cbLeft > 0);

				if (Child_ID != 0xA1 || Child_Size == UINT64_MAX)
				{
					printf("[Matroska] Failed to find the Block element under BlockGroup element.\n");
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					goto done;
				}

				if (Child_Size < 4)
					goto done;

				u64Val = UnpackUnsignedIntVal(bs);
				if (u64Val == UINT64_MAX)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					goto done;
				}

				if (u64Val >= 0x80)
				{
					printf("[Matroska] At present, only support 127 tracks at maximum.\n");
					iRet = RET_CODE_ERROR_NOTIMPL;
					goto done;
				}

				if (TrackBlockMaps.find((uint8_t)u64Val) == TrackBlockMaps.end())
					TrackBlockMaps[(uint8_t)u64Val] = new ClusterBlockMapForOneTrack();

				{
					auto BlockMap = TrackBlockMaps[(uint8_t)u64Val];

					bs.SkipBits(16);
					bool bKeyframe = bs.GetBits(1) ? true : false;
					bs.SkipBits(3);
					bool bInvisible = (uint8_t)bs.GetBits(1) ? true : false;
					bs.SkipBits(2);
					bool bDiscardable = (uint8_t)bs.GetBits(1) ? true : false;

					BlockMap->AppendBlock(block_start_pos / 8, true, bKeyframe, bInvisible, bDiscardable);
				}

				cbLeft -= 4;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);

				return iRet;
			}

		};

		struct SimpleBlockElement : public BinaryElement
		{
			struct SimpleBlockHeader
			{
				uint32_t	track_number : 8;
				uint32_t	timecode : 16;
				uint32_t	Keyframe : 1;
				uint32_t	reserved : 3;
				uint32_t	Invisible : 1;
				uint32_t	Lacing : 2;
				uint32_t	Discardable : 1;
			}PACKED;

			uint64_t			start_offset;
			SimpleBlockHeader	simple_block_hdr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t start_bitpos = bs.Tell();
				AMP_Assert(start_bitpos % 8 == 0);

				uint64_t end_bitpos = start_bitpos;
				uint64_t cbLeft = Size;

				uint64_t u64Val = UnpackUnsignedIntVal(bs);
				if (u64Val == UINT64_MAX)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					goto done;
				}

				if (u64Val >= 0x80)
				{
					printf("[Matroska] At present, only support 127 tracks at maximum.\n");
					iRet = RET_CODE_ERROR_NOTIMPL;
					goto done;
				}

				simple_block_hdr.track_number = (uint8_t)u64Val;
				simple_block_hdr.timecode = bs.GetWord();
				simple_block_hdr.Keyframe = (uint8_t)bs.GetBits(1);
				simple_block_hdr.reserved = (uint8_t)bs.GetBits(3);
				simple_block_hdr.Invisible = (uint8_t)bs.GetBits(1);
				simple_block_hdr.Lacing = (uint8_t)bs.GetBits(2);
				simple_block_hdr.Discardable = (uint8_t)bs.GetBits(1);

				end_bitpos = bs.Tell();
				start_offset = end_bitpos / 8;

				cbLeft -= (end_bitpos - start_bitpos) >> 3;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);

				return iRet;
			}
		};

		struct Block : public BinaryElement
		{
			struct BlockHeader
			{
				uint32_t	track_number : 8;
				uint32_t	timecode : 16;
				uint32_t	reserved_0 : 4;
				uint32_t	Invisible : 1;
				uint32_t	Lacing : 2;
				uint32_t	reserved_1 : 1;
			}PACKED;

			uint64_t			start_offset;
			BlockHeader			block_hdr;

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t start_bitpos = bs.Tell();
				AMP_Assert(start_bitpos % 8 == 0);

				uint64_t end_bitpos = start_bitpos;
				uint64_t cbLeft = Size;

				uint64_t u64Val = UnpackUnsignedIntVal(bs);
				if (u64Val == UINT64_MAX)
				{
					iRet = RET_CODE_BOX_INCOMPATIBLE;
					goto done;
				}

				if (u64Val >= 0x80)
				{
					printf("[Matroska] At present, only support 127 tracks at maximum.\n");
					iRet = RET_CODE_ERROR_NOTIMPL;
					goto done;
				}

				block_hdr.track_number = (uint8_t)u64Val;
				block_hdr.timecode = bs.GetWord();
				block_hdr.reserved_0 = (uint8_t)bs.GetBits(4);
				block_hdr.Invisible = (uint8_t)bs.GetBits(1);
				block_hdr.Lacing = (uint8_t)bs.GetBits(2);
				block_hdr.reserved_1 = (uint8_t)bs.GetBits(1);

				end_bitpos = bs.Tell();
				start_offset = end_bitpos / 8;

				cbLeft -= (end_bitpos - start_bitpos) >> 3;

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);

				return iRet;
			}
		};

		struct CodecPrivateElement : public BinaryElement
		{
			uint8_t*	m_ptrCodecPrivData = nullptr;

			~CodecPrivateElement() {
				if (m_ptrCodecPrivData != nullptr)
					delete[] m_ptrCodecPrivData;
			}

			virtual int Unpack(CBitstream& bs)
			{
				int iRet = 0;
				if ((iRet = EBMLElement::Unpack(bs)) < 0)
					return iRet;

				uint64_t cbLeft = Size;
				if (cbLeft == 0 || cbLeft > INT32_MAX)
					goto done;

				m_ptrCodecPrivData = new uint8_t[(int)cbLeft];
				iRet = bs.Read(m_ptrCodecPrivData, (int)cbLeft);
				if (iRet > 0)
				{
					assert(cbLeft >= (uint64_t)iRet);
					cbLeft -= iRet;
					iRet = 0;
				}

			done:
				if (cbLeft > 0)
					bs.SkipBits(cbLeft << 3);

				return 0;
			}

			int UnpacksAsAVC(ISOBMFF::AVCDecoderConfigurationRecord* avcConfigRecord)
			{
				CBitstream bstCodecPriv(m_ptrCodecPrivData, (size_t)Size<<3);
				return avcConfigRecord->Unpack(bstCodecPriv, Size);
			}

			int UnpackAsHEVC(ISOBMFF::HEVCDecoderConfigurationRecord* hevcConfigRecord)
			{
				CBitstream bstCodecPriv(m_ptrCodecPrivData, (size_t)Size<<3);
				return hevcConfigRecord->Unpack(bstCodecPriv);
			}
		};

	} // namespace Matroska
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED
