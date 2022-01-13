# Terminology
- **TS**: transport stream, the file extension is normally *.ts, .tts, .m2ts, .mts*
- **ISOBMFF**: ISO-based Media File Format, the file extension is normally *.mp4, .mov, m4a, m4v, .m4s, .heif, .heic, .avif*
- **Matroska**: a multimedia container format based on EBML (Extensible Binary Meta Language), the file extension is normally *.mkv, .mka, .mk3d and .webm*
- **MMT**: MPEG Media Transport stream, the file extension is normally *.mmts* 
- **PS**: MPEG program stream, the file extension is normally *.vob, .vro, .mpg, .mpeg* 
- **NAL**: Network Abstract Layer stream, the file extension is normally *.h264, .avc, .h265, .hevc, h266 and .vvc*
- **ADTS** The elementary stream exported from MPEG2-AAC stream which is packetized in TS stream, the file extension is normally .adts
- **LOAS/LATM** The elementary stream exported from MPEG4-AAC stream which is packetized in TS or MMT/TLV stream, the file extension is normally .loas(with sync layer) or .latm, normally exported MPEG4-AAC as .loas

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
    Program Number: 1, program_map_PID: 0X100(256).
    Program(PID:0X0100)
            Stream#0, PID: 0X12A1, stm_type: 0X90 (Unknown)
            Stream#1, PID: 0X1100, stm_type: 0X83 (Dolby Lossless Audio)
            Stream#2, PID: 0X1011, stm_type: 0X24 (HEVC Video)
            Stream#3, PID: 0X12A0, stm_type: 0X90 (Unknown)
            Stream#4, PID: 0X1015, stm_type: 0X24 (HEVC Video)
    ```
    it will show the PAT and PMT informations of 00001.m2ts

- Show TS elementary stream media and codec information
    ```
    DumpTS 00001.m2ts --pid=0x1100 --showinfo
    Dolby Lossless Audio Stream information:
            PID: 0X1100.
            Stream Type: 131(0X83).
            Sample Frequency: 48000 (HZ).
            Bits Per Sample: 24.
            Channel Layout: 7.1ch(L, C, R, Ls, Rs, Lrs, Rrs, LFE).
    Dolby Lossless Audio Stream information:
            PID: 0X1100.
            Stream Type: 131(0X83).
            Sample Frequency: 48000 (HZ).
            Bits Per Sample: 24.
            Channel Layout: 7.1ch(L, C, R, Ls, Rs, Lrs, Rrs, LFE).
            Bitrate: 4202.683 mbps.
    ```
    It will show the elementary audio stream information, including channel layout, sample frequency and so on

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

- Extract block data of the specified track from WebM file 
    ```
    DumpTS tearsofsteel_4sec0025_3840x2160.y4m-20000.av1.webm --trackid=1 --output=tearsofsteel_4sec0025_4K.av1
    ```
    Extract av1 video stream from .webm file

- Show MMT/TLV container information
    ```
    DumpTS 00301.mmts --showinfo
    ```
    Show the MMT payload information, including PLT, MPT and asset/elementary information

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
- Show Data Unit in a MMTP payload
    ```
    DumpTS 00002.mmts --pid=0xF200 --PKTseqno=0x0109895C --showDU
    packet_sequence_number: 0x0109895C, packet_id: 0xF200, MPU sequence number: 0x00000396:
    DataUnit#0(MF_sequence_number: 0X00000000, 
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  00  03  46  03  50                                       | ....F.P
    
    DataUnit#1(MF_sequence_number: 0X00000000,
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  00  B7  44  03  C0  76    F0  2C  21  FF  FE  10  D2  63 | ....D..v.,!....c
     000010  38  E1  83  41  54  09  C5  08    64  C5  71  58  72  0E  88  A1 | 8..AT...d.qXr...
     000020  0F  FA  69  A4  D5  A3  15  26    C8  62  C9  2C  8A  43  1C  B1 | ..i....&.b.,.C..
     000030  04  11  12  51  C2  8C  71  61    61  D1  E0  90  50  B0  5D  06 | ...Q..qaa...P.].
     000040  C4  50  87  FD  7A  F9  3F  11    FD  7E  6F  8A  F0  87  0D  60 | .P..z.?..~o....`
     000050  8A  0A  08  A1  0F  FD  34  D2    6A  D1  8A  93  64  31  64  96 | ......4.j...d1d.
     000060  45  21  8E  58  82  08  89  28    E1  46  38  B0  B0  E8  F0  48 | E!.X...(.F8....H
     000070  28  58  2E  83  62  28  43  FF    5E  BE  4F  C4  7F  5F  9B  E2 | (X..b(C.^.O.._..
     000080  BC  21  C3  58  22  82  82  28    43  FF  4D  34  9A  B4  62  A4 | .!.X"..(C.M4..b.
     000090  D9  0C  59  25  91  48  63  96    20  82  22  4A  38  51  8E  2C | ..Y%.Hc. ."J8Q.,
     0000A0  2C  3A  3C  12  0A  16  0B  A0    DA  10  FF  D7  AF  93  F1  1F | ,:<.............
     0000B0  D7  E6  F8  AF  08  70  D6  08    A0  A1  20                     | .....p....
    
    DataUnit#2(MF_sequence_number: 0X00000000,
             00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
             ----------------------------------------------------------------
     000000  00  00  00  0D  4E  03  01  07    04  00  00  03  02  00  00  05 | ....N...........
     000010  80                                                               | .
    ```
    print the data unit in MMTP packet with packet_sequence_number '0x0109895C' of stream with packet_id 0xF200 in 00002.mmts
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
- list MMT stream MPU time
   ```
   DumpTS 00002.mmts --CID=1 --pid=0xF200 --listMPUtime --start=0x3ea --end=0x3ed
   0, CID: 0x0001(1), packet_id: 0xF200(61952):
     0, MPU_SeqNo: 0x000003ea(1002), presentation_time: 3690620935.983657s, scale: 180,000HZ, decoding_time_offset:9010
     1, MPU_SeqNo: 0x000003eb(1003), presentation_time: 3690620936.517528s, scale: 180,000HZ, decoding_time_offset:9010
     2, MPU_SeqNo: 0x000003ec(1004), presentation_time: 3690620937.051399s, scale: 180,000HZ, decoding_time_offset:9010
   ```
   List the MPU presentation time for the stream with CID=1 and packet_id=0xF200 which MPU_sequence_number is betwwen [0x3ea, 0x3ed)
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
    Show the AVC SPS syntax to learn the configuration of AVC stream
    ```
    DumpTS 00005.h264 --showSPS
    NAL_UNIT:                                                                         // Sequence parameter set
        forbidden_zero_bit: 0                                                         // shall be equal to 0.
        nal_ref_idc: 1(0X1)
        nal_unit_type: 7(0X7)                                                         // Sequence parameter set
        seq_parameter_set_rbsp():
            profile_idc: 100(0X64)                                                    // High Profile
            constraint_set0_flag: 0
            constraint_set1_flag: 0
            constraint_set2_flag: 0
            constraint_set3_flag: 0
            constraint_set4_flag: 0
            constraint_set5_flag: 0
            reserved_zero_2bits: 0(0X0)
            level_idc: 40(0X28)                                                       // 4
            seq_parameter_set_id: 0(0X0)                                              // identifies the sequence parameter set that is referred to...
            chroma_format_idc: 1(0X1)                                                 // 4:2:0
            bit_depth_luma_minus8: 0(0X0)
            bit_depth_chroma_minus8: 0(0X0)
            qpprime_y_zero_transform_bypass_flag: 0                                   // the transform coefficient decoding process and picture co...
            seq_scaling_matrix_present_flag: 0                                        // the flags seq_scaling_list_present_flag[ i ] for i = 0..7...
            log2_max_frame_num_minus4: 3(0X3)                                         // specifies the value of the variable MaxFrameNum that is u...
            pic_order_cnt_type: 0(0X0)                                                // specifies the method to decode picture order count
            log2_max_pic_order_cnt_lsb_minus4: 0(0X0)                                 // the value of the variable MaxPicOrderCntLsb that is used ...
            max_num_ref_frames: 2(0X2)                                                // the maximum number of short-term and long-term reference ...
            gaps_in_frame_num_value_allowed_flag: 0
            pic_width_in_mbs_minus1: 119(0X77)                                        // plus 1 specifies the width of each decoded picture in uni...
            pic_height_in_map_units_minus1: 67(0X43)                                  // plus 1 specifies the height in slice group map units of a...
            frame_mbs_only_flag: 1                                                    // every coded picture of the coded video sequence is a code...
            direct_8x8_inference_flag: 1
            frame_cropping_flag: 1                                                    // the frame cropping offset parameters follow next in the s...
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
                PicWidthInMbs=pic_width_in_mbs_minus1 + 1: 120(0X78)                  // the picture width in units of macroblocks
                PicWidthInSamplesL=PicWidthInMbs * 16: 1920(0X780)                    // picture width for the luma component
                PicWidthInSamplesC=PicWidthInMbs * MbWidthC: 960(0X3C0)               // picture width for the chroma components
                PicHeightInMapUnits=pic_height_in_map_units_minus1 + 1: 68(0X44)
                PicSizeInMapUnits=PicWidthInMbs * PicHeightInMapUnits: 8160(0X1FE0)
                FrameHeightInMbs=(2-frame_mbs_only_flag)*PicHeightInMapUnits: 68(0X44)
                ChromaArrayType: 1(0X1)
                CropUnitX: 2(0X2)
                CropUnitY: 2(0X2)
                BufferWidth: 1920(0X780)                                              // The width of frame buffer
                BufferHeight: 1088(0X440)                                             // The height of frame buffer
                DisplayWidth: 1920(0X780)                                             // The display width
                DisplayHeight: 1080(0X438)                                            // The display height
            vui_parameters_present_flag: 1                                            // the vui_parameters() syntax structure is present
            vui_parameters():
                aspect_ratio_info_present_flag: 1                                     // aspect_ratio_idc is present
                aspect_ratio_idc: 1                                                   // 1:1(Square), example: 3840x2160 16:9 frame without horizo...
                overscan_info_present_flag: 1                                         // the overscan_appropriate_flag is present
                overscan_appropriate_flag: 1                                          // indicates that the cropped decoded pictures output are su...
                video_signal_type_present_flag: 0                                     // specify that video_format, video_full_range_flag and colo...
                chroma_loc_info_present_flag: 0                                       // specifies that chroma_sample_loc_type_top_field and chrom...
                timing_info_present_flag: 1                                           // specifies that vui_num_units_in_tick, vui_time_scale, vui...
                num_units_in_tick: 5005(0X138D)                                       // the number of time units of a clock operating at the freq...
                time_scale: 240000(0X3A980)                                           // the number of time units that pass in one second.
                fixed_frame_rate_flag: 1                                              // the temporal distance between the HRD output times of any...
                nal_hrd_parameters_present_flag: 1                                    // NAL HRD parameters (pertaining to Type II bitstream confo...
                hrd_parameters:                                                       // NAL HRD parameters
                    cpb_cnt_minus1: 0(0X0)                                            // plus 1 specifies the number of alternative CPB specificat...
                    bit_rate_scale: 2(0X2)                                            // (together with bit_rate_value_minus1[ SchedSelIdx ]) spec...
                    cpb_size_scale: 3(0X3)                                            // (together with cpb_size_value_minus1[ SchedSelIdx ]) spec...
                    for(SchedSelIdx=0;SchedSelIdx<=cpb_cnt_minus1;SchedSelIdx++):
                        bit_rate_value_minus1[0]: 78124                               // (together with bit_rate_scale) specifies the maximum inpu...
                        cpb_size_value_minus1[0]: 234374                              // is used together with cpb_size_scale to specify the Sched...
                        cbr_flag[0]: 0                                                // specifies that to decode this bitstream by the HRD using ...
                    initial_cpb_removal_delay_length_minus1: 17(0X11)                 // the length in bits of the initial_cpb_removal_delay[Sched...
                    cpb_removal_delay_length_minus1: 7(0X7)                           // the length in bits of the cpb_removal_delay syntax elemen...
                    dpb_output_delay_length_minus1: 7(0X7)                            // the length in bits of the dpb_output_delay syntax element
                    time_offset_length: 24(0X18)                                      // greater than 0 specifies the length in bits of the time_o...
                vcl_hrd_parameters_present_flag: 1                                    // VCL HRD parameters (clauses E.1.2 and E.2.2) immediately ...
                vcl_parameters:                                                       // VCL HRD parameters
                    cpb_cnt_minus1: 0(0X0)                                            // plus 1 specifies the number of alternative CPB specificat...
                    bit_rate_scale: 2(0X2)                                            // (together with bit_rate_value_minus1[ SchedSelIdx ]) spec...
                    cpb_size_scale: 4(0X4)                                            // (together with cpb_size_value_minus1[ SchedSelIdx ]) spec...
                    for(SchedSelIdx=0;SchedSelIdx<=cpb_cnt_minus1;SchedSelIdx++):
                        bit_rate_value_minus1[0]: 78124                               // (together with bit_rate_scale) specifies the maximum inpu...
                        cpb_size_value_minus1[0]: 78124                               // is used together with cpb_size_scale to specify the Sched...
                        cbr_flag[0]: 0                                                // specifies that to decode this bitstream by the HRD using ...
                    initial_cpb_removal_delay_length_minus1: 17(0X11)                 // the length in bits of the initial_cpb_removal_delay[Sched...
                    cpb_removal_delay_length_minus1: 7(0X7)                           // the length in bits of the cpb_removal_delay syntax elemen...
                    dpb_output_delay_length_minus1: 7(0X7)                            // the length in bits of the dpb_output_delay syntax element
                    time_offset_length: 24(0X18)                                      // greater than 0 specifies the length in bits of the time_o...
                pic_struct_present_flag: 1                                            // picture timing SEI messages (clause D.2.2) are present th...
                bitstream_restriction_flag: 1                                         // the following coded video sequence bitstream restriction ...
                motion_vectors_over_pic_boundaries_flag: 1                            // one or more samples outside picture boundaries may be use...
                max_bytes_per_pic_denom: 4(0X4)                                       // a number of bytes not exceeded by the sum of the sizes of...
                max_bits_per_mb_denom: 0(0X0)                                         // an upper bound for the number of coded bits of macroblock...
                log2_max_mv_length_horizontal: 16(0X10)                               // the maximum absolute value of a decoded horizontal motion...
                log2_max_mv_length_vertical: 16(0X10)                                 // the maximum absolute value of a decoded vertical motion v...
                max_num_reorder_frames: 1(0X1)                                        // an upper bound for the number of frames buffers, in the d...
                max_dec_frame_buffering: 4(0X4)                                       // the required size of the HRD decoded picture buffer (DPB)...
            rbsp_trailing_bits:
                rbsp_stop_one_bit: 1                                                  // Should be 1
                rbsp_alignment_zero_bit: 0                                            // Should be 0
                rbsp_alignment_zero_bit: 0                                            // Should be 0
                rbsp_alignment_zero_bit: 0                                            // Should be 0
                rbsp_alignment_zero_bit: 0                                            // Should be 0
                rbsp_alignment_zero_bit: 0                                            // Should be 0
                rbsp_alignment_zero_bit: 0                                            // Should be 0
                rbsp_alignment_zero_bit: 0                                            // Should be 0
    ```

# More
- For MMT, please see [MMT operation guideline](MMT_Readme.md)

