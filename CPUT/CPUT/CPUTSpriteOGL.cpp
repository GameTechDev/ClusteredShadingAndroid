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

#include "CPUTSpriteOGL.h"
#include "CPUTAssetLibrary.h"
#include "CPUTMaterialEffect.h"


//-----------------------------------------------
CPUTSpriteOGL::~CPUTSpriteOGL()
{
	SAFE_RELEASE( mpMesh );
}

//-----------------------------------------------
CPUTSpriteOGL*  CPUTSpriteOGL::CreateSprite(
    float          spriteX,
    float          spriteY,
    float          spriteWidth,
    float          spriteHeight,
    const cString &spriteMaterialName
)
{
	CPUTSpriteOGL* pCPUTSprite =  new CPUTSpriteOGL();

    // Create resources so we can draw a sprite using the render target as a texture
    pCPUTSprite->mpMaterial = CPUTAssetLibrary::GetAssetLibrary()->GetMaterial( spriteMaterialName, false );

    // Define the input layout
	CPUTBufferElementInfo pVertexInfo[3] = {
        { "POSITION", 0, 0, CPUT_F32, 3, 3*sizeof(float), 0 },            
        { "TEXCOORD", 0, 1, CPUT_F32, 2, 2*sizeof(float), 3*sizeof(float)}
	};
    const float top    = -spriteY; //-1.0f;
    const float bottom = -spriteY - spriteHeight; // 1.0f;
    const float left   =  spriteX; //-1.0f;
    const float right  =  spriteX + spriteWidth; // 1.0f;
    SpriteVertex pVertices[] = {
        {  left,    top, 1.0f,   0.0f, 1.0f },
        { right,    top, 1.0f,   1.0f, 1.0f },
        {  left, bottom, 1.0f,   0.0f, 0.0f },

        { right,    top, 1.0f,   1.0f, 1.0f },
        { right, bottom, 1.0f,   1.0f, 0.0f },
        {  left, bottom, 1.0f,   0.0f, 0.0f }
    };
	SAFE_RELEASE(pCPUTSprite->mpMesh);
	pCPUTSprite->mpMesh = new CPUTMeshOGL();
	pCPUTSprite->mpMesh->CreateNativeResources(NULL, 0, 2, pVertexInfo, 6, pVertices, NULL, 0, NULL);

	return pCPUTSprite;

} // CPUTSprite::CreateSprite()

//-----------------------------------------
void CPUTSpriteOGL::DrawSprite(
    CPUTRenderParameters &renderParams,
    CPUTMaterial         &material
)
{
	UINT finalMaterialIndex =  material.GetCurrentEffect();
	ASSERT(finalMaterialIndex< material.GetMaterialEffectCount(), _L("material index out of range."));
	CPUTMaterialEffect *pMaterialEffect = (CPUTMaterialEffect*)(material.GetMaterialEffects()[finalMaterialIndex]);
    pMaterialEffect->SetRenderStates(renderParams);

    mpMesh->Draw(renderParams, NULL);
} // CPUTSprite::DrawSprite()
