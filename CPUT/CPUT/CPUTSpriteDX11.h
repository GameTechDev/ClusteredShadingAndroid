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

#include "CPUT.h"
#include "d3d11.h"
#include "CPUTSprite.h"

class CPUTMaterial;
class CPUTRenderParameters;
class CPUTTexture;

class CPUTSpriteDX11 : public CPUTSprite
{
protected:

    ID3D11Buffer      *mpVertexBuffer;
    ID3D11InputLayout *mpInputLayout;

public:
    CPUTSpriteDX11()  : CPUTSprite(),
        mpInputLayout(NULL),
        mpVertexBuffer(NULL)
    {
    }
    ~CPUTSpriteDX11();
    static CPUTSpriteDX11* CreateSprite(
        float          spriteX,
        float          spriteY,
        float          spriteWidth,
        float          spriteHeight,
        const cString &materialName 
    );
    void DrawSprite( CPUTRenderParameters &renderParams ) { DrawSprite( renderParams, *mpMaterial ); }
    void DrawSprite( CPUTRenderParameters &renderParams, CPUTMaterial &material );
};

#endif // _CPUTSPRITE_H
