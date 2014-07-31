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
#include "CPUT.h"
#include "CPUTAssetLibrary.h"
#include "CPUTMaterialEffect.h"

#ifdef CPUT_FOR_DX11
#include "CPUTMaterialEffectDX11.h"
#else    
	#if (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
	#include "CPUTMaterialEffectOGL.h"
	#else
		#error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
	#endif
#endif


//--------------------------------------------------------------------------------------
CPUTMaterialEffect *CPUTMaterialEffect::CreateMaterialEffect(
    const cString   &absolutePathAndFilename,
    const CPUTModel *pModel,
          int        meshIndex,
    CPUT_SHADER_MACRO* pShaderMacros, 
          int        externalCount,
          cString   *pExternalName,
          float4    *pExternals,
          int       *pExternalOffset,
          int       *pExternalSize
){
    // Create the material and load it from file.
#ifdef CPUT_FOR_DX11
	CPUTMaterialEffect *pMaterialEffect = new CPUTMaterialEffectDX11();
#else    
	#if (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
		CPUTMaterialEffect *pMaterialEffect = new CPUTMaterialEffectOGL();
	#else
		#error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
	#endif
#endif

    CPUTResult result = pMaterialEffect->LoadMaterialEffect( absolutePathAndFilename, pModel, meshIndex, pShaderMacros );
    ASSERT( CPUTSUCCESS(result), _L("\nError - CPUTAssetLibrary::GetMaterial() - Error in material file: '")+absolutePathAndFilename+_L("'") );
    UNREFERENCED_PARAMETER(result);

    // Add the material to the asset library.
    if( pModel && pMaterialEffect->MaterialRequiresPerModelPayload() )
    {
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterialEffect( absolutePathAndFilename, _L(""), _L(""), pMaterialEffect, pShaderMacros, pModel, meshIndex );
    } else
    {
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterialEffect( absolutePathAndFilename, _L(""), _L(""), pMaterialEffect, pShaderMacros );
    }

    return pMaterialEffect;
}