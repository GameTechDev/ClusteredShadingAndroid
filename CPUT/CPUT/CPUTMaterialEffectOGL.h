/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
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