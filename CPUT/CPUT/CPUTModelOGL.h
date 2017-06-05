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
#ifndef __CPUTMODELOGL_H__
#define __CPUTMODELOGL_H__

#include "CPUTModel.h"
#include "CPUT_OGL.h"
#include "CPUTBufferOGL.h"

class CPUTMeshOGL;
class CPUTMaterialOGL;

//--------------------------------------------------------------------------------------
class CPUTModelOGL : public CPUTModel
{
    friend class CPUTMaterialOGL;
protected:

    // Destructor is not public.  Must release instead of delete.
    ~CPUTModelOGL(){ 
    }

public:
    CPUTModelOGL(){}
    
    CPUTMeshOGL  *GetMesh(const UINT index) const;
    CPUTResult    LoadModel(CPUTConfigBlock *pBlock, int *pParentID, CPUTModel *pMasterModel=NULL, int numSystemMaterials=0, cString *pSystemMaterialNames=NULL);
    void          UpdateShaderConstants(CPUTRenderParameters &renderParams);
    void          Render(CPUTRenderParameters &renderParams, int materialIndex=0);
    void          SetMaterial(UINT ii, CPUTMaterial *pMaterial);
    void          DrawBoundingBox(CPUTRenderParameters &renderParams);
    void          CreateBoundingBoxMesh();

	static bool		  DrawModelCallBack(CPUTModel* pModel, CPUTRenderParameters &renderParams, CPUTMesh* pMesh, CPUTMaterialEffect* pMaterial, CPUTMaterialEffect* , void* );
};


#endif // __CPUTMODELOGL_H__