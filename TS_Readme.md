# TS/TTS/M2TS Operation Guideline

## Extract a elementary stream
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

    ES_PID: 0x100, stream_type: 0x02 -- (Rec. ITU-T H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream):
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

    ES_PID: 0x100, stream_type: 0x02 -- (Rec. ITU-T H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream):
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

    ES_PID: 0x100, stream_type: 0x02 -- (Rec. ITU-T H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream):
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

    
