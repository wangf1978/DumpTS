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

#define FIX_HEADER_FMT_STR		"%s%21s"

namespace AIFF
{
	using MarkerId = int16_t;

	struct AIFF_CHUNK
	{
		uint64_t		start_bitpos = 0;
		uint32_t        ckID = 0;
		uint32_t        ckDataSize = 0;

		AIFF_CHUNK() {}

		virtual ~AIFF_CHUNK() {}

		virtual int Unpack(CBitstream& bs)
		{
			uint64_t left_bits = 0ULL;
			start_bitpos = bs.Tell(&left_bits);

			if (left_bits < (8ULL<<3))
				return RET_CODE_BOX_TOO_SMALL;

			ckID = bs.GetDWord();
			ckDataSize = bs.GetDWord();

			return 8;
		}

		virtual void SkipLeftBits(CBitstream& bs)
		{
			uint64_t left_bits = LeftBits(bs);
			if (left_bits > 0)
				bs.SkipBits(left_bits);
		}

		virtual uint64_t LeftBits(CBitstream& bs)
		{
			uint64_t cur_bitpos = bs.Tell();

			if (cur_bitpos > start_bitpos)
			{
				if (((uint64_t)ckDataSize << 3) + (8ULL << 3) > cur_bitpos - start_bitpos)
					return (int64_t)(((uint64_t)ckDataSize << 3) + (8ULL << 3) - (cur_bitpos - start_bitpos));
			}

			return 0ULL;
		}

		virtual void Print(FILE* fp = nullptr, int indent=0)
		{
			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, "-----------------------------------------------------------------------------------------\n");
			fprintf(out, FIX_HEADER_FMT_STR ": %c%c%c%c\n", szIndent, "ckID",
				isprint(ckID >> 24) ? (ckID >> 24) : '.', isprint((ckID >> 16) & 0xFF) ? ((ckID >> 16) & 0xFF) : '.',
				isprint((ckID >> 8) & 0xFF) ? ((ckID >> 8) & 0xFF) : '.', isprint(ckID & 0xFF) ? (ckID & 0xFF) : '.');
			fprintf(out, FIX_HEADER_FMT_STR ": %" PRIu32 "\n", szIndent, "ckDataSize", ckDataSize);
		}
	};

	struct UnparsedChunk : public AIFF_CHUNK
	{
		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			SkipLeftBits(bs);
			return RET_CODE_SUCCESS;
		}
	};

	struct FormatVersionChunk : public AIFF_CHUNK
	{
		uint32_t		timestamp = 0;

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (4ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			timestamp = bs.GetDWord();

			SkipLeftBits(bs);
			return RET_CODE_SUCCESS;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, FIX_HEADER_FMT_STR ": %s\n", szIndent, "timestamp", DateTimeStr(timestamp).c_str());
		}
	}PACKED;

	struct CommonChunk : public AIFF_CHUNK
	{
		uint16_t        numChannels = 0;
		uint32_t        numSampleFrames = 0;
		int16_t         sampleSize = 0;
		uint8_t         sampleRateBytes[10] = { 0 };
		uint32_t		compressionType = 0;
		uint8_t			count_byte = 0;
		std::string		compressionName;

		int Unpack(CBitstream& bs)
		{
			uint64_t pstring_bytecount;
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (18ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			numChannels = bs.GetWord();
			numSampleFrames = bs.GetDWord();
			sampleSize = bs.GetWord();
			bs.Read(sampleRateBytes, 10);
			left_bits -= 18ULL << 3;

			if (left_bits < (4ULL << 3))
				goto done;
			
			compressionType = bs.GetDWord();
			left_bits -= (4ULL << 3);

			if (left_bits < (1ULL << 3))
				goto done;

			count_byte = bs.GetByte();
			left_bits -= (1ULL << 3);

			pstring_bytecount = (uint64_t)count_byte + ((count_byte % 2 == 0) ? 1 : 0);
			if (left_bits < (pstring_bytecount << 3))
				goto done;

			compressionName.reserve((size_t)count_byte + 1);
			compressionName.resize(count_byte);
			bs.Read((uint8_t*)(&compressionName[0]), count_byte);

		done:
			SkipLeftBits(bs);
			return nRet;
		}

		double GetSampleRate()
		{
			return ConvertExtentToDouble(sampleRateBytes);
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, FIX_HEADER_FMT_STR ": %-16" PRIu16 "%s\n", szIndent, "numChannels", numChannels, "the number of audio channels for the sound");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16" PRIu32 "%s\n", szIndent, "numSampleFrames", numSampleFrames, "the number of sample frames in the Sound Data Chunk, total sample points: numSampleFrames * numChannels");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16" PRIu32 "%s\n", szIndent, "sampleSize", numSampleFrames, "bits per sample");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16f%s\n", szIndent, "sampleRate", GetSampleRate(), "sample rate");

			if (ckDataSize >= 22)
			{
				fprintf(out, FIX_HEADER_FMT_STR ": %c%c%c%c%12s%s\n", szIndent, "compressionType",
					isprint(compressionType >> 24) ? (compressionType >> 24) : '.', isprint((compressionType >> 16) & 0xFF) ? ((compressionType >> 16) & 0xFF) : '.',
					isprint((compressionType >> 8) & 0xFF) ? ((compressionType >> 8) & 0xFF) : '.', isprint(compressionType & 0xFF) ? (compressionType & 0xFF) : '.',
					"", "identify the compression algorithm");
			}

			if (count_byte > 0)
				fprintf(out, FIX_HEADER_FMT_STR ": %s\n", szIndent, "compressionName", compressionName.c_str());
		}
	};

	struct SoundDataChunk : public AIFF_CHUNK
	{
		uint32_t        offset = 0;
		uint32_t        blocksize = 0;

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (8ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			offset = bs.GetDWord();
			blocksize = bs.GetDWord();

			SkipLeftBits(bs);

			return RET_CODE_SUCCESS;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, FIX_HEADER_FMT_STR ": %-16" PRIu32 "%s\n", szIndent, "offset", offset, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16" PRIu32 "%s\n", szIndent, "blocksize", blocksize, "");
		}
	};

	struct MarkerChunk : public AIFF_CHUNK
	{
		struct Marker
		{
			MarkerId		id = 0;
			uint32_t		position = 0;
			uint8_t			markerName_bytecount = 0;
			std::string		markerName;
		};

		uint16_t		numMarkers = 0;
		std::vector<Marker>
						Markers;

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return -1;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (2ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			numMarkers = bs.GetWord();
			left_bits -= 2;

			if (numMarkers > 0)
			{
				Markers.reserve(numMarkers);

				while (left_bits >= (8ULL << 3))
				{
					Markers.emplace_back();
					Markers.back().id = bs.GetShort();
					Markers.back().position = bs.GetDWord();
					Markers.back().markerName_bytecount = bs.GetByte();

					left_bits -= (7ULL << 3);

					auto markerName_bytecount = Markers.back().markerName_bytecount;
					uint64_t pstring_bytecount = (uint64_t)markerName_bytecount + ((markerName_bytecount % 2 == 0) ? 1 : 0);
					if (left_bits < (pstring_bytecount << 3))
						break;

					Markers.back().markerName.reserve((size_t)markerName_bytecount + 1);
					Markers.back().markerName.resize(markerName_bytecount);
					bs.Read((uint8_t*)(&Markers.back().markerName[0]), markerName_bytecount);

					if (markerName_bytecount % 2 == 0)
						bs.SkipBits(8);

					left_bits -= pstring_bytecount << 3;
				}
			}

			return nRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, FIX_HEADER_FMT_STR ": %-16" PRIu16 "%s\n", szIndent, "numMarkers", numMarkers, "");
			for (size_t i = 0; i < Markers.size(); i++)
			{
				fprintf(out, FIX_HEADER_FMT_STR ": %zu\n", szIndent, "Marker index", i);
				fprintf(out, "    " FIX_HEADER_FMT_STR ": %-16" PRIi16 "%s\n", szIndent, "id", Markers[i].id, "a number that uniquely identifies the marker within a FORM AIFC");
				fprintf(out, "    " FIX_HEADER_FMT_STR ": %-16" PRIu32 "%s\n", szIndent, "position", Markers[i].position, "The marker's position in the sound data");

				if (Markers[i].markerName_bytecount > 0)
					fprintf(out, "    " FIX_HEADER_FMT_STR ": %s\n", szIndent, "markerName", Markers[i].markerName.c_str());
			}
		}
	};

	struct CommentsChunk : public AIFF_CHUNK
	{
		struct MarkerComment
		{
			uint32_t		timeStamp = 0;
			MarkerId		marker = 0;
			uint16_t		count = 0;
			std::string		text;
		};

		uint16_t		numComments = 0;
		std::vector<MarkerComment>
						comments;

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return -1;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (2ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			numComments = bs.GetWord();
			left_bits -= 2;

			if (numComments > 0)
			{
				comments.reserve(numComments);

				while (left_bits >= (8ULL << 3))
				{
					comments.emplace_back();
					comments.back().timeStamp = bs.GetDWord();
					comments.back().marker = bs.GetShort();
					comments.back().count = bs.GetWord();

					left_bits -= (8ULL << 3);

					auto count = comments.back().count;
					uint64_t text_bytecount = (uint64_t)count + ((count % 2 == 0) ? 0 : 1);
					if (left_bits < (text_bytecount << 3))
						break;

					if (count > 0)
					{
						comments.back().text.reserve((size_t)count + 1);
						comments.back().text.resize(count);
						bs.Read((uint8_t*)(&comments.back().text[0]), count);

						if (count % 2 == 1)
							bs.SkipBits(8);
					}

					left_bits -= text_bytecount << 3;
				}
			}

			return nRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, FIX_HEADER_FMT_STR ": %-16u%s\n", szIndent, "numMarkers", numComments, "");
			for (size_t i = 0; i < comments.size(); i++)
			{
				fprintf(out, FIX_HEADER_FMT_STR ": %zu\n", szIndent, "Comment index", i);
				fprintf(out, "    " FIX_HEADER_FMT_STR ": %s\n", szIndent, "timeStamp", DateTimeStr(comments[i].timeStamp).c_str());
				fprintf(out, "    " FIX_HEADER_FMT_STR ": %-16" PRIi16 "%s\n", szIndent, "marker", comments[i].marker, comments[i].marker==0?"this comment is not linked to a marker":"the ID of that marker");
				fprintf(out, "    " FIX_HEADER_FMT_STR ": %-16" PRIu16 "%s\n", szIndent, "count", comments[i].count, "the length of the text that makes up the comment");

				if (comments[i].count > 0)
					fprintf(out, "    " FIX_HEADER_FMT_STR ": %s\n", szIndent, "text", comments[i].text.c_str());
			}
		}
	};

	struct InstrumentChunk : public AIFF_CHUNK
	{
		struct Loop
		{
			int16_t			playMode = 0;
			MarkerId		beginLoop = 0;
			MarkerId		endLoop = 0;
		}PACKED;

		int8_t			baseNote = 0;
		int8_t			detune = 0;
		int8_t			lowNote = 0;
		int8_t			highNote = 0;
		int8_t			lowVelocity = 0;
		int8_t			highVelocity = 0;
		int16_t			gain = 0;
		Loop			sustainLoop;
		Loop			releaseLoop;

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (20ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			baseNote = bs.GetChar();
			detune = bs.GetChar();
			lowNote = bs.GetChar();
			highNote = bs.GetChar();
			lowVelocity = bs.GetChar();
			highVelocity = bs.GetChar();
			gain = bs.GetShort();
			
			sustainLoop.playMode = bs.GetShort();
			sustainLoop.beginLoop = bs.GetShort();
			sustainLoop.endLoop = bs.GetShort();

			releaseLoop.playMode = bs.GetShort();
			releaseLoop.beginLoop = bs.GetShort();
			releaseLoop.endLoop = bs.GetShort();

			return RET_CODE_SUCCESS;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			FILE* out = fp ? fp : stdout;
			char szIndent[84];
			memset(szIndent, 0, _countof(szIndent));
			if (indent > 0)
			{
				int ccIndent = AMP_MIN(indent, 80);
				memset(szIndent, ' ', ccIndent);
			}

			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "baseNote", baseNote, "the pitch of the originally recorded sound");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "detune", detune, "make small tuning adjustments to the sound");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "lowNote", lowNote, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "highNote", highNote, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "lowVelocity", lowVelocity, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "highVelocity", highVelocity, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "gain", gain, "the amount to change the gain of the sound");

			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "sustainLoop.playMode", sustainLoop.playMode, 
				sustainLoop.playMode == 0?"NoLooping":(sustainLoop.playMode == 1?"ForwardLooping":(sustainLoop.playMode == 2?"ForwardBackwardLooping":"Unknown")));
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "sustainLoop.beginLoop", sustainLoop.beginLoop, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "sustainLoop.endLoop", sustainLoop.endLoop, "");

			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "releaseLoop.playMode", releaseLoop.playMode, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "releaseLoop.beginLoop", releaseLoop.beginLoop, "");
			fprintf(out, FIX_HEADER_FMT_STR ": %-16d%s\n", szIndent, "releaseLoop.endLoop", releaseLoop.endLoop, 
				releaseLoop.playMode == 0 ? "NoLooping" : (releaseLoop.playMode == 1 ? "ForwardLooping" : (releaseLoop.playMode == 2 ? "ForwardBackwardLooping" : "Unknown")));
		}

	}PACKED;

	struct TextChunk : public AIFF_CHUNK
	{
		std::string		text;

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits == 0)
				return RET_CODE_SUCCESS;

			size_t left_bytes = (size_t)(left_bits >> 3);
			text.resize(left_bytes);
			
			if (left_bytes > INT32_MAX)
			{
				SkipLeftBits(bs);
				return RET_CODE_OUTOFMEMORY;
			}

			bs.Read((uint8_t*)&text[0], (int)left_bytes);
			return RET_CODE_SUCCESS;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			if (text.length() > 0)
			{
				FILE* out = fp ? fp : stdout;
				char szIndent[84];
				memset(szIndent, 0, _countof(szIndent));
				if (indent > 0)
				{
					int ccIndent = AMP_MIN(indent, 80);
					memset(szIndent, ' ', ccIndent);
				}

				fprintf(out, FIX_HEADER_FMT_STR ": %s\n", szIndent, "text", text.c_str());
			}
		}

	};

	struct FormAIFCChunk : public AIFF_CHUNK
	{
		uint32_t        formType = 0;
		std::vector<AIFF_CHUNK*>
						chunks;

		virtual ~FormAIFCChunk() {
			for (size_t i = 0; i<chunks.size(); i++)
			{
				if (chunks[i] != NULL)
					delete chunks[i];
			}
		}

		int Unpack(CBitstream& bs)
		{
			int nRet = AIFF_CHUNK::Unpack(bs);
			if (nRet < 0)
				return nRet;

			uint64_t left_bits = LeftBits(bs);
			if (left_bits < (4ULL << 3))
				return RET_CODE_BOX_TOO_SMALL;

			formType = bs.GetDWord();
			left_bits -= (4ULL << 3);

			while (left_bits > (8ULL<<3))
			{
				AIFF_CHUNK* ptr_chunk = NULL;
				uint32_t peek_chunk_ID = (uint32_t)bs.PeekBits(32);
				switch (peek_chunk_ID)
				{
				case 'COMM':
					ptr_chunk = new CommonChunk();
					break;
				case 'SSND':
					ptr_chunk = new SoundDataChunk();
					break;
				case 'FVER':
					ptr_chunk = new FormatVersionChunk();
					break;
				case 'MARK':
					ptr_chunk = new MarkerChunk();
					break;
				case 'COMT':
					ptr_chunk = new CommentsChunk();
					break;
				case 'INST':
					ptr_chunk = new InstrumentChunk();
					break;
				case 'NAME':
				case 'AUTH':
				case '(c) ':
				case 'ANNO':
					ptr_chunk = new TextChunk();
					break;
				default:
					ptr_chunk = new UnparsedChunk();
				}

				chunks.push_back(ptr_chunk);

				if ((nRet = ptr_chunk->Unpack(bs)) < 0)
					goto done;

				uint64_t ckTotalBits = (((uint64_t)ptr_chunk->ckDataSize + 8)) << 3;

				if (left_bits < ckTotalBits)
					goto done;

				left_bits -= ckTotalBits;
			}

		done:
			SkipLeftBits(bs);
			return nRet;
		}

		virtual void Print(FILE* fp = nullptr, int indent = 0)
		{
			AIFF_CHUNK::Print(fp, indent);

			for (auto& v : chunks)
			{
				if (v == nullptr)
					continue;

				v->Print(stdout, indent + 4);
			}
		}
	};
}

#ifdef _MSC_VER
#pragma pack(pop)
#pragma warning(pop)
#endif
#undef PACKED
