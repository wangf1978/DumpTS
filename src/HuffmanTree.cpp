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
#include "platcomm.h"
#include "ISO14496_12.h"
#include "DataUtil.h"
#include <new>

extern std::map<std::string, std::string, CaseInsensitiveComparator> g_params;
extern int g_verbose_level;

using VLC_ITEM = std::tuple<int64_t, uint8_t, uint64_t>;
using Huffman_Codebook = std::vector<VLC_ITEM>;

using VLC_ITEM_QUAD = std::tuple<std::tuple<int64_t, int, int, int>, uint8_t, uint64_t>;
using Spectrum_Huffman_Codebook_Quad = std::vector<VLC_ITEM_QUAD>;

using VLC_ITEM_PAIR = std::tuple<std::tuple<int64_t, int, int>, uint8_t, uint64_t>;
using Sepctrum_Huffman_Codebook_Pair = std::vector<VLC_ITEM_PAIR>;

/*
	value-length-codeword
*/
Huffman_Codebook Scalefactor_Huffman_Codebook = {
	{ 0   ,18 ,0x3ffe8 },	{ 1   ,18 ,0x3ffe6 },	{ 2   ,18 ,0x3ffe7 },	{ 3   ,18 ,0x3ffe5 },	{ 4   ,19 ,0x7fff5 },	{ 5   ,19 ,0x7fff1 },
	{ 6   ,19 ,0x7ffed },	{ 7   ,19 ,0x7fff6 },	{ 8   ,19 ,0x7ffee },	{ 9   ,19 ,0x7ffef },	{ 10  ,19 ,0x7fff0 },	{ 11  ,19 ,0x7fffc },
	{ 12  ,19 ,0x7fffd },	{ 13  ,19 ,0x7ffff },	{ 14  ,19 ,0x7fffe },	{ 15  ,19 ,0x7fff7 },	{ 16  ,19 ,0x7fff8 },	{ 17  ,19 ,0x7fffb },
	{ 18  ,19 ,0x7fff9 },	{ 19  ,18 ,0x3ffe4 },	{ 20  ,19 ,0x7fffa },	{ 21  ,18 ,0x3ffe3 },	{ 22  ,17 ,0x1ffef },	{ 23  ,17 ,0x1fff0 },
	{ 24  ,16 ,0xfff5 },	{ 25  ,17 ,0x1ffee },	{ 26  ,16 ,0xfff2 },	{ 27  ,16 ,0xfff3 },	{ 28  ,16 ,0xfff4 },	{ 29  ,16 ,0xfff1 },
	{ 30  ,15 ,0x7ff6 },	{ 31  ,15 ,0x7ff7 },	{ 32  ,14 ,0x3ff9 },	{ 33  ,14 ,0x3ff5 },	{ 34  ,14 ,0x3ff7 },	{ 35  ,14 ,0x3ff3 },
	{ 36  ,14 ,0x3ff6 },	{ 37  ,14 ,0x3ff2 },	{ 38  ,13 ,0x1ff7 },	{ 39  ,13 ,0x1ff5 },	{ 40  ,12 ,0xff9 },		{ 41  ,12 ,0xff7 },
	{ 42  ,12 ,0xff6 },		{ 43  ,11 ,0x7f9 },		{ 44  ,12 ,0xff4 },		{ 45  ,11 ,0x7f8 },		{ 46  ,10 ,0x3f9 },		{ 47  ,10 ,0x3f7 },
	{ 48  ,10 ,0x3f5 },		{ 49  ,9  ,0x1f8 },		{ 50  ,9  ,0x1f7 },		{ 51  ,8  ,0xfa },		{ 52  ,8  ,0xf8 },		{ 53  ,8  ,0xf6 },
	{ 54  ,7  ,0x79 },		{ 55  ,6  ,0x3a },		{ 56  ,6  ,0x38 },		{ 57  ,5  ,0x1a },		{ 58  ,4  ,0xb },   	{ 59  ,3  ,0x4 },
	{ 60  ,1  ,0x0 },		{ 61  ,4  ,0xa },		{ 62  ,4  ,0xc },		{ 63  ,5  ,0x1b },		{ 64  ,6  ,0x39 },		{ 65  ,6  ,0x3b },
	{ 66  ,7  ,0x78 },		{ 67  ,7  ,0x7a },		{ 68  ,8  ,0xf7 },		{ 69  ,8  ,0xf9 },		{ 70  ,9  ,0x1f6 },		{ 71  ,9  ,0x1f9 },
	{ 72  ,10 ,0x3f4 },		{ 73  ,10 ,0x3f6 },		{ 74  ,10 ,0x3f8 },		{ 75  ,11 ,0x7f5 },		{ 76  ,11 ,0x7f4 },		{ 77  ,11 ,0x7f6 },
	{ 78  ,11 ,0x7f7 },		{ 79  ,12 ,0xff5 },		{ 80  ,12 ,0xff8 },		{ 81  ,13 ,0x1ff4 },	{ 82  ,13 ,0x1ff6 },	{ 83  ,13 ,0x1ff8 },
	{ 84  ,14 ,0x3ff8 },	{ 85  ,14 ,0x3ff4 },	{ 86  ,16 ,0xfff0 },	{ 87  ,15 ,0x7ff4 },	{ 88  ,16 ,0xfff6 },	{ 89  ,15 ,0x7ff5 },
	{ 90  ,18 ,0x3ffe2 },	{ 91  ,19 ,0x7ffd9 },	{ 92  ,19 ,0x7ffda },	{ 93  ,19 ,0x7ffdb },	{ 94  ,19 ,0x7ffdc },	{ 95  ,19 ,0x7ffdd },
	{ 96  ,19 ,0x7ffde },	{ 97  ,19 ,0x7ffd8 },	{ 98  ,19 ,0x7ffd2 },	{ 99  ,19 ,0x7ffd3 },	{ 100 ,19 ,0x7ffd4 },	{ 101 ,19 ,0x7ffd5 },
	{ 102 ,19 ,0x7ffd6 },	{ 103 ,19 ,0x7fff2 },	{ 104 ,19 ,0x7ffdf },	{ 105 ,19 ,0x7ffe7 },	{ 106 ,19 ,0x7ffe8 },	{ 107 ,19 ,0x7ffe9 },
	{ 108 ,19 ,0x7ffea },	{ 109 ,19 ,0x7ffeb },	{ 110 ,19 ,0x7ffe6 },	{ 111 ,19 ,0x7ffe0 },	{ 112 ,19 ,0x7ffe1 },	{ 113 ,19 ,0x7ffe2 },
	{ 114 ,19 ,0x7ffe3 },	{ 115 ,19 ,0x7ffe4 },	{ 116 ,19 ,0x7ffe5 },	{ 117 ,19 ,0x7ffd7 },	{ 118 ,19 ,0x7ffec },	{ 119 ,19 ,0x7fff4 },
	{ 120 ,19 ,0x7fff3 },
};

/*
	AAC spec 
	Table 4.151 - Spectrum Huffman codebooks parameters
	Codebook number, i	unsigned_cb[i]	Dimension of codebook	LAV for codebook	Codebook listed in Table
			0				-					-						0					-
			1				0					4						1					Table 4.A.2
			2				0					4						1					Table 4.A.3
			3				1					4						2					Table 4.A.4
			4				1					4						2					Table 4.A.5
			5				0					2						4					Table 4.A.6
			6				0					2						4					Table 4.A.7
			7				1					2						7					Table 4.A.8
			8				1					2						7					Table 4.A.9
			9				1					2						12					Table 4.A.10
			10				1					2						12					Table 4.A.11
			11				1					2					16 (with ESC 8191)		Table 4.A.12
			12				-					-					(reserved)					  -
			13				-					-			perceptual noise substitution		  -
			14				-					-				intensity out-of-phase			  -
			15				-					-				intensity in-phase				  -
			16				1					2					16 (w/o ESC 15)			Table 4.A.12
			17				1					2					16 (with ESC 31)		Table 4.A.12
			18				1					2					16 (with ESC 47)		Table 4.A.12
			19				1					2					16 (with ESC 63)		Table 4.A.12
			20				1					2					16 (with ESC 95)		Table 4.A.12
			21				1					2					16 (with ESC 127)		Table 4.A.12
			22				1					2					16 (with ESC 159)		Table 4.A.12
			23				1					2					16 (with ESC 191)		Table 4.A.12
			24				1					2					16 (with ESC 223)		Table 4.A.12
			25				1					2					16 (with ESC 255)		Table 4.A.12
			26				1					2					16 (with ESC 319)		Table 4.A.12
			27				1					2					16 (with ESC 383)		Table 4.A.12
			28				1					2					16 (with ESC 511)		Table 4.A.12
			29				1					2					16 (with ESC 767)		Table 4.A.12
			30				1					2					16 (with ESC 1023)		Table 4.A.12
			31				1					2					16 (with ESC 2047)		Table 4.A.12
*/
std::tuple<bool, int8_t, uint8_t> spectrum_hcb_params[] = {
	{ false, -1, 0 },{ false, 4, 1 },{ false, 4, 1 },{ true, 4, 2 },
	{ true, 4, 2 },{ false, 2, 4 },{ false, 2, 4 },{ true, 2, 7 },
	{ true, 2, 7 },{ true, 2, 12 },{ true, 2, 12 },{ true, 2, 16 },
	{ false, -1, 0 },{ false, -1, 0 },{ false, -1, 0 },{ false, -1, 0 },
	{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },
	{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },
	{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },
	{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },{ true, 2, 16 },
};

bool GetSpectrumQuad(int spectrum_hcb_idx, int value, int& w, int& x, int& y, int& z)
{
	if (spectrum_hcb_idx < 0 || spectrum_hcb_idx >= (int)_countof(spectrum_hcb_params))
		return false;

	if (std::get<1>(spectrum_hcb_params[spectrum_hcb_idx]) != 4)
		return false;

	uint16_t lav = std::get<2>(spectrum_hcb_params[spectrum_hcb_idx]);
	uint16_t mod, off;
	if (std::get<0>(spectrum_hcb_params[spectrum_hcb_idx]))
	{
		mod = lav + 1;
		off = 0;
	}
	else
	{
		mod = 2 * lav + 1;
		off = lav;
	}

	int idx = value;
	w = (int)(idx / (mod*mod*mod)) - off;
	idx -= (w + off)*(mod*mod*mod);
	x = (int)(idx / (mod*mod)) - off;
	idx -= (x + off)*(mod*mod);
	y = (int)(idx / mod) - off;
	idx -= (y + off)*mod;
	z = idx - off;

	return true;
}

bool GetSpectrumPair(int spectrum_hcb_idx, int value, int& y, int& z)
{
	if (spectrum_hcb_idx < 0 || spectrum_hcb_idx >= (int)_countof(spectrum_hcb_params))
		return false;

	if (std::get<1>(spectrum_hcb_params[spectrum_hcb_idx]) != 2)
		return false;

	uint16_t lav = std::get<2>(spectrum_hcb_params[spectrum_hcb_idx]);
	uint16_t mod, off;
	if (std::get<0>(spectrum_hcb_params[spectrum_hcb_idx]))
	{
		mod = lav + 1;
		off = 0;
	}
	else
	{
		mod = 2 * lav + 1;
		off = lav;
	}

	int idx = value;
	y = (int)(idx / mod) - off;
	idx -= (y + off)*mod;
	z = idx - off;

	return true;
}

struct HCOD_TREE
{
	HCOD_TREE*	left = nullptr;
	HCOD_TREE*	right = nullptr;
	HCOD_TREE*	parent = nullptr;

	int64_t		value = 0;
	uint8_t		length = 0;
	uint8_t		reserved[3] = { 0, 0, 0};
	uint64_t	codeword = 0;

	static int	spectrum_hcb_idx;

	HCOD_TREE(int64_t v, uint8_t bit_len, uint64_t cw)
		: value(v), length(bit_len), codeword(cw) {
	}

	virtual ~HCOD_TREE()
	{
		if (left != nullptr)
		{
			delete left;
			left = nullptr;
		}

		if (right != nullptr)
		{
			delete right;
			right = nullptr;
		}
	}

	void CalculateCodeWord(uint8_t& len, uint64_t& cw)
	{
		len = 0;
		cw = 0ULL;
		for (auto node = this; node->parent != nullptr; len++, node = node->parent)
		{
			if (node->parent->right == node)
				cw = (1ULL << len) | cw;
		}
	}

	auto AddChild(bool bLeft, int64_t v, uint8_t bit_length, uint64_t cw)
	{
		HCOD_TREE* child = bLeft ? left : right;

		if (child == nullptr)
		{
			child = new HCOD_TREE(v, bit_length, cw);
			child->parent = this;

			if (bLeft)
				left = child;
			else
				right = child;
		}

		return child;
	}

	int Verify()
	{
		uint8_t len = 0;
		uint64_t cw = 0;
		CalculateCodeWord(len, cw);

		if (len != length || cw != codeword)
			return false;

		return true;
	}

	int GetChildrenCount()
	{
		int nChildren = 0;
		if (left != nullptr)
		{
			nChildren++;
			nChildren += left->GetChildrenCount();
		}

		if (right != nullptr)
		{
			nChildren++;
			nChildren += right->GetChildrenCount();
		}

		return nChildren;
	}

	int GetMaxDepth()
	{
		int nMaxDepth = 1;

		int nLeftDepth = 0;
		if (left != nullptr)
			nLeftDepth = left->GetMaxDepth();

		int nRightDepth = 0;
		if (right != nullptr)
			nRightDepth = right->GetMaxDepth();

		return nMaxDepth + AMP_MAX(nLeftDepth, nRightDepth);
	}

	int64_t GetMaxValue() {
		int64_t nMaxValue = INT64_MIN;

		if (left == nullptr && right == nullptr)
			nMaxValue = AMP_MAX(nMaxValue, value);
		else
		{
			if (left != nullptr)
			{
				auto n = left->GetMaxValue();
				nMaxValue = AMP_MAX(n, nMaxValue);
			}

			if (right != nullptr)
			{
				auto n = right->GetMaxValue();
				nMaxValue = AMP_MAX(n, nMaxValue);
			}
		}

		return nMaxValue;
	}

	int64_t GetMinValue() {
		int64_t nMinValue = INT64_MAX;

		if (left == nullptr && right == nullptr)
			nMinValue = AMP_MIN(nMinValue, value);
		else
		{
			if (left != nullptr)
			{
				auto n = left->GetMinValue();
				nMinValue = AMP_MIN(n, nMinValue);
			}

			if (right != nullptr)
			{
				auto n = right->GetMinValue();
				nMinValue = AMP_MIN(n, nMinValue);
			}
		}

		return nMinValue;
	}

	void Print(FILE* fp=stdout)
	{
		size_t line_chars = (size_t)length * 5 + 2048;
		char* szLine = new char[line_chars];
		memset(szLine, ' ', line_chars);

		const int indent = 2;
		const int level_span = 5;

		char* szText = nullptr;
		if (length >= 1)
		{
			HCOD_TREE* ptr_parent = parent;
			memcpy(szLine + indent + ((ptrdiff_t)length - 1)*level_span, "|--", 3);
			for (int i = length - 2; i >= 0 && ptr_parent != nullptr; i--)
			{
				if ((ptr_parent->codeword&0x1) == 0)
					memcpy(szLine + indent + (ptrdiff_t)i*level_span, "|", 1);
				ptr_parent = ptr_parent->parent;
			}
			szText = szLine + indent + 3 + ((ptrdiff_t)length - 1)*level_span;
		}
		else
			szText = szLine + indent;

		if (parent == nullptr)
			sprintf_s(szText, line_chars - (szText - szLine), ".\n");
		else
		{
			int cbWritten = sprintf_s(szText, line_chars - (szText - szLine), "%" PRIX64 "h", codeword);
			szText += cbWritten;

			if (left == nullptr && right == nullptr)
			{
				bool bGotValueLayout = false;
				if (spectrum_hcb_idx >= 0 && spectrum_hcb_idx < (int)_countof(spectrum_hcb_params))
				{
					if (std::get<1>(spectrum_hcb_params[spectrum_hcb_idx]) == 4)
					{
						int w, x, y, z;
						if (GetSpectrumQuad(spectrum_hcb_idx, (int)value, w, x, y, z))
						{
							cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " ((value: %" PRIi64 ", w: %d, x: %d, y: %d, z: %d), length: %d)", 
								value, w, x, y, z, length);
							bGotValueLayout = true;
						}
					}
					else if (std::get<1>(spectrum_hcb_params[spectrum_hcb_idx]) == 2)
					{
						int y, z;
						if (GetSpectrumPair(spectrum_hcb_idx, (int)value, y, z))
						{
							cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " ((value: %" PRIi64 ", y: %d, z: %d), length: %d)", value, y, z, length);
							bGotValueLayout = true;
						}
					}
				}
			
				if (!bGotValueLayout)
					cbWritten = sprintf_s(szText, line_chars - (szText - szLine), " (value: %" PRIi64 ", length: %d)", value, length);

				szText += cbWritten;
			}

			sprintf_s(szText, line_chars - (szText - szLine), "\n");
		}

		fprintf(fp, "%s", szLine);

		delete[] szLine;

		if (left != nullptr)
			left->Print(fp);

		if (right != nullptr)
			right->Print(fp);

		return;
	}
};

int HCOD_TREE::spectrum_hcb_idx = -1;

HCOD_TREE* LoadHuffmanTree(Huffman_Codebook& hc)
{
	HCOD_TREE* htree = new HCOD_TREE(0, 0, 0);
	for (size_t i = 0; i < hc.size(); i++)
	{
		uint64_t cw = 0;
		auto node = htree;
		auto bit_len = std::get<1>(hc[i]);
		for (uint8_t j = 1; j <= bit_len; j++)
		{
			uint8_t bitset = (std::get<2>(hc[i]) & (1ULL << (bit_len - j))) ? 1 : 0;
			cw = (cw<<1) | bitset;
			node = node->AddChild(bitset?false:true, j == bit_len ? std::get<0>(hc[i]) : 0, j, cw);
		}
	}

	return htree;
}

bool LoadVLCTable(const char* szHeaderFileName, INT_VALUE_LITERAL_FORMAT fmts[3], Huffman_Codebook& VLC_tables)
{
	bool bRet = false;
	FILE* fp = nullptr;
	char* file_buf = nullptr;
	int64_t file_size;

	auto is_delimiter = [](char c) {
		const static char delimiters[] = { ' ', '\t', '\r', '\n', ';', ',', '/', '\\',  ':' };
		for (size_t i = 0; i < _countof(delimiters); i++) {
			if (delimiters[i] == c)
				return true;
		}

		return false;
	};

	if (fopen_s(&fp, szHeaderFileName, "rb") != 0 || fp == nullptr)
	{
		printf("Failed to open the file: %s.\n", szHeaderFileName);
		goto done;
	}

	if (_fseeki64(fp, 0, SEEK_END) != 0)
	{
		printf("Failed to get file size.\n");
		goto done;
	}

	file_size = _ftelli64(fp);
	if (file_size <= 0 || file_size > INT32_MAX)
	{
		printf("The file size(%" PRIi64 ") is unexpected.\n", file_size);
		goto done;
	}

	_fseeki64(fp, 0, SEEK_SET);
	file_buf = new (std::nothrow) char[(size_t)file_size];
	if (file_buf == nullptr)
	{
		printf("Failed to allocate the memory with %" PRIi64 " bytes.\n", file_size);
		goto done;
	}

	if (g_verbose_level > 0)
		printf("begin load VLC tables...\n");

	{
		size_t cbRead = fread(file_buf, 1, (size_t)file_size, fp);

		char* p = file_buf;
		char* file_buf_end = file_buf + cbRead;
		while (p < file_buf_end)
		{
			// Try to get 3 numbers
			int64_t vals[3];
			char* pstart, *pend;
			int i = 0;
			for (; i < 3; i++)
			{
				while (p < file_buf_end && is_delimiter(*p))p++;

				if (p >= file_buf_end)
					break;

				pstart = p;

				p++;
				while (p < file_buf_end && !is_delimiter(*p))p++;

				pend = p;

				if (ConvertToInt(pstart, pend, vals[i], fmts[i]) == false)
				{
					std::string str(pstart, pend);
					printf("Hit an invalid number value: %s\n", str.c_str());
					goto done;
				}

				if (g_verbose_level > 0)
					printf(i == 2 ? "%" PRIX64 "\t" : "%" PRIi64 "\t", vals[i]);
			}
			if (g_verbose_level > 0)
				printf("\n");

			if (i < 3)
				break;

			if (vals[1] <= 0 || ((uint64_t)vals[1] > sizeof(uint64_t) << 3))
			{
				printf("Unexpected bit-length value: %" PRIi64 ".\n", vals[1]);
				goto done;
			}

			VLC_tables.push_back(std::make_tuple(vals[0], (uint8_t)vals[1], (uint64_t)vals[2]));
		}

		bRet = true;
	}

done:
	if (fp != nullptr)
		fclose(fp);

	if (file_buf != nullptr)
		delete[] file_buf;

	return bRet;
}

void GenerateHuffmanBinarySearchArray(const char* szHeaderFileName, INT_VALUE_LITERAL_FORMAT fmts[3], const char* szOutputFile = nullptr, int spectrum_hcb_idx = -1)
{
	FILE* wfp = nullptr;
	HCOD_TREE* htree = nullptr;
	Huffman_Codebook VLC_tables;
	std::vector<HCOD_TREE*> nodes;
	std::vector<uint8_t> layer_node_count;

	HCOD_TREE::spectrum_hcb_idx = spectrum_hcb_idx;

	if (szOutputFile != nullptr)
	{
		if (fopen_s(&wfp, szOutputFile, "wb") != 0)
		{
			printf("Failed to open the output file: %s.\n", szOutputFile);
			goto done;
		}
	}

	if (szHeaderFileName != nullptr)
	{
		// Try to load VLC table into the memory

		if (LoadVLCTable(szHeaderFileName, fmts, VLC_tables) == false)
		{
			printf("Failed to load Huffman VLC codebook from the file: %s.\n", szHeaderFileName);
			goto done;
		}

		htree = LoadHuffmanTree(VLC_tables);
	}
	else
		htree = LoadHuffmanTree(Scalefactor_Huffman_Codebook);

	{
		// Push every node to a array
		int nTreeNodesCount = htree->GetChildrenCount() + 1;
		nodes.reserve(nTreeNodesCount);

		nodes.push_back(htree);

		size_t nPos = 0, nNextPos = nodes.size();
		layer_node_count.push_back(1);
		while (nNextPos > nPos)
		{
			layer_node_count.push_back(0);
			for (size_t i = nPos; i < nNextPos; i++)
			{
				if (nodes[i]->left)
				{
					nodes.push_back(nodes[i]->left);
					layer_node_count.back()++;
				}

				if (nodes[i]->right)
				{
					nodes.push_back(nodes[i]->right);
					layer_node_count.back()++;
				}
			}

			nPos = nNextPos;
			nNextPos = nodes.size();
		}

		if (g_verbose_level > 0)
			printf("Generate a Huffman VLC binary search array for VLC decoding:\n");

		int64_t VMax = htree->GetMaxValue();
		int64_t VMin = htree->GetMinValue();

		FILE* outfile = wfp == nullptr ? stdout : wfp;

		const char* strVType = "";
		if (VMin >= 0)
		{
			if (VMax <= (int64_t)UINT8_MAX)
				strVType = "uint8_t";
			else if (VMax <= (int64_t)UINT16_MAX)
				strVType = "uint16_t";
			else if (VMax <= (int64_t)UINT32_MAX)
				strVType = "uint32_t";
			else
				strVType = "int64_t";
		}
		else
		{
			if (VMin >= (int64_t)INT8_MIN && VMax <= (int64_t)INT8_MAX)
				strVType = "int8_t";
			else if (VMin >= (int64_t)INT16_MIN && VMax <= (int64_t)INT16_MAX)
				strVType = "int16_t";
			else if (VMin >= (int64_t)INT32_MIN && VMax <= (int64_t)INT32_MAX)
				strVType = "int32_t";
			else
				strVType = "int64_t";
		}

		fprintf(outfile, "%s hcb[][2] = {\n", strVType);
		char szRecord[256];

		int iFirst = 1, iSecond = 2;
		int iLayerPos = 0, nextLayerPos = 0;
		uint8_t cur_layer = 0;
		for (size_t i = 0; i < nodes.size(); i++)
		{
			if (cur_layer != nodes[i]->length)
			{
				iLayerPos = 0;
				nextLayerPos = 0;
				cur_layer = nodes[i]->length;
			}

			iFirst = layer_node_count[nodes[i]->length] - iLayerPos + nextLayerPos;
			iSecond = iFirst + 1;
			int cbWritten = 0;
			if (nodes[i]->left == nullptr && nodes[i]->right == nullptr)
				cbWritten = sprintf_s(szRecord, _countof(szRecord), "    {%" PRIi64 ", 0},", nodes[i]->value);
			else
			{
				if (nodes[i]->left != nullptr)
					nextLayerPos++;

				if (nodes[i]->right != nullptr)
					nextLayerPos++;

				cbWritten = sprintf_s(szRecord, _countof(szRecord), "    {%d, %d},", iFirst, iSecond);
			}

			if (cbWritten <= 40 && cbWritten > 0){
				for (int c = 0; c < 40 - cbWritten; c++)
					szRecord[cbWritten + c] = ' ';
			}
			szRecord[40] = '\0';

			if (nodes[i]->left == nullptr && nodes[i]->right == nullptr)
				fprintf(outfile, "%s// leaf node (V:%" PRIi64 ", L:%d C:0X%" PRIX64 ")\n", szRecord, nodes[i]->value, nodes[i]->length, nodes[i]->codeword);
			else
				fprintf(outfile, "%s\n", szRecord);

			iLayerPos++;
		}

		fprintf(outfile, "};\n");
	}

done:
	if (wfp != nullptr)
		fclose(wfp);

	if (htree != nullptr)
		delete htree;

	HCOD_TREE::spectrum_hcb_idx = -1;
}

void PrintHuffmanTree(const char* szHeaderFileName, INT_VALUE_LITERAL_FORMAT fmts[3], const char* szOutputFile = nullptr, int spectrum_hcb_idx = -1)
{
	FILE* wfp = NULL, *outfile = stdout;
	HCOD_TREE* htree = nullptr;
	Huffman_Codebook VLC_tables;

	HCOD_TREE::spectrum_hcb_idx = spectrum_hcb_idx;

	if (szOutputFile != nullptr)
	{
		if (fopen_s(&wfp, szOutputFile, "wb") != 0)
		{
			printf("Failed to open the output file: %s.\n", szOutputFile);
			goto done;
		}

		outfile = wfp;
	}

	if (szHeaderFileName != nullptr)
	{
		// Try to load VLC table into the memory
		if (LoadVLCTable(szHeaderFileName, fmts, VLC_tables) == false)
		{
			printf("Failed to load Huffman VLC codebook from the file: %s.\n", szHeaderFileName);
			goto done;
		}

		htree = LoadHuffmanTree(VLC_tables);
	}
	else
		htree = LoadHuffmanTree(Scalefactor_Huffman_Codebook);

	htree->Print(outfile);

done:
	if (wfp != nullptr)
		fclose(wfp);

	if (htree != nullptr)
		delete htree;

	HCOD_TREE::spectrum_hcb_idx = -1;
}

void GenerateSourceCode(const char* szHeaderFileName, INT_VALUE_LITERAL_FORMAT fmts[3], const char* szOutputFile = nullptr, int spectrum_hcb_idx = -1)
{
	FILE* wfp = NULL, *outfile = stdout;
	HCOD_TREE* htree = nullptr;
	Huffman_Codebook VLC_tables;
	std::vector<HCOD_TREE*> nodes;
	std::vector<uint8_t> layer_node_count;

	int Dimension = -1;
	char szFmtStr[256];
	int VLC_items_per_line = 5;
	int nMaxCharsOfValue = 0, nMaxCharsOfLength = 0, nMaxCharsOfCodeBits = 0;
	int nMaxCharsOfW = 0, nMaxCharsOfX = 0, nMaxCharsOfY = 0, nMaxCharsOfZ = 0;

	HCOD_TREE::spectrum_hcb_idx = spectrum_hcb_idx;

	if (szOutputFile != nullptr)
	{
		if (fopen_s(&wfp, szOutputFile, "wb") != 0)
		{
			printf("Failed to open the output file: %s.\n", szOutputFile);
			goto done;
		}

		outfile = wfp;
	}

	if (szHeaderFileName != nullptr)
	{
		// Try to load VLC table into the memory
		if (LoadVLCTable(szHeaderFileName, fmts, VLC_tables) == false)
		{
			printf("Failed to load Huffman VLC codebook from the file: %s.\n", szHeaderFileName);
			goto done;
		}

		htree = LoadHuffmanTree(VLC_tables);
	}
	else
		htree = LoadHuffmanTree(Scalefactor_Huffman_Codebook);

	// Print the codebook with the fixed format order by the value
	std::sort(VLC_tables.begin(), VLC_tables.end(), [](const VLC_ITEM& a, const VLC_ITEM& b) ->bool {
		return std::get<0>(a) < std::get<0>(b);
	});

	if (spectrum_hcb_idx >= 0 && spectrum_hcb_idx < (int)_countof(spectrum_hcb_params))
	{
		if (std::get<1>(spectrum_hcb_params[spectrum_hcb_idx]) == 4)
			Dimension = 4;
		else if (std::get<1>(spectrum_hcb_params[spectrum_hcb_idx]) == 2)
			Dimension = 2;
	}

	for (size_t i = 0; i < VLC_tables.size(); i++)
	{
		int cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%" PRIu64 "", std::get<0>(VLC_tables[i]));
		if (nMaxCharsOfValue < cbWritten)
			nMaxCharsOfValue = cbWritten;

		if (Dimension == 4)
		{
			int w, x, y, z;
			if (GetSpectrumQuad(spectrum_hcb_idx, (int)std::get<0>(VLC_tables[i]), w, x, y, z))
			{
				cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%d", w);
				if (nMaxCharsOfW < cbWritten)
					nMaxCharsOfW = cbWritten;

				cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%d", x);
				if (nMaxCharsOfX < cbWritten)
					nMaxCharsOfX = cbWritten;

				cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%d", y);
				if (nMaxCharsOfY < cbWritten)
					nMaxCharsOfY = cbWritten;

				cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%d", z);
				if (nMaxCharsOfZ < cbWritten)
					nMaxCharsOfZ = cbWritten;
			}
		}
		else if (Dimension == 2)
		{
			int y, z;
			if (GetSpectrumPair(spectrum_hcb_idx, (int)std::get<0>(VLC_tables[i]), y, z))
			{
				cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%d", y);
				if (nMaxCharsOfY < cbWritten)
					nMaxCharsOfY = cbWritten;

				cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%d", z);
				if (nMaxCharsOfZ < cbWritten)
					nMaxCharsOfZ = cbWritten;
			}
		}

		cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%" PRIu8 "", std::get<1>(VLC_tables[i]));
		if (nMaxCharsOfLength < cbWritten)
			nMaxCharsOfLength = cbWritten;

		cbWritten = sprintf_s(szFmtStr, _countof(szFmtStr), "%" PRIx64 "", std::get<2>(VLC_tables[i]));
		if (nMaxCharsOfCodeBits < cbWritten)
			nMaxCharsOfCodeBits = cbWritten;
	}

	fprintf(outfile, "\n");

	if (Dimension == 4)
	{
		fprintf(outfile, "using VLC_ITEM_QUAD = std::tuple<std::tuple<int64_t, int, int, int, int>, uint8_t, uint64_t>;\n");
		fprintf(outfile, "using Spectrum_Huffman_Codebook_Quad = std::vector<VLC_ITEM_QUAD>;\n");
		sprintf_s(szFmtStr, _countof(szFmtStr), "{ {%%%d" PRIu64 ", %%%dd, %%%dd, %%%dd, %%%dd}, %%%d" PRIu8 ", 0x%%-%d" PRIx64 "},",
			nMaxCharsOfValue, nMaxCharsOfW, nMaxCharsOfX, nMaxCharsOfY, nMaxCharsOfZ, nMaxCharsOfLength, nMaxCharsOfCodeBits);
		fprintf(outfile, "\nSpectrum_Huffman_Codebook_Quad VLC_tables_quad = {");
		VLC_items_per_line = 2;
	}
	else if (Dimension == 2)
	{
		fprintf(outfile, "using VLC_ITEM_PAIR = std::tuple<std::tuple<int64_t, int, int>, uint8_t, uint64_t>;\n");
		fprintf(outfile, "using Sepctrum_Huffman_Codebook_Pair = std::vector<VLC_ITEM_PAIR>;\n");
		sprintf_s(szFmtStr, _countof(szFmtStr), "{ {%%%d" PRIu64 ", %%%dd, %%%dd}, %%%d" PRIu8 ", 0x%%-%d" PRIx64 "},",
			nMaxCharsOfValue, nMaxCharsOfY, nMaxCharsOfZ, nMaxCharsOfLength, nMaxCharsOfCodeBits);
		fprintf(outfile, "\nSpectrum_Huffman_Codebook_Pair VLC_tables_pair = {");
		VLC_items_per_line = 3;
	}
	else
	{
		fprintf(outfile, "using VLC_ITEM = std::tuple<int64_t, uint8_t, uint64_t>;\n");
		fprintf(outfile, "using Huffman_Codebook = std::vector<VLC_ITEM>;\n");
		sprintf_s(szFmtStr, _countof(szFmtStr), "{%%%d" PRIu64 ", %%%d" PRIu8 ", 0x%%-%d" PRIx64 "},", nMaxCharsOfValue, nMaxCharsOfLength, nMaxCharsOfCodeBits);
		fprintf(outfile, "\nHuffman_Codebook VLC_tables = {");
	}

	for (size_t i = 0; i < VLC_tables.size(); i++)
	{
		if (i%VLC_items_per_line == 0)
			fprintf(outfile, "\n    ");
		else
			fprintf(outfile, " ");

		if (Dimension == 4)
		{
			int w, x, y, z;
			GetSpectrumQuad(spectrum_hcb_idx, (int)std::get<0>(VLC_tables[i]), w, x, y, z);
			fprintf(outfile, (const char*)szFmtStr, std::get<0>(VLC_tables[i]), w, x, y, z, std::get<1>(VLC_tables[i]), std::get<2>(VLC_tables[i]));
		}
		else if (Dimension == 2)
		{
			int y, z;
			GetSpectrumPair(spectrum_hcb_idx, (int)std::get<0>(VLC_tables[i]), y, z);
			fprintf(outfile, (const char*)szFmtStr, std::get<0>(VLC_tables[i]), y, z, std::get<1>(VLC_tables[i]), std::get<2>(VLC_tables[i]));
		}
		else
			fprintf(outfile, (const char*)szFmtStr, std::get<0>(VLC_tables[i]), std::get<1>(VLC_tables[i]), std::get<2>(VLC_tables[i]));
	}
	fprintf(outfile, "\n};\n\n");

	{
		// Push every node to a array
		int nTreeNodesCount = htree->GetChildrenCount() + 1;
		nodes.reserve(nTreeNodesCount);

		nodes.push_back(htree);

		size_t nPos = 0, nNextPos = nodes.size();
		layer_node_count.push_back(1);
		while (nNextPos > nPos)
		{
			layer_node_count.push_back(0);
			for (size_t i = nPos; i < nNextPos; i++)
			{
				if (nodes[i]->left)
				{
					nodes.push_back(nodes[i]->left);
					layer_node_count.back()++;
				}

				if (nodes[i]->right)
				{
					nodes.push_back(nodes[i]->right);
					layer_node_count.back()++;
				}
			}

			nPos = nNextPos;
			nNextPos = nodes.size();
		}

		int64_t VMax = htree->GetMaxValue();
		int64_t VMin = htree->GetMinValue();

		FILE* outfile = wfp == nullptr ? stdout : wfp;

		const char* strVType = "";
		if (VMin >= 0)
		{
			if (VMax <= (int64_t)UINT8_MAX)
				strVType = "uint8_t";
			else if (VMax <= (int64_t)UINT16_MAX)
				strVType = "uint16_t";
			else if (VMax <= (int64_t)UINT32_MAX)
				strVType = "uint32_t";
			else
				strVType = "int64_t";
		}
		else
		{
			if (VMin >= (int64_t)INT8_MIN && VMax <= (int64_t)INT8_MAX)
				strVType = "int8_t";
			else if (VMin >= (int64_t)INT16_MIN && VMax <= (int64_t)INT16_MAX)
				strVType = "int16_t";
			else if (VMin >= (int64_t)INT32_MIN && VMax <= (int64_t)INT32_MAX)
				strVType = "int32_t";
			else
				strVType = "int64_t";
		}

		fprintf(outfile, "%s hcb[][2] = {\n", strVType);
		char szRecord[256];

		int iFirst = 1, iSecond = 2;
		int iLayerPos = 0, nextLayerPos = 0;
		uint8_t cur_layer = 0;
		for (size_t i = 0; i < nodes.size(); i++)
		{
			if (cur_layer != nodes[i]->length)
			{
				iLayerPos = 0;
				nextLayerPos = 0;
				cur_layer = nodes[i]->length;
			}

			// Find the index that huffman leaf value lies at
			size_t idxVLCTable = 0;
			for (; idxVLCTable < VLC_tables.size(); idxVLCTable++)
				if (std::get<0>(VLC_tables[idxVLCTable]) == nodes[i]->value)
					break;

			if (idxVLCTable >= VLC_tables.size())
			{
				printf("Failed to find value:%" PRIi64 " in the VLC tables.\n", nodes[i]->value);
				break;
			}

			iFirst = layer_node_count[nodes[i]->length] - iLayerPos + nextLayerPos;
			iSecond = iFirst + 1;
			int cbWritten = 0;
			if (nodes[i]->left == nullptr && nodes[i]->right == nullptr)
				cbWritten = sprintf_s(szRecord, _countof(szRecord), "    {%zu, 0},", idxVLCTable);
			else
			{
				if (nodes[i]->left != nullptr)
					nextLayerPos++;

				if (nodes[i]->right != nullptr)
					nextLayerPos++;

				cbWritten = sprintf_s(szRecord, _countof(szRecord), "    {%d, %d},", iFirst, iSecond);
			}

			if (cbWritten <= 40 && cbWritten > 0) {
				for (int c = 0; c < 40 - cbWritten; c++)
					szRecord[cbWritten + c] = ' ';
			}
			szRecord[40] = '\0';

			if (nodes[i]->left == nullptr && nodes[i]->right == nullptr)
				fprintf(outfile, "%s// leaf node (V-index:%zu(V: %" PRIi64 ") L:%d C:0X%" PRIX64 ")\n", szRecord, idxVLCTable, nodes[i]->value, nodes[i]->length, nodes[i]->codeword);
			else
				fprintf(outfile, "%s\n", szRecord);

			iLayerPos++;
		}

		fprintf(outfile, "};\n");
	}

done:
	if (wfp != nullptr)
		fclose(wfp);

	if (htree != nullptr)
		delete htree;

	HCOD_TREE::spectrum_hcb_idx = -1;
}

int DumpHuffmanCodeBook()
{
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string szOutputFile;
	std::string szOutputFmt;
	std::string szVLCTypes = "aaa";	// x: hex, d: dec, o: oct, b: bin, a: auto
	std::string& szInputFile = g_params["input"];

	if (g_params.find("outputfmt") != g_params.end())
		szOutputFmt = g_params["outputfmt"];

	if (g_params.find("output") != g_params.end())
		szOutputFile = g_params["output"];

	if (g_params.find("VLCTypes") != g_params.end())
		szVLCTypes = g_params["VLCTypes"];

	INT_VALUE_LITERAL_FORMAT fmts[3] = { FMT_AUTO, FMT_AUTO, FMT_AUTO };

	for (size_t i = 0; i < szVLCTypes.size(); i++)
	{
		switch (szVLCTypes[i])
		{
		case 'a': fmts[i] = FMT_AUTO; break;
		case 'h': fmts[i] = FMT_HEX; break;
		case 'd': fmts[i] = FMT_HEX; break;
		case 'o': fmts[i] = FMT_OCT; break;
		case 'b': fmts[i] = FMT_BIN; break;
		}
	}

	if (szOutputFmt.empty())
	{
		PrintHuffmanTree(szInputFile.c_str(), fmts, szOutputFile.empty() ? nullptr : szOutputFile.c_str());
		return RET_CODE_SUCCESS;
	}
	else if (szOutputFmt.compare("binary_search_table") == 0)
	{
		GenerateHuffmanBinarySearchArray(szInputFile.c_str(), fmts, szOutputFile.empty() ? nullptr : szOutputFile.c_str());
		return RET_CODE_SUCCESS;
	}
	else if (szOutputFmt.compare("sourcecode") == 0)
	{
		GenerateSourceCode(szInputFile.c_str(), fmts, szOutputFile.empty() ? nullptr : szOutputFile.c_str());
		return RET_CODE_SUCCESS;
	}

	return -1;
}

int DumpSpectrumHuffmanCodeBook(int nCodebookNumber)
{
	if (g_params.find("input") == g_params.end())
		return -1;

	std::string szOutputFile;
	std::string szOutputFmt;
	std::string szVLCTypes = "aaa";	// x: hex, d: dec, o: oct, b: bin, a: auto
	std::string& szInputFile = g_params["input"];

	if (g_params.find("outputfmt") != g_params.end())
		szOutputFmt = g_params["outputfmt"];

	if (g_params.find("output") != g_params.end())
		szOutputFile = g_params["output"];

	if (g_params.find("VLCTypes") != g_params.end())
		szVLCTypes = g_params["VLCTypes"];

	INT_VALUE_LITERAL_FORMAT fmts[3] = { FMT_AUTO, FMT_AUTO, FMT_AUTO };

	for (size_t i = 0; i < szVLCTypes.size(); i++)
	{
		switch (szVLCTypes[i])
		{
		case 'a': fmts[i] = FMT_AUTO; break;
		case 'h': fmts[i] = FMT_HEX; break;
		case 'd': fmts[i] = FMT_HEX; break;
		case 'o': fmts[i] = FMT_OCT; break;
		case 'b': fmts[i] = FMT_BIN; break;
		}
	}

	if (szOutputFmt.empty())
	{
		PrintHuffmanTree(szInputFile.c_str(), fmts, szOutputFile.empty() ? nullptr : szOutputFile.c_str(), nCodebookNumber);
		return RET_CODE_SUCCESS;
	}
	else if (szOutputFmt.compare("binary_search_table") == 0)
	{
		GenerateHuffmanBinarySearchArray(szInputFile.c_str(), fmts, szOutputFile.empty() ? nullptr : szOutputFile.c_str());
		return RET_CODE_SUCCESS;
	}
	else if (szOutputFmt.compare("sourcecode") == 0)
	{
		GenerateSourceCode(szInputFile.c_str(), fmts, szOutputFile.empty() ? nullptr : szOutputFile.c_str(), nCodebookNumber);
		return RET_CODE_SUCCESS;
	}

	return -1;
}
