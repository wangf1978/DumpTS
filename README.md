# Terminology
- **TS**: transport stream, the file extension is normally *.ts, .tts, .m2ts, .mts*
- **ISOBMFF**: ISO-based Media File Format, the file extension is normally *.mp4, .mov, m4a, m4v, .m4s*
- **Matroska**: a multimedia container formats based on EBML (Extensible Binary Meta Language), the file extension is normally *.mkv, .mka, .mk3d and .webm*

# What is DumpTS?
DumpTS is a simple utility tool to process the multimedia files with main-stream multimedia container formats, which will provide these kinds of features:

- Extract and repack the elementary stream data or PSI sections data from *TS, ISOBMFF and Matroska* file
- Show media information of elementary streams, *ISOBMFF* box and *Matroska EBML* element
- Re-factor a *TS* stream file in place
- Extract some elementary streams, and reconstruct a partial *TS* file
- Provide some utility feature for *ISOBMFF* file reconstruction


# How to build?

Use VS2015 to open DumpTS.sln to build it

# How to run it?

*Usage: DumpTS.exe TSSourceFileName \[OPTION\]...*

|Option|Value|Description|
|:--|:----:|:--|
|**--output**|*filename*|the output dumped file path|
|**--pid**|*0xhhhh*|the PID of dumped stream|
|**--trackid**|*xx*|the track ID of a ISOBMFF/Matroska file|
|**--destpid**|*0xhhhh*|the PID of source stream will be replaced with this PID|
|**--srcfmt**|*ts, m2ts, tts, mp4, mkv*|the source media format, Including: ts, m2ts, mp4 and mkv,if it is not specified, find the sync-word to decide it. <BR>BTW:<BR>**mp4**: it is for the ISOBMFF, for example, .mov, .mp4, .m4s, .m4a...<BR>**mkv**, it is for Matroska based file-format, for example, .mkv, .webm...|
|**--outputfmt**|*ts, m2ts, pes, es, wav, pcm*|the destination dumped format, including: ts, m2ts, pes, es and so on|
|**--stream_id**|*0xhh*|the stream_id in PES header of dumped stream|
|**--stream_id_extension**|*0xhh*|the stream_id_extension in PES header of dumped stream|
|**--removebox**|*xxxx*|remove the box elements in MP4 file|
|**--boxtype**|*xxxx*|**For ISOBMFF/mp4 source:**<BR>the box type FOURCC, i.e. --boxtype=stsd<BR>**For Matroska/mkv source:**<BR>the EBML ID, i.e. --boxtype=0x1A45DFA3|
|**--crc**|*crc-type, all*|Specify the crc type, if crc type is not specified, list all crc types, if 'all' is specified, calculate all types of crc values|
|**--showpts**|*N/A*.|print the pts of every elementary stream packet|
|**--showinfo**|*N/A*|print the media information of elementary stream, for example, PMT stream types, stream type, audio sample rate, audio channel mapping, video resolution, frame-rate and so on|
|**--listcrc**||List all CRC types supported in this program|
|**--listmp4box**||List box types and descriptions defined in ISO-14496 spec|
|**--listmkvEBML**||List EBML elements defined in Matroska spec|
|**--verbose**|*0~n*|print more message in the intermediate process|
 
Here are some examples of command lines:  
```
DumpTS c:\00001.m2ts --showinfo
```
it will show the PAT and PMT informations of 00001.m2ts

```
DumpTS c:\00001.m2ts --output=c:\00001.hevc --pid=0x1011 --srcfmt=m2ts --outputfmt=es --showpts  
```
It will dump a hevc es stream with PID 0x1011 from the m2ts stream file: c:\00001.m2ts, and print the PTS of every frame.

```
DumpTS C:\test.ts --output=c:\00001.m2ts --pid=0x100 --destpid=0x1011 --srcfmt=ts --outputfmt=m2ts  
```
It will re-factor the file: c:\test.ts, and replace the PID 0x100 with 0x1011 in TS pack and PSI, and convert it to a m2ts

```
DumpTS C:\00022.m2ts --output=c:\00022.mlp --pid=0x1100 --srcfmt=m2ts --outputfmt=es 
--stream\_id\_extension=0x72  
```
It will dump a MLP sub-stream from C:\00022.m2ts with the PID 0x1100 and stream\_id\_extension in PES: 0x72
```
DumpTS C:\\test.mp4 --showinfo --removebox='unkn'
```
Show the MP4 file box layout, and remove box with type 'unkn'
```
DumpTS C:\\tes.mp4 --output=c:\test.hevc --trackid=1 --outputfmt=es
```
Dump the track#1 of test.mp4, and save its elementary stream data to file test.hevc, the VSP, SPS and PPS will be merged into elementary stream data.
```
DumpTS C:\\test.mp4 --trackid=1 --boxtype=stsd --showinfo
```
Show the 'stsd' box information, for example, HEVC/AVC resolution, chroma, bit-depth and so on
```
DumpTS C:\\av1.webm --showinfo
```
Show the tree view for the EBML elements of av1.webm
```
DumpTS e:\tearsofsteel_4sec0025_3840x2160.y4m-20000.av1.webm --trackid=1 --output=e:\tearsofsteel_4sec0025_4K.av1
```
Extract av1 video stream from .webm file