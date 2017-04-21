/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

//#include "GPUQuad.h"

// This is a "deferred lighting" implementation of GPU quad which reduces the bandwidth
// required during the accumulation passes by accumulating diffuse and (monchromatic) specular
// components only during the lighting phase.

// Monochromatic specular color implemented as suggested by Naty Hoffman at
//   http://www.realtimerendering.com/blog/deferred-lighting-approaches/

uniform sampler2D gGBufferTextures[4];
uniform sampler2D gLight;
uniform sampler2D gDeferredLightingAccumTexture;

in vec4 gl_FragCoord;

in GPUQuadVSOutBlock
{
    flat uint lightIndex;
}Input;

out vec4 out_Color;


// Resolve separate diffuse/specular components stage
void main()
{
    // Read surface data and accumulated light data
    ivec2 coords = ivec2(gl_FragCoord.xy);
    SurfaceData surface = ComputeSurfaceDataFromGBufferSample(coords);
    vec4 accumulated = texelFetch( gDeferredLightingAccumTexture, coords, 0 );

    vec3 lit = vec3(0.0, 0.0, 0.0);

    //[branch] 
    if (mUI.visualizeLightCount != 0u) 
    {
        lit = accumulated.xxx;
    } 
    else 
    {
        // Resolve accumulated lighting
        float diffuseLum = RGBToLuminance(accumulated.xyz);

        // Prevent divide by zero
        const float epsilon = 0.000001;
        lit = surface.albedo.xyz * (accumulated.xyz + surface.specularAmount * accumulated.xyz * (accumulated.w / (diffuseLum + epsilon)));
    }
    
    out_Color = vec4(lit, 1.0);
}
