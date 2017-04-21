/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FRAMEBUFFER_FLAT_HLSL
#define FRAMEBUFFER_FLAT_HLSL

// - RGBA 16-bit per component packed into a uint2 per texel
vec4 UnpackRGBA16(uvec2 e)
{
    return vec4( unpackHalf2x16(e.x), unpackHalf2x16(e.y) );
}

uvec2 PackRGBA16(vec4 c)
{
    return uvec2( packHalf2x16(c.rg),  packHalf2x16(c.ba) );
}

// Linearize the given 2D address + sample index into our flat framebuffer array
uint GetFramebufferSampleAddress(uvec2 coords)
{
    // Major ordering: Row (x), Col (y), MSAA sample
    return coords.y * mFramebufferDimensions.x + coords.x;
}

#endif // FRAMEBUFFER_FLAT_HLSL
