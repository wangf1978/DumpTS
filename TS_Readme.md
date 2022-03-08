# TS/TTS/M2TS Operation Guideline

## Extract a elementary stream
1. Get the transport stream rough information
    ```
    DumpTS 00023.m2ts --showinfo
    ```
    The elementary streams are listed:
    ```
    Program Number: 1, program_map_PID: 0X100(256).
    Program(PID:0X0100)
            Stream#0, PID: 0X1011, stm_type: 0X02 (MPEG2 Video)
            Stream#1, PID: 0X1100, stm_type: 0X83 (Dolby Lossless Audio (TrueHD/Atmos))
            Stream#2, PID: 0X1200, stm_type: 0X90 (PGS)
    ```
2. Get the PID of extracted stream
    For example, the video PID is 0x1011, and the audio is 0x1100, and the PG subtitle is 0x1200
3. Extract the stream
    Extract the video:
    ```
    DumpTS 00023.m2ts --pid=0x1011 --output=i:\00023.m2v
    ```
    An elementary MPEG2 video stream is extracted to I:\00023.m2v, and it can be played by some media player, for example, VLC player, of course the PES stream with pts and dts can also be extracted by specifying the 'outputfmt', and then it can be used for multiplex;
    ```
    DumpTS 00023.m2ts --pid=0x1011 --output=i:\00023.pes --outputfmt=pes
    ```
    On the other hand, the ts/tts/m2ts packs with the specified PID or PIDs can also be extracted to ts/tts/m2ts packs with outputfmt to 'ts/tts/m2ts':
    ```
    DumpTS 00023.m2ts --pid=0x1011 --output=i:\00023.ts --outputfmt=ts
    ```
    But this ts file can't be played because there only exists the pack with PID '0x10111', there is no PAT or PMT

## Extract a PSI data stream
PSI data can also be extracted when specifying the corresponding PID,
```
DumpTs 00024.m2ts --pid=0X0 --output=i:\00024.psi
```
The PSI data will start from 'pointer_field', and then follow with PSI section data

## Extract a sub-stream from one elementary stream
Some audio stream may have multiple sub-stream, for example, Dolby-TrueHD, the sub-streams can be extracted separately by adding stream_id_extension filter
1. Get the audio information
    ```
    Dumpts e:\BDMV\STREAM\00024.m2ts --showinfo --pid=0x1100
    ```
    All sub-streams will be listed,
    ```
    Dolby Lossless Audio (TrueHD/Atmos) Stream information(AC3 part):
            PID: 0X1100.
            Stream ID: 0XFD
            Stream Type: 131(0X83).
            Stream ID Extension: 0X76
            Sample Frequency: 48000 (HZ).
            Bits Per Sample: 16.
            Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
            Bitrate: 640.000 kbps.
    Dolby Lossless Audio (TrueHD/Atmos) Stream information(MLP part):
            PID: 0X1100.
            Stream ID: 0XFD
            Stream Type: 131(0X83).
            Stream ID Extension: 0X72
            Sample Frequency: 192000 (HZ).
            Bits Per Sample: 24.
            Channel Layout: 5.1ch(L, C, R, Ls, Rs, LFE)@D-Cinema.
    ```
2. Select the sub-stream with PID and stream_id_extension
    For example, select the AC3 sub-stream to extract with PID=0x1100 and stream_id_extension=0x76
3. Extract the AC3 sub-stream
    ```
    DumpTs 00024.m2ts --pid=0x1100 --stream_id_extension=0x76 --output=i:\00024.ac3
    ```

## Show PSI information
    
