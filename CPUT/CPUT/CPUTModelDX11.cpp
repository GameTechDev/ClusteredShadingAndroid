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
#include "CPUTModelDX11.h"
#include "CPUTMaterial.h"
#include "CPUTRenderParamsDX.h"
#include "CPUTFrustum.h"
#include "CPUTTextureDX11.h"
#include "CPUTBufferDX11.h"

DrawModelCallBackFunc CPUTModel::mDrawModelCallBackFunc = CPUTModelDX11::DrawModelCallBack;


// Return the mesh at the given index (cast to the GFX api version of CPUTMeshDX11)
//-----------------------------------------------------------------------------
CPUTMeshDX11* CPUTModelDX11::GetMesh(const UINT index) const
{
    return ( 0==mMeshCount || index > mMeshCount) ? NULL : (CPUTMeshDX11*)mpMesh[index];
}

// Set the render state before drawing this object
//-----------------------------------------------------------------------------
void CPUTModelDX11::UpdateShaderConstants(CPUTRenderParameters &renderParams)
{
    float4x4     world(*GetWorldMatrix());
    float4x4     NormalMatrix(*GetWorldMatrix());
    //TODO:  Is this supposed to be the NormalMatrix?
    float4x4     invWorld = world; invWorld.transpose(); invWorld.invert();

    //Local transform if node baked into animation
    if(mSkeleton && mpCurrentAnimation)
    {
        world        = GetParentsWorldMatrix();
        NormalMatrix = GetParentsWorldMatrix();
    }
    NormalMatrix.invert();
    NormalMatrix.transpose();

    CPUTCamera *pCamera   = renderParams.mpCamera;
    float4x4 viewProjection, worldViewProjection;
    float4 eyePosition;
    
    if( pCamera )
    {
        float4x4 view(*pCamera->GetViewMatrix());
        float4x4 projection(*pCamera->GetProjectionMatrix());
        viewProjection  = view * projection;
        worldViewProjection = world * viewProjection;
        eyePosition = float4(pCamera->GetPosition(), 0.0f);        
    }

    // Shadow camera
    CPUTCamera *pShadowCamera = renderParams.mpShadowCamera;
    float4x4 lightWorldViewProjection;
    if( pShadowCamera )
    {
        float4x4 shadowView, shadowProjection;
        shadowView = float4x4((float*)pShadowCamera->GetViewMatrix());
        shadowProjection = float4x4((float*)pShadowCamera->GetProjectionMatrix());
        lightWorldViewProjection = world * shadowView * shadowProjection;
    }

    ID3D11DeviceContext *pContext  = ((CPUTRenderParametersDX*)&renderParams)->mpContext;
    
    CPUTBufferDX11* pBuffer = (CPUTBufferDX11*)(renderParams.mpPerModelConstants);
    D3D11_MAPPED_SUBRESOURCE mapInfo = pBuffer->MapBuffer(renderParams, CPUT_MAP_WRITE_DISCARD);
    CPUTModelConstantBuffer *pCb = (CPUTModelConstantBuffer*)mapInfo.pData;


    pCb->World = world;
    pCb->NormalMatrix = NormalMatrix;
    pCb->WorldViewProjection = worldViewProjection;
    pCb->InverseWorld = invWorld;
    pCb->LightWorldViewProjection = lightWorldViewProjection;
    pCb->BoundingBoxCenterWorldSpace =  float4(mBoundingBoxCenterWorldSpace, 0.0f);
    pCb->BoundingBoxHalfWorldSpace =    float4(mBoundingBoxHalfWorldSpace, 0.0f);
    pCb->BoundingBoxCenterObjectSpace = float4(mBoundingBoxCenterObjectSpace, 0.0f);
    pCb->BoundingBoxHalfObjectSpace =   float4(mBoundingBoxHalfObjectSpace, 0.0f);

    //TODO: Should this process if object not visible?
    //Only do this if Model has a skin and is animated
    if(mSkeleton && mpCurrentAnimation)
    {
        ASSERT(0, _L("Animation constant buffer temporarily broken"));
        //ASSERT(mSkeleton->mNumberOfJoints < 255, _L("Skin Exceeds maximum number of allowable joints: 255"));
        //for(UINT i = 0; i < mSkeleton->mNumberOfJoints; ++i)
        //{
        //    CPUTJoint *pJoint = &mSkeleton->mJointsList[i];
        //    pCb->SkinMatrix[i] = pJoint->mInverseBindPoseMatrix * pJoint->mScaleMatrix * pJoint->mRTMatrix;
        //    float4x4 skinNormalMatrix = pCb->SkinMatrix[i];
        //    skinNormalMatrix.invert(); skinNormalMatrix.transpose();  
        //    pCb->SkinNormalMatrix[i] = skinNormalMatrix;
        //}
    }
    
    pBuffer->UnmapBuffer(renderParams);
}

// Render this model. Render only this model, not its children or siblings.
//-----------------------------------------------------------------------------
void CPUTModelDX11::Render(CPUTRenderParameters &renderParams, int materialIndex)
{
    CPUTRenderParametersDX *pParams = (CPUTRenderParametersDX*)&renderParams;
    CPUTCamera             *pCamera = pParams->mpCamera;

    // TODO: Move bboxDirty to member and set only when model moves.
    bool bboxDirty = true;
    if( bboxDirty )
    {
        UpdateBoundsWorldSpace();
    }

    if( renderParams.mDrawModels && !pParams->mRenderOnlyVisibleModels || !pCamera || pCamera->mFrustum.IsVisible( mBoundingBoxCenterWorldSpace, mBoundingBoxHalfWorldSpace ) )
    {
        // Update the model's constant buffer.
        // Note that the materials reference this, so we need to do it only once for all of the model's meshes.
        UpdateShaderConstants(renderParams);

        // loop over all meshes in this model and draw them
        for(UINT ii=0; ii<mMeshCount; ii++)
        {
            UINT finalMaterialIndex = GetMaterialIndex(ii, materialIndex);
            ASSERT( finalMaterialIndex < mpLayoutCount[ii], _L("material index out of range."));
            CPUTMaterialEffect *pMaterialEffect = (CPUTMaterialEffect*)(mpMaterialEffect[ii][finalMaterialIndex]);

            //UINT OriginalMaterialIndex = GetMaterialIndex(ii, 0);
            //ASSERT( OriginalMaterialIndex < mpRootMaterial[ii]->GetSubMaterialCount(), _L("material index out of range."));
            //CPUTMaterialDX11 *pOriginalMaterial = (CPUTMaterialDX11*)(mpRootMaterial[ii]->GetSubMaterials()[OriginalMaterialIndex]);

			mDrawModelCallBackFunc(this, renderParams, mpMesh[ii],pMaterialEffect,NULL,mpInputLayout[ii][finalMaterialIndex]);
        }
    }
#ifdef SUPPORT_DRAWING_BOUNDING_BOXES 
    if( renderParams.mShowBoundingBoxes && (!pCamera || pCamera->mFrustum.IsVisible( mBoundingBoxCenterWorldSpace, mBoundingBoxHalfWorldSpace )))
    {
        DrawBoundingBox( renderParams );
    }
#endif
}

bool CPUTModelDX11::DrawModelCallBack(CPUTModel*, CPUTRenderParameters &renderParams, CPUTMesh* pMesh, CPUTMaterialEffect* pMaterial, CPUTMaterialEffect* pOriginalMaterial, void* pInputLayout)
{
	ID3D11InputLayout *pLayout = (ID3D11InputLayout *)pInputLayout;
	pMaterial->SetRenderStates(renderParams);

    ((CPUTMeshDX11*)pMesh)->Draw(renderParams, pLayout);

	return true;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTModelDX11::LoadModel(CPUTConfigBlock *pBlock, int *pParentID, CPUTModel *pMasterModel, int numSystemMaterials, cString *pSystemMaterialNames)
{
    CPUTResult result = CPUT_SUCCESS;
    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();

    cString modelSuffix = ptoc(this);

    // set the model's name
    mName = pBlock->GetValueByName(_L("name"))->ValueAsString();
    mName = mName + _L(".mdl");

    // resolve the full path name
    cString modelLocation;
    cString resolvedPathAndFile;
    modelLocation = ((CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary())->GetModelDirectoryName();
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

    // Get Skin Info
    CPUTConfigEntry *pSkinValue = pBlock->GetValueByName( _L("skeleton") );
    if(pSkinValue != &CPUTConfigEntry::sNullConfigValue)
    {
        cString name = ((CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary())->GetAnimationSetDirectoryName() +pSkinValue->ValueAsString() + _L(".skl");
        cString path;
        CPUTFileSystem::ResolveAbsolutePathAndFilename(name,&path);
        mSkeleton = new CPUTSkeleton;
        mSkeleton->LoadSkeleton(path);

    }

    mMeshCount = pBlock->GetValueByName(_L("meshcount"))->ValueAsInt();
    mpMesh     = new CPUTMesh*[mMeshCount];
    mpLayoutCount = new UINT[mMeshCount];
    mpRootMaterial = new CPUTMaterial*[mMeshCount];
    memset( mpRootMaterial, 0, mMeshCount * sizeof(CPUTMaterial*) );
    mpInputLayout = new ID3D11InputLayout**[mMeshCount];
    memset( mpInputLayout, 0, mMeshCount * sizeof(ID3D11InputLayout**) );
    mpMaterialEffect = new CPUTMaterialEffect**[mMeshCount];
    memset( mpMaterialEffect, 0, mMeshCount * sizeof(CPUTMaterialEffect*) );

    cString materialName, shadowMaterialName;
    char pNumber[4];
    cString materialValueName;

    CPUTModelDX11 *pMasterModelDX = (CPUTModelDX11*)pMasterModel;

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
            mpMesh[ii] = new CPUTMeshDX11();
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
        _itoa_s(ii, pNumber, 4, 10);
        cString meshNumberAsString = s2ws(pNumber);
        materialValueName = _L("material") + meshNumberAsString;
        materialName = pBlock->GetValueByName(materialValueName)->ValueAsString();

        shadowMaterialName = pBlock->GetValueByName(_L("shadowCast") + materialValueName)->ValueAsString();
        bool isSkinned = pBlock->GetValueByName(_L("skeleton")) != &CPUTConfigEntry::sNullConfigValue;
        if( shadowMaterialName.length() == 0 )
        {
            if(!isSkinned)
            {
                shadowMaterialName = _L("%ShadowCast");
            }
            else
            {
                shadowMaterialName = _L("%ShadowCastSkinned");
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

        pFinalSystemNames[totalNameCount + CPUT_MATERIAL_INDEX_SHADOW_CAST]  = shadowMaterialName;
        pFinalSystemNames[totalNameCount + CPUT_MATERIAL_INDEX_BOUNDING_BOX] = _L("%BoundingBox");
        int finalNumSystemMaterials = numSystemMaterials + NUM_GLOBAL_SYSTEM_MATERIALS;
        CPUTMaterial *pMaterial = pAssetLibrary->GetMaterial(materialName, false, this, ii, NULL, finalNumSystemMaterials, pFinalSystemNames);
        ASSERT( pMaterial, _L("Couldn't find material.") );

        delete []pFinalSystemNames;

        mpLayoutCount[ii] = pMaterial->GetMaterialEffectCount();
        HEAPCHECK;
        SetMaterial(ii, pMaterial);
        HEAPCHECK;
  
        // Release the extra refcount we're holding from the GetMaterial operation earlier
        // now the asset library, and this model have the only refcounts on that material
        pMaterial->Release();
    }

    return result;
}

//-----------------------------------------------------------------------------
void CPUTModelDX11::SetMaterial(UINT ii, CPUTMaterial *pMaterial)
{
    CPUTModel::SetMaterial(ii, pMaterial);

    // Can't bind the layout if we haven't loaded the mesh yet.
    CPUTMeshDX11 *pMesh = (CPUTMeshDX11*)mpMesh[ii];
    D3D11_INPUT_ELEMENT_DESC *pDesc = pMesh->GetLayoutDescription();

    // Release the old input layouts and delete the layout array.
    if( mpInputLayout[ii] )
    {
        for( UINT jj=0; jj<mpLayoutCount[ii]; jj++ )
        {
            SAFE_RELEASE(mpInputLayout[ii][jj]);
        }
        SAFE_DELETE(mpInputLayout[ii]);
    }

    mpInputLayout[ii] = new ID3D11InputLayout*[mpLayoutCount[ii]];
    memset( mpInputLayout[ii], 0, sizeof(ID3D11InputLayout*) * mpLayoutCount[ii] );
    for( UINT jj=0; jj<mpLayoutCount[ii]; jj++ )
    {
        if( pDesc )
        {
            pMesh->BindVertexShaderLayout((CPUTMaterialEffect*)mpMaterialEffect[ii][jj], &mpInputLayout[ii][jj]);

            HEAPCHECK;
        }
    }
}

#ifdef SUPPORT_DRAWING_BOUNDING_BOXES
//-----------------------------------------------------------------------------
void CPUTModelDX11::DrawBoundingBox(CPUTRenderParameters &renderParams)
{
    UpdateShaderConstants(renderParams);

    UINT index  = mpSubMaterialCount[0] + CPUT_MATERIAL_INDEX_BOUNDING_BOX;
    CPUTMaterialDX11 *pMaterial = (CPUTMaterialDX11*)(mpMaterial[0][index]);
    pMaterial->SetRenderStates(renderParams);

    ((CPUTMeshDX11*)mpBoundingBoxMesh)->Draw(renderParams, mpInputLayout[0][index]);
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
    CPUTMaterialDX11 *pMaterial = (CPUTMaterialDX11*)(mpMaterial[0][index]);
    ((CPUTMeshDX11*)pMesh)->BindVertexShaderLayout( pMaterial, &mpInputLayout[0][index] );
}
#endif

