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

//#include "Rendering.h"

uniform sampler2D Texture0;
uniform sampler2D Texture1;
uniform sampler2D gLightsBuffer;

in vec4 outPosition;
in vec3 outNormal;
in vec3 outWorldPosition;
in vec3 outViewPosition;
in vec2 outUV0;
in vec2 outUV1;

out vec4 out_Color;

//--------------------------------------------------------------------------------------
void main()
{
    // How many total lights?
    uint totalLights = uint(textureSize(gLightsBuffer, 0).x);

    vec3 lit = vec3(0.0, 0.0, 0.0);

    //[branch] 
    if (mUI.visualizeLightCount != 0u) 
    {
        lit = vec3(float(totalLights), float(totalLights), float(totalLights)) / 255.0;
    } 
    else 
    {
        GeometryVSOut VSOutput;
        VSOutput.positionView = outViewPosition;
        VSOutput.normal = outNormal;
        VSOutput.texCoord0 = outUV0;
        VSOutput.texCoord1 = outUV1;
        SurfaceData surface = ComputeSurfaceDataFromGeometry(VSOutput, Texture0, Texture1);
        for (uint LightInd = 0u; LightInd < totalLights; ++LightInd) 
        {
            PointLight light = LoadLightAttribs(LightInd, gLightsBuffer);
            AccumulateBRDF(surface, light, lit);
        }
        lit += surface.lightMap.rgb;
    }

    out_Color = vec4(lit, 1.0);
}
