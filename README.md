# Contents
* [Terminology](#terminology)
* [What is DumpTS?](#what-is-dumpts)
* [How to build?](#how-to-build)
* [How to run it?](#how-to-run-it)
* [Operations Manual](#operations-manual)
* [References](#References)

# Terminology
- **TS**: transport stream, the file extension is normally *.ts, .tts, .m2ts, .mts*
- **ISOBMFF**: ISO-based Media File Format, the file extension is normally *.mp4, .mov, m4a, m4v, .m4s, .heif, .heic, .avif*
- **Matroska**: a multimedia container format based on EBML (Extensible Binary Meta Language), the file extension is normally *.mkv, .mka, .mk3d and .webm*
- **MMT**: MPEG Media Transport stream, the file extension is normally *.mmts* 
- **PS**: MPEG program stream, the file extension is normally *.vob, .vro, .mpg, .mpeg* 
- **NAL**: Network Abstract Layer stream, the file extension is normally *.h264, .avc, .h265, .hevc, h266 and .vvc*
- **MPV**: MPEG video stream, the file extension is normally *.mpv, .m2v, .mp2v, .m1v* 
- **AV1**: AV1 Open Bitstream with low-overhead or length-delimited(defined in Annex B of AV1) bitstream format, the file extension is normally *.obu,.av1* 
- **ADTS** The elementary stream exported from MPEG2-AAC stream which is packetized in TS stream, the file extension is normally *.adts*
- **LOAS/LATM** The elementary stream exported from MPEG4-AAC stream which is packetized in TS or MMT/TLV stream, the file extension is normally .loas(with sync layer) or .latm, normally exported MPEG4-AAC as *.loas*

[Top](#contents)

# What is DumpTS?
DumpTS is a simple utility tool to process the multimedia files packed into main-stream multimedia container formats, which provides these kinds of features:

- Extract and repack the elementary stream data or PSI sections data from *TS, ISOBMFF, Matroska and MMT/TLV* file
- List the primitives hierarchical elements or meta w or w/o filter, and show the syntax, semantic, payload hex-view, bit offset and count, and so on, then assist to look inside the media essential of any given media file
- Show media information of elementary streams(AC3/DD+/MLP/DTS/DTS-HD/MPEG2-AAC/MPEG-4 AAC/MPEG2 Video/H.264/H.265/AV1...), *ISOBMFF* box, *Matroska EBML* element and *MMT/TLV* and *TS* packet/message/table/descriptors, container layout, ...
- Re-factor a *TS* stream file in place
- Extract some elementary streams, and reconstruct a partial *TS* file
- Show decoder HRD model information for any codec stream, ATC/PCR/pts/dts and so on, and do the related verification
- Provide some utility features for *ISOBMFF* file reconstruction
- Provide some utility functions for codec and container technology, for example, Huffman Codebook, CRC, container layout...

[Top](#contents)

# How to build?

- Windows<br>
    1. VS2015/vc14<br>
        Open build/DumpTS_vc14.sln and build it<br>
        [![DumpTS Build](https://github.com/wangf1978/DumpTS/workflows/MSBuild_vc14/badge.svg)](https://github.com/wangf1978/DumpTS/actions)       
    2. VS2017/vc15<br>
        Open build/DumpTS_vc15.sln and build it<br>
        [![DumpTS Build](https://github.com/wangf1978/DumpTS/workflows/MSBuild_vc15/badge.svg)](https://github.com/wangf1978/DumpTS/actions)
    3. VS2019/vc16<br>
        Open build/DumpTS_vc16.sln and build it<br>
        [![DumpTS Build](https://github.com/wangf1978/DumpTS/workflows/MSBuild_vc16/badge.svg)](https://github.com/wangf1978/DumpTS/actions)
- Linux<br>
    [![DumpTS Build](https://github.com/wangf1978/DumpTS/workflows/Linux_Build/badge.svg)](https://github.com/wangf1978/DumpTS/actions)<br>
    *Make sure gcc with modern C++11/14 support is installed*<br>
    Here are steps to build and install this application
    ```
    git clone https://github.com/wangf1978/DumpTS.git
    cd DumpTS/build/linux
    make
    cd ../../bin/linux/
    ./DumpTS --help
    ```
- MacOS<br>
    [![DumpTS Build](https://github.com/wangf1978/DumpTS/workflows/MacOS_Build/badge.svg)](https://github.com/wangf1978/DumpTS/actions)<br>
    *Make sure clang with modern C++11/14 support is installed*<br>
    Here are steps to build and install this application:
    ```
    git clone https://github.com/wangf1978/DumpTS.git
    cd DumpTS/build/macos
    make
    cd ../../bin/macos/
    ./DumpTS --help
    ```
    ***(\*) both 32-bit and 64-bit are supported***

[Top](#contents)

# How to run it?

*Usage: DumpTS.exe \[MediaFileName\] \[OPTION\]...*

The option parameter pattern is like as
```
--option_name=option_values
```
*In the option, if one parameter value has multiple parts, each part can be delimited with the characters of `.;,:`. As for the detailed option syntax, please see [Option Syntax](doc/OPT_SYNTAX.md).*

Here are some examples of command lines: 
- Show TS information
    ```
    DumpTS 00001.m2ts --showinfo
    Program Number: 103, program_map_PID: 0X1F0(496).
    Program(PID:0X01F0)
            Stream#0, PID: 0X0138, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#1, PID: 0X0130, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#2, PID: 0X0110, stm_type: 0X0F (AAC Audio)
            Stream#3, PID: 0X0100, stm_type: 0X02 (MPEG2 Video)

    Program(PID:0X01F0)
            Stream#0, PID: 0X0138, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#1, PID: 0X0130, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#2, PID: 0X0110, stm_type: 0X0F (AAC Audio)
            Stream#3, PID: 0X0100, stm_type: 0X02 (MPEG2 Video)
            Stream#4, PID: 0X0111, stm_type: 0X0F (AAC Audio)

    The number of transport packets: 9495552
            PID: 0x0000             transport packet count:     10,579 - PAT
            PID: 0x001E             transport packet count:          2 - DIT
            PID: 0x001F             transport packet count:      2,216 - SIT
            PID: 0x0100             transport packet count:  9,174,898 - MPEG2 Video
            PID: 0x0110             transport packet count:    131,186 - AAC Audio
            PID: 0x0111             transport packet count:    130,561 - AAC Audio
            PID: 0x0130             transport packet count:      1,153 - Teletext, ARIB subtitle or TTML
            PID: 0x0138             transport packet count:        222 - Teletext, ARIB subtitle or TTML
            PID: 0x01F0             transport packet count:     10,579 - PMT
            PID: 0x01FF             transport packet count:     29,568 - PCR
            PID: 0x1FFF             transport packet count:      4,588 - Null packet
    ```
    it will show the PAT and PMT informations of 00001.m2ts, and the packet count for each PID.

- Show TS elementary stream media and codec information
    ```
    DumpTS 00001.m2ts --pid=0x1100 --showinfo
    Dolby Lossless Audio (TrueHD/Atmos) Stream information:
            PID: 0X1100.
            Stream Type: 131(0X83).
            Sample Frequency: 192000 (HZ).
            Bits Per Sample: 24.
            Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
    ```
    It will show the elementary audio stream information, including channel layout, sample frequency and so on

- Show Box information
    ```
    DumpTS test.mp4 --trackid=1 --boxtype=stsd --showinfo
    ```
    Show the 'stsd' box information, for example, HEVC/AVC resolution, chroma, bit-depth and so on

- Show WebM media and codec information
    ```
    DumpTS av1.webm --showinfo
    ```
    Show the tree view for the EBML elements of av1.webm

- Show MMT/TLV container information
    ```
    DumpTS 00301.mmts --showinfo
    ```
    Show the MMT payload information, including PLT, MPT and asset/elementary information

- Dump ES stream from TS/M2TS file
    ```
    DumpTS 00001.m2ts --output=00001.hevc --pid=0x1011 --srcfmt=m2ts --outputfmt=es --showpts  
    ```
    It will dump a hevc es stream with PID 0x1011 from the m2ts stream file: 00001.m2ts, and print the PTS of every frame.

- Change the PID value for the specified ES
    ```
    DumpTS test.ts --output=00001.m2ts --pid=0x100 --destpid=0x1011 --srcfmt=ts --outputfmt=m2ts  
    ```
    It will re-factor the file: test.ts, and replace the PID 0x100 with 0x1011 in TS pack and PSI, and convert it to a m2ts

- Dump ES stream with PID and stream_id_extension from a TS/M2TS file
    ```
    DumpTS 00022.m2ts --output=00022.mlp --pid=0x1100 --srcfmt=m2ts --outputfmt=es 
    --stream_id_extension=0x72  
    ```
    It will dump a MLP sub-stream from 00022.m2ts with the PID 0x1100 and stream\_id\_extension in PES: 0x72

- Remove the specified MP4 Box(es)
    ```
    DumpTS test.mp4 --showinfo --removebox='unkn'
    ```
    Show the MP4 file box layout, and remove box with type 'unkn'

- Dump sample data of the specified track from MP4 stream
    ```
    DumpTS tes.mp4 --output=test.hevc --trackid=1 --outputfmt=es
    ```
    Dump the track#1 of test.mp4, and save its sample data to the file test.hevc with NAL annex-b byte stream format, the VSP, SPS and PPS will be merged into elementary stream data.

- Extract block data of the specified track from WebM file 
    ```
    DumpTS tearsofsteel_4sec0025_3840x2160.y4m-20000.av1.webm --trackid=1 --output=tearsofsteel_4sec0025_4K.av1
    ```
    Extract av1 video stream from .webm file

- Show data fields of each TLV/IPv4/IPv6/MMT/message/table/descriptor/MPU/MFU 
    ```
    DumpTS 00301.mmts --showpack
    ```
    Show the detailed information for MMT packets, payloads, messages, tables and descriptors

- Extract ES data from MMT/TLV file
    ```
    DumpTS 00301.mmts --CID=0 --pid=0x100 --output=e00301.hevc
    ```
    Extract the HEVC stream from header compressed IP packet with context_id: 0 and MMT packet id: 0x100 from 00301.mmts

[Top](#contents)

# Operations Manual
- For MMT, please see [MMT operation guideline](doc/MMT_README.md)
- For AV1/OBU, please see [AV1/OBU operation guideline](doc/AV1_README.md)
- For Audio, please see [Audio operation guideline](doc/AUDIO_README.md)
- For NAL, please see [NAL bitstream operation guideline](doc/NAL_README.md)
- For Transport-Stream, please see [TS bitstream operation guideline](doc/TS_README.md)
- For ISOBMFF, please see [ISOBMF media operation guideline](doc/ISOBMFF_README.md)
- For Matroska, please see [Matroska media operation guideline](doc/MKV_README.md)
- Some utility features are also provided, please see [Utility operation guideline](doc/UTILITY_README.md)
- Media Syntax Element Locator protocol, please see [Syntax Element Locator protocol](doc/MSE_LOCATOR.md)

[Top](#contents)

# References
Please see [References](doc/REFERENCES.md)

[Top](#contents)
