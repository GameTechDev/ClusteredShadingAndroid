//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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
#include "CPUTModelOGL.h"
#include "CPUTMaterialEffectOGL.h"

#include "CPUTFrustum.h"
#include "CPUTAssetLibraryOGL.h"
#include "CPUTTextureOGL.h"

DrawModelCallBackFunc CPUTModel::mDrawModelCallBackFunc = CPUTModelOGL::DrawModelCallBack;
//-----------------------------------------------------------------------------
CPUTMeshOGL* CPUTModelOGL::GetMesh(const UINT index) const
{
    return ( 0==mMeshCount || index > mMeshCount) ? NULL : (CPUTMeshOGL*)mpMesh[index];
}

// Set the render state before drawing this object
//-----------------------------------------------------------------------------
void CPUTModelOGL::UpdateShaderConstants(CPUTRenderParameters &renderParams)
{
    float4x4     world(*GetWorldMatrix());
    float4x4     NormalMatrix(*GetWorldMatrix());

	//Local transform if node baked into animation
	if(mSkeleton && mpCurrentAnimation)
	{
		world        = GetParentsWorldMatrix();
		NormalMatrix = GetParentsWorldMatrix();
	}

    if( renderParams.mpPerModelConstants )
    {
        CPUTBufferOGL *pBuffer = (CPUTBufferOGL*)(renderParams.mpPerModelConstants);
        CPUTModelConstantBuffer cb;
        cb.World = world;
        cb.InverseWorld = cb.World;
        cb.InverseWorld.invert();

        CPUTCamera *pCamera   = renderParams.mpCamera;

        if(pCamera)
        {
            cb.WorldViewProjection = cb.World * *pCamera->GetViewMatrix() * *pCamera->GetProjectionMatrix();
        }

        float4x4    shadowView, shadowProjection;
        CPUTCamera *pShadowCamera = gpSample->GetShadowCamera();
        if( pShadowCamera )
        {
            shadowView = *pShadowCamera->GetViewMatrix();
            shadowProjection = *pShadowCamera->GetProjectionMatrix();
            cb.LightWorldViewProjection = cb.World * shadowView * shadowProjection;
        }

        cb.BoundingBoxCenterWorldSpace  = float4(mBoundingBoxCenterWorldSpace, 0);
        cb.BoundingBoxHalfWorldSpace    = float4(mBoundingBoxHalfWorldSpace, 0);
        cb.BoundingBoxCenterObjectSpace = float4(mBoundingBoxCenterObjectSpace, 0);
        cb.BoundingBoxHalfObjectSpace   = float4(mBoundingBoxHalfObjectSpace, 0);

		//TODO: Should this process if object not visible?
		//Only do this if Model has a skin and is animated
		if(mSkeleton && mpCurrentAnimation)
		{
            ASSERT(0, _L("Skinning constant buffer temporarily disabled"));
			//ASSERT(mSkeleton->mNumberOfJoints < 255, _L("Skin Exceeds maximum number of allowable joints: 255"));
			//for(UINT i = 0; i < mSkeleton->mNumberOfJoints; ++i)
			//{
			//	CPUTJoint *pJoint = &mSkeleton->mJointsList[i];
			//	cb.SkinMatrix[i] = pJoint->mInverseBindPoseMatrix * pJoint->mScaleMatrix * pJoint->mRTMatrix;
			//	float4x4 skinNormalMatrix = cb.SkinMatrix[i];
			//	skinNormalMatrix.invert(); skinNormalMatrix.transpose();  
			//	cb.SkinNormalMatrix[i] = skinNormalMatrix;
			//}
		}

#ifndef CPUT_FOR_OGLES2
        pBuffer->SetSubData(0, sizeof(CPUTModelConstantBuffer), &cb);
#else

#warning "Need to do something with uniform buffers here"
#endif
    }
}

// Render - render this model (only)
//-----------------------------------------------------------------------------
void CPUTModelOGL::Render(CPUTRenderParameters &renderParams, int materialIndex)
{
    CPUTCamera* pCamera = renderParams.mpCamera;
    UpdateBoundsWorldSpace();
    if( renderParams.mDrawModels && !renderParams.mRenderOnlyVisibleModels || !pCamera || pCamera->mFrustum.IsVisible( mBoundingBoxCenterWorldSpace, mBoundingBoxHalfWorldSpace ) )
    {
        UpdateShaderConstants(renderParams);
        // loop over all meshes in this model and draw them
        for(UINT ii=0; ii<mMeshCount; ii++)
        {
            UINT finalMaterialIndex = GetMaterialIndex(ii, materialIndex);
            ASSERT( finalMaterialIndex < mpLayoutCount[ii], _L("material index out of range."));
            CPUTMaterialEffect *pMaterialEffect = (CPUTMaterialEffect*)(mpMaterialEffect[ii][finalMaterialIndex]);

		    mDrawModelCallBackFunc(this, renderParams, mpMesh[ii], pMaterialEffect, NULL, NULL);    
        }
    }
}

//-----------------------------------------------------------------------------
// DrawModelCallBack - default model rendering code can be overridden by a callback set in user side code
//-----------------------------------------------------------------------------
bool CPUTModelOGL::DrawModelCallBack(CPUTModel* pModel, CPUTRenderParameters &renderParams, CPUTMesh* pMesh, CPUTMaterialEffect* pMaterial, CPUTMaterialEffect* , void* )
{
  
	pMaterial->SetRenderStates(renderParams);
    if( ((CPUTMaterialEffectOGL*)pMaterial)->Tessellated() )
        ((CPUTMeshOGL*)pMesh)->DrawPatches(renderParams, pModel);
    else
        ((CPUTMeshOGL*)pMesh)->Draw(renderParams, pModel);

	return true;
}

#ifdef SUPPORT_DRAWING_BOUNDING_BOXES
//-----------------------------------------------------------------------------
void CPUTModelOGL::DrawBoundingBox(CPUTRenderParameters &renderParams)
{
    SetRenderStates(renderParams);
    CPUTMaterialOGL *pMaterial = (CPUTMaterialOGL*)mpBoundingBoxMaterial;

    mpBoundingBoxMaterial->SetRenderStates(renderParams);
    ((CPUTMeshOGL*)mpBoundingBoxMesh)->Draw(renderParams, this);
}
#endif

// Load the set file definition of this object
// 1. Parse the block of name/parent/transform info for model block
// 2. Load the model's binary payload (i.e., the meshes)
// 3. Assert the # of meshes matches # of materials
// 4. Load each mesh's material
//-----------------------------------------------------------------------------
CPUTResult CPUTModelOGL::LoadModel(CPUTConfigBlock *pBlock, int *pParentID, CPUTModel *pMasterModel, int numSystemMaterials, cString *pSystemMaterialNames)
{
    CPUTResult result = CPUT_SUCCESS;
    CPUTAssetLibraryOGL *pAssetLibrary = (CPUTAssetLibraryOGL*)CPUTAssetLibrary::GetAssetLibrary();

    cString modelSuffix = ptoc(this);

    // set the model's name
    mName = pBlock->GetValueByName(_L("name"))->ValueAsString();
    mName = mName + _L(".mdl");

    // resolve the full path name
    cString modelLocation;
    cString resolvedPathAndFile;
    modelLocation = ((CPUTAssetLibraryOGL*)CPUTAssetLibrary::GetAssetLibrary())->GetModelDirectoryName();
    modelLocation = modelLocation+mName;

    CPUTFileSystem::ResolveAbsolutePathAndFilename(modelLocation, &resolvedPathAndFile);

    // Get the parent ID.  Note: the caller will use this to set the parent.
    *pParentID = pBlock->GetValueByName(_L("parent"))->ValueAsInt();

    LoadParentMatrixFromParameterBlock( pBlock );

    // Get the bounding box information
    float3 center(0.0f), half(0.0f);
    pBlock->GetValueByName(_L("BoundingBoxCenter"))->ValueAsFloatArray(center.f, 3);
    pBlock->GetValueByName(_L("BoundingBoxHalf"))->ValueAsFloatArray(half.f, 3);
    mBoundingBoxCenterObjectSpace = center;
    mBoundingBoxHalfObjectSpace   = half;

    mMeshCount = pBlock->GetValueByName(_L("meshcount"))->ValueAsInt();
    mpMesh     = new CPUTMesh*[mMeshCount];
    mpLayoutCount = new UINT[mMeshCount];
    mpRootMaterial = new CPUTMaterial*[mMeshCount];
    memset( mpRootMaterial, 0, mMeshCount * sizeof(CPUTMaterial*) );
    mpMaterialEffect = new CPUTMaterialEffect**[mMeshCount];
    memset( mpMaterialEffect, 0, mMeshCount * sizeof(CPUTMaterialEffect*) );
       
    // get the material names, load them, and match them up with each mesh
    cString materialName,shadowMaterialName;
    char pNumber[4];
    cString materialValueName;

    CPUTModelOGL *pMasterModelDX = (CPUTModelOGL*)pMasterModel;

    for(UINT ii=0; ii<mMeshCount; ii++)
    {
        if(pMasterModelDX)
        {
            // Reference the master model's mesh.  Don't create a new one.
            mpMesh[ii] = pMasterModelDX->mpMesh[ii];
            mpMesh[ii]->AddRef();
        }
        else
        {
            mpMesh[ii] = new CPUTMeshOGL();
        }
    }
    if( !pMasterModelDX )
    {
        // Not a clone/instance.  So, load the model's binary payload (i.e., vertex and index buffers)
        // TODO: Change to use GetModel()
        result = LoadModelPayload(resolvedPathAndFile);
        ASSERT( CPUTSUCCESS(result), _L("Failed loading model") );
    }

    for(UINT ii=0; ii<mMeshCount; ii++)
    {
        // get the right material number ('material0', 'material1', 'material2', etc)
        materialValueName = _L("material");
        snprintf(pNumber, 4, "%d", ii);
//        _itoa(ii, pNumber, 4, 10);
        materialValueName.append(s2ws(pNumber));
        materialName = pBlock->GetValueByName(materialValueName)->ValueAsString();
        shadowMaterialName = pBlock->GetValueByName(_L("shadowCast") + materialValueName)->ValueAsString();
        bool isSkinned = pBlock->GetValueByName(_L("skeleton")) != &CPUTConfigEntry::sNullConfigValue;
        if( shadowMaterialName.length() == 0 )
        {
            if(!isSkinned)
            {
                shadowMaterialName = _L("%shadowCast");
            }
            else
            {
                shadowMaterialName = _L("%shadowCastSkinned");
            }
        }

        // Get/load material for this mesh
        UINT totalNameCount = numSystemMaterials + NUM_GLOBAL_SYSTEM_MATERIALS;
        cString *pFinalSystemNames = new cString[totalNameCount];

        // Copy "global" system materials to caller-supplied list
        for( int jj=0; jj<numSystemMaterials; jj++ )
        {
            pFinalSystemNames[jj] = pSystemMaterialNames[jj];
        }
        pFinalSystemNames[totalNameCount + CPUT_MATERIAL_INDEX_SHADOW_CAST]                =  shadowMaterialName;
//        pFinalSystemNames[totalNameCount + CPUT_MATERIAL_INDEX_BOUNDING_BOX]                = _L("%BoundingBox");
        int finalNumSystemMaterials = numSystemMaterials + NUM_GLOBAL_SYSTEM_MATERIALS;
        CPUTMaterial *pMaterial = pAssetLibrary->GetMaterial(materialName, false, this, ii, NULL, finalNumSystemMaterials, pFinalSystemNames);
        ASSERT( pMaterial, _L("Couldn't find material.") );

        delete []pFinalSystemNames;

        mpLayoutCount[ii] = pMaterial->GetMaterialEffectCount();
        SetMaterial(ii, pMaterial);

        // Release the extra refcount we're holding from the GetMaterial operation earlier
        // now the asset library, and this model have the only refcounts on that material
        pMaterial->Release();

        // Create two ID3D11InputLayout objects, one for each material.
     //   mpMesh[ii]->BindVertexShaderLayout( mpMaterial[ii], mpShadowCastMaterial);
        // mpShadowCastMaterial->Release()
    }


    return result;
}

// Set the material associated with this mesh and create/re-use a
//-----------------------------------------------------------------------------
void CPUTModelOGL::SetMaterial(UINT ii, CPUTMaterial *pMaterial)
{
    CPUTModel::SetMaterial(ii, pMaterial);

    // Can't bind the layout if we haven't loaded the mesh yet.
	CPUTMeshOGL *pMesh = (CPUTMeshOGL*)mpMesh[ii];
 //   D3D11_INPUT_ELEMENT_DESC *pDesc = pMesh->GetLayoutDescription();
  //  if( pDesc )
    {
   //     pMesh->BindVertexShaderLayout(pMaterial, mpMaterial[ii]);
    }
}

#ifdef SUPPORT_DRAWING_BOUNDING_BOXES
//-----------------------------------------------------------------------------
void CPUTModelOGL::DrawBoundingBox(CPUTRenderParameters &renderParams)
{
    UpdateShaderConstants(renderParams);

    UINT index  = mpSubMaterialCount[0] + CPUT_MATERIAL_INDEX_BOUNDING_BOX;
    CPUTMaterialOGL *pMaterial = (CPUTMaterialOGL*)(mpMaterial[0][index]);
    pMaterial->SetRenderStates(renderParams);

    ((CPUTMeshOGL*)mpBoundingBoxMesh)->Draw(renderParams, mpInputLayout[0][index]);
}

// Note that we only need one of these.  We don't need to re-create it for every model.
//-----------------------------------------------------------------------------
void CPUTModelDX11::CreateBoundingBoxMesh()
{
    CPUTResult result = CPUT_SUCCESS;

    float3 pVertices[8] = {
        float3(  1.0f,  1.0f,  1.0f ), // 0
        float3(  1.0f,  1.0f, -1.0f ), // 1
        float3( -1.0f,  1.0f,  1.0f ), // 2
        float3( -1.0f,  1.0f, -1.0f ), // 3
        float3(  1.0f, -1.0f,  1.0f ), // 4
        float3(  1.0f, -1.0f, -1.0f ), // 5
        float3( -1.0f, -1.0f,  1.0f ), // 6
        float3( -1.0f, -1.0f, -1.0f )  // 7
    };
    USHORT pIndices[24] = {
       0,1,  1,3,  3,2,  2,0,  // Top
       4,5,  5,7,  7,6,  6,4,  // Bottom
       0,4,  1,5,  2,6,  3,7   // Verticals
    };
    CPUTVertexElementDesc pVertexElements[] = {
        { CPUT_VERTEX_ELEMENT_POSITON, tFLOAT, 12, 0 },
    };

    CPUTMesh *pMesh = mpBoundingBoxMesh = new CPUTMeshDX11();
    pMesh->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_LINE_LIST);

    CPUTBufferElementInfo vertexElementInfo;
    vertexElementInfo.mpSemanticName         = "POSITION";
    vertexElementInfo.mSemanticIndex         = 0;
    vertexElementInfo.mElementType           = CPUT_F32;
    vertexElementInfo.mElementComponentCount = 3;
    vertexElementInfo.mElementSizeInBytes    = 12;
    vertexElementInfo.mOffset                = 0;

    CPUTBufferElementInfo indexDataInfo;
    indexDataInfo.mElementType           = CPUT_U16;
    indexDataInfo.mElementComponentCount = 1;
    indexDataInfo.mElementSizeInBytes    = sizeof(UINT16);
    indexDataInfo.mOffset                = 0;
    indexDataInfo.mSemanticIndex         = 0;
    indexDataInfo.mpSemanticName         = NULL;

    result = pMesh->CreateNativeResources(
        this, -1,
        1,
        &vertexElementInfo,
        8, // vertexCount
        pVertices,
        &indexDataInfo,
        24, // indexCount
        pIndices
    );
    ASSERT( CPUTSUCCESS(result), _L("Failed building bounding box mesh") );

    cString modelSuffix = ptoc(this);
    UINT index  = mpSubMaterialCount[0] + CPUT_MATERIAL_INDEX_BOUNDING_BOX;
    CPUTMaterialOGL *pMaterial = (CPUTMaterialOGL*)(mpMaterial[0][index]);
    ((CPUTMeshOGL*)pMesh)->BindVertexShaderLayout( pMaterial, &mpInputLayout[0][index] );
}
#endif
