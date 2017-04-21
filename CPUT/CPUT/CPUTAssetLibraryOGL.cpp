/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
//#include "D3dx9tex.h"  // super-annoying - must be first or you get new() operator overloading errors during compile b/c of D3DXGetImageInfoFromFile() function

#include "CPUTAssetLibraryOGL.h"

// define the objects we'll need
#include "CPUTMaterialEffectOGL.h"
#include "CPUTTextureOGL.h"
#include "CPUTLight.h"
#include "CPUTCamera.h"
#include "CPUTShaderOGL.h"

// MPF: opengl es - yipe - can't do both at the same time - need to have it bind dynamically/via compile-time
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpPixelShaderList    = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpComputeShaderList  = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpVertexShaderList   = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpGeometryShaderList = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpHullShaderList = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpDomainShaderList = NULL;

CPUTAssetListEntry *CPUTAssetLibraryOGL::mpPixelShaderListTail    = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpComputeShaderListTail  = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpVertexShaderListTail   = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpGeometryShaderListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpHullShaderListTail = NULL;
CPUTAssetListEntry *CPUTAssetLibraryOGL::mpDomainShaderListTail = NULL;

CPUTAssetLibrary* CPUTAssetLibrary::GetAssetLibrary()
{
    if(NULL==mpAssetLibrary)
    {
        mpAssetLibrary = new CPUTAssetLibraryOGL();
    }
    return mpAssetLibrary;
}

// TODO: Change OS Services to a flat list of CPUT* functions.  Avoid calls all over the place like:
// CPUTOSServices::GetOSServices();

// Deletes and properly releases all asset library lists that contain
// unwrapped IUnknown DirectX objects.
//-----------------------------------------------------------------------------
void CPUTAssetLibraryOGL::ReleaseAllLibraryLists()
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
    return CPUTAssetLibrary::ReleaseAllLibraryLists();
}

// Erase the specified list, Release()-ing underlying objects
//-----------------------------------------------------------------------------
void CPUTAssetLibraryOGL::ReleaseIunknownList( CPUTAssetListEntry *pList )
{
    CPUTAssetListEntry *pNode = pList;
    CPUTAssetListEntry *pOldNode = NULL;

    while( NULL!=pNode )
    {
        // release the object using the DirectX IUnknown interface
        // ##### do you need to release OGL data?
//        ((IUnknown*)(pNode->pData))->Release();
        pOldNode = pNode;
        pNode = pNode->pNext;
        delete pOldNode;
    }
    HEAPCHECK;
}

// Retrieve specified pixel shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryOGL::GetPixelShader(
    const cString        &name,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTShaderOGL  **ppPixelShader,
    bool                  nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO    *pShaderMacros
)
{
    std::vector<cString> filenames;
    filenames.push_back(name);
    return GetPixelShader(filenames, shaderMain, shaderProfile, ppPixelShader, nameIsFullPathAndFilename, pShaderMacros);
}

CPUTResult CPUTAssetLibraryOGL::GetPixelShader(
    const std::vector<cString>  &fileNames,
    const cString               &shaderMain,
    const cString               &shaderProfile,
    CPUTShaderOGL          **ppPixelShader,
    bool                        nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO           *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    std::vector<cString> finalNames;
    for(UINT i=0; i<fileNames.size(); i++)   
    {   
        cString name = fileNames[i];
        cString finalName;
        if( fileNames[i].at(0) == '%' )
        {
            // Consider moving slashes to some sub variable dependent from OS,
            // or even MACRO would be better in such situation.
            finalName = mSystemDirectoryName + _L("Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        } else if( fileNames[i].at(0) == '$' )
        {
            finalName = fileNames[i];
        } else
        {
            CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName );
        }
        finalNames.push_back(finalName);
    }
    // see if the shader is already in the library
    cString name = CPUTShaderOGL::CreateShaderName(finalNames, GL_FRAGMENT_SHADER, pShaderMacros);
    
    void *pShader = FindPixelShader(name, true);
    if(NULL!=pShader)
    {
        *ppPixelShader = (CPUTShaderOGL*)pShader;
        (*ppPixelShader)->AddRef();
        return result;
    }
    *ppPixelShader = CPUTShaderOGL::CreateShaderFromFiles( finalNames, GL_FRAGMENT_SHADER, CPUT_OGL::GLSL_VERSION, CPUT_OGL::DEFAULT_MACROS, pShaderMacros);

    return result;
}

// Retrieve specified vertex shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryOGL::GetVertexShader(
    const cString       &name,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppVertexShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    std::vector<cString> filenames;
    filenames.push_back(name);
    return GetVertexShader(filenames, shaderMain, shaderProfile, ppVertexShader, nameIsFullPathAndFilename, pShaderMacros);
}

CPUTResult CPUTAssetLibraryOGL::GetVertexShader(
    const std::vector<cString> &fileNames,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppVertexShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    std::vector<cString> finalNames;
    for(UINT i=0; i<fileNames.size(); i++)   
    {   
        cString name = fileNames[i];
        cString finalName;
        if( fileNames[i].at(0) == '%' )
        {
            // Consider moving slashes to some sub variable dependent from OS,
            // or even MACRO would be better in such situation.
            finalName = mSystemDirectoryName + _L("Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        } else if( fileNames[i].at(0) == '$' )
        {
            finalName = fileNames[i];
        } else
        {
            CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName );
        }
        finalNames.push_back(finalName);
    }
    // see if the shader is already in the library
    cString name = CPUTShaderOGL::CreateShaderName(finalNames, GL_VERTEX_SHADER, pShaderMacros);
    void *pShader = FindPixelShader(name, true);
    if(NULL!=pShader)
    {
        *ppVertexShader = (CPUTShaderOGL*)pShader;
        (*ppVertexShader)->AddRef();
        return result;
    }
    *ppVertexShader = CPUTShaderOGL::CreateShaderFromFiles( finalNames, GL_VERTEX_SHADER, CPUT_OGL::GLSL_VERSION, CPUT_OGL::DEFAULT_MACROS, pShaderMacros);

    return result;
}

CPUTResult CPUTAssetLibraryOGL::GetHullShader(
    const cString       &name,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppHullShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    std::vector<cString> filenames;
    filenames.push_back(name);
    return GetHullShader(filenames, shaderMain, shaderProfile, ppHullShader, nameIsFullPathAndFilename, pShaderMacros);
}

CPUTResult CPUTAssetLibraryOGL::GetHullShader(
    const std::vector<cString> &fileNames,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppHullShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    std::vector<cString> finalNames;
    for(UINT i=0; i<fileNames.size(); i++)   
    {   
        cString name = fileNames[i];
        cString finalName;
        if( fileNames[i].at(0) == '%' )
        {
            // Consider moving slashes to some sub variable dependent from OS,
            // or even MACRO would be better in such situation.
            finalName = mSystemDirectoryName + _L("Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        } else if( fileNames[i].at(0) == '$' )
        {
            finalName = fileNames[i];
        } else
        {
            CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName );
        }
        finalNames.push_back(finalName);
    }
    // see if the shader is already in the library
    cString name = CPUTShaderOGL::CreateShaderName(finalNames, GL_TESS_CONTROL_SHADER, pShaderMacros);
    void *pShader = FindPixelShader(name, true);
    if(NULL!=pShader)
    {
        *ppHullShader = (CPUTShaderOGL*)pShader;
        (*ppHullShader)->AddRef();
        return result;
    }
    *ppHullShader = CPUTShaderOGL::CreateShaderFromFiles( finalNames, GL_TESS_CONTROL_SHADER, CPUT_OGL::GLSL_VERSION, CPUT_OGL::DEFAULT_MACROS, pShaderMacros);

    return result;
}

CPUTResult CPUTAssetLibraryOGL::GetDomainShader(
    const cString       &name,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppDomainShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    std::vector<cString> filenames;
    filenames.push_back(name);
    return GetDomainShader(filenames, shaderMain, shaderProfile, ppDomainShader, nameIsFullPathAndFilename, pShaderMacros);
}

CPUTResult CPUTAssetLibraryOGL::GetDomainShader(
    const std::vector<cString> &fileNames,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppDomainShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    std::vector<cString> finalNames;
    for(UINT i=0; i<fileNames.size(); i++)   
    {   
        cString name = fileNames[i];
        cString finalName;
        if( fileNames[i].at(0) == '%' )
        {
            // Consider moving slashes to some sub variable dependent from OS,
            // or even MACRO would be better in such situation.
            finalName = mSystemDirectoryName + _L("Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        } else if( fileNames[i].at(0) == '$' )
        {
            finalName = fileNames[i];
        } else
        {
            CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName );
        }
        finalNames.push_back(finalName);
    }
    // see if the shader is already in the library
    cString name = CPUTShaderOGL::CreateShaderName(finalNames, GL_TESS_EVALUATION_SHADER, pShaderMacros);
    void *pShader = FindPixelShader(name, true);
    if(NULL!=pShader)
    {
        *ppDomainShader = (CPUTShaderOGL*)pShader;
        (*ppDomainShader)->AddRef();
        return result;
    }
    *ppDomainShader = CPUTShaderOGL::CreateShaderFromFiles( finalNames, GL_TESS_EVALUATION_SHADER, CPUT_OGL::GLSL_VERSION, CPUT_OGL::DEFAULT_MACROS, pShaderMacros);

    return result;
}

CPUTResult CPUTAssetLibraryOGL::GetGeometryShader(
    const cString       &name,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppGeometryShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    std::vector<cString> filenames;
    filenames.push_back(name);
    return GetGeometryShader(filenames, shaderMain, shaderProfile, ppGeometryShader, nameIsFullPathAndFilename, pShaderMacros);
}

CPUTResult CPUTAssetLibraryOGL::GetGeometryShader(
    const std::vector<cString> &fileNames,
    const cString       &shaderMain,
    const cString       &shaderProfile,
    CPUTShaderOGL       **ppGeometryShader,
    bool                nameIsFullPathAndFilename,
    CPUT_SHADER_MACRO   *pShaderMacros
)
{
    CPUTResult result = CPUT_SUCCESS;
    std::vector<cString> finalNames;
    for(UINT i=0; i<fileNames.size(); i++)   
    {   
        cString name = fileNames[i];
        cString finalName;
        if( fileNames[i].at(0) == '%' )
        {
            // Consider moving slashes to some sub variable dependent from OS,
            // or even MACRO would be better in such situation.
            finalName = mSystemDirectoryName + _L("Shader/") + name.substr(1);  // TODO: Instead of having the Shader/ directory hardcoded here it could be set like the normal material directory. But then there would need to be a bunch new variables like SetSystemMaterialDirectory
        } else if( fileNames[i].at(0) == '$' )
        {
            finalName = fileNames[i];
        } else
        {
            CPUTFileSystem::ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName );
        }
        finalNames.push_back(finalName);
    }
    // see if the shader is already in the library
    cString name = CPUTShaderOGL::CreateShaderName(finalNames, GL_GEOMETRY_SHADER, pShaderMacros);
    void *pShader = FindPixelShader(name, true);
    if(NULL!=pShader)
    {
        *ppGeometryShader = (CPUTShaderOGL*)pShader;
        (*ppGeometryShader)->AddRef();
        return result;
    }
    *ppGeometryShader = CPUTShaderOGL::CreateShaderFromFiles( finalNames, GL_GEOMETRY_SHADER, CPUT_OGL::GLSL_VERSION, CPUT_OGL::DEFAULT_MACROS, pShaderMacros);

    return result;
}

/*
// Retrieve specified geometry shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetGeometryShader(
    const cString           &name,
    ID3D11Device            *pD3dDevice,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTGeometryShaderDX11 **ppGeometryShader,
    bool                     nameIsFullPathAndFilename
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindGeometryShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppGeometryShader = (CPUTGeometryShaderDX11*) pShader;
        (*ppGeometryShader)->AddRef();
        return result;
    }
    *ppGeometryShader = CPUTGeometryShaderDX11::CreateGeometryShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}

// Retrieve specified hull shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetHullShader(
    const cString           &name,
    ID3D11Device            *pD3dDevice,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTHullShaderDX11 **ppHullShader,
    bool                     nameIsFullPathAndFilename
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindHullShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppHullShader = (CPUTHullShaderDX11*) pShader;
        (*ppHullShader)->AddRef();
        return result;
    }
    *ppHullShader = CPUTHullShaderDX11::CreateHullShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;

}

// Retrieve specified domain shader
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::GetDomainShader(
    const cString           &name,
    ID3D11Device            *pD3dDevice,
    const cString           &shaderMain,
    const cString           &shaderProfile,
    CPUTDomainShaderDX11 **ppDomainShader,
    bool                     nameIsFullPathAndFilename
    )
{
    CPUTResult result = CPUT_SUCCESS;
    cString finalName;
    if( name.at(0) == '$' )
    {
        finalName = name;
    } else
    {
        // Resolve name to absolute path
        CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
        pServices->ResolveAbsolutePathAndFilename( nameIsFullPathAndFilename? name : (mShaderDirectoryName + name), &finalName);
    }

    // see if the shader is already in the library
    void *pShader = FindDomainShader(finalName + shaderMain + shaderProfile, true);
    if(NULL!=pShader)
    {
        *ppDomainShader = (CPUTDomainShaderDX11*) pShader;
        (*ppDomainShader)->AddRef();
        return result;
    }
    *ppDomainShader = CPUTDomainShaderDX11::CreateDomainShader( finalName, pD3dDevice, shaderMain, shaderProfile );

    return result;
}


//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreatePixelShaderFromMemory(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTPixelShaderDX11 **ppShader,
    char                 *pShaderSource
)
{
    CPUTResult result = CPUT_SUCCESS;
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
    *ppShader = CPUTPixelShaderDX11::CreatePixelShaderFromMemory( name, pD3dDevice, shaderMain, shaderProfile, pShaderSource);
    return result;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreateVertexShaderFromMemory(
    const cString        &name,
    ID3D11Device         *pD3dDevice,
    const cString        &shaderMain,
    const cString        &shaderProfile,
    CPUTVertexShaderDX11 **ppShader,
    char                 *pShaderSource
)
{
    CPUTResult result = CPUT_SUCCESS;
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
    *ppShader = CPUTVertexShaderDX11::CreateVertexShaderFromMemory( name, pD3dDevice, shaderMain, shaderProfile, pShaderSource);
    return result;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CreateComputeShaderFromMemory(
    const cString          &name,
    ID3D11Device           *pD3dDevice,
    const cString          &shaderMain,
    const cString          &shaderProfile,
    CPUTComputeShaderDX11 **ppShader,
    char                   *pShaderSource
)
{
    CPUTResult result = CPUT_SUCCESS;
    void *pShader = FindPixelShader(name + shaderMain + shaderProfile, true);
    ASSERT( NULL == pShader, _L("Shader already exists.") );
    *ppShader = CPUTComputeShaderDX11::CreateComputeShaderFromMemory( name, pD3dDevice, shaderMain, shaderProfile, pShaderSource);
    return result;
}

// Use DX11 compile from file method to do all the heavy lifting
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CompileShaderFromFile(
    const cString  &fileName,
    const cString  &shaderMain,
    const cString  &shaderProfile,
    ID3DBlob      **ppBlob
)
{
    CPUTResult result = CPUT_SUCCESS;

    char pShaderMainAsChar[128];
    char pShaderProfileAsChar[128];
    ASSERT( shaderMain.length()     < 128, _L("Shader main name '")    + shaderMain    + _L("' longer than 128 chars.") );
    ASSERT( shaderProfile.length()  < 128, _L("Shader profile name '") + shaderProfile + _L("' longer than 128 chars.") );
    size_t count;
    wcstombs_s( &count, pShaderMainAsChar,    shaderMain.c_str(),    128 );
    wcstombs_s( &count, pShaderProfileAsChar, shaderProfile.c_str(), 128 );

    // use DirectX to compile the shader file
    ID3DBlob *pErrorBlob = NULL;
    D3D10_SHADER_MACRO pShaderMacros[2] = { "_CPUT", "1", NULL, NULL };
    HRESULT hr = D3DX11CompileFromFile(
        fileName.c_str(),     // fileName
        pShaderMacros,        // macro define's
        NULL,                 // includes
        pShaderMainAsChar,    // main function name
        pShaderProfileAsChar, // shader profile/feature level
        0,                    // flags 1
        0,                    // flags 2
        NULL,                 // threaded load? (no for right now)
        ppBlob,               // blob data with compiled code
        &pErrorBlob,          // any compile errors stored here
        NULL
    );
    ASSERT( SUCCEEDED(hr), _L("Error compiling shader '") + fileName + _L("'.\n") + (pErrorBlob ? s2ws((char*)pErrorBlob->GetBufferPointer()) : _L("no error message") ) );
    if(pErrorBlob)
    {
        pErrorBlob->Release();
    }
    return result;
}

// Use DX11 compile from file method to do all the heavy lifting
//-----------------------------------------------------------------------------
CPUTResult CPUTAssetLibraryDX11::CompileShaderFromMemory(
    const char     *pShaderSource,
    const cString  &shaderMain,
    const cString  &shaderProfile,
    ID3DBlob      **ppBlob
)
{
    CPUTResult result = CPUT_SUCCESS;

    char pShaderMainAsChar[128];
    char pShaderProfileAsChar[128];
    ASSERT( shaderMain.length()     < 128, _L("Shader main name '")    + shaderMain    + _L("' longer than 128 chars.") );
    ASSERT( shaderProfile.length()  < 128, _L("Shader profile name '") + shaderProfile + _L("' longer than 128 chars.") );
    size_t count;
    wcstombs_s( &count, pShaderMainAsChar,    shaderMain.c_str(),    128 );
    wcstombs_s( &count, pShaderProfileAsChar, shaderProfile.c_str(), 128 );

    // use DirectX to compile the shader file
    ID3DBlob *pErrorBlob = NULL;
    D3D10_SHADER_MACRO pShaderMacros[2] = { "_CPUT", "1", NULL, NULL }; // TODO: Support passed-in, and defined in .mtl file.  Perhaps under [Shader Defines], etc
    char *pShaderMainAsChars = ws2s(shaderMain.c_str());
    HRESULT hr = D3DX11CompileFromMemory(
        pShaderSource,     // shader as a string
        strlen( pShaderSource ), //
        pShaderMainAsChars, // Use entrypoint as file name
        pShaderMacros,        // macro define's
        NULL,                 // includes
        pShaderMainAsChar,    // main function name
        pShaderProfileAsChar, // shader profile/feature level
        0,                    // flags 1
        0,                    // flags 2
        NULL,                 // threaded load? (no for right now)
        ppBlob,               // blob data with compiled code
        &pErrorBlob,          // any compile errors stored here
        NULL
    );
    ASSERT( SUCCEEDED(hr), _L("Error compiling shader '") + shaderMain + _L("'.\n") + (pErrorBlob ? s2ws((char*)pErrorBlob->GetBufferPointer()) : _L("no error message") ) );
    if(pErrorBlob)
    {
        pErrorBlob->Release();
    }
    delete pShaderMainAsChars;
    return result;
}
*/
