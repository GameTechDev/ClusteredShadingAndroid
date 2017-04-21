/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
