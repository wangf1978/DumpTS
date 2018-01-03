#pragma once
#include <tuple>
#include <string>

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

enum CHANNEL_LOC
{
	CH_LOC_LEFT = 0,
	CH_LOC_CENTER,
	CH_LOC_RIGHT,
	CH_LOC_LS,
	CH_LOC_RS,
	CH_LOC_LC,
	CH_LOC_RC,
	CH_LOC_LRS,
	CH_LOC_RRS,
	CH_LOC_CS,
	CH_LOC_TS,
	CH_LOC_LSD,
	CH_LOC_RSD,
	CH_LOC_LW,
	CH_LOC_RW,
	CH_LOC_VHL,
	CH_LOC_VHR,
	CH_LOC_VHC,
	CH_LOC_LTS,
	CH_LOC_RTS,
	CH_LOC_LFE,
	CH_LOC_LFE2,
	CH_LOC_LHR,	// Height channel surround rear left loudspeaker
	CH_LOC_RHR,	// Height channel surround rear right loudspeaker
	CH_LOC_CHR,	// Height channel surround rear center loudspeaker
	CH_LOC_LSS,	// Left side surround (Left Surround, directly to the side of the listener)
	CH_LOC_RSS,	// Right side surround (Right Surround, directly to the side of the listener)
	CH_LOC_LHS,	// Height channel surround rear left loudspeaker
	CH_LOC_RHS, // Height channel surround right side loudspeaker
	// Special channel signal
	CH_MONO		= 0x80,		// mono channel
	CH_SURROUND,			// Surround
	CH_LR_SUM,				// L + R
	CH_LR_DIFF,				// L - R
	CH_LT,					// Left Total
	CH_RT,					// Right Total
	CH_LOC_DUALMONO = 31
};

#define CHANNEL_BITMASK(loc)			(1<<loc)
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
#define SPEAKER_MONO					SPEAKER_FRONT_CENTER
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

extern std::tuple<std::string, std::string, std::string> channel_descs[22];
extern std::string GetChannelMappingDesc(unsigned long channel_mapping);
extern unsigned long acmod_ch_assignments[8];
extern unsigned long ddp_ch_assignment[16];
extern std::tuple<uint32_t, std::vector<uint8_t>> hdmv_lpcm_ch_assignments[16];
extern unsigned long FBB_Channel_assignments[21];
extern unsigned long FBA_Channel_Loc_mapping_0[13];
extern unsigned long FBA_Channel_Loc_mapping_1[5];
extern unsigned long Channel_Speaker_Mapping[22];
extern std::vector<CHANNEL_LOC> dts_audio_channel_arragements[16];
extern std::tuple<std::string/*Notation*/, std::string/*Desc*/, uint32_t/*ch_loc*/, uint8_t/*number of channels*/> dtshd_speaker_bitmask_table[16];
extern std::vector<CHANNEL_LOC> aac_channel_configurations[8];
