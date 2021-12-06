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
//#include "h265_video.h"
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
	/* 54*/"reserved_sei_message",
	/* 55*/"reserved_sei_message",
	/* 56*/"reserved_sei_message",
	/* 57*/"reserved_sei_message",
	/* 58*/"reserved_sei_message",
	/* 59*/"reserved_sei_message",
	/* 60*/"reserved_sei_message",
	/* 61*/"reserved_sei_message",
	/* 62*/"reserved_sei_message",
	/* 63*/"reserved_sei_message",
	/* 64*/"reserved_sei_message",
	/* 65*/"reserved_sei_message",
	/* 66*/"reserved_sei_message",
	/* 67*/"reserved_sei_message",
	/* 68*/"reserved_sei_message",
	/* 69*/"reserved_sei_message",
	/* 70*/"reserved_sei_message",
	/* 71*/"reserved_sei_message",
	/* 72*/"reserved_sei_message",
	/* 73*/"reserved_sei_message",
	/* 74*/"reserved_sei_message",
	/* 75*/"reserved_sei_message",
	/* 76*/"reserved_sei_message",
	/* 77*/"reserved_sei_message",
	/* 78*/"reserved_sei_message",
	/* 79*/"reserved_sei_message",
	/* 80*/"reserved_sei_message",
	/* 81*/"reserved_sei_message",
	/* 82*/"reserved_sei_message",
	/* 83*/"reserved_sei_message",
	/* 84*/"reserved_sei_message",
	/* 85*/"reserved_sei_message",
	/* 86*/"reserved_sei_message",
	/* 87*/"reserved_sei_message",
	/* 88*/"reserved_sei_message",
	/* 89*/"reserved_sei_message",
	/* 90*/"reserved_sei_message",
	/* 91*/"reserved_sei_message",
	/* 92*/"reserved_sei_message",
	/* 93*/"reserved_sei_message",
	/* 94*/"reserved_sei_message",
	/* 95*/"reserved_sei_message",
	/* 96*/"reserved_sei_message",
	/* 97*/"reserved_sei_message",
	/* 98*/"reserved_sei_message",
	/* 99*/"reserved_sei_message",
	/*100*/"reserved_sei_message",
	/*101*/"reserved_sei_message",
	/*102*/"reserved_sei_message",
	/*103*/"reserved_sei_message",
	/*104*/"reserved_sei_message",
	/*105*/"reserved_sei_message",
	/*106*/"reserved_sei_message",
	/*107*/"reserved_sei_message",
	/*108*/"reserved_sei_message",
	/*109*/"reserved_sei_message",
	/*110*/"reserved_sei_message",
	/*111*/"reserved_sei_message",
	/*112*/"reserved_sei_message",
	/*113*/"reserved_sei_message",
	/*114*/"reserved_sei_message",
	/*115*/"reserved_sei_message",
	/*116*/"reserved_sei_message",
	/*117*/"reserved_sei_message",
	/*118*/"reserved_sei_message",
	/*119*/"reserved_sei_message",
	/*120*/"reserved_sei_message",
	/*121*/"reserved_sei_message",
	/*122*/"reserved_sei_message",
	/*123*/"reserved_sei_message",
	/*124*/"reserved_sei_message",
	/*125*/"reserved_sei_message",
	/*126*/"reserved_sei_message",
	/*127*/"reserved_sei_message",
	/*128*/"structure_of_pictures_info",
	/*129*/"active_parameter_sets",
	/*130*/"decoding_unit_info",
	/*131*/"temporal_sub_layer_zero_index",
	/*132*/"reserved_sei_message",
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
	/*144*/"reserved_sei_message",
	/*145*/"reserved_sei_message",
	/*146*/"reserved_sei_message",
	/*147*/"reserved_sei_message",
	/*148*/"reserved_sei_message",
	/*149*/"reserved_sei_message",
	/*150*/"reserved_sei_message",
	/*151*/"reserved_sei_message",
	/*152*/"reserved_sei_message",
	/*153*/"reserved_sei_message",
	/*154*/"reserved_sei_message",
	/*155*/"reserved_sei_message",
	/*156*/"reserved_sei_message",
	/*157*/"reserved_sei_message",
	/*158*/"reserved_sei_message",
	/*159*/"reserved_sei_message",
	/*160*/"layers_not_present",
	/*161*/"inter_layer_constrained_tile_sets",
	/*162*/"bsp_nesting",
	/*163*/"bsp_initial_arrival_time",
	/*164*/"sub_bitstream_property",
	/*165*/"alpha_channel_info",
	/*166*/"overlay_info",
	/*167*/"temporal_mv_prediction_constraints",
	/*168*/"frame_field_info",
	/*169*/"reserved_sei_message",
	/*170*/"reserved_sei_message",
	/*171*/"reserved_sei_message",
	/*172*/"reserved_sei_message",
	/*173*/"reserved_sei_message",
	/*174*/"reserved_sei_message",
	/*175*/"reserved_sei_message",
	/*176*/"three_dimensional_reference_displays_info",
	/*177*/"depth_representation_info",
	/*178*/"multiview_scene_info",
	/*179*/"multiview_acquisition_info",
	/*180*/"multiview_view_position",
	/*181*/"alternative_depth_info",
	/*182*/"reserved_sei_message",
	/*183*/"reserved_sei_message",
	/*184*/"reserved_sei_message",
	/*185*/"reserved_sei_message",
	/*186*/"reserved_sei_message",
	/*187*/"reserved_sei_message",
	/*188*/"reserved_sei_message",
	/*189*/"reserved_sei_message",
	/*190*/"reserved_sei_message",
	/*191*/"reserved_sei_message",
	/*192*/"reserved_sei_message",
	/*193*/"reserved_sei_message",
	/*194*/"reserved_sei_message",
	/*195*/"reserved_sei_message",
	/*196*/"reserved_sei_message",
	/*197*/"reserved_sei_message",
	/*198*/"reserved_sei_message",
	/*199*/"reserved_sei_message",
	/*200*/"reserved_sei_message",
	/*201*/"reserved_sei_message",
	/*202*/"reserved_sei_message",
	/*203*/"reserved_sei_message",
	/*204*/"reserved_sei_message",
	/*205*/"reserved_sei_message",
	/*206*/"reserved_sei_message",
	/*207*/"reserved_sei_message",
	/*208*/"reserved_sei_message",
	/*209*/"reserved_sei_message",
	/*210*/"reserved_sei_message",
	/*211*/"reserved_sei_message",
	/*212*/"reserved_sei_message",
	/*213*/"reserved_sei_message",
	/*214*/"reserved_sei_message",
	/*215*/"reserved_sei_message",
	/*216*/"reserved_sei_message",
	/*217*/"reserved_sei_message",
	/*218*/"reserved_sei_message",
	/*219*/"reserved_sei_message",
	/*220*/"reserved_sei_message",
	/*221*/"reserved_sei_message",
	/*222*/"reserved_sei_message",
	/*223*/"reserved_sei_message",
	/*224*/"reserved_sei_message",
	/*225*/"reserved_sei_message",
	/*226*/"reserved_sei_message",
	/*227*/"reserved_sei_message",
	/*228*/"reserved_sei_message",
	/*229*/"reserved_sei_message",
	/*230*/"reserved_sei_message",
	/*231*/"reserved_sei_message",
	/*232*/"reserved_sei_message",
	/*233*/"reserved_sei_message",
	/*234*/"reserved_sei_message",
	/*235*/"reserved_sei_message",
	/*236*/"reserved_sei_message",
	/*237*/"reserved_sei_message",
	/*238*/"reserved_sei_message",
	/*239*/"reserved_sei_message",
	/*240*/"reserved_sei_message",
	/*241*/"reserved_sei_message",
	/*242*/"reserved_sei_message",
	/*243*/"reserved_sei_message",
	/*244*/"reserved_sei_message",
	/*245*/"reserved_sei_message",
	/*246*/"reserved_sei_message",
	/*247*/"reserved_sei_message",
	/*248*/"reserved_sei_message",
	/*249*/"reserved_sei_message",
	/*250*/"reserved_sei_message",
	/*251*/"reserved_sei_message",
	/*252*/"reserved_sei_message",
	/*253*/"reserved_sei_message",
	/*254*/"reserved_sei_message",
	/*255*/"reserved_sei_message"
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
#if 0
		H265Video::NAL_UNIT* cur_h265_sps = NULL;
#endif
		H264_NU cur_h264_sps;
		NAL_CODING coding = NAL_CODING_UNKNOWN;
		if (ptr_sei_payload &&
			ptr_sei_payload->ptr_sei_message &&
			ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp &&
			ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL)
			coding = ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL->GetNALCoding();

		do
		{
			if (coding == NAL_CODING_AVC)
			{
				INALAVCContext* pAVCCtx = NULL;
				if (SUCCEEDED(ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL->QueryInterface(IID_INALAVCContext, (void**)&pAVCCtx)))
				{
					cur_h264_sps = pAVCCtx->GetAVCSPS(bp_seq_parameter_set_id);
					pAVCCtx->Release();
				}
			}
#if 0
			else if (coding == NAL_CODING_HEVC)
			{
				auto h265_spses = static_cast<CAMArrayRefHash<H265Video::BYTE_STREAM_NAL_UNIT::NAL_UNIT>*>(AMTLS_GetEnvPointer(_T("BST_CONTAINER_H265_SPS"), NULL));

				if (h265_spses == NULL)
					break;

				int sel_id = 0;
				std::vector<H265Video::BYTE_STREAM_NAL_UNIT::NAL_UNIT*> sel_spses(h265_spses->Length());
				// how many SPSes are int current set with the specified seq_parameter_set_id
				for (auto iter = h265_spses->begin(); iter != h265_spses->end(); iter++)
				{
					if (iter->value->ptr_seq_parameter_set_rbsp->sps_seq_parameter_set_id == bp_seq_parameter_set_id)
						sel_spses[sel_id] = iter->value;
				}

				if (sel_spses.size() > 1)
				{
					size_t vps_id = (size_t)AMTLS_GetEnvInteger(_T("BST_CONTAINER_ACTIVE_H265_VPS_ID"), -1LL);
					for (auto v : sel_spses)
					{
						if (v->ptr_seq_parameter_set_rbsp->sps_video_parameter_set_id == vps_id)
						{
							cur_h265_sps = v;
							break;
						}
					}

					if (cur_h265_sps == NULL)
						cur_h265_sps = (H265Video::BYTE_STREAM_NAL_UNIT::NAL_UNIT*)AMTLS_GetEnvPointer(_T("BST_CONTAINER_ACTIVE_H265_SPS"), NULL);
				}
				else if (sel_spses.size() == 1)
				{
					cur_h265_sps = sel_spses[0];
				}
				else
					break;
			}
#endif
		} while (0);

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
					int CpbCnt = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->nal_hrd_parameters->cpb_cnt_minus1;
					nal_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (int i = 0; i <= CpbCnt; i++)
					{
						nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						nal_read_u(in_bst, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
					}
				}

				if (VclHrdBpPresentFlag)
				{
					uint8_t initial_cpb_removal_delay_length_minus1 =
						cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters->initial_cpb_removal_delay_length_minus1;
					int CpbCnt = cur_h264_sps->ptr_seq_parameter_set_rbsp->seq_parameter_set_data.vui_parameters->vcl_hrd_parameters->cpb_cnt_minus1;
					vcl_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (int i = 0; i <= CpbCnt; i++)
					{
						nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
						nal_read_u(in_bst, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, initial_cpb_removal_delay_length_minus1 + 1, uint32_t);
					}
				}
			}
		}
#if 0
		else if (coding == NAL_CODING_HEVC)
		{
			ctx_data = (void*)cur_h265_sps;
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
					int CpbCnt = pSubLayer0Info != NULL?pSubLayer0Info->cpb_cnt_minus1:0;
					nal_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (int i = 0; i <= CpbCnt; i++)
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
					int CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
					vcl_initial_cpb_removal_info.resize(CpbCnt + 1);
					for (int i = 0; i <= CpbCnt; i++)
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
#endif

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
		NAL_CODING coding = NAL_CODING_UNKNOWN;
		if (ptr_sei_payload &&
			ptr_sei_payload->ptr_sei_message &&
			ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp &&
			ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL)
			coding = ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL->GetNALCoding();

		if (NAL_CODING_AVC == coding)
		{
			bool NalHrdBpPresentFlag = false, VclHrdBpPresentFlag = false;

			INALAVCContext* pAVCCtx = NULL;
			if (SUCCEEDED(ptr_sei_payload->ptr_sei_message->ptr_sei_rbsp->ctx_NAL->QueryInterface(IID_INALAVCContext, (void**)&pAVCCtx)))
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
					NAV_WRITE_TAG_ARRAY_BEGIN("nal_initial_cpb_removal", i, " ");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_delay", i, initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_offset", i, initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
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
					NAV_WRITE_TAG_ARRAY_BEGIN("vcl_initial_cpb_removal", i, " ");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_delay", i, initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_offset", i, initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					NAV_WRITE_TAG_END("vcl_initial_cpb_removal");
				}
				NAV_WRITE_TAG_END("Tag1");
			}
		}
#if 0
		else if (bst_stream_type == HEVC_VIDEO_STREAM)
		{
			bool sub_pic_hrd_params_present_flag = false;
			bool NalHrdBpPresentFlag = false;
			bool VclHrdBpPresentFlag = false;
			uint8_t au_cpb_removal_delay_length_minus1 = 0;
			uint8_t dpb_output_delay_length_minus1 = 0;
			uint8_t initial_cpb_removal_delay_length_minus1 = 0;

			H265Video::BYTE_STREAM_NAL_UNIT::NAL_UNIT* cur_h265_sps = (H265Video::BYTE_STREAM_NAL_UNIT::NAL_UNIT*)ctx_data;

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
				BST_FIELD_PROP_2NUMBER1(cpb_delay_offset, au_cpb_removal_delay_length_minus1 + 1, "");
				BST_FIELD_PROP_2NUMBER1(dpb_delay_offset, dpb_output_delay_length_minus1 + 1, "");
			}

			BST_FIELD_PROP_BOOL(concatenation_flag, "", "");
			BST_FIELD_PROP_2NUMBER1(au_cpb_removal_delay_delta_minus1, au_cpb_removal_delay_length_minus1 + 1, "");

			if (NalHrdBpPresentFlag)
			{
				auto pSubLayer0Info = cur_h265_sps->ptr_seq_parameter_set_rbsp->vui_parameters->hrd_parameters->sub_layer_infos[0];
				int CpbCnt = pSubLayer0Info != NULL ? pSubLayer0Info->cpb_cnt_minus1 : 0;
				NAV_WRITE_TAG_BEGIN_WITH_ALIAS("Tag0", "for(i = 0; i &lt;= CpbCnt; i++)", "");
				for (i = 0; i <= CpbCnt; i++)
				{
					NAV_WRITE_TAG_BEGIN3_1("nal_initial_cpb_removal", i, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_delay", i, initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_cpb_removal_offset", i, initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					if (sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
					{
						BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_alt_cpb_removal_delay", i, initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay, "");
						BST_ARRAY_FIELD_PROP_NUMBER("nal_initial_alt_cpb_removal_offset", i, initial_cpb_removal_delay_length_minus1 + 1, nal_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset, "");
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
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_delay", i, initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_delay, "");
					BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_cpb_removal_offset", i, initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_cpb_removal_offset, "");
					if (sub_pic_hrd_params_present_flag || irap_cpb_params_present_flag)
					{
						BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_alt_cpb_removal_delay", i, initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_delay, "");
						BST_ARRAY_FIELD_PROP_NUMBER("vcl_initial_alt_cpb_removal_offset", i, initial_cpb_removal_delay_length_minus1 + 1, vcl_initial_cpb_removal_info[i].initial_alt_cpb_removal_offset, "");
					}
					NAV_WRITE_TAG_END3("vcl_initial_cpb_removal", i);
				}
				NAV_WRITE_TAG_END("Tag1");
			}
		}
#endif
	}

	if (reserved_sei_message_payload_bytes.size() > 0)
	{
		NAV_FIELD_PROP_FIXSIZE_BINSTR("reserved_sei_message_payload_byte",
			(uint32_t)(8 * reserved_sei_message_payload_bytes.size()),
			reserved_sei_message_payload_bytes.data(),
			(uint32_t)reserved_sei_message_payload_bytes.size(), "");
	}

	return cbRequired;
}