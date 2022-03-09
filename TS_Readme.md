# TS/TTS/M2TS Operation Guideline

* [Extract an elementary stream](#extract-an-elementary-stream)
* [Extract a PSI data stream](#extract-a-psi-data-stream)
* [Extract a sub-stream from one elementary stream](#extract-a-sub-stream-from-one-elementary-stream)
* [Extract a part of transport stream](#extract-a-part-of-transport-stream)
* [Show PSI information](#show-psi-information)
  * [Show PAT](#show-pai)
  * [Show PMT](#show-pmt)
  * [Show SIT](#show-sit)

## Extract an elementary stream
1. Get the transport stream rough information
    ```
    DumpTS 00023.m2ts --showinfo
    ```
    The elementary streams are listed:
    ```
    Program Number: 1, program_map_PID: 0X100(256).
    Program(PID:0X0100)
            Stream#0, PID: 0X1011, stm_type: 0X02 (MPEG2 Video)
            Stream#1, PID: 0X1100, stm_type: 0X83 (Dolby Lossless Audio (TrueHD/Atmos))
            Stream#2, PID: 0X1200, stm_type: 0X90 (PGS)
    ```
2. Get the PID of extracted stream
    For example, the video PID is 0x1011, and the audio is 0x1100, and the PG subtitle is 0x1200
3. Extract the stream
    Extract the video:
    ```
    DumpTS 00023.m2ts --pid=0x1011 --output=00023.m2v
    ```
    An elementary MPEG2 video stream is extracted to 00023.m2v, and it can be played by some media player, for example, VLC player, of course the PES stream with pts and dts can also be extracted by specifying the 'outputfmt', and then it can be used for multiplex;
    ```
    DumpTS 00023.m2ts --pid=0x1011 --output=00023.pes --outputfmt=pes
    ```
    On the other hand, the ts/tts/m2ts packs with the specified PID or PIDs can also be extracted to ts/tts/m2ts packs with outputfmt to 'ts/tts/m2ts':
    ```
    DumpTS 00023.m2ts --pid=0x1011 --output=00023.ts --outputfmt=ts
    ```
    But this ts file can't be played because there only exists the pack with PID '0x10111', there is no PAT or PMT

## Extract a PSI data stream
PSI data can also be extracted when specifying the corresponding PID,
```
DumpTs 00024.m2ts --pid=0X0 --output=00024.psi
```
The PSI data will start from 'pointer_field', and then follow with PSI section data

## Extract a sub-stream from one elementary stream
Some audio stream may have multiple sub-stream, for example, Dolby-TrueHD, the sub-streams can be extracted separately by adding stream_id_extension filter
1. Get the audio information
    ```
    DumpTS 00024.m2ts --showinfo --pid=0x1100
    ```
    All sub-streams will be listed,
    ```
    Dolby Lossless Audio (TrueHD/Atmos) Stream information(AC3 part):
            PID: 0X1100.
            Stream ID: 0XFD
            Stream Type: 131(0X83).
            Stream ID Extension: 0X76
            Sample Frequency: 48000 (HZ).
            Bits Per Sample: 16.
            Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
            Bitrate: 640.000 kbps.
    Dolby Lossless Audio (TrueHD/Atmos) Stream information(MLP part):
            PID: 0X1100.
            Stream ID: 0XFD
            Stream Type: 131(0X83).
            Stream ID Extension: 0X72
            Sample Frequency: 192000 (HZ).
            Bits Per Sample: 24.
            Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
    ```
2. Select the sub-stream with PID and stream_id_extension
    For example, select the AC3 sub-stream to extract with PID=0x1100 and stream_id_extension=0x76
3. Extract the AC3 sub-stream
    ```
    DumpTs 00024.m2ts --pid=0x1100 --stream_id_extension=0x76 --output=00024.ac3
    ```
## Extract a part of transport stream
When output format is set to ts/tts/m2ts, and source file is also a transport stream, you can specify multiple PIDs in --pid delimited with one of ,;:., and then related transport packets will be extracted
```
DumpTS Mono_AAC_test.m2ts --pid=0x0,0x1f,0x100,0x110,0x1f0 --outputfmt=m2ts --output=Mono_AAC_partial.m2ts
```
The transport packet with PID 0, 0x1F, 0x100, 0x110 and 0x1F0,will extracted consequently, and save them to new file.
*(\*)No PSI sections will be modified in this case, it may cause some players with more strict check can't play it*

## Show PSI information
### Show PAT
```
DumpTs Mono_AAC_test.m2ts --showPAT
PAT(ver: 0):
    program number: 0x0000(00000), Network_PID:0x1F
    program number: 0x0065(00101), program_map_PID:0x1F0
```
### Show PMT
```
DumpTs mono_dualmono.m2ts --showPMT
```
All elementary streams and their descriptors are be shown like as
```
PMT(ver: 3):
    PCR_PID: 0x1FF
    descriptor_tag/descriptor_length: 0X88/  4 - Unsupported descriptor

    descriptor_tag/descriptor_length: 0XC1/  1 - Digital copy control descriptor
        Digital copy control information: 2 -- Copy can be made for only one generation

    ES_PID: 0x100, stream_type: 0x02 -- (MPEG-2 Video or MPEG-1 constrained parameter video stream):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x00

            descriptor_tag/descriptor_length: 0XC8/  1 - Video decode control descriptor
                    still_picture_flag: 0
                sequence_end_code_flag: 1
                   video_encode_format: 1

    ES_PID: 0x110, stream_type: 0x0F -- (ISO/IEC 13818-7 Audio with ADTS transport syntax):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x10

    ES_PID: 0x138, stream_type: 0x06 -- (Rec. ITU-T H.222.0 | ISO/IEC 13818-1 PES packets containing private data):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x38

            descriptor_tag/descriptor_length: 0XFD/  3 - Data component descriptor

PMT(ver: 4):
    PCR_PID: 0x1FF
    descriptor_tag/descriptor_length: 0X88/  4 - Unsupported descriptor

    descriptor_tag/descriptor_length: 0XC1/  1 - Digital copy control descriptor
        Digital copy control information: 2 -- Copy can be made for only one generation

    ES_PID: 0x100, stream_type: 0x02 -- (MPEG-2 Video or MPEG-1 constrained parameter video stream):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x00

            descriptor_tag/descriptor_length: 0XC8/  1 - Video decode control descriptor
                    still_picture_flag: 0
                sequence_end_code_flag: 1
                   video_encode_format: 1

    ES_PID: 0x110, stream_type: 0x0F -- (ISO/IEC 13818-7 Audio with ADTS transport syntax):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x10

    ES_PID: 0x130, stream_type: 0x06 -- (Rec. ITU-T H.222.0 | ISO/IEC 13818-1 PES packets containing private data):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x30

            descriptor_tag/descriptor_length: 0XFD/  3 - Data component descriptor

    ES_PID: 0x138, stream_type: 0x06 -- (Rec. ITU-T H.222.0 | ISO/IEC 13818-1 PES packets containing private data):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x38

            descriptor_tag/descriptor_length: 0XFD/  3 - Data component descriptor

PMT(ver: 5):
    PCR_PID: 0x1FF
    descriptor_tag/descriptor_length: 0X88/  4 - Unsupported descriptor

    descriptor_tag/descriptor_length: 0XC1/  1 - Digital copy control descriptor
        Digital copy control information: 2 -- Copy can be made for only one generation

    ES_PID: 0x100, stream_type: 0x02 -- (MPEG-2 Video or MPEG-1 constrained parameter video stream):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x00

            descriptor_tag/descriptor_length: 0XC8/  1 - Video decode control descriptor
                    still_picture_flag: 0
                sequence_end_code_flag: 1
                   video_encode_format: 1

    ES_PID: 0x110, stream_type: 0x0F -- (ISO/IEC 13818-7 Audio with ADTS transport syntax):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x10

    ES_PID: 0x111, stream_type: 0x0F -- (ISO/IEC 13818-7 Audio with ADTS transport syntax):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x11

    ES_PID: 0x130, stream_type: 0x06 -- (Rec. ITU-T H.222.0 | ISO/IEC 13818-1 PES packets containing private data):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x30

            descriptor_tag/descriptor_length: 0XFD/  3 - Data component descriptor

    ES_PID: 0x138, stream_type: 0x06 -- (Rec. ITU-T H.222.0 | ISO/IEC 13818-1 PES packets containing private data):
            descriptor_tag/descriptor_length: 0X52/  1 - Stream identifier descriptor
                component_tag: 0x38

            descriptor_tag/descriptor_length: 0XFD/  3 - Data component descriptor

```
### Show SIT
```
DumpTs DualMono_AAC_test.m2ts --showSIT
```
The descriptors will be shown as
```
SIT(ver: 4):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 179 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 42):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  3B  7E  4F  40  38  78  4F  40    21  56  3F  4D  38  22  C7  3E | ;~O@8xO@!V?M8".>
             000010  57  46  4D  B9  EB  4A  46  43    66  21  21  46  7C  4B  5C  34 | WFM..JFCf!!F|K\4
             000020  6B  36  48  CE  36  6C  47  3A    21  57                         | k6H.6lG:!W
                text(length: 132):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  3F  37  61  45  1B  7C  A6  A4    B0  EB  3C  2B  3C  23  36  68 | ?7aE.|....<+<#6h
             000010  1B  7D  CA  C9  3F  4D  38  22    4C  64  42  6A  C7  CE  4A  46 | .}..?M8"LdBj..JF
             000020  43  66  42  50  4E  29  AC  3F    3C  DE  EB  43  66  21  22  CF | CfBPN).?<..Cf!".
             000030  B6  DE  CB  4E  29  C4  46  7C    4B  5C  34  6B  36  48  AC  38 | ...N).F|K\4k6H.8
             000040  37  B7  A4  4E  29  3E  6C  CB    21  23  4A  46  40  47  34  58 | 7..N)>l.!#JF@G4X
             000050  CE  1B  7C  E6  CB  AF  ED  40    3D  49  4A  4D  22  46  7E  3A | ..|....@=IJM"F~:
             000060  39  1B  7D  B7  3B  5F  E1  CA    C9  21  22  46  7C  4B  5C  34 | 9.}.;_...!"F|K\4
             000070  6B  36  48  CE  38  3D  3E  75    C8  42  50  31  7E  F2  39  4D | k6H.8=>u.BP1~.9M
             000080  A8  EB  21  23                                                   | ..!#

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/  9 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x1 -- 1/0 mode (single monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x0 --
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0X54/  6 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 6
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 05:50:00
                duration: 00:10:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 0
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 05:59:54

            descriptor_tag/descriptor_length: 0X4E/ 49 - extended_event_descriptor
                descriptor_number: 0x0
                last_descriptor_number: 0x1
                ISO_639_language_code: jpn
                item#0_description(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  48  56  41  48  46  62  4D  46                                   | HVAHFbMF
                item#0(length: 33):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  21  5A  3D  50  31  69  21  5B    1B  7E  CE  C8  CB  32  72  40 | !Z=P1i![.~...2r@
             000010  62  30  51  30  77  21  44  3F    40  3B  52  45  44  3E  4F  47 | b0Q0w!D?@;RED>OG
             000020  6E                                                               | n

            descriptor_tag/descriptor_length: 0X4E/ 47 - extended_event_descriptor
                descriptor_number: 0x1
                last_descriptor_number: 0x1
                ISO_639_language_code: jpn
                item#0_description(length: 6):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  3D  50  31  69  3C  54                                           | =P1i<T
                item#0(length: 33):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  21  5A  3D  50  31  69  21  5B    1B  7E  CE  C8  CB  32  72  40 | !Z=P1i![.~...2r@
             000010  62  30  51  30  77  21  44  3F    40  3B  52  45  44  3E  4F  47 | b0Q0w!D?@;RED>OG
             000020  6E                                                               | n



SIT(ver: 5):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 137 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 20):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7C  EF  21  3C  EB  C9  CB    E5  21  3C  B9  1B  24  2A  3B | .|.!<....!<..$*;
             000010  1B  7D  FA  DA                                                   | .}..
                text(length: 112):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  22  26  1B  7C  A4  AE  EA  B9    21  26  1B  7E  C2  C2  C3  AC | "&.|....!&.~....
             000010  1B  7C  C9  A4  C4  21  26  1B    7E  DA  C4  C6  AC  1B  7C  ED | .|...!&.~.....|.
             000020  B7  A2  21  26  ED  B7  A2  1B    7E  D4  D6  AC  1B  7C  AB  BF | ..!&....~....|..
             000030  21  3C  EB  21  26  A2  EB  B8    E3  B8  21  3C  E9  1B  7E  AC | !<.!&.....!<..~.
             000040  1B  7C  AA  21  3C  B9  C8  E9    EA  A2  21  26  1B  7E  C1  C2 | .|.!<.....!&.~..
             000050  C3  AC  1B  7C  A4  F3  C9  21    26  1B  7E  CE  C4  D4  D6  AC | ...|...!&.~.....
             000060  1B  7C  B7  F3  AC  DD  21  3C    EB  21  26  1B  7E  C3  CE  C1 | .|....!<.!&.~...

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/ 25 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x2 -- 1/0 + 1/0 mode (dual monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x1 -- Dual-Mono
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn
                ISO_639_language_code_2: etc
                text(length: 13):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  46  7C  4B  5C  38  6C  0D  33    30  39  71  38  6C             | F|K\8l.309q8l

            descriptor_tag/descriptor_length: 0X54/  2 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 5
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 06:00:00
                duration: 00:50:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 1
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 06:00:04



SIT(ver: 6):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 137 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 20):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7C  EF  21  3C  EB  C9  CB    E5  21  3C  B9  1B  24  2A  3B | .|.!<....!<..$*;
             000010  1B  7D  FA  DA                                                   | .}..
                text(length: 112):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  22  26  1B  7C  A4  AE  EA  B9    21  26  1B  7E  C2  C2  C3  AC | "&.|....!&.~....
             000010  1B  7C  C9  A4  C4  21  26  1B    7E  DA  C4  C6  AC  1B  7C  ED | .|...!&.~.....|.
             000020  B7  A2  21  26  ED  B7  A2  1B    7E  D4  D6  AC  1B  7C  AB  BF | ..!&....~....|..
             000030  21  3C  EB  21  26  A2  EB  B8    E3  B8  21  3C  E9  1B  7E  AC | !<.!&.....!<..~.
             000040  1B  7C  AA  21  3C  B9  C8  E9    EA  A2  21  26  1B  7E  C1  C2 | .|.!<.....!&.~..
             000050  C3  AC  1B  7C  A4  F3  C9  21    26  1B  7E  CE  C4  D4  D6  AC | ...|...!&.~.....
             000060  1B  7C  B7  F3  AC  DD  21  3C    EB  21  26  1B  7E  C3  CE  C1 | .|....!<.!&.~...

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/ 25 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x2 -- 1/0 + 1/0 mode (dual monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x1 -- Dual-Mono
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn
                ISO_639_language_code_2: etc
                text(length: 13):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  46  7C  4B  5C  38  6C  0D  33    30  39  71  38  6C             | F|K\8l.309q8l

            descriptor_tag/descriptor_length: 0X54/  2 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 5
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 06:00:00
                duration: 00:50:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 0
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 06:00:14



SIT(ver: 7):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 137 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 20):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7C  EF  21  3C  EB  C9  CB    E5  21  3C  B9  1B  24  2A  3B | .|.!<....!<..$*;
             000010  1B  7D  FA  DA                                                   | .}..
                text(length: 112):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  22  26  1B  7C  A4  AE  EA  B9    21  26  1B  7E  C2  C2  C3  AC | "&.|....!&.~....
             000010  1B  7C  C9  A4  C4  21  26  1B    7E  DA  C4  C6  AC  1B  7C  ED | .|...!&.~.....|.
             000020  B7  A2  21  26  ED  B7  A2  1B    7E  D4  D6  AC  1B  7C  AB  BF | ..!&....~....|..
             000030  21  3C  EB  21  26  A2  EB  B8    E3  B8  21  3C  E9  1B  7E  AC | !<.!&.....!<..~.
             000040  1B  7C  AA  21  3C  B9  C8  E9    EA  A2  21  26  1B  7E  C1  C2 | .|.!<.....!&.~..
             000050  C3  AC  1B  7C  A4  F3  C9  21    26  1B  7E  CE  C4  D4  D6  AC | ...|...!&.~.....
             000060  1B  7C  B7  F3  AC  DD  21  3C    EB  21  26  1B  7E  C3  CE  C1 | .|....!<.!&.~...

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/ 25 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x2 -- 1/0 + 1/0 mode (dual monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x1 -- Dual-Mono
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn
                ISO_639_language_code_2: etc
                text(length: 13):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  46  7C  4B  5C  38  6C  0D  33    30  39  71  38  6C             | F|K\8l.309q8l

            descriptor_tag/descriptor_length: 0X54/  2 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 5
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 06:00:00
                duration: 00:50:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 0
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 06:00:24



SIT(ver: 8):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 137 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 20):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7C  EF  21  3C  EB  C9  CB    E5  21  3C  B9  1B  24  2A  3B | .|.!<....!<..$*;
             000010  1B  7D  FA  DA                                                   | .}..
                text(length: 112):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  22  26  1B  7C  A4  AE  EA  B9    21  26  1B  7E  C2  C2  C3  AC | "&.|....!&.~....
             000010  1B  7C  C9  A4  C4  21  26  1B    7E  DA  C4  C6  AC  1B  7C  ED | .|...!&.~.....|.
             000020  B7  A2  21  26  ED  B7  A2  1B    7E  D4  D6  AC  1B  7C  AB  BF | ..!&....~....|..
             000030  21  3C  EB  21  26  A2  EB  B8    E3  B8  21  3C  E9  1B  7E  AC | !<.!&.....!<..~.
             000040  1B  7C  AA  21  3C  B9  C8  E9    EA  A2  21  26  1B  7E  C1  C2 | .|.!<.....!&.~..
             000050  C3  AC  1B  7C  A4  F3  C9  21    26  1B  7E  CE  C4  D4  D6  AC | ...|...!&.~.....
             000060  1B  7C  B7  F3  AC  DD  21  3C    EB  21  26  1B  7E  C3  CE  C1 | .|....!<.!&.~...

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/ 25 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x2 -- 1/0 + 1/0 mode (dual monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x1 -- Dual-Mono
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn
                ISO_639_language_code_2: etc
                text(length: 13):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  46  7C  4B  5C  38  6C  0D  33    30  39  71  38  6C             | F|K\8l.309q8l

            descriptor_tag/descriptor_length: 0X54/  2 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 5
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 06:00:00
                duration: 00:50:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 0
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 06:00:34



SIT(ver: 9):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 137 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 20):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7C  EF  21  3C  EB  C9  CB    E5  21  3C  B9  1B  24  2A  3B | .|.!<....!<..$*;
             000010  1B  7D  FA  DA                                                   | .}..
                text(length: 112):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  22  26  1B  7C  A4  AE  EA  B9    21  26  1B  7E  C2  C2  C3  AC | "&.|....!&.~....
             000010  1B  7C  C9  A4  C4  21  26  1B    7E  DA  C4  C6  AC  1B  7C  ED | .|...!&.~.....|.
             000020  B7  A2  21  26  ED  B7  A2  1B    7E  D4  D6  AC  1B  7C  AB  BF | ..!&....~....|..
             000030  21  3C  EB  21  26  A2  EB  B8    E3  B8  21  3C  E9  1B  7E  AC | !<.!&.....!<..~.
             000040  1B  7C  AA  21  3C  B9  C8  E9    EA  A2  21  26  1B  7E  C1  C2 | .|.!<.....!&.~..
             000050  C3  AC  1B  7C  A4  F3  C9  21    26  1B  7E  CE  C4  D4  D6  AC | ...|...!&.~.....
             000060  1B  7C  B7  F3  AC  DD  21  3C    EB  21  26  1B  7E  C3  CE  C1 | .|....!<.!&.~...

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/ 25 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x2 -- 1/0 + 1/0 mode (dual monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x1 -- Dual-Mono
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn
                ISO_639_language_code_2: etc
                text(length: 13):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  46  7C  4B  5C  38  6C  0D  33    30  39  71  38  6C             | F|K\8l.309q8l

            descriptor_tag/descriptor_length: 0X54/  2 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 5
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 06:00:00
                duration: 00:50:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 0
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 06:00:44



SIT(ver: 10):
    descriptor_tag/descriptor_length: 0X63/  8 - partial_transport_stream_descriptor
        peak_rate: 67500(27000000bps/27Mbps)
        minimum_overall_smoothing_rate: undefined
        maximum_overall_smoothing_buffer: undefined

    descriptor_tag/descriptor_length: 0XC2/  7 - Network identification descriptor
        Country Code: JPN
        Media Type: 0X4253(BS/broadband CS)
        Network ID: 0X4

    service_id: 0X65, running_state: 0
            descriptor_tag/descriptor_length: 0X48/ 16 - service_descriptor
                service_type: 0x01 - Digital TV service
                service_provider_name(length: 5):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB                                               | .~...
                service_name(length: 8):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7E  CE  C8  CB  C2  D3  B1                                   | .~......

            descriptor_tag/descriptor_length: 0X4D/ 137 - short_event_descriptor
                ISO_639_language_code: jpn
                event name(length: 20):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  1B  7C  EF  21  3C  EB  C9  CB    E5  21  3C  B9  1B  24  2A  3B | .|.!<....!<..$*;
             000010  1B  7D  FA  DA                                                   | .}..
                text(length: 112):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  22  26  1B  7C  A4  AE  EA  B9    21  26  1B  7E  C2  C2  C3  AC | "&.|....!&.~....
             000010  1B  7C  C9  A4  C4  21  26  1B    7E  DA  C4  C6  AC  1B  7C  ED | .|...!&.~.....|.
             000020  B7  A2  21  26  ED  B7  A2  1B    7E  D4  D6  AC  1B  7C  AB  BF | ..!&....~....|..
             000030  21  3C  EB  21  26  A2  EB  B8    E3  B8  21  3C  E9  1B  7E  AC | !<.!&.....!<..~.
             000040  1B  7C  AA  21  3C  B9  C8  E9    EA  A2  21  26  1B  7E  C1  C2 | .|.!<.....!&.~..
             000050  C3  AC  1B  7C  A4  F3  C9  21    26  1B  7E  CE  C4  D4  D6  AC | ...|...!&.~.....
             000060  1B  7C  B7  F3  AC  DD  21  3C    EB  21  26  1B  7E  C3  CE  C1 | .|....!<.!&.~...

            descriptor_tag/descriptor_length: 0X50/  6 - component_descriptor
                stream_content: 0x1
                component_type: 0xB3
                component_tag: 0
                ISO_639_language_code: jpn

            descriptor_tag/descriptor_length: 0XC4/ 25 - Audio component descriptor
                stream_content: 0x2
                component_type: 0x2 -- 1/0 + 1/0 mode (dual monaural channel)
                component_tag: 0x10
                stream_type: 0xf
                simulcast_group_tag: 0xf
                ES_multi_lingual_flag: 0x1 -- Dual-Mono
                main_component_flag: 0x1
                quality_indicator: 0x2 -- Mode 2
                sampling_rate: 0x7 -- 48kHz
                ISO_639_language_code: jpn
                ISO_639_language_code_2: etc
                text(length: 13):
                     00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                     ----------------------------------------------------------------
             000000  46  7C  4B  5C  38  6C  0D  33    30  39  71  38  6C             | F|K\8l.309q8l

            descriptor_tag/descriptor_length: 0X54/  2 - content_descriptor
                 content_nibble_level_1: 0
                 content_nibble_level_2: 5
                 user_nibble: 15
                 user_nibble: 15

            descriptor_tag/descriptor_length: 0XD6/  9 - Event group descriptor

            descriptor_tag/descriptor_length: 0XC3/ 18 - Partial Transport Stream time descriptor
                event_start_time Date(yyyy-mm-dd): 2021-5-25
                event_start_time Time(HH:MM:SS): 06:00:00
                duration: 00:50:00
                offset: 00:00:00
                offset_flag: 0
                other_descriptor_status: 0
                JST_time_flag: 1
                JST_time Date(yyyy-mm-dd): 2021-5-25
                JST_time Time(HH:MM:SS): 06:00:54
```
    
