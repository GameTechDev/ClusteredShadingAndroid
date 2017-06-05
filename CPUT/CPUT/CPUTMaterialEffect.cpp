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