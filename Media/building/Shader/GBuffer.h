/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

//#include "Rendering.h"

//--------------------------------------------------------------------------------------
// GBuffer and related common utilities and structures
struct GBuffer
{
    vec4 normal_specular;
    vec4 albedo;
    vec2 positionZGrad;
};

vec2 EncodeSphereMap(vec3 n)
{
    float oneMinusZ = 1.0 - n.z;
    float p = sqrt(n.x * n.x + n.y * n.y + oneMinusZ * oneMinusZ);
    return n.xy / p * 0.5 + 0.5;
}

vec3 DecodeSphereMap(vec2 e)
{
    vec2 tmp = e - e * e;
    float f = tmp.x + tmp.y;
    float m = sqrt(4.0 * f - 1.0);
    
    vec3 n;
    n.xy = m * (e * 4.0 - 2.0);
    n.z  = 3.0 - 8.0 * f;
    return n;
}

vec3 ComputePositionViewFromZ(vec2 positionScreen,
                                float viewSpaceZ)
{
    vec2 screenSpaceRay = vec2( positionScreen.x / Projection[0][0],
                                positionScreen.y / Projection[1][1] );
    
    vec3 positionView;
    positionView.z = viewSpaceZ;
    // Solve the two projection equations
    positionView.xy = screenSpaceRay.xy * positionView.z;
    
    return positionView;
}

uniform sampler2D gGBufferDiffuse;
uniform sampler2D gGBufferNormal;
uniform sampler2D gGBufferLightMap;
//uniform sampler2D gDepthBuffer;

SurfaceData ComputeSurfaceDataFromGBufferSample( ivec2 positionViewport )
{
    SurfaceData data;
    
    data.albedo = texelFetch( gGBufferDiffuse, positionViewport.xy, 0 );
    vec2 PackedNormal = texelFetch(gGBufferNormal, positionViewport.xy, 0 ).xy;
    data.normal   = DecodeSphereMap( PackedNormal );
    data.lightMap = texelFetch(gGBufferLightMap, positionViewport.xy, 0 );
            
    data.specularAmount = 0.9;
    data.specularPower = 15.0;

    vec2 gbufferDim = vec2(textureSize(gGBufferDiffuse, 0));
    
    // Compute screen/clip-space position and neighbour positions
    // NOTE: Mind DX11 viewport transform and pixel center!
    // NOTE: This offset can actually be precomputed on the CPU but it's actually slower to read it from
    // a constant buffer than to just recompute it.
    // In OpenGL y axis goes up on the screen
    vec2 screenPixelOffset = vec2(2.0, 2.0) / gbufferDim;
    vec2 positionScreen = (vec2(positionViewport.xy) + 0.5) * screenPixelOffset.xy + vec2(-1.0, -1.0);
        
    //float Depth = texelFetch( gDepthBuffer, ivec2(positionViewport.xy), 0 ).x;
    // Unproject depth buffer Z value into view space
    //float viewSpaceZ = Projection[2][3] / (Depth - Projection[2][2]);
    float viewSpaceZ = data.albedo.w; // View space Z reconstructed from depth is incorrect now for some reason

    data.positionView.xyz = ComputePositionViewFromZ(positionScreen, viewSpaceZ);
  
    return data;
}

#endif // GBUFFER_HLSL
