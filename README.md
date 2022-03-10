# Table of Content
* [Terminology](#terminology)
* [What is DumpTS?](#what-is-dumpts)
* [How to build?](#how-to-build)
* [How to run it?](#how-to-run-it)
* [Operations Manual](#operations-manual)

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

[top](#table-of-content)

# What is DumpTS?
DumpTS is a simple utility tool to process the multimedia files packed into main-stream multimedia container formats, which provides these kinds of features:

- Extract and repack the elementary stream data or PSI sections data from *TS, ISOBMFF, Matroska and MMT/TLV* file
- Show media information of elementary streams(AC3/DD+/MLP/DTS/DTS-HD/MPEG2-AAC/MPEG-4 AAC/MPEG2 Video/H.264/H.265/AV1...), *ISOBMFF* box, *Matroska EBML* element and *MMT/TLV* and *TS* packet/message/table/descriptors, container layout, ...
- Re-factor a *TS* stream file in place
- Extract some elementary streams, and reconstruct a partial *TS* file
- Show decoder HRD model information, ATC/PCR/pts/dts and so on, and do the simple verification
- Show the primitive syntax and structure of media file of ISOBMFF/Matroska/TS/PS/TLV-MMT/NAL/AV1.... 
- Provide some utility features for *ISOBMFF* file reconstruction
- Provide some utility functions for codec and container technology, for example, Huffman Codebook, CRC, container layout...

[top](#table-of-content)


# How to build?

- Windows<br>
    1. VS2015/vc14<br>
        Open build/DumpTS_vc14.sln and build it
    2. VS2017/vc15<br>
        Open build/DumpTS_vc15.sln and build it
    3. VS2019/vc16<br>
        Open build/DumpTS_vc16.sln and build it
- Linux<br>
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
    *Make sure clang with modern C++11/14 support is installed*<br>
    Here are steps to build and install this application:
    ```
    git clone https://github.com/wangf1978/DumpTS.git
    cd DumpTS/build/macos
    make
    cd ../../bin/macos/
    ./DumpTS --help
    ```
    ***(\*) both x64 and x86 are supported***

[top](#table-of-content)

# How to run it?

*Usage: DumpTS.exe MediaFileName \[OPTION\]...*

|Option|Value|Description|
|:--|:----:|:--|
|**--output**|*filename*|the output dumped file path|
|**--pid**|*0xhhhh*|the PID of dumped TS stream, or the packet_id of dumped MMT asset|
|**--trackid**|*xx*|the track ID of a ISOBMFF/Matroska file|
|**--CID**|*xx*|the context ID of a header compressed IP packet in MMT/TLV stream|
|**--destpid**|*0xhhhh*|the PID of source stream will be replaced with this PID|
|**--srcfmt**|*ts, m2ts, tts, <br>mp4, <br>mkv, <br>huffman_codebook, <br>spectrum_huffman_codebook_n, <br>aiff, <br>mmt,<br>vob/mpg/mpeg*|the source media format, Including: ts, m2ts, mp4, mkv and huffman_codebook,if it is not specified, find the sync-word to decide it. <BR>BTW:<BR>**mp4**: <br>it is for the ISOBMFF, for example, .mov, .mp4, .m4s, .m4a, .heic, .heif...<BR>**mkv**:<br>it is for Matroska based file-format, for example, .mkv, .webm...<BR>**huffman_codebook:**<br>the VLC tables<BR>**spectrum_huffman_codebook_1~9:**<br>AAC spectrum huffman_codebook 1~11<BR>**aiff:**<br>AIFF or AIFF-C<br>**mmt:**<br>The MMT/TLV stream<br>**vob:**<br>The DVD VOB stream<br>**MPG:**<br>The MPEG PS stream|
|**--outputfmt**|*ts, m2ts, <br>pes, <br>es, <br>wav, pcm, <br>binary_search_table, <br>sourcecode, <br>copy*|the destination dumped format, including: ts, m2ts, pes, es and so on<br>**binary_search_table:**<br>generate the binary search table for Huffman VLC codebook<br>**sourcecode:**<br>generate C/C++ source code<br>**copy**<br>copy the original stream|
|**--stream_id**|*0xhh*|the stream_id in PES header of dumped stream|
|**--sub_stream_id**|*0xhh*|the sub_stream_id in the private data of pack of dumped stream|
|**--stream_id_extension**|*0xhh*|the stream_id_extension in PES header of dumped stream|
|**--MPUseqno**|*xxxx*|the MPU sequence number of MMT stream|
|**--PKTseqno**|*xxxx*|the packet sequence number of MMT stream|
|**--MFU**|N/A|Dumping the each MFU as a saparate file, filename will be {MPUseqno}_xxxx.{assert_type}|
|**--removebox**|*xxxx*|remove the box elements in MP4 file|
|**--boxtype**|*xxxx*|**For ISOBMFF/mp4 source:**<BR>the box type FOURCC, i.e. --boxtype=stsd<BR>**For Matroska/mkv source:**<BR>the EBML ID, i.e. --boxtype=0x1A45DFA3|
|**--crc**|*crc-type, all*|Specify the crc type, if crc type is not specified, list all crc types, if 'all' is specified, calculate all types of crc values|
|**--showinfo**|*N/A*|print the media information of elementary stream, for example, PMT stream types, stream type, audio sample rate, audio channel mapping, video resolution, frame-rate and so on|
|**--showpack<br>--showIPv4pack<br>--showIPv6pack<br>--showHCIPpack<br>--showTCSpack**|*page size<br>default:20*|Show packs in the specified TS/MMT/TLV stream file, pagesize<=0, show all packs w/o interrupt  |
|**--showpts**|*N/A*|print the pts of every elementary stream packet|
|**--showSIT**|*N/A*|print the SIT information, at present only supported ISDB Transport Stream|
|**--showPMT**|*N/A*|print the PMT information in TS stream|
|**--showPAT**|*N/A*|print the PAT information in TS stream|
|**--showMPT**|*N/A*|print the MPT information in MMT/TLV stream|
|**--showCAT**|*N/A*|print the CAT information in MMT/TLV stream|
|**--showPLT**|*N/A*|print the PLT information in MMT/TLV stream|
|**--showEIT**|*N/A*|print the MH-EIT information in MMT/TLV stream or EIT in transport stream|
|**--showPCR**|*[video][audio][full]*|print the PCR clock information in TS stream|
|**--showPCRDiagram**|*[csv filename]*|Export ATC, PCR, PTS/DTS of elementary streams into csv database based on 27MHZ|
|**--showNTP**|*N/A*|print the NTP information in MMT/TLV stream|
|**--diffATC**|diff threshold<br>xxxx(27MHZ)|list the each TS packet arrive time and the diff with the previous TS pack|
|**--showNU**|*[AU];[NU];[SEIMSG];[SEIPAYLOAD]*|print the Access-Unit/nal-unit/sei-message/sei-payload tree of AVC/HEVC/VVC stream|
|**--showVPS**|*N/A*|print the VPS syntax form of HEVC/VVC stream|
|**--showSPS**|*N/A*|print the SPS syntax form of AVC/HEVC/VVC stream|
|**--showPPS**|*N/A*|print the PPS syntax form of AVC/HEVC/VVC stream|
|**--showHRD**|*N/A*|print the Hypothetical reference decoder parameters of AVC/HEVC/VVC stream|
|**--showStreamMuxConfig**|*N/A*|print MPEG4-AAC StreamMuxConfig|
|**--listMMTPpacket**|*N/A*|List the specified MMTP packets|
|**--listMMTPpayload**|*N/A*|List the specified MMTP payloads|
|**--listMPUtime**|*simple(default)<br>full*|List MPU presentation time and its pts/dts offset|
|**--listcrc**|*N/A*|List all CRC types supported in this program|
|**--listmp4box**|*N/A*|List box types and descriptions defined in ISO-14496 spec|
|**--listMMTPpacketid**|*N/A*|Show Assignment of Packet ID of MMTP transmitting message and data|
|**--listMMTSImsg**|*N/A*|Show Assignment of message identifier of MMT-SI|
|**--listMMTSItable**|*N/A*|Show Assignment of identifier of table of MMT-SI|
|**--listMMTSIdesc**|*N/A*|Show Assignment of descriptor tag of MMT-SI|
|**--listmkvEBML**|*N/A*|List EBML elements defined in Matroska spec|
|**--dashinitmp4**|*filename*|the initialization MP4 file to describe the DASH stream global information|
|**--VLCTypes**|*[ahdob][ahdob][ahdob]*|Specify the number value literal formats, a: auto; h: hex; d: dec; o: oct; b: bin, for example, "aah" means:<br>Value and length will be parsed according to literal string, codeword will be parsed according as hexadecimal|
|**--video**|*N/A*|indicate the current dumped stream is a video stream explicitly|
|**--audio**|*N/A*|indicate the current dumped stream is a audio stream explicitly|
|**--subtitle**|*N/A*|indicate the current dumped stream is a subtitle stream explicitly|
|**--stn<br>--stream_number**|*dd/0xhh*|indicate the stream number(1-based) explicitly, if the video option is specified, and this field is 1, it means to do operation for video stream#1|
|**--progseq**|ddd/0xhhh|specify which program sequence the operation is limited to|
|**--start**|ddd/0xhhh|specify where to start dumping the stream data, <BR>for TS, the value should be in unit of TS pack|
|**--end**|ddd/0xhhh|specify where to stop dumping the stream data, <BR>for TS, the value should be in unit of TS pack|
|**--verbose**|*0~n*|print more message in the intermediate process|

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

[top](#table-of-content)

# Operations Manual
- For MMT, please see [MMT operation guideline](MMT_Readme.md)
- For AV1/OBU, please see [AV1/OBU operation guideline](AV1_Readme.md)
- For Audio, please see [Audio operation guideline](Audio_Readme.md)
- For NAL, please see [NAL bitstream operation guideline](NAL_Readme.md)
- For Transport-Stream, please see [TS bitstream operation guideline](TS_Readme.md)
- Some utility features are also provided, please see [Utility operation guideline](Utility_Readme.md)

[top](#table-of-content)
