// Copyright 2010 Intel Corporation
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