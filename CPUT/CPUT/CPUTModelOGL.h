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