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
#include <tuple>
#include <string>
#include <initializer_list>

/*
	Some basic concepts:
	Channel number: 
		the decoded audio sample output order, for example, ch#0, ch#1, ...
		the channel number should be consequently
	Channel location:
		It means which speaker the channel data are expected to be output in, it is different with speaker position.
		it may have mono or surround type of information
	Channel label/name:
		The friendly short channel label/name
	Speaker Position:
		The speaker location defined in SMPTE 428-3
	Platform output arrangement:
		The default channel output arrangement priority
		For example, in windows, the PCM output will be arranged with this below priority from higher to lower
		front_left, front_right, front center, LFE, ...
	output mask:
		It describes which speaker has audio sample to be output

	Some mappings:
	Channel Number -> Channel Location: specify which kind of audio channel signal data is placed in channel#n
	Channel Location -> Channel label/name: give the friendly short name for channel location
	Channel Location -> Speaker Position: give the mapping between channel location and speaker position


*/

// SMPTE Standard 428M (theater application)
// HDMV LPCM audio
// Dolby Digital, Dolby Digital Plus
// MLP from Dolby Lossless
enum DCINEMA_CH_LOC
{
	DCINEMA_L = 0,		// Left
	DCINEMA_C,			// Center
	DCINEMA_R,			// Right
	DCINEMA_Ls,			// Left Surround
	DCINEMA_Rs,			// Right Surround
	DCINEMA_LFE,		// LFE
	DCINEMA_Rls,		// Rear Surround left
	DCINEMA_Cs,			// Center Surround
	DCINEMA_Rrs,		// Real Surround right
	DCINEMA_Lc,			// Left Center
	DCINEMA_Rc,			// Right Center
	DCINEMA_Vhl,		// Vertical height left
	DCINEMA_Vhc,		// Vertical height center
	DCINEMA_Vhr,		// Vertical height right
	DCINEMA_Ts,			// Top center surround
	DCINEMA_Lw,			// Left wide
	DCINEMA_Rw,			// Right wide
	DCINEMA_Lsd,		// Left surround direct
	DCINEMA_Rsd,		// Right surround direct
	DCINEMA_LFE2,		// LFE2
	// they are mentioned in MLP/ATMOS
	ATMOS_Lts,			// Left Top Surround
	ATMOS_Rts,			// Right Top Surround
};

// DTS, DTS-HD
enum DTS_CH_LOC
{
	DTS_C = 0,			// Centre in front of listener(0)
	DTS_L,				// Left in front(-30)
	DTS_R,				// Right in front(30)
	DTS_Ls,				// Left surround on side in rear(-110)
	DTS_Rs,				// Right surround on side in rear(110)
	DTS_LFE1,			// Low frequency effects subwoofer
	DTS_Cs,				// Centre surround in rear(180)
	DTS_Lsr,			// Left surround in rear(-150)
	DTS_Rsr,			// Right surround in rear(150)
	DTS_Lss,			// Left surround on side(-90)
	DTS_Rss,			// Right surround on side(90)
	DTS_Lc,				// Between left and centre in front(-15)
	DTS_Rc,				// Between right and centre in front(15)
	DTS_Lh,				// Left height in front
	DTS_Ch,				// Centre Height in front
	DTS_Rh,				// Right Height in front
	DTS_LFE2,			// Second low frequency effects subwoofer
	DTS_Lw,				// Left on side in front(-60)
	DTS_Rw,				// Right on side in front(60)
	DTS_Oh,				// Over the listener's head
	DTS_Lhs,			// Left height on side
	DTS_Rhs,			// Right height on side
	DTS_Chr,			// Centre height in rear
	DTS_Lhr,			// Left height in rear
	DTS_Rhr,			// Right height in rear
	DTS_Cl,				// Centre in the plane lower then listener's ears
	DTS_Ll,				// Left in the plane lower then listener's ears
	DTS_Rl,				// Right in the plane lower then listener's ears
};

// DRA, DRA Extension
enum DRA_CH_LOC
{
	DRA_Front_Left = 0,
	DRA_Front_Center,
	DRA_Front_Right,
	DRA_Rear_Left,
	DRA_Rear_Right,
	DRA_LFE,
	DRA_Side_Left,
	DRA_Rear_Center,
	DRA_Side_Right,
};

// ITU-R BS.2051-2
// ARIB STD-B59 Version 2.0
// MP4 AAC 22.2 follow this standard
enum BS_CHANNEL_LOC
{
	BS_FL = 0,
	BS_FR,
	BS_FC,
	BS_LFE1,
	BS_BL,
	BS_BR,
	BS_FLc,
	BS_FRc,
	BS_BC,
	BS_LFE2,
	BS_SiL,
	BS_SiR,
	BS_TpFL,
	BS_TpFR,
	BS_TpFC,
	BS_TpC,
	BS_TpBL,
	BS_TpBR,
	BS_TpSiL,
	BS_TpSiR,
	BS_TpBC,
	BS_BtFC,
	BS_BtFL,
	BS_BtFR,
	BS_L,
	BS_R,
	BS_C,
	BS_LFE,
	BS_Ls,
	BS_Rs,
	BS_Ltf,
	BS_Rtf,
};

enum CH_MAPPING_CAT
{
	CH_MAPPING_CAT_UNKNOWN = 0,
	CH_MAPPING_CAT_DCINEMA = 1,				// SMPTE Standard 428M (theater application)
											// HDMV LPCM audio
											// Dolby Digital, Dolby Digital Plus
											// MLP from Dolby Lossless
	CH_MAPPING_CAT_DTS,						// DTS, DTS-HD
	CH_MAPPING_CAT_DRA,						// DRA, DRA Extension
	CH_MAPPING_CAT_BS,						// ITU-R BS.2051-2
											// ARIB STD-B59 Version 2.0
											// MP4 AAC 22.2 follow this standard
	CH_MAPPING_CAT_MAX = 7,
};

// bit 61, 62 and 63: Store the channel mapping category
#define CH_MAPPING_CATEGORY_MASK		0xE000000000000000ULL
// bit 60
#define CH_LOC_DUAL						60
#define CH_MAPPING_DUAL					(1ULL<<CH_LOC_DUAL)
// bit 59
#define CH_LOC_MS						59
#define CH_MAPPING_MS					(1ULL<<CH_LOC_MS))
// bit 58
#define CH_LOC_LR_SUM					58
#define CH_MAPPING_LR_SUM				(1ULL<<CH_LOC_LR_SUM))
// bit 57
#define CH_LOC_LR_DIFF					57
#define CH_MAPPING_LR_DIFF				(1ULL<<CH_LOC_LR_DIFF)
// bit 56
#define CH_LOC_LT						56
#define CH_MAPPING_LT					(1ULL<<CH_LOC_LT)
// bit 55
#define CH_LOC_RT						55
#define CH_MAPPING_RT					(1ULL<<CH_LOC_RT)


#define CH_SPECIAL_LOC_MIN				50

#define DCINEMA_CH_LOC_BITMASK			0xFFFFFULL		// 20 bits
#define DTS_CH_LOC_BITMASK				0xFFFFFFFULL	// 28 bits
#define DRA_CH_LOC_BITMASK				0x1FF			// 9 bits
#define BS_CH_LOC_BITMASK				0xFFFFFFFFULL	// 32 bits

#ifdef _MSC_VER
#pragma pack(push,1)
#define PACKED
#else
#define PACKED __attribute__ ((__packed__))
#endif

struct CH_MAPPING
{
	union {
		uint64_t						u64Val = 0;
#ifdef _BIG_ENDIAN_
		struct {
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LT : 1;		// L total
			uint64_t					RT : 1;		// R total

			uint64_t					reserved : 33;
			uint64_t					Rts : 1;
			uint64_t					Lts : 1;
			uint64_t					LFE2 : 1;
			uint64_t					Rsd : 1;
			uint64_t					Lsd : 1;
			uint64_t					Rw : 1;
			uint64_t					Lw : 1;
			uint64_t					Ts : 1;
			uint64_t					Vhr : 1;
			uint64_t					Vhc : 1;
			uint64_t					Vhl : 1;
			uint64_t					Rc : 1;
			uint64_t					Lc : 1;
			uint64_t					Rrs : 1;
			uint64_t					Cs : 1;
			uint64_t					Rls : 1;
			uint64_t					LFE : 1;
			uint64_t					Rs : 1;
			uint64_t					Ls : 1;
			uint64_t					R : 1;
			uint64_t					C : 1;
			uint64_t					L : 1;
		} PACKED;
		struct {
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LT : 1;		// L total
			uint64_t					RT : 1;		// R total

			uint64_t					reserved : 27;
			uint64_t					Rl : 1;
			uint64_t					Ll : 1;
			uint64_t					Cl : 1;
			uint64_t					Rhr : 1;
			uint64_t					Lhr : 1;
			uint64_t					Chr : 1;
			uint64_t					Rhs : 1;
			uint64_t					Lhs : 1;
			uint64_t					Oh : 1;
			uint64_t					Rw : 1;
			uint64_t					Lw : 1;
			uint64_t					LFE2 : 1;
			uint64_t					Rh : 1;
			uint64_t					Ch : 1;
			uint64_t					Lh : 1;
			uint64_t					Rc : 1;
			uint64_t					Lc : 1;
			uint64_t					Rss : 1;
			uint64_t					Lss : 1;
			uint64_t					Rsr : 1;
			uint64_t					LSr : 1;
			uint64_t					Cs : 1;
			uint64_t					LFE1 : 1;
			uint64_t					Rs : 1;
			uint64_t					Ls : 1;
			uint64_t					R : 1;
			uint64_t					L : 1;
			uint64_t					C : 1;
		} PACKED DTS;
		struct {
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LT : 1;		// L total
			uint64_t					RT : 1;		// R total

			uint64_t					reserved : 46;
			uint64_t					Side_Right : 1;
			uint64_t					Rear_Center : 1;
			uint64_t					Side_Left : 1;
			uint64_t					LFE : 1;
			uint64_t					Rear_Right : 1;
			uint64_t					Rear_Left : 1;
			uint64_t					Front_Right : 1;
			uint64_t					Front_Center : 1;
			uint64_t					Front_Left : 1;
		} PACKED DRA;
		struct {
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LT : 1;		// L total
			uint64_t					RT : 1;		// R total

			uint64_t					reserved : 23;
			uint64_t					Rtf : 1;
			uint64_t					Ltf : 1;
			uint64_t					Rs : 1;
			uint64_t					Ls : 1;
			uint64_t					LFE : 1;
			uint64_t					C : 1;
			uint64_t					R : 1;
			uint64_t					L : 1;
			uint64_t					BtFR : 1;
			uint64_t					BtFL : 1;
			uint64_t					BtFC : 1;
			uint64_t					TpBC : 1;
			uint64_t					TpSiR : 1;
			uint64_t					TpSiL : 1;
			uint64_t					TpBR : 1;
			uint64_t					TpBL : 1;
			uint64_t					TpC : 1;
			uint64_t					TpFC : 1;
			uint64_t					TpFR : 1;
			uint64_t					TpFL : 1;
			uint64_t					SiR : 1;
			uint64_t					SiL : 1;
			uint64_t					LFE2 : 1;
			uint64_t					BC : 1;
			uint64_t					FRc : 1;
			uint64_t					FLc : 1;
			uint64_t					BR : 1;
			uint64_t					BL : 1;
			uint64_t					LFE1 : 1;
			uint64_t					FC : 1;
			uint64_t					FR : 1;
			uint64_t					FL : 1;
		} PACKED BS;	// based on ITU-R.2051-2
#else
		struct {
			uint64_t					L : 1;
			uint64_t					C : 1;
			uint64_t					R : 1;
			uint64_t					Ls : 1;
			uint64_t					Rs : 1;
			uint64_t					LFE : 1;
			uint64_t					Rls : 1;
			uint64_t					Cs : 1;
			uint64_t					Rrs : 1;
			uint64_t					Lc : 1;
			uint64_t					Rc : 1;
			uint64_t					Vhl : 1;
			uint64_t					Vhc : 1;
			uint64_t					Vhr : 1;
			uint64_t					Ts : 1;
			uint64_t					Lw : 1;
			uint64_t					Rw : 1;
			uint64_t					Lsd : 1;
			uint64_t					Rsd : 1;
			uint64_t					LFE2 : 1;
			uint64_t					Lts : 1;
			uint64_t					Rts : 1;
			uint64_t					reserved : 33;

			uint64_t					RT : 1;		// R total
			uint64_t					LT : 1;		// L total
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
		} PACKED;
		struct {
			uint64_t					C : 1;
			uint64_t					L : 1;
			uint64_t					R : 1;
			uint64_t					Ls : 1;
			uint64_t					Rs : 1;
			uint64_t					LFE1 : 1;
			uint64_t					Cs : 1;
			uint64_t					LSr : 1;
			uint64_t					Rsr : 1;
			uint64_t					Lss : 1;
			uint64_t					Rss : 1;
			uint64_t					Lc : 1;
			uint64_t					Rc : 1;
			uint64_t					Lh : 1;
			uint64_t					Ch : 1;
			uint64_t					Rh : 1;
			uint64_t					LFE2 : 1;
			uint64_t					Lw : 1;
			uint64_t					Rw : 1;
			uint64_t					Oh : 1;
			uint64_t					Lhs : 1;
			uint64_t					Rhs : 1;
			uint64_t					Chr : 1;
			uint64_t					Lhr : 1;
			uint64_t					Rhr : 1;
			uint64_t					Cl : 1;
			uint64_t					Ll : 1;
			uint64_t					Rl : 1;
			uint64_t					reserved : 27;

			uint64_t					RT : 1;		// R total
			uint64_t					LT : 1;		// L total
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
		} PACKED DTS;
		struct {
			uint64_t					Front_Left : 1;
			uint64_t					Front_Center : 1;
			uint64_t					Front_Right : 1;
			uint64_t					Rear_Left : 1;
			uint64_t					Rear_Right : 1;
			uint64_t					LFE : 1;
			uint64_t					Side_Left : 1;
			uint64_t					Rear_Center : 1;
			uint64_t					Side_Right : 1;
			uint64_t					reserved : 46;

			uint64_t					RT : 1;		// R total
			uint64_t					LT : 1;		// L total
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
		} PACKED DRA;
		struct {
			uint64_t					FL : 1;
			uint64_t					FR : 1;
			uint64_t					FC : 1;
			uint64_t					LFE1 : 1;
			uint64_t					BL : 1;
			uint64_t					BR : 1;
			uint64_t					FLc : 1;
			uint64_t					FRc : 1;
			uint64_t					BC : 1;
			uint64_t					LFE2 : 1;
			uint64_t					SiL : 1;
			uint64_t					SiR : 1;
			uint64_t					TpFL : 1;
			uint64_t					TpFR : 1;
			uint64_t					TpFC : 1;
			uint64_t					TpC : 1;
			uint64_t					TpBL : 1;
			uint64_t					TpBR : 1;
			uint64_t					TpSiL : 1;
			uint64_t					TpSiR : 1;
			uint64_t					TpBC : 1;
			uint64_t					BtFC : 1;
			uint64_t					BtFL : 1;
			uint64_t					BtFR : 1;
			uint64_t					L : 1;
			uint64_t					R : 1;
			uint64_t					C : 1;
			uint64_t					LFE : 1;
			uint64_t					Ls : 1;
			uint64_t					Rs : 1;
			uint64_t					Ltf : 1;
			uint64_t					Rtf : 1;
			uint64_t					reserved : 23;

			uint64_t					RT : 1;		// R total
			uint64_t					LT : 1;		// L total
			uint64_t					LR_DIFF : 1;// L-R
			uint64_t					LR_SUM : 1;	// L+R
			uint64_t					MS : 1;		// Mono Surround
			uint64_t					dual_ch : 1;// The related masked channel is dual, for example, C:1 + dual_ch: 1 ==> dual-mono
			uint64_t					cat : 3;	// Please see CH_MAPPING_CAT_DCINEMA, CH_MAPPING_CAT_DTS, CH_MAPPING_CAT_DRA and CH_MAPPING_CAT_BS
		} PACKED BS;	// based on ITU-R.2051-2
#endif
	};

	CH_MAPPING() {}
	CH_MAPPING(CH_MAPPING_CAT CAT) {
		if (CAT >= CH_MAPPING_CAT_DCINEMA && CAT <= CH_MAPPING_CAT_MAX)
			cat = CAT;
	}
	CH_MAPPING(CH_MAPPING_CAT CAT, std::initializer_list<int> ch_locs);
	// clear the channel mapping bits
	void clear();
	// mark the current channel mapping is dual-mono
	int set_dual_mono();
	// get the channel mapping layout description
	std::string get_desc();
	// check whether the channel location in each category is LFE or not
	bool is_lfe(size_t i);
	// check whether the channel mapping is a dual-mono case
	bool is_dual_mono();
	// check whether the channel mapping is invalid or not
	bool is_invalid();
	// check the channel location is present in the current channel mapping
	bool is_present(int ch_loc);
};

constexpr size_t size_of_CH_MAPPING = sizeof(CH_MAPPING);

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef PACKED

#define CHANNEL_BITMASK(loc)			(1ULL<<loc)
#define CHANNLE_PRESENT(chanmap, loc)	(chanmap&(CHANNEL_BITMASK(loc)))

enum SPEAKER_POS
{
	SPEAKER_POS_FRONT_LEFT = 0,
	SPEAKER_POS_FRONT_RIGHT,
	SPEAKER_POS_FRONT_CENTER,
	SPEAKER_POS_LOW_FREQUENCY,
	SPEAKER_POS_BACK_LEFT,
	SPEAKER_POS_BACK_RIGHT,
	SPEAKER_POS_FRONT_LEFT_OF_CENTER,
	SPEAKER_POS_FRONT_RIGHT_OF_CENTER,
	SPEAKER_POS_BACK_CENTER,
	SPEAKER_POS_SIDE_LEFT,
	SPEAKER_POS_SIDE_RIGHT,
	SPEAKER_POS_TOP_CENTER,
	SPEAKER_POS_TOP_FRONT_LEFT,
	SPEAKER_POS_TOP_FRONT_CENTER,
	SPEAKER_POS_TOP_FRONT_RIGHT,
	SPEAKER_POS_TOP_BACK_LEFT,
	SPEAKER_POS_TOP_BACK_CENTER,
	SPEAKER_POS_TOP_BACK_RIGHT,
	SPEAKER_POS_MAX
};

// speaker geometry configuration flags, specifies assignment of channels to speaker positions, defined as per WAVEFORMATEXTENSIBLE.dwChannelMask
#if !defined(_SPEAKER_POSITIONS_)
#define _SPEAKER_POSITIONS_
#define SPEAKER_FRONT_LEFT				0x00000001
#define SPEAKER_FRONT_RIGHT				0x00000002
#define SPEAKER_FRONT_CENTER			0x00000004
#define SPEAKER_LOW_FREQUENCY			0x00000008
#define SPEAKER_BACK_LEFT				0x00000010
#define SPEAKER_BACK_RIGHT				0x00000020
#define SPEAKER_FRONT_LEFT_OF_CENTER	0x00000040
#define SPEAKER_FRONT_RIGHT_OF_CENTER	0x00000080
#define SPEAKER_BACK_CENTER				0x00000100
#define SPEAKER_SIDE_LEFT				0x00000200
#define SPEAKER_SIDE_RIGHT				0x00000400
#define SPEAKER_TOP_CENTER				0x00000800
#define SPEAKER_TOP_FRONT_LEFT			0x00001000
#define SPEAKER_TOP_FRONT_CENTER		0x00002000
#define SPEAKER_TOP_FRONT_RIGHT			0x00004000
#define SPEAKER_TOP_BACK_LEFT			0x00008000
#define SPEAKER_TOP_BACK_CENTER			0x00010000
#define SPEAKER_TOP_BACK_RIGHT			0x00020000
#define SPEAKER_RESERVED				0x7FFC0000 // bit mask locations reserved for future use
#define SPEAKER_ALL						0x80000000 // used to specify that any possible permutation of speaker configurations
#endif

// standard speaker geometry configurations, used with X3DAudioInitialize
#if !defined(SPEAKER_MONO)
#define SPEAKER_MONO					(SPEAKER_FRONT_CENTER)
#define SPEAKER_STEREO					(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
#define SPEAKER_2POINT1					(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY)
#define SPEAKER_SURROUND				(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
#define SPEAKER_QUAD					(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define SPEAKER_4POINT1					(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define SPEAKER_5POINT1					(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
#define SPEAKER_7POINT1					(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)
#define SPEAKER_5POINT1_SURROUND		(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
#define SPEAKER_7POINT1_SURROUND		(SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
#endif

extern CH_MAPPING acmod_ch_assignments[8];
extern CH_MAPPING ddp_ch_assignment[16];
extern std::tuple<CH_MAPPING, std::vector<uint8_t>> hdmv_lpcm_ch_assignments[16];
extern CH_MAPPING FBB_Channel_assignments[21];
extern CH_MAPPING FBA_Channel_Loc_mapping_0[13];
extern CH_MAPPING FBA_Channel_Loc_mapping_1[5];
extern uint32_t DCINEMA_Channel_Speaker_Mapping[22];
extern CH_MAPPING dts_audio_channel_arragements[16];
extern std::tuple<std::string/*Notation*/, std::string/*Desc*/, CH_MAPPING/*ch_loc*/, uint8_t/*number of channels*/> dtshd_speaker_bitmask_table[16];
extern CH_MAPPING aac_channel_configurations[16];
extern CH_MAPPING mpega_channel_mode_layouts[4];
