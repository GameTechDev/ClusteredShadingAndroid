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
#ifndef __CPUTMESHOGL_H__
#define __CPUTMESHOGL_H__

#pragma once

#include "CPUTMesh.h"
#include "CPUTBufferOGL.h"
#include "CPUT.h"
#include "CPUTOSServices.h"

class CPUTMaterial;
class CPUTMaterialOGL;
class CPUTBufferOGL;
class CPUTModelOGL;
class CPUTVertexArrayOGL;

//-----------------------------------------------------------------------------
class CPUTMeshOGL : public CPUTMesh
{
protected:

//    int            mD3DMeshTopology;

    CPUTVertexArrayOGL * mpVertexArray;
    
    CPUTBufferOGL *mpVertexBuffer;
    UINT           mVertexCount;
    UINT           mVertexStride;

    CPUTBufferOGL *mpIndexBuffer;
    UINT           mIndexCount;
    
    // This is a handle to the shader program
    GLuint shaderprogram;

    GLuint vao,vbo[2]; /* Create handles for our Vertex Array Object and two Vertex Buffer Objects */
    eCPUTMapType              mIndexBufferMappedType;

public:
    CPUTMeshOGL();
    virtual ~CPUTMeshOGL();
    
    CPUTResult CreateNativeResources(
                                  CPUTModel             *pModel,
                                  UINT                   meshIdx,
                                  int                    vertexElementCount,
                                  CPUTBufferElementInfo *pVertexInfo,
                                  UINT                   vertexCount,
                                  void                  *pVertexData,
                                  CPUTBufferElementInfo *pIndexInfo,
                                  UINT                   indexCount,
                                  void                  *pIndex
                              );
    void BindVertexShaderLayout(CPUTMaterial *pMaterial, CPUTMaterial *pShadowCastMaterial);
    void Draw(CPUTRenderParameters &renderParams, CPUTModel *pModel);
    void DrawPatches(CPUTRenderParameters &renderParams, CPUTModel *pModel);

    void SetNumVertices(int numVertices) {this->mVertexCount = numVertices;};
    void SetNumIndices(int numIndices) {this->mIndexCount = numIndices;};

	// FIXME force get of buffer and map there
    void SetIndexSubData( UINT offset, UINT size, void* pData);
    void SetVertexSubData( UINT offset, UINT size, void* pData);
	void* MapVertices(   CPUTRenderParameters &params, eCPUTMapType type, bool wait=true ) {ASSERT(0, _L("unsupported")); return NULL;};
    void* MapIndices(    CPUTRenderParameters &params, eCPUTMapType type, bool wait=true ){ASSERT(0, _L("unsupported"));return NULL;};
    void  UnmapVertices( CPUTRenderParameters &params ){ASSERT(0, _L("unsupported"));};
    void  UnmapIndices(  CPUTRenderParameters &params ){ASSERT(0, _L("unsupported"));};
};

#endif // __CPUTMESHOGL_H__