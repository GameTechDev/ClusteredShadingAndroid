/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or imlied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
