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
