# Utility operation guideline

## Other utilities
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
