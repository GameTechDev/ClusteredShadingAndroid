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

#include "CPUT_DX11.h"
#include "CPUTDomainShaderDX11.h"
#include "CPUTAssetLibraryDX11.h"

CPUTDomainShaderDX11 *CPUTDomainShaderDX11::CreateDomainShader(
    const cString     &name,
    const cString     &shaderMain,
    const cString     &shaderProfile,
    CPUT_SHADER_MACRO *pShaderMacros
)
{
    ID3DBlob            *pCompiledBlob = NULL;
    ID3D11DomainShader  *pNewDomainShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromFile(name, shaderMain, shaderProfile, &pCompiledBlob, pShaderMacros);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling Domain shader:\n\n") );
    UNREFERENCED_PARAMETER(result);

    // Create the Domain shader
    ID3D11Device      *pD3dDevice = CPUT_DX11::GetDevice();
    HRESULT hr = pD3dDevice->CreateDomainShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewDomainShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Domain shader:\n\n") );
    UNREFERENCED_PARAMETER(hr);
    // cString DebugName = _L("CPUTAssetLibraryDX11::GetDomainShader ")+name;
    // CPUTSetDebugName(pNewDomainShader, DebugName);

    CPUTDomainShaderDX11 *pNewCPUTDomainShader = new CPUTDomainShaderDX11( pNewDomainShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddDomainShader(name, _L(""), shaderMain + shaderProfile, pNewCPUTDomainShader, pShaderMacros);

    // return the shader (and blob)
    return pNewCPUTDomainShader;
}

//--------------------------------------------------------------------------------------
CPUTDomainShaderDX11 *CPUTDomainShaderDX11::CreateDomainShaderFromMemory(
    const cString    &name,
    const cString    &shaderMain,
    const cString    &shaderProfile,
    const char       *pShaderSource,
    CPUT_SHADER_MACRO *pShaderMacros
)
{
    ID3DBlob            *pCompiledBlob = NULL;
    ID3D11DomainShader  *pNewDomainShader = NULL;

    CPUTAssetLibraryDX11 *pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibrary::GetAssetLibrary();
    CPUTResult result = pAssetLibrary->CompileShaderFromMemory(pShaderSource, shaderMain, shaderProfile, &pCompiledBlob, pShaderMacros);
    ASSERT( CPUTSUCCESS(result), _L("Error compiling Domain shader:\n\n") );
    UNREFERENCED_PARAMETER(result);

    // Create the Domain shader
    ID3D11Device      *pD3dDevice = CPUT_DX11::GetDevice();
    HRESULT hr = pD3dDevice->CreateDomainShader( pCompiledBlob->GetBufferPointer(), pCompiledBlob->GetBufferSize(), NULL, &pNewDomainShader );
    ASSERT( SUCCEEDED(hr), _L("Error creating Domain shader:\n\n") );
    UNREFERENCED_PARAMETER(hr);

    // cString DebugName = _L("CPUTAssetLibraryDX11::GetDomainShader ")+name;
    // CPUTSetDebugName(pNewDomainShader, DebugName);
    CPUTDomainShaderDX11 *pNewCPUTDomainShader = new CPUTDomainShaderDX11( pNewDomainShader, pCompiledBlob );

    // add shader to library
    pAssetLibrary->AddDomainShader(name, _L(""), shaderMain + shaderProfile, pNewCPUTDomainShader, pShaderMacros);
    // pNewCPUTDomainShader->Release(); // We've added it to the library, so release our reference

    // return the shader (and blob)
    return pNewCPUTDomainShader;
}
