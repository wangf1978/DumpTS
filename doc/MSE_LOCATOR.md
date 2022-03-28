# Media Syntax Element Locator
## Contents
* [Abbreviation](#abbreviation)
* [What's media syntax element?](#whats-media-syntax-element)
* [How to locate the media syntax element?](#how-to-locate-the-media-syntax-element)
	* [MPEG video bitstream media syntax element locator](#mpeg-video-bitstream-media-syntax-element-locator)
	* [NAL bitstream media syntax element locator](#nal-bitstream-media-syntax-element-locator)
	* [AV1 bitstream media syntax element locator](#av1-bitstream-media-syntax-element-locator)
	* [ISOBMFF media syntax element locator](#isobmff-media-syntax-element-locator)
* [Commands](#commands)
	* [`listMSE` command](#listmse-command)
		* [MPEG2 Video](#mpeg2-video)
			* [1. List all video sequences](#list-all-video-sequences)
			* [2. List all GOPs](#list-all-gops)
			* [3. List all access units](#list-all-access-units)
			* [4. List all MPEG2 syntactic elements](#list-all-mpeg2-syntactic-elements)
            * [5. List all slices](#list-all-slices)
            * [6. List all non-slices syntactic elements](#list-all-non-slices-syntactic-elements)
            * [7. List all macro-blocks](#list-all-macro-blocks)
            * [8. List all GOPs in a specified video sequence](#list-all-gops-in-a-specified-video-sequence)
            * [9. List all access-units in a specified GOP](#list-all-access-units-in-a-specified-gop)
            * [10. List some syntax elements in a/some specified GOP(s) or/and Access Unit(s)](#list-some-syntax-elements-in-asome-specified-gops-orand-access-units)
	* [`showMSE` command](#showmse-command)
	* [`showMSEHex` command](#showmse-command)

[Return to Main](../README.md)
## Abbreviation
* **URI**: Uniform Resource Identifier
* **MSE**: Media Syntax Element
* **VSEQ**: Video Sequence, one VSEQ should have the same profile, level, tier(if have), resolution, fps, aspect-ratio, colour primaries, transfer characteristics and so on
* **AU**: Access Unit
* **CVS**: Codec Video Sequence, like as GOP
* **TU**: Temporal Unit
* **FU**: Frame Unit
* **OBU**: Open Bitstream Unit
* **GOP**: Group of Picture, included open GOP, closed GOP
* **NU**: NAL Unit, including VCL/none-VCL NAL Unit
* **PL**: Payload
* **MSG**: Message
* **SEIPL**: SEI payload
* **SEIMSG**: SEI Message
* **SE**: Syntax Element, normally the minimum organization unit

[Top](#contents)

## What's media syntax element?
For the audio, video and other multimedia payload, in the corresponding technical specification, it defined its data organization, and let's call the minimum organization units as `syntax element`, normally it is hierarchical, and defined in tabular form, for example, H.264, it consists of:
```mermaid
flowchart TD
    CVS[Codec video sequence] -->|1:n| AU(Access Unit)
    AU -->|1:n| NU{NAL unit}
    NU -->|1:1| SPSPPS[SPS or PPS]
    NU -->|1:n| SEIMSG[SEI message]
    NU -->|1:1| SLC(slice)
    NU --> Others
    SEIMSG -->|1:n| SEIPAYLOAD(sei payload)
    SLC -->|1:n| MB(macroblock)
```

[Top](#contents)
## How to locate the media syntax element?
Since every media syntax element can be unpacked, and parsed, and its syntax view can also be displayed, we need defined a media syntax element locator protocol to locate it accurately.
In this document, we follow the RFC3986,
```
      URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]

      hier-part   = "//" authority path-abempty
                  / path-absolute
                  / path-rootless
                  / path-empty
```
scheme: only support `MSE`, it can be also ignored
hier-part: the media syntax element locate part, it is the media object, or syntactic elements defined in each multimedia specification scheme
query and framgment: it is the syntax structure in the media object or syntactic element.

**hier-part** *: `[syntax-element-filter[.syntax-element-filter[....]]][/syntax/element/inside/...][#leaf-field]`*

**syntax_element-fitler** *: `[`~`]syntax_element[s][-][e]]`*

**~** *:exclude the followed syntax element(s), or range*

**s** *:the start index(0-based) of syntax element in its parents*

**e** *:the end index(0-based) of syntax element in its parents*

### MPEG video bitstream media syntax element locator
*[MSE://][[`~`]**MB**[`m₀`][`-`][`mₙ`]][.]\([[`~`]**SE**[`s₀`][`-`][`sₙ`]] | [[`~`]**SLICE**[`s₀`][`-`][`sₙ`]])[.][[`~`]**AU**[`a₀`][`-`][`aₙ`]][.][[`~`]**GOP**[`g₀`][`-`][`gₙ`]][.][[`~`]**VSEQ**[`v₀`][`-`][`vₙ`]][/part/part/...][#field]*

- AU
    Normally a MPEG2 video frame, I frame may start video sequence_header + sequence_extension + extension_and_user_data(0) + group_of_pictures_header + extension_and_user_data(1) + picture_header + ...
- SE (syntactic element)
	The basic element start with start_code, for example, sequence_header, sequence_extension and so on 

| URI | comment |
| --- | --- |
| VSEQ`1` | The 2nd video sequence in the MPEG video bitstream |
| GOP`50` | The GOP#50(0-based) in the whole MPEG video bitstream instead of a specified video sequence |
| AU`50` | The access-unit#50(0-based) in the whole MPEG video bitstream instead of a specified video sequence or/and a specified GOP |
| AU`50`.VSEQ`1` |The access-unit#50(0-based) in the second video sequence|
| AU`1`.GOP`0`.VSEQ`1` |The 2nd access-unit of the first GOP in the 2nd video sequence|
| SE`0`.GOP`0`.VSEQ`1`/#aspect_ratio_information |aspect_ratio_information of sequence header if SE#0 is a sequence header|
|~slice.au`1`|all non-slice syntax elements of the 2nd access-unit|
|slice.au`1-2`|all slices of access-unit 1 and 2|

Here are some command examples:
```
DumpTS 00023.m2v --listmse
```
Show all syntax elements in a MPEG2 video stream:
```
------------Name-------------------------------|-----len-----|------------URI-------------
Video Sequence#0                               |             |                      VSEQ0
    GOP#0 (closed)                             |             |                 GOP0.VSEQ0
        AU#0 (I)                               |    91,997 B |             AU0.GOP0.VSEQ0
            SE#0 sequence_header               |       140 B |         SE0.AU0.GOP0.VSEQ0
            SE#1 sequence_extension            |        10 B |         SE1.AU0.GOP0.VSEQ0
            SE#2 group_of_pictures_header      |         8 B |         SE2.AU0.GOP0.VSEQ0
            SE#3 picture_header                |         8 B |         SE3.AU0.GOP0.VSEQ0
            SE#4 picture_coding_extension      |         9 B |         SE4.AU0.GOP0.VSEQ0
            SE#5 slice1                        |       688 B |         SE5.AU0.GOP0.VSEQ0
            SE#6 slice2                        |       692 B |         SE6.AU0.GOP0.VSEQ0
            SE#7 slice3                        |       698 B |         SE7.AU0.GOP0.VSEQ0
            SE#8 slice4                        |       714 B |         SE8.AU0.GOP0.VSEQ0
            SE#9 slice5                        |       831 B |         SE9.AU0.GOP0.VSEQ0
            SE#10 slice6                       |     1,691 B |        SE10.AU0.GOP0.VSEQ0
            SE#11 slice7                       |     1,798 B |        SE11.AU0.GOP0.VSEQ0
            SE#12 slice8                       |     1,730 B |        SE12.AU0.GOP0.VSEQ0
            SE#13 slice9                       |     1,747 B |        SE13.AU0.GOP0.VSEQ0
            SE#14 slice10                      |     1,746 B |        SE14.AU0.GOP0.VSEQ0
            SE#15 slice11                      |     1,714 B |        SE15.AU0.GOP0.VSEQ0
            SE#16 slice12                      |     1,723 B |        SE16.AU0.GOP0.VSEQ0
            SE#17 slice13                      |     1,762 B |        SE17.AU0.GOP0.VSEQ0
            SE#18 slice14                      |     1,846 B |        SE18.AU0.GOP0.VSEQ0
            SE#19 slice15                      |     1,824 B |        SE19.AU0.GOP0.VSEQ0
            SE#20 slice16                      |     1,842 B |        SE20.AU0.GOP0.VSEQ0
            SE#21 slice17                      |     1,792 B |        SE21.AU0.GOP0.VSEQ0
            SE#22 slice18                      |     1,779 B |        SE22.AU0.GOP0.VSEQ0
            SE#23 slice19                      |     1,769 B |        SE23.AU0.GOP0.VSEQ0
            SE#24 slice20                      |     1,791 B |        SE24.AU0.GOP0.VSEQ0
            SE#25 slice21                      |     1,827 B |        SE25.AU0.GOP0.VSEQ0
            SE#26 slice22                      |     1,923 B |        SE26.AU0.GOP0.VSEQ0
            SE#27 slice23                      |     1,869 B |        SE27.AU0.GOP0.VSEQ0
            SE#28 slice24                      |     1,903 B |        SE28.AU0.GOP0.VSEQ0
            SE#29 slice25                      |     1,847 B |        SE29.AU0.GOP0.VSEQ0
            SE#30 slice26                      |     1,842 B |        SE30.AU0.GOP0.VSEQ0
            SE#31 slice27                      |     1,778 B |        SE31.AU0.GOP0.VSEQ0
            SE#32 slice28                      |     1,678 B |        SE32.AU0.GOP0.VSEQ0
            SE#33 slice29                      |     1,620 B |        SE33.AU0.GOP0.VSEQ0
......
```
Ok, now want to see the sequence header syntax according to its given URI,
```
DumpTS 00023.m2v --showmse=SE0.AU0.GOP0.VSEQ0
```
And then,
```
Video Sequence#0                               |             |                      VSEQ0
    GOP#0 (closed)                             |             |                 GOP0.VSEQ0
        AU#0 (I)                               |    91,997 B |             AU0.GOP0.VSEQ0
            SE#0 sequence_header               |       140 B |         SE0.AU0.GOP0.VSEQ0
            ------------------------------------------------------------------------------
            sequence_header_code: 00 00 01 b3 // should be 00 00 01 B3
            horizontal_size_value: 1920(0X780)// This word forms the 12 least significant bits of horizontal_size
            vertical_size_value: 1080(0X438)  // This word forms the 12 least significant bits of vertical_size
            aspect_ratio_information: 3(0X3)  // 16:9
            frame_rate_code: 4(0X4)           // 30 000/1001 (29.97...)
            bit_rate_value: 87500(0X155CC)    // The lower 18 bits of bit_rate, and the upper 12 bits are in bit_rat...
            marker_bit: 1
            vbv_buffer_size_value: 597(0X255) // the lower 10 bits of vbv_buffer_size, and the upper 8 bits are in v...
            constrained_parameters_flag: 0    // This flag (used in ISO/IEC 11172-2) has no meaning in this Specific...
            load_intra_quantiser_matrix: 1    // See 6.3.11 "Quant matrix extension".
            intra_quantiser_matrix:           // See 6.3.11 "Quant matrix extension".
                  8  16  16  19  16  19  22  22
                 22  22  22  22  26  24  26  27
                 27  27  26  26  26  26  27  27
                 27  29  29  29  34  34  34  29
                 29  29  27  27  29  29  32  32
                 34  34  37  38  37  35  35  34
                 35  38  38  40  40  40  48  48
                 46  46  56  56  58  69  69  83

            load_non_intra_quantiser_matrix: 1// See 6.3.11 "Quant matrix extension".
            non_intra_quantiser_matrix:       // See 6.3.11 "Quant matrix extension".
                 16  17  17  18  18  18  19  19
                 19  19  20  20  20  20  20  21
                 21  21  21  21  21  22  22  22
                 22  22  22  22  23  23  23  23
                 23  23  23  23  24  24  24  25
                 24  24  24  25  26  26  26  26
                 25  27  27  27  27  27  28  28
                 28  28  30  30  30  31  31  33
```
Now, sequence_header and sequence_extension want to be shown together,
```
DumpTS 00023.m2v --showmse=SE0.AU0.GOP0.VSEQ0
```
And then
```
Video Sequence#0                               |             |                      VSEQ0
    GOP#0 (closed)                             |             |                 GOP0.VSEQ0
        AU#0 (I)                               |    91,997 B |             AU0.GOP0.VSEQ0
            SE#0 sequence_header               |       140 B |         SE0.AU0.GOP0.VSEQ0
            ------------------------------------------------------------------------------
            sequence_header_code: 00 00 01 b3 // should be 00 00 01 B3
            horizontal_size_value: 1920(0X780)// This word forms the 12 least significant bits of horizontal_size
            vertical_size_value: 1080(0X438)  // This word forms the 12 least significant bits of vertical_size
            aspect_ratio_information: 3(0X3)  // 16:9
            frame_rate_code: 4(0X4)           // 30 000/1001 (29.97...)
            bit_rate_value: 87500(0X155CC)    // The lower 18 bits of bit_rate, and the upper 12 bits are in bit_rat...
            marker_bit: 1
            vbv_buffer_size_value: 597(0X255) // the lower 10 bits of vbv_buffer_size, and the upper 8 bits are in v...
            constrained_parameters_flag: 0    // This flag (used in ISO/IEC 11172-2) has no meaning in this Specific...
            load_intra_quantiser_matrix: 1    // See 6.3.11 "Quant matrix extension".
            intra_quantiser_matrix:           // See 6.3.11 "Quant matrix extension".
                  8  16  16  19  16  19  22  22
                 22  22  22  22  26  24  26  27
                 27  27  26  26  26  26  27  27
                 27  29  29  29  34  34  34  29
                 29  29  27  27  29  29  32  32
                 34  34  37  38  37  35  35  34
                 35  38  38  40  40  40  48  48
                 46  46  56  56  58  69  69  83

            load_non_intra_quantiser_matrix: 1// See 6.3.11 "Quant matrix extension".
            non_intra_quantiser_matrix:       // See 6.3.11 "Quant matrix extension".
                 16  17  17  18  18  18  19  19
                 19  19  20  20  20  20  20  21
                 21  21  21  21  21  22  22  22
                 22  22  22  22  23  23  23  23
                 23  23  23  23  24  24  24  25
                 24  24  24  25  26  26  26  26
                 25  27  27  27  27  27  28  28
                 28  28  30  30  30  31  31  33


            SE#1 sequence_extension            |        10 B |         SE1.AU0.GOP0.VSEQ0
            ------------------------------------------------------------------------------
            extension_start_code: 00 00 01 b5      // should be 00 00 01 B5
            extension_start_code_identifier: 1(0X1)// Should be 1
            profile_and_level_indication: 68(0X44) // Main@High
            progressive_sequence: 0                // the coded video sequence may contain both frame-pictures and field-...
            chroma_format: 1(0X1)                  // 4:2:0
            horizontal_size_extension: 0(0X0)      // (horizontal_size_extension<<12)|horizontal_size_value
            vertical_size_extension: 0(0X0)        // (vertical_size_extension<<12)|vertical_size_value
            bit_rate_extension: 0(0X0)             // (bit_rate_extension<18)|bit_rate_value
            marker_bit: 1
            vbv_buffer_size_extension: 0(0X0)      // (vbv_buffer_size_extension<10)|vbv_buffer_size_value
            low_delay: 0                           // the sequence may contain B-pictures, that the frame re-ordering del...
            frame_rate_extension_n: 0(0X0)         // frame_rate = frame_rate_value * (frame_rate_extension_n + 1) / (fra...
            frame_rate_extension_d: 0(0X0)         // frame_rate = frame_rate_value * (frame_rate_extension_n + 1) / (fra...

```
According to the command `listmse=gop`, we can know how many gops in this MPEG2 video stream, and now we want to show access-units of  the first and the last GOP,
```
DumpTS 00023.m2v --listmse=au.~gop1-119
```
And then
```
------------Name-------------------------------|-----len-----|------------URI-------------
GOP#0 (closed)                                 |             |                       GOP0
    AU#0 (I)                                   |    91,997 B |                   AU0.GOP0
    AU#1 (P)                                   |    83,847 B |                   AU1.GOP0
    AU#2 (B)                                   |    22,773 B |                   AU2.GOP0
    AU#3 (B)                                   |    53,424 B |                   AU3.GOP0
    AU#4 (P)                                   |   111,616 B |                   AU4.GOP0
    AU#5 (B)                                   |    63,292 B |                   AU5.GOP0
    AU#6 (B)                                   |    64,618 B |                   AU6.GOP0
    AU#7 (P)                                   |   131,912 B |                   AU7.GOP0
    AU#8 (B)                                   |    67,947 B |                   AU8.GOP0
    AU#9 (B)                                   |    70,578 B |                   AU9.GOP0
    AU#10 (P)                                  |   140,350 B |                  AU10.GOP0
    AU#11 (B)                                  |    79,321 B |                  AU11.GOP0
    AU#12 (B)                                  |    68,619 B |                  AU12.GOP0
GOP#120 (closed)                               |             |                     GOP120
    AU#0 (I)                                   |   117,313 B |                 AU0.GOP120
```
### NAL bitstream media syntax element locator
*[MSE://][[`~`]**SEIPL**[`sp₀`][`-`][`spₙ`]][.][[`~`]**SEIMSG**[`sm₀`][`-`][`smₙ`]][.]\([[`~`]**NU**[`n₀`][`-`][`nₙ`]] | [[`~`]**VCL**[`v₀`][`-`][`vₙ`]])[.][[`~`]**AU**[`a₀`][`-`][`aₙ`]][.][[`~`]**CVS**[`c₀`][`-`][`cₙ`]][.][[`~`]**VSEQ**[`v₀`][`-`][`vₙ`]][/part/part/...][#field]*

| URI | comment |
| --- | --- |
| AU`50` | The access-unit#50(0-based) |
| NU`50` | The NAL unit#50(0-based) in the whole NAL bitstream |
| SEIMSG`50`| The SEI message#50(0-based) in the whole NAL bitstream|
| SEIPL`50`|The SEI payload#50(0-based) in the whole NAL bitstream|
| SEIPL`0`.AU`50`|The SEI payload#0 in the whole Access-unit#50|
| SEIPL`1`.SEIMSG`0`.NU`50`|The SEI payload#1(0-based) of SEI message#0 of NAL-unit#50 in the whole NAL bitstream|
| SEIPL`0`.SEIMSG`0`.NU`4`.AU`100`|The SEI payload#0 of SEI message#0 of NAL-unit#4 of Access-unit#100 in the whole NAL stream|
| NU`2`.AU`60`|The NAL-unit#2(0-based) of Access-Unit#60 in the whole NAL stream|
| NU`1`/seq_parameter_set_rbsp/vui_parameters|vui_parameters of NAL-unit#1(it is a SPS NU)|
| NU`1`/*/vui_parameters#aspect_ratio_idc |the `aspect_ratio_idc` field of VUI of NAL-unit#1|
|~vcl.au`1`|all VCL NAL units of the 2nd access-unit|
|vcl.au`1-2`|all VCL NAL units of access-unit 1 and 2|

for example,
```
DumpTS 00005.h264 --listmse
```
And all syntax elements with the hierarchical layout will be shown as,
```
------------Name-------------------------------|-----len-----|------------URI----------------------
Video Sequence#0                               |             |                               VSEQ0
    CVS#1 (IDR, closed GOP)                    |             |                          CVS1.VSEQ0
        AU#0 (I)                               |   288,631 B |                      AU0.CVS1.VSEQ0
            NU#0 non-VCL::AUD                  |         2 B |                  NU0.AU0.CVS1.VSEQ0
            NU#1 non-VCL::SPS                  |        51 B |                  NU1.AU0.CVS1.VSEQ0
            NU#2 non-VCL::PPS                  |         5 B |                  NU2.AU0.CVS1.VSEQ0
            NU#3 non-VCL::SEI                  |        15 B |                  NU3.AU0.CVS1.VSEQ0
                SEI message#0                  |        12 B |          SEIMSG0.NU3.AU0.CVS1.VSEQ0
                    #0 buffering_period        |        10 B |   SEIPL0.SEIMSG0.NU3.AU0.CVS1.VSEQ0
            NU#4 non-VCL::SEI                  |        14 B |                  NU4.AU0.CVS1.VSEQ0
                SEI message#0                  |        11 B |          SEIMSG0.NU4.AU0.CVS1.VSEQ0
                    #0 pic_timing              |         9 B |   SEIPL0.SEIMSG0.NU4.AU0.CVS1.VSEQ0
            NU#5 non-VCL::SEI                  |         5 B |                  NU5.AU0.CVS1.VSEQ0
                SEI message#0                  |         3 B |          SEIMSG0.NU5.AU0.CVS1.VSEQ0
                    #0 recovery_point          |         1 B |   SEIPL0.SEIMSG0.NU5.AU0.CVS1.VSEQ0
            NU#6 VCL::IDR                      |    81,092 B |                  NU6.AU0.CVS1.VSEQ0
            NU#7 VCL::IDR                      |    71,634 B |                  NU7.AU0.CVS1.VSEQ0
            NU#8 VCL::IDR                      |    74,525 B |                  NU8.AU0.CVS1.VSEQ0
            NU#9 VCL::IDR                      |    61,255 B |                  NU9.AU0.CVS1.VSEQ0
        AU#1 (P)                               |    81,464 B |                      AU1.CVS1.VSEQ0
            NU#0 non-VCL::AUD                  |         2 B |                  NU0.AU1.CVS1.VSEQ0
            NU#1 non-VCL::SEI                  |        14 B |                  NU1.AU1.CVS1.VSEQ0
                SEI message#0                  |        11 B |          SEIMSG0.NU1.AU1.CVS1.VSEQ0
                    #0 pic_timing              |         9 B |   SEIPL0.SEIMSG0.NU1.AU1.CVS1.VSEQ0
            NU#2 VCL::non-IDR                  |    29,112 B |                  NU2.AU1.CVS1.VSEQ0
            NU#3 VCL::non-IDR                  |    16,360 B |                  NU3.AU1.CVS1.VSEQ0
            NU#4 VCL::non-IDR                  |    19,939 B |                  NU4.AU1.CVS1.VSEQ0
            NU#5 VCL::non-IDR                  |    16,018 B |                  NU5.AU1.CVS1.VSEQ0
        AU#2 (B)                               |    31,964 B |                      AU2.CVS1.VSEQ0
            NU#0 non-VCL::AUD                  |         2 B |                  NU0.AU2.CVS1.VSEQ0
            NU#1 non-VCL::SEI                  |        14 B |                  NU1.AU2.CVS1.VSEQ0
                SEI message#0                  |        11 B |          SEIMSG0.NU1.AU2.CVS1.VSEQ0
                    #0 pic_timing              |         9 B |   SEIPL0.SEIMSG0.NU1.AU2.CVS1.VSEQ0
            NU#2 VCL::non-IDR                  |    11,297 B |                  NU2.AU2.CVS1.VSEQ0
            NU#3 VCL::non-IDR                  |     6,227 B |                  NU3.AU2.CVS1.VSEQ0
            NU#4 VCL::non-IDR                  |     7,861 B |                  NU4.AU2.CVS1.VSEQ0
            NU#5 VCL::non-IDR                  |     6,544 B |                  NU5.AU2.CVS1.VSEQ0
        AU#3 (B)                               |    32,751 B |                      AU3.CVS1.VSEQ0
            NU#0 non-VCL::AUD                  |         2 B |                  NU0.AU3.CVS1.VSEQ0
            NU#1 non-VCL::SEI                  |        14 B |                  NU1.AU3.CVS1.VSEQ0
                SEI message#0                  |        11 B |          SEIMSG0.NU1.AU3.CVS1.VSEQ0
                    #0 pic_timing              |         9 B |   SEIPL0.SEIMSG0.NU1.AU3.CVS1.VSEQ0
            NU#2 VCL::non-IDR                  |    11,903 B |                  NU2.AU3.CVS1.VSEQ0
```

### AV1 bitstream media syntax element locator
*[MSE://][OBU`i`].[FU`j`].[TU`k`]/part/part/...#field*

| URI | comment |
| --- | --- |
| TU`50` | The temporal-unit#50(0-based) |
| FU`50` | The Frame unit#50(0-based) in the whole AV1 bitstream instead of a specified TU |
| OBU`50`| The OBU#50(0-based) in the whole AV1 bitstream instead of a specified TU or/and a specified FU|
| OBU`0`.TU`50`|The first OBU in temporal-unit#50|
| OBU`1`.FU`0`.TU`50`|The 2nd OBU in first the frame-unit of the temporal-unit#50|
| OBU`0`.FU`100`|The 1st OBU of frame-unit#100 in the whole AV1 stream|
| OBU`0`/sequence_header_obu/color_config|color_config in sequence header OBU|
| OBU`0`/sequence_header_obu/color_config#BitDepth|The `BitDepth` field in sequence_header_obu's color config|

### ISOBMFF media syntax element locator
*[MSE://]box`i`].[box`j`]........[box`n`]/sub/object/#field*
Here are some examples:
```
mvhd.moov/#creation_time
stsd.stbl.minf.mdia.track0.moov/AVCSampleEntry#width
//*/*/Projection
```
## Commands
At present, support 3 kinds of command, they are `listMSE` , `showMSE` and `showMSEHex`,

### `listMSE` command
`listMSE` is used to list the media syntax element， if there is no option value for it, all elements with hierarchical layout will be listed, please see the examples in the each byte stream scheme in the previous part, here are more examples:
#### MPEG2 Video

1. ##### List all video sequences
	```
	DumpTS 00023.m2v --listmse=vseq
	```
	And then,
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	Video Sequence#0                               |             |                      VSEQ0
	```
2. #### List all GOPs
	```
	DumpTS 00023.m2v --listmse=gop
	```
	And then,
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	GOP#0 (closed)                                 |             |                       GOP0
	GOP#1 (open)                                   |             |                       GOP1
	GOP#2 (open)                                   |             |                       GOP2
	GOP#3 (open)                                   |             |                       GOP3
	GOP#4 (open)                                   |             |                       GOP4
	GOP#5 (open)                                   |             |                       GOP5
	GOP#6 (open)                                   |             |                       GOP6
	GOP#7 (open)                                   |             |                       GOP7
	GOP#8 (open)                                   |             |                       GOP8
	GOP#9 (open)                                   |             |                       GOP9
	GOP#10 (open)                                  |             |                      GOP10
	GOP#11 (open)                                  |             |                      GOP11
	GOP#12 (open)                                  |             |                      GOP12
	GOP#13 (open)                                  |             |                      GOP13
	GOP#14 (open)                                  |             |                      GOP14
	GOP#15 (open)                                  |             |                      GOP15
	GOP#16 (open)                                  |             |                      GOP16
	...
	```
3. ### List all access units
	```
	DumpTS 00023.m2v --listmse=au
	```
	And then,
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	AU#0 (I)                                       |    91,997 B |                        AU0
	AU#1 (P)                                       |    83,847 B |                        AU1
	AU#2 (B)                                       |    22,773 B |                        AU2
	AU#3 (B)                                       |    53,424 B |                        AU3
	AU#4 (P)                                       |   111,616 B |                        AU4
	AU#5 (B)                                       |    63,292 B |                        AU5
	AU#6 (B)                                       |    64,618 B |                        AU6
	AU#7 (P)                                       |   131,912 B |                        AU7
	AU#8 (B)                                       |    67,947 B |                        AU8
	AU#9 (B)                                       |    70,578 B |                        AU9
	AU#10 (P)                                      |   140,350 B |                       AU10
	AU#11 (B)                                      |    79,321 B |                       AU11
	AU#12 (B)                                      |    68,619 B |                       AU12
	AU#13 (I)                                      |   282,790 B |                       AU13
	AU#14 (B)                                      |    53,920 B |                       AU14
	AU#15 (B)                                      |    52,587 B |                       AU15
	AU#16 (P)                                      |   107,529 B |                       AU16
	AU#17 (B)                                      |    48,750 B |                       AU17
	...
	```
4. ### List all MPEG2 syntactic elements
	```
	DumpTS 00023.m2v --listmse=se
	```
	And then,
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	SE#0 sequence_header                           |       140 B |                        SE0
	SE#1 sequence_extension                        |        10 B |                        SE1
	SE#2 group_of_pictures_header                  |         8 B |                        SE2
	SE#3 picture_header                            |         8 B |                        SE3
	SE#4 picture_coding_extension                  |         9 B |                        SE4
	SE#5 slice1                                    |       688 B |                        SE5
	SE#6 slice2                                    |       692 B |                        SE6
	SE#7 slice3                                    |       698 B |                        SE7
	SE#8 slice4                                    |       714 B |                        SE8
	SE#9 slice5                                    |       831 B |                        SE9
	SE#10 slice6                                   |     1,691 B |                       SE10
	SE#11 slice7                                   |     1,798 B |                       SE11
	SE#12 slice8                                   |     1,730 B |                       SE12
	SE#13 slice9                                   |     1,747 B |                       SE13
	SE#14 slice10                                  |     1,746 B |                       SE14
	SE#15 slice11                                  |     1,714 B |                       SE15
	SE#16 slice12                                  |     1,723 B |                       SE16
	SE#17 slice13                                  |     1,762 B |                       SE17
	...
	```
5. #### List all slices
	```
	DumpTS 00023.m2v --listmse=slice
	```
	And then,
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	Slice#0 slice1                                 |       688 B |                     SLICE0
	Slice#1 slice2                                 |       692 B |                     SLICE1
	Slice#2 slice3                                 |       698 B |                     SLICE2
	Slice#3 slice4                                 |       714 B |                     SLICE3
	Slice#4 slice5                                 |       831 B |                     SLICE4
	Slice#5 slice6                                 |     1,691 B |                     SLICE5
	Slice#6 slice7                                 |     1,798 B |                     SLICE6
	Slice#7 slice8                                 |     1,730 B |                     SLICE7
	Slice#8 slice9                                 |     1,747 B |                     SLICE8
	Slice#9 slice10                                |     1,746 B |                     SLICE9
	Slice#10 slice11                               |     1,714 B |                    SLICE10
	Slice#11 slice12                               |     1,723 B |                    SLICE11
	Slice#12 slice13                               |     1,762 B |                    SLICE12
	Slice#13 slice14                               |     1,846 B |                    SLICE13
	Slice#14 slice15                               |     1,824 B |                    SLICE14
	Slice#15 slice16                               |     1,842 B |                    SLICE15
	Slice#16 slice17                               |     1,792 B |                    SLICE16
	Slice#17 slice18                               |     1,779 B |                    SLICE17
	Slice#18 slice19                               |     1,769 B |                    SLICE18
	Slice#19 slice20                               |     1,791 B |                    SLICE19
	...
	```
6. #### List all non-slices syntactic elements
	```
	DumpTS 00023.m2v --listmse=~slice
	```
	And then,
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	SE#0 sequence_header                           |       140 B |                        SE0
	SE#1 sequence_extension                        |        10 B |                        SE1
	SE#2 group_of_pictures_header                  |         8 B |                        SE2
	SE#3 picture_header                            |         8 B |                        SE3
	SE#4 picture_coding_extension                  |         9 B |                        SE4
	SE#73 picture_header                           |         9 B |                       SE73
	SE#74 picture_coding_extension                 |         9 B |                       SE74
	SE#143 picture_header                          |         9 B |                      SE143
	SE#144 picture_coding_extension                |         9 B |                      SE144
	SE#213 picture_header                          |         9 B |                      SE213
	SE#214 picture_coding_extension                |         9 B |                      SE214
	SE#283 picture_header                          |         9 B |                      SE283
	SE#284 picture_coding_extension                |         9 B |                      SE284
	SE#353 picture_header                          |         9 B |                      SE353
	SE#354 picture_coding_extension                |         9 B |                      SE354
	SE#423 picture_header                          |         9 B |                      SE423
	SE#424 picture_coding_extension                |         9 B |                      SE424
	SE#493 picture_header                          |         9 B |                      SE493
	SE#494 picture_coding_extension                |         9 B |                      SE494
	...
	```
7. #### List all macro-blocks
	Not supported at present
8. #### List all GOPs in a specified video sequence
	```
	DumpTS 00023.m2v --listmse=gop.vseq0
	```
	And then
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	Video Sequence#0                               |             |                      VSEQ0
	    GOP#0 (closed)                             |             |                 GOP0.VSEQ0
	    GOP#1 (open)                               |             |                 GOP1.VSEQ0
	    GOP#2 (open)                               |             |                 GOP2.VSEQ0
	    GOP#3 (open)                               |             |                 GOP3.VSEQ0
	    GOP#4 (open)                               |             |                 GOP4.VSEQ0
	    GOP#5 (open)                               |             |                 GOP5.VSEQ0
	    GOP#6 (open)                               |             |                 GOP6.VSEQ0
	    GOP#7 (open)                               |             |                 GOP7.VSEQ0
	    GOP#8 (open)                               |             |                 GOP8.VSEQ0
	    GOP#9 (open)                               |             |                 GOP9.VSEQ0
	    GOP#10 (open)                              |             |                GOP10.VSEQ0
	    GOP#11 (open)                              |             |                GOP11.VSEQ0
	    GOP#12 (open)                              |             |                GOP12.VSEQ0
	    GOP#13 (open)                              |             |                GOP13.VSEQ0
	...
	```
9. #### List all access-units in a specified GOP
	```
	DumpTS 00023.m2v --listmse=au.gop2
	```
	And then, all access-units of the 3rd GOP will be listed
	```
	------------Name-------------------------------|-----len-----|------------URI-------------
	GOP#2 (open)                                   |             |                       GOP2
	    AU#0 (I)                                   |   361,074 B |                   AU0.GOP2
	    AU#1 (B)                                   |    53,397 B |                   AU1.GOP2
	    AU#2 (B)                                   |    50,591 B |                   AU2.GOP2
	    AU#3 (P)                                   |   117,745 B |                   AU3.GOP2
	    AU#4 (B)                                   |    50,194 B |                   AU4.GOP2
	    AU#5 (B)                                   |    48,185 B |                   AU5.GOP2
	    AU#6 (P)                                   |   132,027 B |                   AU6.GOP2
	    AU#7 (B)                                   |    57,506 B |                   AU7.GOP2
	    AU#8 (B)                                   |    61,382 B |                   AU8.GOP2
	    AU#9 (P)                                   |   140,204 B |                   AU9.GOP2
	    AU#10 (B)                                  |    61,736 B |                  AU10.GOP2
	    AU#11 (B)                                  |    63,833 B |                  AU11.GOP2
	    AU#12 (P)                                  |   147,079 B |                  AU12.GOP2
	    AU#13 (B)                                  |    65,729 B |                  AU13.GOP2
	    AU#14 (B)                                  |    67,136 B |                  AU14.GOP2
	...
	```
10. #### List some syntax elements in a or some specified GOP(s) or/and Access Unit(s)
	```
	DumpTS 00023.m2v --listmse=se1-4.au1-2.gop2-3
	```
	And then, the #1,#2,#3 and #4(0-based)syntax elements of the #2 and #3(0-based) access-units of GOP#2 and #3(0-based) will be listed,
	```
	GOP#2 (open)                                   |             |                       GOP2
	    AU#1 (B)                                   |    53,397 B |                   AU1.GOP2
	        SE#1 picture_coding_extension          |         9 B |               SE1.AU1.GOP2
	        SE#2 slice1                            |       515 B |               SE2.AU1.GOP2
	        SE#3 slice2                            |       405 B |               SE3.AU1.GOP2
	        SE#4 slice3                            |       382 B |               SE4.AU1.GOP2
	    AU#2 (B)                                   |    50,591 B |                   AU2.GOP2
	        SE#1 picture_coding_extension          |         9 B |               SE1.AU2.GOP2
	        SE#2 slice1                            |       235 B |               SE2.AU2.GOP2
	        SE#3 slice2                            |       225 B |               SE3.AU2.GOP2
	        SE#4 slice3                            |       246 B |               SE4.AU2.GOP2
	GOP#3 (open)                                   |             |                       GOP3
	    AU#1 (B)                                   |    56,516 B |                   AU1.GOP3
	        SE#1 picture_coding_extension          |         9 B |               SE1.AU1.GOP3
	        SE#2 slice1                            |       298 B |               SE2.AU1.GOP3
	        SE#3 slice2                            |       273 B |               SE3.AU1.GOP3
	        SE#4 slice3                            |       300 B |               SE4.AU1.GOP3
	    AU#2 (B)                                   |    54,804 B |                   AU2.GOP3
	        SE#1 picture_coding_extension          |         9 B |               SE1.AU2.GOP3
	        SE#2 slice1                            |       126 B |               SE2.AU2.GOP3
	        SE#3 slice2                            |       105 B |               SE3.AU2.GOP3
	        SE#4 slice3                            |       145 B |               SE4.AU2.GOP3
	```
```
DumpTS xxxx.h264 --listMSE=MSE://AU
```
And it may show the below output

```
...
---------Name---------------|--URI---|
Access-Unit#123	            |  AU123 |
...
```
List AU/NU tree
```
DumpTS xxxxx.h264 --listMSE=NU.AU
```
And then
```
----------Name--------------|----len----|----URI-----|------------Description-------------------
...
Access-Unit#84              |           | AU84     | Access unit delimiter
    NAL Unit#0 non-VCL::AUD |       2 B | NU0.AU84   | Sequence parameter set
    NAL Unit#1 non-VCL::SPS |      51 B | NU1.AU84   | Picture parameter set
    NAL Unit#2 non-VCL::PPS |       6 B | NU2.AU84   | Supplemental enhancement information(SEI)
    NAL Unit#3 non-VCL::SEI |      14 B | NU3.AU84   | Supplemental enhancement information(SEI)
    NAL Unit#4 non-VCL::SEI |      14 B | NU4.AU84   | Supplemental enhancement information(SEI)
    NAL Unit#5 non-VCL::SEI |       5 B | NU5.AU84   | Supplemental enhancement information(SEI)
    NAL Unit#6 VCL::IDR     | 113,827 B | NU6.AU84   | Coded slice of an IDR picture
    NAL Unit#7 VCL::IDR     | 114,431 B | NU7.AU84   | Coded slice of an IDR picture
    NAL Unit#8 VCL::IDR     |  94,709 B | NU8.AU84   | Coded slice of an IDR picture
    NAL Unit#9 VCL::IDR     |  75,413 B | NU9.AU84   | Coded slice of an IDR picture
...
```
List TU/FR/OBU tree
```
DumpTS xxxxx.av1 --listMSE=OBU.FU.TU
```
And then
```
----Name-------------------|--len---|------URI------|obu_size|OBU start|
...
Temporal Unit#41           |  xxx B |          TU41 |        |         |
    Frame Unit#0           |  xxx B |      FU0.TU41 |        |         |
        OBU#0 Frame OBU    |  580 B | OBU0.FU0.TU41 |  579 B | OBU100  |
    Frame Unit#1           |  xxx B |      FU1.TU41 |        |         |
        OBU#0: Frame OBU   |  439 B | OBU0.FU1.TU41 |  438 B | OBU101  |
    Frame Unit#2           |  xxx B |      FU2.TU41 |        |         |
        OBU#0: Frame OBU   |  305 B | OBU0.FU2.TU41 |  304 B | OBU102  |
...
```

List ISOBMFF boxes
```
DumpTS xxxxx.mp4 --listMSE
```
And then
```
  --------------Box name-----------------------------|---len----|-----Description-----------------
  .
  |--ftyp                                            |          | File Type Box
  |--free                                            |          | Free Space Box
  |--mdat                                            |          | Media Data Box
  |--moov                                            |          | Movie Box
       |--mvhd                                       |          | Movie Header Box
       |--trak -- track_ID: 1, duration: 8.008s      |          | Track Box
       |    |--tkhd                                  |          | 
       |    |--edts                                  |          | 
       |    |    |--elst                             |          | 
       |    |--mdia                                  |          | 
       |         |--mdhd                             |          | 
       |         |--hdlr -- Video track              |          | 
       |         |--minf                             |          | 
       |              |--vmhd                        |          | 
       |              |--dinf                        |          | 
       |              |    |--dref                   |          | 
       |              |--stbl                        |          | 
       |                   |--stsd -- avc1@1920x1080 |          | 
       |                   |    |--avc1              |          | 
       |                   |         |--avcC         |          | 
       |                   |         |--pasp         |          | 
       |                   |--stts                   |          | 
       |                   |--stss                   |          | 
       |                   |--ctts                   |          | 
       |                   |--stsc                   |          | 
       |                   |--stsz                   |          | 
       |                   |--stco                   |          | 
       |--trak -- track_ID: 2, duration: 8.054s      |          | 
       |    |--tkhd                                  |          | 
       |    |--edts                                  |          | 
       |    |    |--elst                             |          | 
       |    |--mdia                                  |          | 
       |         |--mdhd                             |          | 
       |         |--hdlr -- Audio track              |          | 
       |         |--minf                             |          | 
       |              |--smhd                        |          | 
       |              |--dinf                        |          | 
       |              |    |--dref                   |          | 
       |              |--stbl                        |          | 
       |                   |--stsd -- mp4a@48000HZ   |          | 
       |                   |    |--mp4a              |          | 
       |                   |         |--esds         |          | 
       |                   |--stts                   |          | 
       |                   |--stsc                   |          | 
       |                   |--stsz                   |          | 
       |                   |--stco                   |          | 
       |--udta                                       |          | 
            |--meta                                  |          | 
                 |--hdlr                             |          | 
                 |--ilst                             |          |
```
Show a part of ISOBMFF tree
```
DumpTS xxxxx.mp4 --listMSE=minf.mdia.trak0.moov
```
And then,
```
moov.trak[0].mdia.minf 
  ------- Box Name --------------|---len----|-----Description-----------------
  |--vmhd                        |          | 
  |--dinf                        |          | 
  |    |--dref                   |          | 
  |--stbl                        |          | 
       |--stsd -- avc1@1920x1080 |          | 
       |    |--avc1              |          | 
       |         |--avcC         |          | 
       |         |--pasp         |          | 
       |--stts                   |          | 
       |--stss                   |          | 
       |--ctts                   |          | 
       |--stsc                   |          | 
       |--stsz                   |          | 
       |--stco                   |          | 
  
```

### `showMSE` command

[Top](#contents)
### `showMSEHex` command
Show the located media syntax element buffer w/o any modification and parsing, for example,
```
DumpTS 00023.m2v --showmsehex=slice46.au11.gop15
```
And then the 46th slice of access-unit #11 of the 16th GOP buffer will be printed as,
```
------------Name-------------------------------|-----len-----|------------URI-------------
GOP#15 (closed)                                 |             |                       GOP0
    AU#11 (B)                                  |    79,321 B |                  AU11.GOP0
        Slice#46 slice47                       |       993 B |          SLICE46.AU11.GOP0
        ----------------------------------------------------------------------------------
                 00  01  02  03  04  05  06  07    08  09  0A  0B  0C  0D  0E  0F
                 ----------------------------------------------------------------
         000000  00  00  01  2F  1A  73  0B  A1    75  74  E6  AA  AA  AE  9C  66 | .../.s..ut.....f
         000010  88  42  06  8E  A1  3E  A3  2B    8D  51  FD  DC  BA  1C  32  18 | .B...>.+.Q....2.
         000020  59  3A  AA  E8  70  68  D9  45    52  28  EB  A7  58  C0  75  55 | Y:..ph.ER(..X.uU
         000030  D3  99  8B  84  4D  D7  22  EE    9C  63  07  AA  8E  BA  1C  12 | ....M."..c......
         000040  34  7D  A4  69  0A  90  4F  1F    43  85  63  05  BD  1D  47  3D | 4}.i..O.C.c...G=
         000050  D0  E0  B3  2C  3A  A8  2B  C8    75  D9  B0  AA  1C  2E  E3  2A | ...,:.+.u......*
         000060  BD  0E  0B  54  01  CC  51  C3    D7  4B  C9  57  D2  0F  35  DE | ...T..Q..K.W..5.
         000070  4D  39  A2  E5  F1  4B  14  17    20  BF  86  A4  E5  56  ED  4C | M9...K.. ....V.L
         000080  E2  83  D8  0D  5D  39  A2  30    24  D3  24  30  C3  0F  83  07 | ....]9.0$.$0....
         000090  85  48  7E  FB  66  5B  9F  31    80  03  BA  1C  1F  0A  09  52 | .H~.f[.1.......R
         0000A0  C5  B5  0E  19  8D  B4  2A  8E    A7  10  E8  9D  4E  59  F5  35 | ......*.....NY.5
         0000B0  48  F4  E6  33  2A  36  A9  C6    34  0A  9E  11  57  43  83  43 | H..3*6..4...WC.C
         0000C0  45  BA  E4  09  91  BE  90  E7    EB  A1  C3  67  54  CD  4E  77 | E..........gT.Nw
         0000D0  50  E0  FE  8A  33  30  7D  71    54  75  29  0E  1C  33  19  F5 | P...30}qTu)..3..
         0000E0  53  88  59  3E  7A  A5  31  5E    1C  13  16  D0  51  16  46  03 | S.Y>z.1^....Q.F.
         0000F0  E9  1E  20  83  46  75  98  B8    30  E0  11  D9  D4  38  54  2C | .. .Fu..0....8T,
         000100  BA  AA  A7  28  82  38  EA  85    DD  0E  0E  CF  41  CC  46  AE | ...(.8......A.F.
         000110  73  69  B7  CC  46  F2  28  70    B8  25  55  74  38  26  2C  98 | si..F.(p.%Ut8&,.
         000120  51  3E  46  09  E6  88  61  D2    0E  0F  16  B8  30  F1  E4  F0 | Q>F...a.....0...
         000130  39  77  43  86  42  9C  16  F0    BA  1C  2A  18  DD  43  EE  87 | 9wC.B.....*..C..
         000140  0C  CD  38  80  BA  7A  71  0B    A7  D8  DA  A1  C1  FE  90  27 | ..8..zq........'
         000150  38  80  3B  4E  F2  14  6F  20    D1  43  68  CB  F0  17  57  36 | 8.;N..o .Ch...W6
         000160  31  EA  87  06  C7  E4  EC  A3    FC  19  9D  88  CB  54  81  CE | 1............T..
         000170  4F  54  53  12  CE  CB  34  FF    45  0C  86  0D  E4  DA  DE  46 | OTS...4.E......F
         000180  3E  33  53  8C  6F  A5  1D  E9    A9  C4  38  D2  E9  CF  D5  54 | >3S.o.....8....T
         000190  BA  71  0B  A3  35  5D  0E  0E    AF  43  6A  C0  35  90  66  3A | .q..5]...Cj.5.f:
         0001A0  41  B4  E6  EA  80  59  54  38    66  3F  55  0F  AA  1C  1E  F9 | A....YT8f?U.....
         0001B0  DE  28  FA  63  A9  C6  35  1C    3E  3B  9E  B8  30  06  53  E1 | .(.c..5.>;..0.S.
         0001C0  21  57  43  86  43  EB  97  4D    E5  5E  51  03  A2  09  B1  56 | !WC.C..M.^Q....V
         0001D0  56  36  5B  DD  39  9F  55  39    C2  AA  55  88  A0  ED  11  84 | V6[.9.U9..U.....
         0001E0  53  14  51  D3  2C  07  F2  30    31  00  0C  86  5F  D7  74  38 | S.Q.,..01..._.t8
         0001F0  6A  71  80  67  3D  0E  16  CF    8C  7A  AE  B0  35  E5  9C  41 | jq.g=....z..5..A
         000200  A2  83  EC  98  57  C4  93  43    9A  0C  64  B1  6C  78  9F  9E | ....W..C..d.lx..
         000210  42  86  BE  60  A8  B6  47  1A    A2  3D  DD  0E  1F  75  47  AE | B..`..G..=...uG.
         000220  5D  0E  1B  55  50  A9  15  4E    68  AA  42  A4  5D  39  AA  A0 | ]..UP..Nh.B.]9..
         000230  19  D4  80  19  D5  14  3A  13    21  5E  8C  C9  F2  A8  A1  98 | ......:.!^......
         000240  BA  42  8C  6D  26  87  06  F5    55  6B  5B  2C  3E  43  FE  06 | .B.m&...Uk[,>C..
         000250  AC  E4  25  64  0A  77  7B  A7    16  BA  A8  92  A9  82  24  63 | ..%d.w{.......$c
         000260  E2  8B  5A  AE  87  0C  84  BC    EA  F4  38  36  7F  1E  03  FA | ..Z.......86....
         000270  36  80  67  42  89  EB  A1  C3    21  CE  32  A9  CF  D5  55  79 | 6.gB....!.2...Uy
         000280  44  C4  17  6F  36  35  80  07    50  BA  71  AD  95  48  A7  12 | D..o65..P.q..H..
         000290  AF  3A  A9  CB  E1  28  91  EA    E9  DB  2C  AA  A8  70  6C  FD | .:...(....,..pl.
         0002A0  14  6C  53  03  08  00  C5  05    D3  88  60  F4  02  F5  55  D0 | .lS.......`...U.
         0002B0  E1  D0  F6  A1  C1  A1  67  AC    7C  87  1A  07  24  2F  35  1A | ......g.|...$/5.
         0002C0  06  A8  70  D4  67  55  0E  1D    AE  35  50  E1  91  B3  EA  A8 | ..p.gU...5P.....
         0002D0  8F  43  83  42  8F  44  57  A7    02  5D  11  4F  B9  9A  9C  63 | .C.B.DW..].O...c
         0002E0  E8  8A  DE  28  9E  4E  91  4E    21  67  A2  47  F2  1C  E9  14 | ...(.N.N!g.G....
         0002F0  E3  15  FD  22  3C  0E  48  0E    7A  72  EF  47  90  7B  C8  1F | ..."<.H.zr.G.{..
         000300  14  7D  3B  57  A2  54  89  14    E2  37  7D  22  2A  A9  C6  EC | .};W.T...7}"*...
         000310  F4  1F  22  A9  74  E0  9D  7E    83  4F  A2  87  CC  B1  74  65 | ..".t..~.O....te
         000320  38  C7  E5  C8  86  81  AA  A1    C3  5D  CA  87  06  86  84  48 | 8........].....H
         000330  91  57  4E  02  DE  1F  F4  48    1D  22  41  F5  C1  18  1A  60 | .WN....H."A....`
         000340  E1  A0  4A  36  2C  75  53  AE    55  63  DD  53  A4  B6  26  EA | ..J6,uS.Uc.S..&.
         000350  9D  1D  BA  2F  DF  81  FC  00    69  D3  38  33  AC  74  EB  91 | .../....i.83.t..
         000360  03  FF  FF  D3  8D  7E  8A  11    22  45  38  87  24  55  38  0F | .....~.."E8.$U8.
         000370  E5  EE  3E  9D  6F  F5  54  8A    1D  11  3F  94  64  8F  20  F8 | ..>.o.T...?.d. .
         000380  99  E9  D7  7E  88  81  15  38    5D  38  DC  12  FD  0F  15  9A | ...~...8]8......
         000390  44  D5  C1  38  19  2A  36  A3    7A  6F  40  2E  A7  53  D0  88 | D..8.*6.zo@..S..
         0003A0  C8  32  2B  AA  E9  CF  86  1F    20  93  21  C4  3C  56  23  D7 | .2+..... .!.<V#.
         0003B0  02  76  AC  90  E4  18  D0  37    06  99  25  8F  17  67  53  9F | .v.....7..%..gS.
         0003C0  A0  C4  26  47  90  68  8A  71    1D  08  89  23  E4  E4  D9  F7 | ..&G.h.q...#....
         0003D0  43  83  66  CC  25  D5  3A  D5    10  2D  22  AE  A9  D4  B8  3D | C.f.%.:..-"....=
         0003E0  F8                                                               | .

```

[Top](#contents)
