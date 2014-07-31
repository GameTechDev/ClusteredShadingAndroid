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
#ifndef __CPUTMATERIAL_H__
#define __CPUTMATERIAL_H__

#include <stdio.h>
#include "CPUT.h"
#include "CPUTRefCount.h"
#include "CPUTConfigBlock.h"
#include "CPUTTexture.h"
#include "CPUTMaterialEffect.h"
#include "CPUTRenderStateBlock.h"

class CPUTModel;

// TODO: Where did this number come frome?  It should also be different for each API
#define CPUT_MATERIAL_MAX_TEXTURE_SLOTS         32
#define CPUT_MATERIAL_MAX_BUFFER_SLOTS          32
#define CPUT_MATERIAL_MAX_CONSTANT_BUFFER_SLOTS 32
#define CPUT_MATERIAL_MAX_SRV_SLOTS             32

#if 1 // Need to handle >=DX11 vs. < DX11, where max UAV slots == 1;
#   define CPUT_MATERIAL_MAX_UAV_SLOTS             7
#else
#   define CPUT_MATERIAL_MAX_UAV_SLOTS             1
#endif

class CPUTMaterial:public CPUTRefCount
{
protected:
    UINT                  mMaterialNameHash;
    cString               mMaterialName;
    const CPUTModel      *mpModel; // We use pointers to the model and mesh to distinguish instanced materials.
    int                   mMeshIndex;
    CPUTConfigBlock       mConfigBlock;
    CPUTRenderStateBlock *mpRenderStateBlock;

    // A material can have multiple submaterials.  If it does, then that's all it does.  It just "branches" to other materials.
    // TODO: We could make that a special object, and derive them from material.  Not sure that's worth the effort.
    // The alternative we choose here is to simply comandeer this one, ignoring most of its state and functionality.
    int                   mMaterialEffectCount;
    CPUTMaterialEffect **mpMaterialEffects; 
    cString              *mpMaterialEffectNames;
    UINT                  mCurrentMaterialEffect;

    CPUTMaterial         *mpMaterialNextClone;
    CPUTMaterial         *mpMaterialLastClone;

    // Destructor is not public.  Must release instead of delete.
    virtual ~CPUTMaterial()
    {
        for(int ii = 0; ii < mMaterialEffectCount; ii++)
        {
            SAFE_RELEASE(mpMaterialEffects[ii]);
        }
        SAFE_DELETE_ARRAY(mpMaterialEffects);
        SAFE_DELETE_ARRAY(mpMaterialEffectNames);
    }
    
public:
    static CPUTMaterial *CreateMaterial(
        const cString   &absolutePathAndFilename,
        const CPUTModel *pModel=NULL,
              int        meshIndex=-1,
        CPUT_SHADER_MACRO* pShaderMacros=NULL,
              int        numSystemMaterials=0,
              cString   *pSystemMaterialNames=NULL,
              int        externalCount=0,
              cString   *pExternalName=NULL,
              float4    *pExternals=NULL,
              int       *pExternalOffset=NULL,
              int       *pExternalSize=NULL
    );
    static CPUTConfigBlock mGlobalProperties;

    CPUTMaterial() :
        mpRenderStateBlock(NULL),
        mpMaterialNextClone(NULL),
        mpMaterialLastClone(NULL),
        mMaterialNameHash(0),
        mpModel(NULL),
        mMeshIndex(-1),
        mCurrentMaterialEffect(0),
        mMaterialEffectCount(0),
        mpMaterialEffectNames(NULL),
        mpMaterialEffects(NULL)
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
    //virtual void          ReleaseTexturesAndBuffers() = 0;
    //virtual void          RebindTexturesAndBuffers() = 0;
    //virtual void          SetRenderStates(CPUTRenderParameters &renderParams) { if( mpRenderStateBlock ) { mpRenderStateBlock->SetRenderStates(renderParams); } }
    //virtual bool          MaterialRequiresPerModelPayload() = 0;
    //virtual CPUTMaterial *CloneMaterial( const cString &absolutePathAndFilename, const CPUTModel *pModel=NULL, int meshIndex=-1 ) = 0;
    //CPUTMaterial        **GetSubMaterials() { return mpSubMaterials; }
    //bool                  IsMultiMaterial() const { return mSubMaterialCount > 1; }
    //UINT                  GetSubMaterialCount() { return mSubMaterialCount; }
    CPUTMaterialEffect    **GetMaterialEffects() { return mpMaterialEffects; }
    UINT                  GetMaterialEffectCount() { return mMaterialEffectCount; }
    UINT GetCurrentEffect() { return mCurrentMaterialEffect; }
    void SetCurrentEffect(int CurrentMaterial) { mCurrentMaterialEffect = CurrentMaterial; }

    CPUTMaterial         *GetNextClone() { return mpMaterialNextClone; }
    //const CPUTModel      *GetModel() { return mpModel; }
    //int                   GetMeshIndex() { return mMeshIndex; }
    CPUTResult    LoadMaterial(
        const cString   &fileName,
        const CPUTModel *pModel=NULL,
              int        meshIndex=-1,
        CPUT_SHADER_MACRO* pShaderMacros=NULL,
              int        systemMaterialCount=0,
              cString   *pSystemMaterialNames=NULL,
              int        externalCount=0,
              cString   *pExternalName=NULL,
              float4    *pExternals=NULL,
              int       *pExternalOffset=NULL,
              int       *pExternalSize=NULL
    );
};

#endif //#ifndef __CPUTMATERIAL_H__
