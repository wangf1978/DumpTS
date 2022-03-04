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

const char* channel_mapping_category_names[] =
{
	"",
	"D-Cinema",
	"DTS/DTSHD",
	"DRA/DRA Extension",
	"ITU-R BS",
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

CH_MAPPING acmod_ch_assignments[8] =
{
	/* 00000b(0x00) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C, CH_LOC_DUAL}),
	/* 00000b(0x01) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C}),
	/* 00000b(0x02) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R}),
	/* 00000b(0x03) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_C, DCINEMA_R}),
	/* 00000b(0x04) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, CH_LOC_MS}),
	/* 00000b(0x05) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_C, DCINEMA_R, CH_LOC_MS}),
	/* 00000b(0x06) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_Ls, DCINEMA_Rs}),
	/* 00000b(0x07) */	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_C, DCINEMA_R, DCINEMA_Ls, DCINEMA_Rs}),
};

CH_MAPPING ddp_ch_assignment[16] =
{
	/*0  Left			*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L}),
	/*1  Center			*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C}),
	/*2  Right			*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_R}),
	/*3  Left Surround	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Ls}),
	/*4  Right Surround	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Rs}),
	/*5  Lc / Rc pair	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Lc,DCINEMA_Rc}),
	/*6  Lrs / Rrs pair	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Rls,DCINEMA_Rrs}),
	/*7  Cs				*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Cs}),
	/*8  Ts				*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Ts}),
	/*9  Lsd / Rsd pair	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Lsd, DCINEMA_Rsd}),
	/*10 Lw / Rw pair	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Lw, DCINEMA_Rw}),
	/*11 Vhl / Vhr pair	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Vhl ,DCINEMA_Vhr}),
	/*12 Vhc			*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Vhc}),
	/*13 Lts / Rts pair	*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {ATMOS_Lts, ATMOS_Rts}),
	/*14 LFE2			*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_LFE2}),
	/*15 LFE			*/	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_LFE}),
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
std::tuple<std::string/*Notation*/, std::string/*Desc*/, CH_MAPPING/*ch_loc*/, uint8_t> dtshd_speaker_bitmask_table[16]
{
	{ "C",		"Centre in front of listener",				CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_C}),				1 },
	{ "LR",		"Left / Right in front",					CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_L, DTS_R}),			2 },
	{ "LsRs",	"Left / Right surround on side in rear",	CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Ls, DTS_Rs}),		2 },
	{ "LFE1",	"Low frequency effects subwoofer",			CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_LFE1}),				1 },
	{ "Cs",		"Centre surround in rear",					CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Cs}),				1 },
	{ "LhRh",	"Left / Right height in front",				CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lh, DTS_Rh}),		2 },
	{ "LsrRrs", "Left / Right surround in rear",			CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lsr, DTS_Rsr}),		2 },
	{ "Ch",		"Centre Height in front",					CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Ch}),				1 },
	{ "Oh",		"Over the listener's head",					CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Oh}),				1 },
	{ "LcRc",	"Between left / right and centre in front", CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lc, DTS_Rc}),		2 },
	{ "LwRw",	"Left / Right on side in front",			CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lw, DTS_Rw}),		2 },
	{ "LssRss", "Left / Right surround on side",			CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lss, DTS_Rss}),		2 },
	{ "LFE2",	"Second low frequency effects subwoofer",	CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_LFE2}),				1 },
	{ "LhsRhs", "Left / Right height on side",				CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lhs, DTS_Rhs}),		2 },
	{ "Chr",	"Centre height in rear",					CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Chr}),				1 },
	{ "LhrRhr", "Left / Right height in rear",				CH_MAPPING(CH_MAPPING_CAT_DTS, {DTS_Lhr, DTS_Rhr}),		2 },
};

std::tuple<CH_MAPPING, std::vector<uint8_t>> hdmv_lpcm_ch_assignments[16] =
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
	{CH_MAPPING(CH_MAPPING_CAT_DCINEMA), {} },
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C}),
		{DCINEMA_C}
	},
	{CH_MAPPING(CH_MAPPING_CAT_DCINEMA), {} },
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R}),
		{ DCINEMA_L, DCINEMA_R}
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C}),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_C}
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R,CH_LOC_MS }),
		{ DCINEMA_L, DCINEMA_R,CH_LOC_MS}
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R, DCINEMA_C, CH_LOC_MS }),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_C, CH_LOC_MS }
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R, DCINEMA_Ls, DCINEMA_Rs }),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_Ls, DCINEMA_Rs }
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs }),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs }
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs, DCINEMA_LFE }),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs, DCINEMA_LFE }
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rls, DCINEMA_Rrs, DCINEMA_Rs }),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rls, DCINEMA_Rrs, DCINEMA_Rs }
	},
	{
		CH_MAPPING(CH_MAPPING_CAT_DCINEMA, { DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rls, DCINEMA_Rrs, DCINEMA_Rs, DCINEMA_LFE }),
		{ DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rls, DCINEMA_Rrs, DCINEMA_Rs, DCINEMA_LFE }
	},
	{CH_MAPPING(CH_MAPPING_CAT_DCINEMA), {} },
	{CH_MAPPING(CH_MAPPING_CAT_DCINEMA), {} },
	{CH_MAPPING(CH_MAPPING_CAT_DCINEMA), {} },
	{CH_MAPPING(CH_MAPPING_CAT_DCINEMA), {} }
};

CH_MAPPING FBB_Channel_assignments[21] = {
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_LFE}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_LFE, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_LFE}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_LFE, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_LFE}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_LFE, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_LFE, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R, DCINEMA_C, DCINEMA_LFE, DCINEMA_Ls, DCINEMA_Rs}),
};

CH_MAPPING FBA_Channel_Loc_mapping_0[13] = {
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
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_LFE}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Vhl, DCINEMA_Vhr}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Lc, DCINEMA_Rc}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Rls, DCINEMA_Rrs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Cs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Ts}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Lsd, DCINEMA_Rsd}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Lw, DCINEMA_Rw}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Vhc}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_LFE2}),
};

CH_MAPPING FBA_Channel_Loc_mapping_1[5] = {
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
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_C}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_LFE}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_Ls, DCINEMA_Rs}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {ATMOS_Lts, ATMOS_Rts}),
};

uint32_t DCINEMA_Channel_Speaker_Mapping[] = {
	/* DCINEMA_L	*/	SPEAKER_POS_FRONT_LEFT,
	/* DCINEMA_C	*/	SPEAKER_POS_FRONT_CENTER,
	/* DCINEMA_R	*/	SPEAKER_POS_FRONT_RIGHT,
	/* DCINEMA_Ls	*/	SPEAKER_POS_SIDE_LEFT,
	/* DCINEMA_Rs	*/	SPEAKER_POS_SIDE_RIGHT,
	/* DCINEMA_LFE	*/	SPEAKER_POS_LOW_FREQUENCY,
	/* DCINEMA_Rls	*/	SPEAKER_POS_BACK_LEFT,
	/* DCINEMA_Cs	*/	SPEAKER_POS_BACK_CENTER,
	/* DCINEMA_Rrs	*/	SPEAKER_POS_BACK_RIGHT,
	/* DCINEMA_Lc	*/	SPEAKER_POS_FRONT_LEFT_OF_CENTER,
	/* DCINEMA_Rc	*/	SPEAKER_POS_FRONT_RIGHT_OF_CENTER,
	/* DCINEMA_Vhl	*/	SPEAKER_POS_TOP_FRONT_LEFT,
	/* DCINEMA_Vhc	*/	SPEAKER_POS_TOP_FRONT_CENTER,
	/* DCINEMA_Vhr	*/	SPEAKER_POS_TOP_FRONT_RIGHT,
	/* DCINEMA_Ts	*/	SPEAKER_POS_TOP_CENTER,
	/* DCINEMA_Lw	*/	UINT32_MAX,					// Can't find speaker mapping
	/* DCINEMA_Rw	*/	UINT32_MAX,					// Can't find speaker mapping
	/* DCINEMA_Lsd	*/	SPEAKER_POS_BACK_LEFT,		// Not sure
	/* DCINEMA_Rsd	*/	SPEAKER_POS_BACK_RIGHT,		// Not sure
	/* DCINEMA_LFE2 */	SPEAKER_POS_LOW_FREQUENCY,
	/* ATMOS_Lts	*/	SPEAKER_POS_TOP_BACK_LEFT,
	/* ATMOS_Rts	*/	SPEAKER_POS_TOP_BACK_RIGHT,
};

CH_MAPPING dts_audio_channel_arragements[16] = {
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
	/*0b000000*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C}),
	/*0b000001*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, CH_LOC_DUAL}),
	/*0b000010*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_L, DTS_R }),
	/*0b000011*/CH_MAPPING(CH_MAPPING_CAT_DTS, { CH_LOC_LR_SUM, CH_LOC_LR_DIFF }),
	/*0b000100*/CH_MAPPING(CH_MAPPING_CAT_DTS, { CH_LOC_LT, CH_LOC_RT }),
	/*0b000101*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, DTS_L, DTS_R }),
	/*0b000110*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_L, DTS_R, CH_LOC_MS }),
	/*0b000111*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, DTS_L, DTS_R, CH_LOC_MS }),
	/*0b001000*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_L, DTS_R, DTS_Ls, DTS_Rs }),
	/*0b001001*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, DTS_L, DTS_R, DTS_Ls, DTS_Rs }),
	/*0b001010*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_Lc, DTS_Rc, DTS_L, DTS_R, DTS_Ls, DTS_Rs }),
	/*0b001011*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, DTS_L, DTS_R, DTS_Lsr, DTS_Rsr, DTS_Oh }),
	/*0b001100*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, DTS_Rc, DTS_L, DTS_R, DTS_Lsr, DTS_Rsr }),
	/*0b001101*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_Lc, DTS_C, DTS_Rc, DTS_L, DTS_R, DTS_Ls, DTS_Rs }),
	/*0b001110*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_Lc, DTS_Rc, DTS_L, DTS_R, DTS_Ls, DTS_Ls, DTS_Rs, DTS_Rs }),
	/*0b001111*/CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_Lc, DTS_C, DTS_Rc, DTS_L, DTS_R, DTS_Ls, CH_LOC_MS, DTS_Rs }),
};

CH_MAPPING aac_channel_configurations[16] = {
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
	/* 0 */CH_MAPPING(CH_MAPPING_CAT_BS),
	/* 1 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_C}),
	/* 2 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_L, BS_R}),
	/* 3 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_C, BS_L, BS_R}),
	/* 4 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_C, BS_L, BS_R, CH_LOC_MS}),
	/* 5 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_C, BS_L, BS_R, BS_Ls, BS_Rs}),
	/* 6 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_C, BS_L, BS_R, BS_Ls, BS_Rs, BS_LFE}),
	/* 7 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_FC, BS_FLc, BS_FRc, BS_FL, BS_FR, BS_BL, BS_BR, BS_LFE}),
	/* 8 */CH_MAPPING(CH_MAPPING_CAT_BS),
	/* 9 */CH_MAPPING(CH_MAPPING_CAT_BS),
	/*10 */CH_MAPPING(CH_MAPPING_CAT_BS),
	/*11 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_FC, BS_FL, BS_FR, BS_BL, BS_BR, BS_BC, BS_LFE}),
	/*12 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_FC, BS_FL, BS_FR, BS_SiL, BS_SiR, BS_BL, BS_BR, BS_LFE}),
	/*13 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_FC, BS_FLc, BS_FRc, BS_FL, BS_FR, BS_SiL, BS_SiR, BS_BL, BS_BR, BS_BC, BS_LFE1, BS_LFE2, BS_TpFC, BS_TpFL, BS_TpFR, BS_TpSiL, BS_TpSiR, BS_TpC, BS_TpBL, BS_TpBR, BS_TpBC, BS_BtFC, BS_BtFL, BS_BtFR}),
	/*14 */CH_MAPPING(CH_MAPPING_CAT_BS, {BS_FC, BS_FL, BS_FR, BS_Ls, BS_Rs, BS_LFE, BS_TpFL, BS_TpFR}),
	/*15 */CH_MAPPING(CH_MAPPING_CAT_BS),
};

CH_MAPPING mpega_channel_mode_layouts[4] = {
	/*
	Channel Mode
	00 - Stereo
	01 - Joint stereo (Stereo)
	10 - Dual channel (2 mono channels)
	11 - Single channel (Mono)
	*/
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R}),
	CH_MAPPING(CH_MAPPING_CAT_DCINEMA, {DCINEMA_L, DCINEMA_R}),
	CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C, CH_LOC_DUAL}),
	CH_MAPPING(CH_MAPPING_CAT_DTS, { DTS_C}),
};

CH_MAPPING::CH_MAPPING(CH_MAPPING_CAT CAT, std::initializer_list<int> ch_locs)
{
	if (CAT >= CH_MAPPING_CAT_DCINEMA && CAT <= CH_MAPPING_CAT_MAX)
		cat = CAT;

	std::vector<int> locs = ch_locs;
	for (size_t i = 0; i < locs.size(); i++)
	{
		if (locs[i] >= 0 && locs[i] < CH_SPECIAL_LOC_MIN)
		{
			u64Val |= CHANNEL_BITMASK(locs[i]);
		}
		else
		{
			switch (locs[i])
			{
			case CH_LOC_DUAL: dual_ch = 1; break;
			case CH_LOC_MS: MS = 1; break;
			case CH_LOC_LR_SUM: LR_SUM = 1; break;
			case CH_LOC_LR_DIFF: LR_DIFF = 1; break;
			case CH_LOC_LT: LT = 1; break;
			case CH_LOC_RT: RT = 1; break;
			}
		}
	}
}

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

bool CH_MAPPING::is_invalid()
{
	return u64Val == 0 ? true : false;
}

bool CH_MAPPING::is_present(int ch_loc)
{
	if (ch_loc < 0 || ch_loc >= 64)
		return false;

	return CHANNLE_PRESENT(u64Val, ch_loc);
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
	if (dual_ch){
		num_ch++;
	}

	if (MS){
		if (sDesc.length() > 0)
			sDesc += ", ";
		sDesc += "MS";
		num_ch++;
	}

	if (LR_SUM) {
		if (sDesc.length() > 0)
			sDesc += ", ";
		sDesc += "L+R";
		num_ch++;
	}

	if (LR_DIFF) {
		if (sDesc.length() > 0)
			sDesc += ", ";
		sDesc += "L-R";
		num_ch++;
	}

	if (LT){
		if (sDesc.length() > 0)
			sDesc += ", ";
		sDesc += "LT";
		num_ch++;
	}

	if (RT) {
		if (sDesc.length() > 0)
			sDesc += ", ";
		sDesc += "RT";
		num_ch++;
	}

	std::string sChDesc;
	if(num_ch == 2 && is_dual_mono())
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
		if (channel_mapping_category_names[cat][0] != '\0')
		{
			sChDesc += "@";
			sChDesc += channel_mapping_category_names[cat];
		}
		
	}

	return sChDesc;
}

