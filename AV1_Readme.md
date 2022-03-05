# AV1 operation guideline

## Export av1 bitstream from webm file
Here are the steps to get the AV1 bitstream from WebM file
```
dumpts d:\Materials\AV1\Stream3_AV1_720p_3.9mbps.webm --showinfo | more
```
Show the rough information of WebM file
```
 .
  |--EBML (Size: 31)
  |    |--EBMLVersion (Size: 1): 1
  |    |--EBMLReadVersion (Size: 1): 1
  |    |--EBMLMaxIDLength (Size: 1): 4
  |    |--EBMLMaxSizeLength (Size: 1): 8
  |    |--DocType (Size: 4): webm
  |    |--DocTypeVersion (Size: 1): 4
  |    |--DocTypeReadVersion (Size: 1): 2
  |--Segment (Size: 89056397)
       |--SeekHead (Size: 59)
       |    |--Seek (Size: 11)
       |    |    |--SeekID (Size: 4): Binary
       |    |    |--SeekPosition (Size: 1): 110
       |    |--Seek (Size: 11)
       |    |    |--SeekID (Size: 4): Binary
       |    |    |--SeekPosition (Size: 1): 162
       |    |--Seek (Size: 11)
       |    |    |--SeekID (Size: 4): Binary
       |    |    |--SeekPosition (Size: 1): 202
       |    |--Seek (Size: 14)
       |         |--SeekID (Size: 4): Binary
       |         |--SeekPosition (Size: 4): 89054605
       |--Void (Size: 44): Binary
       |--Info (Size: 47)
       |    |--TimecodeScale (Size: 3): 1000000
       |    |--Duration (Size: 4): 181520.000000
       |    |--MuxingApp (Size: 15): libwebm-0.2.1.0
       |    |--WritingApp (Size: 12): aomenc 1.0.0
       |--Tracks (Size: 35)
       |    |--TrackEntry (Size: 33)
       |         |--TrackNumber (Size: 1): 1
       |         |--TrackUID (Size: 7): 43893850464213234
       |         |--TrackType (Size: 1): 1
       |         |--CodecID (Size: 5): V_AV1
```
It can conclude that the track number is 1, ok use the below command to show the information of this track:
```
dumpts d:\Materials\AV1\Stream3_AV1_720p_3.9mbps.webm --trackid=1 --showinfo
```
And then extract the AV1 stream to your local drive:
```
dumpts d:\Materials\AV1\Stream3_AV1_720p_3.9mbps.webm --trackid=1 --output=i:\Stream3_AV1_720p_3.9mbps.av1
```
Finally to check this AV1 stream file
```
dumpts i:\Stream3_AV1_720p_3.9mbps.av1 --showobu | more
[AV1] hit one temporal_unit (Leb128Bytes: 1, unit_size: 18).
[AV1]   hit one frame_unit (Leb128Bytes: 1, unit_size: 0).
[AV1][obu unit] The sub unit costs too many bytes(1) which exceed the upper unit size(0).
[AV1][temporal_unit#00000000] The current file: i:\Stream3_AV1_720p_3.9mbps.av1 is NOT an Annex-B length delimited bitstream.
[AV1]           hit obu_type: Temporal delimiter OBU.
[AV1]           hit obu_type: Sequence header OBU.
[AV1] The current file: i:\Stream3_AV1_720p_3.9mbps.av1 is a low overhead bitstream format.
Low-Overhead AV1 bitstream...
Temporal Unit#0
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Sequence header OBU
                OBU#2: Frame OBU
Temporal Unit#1
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame OBU
                OBU#2: Frame OBU
                OBU#3: Frame OBU
                OBU#4: Frame OBU
Temporal Unit#2
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame header OBU
Temporal Unit#3
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame OBU
Temporal Unit#4
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame header OBU
Temporal Unit#5
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame OBU
                OBU#2: Frame OBU
                OBU#3: Frame OBU
Temporal Unit#6
        Frame Unit#0
                OBU#0: Temporal delimiter OBU
                OBU#1: Frame header OBU
Temporal Unit#7
        Frame Unit#0
```
