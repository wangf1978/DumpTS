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
#include "h265_video.h"
#include "AMException.h"
#include "AMRingBuffer.h"

#include "NAL.h"
#include "DataUtil.h"
#include "nal_com.h"

const char* hevc_profile_name[36] = {
	"Monochrome",
	"Monochrome 10",
	"Monochrome 12",
	"Monochrome 16",
	"Main",
	"Screen-Extended Main",
	"Main 10",
	"Screen-Extended Main 10",
	"Main 12",
	"Main Still Picture",
	"Main 10 Still Picture",
	"Main 4:2:2 10",
	"Main 4:2:2 12",
	"Main 4:4:4",
	"High Throughput 4:4:4",
	"Screen-Extended Main 4:4:4",
	"Screen-Extended High Throughput 4:4:4",
	"Main 4:4:4 10",
	"High Throughput 4:4:4 10",
	"Screen-Extended Main 4:4:4 10",
	"Screen-Extended High Throughput 4:4:4 10",
	"Main 4:4:4 12",
	"High Throughput 4:4:4 14",
	"Screen-Extended High Throughput 4:4:4 14",
	"Main Intra",
	"Main 10 Intra",
	"Main 12 Intra",
	"Main 4:2:2 10 Intra",
	"Main 4:2:2 12 Intra",
	"Main 4:4:4 Intra",
	"Main 4:4:4 10 Intra",
	"Main 4:4:4 12 Intra",
	"Main 4:4:4 16 Intra",
	"Main 4:4:4 Still Picture",
	"Main 4:4:4 16 Still Picture",
	"High Throughput 4:4:4 16 Intra"
};

const char* hevc_nal_unit_type_names[64] = {
	/*00*/ "VCL::TRAIL_N",
	/*01*/ "VCL::TRAIL_R",
	/*02*/ "VCL::TSA_N",
	/*03*/ "VCL::TSA_R",
	/*04*/ "VCL::STSA_N",
	/*05*/ "VCL::STSA_R",
	/*06*/ "VCL::RADL_N",
	/*07*/ "VCL::RADL_R",
	/*08*/ "VCL::RASL_N",
	/*09*/ "VCL::RASL_R",
	/*10*/ "VCL::RSV_VCL_N10",
	/*11*/ "VCL::RSV_VCL_R11",
	/*12*/ "VCL::RSV_VCL_N12",
	/*13*/ "VCL::RSV_VCL_R13",
	/*14*/ "VCL::RSV_VCL_N14",
	/*15*/ "VCL::RSV_VCL_R15",
	/*16*/ "VCL::BLA_W_LP Coded",
	/*17*/ "VCL::BLA_W_RADL",
	/*18*/ "VCL::BLA_N_LP",
	/*19*/ "VCL::IDR_W_RADL",
	/*20*/ "VCL::IDR_N_LP",
	/*21*/ "VCL::CRA_NUT",
	/*22*/ "VCL::RSV_IRAP_VCL22",
	/*23*/ "VCL::RSV_IRAP_VCL23",
	/*24*/ "VCL::RSV_VCL24",
	/*25*/ "VCL::RSV_VCL25",
	/*26*/ "VCL::RSV_VCL26",
	/*27*/ "VCL::RSV_VCL27",
	/*28*/ "VCL::RSV_VCL28",
	/*29*/ "VCL::RSV_VCL29",
	/*30*/ "VCL::RSV_VCL30",
	/*31*/ "VCL::RSV_VCL31",
	/*32*/ "non-VCL::VPS_NUT",
	/*33*/ "non-VCL::SPS_NUT",
	/*34*/ "non-VCL::PPS_NUT",
	/*35*/ "non-VCL::AUD_NUT",
	/*36*/ "non-VCL::EOS_NUT",
	/*37*/ "non-VCL::EOB_NUT",
	/*38*/ "non-VCL::FD_NUT",
	/*39*/ "non-VCL::PREFIX_SEI_NUT",
	/*40*/ "non-VCL::SUFFIX_SEI_NUT",
	/*41*/ "non-VCL::RSV_NVCL41",
	/*42*/ "non-VCL::RSV_NVCL42",
	/*43*/ "non-VCL::RSV_NVCL43",
	/*44*/ "non-VCL::RSV_NVCL44",
	/*45*/ "non-VCL::RSV_NVCL45",
	/*46*/ "non-VCL::RSV_NVCL46",
	/*47*/ "non-VCL::RSV_NVCL47",
	/*48*/ "non-VCL::UNSPEC48",
	/*49*/ "non-VCL::UNSPEC49",
	/*50*/ "non-VCL::UNSPEC50",
	/*51*/ "non-VCL::UNSPEC51",
	/*52*/ "non-VCL::UNSPEC52",
	/*53*/ "non-VCL::UNSPEC53",
	/*54*/ "non-VCL::UNSPEC54",
	/*55*/ "non-VCL::UNSPEC55",
	/*56*/ "non-VCL::UNSPEC56",
	/*57*/ "non-VCL::UNSPEC57",
	/*58*/ "non-VCL::UNSPEC58",
	/*59*/ "non-VCL::UNSPEC59",
	/*60*/ "non-VCL::UNSPEC60",
	/*61*/ "non-VCL::UNSPEC61",
	/*62*/ "non-VCL::UNSPEC62",
	/*63*/ "non-VCL::UNSPEC63",
};

const char* hevc_nal_unit_type_short_names[64] = {
	/*00*/ "TRAIL_N",
	/*01*/ "TRAIL_R",
	/*02*/ "TSA_N",
	/*03*/ "TSA_R",
	/*04*/ "STSA_N",
	/*05*/ "STSA_R",
	/*06*/ "RADL_N",
	/*07*/ "RADL_R",
	/*08*/ "RASL_N",
	/*09*/ "RASL_R",
	/*10*/ "RSV_VCL_N10",
	/*11*/ "RSV_VCL_R11",
	/*12*/ "RSV_VCL_N12",
	/*13*/ "RSV_VCL_R13",
	/*14*/ "RSV_VCL_N14",
	/*15*/ "RSV_VCL_R15",
	/*16*/ "BLA_W_LP Coded",
	/*17*/ "BLA_W_RADL",
	/*18*/ "BLA_N_LP",
	/*19*/ "IDR_W_RADL",
	/*20*/ "IDR_N_LP",
	/*21*/ "CRA_NUT",
	/*22*/ "RSV_IRAP_VCL22",
	/*23*/ "RSV_IRAP_VCL23",
	/*24*/ "RSV_VCL24",
	/*25*/ "RSV_VCL25",
	/*26*/ "RSV_VCL26",
	/*27*/ "RSV_VCL27",
	/*28*/ "RSV_VCL28",
	/*29*/ "RSV_VCL29",
	/*30*/ "RSV_VCL30",
	/*31*/ "RSV_VCL31",
	/*32*/ "VPS_NUT",
	/*33*/ "SPS_NUT",
	/*34*/ "PPS_NUT",
	/*35*/ "AUD_NUT",
	/*36*/ "EOS_NUT",
	/*37*/ "EOB_NUT",
	/*38*/ "FD_NUT",
	/*39*/ "PREFIX_SEI_NUT",
	/*40*/ "SUFFIX_SEI_NUT",
	/*41*/ "RSV_NVCL41",
	/*42*/ "RSV_NVCL42",
	/*43*/ "RSV_NVCL43",
	/*44*/ "RSV_NVCL44",
	/*45*/ "RSV_NVCL45",
	/*46*/ "RSV_NVCL46",
	/*47*/ "RSV_NVCL47",
	/*48*/ "UNSPEC48",
	/*49*/ "UNSPEC49",
	/*50*/ "UNSPEC50",
	/*51*/ "UNSPEC51",
	/*52*/ "UNSPEC52",
	/*53*/ "UNSPEC53",
	/*54*/ "UNSPEC54",
	/*55*/ "UNSPEC55",
	/*56*/ "UNSPEC56",
	/*57*/ "UNSPEC57",
	/*58*/ "UNSPEC58",
	/*59*/ "UNSPEC59",
	/*60*/ "UNSPEC60",
	/*61*/ "UNSPEC61",
	/*62*/ "UNSPEC62",
	/*63*/ "UNSPEC63",
};

const char* hevc_nal_unit_type_descs[64] = {
	/*00*/ "VCL::TRAIL_N Coded slice segment of a non-TSA, non-STSA trailing picture slice_segment_layer_rbsp()",
	/*01*/ "VCL::TRAIL_R Coded slice segment of a non-TSA, non-STSA trailing picture slice_segment_layer_rbsp()",
	/*02*/ "VCL::TSA_N Coded slice segment of a TSA picture slice_segment_layer_rbsp()",
	/*03*/ "VCL::TSA_R Coded slice segment of a TSA picture slice_segment_layer_rbsp()",
	/*04*/ "VCL::STSA_N Coded slice segment of an STSA picture slice_segment_layer_rbsp( )",
	/*05*/ "VCL::STSA_R Coded slice segment of an STSA picture slice_segment_layer_rbsp( )",
	/*06*/ "VCL::RADL_N Coded slice segment of a RADL picture slice_segment_layer_rbsp( )",
	/*07*/ "VCL::RADL_R Coded slice segment of a RADL picture slice_segment_layer_rbsp( )",
	/*08*/ "VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( )",
	/*09*/ "VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( )",
	/*10*/ "VCL::RSV_VCL_N10 Reserved non-IRAP SLNR VCL NAL unit types",
	/*11*/ "VCL::RSV_VCL_R11 Reserved non-IRAP sub-layer reference VCL NAL unit types",
	/*12*/ "VCL::RSV_VCL_N12 Reserved non-IRAP SLNR VCL NAL unit types",
	/*13*/ "VCL::RSV_VCL_R13 Reserved non-IRAP sub-layer reference VCL NAL unit types",
	/*14*/ "VCL::RSV_VCL_N14 Reserved non-IRAP SLNR VCL NAL unit types",
	/*15*/ "VCL::RSV_VCL_R15 Reserved non-IRAP sub-layer reference VCL NAL unit types",
	/*16*/ "VCL::BLA_W_LP Coded slice segment of a BLA picture slice_segment_layer_rbsp( )",
	/*17*/ "VCL::BLA_W_RADL Coded slice segment of a BLA picture slice_segment_layer_rbsp( )",
	/*18*/ "VCL::BLA_N_LP Coded slice segment of a BLA picture slice_segment_layer_rbsp( )",
	/*19*/ "VCL::IDR_W_RADL Coded slice segment of an IDR picture slice_segment_layer_rbsp( )",
	/*20*/ "VCL::IDR_N_LP Coded slice segment of an IDR picture slice_segment_layer_rbsp( )",
	/*21*/ "VCL::CRA_NUT Coded slice segment of a CRA picture slice_segment_layer_rbsp( )",
	/*22*/ "VCL::RSV_IRAP_VCL22 Reserved IRAP VCL NAL unit types",
	/*23*/ "VCL::RSV_IRAP_VCL23 Reserved IRAP VCL NAL unit types",
	/*24*/ "VCL::RSV_VCL24 Reserved non-IRAP VCL NAL unit types",
	/*25*/ "VCL::RSV_VCL25 Reserved non-IRAP VCL NAL unit types",
	/*26*/ "VCL::RSV_VCL26 Reserved non-IRAP VCL NAL unit types",
	/*27*/ "VCL::RSV_VCL27 Reserved non-IRAP VCL NAL unit types",
	/*28*/ "VCL::RSV_VCL28 Reserved non-IRAP VCL NAL unit types",
	/*29*/ "VCL::RSV_VCL29 Reserved non-IRAP VCL NAL unit types",
	/*30*/ "VCL::RSV_VCL30 Reserved non-IRAP VCL NAL unit types",
	/*31*/ "VCL::RSV_VCL31 Reserved non-IRAP VCL NAL unit types",
	/*32*/ "non-VCL::VPS_NUT Video parameter set video_parameter_set_rbsp( )",
	/*33*/ "non-VCL::SPS_NUT Sequence parameter set seq_parameter_set_rbsp( )",
	/*34*/ "non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( )", 
	/*35*/ "non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( )", 
	/*36*/ "non-VCL::EOS_NUT End of sequence end_of_seq_rbsp( )",
	/*37*/ "non-VCL::EOB_NUT End of bitstream end_of_bitstream_rbsp( )",
	/*38*/ "non-VCL::FD_NUT Filler data filler_data_rbsp( )",
	/*39*/ "non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( )",
	/*40*/ "non-VCL::SUFFIX_SEI_NUT Supplemental enhancement information sei_rbsp( )",
	/*41*/ "non-VCL::RSV_NVCL41 Reserved",
	/*42*/ "non-VCL::RSV_NVCL42 Reserved",
	/*43*/ "non-VCL::RSV_NVCL43 Reserved",
	/*44*/ "non-VCL::RSV_NVCL44 Reserved",
	/*45*/ "non-VCL::RSV_NVCL45 Reserved",
	/*46*/ "non-VCL::RSV_NVCL46 Reserved",
	/*47*/ "non-VCL::RSV_NVCL47 Reserved",
	/*48*/ "non-VCL::UNSPEC48 Unspecified",
	/*49*/ "non-VCL::UNSPEC49 Unspecified",
	/*50*/ "non-VCL::UNSPEC50 Unspecified",
	/*51*/ "non-VCL::UNSPEC51 Unspecified",
	/*52*/ "non-VCL::UNSPEC52 Unspecified",
	/*53*/ "non-VCL::UNSPEC53 Unspecified",
	/*54*/ "non-VCL::UNSPEC54 Unspecified",
	/*55*/ "non-VCL::UNSPEC55 Unspecified",
	/*56*/ "non-VCL::UNSPEC56 Unspecified",
	/*57*/ "non-VCL::UNSPEC57 Unspecified",
	/*58*/ "non-VCL::UNSPEC58 Unspecified",
	/*59*/ "non-VCL::UNSPEC59 Unspecified",
	/*60*/ "non-VCL::UNSPEC60 Unspecified",
	/*61*/ "non-VCL::UNSPEC61 Unspecified",
	/*62*/ "non-VCL::UNSPEC62 Unspecified",
	/*63*/ "non-VCL::UNSPEC63 Unspecified",
};

const std::map<uint8_t, GENERAL_TIER_AND_LEVEL_LIMIT> general_tier_and_level_limits = {
	/* 30*/	{30,	{ 36864,	350,	(uint32_t)-1, 16,	 1,	 1 , 552960		,1280	,(uint32_t)-1	,2	,2}},	// 1
	/* 60*/	{60,	{ 122880,	1500,	(uint32_t)-1, 16,	 1,	 1 , 3686400 	,1500	,(uint32_t)-1	,2 	,2}},	// 2,
	/* 63*/	{63,	{ 245760,	300,	(uint32_t)-1, 20,	 1,	 1 , 7372800 	,3000	,(uint32_t)-1	,2 	,2}},	// 2.1
	/* 90*/	{90,	{ 552960,	6000,	(uint32_t)-1, 30,	 2,	 2 , 16588800 	,6000	,(uint32_t)-1	,2 	,2}},	// 3
	/* 93*/	{93,	{ 983040,	10000,	(uint32_t)-1, 40,	 3,	 3 , 33177600 	,10000	,(uint32_t)-1	,2 	,2}},	// 3.1
	/*120*/	{120,	{ 2228224,	12000,	30000,		  75,	 5,	 5 , 66846720 	,12000 	,30000 			,4 	,4}},	// 4
	/*123*/	{123,	{ 2228224,	20000,	50000,		  75,	 5,	 5 , 133693440 	,20000 	,50000 			,4 	,4}},	// 4.1
	/*150*/	{150,	{ 8912896,	25000,	100000,		  200,	 11, 10, 267386880 	,25000 	,100000 		,6 	,4}},	// 5
	/*153*/	{153,	{ 8912896,	40000,	160000,		  200,	 11, 10, 534773760 	,40000 	,160000 		,8 	,4}},	// 5.1
	/*156*/	{156,	{ 8912896,	60000,	240000,		  200,	 11, 10, 1069547520 ,60000 	,240000 		,8 	,4}},	// 5.2
	/*180*/	{180,	{ 35651584,	60000,	240000,		  600,	 22, 20, 1069547520 ,60000 	,240000 		,8 	,4}},	// 6
	/*183*/	{183,	{ 35651584,	120000,	480000,		  600,	 22, 20, 2139095040 ,120000 ,480000 		,8 	,4}},	// 6.1
	/*186*/	{186,	{ 35651584,	240000,	800000,		  600,	 22, 20, 4278190080 ,240000 ,800000 		,6 	,4}},	// 6.2
};

const HEVC_PROFILE_FACTOR hevc_profile_factors[36] =
{
	/* Monochrome								*/	{667,	733,	1.000f,	1.0f},
	/* Monochrome 10							*/	{833,	917,	1.250f,	1.0f},
	/* Monochrome 12							*/	{1000,	1100,	1.500f,	1.0f},
	/* Monochrome 16							*/	{1333,	1467,	2.000f,	1.0f},
	/* Main										*/	{1000,	1100,	1.500f,	1.0f},
	/* Screen-Extended Main						*/	{1000,	1100,	1.500f,	1.0f},
	/* Main 10									*/	{1000,	1100,	1.875f,	1.0f},
	/* Screen-Extended Main 10					*/	{1000,	1100,	1.875f,	1.0f},
	/* Main 12									*/	{1500,	1650,	2.250f,	1.0f},
	/* Main Still Picture						*/	{1000,	1100,	1.500f,	1.0f},
	/* Main 10 Still Picture					*/	{1000,	1100,	1.875f,	1.0f},
	/* Main 4:2:2 10							*/	{1667,	1833,	2.500f,	0.5f},
	/* Main 4:2:2 12							*/	{2000,	2200,	3.000f,	0.5f},
	/* Main 4:4:4								*/	{2000,	2200,	3.000f,	0.5f},
	/* High Throughput 4:4:4					*/	{2000,	2200,	3.000f,	0.5f},
	/* Screen-Extended Main 4:4:4				*/	{2000,	2200,	3.000f,	0.5f},
	/* Screen-Extended High Throughput 4:4:4	*/	{2000,	2200,	3.000f,	0.5f},
	/* Main 4:4:4 10							*/	{2500,	2750,	3.750f,	0.5f},
	/* High Throughput 4:4:4 10					*/	{2500,	2750,	3.750f,	0.5f},
	/* Screen-Extended Main 4:4:4 10			*/	{2500,	2750,	3.750f,	0.5f},
	/* Screen-Extended High Throughput 4:4:4 10	*/	{2500,	2750,	3.750f,	0.5f},
	/* Main 4:4:4 12							*/	{3000,	3300,	4.500f,	0.5f},
	/* High Throughput 4:4:4 14					*/	{3500,	3850,	5.250f,	0.5f},
	/* Screen-Extended High Throughput 4:4:4 14	*/	{3500,	3850,	5.250f,	0.5f},
	/* Main Intra								*/	{1000,	1100,	1.500f,	1.0f},
	/* Main 10 Intra							*/	{1000,	1100,	1.875f,	1.0f},
	/* Main 12 Intra							*/	{1500,	1650,	2.250f,	1.0f},
	/* Main 4:2:2 10 Intra						*/	{1667,	1833,	2.500f,	0.5f},
	/* Main 4:2:2 12 Intra						*/	{2000,	2200,	3.000f,	0.5f},
	/* Main 4:4:4 Intra							*/	{2000,	2200,	3.000f,	0.5f},
	/* Main 4:4:4 10 Intra						*/	{2500,	2750,	3.750f,	0.5f},
	/* Main 4:4:4 12 Intra						*/	{3000,	3300,	4.500f,	0.5f},
	/* Main 4:4:4 16 Intra						*/	{4000,	4400,	6.000f,	0.5f},
	/* Main 4:4:4 Still Picture					*/	{2000,	2200,	3.000f,	0.5f},
	/* Main 4:4:4 16 Still Picture				*/	{4000,	4400,	6.000f,	0.5f},
	/* High Throughput 4:4:4 16 Intra			*/	{4000,	4400,	6.000f,	0.5f},
};

const char* get_hevc_profile_name(int profile)
{
	if (profile >= 0 && (size_t)profile < sizeof(hevc_profile_name) / sizeof(hevc_profile_name[0]))
		return hevc_profile_name[profile];

	return "Unknown HEVC Profile";
}

GENERAL_TIER_AND_LEVEL_LIMIT get_hevc_tier_and_level_limit(uint8_t level_idc)
{
	auto iter = general_tier_and_level_limits.find(level_idc);
	if (iter == general_tier_and_level_limits.cend())
		return {(uint32_t)-1, (uint32_t)-1, (uint32_t)-1, (uint16_t)-1, (uint8_t)-1, (uint8_t)-1, (uint32_t)-1, (uint32_t)-1, (uint32_t)-1, (uint8_t)-1, (uint8_t)-1};

	return iter->second;
}

const int hevc_sample_aspect_ratio[17][2] = {
	{0, 0}, {1,1}, {12, 11}, {10,11}, {16,11}, {40, 33}, {24, 11}, {20, 11}, {32, 11}, {80, 33}, {18, 11}, {15, 11}, {64, 33}, {160, 99 }, {4, 3}, {3, 2}, {2, 1}
};

RET_CODE CreateHEVCNALContext(INALHEVCContext** ppNALCtx)
{
	if (ppNALCtx == NULL)
		return RET_CODE_INVALID_PARAMETER;

	auto pCtx = new BST::H265Video::VideoBitstreamCtx();
	pCtx->AddRef();
	*ppNALCtx = (INALHEVCContext*)pCtx;
	return RET_CODE_SUCCESS;
}

namespace BST {

	float get_hevc_sar(uint8_t aspect_ratio_idc, uint16_t sar_width, uint16_t sar_height)
	{
		if (aspect_ratio_idc > 0 && aspect_ratio_idc < sizeof(hevc_sample_aspect_ratio) / sizeof(hevc_sample_aspect_ratio[0]))
			return (float)hevc_sample_aspect_ratio[aspect_ratio_idc][0] / hevc_sample_aspect_ratio[aspect_ratio_idc][1];

		if (aspect_ratio_idc == 255 && sar_height != 0)
			return (float)sar_width / sar_height;

		return 0.f;
	}

	namespace H265Video {

		RET_CODE VideoBitstreamCtx::SetNUFilters(std::initializer_list<uint8_t> NU_type_filters)
		{
			nal_unit_type_filters = NU_type_filters;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::GetNUFilters(std::vector<uint8_t>& NU_type_filters)
		{
			NU_type_filters = nal_unit_type_filters;

			return RET_CODE_SUCCESS;
		}

		bool VideoBitstreamCtx::IsNUFiltered(uint8_t nal_unit_type)
		{
			return nal_unit_type_filters.empty() ||
				std::find(nal_unit_type_filters.cbegin(), nal_unit_type_filters.cend(), nal_unit_type) != nal_unit_type_filters.cend();
		}

		void VideoBitstreamCtx::Reset()
		{
			nal_unit_type_filters.clear();
			sps_seq_parameter_set_id.clear();
			prev_vps_video_parameter_set_id = -1;
			sp_h265_vpses.clear();
			sp_h265_spses.clear(); 
			sp_h265_ppses.clear(); 
			sp_prev_nal_unit = nullptr; 
			m_active_nu_type = -1;
		}

		H265_NU VideoBitstreamCtx::GetHEVCVPS(uint8_t vps_id)
		{
			auto iter = sp_h265_vpses.find(vps_id);
			if (iter != sp_h265_vpses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		H265_NU VideoBitstreamCtx::GetHEVCSPS(uint8_t sps_id)
		{
			auto iter = sp_h265_spses.find(sps_id);
			if (iter != sp_h265_spses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		H265_NU VideoBitstreamCtx::GetHEVCPPS(uint8_t pps_id)
		{
			auto iter = sp_h265_ppses.find(pps_id);
			if (iter != sp_h265_ppses.end())
				return iter->second;

			return std::shared_ptr<NAL_UNIT>();
		}

		RET_CODE VideoBitstreamCtx::UpdateHEVCVPS(H265_NU vps_nu)
		{
			if (!vps_nu || vps_nu->nal_unit_header.nal_unit_type != VPS_NUT || vps_nu->ptr_video_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			sp_h265_vpses[vps_nu->ptr_video_parameter_set_rbsp->vps_video_parameter_set_id] = vps_nu;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::UpdateHEVCSPS(H265_NU sps_nu)
		{
			if (!sps_nu || sps_nu->nal_unit_header.nal_unit_type != SPS_NUT || sps_nu->ptr_seq_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			sp_h265_spses[sps_nu->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id] = sps_nu;
			return RET_CODE_SUCCESS;

		}

		RET_CODE VideoBitstreamCtx::UpdateHEVCPPS(H265_NU pps_nu)
		{
			if (!pps_nu || pps_nu->nal_unit_header.nal_unit_type != PPS_NUT || pps_nu->ptr_pic_parameter_set_rbsp == nullptr)
				return RET_CODE_INVALID_PARAMETER;

			sp_h265_ppses[pps_nu->ptr_pic_parameter_set_rbsp->pps_pic_parameter_set_id] = pps_nu;
			return RET_CODE_SUCCESS;
		}

		H265_NU VideoBitstreamCtx::CreateHEVCNU()
		{
			auto ptr_HEVC_NU = new NAL_UNIT;
			ptr_HEVC_NU->UpdateCtx(this);
			return std::shared_ptr<NAL_UNIT>(ptr_HEVC_NU);
		}

		int8_t VideoBitstreamCtx::GetActiveSPSID() 
		{
			return m_active_sps_id;
		}

		RET_CODE VideoBitstreamCtx::ActivateSPS(int8_t sps_id)
		{
			m_active_sps_id = sps_id;
			return RET_CODE_SUCCESS;
		}

		RET_CODE VideoBitstreamCtx::DetactivateSPS()
		{
			m_active_sps_id = -1;
			return RET_CODE_SUCCESS;
		}

	}	// namespace H265Video
}	// namespace BST

