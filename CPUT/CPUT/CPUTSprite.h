/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPUTSprite_H
#define _CPUTSprite_H

#include "CPUT.h"
#include "CPUTMaterial.h"

class CPUTRenderParameters;

class CPUTSprite
{
protected:
    class SpriteVertex
    {
    public:
        float mpPos[3];
        float mpUV[2];
    };

    CPUTMaterial      *mpMaterial;

public:
    CPUTSprite() :
        mpMaterial(NULL)
    {
    }
    virtual ~CPUTSprite()
	{
		SAFE_RELEASE( mpMaterial );
	}
    void DrawSprite( CPUTRenderParameters &renderParams ) { DrawSprite( renderParams, *mpMaterial ); }
    virtual void DrawSprite( CPUTRenderParameters &renderParams, CPUTMaterial &material )=0;

	static CPUTSprite *CreateSprite(
		float          spriteX = -1.0f,
        float          spriteY = -1.0f,
        float          spriteWidth  = 2.0f,
        float          spriteHeight = 2.0f,
        const cString &materialName = cString(_L("Sprite"))
		);

};

#endif 
