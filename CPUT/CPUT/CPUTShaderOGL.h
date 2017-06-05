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
