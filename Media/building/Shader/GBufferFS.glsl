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

in vec4 outPosition;
in vec3 outNormal;
in vec3 outWorldPosition;
in vec3 outViewPosition;
in vec2 outUV0;
in vec2 outUV1;

layout(location = 0) out vec4 out_Diffuse;
layout(location = 1) out vec2 out_Normal;
layout(location = 2) out vec4 out_LightMap;

//--------------------------------------------------------------------------------------
// G-buffer rendering
//--------------------------------------------------------------------------------------
void main()
{
    GeometryVSOut VSOutput;
    VSOutput.positionView = outViewPosition;
    VSOutput.normal = outNormal;
    VSOutput.texCoord0 = outUV0;
    VSOutput.texCoord1 = outUV1;
    SurfaceData surface = ComputeSurfaceDataFromGeometry(VSOutput, Texture0, Texture1);

    out_Diffuse = surface.albedo;
    out_Diffuse.w = outViewPosition.z;
    out_Normal = EncodeSphereMap(surface.normal);
    out_LightMap = surface.lightMap;
}
