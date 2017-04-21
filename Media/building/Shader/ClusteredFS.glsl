/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

//#include "Rendering.h"

uniform isampler2D gLightGrid;
uniform sampler2D gLightsBuffer;

uniform sampler2D Texture0;
uniform sampler2D Texture1;

in vec4 outPosition;
in vec3 outNormal;
in vec3 outWorldPosition;
in vec3 outViewPosition;
in vec2 outUV0;
in vec2 outUV1;

in vec4 gl_FragCoord;
out vec4 out_Color;


void LightTest(SurfaceData surface, PointLight light, inout int loaded, inout int touched, inout int visible)
{
    vec3 directionToLight = light.positionView - surface.positionView;
    float distanceToLight = length(directionToLight);

    loaded++;
    //[branch] 
    if (distanceToLight < light.attenuationEnd) 
    {
        touched++;
        float NdotL = dot(surface.normal, directionToLight);
        //[flatten] 
        if (NdotL > 0.0) 
        {
            visible++;
        }
    }
}

void fill_array4(out int array[4], ivec4 src)
{
    array[0] = src.x;
    array[1] = src.y;
    array[2] = src.z;
    array[3] = src.w;
}

void main()
{
    ivec3 grid = ivec3(2, 1, 8) * ivec3(mUI.clusteredGridScale);

    vec3 lit = vec3(0.0, 0.0, 0.0);

    GeometryVSOut VSOutput;
    VSOutput.positionView = outViewPosition;
    VSOutput.normal = outNormal;
    VSOutput.texCoord0 = outUV0;
    VSOutput.texCoord1 = outUV1;
    SurfaceData surface = ComputeSurfaceDataFromGeometry(VSOutput, Texture0, Texture1);

    vec2 screenPosition = outPosition.xy/outPosition.w * vec2(0.5,-0.5) + vec2(0.5,0.5);
    vec2 CameraNearFar = GetNearFarFromProjMatr(Projection);
    // NOTE: Complementary Z => swap near/far back
    float zPosition = (surface.positionView.z - CameraNearFar.y) / (CameraNearFar.x - CameraNearFar.y);

    lit = surface.lightMap.rgb;
    
    ivec3 clusterPosition = ivec3( ivec2(screenPosition*vec2(grid.xy)), int(zPosition * float(grid.z)));
    int cluster_index = (clusterPosition.y * grid.x + clusterPosition.x) * grid.z + clusterPosition.z;
    int clusterMask = 0xFFFF;

    int light_count = 0;
    int lights_touched = 0;
    int lights_visible = 0;
    int lights_loaded = 0;


    int lightIndexBlock[4];
    fill_array4(lightIndexBlock, LightGridFetch(gLightGrid, cluster_index) );

    int list_size = lightIndexBlock[0] & 255;
    int list_index = lightIndexBlock[0] >> 8;
    light_count = list_size;

    ivec4 prefetch = LightGridFetch(gLightGrid, list_index++);

    //[loop]
    for (int k = 2; k < list_size + 2; k++)
    {
        int lightIndex = (lightIndexBlock[(k & 7)>>1] >> ((k&1)<<4)) & 0xFFFF;
        if ((k & 7) == 7)
        {
            fill_array4(lightIndexBlock, prefetch);
            prefetch = LightGridFetch(gLightGrid, list_index++);
        }

        PointLight light = LoadLightAttribs(uint(lightIndex), gLightsBuffer);
        AccumulateBRDF(surface, light, lit);
        LightTest(surface, light, lights_loaded, lights_touched, lights_visible);
    }


    //[flatten] 
    if ( mUI.visualizeLightCount != 0u)
    {
        int value = (mUI.visualizePerSampleShading!=0u) ? lights_touched : lights_loaded;
        float fval = float(value) / 255.0;
        lit = vec3(fval, fval, fval);
    }

    out_Color = vec4(lit, 1.0);
}
