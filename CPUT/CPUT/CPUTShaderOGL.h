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
#ifndef _CPUTSHADEROGL_H
#define _CPUTSHADEROGL_H

#include "CPUT.h"
#include "CPUTRefCount.h"
#include <vector>
class CPUTConfigBlock;

class CPUTShaderOGL : public CPUTRefCount
{
protected:
    GLuint mShaderID;
    const cString mName;
     // Destructor is not public.  Must release instead of delete.
    static char* mGLSLVersion;
    static CPUT_SHADER_MACRO* mpDefaultMacros;
    ~CPUTShaderOGL(){}
    CPUTShaderOGL() : mShaderID(0), mName() {}
    CPUTShaderOGL(GLuint shaderID) : mShaderID(shaderID), mName() {};
public:
    static CPUTShaderOGL *CreateShaderFromFiles(
        const std::vector<cString> &fileNames,
        GLuint shaderType,
        char* glslVersion,
        CPUT_SHADER_MACRO *pGlobalMacros=NULL,
        CPUT_SHADER_MACRO *pShaderMacros=NULL
    );
    static CPUTShaderOGL *CreateShaderFromMemory(
        const std::vector<char*>     &source,
        const cString     &shaderProfile,
        CPUT_SHADER_MACRO *pShaderMacros=NULL
    );
    GLuint GetShaderID() { return mShaderID; };
    static cString CreateShaderName(const std::vector<cString> &fileNames, GLuint shaderType, CPUT_SHADER_MACRO *pShaderMacros);
    bool ShaderRequiresPerModelPayload( CPUTConfigBlock &properties );
};

#endif //_CPUTSHADEROGL_H
