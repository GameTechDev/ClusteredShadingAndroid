/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CPUTRENDERSTATEBLOCK_H
#define _CPUTRENDERSTATEBLOCK_H

#include "CPUT.h"
#include "CPUTRefCount.h"

class CPUTRenderParameters;

class CPUTRenderStateBlock : public CPUTRefCount
{
protected:
    static CPUTRenderStateBlock *mpDefaultRenderStateBlock;

    cString mMaterialName;
    
    ~CPUTRenderStateBlock(){} // Destructor is not public.  Must release instead of delete.

public:
    static CPUTRenderStateBlock *CreateRenderStateBlock( const cString &name, const cString &absolutePathAndFilename );
    static CPUTRenderStateBlock *GetDefaultRenderStateBlock() { return mpDefaultRenderStateBlock; }
    static void SetDefaultRenderStateBlock( CPUTRenderStateBlock *pBlock ) { SAFE_RELEASE( mpDefaultRenderStateBlock ); mpDefaultRenderStateBlock = pBlock; }


    CPUTRenderStateBlock(){}
    virtual CPUTResult LoadRenderStateBlock(const cString &fileName) = 0;
    virtual void SetRenderStates(CPUTRenderParameters &renderParams) = 0;
    virtual void CreateNativeResources() = 0;
};

//-----------------------------------------------------------------------------
class StringToIntMapEntry
{
public:
    cString mName;
    int     mValue;

	// Why not std::map ????

    bool FindMapEntryByName( int *pValue, const cString name ) const{
        StringToIntMapEntry const *pstart = this;
        for( StringToIntMapEntry const *pEntry = pstart; pEntry->mName.length() > 0; pEntry++ ){
#ifndef CPUT_OS_WINDOWS
            if( 0 == strcasecmp( pEntry->mName.data(), name.data() ) )
#else
            if( 0 == _wcsicmp( pEntry->mName.data(), name.data() ) )
#endif
			{
                *pValue = pEntry->mValue;
                return true;
            }
        }
        return false;
    }
};

#endif // _CPUTRENDERSTATEBLOCK_H
