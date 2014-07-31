// Copyright 2014 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#pragma once

#include <vector>
#include <cstdint>

using std::vector;

struct LightGridEntry
{
    uint32_t sizeAndLink; // uint8 size, uint24 link
    uint32_t lightIndex[3];
};

struct LightGridDimensions
{
    int width;
    int height;
    int depth;

    LightGridDimensions(int _width, int _height, int _depth)
        : width(_width), height(_height), depth(_depth) {}

    // cell: 4x4x4 entries or 2x2x1 packed entries
    int cellIndex(int x, int y, int z)
    {
        assert(((width | height | depth) % 4 == 0) && "dimensions must be cell-aligned");
        assert(x >= 0 && y >= 0 && z >= 0);
        assert(x < width / 4 && y < height / 4 && z < depth / 4);

        return (y*width / 4 + x)*depth / 4 + z;
    }
};

class LightGridBuilder
{
public:
    LightGridBuilder();

    // packedCells: 2x2x1 packed entries per cell, with 16-bit coverage mask
    void reset(LightGridDimensions dim);
    LightGridDimensions dimensions();

    void clearAllFragments();
    void pushFragment(int cellIndex, int lightIndex, uint64_t coverage);
    void buildAndUpload(void* gpuBuffer, size_t bufferSize);
    
private:
    LightGridDimensions dim;
    size_t cellCount();

    // cell grid
    vector<vector<int>> lightIndexLists;
    vector<vector<uint64_t>> coverageLists;

    uint8_t* pDstBuffer;
    size_t allocatedBytes;
    void buildFlatEntries(int x, int y, int z);
};
