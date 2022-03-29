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
#include "sei.h"
#include "h265_video.h"
#include "h264_video.h"
#include <vector>

const char* sei_payload_type_names[256] = {
	/*  0*/"buffering_period",
	/*  1*/"pic_timing",
	/*  2*/"pan_scan_rect",
	/*  3*/"filler_payload",
	/*  4*/"user_data_registered_itu_t_t35",
	/*  5*/"user_data_unregistered",
	/*  6*/"recovery_point",
	/*  7*/"dec_ref_pic_marking_repetition",
	/*  8*/"spare_pic",
	/*  9*/"scene_info",
	/* 10*/"sub_seq_info",
	/* 11*/"sub_seq_layer_characteristics",
	/* 12*/"sub_seq_characteristics",
	/* 13*/"full_frame_freeze",
	/* 14*/"full_frame_freeze_release",
	/* 15*/"picture_snapshot",
	/* 16*/"progressive_refinement_segment_start",
	/* 17*/"progressive_refinement_segment_end",
	/* 18*/"motion_constrained_slice_group_set",
	/* 19*/"film_grain_characteristics",
	/* 20*/"deblocking_filter_display_preference",
	/* 21*/"stereo_video_info",
	/* 22*/"post_filter_hint",
	/* 23*/"tone_mapping_info",
	/* 24*/"scalability_info",
	/* 25*/"sub_pic_scalable_layer",
	/* 26*/"non_required_layer_rep",
	/* 27*/"priority_layer_info",
	/* 28*/"layers_not_present",
	/* 29*/"layer_dependency_change",
	/* 30*/"scalable_nesting",
	/* 31*/"base_layer_temporal_hrd",
	/* 32*/"quality_layer_integrity_check",
	/* 33*/"redundant_pic_property",
	/* 34*/"tl0_dep_rep_index",
	/* 35*/"tl_switching_point",
	/* 36*/"parallel_decoding_info",
	/* 37*/"mvc_scalable_nesting",
	/* 38*/"view_scalability_info",
	/* 39*/"multiview_scene_info",
	/* 40*/"multiview_acquisition_info",
	/* 41*/"non_required_view_component",
	/* 42*/"view_dependency_change",
	/* 43*/"operation_points_not_present",
	/* 44*/"base_view_temporal_hrd",
	/* 45*/"frame_packing_arrangement",
	/* 46*/"multiview_view_position",
	/* 47*/"display_orientation",
	/* 48*/"mvcd_scalable_nesting",
	/* 49*/"mvcd_view_scalability_info",
	/* 50*/"depth_representation_info",
	/* 51*/"three_dimensional_reference_displays_info",
	/* 52*/"depth_timing",
	/* 53*/"depth_sampling_info",
	/* 54*/"reserved_sei_payload",
	/* 55*/"reserved_sei_payload",
	/* 56*/"reserved_sei_payload",
	/* 57*/"reserved_sei_payload",
	/* 58*/"reserved_sei_payload",
	/* 59*/"reserved_sei_payload",
	/* 60*/"reserved_sei_payload",
	/* 61*/"reserved_sei_payload",
	/* 62*/"reserved_sei_payload",
	/* 63*/"reserved_sei_payload",
	/* 64*/"reserved_sei_payload",
	/* 65*/"reserved_sei_payload",
	/* 66*/"reserved_sei_payload",
	/* 67*/"reserved_sei_payload",
	/* 68*/"reserved_sei_payload",
	/* 69*/"reserved_sei_payload",
	/* 70*/"reserved_sei_payload",
	/* 71*/"reserved_sei_payload",
	/* 72*/"reserved_sei_payload",
	/* 73*/"reserved_sei_payload",
	/* 74*/"reserved_sei_payload",
	/* 75*/"reserved_sei_payload",
	/* 76*/"reserved_sei_payload",
	/* 77*/"reserved_sei_payload",
	/* 78*/"reserved_sei_payload",
	/* 79*/"reserved_sei_payload",
	/* 80*/"reserved_sei_payload",
	/* 81*/"reserved_sei_payload",
	/* 82*/"reserved_sei_payload",
	/* 83*/"reserved_sei_payload",
	/* 84*/"reserved_sei_payload",
	/* 85*/"reserved_sei_payload",
	/* 86*/"reserved_sei_payload",
	/* 87*/"reserved_sei_payload",
	/* 88*/"reserved_sei_payload",
	/* 89*/"reserved_sei_payload",
	/* 90*/"reserved_sei_payload",
	/* 91*/"reserved_sei_payload",
	/* 92*/"reserved_sei_payload",
	/* 93*/"reserved_sei_payload",
	/* 94*/"reserved_sei_payload",
	/* 95*/"reserved_sei_payload",
	/* 96*/"reserved_sei_payload",
	/* 97*/"reserved_sei_payload",
	/* 98*/"reserved_sei_payload",
	/* 99*/"reserved_sei_payload",
	/*100*/"reserved_sei_payload",
	/*101*/"reserved_sei_payload",
	/*102*/"reserved_sei_payload",
	/*103*/"reserved_sei_payload",
	/*104*/"reserved_sei_payload",
	/*105*/"reserved_sei_payload",
	/*106*/"reserved_sei_payload",
	/*107*/"reserved_sei_payload",
	/*108*/"reserved_sei_payload",
	/*109*/"reserved_sei_payload",
	/*110*/"reserved_sei_payload",
	/*111*/"reserved_sei_payload",
	/*112*/"reserved_sei_payload",
	/*113*/"reserved_sei_payload",
	/*114*/"reserved_sei_payload",
	/*115*/"reserved_sei_payload",
	/*116*/"reserved_sei_payload",
	/*117*/"reserved_sei_payload",
	/*118*/"reserved_sei_payload",
	/*119*/"reserved_sei_payload",
	/*120*/"reserved_sei_payload",
	/*121*/"reserved_sei_payload",
	/*122*/"reserved_sei_payload",
	/*123*/"reserved_sei_payload",
	/*124*/"reserved_sei_payload",
	/*125*/"reserved_sei_payload",
	/*126*/"reserved_sei_payload",
	/*127*/"reserved_sei_payload",
	/*128*/"structure_of_pictures_info",
	/*129*/"active_parameter_sets",
	/*130*/"decoding_unit_info",
	/*131*/"temporal_sub_layer_zero_index",
	/*132*/"reserved_sei_payload",
	/*133*/"scalable_nesting",
	/*134*/"region_refresh_info",
	/*135*/"no_display",
	/*136*/"time_code",
	/*137*/"mastering_display_colour_volume",
	/*138*/"segmented_rect_frame_packing_arrangement",
	/*139*/"temporal_motion_constrained_tile_sets",
	/*140*/"chroma_resampling_filter_hint",
	/*141*/"knee_function_info",
	/*142*/"colour_remapping_info",
	/*143*/"deinterlaced_field_identification",
	/*144*/"reserved_sei_payload",
	/*145*/"reserved_sei_payload",
	/*146*/"reserved_sei_payload",
	/*147*/"reserved_sei_payload",
	/*148*/"reserved_sei_payload",
	/*149*/"reserved_sei_payload",
	/*150*/"reserved_sei_payload",
	/*151*/"reserved_sei_payload",
	/*152*/"reserved_sei_payload",
	/*153*/"reserved_sei_payload",
	/*154*/"reserved_sei_payload",
	/*155*/"reserved_sei_payload",
	/*156*/"reserved_sei_payload",
	/*157*/"reserved_sei_payload",
	/*158*/"reserved_sei_payload",
	/*159*/"reserved_sei_payload",
	/*160*/"layers_not_present",
	/*161*/"inter_layer_constrained_tile_sets",
	/*162*/"bsp_nesting",
	/*163*/"bsp_initial_arrival_time",
	/*164*/"sub_bitstream_property",
	/*165*/"alpha_channel_info",
	/*166*/"overlay_info",
	/*167*/"temporal_mv_prediction_constraints",
	/*168*/"frame_field_info",
	/*169*/"reserved_sei_payload",
	/*170*/"reserved_sei_payload",
	/*171*/"reserved_sei_payload",
	/*172*/"reserved_sei_payload",
	/*173*/"reserved_sei_payload",
	/*174*/"reserved_sei_payload",
	/*175*/"reserved_sei_payload",
	/*176*/"three_dimensional_reference_displays_info",
	/*177*/"depth_representation_info",
	/*178*/"multiview_scene_info",
	/*179*/"multiview_acquisition_info",
	/*180*/"multiview_view_position",
	/*181*/"alternative_depth_info",
	/*182*/"reserved_sei_payload",
	/*183*/"reserved_sei_payload",
	/*184*/"reserved_sei_payload",
	/*185*/"reserved_sei_payload",
	/*186*/"reserved_sei_payload",
	/*187*/"reserved_sei_payload",
	/*188*/"reserved_sei_payload",
	/*189*/"reserved_sei_payload",
	/*190*/"reserved_sei_payload",
	/*191*/"reserved_sei_payload",
	/*192*/"reserved_sei_payload",
	/*193*/"reserved_sei_payload",
	/*194*/"reserved_sei_payload",
	/*195*/"reserved_sei_payload",
	/*196*/"reserved_sei_payload",
	/*197*/"reserved_sei_payload",
	/*198*/"reserved_sei_payload",
	/*199*/"reserved_sei_payload",
	/*200*/"reserved_sei_payload",
	/*201*/"reserved_sei_payload",
	/*202*/"reserved_sei_payload",
	/*203*/"reserved_sei_payload",
	/*204*/"reserved_sei_payload",
	/*205*/"reserved_sei_payload",
	/*206*/"reserved_sei_payload",
	/*207*/"reserved_sei_payload",
	/*208*/"reserved_sei_payload",
	/*209*/"reserved_sei_payload",
	/*210*/"reserved_sei_payload",
	/*211*/"reserved_sei_payload",
	/*212*/"reserved_sei_payload",
	/*213*/"reserved_sei_payload",
	/*214*/"reserved_sei_payload",
	/*215*/"reserved_sei_payload",
	/*216*/"reserved_sei_payload",
	/*217*/"reserved_sei_payload",
	/*218*/"reserved_sei_payload",
	/*219*/"reserved_sei_payload",
	/*220*/"reserved_sei_payload",
	/*221*/"reserved_sei_payload",
	/*222*/"reserved_sei_payload",
	/*223*/"reserved_sei_payload",
	/*224*/"reserved_sei_payload",
	/*225*/"reserved_sei_payload",
	/*226*/"reserved_sei_payload",
	/*227*/"reserved_sei_payload",
	/*228*/"reserved_sei_payload",
	/*229*/"reserved_sei_payload",
	/*230*/"reserved_sei_payload",
	/*231*/"reserved_sei_payload",
	/*232*/"reserved_sei_payload",
	/*233*/"reserved_sei_payload",
	/*234*/"reserved_sei_payload",
	/*235*/"reserved_sei_payload",
	/*236*/"reserved_sei_payload",
	/*237*/"reserved_sei_payload",
	/*238*/"reserved_sei_payload",
	/*239*/"reserved_sei_payload",
	/*240*/"reserved_sei_payload",
	/*241*/"reserved_sei_payload",
	/*242*/"reserved_sei_payload",
	/*243*/"reserved_sei_payload",
	/*244*/"reserved_sei_payload",
	/*245*/"reserved_sei_payload",
	/*246*/"reserved_sei_payload",
	/*247*/"reserved_sei_payload",
	/*248*/"reserved_sei_payload",
	/*249*/"reserved_sei_payload",
	/*250*/"reserved_sei_payload",
	/*251*/"reserved_sei_payload",
	/*252*/"reserved_sei_payload",
	/*253*/"reserved_sei_payload",
	/*254*/"reserved_sei_payload",
	/*255*/"reserved_sei_payload"
};

const char* sei_GOP_picture_structure_names[8] = {
	"field",
	"frame",
	"reserved",
	"frame with pic_struct field being set to 5 or 6(top field, bottom field, top field repeated/bottom field, top field, bottom field repeated)",
	"reserved",
	"frame with pic_struct field being set to 7(frame doubling)",
	"reserved",
	"frame with pic_struct field being set to 8(frame tripling)",
};

const char* sei_HEVC_GOP_picture_structure_names[8] = {
	"reserved",
	"frame",
	"reserved",
	"reserved",
	"reserved",
	"frame with pic_struct field being set to 7(frame doubling)",
	"reserved",
	"frame with pic_struct field being set to 8(frame tripling)",
};

const char* sei_GOP_picture_type_names[16] = {
	"reserved",
	"reserved",
	"non-reference B picture",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"I picture",
	"P picture",
	"reference B picture",
	"I/P picture that is necessary to decode the next AP P picture in decoding order",
	"reserved",
	"reserved",
	"reserved",
	"AP P picture"
};

const char* sei_HEVC_GOP_picture_type_names[16] = {
	"reserved",
	"reserved",
	"Non-reference Bb picture",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"I picture",
	"P picture or Bu picture",
	"Reference Bb picture",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"reserved"
};

const char* scene_transition_type_names[7] = {
	"No transition",
	"Fade to black",
	"Fade from black",
	"Unspecified transition from or to constant colour",
	"Dissolve",
	"Wipe",
	"Unspecified mixture of two scenes"
};

const char* film_grain_model_id_names[4] = {
	"Frequency filtering",
	"Auto - regression",
	"reserved",
	"reserved"
};

const char* blending_mode_id_names[4] = {
	"Additive",
	"Multiplicative",
	"reserved",
	"reserved"
};

const char* vui_colour_primaries_names[256] = {
	"For future use by ITU-T | ISO/IEC",
	"BT.709/BT.1361/sRGB/sYCC",
	"Unknown",
	"For future use by ITU-T | ISO/IEC",
	"BT.470-6 System M",
	"BT.470-6 System B, G/BT.601-6 625/BT.1358 625/BT.1700 625 PAL and 625 SECAM",
	"BT.601-6 525/BT.1358 525/BT.1700 NTSC/SMPTE 170M (2004)",
	"SMPTE 240M (1999)",
	"Generic film",
	"BT.2020",
	"SMPTE ST 428-1 (CIE 1931 XYZ)",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
};

const char* vui_transfer_characteristics_names[256] = {
	"Reserved",
	"BT.709-5/BT.1361",
	"Image characteristics are unknown or are determined by the application",
	"For future use by ITU-T | ISO/IEC",
	"BT.470-6 System M",
	"BT.470-6 System B, G",
	"BT.601-6 525 or 625/BT.1358 525 or 625/BT.1700 NTSC/SMPTE 170M (2004)",
	"SMPTE 240M (1999)",
	"Linear transfer characteristics",
	"Logarithmic transfer characteristic (100:1 range)",
	"Logarithmic transfer characteristic (100 * Sqrt( 10 ) : 1 range)",
	"IEC 61966-2-4",
	"BT.1361 extended colour gamut system",
	"IEC 61966-2-1 (sRGB or sYCC)",
	"BT.2020",
	"BT.2020",
	"SMPTE ST 2084 or BT.2100-2 PQ",
	"SMPTE ST 428-1",
	"BT.2100-2 hybrid log-gamma (HLG) system",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
};

const char* vui_matrix_coeffs_descs[256] = {
	"Identity, The identity matrix",
	"KR = 0.2126; KB = 0.0722, BT.709-5/BT.1361/sYCC/xvYCC709/Annex B of SMPTE RP 177 (1993)",
	"Unspecified, Image characteristics are unknown or are determined by the application",
	"For future use by ITU-T | ISO/IEC",
	"KR = 0.30; KB = 0.11 -- United States Federal Communications Commission Title 47 Code"
	"KR = 0.299; KB = 0.114 -- BT.470-6 System B, G/BT.601-6 625/BT.1358 625/BT.1700 625 PAL and 625 SECAM/xvYCC601",
	"KR = 0.299; KB = 0.114 -- BT.601-6 525/BT.1358 525/BT.1700 NTSC/SMPTE 170M (2004)",
	"KR = 0.212; KB = 0.087 -- SMPTE 240M (1999)",
	"YCgCo -- See Equations E-22 to E-36",
	"KR = 0.2627; KB = 0.0593 -- BT.2020 non-constant luminance system",
	"KR = 0.2627; KB = 0.0593 -- BT.2020 constant luminance system",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC",
	"For future use by ITU-T | ISO/IEC"
};

const std::tuple<int16_t, int16_t> sample_aspect_ratios[18] = {
	{0, 0},
	{1, 1},
	{12, 11},
	{10, 11},
	{16, 11},
	{40, 33},
	{24, 11},
	{20, 11},
	{32, 11},
	{80, 33},
	{18, 11},
	{15, 11},
	{64, 33},
	{160, 99},
	{4, 3},
	{3, 2},
	{2, 1},
	{0, 0},
};

const char* sample_aspect_ratio_descs[256] = {
	"Unspecified",
	"1:1(Square), example: 3840x2160 16:9 frame without horizontal overscan",
	"12:11, example: 720x576 4:3 frame with horizontal overscan",
	"10:11, example: 720x480 4:3 frame with horizontal overscan",
	"16:11, example: 720x576 16:9 frame with horizontal overscan",
	"40:33, example: 720x480 16:9 frame with horizontal overscan",
	"24:11, example: 352x576 4:3 frame without horizontal overscan",
	"20:11, example: 480x480 16:9 frame with horizontal overscan",
	"32:11, example: 352x576 16:9 frame without horizontal overscan",
	"80:33, example: 352x480 16:9 frame without horizontal overscan",
	"18:11, example: 480x576 4:3 frame with horizontal overscan",
	"15:11, example: 480x480 4:3 frame with horizontal overscan",
	"64:33, example: 528x576 16:9 frame without horizontal overscan",
	"160:99, example: 528x480 16:9 frame without horizontal overscan",
	"4:3, example: 1440x1080 16:9 frame without horizontal overscan",
	"3:2, example: 1280x1080 16:9 frame without horizontal overscan",
	"2:1, example: 960x1080 16:9 frame without horizontal overscan",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"EXTENDED_SAR"
};

const char* vui_video_format_names[7] = {
	"Component",
	"PAL",
	"NTSC"
	"SECAM",
	"MAC",
	"Unspecified video format",
	"Unspecified video format",
	"Unspecified video format"
};

int64_t GetUEFromUint64(uint64_t v, int bits)
{
	if (bits < (int)sizeof(v))
		throw AMException(RET_CODE_INVALID_PARAMETER, _T("Invalid parameter"));

	int leadingZeroBits = -1;
	for (bool b = false; !b && (leadingZeroBits + 1) < bits; leadingZeroBits++)
		b =  ((v>>(bits - leadingZeroBits - 2))&0x01)? true : false;

	if (2*leadingZeroBits + 1 > bits)
		throw AMException(RET_CODE_BUFFER_NOT_COMPATIBLE, _T("Bitstream is not compatible"));

	return (1LL << leadingZeroBits) - 1LL + ((v >> (bits - 2 * leadingZeroBits - 1))&((1LL << leadingZeroBits) - 1));
}

BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD::BUFFERING_PERIOD(int payloadSize, INALContext* pNALCtx)
	: bp_seq_parameter_set_id(0)
	, irap_cpb_params_present_flag(0)
	, concatenation_flag(0)
	, use_alt_cpb_params_flag(0)
	, reserved_0(0)
	, payload_size(payloadSize)
	, ptr_NAL_Context(pNALCtx)
{
}

int BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD::Map(AMBst in_bst)
{
	int iRet = RET_CODE_SUCCESS;
	SYNTAX_BITSTREAM_MAP::Map(in_bst);

	try
	{
		MAP_BST_BEGIN(0);
		uint64_t bitsValue = AMBst_PeekBits(in_bst, 9);
		bp_seq_parameter_set_id = (uint8_t)GetUEFromUint64(bitsValue, 9);

		bool bMappedSucceeded = false;
		H265_NU cur_h265_sps;
		H264_NU cur_h264_sps;
		NAL_CODING coding = ptr_NAL_Context->GetNALCoding();

		if (coding == NAL_CODING_AVC)
		{
			INALAVCContext* pAVCCtx = NULL;
			if (SUCCEEDED(ptr_NAL_Context->QueryInterface(IID_INALAVCContext, (void**)&pAVCCtx)))
			{
				cur_h264_sps = pAVCCtx->GetAVCSPS(bp_seq_parameter_set_id);

				pAVCCtx->ActivateSPS(bp_seq_parameter_set_id);

				pAVCCtx->Release();
			}
		}
		else if (coding == NAL_CODING_HEVC)
		{
			INALHEVCContext* pHEVCCtx = NULL;
			if (SUCCEEDED(ptr_NAL_Context->QueryInterface(IID_INALHEVCContext, (void**)&pHEVCCtx)))
			{
				cur_h265_sps = pHEVCCtx->GetHEVCSPS(bp_seq_parameter_set_id);
				pHEVCCtx->Release();
			}
		}

		if (coding == NAL_CODING_AVC)
		{
			if (!cur_h264_sps)
			{
				try
				{
					MAP_BST_BEGIN(0);
					uint8_t payload_byte;
					reserved_sei_message_payload_bytes.resize(payload_size);
					for (int i = 0; i < payload_size; i++) {
						nal_read_b(in_bst, payload_byte, 8, uint8_t);
						reserved_sei_message_payload_bytes[i] = payload_byte;
					}
					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}
			}
			else
			{
				nal_read_ue(in_bst, bp_seq_parameter_set_id, uint8_t);
				bool NalHrdBpPresentFlag = false, VclHrdBpPresentFlag = false;
				if (cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters)
				{
					NalHrdBpPresentFlag = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters_present_flag ? true : false;
					VclHrdBpPresentFlag = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters_present_flag ? true : false;
				}

				if (NalHrdBpPresentFlag)
				{
					uint8_t initial_cpb_removal_delay_length_minus1 =
						cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters->initial_cpb_removal_delay_length_minus1;
					size_t CpbCnt = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters->cpb_cnt_minus1;
					nal_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (size_t i = 0; i <= CpbCnt; i++)
					{
						nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
					}
				}

				if (VclHrdBpPresentFlag)
				{
					uint8_t initial_cpb_removal_delay_length_minus1 =
						cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters->initial_cpb_removal_delay_length_minus1;
					size_t CpbCnt = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters->cpb_cnt_minus1;
					vcl_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (size_t i = 0; i <= CpbCnt; i++)
					{
						nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
					}
				}
			}
		}
		else if (coding == NAL_CODING_HEVC)
		{
			if (cur_h265_sps == NULL)
			{
				try
				{
					MAP_BST_BEGIN(0);
					uint8_t payload_byte;
					reserved_sei_message_payload_bytes.resize(payload_size);
					for (int i = 0; i < payload_size; i++) {
						nal_read_b(in_bst, payload_byte, 8, uint8_t);
						reserved_sei_message_payload_bytes[i] = payload_byte;
					}
					MAP_BST_END();
				}
				catch (AMException e)
				{
					return e.RetCode();
				}
			}
			else
			{
				bool sub_pic_hrd_params_present_flag = false;
				bool NalHrdBpPresentFlag = false;
				bool VclHrdBpPresentFlag = false;
				uint8_t au_cpb_removal_delay_length_minus1 = 0;
				uint8_t dpb_output_delay_length_minus1 = 0;
				uint8_t initial_cpb_removal_delay_length_minus1 = 0;
				nal_read_ue(in_bst, bp_seq_parameter_set_id, uint8_t);
				if (cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters &&
					cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters)
				{
					sub_pic_hrd_params_present_flag = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_pic_hrd_params_present_flag;
					au_cpb_removal_delay_length_minus1 = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->au_cpb_removal_delay_length_minus1;
					dpb_output_delay_length_minus1 = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->dpb_output_delay_length_minus1;
					initial_cpb_removal_delay_length_minus1 = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->initial_cpb_removal_delay_length_minus1;
					NalHrdBpPresentFlag = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->nal_hrd_parameters_present_flag ? true : false;
					VclHrdBpPresentFlag = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->vcl_hrd_parameters_present_flag ? true : false;
				}

				if(!sub_pic_hrd_params_present_flag)
					nal_read_b(in_bst, irap_cpb_params_present_flag, 1, uint8_t);

				if (irap_cpb_params_present_flag)
				{
					nal_read_u(in_bst, cpb_delay_offset, au_cpb_removal_delay_length_minus1 + 1, uint32_t);
					nal_read_u(in_bst, dpb_delay_offset, dpb_output_delay_length_minus1 + 1, uint32_t);
				}

				nal_read_u(in_bst, concatenation_flag, 1, uint8_t);
				nal_read_u(in_bst, au_cpb_removal_delay_delta_minus1, au_cpb_removal_delay_length_minus1 + 1, uint32_t);

				if (NalHrdBpPresentFlag)
				{
					auto pSubLayer0Info = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
					size_t CpbCnt = pSubLayer0Info != NULL?pSubLayer0Info->cpb_cnt_minus1:0;
					nal_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (size_t i = 0; i <= CpbCnt; i++)
					{
						nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						if (sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
						{
							nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
							nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						}
					}
				}

				if (VclHrdBpPresentFlag)
				{
					auto pSubLayer0Info = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
					size_t CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
					vcl_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (size_t i = 0; i <= CpbCnt; i++)
					{
						nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						if (sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
						{
							nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
							nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						}
					}
				}
			}
		}

		MAP_BST_END();
	}
	catch (AMException e)
	{
		return e.RetCode();
	}

	return RET_CODE_SUCCESS;
}

size_t BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::BUFFERING_PERIOD::ProduceDesc(_Out_writes_(cbLen) char* szOutXml, size_t cbLen, bool bPrint, long long* bit_offset)
{
	size_t cbRequired = 0;
	int i = 0, field_prop_idx = 0;
	char szTemp[TEMP_SIZE], szTemp2[TEMP2_SIZE], szTemp3[TEMP3_SIZE], szTemp4[TEMP4_SIZE], szTagName[TAGNAME_SIZE];
	char* pTemp = NULL;
	DBG_UNREFERENCED_LOCAL_VARIABLE(szTemp);
	DBG_UNREFERENCED_LOCAL_VARIABLE(i);
	DBG_UNREFERENCED_LOCAL_VARIABLE(field_prop_idx);
	memset(szTemp2, 0, sizeof(szTemp2));
	memset(szTemp3, 0, sizeof(szTemp3));
	memset(szTemp4, 0, sizeof(szTemp4));
	memset(szTagName, 0, sizeof(szTagName));
	if (szOutXml != 0 && cbLen > 0)
	{
		memset(szOutXml, 0, cbLen);
	}

	BST_FIELD_PROP_UE(bp_seq_parameter_set_id, "indicates and shall be equal to the sps_seq_parameter_set_id for the SPS that is active for the coded picture associated with the buffering period SEI message");

	if (reserved_sei_message_payload_bytes.size() <= 0)
	{
		H264_NU cur_h264_sps;
		H265_NU cur_h265_sps;
		NAL_CODING coding = ptr_NAL_Context->GetNALCoding();

		if (NAL_CODING_AVC == coding)
		{
			bool NalHrdBpPresentFlag = false, VclHrdBpPresentFlag = false;

			INALAVCContext* pAVCCtx = NULL;
			if (SUCCEEDED(ptr_NAL_Context->QueryInterface(IID_INALAVCContext, (void**)&pAVCCtx)))
			{
				cur_h264_sps = pAVCCtx->GetAVCSPS(bp_seq_parameter_set_id);
				pAVCCtx->Release();
			}

			if (cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters)
			{
				NalHrdBpPresentFlag = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters_present_flag ? true : false;
				VclHrdBpPresentFlag = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters_present_flag ? true : false;
			}

			if (NalHrdBpPresentFlag)
			{
				uint8_t initial_cpb_removal_delay_length_minus1 =
					cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters->initial_cpb_removal_delay_length_minus1;
				int CpbCnt = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters->cpb_cnt_minus1;
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(SchedSelIdx=0;SchedSelIdx&lt;=cpb_cnt_minus1;SchedSelIdx++)", "");
				for (i = 0; i <= CpbCnt; i++)
				{
					NAV_WRITE_TAG_ARRAY_BEGIN0("nal_initial_cpb_removal", i, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_delay", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_offset", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					NAV_WRITE_TAG_END("nal_initial_cpb_removal");
				}
				NAV_WRITE_TAG_END("Tag0");
			}

			if (VclHrdBpPresentFlag)
			{
				uint8_t initial_cpb_removal_delay_length_minus1 =
					cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters->initial_cpb_removal_delay_length_minus1;
				int CpbCnt = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters->cpb_cnt_minus1;
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(SchedSelIdx=0;SchedSelIdx&lt;=cpb_cnt_minus1;SchedSelIdx++)", "");
				for (i = 0; i <= CpbCnt; i++)
				{
					NAV_WRITE_TAG_ARRAY_BEGIN0("vcl_initial_cpb_removal", i, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_delay", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_offset", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					NAV_WRITE_TAG_END("vcl_initial_cpb_removal");
				}
				NAV_WRITE_TAG_END("Tag1");
			}
		}
		else if (NAL_CODING_HEVC == coding)
		{
			bool sub_pic_hrd_params_present_flag = false;
			bool NalHrdBpPresentFlag = false;
			bool VclHrdBpPresentFlag = false;
			uint8_t au_cpb_removal_delay_length_minus1 = 0;
			uint8_t dpb_output_delay_length_minus1 = 0;
			uint8_t initial_cpb_removal_delay_length_minus1 = 0;

			INALHEVCContext* pHEVCCtx = NULL;
			if (SUCCEEDED(ptr_NAL_Context->QueryInterface(IID_INALHEVCContext, (void**)&pHEVCCtx)))
			{
				cur_h265_sps = pHEVCCtx->GetHEVCSPS(bp_seq_parameter_set_id);
				pHEVCCtx->Release();
			}

			if (cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters &&
				cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters)
			{
				sub_pic_hrd_params_present_flag = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_pic_hrd_params_present_flag;
				au_cpb_removal_delay_length_minus1 = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->au_cpb_removal_delay_length_minus1;
				dpb_output_delay_length_minus1 = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->dpb_output_delay_length_minus1;
				initial_cpb_removal_delay_length_minus1 = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->initial_cpb_removal_delay_length_minus1;
				NalHrdBpPresentFlag = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->nal_hrd_parameters_present_flag ? true : false;
				VclHrdBpPresentFlag = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->vcl_hrd_parameters_present_flag ? true : false;
			}

			BST_FIELD_PROP_2NUMBER1(bp_seq_parameter_set_id, 4, "");
			if (!sub_pic_hrd_params_present_flag){
				BST_FIELD_PROP_BOOL(irap_cpb_params_present_flag, "", "");
			}

			if (irap_cpb_params_present_flag) {
				BST_FIELD_PROP_2NUMBER1(cpb_delay_offset, (long long)au_cpb_removal_delay_length_minus1 + 1, "");
				BST_FIELD_PROP_2NUMBER1(dpb_delay_offset, (long long)dpb_output_delay_length_minus1 + 1, "");
			}

			BST_FIELD_PROP_BOOL(concatenation_flag, "", "");
			BST_FIELD_PROP_2NUMBER1(au_cpb_removal_delay_delta_minus1, (long long)au_cpb_removal_delay_length_minus1 + 1, "");

			if (NalHrdBpPresentFlag)
			{
				auto pSubLayer0Info = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
				int CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i = 0; i &lt;= CpbCnt; i++)", "");
				for (i = 0; i <= CpbCnt; i++)
				{
					NAV_WRITE_TAG_BEGIN3_1("nal_initial_cpb_removal", i, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_delay", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_offset", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					if (sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
					{
						BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_alt_cpb_removal_delay", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay, "");
						BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_alt_cpb_removal_offset", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset, "");
					}
					NAV_WRITE_TAG_END3("nal_initial_cpb_removal", i);
				}
				NAV_WRITE_TAG_END("Tag0");
			}

			if (VclHrdBpPresentFlag)
			{
				auto pSubLayer0Info = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
				int CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag1", "for(i = 0; i &lt;= CpbCnt; i++)", "");
				for (i = 0; i <= CpbCnt; i++)
				{
					NAV_WRITE_TAG_BEGIN3_1("vcl_initial_cpb_removal", i, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_delay", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_offset", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					if (sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
					{
						BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_alt_cpb_removal_delay", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay, "");
						BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_alt_cpb_removal_offset", i, (long long)initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset, "");
					}
					NAV_WRITE_TAG_END3("vcl_initial_cpb_removal", i);
				}
				NAV_WRITE_TAG_END("Tag1");
			}
		}
	}

	if (reserved_sei_message_payload_bytes.size() > 0)
	{
		NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved_sei_message_payload_byte",
			(uint32_t)(8ULL * reserved_sei_message_payload_bytes.size()),
			reserved_sei_message_payload_bytes.data(),
			(uint32_t)reserved_sei_message_payload_bytes.size(), "");
	}

	return cbRequired;
}

BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264::PIC_TIMING_H264(int payloadSize, INALContext* pNALCtx)
	: cpb_removal_delay(0)
	, dpb_output_delay(0)
	, CpbDpbDelaysPresentFlag(0)
	, CpbDpbDelaysPresentIfNAL(1)
	, pic_struct_present_flag(0)
	, pic_struct(0)
	, payload_size(payloadSize)
	, ptr_NAL_Context(pNALCtx){
	memset(ClockTS, 0, sizeof(ClockTS));
}

int BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H264::Map(AMBst in_bst)
{
	int iRet = RET_CODE_SUCCESS;
	SYNTAX_BITSTREAM_MAP::Map(in_bst);

	bool bMappedSucceeded = false;
	H264_NU cur_h264_sps;
	NAL_CODING coding = ptr_NAL_Context->GetNALCoding();
	if (coding != NAL_CODING_AVC)
		return RET_CODE_ERROR_NOTIMPL;

	// If there is no active SPS, skip all bytes in this payload
	INALAVCContext* pAVCCtx = NULL;
	if (SUCCEEDED(ptr_NAL_Context->QueryInterface(IID_INALAVCContext, (void**)&pAVCCtx)))
	{
		int8_t sps_id = pAVCCtx->GetActiveSPSID();
		if (sps_id >= 0 && sps_id <= 31)
			cur_h264_sps = pAVCCtx->GetAVCSPS((uint8_t)sps_id);
		
		pAVCCtx->Release();
		pAVCCtx = NULL;
	}
	else
		return RET_CODE_ERROR_NOTIMPL;

	if (!cur_h264_sps)
	{
		// No SPS is found, try to skip the payload bytes
		printf("No associated SPS is found for pic_timing SEI payload!.\n");
		uint8_t payload_byte;
		MAP_BST_BEGIN(0);
		if (payload_size > 0)
		{
			reserved_sei_message_payload_bytes.reserve(payload_size);
			for (int i = 0; i < payload_size; i++) {
				nal_read_b(in_bst, payload_byte, 8, uint8_t);
				reserved_sei_message_payload_bytes[i] = payload_byte;
			}
			MAP_BST_END();
		}
		return RET_CODE_SUCCESS;
	}

	try
	{
		MAP_BST_BEGIN(0);
		
		size_t parsed_payload_bits = 0;
		auto vui_parameters = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters;
		if (vui_parameters && (vui_parameters->nal_hrd_parameters_present_flag || vui_parameters->vcl_hrd_parameters_present_flag))
		{
			CpbDpbDelaysPresentFlag = 1;
			if (vui_parameters->nal_hrd_parameters_present_flag && vui_parameters->nal_hrd_parameters)
			{
				CpbDpbDelaysPresentIfNAL = 1;
				cpb_removal_delay_length_minus1 = vui_parameters->nal_hrd_parameters->cpb_removal_delay_length_minus1;
				dpb_output_delay_length_minus1 = vui_parameters->nal_hrd_parameters->dpb_output_delay_length_minus1;
			}
			else if (vui_parameters->vcl_hrd_parameters_present_flag && vui_parameters->vcl_hrd_parameters)
			{
				CpbDpbDelaysPresentIfNAL = 0;
				cpb_removal_delay_length_minus1 = vui_parameters->vcl_hrd_parameters->cpb_removal_delay_length_minus1;
				dpb_output_delay_length_minus1 = vui_parameters->vcl_hrd_parameters->dpb_output_delay_length_minus1;
			}

			cpb_removal_delay = (uint32_t)AMBst_GetBits(in_bst, cpb_removal_delay_length_minus1 + 1);
			dpb_output_delay = (uint32_t)AMBst_GetBits(in_bst, dpb_output_delay_length_minus1 + 1);

			parsed_payload_bits += (size_t)cpb_removal_delay_length_minus1 + 1;
			parsed_payload_bits += (size_t)dpb_output_delay_length_minus1 + 1;
		}

		if (vui_parameters && vui_parameters->pic_struct_present_flag)
		{
			pic_struct_present_flag = 1;
			pic_struct = (uint8_t)AMBst_GetBits(in_bst, 4);
			parsed_payload_bits += 4;

			int NumClockTS = 0;
			if (pic_struct >= 0 && pic_struct <= 2)
				NumClockTS = 1;
			else if (pic_struct == 3 || pic_struct == 4 || pic_struct == 7)
				NumClockTS = 2;
			else if (pic_struct == 5 || pic_struct == 6 || pic_struct == 8)
				NumClockTS = 3;

			if (NumClockTS > 0)
			{
				for (int i = 0; i < NumClockTS; i++)
				{
					ClockTS[i].clock_timestamp_flag = (uint16_t)AMBst_GetBits(in_bst, 1);
					parsed_payload_bits++;
					if (ClockTS[i].clock_timestamp_flag)
					{
						ClockTS[i].ct_type = (uint16_t)AMBst_GetBits(in_bst, 2);
						ClockTS[i].nuit_field_based_flag = (uint16_t)AMBst_GetBits(in_bst, 1);
						ClockTS[i].counting_type = (uint16_t)AMBst_GetBits(in_bst, 5);
						ClockTS[i].full_timestamp_flag = (uint16_t)AMBst_GetBits(in_bst, 1);
						ClockTS[i].discontinuity_flag = (uint16_t)AMBst_GetBits(in_bst, 1);
						ClockTS[i].cnt_dropped_flag = (uint16_t)AMBst_GetBits(in_bst, 1);
						ClockTS[i].n_frames = (uint16_t)AMBst_GetBits(in_bst, 8);
						parsed_payload_bits += 19;

						if (ClockTS[i].full_timestamp_flag)
						{
							ClockTS[i].seconds_value = (uint32_t)AMBst_GetBits(in_bst, 6);
							ClockTS[i].minutes_value = (uint32_t)AMBst_GetBits(in_bst, 6);
							ClockTS[i].hours_value = (uint32_t)AMBst_GetBits(in_bst, 5);
							parsed_payload_bits += 17;
						}
						else
						{
							ClockTS[i].seconds_flag = (uint32_t)AMBst_GetBits(in_bst, 1);
							parsed_payload_bits++;
							if (ClockTS[i].seconds_flag)
							{
								ClockTS[i].seconds_value = (uint32_t)AMBst_GetBits(in_bst, 6);
								ClockTS[i].minutes_flag = (uint32_t)AMBst_GetBits(in_bst, 1);
								parsed_payload_bits += 7;
								if (ClockTS[i].minutes_flag)
								{
									ClockTS[i].minutes_value = (uint32_t)AMBst_GetBits(in_bst, 6);
									ClockTS[i].hours_flag = (uint32_t)AMBst_GetBits(in_bst, 1);
									parsed_payload_bits += 7;
									if (ClockTS[i].hours_flag)
									{
										ClockTS[i].hours_value = (uint32_t)AMBst_GetBits(in_bst, 5);
										parsed_payload_bits += 5;
									}
								}
							}
						}

						if (vui_parameters->nal_hrd_parameters_present_flag && 
							vui_parameters->nal_hrd_parameters && 
							vui_parameters->nal_hrd_parameters->time_offset_length > 0)
							ClockTS[i].time_offset_length = vui_parameters->nal_hrd_parameters->time_offset_length;

						else if (vui_parameters->vcl_hrd_parameters_present_flag && 
							vui_parameters->vcl_hrd_parameters &&
							vui_parameters->vcl_hrd_parameters->time_offset_length > 0)
							ClockTS[i].time_offset_length = vui_parameters->vcl_hrd_parameters->time_offset_length;

						if (ClockTS[i].time_offset_length > 0)
						{
							parsed_payload_bits += (size_t)ClockTS[i].time_offset_length;
							ClockTS[i].time_offset = (int32_t)AMBst_GetTCLongLong(in_bst, ClockTS[i].time_offset_length);
						}
					}
				}
			}
		}

		// Don't parse the left bits, but skip them
		if (parsed_payload_bits < ((size_t)payload_size<<3))
		{
			size_t unparsed_bits = ((size_t)payload_size << 3) - parsed_payload_bits;
			AMBst_SkipBits(in_bst, (int)unparsed_bits);
		}

		MAP_BST_END();
	}
	catch (AMException e)
	{
		return e.RetCode();
	}

	return RET_CODE_SUCCESS;
}

BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H265::PIC_TIMING_H265(int payloadSize, INALContext* pNALCtx)
	: u32_Value_0(0)
	, au_cpb_removal_delay_minus1(0)
	, pic_dpb_output_delay(0)
	, pic_dpb_output_du_delay(0)
	, num_decoding_units_minus1(0)
	, du_common_cpb_removal_delay_increment_minus1(0)
	, payload_size(payloadSize)
	, ptr_NAL_Context(pNALCtx)
{

}

int BST::SEI_RBSP::SEI_MESSAGE::SEI_PAYLOAD::PIC_TIMING_H265::Map(AMBst in_bst)
{
	int iRet = RET_CODE_SUCCESS;
	SYNTAX_BITSTREAM_MAP::Map(in_bst);

	// If there is no active SPS, skip all bytes in this payload
	INALHEVCContext* pHEVCCtx = NULL;
	if (SUCCEEDED(ptr_NAL_Context->QueryInterface(IID_INALHEVCContext, (void**)&pHEVCCtx)))
	{
		int8_t sps_id = pHEVCCtx->GetActiveSPSID();
		if (sps_id >= 0)
		{
			auto active_sps = pHEVCCtx->GetHEVCSPS((uint8_t)sps_id);
			if (active_sps &&
				active_sps->ptr_seq_parameter_set_rbsp &&
				active_sps->ptr_seq_parameter_set_rbsp->vui_parameters_present_flag &&
				active_sps->ptr_seq_parameter_set_rbsp->vui_parameters)
			{
				auto vui_parameters = active_sps->ptr_seq_parameter_set_rbsp->vui_parameters;
				frame_field_info_present_flag = vui_parameters->frame_field_info_present_flag;

				if (vui_parameters->vui_hrd_parameters_present_flag && vui_parameters->hrd_parameters)
				{
					if (vui_parameters->hrd_parameters->vcl_hrd_parameters_present_flag ||
						vui_parameters->hrd_parameters->nal_hrd_parameters_present_flag)
					{
						CpbDpbDelaysPresentFlag = true;

						if (vui_parameters->hrd_parameters->sub_pic_hrd_params_present_flag)
						{
							sub_pic_hrd_params_present_flag = true;
							du_cpb_removal_delay_increment_length_minus1 =
								vui_parameters->hrd_parameters->du_cpb_removal_delay_increment_length_minus1;
							sub_pic_cpb_params_in_pic_timing_sei_flag =
								vui_parameters->hrd_parameters->sub_pic_cpb_params_in_pic_timing_sei_flag;
							dpb_output_delay_du_length_minus1 =
								vui_parameters->hrd_parameters->dpb_output_delay_du_length_minus1;
						}

						initial_cpb_removal_delay_length_minus1 = vui_parameters->hrd_parameters->initial_cpb_removal_delay_length_minus1;
						au_cpb_removal_delay_length_minus1 = vui_parameters->hrd_parameters->au_cpb_removal_delay_length_minus1;
						dpb_output_delay_length_minus1 = vui_parameters->hrd_parameters->dpb_output_delay_length_minus1;
					}
				}
			}
		}

		pHEVCCtx->Release();
		pHEVCCtx = nullptr;
	}

	try
	{
		MAP_BST_BEGIN(0);

		size_t parsed_payload_bits = 0;

		if (frame_field_info_present_flag)
		{
			pic_struct = AMBst_GetBits(in_bst, 4);
			source_scan_type = AMBst_GetBits(in_bst, 2);
			duplicate_flag = AMBst_GetBits(in_bst, 1);
			parsed_payload_bits += 7;
		}

		if (CpbDpbDelaysPresentFlag)
		{
			au_cpb_removal_delay_minus1 = (uint64_t)AMBst_GetBits(in_bst, au_cpb_removal_delay_length_minus1 + 1);
			parsed_payload_bits += (size_t)au_cpb_removal_delay_length_minus1 + 1;
			pic_dpb_output_delay = (uint64_t)AMBst_GetBits(in_bst, dpb_output_delay_length_minus1 + 1);
			parsed_payload_bits += (size_t)dpb_output_delay_length_minus1 + 1;

			if (sub_pic_hrd_params_present_flag)
			{
				pic_dpb_output_du_delay = (uint64_t)AMBst_GetBits(in_bst, dpb_output_delay_du_length_minus1 + 1);
				parsed_payload_bits += (size_t)dpb_output_delay_du_length_minus1 + 1;

				if (sub_pic_cpb_params_in_pic_timing_sei_flag)
				{
					num_decoding_units_minus1 = AMBst_Get_ue(in_bst);
					parsed_payload_bits += (size_t)quick64_log2(num_decoding_units_minus1 + 1) * 2 + 1;

					du_common_cpb_removal_delay_flag = (uint8_t)AMBst_GetBits(in_bst, 1);
					parsed_payload_bits++;

					if (du_common_cpb_removal_delay_flag)
					{
						du_common_cpb_removal_delay_increment_minus1 = (uint64_t)AMBst_GetBits(in_bst, du_cpb_removal_delay_increment_length_minus1 + 1);
						parsed_payload_bits += (size_t)du_cpb_removal_delay_increment_length_minus1 + 1;
					}

					for (uint64_t i = 0; i <= num_decoding_units_minus1; i++)
					{
						DECODE_UNIT du;
						du.num_nalus_in_du_minus1 = AMBst_Get_ue(in_bst);
						parsed_payload_bits += (size_t)quick64_log2(du.num_nalus_in_du_minus1 + 1) * 2 + 1;

						if (!du_common_cpb_removal_delay_flag && i < num_decoding_units_minus1)
						{
							du.du_cpb_removal_delay_increment_minus1 = AMBst_GetBits(in_bst, du_cpb_removal_delay_increment_length_minus1 + 1);
							parsed_payload_bits += (size_t)du_cpb_removal_delay_increment_length_minus1 + 1;
						}
						else
							du.du_cpb_removal_delay_increment_minus1 = 0;

						dus.push_back(du);
					}
				}
			}
		}

		// Don't parse the left bits, but skip them
		if (parsed_payload_bits < ((size_t)payload_size << 3))
		{
			size_t unparsed_bits = ((size_t)payload_size << 3) - parsed_payload_bits;
			AMBst_SkipBits(in_bst, (int)unparsed_bits);
		}

		MAP_BST_END();
	}
	catch (AMException e)
	{
		return e.RetCode();
	}

	return iRet;
}
