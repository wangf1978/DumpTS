# Terminology
- **TS**: transport stream, the file extension is normally *.ts, .tts, .m2ts, .mts*
- **ISOBMFF**: ISO-based Media File Format, the file extension is normally *.mp4, .mov, m4a, m4v, .m4s, .heif, .heic, .avif*
- **Matroska**: a multimedia container format based on EBML (Extensible Binary Meta Language), the file extension is normally *.mkv, .mka, .mk3d and .webm*
- **MMT**: MPEG Media Transport stream, the file extension is normally *.mmts* 
- **PS**: MPEG program stream, the file extension is normally *.vob, .vro, .mpg, .mpeg* 

# What is DumpTS?
DumpTS is a simple utility tool to process the multimedia files packed into main-stream multimedia container formats, which provides these kinds of features:

- Extract and repack the elementary stream data or PSI sections data from *TS, ISOBMFF, Matroska and MMT/TLV* file
- Show media information of elementary streams, *ISOBMFF* box, *Matroska EBML* element and *MMT/TLV* packet/message/table/descriptors.
- Re-factor a *TS* stream file in place
- Extract some elementary streams, and reconstruct a partial *TS* file
- Provide some utility features for *ISOBMFF* file reconstruction
- Provide some utility functions for codec and container technology, for example, Huffman Codebook, CRC, container layout...


# How to build?

- Windows<br>
    1. VS2015/vc14<br>
        Open build/DumpTS_vc14.sln and build it
    2. VS2017/vc15<br>
        Open build/DumpTS_vc15.sln and build it
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
|**--removebox**|*xxxx*|remove the box elements in MP4 file|
|**--boxtype**|*xxxx*|**For ISOBMFF/mp4 source:**<BR>the box type FOURCC, i.e. --boxtype=stsd<BR>**For Matroska/mkv source:**<BR>the EBML ID, i.e. --boxtype=0x1A45DFA3|
|**--crc**|*crc-type, all*|Specify the crc type, if crc type is not specified, list all crc types, if 'all' is specified, calculate all types of crc values|
|**--showinfo**|*N/A*|print the media information of elementary stream, for example, PMT stream types, stream type, audio sample rate, audio channel mapping, video resolution, frame-rate and so on|
|**--showpack[=pagesize]**|*N/A*|Show packs in the specified TS/MMT/TLV stream file, pagesize<=0, show all packs w/o interrupt  |
|**--showpts**|*N/A*|print the pts of every elementary stream packet|
|**--showSIT**|*N/A*|print the SIT information, at present only supported ISDB Transport Stream|
|**--showPMT**|*N/A*|print the PMT information in TS stream|
|**--showPAT**|*N/A*|print the PAT information in TS stream|
|**--showMPT**|*N/A*|print the MPT information in MMT/TLV stream|
|**--showPLT**|*N/A*|print the PLT information in MMT/TLV stream|
|**--listcrc**|*N/A*|List all CRC types supported in this program|
|**--listmp4box**|*N/A*|List box types and descriptions defined in ISO-14496 spec|
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
