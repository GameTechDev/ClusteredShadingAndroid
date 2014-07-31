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
#ifndef __CPUTMODEL_H__
#define __CPUTMODEL_H__

// Define the following to support drawing bounding boxes.
// Note that you also need bounding-box materials and shaders.  TODO: Include them in the exe.
//#define SUPPORT_DRAWING_BOUNDING_BOXES 1

#include "CPUTRenderNode.h"
#include "CPUTMath.h"
#include "CPUTConfigBlock.h"
#include "CPUTMesh.h"

#include "CPUTSkeleton.h"

class CPUTMaterialEffect;
class CPUTMesh;

enum {
    CPUT_MATERIAL_INDEX_SHADOW_CAST                 = -1,
    CPUT_MATERIAL_INDEX_BOUNDING_BOX                = -2,
};
#ifdef CPUT_FOR_DX11
    const int NUM_GLOBAL_SYSTEM_MATERIALS = 2;
#elif (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
    const int NUM_GLOBAL_SYSTEM_MATERIALS = 1;
#else    
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif


typedef bool (*DrawModelCallBackFunc)(CPUTModel*, CPUTRenderParameters &renderParams, CPUTMesh*, CPUTMaterialEffect*, CPUTMaterialEffect* pMaterial, void* );

//-----------------------------------------------------------------------------
class CPUTModel : public CPUTRenderNode
{
protected:  
    CPUTMesh      **mpMesh;
    UINT           *mpLayoutCount;
    CPUTMaterial **mpRootMaterial; // Have arrays of submaterials, one array for each mesh.
	CPUTMaterialEffect ***mpMaterialEffect;

    UINT            mMeshCount;
    bool            mIsRenderable;
    float3          mBoundingBoxCenterObjectSpace;
    float3          mBoundingBoxHalfObjectSpace;
    float3          mBoundingBoxCenterWorldSpace;
    float3          mBoundingBoxHalfWorldSpace;
    CPUTMesh       *mpBoundingBoxMesh;
	static DrawModelCallBackFunc mDrawModelCallBackFunc;

public:
    static CPUTModel* CreateModel();
    //Skinning stuff
    CPUTSkeleton *mSkeleton;
    //End Skinning stuff
    CPUTModel():
        mMeshCount(0),
        mpLayoutCount(NULL),
        mpRootMaterial(NULL),
		mpMaterialEffect(NULL),
        mpMesh(NULL),
        mIsRenderable(true),
        mBoundingBoxCenterObjectSpace(0.0f),
        mBoundingBoxHalfObjectSpace(0.0f),
        mBoundingBoxCenterWorldSpace(0.0f),
        mBoundingBoxHalfWorldSpace(0.0f),
        mpBoundingBoxMesh(NULL),
        mSkeleton(NULL)
    {}
    virtual ~CPUTModel();

	static			   void SetDrawModelCallBack(DrawModelCallBackFunc Func){mDrawModelCallBackFunc = Func;}

    virtual void       UpdateRecursive(float deltaSeconds);
    bool               IsRenderable() { return mIsRenderable; }
    void               SetRenderable(bool isRenderable) { mIsRenderable = isRenderable; }
    virtual bool       IsModel() { return true; }
    CPUT_NODE_TYPE     GetNodeType() { return CPUT_NODE_MODEL;};

    void               GetBoundsObjectSpace(float3 *pCenter, float3 *pHalf);
    void               GetBoundsWorldSpace(float3 *pCenter, float3 *pHalf);
    void               UpdateBoundsWorldSpace();
    int                GetMeshCount() const { return mMeshCount; }
    CPUTMesh          *GetMesh( UINT ii ) { return mpMesh[ii]; }
    virtual CPUTResult LoadModel(CPUTConfigBlock *pBlock, int *pParentID, CPUTModel *pMasterModel=NULL, int numSystemMaterials=0, cString *pSystemMaterialNames=NULL) = 0;
    CPUTResult         LoadModelPayload(const cString &File);

    virtual void       SetMaterial(UINT ii, CPUTMaterial *pMaterial);
    UINT GetMaterialIndex(int meshIndex, int index);

#ifdef SUPPORT_DRAWING_BOUNDING_BOXES
    virtual void       DrawBoundingBox(CPUTRenderParameters &renderParams) = 0;
    virtual void       CreateBoundingBoxMesh() = 0;
#endif

    void GetBoundingBoxRecursive( float3 *pCenter, float3 *pHalf)
    {
        if( *pHalf == float3(0.0f) )
        {
            *pCenter = mBoundingBoxCenterWorldSpace;
            *pHalf   = mBoundingBoxHalfWorldSpace;
        }
        else
        {
            float3 minExtent = *pCenter - *pHalf;
            float3 maxExtent = *pCenter + *pHalf;
            float3 minDistance = mBoundingBoxCenterWorldSpace - mBoundingBoxHalfWorldSpace;
            float3 maxDistance = mBoundingBoxCenterWorldSpace + mBoundingBoxHalfWorldSpace;
            minExtent = Min( minDistance, minExtent );
            maxExtent = Max( maxDistance, maxExtent );
            *pCenter = (minExtent + maxExtent) * 0.5f;
            *pHalf   = (maxExtent - minExtent) * 0.5f;
        }
        if(mpChild)   { mpChild->GetBoundingBoxRecursive(   pCenter, pHalf ); }
        if(mpSibling) { mpSibling->GetBoundingBoxRecursive( pCenter, pHalf ); }
    }

    CPUTMaterial *GetMaterial( int meshIndex ) const
    {
        return mpRootMaterial[meshIndex];
    }
};
#endif // __CPUTMODEL_H__