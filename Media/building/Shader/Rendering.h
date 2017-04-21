/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef RENDERING_HLSL
#define RENDERING_HLSL

//#include "PerFrameConstants.h"

//--------------------------------------------------------------------------------------
// Utility
//--------------------------------------------------------------------------------------
float linstep(float min, float max, float v)
{
    return clamp((v - min) / (max - min), 0.0, 1.0);
}


//--------------------------------------------------------------------------------------
// Geometry phase
//--------------------------------------------------------------------------------------

layout (std140, row_major) uniform cbPerModelValues
{
   mat4 World;
   mat4 NormalMatrix;
   mat4 WorldViewProjection;
   mat4 InverseWorld;
   mat4 LightWorldViewProjection;
   vec4 BoundingBoxCenterWorldSpace;
   vec4 BoundingBoxHalfWorldSpace;
   vec4 BoundingBoxCenterObjectSpace;
   vec4 BoundingBoxHalfObjectSpace;
};

//struct GeometryVSIn
//{
//    vec3 position;
//    vec3 normal;
//    vec2 texCoord;
//};

struct GeometryVSOut
{
    //vec4 position     : SV_Position;
    vec3 positionView;      // View space position
    vec3 normal;
    vec2 texCoord0;
    vec2 texCoord1;
};

//vec3 ComputeFaceNormal(vec3 position)
//{
//    return cross(dFdx(position), dFdy(position));
//}

// Data that we can read or derive from the surface shader outputs
struct SurfaceData
{
    vec3 positionView;         // View space position
    vec3 normal;               // View space normal
    vec4 albedo;
    vec4 lightMap;
    float specularAmount;
    float specularPower;
};

SurfaceData ComputeSurfaceDataFromGeometry(GeometryVSOut Input, sampler2D gDiffuseTexture, sampler2D LightMapI)
{
    SurfaceData surface;
    surface.positionView = Input.positionView;

    // Optionally use face normal instead of shading normal
    //vec3 faceNormal = ComputeFaceNormal(Input.positionView);
    surface.normal = normalize(Input.normal);//normalize( mUI.faceNormals != 0 ? faceNormal : Input.normal );
    
    surface.albedo = texture(gDiffuseTexture, Input.texCoord0);
    surface.albedo.rgb = (mUI.lightingOnly != 0u) ? vec3(1.0, 1.0, 1.0) : surface.albedo.rgb;

    vec4 lightMapI = texture(LightMapI, Input.texCoord1);
    surface.lightMap = surface.albedo * lightMapI;

    // We don't really have art asset-related values for these, so set them to something
    // reasonable for now... the important thing is that they are stored in the G-buffer for
    // representative performance measurement.
    surface.specularAmount = 0.9;
    surface.specularPower = 15.0;

    return surface;
}


//--------------------------------------------------------------------------------------
// Lighting phase utilities
//--------------------------------------------------------------------------------------
struct PointLight
{
    vec3 positionView;
    float attenuationBegin;
    vec3 color;
    float attenuationEnd;
};

//StructuredBuffer<PointLight> gLight : register(t5);

// As below, we separate this for diffuse/specular parts for convenience in deferred lighting
void AccumulatePhongBRDF(vec3 normal,
                         vec3 lightDir,
                         vec3 viewDir,
                         vec3 lightContrib,
                         float specularPower,
                         inout vec3 litDiffuse,
                         inout vec3 litSpecular)
{
    // Simple Phong
    float NdotL = dot(normal, lightDir);
    //[flatten] 
    if (NdotL > 0.0) 
    {
        vec3 h = normalize(lightDir - viewDir);
        float HdotN = max(0.0, dot(h, normal));
        float specular = pow(HdotN, specularPower);

        litDiffuse += lightContrib * NdotL;
        litSpecular += lightContrib * specular;
    }
}

// Accumulates separate "diffuse" and "specular" components of lighting from a given
// This is not possible for all BRDFs but it works for our simple Phong example here
// and this separation is convenient for deferred lighting.
// Uses an in-out for accumulation to avoid returning and accumulating 0
void AccumulateBRDFDiffuseSpecular(SurfaceData surface, PointLight light,
                                   inout vec3 litDiffuse,
                                   inout vec3 litSpecular)
{
    vec3 directionToLight = light.positionView - surface.positionView;
    float distanceToLight = length(directionToLight);

    //[branch] 
    if (distanceToLight < light.attenuationEnd) 
    {
        float attenuation = linstep(light.attenuationEnd, light.attenuationBegin, distanceToLight);
        directionToLight /= distanceToLight;       // A full normalize/RSQRT might be as fast here anyways...
        
        AccumulatePhongBRDF(surface.normal, directionToLight, normalize(surface.positionView),
            attenuation * light.color, surface.specularPower, litDiffuse, litSpecular);
    }
}

// Uses an in-out for accumulation to avoid returning and accumulating 0
void AccumulateBRDF(SurfaceData surface, PointLight light,
                    inout vec3 lit)
{
    vec3 directionToLight = light.positionView - surface.positionView;
    float distanceToLight = length(directionToLight);

    //[branch] 
    if (distanceToLight < light.attenuationEnd) 
    {
        float attenuation = linstep(light.attenuationEnd, light.attenuationBegin, distanceToLight);
        directionToLight /= distanceToLight;       // A full normalize/RSQRT might be as fast here anyways...

        vec3 litDiffuse = vec3(0.0, 0.0, 0.0);
        vec3 litSpecular = vec3(0.0, 0.0, 0.0);
        AccumulatePhongBRDF(surface.normal, directionToLight, normalize(surface.positionView),
            attenuation * light.color, surface.specularPower, litDiffuse, litSpecular);
        
        lit += surface.albedo.rgb * (litDiffuse + surface.specularAmount * litSpecular);
    }
}

PointLight LoadLightAttribs(uint LightID, sampler2D Light)
{
    vec4 Data0 = texelFetch(Light, ivec2(LightID, 0), 0);
    vec4 Data1 = texelFetch(Light, ivec2(LightID, 1), 0);
    
    PointLight light;
    light.positionView = Data0.xyz;
    light.attenuationBegin = Data0.w;
    light.color = Data1.xyz;
    light.attenuationEnd = Data1.w;
    
    return light;
}

ivec4 LightGridFetch(isampler2D LightGrid, int Index)
{
    int iRow = Index / LIGHT_GRID_TEXTURE_WIDTH;
    int iCol = Index % LIGHT_GRID_TEXTURE_WIDTH;
    return texelFetch(LightGrid, ivec2(iCol,iRow), 0);
}

vec2 GetNearFarFromProjMatr(mat4 mProj)
{
    vec2 NearFar;
    NearFar.x = -mProj[2][3] / mProj[2][2]; // OpenGL matrices are column-major depsite of the storage qualifier
                                            // Thus the first index is column number
    NearFar.y = mProj[2][2] / (mProj[2][2]-1.0) * NearFar.x;
    return NearFar;
}

#endif // RENDERING_HLSL
