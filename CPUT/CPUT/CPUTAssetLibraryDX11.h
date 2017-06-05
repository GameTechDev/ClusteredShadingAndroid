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
#ifndef __CPUTASSETLIBRARYDX11_H__
#define __CPUTASSETLIBRARYDX11_H__

#include "CPUTAssetLibrary.h"
#include "CPUTConfigBlock.h"

#include <d3d11.h>

class CPUTAssetSet;
class CPUTMaterial;
class CPUTModel;
class CPUTNullNode;
class CPUTCamera;
class CPUTRenderStateBlock;
class CPUTLight;
class CPUTTexture;
class CPUTVertexShaderDX11;
class CPUTPixelShaderDX11;
class CPUTComputeShaderDX11;
class CPUTGeometryShaderDX11;
class CPUTHullShaderDX11;
class CPUTDomainShaderDX11;

//-----------------------------------------------------------------------------
struct CPUTSRGBLoadFlags
{
    bool bInterpretInputasSRGB;
    bool bWritetoSRGBOutput;
};

//-----------------------------------------------------------------------------
class CPUTAssetLibraryDX11:public CPUTAssetLibrary
{
protected:
    static CPUTAssetListEntry  *mpPixelShaderList;
    static CPUTAssetListEntry  *mpComputeShaderList;
    static CPUTAssetListEntry  *mpVertexShaderList;
    static CPUTAssetListEntry  *mpGeometryShaderList;
    static CPUTAssetListEntry  *mpHullShaderList;
    static CPUTAssetListEntry  *mpDomainShaderList;

    static CPUTAssetListEntry  *mpPixelShaderListTail;
    static CPUTAssetListEntry  *mpComputeShaderListTail;
    static CPUTAssetListEntry  *mpVertexShaderListTail;
    static CPUTAssetListEntry  *mpGeometryShaderListTail;
    static CPUTAssetListEntry  *mpHullShaderListTail;
    static CPUTAssetListEntry  *mpDomainShaderListTail;

public:
    CPUTAssetLibraryDX11(){}
    virtual ~CPUTAssetLibraryDX11()
    {
        ReleaseAllLibraryLists();
    }

    virtual void ReleaseAllLibraryLists();
    void ReleaseIunknownList( CPUTAssetListEntry *pList );

    void AddPixelShader(    const cString &name, const cString prefixDecoration, const cString &suffixDecoration, CPUTPixelShaderDX11    *pShader, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { AddAsset( name, prefixDecoration, suffixDecoration, pShader, &mpPixelShaderList,    &mpPixelShaderListTail,    pShaderMacros ); }
    void AddComputeShader(  const cString &name, const cString prefixDecoration, const cString &suffixDecoration, CPUTComputeShaderDX11  *pShader, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { AddAsset( name, prefixDecoration, suffixDecoration, pShader, &mpComputeShaderList,  &mpComputeShaderListTail,  pShaderMacros ); }
    void AddVertexShader(   const cString &name, const cString prefixDecoration, const cString &suffixDecoration, CPUTVertexShaderDX11   *pShader, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { AddAsset( name, prefixDecoration, suffixDecoration, pShader, &mpVertexShaderList,   &mpVertexShaderListTail,   pShaderMacros ); }
    void AddGeometryShader( const cString &name, const cString prefixDecoration, const cString &suffixDecoration, CPUTGeometryShaderDX11 *pShader, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { AddAsset( name, prefixDecoration, suffixDecoration, pShader, &mpGeometryShaderList, &mpGeometryShaderListTail, pShaderMacros ); }
    void AddHullShader(     const cString &name, const cString prefixDecoration, const cString &suffixDecoration, CPUTHullShaderDX11     *pShader, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { AddAsset( name, prefixDecoration, suffixDecoration, pShader, &mpHullShaderList,     &mpHullShaderListTail,     pShaderMacros ); }
    void AddDomainShader(   const cString &name, const cString prefixDecoration, const cString &suffixDecoration, CPUTDomainShaderDX11   *pShader, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { AddAsset( name, prefixDecoration, suffixDecoration, pShader, &mpDomainShaderList,   &mpDomainShaderListTail,   pShaderMacros ); }
    
    CPUTPixelShaderDX11    *FindPixelShader(    const cString &name, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { return    (CPUTPixelShaderDX11*)FindAsset( name, mpPixelShaderList,    nameIsFullPathAndFilename, pShaderMacros ); }
    CPUTComputeShaderDX11  *FindComputeShader(  const cString &name, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { return  (CPUTComputeShaderDX11*)FindAsset( name, mpComputeShaderList,  nameIsFullPathAndFilename, pShaderMacros ); }
    CPUTVertexShaderDX11   *FindVertexShader(   const cString &name, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { return   (CPUTVertexShaderDX11*)FindAsset( name, mpVertexShaderList,   nameIsFullPathAndFilename, pShaderMacros ); }
    CPUTGeometryShaderDX11 *FindGeometryShader( const cString &name, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO*pShaderMacros=NULL ) { return (CPUTGeometryShaderDX11*)FindAsset( name, mpGeometryShaderList, nameIsFullPathAndFilename, pShaderMacros ); }
    CPUTHullShaderDX11     *FindHullShader(     const cString &name, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { return     (CPUTHullShaderDX11*)FindAsset( name, mpHullShaderList,     nameIsFullPathAndFilename, pShaderMacros ); }
    CPUTDomainShaderDX11   *FindDomainShader(   const cString &name, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL ) { return   (CPUTDomainShaderDX11*)FindAsset( name, mpDomainShaderList,   nameIsFullPathAndFilename, pShaderMacros ); }

    // shaders - vertex, pixel
    CPUTResult GetPixelShader(     const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTPixelShaderDX11    **ppShader, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult GetComputeShader(   const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTComputeShaderDX11  **ppShader, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult GetVertexShader(    const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTVertexShaderDX11   **ppShader, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult GetGeometryShader(  const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTGeometryShaderDX11 **ppShader, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult GetHullShader(      const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTHullShaderDX11     **ppShader, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult GetDomainShader(    const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTDomainShaderDX11   **ppShader, bool nameIsFullPathAndFilename=false, CPUT_SHADER_MACRO *pShaderMacros=NULL );
 
    // shaders - vertex, pixel
    CPUTResult CreatePixelShaderFromMemory(     const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTPixelShaderDX11    **ppShader, char *pShaderSource, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult CreateComputeShaderFromMemory(   const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTComputeShaderDX11  **ppShader, char *pShaderSource, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult CreateVertexShaderFromMemory(    const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTVertexShaderDX11   **ppShader, char *pShaderSource, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult CreateGeometryShaderFromMemory(  const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTGeometryShaderDX11 **ppShader, char *pShaderSource, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult CreateHullShaderFromMemory(      const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTHullShaderDX11     **ppShader, char *pShaderSource, CPUT_SHADER_MACRO *pShaderMacros=NULL );
    CPUTResult CreateDomainShaderFromMemory(    const cString &name, const cString &shaderMain, const cString &shaderProfile, CPUTDomainShaderDX11   **ppShader, char *pShaderSource, CPUT_SHADER_MACRO *pShaderMacros=NULL );
 
    CPUTResult CompileShaderFromFile(  const cString &fileName,   const cString &shaderMain, const cString &shaderProfile, ID3DBlob **ppBlob, CPUT_SHADER_MACRO  *pShaderMacros=NULL );
    CPUTResult CompileShaderFromMemory(const char *pShaderSource, const cString &shaderMain, const cString &shaderProfile, ID3DBlob **ppBlob, CPUT_SHADER_MACRO  *pShaderMacros=NULL );
};

#endif // #ifndef __CPUTASSETLIBRARYDX11_H__
