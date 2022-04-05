# AV1 operation guideline
## Contents
* [Export av1 bitstream from webm file](#export-av1-bitstream-from-webm-file)
* [Show AV1 Sequence Header OBU](#show-av1-sequence-header-obu)

[Return to Main](../README.md)
## Export av1 bitstream from webm file
Here are the steps to get the AV1 bitstream from WebM file
```
DumpTS Stream3_AV1_720p_3.9mbps.webm --showinfo | more
```
Show the rough information of WebM file
```
 .
  |--EBML (Size: 31)
  |    |--EBMLVersion (Size: 1): 1
  |    |--EBMLReadVersion (Size: 1): 1
  |    |--EBMLMaxIDLength (Size: 1): 4
  |    |--EBMLMaxSizeLength (Size: 1): 8
  |    |--DocType (Size: 4): webm
  |    |--DocTypeVersion (Size: 1): 4
  |    |--DocTypeReadVersion (Size: 1): 2
  |--Segment (Size: 89056397)
       |--SeekHead (Size: 59)
       |    |--Seek (Size: 11)
       |    |    |--SeekID (Size: 4): Binary
       |    |    |--SeekPosition (Size: 1): 110
       |    |--Seek (Size: 11)
       |    |    |--SeekID (Size: 4): Binary
       |    |    |--SeekPosition (Size: 1): 162
       |    |--Seek (Size: 11)
       |    |    |--SeekID (Size: 4): Binary
       |    |    |--SeekPosition (Size: 1): 202
       |    |--Seek (Size: 14)
       |         |--SeekID (Size: 4): Binary
       |         |--SeekPosition (Size: 4): 89054605
       |--Void (Size: 44): Binary
       |--Info (Size: 47)
       |    |--TimecodeScale (Size: 3): 1000000
       |    |--Duration (Size: 4): 181520.000000
       |    |--MuxingApp (Size: 15): libwebm-0.2.1.0
       |    |--WritingApp (Size: 12): aomenc 1.0.0
       |--Tracks (Size: 35)
       |    |--TrackEntry (Size: 33)
       |         |--TrackNumber (Size: 1): 1
       |         |--TrackUID (Size: 7): 43893850464213234
       |         |--TrackType (Size: 1): 1
       |         |--CodecID (Size: 5): V_AV1
```
It can conclude that the track number is 1, ok use the below command to show the information of this track:
```
DumpTS Stream3_AV1_720p_3.9mbps.webm --trackid=1 --showinfo
```
And then extract the AV1 stream to your local drive:
```
DumpTS Stream3_AV1_720p_3.9mbps.webm --trackid=1 --output=Stream3_AV1_720p_3.9mbps.av1
```
Finally to check this AV1 stream file
```
DumpTS Stream3_AV1_720p_3.9mbps.av1 --showobu | more
[AV1] hit one temporal_unit (Leb128Bytes: 1, unit_size: 18).
[AV1]   hit one frame_unit (Leb128Bytes: 1, unit_size: 0).
[AV1][obu unit] The sub unit costs too many bytes(1) which exceed the upper unit size(0).
[AV1][temporal_unit#00000000] The current file: Stream3_AV1_720p_3.9mbps.av1 is NOT an Annex-B length delimited bitstream.
[AV1]           hit obu_type: Temporal delimiter OBU.
[AV1]           hit obu_type: Sequence header OBU.
[AV1] The current file: Stream3_AV1_720p_3.9mbps.av1 is a low overhead bitstream format.
Low-Overhead AV1 bitstream...
Temporal Unit#0
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Sequence header OBU
                OBU#2: Frame OBU
Temporal Unit#1
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame OBU
                OBU#2: Frame OBU
                OBU#3: Frame OBU
                OBU#4: Frame OBU
Temporal Unit#2
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame header OBU
Temporal Unit#3
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame OBU
Temporal Unit#4
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame header OBU
Temporal Unit#5
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame OBU
                OBU#2: Frame OBU
                OBU#3: Frame OBU
Temporal Unit#6
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame header OBU
Temporal Unit#7
        Frame Unit#0
```

[Top](#contents)
## Show AV1 Sequence Header OBU
```
DumpTs stream3_av1_720p_3.av1 --showSeqHdr
```
And then show
```
obu_header():
    obu_forbidden_bit: 0                          // must be set to 0
    obu_type: 1(0X1)                              // Sequence header OBU
    obu_extension_flag: 0                         // indicates if the optional obu_extension_header is present
    obu_has_size_field: 1                         // indicates that the obu_size syntax element will be present
    obu_reserved_1bit: 0                          // must be set to 0
obu_size: 11(0XB)                                 // contains the size in bytes of the OBU not including the bytes withi...
sequence_header_obu():
    seq_profile: 0(0X0)                           // specifies the features that can be used in the coded video sequence
    still_picture: 0                              // the coded video sequence contains one or more coded frames
    reduced_still_picture_header: 0               // specifies that the syntax elements not needed by a still picture ar...
    timing_info_present_flag: 0                   // timing info is NOT present in the coded video sequence
    initial_display_delay_present_flag: 0         // specifies whether initial display delay information is present in t...
    operating_points_cnt_minus_1: 0(0X0)          // plus 1 indicates the number of operating points present in the code...
    for(i=0;i<=operating_points_cnt_minus_1;i++):
        operating_point[0]:
            operating_point_idc: 0(0X0)           // contains a bitmask that indicates which spatial and temporal layers...
            seq_level_idx: 5(0X5)                 // specifies the level that the coded video sequence conforms to when ...
            decoder_model_present_for_this_op: 0
    frame_width_bits_minus_1: 10(0XA)             // the number of bits minus 1 used for transmitting the frame width sy...
    frame_height_bits_minus_1: 9(0X9)             // the number of bits minus 1 used for transmitting the frame height s...
    max_frame_width_minus_1: 1279(0X4FF)          // the maximum frame width minus 1 for the frames represented by this ...
    max_frame_height_minus_1: 719(0X2CF)          // the maximum frame height minus 1 for the frames represented by this...
    frame_id_numbers_present_flag: 0              // frame id numbers are NOT present in the coded video sequence
        delta_frame_id_length_minus_2: 6(0X6)     // plus 2 used to encode delta_frame_id syntax elements
        additional_frame_id_length_minus_1: 6(0X6)// is used to calculate the number of bits used to encode the frame_id...
    use_128x128_superblock: 1                     // superblocks contain 128x128 luma samples
    enable_filter_intra: 1                        // the use_filter_intra syntax element may be present
    enable_intra_edge_filter: 1                   // the intra edge filtering process should be enabled
    enable_interintra_compound: 1                 // the mode info for inter blocks may contain the syntax element inter...
    enable_masked_compound: 1                     // the mode info for inter blocks may contain the syntax element compo...
    enable_warped_motion: 1                       // the allow_warped_motion syntax element may be present
    enable_dual_filter: 1                         // the inter prediction filter type may be specified independently in ...
    enable_order_hint: 1                          // tools based on the values of order hints may be used
    enable_jnt_comp: 1                            // the distance weights process may be used for inter prediction
    enable_ref_frame_mvs: 1                       // the use_ref_frame_mvs syntax element may be present
    seq_choose_screen_content_tools: 1            // seq_force_screen_content_tools should be set equal to SELECT_SCREEN...
    seq_force_screen_content_tools: 2(0X2)        // the allow_screen_content_tools syntax element will be present in th...
    seq_choose_integer_mv: 1                      // seq_force_integer_mv should be set equal to SELECT_INTEGER_MV
    seq_force_integer_mv: 2(0X2)                  // Should be SELECT_INTEGER_MV
    order_hint_bits_minus_1: 6(0X6)               // plus 1 specifies the number of bits used for the order_hint syntax ...
    OrderHintBits: 7                              // the number of bits used for the order_hint syntax element
    enable_superres: 0                            // the use_superres syntax element will not be present
    enable_cdef: 1                                // cdef filtering may be enabled
    enable_restoration: 1                         // loop restoration filtering may be enabled
    color_config():
        high_bitdepth: 0                          // together with seq_profile, determine the bit depth
        BitDepth: 8(0X8)                          // BitDepth = high_bitdepth ? 10 : 8
        mono_chrome: 0                            // indicates that the video contains Y, U, and V color planes
        NumPlanes: 3(0X3)                         // NumPlanes = mono_chrome ? 1 : 3
        color_description_present_flag: 0         // specifies that color_primaries, transfer_characteristics and matrix...
            color_primaries: 2(0X2)               // Unspecified
            transfer_characteristics: 2(0X2)      // Unspecified
            matrix_coefficients: 2(0X2)           // Unspecified
        color_range: 0                            // shall be referred to as the studio swing representation
        subsampling_x: 1(0X1)
        subsampling_y: 1(0X1)
        YUV Color-space:                          // YUV 4:2:0
        chroma_sample_position: 0(0X0)            // Unknown(in this case the source video transfer function must be sig...
    film_gain_params_present: 0                   // film grain parameters are NOT present in the coded video sequence
trailing_bits():
    trailing_one_bit: 1                           // shall be equal to 1
    trailing_zero_bit[0]: 0
    trailing_zero_bit[1]: 0
```

[Top](#contents)
