/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
