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
