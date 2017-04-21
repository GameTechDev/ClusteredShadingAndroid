/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CPUTMATERIALEFFECT_H__
#define __CPUTMATERIALEFFECT_H__

#include "CPUTConfigBlock.h"
#include "CPUTRenderStateBlock.h"

class CPUTModel;

enum CPUTLayerType
{
    CPUT_LAYER_SOLID,
    CPUT_LAYER_TRANSPARENT,
};

//-----------------------------------------------------------------------------
static const StringToIntMapEntry pLayerMap[] = {
    { _L("CPUT_LAYER_SOLID"), CPUT_LAYER_SOLID },
    { _L("CPUT_LAYER_TRANSPARENT"),  CPUT_LAYER_TRANSPARENT },
    { _L(""), -1 }
};

class CPUTMaterialEffect:public CPUTRefCount
{
protected:
    UINT                  mMaterialNameHash;
    cString               mMaterialName;
    const CPUTModel      *mpModel; // We use pointers to the model and mesh to distinguish instanced materials.
    int                   mMeshIndex;
    CPUTConfigBlock       mConfigBlock;
    CPUTRenderStateBlock *mpRenderStateBlock;

    // TODO: We could make that a special object, and derive them from material.  Not sure that's worth the effort.
    // The alternative we choose here is to simply comandeer this one, ignoring most of its state and functionality.

    CPUTMaterialEffect         *mpMaterialNextClone;
    CPUTMaterialEffect         *mpMaterialLastClone;


    // Destructor is not public.  Must release instead of delete.
    virtual ~CPUTMaterialEffect(){
        // SAFE_RELEASE(mpRenderStateBlock); // Allocated in derived class, so released there too.
    }

public:
	CPUTLayerType		  mLayer;
    static CPUTMaterialEffect *CreateMaterialEffect(
        const cString   &absolutePathAndFilename,
        const CPUTModel *pModel=NULL,
              int        meshIndex=-1,
        CPUT_SHADER_MACRO* pShaderMacros=NULL,
              int        externalCount=0,
              cString   *pExternalName=NULL,
              float4    *pExternals=NULL,
              int       *pExternalOffset=NULL,
              int       *pExternalSize=NULL
    );

    CPUTMaterialEffect() :
		mLayer(CPUT_LAYER_SOLID),
        mpRenderStateBlock(NULL),
        mpMaterialNextClone(NULL),
        mpMaterialLastClone(NULL),
        mMaterialNameHash(0),
        mpModel(NULL),
        mMeshIndex(-1)
    {
    };

    // TODO: Where to put this?
    UINT CPUTComputeHash( const cString &string )
    {
        size_t length = string.length();
        UINT hash = 0;
        for( size_t ii=0; ii<length; ii++ )
        {
            hash += tolow(string[ii]);
        }
        return hash;
    }
    UINT GetNameHash() { return mMaterialNameHash; }

    void                  SetMaterialName(const cString materialName) { mMaterialName = materialName; mMaterialNameHash = CPUTComputeHash(materialName); }
    cString              *GetMaterialName() { return &mMaterialName; }
    virtual void          ReleaseTexturesAndBuffers() = 0;
    virtual void          RebindTexturesAndBuffers() = 0;
    virtual void          SetRenderStates(CPUTRenderParameters &renderParams) { if( mpRenderStateBlock ) { mpRenderStateBlock->SetRenderStates(renderParams); } }
    virtual bool          MaterialRequiresPerModelPayload() = 0;
    virtual CPUTMaterialEffect *CloneMaterialEffect(  const CPUTModel *pModel=NULL, int meshIndex=-1 ) = 0;
    CPUTMaterialEffect    *GetNextClone() { return mpMaterialNextClone; }
    const CPUTModel      *GetModel() { return mpModel; }
    int                   GetMeshIndex() { return mMeshIndex; }
    virtual CPUTResult    LoadMaterialEffect(
        const cString   &fileName,
        const CPUTModel *pModel=NULL,
              int        meshIndex=-1,
        CPUT_SHADER_MACRO* pShaderMacros=NULL,
            int        externalCount=0,
              cString   *pExternalName=NULL,
              float4    *pExternals=NULL,
              int       *pExternalOffset=NULL,
              int       *pExternalSize=NULL
    ) = 0;
};

#endif