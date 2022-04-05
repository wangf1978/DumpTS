# Utility operation guideline
## Contents
* [Other utilities](#other-utilities)
* [CRC](#crc)

[Return to Main](../README.md)
## Other utilities
```
DumpTs AACScalefactorHuffmanCodebook.txt --VLCTypes=aah --srcfmt=huffman_codebook
```
Load huffman-codebook from the specified file, and print its huffman-tree
```
DumpTs AACScalefactorHuffmanCodebook.txt --VLCTypes=aah --srcfmt=huffman_codebook --outputfmt=binary_search_table
```

[Top](#contents)
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

[Top](#contents)
## CRC
### list CRC algorithms
```
DumpTS --listcrc
```
All supported CRC algorithms are supported,
```
Name                        Polynomial              Init-value
---------------------------------------------------------------
crc-8                       0X07                    0X00
crc-8-darc                  0X39                    0X00
crc-8-i-code                0X1D                    0XFD
crc-8-itu                   0X07                    0X55
crc-8-maxim                 0X31                    0X00
crc-8-rohc                  0X07                    0XFF
crc-8-wcdma                 0X9B                    0X00
crc-16                      0X8005                  0X0000
crc-16-buypass              0X8005                  0X0000
crc-16-dds-110              0X8005                  0X800D
crc-16-dect                 0X0589                  0X0001
crc-16-dnp                  0X3D65                  0XFFFF
crc-16-en-13757             0X3D65                  0XFFFF
crc-16-genibus              0X1021                  0X0000
crc-16-maxim                0X8005                  0XFFFF
crc-16-mcrf4xx              0X1021                  0XFFFF
crc-16-riello               0X1021                  0X554D
crc-16-t10-dif              0X8BB7                  0X0000
crc-16-teledisk             0XA097                  0X0000
crc-16-usb                  0X8005                  0X0000
x-25                        0X1021                  0X0000
xmodem                      0X1021                  0X0000
modbus                      0X8005                  0XFFFF
kermit                      0X1021                  0X0000
crc-ccitt-false             0X1021                  0XFFFF
crc-aug-ccitt               0X1021                  0X1D0F
crc-24                      0X864CFB                0XB704CE
crc-24-flexray-a            0X5D6DCB                0XFEDCBA
crc-24-flexray-b            0X5D6DCB                0XABCDEF
crc-32                      0X04C11DB7              0XFFFFFFFF
crc-32-bzip2                0X04C11DB7              0XFFFFFFFF
crc-32c                     0X1EDC6F41              0X00000000
crc-32d                     0XA833982B              0X00000000
crc-32-mpeg                 0X04C11DB7              0XFFFFFFFF
posix                       0X04C11DB7              0XFFFFFFFF
crc-32q                     0X814141AB              0X00000000
jamcrc                      0X04C11DB7              0XFFFFFFFF
xfer                        0X000000AF              0X00000000
crc-64                      0X000000000000001B      0X0000000000000000
crc-64-we                   0X42F0E1EBA9EA3693      0X0000000000000000
crc-64-jones                0XAD93D23594C935A9      0XFFFFFFFFFFFFFFFF
```
### Calculate the crc value of a file
```
DumpTS pmt_section.psi --crc=crc-32
```
it will output the crc-32 value of a PMT section.

[Top](#contents)
