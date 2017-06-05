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
#ifndef __CPUTMODELDX11_H__
#define __CPUTMODELDX11_H__

#include "CPUTModel.h"
#include "CPUT_DX11.h"

class CPUTMeshDX11;
class CPUTRenderParametersDX;
class CPUTMaterialDX11;

//--------------------------------------------------------------------------------------
class CPUTModelDX11 : public CPUTModel
{
protected:
    ID3D11InputLayout ***mpInputLayout;

    // Destructor is not public.  Must release instead of delete.
    ~CPUTModelDX11(){
        if( mpInputLayout )
        {
            for( UINT ii=0; ii<mMeshCount; ii++ )
            {
                for( UINT jj=0; jj<mpLayoutCount[ii]; jj++ )
                {
                    SAFE_RELEASE(mpInputLayout[ii][jj]);
                }
                SAFE_DELETE(mpInputLayout[ii]);
            }
            SAFE_DELETE(mpInputLayout);
        }
        SAFE_DELETE(mpInputLayout);
    }

public:
    CPUTModelDX11() : mpInputLayout(NULL) {}

    CPUTMeshDX11 *GetMesh(const UINT index) const;
    CPUTResult    LoadModel(CPUTConfigBlock *pBlock, int *pParentID, CPUTModel *pMasterModel=NULL, int numSystemMaterials=0, cString *pSystemMaterialNames=NULL);
    void          UpdateShaderConstants(CPUTRenderParameters &renderParams);
    void          Render(CPUTRenderParameters &renderParams, int materialIndex=0);
    void          SetMaterial(UINT ii, CPUTMaterial *pMaterial);
    void          DrawBoundingBox(CPUTRenderParameters &renderParams);
    void          CreateBoundingBoxMesh();

	static			   bool DrawModelCallBack(CPUTModel*, CPUTRenderParameters &renderParams, CPUTMesh*, CPUTMaterialEffect*, CPUTMaterialEffect* pMaterial, void* );

};


#endif // __CPUTMODELDX11_H__
