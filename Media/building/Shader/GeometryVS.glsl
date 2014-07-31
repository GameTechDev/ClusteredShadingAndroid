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

#define POSITION  0
#define NORMAL    1
#define BINORMAL  2
#define TANGENT   3
#define COLOR   4
#define TEXCOORD0 5
#define TEXCOORD1 6
// -------------------------------------
layout (location = POSITION)  in vec3 Position; // Projected position
layout (location = NORMAL)    in vec3 Normal;
layout (location = TEXCOORD0) in vec2 UV0;
layout (location = TEXCOORD1) in vec2 UV1;
// -------------------------------------
out vec4 outPosition;
out vec3 outNormal;
out vec3 outWorldPosition;
out vec3 outViewPosition;
out vec2 outUV0;
out vec2 outUV1;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    outPosition = vec4( Position, 1.0) * WorldViewProjection;
    outNormal   = (Normal * mat3(World)) * mat3(View);
    outWorldPosition = (vec4( Position, 1.0) * World ).xyz;
    outUV0 = UV0;
    outUV1 = UV1;
    outViewPosition = ( vec4( outWorldPosition, 1.0 ) * View ).xyz;

    gl_Position = outPosition;
}
