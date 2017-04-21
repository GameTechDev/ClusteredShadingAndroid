/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "CPUTBufferOGL.h"


CPUTBufferOGL::CPUTBufferOGL()
{
    GL_CHECK(glGenBuffers(1, &mBuffer));
}

CPUTBufferOGL::CPUTBufferOGL(cString name) :
    CPUTBuffer(name)
{
    GL_CHECK(glGenBuffers(1, &mBuffer));
}
CPUTBufferOGL::~CPUTBufferOGL()
{
    GL_CHECK(glDeleteBuffers(1, &mBuffer));
}
CPUTBufferOGL::CPUTBufferOGL(cString name, GLenum target, GLenum usage, UINT size, void* pData):
    CPUTBuffer(name), mBuffer(0), mSize(size), mTarget(target), mUsage(usage)

{
    GL_CHECK(glGenBuffers(1, &mBuffer));
    GL_CHECK(glBindBuffer(mTarget, mBuffer));
    GL_CHECK(glBufferData(mTarget, mSize, pData, mUsage));
    GL_CHECK(glBindBuffer(mTarget, 0));
};

void CPUTBufferOGL::SetSubData(UINT offset, UINT size, void* pData)
{
    if(offset+size <= mSize)
    {
        GL_CHECK(glBindBuffer(mTarget, mBuffer));
        GL_CHECK(glBufferSubData(mTarget, offset, size, pData));
	    GL_CHECK(glBindBuffer(mTarget, 0));
	}
}
