# NAL bitstream operation guide(For H.264/.265/.266)
## Contents
* [Show the information of NAL bitstream](#Show-the-information-of-nal-bitstream)
* [List Access-Unit, NAL-unit and SEI messages and payloads](#list-access-unit-nal-unit-and-sei-messages-and-payloads)
* [Show syntax fields](#show-syntax-fields)
  * [Show VPS syntax fields](#show-vps-syntax-fields)
  * [Show SPS syntax fields](#show-sps-syntax-fields)
  * [Show PPS syntax fields](#show-pps-syntax-fields)
* [Show HRD model information](#show-hrd-model-information)

[Return to Main](../README.md)

H.264(AVC), H.265(HEVC and H.266(VVC) uses NAL bitstream

## Show the information of NAL bitstream
```
DumpTS 00002.hevc --showinfo
```
Here is the output:
```
A new H.265 sequence(seq_parameter_set_id:0):
        HEVC Profile: Main 10
        Tiger: Main
        Level 5.1
        Chroma: 4:2:0
        Scan type: Progressive
        Coded Frame resolution: 3840x2160
        Display resolution: 3840x2160
        Sample Aspect-Ratio: 1:1(Square), example: 3840x2160 16:9 frame without horizontal overscan
        Colour Primaries: 9(BT.2020)
        Transfer Characteristics: 14(BT.2020)
        Matrix Coeffs: 9(KR = 0.2627; KB = 0.0593 -- BT.2020 constant luminance system)
        Frame-Rate: 59.940060 fps
```
For H.264 stream,
```
DumpTS 00005.h264 --showinfo
A new H.264 sequence(seq_parameter_set_id:0):
        AVC Profile: High Profile
        AVC Level: 4
        Chroma: 4:2:0
        Scan type: Progressive
        Coded Frame resolution: 1920x1088
        Display resolution: 1920x1080
        Sample Aspect-Ratio: 1:1(Square), example: 3840x2160 16:9 frame without horizontal overscan
        Frame-Rate: 23.976025 fps
```

[Top](#contents)
## List Access-Unit, NAL-unit and SEI messages and payloads
```
DumpTS 00002.hevc --showNU
```
Here is the output:
```
Access-Unit#0
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 12
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::TSA_N -- VCL::TSA_N Coded slice segment of a TSA picture slice_segment_layer_rbsp(), len: 14894
Access-Unit#1
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::VPS_NUT -- non-VCL::VPS_NUT Video parameter set video_parameter_set_rbsp( ), len: 27
	NAL Unit non-VCL::SPS_NUT -- non-VCL::SPS_NUT Sequence parameter set seq_parameter_set_rbsp( ), len: 95
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 30
		SEI message
			SEI payload buffering_period, length: 10
		SEI message
			SEI payload pic_timing, length: 7
		SEI message
			SEI payload pan_scan_rect, length: 1
		SEI message
			SEI payload recovery_point, length: 6
	NAL Unit VCL::CRA_NUT -- VCL::CRA_NUT Coded slice segment of a CRA picture slice_segment_layer_rbsp( ), len: 601348
Access-Unit#2
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 14
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_R -- VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 37262
Access-Unit#3
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_R -- VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 25678
Access-Unit#4
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 16453
Access-Unit#5
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 16635
Access-Unit#6
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 12
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_R -- VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 24048
Access-Unit#7
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 16385
Access-Unit#8
	NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
	NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
	NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 12
		SEI message
			SEI payload pic_timing, length: 7
	NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 13847
...
```
If only want to show access-unit, or nal-unit or sei-message or paylods, can specify the 'au', 'nu' or 'seimsg' or 'seipayload' w/ or w/o ',;.:' separators to filter the related objects, ex,
```
DumpTS 00002.hevc --showNU=nu;seipayload
```
Here is the output:
```
...
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 292
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 14
                SEI payload pic_timing, length: 7
NAL Unit VCL::RASL_R -- VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 17153
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 292
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
                SEI payload pic_timing, length: 7
NAL Unit VCL::RASL_R -- VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 15052
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 292
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
                SEI payload pic_timing, length: 7
NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 8690
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
                SEI payload pic_timing, length: 7
NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 8934
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 12
                SEI payload pic_timing, length: 7
NAL Unit VCL::RASL_R -- VCL::RASL_R Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 16011
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 13
                SEI payload pic_timing, length: 7
NAL Unit VCL::RASL_N -- VCL::RASL_N Coded slice segment of a RASL picture slice_segment_layer_rbsp( ), len: 10778
NAL Unit non-VCL::AUD_NUT -- non-VCL::AUD_NUT Access unit delimiter access_unit_delimiter_rbsp( ), len: 3
NAL Unit non-VCL::PPS_NUT -- non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( ), len: 183
NAL Unit non-VCL::PREFIX_SEI_NUT -- non-VCL::PREFIX_SEI_NUT Supplemental enhancement information sei_rbsp( ), len: 12
                SEI payload pic_timing, length: 7
...
```

[Top](#contents)
## Show syntax fields
### Show VPS syntax fields
```
DumpTS 00002.hevc --showvps
```
Here is the output:
```
NAL_UNIT:                                         // non-VCL::VPS_NUT Video parameter set video_parameter_set_rbsp( )
forbidden_zero_bit: 0                             // shall be equal to 0.
nal_unit_type: 32(0X20)                           // non-VCL::VPS_NUT Video parameter set video_parameter_set_rbsp( )
nuh_layer_id: 0(0X0)                              // the identifier of the layer to which a VCL NAL unit belongs or the ...
nuh_temporal_id_plus1: 1(0X1)                     // Should be 1
vps_video_parameter_set_id: 0(0X0)                // identifies the VPS for reference by other syntax elements
vps_base_layer_internal_flag: 1                   
vps_base_layer_available_flag: 1                  
vps_max_layers_minus1: 0(0X0)                     // plus 1 specifies the maximum allowed number of layers in each CVS r...
vps_max_sub_layers_minus1: 3(0X3)                 // plus 1 specifies the maximum number of temporal sub-layers that may...
vps_temporal_id_nesting_flag: 0                   // when vps_max_sub_layers_minus1 is greater than 0, specifies whether...
vps_reserved_0xffff_16bits: 65535(0XFFFF)         // shall be equal to 0xFFFF
profile_tier_level(1, vps_max_sub_layers_minus1): // Main 10
    general_profile_space: 0(0X0)                 
    general_tier_flag: 0(0X0)                     
    general_profile_idc: 2(0X2)                   
    profile_compatibility_flag:                   
        general_profile_compatibility_flag[0]: 0  
        general_profile_compatibility_flag[1]: 0  
        general_profile_compatibility_flag[2]: 1  
        general_profile_compatibility_flag[3]: 0  
        general_profile_compatibility_flag[4]: 0  
        general_profile_compatibility_flag[5]: 0  
        general_profile_compatibility_flag[6]: 0  
        general_profile_compatibility_flag[7]: 0  
        general_profile_compatibility_flag[8]: 0  
        general_profile_compatibility_flag[9]: 0  
        general_profile_compatibility_flag[10]: 0 
        general_profile_compatibility_flag[11]: 0 
        general_profile_compatibility_flag[12]: 0 
        general_profile_compatibility_flag[13]: 0 
        general_profile_compatibility_flag[14]: 0 
        general_profile_compatibility_flag[15]: 0 
        general_profile_compatibility_flag[16]: 0 
        general_profile_compatibility_flag[17]: 0 
        general_profile_compatibility_flag[18]: 0 
        general_profile_compatibility_flag[19]: 0 
        general_profile_compatibility_flag[20]: 0 
        general_profile_compatibility_flag[21]: 0 
        general_profile_compatibility_flag[22]: 0 
        general_profile_compatibility_flag[23]: 0 
        general_profile_compatibility_flag[24]: 0 
        general_profile_compatibility_flag[25]: 0 
        general_profile_compatibility_flag[26]: 0 
        general_profile_compatibility_flag[27]: 0 
        general_profile_compatibility_flag[28]: 0 
        general_profile_compatibility_flag[29]: 0 
        general_profile_compatibility_flag[30]: 0 
        general_profile_compatibility_flag[31]: 0 
    general_progressive_source_flag: 1            
    general_interlaced_source_flag: 0             
    general_non_packed_constraint_flag: 1         
    general_frame_only_constraint_flag: 1         
    general_reserved_zero_43bits: 0(0X0)          
    general_inbld_flag: 0                         
    general_level_idc: 153(0X99)                  // Level 5.1
    sub_layer_profile_present_flag[0]: 0          
    sub_layer_level_present_flag[0]: 0            
    sub_layer_profile_present_flag[1]: 0          
    sub_layer_level_present_flag[1]: 0            
    sub_layer_profile_present_flag[2]: 0          
    sub_layer_level_present_flag[2]: 0            
    reserved_zero_2bits[3]: 0                     
    reserved_zero_2bits[4]: 0                     
    reserved_zero_2bits[5]: 0                     
    reserved_zero_2bits[6]: 0                     
    reserved_zero_2bits[7]: 0                     
vps_sub_layer_ordering_info_present_flag: 0       // specifies that the values of vps_max_dec_pic_buffering_minus1[ vps_...
for(i = ( vps_sub_layer_ordering_info_present_flag ? 0 : vps_max_sub_layers_minus1 ); i <= vps_max_sub_layers_minus1; i++ ) {: 
    vps_max_dec_pic_buffering_minus1[3]: 64322    // plus 1 specifies the maximum required size of the decoded picture b...
    vps_max_num_reorder_pics[3]: 25348            // indicates the maximum allowed number of pictures with PicOutputFlag...
    vps_max_latency_increase_plus1[3]: 2483036160 // VpsMaxLatencyPictures[i] = vps_max_num_reorder_pics[i] + vps_max_la...
vps_max_layer_id: 0(0X0)                          // specifies the maximum allowed value of nuh_layer_id of all NAL unit...
vps_num_layer_sets_minus1: 0(0X0)                 // plus 1 specifies the number of layer sets that are specified by the...
vps_timing_info_present_flag: 0                   // specifies that vps_num_units_in_tick, vps_time_scale, vps_poc_propo...
vps_extension_flag: 0                             
rbsp_trailing_bits:                               
    rbsp_stop_one_bit: 1                          // Should be 1
    rbsp_alignment_zero_bit: 0                    // Should be 0
    rbsp_alignment_zero_bit: 0                    // Should be 0
    rbsp_alignment_zero_bit: 0                    // Should be 0
    rbsp_alignment_zero_bit: 0                    // Should be 0
    rbsp_alignment_zero_bit: 0                    // Should be 0
    rbsp_alignment_zero_bit: 0                    // Should be 0
```

[Top](#contents)
### Show SPS syntax fields
```
DumpTS 00005.h264 --showSPS
```
Here is the output:
```
NAL_UNIT:                                                      // Sequence parameter set
forbidden_zero_bit: 0                                          // shall be equal to 0.
nal_ref_idc: 1(0X1)
nal_unit_type: 7(0X7)                                          // Sequence parameter set
seq_parameter_set_rbsp():
    profile_idc: 100(0X64)                                     // High Profile
    constraint_set0_flag: 0
    constraint_set1_flag: 0
    constraint_set2_flag: 0
    constraint_set3_flag: 0
    constraint_set4_flag: 0
    constraint_set5_flag: 0
    reserved_zero_2bits: 0(0X0)
    level_idc: 40(0X28)                                        // 4
    seq_parameter_set_id: 0(0X0)                               // identifies the sequence parameter set that is referred to by the pi...
    chroma_format_idc: 1(0X1)                                  // 4:2:0
    bit_depth_luma_minus8: 0(0X0)
    bit_depth_chroma_minus8: 0(0X0)
    qpprime_y_zero_transform_bypass_flag: 0                    // the transform coefficient decoding process and picture construction...
    seq_scaling_matrix_present_flag: 0                         // the flags seq_scaling_list_present_flag[ i ] for i = 0..7 or i = 0....
    log2_max_frame_num_minus4: 3(0X3)                          // specifies the value of the variable MaxFrameNum that is used in fra...
    pic_order_cnt_type: 0(0X0)                                 // specifies the method to decode picture order count
    log2_max_pic_order_cnt_lsb_minus4: 0(0X0)                  // the value of the variable MaxPicOrderCntLsb that is used in the dec...
    max_num_ref_frames: 2(0X2)                                 // the maximum number of short-term and long-term reference frames, co...
    gaps_in_frame_num_value_allowed_flag: 0
    pic_width_in_mbs_minus1: 119(0X77)                         // plus 1 specifies the width of each decoded picture in units of macr...
    pic_height_in_map_units_minus1: 67(0X43)                   // plus 1 specifies the height in slice group map units of a decoded f...
    frame_mbs_only_flag: 1                                     // every coded picture of the coded video sequence is a coded frame co...
    direct_8x8_inference_flag: 1
    frame_cropping_flag: 1                                     // the frame cropping offset parameters follow next in the sequence pa...
    frame_crop_left_offset: 0(0X0)
    frame_crop_right_offset: 0(0X0)
    frame_crop_top_offset: 0(0X0)
    frame_crop_bottom_offset: 4(0X4)
    Concluded Values:
        SubWidthC: 2(0X2)
        SubHeightC: 2(0X2)
        BitDepthY=8+bit_depth_luma_minus8: 8(0X8)
        QpBdOffsetY=6*bit_depth_luma_minus8: 0(0X0)
        BitDepthC=8+bit_depth_chroma_minus8: 8(0X8)
        QpBdOffsetC=6*bit_depth_chroma_minus8: 0(0X0)
        MbWidthC: 8(0X8)
        MbHeightC: 8(0X8)
        RawMbBits=256*BitDepthY + 2*MbWidthC*MbHeightC*BitDepthC: 3072(0XC00)
        MaxFrameNum=2^(log2_max_frame_num_minus4 + 4): 5(0X5)
        MaxPicOrderCntLsb=2^(log2_max_pic_order_cnt_lsb_minus4 + 4): 6(0X6)
        PicWidthInMbs=pic_width_in_mbs_minus1 + 1: 120(0X78)   // the picture width in units of macroblocks
        PicWidthInSamplesL=PicWidthInMbs * 16: 1920(0X780)     // picture width for the luma component
        PicWidthInSamplesC=PicWidthInMbs * MbWidthC: 960(0X3C0)// picture width for the chroma components
        PicHeightInMapUnits=pic_height_in_map_units_minus1 + 1: 68(0X44)
        PicSizeInMapUnits=PicWidthInMbs * PicHeightInMapUnits: 8160(0X1FE0)
        FrameHeightInMbs=(2-frame_mbs_only_flag)*PicHeightInMapUnits: 68(0X44)
        ChromaArrayType: 1(0X1)
        CropUnitX: 2(0X2)
        CropUnitY: 2(0X2)
        BufferWidth: 1920(0X780)                               // The width of frame buffer
        BufferHeight: 1088(0X440)                              // The height of frame buffer
        DisplayWidth: 1920(0X780)                              // The display width
        DisplayHeight: 1080(0X438)                             // The display height
    vui_parameters_present_flag: 1                             // the vui_parameters() syntax structure is present
    vui_parameters():
        aspect_ratio_info_present_flag: 1                      // aspect_ratio_idc is present
        aspect_ratio_idc: 1                                    // 1:1(Square), example: 3840x2160 16:9 frame without horizontal overs...
        overscan_info_present_flag: 1                          // the overscan_appropriate_flag is present
        overscan_appropriate_flag: 1                           // indicates that the cropped decoded pictures output are suitable for...
        video_signal_type_present_flag: 0                      // specify that video_format, video_full_range_flag and colour_descrip...
        chroma_loc_info_present_flag: 0                        // specifies that chroma_sample_loc_type_top_field and chroma_sample_l...
        timing_info_present_flag: 1                            // specifies that vui_num_units_in_tick, vui_time_scale, vui_poc_propo...
        num_units_in_tick: 5005(0X138D)                        // the number of time units of a clock operating at the frequency vui_...
        time_scale: 240000(0X3A980)                            // the number of time units that pass in one second.
        fixed_frame_rate_flag: 1                               // the temporal distance between the HRD output times of any two conse...
        nal_hrd_parameters_present_flag: 1                     // NAL HRD parameters (pertaining to Type II bitstream conformance) ar...
        hrd_parameters:                                        // NAL HRD parameters
            cpb_cnt_minus1: 0(0X0)                             // plus 1 specifies the number of alternative CPB specifications in th...
            bit_rate_scale: 2(0X2)                             // (together with bit_rate_value_minus1[ SchedSelIdx ]) specifies the ...
            cpb_size_scale: 3(0X3)                             // (together with cpb_size_value_minus1[ SchedSelIdx ]) specifies the ...
            for(SchedSelIdx=0;SchedSelIdx<=cpb_cnt_minus1;SchedSelIdx++):
                bit_rate_value_minus1[0]: 78124                // (together with bit_rate_scale) specifies the maximum input bit rate...
                cpb_size_value_minus1[0]: 234374               // is used together with cpb_size_scale to specify the SchedSelIdx-th ...
                cbr_flag[0]: 0                                 // specifies that to decode this bitstream by the HRD using the SchedS...
                BitRate: 20000000                              // The bit rate in bits per second
                CpbSize: 30000000                              // The CPB size in bits
            initial_cpb_removal_delay_length_minus1: 17(0X11)  // the length in bits of the initial_cpb_removal_delay[SchedSelIdx] an...
            cpb_removal_delay_length_minus1: 7(0X7)            // the length in bits of the cpb_removal_delay syntax element
            dpb_output_delay_length_minus1: 7(0X7)             // the length in bits of the dpb_output_delay syntax element
            time_offset_length: 24(0X18)                       // greater than 0 specifies the length in bits of the time_offset synt...
        vcl_hrd_parameters_present_flag: 1                     // VCL HRD parameters (clauses E.1.2 and E.2.2) immediately follow the...
        vcl_parameters:                                        // VCL HRD parameters
            cpb_cnt_minus1: 0(0X0)                             // plus 1 specifies the number of alternative CPB specifications in th...
            bit_rate_scale: 2(0X2)                             // (together with bit_rate_value_minus1[ SchedSelIdx ]) specifies the ...
            cpb_size_scale: 4(0X4)                             // (together with cpb_size_value_minus1[ SchedSelIdx ]) specifies the ...
            for(SchedSelIdx=0;SchedSelIdx<=cpb_cnt_minus1;SchedSelIdx++):
                bit_rate_value_minus1[0]: 78124                // (together with bit_rate_scale) specifies the maximum input bit rate...
                cpb_size_value_minus1[0]: 78124                // is used together with cpb_size_scale to specify the SchedSelIdx-th ...
                cbr_flag[0]: 0                                 // specifies that to decode this bitstream by the HRD using the SchedS...
                BitRate: 20000000                              // The bit rate in bits per second
                CpbSize: 20000000                              // The CPB size in bits
            initial_cpb_removal_delay_length_minus1: 17(0X11)  // the length in bits of the initial_cpb_removal_delay[SchedSelIdx] an...
            cpb_removal_delay_length_minus1: 7(0X7)            // the length in bits of the cpb_removal_delay syntax element
            dpb_output_delay_length_minus1: 7(0X7)             // the length in bits of the dpb_output_delay syntax element
            time_offset_length: 24(0X18)                       // greater than 0 specifies the length in bits of the time_offset synt...
        low_delay_hrd_flag: 0
        pic_struct_present_flag: 1                             // picture timing SEI messages (clause D.2.2) are present that include...
        bitstream_restriction_flag: 1                          // the following coded video sequence bitstream restriction parameters...
        motion_vectors_over_pic_boundaries_flag: 1             // one or more samples outside picture boundaries may be used in inter...
        max_bytes_per_pic_denom: 4(0X4)                        // a number of bytes not exceeded by the sum of the sizes of the VCL N...
        max_bits_per_mb_denom: 0(0X0)                          // an upper bound for the number of coded bits of macroblock_layer( ) ...
        log2_max_mv_length_horizontal: 16(0X10)                // the maximum absolute value of a decoded horizontal motion vector co...
        log2_max_mv_length_vertical: 16(0X10)                  // the maximum absolute value of a decoded vertical motion vector comp...
        max_num_reorder_frames: 1(0X1)                         // an upper bound for the number of frames buffers, in the decoded pic...
        max_dec_frame_buffering: 4(0X4)                        // the required size of the HRD decoded picture buffer (DPB) in units ...
    rbsp_trailing_bits:
        rbsp_stop_one_bit: 1                                   // Should be 1
        rbsp_alignment_zero_bit: 0                             // Should be 0
        rbsp_alignment_zero_bit: 0                             // Should be 0
        rbsp_alignment_zero_bit: 0                             // Should be 0
        rbsp_alignment_zero_bit: 0                             // Should be 0
        rbsp_alignment_zero_bit: 0                             // Should be 0
        rbsp_alignment_zero_bit: 0                             // Should be 0
        rbsp_alignment_zero_bit: 0                             // Should be 0
```

[Top](#contents)
### Show PPS syntax fields
```
DumpTS 00002.hevc --showpps
```
Here is the output:
```
NAL_UNIT:                                         // non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( )
    forbidden_zero_bit: 0                         // shall be equal to 0.
    nal_unit_type: 34(0X22)                       // non-VCL::PPS_NUT Picture parameter set pic_parameter_set_rbsp( )
    nuh_layer_id: 0(0X0)                          // the identifier of the layer to which a VCL NAL unit belongs or the ...
    nuh_temporal_id_plus1: 0(0X0)                 // TemporalId = nuh_temporal_id_plus1 - 1
    pps_pic_parameter_set_id: 0(0X0)              // identifies the PPS for reference by other syntax elements
    pps_seq_parameter_set_id: 0(0X0)              // the value of sps_seq_parameter_set_id for the active SPS
    dependent_slice_segments_enabled_flag: 0      // specifies the absence of the syntax element dependent_slice_segment...
    output_flag_present_flag: 0                   // indicates that the pic_output_flag syntax element is not present in...
    num_extra_slice_header_bits: 0(0X0)           // the number of extra slice header bits that are present in the slice...
    sign_data_hiding_enabled_flag: 0              // specifies that sign bit hiding is disabled
    cabac_init_present_flag: 0                    // specifies that cabac_init_flag is not present in slice headers refe...
    num_ref_idx_l0_default_active_minus1: 0(0X0)  // the inferred value of num_ref_idx_l0_active_minus1 for P and B slic...
    num_ref_idx_l1_default_active_minus1: 0(0X0)  // the inferred value of num_ref_idx_l1_active_minus1 with num_ref_idx...
    init_qp_minus26: 0                            // plus 26 specifies the initial value of SliceQpY for each slice refe...
    constrained_intra_pred_flag: 0                // specifies that intra prediction allows usage of residual data and d...
    transform_skip_enabled_flag: 1                // specifies that transform_skip_flag may be present in the residual c...
    cu_qp_delta_enabled_flag: 1                   // specifies that the diff_cu_qp_delta_depth syntax element is present...
    if( cu_qp_delta_enabled_flag ):               
        diff_cu_qp_delta_depth: 2(0X2)            // specifies the difference between the luma coding tree block size an...
    pps_cb_qp_offset: 0                           // specify the offsets to the luma quantization parameter Qp'Y used fo...
    pps_cr_qp_offset: 0                           // specify the offsets to the luma quantization parameter Qp'Y used fo...
    pps_slice_chroma_qp_offsets_present_flag: 0   // these syntax elements are not present in the associated slice heade...
    weighted_pred_flag: 0                         // specifies that weighted prediction is not applied to P slices
    weighted_bipred_flag: 0                       // specifies that the default weighted prediction is applied to B slic...
    transquant_bypass_enabled_flag: 0             // specifies that cu_transquant_bypass_flag is not present
    tiles_enabled_flag: 0                         // specifies that there is only one tile in each picture referring to ...
    entropy_coding_sync_enabled_flag: 0           // specifies that no specific synchronization process for context vari...
    pps_loop_filter_across_slices_enabled_flag: 1 // specifies that in-loop filtering operations may be performed across...
    deblocking_filter_control_present_flag: 0     // specifies the absence of deblocking filter control syntax elements ...
    pps_scaling_list_data_present_flag: 1         // specifies that parameters are present in the PPS to modify the scal...
    scaling_list_data:                            
        for(sizeId=0;sizeId<4;sizeId++):          
            for(matrixId=0;matrixId<6;matrixId+=(sizeId==3)?3:1): 
                scaling_list_pred_mode_flag[0][0]: 1
                nextCoef = 8: 8(0X8)              
                coefNum = Min(64, (1 << (4 + (sizeId << 1)))): 16(0X10)
                if(sizeId>1):                     
                for(i = 0; i < coefNum; i++):     
                    scaling_list_delta_coef[0][0][0]: 8
                    nextCoef[0][0][0] = (nextCoef + scaling_list_delta_coef[0][0][0] + 256)%256: 16(0X10)
                    nScalingList[0][0][0] = nextCoef[0][0][0]: 16(0X10)
                    scaling_list_delta_coef[0][0][1]: 0
                    nextCoef[0][0][1] = (nextCoef + scaling_list_delta_coef[0][0][1] + 256)%256: 16(0X10)
                    nScalingList[0][0][1] = nextCoef[0][0][1]: 16(0X10)
                    scaling_list_delta_coef[0][0][2]: 0
                    nextCoef[0][0][2] = (nextCoef + scaling_list_delta_coef[0][0][2] + 256)%256: 16(0X10)
                    nScalingList[0][0][2] = nextCoef[0][0][2]: 16(0X10)
                    scaling_list_delta_coef[0][0][3]: 0
                    nextCoef[0][0][3] = (nextCoef + scaling_list_delta_coef[0][0][3] + 256)%256: 16(0X10)
                    nScalingList[0][0][3] = nextCoef[0][0][3]: 16(0X10)
                    scaling_list_delta_coef[0][0][4]: 0
                    nextCoef[0][0][4] = (nextCoef + scaling_list_delta_coef[0][0][4] + 256)%256: 16(0X10)
                    nScalingList[0][0][4] = nextCoef[0][0][4]: 16(0X10)
                    scaling_list_delta_coef[0][0][5]: 0
                    nextCoef[0][0][5] = (nextCoef + scaling_list_delta_coef[0][0][5] + 256)%256: 16(0X10)
                    nScalingList[0][0][5] = nextCoef[0][0][5]: 16(0X10)
                    scaling_list_delta_coef[0][0][6]: 0
                    nextCoef[0][0][6] = (nextCoef + scaling_list_delta_coef[0][0][6] + 256)%256: 16(0X10)
                    nScalingList[0][0][6] = nextCoef[0][0][6]: 16(0X10)
                    scaling_list_delta_coef[0][0][7]: 0
                    nextCoef[0][0][7] = (nextCoef + scaling_list_delta_coef[0][0][7] + 256)%256: 16(0X10)
                    nScalingList[0][0][7] = nextCoef[0][0][7]: 16(0X10)
                    scaling_list_delta_coef[0][0][8]: 0
                    nextCoef[0][0][8] = (nextCoef + scaling_list_delta_coef[0][0][8] + 256)%256: 16(0X10)
                    nScalingList[0][0][8] = nextCoef[0][0][8]: 16(0X10)
                    scaling_list_delta_coef[0][0][9]: 0
                    nextCoef[0][0][9] = (nextCoef + scaling_list_delta_coef[0][0][9] + 256)%256: 16(0X10)
                    nScalingList[0][0][9] = nextCoef[0][0][9]: 16(0X10)
                    scaling_list_delta_coef[0][0][10]: 0
                    nextCoef[0][0][10] = (nextCoef + scaling_list_delta_coef[0][0][10] + 256)%256: 16(0X10)
                    nScalingList[0][0][10] = nextCoef[0][0][10]: 16(0X10)
                    scaling_list_delta_coef[0][0][11]: 0
                    nextCoef[0][0][11] = (nextCoef + scaling_list_delta_coef[0][0][11] + 256)%256: 16(0X10)
                    nScalingList[0][0][11] = nextCoef[0][0][11]: 16(0X10)
                    scaling_list_delta_coef[0][0][12]: 0
                    nextCoef[0][0][12] = (nextCoef + scaling_list_delta_coef[0][0][12] + 256)%256: 16(0X10)
                    nScalingList[0][0][12] = nextCoef[0][0][12]: 16(0X10)
                    scaling_list_delta_coef[0][0][13]: 0
                    nextCoef[0][0][13] = (nextCoef + scaling_list_delta_coef[0][0][13] + 256)%256: 16(0X10)
                    nScalingList[0][0][13] = nextCoef[0][0][13]: 16(0X10)
                    scaling_list_delta_coef[0][0][14]: 0
                    nextCoef[0][0][14] = (nextCoef + scaling_list_delta_coef[0][0][14] + 256)%256: 16(0X10)
                    nScalingList[0][0][14] = nextCoef[0][0][14]: 16(0X10)
                    scaling_list_delta_coef[0][0][15]: 0
                    nextCoef[0][0][15] = (nextCoef + scaling_list_delta_coef[0][0][15] + 256)%256: 16(0X10)
                    nScalingList[0][0][15] = nextCoef[0][0][15]: 16(0X10)
...
```

[Top](#contents)
## Show HRD model information
HRD model information may exist in SPS, SEI payload, 
```
DumpTS 00005.h264 --showHRD
```
And then
```
Type-I HRD:
        SchedSelIdx#0:
                the maximum input bit rate for the CPB: 20000000bps/20Mbps
                the CPB size: 20000000b/20Mb
                VBR mode
Type-II HRD:
        SchedSelIdx#0:
                the maximum input bit rate for the CPB: 20000000bps/20Mbps
                the CPB size: 30000000b/30Mb
                VBR mode
Type-I HRD:
        SchedSelIdx#0:
                initial_cpb_removal_delay: 90000
                initial_cpb_removal_delay_offset: 0
Type-II HRD:
        SchedSelIdx#0:
                initial_cpb_removal_delay: 90000
                initial_cpb_removal_delay_offset: 0
Picture Timing(payloadLength: 9):
        cpb_removal_delay: 0
        dpb_output_delay: 2
        pic_struct: 0 (progressive frame)
        ClockTS#0:
                clock_timestamp_flag: 1
                ct_type: 0(progressive)
                nuit_field_based_flag: 1
                counting_type: 3
                full_timestamp_flag: 0
                discontinuity_flag: 1
                cnt_dropped_flag: 0
                time code: 0f
                time offset: 0
Picture Timing(payloadLength: 9):
        cpb_removal_delay: 2
        dpb_output_delay: 6
        pic_struct: 0 (progressive frame)
        ClockTS#0:
                clock_timestamp_flag: 1
                ct_type: 0(progressive)
                nuit_field_based_flag: 1
                counting_type: 3
                full_timestamp_flag: 0
                discontinuity_flag: 1
                cnt_dropped_flag: 0
......
```
For HEVC stream,
```
DumpTS 02021_interlaced.hevc --showHRD
```
And then
```
Type-I HRD:
        Sublayer#0:
                SchedSelIdx#0:
                        the maximum input bit rate for the CPB: 10000000bps/10Mbps
                        the CPB size: 10000000b/10Mb
                        VBR mode
Type-II HRD:
        Sublayer#0:
                SchedSelIdx#0:
                        the maximum input bit rate for the CPB: 10000000bps/10Mbps
                        the CPB size: 10000000b/10Mb
                        VBR mode
Picture Timing(payloadLength: 3):
        pic_struct: 1 (top field)
        source_scan_type: 0 (Interlaced)
        duplicate_flag: 0
        au_cpb_removal_delay: 1
        pic_dpb_output_delay: 4
Picture Timing(payloadLength: 3):
        pic_struct: 2 (bottom field)
        source_scan_type: 0 (Interlaced)
        duplicate_flag: 0
        au_cpb_removal_delay: 1
        pic_dpb_output_delay: 4
Picture Timing(payloadLength: 3):
        pic_struct: 1 (top field)
        source_scan_type: 0 (Interlaced)
        duplicate_flag: 0
        au_cpb_removal_delay: 2
        pic_dpb_output_delay: 10
Picture Timing(payloadLength: 3):
        pic_struct: 2 (bottom field)
        source_scan_type: 0 (Interlaced)
        duplicate_flag: 0
        au_cpb_removal_delay: 3
        pic_dpb_output_delay: 10
Picture Timing(payloadLength: 3):
        pic_struct: 1 (top field)
        source_scan_type: 0 (Interlaced)
        duplicate_flag: 0
        au_cpb_removal_delay: 4
        pic_dpb_output_delay: 4
Picture Timing(payloadLength: 3):
        pic_struct: 2 (bottom field)
...
```
[Top](#contents)

## Transcode NAL video to AVC with the specified bitrate
At present, only support it in the windows platform
```
DumpTS FILEZ058.h264 --bitrate=2000000 --output=FILEZ058_2M.h264
```
It will convert the current AVC video clip into 2Mbps
