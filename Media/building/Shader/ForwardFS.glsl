/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
