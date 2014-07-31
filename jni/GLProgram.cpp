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

#include "GLProgram.h"
#include "CPUTOSServices.h"
#include "CPUT_OGL.h"

CGLProgram::CGLProgram()
{
}

CGLProgram::~CGLProgram()
{
    glDeleteProgram(m_GLProgram);
}


GLuint CGLProgram::CreateShader(const cString &ShaderDirectory, 
                                const std::vector<cString> &IncludeFiles, 
                                const cString &SrcFile, 
                                GLenum ShaderType)
{
    std::vector< const char* > SrcFileData;
    SrcFileData.reserve(IncludeFiles.size()+2);
#ifdef CPUT_OS_WINDOWS
    SrcFileData.push_back( "#version 430\n"
                           "layout(row_major) uniform;\n" );
#endif
#ifdef CPUT_OS_ANDROID
    SrcFileData.push_back( "#version 310 es\n" );
#endif

    int i=0;
    for(auto It = IncludeFiles.begin(); It != IncludeFiles.end(); ++It, ++i)
    {
        unsigned int FileSize;
        void *pData;
	    CPUTResult result = CPUTFileSystem::ReadFileContents( ShaderDirectory + *It, &FileSize, &pData, true, false );
        SrcFileData.push_back( (char*)pData );
    }

    {
        unsigned int FileSize;
        void *pData;
	    CPUTResult result = CPUTFileSystem::ReadFileContents( ShaderDirectory + SrcFile, &FileSize, &pData, true, false );
        SrcFileData.push_back( (char*)pData );
    }

    GLuint Shader = glCreateShader(ShaderType);


    GL_CHECK( glShaderSource(Shader, (GLsizei)SrcFileData.size(), &SrcFileData[0], nullptr) );
    GL_CHECK( glCompileShader(Shader) );
    GLint isCompiled;
    GL_CHECK( glGetShaderiv(Shader, GL_COMPILE_STATUS, &isCompiled) );
    if(isCompiled == false)
    {
        int maxLength = 0;
        GL_CHECK(glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &maxLength));
 
        // The maxLength includes the NULL character
        char* infoLog = (char *)malloc(maxLength*40);
 
        glGetShaderInfoLog(Shader, maxLength, &maxLength, infoLog);
 
        // Handle the error in an appropriate way such as displaying a message or writing to a log file.
        // In this simple program, we'll just leave
        DEBUG_PRINT("Failed to compile shader:\n%s\n", infoLog);
		ASSERT(false, _L("Compile frag shader failed"));

        free(infoLog);
    }

    // The first pointer is the string with version
    SrcFileData.erase(SrcFileData.begin());

    for(auto It = SrcFileData.begin(); It != SrcFileData.end(); ++It)
        delete[] *It;
    
    return Shader;
}

void CGLProgram::CreateProgram(const cString &ShaderDirectory,
                               const std::vector<cString> &IncludeFiles, 
                               const cString &VSSrcFile,  
							   const cString &PSSrcFile,
							   const cString &CSSrcFile)
{
    GLuint VS = 0, PS = 0, CS=0;
    if( !VSSrcFile.empty() )
    {
        VS = CreateShader(ShaderDirectory, IncludeFiles, VSSrcFile, GL_VERTEX_SHADER);
    }
    if( !PSSrcFile.empty() )
        PS = CreateShader(ShaderDirectory, IncludeFiles, PSSrcFile, GL_FRAGMENT_SHADER);

	if (!CSSrcFile.empty())
		CS = CreateShader(ShaderDirectory, IncludeFiles, CSSrcFile, GL_COMPUTE_SHADER);

    m_GLProgram = glCreateProgram();
	if (VS && PS)
	{
		GL_CHECK(glAttachShader(m_GLProgram, VS));
		GL_CHECK(glAttachShader(m_GLProgram, PS));
	}
	else if( CS )
	{
		GL_CHECK(glAttachShader(m_GLProgram, CS));
	}
    GL_CHECK( glLinkProgram(m_GLProgram) );
    int IsLinked;
    GL_CHECK(glGetProgramiv(m_GLProgram, GL_LINK_STATUS, (int *)&IsLinked));
    if( !IsLinked )
    {
        int maxLength;
        // Noticed that glGetProgramiv is used to get the length for a shader program, not glGetShaderiv.
        glGetProgramiv(m_GLProgram, GL_INFO_LOG_LENGTH, &maxLength);
 
        // The maxLength includes the NULL character
        char *shaderProgramInfoLog = (char *)malloc(maxLength);
 
        // Notice that glGetProgramInfoLog, not glGetShaderInfoLog.
        glGetProgramInfoLog(m_GLProgram, maxLength, &maxLength, shaderProgramInfoLog);
        DEBUG_PRINT_ALWAYS((_L("Failed to link shader program:\n%s\n"), shaderProgramInfoLog));
        ASSERT(false, _L("glLinkProgram failed"));

        // Handle the error in an appropriate way such as displaying a message or writing to a log file.
        // In this simple program, we'll just leave
        //DEBUG_PRINT_ALWAYS(("Failed to link shader program:\n%s\n", shaderProgramInfoLog));
        free(shaderProgramInfoLog);
    }

    if(VS)glDeleteShader(VS);
    if(PS)glDeleteShader(PS);
	if(CS)glDeleteShader(CS);
}
