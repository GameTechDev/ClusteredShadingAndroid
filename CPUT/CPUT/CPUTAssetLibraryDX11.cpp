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

#include "CPUTAssetLibraryDX11.h"

// define the objects we'll need
#include "CPUTModelDX11.h"
#include "CPUTMaterialEffectDX11.h"
#include "CPUTTextureDX11.h"
#include "CPUTRenderStateBlockDX11.h"
#include "CPUTLight.h"
#include "CPUTCamera.h"
#include "CPUTVertexShaderDX11.h"
#include "CPUTPixelShaderDX11.h"
#include "CPUTGeometryShaderDX11.h"
#include "CPUTComputeShaderDX11.h"
#include "CPUTHullShaderDX11.h"
#include "CPUTDomainShaderDX11.h"

// MPF: opengl es - yipe - can't do both at the same time - need to have it bind dynamically/via compile-time
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpPixelShaderList    = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpComputeShaderList  = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpVertexShaderList   = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpGeometryShaderList = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpHullShaderList = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpDomainShaderList = NULL;

CPUTAssetListEntry *CPUTAssetLibraryDX11::mpPixelShaderListTail    = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpComputeShaderListTail  = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpVertexShaderListTail   = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpGeometryShaderListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpHullShaderListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibraryDX11::mpDomainShaderListTail = NULL;

CPUTAssetLibrary* CPUTAssetLibrary::GetAssetLibrary()
{
    if(NULL==mpAssetLibrary)
    {
        mpAssetLibrary = new CPUTAssetLibraryDX11();
    }
    return mpAssetLibrary;
}

// Deletes and properly releases all asset library lists that contain
// unwrapped IUnknown DirectX objects.
//-----------------------------------------------------------------------------
void CPUTAssetLibraryDX11::ReleaseAllLibraryLists()
{
    // TODO: we really need to wrap the DX assets so we don't need to distinguish their IUnknown type.
    SAFE_RELEASE_LIST(mpPixelShaderList);
    SAFE_RELEASE_LIST(mpComputeShaderList);
    SAFE_RELEASE_LIST(mpVertexShaderList);
    SAFE_RELEASE_LIST(mpGeometryShaderList);
    SAFE_RELEASE_LIST(mpHullShaderList);
    SAFE_RELEASE_LIST(mpDomainShaderList);

	mpPixelShaderListTail    = NULL;
	mpComputeShaderListTail  = NULL;
	mpVertexShaderListTail   = NULL;
	mpGeometryShaderListTail = NULL;
	mpHullShaderListTail = NULL;
	mpDomainShaderListTail = NULL;
	
    // Call base class implementation to clean up the non-DX object lists
    CPUTAssetLibrary::ReleaseAllLibraryLists();
}

// Erase the specified list, Release()-ing underlying objects
//-----------------------------------------------------------------------------
void CPUTAssetLibraryDX11::ReleaseIunknownList( CPUTAssetListEntry *pList )
{
    CPUTAssetListEntry *pNode = pList;
    CPUTAssetListEntry *pOldNode = NULL;

    while( NULL!=pNode )
    {
        // release the object using the DirectX IUnknown interface
        ((IUnknown*)(pNode->pData))->Release();
        pOldNode = pNode;
        pNode = pNode->pNext;
        delete pOldNode;
    }
    HEAPCHECK;
}

// Retrieve specified pixel shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetPixelShader(
    const cString        &name,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTPixelShaderDX11 **ppPixelShader,
    bool                  nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO    *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;

    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName );
    }

    // see if the shader is already in the library
    void *pShader = FindPixelShader(finalName + shaderMain + shaderProfile, true, pShaderMacros);
    if(NULL!=pShader)
    {
        *ppPixelShader = (CPUTPixelShaderDX11*) pShader;
        (*ppPixelShader)->AddRef();
        return result;
    }
    *ppPixelShader = CPUTPixelShaderDX11::CreatePixelShader( finalName, shaderMain, shaderProfile, pShaderMacros );

    return result;
}

// Retrieve specified pixel shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetComputeShader(
    const cString          &name,
    const cString          &shaderMain,
    const cString          &shaderProfile,
    CPUTComputeShaderDX11 **ppComputeShader,
    bool                    nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO      *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindComputeShader(finalName + shaderMain + shaderProfile, true, pShaderMacros);
    if(NULL!=pShader)
    {
        *ppComputeShader = (CPUTComputeShaderDX11*) pShader;
        (*ppComputeShader)->AddRef();
        return result;
    }
    *ppComputeShader = CPUTComputeShaderDX11::CreateComputeShader( finalName, shaderMain, shaderProfile, pShaderMacros );

    return result;
}

// Retrieve specified vertex shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetVertexShader(
    const cString         &name,
    const cString         &shaderMain,
    const cString         &shaderProfile,
    CPUTVertexShaderDX11 **ppVertexShader,
    bool                   nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO     *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindVertexShader(finalName + shaderMain + shaderProfile, true, pShaderMacros);
    if(NULL!=pShader)
    {
        *ppVertexShader = (CPUTVertexShaderDX11*) pShader;
        (*ppVertexShader)->AddRef();
        return result;
    }
    *ppVertexShader = CPUTVertexShaderDX11::CreateVertexShader( finalName, shaderMain, shaderProfile, pShaderMacros );

    return result;
}

// Retrieve specified geometry shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetGeometryShader(
    const cString           &name,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTGeometryShaderDX11 **ppGeometryShader,
    bool                     nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO       *pShaderMacros
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindGeometryShader(finalName + shaderMain + shaderProfile, true, pShaderMacros);
    if(NULL!=pShader)
    {
        *ppGeometryShader = (CPUTGeometryShaderDX11*) pShader;
        (*ppGeometryShader)->AddRef();
        return result;
    }
    *ppGeometryShader = CPUTGeometryShaderDX11::CreateGeometryShader( finalName, shaderMain, shaderProfile, pShaderMacros );

    return result;
}

// Retrieve specified hull shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetHullShader(
    const cString       &name,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTHullShaderDX11 **ppHullShader,
    bool                 nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindHullShader(finalName + shaderMain + shaderProfile, true, pShaderMacros);
    if(NULL!=pShader)
    {
        *ppHullShader = (CPUTHullShaderDX11*) pShader;
        (*ppHullShader)->AddRef();
        return result;
    }
    *ppHullShader = CPUTHullShaderDX11::CreateHullShader( finalName, shaderMain, shaderProfile, pShaderMacros );

    return result;

}

// Retrieve specified domain shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetDomainShader(
    const cString         &name,
    const cString         &shaderMain,
    const cString         &shaderProfile,
    CPUTDomainShaderDX11 **ppDomainShader,
    bool                   nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO     *pShaderMacros
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '%' )
    {
        finalName = mSystemDirectoryName + _L("/Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        CPUTFileSystem::ResolveAbsolutePathAndFilename(finalName, &finalName);
    } else if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindDomainShader(finalName + shaderMain + shaderProfile, true, pShaderMacros);
    if(NULL!=pShader)
    {
        *ppDomainShader = (CPUTDomainShaderDX11*) pShader;
        (*ppDomainShader)->AddRef();
        return result;
    }
    *ppDomainShader = CPUTDomainShaderDX11::CreateDomainShader( finalName, shaderMain, shaderProfile, pShaderMacros );

    return result;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreatePixelShaderFromMemory(
    const cString        &name,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTPixelShaderDX11 **ppShader,
    char                 *pShaderSource,
    CPUT_SHADER_MACRO    *pShaderMacros
)
{
#ifdef _DEBUG
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true, pShaderMacros);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
#endif
    *ppShader = CPUTPixelShaderDX11::CreatePixelShaderFromMemory( name, shaderMain, shaderProfile, pShaderSource, pShaderMacros);
    return CPUT_SUCCESS;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreateVertexShaderFromMemory(
    const cString        &name,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTVertexShaderDX11 **ppShader,
    char                 *pShaderSource,
    CPUT_SHADER_MACRO    *pShaderMacros
)
{
#ifdef _DEBUG
    void *pShader = FindVertexShader(name + shaderMain + shaderProfile, true, pShaderMacros);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
#endif
    *ppShader = CPUTVertexShaderDX11::CreateVertexShaderFromMemory( name, shaderMain, shaderProfile, pShaderSource, pShaderMacros );
    return CPUT_SUCCESS;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreateComputeShaderFromMemory(
    const cString          &name,
    const cString          &shaderMain,
    const cString          &shaderProfile,
    CPUTComputeShaderDX11 **ppShader,
    char                   *pShaderSource,
    CPUT_SHADER_MACRO      *pShaderMacros
)
{
#ifdef _DEBUG
    void *pShader = FindComputeShader(name + shaderMain + shaderProfile, true, pShaderMacros);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
#endif
    *ppShader = CPUTComputeShaderDX11::CreateComputeShaderFromMemory( name, shaderMain, shaderProfile, pShaderSource, pShaderMacros );
    return CPUT_SUCCESS;
}

// If filename ends in .fxo or .cso, then simply read the binary contents to an ID3DBlob.
// Otherwise, generate the ID3DBlob using D3DCompileFromFile().
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CompileShaderFromFile(
    const cString     &fileName,
    const cString     &shaderMain,
    const cString     &shaderProfile,
    ID3DBlob         **ppBlob,
    CPUT_SHADER_MACRO *pShaderMacros
)
{
    size_t len = fileName.length();
    const wchar_t *pExt = &(fileName.c_str())[len-3];
    if( _wcsicmp( pExt, _L("fxo") ) == 0 || _wcsicmp( pExt, _L("cso") ) == 0 )
    {
        // Binary file, load from file.
        FILE *fp;
        errno_t ret = _wfopen_s( &fp, fileName.c_str(), _L("rb") );
        ASSERT( ret == 0, _L("Failed opening file ") + fileName );
        if( ret != 0 )
        {
            return CPUT_ERROR_FILE_NOT_FOUND;
        }

        fseek( fp, 0, SEEK_END );
        int size = ftell( fp );
        fseek( fp, 0L, SEEK_SET );

        D3DCreateBlob( size, ppBlob );
        fread( (*ppBlob)->GetBufferPointer(), size, 1, fp );
        fclose( fp );
    }
    else
    {
        char pShaderMainAsChar[128];
        char pShaderProfileAsChar[128];
        ASSERT( shaderMain.length()     < 128, _L("Shader main name '")    + shaderMain    + _L("' longer than 128 chars.") );
        ASSERT( shaderProfile.length()  < 128, _L("Shader profile name '") + shaderProfile + _L("' longer than 128 chars.") );
        size_t count;
        wcstombs_s( &count, pShaderMainAsChar,    shaderMain.c_str(),    128 );
        wcstombs_s( &count, pShaderProfileAsChar, shaderProfile.c_str(), 128 );

        // Prepare the macros
        int macroCount = 0;
        D3D_SHADER_MACRO *pTmp = (D3D_SHADER_MACRO *)pShaderMacros;
        if( pTmp )
        {
            while( (pTmp++)->Name ) { macroCount++; }
        }
        const UINT NUM_ADDITIONAL_MACROS = 5;
        D3D_SHADER_MACRO pAdditionalMacros[NUM_ADDITIONAL_MACROS] = {
            { "_CPUT", "1" },
            { "FOG_COLOR", "(float3( 0.89f, 0.92f, 0.88f )*0.5f)" },
            { "FOG_START", "2000.0f" },
            { "FOG_END",   "6000.0f" },
            { NULL, NULL }
        };
        const int MAX_DEFINES = 128; // Note: use MAX_DEFINES to avoid dynamic allocation.  Arbitrarily choose 128.  Not sure if there is a real limit.
        ASSERT( macroCount<(MAX_DEFINES-NUM_ADDITIONAL_MACROS), _L("Too many shader macros.") );
        D3D_SHADER_MACRO pFinalShaderMacros[MAX_DEFINES];
        memcpy( pFinalShaderMacros, pShaderMacros, macroCount*sizeof(D3D_SHADER_MACRO));
        memcpy( &pFinalShaderMacros[macroCount], pAdditionalMacros, sizeof(pAdditionalMacros));

        ID3DBlob *pErrorBlob = NULL;
        HRESULT hr = D3DCompileFromFile(
            fileName.c_str(),
            pFinalShaderMacros,
            D3D_COMPILE_STANDARD_FILE_INCLUDE, // includes
            pShaderMainAsChar,
            pShaderProfileAsChar,
            0,                    // flags 1
            0,                    // flags 2
            ppBlob,
            &pErrorBlob
        );
        ASSERT( SUCCEEDED(hr), _L("Error compiling shader '") + fileName + _L("'.\n") + (pErrorBlob ? s2ws((char*)pErrorBlob->GetBufferPointer()) : _L("no error message") ) );
        UNREFERENCED_PARAMETER(hr);

        if(pErrorBlob)
        {
            pErrorBlob->Release();
        }
    } // Compiled from source
    return CPUT_SUCCESS;
}

// Use DX11 compile from file method to do all the heavy lifting
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CompileShaderFromMemory(
    const char        *pShaderSource,
    const cString     &shaderMain,
    const cString     &shaderProfile,
    ID3DBlob         **ppBlob,
    CPUT_SHADER_MACRO *pShaderMacros
)
{
    char pShaderMainAsChar[128];
    char pShaderProfileAsChar[128];
    ASSERT( shaderMain.length()     < 128, _L("Shader main name '")    + shaderMain    + _L("' longer than 128 chars.") );
    ASSERT( shaderProfile.length()  < 128, _L("Shader profile name '") + shaderProfile + _L("' longer than 128 chars.") );
    size_t count;
    wcstombs_s( &count, pShaderMainAsChar,    shaderMain.c_str(),    128 );
    wcstombs_s( &count, pShaderProfileAsChar, shaderProfile.c_str(), 128 );

    // Prepare the macros
    int macroCount = 0;
    D3D_SHADER_MACRO *pTmp = (D3D_SHADER_MACRO *)pShaderMacros;
    if( pTmp )
    {
        while( (pTmp++)->Name ) { macroCount++; }
    }
    const UINT NUM_ADDITIONAL_MACROS = 5;
    D3D_SHADER_MACRO pAdditionalMacros[NUM_ADDITIONAL_MACROS] = {
        { "_CPUT", "1" },
        { "FOG_COLOR", "(float3( 0.89f, 0.92f, 0.88f )*0.5f)" },
        { "FOG_START", "2000.0f" },
        { "FOG_END",   "6000.0f" },
        { NULL, NULL }
    };
    const int MAX_DEFINES = 128; // Note: use MAX_DEFINES to avoid dynamic allocation.  Arbitrarily choose 128.  Not sure if there is a real limit.
    ASSERT( macroCount<(MAX_DEFINES-NUM_ADDITIONAL_MACROS), _L("Too many shader macros.") );
    D3D_SHADER_MACRO pFinalShaderMacros[MAX_DEFINES]; 
    memcpy( pFinalShaderMacros, pShaderMacros, macroCount*sizeof(D3D_SHADER_MACRO));
    memcpy( &pFinalShaderMacros[macroCount], pAdditionalMacros, sizeof(pAdditionalMacros));

    ID3DBlob *pErrorBlob = NULL;
    char *pShaderMainAsChars = ws2s(shaderMain.c_str());
    HRESULT hr = D3DCompile(
        pShaderSource,
        strlen( pShaderSource ),
        pShaderMainAsChars,   // Use entrypoint as file name
        pFinalShaderMacros,
        D3D_COMPILE_STANDARD_FILE_INCLUDE, // includes
        pShaderMainAsChar,
        pShaderProfileAsChar,
        0,                    // flags 1
        0,                    // flags 2
        ppBlob,
        &pErrorBlob
    );
    ASSERT( SUCCEEDED(hr), _L("Error compiling shader '") + shaderMain + _L("'.\n") + (pErrorBlob ? s2ws((char*)pErrorBlob->GetBufferPointer()) : _L("no error message") ) );
    UNREFERENCED_PARAMETER(hr);
    if(pErrorBlob)
    {
        pErrorBlob->Release();
    }
    delete pShaderMainAsChars;
    return CPUT_SUCCESS;
}
