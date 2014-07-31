// Copyright 2010 Intel Corporation
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
