/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
