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
#include "CPUTAssetLibrary.h"

#include "CPUTRenderStateBlock.h"
#ifdef CPUT_FOR_DX11
#include "CPUTRenderStateBlockDX11.h"
#elif (defined(CPUT_FOR_OGL) || defined (CPUT_FOR_OGLES))
    #include "CPUTRenderStateBlockOGL.h"
#else
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif


CPUTRenderStateBlock *CPUTRenderStateBlock::mpDefaultRenderStateBlock = NULL;

CPUTRenderStateBlock *CPUTRenderStateBlock::CreateRenderStateBlock( const cString &name, const cString &absolutePathAndFilename )
{
#ifdef CPUT_FOR_DX11
    CPUTRenderStateBlock *pRenderStateBlock = new CPUTRenderStateBlockDX11();
#elif (defined(CPUT_FOR_OGL) || defined (CPUT_FOR_OGLES))
    CPUTRenderStateBlock *pRenderStateBlock = new CPUTRenderStateBlockOGL();
#else
    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
#endif
    pRenderStateBlock->LoadRenderStateBlock( absolutePathAndFilename );

    // add to library
    CPUTAssetLibrary::GetAssetLibrary()->AddRenderStateBlock( absolutePathAndFilename, _L(""), _L(""), pRenderStateBlock );

    return pRenderStateBlock;
}
