/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PER_FRAME_CONSTANTS_HLSL
#define PER_FRAME_CONSTANTS_HLSL

// NOTE: Must match host equivalent structure
struct UIConstants
{
    uint lightingOnly;
    uint faceNormals;
    uint visualizeLightCount;
    uint visualizePerSampleShading;
    
    uint lightCullTechnique;
    uint clusteredGridScale;
    uint Dummy0;
    uint Dummy1;
};


layout(std140, row_major) uniform cbUIConstants
{
    vec4 mCameraNearFar;
    uvec4 mFramebufferDimensions;
    
    UIConstants mUI;
};

layout (std140, row_major) uniform cbPerFrameValues
{
   mat4  View;
   mat4  InverseView;
   mat4  Projection;
   mat4  ViewProjection;
   vec4  AmbientColor;
   vec4  LightColor;
   vec4  LightDirection;
   vec4  EyePosition;
   vec4  TotalTimeInSeconds;
};

#endif // PER_FRAME_CONSTANTS_HLSL
