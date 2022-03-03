/*

MIT License

Copyright (c) 2022 Ravin.Wang(wangf1978@hotmail.com)

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
#include "AudChMap.h"


std::tuple<std::string, std::string, std::string> channel_descs[22] =
{
	{ "L", "Left", "A loudspeaker position behind the screen to the left edge, horizontally, of the screen center as viewed from the seating area" },
	{ "C", "Center", "" },
	{ "R", "Right", "" },
	{ "Ls", "Left Surround", "" },
	{ "Rs", "Right Surround", "" },
	{ "Lc", "Left Center", "" },
	{ "Rc", "Right Center", "" },
	{ "Lrs", "Left Rear Surround", "" },
	{ "Rrs", "Right Rear Surround", "" },
	{ "Cs", "Center Surround", "" },
	{ "Ts", "Top Center Surround", "" },
	{ "Lsd", "Left Surround Direct", "" },
	{ "Rsd", "Right Surround Direct", "" },
	{ "Lw", "Left Wide", "" },
	{ "Rw", "Right Wide", "" },
	{ "Vhl", "Vertical Height Left", "" },
	{ "Vhr", "Vertical Height Right", "" },
	{ "Vhc", "Vertical Height Center", "" },
	{ "Lts", "Left Top Surround", "" },
	{ "Rts", "Right Top Surround", "" },
	{ "LFE2", "Secondary low-frequency effects", "" },
	{ "LFE", "Low-frequency effects", "" },
};

const char* channel_mapping_category_names[] =
{
	"D-Cinema",
	"DTS/DTSHD",
	"DRA/DRA Extension",
	"ITU-R BS",
	"",
	"",
	"",
	"",
};

using CH_LOC_DESC = std::tuple<std::string/*label*/, std::string/*name*/, std::string/*explanation*/>;

CH_LOC_DESC dcinema_channel_descs[] =
{
	{ "L",		"Left",						"A loudspeaker position behind the screen to the left edge, horizontally, of the screen center as viewed from the seating area" },
	{ "C",		"Center",					"" },
	{ "R",		"Right",					"" },
	{ "Ls",		"Left Surround",			"" },
	{ "Rs",		"Right Surround",			"" },
	{ "LFE",	"Low-frequency effects",	"" },
	{ "Rls",	"Rear surround left",		"" },
	{ "Cs",		"Center Surround",			"" },
	{ "Rrs",	"Rear surround right",		"" },
	{ "Lc",		"Left Center",				"" },
	{ "Rc",		"Right Center",				"" },
	{ "Vhl",	"Vertical height left",		"" },
	{ "Vhc",	"Vertical height center",	"" },
	{ "Vhr",	"Vertical height right",	"" },
	{ "Ts",		"Top center surround",		"" },
	{ "Lw",		"Left wide",				"" },
	{ "Rw",		"Right wide",				"" },
	{ "Lsd",	"Left surround direct",		"" },
	{ "Rsd",	"Right surround direct",	"" },
	{ "LFE2",	"LFE 2", ""},
};

CH_LOC_DESC dts_channel_descs[] =
{
	{ "C",		"Centre in front of listener (0)",					""},
	{ "L",		"Left in front (-30)",								""},
	{ "R",		"Right in front (30)",								""},
	{ "Ls",		"Left surround on side in rear (-110)",				""},
	{ "Rs",		"Right surround on side in rear (110)",				""},
	{ "LFE1",	"Low frequency effects subwoofer",					""},
	{ "Cs",		"Centre surround in rear (180)",					""},
	{ "LSr",	"Left surround in rear (-150)",						""},
	{ "Rsr",	"Right surround in rear (150)",						""},
	{ "Lss",	"Left surround on side (-90)",						""},
	{ "Rss",	"Right surround on side (90)",						""},
	{ "Lc",		"Between left and centre in front (-15)",			""},
	{ "Rc",		"Between right and centre in front (15)",			""},
	{ "Lh",		"Left height in front",								""},
	{ "Ch",		"Centre Height in front",							""},
	{ "Rh",		"Right Height in front",							""},
	{ "LFE2",	"Second low frequency effects subwoofer",			""},
	{ "Lw",		"Left on side in front (-60)",						""},
	{ "Rw",		"Right on side in front (60)",						""},
	{ "Oh",		"Over the listener's head",							""},
	{ "Lhs",	"Left height on side",								""},
	{ "Rhs",	"Right height on side",								""},
	{ "Chr",	"Centre height in rear",							""},
	{ "Lhr",	"Left height in rear",								""},
	{ "Rhr",	"Right height in rear",								""},
	{ "Cl",		"Centre in the plane lower then listener's ears",	""},
	{ "Ll",		"Left in the plane lower then listener's ears",		""},
	{ "Rl",		"Right in the plane lower then listener's ears",	""},
};

CH_LOC_DESC dra_channel_descs[] =
{
	{ "Front Left",		"Front Left",	""},
	{ "Front Center",	"Front Center",	""},
	{ "Front Right",	"Front Right",	""},
	{ "Rear Left",		"Rear Left",	""},
	{ "Rear Right",		"Rear Right",	""},
	{ "LFE",			"LFE",			""},
	{ "Side Left",		"Side Left",	""},
	{ "Rear Center",	"Rear Center",	""},
	{ "Side Right",		"Side Right",	""},
};

CH_LOC_DESC bs_channel_descs[] =
{
	{ "FL",		"Front left",			"Front left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-left front on Middle layer"},
	{ "FR",		"Front right",			"Front right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-right front on Middle layer"},
	{ "FC",		"Front centre",			"Front centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the centre front(straight ahead) on Middle layer"},
	{ "LFE1",	"LFE1",					"LFE channel of the 22.2 multichannel sound system, which is reproduced from a specific low frequency loudspeaker located at the far-left front on Bottom layer"},
	{ "BL",		"Back left",			"Back left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-left back on Middle layer"},
	{ "BR",		"Back right",			"Back right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-right back on Middle layer"},
	{ "FLc",	"Front left centre",	"Front left centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the mid-way between the front centre and the front left on Middle layer"},
	{ "FRc",	"Front right centre",	"Front right centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the mid-way between the front centre and the front right on Middle layer"},
	{ "BC",		"Back centre",			"Back centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the centre back(straight behind) on Middle layer"},
	{ "LFE2",	"LFE2",					"LFE channel of the 22.2 multichannel sound system, which is reproduced from a specific low frequency loudspeaker located at the far-right front on Bottom layer"},
	{ "SiL",	"Side left",			"Side left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the left side on Middle layer"},
	{ "SiR",	"Side right",			"Side right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the right side on Middle layer"},
	{ "TpFL",	"Top front left",		"Top front left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-left front on Top layer"},
	{ "TpFR",	"Top front right",		"Top front right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-right front on Top layer"},
	{ "TpFC",	"Top front centre",		"Top front centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the centre front(straight ahead) on Top layer"},
	{ "TpC",	"Top centre",			"Top centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the centre on Top layer just above the seating position"},
	{ "TpBL",	"Top back left",		"Top back left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-left back on Top layer"},
	{ "TpBR",	"Top back right",		"Top back right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-right back on Top layer"},
	{ "TpSiL",	"Top side left",		"Top side left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the left side on Top layer"},
	{ "TpSiR",	"Top side right",		"Top side right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the right side on Top layer"},
	{ "TpBC",	"Top back centre",		"Top back centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the centre back(straight behind) on Top layer"},
	{ "BtFC",	"Bottom front centre",	"Bottom front centre channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the centre front(straight ahead) on Bottom layer"},
	{ "BtFL",	"Bottom front left",	"Bottom front left channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-left front on Bottom layer"},
	{ "BtFR",	"Bottom front right",	"Bottom front right channel of the 22.2 multichannel sound system, which is reproduced from a loudspeaker located at the far-right front on Bottom layer"},
	{ "L",		"Left",					"Left channel of the 2 channel stereo system, the 5.1 channel stereophonic sound system, or the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the left front on Middle layer"},
	{ "R",		"Right",				"Right channel of the 2 channel stereo system, the 5.1 channel stereophonic sound system, or the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the right front on Middle layer"},
	{ "C",		"Centre",				"Centre channel of the 5.1 channel stereophonic sound system, or the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the centre front(straight ahead) on Middle layer"},
	{ "LFE",	"LFE",					"LFE channel of the 5.1 channel stereophonic sound system, or the 7.1 multichannel sound system, which is reproduced from a specific low frequency loudspeaker"},
	{ "Ls",		"Left surround",		"Left surround channel of the 5.1 channel stereophonic sound system, or the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the left back on Middle layer"},
	{ "Rs",		"Right surround",		"Right surround channel of the 5.1 channel stereophonic sound system, or the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the right back on Middle layer"},
	{ "Ltf",	"Left top front",		"Left top front channel of the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the left front on Top layer"},
	{ "Rtf",	"Right top front",		"Right top front channel of the 7.1 multichannel sound system, which is reproduced from a loudspeaker located at the right front on Top layer"},
};

std::string GetChannelMappingDesc(unsigned long channel_mapping)
{
	std::string sDesc;
	int num_ch = 0, lfe = 0;
	for (size_t i = 0; i < _countof(channel_descs); i++)
	{
		if (channel_mapping&(1 << i))
		{
			if (sDesc.length() > 0)
				sDesc += ", ";
			sDesc += std::get<0>(channel_descs[i]);
			if (i == CH_LOC_LFE || i == CH_LOC_LFE2)
				lfe++;
			else
				num_ch++;
		}
	}

	std::string sChDesc;
	if (num_ch == 2 && lfe == 0 && CHANNLE_PRESENT(channel_mapping, CH_LOC_DUALMONO))
	{
		sChDesc = "2ch (Dual-Mono)";
	}
	else
	{
		sChDesc = std::to_string(num_ch);
		if (lfe > 0)
		{
			sChDesc += ".";
			sChDesc += std::to_string(lfe);
		}

		sChDesc += "ch(";
		sChDesc += sDesc;
		sChDesc += ")";
	}

	return sChDesc;
}

unsigned long acmod_ch_assignments[8] =
{
	/* 00000b(0x00) */		(unsigned long)(CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_DUALMONO)),
	/* 00000b(0x01) */		CHANNEL_BITMASK(CH_LOC_CENTER),
	/* 00000b(0x02) */		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT),
	/* 00000b(0x03) */		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT),
	/* 00000b(0x04) */		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS),
	/* 00000b(0x05) */		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS),
	/* 00000b(0x06) */		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 00000b(0x07) */		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
};

unsigned long ddp_ch_assignment[16] =
{
	/*0  Left			*/	CHANNEL_BITMASK(CH_LOC_LEFT),
	/*1  Center			*/	CHANNEL_BITMASK(CH_LOC_CENTER),
	/*2  Right			*/	CHANNEL_BITMASK(CH_LOC_RIGHT),
	/*3  Left Surround	*/	CHANNEL_BITMASK(CH_LOC_LS),
	/*4  Right Surround	*/	CHANNEL_BITMASK(CH_LOC_RS),
	/*5  Lc / Rc pair	*/	CHANNEL_BITMASK(CH_LOC_LC) | CHANNEL_BITMASK(CH_LOC_RC),
	/*6  Lrs / Rrs pair	*/	CHANNEL_BITMASK(CH_LOC_LRS) | CHANNEL_BITMASK(CH_LOC_RRS),
	/*7  Cs				*/	CHANNEL_BITMASK(CH_LOC_CS),
	/*8  Ts				*/	CHANNEL_BITMASK(CH_LOC_TS),
	/*9  Lsd / Rsd pair	*/	CHANNEL_BITMASK(CH_LOC_LSD) | CHANNEL_BITMASK(CH_LOC_RSD),
	/*10 Lw / Rw pair	*/	CHANNEL_BITMASK(CH_LOC_LW) | CHANNEL_BITMASK(CH_LOC_RW),
	/*11 Vhl / Vhr pair	*/	CHANNEL_BITMASK(CH_LOC_VHL) | CHANNEL_BITMASK(CH_LOC_VHR),
	/*12 Vhc			*/	CHANNEL_BITMASK(CH_LOC_VHC),
	/*13 Lts / Rts pair	*/	CHANNEL_BITMASK(CH_LOC_LTS) | CHANNEL_BITMASK(CH_LOC_RTS),
	/*14 LFE2			*/	CHANNEL_BITMASK(CH_LOC_LFE2),
	/*15 LFE			*/	CHANNEL_BITMASK(CH_LOC_LFE),
};

/*
Notation       Loudspeaker Location Description             Bit Mask      Number of Channels
--------------------------------------------------------------------------------------------
C              Centre in front of listener                    0x0001		1
LR             Left / Right in front                          0x0002		2
LsRs           Left / Right surround on side in rear          0x0004		2
LFE1           Low frequency effects subwoofer                0x0008		1
Cs             Centre surround in rear                        0x0010		1
LhRh           Left / Right height in front                   0x0020		2
LsrRsr         Left / Right surround in rear                  0x0040		2
Ch             Centre Height in front                         0x0080		1
Oh             Over the listener's head                       0x0100		1
LcRc           Between left / right and centre in front       0x0200		2
LwRw           Left / Right on side in front                  0x0400		2
LssRss         Left / Right surround on side                  0x0800		2
LFE2           Second low frequency effects subwoofer         0x1000		1
LhsRhs         Left / Right height on side                    0x2000		2
Chr            Centre height in rear                          0x4000		1
LhrRhr         Left / Right height in rear                    0x8000		2
*/
std::tuple<std::string/*Notation*/, std::string/*Desc*/, uint32_t/*ch_loc*/, uint8_t> dtshd_speaker_bitmask_table[16]
{
	{ "C",		"Centre in front of listener",				CHANNEL_BITMASK(CH_LOC_CENTER),									1 },
	{ "LR",		"Left / Right in front",					CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT),	2 },
	{ "LsRs",	"Left / Right surround on side in rear",	CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),		2 },
	{ "LFE1",	"Low frequency effects subwoofer",			CHANNEL_BITMASK(CH_LOC_LFE),									1 },
	{ "Cs",		"Centre surround in rear",					CHANNEL_BITMASK(CH_LOC_CS),										1 },
	{ "LhRh",	"Left / Right height in front",				CHANNEL_BITMASK(CH_LOC_VHL) | CHANNEL_BITMASK(CH_LOC_VHR),		2 },
	{ "LsrRrs", "Left / Right surround in rear",			CHANNEL_BITMASK(CH_LOC_LRS) | CHANNEL_BITMASK(CH_LOC_RRS),		2 },
	{ "Ch",		"Centre Height in front",					CHANNEL_BITMASK(CH_LOC_VHC),									1 },
	{ "Oh",		"Over the listener's head",					CHANNEL_BITMASK(CH_LOC_CENTER),									1 },
	{ "LcRc",	"Between left / right and centre in front", CHANNEL_BITMASK(CH_LOC_TS),										2 },
	{ "LwRw",	"Left / Right on side in front",			CHANNEL_BITMASK(CH_LOC_LW) | CHANNEL_BITMASK(CH_LOC_RW),		2 },
	{ "LssRss", "Left / Right surround on side",			CHANNEL_BITMASK(CH_LOC_CENTER),									2 },
	{ "LFE2",	"Second low frequency effects subwoofer",	CHANNEL_BITMASK(CH_LOC_LFE2),									1 },
	{ "LhsRhs", "Left / Right height on side",				CHANNEL_BITMASK(CH_LOC_LHS) | CHANNEL_BITMASK(CH_LOC_RHS),		2 },
	{ "Chr",	"Centre height in rear",					CHANNEL_BITMASK(CH_LOC_CHR),									1 },
	{ "LhrRhr", "Left / Right height in rear",				CHANNEL_BITMASK(CH_LOC_LHR) | CHANNEL_BITMASK(CH_LOC_RHR),		2 },
};

std::tuple<uint32_t, std::vector<uint8_t>> hdmv_lpcm_ch_assignments[16] =
{
	/*
	0	-				reserved
	---------------------------------------------------------------------------------------------------
	1	2 ch	mono											M1
	2			reserved
	3			stereo											L	R
	---------------------------------------------------------------------------------------------------
	4	4 ch	L, C, R(3 / 0)									L	R	C
	5			L, R, S(2 / 1)									L	R	S
	6			L,C,R,S(3 / 1)									L	R	C	S
	7			L,R,LS,RS(2 / 2)								L	R	LS	RS
	---------------------------------------------------------------------------------------------------
	8	6 ch	L, C, R, LS, RS(3 / 2)							L	R	C	LS	RS
	9			L, C, R, LS, RS, lfe(3 / 2 + lfe)				L	R	C	LS	RS	lfe
	---------------------------------------------------------------------------------------------------
	10	8 ch	L, C, R, LS, Rls, Rrs, RS (3 / 4)				L	R	C	LS	Rls	Rrs	RS
	11			L, C, R, LS, Rls, Rrs, RS, lfe(3 / 4 + lfe)		L	R	C	LS	Rls	Rrs	RS	lfe
	*/
	{0, {} },
	{
		CHANNEL_BITMASK(CH_LOC_CENTER), 
		{CH_LOC_CENTER}
	},
	{0, {} },
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT),
		{ CH_LOC_LEFT, CH_LOC_RIGHT}
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT), 
		{CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_CENTER}
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS),
		{CH_LOC_LEFT, CH_LOC_RIGHT,CH_LOC_LS }
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS),
		{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_CENTER, CH_LOC_LS }
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
		{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_LOC_RS }
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
		{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_CENTER, CH_LOC_LS, CH_LOC_RS }
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS) | CHANNEL_BITMASK(CH_LOC_LFE),
		{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_CENTER, CH_LOC_LS, CH_LOC_RS, CH_LOC_LFE }
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS) | CHANNEL_BITMASK(CH_LOC_LRS) | CHANNEL_BITMASK(CH_LOC_RRS),
		{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_CENTER, CH_LOC_LS, CH_LOC_LRS, CH_LOC_RRS, CH_LOC_RS}
	},
	{
		CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS) | CHANNEL_BITMASK(CH_LOC_LRS) | CHANNEL_BITMASK(CH_LOC_RRS) | CHANNEL_BITMASK(CH_LOC_LFE),
		{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_CENTER, CH_LOC_LS, CH_LOC_LRS, CH_LOC_RRS, CH_LOC_RS, CH_LOC_LFE}
	},
	{0, {} },
	{0, {} },
	{0, {} },
	{0, {} }
};

unsigned long FBB_Channel_assignments[21] = {
	/* 00000b(0x00)  */CHANNEL_BITMASK(CH_LOC_CENTER),
	/* 00001b(0x01)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT),
	/* 00010b(0x02)  */(unsigned long)-1,
	/* 00011b(0x03)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 00100b(0x04)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LFE),
	/* 00101b(0x05)  */(unsigned long)-1,
	/* 00110b(0x06)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LFE) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 00111b(0x07)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER),
	/* 01000b(0x08)  */(unsigned long)-1,
	/* 01001b(0x09)  */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 01011b(0x0A)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LFE),
	/* 01010b(0x0B)  */(unsigned long)-1,
	/* 01011b(0x0C)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LFE) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 01100b(0x0D)  */(unsigned long)-1,
	/* 01110b(0x0E)o */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 01111b(0x0F)  */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LFE),
	/* 10000b(0x10)  */(unsigned long)-1,
	/* 10001b(0x11)  */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LFE) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	/* 10010b(0x12)  */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS) | CHANNEL_BITMASK(CH_LOC_LFE),
	/* 10011b(0x13)  */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS) | CHANNEL_BITMASK(CH_LOC_CENTER),
	/* 10100b(0x14)  */CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT) | CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS) | CHANNEL_BITMASK(CH_LOC_CENTER) | CHANNEL_BITMASK(CH_LOC_LFE) ,
};

unsigned long FBA_Channel_Loc_mapping_0[13] = {
	/*
	8ch_presentation_ channel_assignment bit	Exists									Number of channels
	0											Left / Right										2
	1											Centre												1
	2											LFE													1
	3											Left Surround / Right Surround(Ls / Rs)				2
	4											Left Front Height / Right Front Height(Lvh / Rvh)	2
	5											Left Centre / Right Centre(Lc / Rc)					2
	6											Left Rear Surround / Right Rear Surround(Lrs / Rrs)	2
	7											Centre Surround(Cs)									1
	8											Top Surround(Ts)									1
	9											Left Surround Direct/Right Surround Direct(Lsd/Rsd)	2
	10											Left Wide / Right Wide(Lw / Rw)						2
	11											Centre Front Height(Cvh)							1
	12											LFE2												1
	*/
	CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT),
	CHANNEL_BITMASK(CH_LOC_CENTER),
	CHANNEL_BITMASK(CH_LOC_LFE),
	CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	CHANNEL_BITMASK(CH_LOC_VHL) | CHANNEL_BITMASK(CH_LOC_VHR),
	CHANNEL_BITMASK(CH_LOC_LC) | CHANNEL_BITMASK(CH_LOC_RC),
	CHANNEL_BITMASK(CH_LOC_LRS) | CHANNEL_BITMASK(CH_LOC_RRS),
	CHANNEL_BITMASK(CH_LOC_CS),
	CHANNEL_BITMASK(CH_LOC_TS),
	CHANNEL_BITMASK(CH_LOC_LSD) | CHANNEL_BITMASK(CH_LOC_RSD),
	CHANNEL_BITMASK(CH_LOC_LW) | CHANNEL_BITMASK(CH_LOC_RW),
	CHANNEL_BITMASK(CH_LOC_VHC),
	CHANNEL_BITMASK(CH_LOC_LFE2)
};

unsigned long FBA_Channel_Loc_mapping_1[5] = {
	/*
	8ch_presentation_ channel_assignment bit	Exists									Number of channels
	0											Left/Right										2
	1											Centre											1
	2											LFE												1
	3											Left Surround/Right Surround (Ls/Rs)			2
	4											Left Top Surround/Right Top Surround (Lts/Rts)	2
	5 - 11										Reserved										N/A
	12											Reserved										N/A
	*/
	CHANNEL_BITMASK(CH_LOC_LEFT) | CHANNEL_BITMASK(CH_LOC_RIGHT),
	CHANNEL_BITMASK(CH_LOC_CENTER),
	CHANNEL_BITMASK(CH_LOC_LFE),
	CHANNEL_BITMASK(CH_LOC_LS) | CHANNEL_BITMASK(CH_LOC_RS),
	CHANNEL_BITMASK(CH_LOC_LTS) | CHANNEL_BITMASK(CH_LOC_RTS)
};

unsigned long Channel_Speaker_Mapping[] = {
	/* CH_LOC_LEFT = 0,		*/	SPEAKER_POS_FRONT_LEFT,
	/* CH_LOC_CENTER,		*/	SPEAKER_POS_FRONT_CENTER,
	/* CH_LOC_RIGHT,		*/	SPEAKER_POS_FRONT_RIGHT,
	/* CH_LOC_LS,			*/	SPEAKER_POS_SIDE_LEFT,
	/* CH_LOC_RS,			*/	SPEAKER_POS_SIDE_RIGHT,
	/* CH_LOC_LC,			*/	SPEAKER_POS_FRONT_LEFT_OF_CENTER,
	/* CH_LOC_RC,			*/	SPEAKER_POS_FRONT_RIGHT_OF_CENTER,
	/* CH_LOC_LRS,			*/	SPEAKER_POS_BACK_LEFT,
	/* CH_LOC_RRS,			*/	SPEAKER_POS_BACK_RIGHT,
	/* CH_LOC_CS,			*/	SPEAKER_POS_BACK_CENTER,
	/* CH_LOC_TS,			*/	SPEAKER_POS_TOP_CENTER,
	/* CH_LOC_LSD,			*/	SPEAKER_POS_BACK_LEFT,		// Not sure
	/* CH_LOC_RSD,			*/	SPEAKER_POS_BACK_RIGHT,		// Not sure
	/* CH_LOC_LW,			*/	0,						// Can't find speaker mapping
	/* CH_LOC_RW,			*/	0,						// Can't find speaker mapping
	/* CH_LOC_VHL,			*/	SPEAKER_POS_TOP_FRONT_LEFT,
	/* CH_LOC_VHR,			*/	SPEAKER_POS_TOP_FRONT_RIGHT,
	/* CH_LOC_VHC,			*/	SPEAKER_POS_TOP_FRONT_CENTER,
	/* CH_LOC_LTS,			*/	SPEAKER_POS_TOP_BACK_LEFT,
	/* CH_LOC_RTS,			*/	SPEAKER_POS_TOP_BACK_RIGHT,
	/* CH_LOC_LFE2,			*/	SPEAKER_POS_LOW_FREQUENCY,
	/* CH_LOC_LFE,			*/	SPEAKER_POS_LOW_FREQUENCY,
	/* CH_LOC_DUALMONO = 31	*/
};

std::vector<CHANNEL_LOC> dts_audio_channel_arragements[16] = {
	/*
	AMODE	CHS	Arrangement
	0b000000	1	A
	0b000001	2	A + B(dual mono)
	0b000010	2	L + R(stereo)
	0b000011	2	(L + R) + (L - R) (sum - difference)
	0b000100	2	LT + RT(left and right total)
	0b000101	3	C + L + R
	0b000110	3	L + R + S
	0b000111	4	C + L + R + S
	0b001000	4	L + R + SL + SR
	0b001001	5	C + L + R + SL + SR
	0b001010	6	CL + CR + L + R + SL + SR
	0b001011	6	C + L + R + LR + RR + OV
	0b001100	6	CF + CR + LF + RF + LR + RR
	0b001101	7	CL + C + CR + L + R + SL + SR
	0b001110	8	CL + CR + L + R + SL1 + SL2 + SR1 + SR2
	0b001111	8	CL + C + CR + L + R + SL + S + SR
	0b010000 - 0b111111		User defined
	NOTE : L = left, R = right, C = centre, S = surround, F = front, R = rear, T = total, OV = overhead, A = first mono channel, B = second mono channel.
	*/
	/*0b000000*/{ CH_MONO },
	/*0b000001*/{ CH_MONO, CH_MONO },
	/*0b000010*/{ CH_LOC_LEFT, CH_LOC_RIGHT },
	/*0b000011*/{ CH_LR_SUM, CH_LR_DIFF },
	/*0b000100*/{ CH_LT, CH_RT },
	/*0b000101*/{ CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT },
	/*0b000110*/{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_SURROUND },
	/*0b000111*/{ CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT, CH_SURROUND },
	/*0b001000*/{ CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_LOC_RS },
	/*0b001001*/{ CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_LOC_RS },
	/*0b001010*/{ CH_LOC_LC, CH_LOC_RC, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_LOC_RS },
	/*0b001011*/{ CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LRS, CH_LOC_RRS, CH_LOC_TS },
	/*0b001100*/{ CH_LOC_CENTER, CH_LOC_RC, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LRS, CH_LOC_RRS },
	/*0b001101*/{ CH_LOC_LC, CH_LOC_CENTER, CH_LOC_RC, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_LOC_RS },
	/*0b001110*/{ CH_LOC_LC, CH_LOC_RC, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_LOC_LS, CH_LOC_RS, CH_LOC_RS },
	/*0b001111*/{ CH_LOC_LC, CH_LOC_CENTER, CH_LOC_RC, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LS, CH_SURROUND, CH_LOC_RS },
};

std::vector<CHANNEL_LOC> aac_channel_configurations[8] = {
	/*
	Default bitstream	number of speakers 	audio syntactic elements, 	default element to 
	index number							listed in order received	speaker mapping
	1					1					single_channel_element		center front speaker
	2					2					channel_pair_element		left, right front speakers
	3					3					single_channel_element(), 
											channel_pair_element()		center front speaker/left, right front speakers
	4					4					single_channel_element(),
											channel_pair_element(),
											single_channel_element() 	center front speaker/left, right center front speakers, rear surround
	5					5					single_channel_element(), 
											channel_pair_element(), 
											channel_pair_element()		center front speaker/left, right front speakers, left surround, right surround rear speakers
	6					5+1					single_channel_element(), 
											channel_pair_element(), 
											channel_pair_element(),
											lfe _element()				center front speaker, left, right front speakers, 
																		left surround, right surround rear speakers, front low frequency effects speaker
	7					7+1					single_channel_element(),
											channel_pair_element(),
											channel_pair_element(),
											channel_pair_element(),
											lfe_element()				center front speaker, left, right center front speakers,
																		left, right outside front speakers,
																		left surround, right surround rear speakers, 
																		front low frequency effects speaker
	*/
	/* 0 */{},
	/* 1 */{CH_LOC_CENTER},
	/* 2 */{CH_LOC_LEFT, CH_LOC_RIGHT},
	/* 3 */{CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT},
	/* 4 */{CH_LOC_CENTER, CH_LOC_LC, CH_LOC_RC, CH_LOC_CS},
	/* 5 */{CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LRS, CH_LOC_RRS},
	/* 6 */{CH_LOC_CENTER, CH_LOC_LEFT, CH_LOC_RIGHT, CH_LOC_LRS, CH_LOC_RRS, CH_LOC_LFE},
	/* 7 */{CH_LOC_CENTER, CH_LOC_LC, CH_LOC_RC, CH_LOC_LS, CH_LOC_RS, CH_LOC_LRS, CH_LOC_RRS, CH_LOC_LFE}
};

std::vector<CHANNEL_LOC> mpega_channel_mode_layouts[4] = {
	/*
	Channel Mode
	00 - Stereo
	01 - Joint stereo (Stereo)
	10 - Dual channel (2 mono channels)
	11 - Single channel (Mono)
	*/
	{CH_LOC_LEFT, CH_LOC_RIGHT},
	{CH_LOC_LEFT, CH_LOC_RIGHT},
	{CH_MONO, CH_MONO},
	{CH_MONO}
};

void CH_MAPPING::clear()
{
	// backup the category at first
	uint64_t  backup_cat = cat;
	u64Val = backup_cat << 61;
}

int CH_MAPPING::set_dual_mono()
{
	int iRet = 0;
	uint64_t  backup_cat = cat;
	u64Val = (backup_cat << 61);
	dual_ch = 1;
	switch ((int)cat)
	{
	case CH_MAPPING_CAT_DCINEMA:
		C = 1;
		break;
	case CH_MAPPING_CAT_DTS:
		DTS.C = 1;
		break;
	case CH_MAPPING_CAT_DRA:
		DRA.Front_Center = 1;
		break;
	case CH_MAPPING_CAT_BS:
		BS.C = 1;
		break;
	default:
		iRet = -1;
		break;
	}
	return iRet;
}

bool CH_MAPPING::is_lfe(size_t i)
{
	if (cat == CH_MAPPING_CAT_DCINEMA)
		return i == DCINEMA_LFE || i == DCINEMA_LFE2;
	else if (cat == CH_MAPPING_CAT_DTS)
		return i == DTS_LFE1 || i == DTS_LFE2;
	else if (cat == CH_MAPPING_CAT_DRA)
		return i == DRA_LFE;
	else if (cat == CH_MAPPING_CAT_BS)
		return i == BS_LFE || i == BS_LFE1 || i == BS_LFE2;

	return false;
}

bool CH_MAPPING::is_dual_mono()
{
	if (cat == CH_MAPPING_CAT_DCINEMA)
		return (u64Val&DCINEMA_CH_LOC_BITMASK) == (1ULL << DCINEMA_C) && dual_ch && !MS;
	else if (cat == CH_MAPPING_CAT_DTS)
		return (u64Val&DTS_CH_LOC_BITMASK) == (1ULL << DTS_C) && dual_ch && !MS;
	else if (cat == CH_MAPPING_CAT_DRA)
		return (u64Val&DRA_CH_LOC_BITMASK) == (1ULL << DRA_Front_Center) && dual_ch && !MS;
	else if (cat == CH_MAPPING_CAT_BS)
		return (u64Val&BS_CH_LOC_BITMASK) == (1ULL << BS_C) && dual_ch && !MS;

	return false;
}

std::string CH_MAPPING::get_desc()
{
	std::string sDesc;
	int num_ch = 0, lfe = 0;

	CH_LOC_DESC* ch_loc_descs = NULL;
	size_t num_of_ch_loc_descs = 0;
	if (cat == CH_MAPPING_CAT_DCINEMA)
	{
		ch_loc_descs = dcinema_channel_descs;
		num_of_ch_loc_descs = _countof(dcinema_channel_descs);
	}
	else if (cat == CH_MAPPING_CAT_DTS)
	{
		ch_loc_descs = dts_channel_descs;
		num_of_ch_loc_descs = _countof(dts_channel_descs);
	}
	else if (cat == CH_MAPPING_CAT_DRA)
	{
		ch_loc_descs = dra_channel_descs;
		num_of_ch_loc_descs = _countof(dra_channel_descs);
	}
	else if (cat == CH_MAPPING_CAT_BS)
	{
		ch_loc_descs = bs_channel_descs;
		num_of_ch_loc_descs = _countof(bs_channel_descs);
	}
	else
		return "";

	for (size_t i = 0; i < num_of_ch_loc_descs; i++)
	{
		if (u64Val&(1ULL << i))
		{
			if (sDesc.length() > 0)
				sDesc += ", ";
			sDesc += std::get<0>(ch_loc_descs[i]);
			if (is_lfe(i))
				lfe++;
			else
				num_ch++;
		}
	}

	// Check the special channel location
	if (MS)
	{
		if (sDesc.length() > 0)
			sDesc += ", ";
		sDesc += "MS";
		num_ch++;
	}

	std::string sChDesc;
	if(num_ch == 1 && is_dual_mono())
	{
		sChDesc = "2ch (Dual-Mono)";
	}
	else
	{
		sChDesc = std::to_string(num_ch);
		if (lfe > 0)
		{
			sChDesc += ".";
			sChDesc += std::to_string(lfe);
		}

		sChDesc += "ch(";
		sChDesc += sDesc;
		if (channel_mapping_category_names[cat][0] != '\0')
		{
			sChDesc += "/";
			sChDesc += channel_mapping_category_names[cat];
		}
		sChDesc += ")";
	}

	return sChDesc;
}

