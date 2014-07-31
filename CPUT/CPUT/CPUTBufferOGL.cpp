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
