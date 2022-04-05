# ISOBMFF media operation guideline

## Contents
* [Extract an elementary stream](#extract-an-elementary-stream)
* [Extract an elementary stream from fragmented ISOBMFF](#extract-an-elementary-stream-from-fragmented-isobmff)
* [Show ISOBMFF file layout](#show-isobmff-file-layout)
  * [Show Sample To Chunk Box](#show-sample-to-chunk-box)
  * [Show Chunk Offset Box](#show-chunk-offset-box)
  * [Show Sample Size Boxes](#show-sample-size-boxes)
  * [Show Sample Description Box](#show-sample-description-box)
  * [Show Degradation Priority Box](#show-degradation-priority-box)
  * [Show Composition Time to Sample Box](#show-composition-time-to-sample-box)
  * [Show Decoding Time to Sample Box](#show-decoding-time-to-sample-box)
  * [Show Composition to Decode Box](#show-composition-to-decode-box)
  * [Show Sync Sample Box](#show-sync-sample-box)
* [Remove Box](#remove-box)
* [List the standard boxes in ISOBMF](#list-the-standard-boxes-in-isobmf)

[Return to Main](../README.md)
## Extract an elementary stream
Specified `--trackid` to extract the elementary stream from MP4 file, 
1. Get the ISOBMFF file layout
```
DumpTS test.mp4 --showinfo
  |--ftyp
  |--moov
  |    |--mvhd
  |    |--trak -- track_ID: 1, duration: 1294.267s
  |    |    |--tkhd
  |    |    |--mdia
  |    |         |--mdhd
  |    |         |--hdlr -- Video track
  |    |         |--minf
  |    |              |--dinf
  |    |              |    |--dref
  |    |              |--stbl
  |    |              |    |--stsd -- avc1@640x360
  |    |              |    |    |--avc1
  |    |              |    |         |--avcC
  |    |              |    |--stts
  |    |              |    |--stsc
  |    |              |    |--stco
  |    |              |    |--stsz
  |    |              |    |--stss
  |    |              |--vmhd
  |    |--trak -- track_ID: 2, duration: 1294.350s
  |    |    |--tkhd
  |    |    |--mdia
  |    |         |--mdhd
  |    |         |--hdlr -- Audio track
  |    |         |--minf
  |    |              |--dinf
  |    |              |    |--dref
  |    |              |--stbl
  |    |              |    |--stsd -- mp4a@44100HZ
  |    |              |    |    |--mp4a
  |    |              |    |         |--esds
  |    |              |    |--stts
  |    |              |    |--stsc
  |    |              |    |--stco
  |    |              |    |--stsz
  |    |              |--smhd
  |    |--udta
  |         |--meta
  |              |--hdlr
  |              |--ilst
  |--mdat
```
2. Find which track will be extracted
Here the video track which track_ID is 1 will be extracted, and it is a H.264 stream, so the output filename is better to end with .`h264` or `.avc`
3. Extract the elementary stream
```
DumpTS FILEZ058.mp4 --trackid=1 --output=FILEZ058.h264
```
If there is no VPS/SPS in sample data, they can be merged together with key frames, and guarantee that the output elementary stream file can be decoded independently, and can also be used to multiplex in other containers.

4. Verify the output ES file is valid or not with ffmpeg
```
ffmpeg -i /mnt/i/FILEZ058.h264 -f null /dev/null
```

[Top](#contents)
## Extract an elementary stream from fragmented ISOBMFF
If the ISOBMFF file is fragmented, which may be used in DAHS live streaming scenario, you can specify `--dashinitmp4`,
1. Get the trackid with `--showinfo`
```
DumpTS d:\Materials\DASH\seg_firstinit.mp4 --showinfo
  .
  |--ftyp
  |--free
  |--free
  |--moov
       |--mvhd
       |--mvex
       |    |--mehd
       |    |--trex
       |    |--trep
       |--trak -- track_ID: 1, duration: 0.000s
       |    |--tkhd
       |    |--edts
       |    |    |--elst
       |    |--mdia
       |         |--mdhd
       |         |--hdlr -- Video track
       |         |--minf
       |              |--vmhd
       |              |--dinf
       |              |    |--dref
       |              |--stbl
       |                   |--stsd -- avc1@640x360
       |                   |    |--avc1
       |                   |         |--avcC
       |                   |         |--pasp
       |                   |--stts
       |                   |--stsc
       |                   |--stsz
       |                   |--stco
       |--udta
            |--meta
                 |--hdlr
                 |--ilst
```
2. Select the track
Here select the trackid#1 which is a H.264 stream.
3. Extract the track
```
DumpTS seg_first1.m4s --dashinitmp4=seg_firstinit.mp4 --trackid=1 --output=first.h264
```
4. Verify the output
```
ffmpeg -i first.h264 -f null /dev/null
```

[Top](#contents)
## Show ISOBMFF file layout
```
DumpTS test.mp4 --showinfo
```
And then,
```
  .
  |--ftyp
  |--moov
  |    |--mvhd
  |    |--trak -- track_ID: 1, duration: 1294.267s
  |    |    |--tkhd
  |    |    |--mdia
  |    |         |--mdhd
  |    |         |--hdlr -- Video track
  |    |         |--minf
  |    |              |--dinf
  |    |              |    |--dref
  |    |              |--stbl
  |    |              |    |--stsd -- avc1@640x360
  |    |              |    |    |--avc1
  |    |              |    |         |--avcC
  |    |              |    |--stts
  |    |              |    |--stsc
  |    |              |    |--stco
  |    |              |    |--stsz
  |    |              |    |--stss
  |    |              |--vmhd
  |    |--trak -- track_ID: 2, duration: 1294.350s
  |    |    |--tkhd
  |    |    |--mdia
  |    |         |--mdhd
  |    |         |--hdlr -- Audio track
  |    |         |--minf
  |    |              |--dinf
  |    |              |    |--dref
  |    |              |--stbl
  |    |              |    |--stsd -- mp4a@44100HZ
  |    |              |    |    |--mp4a
  |    |              |    |         |--esds
  |    |              |    |--stts
  |    |              |    |--stsc
  |    |              |    |--stco
  |    |              |    |--stsz
  |    |              |--smhd
  |    |--udta
  |         |--meta
  |              |--hdlr
  |              |--ilst
  |--mdat
```
For HEIF file,
```
DumpTS IMG_4917.HEIC --showinfo
  .
  |--ftyp
  |--meta
  |    |--hdlr
  |    |--dinf
  |    |    |--dref
  |    |--pitm
  |    |--iinf
  |    |--iref
  |    |--iprp
  |    |--idat
  |    |--iloc
  |--mdat
```
For AVIF file,
```
DumpTS test.avif --showinfo
  .
  |--ftyp
  |--meta
  |    |--hdlr
  |    |--pitm
  |    |--iinf
  |    |--iloc
  |    |--iprp
  |    |--iref
  |--free
  |--mdat
```

[Top](#contents)
## Show Sample To Chunk Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=stsc
```
And then,
```
=========================='stsc' box information======================
Entry Count: 293.
--Entry_ID-------First Chunk------Samples Per Chunk----Sample Description Index--
  #000001              1               12               1
  #000002              3               11               1
  #000003              5               12               1
  #000004              6               11               1
  #000005              8               12               1
  #000006              9               11               1
  #000007             12               12               1
  #000008             13               11               1
  #000009             15               12               1
  #000010             16               11               1
  #000011             19               12               1
  #000012             20               11               1
  #000013             22               12               1
  #000014             23               11               1
  #000015             25               12               1
  #000016             26               11               1
  #000017             29               12               1
  #000018             30               11               1
  #000019             32               12               1
  #000020             33               11               1
  #000021             37               12               1
......
```

[Top](#contents)
## Show Chunk Offset Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=stco
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=co64
```
And then
```
=========================='stco' box information======================
Entry Count: 512.
--- Chunk ID ------------- Chunk Offset --
 #1                          169346
 #2                          170166
 #3                          170799
 #4                          225774
 #5                          350722
 #6                          481736
 #7                          620608
 #8                          728288
 #9                          885813
 #10                        1094612
 #11                        1294372
 #12                        1483898
 #13                        1680790
......
```

[Top](#contents)
## Show Sample Size Boxes
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=stsz
```
And then,
```
=========================='stsz' box information======================
Default sample size: 0
The number of samples in the current track: 5777
-- Sample ID ------------- Sample Size --
  #1                            215
  #2                             37
  #3                             37
  #4                             37
  #5                             37
  #6                             37
  #7                             37
  #8                             37
  #9                             37
  #10                            37
  #11                            37
  #12                            37
  #13                            37
  #14                            37
  #15                            37
  #16                            37
  #17                            37
  #18                            37
  #19                            37
  #20                            37
  #21                            37
  #22                            37
  #23                            37
  #24                            37
  #25                            37
  #26                            37
  #27                            37
  #28                            37
  #29                          2158
  #30                          4982
  #31                          6981
  #32                          6697
......
```

[Top](#contents)
## Show Sample Description Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=stsd
```
And then,
```
entry count: 1
Resolution: 1280x720
Compressorname:
Depth: 24 bits/pixel
Coding name: 'avc1'
configurationVersion: 1
AVCProfileIndication: 100
profile_compatibility: 0X0
AVCLevelIndication: 31
lengthSizeMinusOne: 3
numOfSequenceParameterSets: 1
        SequenceParameterSet#0 (nalUnitLength: 24)
                      00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                      ----------------------------------------------------------------
                 000  67  64  00  1F  AC  24  88  05    00  5B  A1  00  00  03  03  E9
                 016  00  00  BB  80  0F  18  32  A0

numOfPictureParameterSets: 1
        PictureParameterSet#0 (nalUnitLength: 4)
                      00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                      ----------------------------------------------------------------
                 000  68  EE  3C  B0

chroma_format: 0
bit_depth_luma_minus8: 0
bit_depth_chroma_minus8: 0
numOfSequenceParameterSetExt: 0
```

[Top](#contents)
## Show Degradation Priority Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=sdtp
```

[Top](#contents)
## Show Composition Time to Sample Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=ctts
```
And then,
```
=========================='ctts' box information======================
entry count: 4868
== Entry ID ===== Sample Count ========= Sample Offset ==
   #1               1                     3754
   #2               1                     7508
   #3               1                     0
   #4               1                     11261
   #5               2                     0
   #6               1                     11261
   #7               2                     0
   #8               1                     7508
   #9               1                     0
   #10              1                     7507
   #11              1                     0
   #12              1                     11261
   #13              2                     0
   #14              1                     7508
   #15              1                     0
   #16              1                     7507
   #17              1                     0
   #18              1                     7508
   #19              1                     0
   #20              1                     7507
   #21              1                     0
   #22              1                     11262
   #23              2                     0
   #24              1                     7507
   #25              1                     0
   #26              1                     11261
   #27              2                     0
   #28              1                     7508
   #29              1                     0
   #30              1                     11261
   #31              2                     0
   #32              1                     7508
   #33              1                     0
   #34              1                     11261
   #35              2                     0
   #36              1                     11261
   #37              2                     0
   #38              1                     11261
   #39              2                     0
   #40              1                     11262
......
```

[Top](#contents)
## Show Decoding Time to Sample Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=stts
```
And then,
```
=========================='stts' box information======================
entry count: 3214
== Entry ID ===== Sample Count ========= Sample Delta ==
  #1               3                         3754(0.041s)
  #2               1                         3753(0.041s)
  #3               3                         3754(0.041s)
  #4               1                         3753(0.041s)
  #5               3                         3754(0.041s)
  #6               1                         3753(0.041s)
  #7               3                         3754(0.041s)
  #8               1                         3753(0.041s)
  #9               3                         3754(0.041s)
  #10              1                         3753(0.041s)
  #11              3                         3754(0.041s)
  #12              1                         3753(0.041s)
  #13              3                         3754(0.041s)
  #14              1                         3753(0.041s)
  #15              3                         3754(0.041s)
  #16              1                         3753(0.041s)
  #17              3                         3754(0.041s)
  #18              1                         3753(0.041s)
  #19              3                         3754(0.041s)
  #20              1                         3753(0.041s)
  #21              3                         3754(0.041s)
  #22              1                         3753(0.041s)
  #23              3                         3754(0.041s)
  #24              1                         3753(0.041s)
  #25              3                         3754(0.041s)
  #26              1                         3753(0.041s)
  #27              3                         3754(0.041s)
  #28              1                         3753(0.041s)
  #29              3                         3754(0.041s)
  #30              1                         3753(0.041s)
......
```

[Top](#contents)
## Show Composition to Decode Box
```
DumpTS FILEZ058.mp4 --trackid=1 --showinfo --boxtype=cslg
```

[Top](#contents)
## Show Sync Sample Box
```
DumpTS STREAMABLE_OUTPUT.mp4 --showinfo --boxtype=stss --trackid=1
```
And then,
```
=========================='stss' box information======================
entry_count: 248
== Entry ID ====== sample_number ===
   #0                        1
   #1                       49
   #2                       97
   #3                      145
   #4                      193
   #5                      241
   #6                      289
   #7                      337
   #8                      385
   #9                      433
   #10                     481
   #11                     529
   #12                     577
   #13                     625
   #14                     673
   #15                     721
   #16                     769
   #17                     817
   #18                     865
   #19                     913
   #20                     961
   #21                    1009
   #22                    1057
   #23                    1105
   #24                    1153
   #25                    1201
   #26                    1249
   #27                    1297
   #28                    1345
   #29                    1393
   #30                    1441
   #31                    1489
......
```

[Top](#contents)
## Remove Box
```
DumpTS test.mp4 --showinfo --removebox='unkn'
```
Show the MP4 file box layout, and remove box with type 'unkn'. If you want not to overwrite the input file, you can specify `--output` to specify the new output file.

[Top](#contents)
## List the standard boxes in ISOBMF
```
DumpTS --listmp4box
```
And then,
```
  -----------Box Type--------------Manda--------------------------Description--------------------------
  .
  |--ftyp                            *   file type and compatibility
  |--pdin                                progressive download information
  |--moov                            *   container for all the metadata
  |    |--mvhd                       *   movie header, overall declarations
  |    |--trak                       *   container for an individual track or stream
  |    |    |--tkhd                  *   track header, overall information about the track
  |    |    |--tref                      track reference container
  |    |    |--trgr                      track grouping indication
  |    |    |--edts                      edit list container
  |    |    |    |--elst                 an edit list
  |    |    |--mdia                  *   container for the media information in a track
  |    |    |    |--mdhd             *   media header, overall information about the media
  |    |    |    |--hdlr             *   handler, declares the media (handler) type
  |    |    |    |--minf             *   media information container
  |    |    |         |--vmhd            video media header, overall information (video track only)
  |    |    |         |--smhd            sound media header, overall information (sound track only)
  |    |    |         |--hmhd            hint media header, overall information (hint track only)
  |    |    |         |--nmhd            Null media header, overall information (some tracks only)
  |    |    |         |--dinf        *   data information box, container
  |    |    |         |    |--dref   *   data reference box, declares source(s) of media data in track
  |    |    |         |--stbl        *   sample table box, container for the time/space map
  |    |    |              |--stsd   *   sample descriptions (codec types, initialization etc.)
  |    |    |              |--stts   *   (decoding) time-to-sample
  |    |    |              |--ctts       (composition) time to sample
  |    |    |              |--cslg       composition to decode timeline mapping
  |    |    |              |--stsc   *   sample-to-chunk, partial data-offset information
  |    |    |              |--stsz       sample sizes (framing)
  |    |    |              |--stz2       compact sample sizes (framing)
  |    |    |              |--stco   *   chunk offset, partial data-offset information
  |    |    |              |--co64       64-bit chunk offset
  |    |    |              |--stss       sync sample table
  |    |    |              |--stsh       shadow sync sample table
  |    |    |              |--padb       sample padding bits
  |    |    |              |--stdp       sample degradation priority
  |    |    |              |--sdtp       independent and disposable samples
  |    |    |              |--sbgp       sample-to-group
  |    |    |              |--sgpd       sample group description
  |    |    |              |--subs       sub-sample information
  |    |    |              |--saiz       sample auxiliary information sizes
  |    |    |              |--saio       sample auxiliary information offsets
  |    |    |--udta                      user-data
  |    |--mvex                           movie extends box
  |         |--mehd                      movie extends header box
  |         |--trex                  *   track extends defaults
  |         |--leva                      level assignment
  |--moof                                movie fragment
  |    |--mfhd                       *   movie fragment header
  |    |--traf                           track fragment
  |         |--tfhd                  *   track fragment header
  |         |--trun                      track fragment run
  |         |--sbgp                      sample-to-group
  |         |--sgpd                      sample group description
  |         |--subs                      sub-sample information
  |         |--saiz                      sample auxiliary information sizes
  |         |--saio                      sample auxiliary information offsets
  |         |--tfdt                      track fragment decode time
  |--mfra                                movie fragment random access
  |    |--tfra                           track fragment random access
  |    |--mfro                       *   movie fragment random access offset
  |--mdat                                media data container
  |--free                                free space
  |--skip                                free space
  |    |--udta                           user-data
  |         |--cprt                      copyright etc.
  |         |--tsel                      track selection box
  |         |--strk                      sub track box
  |              |--stri                 sub track information box
  |              |--strd                 sub track definition box
  |--meta                                metadata
  |    |--hdlr                       *   handler, declares the media (handler) type
  |    |--dinf                           data information box, container
  |    |    |--dref                      data reference box, declares source(s) of media data in track
  |    |--iloc                           item location
  |    |--ipro                           item protection
  |    |    |--sinf                      protection scheme information box
  |    |         |--frma                 original format box
  |    |         |--schm                 scheme type box
  |    |         |--schi                 scheme information box
  |    |--iinf                           item information
  |    |--xml                            XML container
  |    |--bxml                           binary XML container
  |    |--pitm                           primary item reference
  |    |--fiin                           file delivery item information
  |    |    |--paen                      partition entry
  |    |    |    |--fire                 file reservoir
  |    |    |    |--fpar                 file partition
  |    |    |    |--fecr                 FEC reservoir
  |    |    |--segr                      file delivery session group
  |    |    |--gitn                      group id to name
  |    |--idat                           item data
  |    |--iref                           item reference
  |--meco                                additional metadata container
  |    |--mere                           metabox relation
  |--styp                                segment type
  |--sidx                                segment index
  |--ssix                                subsegment index
  |--prft                                producer reference time

```
[Top](#contents)
