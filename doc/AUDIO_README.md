# Audio Operation Guideline
## Contents
* [Extract a MPEG-4 AAC stream from MMT stream file](#extract-a-mpeg-4-aac-stream-from-mmt-stream-file)
* [Show the channel layout and mapping](#show-the-channel-layout-and-mapping)
* [Show MPEG4 AAC StreamMuxConfig](#show-mpeg4-aac-streammuxconfig)

[Return to Main](../README.md)
## Extract a MPEG-4 AAC stream from MMT stream file
1. Get the MMT information 
```
DumpTS 03001.mmts --showinfo
Layout of MMT:
    CID: 0X0(0):
        #0000 PLT (Package List Table):
            --------------------------------------------------------------------------------
            #00000 MPT (MMT Package Table), Package_id: 0X1(1), packet_id: 0X9000(36864):
                #00000 Asset, asset_id: 0X8(8), asset_type: hev1(0X68657631):
                    packet_id: 0X100(256)
                    ++++++++++++++ Descriptor: Video component Descriptor +++++++++++++
                              file offset: 5
                           descriptor_tag: 0X8010
                        descriptor_length: 8
                         video_resolution: 6(0X6), height: 2160
                       video_aspect_ratio: 3(0X3), 16:9 without pan vector
                          video_scan_flag: 1, progressive
                         video_frame_rate: 8(0X8), 60/1.001 fps
                            component_tag: 0(0X0)
                    trans_characteristics: 3(0X3), Rec. ITU-R BT.2020
                                 language: jpn (0X006A706E)
                #00001 Asset, asset_id: 0X18(24), asset_type: mp4a(0X6D703461):
                    packet_id: 0X110(272)
                    ++++++++++++++ Descriptor: MH-Audio component Descriptor +++++++++++++
                              file offset: 5
                           descriptor_tag: 0X8014
                        descriptor_length: 10
                           stream_content: 3(0X3), Sound stream by MPEG-4 AAC
                           Dialog_control: 0, Audio stream does not include dialog control information
                        Sound_handicapped: 0(0X0), Audio for the handicapped is not specified
                               Audio_mode: 3(0X3), 2/0 mode (stereo)
                            component_tag: 16(0X10)
                              stream_type: 17(0X11)
                      simulcast_group_tag: 255(0XFF)
                    ES_multi_lingual_flag: 0
                      main_component_flag: 1, main audio
                        quality_indicator: 1Mode 1
                            sampling_rate: 48 kHZ
                                 language: jpn (0X006A706E)

The total number of TLV packets: 135239.
The number of IPv4 TLV packets: 0.
The number of IPv6 TLV packets: 910.
The number of Header Compressed IP packets: 116378.
    CID: 0x0000, packet_id: 0x9000, count:                        299 - MPT
    CID: 0x0000, packet_id: 0x0000, count:                        299 - PA Message
    CID: 0x0000, packet_id: 0x0100, count:                    114,370 - HEVC Video Stream
    CID: 0x0000, packet_id: 0x0110, count:                      1,406 - MPEG-4 AAC Audio Stream
The number of Transmission Control Signal TLV packets: 0.
The number of Null TLV packets: 17951.
The number of other TLV packets: 0.
```
2. Find the MPEG-4 AAC CID and packet_id
```
CID is 0x0000, and packet_id is 0x0110
```
3. Export the MPEG-4 AAC audio stream
```
DumpTS 03001.mmts --CID=0 --pid=0x110 --output=03001_mmts_AAC.loas
Total cost: 3052.383000 ms
Total input TLV packets: 135239.
Total MMT/TLV packets with the packet_id(0X110): 1407
Total MFUs in MMT/TLV packets with the packet_id(0X110): 0
```
4. Show the MPEG-4 AAC audio information
```
DumpTS 03001_mmts_AAC.loas --showinfo
[MP4AAC] Successfully located the syncword in LOAS AudioMuxElement.
Audio Stream#0:
AudioSpecificConfig:
    audioObjectType: 2(0X2)       // AAC LC
    samplingFrequencyIndex: 3(0X3)// 48000HZ
    channelConfiguration: 2(0X2)  // left, right front speakers
    GASpecificConfig:
        frameLengthFlag: 0(0X0)   // 1024/128 lines IMDCT is used and frameLength is set to 1024
        dependsOnCoreCoder: 0(0X0)// Signals that a core coder has been used in an underlying base layer...
        extensionFlag: 0(0X0)     // Shall be '0' for audio object types 1, 2, 3, 4, 6, 7. Shall be '1' ...
        Frame Duration: 21.333ms
```

[Top](#contents)
## Show the channel layout and mapping
There are the below kinds of audio channel layout:
- D-Cinema(SMPTE Standard 428M), AC3, DD+, Dolby True-HD and LPCM follow or inherit it
- DTS/DTS-HD, DTS or DTS-HD audio stream
- ITU-R BS.2051-2, MPEG2 AAC, MP4 AAC and ISOBMFF follow

### For stereo AC3:
```
DumpTS 00015.m2ts --showinfo --pid=0x1100
AC3 Audio Stream information:
        PID: 0X1100.
        Stream Type: 129(0X81).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 2ch(L, R)@D-Cinema.
        Bitrate: 192.000 kbps.
```
### For 5.1ch AC3:
```
DumpTS 00016.m2ts --showinfo --pid=0x1100
AC3 Audio Stream information:
        PID: 0X1100.
        Stream Type: 129(0X81).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
        Bitrate: 640.000 kbps.
```
### For 7.1ch DD+
```
DumpTS 00020.m2ts --showinfo --pid=0x1100
DD+ Audio Stream information:
        PID: 0X1100.
        Stream Type: 132(0X84).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 7.1ch(L, C, R, Ls, Rs, LFE, Rls, Rrs)@D-Cinema.
```
### For Mono Dolby True-HD/Atmos:
```
DumpTS 00022.m2ts --showinfo --pid=0x1100
Dolby Lossless Audio (TrueHD/Atmos) Stream information:
        PID: 0X1100.
        Stream Type: 131(0X83).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 24.
        Channel Layout: 1ch(C)@D-Cinema.
```
### For 2ch Dolby True-HD/Atmos
```
DumpTS 00023.m2ts --showinfo --pid=0x1100
Dolby Lossless Audio (TrueHD/Atmos) Stream information:
        PID: 0X1100.
        Stream Type: 131(0X83).
        Sample Frequency: 96000 (HZ).
        Bits Per Sample: 24.
        Channel Layout: 2ch(Rls, Rrs)@D-Cinema.
```
### For 6ch Dolby True-HD/Atmos
```
DumpTS 00024.m2ts --showinfo --pid=0x1100
Dolby Lossless Audio (TrueHD/Atmos) Stream information:
        PID: 0X1100.
        Stream Type: 131(0X83).
        Sample Frequency: 192000 (HZ).
        Bits Per Sample: 24.
        Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
```
### For stereo DTS:
```
DumpTS 00017.m2ts --showinfo --pid=0x1100
DTS Audio Stream information:
        PID: 0X1100.
        Stream Type: 130(0X82).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 2ch(L, R)@DTS/DTSHD.
        Bitrate: 384.000 kbps.
```
### For 5.1ch DTS:
```
DumpTS 00018.m2ts --showinfo --pid=0x1100
DTS Audio Stream information:
        PID: 0X1100.
        Stream Type: 130(0X82).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 0.
        Channel Layout: 5.1ch(C, L, R, Ls, Rs, LFE1)@DTS/DTSHD.
        Bitrate: 0.000 Mbps.
```
### For 7.1ch DTS-HD:
```
DumpTS 00025.m2ts --showinfo --pid=0x1100
DTS-HD audio Stream information:
        PID: 0X1100.
        Stream Type: 133(0X85).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 7.1ch(C, L, R, Ls, Rs, LFE1, LSr, Rsr)@DTS/DTSHD.
```
### For 6.1ch DTS-HD:
```
DumpTS 00026.m2ts --showinfo --pid=0x1100
DTS-HD audio Stream information:
        PID: 0X1100.
        Stream Type: 133(0X85).
        Sample Frequency: 96000 (HZ).
        Bits Per Sample: 24.
        Channel Layout: 6.1ch(C, L, R, Ls, Rs, LFE1, Cs)@DTS/DTSHD.
```
### For 5.1ch DTS-HD:
```
DumpTS 00027.m2ts --showinfo --pid=0x1100
DTS-HD audio Stream information:
        PID: 0X1100.
        Stream Type: 133(0X85).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 5.1ch(C, L, R, Ls, Rs, LFE1)@DTS/DTSHD.
```
### DTS 5.1ch DTS-HD lossless@24bit@192KHZ:
```
DumpTS 00028.m2ts --showinfo --pid=0x1100
DTS-HD Lossless Audio Stream information:
        PID: 0X1100.
        Stream Type: 134(0X86).
        Sample Frequency: 192000 (HZ).
        Bits Per Sample: 24.
        Channel Layout: 5.1ch(C, L, R, Ls, Rs, LFE1)@DTS/DTSHD.
```
### For stereo LPCM:
```
DumpTS 00013.m2ts --showinfo --pid=0x1100
HDMV LPCM Audio Stream information:
        PID: 0X1100.
        Stream Type: 128(0X80).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 2ch(L, R)@D-Cinema.
```
### For 5.1ch LPCM:
```
DumpTS 00019.m2ts --showinfo --pid=0x1100
HDMV LPCM Audio Stream information:
        PID: 0X1100.
        Stream Type: 128(0X80).
        Sample Frequency: 192000 (HZ).
        Bits Per Sample: 24.
        Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
```
### For Mono MPEG2 AAC
```
DumpTS Mono_AAC_test.m2ts --showinfo --pid=0x110
AAC Audio Stream information:
        PID: 0X110.
        Stream Type: 15(0X0F).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 1ch(C)@ITU-R BS.
```
### For Dual-Mono MPEG2 AAC
```
DumpTS DualMono_AAC_test.m2ts --showinfo --pid=0x110
AAC Audio Stream information:
        PID: 0X110.
        Stream Type: 15(0X0F).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 2ch (Dual-Mono).
```
### For 5.1ch to stereo MPEG2 AAC
```
DumpTS 5.1ch_stereo.m2ts --showinfo --pid=0x110
AAC Audio Stream information:
        PID: 0X110.
        Stream Type: 15(0X0F).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 5.1ch(L, R, C, LFE, Ls, Rs)@ITU-R BS.
AAC Audio Stream information:
        PID: 0X110.
        Stream Type: 15(0X0F).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 2ch(L, R)@ITU-R BS.
```
### For stereo MPEG4 AAC
```
DumpTS 00001.m2ts --showinfo --pid=0x1100
MPEG4 AAC Audio Stream information:
        PID: 0X1100.
        Stream Type: 17(0X11).
        Sample Frequency: 48000 (HZ).
        Bits Per Sample: 16.
        Channel Layout: 2ch(L, R)@ITU-R BS.
```

[Top](#contents)
## Show MPEG4 AAC StreamMuxConfig
Show `StreamMuxConfig` syntax fields of the MPEG4 AAC stream,
```
DumpTS test.loas --showStreamMuxConfig
```
And then,
```
[MP4AAC] Successfully located the syncword in LOAS AudioMuxElement.
Updated Stream Mux Config:
audioMuxVersion: 0
audioMuxVersionA: 0                                            // default
streamCnt: 0                                                   // The count of streams present in this structure
allStreamsSameTimeFraming: 1                                   // A data element indicating whether all payloads, which are multiplex...
numSubFrames: 0(0X0)                                           // 1 PayloadMux() frames are multiplexed
numProgram: 0(0X0)                                             // 1 programs are multiplexed
for (prog = 0; prog <= numProgram:0; prog++):
    numLayer[0]: 0                                             // 1 scalable layers are multiplexed
    for (lay = 0; lay <= numLayer:0; lay++):
        progSIndx[streamCnt:0]: 0(0X0)
        laySIndx[streamCnt:0]: 0(0X0)
        streamID[prog:0][lay:0]=0: 0(0X0)
        useSameConfig: 0                                       // AudioSpecificConfig() is present
        AudioSpecificConfig():
            AudioSpecificConfig:
                audioObjectType: 2(0X2)                        // AAC LC
                samplingFrequencyIndex: 3(0X3)                 // 48000HZ
                channelConfiguration: 2(0X2)                   // left, right front speakers
                GASpecificConfig:
                    frameLengthFlag: 0(0X0)                    // 1024/128 lines IMDCT is used and frameLength is set to 1024
                    dependsOnCoreCoder: 0(0X0)                 // Signals that a core coder has been used in an underlying base layer...
                    extensionFlag: 0(0X0)                      // Shall be '0' for audio object types 1, 2, 3, 4, 6, 7. Shall be '1' ...
        frameLengthType[streamID[prog:0][lay:0]:0]: 0(0X0)     // Payload with variable frame length
        latmBufferFullness[streamID[prog:0][lay:0]:0]: 96(0X60)// the state of the bit reservoir in the course of encoding the first ...
otherDataPresent: 0                                            // The other data than audio payload otherData is not multiplexed
crcCheckPresent: 1                                             // CRC check bits are present
crcCheckSum: 109(0X6D)                                         // CRC error detection data
Updated Stream Mux Config:
audioMuxVersion: 0
audioMuxVersionA: 0                                             // default
streamCnt: 0                                                    // The count of streams present in this structure
allStreamsSameTimeFraming: 1                                    // A data element indicating whether all payloads, which are multiplex...
numSubFrames: 0(0X0)                                            // 1 PayloadMux() frames are multiplexed
numProgram: 0(0X0)                                              // 1 programs are multiplexed
for (prog = 0; prog <= numProgram:0; prog++):
    numLayer[0]: 0                                              // 1 scalable layers are multiplexed
    for (lay = 0; lay <= numLayer:0; lay++):
        progSIndx[streamCnt:0]: 0(0X0)
        laySIndx[streamCnt:0]: 0(0X0)
        streamID[prog:0][lay:0]=0: 0(0X0)
        useSameConfig: 0                                        // AudioSpecificConfig() is present
        AudioSpecificConfig():
            AudioSpecificConfig:
                audioObjectType: 2(0X2)                         // AAC LC
                samplingFrequencyIndex: 3(0X3)                  // 48000HZ
                channelConfiguration: 6(0X6)                    // center front speaker, left, right front speakers,left surround, rig...
                GASpecificConfig:
                    frameLengthFlag: 0(0X0)                     // 1024/128 lines IMDCT is used and frameLength is set to 1024
                    dependsOnCoreCoder: 0(0X0)                  // Signals that a core coder has been used in an underlying base layer...
                    extensionFlag: 0(0X0)                       // Shall be '0' for audio object types 1, 2, 3, 4, 6, 7. Shall be '1' ...
        frameLengthType[streamID[prog:0][lay:0]:0]: 0(0X0)      // Payload with variable frame length
        latmBufferFullness[streamID[prog:0][lay:0]:0]: 149(0X95)// the state of the bit reservoir in the course of encoding the first ...
otherDataPresent: 0                                             // The other data than audio payload otherData is not multiplexed
crcCheckPresent: 1                                              // CRC check bits are present
crcCheckSum: 54(0X36)                                           // CRC error detection data
```

[Top](#contents)
