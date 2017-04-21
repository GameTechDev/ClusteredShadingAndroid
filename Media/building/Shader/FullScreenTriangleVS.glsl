/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

in int gl_VertexID;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
    // Parametrically work out vertex location for full screen triangle
    vec2 grid = vec2( float((gl_VertexID << 1) & 2), float(gl_VertexID & 2) );
    gl_Position = vec4(grid * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 1.0, 1.0);
}
