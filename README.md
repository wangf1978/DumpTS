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

# What is DumpTS?
DumpTS is a simple utility tool to process the multimedia files packed into main-stream multimedia container formats, which provides these kinds of features:

- Extract and repack the elementary stream data or PSI sections data from *TS, ISOBMFF, Matroska and MMT/TLV* file
- Show media information of elementary streams, *ISOBMFF* box, *Matroska EBML* element and *MMT/TLV* packet/message/table/descriptors.
- Re-factor a *TS* stream file in place
- Extract some elementary streams, and reconstruct a partial *TS* file
- Show the primitive syntax and structure of media file of ISOBMFF/Matroska/TS/PS/TLV-MMT/NAL/.... 
- Provide some utility features for *ISOBMFF* file reconstruction
- Provide some utility functions for codec and container technology, for example, Huffman Codebook, CRC, container layout...


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
    Program Number: 1024, program_map_PID: 0X1F0(496).
    Program(PID:0X01F0)
            Stream#0, PID: 0X0138, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#1, PID: 0X0110, stm_type: 0X0F (AAC Audio)
            Stream#2, PID: 0X0100, stm_type: 0X02 (MPEG2 Video)

    Program(PID:0X01F0)
            Stream#0, PID: 0X0130, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#1, PID: 0X0138, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#2, PID: 0X0110, stm_type: 0X0F (AAC Audio)
            Stream#3, PID: 0X0100, stm_type: 0X02 (MPEG2 Video)

    Program(PID:0X01F0)
            Stream#0, PID: 0X0130, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#1, PID: 0X0138, stm_type: 0X06 (Teletext, ARIB subtitle or TTML)
            Stream#2, PID: 0X0110, stm_type: 0X0F (AAC Audio)
            Stream#3, PID: 0X0100, stm_type: 0X02 (MPEG2 Video)
            Stream#4, PID: 0X0111, stm_type: 0X0F (AAC Audio)

    The number of transport stream packs: 15912960
            PID: 0x0000             transport packet count:     21208 - PAT
            PID: 0x001E             transport packet count:         2 - DIT
            PID: 0x001F             transport packet count:      3599 - SIT
            PID: 0x0100             transport packet count:  15581546 - MPEG2 Video
            PID: 0x0110             transport packet count:    250657 - AAC Audio
            PID: 0x0111             transport packet count:       103 - AAC Audio
            PID: 0x0130             transport packet count:      2388 - Teletext, ARIB subtitle or TTML
            PID: 0x0138             transport packet count:      1800 - Teletext, ARIB subtitle or TTML
            PID: 0x01F0             transport packet count:     21207 - PMT
            PID: 0x01FF             transport packet count:     30268 - PCR
            PID: 0x1FFF             transport packet count:       182 - Null packet
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
- Show ATC diff in TTS or M2TS
    ```
    DumpTS 00001.tts --diffATC --start=110 --end=140
    pkt_idx:  0 [header 4bytes: 01 92 62 65] ATC: 0x01926265(  26370661), diff:
    pkt_idx:110 [header 4bytes: 01 94 6C 09] ATC: 0x01946C09(  26504201), diff: 1214(0.044963s)
    pkt_idx:111 [header 4bytes: 01 94 70 C7] ATC: 0x019470C7(  26505415), diff: 1214(0.044963s)
    pkt_idx:112 [header 4bytes: 01 94 75 85] ATC: 0x01947585(  26506629), diff: 1214(0.044963s)
    pkt_idx:113 [header 4bytes: 01 94 7A 43] ATC: 0x01947A43(  26507843), diff: 1214(0.044963s)
    pkt_idx:114 [header 4bytes: 01 94 7F 01] ATC: 0x01947F01(  26509057), diff: 1214(0.044963s)
    pkt_idx:115 [header 4bytes: 01 94 83 BF] ATC: 0x019483BF(  26510271), diff: 1214(0.044963s)
    pkt_idx:116 [header 4bytes: 01 94 88 7D] ATC: 0x0194887D(  26511485), diff: 1214(0.044963s)
    pkt_idx:117 [header 4bytes: 01 94 8D 3B] ATC: 0x01948D3B(  26512699), diff: 1214(0.044963s)
    pkt_idx:118 [header 4bytes: 01 94 91 F9] ATC: 0x019491F9(  26513913), diff: 1214(0.044963s)
    pkt_idx:119 [header 4bytes: 01 94 96 B7] ATC: 0x019496B7(  26515127), diff: 1214(0.044963s)
    pkt_idx:120 [header 4bytes: 01 94 9B 75] ATC: 0x01949B75(  26516341), diff: 1214(0.044963s)
    pkt_idx:121 [header 4bytes: 01 94 A0 33] ATC: 0x0194A033(  26517555), diff: 1214(0.044963s)
    pkt_idx:122 [header 4bytes: 01 94 A4 F1] ATC: 0x0194A4F1(  26518769), diff: 1214(0.044963s)
    pkt_idx:123 [header 4bytes: 01 94 A9 AF] ATC: 0x0194A9AF(  26519983), diff: 1214(0.044963s)
    pkt_idx:124 [header 4bytes: 01 94 AE 6D] ATC: 0x0194AE6D(  26521197), diff: 1214(0.044963s)
    pkt_idx:125 [header 4bytes: 01 94 B3 2B] ATC: 0x0194B32B(  26522411), diff: 1214(0.044963s)
    pkt_idx:126 [header 4bytes: 01 94 B7 E9] ATC: 0x0194B7E9(  26523625), diff: 1214(0.044963s)
    pkt_idx:127 [header 4bytes: 01 94 BC A7] ATC: 0x0194BCA7(  26524839), diff: 1214(0.044963s)
    pkt_idx:128 [header 4bytes: 01 94 C1 65] ATC: 0x0194C165(  26526053), diff: 1214(0.044963s)
    pkt_idx:129 [header 4bytes: 01 94 C6 23] ATC: 0x0194C623(  26527267), diff: 1214(0.044963s)
    pkt_idx:130 [header 4bytes: 01 94 CA E1] ATC: 0x0194CAE1(  26528481), diff: 1214(0.044963s)
    pkt_idx:131 [header 4bytes: 01 94 CF 9F] ATC: 0x0194CF9F(  26529695), diff: 1214(0.044963s)
    pkt_idx:132 [header 4bytes: 01 AD 27 48] ATC: 0x01AD2748(  28125000), diff: 1595305(59.085369s)
    pkt_idx:133 [header 4bytes: 01 AD 48 7F] ATC: 0x01AD487F(  28133503), diff: 8503(0.314926s)
    pkt_idx:134 [header 4bytes: 01 AD 4D 3D] ATC: 0x01AD4D3D(  28134717), diff: 1214(0.044963s)
    pkt_idx:135 [header 4bytes: 01 AD 51 FB] ATC: 0x01AD51FB(  28135931), diff: 1214(0.044963s)
    pkt_idx:136 [header 4bytes: 01 AD 56 B9] ATC: 0x01AD56B9(  28137145), diff: 1214(0.044963s)
    pkt_idx:137 [header 4bytes: 01 AD 5B 77] ATC: 0x01AD5B77(  28138359), diff: 1214(0.044963s)
    pkt_idx:138 [header 4bytes: 01 AD 60 35] ATC: 0x01AD6035(  28139573), diff: 1214(0.044963s)
    pkt_idx:139 [header 4bytes: 01 AD 64 F3] ATC: 0x01AD64F3(  28140787), diff: 1214(0.044963s)
    ```
- Other utilities
    ```
    DumpTs AACScalefactorHuffmanCodebook.txt --VLCTypes=aah --srcfmt=huffman_codebook
    ```
    Load huffman-codebook from the specified file, and print its huffman-tree
    ```
    DumpTs AACScalefactorHuffmanCodebook.txt --VLCTypes=aah --srcfmt=huffman_codebook --outputfmt=binary_search_table
    ```
    Load huffman-codebook from the specified file, and print binary search table for huffman-tree
    ```
    DumpTS Spectrum_Huffman_cb1.txt --VLCTypes=aah --srcfmt=spectrum_huffman_codebook_1
  .
  |--0h ((value: 40, w: 0, x: 0, y: 0, z: 0), length: 1)
  |--1h
       |--2h
       |    |--4h
       |    |    |--8h
       |    |    |    |--10h ((value: 67, w: 1, x: 0, y: 0, z: 0), length: 5)
       |    |    |    |--11h ((value: 13, w: -1, x: 0, y: 0, z: 0), length: 5)
       |    |    |--9h
       |    |         |--12h ((value: 39, w: 0, x: 0, y: 0, z: -1), length: 5)
       |    |         |--13h ((value: 49, w: 0, x: 1, y: 0, z: 0), length: 5)
       |    |--5h
       |         |--Ah
       |         |    |--14h ((value: 41, w: 0, x: 0, y: 0, z: 1), length: 5)
       |         |    |--15h ((value: 37, w: 0, x: 0, y: -1, z: 0), length: 5)
       |         |--Bh
       |              |--16h ((value: 43, w: 0, x: 0, y: 1, z: 0), length: 5)
       |              |--17h ((value: 31, w: 0, x: -1, y: 0, z: 0), length: 5)
  ```
    Load spectrum huffman-codebook#1 from the specified file, and print its huffman-tree
    ```
    DumpTS Spectrum_Huffman_cb1.txt --VLCTypes=aah --srcfmt=spectrum_huffman_codebook_1 --outputfmt=sourcecode
    using VLC_ITEM_QUAD = std::tuple<std::tuple<int64_t, int, int, int>, uint8_t, uint64_t>;
    using Spectrum_Huffman_Codebook_Quad = std::vector<VLC_ITEM_QUAD>;
  
    Spectrum_Huffman_Codebook_Quad VLC_tables_quad = {
        { { 0, -1, -1, -1, -1}, 11, 0x7f8}, { { 1, -1, -1, -1,  0},  9, 0x1f1},
        { { 2, -1, -1, -1,  1}, 11, 0x7fd}, { { 3, -1, -1,  0, -1}, 10, 0x3f5},
        { { 4, -1, -1,  0,  0},  7, 0x68 }, { { 5, -1, -1,  0,  1}, 10, 0x3f0},
        { { 6, -1, -1,  1, -1}, 11, 0x7f7}, { { 7, -1, -1,  1,  0},  9, 0x1ec},
        { { 8, -1, -1,  1,  1}, 11, 0x7f5}, { { 9, -1,  0, -1, -1}, 10, 0x3f1},
        { {10, -1,  0, -1,  0},  7, 0x72 }, { {11, -1,  0, -1,  1}, 10, 0x3f4},
        ......
    };
  
    uint8_t hcb[][2] = {
        {1, 2},
        {40, 0},                            // leaf node (V-index:40(V: 40) L:1 C:0X0)
        {1, 2},
        {2, 3},
        ......
    };
    ```

# More
- For MMT, please see [MMT operation guideline](MMT_Readme.md)
- For AV1/OBU, please see [AV1/OBU operation guideline](AV1_Readme.md)
- For Audio, please see [Audio operation guideline](Audio_Readme.md)
- For NAL, please see [NAL bitstream operation guideline](NAL_Readme.md)
- For Transport-Stream, please see [TS bitstream operation guideline](TS_Readme.md)

