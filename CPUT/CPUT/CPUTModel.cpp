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
#include "CPUTModel.h"
#include "CPUTMaterial.h"
#include "CPUTMesh.h"
#include "CPUTAssetLibrary.h"
#include "float.h"
#include <fstream>

#if (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
#include "CPUTModelOGL.h"
CPUTModel* CPUTModel::CreateModel()
{
    return (CPUTModel*)(new CPUTModelOGL());
}
#elif CPUT_FOR_DX11
#include "CPUTModelDX11.h"
CPUTModel* CPUTModel::CreateModel()
{
    return (CPUTModel*)(new CPUTModelDX11());
}
#else
#error "CPUTModel no API specified"
#endif

//-----------------------------------------------------------------------------
CPUTModel::~CPUTModel()
{
    // TODO: Do we need a shadowCastMaterial per mesh?  What happens if the meshes have different layouts?
    for( UINT ii=0; ii<mMeshCount; ii++ )
    {
        for( UINT jj=0; jj<mpLayoutCount[ii]; jj++ )
        {
            SAFE_RELEASE(mpMaterialEffect[ii][jj]);
        }
        SAFE_DELETE( mpMaterialEffect[ii] );

        SAFE_RELEASE( mpRootMaterial[ii] );


        HEAPCHECK;
        if( mpMesh[ii] )
        {
            mpMesh[ii]->DecrementInstanceCount();
        }
        SAFE_RELEASE(mpMesh[ii]);
        HEAPCHECK;
    }

    SAFE_RELEASE(mpBoundingBoxMesh);
    SAFE_DELETE_ARRAY(mpRootMaterial);
    SAFE_DELETE_ARRAY(mpLayoutCount);
    SAFE_DELETE_ARRAY(mpMaterialEffect);

    SAFE_DELETE_ARRAY(mpMesh);

    if(mSkeleton)
    {
        delete mSkeleton;
    }
}

//-----------------------------------------------------------------------------
void CPUTModel::GetBoundsObjectSpace(float3 *pCenter, float3 *pHalf)
{
    *pCenter = mBoundingBoxCenterObjectSpace;
    *pHalf   = mBoundingBoxHalfObjectSpace;
}

//-----------------------------------------------------------------------------
void CPUTModel::GetBoundsWorldSpace(float3 *pCenter, float3 *pHalf)
{
    *pCenter = mBoundingBoxCenterWorldSpace;
    *pHalf   = mBoundingBoxHalfWorldSpace;
}

//-----------------------------------------------------------------------------
void CPUTModel::UpdateBoundsWorldSpace()
{
    // If an object is rigid, then it's object-space bounding box doesn't change.
    // However, if it moves, then it's world-space bounding box does change.
    // Call this function when the model moves

    float4x4 *pWorld =  GetWorldMatrix();
    float4 center    =  float4(mBoundingBoxCenterObjectSpace, 1.0f); // W = 1 because we want the xlation (i.e., center is a position)
    float4 half      =  float4(mBoundingBoxHalfObjectSpace,   0.0f); // W = 0 because we don't want xlation (i.e., half is a direction)

    // TODO: optimize this
    float4 positions[8] = {
        center + float4( 1.0f, 1.0f, 1.0f, 0.0f ) * half,
        center + float4( 1.0f, 1.0f,-1.0f, 0.0f ) * half,
        center + float4( 1.0f,-1.0f, 1.0f, 0.0f ) * half,
        center + float4( 1.0f,-1.0f,-1.0f, 0.0f ) * half,
        center + float4(-1.0f, 1.0f, 1.0f, 0.0f ) * half,
        center + float4(-1.0f, 1.0f,-1.0f, 0.0f ) * half,
        center + float4(-1.0f,-1.0f, 1.0f, 0.0f ) * half,
        center + float4(-1.0f,-1.0f,-1.0f, 0.0f ) * half
    };

    float4 minPosition( FLT_MAX,  FLT_MAX,  FLT_MAX, 1.0f );
    float4 maxPosition(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f );
    for( UINT ii=0; ii<8; ii++ )
    {
        float4 position = positions[ii] * *pWorld;
        minPosition = Min( minPosition, position );
        maxPosition = Max( maxPosition, position );
    }
    mBoundingBoxCenterWorldSpace = (maxPosition + minPosition) * 0.5f;
    mBoundingBoxHalfWorldSpace   = (maxPosition - minPosition) * 0.5f;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTModel::LoadModelPayload(const cString &FileName)
{
    CPUTResult result = CPUT_SUCCESS;

    CPUTFileSystem::CPUTOSifstream file(FileName, std::ios::in | std::ios::binary);

    ASSERT( !file.fail(), _L("CPUTModel::LoadModelPayload() - Could not find binary model file: ") + FileName );

    // set up for mesh creation loop
    UINT meshIndex = 0;
    while(file.good() && !file.eof())
    {
        // TODO: rearrange while() to avoid if(eof).  Should perform only one branch per loop iteration, not two
        CPUTRawMeshData vertexFormatDesc;
        vertexFormatDesc.Read(file);
        if(file.eof())
        {
            // TODO:  Wtf?  Why would we get here?  We check eof at the top of loop.  If it isn't eof there, why is it eof here?
            break;
        }
        ASSERT( meshIndex < mMeshCount, _L("Actual mesh count doesn't match stated mesh count"));

        // create the mesh.
        CPUTMesh *pMesh = mpMesh[meshIndex];

        // always a triangle list (at this point)
        pMesh->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);

        // get number of data blocks in the vertex element (pos,norm,uv,etc)
        CPUTBufferElementInfo *pVertexElementInfo = new CPUTBufferElementInfo[vertexFormatDesc.mFormatDescriptorCount];

        // running count of each type of  element
        int positionStreamCount=0;
        int normalStreamCount=0;
        int texCoordStreamCount=0;
        int tangentStreamCount=0;
        int binormalStreamCount=0;
        int colorStreamCount=0;
		int blendWeightStreamCount = 0;
		int blendIndexStreamCount = 0;

        int runningOffset = 0;
        for(UINT ii=0; ii<vertexFormatDesc.mFormatDescriptorCount; ii++)
        {
            // lookup the CPUT data type equivalent
            pVertexElementInfo[ii].mElementType = CPUT_FILE_ELEMENT_TYPE_TO_CPUT_TYPE_CONVERT(vertexFormatDesc.mpElements[ii].mVertexElementType);
            ASSERT((pVertexElementInfo[ii].mElementType !=CPUT_UNKNOWN ) , _L(".MDL file load error.  This model file has an unknown data type in it's model data."));
            // calculate the number of elements in this stream block (i.e. F32F32F32 = 3xF32)
            pVertexElementInfo[ii].mElementComponentCount = vertexFormatDesc.mpElements[ii].mElementSizeInBytes/CPUT_DATA_FORMAT_SIZE[pVertexElementInfo[ii].mElementType];
            // store the size of each element type in bytes (i.e. 3xF32, each element = F32 = 4 bytes)
            pVertexElementInfo[ii].mElementSizeInBytes = vertexFormatDesc.mpElements[ii].mElementSizeInBytes;
            // store the number of elements (i.e. 3xF32, 3 elements)
            // calculate the offset from the first element of the stream - assumes all blocks appear in the vertex stream as the order that appears here
            pVertexElementInfo[ii].mOffset = runningOffset;
            runningOffset = runningOffset + pVertexElementInfo[ii].mElementSizeInBytes;

            // extract the name of stream
            pVertexElementInfo[ii].mpSemanticName = CPUT_VERTEX_ELEMENT_SEMANTIC_AS_STRING[ii];

            //TODO: Calculate Opengl semantic index elsewhere
            switch(vertexFormatDesc.mpElements[ii].mVertexElementSemantic)
            {
			//FIXME - this isn't right, and needs to change for DX and OpenGL
			//Probably just need to move semantic bind point into OpenGL, or something.
            //Currently, TEXCOORD is the only semantic with multiples in common use. Adding
            //semantic index works provided addtional attributes (e.g. vertex color) are not
            //present
            case eCPUT_VERTEX_ELEMENT_SEMANTIC::CPUT_VERTEX_ELEMENT_POSITON:
                pVertexElementInfo[ii].mpSemanticName = "POSITION";
                pVertexElementInfo[ii].mSemanticIndex = positionStreamCount++;
				pVertexElementInfo[ii].mBindPoint = CPUTSemanticBindPoint::POSITION;
                break;
            case eCPUT_VERTEX_ELEMENT_SEMANTIC::CPUT_VERTEX_ELEMENT_NORMAL:
                pVertexElementInfo[ii].mpSemanticName = "NORMAL";
                pVertexElementInfo[ii].mSemanticIndex = normalStreamCount++;
				pVertexElementInfo[ii].mBindPoint = CPUTSemanticBindPoint::NORMAL;
                break;
            case eCPUT_VERTEX_ELEMENT_SEMANTIC::CPUT_VERTEX_ELEMENT_TEXTURECOORD:
                pVertexElementInfo[ii].mpSemanticName = "TEXCOORD";
				pVertexElementInfo[ii].mSemanticIndex = texCoordStreamCount++;
                pVertexElementInfo[ii].mBindPoint = CPUTSemanticBindPoint::TEXCOORD + pVertexElementInfo[ii].mSemanticIndex;
                break;
            case eCPUT_VERTEX_ELEMENT_SEMANTIC::CPUT_VERTEX_ELEMENT_TANGENT:
                pVertexElementInfo[ii].mpSemanticName = "TANGENT";
                pVertexElementInfo[ii].mSemanticIndex = tangentStreamCount++;
				pVertexElementInfo[ii].mBindPoint = CPUTSemanticBindPoint::TANGENT;
                break;
            case eCPUT_VERTEX_ELEMENT_SEMANTIC::CPUT_VERTEX_ELEMENT_BINORMAL:
                pVertexElementInfo[ii].mpSemanticName = "BINORMAL";
                pVertexElementInfo[ii].mSemanticIndex = binormalStreamCount++;
				pVertexElementInfo[ii].mBindPoint = CPUTSemanticBindPoint::BINORMAL;
                break;
            case eCPUT_VERTEX_ELEMENT_SEMANTIC::CPUT_VERTEX_ELEMENT_VERTEXCOLOR:
                pVertexElementInfo[ii].mpSemanticName = "COLOR";
                pVertexElementInfo[ii].mSemanticIndex = colorStreamCount++;
				pVertexElementInfo[ii].mBindPoint = CPUTSemanticBindPoint::COLOR;
                break;
            default:
                cString errorString = _L("Invalid vertex semantic in: '")+FileName+_L("'\n");
                TRACE(errorString.c_str());
                ASSERT(0, errorString);
            }
        }

        // Index buffer
        CPUTBufferElementInfo indexDataInfo;
        indexDataInfo.mElementType           = (vertexFormatDesc.mIndexType == eCPUT_VERTEX_ELEMENT_TYPE::tUINT32) ? CPUT_U32 : CPUT_U16;
        indexDataInfo.mElementComponentCount = 1;
        indexDataInfo.mElementSizeInBytes    = (vertexFormatDesc.mIndexType == eCPUT_VERTEX_ELEMENT_TYPE::tUINT32) ? sizeof(uint32_t) : sizeof(uint16_t);
        indexDataInfo.mOffset                = 0;
        indexDataInfo.mSemanticIndex         = 0;
        indexDataInfo.mpSemanticName         = NULL;

        if( vertexFormatDesc.mVertexCount && vertexFormatDesc.mIndexCount )
        {
            result = pMesh->CreateNativeResources(
                this,
                meshIndex,
                vertexFormatDesc.mFormatDescriptorCount,
                pVertexElementInfo,
                vertexFormatDesc.mVertexCount,
                (void*)vertexFormatDesc.mpVertices,
                &indexDataInfo,
                vertexFormatDesc.mIndexCount,
                vertexFormatDesc.mpIndices
            );
            if(CPUTFAILED(result))
            {
                return result;
            }
        }
        delete [] pVertexElementInfo;
        pVertexElementInfo = NULL;
        ++meshIndex;
    }
    ASSERT( file.eof(), _L("") );

    // close file
    file.close();
    
    return result;
}

// Set the material associated with this mesh and create/re-use a
void CPUTModel::SetMaterial(UINT ii, CPUTMaterial *pMaterial)
{
    // release previous submaterials

	SAFE_RELEASE(mpRootMaterial[ii]);

	mpRootMaterial[ii] = pMaterial;
	pMaterial->AddRef();

    // release previous submaterials
    CPUTMaterialEffect **pCur = mpMaterialEffect[ii];
    if( pCur )
    {
        while( *pCur )
        {
            SAFE_RELEASE( *pCur++ );
        }
        SAFE_DELETE( mpMaterialEffect[ii] );
    }
    // Count the new submaterials
    UINT count = 0;
	pCur = pMaterial->GetMaterialEffects();
    while( *pCur++ )
    {
        count++;
    }

    // Copy the pointers to the sub materials
    mpLayoutCount[ii] = count;
    mpMaterialEffect[ii] = new CPUTMaterialEffect*[count+1];
    for( UINT jj=0; jj<count; jj++ )
    {
        mpMaterialEffect[ii][jj] = pMaterial->GetMaterialEffects()[jj];
        if( mpMaterialEffect[ii][jj] )
        {
			bool needClone = pMaterial->GetMaterialEffects()[jj]->MaterialRequiresPerModelPayload();
			if( needClone )
			{
				mpMaterialEffect[ii][jj] = pMaterial->GetMaterialEffects()[jj]->CloneMaterialEffect( this, ii);
			}
			else
        {
	            mpMaterialEffect[ii][jj]->AddRef();
			}
        } 
		else
        {
            mpMaterialEffect[ii][jj] = NULL;
        }
    }
    mpMaterialEffect[ii][count] = NULL;
}

//-----------------------------------------------------------------------------
UINT CPUTModel::GetMaterialIndex(int meshIndex, int index)
{
	return index >= 0 ? mpRootMaterial[meshIndex]->GetCurrentEffect() : mpRootMaterial[meshIndex]->GetMaterialEffectCount() + index;
}

void CPUTModel::UpdateRecursive( float deltaSeconds )
{
    if(mSkeleton && mpCurrentAnimation)
    {
        std::vector<CPUTNodeAnimation * > *jointAnimation = mpCurrentAnimation->FindJointNodeAnimation(mSkeleton->mJointsList[0].mName);
        for(UINT i = 0; i < mSkeleton->mNumberOfJoints && jointAnimation != NULL; ++i)
        {
            float4x4 worldXform = (*jointAnimation)[i]->Interpolate(mAnimationTime,mSkeleton->mJointsList[i],mIsLoop);
            UINT parentId = (UINT)mSkeleton->mJointsList[i].mParentIndex;
            
            if( parentId < 255)
            {
                mSkeleton->mJointsList[i].mRTMatrix =  worldXform * mSkeleton->mJointsList[parentId].mRTMatrix;
            }
            else
            {
                mSkeleton->mJointsList[i].mRTMatrix = worldXform;
            }
        }
        mAnimationTime += deltaSeconds * mPlaybackSpeed;
    }
    else if(mpCurrentNodeAnimation != NULL && mpCurrentNodeAnimation->IsValidAnimation())
    {
        SetParentMatrix(mpCurrentNodeAnimation->Interpolate(mAnimationTime,mIsLoop));
        mAnimationTime += deltaSeconds * mPlaybackSpeed;
    }

    Update(deltaSeconds);

    if(mpSibling)
    {
        mpSibling->UpdateRecursive(deltaSeconds);
    }
    if(mpChild)
    {
        mpChild->UpdateRecursive(deltaSeconds);
    }
}
