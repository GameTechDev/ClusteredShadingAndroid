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
