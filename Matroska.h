#pragma once

#include <stdint.h>
#include "Bitstream.h"
#include "combase.h"
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

namespace Matroska
{
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
		uint32_t		ID;
		uint64_t		Size;

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

		static uint64_t UnpackUnsignedIntVal(CBitstream&bs, uint8_t max_octs = 8, bool unPackVal=true)
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

			return u64Val;
		}

		virtual int Unpack(CBitstream& bs)
		{
			// Read the element ID
			uint64_t u64Val = UnpackUnsignedIntVal(bs, 4, false);
			if (u64Val == UINT64_MAX)
				return -1;

			ID = (uint32_t)u64Val;

			if ((u64Val = UnpackUnsignedIntVal(bs)) == UINT64_MAX)
				return -1;

			Size = u64Val;

			//printf("ID: 0X%X, Size: %lld(0X%llX)\n", ID, Size, Size);

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
			if (desc_idx < 0 || desc_idx >= _countof(EBML_element_descriptors))
				return EBML_DT_UNKNOWN;

			return (EBML_DATA_TYPE)EBML_element_descriptors[desc_idx].data_type;
		}

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
		int64_t			iVal;

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
		uint64_t			uVal;

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
		double			fVal;

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
		char*	szVal;

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = 0;
			if ((iRet = EBMLElement::Unpack(bs)) < 0)
				return iRet;

			uint64_t cbLeft = Size;
			if (Size >= UINT32_MAX)
			{
				iRet = RET_CODE_BOX_INCOMPATIBLE;
				_tprintf(_T("The EBML element size: %llu is too big.\n"), Size);
				goto done;
			}

			szVal = new(std::nothrow) char[(uint32_t)(Size + 1)];
			if (szVal == NULL)
			{
				iRet = RET_CODE_OUTOFMEMORY;
				_tprintf(_T("Failed to allocate the memory with the size: %llu.\n"), Size);
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
		char*	szUTF8;

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = 0;
			if ((iRet = EBMLElement::Unpack(bs)) < 0)
				return iRet;

			uint64_t cbLeft = Size;
			if (Size >= UINT32_MAX)
			{
				iRet = RET_CODE_BOX_INCOMPATIBLE;
				_tprintf(_T("The EBML element size: %llu is too big.\n"), Size);
				goto done;
			}

			szUTF8 = new(std::nothrow) char[(uint32_t)(Size + 1)];
			if (szUTF8 == NULL)
			{
				iRet = RET_CODE_OUTOFMEMORY;
				_tprintf(_T("Failed to allocate the memory with the size: %llu.\n"), Size);
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
		int64_t ns_since_20010101_midnight;	// Original value with high resolution
		time_t	timeVal;					// The standard C/C++ time type

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = 0;
			if ((iRet = EBMLElement::Unpack(bs)) < 0)
				return iRet;

			uint64_t cbLeft = Size;

			if (Size < 8)
			{
				iRet = RET_CODE_BOX_INCOMPATIBLE;
				_tprintf(_T("The EBML element size: %llu is too small for Date type.\n"), Size);
				goto done;
			}

			ns_since_20010101_midnight = (int64_t)bs.GetQWord();
			cbLeft -= 8;

			// Convert it to standard c/c++ time_t
			struct tm UTC20010101Midnight;
			memset(&UTC20010101Midnight, 0, sizeof(UTC20010101Midnight));
			UTC20010101Midnight.tm_year = 2001 - 1990;
			UTC20010101Midnight.tm_mday = 1;

			time_t timeUTC20010101Midnight = -1;
	#ifdef _WIN32
			if ((timeUTC20010101Midnight = _mkgmtime(&UTC20010101Midnight)) == -1)
			{
				_tprintf(_T("Failed to generate UTC time value for 2001-01-01T00:00:00,000000000 UTC.\n"));
				timeVal = -1;
			}

			timeVal = timeUTC20010101Midnight + ns_since_20010101_midnight / 1000000000LL;
	#else
			if ((timeUTC20010101Midnight = timegm(&UTC20010101Midnight)) <= 0)
			{
				printf("Failed to generate UTC time value for 2001-01-01T00:00:00,000000000 UTC\n");
				timeVal = -1;
			}
	#endif

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
			while (LoadEBMLElements(this, bs) >= 0);

			return 0;
		}
	};

	struct SimpleBlockHeader
	{
		uint32_t	track_number : 8;
		uint32_t	timecode : 16;
		uint32_t	Keyframe : 1;
		uint32_t	reserved : 3;
		uint32_t	Invisible : 1;
		uint32_t	Lacing : 2;
		uint32_t	Discardable : 1;

		uint8_t		num_frames;
	}PACKED;

	struct SimpleBlockElement : public BinaryElement
	{
		uint64_t			start_offset;
		SimpleBlockHeader	simple_block_hdr;

		virtual int Unpack(CBitstream& bs)
		{
			int iRet = 0;
			if ((iRet = EBMLElement::Unpack(bs)) < 0)
				return iRet;

			uint64_t start_bitpos = bs.Tell();
			AMP_Assert(start_bitpos % 8 == 0);
			start_offset = start_bitpos / 8;

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

			simple_block_hdr.num_frames = (uint8_t)bs.GetByte();

			cbLeft -= (bs.Tell() - start_bitpos) >> 3;

		done:
			if (cbLeft > 0)
				bs.SkipBits(cbLeft << 3);

			return iRet;
		}
	};

} // namespace Matroska

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED
