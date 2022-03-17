# OPTION SYNTAX
## Contents
* [Options table](#option-table)

## Option Table

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
|**--PKTno**|*xxxx*|the packet number 1-based of media stream|
|**--PKTid**|*xxxx*|the packet index 0-based of media stream|
|**--MFU**|N/A|Dumping the each MFU as a saparate file, filename will be {MPUseqno}_xxxx.{assert_type}|
|**--removebox**|*xxxx*|remove the box elements in MP4 file|
|**--boxtype**|*xxxx*|**For ISOBMFF/mp4 source:**<BR>the box type FOURCC, i.e. --boxtype=stsd<BR>**For Matroska/mkv source:**<BR>the EBML ID, i.e. --boxtype=0x1A45DFA3|
|**--crc**|*crc-type, all*|Specify the crc type, if crc type is not specified, list all crc types, if 'all' is specified, calculate all types of crc values|
|**--showinfo**|*N/A*|print the media information of elementary stream, for example, PMT stream types, stream type, audio sample rate, audio channel mapping, video resolution, frame-rate and so on|
|**--showpack<br>--showIPv4pack<br>--showIPv6pack<br>--showHCIPpack<br>--showTCSpack**|*page size<br>default:20*|Show packs in the specified TS/MMT/TLV stream file, pagesize<=0, show all packs w/o interrupt  |
|**--showOBU**|*[tu[\|fu[\|obu]]]*|Show AV1 bitstream hierarchical layer|
|**--showpts**|*N/A*|print the pts of every elementary stream packet|
|**--showSIT**|*N/A*|print the SIT information, at present only supported ISDB Transport Stream|
|**--showPMT**|*N/A*|print the PMT information in TS stream|
|**--showPAT**|*N/A*|print the PAT information in TS stream|
|**--showMPT**|*N/A*|print the MPT information in MMT/TLV stream|
|**--showCAT**|*N/A*|print the CAT information in MMT/TLV stream or transport stream|
|**--showPLT**|*N/A*|print the PLT information in MMT/TLV stream|
|**--showEIT**|*N/A*|print the MH-EIT information in MMT/TLV stream or EIT in transport stream|
|**--showDU**|*N/A*|show the data unit of MMT payload of the MMT/TLV stream|
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
