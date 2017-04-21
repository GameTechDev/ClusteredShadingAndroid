/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

//#include "GPUQuad.h"

uniform sampler2D gLightsBuffer;

out GPUQuadVSOutBlock
{
    flat uint lightIndex;
}Output;

out gl_PerVertex
{
	vec4 gl_Position;
};

in int gl_VertexID;

void main()
{
    uint lightIndex = uint(gl_VertexID / 6);

    Output.lightIndex = lightIndex;
	vec2 CameraNearFar = GetNearFarFromProjMatr(Projection);

    // Work out tight clip-space rectangle
	PointLight light = LoadLightAttribs(lightIndex, gLightsBuffer);
	vec4 coords = ComputeClipRegion(light.positionView, light.attenuationEnd, CameraNearFar);

    // Branch around empty regions (i.e. light entirely offscreen)
    if ( coords.x < coords.z && coords.y < coords.w ) 
    {
        vec2 posViewPortXY[6] = 
        {
            coords.xw,
            coords.zw,
            coords.xy,

            coords.xy,
            coords.zw,
            coords.zy
        };

        gl_Position.xy = posViewPortXY[gl_VertexID % 6];

		// Work out nearest depth for quad Z
        // Clamp to near plane in case this light intersects the near plane... don't want our quad to be clipped
		float quadDepth = max(CameraNearFar.y, light.positionView.z - light.attenuationEnd);

        // Project quad depth into clip space
        vec4 quadClip = vec4(0.0, 0.0, quadDepth, 1.0) * Projection;
        gl_Position.zw = vec2( quadClip.z / quadClip.w, 1.0 );
    }
    else
    {
        gl_Position = vec4(-100.0, -100.0, 100.0, 1.0);
    } 
}
