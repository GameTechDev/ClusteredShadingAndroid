/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

//#include "GBuffer.h"
//#include "FramebufferFlat.h"
//#include "ShaderDefines.h"

uniform sampler2D gLightsBuffer;
layout(rgba16f, binding = 0) uniform image2D gFramebuffer;

shared int sMinZ;
shared int sMaxZ;

// Light list for the tile
shared uint sTileLightIndices[MAX_SMEM_LIGHTS];
shared int sTileNumLights;

//--------------------------------------------------------------------------------------
// Utility for writing to our flat MSAAed UAV
void WriteSample(uvec2 coords, vec4 value)
{
	imageStore(gFramebuffer, ivec2(coords), value);
    //gFramebuffer[GetFramebufferSampleAddress(coords)] = PackRGBA16(value);
}

layout(local_size_x = COMPUTE_SHADER_TILE_GROUP_DIM, local_size_y = COMPUTE_SHADER_TILE_GROUP_DIM, local_size_z = 1) in;
void main()
{
	uvec3 groupId = gl_WorkGroupID;
	uvec3 dispatchThreadId = gl_GlobalInvocationID;
	uvec3 groupThreadId = gl_LocalInvocationID;

    // NOTE: This is currently necessary rather than just using SV_GroupIndex to work
    // around a compiler bug on Fermi.
    uint groupIndex = groupThreadId.y * uint(COMPUTE_SHADER_TILE_GROUP_DIM) + groupThreadId.x;
    
	// How many total lights?
	uint totalLights = uint(textureSize(gLightsBuffer, 0).x);

    uvec2 globalCoords = dispatchThreadId.xy;

    SurfaceData surfaceSample = ComputeSurfaceDataFromGBufferSample( ivec2(globalCoords) );
	vec3 lit = surfaceSample.lightMap.rgb;

	vec2 CameraNearFar = GetNearFarFromProjMatr(Projection);
    // Work out Z bounds for our samples
	float minZSample = CameraNearFar.x; // Far plane
	float maxZSample = CameraNearFar.y; // Near plane

    {
        // Avoid shading skybox/background or otherwise invalid pixels
        float viewSpaceZ = surfaceSample.positionView.z;
        bool validPixel = 
                viewSpaceZ >= CameraNearFar.y &&
                viewSpaceZ <  CameraNearFar.x;
        //[flatten] 
		if (validPixel) 
        {
            minZSample = min(minZSample, viewSpaceZ);
            maxZSample = max(maxZSample, viewSpaceZ);
        }
    }
    
    // Initialize shared memory light list and Z bounds
    if (groupIndex == 0u) {
        sTileNumLights = 0;
        sMinZ = 0x7F7FFFFF;      // Max float
        sMaxZ = 0;
    }

    //GroupMemoryBarrierWithGroupSync();
	barrier();

    // NOTE: Can do a parallel reduction here but now that we have MSAA and store sample frequency pixels
    // in shaded memory the increased shared memory pressure actually *reduces* the overall speed of the kernel.
    // Since even in the best case the speed benefit of the parallel reduction is modest on current architectures
    // with typical tile sizes, we have reverted to simple atomics for now.
    // Only scatter pixels with actual valid samples in them
    if (maxZSample >= minZSample) 
	{
        atomicMin(sMinZ, floatBitsToInt(minZSample));
		atomicMax(sMaxZ, floatBitsToInt(maxZSample));
    }

    //GroupMemoryBarrierWithGroupSync();
	barrier();


	float minTileZ = intBitsToFloat(sMinZ);
	float maxTileZ = intBitsToFloat(sMaxZ);

    // NOTE: This is all uniform per-tile (i.e. no need to do it per-thread) but fairly inexpensive
    // We could just precompute the frusta planes for each tile and dump them into a constant buffer...
    // They don't change unless the projection matrix changes since we're doing it in view space.
    // Then we only need to compute the near/far ones here tightened to our actual geometry.
    // The overhead of group synchronization/LDS or global memory lookup is probably as much as this
    // little bit of math anyways, but worth testing.

	ivec2 FramebufferDimensions = imageSize(gFramebuffer);

    // Work out scale/bias from [0, 1]
    vec2 tileScale = vec2(FramebufferDimensions.xy) / (float(2 * COMPUTE_SHADER_TILE_GROUP_DIM));
    vec2 tileBias = tileScale - vec2(groupId.xy);

    // Now work out composite projection matrix
    // Relevant matrix columns for this tile frusta
    vec4 c1 = vec4(Projection[0][0] * tileScale.x, 0.0, tileBias.x, 0.0);
	vec4 c2 = vec4(0.0, Projection[1][1] * tileScale.y, tileBias.y, 0.0);
    vec4 c4 = vec4(0.0, 0.0, 1.0, 0.0);

    // Derive frustum planes
    vec4 frustumPlanes[6];
    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;
    // Near/far
    frustumPlanes[4] = vec4(0.0, 0.0,  1.0, -minTileZ);
    frustumPlanes[5] = vec4(0.0, 0.0, -1.0,  maxTileZ);
    
    // Normalize frustum planes (near/far already normalized)
    //[unroll] 
	for (uint i = 0u; i < 4u; ++i) {
        frustumPlanes[i] /= length(frustumPlanes[i].xyz);
    }

    // Cull lights for this tile
    for (uint lightIndex = groupIndex; lightIndex < totalLights; lightIndex += uint(COMPUTE_SHADER_TILE_GROUP_SIZE)) 
	{
		PointLight light = LoadLightAttribs(lightIndex, gLightsBuffer);

        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        //[unroll] 
		for (uint i = 0u; i < 6u; ++i) 
		{
            float d = dot(frustumPlanes[i], vec4(light.positionView, 1.0));
            inFrustum = inFrustum && (d >= -light.attenuationEnd);
        }

        //[branch] 
		if (inFrustum) 
		{
            // Append light to list
            // Compaction might be better if we expect a lot of lights
            int listIndex;
            //InterlockedAdd(sTileNumLights, 1, listIndex);
			listIndex = atomicAdd(sTileNumLights, 1);
            sTileLightIndices[listIndex] = lightIndex;
        }
    }

    //GroupMemoryBarrierWithGroupSync();
	barrier();
    
    uint numLights = uint(sTileNumLights);

	

    // Only process onscreen pixels (tiles can span screen edges)
    //if (all(globalCoords < FramebufferDimensions.xy))
	if ( globalCoords.x < uint(FramebufferDimensions.x) && 
		 globalCoords.y < uint(FramebufferDimensions.y) )
    {
        //[branch] 
		if ( mUI.visualizeLightCount != 0u ) 
        {
			float c = float(sTileNumLights) / 255.0;
            lit = vec3(c,c,c);
            if (sTileNumLights >= MAX_SMEM_LIGHTS) lit.gb = vec2(0.0,0.0); // red for overflow
        } 
        else if (numLights > 0u) 
        {
            //vec3 lit = vec3(0.0, 0.0, 0.0);
            for (uint tileLightIndex = 0u; tileLightIndex < numLights; ++tileLightIndex) 
            {
				PointLight light = LoadLightAttribs(sTileLightIndices[tileLightIndex], gLightsBuffer);
                AccumulateBRDF(surfaceSample, light, lit);
            }
        } 

		// Write sample 0 result
		WriteSample(globalCoords, vec4(lit, 1.0));
    }
}
