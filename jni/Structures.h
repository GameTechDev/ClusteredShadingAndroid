/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

enum SHADING_TECHNIQUE
{
    CULL_FORWARD_NONE = 0,
    CULL_DEFERRED_NONE,
    CULL_QUAD,
    CULL_CLUSTERED,
    CULL_COMPUTE_SHADER_TILE
};

// NOTE: Must match shader equivalent structure
//__declspec(align(16))
struct UIConstants
{
    unsigned int lightingOnly;
    unsigned int faceNormals;
    unsigned int visualizeLightCount;
    unsigned int visualizePerSampleShading;

    unsigned int lightCullTechnique;
    unsigned int clusteredGridScale;
    unsigned int Dummy0;
    unsigned int Dummy1;
};

// NOTE: Must match shader equivalent structure
struct PointLight
{
    float3 positionView;
    float attenuationBegin;
    float3 color;
    float attenuationEnd;
};


// Host-side world-space initial transform data
struct PointLightInitTransform
{
    // Cylindrical coordinates
    float radius;
    float angle;
    float height;
    float animationSpeed;
};

// Flat framebuffer RGBA16-encoded
struct FramebufferFlatElement
{
    unsigned int rb;
    unsigned int ga;
};
