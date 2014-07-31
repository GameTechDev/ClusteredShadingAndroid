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

in vec4 gl_FragCoord;

in GPUQuadVSOutBlock
{
    flat uint lightIndex;
}Input;

out vec4 out_Color;

void main()
{
    vec3 lit = vec3(0.0, 0.0, 0.0);
    
    //[branch] 
    if (mUI.visualizeLightCount !=0u ) 
    {
		float t = 1.0/255.0;
        lit = vec3(t,t,t);
    } 
    else 
    {
        SurfaceData surface = ComputeSurfaceDataFromGBufferSample( ivec2(gl_FragCoord.xy) );

		vec2 CameraNearFar = GetNearFarFromProjMatr(Projection);
	
        // Avoid shading skybox/background pixels
        // NOTE: Compiler doesn't quite seem to move all the unrelated surface computations inside here
        // We could force it to by restructuring the code a bit, but the "all skybox" case isn't useful for
        // our benchmarking anyways.
		if (surface.positionView.z < CameraNearFar.x)
        {
            PointLight light = LoadLightAttribs(Input.lightIndex, gLightsBuffer);
            AccumulateBRDF(surface, light, lit);
        }    
    }

    out_Color = vec4(lit, 1.0);
}
