//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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