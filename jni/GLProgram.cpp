/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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
