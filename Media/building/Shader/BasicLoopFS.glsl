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

//#include "GBuffer.h"

uniform sampler2D gLightsBuffer;

//--------------------------------------------------------------------------------------

in vec4 gl_FragCoord;
out vec4 out_Color;

void main()
{
    // How many total lights?
    uint totalLights = uint(textureSize(gLightsBuffer, 0).x);
    
    vec3 lit = vec3(0.0, 0.0, 0.0);
    //[branch] 
    if (mUI.visualizeLightCount != 0u) 
    {
        lit = vec3(float(totalLights) / 255.0);
    } 
    else 
    {
        vec2 CameraNearFar = GetNearFarFromProjMatr(Projection);
        SurfaceData surface = ComputeSurfaceDataFromGBufferSample( ivec2(gl_FragCoord.xy) );
        // Avoid shading skybox/background pixels
        lit = surface.lightMap.rgb;
        if (surface.positionView.z < CameraNearFar.x) 
        {
            for (uint LightInd = 0u; LightInd < totalLights; ++LightInd) 
            {
                PointLight light = LoadLightAttribs(LightInd, gLightsBuffer);
                AccumulateBRDF(surface, light, lit);
            }
        }
    }

    out_Color = vec4(lit, 1.0);
}
