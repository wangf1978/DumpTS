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
#include "tinyxml2.h"
#include "AMSHA1.h"
#include "DataUtil.h"
#include "nal_parser.h"

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

const char* nal_unit_type_name[64] = {
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"Reserved non-IRAP SLNR VCL",
	"Reserved non-IRAP sub-layer reference VCL",
	"Reserved non-IRAP SLNR VCL",
	"Reserved non-IRAP sub-layer reference VCL",
	"Reserved non-IRAP SLNR VCL",
	"Reserved non-IRAP sub-layer reference VCL",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"slice_segment_layer_rbsp",
	"Reserved IRAP VCL",
	"Reserved IRAP VCL",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"Reserved non-IRAP",
	"video_parameter_set_rbsp",
	"seq_parameter_set_rbsp",
	"pic_parameter_set_rbsp",
	"access_unit_delimiter_rbsp",
	"end_of_seq_rbsp",
	"end_of_bitstream_rbsp",
	"filler_data_rbsp",
	"sei_rbsp",
	"sei_rbsp",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
	"Unspecified",
};

const GENERAL_TIER_AND_LEVEL_LIMIT general_tier_and_level_limits[256] = {
	/*  0*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  1*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  2*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  3*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  4*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  5*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  6*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  7*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  8*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*  9*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 10*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 11*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 12*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 13*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 14*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 15*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 16*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 17*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 18*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 19*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 20*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 21*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 22*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 23*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 24*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 25*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 26*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 27*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 28*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 29*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 30*/	{36864,			350,			(uint32_t)-1,	16,				1,				1 } ,				// 1
	/* 31*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 32*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 33*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 34*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 35*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 36*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 37*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 38*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 39*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 40*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 41*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 42*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 43*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 44*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 45*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 46*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 47*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 48*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 49*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 50*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 51*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 52*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 53*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 54*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 55*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 56*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 57*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 58*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 59*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 60*/	{122880,		1500,			(uint32_t)-1,	16,				1,				1 },	// 2
	/* 61*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 62*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 63*/	{245760,		300,			(uint32_t)-1,	20,				1,				1 },	// 2.1
	/* 64*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 65*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 66*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 67*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 68*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 69*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 70*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 71*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 72*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 73*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 74*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 75*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 76*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 77*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 78*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 79*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 80*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 81*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 82*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 83*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 84*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 85*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 86*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 87*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 88*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 89*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 90*/	{552960,		6000,			(uint32_t)-1,	30,				2,				2 },	// 3
	/* 91*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 92*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 93*/	{983040,		10000,			(uint32_t)-1,	40,				3,				3 },	// 3.1
	/* 94*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 95*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 96*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 97*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 98*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/* 99*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*100*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*101*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*102*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*103*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*104*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*105*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*106*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*107*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*108*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*109*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*110*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*111*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*112*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*113*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*114*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*115*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*116*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*117*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*118*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*119*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*120*/	{ 2228224,		12000,			30000,			75,				5,				5 },	// 4
	/*121*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*122*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*123*/	{ 2228224,		20000,			50000,			75,				5,				5 },	// 4.1
	/*124*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*125*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*126*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*127*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*128*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*129*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*130*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*131*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*132*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*133*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*134*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*135*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*136*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*137*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*138*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*139*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*140*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*141*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*142*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*143*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*144*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*145*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*146*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*147*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*148*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*149*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*150*/	{ 8912896,		25000,			100000,			200,			11,				10},	// 5
	/*151*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*152*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*153*/	{ 8912896,		40000,			160000,			200,			11,				10 },	// 5.1
	/*154*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*155*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*156*/	{ 8912896,		60000,			240000,			200,			11,				10 },	// 5.2
	/*157*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*158*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*159*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*160*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*161*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*162*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*163*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*164*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*165*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*166*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*167*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*168*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*169*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*170*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*171*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*172*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*173*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*174*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*175*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*176*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*177*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*178*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*179*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*180*/	{ 35651584,		60000,			240000,			600,			22,				20},	// 6
	/*181*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*182*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*183*/	{ 35651584,		120000,			480000,			600,			22,				20 },	// 6.1
	/*184*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*185*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*186*/	{ 35651584,		240000,			800000,			600,			22,				20 },	// 6.2
	/*187*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*188*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*189*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*190*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*191*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*192*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*193*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*194*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*195*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*196*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*197*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*198*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*199*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*200*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*201*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*202*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*203*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*204*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*205*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*206*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*207*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*208*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*209*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*210*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*211*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*212*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*213*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*214*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*215*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*216*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*217*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*218*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*219*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*220*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*221*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*222*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*223*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*224*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*225*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*226*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*227*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*228*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*229*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*230*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*231*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*232*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*233*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*234*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*235*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*236*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*237*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*238*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*239*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*240*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*241*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*242*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*243*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*244*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*245*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*246*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*247*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*248*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*249*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*250*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*251*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*252*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*253*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*254*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
	/*255*/	{(uint32_t)-1,	(uint32_t)-1,	(uint32_t)-1,	(uint16_t)-1,	(uint8_t)-1,	(uint8_t)-1 },
};

const char* get_hevc_profile_name(int profile)
{
	switch (profile)
	{
	case BST::H265Video::MAIN_PROFILE: return "Main Profile"; break;
	case BST::H265Video::MAIN_10_PROFILE: return "Main 10 Profile"; break;
	case BST::H265Video::MAIN_STILL_PICTURE_PROFILE: return "Main Still Picture Profile"; break;
	case BST::H265Video::MONOCHROME_PROFILE: return "Monochrome Profile"; break;
	case BST::H265Video::MONOCHROME_12_PROFILE: return "Monochrome 12 Profile"; break;
	case BST::H265Video::MONOCHROME_16_PROFILE: return "Monochrome 16 Profile"; break;
	case BST::H265Video::MAIN_12_PROFILE: return "Main 12 Profile"; break;
	case BST::H265Video::MAIN_422_10_PROFILE: return "Main 4:2:2 10 Profile"; break;
	case BST::H265Video::MAIN_422_12_PROFILE: return "Main 4:2:2 12 Profile"; break;
	case BST::H265Video::MAIN_444_PROFILE: return "Main 4:4:4 Profile"; break;
	case BST::H265Video::MAIN_444_10_PROFILE: return "Main 4:4:4 10 Profile"; break;
	case BST::H265Video::MAIN_444_12_PROFILE: return "Main 4:4:4 12 Profile"; break;
	case BST::H265Video::MAIN_INTRA_PROFILE: return "Main Intra Profile"; break;
	case BST::H265Video::MAIN_10_INTRA_PROFILE: return "Main 10 Intra Profile"; break;
	case BST::H265Video::MAIN_12_INTRA_PROFILE: return "Main 12 Intra Profile"; break;
	case BST::H265Video::MAIN_422_10_INTRA_PROFILE: return "Main 4:2:2 10 Intra Profile"; break;
	case BST::H265Video::MAIN_422_12_INTRA_PROFILE: return "Main 4:2:2 12 Intra Profile"; break;
	case BST::H265Video::MAIN_444_INTRA_PROFILE: return "Main 4:4:4 Intra Profile"; break;
	case BST::H265Video::MAIN_444_10_INTRA_PROFILE: return "Main 4:4:4 10 Intra Profile"; break;
	case BST::H265Video::MAIN_444_12_INTRA_PROFILE: return "Main 4:4:4 12 Intra Profile"; break;
	case BST::H265Video::MAIN_444_16_INTRA_PROFILE: return "Main 4:4:4 16 Intra Profile"; break;
	case BST::H265Video::MAIN_444_STILL_PICTURE: return "Main 4:4:4 Still Picture Profile"; break;
	case BST::H265Video::MAIN_444_16_STILL_PICTURE: return "Main 4:4:4 16 Still Picture Profile"; break;
	}

	return "Unknown HEVC Profile";
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

	}	// namespace H265Video
}	// namespace BST

