#include "StdAfx.h"
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
	{ "LFE", "Low-frequency effects", "" },
	{ "LFE2", "Secondary low-frequency effects", "" },
};

std::string GetChannelMappingDesc(unsigned long channel_mapping)
{
	std::string sDesc;
	int num_ch = 0, lfe = 0;
	for (int i = 0; i < sizeof(channel_descs) / sizeof(channel_descs[0]); i++)
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
	/* CH_LOC_LFE,			*/	SPEAKER_POS_LOW_FREQUENCY,
	/* CH_LOC_LFE2,			*/	SPEAKER_POS_LOW_FREQUENCY,
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

