//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
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
//--------------------------------------------------------------------------------------

// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos      : POSITION; // Projected position
    float3 Norm     : NORMAL;
    float2 Uv       : TEXCOORD0;
};
struct PS_INPUT
{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float2 Uv       : TEXCOORD0;
    float4 LightUv  : TEXCOORD1;
    float3 Position : TEXCOORD2; // Object space position 
};
// ********************************************************************************************************
    Texture2D    TEXTURE0 : register( t0 );
    SamplerState SAMPLER0 : register( s0 );
    Texture2D    _Shadow  : register( t1 );
    SamplerComparisonState SAMPLER1 : register( s1 );
// ********************************************************************************************************
cbuffer cbPerModelValues
{
    row_major float4x4 World : WORLD;
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    row_major float4x4 InverseWorld : INVERSEWORLD;
              float4   LightDirection;
              float4   EyePosition;
    row_major float4x4 LightWorldViewProjection;
              float4   BoundingBoxCenterWorldSpace;
              float4   BoundingBoxHalfWorldSpace;
              float4   BoundingBoxCenterObjectSpace;
              float4   BoundingBoxHalfObjectSpace;
};
// ********************************************************************************************************
// TODO: Note: nothing sets these values yet
cbuffer cbPerFrameValues
{
    row_major float4x4 View;
    row_major float4x4 Projection;
              float3   AmbientColor;
              float3   LightColor;
              float3   TotalTimeInSeconds;
    row_major float4x4 InverseView;
    // row_major float4x4 ViewProjection;
};
// ********************************************************************************************************
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;
    // TODO: transform the light into object space instead of the normal into world space
    output.Norm = mul( input.Norm, (float3x3)World );
    output.Uv   = float2(input.Uv.x, input.Uv.y);
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );
    return output;
}
// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{
    float3  lightUv = input.LightUv.xyz / input.LightUv.w;
    lightUv.xy = lightUv.xy * 0.5f + 0.5f; // TODO: Move scale and offset to matrix.
    lightUv.y  = 1.0f - lightUv.y;
    float   shadowAmount = _Shadow.SampleCmp( SAMPLER1, lightUv, lightUv.z );
    float3 normal         = normalize(input.Norm);
    float  nDotL          = saturate( dot( normal, -LightDirection ) );
    float3 eyeDirection   = normalize(input.Position - InverseView._m30_m31_m32);
    float3 reflection     = reflect( eyeDirection, normal );
    float  rDotL          = saturate(dot( reflection, -LightDirection ));
    float3 specular       = pow(rDotL, 16.0f);
    specular              = min( shadowAmount, specular );
    float4 diffuseTexture = TEXTURE0.Sample( SAMPLER0, input.Uv );
    float ambient = 0.05;
    float3 result = (min(shadowAmount, nDotL)+ambient) * diffuseTexture + shadowAmount*specular;
    return float4( result, 1.0f );
}

// ********************************************************************************************************
struct VS_INPUT_NO_TEX
{
    float3 Pos      : POSITION; // Projected position
    float3 Norm     : NORMAL;
};
struct PS_INPUT_NO_TEX
{
    float4 Pos      : SV_POSITION;
    float3 Norm     : NORMAL;
    float4 LightUv  : TEXCOORD1;
    float3 Position : TEXCOORD0; // Object space position 
};
// ********************************************************************************************************
PS_INPUT_NO_TEX VSMainNoTexture( VS_INPUT_NO_TEX input )
{
    PS_INPUT_NO_TEX output = (PS_INPUT_NO_TEX)0;
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;
    // TODO: transform the light into object space instead of the normal into world space
    output.Norm = mul( input.Norm, (float3x3)World );
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );
    return output;
}
// ********************************************************************************************************
float4 PSMainNoTexture( PS_INPUT_NO_TEX input ) : SV_Target
{
    float3 lightUv = input.LightUv.xyz / input.LightUv.w;
    float2 uv = lightUv.xy * 0.5f + 0.5f;
    float2 uvInvertY = float2(uv.x, 1.0f-uv.y);
    float shadowAmount = _Shadow.SampleCmp( SAMPLER1, uvInvertY, lightUv.z );
    float3 eyeDirection = normalize(input.Position - InverseView._m30_m31_m32);
    float3 normal       = normalize(input.Norm);
    float  nDotL = saturate( dot( normal, -normalize(LightDirection.xyz) ) );
    nDotL = shadowAmount * nDotL;
    float3 reflection   = reflect( eyeDirection, normal );
    float  rDotL        = saturate(dot( reflection, -LightDirection.xyz ));
    float  specular     = 0.2f * pow( rDotL, 4.0f );
    specular = min( shadowAmount, specular );
    return float4( (nDotL + specular).xxx, 1.0f);
}



