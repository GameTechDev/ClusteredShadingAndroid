/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _GLPROGRAM_H_
#define _GLPROGRAM_H_

#include "CPUT.h"

#include <vector>

class CGLProgram
{
public:
    CGLProgram();
    ~CGLProgram();
    void CreateProgram(const cString &ShaderDirectory,
                       const std::vector<cString> &IncludeFiles, 
                       const cString &VSSrcFile,  
                       const cString &PSSrcFile,
					   const cString &CSSrcFile = _L(""));
    
    GLuint CreateShader(const cString &ShaderDirectory, 
                        const std::vector<cString> &IncludeFiles, 
                        const cString &SrcFile, 
                        GLenum ShaderType);
    GLuint GetProgram(){return m_GLProgram;}
private:
    GLuint m_GLProgram;
};

#endif 