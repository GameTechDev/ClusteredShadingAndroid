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
#include "CPUTMaterial.h"
//#ifdef CPUT_FOR_DX11
//#include "CPUTMaterialDX11.h"
//#elif (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
//#include "CPUTMaterialOGL.h"
//#else    
//#error "No gfx APi defined"
//#endif
CPUTConfigBlock CPUTMaterial::mGlobalProperties;
//--------------------------------------------------------------------------------------
CPUTMaterial *CPUTMaterial::CreateMaterial(
    const cString   &absolutePathAndFilename,
    const CPUTModel *pModel,
    int        meshIndex,
    CPUT_SHADER_MACRO* pShaderMacros, // Note: this is honored only on first load.  Subsequent GetMaterial calls will return the material with shaders as compiled with original macros.
    int        numSystemMaterials,
    cString   *pSystemMaterialNames,
    int        externalCount,
    cString   *pExternalName,
    float4    *pExternals,
    int       *pExternalOffset,
    int       *pExternalSize
    ){
        // Create the material and load it from file.
        //#ifdef CPUT_FOR_DX11
        //    CPUTMaterial *pMaterial = new CPUTMaterialDX11();
        //#elif (defined(CPUT_FOR_OGL) || defined(CPUT_FOR_OGLES))
        //    CPUTMaterial *pMaterial = new CPUTMaterialOGL();
        //#else    
        //    #error You must supply a target graphics API (ex: #define CPUT_FOR_DX11), or implement the target API for this file.
        //#endif

        //    pMaterial->mpSubMaterials = NULL;
        CPUTMaterial* pMaterial = new CPUTMaterial();

        CPUTResult result = pMaterial->LoadMaterial( absolutePathAndFilename, pModel, meshIndex, pShaderMacros, numSystemMaterials, pSystemMaterialNames, externalCount, pExternalName, pExternals, pExternalOffset, pExternalSize );
        ASSERT( CPUTSUCCESS(result), _L("\nError - CPUTAssetLibrary::GetMaterial() - Error in material file: '")+absolutePathAndFilename+_L("'") );
        UNREFERENCED_PARAMETER(result);

        // Add the material to the asset library.
        CPUTAssetLibrary::GetAssetLibrary()->AddMaterial( absolutePathAndFilename, _L(""), _L(""), pMaterial, pShaderMacros );

        return pMaterial;
}

//-----------------------------------------------------------------------------
void ReadMacrosFromConfigBlock(
    CPUTConfigBlock   *pMacrosBlock,
    CPUT_SHADER_MACRO  *pShaderMacros,
    CPUT_SHADER_MACRO **pUserSpecifiedMacros,
    int               *pNumUserSpecifiedMacros,
    CPUT_SHADER_MACRO **pFinalShaderMacros
    ){
        *pNumUserSpecifiedMacros = pMacrosBlock->ValueCount();

        // Count the number of macros passed in
        CPUT_SHADER_MACRO *pMacro = (CPUT_SHADER_MACRO*)pShaderMacros;
        int numPassedInMacros = 0;
        if( pMacro )
        {
            while( pMacro->Name )
            {
                ++numPassedInMacros;
                ++pMacro;
            }
        }

        // Allocate an array of macro pointer large enough to contain the passed-in macros plus those specified in the .mtl file.
        *pFinalShaderMacros = new CPUT_SHADER_MACRO[*pNumUserSpecifiedMacros + numPassedInMacros + 1];

        // Copy the passed-in macro pointers to the final array
        int jj;
        for( jj=0; jj<numPassedInMacros; jj++ )
        {
            (*pFinalShaderMacros)[jj] = *(CPUT_SHADER_MACRO*)&pShaderMacros[jj];
        }

        // Create a CPUT_SHADER_MACRO for each of the macros specified in the .mtl file.
        // And, add their pointers to the final array
        *pUserSpecifiedMacros = new CPUT_SHADER_MACRO[*pNumUserSpecifiedMacros];
        for( int kk=0; kk<*pNumUserSpecifiedMacros; kk++, jj++ )
        {
            CPUTConfigEntry *pValue   = pMacrosBlock->GetValue(kk);
            (*pUserSpecifiedMacros)[kk].Name       = ws2s(pValue->NameAsString());
            (*pUserSpecifiedMacros)[kk].Definition = ws2s(pValue->ValueAsString());
            (*pFinalShaderMacros)[jj] = (*pUserSpecifiedMacros)[kk];
        }
        (*pFinalShaderMacros)[jj].Name = NULL;
        (*pFinalShaderMacros)[jj].Definition = NULL;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTMaterial::LoadMaterial(
    const cString   &fileName,
    const CPUTModel *pModel,
    int        meshIndex,
    CPUT_SHADER_MACRO* pShaderMacros,
    int        systemMaterialCount,
    cString   *pSystemMaterialNames,
    int        externalCount,
    cString   *pExternalName,
    float4    *pExternals,
    int       *pExternalOffset,
    int       *pExternalSize
    ){
        CPUTResult result = CPUT_SUCCESS;

        mMaterialName = fileName;
        mMaterialNameHash = CPUTComputeHash( mMaterialName );

        // Open/parse the file
        CPUTConfigFile file;
        result = file.LoadFile(fileName);
        if(CPUTFAILED(result))
        {
            return result;
        }

        // Make a local copy of all the parameters
        CPUTConfigBlock *pBlock = file.GetBlock(0);
        ASSERT( pBlock, _L("Error getting parameter block") );
        if( !pBlock )
        {
            return CPUT_ERROR_PARAMETER_BLOCK_NOT_FOUND;
        }
        mConfigBlock = *pBlock;

        CPUTAssetLibrary *pAssetLibrary = (CPUTAssetLibrary*)CPUTAssetLibrary::GetAssetLibrary();


        mMaterialEffectCount = 0;
        if( mConfigBlock.GetValueByName( _L("MultiMaterial") )->ValueAsBool() )
        {
            // Count materials;
            for(;;)
            {
                CPUTConfigEntry *pValue = mConfigBlock.GetValueByName( _L("Material") + itoc(mMaterialEffectCount) );
                if( pValue->IsValid() )
                {
                    ++mMaterialEffectCount;
                } else
                {
                    break;
                }
            }
            ASSERT(mMaterialEffectCount != 0, _L("MultiMaterial specified, but no sub materials given.") );

            // Reserve space for "authored" materials plus system materials
            mpMaterialEffectNames = new cString[mMaterialEffectCount+systemMaterialCount];
            int ii;
            for( ii=0; ii<mMaterialEffectCount; ii++ )
            {
                CPUTConfigEntry *pValue = mConfigBlock.GetValueByName( _L("Material") + itoc(ii) );
                mpMaterialEffectNames[ii] = pAssetLibrary->GetMaterialEffectPath(pValue->ValueAsString(), false);
            }
        }
        else
        {
            mMaterialEffectCount = 1;
            mpMaterialEffectNames = new cString[mMaterialEffectCount+systemMaterialCount];
            mpMaterialEffectNames[0] = fileName;
        }


        CPUT_SHADER_MACRO *pFinalShaderMacros = (CPUT_SHADER_MACRO*)pShaderMacros;
        CPUT_SHADER_MACRO *pUserSpecifiedMacros = NULL;
        // Read additional macros from .mtl file
        cString macroBlockName = _L("defines") + itoc(0);
        CPUTConfigBlock *pMacrosBlock = file.GetBlockByName(macroBlockName);
        int numUserSpecifiedMacros = 0;
        if( pMacrosBlock )
        {
            ReadMacrosFromConfigBlock( pMacrosBlock, pShaderMacros, &pUserSpecifiedMacros, &numUserSpecifiedMacros, &pFinalShaderMacros );
        } 

        // The real material count includes the system material count
        mMaterialEffectCount += systemMaterialCount;
        for( int ii=0; ii<systemMaterialCount; ii++ )
        {
            
            // System materials "grow" from the end; the 1st system material is at the end of the list.
            mpMaterialEffectNames[mMaterialEffectCount-systemMaterialCount+ii] = pSystemMaterialNames[ii];
        }
        mpMaterialEffects = new CPUTMaterialEffect*[mMaterialEffectCount+1];
        for( int ii=0; ii<mMaterialEffectCount; ii++ )
        {
            mpMaterialEffects[ii] = pAssetLibrary->GetMaterialEffect( mpMaterialEffectNames[ii], true, pModel, meshIndex, pFinalShaderMacros );
        }
        mpMaterialEffects[mMaterialEffectCount] = NULL;

        for( int kk=0; kk<numUserSpecifiedMacros; kk++ )
        {
            // ReadMacrosFromConfigBlock allocates memory (ws2s does).  Delete it here.
            SAFE_DELETE(pUserSpecifiedMacros[kk].Name);
            SAFE_DELETE(pUserSpecifiedMacros[kk].Definition);
        }
        SAFE_DELETE_ARRAY( pFinalShaderMacros );
        SAFE_DELETE_ARRAY( pUserSpecifiedMacros );
        return result; // This material is a multi-material, so we're done.
}