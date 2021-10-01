# MMT operation guideline



## Show MMT information

This command option will show the layout and summary information of the MMT/TVL stream

```
DumpTS 29999.mmts --showinfo
Layout of MMT:
    CID: 0X1(1):
        #00000 MPT (MMT Package Table), Package_id: 0XD3(211), :
            #00000 Asset, asset_id: 0X788(1928), asset_type: hev1(0X68657631):
                packet_id: 0XF100(61696)
                ++++++++++++++ Descriptor: Video component Descriptor +++++++++++++
                          file offset: 44
                       descriptor_tag: 0X8010
                    descriptor_length: 8
                     video_resolution: 6(0X6), height: 2160
                   video_aspect_ratio: 3(0X3), 16:9 without pan vector
                      video_scan_flag: 1, interlaced
                     video_frame_rate: 8(0X8), 60/1.001 fps
                        component_tag: 0(0X0)
                trans_characteristics: 5(0X5), Rec. ITU-R BT.2100 HLG
                             language: jpn (0X006A706E)
            #00001 Asset, asset_id: 0X798(1944), asset_type: mp4a(0X6D703461):
                packet_id: 0XF110(61712)
                ++++++++++++++ Descriptor: MH-Audio component Descriptor +++++++++++++
                          file offset: 188
                       descriptor_tag: 0X8014
                    descriptor_length: 16
                       stream_content: 3(0X3), Sound stream by MPEG-4 AAC
                       Dialog_control: 0, Audio stream doesn't include dialog control information
                    Sound_handicapped: 0(0X0), Audio for the handicapped is not specified
                           Audio_mode: 3(0X3), 2/0 mode (stereo)
                        component_tag: 16(0X10)
                          stream_type: 17(0X11)
                  simulcast_group_tag: 255(0XFF)
                ES_multi_lingual_flag: 0
                  main_component_flag: 1, main audio
                    quality_indicator: 1Mode 1
                        sampling_rate: 48 kHZ
                             language: jpn (0X006A706E)
                                 text: 音声
            #00002 Asset, asset_id: 0X7B8(1976), asset_type: stpp(0X73747070):
                packet_id: 0XF138(61752)
                ++++++++++++++ Descriptor: MH-Data coding system Descriptor +++++++++++++
                          file offset: 5
                       descriptor_tag: 0X8020
                    descriptor_length: 10
                    data_component_id: 32(0X20) - Closed-caption coding system
                         subtitle_tag: 56(0X38)
                subtitle_info_version: 0(0X0)
                start_mpu_sequence_number_flag: 0
                ISO_639_language_code: jpn
                                 type: 1(0X1) - Superimposition
                      subtitle_format: 0(0X0) - ARIB-TTML
                                  OPM: 1(0X1) - Segment mode
                                  TMD: 15(0XF)- without time control
                                  DMF: 2(0X2) - Automatic display when received
                           resolution: 1(0X1) - 3840 x 2160
                     compression_type: 0(0X0)
        #0001 PLT (Package List Table):
            #00000 MPT (MMT Package Table), Package_id: 0XD3(211), packet_id: 0XFF01(65281):
                #00000 Asset, asset_id: 0X788(1928), asset_type: hev1(0X68657631):
                    packet_id: 0XF100(61696)
                    ++++++++++++++ Descriptor: Video component Descriptor +++++++++++++
                              file offset: 44
                           descriptor_tag: 0X8010
                        descriptor_length: 8
                         video_resolution: 6(0X6), height: 2160
                       video_aspect_ratio: 3(0X3), 16:9 without pan vector
                          video_scan_flag: 1, interlaced
                         video_frame_rate: 8(0X8), 60/1.001 fps
                            component_tag: 0(0X0)
                    trans_characteristics: 5(0X5), Rec. ITU-R BT.2100 HLG
                                 language: jpn (0X006A706E)
                #00001 Asset, asset_id: 0X798(1944), asset_type: mp4a(0X6D703461):
                    packet_id: 0XF110(61712)
                    ++++++++++++++ Descriptor: MH-Audio component Descriptor +++++++++++++
                              file offset: 188
                           descriptor_tag: 0X8014
                        descriptor_length: 16
                           stream_content: 3(0X3), Sound stream by MPEG-4 AAC
                           Dialog_control: 0, Audio stream does not include dialog control information
                        Sound_handicapped: 0(0X0), Audio for the handicapped is not specified
                               Audio_mode: 3(0X3), 2/0 mode (stereo)
                            component_tag: 16(0X10)
                              stream_type: 17(0X11)
                      simulcast_group_tag: 255(0XFF)
                    ES_multi_lingual_flag: 0
                      main_component_flag: 1, main audio
                        quality_indicator: 1Mode 1
                            sampling_rate: 48 kHZ
                                 language: jpn (0X006A706E)
                                     text: 音声
                #00002 Asset, asset_id: 0X7B8(1976), asset_type: stpp(0X73747070):
                    packet_id: 0XF138(61752)
                    ++++++++++++++ Descriptor: MH-Data coding system Descriptor +++++++++++++
                              file offset: 5
                           descriptor_tag: 0X8020
                        descriptor_length: 10
                        data_component_id: 32(0X20) - Closed-caption coding system
                             subtitle_tag: 56(0X38)
                    subtitle_info_version: 0(0X0)
                    start_mpu_sequence_number_flag: 0
                    ISO_639_language_code: jpn
                                     type: 1(0X1) - Superimposition
                          subtitle_format: 0(0X0) - ARIB-TTML
                                      OPM: 1(0X1) - Segment mode
                                      TMD: 15(0XF)- without time control
                                      DMF: 2(0X2) - Automatic display when received
                               resolution: 1(0X1) - 3840 x 2160
                         compression_type: 0(0X0)

The total number of TLV packets: 150437.
The number of IPv4 TLV packets: 0.
The number of IPv6 TLV packets: 1849.
The number of Header Compressed IP packets: 148589.
    CID: 0x0001, packet_id: 0x8000, count:     777 - MH-EIT
    CID: 0x0001, packet_id: 0x0000, count:     610 - PA Message
    CID: 0x0001, packet_id: 0xF110, count:   2,865 - MPEG-4 AAC Audio Stream
    CID: 0x0001, packet_id: 0xFF01, count:     611 - MPT
    CID: 0x0001, packet_id: 0x8004, count:      84 - MH-SDT
    CID: 0x0001, packet_id: 0xF100, count: 143,636 - HEVC Video Stream
The number of Transmission Control Signal TLV packets: 0.
The number of Null TLV packets: 0.
The number of other TLV packets: 0.
```

From the above text, you can see the PLT and MPT rough information, and the related assets in MPT, and the Stat. of every CID/packet_id stream or message, for example, there is one HEVC stream which CID and packet_id is 0x0001 and 0xF100 separately.

## Dump the specified elementary stream

Specified the CID and packet_id to dump the elementary stream raw data, or the control message raw data, for HEVC and MP4A, the assistant NAL length header and MP4 LOAS header will be removed at default

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --output=29999.hevc
Total cost: 4405.854400 ms
Total input TLV packets: 150437.
Total MMT/TLV packets with the packet_id(0XF100): 143636
Total MFUs in MMT/TLV packets with the packet_id(0XF100): 139965
```

Dump the HEVC stream with CID and packet_id are equal to 0x0001 and 0xF1000 separately, and save it to 29999.hevc, this HEVC stream will be Annex-B byte-stream format, and can be played with some players, or is used by other author software directly, for example ffmpeg:

```
ffmpeg -i 29999.hevc -pix_fmt yuv420p -f null /dev/null
```

## List the MPUs and the presentation time

List all MPU entries for the stream with the specified CID and packet_id

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --listMPUtime
0, CID: 0x0001(    1), packet_id: 0xF100(61696):
      0, MPU(SeqNo: 0x3BDFBA(3923898), presentation_time: 3763428926.080452s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0x00000000
      1, MPU(SeqNo: 0x3BDFBB(3923899), presentation_time: 3763428926.614311s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80103EC
      2, MPU(SeqNo: 0x3BDFBC(3923900), presentation_time: 3763428927.148182s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80108D5
      3, MPU(SeqNo: 0x3BDFBD(3923901), presentation_time: 3763428927.682049s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8010E5B
      4, MPU(SeqNo: 0x3BDFBE(3923902), presentation_time: 3763428928.215909s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8011358
      5, MPU(SeqNo: 0x3BDFBF(3923903), presentation_time: 3763428928.749780s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE801170C
      6, MPU(SeqNo: 0x3BDFC0(3923904), presentation_time: 3763428929.283646s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8011B3C
      7, MPU(SeqNo: 0x3BDFC1(3923905), presentation_time: 3763428929.817517s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8011FC1
      8, MPU(SeqNo: 0x3BDFC2(3923906), presentation_time: 3763428930.351377s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8012471
      9, MPU(SeqNo: 0x3BDFC3(3923907), presentation_time: 3763428930.885247s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8012950
     10, MPU(SeqNo: 0x3BDFC4(3923908), presentation_time: 3763428931.419118s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8012E47
     11, MPU(SeqNo: 0x3BDFC5(3923909), presentation_time: 3763428931.952980s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8013445
     12, MPU(SeqNo: 0x3BDFC6(3923910), presentation_time: 3763428932.486844s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80138F9
     13, MPU(SeqNo: 0x3BDFC7(3923911), presentation_time: 3763428933.020715s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8013D07
     14, MPU(SeqNo: 0x3BDFC8(3923912), presentation_time: 3763428933.554585s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8014180
     15, MPU(SeqNo: 0x3BDFC9(3923913), presentation_time: 3763428934.088443s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80146A2
     16, MPU(SeqNo: 0x3BDFCA(3923914), presentation_time: 3763428934.622312s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8014B92
     17, MPU(SeqNo: 0x3BDFCB(3923915), presentation_time: 3763428935.156176s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE801509A
     18, MPU(SeqNo: 0x3BDFCC(3923916), presentation_time: 3763428935.690042s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8015652
     19, MPU(SeqNo: 0x3BDFCD(3923917), presentation_time: 3763428936.223909s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8015B27
     20, MPU(SeqNo: 0x3BDFCE(3923918), presentation_time: 3763428936.757780s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE8015E5B
```

If you want to show the au time information and pts/dts, give the option 'listMPUtime' to 'full':

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --listMPUtime=full
0, CID: 0x0001(    1), packet_id: 0xF100(61696):
      0, MPU(SeqNo: 0x3BDFBA(3923898), presentation_time: 3763428926.080452s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0x00000000
         0, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482395190, dts: 7482380175
         1, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482389184, dts: 7482381677
         2, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482386181, dts: 7482383178
         3, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482384680, dts: 7482384680
         4, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482387683, dts: 7482386181
         5, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482392187, dts: 7482387683
         6, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482390686, dts: 7482389184
         7, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482393689, dts: 7482390686
         8, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482407202, dts: 7482392187
         9, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482401196, dts: 7482393689
        10, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482398193, dts: 7482395190
        11, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482396692, dts: 7482396692
        12, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482399695, dts: 7482398193
        13, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482404199, dts: 7482399695
        14, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482402698, dts: 7482401196
        15, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482405701, dts: 7482402698
        16, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482419214, dts: 7482404199
        17, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482413208, dts: 7482405701
        18, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482410205, dts: 7482407202
        19, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482408704, dts: 7482408704
        20, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482411707, dts: 7482410205
        21, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482416211, dts: 7482411707
        22, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482414710, dts: 7482413208
        23, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482417713, dts: 7482414710
        24, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482431226, dts: 7482416211
        25, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482425220, dts: 7482417713
        26, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482422217, dts: 7482419214
        27, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482420716, dts: 7482420716
        28, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482423719, dts: 7482422217
        29, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482428223, dts: 7482423719
        30, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482426722, dts: 7482425220
        31, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482429725, dts: 7482426722
      1, MPU(SeqNo: 0x3BDFBB(3923899), presentation_time: 3763428926.614311s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80103EC
         0, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482443238, dts: 7482428223
         1, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482437231, dts: 7482429724
         2, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482434229, dts: 7482431226
         3, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482432727, dts: 7482432727
         4, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482435731, dts: 7482434229
         5, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482440234, dts: 7482435730
         6, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482438734, dts: 7482437232
         7, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482441736, dts: 7482438733
         8, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482455250, dts: 7482440235
         9, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482449243, dts: 7482441736
        10, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482446241, dts: 7482443238
        11, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482444739, dts: 7482444739
        12, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482447743, dts: 7482446241
        13, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482452246, dts: 7482447742
        14, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482450746, dts: 7482449244
        15, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482453748, dts: 7482450745
        16, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482467262, dts: 7482452247
        17, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482461255, dts: 7482453748
        18, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482458253, dts: 7482455250
        19, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482456751, dts: 7482456751
        20, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482459755, dts: 7482458253
        21, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482464258, dts: 7482459754
        22, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482462758, dts: 7482461256
        23, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482465760, dts: 7482462757
        24, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482479274, dts: 7482464259
        25, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482473267, dts: 7482465760
        26, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482470265, dts: 7482467262
        27, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482468763, dts: 7482468763
        28, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482471767, dts: 7482470265
        29, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482476270, dts: 7482471766
        30, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482474770, dts: 7482473268
        31, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482477772, dts: 7482474769
      2, MPU(SeqNo: 0x3BDFBC(3923900), presentation_time: 3763428927.148182s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80108D5
         0, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482491286, dts: 7482476271
         1, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482485279, dts: 7482477772
         2, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482482277, dts: 7482479274
         3, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482480775, dts: 7482480775
         4, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482483779, dts: 7482482277
         5, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482488282, dts: 7482483778
         6, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482486782, dts: 7482485280
         7, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482489784, dts: 7482486781
         8, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482503298, dts: 7482488283
         9, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482497291, dts: 7482489784
        10, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482494289, dts: 7482491286
        11, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482492787, dts: 7482492787
        12, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482495791, dts: 7482494289
        13, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482500294, dts: 7482495790
        14, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482498794, dts: 7482497292
        15, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482501796, dts: 7482498793
        16, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7482515310, dts: 7482500295
        17, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7482509303, dts: 7482501796
        18, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482506301, dts: 7482503298
        19, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7482504799, dts: 7482504799
        20, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482507803, dts: 7482506301
        21, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7482512306, dts: 7482507802
        22, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7482510806, dts: 7482509304
        23, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7482513808, dts: 7482510805
         ......
```

List the MPUs with the specified range

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --listMPUtime=full --start=0x3BE023 --end=0x3BE025
0, CID: 0x0001(    1), packet_id: 0xF100(61696):
      0, MPU(SeqNo: 0x3BE023(3924003), presentation_time: 3763428982.136444s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE802FFA6
         0, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487440229, dts: 7487425214
         1, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487434223, dts: 7487426716
         2, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487431220, dts: 7487428217
         3, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487429719, dts: 7487429719
         4, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487432722, dts: 7487431220
         5, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487437226, dts: 7487432722
         6, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487435725, dts: 7487434223
         7, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487438728, dts: 7487435725
         8, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487452241, dts: 7487437226
         9, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487446235, dts: 7487438728
        10, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487443232, dts: 7487440229
        11, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487441731, dts: 7487441731
        12, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487444734, dts: 7487443232
        13, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487449238, dts: 7487444734
        14, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487447737, dts: 7487446235
        15, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487450740, dts: 7487447737
        16, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487464253, dts: 7487449238
        17, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487458247, dts: 7487450740
        18, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487455244, dts: 7487452241
        19, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487453743, dts: 7487453743
        20, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487456746, dts: 7487455244
        21, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487461250, dts: 7487456746
        22, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487459749, dts: 7487458247
        23, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487462752, dts: 7487459749
        24, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487476265, dts: 7487461250
        25, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487470259, dts: 7487462752
        26, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487467256, dts: 7487464253
        27, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487465755, dts: 7487465755
        28, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487468758, dts: 7487467256
        29, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487473262, dts: 7487468758
        30, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487471761, dts: 7487470259
        31, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487474764, dts: 7487471761
      1, MPU(SeqNo: 0x3BE024(3924004), presentation_time: 3763428982.670314s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80304BC
         0, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487488278, dts: 7487473263
         1, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487482271, dts: 7487474764
         2, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487479269, dts: 7487476266
         3, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487477767, dts: 7487477767
         4, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487480771, dts: 7487479269
         5, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487485274, dts: 7487480770
         6, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487483774, dts: 7487482272
         7, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487486776, dts: 7487483773
         8, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487500290, dts: 7487485275
         9, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487494283, dts: 7487486776
        10, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487491281, dts: 7487488278
        11, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487489779, dts: 7487489779
        12, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487492783, dts: 7487491281
        13, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487497286, dts: 7487492782
        14, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487495786, dts: 7487494284
        15, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487498788, dts: 7487495785
        16, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487512302, dts: 7487497287
        17, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487506295, dts: 7487498788
        18, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487503293, dts: 7487500290
        19, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487501791, dts: 7487501791
        20, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487504795, dts: 7487503293
        21, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487509298, dts: 7487504794
        22, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487507798, dts: 7487506296
        23, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487510800, dts: 7487507797
        24, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487524314, dts: 7487509299
        25, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487518307, dts: 7487510800
        26, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487515305, dts: 7487512302
        27, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487513803, dts: 7487513803
        28, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487516807, dts: 7487515305
        29, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487521310, dts: 7487516806
        30, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487519810, dts: 7487518308
        31, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487522812, dts: 7487519809
```

List the MPU which sequence number is between 0x3BE023 and 0x3BE025, and its full time information

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --listMPUtime=full --MPUseqno=0x3BE023
0, CID: 0x0001(    1), packet_id: 0xF100(61696):
      0, MPU(SeqNo: 0x3BE023(3924003), presentation_time: 3763428982.136444s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE802FFA6
         0, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487440229, dts: 7487425214
         1, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487434223, dts: 7487426716
         2, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487431220, dts: 7487428217
         3, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487429719, dts: 7487429719
         4, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487432722, dts: 7487431220
         5, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487437226, dts: 7487432722
         6, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487435725, dts: 7487434223
         7, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487438728, dts: 7487435725
         8, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487452241, dts: 7487437226
         9, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487446235, dts: 7487438728
        10, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487443232, dts: 7487440229
        11, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487441731, dts: 7487441731
        12, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487444734, dts: 7487443232
        13, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487449238, dts: 7487444734
        14, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487447737, dts: 7487446235
        15, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487450740, dts: 7487447737
        16, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487464253, dts: 7487449238
        17, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487458247, dts: 7487450740
        18, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487455244, dts: 7487452241
        19, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487453743, dts: 7487453743
        20, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487456746, dts: 7487455244
        21, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487461250, dts: 7487456746
        22, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487459749, dts: 7487458247
        23, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487462752, dts: 7487459749
        24, dts_pts_offset: 0x754E(30030), pts_offset: 0x0BBB( 3003), pts: 7487476265, dts: 7487461250
        25, dts_pts_offset: 0x3AA6(15014), pts_offset: 0x0BBB( 3003), pts: 7487470259, dts: 7487462752
        26, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487467256, dts: 7487464253
        27, dts_pts_offset: 0x0000(    0), pts_offset: 0x0BBB( 3003), pts: 7487465755, dts: 7487465755
        28, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487468758, dts: 7487467256
        29, dts_pts_offset: 0x2330( 9008), pts_offset: 0x0BBB( 3003), pts: 7487473262, dts: 7487468758
        30, dts_pts_offset: 0x0BBC( 3004), pts_offset: 0x0BBB( 3003), pts: 7487471761, dts: 7487470259
        31, dts_pts_offset: 0x1776( 6006), pts_offset: 0x0BBB( 3003), pts: 7487474764, dts: 7487471761
```

Show the every AU pts and dts with NTP time format and 90KHZ of MPU#3924003

## Dump one MPU

Can specify the option 'MPUseqno' with the specified value

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --MPUseqno=0x3BDFBB --output=29999_3BDFBB.hevc
```

It will dump all elementary stream raw data of MPU which sequence number is 0x3BDFBB, and save it to 29999_3BDFBB.hevc

## Dump multiple MPUs

From the command options 'listMPUtime', get the every MPU start packet sequence number

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --listMPUtime --start=0x3BE023 --end=0x3BE026
0, CID: 0x0001(    1), packet_id: 0xF100(61696):
      0, MPU(SeqNo: 0x3BE023(3924003), presentation_time: 3763428982.136444s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE802FFA6
      1, MPU(SeqNo: 0x3BE024(3924004), presentation_time: 3763428982.670314s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80304BC
      2, MPU(SeqNo: 0x3BE025(3924005), presentation_time: 3763428983.204182s, timescale: 180,000HZ, decoding_time_offset:  9010), start_pkt_seqno: 0xE80309D6
```

If want to dump the elementary stream of MPU#3924003 and 3924004, get the start packet_sequence_number(0xE802FFA6) and its end packet_sequence_number(0xE80309D6)

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --start=0xE802FFA6 --end=0xE80309D6 --output=29999_3BE023_3BE024.hevc
```

## List MMTP packets

```
DumpTS 29999.mmts --listMMTPpacket
[     Message] CID: 0x0001, pkt_id:0xFF01, PKTSeqNo:0x070F54CA, pkt_len:   856, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:0, timestamp:1900-01-01 06h:42m:05.000011s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE80101AB, pkt_len:   585, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:42m:05.000011s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE80101AC, pkt_len:   362, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:42m:05.000011s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE80101AD, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:42m:05.000011s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE80101AE, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:42m:05.000011s
.......
```

All MMTP packets will be displayed.

```
DumpTS.exe 29999.mmts --CID=1 --pid=0xF100 --listMMTPpacket --start=0xE802FFA6 --end=0xE80304BC
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFA6, pkt_len:   535, ver: 0, RAP: 1, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFA7, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFA8, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFA9, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFAA, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFAB, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFAC, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFAD, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFAE, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFAF, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB0, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB1, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB2, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB3, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB4, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB5, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB6, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB7, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB8, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFB9, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFBA, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
[Fragment MPU] CID: 0x0001, pkt_id:0xF100, PKTSeqNo:0xE802FFBB, pkt_len:  1443, ver: 0, RAP: 0, pcf:0, FEC:0, ext_f:1, timestamp:1900-01-01 06h:43m:01.000008s
```

List all MMTP packets in one MPU which sequence number is 3924003.

## List MMTP payloads

```
DumpTS 29999.mmts --listMMTPpayload
......
 PKTSeqNo:0xE80103E2,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E3,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E4,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E5,   MFU, tf: 1,   Tail, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:   929, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E6,   MFU, tf: 1,  1+ DU, aggr: 1, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:    21, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
                                                                                    Len:   280, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
                                                                                    Len:    26, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E7,   MFU, tf: 1, Header, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E8,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103E9,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103EA,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103EB,   MFU, tf: 1,   Tail, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBA, Len:  1045, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
*PKTSeqNo:0xE80103EC,   MFU, tf: 1,  1+ DU, aggr: 1, Fc: 000, MPUSeqNo: 0x003BDFBB, Len:    21, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
                                                                                    Len:    45, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
                                                                                    Len:   113, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
                                                                                    Len:   280, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
                                                                                    Len:    37, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103ED,   MFU, tf: 1, Header, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBB, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 PKTSeqNo:0xE80103EE,   MFU, tf: 1, Middle, aggr: 0, Fc: 000, MPUSeqNo: 0x003BDFBB, Len:  1414, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
 ......
```

list all MMTP payloads, and if the MMTP payload is a random access point(RAP), there is a '*' at the beginning of the list item, for example:

```
*PKTSeqNo:0xE80103EC,   MFU, tf: 1,  1+ DU, aggr: 1, Fc: 000, MPUSeqNo: 0x003BDFBB, Len:    21, MFSeqNo: 0x00000000, SampleNo: 0x00000000, Offset: 0x00000000, DepC: 0
```

## Show MMT/TLV packs

The option 'showpack' can be used to print all TLV, MMT data syntax

```
Dump 29999.mmts --showpack
-----------------------------------------------------------------------------------------
          file offset: 863
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: Header compressed IP packet
          Data_length: 588
           Context_id: 1
      Sequence_number: 5
          Header_type: 97 (0X61), No compressed header
    ++++++++++++++ MMTP Packet +++++++++++++
              file offset: 870
                  version: 0X0
      Packet_counter_flag: 0
                 FEC_type: Non-protected MMTP packet by AL-FEC
           extension_flag: 1
                 RAP_flag: 0
             Payload_type: 0, MPU, a media-aware fragment of the MPU
                Packet_id: 0XF100(61696)
       Delivery_timestamp: 1900-01-01 06h:42m:05.000011s(24125.720459s)
      Pkt_sequence_number: 3892380075
          Ext_header_type: 0
        Ext_header_length: 5
         header_ext_value: 80 01 00 01 E0
        ++++++++++++++++ MPU +++++++++++++
               Payload_length: 562
                Fragment_type: 2, MFU
                   timed_flag: 1
               frag_indicator: 3, Divided, Including the end part of the data before division
               Aggregate_flag: 0
             fragment_counter: 0
          MPU_sequence_number: 3923898
               DataUnit index: [#0]
                        DU_length: 556
                        MF_seq_no: 0
                        sample_no: 0
                           offset: 0
                         priority: 0
                      dep_counter: 0
                   MFU_data_bytes: 65 41 1A 56 D3 43 43 95 F0 9A D8 12 F4 AC 61 61
                                   BC FF 66 EB 1D F7 C8 7E 4F F8 47 D8 F1 BD C3 48
                                   5E AE 56 C2 09 C4 12 66 56 B7 AC 7A 2C EC 1E A0
                                   10 98 15 18 43 E8 1C BA 0D F1 C9 E7 8F 8F 80 BB
                                   21 D0 D2 CC 37 7C B1 4C AC AB 5A 42 66 51 37 C0
                                   48 3C 44 21 72 53 53 FD C9 71 18 FE E0 26 1A 52
                                   54 3F BB 6E 96 FA 4B 17 CA 34 A1 84 D7 38 46 A5
                                   3B 58 86 60 E0 DF 57 81 E6 CA 06 C7 E4 60 E5 94
                                   42 89 F8 E3 7D E7 AD DA D6 E0 AA 21 4F 15 E7 D4
                                   16 32 28 5F 91 8E CB 01 B1 73 72 1B AB A6 9B 87
                                   5D CC F2 61 A7 69 6D 93 C9 32 1B 1B 84 2D 3D CB
                                   24 74 B5 FE 3A 19 DA 9E D1 9B 3E 9E 90 1F 96 89
                                   8F 13 6A F7 2D F4 CE 20 E3 DE 3E 80 69 4A E7 41
                                   23 90 82 29 8F F6 BB FD C0 4E 48 E6 90 6D E1 B6
                                   03 25 3B B3 04 8C 1F 3C D2 93 B7 54 31 91 CD A5
                                   CE CB 52 3C 59 DB DA 9B 9F 6F B2 8E E9 C4 02 5B
                                   ...
-----------------------------------------------------------------------------------------
......
```

It may take very long time to show all data syntax, and you can use option 'start' and 'end' to specify the packet sequence number range, for example:

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --showpack --start=0xE80103EC --end=0xE80103ED
-----------------------------------------------------------------------------------------
          file offset: 814914
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: Header compressed IP packet
          Data_length: 538
           Context_id: 1
      Sequence_number: 12
          Header_type: 97 (0X61), No compressed header
    ++++++++++++++ MMTP Packet +++++++++++++
              file offset: 814921
                  version: 0X0
      Packet_counter_flag: 0
                 FEC_type: Non-protected MMTP packet by AL-FEC
           extension_flag: 1
                 RAP_flag: 1
             Payload_type: 0, MPU, a media-aware fragment of the MPU
                Packet_id: 0XF100(61696)
       Delivery_timestamp: 1900-01-01 06h:42m:05.000015s(24125.987289s)
      Pkt_sequence_number: 3892380652
          Ext_header_type: 0
        Ext_header_length: 5
         header_ext_value: 80 01 00 01 E0
        ++++++++++++++++ MPU +++++++++++++
               Payload_length: 512
                Fragment_type: 2, MFU
                   timed_flag: 1
               frag_indicator: 0, Undivided
               Aggregate_flag: 1
             fragment_counter: 0
          MPU_sequence_number: 3923899
               DataUnit index: [#0]
                        DU_length: 21
                        MF_seq_no: 0
                        sample_no: 0
                           offset: 0
                         priority: 0
                      dep_counter: 0
                   MFU_data_bytes: 00 00 00 03 46 01 10
               DataUnit index: [#1]
                        DU_length: 45
                        MF_seq_no: 0
                        sample_no: 0
                           offset: 0
                         priority: 0
                      dep_counter: 0
                   MFU_data_bytes: 00 00 00 1B 40 01 0C 06 FF FF 02 20 00 00 03 00
                                   B0 00 00 03 00 00 03 00 99 00 00 18 86 02 40
               DataUnit index: [#2]
                        DU_length: 113
                        MF_seq_no: 0
                        sample_no: 0
                           offset: 0
                         priority: 0
                      dep_counter: 0
                   MFU_data_bytes: 00 00 00 5F 42 01 06 02 20 00 00 03 00 B0 00 00
                                   03 00 00 03 00 99 00 00 A0 01 E0 20 02 1C 4D 8D
                                   18 86 92 42 96 53 80 A1 09 12 09 B2 94 00 00 0F
                                   A4 00 03 A9 82 00 FA 70 0A 05 2F 00 00 13 12 D0
                                   00 00 4C 5E C7 00 00 13 12 D0 00 00 4C 5E C7 00
                                   00 13 12 D0 00 00 4C 5E C7 00 00 13 12 D0 00 00
                                   4C 5E C1
               DataUnit index: [#3]
                        DU_length: 280
                        MF_seq_no: 0
                        sample_no: 0
                           offset: 0
                         priority: 0
                      dep_counter: 0
                   MFU_data_bytes: 00 00 01 06 44 01 C0 76 F0 2C 20 14 87 30 41 C2
                                   18 52 14 84 34 98 CE 38 60 D0 55 02 71 42 01 88
                                   63 08 71 98 C8 42 10 C9 8A E2 B0 E4 1D 14 21 FF
                                   FF E9 AD A4 EA EB A7 AA B9 12 6C 84 94 52 63 23
                                   21 0F FA 69 A4 D5 A3 15 26 C8 62 C9 2C 8A 43 1C
                                   B1 04 11 12 51 C2 8C 71 61 61 D1 E0 90 50 B0 5D
                                   06 C5 08 7F FF FA FD 7F FE BE BC 9C D8 88 A8 43
                                   FE BD 7C 9F 88 FE BF 37 C5 78 43 86 B0 45 05 10
                                   87 FD 35 A2 93 90 48 8F 10 11 08 7C 22 12 F9 BE
                                   2B C2 1C 35 82 28 28 84 3F FF FE 9A DA 4E AE BA
                                   7A AB 91 26 C8 49 45 26 32 32 10 FF D3 4D 26 AD
                                   18 A9 36 43 16 49 64 52 18 E5 88 20 88 92 8E 14
                                   63 8B 0B 0E 8F 04 82 85 82 E8 36 28 43 FF FF EB
                                   F5 FF FA FA F2 73 62 22 A1 0F FD 7A F9 3F 11 FD
                                   7E 6F 8A F0 87 0D 60 8A 0A 0A 10 FF FF FA 6B 69
                                   3A BA E9 EA AE 44 9B 21 25 14 98 C8 C8 43 FF FF
                                   ...
               DataUnit index: [#4]
                        DU_length: 37
                        MF_seq_no: 0
                        sample_no: 0
                           offset: 0
                         priority: 0
                      dep_counter: 0
                   MFU_data_bytes: 00 00 00 13 4E 01 00 06 83 AB FD 00 00 10 01 03
                                   04 F9 50 06 01 D0 80
```

Show the MMTP packet with CID and packet_id setting to 1 and 0xF100 separately, and its packet sequence number is 0xE80103EC. 

## Show IPv6 packs

```
DumpTS 00001.mmts --showIPv6pack
-----------------------------------------------------------------------------------------
          file offset: 40346
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: IPv6 packet
          Data_length: 96
              Version: 6
        Traffic class: 0
           Flow label: 0
       Payload length: 56
          Next header: 17, UDP, User Datagram
            Hop limit: 32
       Source address: 2001::1:1:0:1
         Dest address: ff02::101
          source port: 456
            dest port: 123
          Data length: 56
             Checksum: 10579(0X2953)
       leap_indicator: Without alarm
              version: 4
                 mode: broadcast
              stratum: 0, indefinite or invalid
                 poll: 0
            precision: 0
           root_delay:     0.000000s
      root_dispersion:     0.000000s
reference_identification: .... (0X00000000)
  reference_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
     origin_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
    receive_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
   transmit_timestamp: 3690620891.093905s, 2016-13-10 12h:28m:11.093906s
-----------------------------------------------------------------------------------------
          file offset: 152077
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: IPv6 packet
          Data_length: 96
              Version: 6
        Traffic class: 0
           Flow label: 0
       Payload length: 56
          Next header: 17, UDP, User Datagram
            Hop limit: 32
       Source address: 2001::1:1:0:1
         Dest address: ff02::101
          source port: 456
            dest port: 123
          Data length: 56
             Checksum: 14557(0X38DD)
       leap_indicator: Without alarm
              version: 4
                 mode: broadcast
              stratum: 0, indefinite or invalid
                 poll: 0
            precision: 0
           root_delay:     0.000000s
      root_dispersion:     0.000000s
reference_identification: .... (0X00000000)
  reference_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
     origin_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
    receive_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
   transmit_timestamp: 3690620891.126955s, 2016-13-10 12h:28m:11.126955s
-----------------------------------------------------------------------------------------
          file offset: 264605
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: IPv6 packet
          Data_length: 96
              Version: 6
        Traffic class: 0
           Flow label: 0
       Payload length: 56
          Next header: 17, UDP, User Datagram
            Hop limit: 32
       Source address: 2001::1:1:0:1
         Dest address: ff02::101
          source port: 456
            dest port: 123
          Data length: 56
             Checksum: 28519(0X6F67)
       leap_indicator: Without alarm
              version: 4
                 mode: broadcast
              stratum: 0, indefinite or invalid
                 poll: 0
            precision: 0
           root_delay:     0.000000s
      root_dispersion:     0.000000s
reference_identification: .... (0X00000000)
  reference_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
     origin_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
    receive_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
   transmit_timestamp: 3690620891.160002s, 2016-13-10 12h:28m:11.160002s
-----------------------------------------------------------------------------------------
          file offset: 372458
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: IPv6 packet
          Data_length: 96
              Version: 6
        Traffic class: 0
           Flow label: 0
       Payload length: 56
          Next header: 17, UDP, User Datagram
            Hop limit: 32
       Source address: 2001::1:1:0:1
         Dest address: ff02::101
          source port: 456
            dest port: 123
          Data length: 56
             Checksum: 55793(0XD9F1)
       leap_indicator: Without alarm
              version: 4
                 mode: broadcast
              stratum: 0, indefinite or invalid
                 poll: 0
            precision: 0
           root_delay:     0.000000s
      root_dispersion:     0.000000s
reference_identification: .... (0X00000000)
  reference_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
     origin_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
    receive_timestamp:          0.000000s, 1900-01-01 00h:00m:00s
   transmit_timestamp: 3690620891.193045s, 2016-13-10 12h:28m:11.193045s
-----------------------------------------------------------------------------------------
......
```

## Show Transmission Control Signal packets

```
DumpTS 00001.mmts --showTCSpack
-----------------------------------------------------------------------------------------
          file offset: 2417813
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: Transmission control signal packet
          Data_length: 92
             table_id: 0x40
section_syntax_indicator: 1
           reserved_0: 1
           reserved_1: 0x3
       section_length: 0x59(89)
   table_id_extension: 0x0B
           reserved_2: 0x03
       version_number: 1
current_next_indicator: 1
       section_number: 0x00(0)
  last_section_number: 0x00(0)
  reserved_future_use: 0xF
network_descriptors_length: 0x31(49)
   descriptor_tag/len: 0x40( 64)/0x1E( 30) -- Network Name Descriptor
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  E9  AB  98  E5  BA  A6  EF  BC    A2  EF  BC  B3  E3  83  87  E3 | ................
     000010  82  B8  E3  82  BF  E3  83  AB    E6  94  BE  E9                 | ............

   descriptor_tag/len: 0xFE(254)/0x02(  2) -- System Management Descriptor
        broadcasting_flag: 0x0
          broadcasting_id: 0x08(  8) // broadcasting_identifier
      ext_broadcasting_id: 0x01(  1) // additional_broadcasting_identification

   descriptor_tag/len: 0xCD(205)/0x0B( 11) -- Remote Control Key Descriptor
    num_of_remote_control_key_id: 2
        key_id/service_id[ 0]: 0X01/0x0001
        key_id/service_id[ 1]: 0XFF/0x0002

  reserved_future_use: 0xF
      2nd_loop_length: 0x1B(27) // TLV_stream_loop_length
            tlv_stream_id: 0xB110(45328)
      original_network_id: 0x000B(11)
      original_network_id: 0xF
       descriptors_length: 0x15(21) // tlv_stream_descriptors_length
       descriptor_tag/len: 0x41( 65)/0x06(  6) -- Service List Descriptor
                   service_id[00]: 0x1
                 service_type[00]: 0x1 -- Digital TV service
                   service_id[01]: 0x2
                 service_type[01]: 0x1 -- Digital TV service

       descriptor_tag/len: 0x43( 67)/0x0B( 11) -- Satellite Delivery System Descriptor
                    frequency: 012.03436GHZ
                    frequency: 110.0GHZ
               west_east_flag: 1
                 polarisation: 3 -- Clockwise rotation
                   modulation: 0xB(Advanced wide band satellite digital broadcasting system)
                  symbol_rate: 033.7561
                    FEC_inner: 0x1(Coding rate 1/2)

               CRC_32: 0x3340CC0C
-----------------------------------------------------------------------------------------
          file offset: 18892964
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: Transmission control signal packet
          Data_length: 90
             table_id: 0xFE
section_syntax_indicator: 1
           reserved_0: 1
           reserved_1: 0x3
       section_length: 0x57(87)
   table_id_extension: 0x00
           reserved_2: 0x03
       version_number: 5
current_next_indicator: 1
       section_number: 0x00(0)
  last_section_number: 0x00(0)
-----------------------------------------------------------------------------------------
          file offset: 35320443
          upper_2bits: 0X1
          upper_6bits: 0X3F
          Packet_type: Transmission control signal packet
          Data_length: 92
             table_id: 0x40
section_syntax_indicator: 1
           reserved_0: 1
           reserved_1: 0x3
       section_length: 0x59(89)
   table_id_extension: 0x0B
           reserved_2: 0x03
       version_number: 1
       ......
```

 ## Show the pts/dts of each AU

```
DumpTS 29999.mmts --CID=1 --pid=0xF100 --showpts
#0         , MPU/packet sequence number: 0x003BDFBA/0xE80101AC, Trailing, pts: N/A                    , dts: N/A                    .
#1         , MPU/packet sequence number: 0x003BDFBA/0xE80101BB, Trailing, pts: N/A                    , dts: N/A                    .
#2         , MPU/packet sequence number: 0x003BDFBA/0xE80101C4, Trailing, pts: N/A                    , dts: N/A                    .
#3         , MPU/packet sequence number: 0x003BDFBA/0xE80101CC, Trailing, pts: N/A                    , dts: N/A                    .
#4         , MPU/packet sequence number: 0x003BDFBA/0xE80101D2, Trailing, pts: N/A                    , dts: N/A                    .
#5         , MPU/packet sequence number: 0x003BDFBA/0xE80101D7, Trailing, pts: N/A                    , dts: N/A                    .
#6         , MPU/packet sequence number: 0x003BDFBA/0xE80101DC, Trailing, pts: N/A                    , dts: N/A                    .
#7         , MPU/packet sequence number: 0x003BDFBA/0xE8010284, Trailing, pts: N/A                    , dts: N/A                    .
#8         , MPU/packet sequence number: 0x003BDFBA/0xE80102F9, Trailing, pts: N/A                    , dts: N/A                    .
#9         , MPU/packet sequence number: 0x003BDFBA/0xE8010307, Trailing, pts: N/A                    , dts: N/A                    .
#10        , MPU/packet sequence number: 0x003BDFBA/0xE801030C, Trailing, pts: N/A                    , dts: N/A                    .
#11        , MPU/packet sequence number: 0x003BDFBA/0xE801031C, Trailing, pts: N/A                    , dts: N/A                    .
#12        , MPU/packet sequence number: 0x003BDFBA/0xE801032B, Trailing, pts: N/A                    , dts: N/A                    .
#13        , MPU/packet sequence number: 0x003BDFBA/0xE8010336, Trailing, pts: N/A                    , dts: N/A                    .
#14        , MPU/packet sequence number: 0x003BDFBA/0xE8010346, Trailing, pts: N/A                    , dts: N/A                    .
#15        , MPU/packet sequence number: 0x003BDFBA/0xE8010388, Trailing, pts: N/A                    , dts: N/A                    .
#16        , MPU/packet sequence number: 0x003BDFBA/0xE80103AF, Trailing, pts: N/A                    , dts: N/A                    .
#17        , MPU/packet sequence number: 0x003BDFBA/0xE80103BB, Trailing, pts: N/A                    , dts: N/A                    .
#18        , MPU/packet sequence number: 0x003BDFBA/0xE80103C6, Trailing, pts: N/A                    , dts: N/A                    .
#19        , MPU/packet sequence number: 0x003BDFBA/0xE80103D0, Trailing, pts: N/A                    , dts: N/A                    .
#20        , MPU/packet sequence number: 0x003BDFBA/0xE80103D8, Trailing, pts: N/A                    , dts: N/A                    .
#21        , MPU/packet sequence number: 0x003BDFBA/0xE80103E6, Trailing, pts: N/A                    , dts: N/A                    .
#22        , MPU/packet sequence number: 0x003BDFBB/0xE80103EC, CRA     , pts: 0x1BDFD05E6(7482443238), dts: 0x1BDFCCB3F(7482428223).
#23        , MPU/packet sequence number: 0x003BDFBB/0xE80105C8, Leading , pts: 0x1BDFCEE6F(7482437231), dts: 0x1BDFCD11C(7482429724).
#24        , MPU/packet sequence number: 0x003BDFBB/0xE80105D8, Leading , pts: 0x1BDFCE2B5(7482434229), dts: 0x1BDFCD6FA(7482431226).
#25        , MPU/packet sequence number: 0x003BDFBB/0xE80105E3, Leading , pts: 0x1BDFCDCD7(7482432727), dts: 0x1BDFCDCD7(7482432727).
#26        , MPU/packet sequence number: 0x003BDFBB/0xE80105EA, Leading , pts: 0x1BDFCE893(7482435731), dts: 0x1BDFCE2B5(7482434229).
#27        , MPU/packet sequence number: 0x003BDFBB/0xE80105F1, Leading , pts: 0x1BDFCFA2A(7482440234), dts: 0x1BDFCE892(7482435730).
#28        , MPU/packet sequence number: 0x003BDFBB/0xE80105F7, Leading , pts: 0x1BDFCF44E(7482438734), dts: 0x1BDFCEE70(7482437232).
#29        , MPU/packet sequence number: 0x003BDFBB/0xE80105FC, Leading , pts: 0x1BDFD0008(7482441736), dts: 0x1BDFCF44D(7482438733).
#30        , MPU/packet sequence number: 0x003BDFBB/0xE8010602, Trailing, pts: 0x1BDFD34D2(7482455250), dts: 0x1BDFCFA2B(7482440235).
#31        , MPU/packet sequence number: 0x003BDFBB/0xE8010695, Trailing, pts: 0x1BDFD1D5B(7482449243), dts: 0x1BDFD0008(7482441736).
#32        , MPU/packet sequence number: 0x003BDFBB/0xE80106AE, Trailing, pts: 0x1BDFD11A1(7482446241), dts: 0x1BDFD05E6(7482443238).
#33        , MPU/packet sequence number: 0x003BDFBB/0xE80106B5, Trailing, pts: 0x1BDFD0BC3(7482444739), dts: 0x1BDFD0BC3(7482444739).
#34        , MPU/packet sequence number: 0x003BDFBB/0xE80106BC, Trailing, pts: 0x1BDFD177F(7482447743), dts: 0x1BDFD11A1(7482446241).
#35        , MPU/packet sequence number: 0x003BDFBB/0xE80106C1, Trailing, pts: 0x1BDFD2916(7482452246), dts: 0x1BDFD177E(7482447742).
#36        , MPU/packet sequence number: 0x003BDFBB/0xE80106CD, Trailing, pts: 0x1BDFD233A(7482450746), dts: 0x1BDFD1D5C(7482449244).
#37        , MPU/packet sequence number: 0x003BDFBB/0xE80106D3, Trailing, pts: 0x1BDFD2EF4(7482453748), dts: 0x1BDFD2339(7482450745).
#38        , MPU/packet sequence number: 0x003BDFBB/0xE80106DB, Trailing, pts: 0x1BDFD63BE(7482467262), dts: 0x1BDFD2917(7482452247).
#39        , MPU/packet sequence number: 0x003BDFBB/0xE8010789, Trailing, pts: 0x1BDFD4C47(7482461255), dts: 0x1BDFD2EF4(7482453748).
#40        , MPU/packet sequence number: 0x003BDFBB/0xE80107F3, Trailing, pts: 0x1BDFD408D(7482458253), dts: 0x1BDFD34D2(7482455250).
#41        , MPU/packet sequence number: 0x003BDFBB/0xE8010804, Trailing, pts: 0x1BDFD3AAF(7482456751), dts: 0x1BDFD3AAF(7482456751).
#42        , MPU/packet sequence number: 0x003BDFBB/0xE8010814, Trailing, pts: 0x1BDFD466B(7482459755), dts: 0x1BDFD408D(7482458253).
#43        , MPU/packet sequence number: 0x003BDFBB/0xE801081E, Trailing, pts: 0x1BDFD5802(7482464258), dts: 0x1BDFD466A(7482459754).
```
## Show the MFU data
The option 'showDU' can be used to print the MFU data units
```
DumpTS 00001.mmts --CID=1 --pid=0xF300 --showDU
packet_sequence_number: 0x056E5D32, packet_id: 0xF300, MPU sequence number: 0x000E1D69:
    DataUnit#0(MFseqno: 0X00000000, sample_number: 0x00000000, offset: 0x00000000, priority: 0x00, DC: 0x00):
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  00  03  46  01  10                                       | ....F..

    DataUnit#1(MFseqno: 0X00000000, sample_number: 0x00000000, offset: 0x00000000, priority: 0x00, DC: 0x00):
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  00  1B  40  01  0C  06    FF  FF  02  20  00  00  03  00 | ....@...... ....
     000010  B0  00  00  03  00  00  03  00    99  00  00  18  82  40  90     | .............@.

    DataUnit#2(MFseqno: 0X00000000, sample_number: 0x00000000, offset: 0x00000000, priority: 0x00, DC: 0x00):
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  00  63  42  01  06  02    20  00  00  03  00  B0  00  00 | ...cB... .......
     000010  03  00  00  03  00  99  00  00    A0  01  E0  20  02  1C  4D  8D | ........... ..M.
     000020  18  82  64  90  A5  94  E0  28    42  43  82  6D  94  00  00  0F | ..d....(BC.m....
     000030  A4  00  03  A9  82  00  FA  70    02  F7  BF  00  00  13  12  D0 | .......p........
     000040  00  00  13  17  B2  F0  00  01    31  2D  00  00  03  01  31  7B | ........1-....1{
     000050  2F  00  00  13  12  D0  00  00    13  17  B2  F0  00  01  31  2D | /.............1-
     000060  00  00  03  01  31  7B  29                                       | ....1{)

    DataUnit#3(MFseqno: 0X00000000, sample_number: 0x00000000, offset: 0x00000000, priority: 0x00, DC: 0x00):
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  01  06  44  01  C0  76    F0  2C  20  14  87  30  41  C2 | ....D..v., ..0A.
     000010  18  52  14  84  34  98  CE  38    60  D0  55  02  71  42  01  88 | .R..4..8`.U.qB..
     000020  63  08  71  98  C8  42  10  C9    8A  E2  B0  E4  1D  14  21  FF | c.q..B........!.
     000030  FF  E9  AD  A4  EA  EB  A7  AA    B9  12  6C  84  94  52  63  23 | ..........l..Rc#
     000040  21  0F  FA  69  A4  D5  A3  15    26  C8  62  C9  2C  8A  43  1C | !..i....&.b.,.C.
     000050  B1  04  11  12  51  C2  8C  71    61  61  D1  E0  90  50  B0  5D | ....Q..qaa...P.]
     000060  06  C5  08  7F  FF  FA  FD  7F    FE  BE  BC  9C  D8  88  A8  43 | ...............C
     000070  FE  BD  7C  9F  88  FE  BF  37    C5  78  43  86  B0  45  05  10 | ..|....7.xC..E..
     000080  87  FD  35  A2  93  90  48  8F    10  11  08  7C  22  12  F9  BE | ..5...H....|"...
     000090  2B  C2  1C  35  82  28  28  84    3F  FF  FE  9A  DA  4E  AE  BA | +..5.((.?....N..
     0000A0  7A  AB  91  26  C8  49  45  26    32  32  10  FF  D3  4D  26  AD | z..&.IE&22...M&.
     0000B0  18  A9  36  43  16  49  64  52    18  E5  88  20  88  92  8E  14 | ..6C.IdR... ....
     0000C0  63  8B  0B  0E  8F  04  82  85    82  E8  36  28  43  FF  FF  EB | c.........6(C...
     0000D0  F5  FF  FA  FA  F2  73  62  22    A1  0F  FD  7A  F9  3F  11  FD | .....sb"...z.?..
     0000E0  7E  6F  8A  F0  87  0D  60  8A    0A  0A  10  FF  FF  FA  6B  69 | ~o....`.......ki
     0000F0  3A  BA  E9  EA  AE  44  9B  21    25  14  98  C8  C8  43  FF  FF | :....D.!%....C..
     000100  EB  F5  FF  FA  FA  F2  73  62    22  92                         | ......sb".
......
```

You can see the HEVC video stream NAL unit with length header is print.

If you want to only show the data unit of one packet, you may specify option PKTseqno, for example:

```
DumpTS 00001.mmts --CID=1 --pid=0xF310 --PKTseqno=0x1BD50E63 --showDU
packet_sequence_number: 0x1BD50E63, packet_id: 0xF310, MPU sequence number: 0x00575F67:
    DataUnit#0(MFseqno: 0X00000000, sample_number: 0x00000000, offset: 0x00000000, priority: 0x00, DC: 0x00):
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  20  00  11  90  0D  48  0F  FF    FC  F9  08  D4  A5  6F  E8  A1 |  ....H.......o..
     000010  28  51  CF  93  BE  07  07  F1    AB  35  E7  51  E6  46  73  89 | (Q.......5.Q.Fs.
     000020  C7  0E  A8  E4  3D  49  C5  A7    3A  9B  E7  E7  78  D2  A0  F2 | ....=I..:...x...
     000030  EE  D6  ED  AE  FE  22  B9  E4    B5  C4  9D  49  EB  A9  EB  11 | .....".....I....
     000040  13  93  26  59  C9  F7  2E  CB    D3  F5  66  4F  5F  7B  46  6D | ..&Y......fO_{Fm
     000050  D0  91  A5  21  31  3A  63  08    CA  9A  26  DB  13  28  22  D3 | ...!1:c...&..(".
     000060  13  65  06  71  E0  49  D3  2D    C8  F8  3B  F8  A5  92  DA  1E | .e.q.I.-..;.....
     000070  41  90  17  A6  97  1D  67  37    93  08  A6  50  28  5F  D0  62 | A.....g7...P(_.b
     000080  53  A2  18  90  3C  A3  6A  ED    7F  DE  C3  B3  9B  3D  11  FA | S...<.j......=..
     000090  CE  E6  48  DA  83  ED  FA  98    96  68  1B  0A  D2  2B  B2  75 | ..H......h...+.u
     0000A0  1D  CB  1A  B2  45  73  CE  C5    A9  83  94  DA  8E  F2  A9  10 | ....Es..........
     0000B0  0B  37  3B  36  DD  39  36  0C    0A  28  79  AF  B0  F2  3F  18 | .7;6.96..(y...?.
     0000C0  4A  02  A4  B1  2C  35  F0  95    DD  66  1F  2F  6A  FC  D8  6C | J...,5...f./j..l
     0000D0  F0  3C  1B  A8  EB  E5  2B  B0    5A  0C  67  F9  94  39  37  77 | .<....+.Z.g..97w
     0000E0  B5  65  51  6B  CD  13  37  5C    19  6B  6D  89  FE  5A  98  7E | .eQk..7\.km..Z.~
     0000F0  57  01  FB  A2  1D  8D  D7  FA    1F  06  0C  8B  FA  1F  6B  E7 | W.............k.
     000100  CE  61  C8  22  EA  4F  62  E9    05  AB  30  7A  8B  9C  71  7E | .a.".Ob...0z..q~
     000110  BB  7F  EB  79  05  73  8A  79    BE  9C  FC  EF  AD  4D  6E  35 | ...y.s.y.....Mn5
     000120  F5  8D  00  BC  71  4B  B0  C8    D3  DC  AC  05  47  55  39  8B | ....qK......GU9.
     000130  6E  4B  14  62  C8  A6  D7  FA    CF  CA  30  60  92  18  3F  F2 | nK.b......0`..?.
     000140  F6  2B  28  88  C3  16  9C  BF    E4  BA  DC  A4  8A  2C  33  4A | .+(..........,3J
     000150  38  0E  66  6F  25  9B  E1  29    21  53  4D  44  D8  38  50  D4 | 8.fo%..)!SMD.8P.
     000160  A5  EA  B6  E1  61  55  4E  75    DC  A3  AC  F3  04  71  9A  E9 | ....aUNu.....q..
     000170  D8  43  B6  E1  9C  78  ED  3C    DF  89  E2  5F  80  71  34  70 | .C...x.<..._.q4p
     000180  C7  26  E5  74  2C  BE  2D  B1    64  A2  F4  D6  25  8B  68  CA | .&.t,.-.d...%.h.
     000190  63  DC  9F  83  B5  D3  76  53    FD  1B  B3  31  E1  3A  82  94 | c.....vS...1.:..
     0001A0  70  7A  4D  F3  54  38  96  2A    00  4C  75  90  89  08  5C  9D | pzM.T8.*.Lu...\.
     0001B0  12  AD  81  D6  FC  EB  19  61    56  51  EE  43  69  1F  FC  6A | .......aVQ.Ci..j
     0001C0  E5  8F  FD  A3  FC  EC  F1  36    61  DE  8B  E2  9F  3D  98  FB | .......6a....=..
     0001D0  58  6B  B4  25  5A  F5  44  C0    F9  CD  84  A7  FB  4B  EE  9A | Xk.%Z.D......K..
     0001E0  4A  B1  07  76  5F  02  F5  4E    56  06  93  0C  C9  6A  EF  DD | J..v_..NV....j..
     0001F0  8F  9F  38  C7  3F  2E  71  00    B8  0F  AC  62  B0  00  80  07 | ..8.?.q....b....
     000200  BC  C0  1C  AA  01  87  00  DF    1E  21  4B  4B  4B  4B  4B  4B | .........!KKKKKK
     000210  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000220  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000230  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000240  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000250  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000260  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000270  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000280  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     000290  4B  4B  4B  4B  4B  4B  4B  4B    4B  4B  4B  4B  4B  4B  4B  4B | KKKKKKKKKKKKKKKK
     0002A0  4B  4B  4B  4B  4B  4B  F8                                       | KKKKKK.
```





