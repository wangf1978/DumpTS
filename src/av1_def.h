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
#ifndef __AV1_DEF_H__
#define __AV1_DEF_H__

#pragma once

#define REFS_PER_FRAME				7				// Number of reference frames that can be used for inter prediction
#define TOTAL_REFS_PER_FRAME		8				// Number of reference frame types(including intra type)
#define BLOCK_SIZE_GROUPS			4				// Number of contexts when decoding y_mode
#define BLOCK_SIZES					22				// Number of different block sizes used
#define BLOCK_INVALID				22				// Sentinel value to mark partition choices that are not allowed
#define MAX_SB_SIZE					128				// Maximum size of a superblock in luma samples
#define MI_SIZE						4				// Smallest size of a mode info block in luma samples
#define MI_SIZE_LOG2				2				// Base 2 logarithm of smallest size of a mode info block
#define MAX_TILE_WIDTH				4096			// Maximum width of a tile in units of luma samples
#define MAX_TILE_AREA				4096 * 2304		// Maximum area of a tile in units of luma samples
#define MAX_TILE_ROWS				64				// Maximum number of tile rows
#define MAX_TILE_COLS				64				// Maximum number of tile columns
#define INTRABC_DELAY_PIXELS		256				// Number of horizontal luma samples before intra block copy can be used
#define INTRABC_DELAY_SB64			4				// Number of 64 by 64 blocks before intra block copy can be used
#define NUM_REF_FRAMES				8				// Number of frames that can be stored for future reference
#define IS_INTER_CONTEXTS			4				// Number of contexts for is_inter
#define REF_CONTEXTS				3				// Number of contexts for single_ref, comp_ref, comp_bwdref, uni_comp_ref, uni_comp_ref_p1 and uni_comp_ref_p2
#define MAX_SEGMENTS				8				// Number of segments allowed in segmentation map
#define SEGMENT_ID_CONTEXTS			3				// Number of contexts for segment_id
#define FRAME_BUFFERS				(NUM_REF_FRAMES + 7)
#define BUFFER_POOL_MAX_SIZE		10				// Number of frames in buffer pool

typedef enum {
	SEG_LVL_ALT_Q = 0,								// Use alternate Quantizer ....
	SEG_LVL_ALT_LF_Y_V,								// Use alternate loop filter value on y plane vertical
	SEG_LVL_ALT_LF_Y_H,								// Use alternate loop filter value on y plane horizontal
	SEG_LVL_ALT_LF_U,								// Use alternate loop filter value on u plane
	SEG_LVL_ALT_LF_V,								// Use alternate loop filter value on v plane
	SEG_LVL_REF_FRAME,								// Optional Segment reference frame
	SEG_LVL_SKIP,									// Optional Segment (0,0) + skip mode
	SEG_LVL_GLOBALMV,
	SEG_LVL_MAX
} SEG_LVL_FEATURES;

#define SEG_LVL_NAMEA(seg_lvl)		((seg_lvl) == 0?"ALT_Q":(\
									 (seg_lvl) == 1?"ALT_LF_Y_V":(\
									 (seg_lvl) == 2?"ALT_LF_Y_H":(\
									 (seg_lvl) == 3?"ALT_LF_U":(\
									 (seg_lvl) == 4?"ALT_LF_V":(\
									 (seg_lvl) == 5?"REF_FRAME":(\
									 (seg_lvl) == 6?"SKIP":(\
									 (seg_lvl) == 7?"GLOBALMV":""))))))))

#if 0
#define SEG_LVL_ALT_Q				0				// Index for quantizer segment feature
#define SEG_LVL_ALT_LF_Y_V			1				// Index for vertical luma loop filter segment feature
#define SEG_LVL_REF_FRAME			5				// Index for reference frame segment feature
#define SEG_LVL_SKIP				6				// Index for skip segment feature
#define SEG_LVL_GLOBALMV			7				// Index for global mv feature
#define SEG_LVL_MAX					8				// Number of segment features
#endif

#define PLANE_TYPES					2				// Number of different plane types(luma or chroma)
#define TX_SIZE_CONTEXTS			3				// Number of contexts for transform size
#define INTERP_FILTERS				3				// Number of values for interp_filter
#define INTERP_FILTER_CONTEXTS		16				// Number of contexts for interp_filter
#define SKIP_MODE_CONTEXTS			3				// Number of contexts for decoding skip_mode
#define SKIP_CONTEXTS				3				// Number of contexts for decoding skip
#define PARTITION_CONTEXTS			4				// Number of contexts when decoding partition
#define TX_SIZES					5				// Number of square transform sizes
#define TX_SIZES_ALL				19				// Number of transform sizes(including non - square sizes)
#define TX_MODES					3				// Number of values for tx_mode
#define DCT_DCT						0				// Inverse transform rows with DCT and columns with DCT
#define ADST_DCT					1				// Inverse transform rows with DCT and columns with ADST
#define DCT_ADST					2				// Inverse transform rows with ADST and columns with DCT
#define ADST_ADST					3				// Inverse transform rows with ADST and columns with ADST
#define FLIPADST_DCT				4				// Inverse transform rows with FLIPADST and columns with DCT
#define DCT_FLIPADST				5				// Inverse transform rows with DCT and columns with FLIPADST
#define FLIPADST_FLIPADST			6				// Inverse transform rows with FLIPADST and columns with FLIPADST
#define ADST_FLIPADST				7				// Inverse transform rows with ADST and columns with FLIPADST
#define FLIPADST_ADST				8				// Inverse transform rows with FLIPADST and columns with ADST
#define IDTX						9				// Inverse transform rows with identity and columns with identity
#define V_DCT						10				// Inverse transform rows with identity and columns with DCT
#define H_DCT						11				// Inverse transform rows with DCT and columns with identity
#define V_ADST						12				// Inverse transform rows with identity and columns with ADST
#define H_ADST						13				// Inverse transform rows with ADST and columns with identity
#define V_FLIPADST					14				// Inverse transform rows with identity and columns with FLIPADST
#define H_FLIPADST					15				// Inverse transform rows with FLIPADST and columns with identity
#define TX_TYPES					16				// Number of inverse transform types
#define MB_MODE_COUNT				17				// Number of values for YMode
#define INTRA_MODES					13				// Number of values for y_mode
#define UV_INTRA_MODES_CFL_NOT_ALLOWED\
									13				// Number of values for uv_mode when chroma from luma is not allowed
#define UV_INTRA_MODES_CFL_ALLOWED	14				// Number of values for uv_mode when chroma from luma is allowed
#define COMPOUND_MODES				8				// Number of values for compound_mode
#define COMPOUND_MODE_CONTEXTS		8				// Number of contexts for compound_mode
#define COMP_NEWMV_CTXS				5				// Number of new mv values used when constructing context for compound_mode
#define NEW_MV_CONTEXTS				6				// Number of contexts for new_mv
#define ZERO_MV_CONTEXTS			2				// Number of contexts for zero_mv
#define REF_MV_CONTEXTS				6				// Number of contexts for ref_mv
#define DRL_MODE_CONTEXTS			3				// Number of contexts for drl_mode
#define MV_CONTEXTS					2				// Number of contexts for decoding motion vectors including one for intra block copy
#define MV_INTRABC_CONTEXT			1				// Motion vector context used for intra block copy
#define MV_JOINTS					4				// Number of values for mv_joint
#define MV_CLASSES					11				// Number of values for mv_class
#define CLASS0_SIZE					2				// Number of values for mv_class0_bit
#define MV_OFFSET_BITS				10				// Maximum number of bits for decoding motion vectors
#define MAX_LOOP_FILTER				63				// Maximum value used for loop filtering
#define REF_SCALE_SHIFT				14				// Number of bits of precision when scaling reference frames
#define SUBPEL_BITS					4				// Number of bits of precision when choosing an inter prediction filter kernel
#define SUBPEL_MASK					15				// (1 << SUBPEL_BITS) - 1
#define SCALE_SUBPEL_BITS			10				// Number of bits of precision when computing inter prediction locations
#define MV_BORDER					128				// Value used when clipping motion vectors
#define PALETTE_COLOR_CONTEXTS		5				// Number of values for color contexts
#define PALETTE_MAX_COLOR_CONTEXT_HASH\
									8				// Number of mappings between color context hash and color context
#define PALETTE_BLOCK_SIZE_CONTEXTS	7				// Number of values for palette block size
#define PALETTE_Y_MODE_CONTEXTS		3				// Number of values for palette Y plane mode contexts
#define PALETTE_UV_MODE_CONTEXTS	2				// Number of values for palette U and V plane mode contexts
#define PALETTE_SIZES				7				// Number of values for palette_size
#define PALETTE_COLORS				8				// Number of values for palette_color
#define PALETTE_NUM_NEIGHBORS		3				// Number of neighbors considered within palette computation
#define DELTA_Q_SMALL				3				// Value indicating alternative encoding of quantizer index delta values
#define DELTA_LF_SMALL				3				// Value indicating alternative encoding of loop filter delta values
#define QM_TOTAL_SIZE				3344			// Number of values in the quantizer matrix
#define MAX_ANGLE_DELTA				3				// Maximum magnitude of AngleDeltaY and AngleDeltaUV
#define DIRECTIONAL_MODES			8				// Number of directional intra modes
#define ANGLE_STEP					3				// Number of degrees of step per unit increase in AngleDeltaY or AngleDeltaUV.
#define TX_SET_TYPES_INTRA			3				// Number of intra transform set types
#define TX_SET_TYPES_INTER			4				// Number of inter transform set types
#define WARPEDMODEL_PREC_BITS		16				// Internal precision of warped motion models
//#define IDENTITY					0				// Warp model is just an identity transform
//#define TRANSLATION					1				// Warp model is a pure translation
//#define ROTZOOM						2				// Warp model is a rotation + symmetric zoom + translation
//#define AFFINE						3				// Warp model is a general affine transform
#define GM_ABS_TRANS_BITS			12				// Number of bits encoded for translational components of global motion models, if part of a ROTZOOM or AFFINE model
#define GM_ABS_TRANS_ONLY_BITS		9				// Number of bits encoded for translational components of global motion models, if part of a TRANSLATION model
#define GM_ABS_ALPHA_BITS			12				// Number of bits encoded for non - translational components of global motion models
#define DIV_LUT_PREC_BITS			14				// Number of fractional bits of entries in divisor lookup table
#define DIV_LUT_BITS				8				// Number of fractional bits for lookup in divisor lookup table
#define DIV_LUT_NUM					257				// Number of entries in divisor lookup table
#define MOTION_MODES				3				// Number of values for motion modes
#define SIMPLE						0				// Use translation or global motion compensation
#define OBMC						1				// Use overlapped block motion compensation
#define LOCALWARP					2				// Use local warp motion compensation
#define LEAST_SQUARES_SAMPLES_MAX	8				// Largest number of samples used when computing a local warp
#define LS_MV_MAX					256				// Largest motion vector difference to include in local warp computation
#define WARPEDMODEL_TRANS_CLAMP		(1 << 23)		// Clamping value used for translation components of warp
#define WARPEDMODEL_NONDIAGAFFINE_CLAMP\
									(1 << 13)		// Clamping value used for matrix components of warp
#define WARPEDPIXEL_PREC_SHIFTS		(1 << 6)		// Number of phases used in warped filtering
#define WARPEDDIFF_PREC_BITS		10				// Number of extra bits of precision in warped filtering
#define GM_ALPHA_PREC_BITS			15				// Number of fractional bits for sending non - translational warp model coefficients
#define GM_TRANS_PREC_BITS			6				// Number of fractional bits for sending translational warp model coefficients
#define GM_TRANS_ONLY_PREC_BITS		3				// Number of fractional bits used for pure translational warps
#define INTERINTRA_MODES			4				// Number of inter intra modes
#define MASK_MASTER_SIZE			64				// Size of MasterMask array
#define SEGMENT_ID_PREDICTED_CONTEXTS	3			// Number of contexts for segment_id_predicted
#define IS_INTER_CONTEXTS			4				// Number of contexts for is_inter
#define SKIP_CONTEXTS				3				// Number of contexts for skip
#define FWD_REFS					4				// Number of syntax elements for forward reference frames
#define BWD_REFS					3				// Number of syntax elements for backward reference frames
#define SINGLE_REFS					7				// Number of syntax elements for single reference frames
#define UNIDIR_COMP_REFS			4				// Number of syntax elements for unidirectional compound reference frames
#define COMPOUND_TYPES				2				// Number of values for compound_type
#define CFL_JOINT_SIGNS				8				// Number of values for cfl_alpha_signs
#define CFL_ALPHABET_SIZE			16				// Number of values for cfl_alpha_u and cfl_alpha_v
#define COMP_INTER_CONTEXTS			5				// Number of contexts for comp_mode
#define COMP_REF_TYPE_CONTEXTS		5				// Number of contexts for comp_ref_type
#define CFL_ALPHA_CONTEXTS			6				// Number of contexts for cfl_alpha_u and cfl_alpha_v
#define INTRA_MODE_CONTEXTS			5				// Number of contexts for intra_frame_y_mode
#define COMP_GROUP_IDX_CONTEXTS		6				// Number of contexts for comp_group_idx
#define COMPOUND_IDX_CONTEXTS		6				// Number of contexts for compound_idx
#define INTRA_EDGE_KERNELS			3				// Number of filter kernels for the intra edge filter
#define INTRA_EDGE_TAPS				5				// Number of kernel taps for the intra edge filter
#define FRAME_LF_COUNT				4				// Number of loop filter strength values
#define MAX_VARTX_DEPTH				2				// Maximum depth for variable transform trees
#define TXFM_PARTITION_CONTEXTS		21				// Number of contexts for txfm_split
#define REF_CAT_LEVEL				640				// Bonus weight for close motion vectors
#define MAX_REF_MV_STACK_SIZE		8				// Maximum number of motion vectors in the stack
#define MFMV_STACK_SIZE				3				// Stack size for motion field motion vectors
#define MAX_TX_DEPTH				2				// Number of contexts for tx_depth when the maximum transform size is 8x8
#define WEDGE_TYPES					16				// Number of directions for the wedge mask process
#define FILTER_BITS					7				// Number of bits used in Wiener filter coefficients
#define WIENER_COEFFS				3				// Number of Wiener filter coefficients to read
#define SGRPROJ_PARAMS_BITS			4				// Number of bits needed to specify self guided filter set
#define SGRPROJ_PRJ_SUBEXP_K		4				// Controls how self guided deltas are read
#define SGRPROJ_PRJ_BITS			7				// Precision bits during self guided restoration
#define SGRPROJ_RST_BITS			4				// Restoration precision bits generated higher than source before projection
#define SGRPROJ_MTABLE_BITS			20				// Precision of mtable division table
#define SGRPROJ_RECIP_BITS			12				// Precision of division by n table
#define SGRPROJ_SGR_BITS			8				// Internal precision bits for core selfguided_restoration
#define EC_PROB_SHIFT				6				// Number of bits to reduce CDF precision during arithmetic coding
#define EC_MIN_PROB					4				// Minimum probability assigned to each symbol during arithmetic coding
#define SELECT_SCREEN_CONTENT_TOOLS	2				// Value that indicates the allow_screen_content_tools syntax element is coded
#define SELECT_INTEGER_MV			2				// Value that indicates the force_integer_mv syntax element is coded
#define RESTORATION_TILESIZE_MAX	256				// Maximum size of a loop restoration tile
#define MAX_FRAME_DISTANCE			31				// Maximum distance when computing weighted prediction
#define MAX_OFFSET_WIDTH			8				// Maximum horizontal offset of a projected motion vector
#define MAX_OFFSET_HEIGHT			0				// Maximum vertical offset of a projected motion vector
#define WARP_PARAM_REDUCE_BITS		6				// Rounding bitwidth for the parameters to the shear process
#define NUM_BASE_LEVELS				2				// Number of quantizer base levels
#define COEFF_BASE_RANGE			12				// The quantizer range above NUM_BASE_LEVELS above which the Exp - Golomb coding process is activated
#define BR_CDF_SIZE					4				// Number of contexts for coeff_br
#define SIG_COEF_CONTEXTS_EOB		4				// Number of contexts for coeff_base_eob
#define SIG_COEF_CONTEXTS_2D		26				// Context offset for coeff_base for horizontal - only or vertical - only transforms.
#define SIG_COEF_CONTEXTS			42				// Number of contexts for coeff_base
#define SIG_REF_DIFF_OFFSET_NUM		5				// Maximum number of context samples to be used in determining the context index for coeff_base and coeff_base_eob.
#define SUPERRES_NUM				8				// Numerator for upscaling ratio
#define SUPERRES_DENOM_MIN			9				// Smallest denominator for upscaling ratio
#define SUPERRES_DENOM_BITS			3				// Number of bits sent to specify denominator of upscaling ratio
#define SUPERRES_FILTER_BITS		6				// Number of bits of fractional precision for upscaling filter selection
#define SUPERRES_FILTER_SHIFTS		(1 << SUPERRES_FILTER_BITS)
													// Number of phases of upscaling filters
#define SUPERRES_FILTER_TAPS		8				// Number of taps of upscaling filters
#define SUPERRES_FILTER_OFFSET		3				// Sample offset for upscaling filters
#define SUPERRES_SCALE_BITS			14				// Number of fractional bits for computing position in upscaling
#define SUPERRES_SCALE_MASK			((1 << 14) - 1)	// Mask for computing position in upscaling
#define SUPERRES_EXTRA_BITS			8				// Difference in precision between SUPERRES_SCALE_BITS and SUPERRES_FILTER_BITS
#define TXB_SKIP_CONTEXTS			13				// Number of contexts for all_zero
#define EOB_COEF_CONTEXTS			9				// Number of contexts for eob_extra
#define DC_SIGN_CONTEXTS			3				// Number of contexts for dc_sign
#define LEVEL_CONTEXTS				21				// Number of contexts for coeff_br
#define TX_CLASS_2D					0				// Transform class for transform types performing non - identity transforms in both directions
#define TX_CLASS_HORIZ				1				// Transform class for transforms performing only a horizontal non - identity transform
#define TX_CLASS_VERT				2				// Transform class for transforms performing only a vertical non - identity transform
#define REFMVS_LIMIT				((1 << 12) - 1)	// Largest reference MV component that can be saved
#define INTRA_FILTER_SCALE_BITS		4				// Scaling shift for intra filtering process
#define INTRA_FILTER_MODES			5				// Number of types of intra filtering
#define COEFF_CDF_Q_CTXS			4				// Number of selectable context types for the coeff() syntax structure
#define PRIMARY_REF_NONE			7				// Value of primary_ref_frame indicating that there is no primary reference frame

#define SCALABILITY_L1T2			0
#define SCALABILITY_L1T3			1
#define SCALABILITY_L2T1			2
#define SCALABILITY_L2T2			3
#define SCALABILITY_L2T3			4
#define SCALABILITY_S2T1			5
#define SCALABILITY_S2T2			6
#define SCALABILITY_S2T3			7
#define SCALABILITY_L2T1h			8
#define SCALABILITY_L2T2h			9
#define SCALABILITY_L2T3h			10
#define SCALABILITY_S2T1h			11
#define SCALABILITY_S2T2h			12
#define SCALABILITY_S2T3h			13
#define SCALABILITY_SS				14

#define METADATA_TYPE_HDR_CLL		1
#define METADATA_TYPE_HDR_MDCV		2
#define METADATA_TYPE_SCALABILITY	3
#define METADATA_TYPE_ITUT_T35		4
#define METADATA_TYPE_TIMECODE		5

#define INTER_REFS_PER_FRAME		(ALTREF_FRAME - LAST_FRAME + 1)

// 4 frame filter levels: y plane vertical, y plane horizontal,
// u plane, and v plane
#define FRAME_LF_COUNT 4
#define DEFAULT_DELTA_LF_MULTI 0
#define MAX_MODE_LF_DELTAS 2

enum AV1_FRAME_TYPE
{
	KEY_FRAME = 0,
	INTER_FRAME = 1,
	INTRA_ONLY_FRAME = 2,
	SWITCH_FRAME = 3,
};

#define AV1_FRAME_TYPE_NAMEA(ft)	(\
	(ft) == KEY_FRAME?"Key frame":(\
	(ft) == INTER_FRAME?"Inter frame":(\
	(ft) == INTRA_ONLY_FRAME?"Intra Only Frame":(\
	(ft) == SWITCH_FRAME?"Switch Frame":"Unknown"))))

#define AV1_FRAME_TYPE_NAMEW(ft)	(\
	(ft) == KEY_FRAME?"Key frame":(\
	(ft) == INTER_FRAME?"Inter frame":(\
	(ft) == INTRA_ONLY_FRAME?"Intra Only Frame":(\
	(ft) == SWITCH_FRAME?"Switch Frame":"Unknown"))))

#ifdef _UNICODE
#define AV_FRAME_TYPE_NAME(t)	AV1_FRAME_TYPE_NAMEW(t)
#else
#define AV_FRAME_TYPE_NAME(t)	AV1_FRAME_TYPE_NAMEA(t)
#endif

enum AV1_REF_FRAME
{
	INTRA_FRAME = 0,
	LAST_FRAME,
	LAST2_FRAME,
	LAST3_FRAME,
	GOLDEN_FRAME,
	BWDREF_FRAME,
	ALTREF2_FRAME,
	ALTREF_FRAME
};

#define AV1_REF_FRAME_NAMEA(refFrame)	(\
	(refFrame == INTRA_FRAME)?"INTRA_FRAME":(\
	(refFrame == LAST_FRAME)?"LAST_FRAME":(\
	(refFrame == LAST2_FRAME)?"LAST2_FRAME":(\
	(refFrame == LAST3_FRAME)?"LAST3_FRAME":(\
	(refFrame == GOLDEN_FRAME)?"GOLDEN_FRAME":(\
	(refFrame == BWDREF_FRAME)?"BWDREF_FRAME":(\
	(refFrame == ALTREF2_FRAME)?"ALTREF2_FRAME":(\
	(refFrame == ALTREF_FRAME)?"ALTREF_FRAME":"Unknown"))))))))

#define AV1_REF_FRAME_NAMEW(refFrame)	(\
	(refFrame == INTRA_FRAME)?L"INTRA_FRAME":(\
	(refFrame == LAST_FRAME)?L"LAST_FRAME":(\
	(refFrame == LAST2_FRAME)?L"LAST2_FRAME":(\
	(refFrame == LAST3_FRAME)?L"LAST3_FRAME":(\
	(refFrame == GOLDEN_FRAME)?L"GOLDEN_FRAME":(\
	(refFrame == BWDREF_FRAME)?L"BWDREF_FRAME":(\
	(refFrame == ALTREF2_FRAME)?L"ALTREF2_FRAME":(\
	(refFrame == ALTREF_FRAME)?L"ALTREF_FRAME":L"Unknown"))))))))

#ifdef _UNICODE
#define AV1_REF_FRAME_NAME(r)	AV1_REF_FRAME_NAMEW(r)
#else
#define AV1_REF_FRAME_NAME(r)	AV1_REF_FRAME_NAMEA(r)
#endif

enum AV1_INTERPOLATION_FILTER
{
	EIGHTTAP = 0,
	EIGHTTAP_SMOOTH,
	EIGHTTAP_SHARP,
	BILINEAR,
	SWITCHABLE,
};

enum FRAME_RESTORATION_TYPE
{
	RESTORE_NONE = 0,
	RESTORE_WIENER = 1,
	RESTORE_SGRPROJ = 2,
	RESTORE_SWITCHABLE = 3
};

enum TX_MODE
{
	ONLY_4X4 = 0,
	TX_MODE_LARGEST,
	TX_MODE_SELECT
};

enum TRANSFORMATIONTYPE {
	IDENTITY = 0,      // identity transformation, 0-parameter
	TRANSLATION = 1,   // translational motion 2-parameter
	ROTZOOM = 2,       // simplified affine with rotation + zoom only, 4-parameter
	AFFINE = 3,        // affine, 6-parameter
	TRANS_TYPES,
};

// The order of values in the wmmat matrix below is best described
// by the homography:
//      [x'     (m2 m3 m0   [x
//  z .  y'  =   m4 m5 m1 *  y
//       1]      m6 m7 1)    1]
struct WarpedMotionParams
{
	TRANSFORMATIONTYPE	wmtype;
	int32_t				wmmat[8];
	int16_t				alpha, beta, gamma, delta;
	int8_t				invalid;
};

//
// Some inline functions defined in the AV1 specification
//
namespace BST {
	namespace AV1 {
		template <typename T>
		INLINE int8_t FloorLog2(T x)
		{
			int8_t s = 0;
			while (x != 0)
			{
				x = x >> 1;
				s++;
			}
			return s - 1;
		}

		template <typename T>
		INLINE int8_t CeilLog2(T x)
		{
			if (x < 2)
				return 0;

			int8_t i = 1;
			T p = 2;
			while (p < x) {
				i++;
				p = p << 1;
			}

			return i;
		}

		INLINE uint8_t tile_log2(uint16_t blkSize, uint32_t target)
		{
			uint8_t k = 0;
			for (; ((uint32_t)blkSize << k) < target; k++);
			return k;
		}

		INLINE uint8_t quick_log2(uint32_t v)
		{
			for (int i = 31; i >= 0; i--)
				if ((v&(1 << i)))
					return i;
			return 0;
		}

		INLINE uint32_t leb128(uint8_t* p, uint32_t cb, uint8_t* pcbLeb128 = nullptr, uint8_t* pbNeedMoreData = nullptr)
		{
			int64_t value = 0;
			uint8_t Leb128Bytes = 0;
			size_t i = 0, cbAvailLeb128ExtractBytes = AMP_MIN(8, cb);

			for (; i < cbAvailLeb128ExtractBytes; i++)
			{
				value |= (((uint64_t)p[i] & 0x7f) << ((uint64_t)i * 7));
				Leb128Bytes++;
				if (!(p[i] & 0x80))
					break;
			}

			if (i >= 8 || value >= UINT32_MAX)
			{
				AMP_SAFEASSIGN(pbNeedMoreData, 0);
				return UINT32_MAX;
			}

			if (i >= cbAvailLeb128ExtractBytes)
			{
				AMP_SAFEASSIGN(pbNeedMoreData, 1);
				return UINT32_MAX;
			}

			AMP_SAFEASSIGN(pcbLeb128, Leb128Bytes);

			return (uint32_t)value;
		}

		INLINE uint8_t ns_len(uint64_t v_max, uint64_t ns)
		{
			int8_t w = 0;
			unsigned long long vv = v_max;
			while (vv != 0)
			{
				vv = vv >> 1;
				w++;
			}

			uint64_t m = (1ULL << w) - v_max;
			if (ns < m)
				return w - 1;

			return w;
		}

		INLINE uint64_t f(AMBst bs, uint8_t n)
		{
			return AMBst_GetBits(bs, n);
		}

		INLINE uint64_t uvlc(AMBst bs)
		{
			return AMBst_Get_uvlc(bs);
		}

		INLINE uint64_t le(AMBst bs, uint8_t nBytes)
		{
			return AMBst_Get_le(bs, nBytes);
		}

		INLINE uint64_t leb128(AMBst bs)
		{
			return AMBst_Get_leb128(bs);
		}

		INLINE int64_t su(AMBst bs, uint8_t n)
		{
			return AMBst_Get_su(bs, n);
		}

		INLINE int32_t inverse_recenter(int32_t r, int32_t v)
		{
			if (v > 2 * r)
				return v;
			else if (v & 1)
				return r - ((v + 1) >> 1);
			else
				return r + (v >> 1);
		}

		INLINE int8_t find_latest_backward(int16_t shiftedOrderHints[REFS_PER_FRAME], bool usedFrame[REFS_PER_FRAME], uint8_t curFrameHint) {
			int8_t ref = -1;
			int16_t latestOrderHint = -1;
			for (int8_t i = 0; i < NUM_REF_FRAMES; i++) {
				auto hint = shiftedOrderHints[i];
				if (!usedFrame[i] &&
					hint >= curFrameHint &&
					(ref < 0 || hint >= latestOrderHint)) {
					ref = i;
					latestOrderHint = hint;
				}
			}
			return ref;
		}

		INLINE int8_t find_earliest_backward(int16_t shiftedOrderHints[REFS_PER_FRAME], bool usedFrame[REFS_PER_FRAME], uint8_t curFrameHint) {
			int8_t ref = -1;
			int16_t earliestOrderHint = INT8_MAX;
			for (int8_t i = 0; i < NUM_REF_FRAMES; i++) {
				auto hint = shiftedOrderHints[i];
				if (!usedFrame[i] &&
					hint >= curFrameHint &&
					(ref < 0 || hint < earliestOrderHint)) {
					ref = i;
					earliestOrderHint = hint;
				}
			}
			return ref;
		}

		INLINE int8_t find_latest_forward(int16_t shiftedOrderHints[REFS_PER_FRAME], bool usedFrame[REFS_PER_FRAME], uint8_t curFrameHint) {
			int8_t ref = -1;
			int16_t latestOrderHint = -1;
			for (int8_t i = 0; i < NUM_REF_FRAMES; i++) {
				auto hint = shiftedOrderHints[i];
				if (!usedFrame[i] &&
					hint < curFrameHint &&
					(ref < 0 || hint >= latestOrderHint)) {
					ref = i;
					latestOrderHint = hint;
				}
			}
			return ref;
		}
	}
}

#endif
