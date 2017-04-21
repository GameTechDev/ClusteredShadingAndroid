/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef GPU_QUAD_HLSL
#define GPU_QUAD_HLSL

//#include "GBuffer.h"

//--------------------------------------------------------------------------------------
// Bounds computation utilities, similar to PointLightBounds.cpp
void UpdateClipRegionRoot(float nc,          // Tangent plane x/y normal coordinate (view space)
                          float lc,          // Light x/y coordinate (view space)
                          float lz,          // Light z coordinate (view space)
                          float lightRadius,
                          float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
                          inout float clipMin,
                          inout float clipMax)
{
    float nz = (lightRadius - nc * lc) / lz;
    float pz = (lc * lc + lz * lz - lightRadius * lightRadius) /
               (lz - (nz / nc) * lc);

    //[flatten] 
    if (pz > 0.0) 
    {
        float c = -nz * cameraScale / nc;
        //[flatten] 
        if (nc > 0.0) 
        {        
            // Left side boundary
            clipMin = max(clipMin, c);
        } 
        else 
        {                          
            // Right side boundary
            clipMax = min(clipMax, c);
        }
    }
}

void UpdateClipRegion(float lc,          // Light x/y coordinate (view space)
                      float lz,          // Light z coordinate (view space)
                      float lightRadius,
                      float cameraScale, // Project scale for coordinate (_11 or _22 for x/y respectively)
                      inout float clipMin,
                      inout float clipMax)
{
    float rSq = lightRadius * lightRadius;
    float lcSqPluslzSq = lc * lc + lz * lz;
	float d = rSq * lc * lc - lcSqPluslzSq * (rSq - lz * lz);

    if (d > 0.0) 
    {
        float a = lightRadius * lc;
        float b = sqrt(d);
        float nx0 = (a + b) / lcSqPluslzSq;
        float nx1 = (a - b) / lcSqPluslzSq;
        
        UpdateClipRegionRoot(nx0, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
        UpdateClipRegionRoot(nx1, lc, lz, lightRadius, cameraScale, clipMin, clipMax);
    }
}

// Returns bounding box [min.xy, max.xy] in clip [-1, 1] space.
vec4 ComputeClipRegion(vec3 lightPosView, float lightRadius, vec2 CameraNearFar)
{
    // Early out with empty rectangle if the light is too far behind the view frustum
    vec4 clipRegion = vec4(1.0, 1.0, 0.0, 0.0);
    if (lightPosView.z + lightRadius >= CameraNearFar.y) 
    {
        vec2 clipMin = vec2(-1.0, -1.0);
        vec2 clipMax = vec2( 1.0,  1.0);
    
		UpdateClipRegion(lightPosView.x, lightPosView.z, lightRadius, Projection[0][0], clipMin.x, clipMax.x);
		UpdateClipRegion(lightPosView.y, lightPosView.z, lightRadius, Projection[1][1], clipMin.y, clipMax.y);

        clipRegion = vec4(clipMin, clipMax);
    }

    return clipRegion;
}

float RGBToLuminance(vec3 color)
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}


#endif // GPU_QUAD_HLSL
