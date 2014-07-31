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
#ifndef _CPUTSPRITE_H
#define _CPUTSPRITE_H

#include "CPUTSprite.h"
#include "CPUT_OGL.h"
#include "CPUTMeshOGL.h"

class CPUTMaterial;
class CPUTRenderParameters;
class CPUTTexture;


class CPUTSpriteOGL : public CPUTSprite
{
protected:

	
	CPUTMeshOGL *mpMesh;
	

public:
    CPUTSpriteOGL() : CPUTSprite(),
        mpMesh(NULL)
    {
	}
    ~CPUTSpriteOGL();
    static CPUTSpriteOGL* CreateSprite(
        float          spriteX ,
        float          spriteY,
        float          spriteWidth,
        float          spriteHeight,
        const cString &materialName
    );
    void DrawSprite( CPUTRenderParameters &renderParams ) { DrawSprite( renderParams, *mpMaterial ); }
    void DrawSprite( CPUTRenderParameters &renderParams, CPUTMaterial &material );
};

#endif // _CPUTSPRITE_H
