/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

//#include "GPUQuad.h"

// This is a "deferred lighting" implementation of GPU quad which reduces the bandwidth
// required during the accumulation passes by accumulating diffuse and (monchromatic) specular
// components only during the lighting phase.

// Monochromatic specular color implemented as suggested by Naty Hoffman at
//   http://www.realtimerendering.com/blog/deferred-lighting-approaches/

uniform sampler2D gGBufferTextures[4];
uniform sampler2D gLightsBuffer;

in vec4 gl_FragCoord;

in GPUQuadVSOutBlock
{
    flat uint lightIndex;
}Input;

out vec4 out_Color;

// Only the pixel shader changes... quad generation is the same
void main()
{
    vec4 result;

    //[branch] 
    if (mUI.visualizeLightCount != 0u) 
    {
        result = (1.0/255.0).xxxx;
    } 
    else 
    {
        SurfaceData surface = ComputeSurfaceDataFromGBufferSample( ivec2(gl_FragCoord.xy)  );
		vec2 CameraNearFar = GetNearFarFromProjMatr(Projection);

        // Avoid shading skybox/background pixels
        // NOTE: Compiler doesn't quite seem to move all the unrelated surface computations inside here
        // We could force it to by restructuring the code a bit, but the "all skybox" case isn't useful for
        // our benchmarking anyways.
        vec3 litDiffuse = vec3(0.0, 0.0, 0.0);
        vec3 litSpecular = vec3(0.0, 0.0, 0.0);
		if (surface.positionView.z < CameraNearFar.x)
        {
            PointLight light = LoadLightAttribs(Input.lightIndex.x, gLightsBuffer);
            AccumulateBRDFDiffuseSpecular(surface, light, litDiffuse, litSpecular);
        }

        // Convert to monochromatic specular for accumulation
        float specularLum = RGBToLuminance(litSpecular);
        result = vec4(litDiffuse, specularLum);
    }

    out_Color = result;
}
