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
#ifndef __CPUTMATERIALEFFECTOGL_H__
#define __CPUTMATERIALEFFECTOGL_H__

#include "CPUTMaterialEffect.h"
#include "CPUTMaterial.h"

#include <vector>

class CPUTShaderOGL;
class CPUTShaderParameters
{
public:
    UINT                       mTextureCount;
    UINT                       mTextureParameterCount;
    std::vector<std::string>   mpTextureParameterNames;
    std::vector<GLint>         mpTextureParameterBindPoints;
    std::vector<GLint>         mpTextureParameterLocations;

    CPUTTexture               *mpTexture[CPUT_MATERIAL_MAX_TEXTURE_SLOTS];
    CPUTBuffer                *mpBuffer[CPUT_MATERIAL_MAX_BUFFER_SLOTS];
    CPUTTexture               *mpUAV[CPUT_MATERIAL_MAX_UAV_SLOTS];
    GLint                      mpUAVMode[CPUT_MATERIAL_MAX_UAV_SLOTS];
    CPUTBuffer                *mpConstantBuffer[CPUT_MATERIAL_MAX_CONSTANT_BUFFER_SLOTS];

    // UINT                       mSamplerCount; // TODO: Why don't we need this? We should probably be rebinding samplers too
    cString                   *mpSamplerParameterName;
    UINT                      *mpSamplerParameterBindPoint;
    UINT                       mSamplerParameterCount;

    UINT                       mBufferCount;
    UINT                       mBufferParameterCount;
    cString                   *mpBufferParameterName;
    UINT                      *mpBufferParameterBindPoint;

    UINT                       mUAVCount;
    UINT                       mUAVParameterCount;

    std::vector<std::string>   mpUAVParameterNames;
    std::vector<GLint>         mpUAVParameterLocations;
    std::vector<GLint>         mpUAVParameterBindPoints;

    UINT                       mConstantBufferCount;
    UINT                       mConstantBufferParameterCount;
    std::vector<std::string>   mConstantBufferParameterNames;
    std::vector<GLint>         mConstantBufferBindPoints;


    UINT                       mBindViewMin;
    UINT                       mBindViewMax;

    UINT                       mBindUAVMin;
    UINT                       mBindUAVMax;

    UINT                       mBindConstantBufferMin;
    UINT                       mBindConstantBufferMax;


    CPUTShaderParameters() :
        mTextureCount(0),
        mTextureParameterCount(0),
        mSamplerParameterCount(0),
        mpSamplerParameterName(NULL),
        mpSamplerParameterBindPoint(NULL),
        mBufferCount(0),
        mBufferParameterCount(0),
        mpBufferParameterName(NULL),
        mpBufferParameterBindPoint(NULL),
        mUAVCount(0),
        mUAVParameterCount(0),
        //mpUAVParameterNames(),
        //mpUAVParameterBindPoints(),
        mConstantBufferCount(0),
        mConstantBufferParameterCount(0),
        mBindViewMin(CPUT_MATERIAL_MAX_SRV_SLOTS),
        mBindViewMax(0),
        mBindUAVMin(CPUT_MATERIAL_MAX_UAV_SLOTS),
        mBindUAVMax(0),
        mBindConstantBufferMin(CPUT_MATERIAL_MAX_CONSTANT_BUFFER_SLOTS),
        mBindConstantBufferMax(0)
    {
        // initialize texture slot list to null
        for(int ii=0; ii<CPUT_MATERIAL_MAX_TEXTURE_SLOTS; ii++)
        {
            mpTexture[ii] = NULL;
        }
        for(int ii=0; ii<CPUT_MATERIAL_MAX_BUFFER_SLOTS; ii++)
        {
            mpBuffer[ii] = NULL;
        }
        for(int ii=0; ii<CPUT_MATERIAL_MAX_UAV_SLOTS; ii++)
        {
            mpUAV[ii] = NULL;
        }
        for(int ii=0; ii<CPUT_MATERIAL_MAX_CONSTANT_BUFFER_SLOTS; ii++)
        {
            mpConstantBuffer[ii] = NULL;
        }
    };
    ~CPUTShaderParameters();
    void CloneShaderParameters( CPUTShaderParameters *pShaderParameter );
};

static const int CPUT_NUM_SHADER_PARAMETER_LISTS = 7;
class CPUTMaterialEffectOGL : public CPUTMaterialEffect
{
protected:
    ~CPUTMaterialEffectOGL();  // Destructor is not public.  Must release instead of delete.
    void ReadShaderSamplersAndTextures( GLuint shaderProgram, CPUTShaderParameters *pShaderParameter );
    
    void BindTextures(        CPUTShaderParameters &params, const CPUTModel *pModel=NULL, int meshIndex=-1 );
    void BindConstantBuffers( CPUTShaderParameters &params, const CPUTModel *pModel=NULL, int meshIndex=-1 );
	void BindUAVs( CPUTShaderParameters &params, const CPUTModel *pModel, int meshIndex );

    CPUTShaderOGL *mpFragmentShader;
    CPUTShaderOGL *mpVertexShader;
	CPUTShaderOGL *mpControlShader;
	CPUTShaderOGL *mpEvaluationShader;
	CPUTShaderOGL *mpGeometryShader;

    GLuint mShaderProgram;
    GLuint mSamplerIDs[CPUT_MATERIAL_MAX_TEXTURE_SLOTS];
    
    static void *mpLastRenderStateBlock;
    int mRequiresPerModelPayload;

public:
    CPUTMaterialEffectOGL();
    CPUTResult            LoadMaterialEffect(
        const cString   &fileName,
        const CPUTModel *pModel=NULL,
              int        meshIndex=-1,
        CPUT_SHADER_MACRO* pShaderMacros=NULL,
              int        externalCount=0,
              cString   *pExternalName=NULL,
              float4    *pExternals=NULL,
              int       *pExternalOffset=NULL,
              int       *pExternalSize=NULL
    );

    virtual void          ReleaseTexturesAndBuffers();
    virtual void          RebindTexturesAndBuffers();
    CPUTShaderOGL   *GetVertexShader()      { return mpVertexShader; }
    CPUTShaderOGL   *GetPixelShader()       { return mpFragmentShader; }
	CPUTShaderOGL	*GetControlShader()     { return mpControlShader; }
	CPUTShaderOGL	*GetEvaluationShader()  { return mpEvaluationShader; }
    CPUTShaderOGL   *GetGeometryShader()    { return mpGeometryShader; }
    bool Tessellated() { return mpEvaluationShader != NULL; };

    virtual void          SetRenderStates(CPUTRenderParameters &renderParams);
    virtual bool          MaterialRequiresPerModelPayload();
    virtual CPUTMaterialEffect *CloneMaterialEffect(  const CPUTModel *pModel=NULL, int meshIndex=-1 );
    
    // OGL does not support all of these but we keep them the same for now to match the DX version
    CPUTShaderParameters     mPixelShaderParameters;
    CPUTShaderParameters     mComputeShaderParameters;
    CPUTShaderParameters     mVertexShaderParameters;
    CPUTShaderParameters     mGeometryShaderParameters;
    CPUTShaderParameters     mHullShaderParameters;
    CPUTShaderParameters     mDomainShaderParameters;
    CPUTShaderParameters    *mpShaderParametersList[CPUT_NUM_SHADER_PARAMETER_LISTS]; // Constructor initializes this as a list of pointers to the above shader parameters.
};
#endif