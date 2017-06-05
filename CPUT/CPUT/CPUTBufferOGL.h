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
#ifndef _CPUTBUFFEROGL_H
#define _CPUTBUFFEROGL_H

#include "CPUT_OGL.h"
#include "CPUTBuffer.h"

//--------------------------------------------------------------------------------------
// TODO: Move to dedicated file
class CPUTBufferOGL : public CPUTBuffer
{
private:
    GLuint mBuffer;
    UINT mSize;
    GLenum mTarget;
    GLenum mUsage;
    // Destructor is not public.  Must release instead of delete.
    ~CPUTBufferOGL();
public:
    CPUTBufferOGL();
    GLuint GetBufferID() { return mBuffer; }
    CPUTBufferOGL(cString name);
    CPUTBufferOGL(cString name, GLenum target, GLenum usage, UINT size, void* pData);
    void SetSubData(UINT offset, UINT size, void* pData);

};
#endif //_CPUTBUFFEROGL_H

