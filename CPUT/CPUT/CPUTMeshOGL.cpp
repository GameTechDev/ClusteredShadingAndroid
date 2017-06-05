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
#include "CPUT_OGL.h"
#include "CPUTMeshOGL.h"
#include "CPUTMaterialEffect.h"

GLenum ConvertToOpenGLFormat(CPUT_DATA_FORMAT_TYPE dataFormatType) {

    // TODO:
    // Please consider removing this in favor for lookup static array of GL types 
    // indexed by CPUT_DATA_FORMAT_TYPE. Before lookup we do range check in assert
    // of course.
    switch( dataFormatType ) {
#ifndef CPUT_FOR_OGLES
         case CPUT_DOUBLE:
            return GL_DOUBLE;
            break;
#endif
        case CPUT_F32:
            return GL_FLOAT;
            break;
        case CPUT_U32:
            return GL_UNSIGNED_INT;
            break;
        case CPUT_I32:
            return GL_INT;
            break;
        case CPUT_U16:
            return GL_UNSIGNED_SHORT;
            break;
        case CPUT_I16:
            return GL_SHORT;
            break;
        case CPUT_U8:
            return GL_UNSIGNED_BYTE;
            break;
        case CPUT_I8:
        case CPUT_CHAR:
            return GL_BYTE;
            break;
        case CPUT_BOOL:
            return GL_BOOL;
            break;
        default:
            assert(0);
            break;
    }
assert(0);
return 0;
}

#ifdef CPUT_FOR_OGLES2
class CPUTVertexAttribPointer
{
public:
    GLint     mIndex;
    GLint     mCount;
    GLenum    mType;
    GLboolean mNormalise;
    GLint     mStride;
    void *    mOffset;
    
public:
    CPUTVertexAttribPointer() : mIndex(0), mCount(0), mType(0), mNormalise(GL_FALSE), mStride(0), mOffset(NULL) 
    {}
    
    CPUTVertexAttribPointer(GLint index, GLint count, GLenum type, GLboolean norm, GLint stride, void * offset) :
    mIndex(index), mCount(count), mType(type), mNormalise(norm), mStride(stride), mOffset(offset)
    {}
    
    virtual ~CPUTVertexAttribPointer() {}
    
    void Enable()
    {
        GL_CHECK(glEnableVertexAttribArray(mIndex));
        GL_CHECK(glVertexAttribPointer(mIndex, mCount, mType, mNormalise, mStride, mOffset));
    }
    
    void Disable()
    {
        GL_CHECK(glDisableVertexAttribArray(mIndex));
    }
};
#endif

//
// This class mimics the behaviour of GL vertex arrays which were introduced in GLES3.0
//
class CPUTVertexArrayOGL
{
public:
    GLint mIndexBufferID;
    GLint mVertexBufferID;
    GLint mVertexElementCount;
    GLuint mVertexArray;
    
#ifdef CPUT_FOR_OGLES2
    CPUTVertexAttribPointer * mpVertexAttribList;
#endif
    
public:
    CPUTVertexArrayOGL(GLint vertexElementCount) :
    mIndexBufferID(0), mVertexBufferID(0), mVertexElementCount(vertexElementCount)
    {
#ifdef CPUT_FOR_OGLES2
        mpVertexAttribList = new CPUTVertexAttribPointer[mVertexElementCount];
#else
        GL_CHECK(ES3_COMPAT(glGenVertexArrays(1, &mVertexArray))); 
        GL_CHECK(ES3_COMPAT(glBindVertexArray(mVertexArray)));
#endif
    }
    
    void AddIBO(CPUTBufferOGL *pBuffer) { 
        if(pBuffer) 
            mIndexBufferID = pBuffer->GetBufferID(); } 
    void AddVBO(CPUTBufferOGL *pBuffer) { if(pBuffer) mVertexBufferID = pBuffer->GetBufferID(); } 
    
    virtual ~CPUTVertexArrayOGL()
    {
#ifdef CPUT_FOR_OGLES2
        delete[] mpVertexAttribList;
#else
        if (mVertexArray != 0)
        {
            GL_CHECK(ES3_COMPAT(glDeleteVertexArrays(1, &mVertexArray)));
        }
#endif
    }
    
    void AddVertexPointer(GLint index, GLint count, GLenum type, GLboolean norm, GLint stride, void * offset)
    {
		GL_CHECK(glBindBuffer( GL_ARRAY_BUFFER, mVertexBufferID));
		GL_CHECK(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIndexBufferID));
#ifdef CPUT_FOR_OGLES2
        if (index < mVertexElementCount)
        {
            mpVertexAttribList[index] = CPUTVertexAttribPointer(index, count, type, norm, stride, offset);
        }
#else
        GL_CHECK(glEnableVertexAttribArray(index));
        GL_CHECK(glVertexAttribPointer(index, count, type, norm, stride, offset));
#endif
    }
    
#ifndef CPUT_FOR_OGLES
    // Not supported in ES
    void AddVertexLPointer(GLint index, GLint count, GLenum type, GLint stride, void * offset)
    {
        GL_CHECK(glEnableVertexAttribArray(index));
        GL_CHECK(glVertexAttribLPointer(index, count, type, stride, offset));
    }
#endif
#ifndef CPUT_FOR_OGLES2
    // Not supported in ES
    void AddVertexIPointer(GLint index, GLint count, GLenum type, GLint stride, void * offset)
    {
        GL_CHECK(glEnableVertexAttribArray(index));
        GL_CHECK(ES3_COMPAT(glVertexAttribIPointer(index, count, type, stride, offset)));
    }
#endif
    
    void Enable()
    {
#ifdef CPUT_FOR_OGLES2
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferID));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));
        for (GLint ii = 0; ii < mVertexElementCount; ii++)
        {
            mpVertexAttribList[ii].Enable();
        }
#else
        GL_CHECK(ES3_COMPAT(glBindVertexArray(mVertexArray)));
#endif
    }
    
    void Disable()
    {
#ifdef CPUT_FOR_OGLES2
        for (GLint ii = 0; ii < mVertexElementCount; ii++)
        {
            mpVertexAttribList[ii].Disable();
        }
#else
        GL_CHECK(ES3_COMPAT(glBindVertexArray(0)));
#endif
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
	void AddVertexPointers(int vertexElementCount, CPUTBufferElementInfo *pVertexDataInfo)
	{
		int vertexStride = pVertexDataInfo[vertexElementCount-1].mOffset + pVertexDataInfo[vertexElementCount-1].mElementSizeInBytes; // size in bytes of a single vertex block

		//Enable();
        for (int i = 0; i < vertexElementCount; i++) 
		{
			switch (pVertexDataInfo[i].mElementType) 
			{
			case CPUT_F32:
				AddVertexPointer(pVertexDataInfo[i].mBindPoint, pVertexDataInfo[i].mElementComponentCount, ConvertToOpenGLFormat(pVertexDataInfo[i].mElementType), GL_FALSE, vertexStride, (void *)(pVertexDataInfo[i].mOffset));
				break;

#ifndef CPUT_FOR_OGLES
			case CPUT_DOUBLE:
				AddVertexLPointer(pVertexDataInfo[i].mBindPoint, pVertexDataInfo[i].mElementComponentCount, ConvertToOpenGLFormat(pVertexDataInfo[i].mElementType), vertexStride, (void *)(pVertexDataInfo[i].mOffset));
				break;
#endif
#ifndef CPUT_FOR_OGLES2
			case CPUT_U32:
			case CPUT_I32:
			case CPUT_U16:
			case CPUT_I16:
			case CPUT_U8:
			case CPUT_I8:
				AddVertexIPointer(pVertexDataInfo[i].mBindPoint, pVertexDataInfo[i].mElementComponentCount, ConvertToOpenGLFormat(pVertexDataInfo[i].mElementType), vertexStride, (void *)(pVertexDataInfo[i].mOffset));
				break;
#endif
	        default:
            // unrecognized type
//            DEBUG_PRINT("Unrecognised type for vertex data");
            break;
			}
		}
        Disable();
	};
};

//-----------------------------------------------------------------------------
CPUTMeshOGL::CPUTMeshOGL() :
//    mD3DMeshTopology(0),
    mpVertexBuffer(NULL),
    mpIndexBuffer(NULL),
    mIndexCount(0),
    mVertexCount(0),
    mVertexStride(0),
    mpVertexArray(NULL)
{
}

//-----------------------------------------------------------------------------
CPUTMeshOGL::~CPUTMeshOGL()
{
    SAFE_RELEASE(mpVertexBuffer);
    SAFE_RELEASE(mpIndexBuffer);
    SAFE_DELETE(mpVertexArray);
}

//-----------------------------------------------------------------------------
CPUTResult CPUTMeshOGL::CreateNativeResources(
    CPUTModel              *pModel0,
    UINT                    meshIdx0,
    int                     vertexElementCount,
    CPUTBufferElementInfo  *pVertexDataInfo,
    UINT                    vertexCount,
    void                   *pVertexData,
    CPUTBufferElementInfo  *pIndexDataInfo,
    UINT                    indexCount,
    void                   *pIndexData
)
{
    mpVertexArray = new CPUTVertexArrayOGL(vertexElementCount);
	mIndexCount = indexCount;
	mVertexCount = vertexCount;
	SAFE_RELEASE(mpIndexBuffer);
	if(indexCount > 0)
	{
		mpIndexBuffer = new CPUTBufferOGL(_L("index buffer"), GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, mIndexCount * pIndexDataInfo->mElementSizeInBytes, NULL);
		if(pIndexData != NULL)
		mpIndexBuffer->SetSubData(0, mIndexCount * pIndexDataInfo->mElementSizeInBytes, pIndexData);
	}
    
	
	SAFE_RELEASE(mpVertexBuffer);
	if(vertexCount > 0)
	{
		mVertexCount  = vertexCount;
		mVertexStride = pVertexDataInfo[vertexElementCount-1].mOffset + pVertexDataInfo[vertexElementCount-1].mElementSizeInBytes; // size in bytes of a single vertex block
		mpVertexBuffer = new CPUTBufferOGL(_L("vertex buffer"), GL_ARRAY_BUFFER, GL_STATIC_DRAW, mVertexCount * mVertexStride, NULL);
        if(pVertexData != NULL)
		mpVertexBuffer->SetSubData(0, mVertexCount * mVertexStride, pVertexData);
	}
    else
    {
        mpVertexBuffer = new CPUTBufferOGL(_L("vertex buffer"), GL_ARRAY_BUFFER, GL_STATIC_DRAW, NULL, NULL);
    }
    
    mpVertexArray->AddIBO(mpIndexBuffer);
    mpVertexArray->AddVBO(mpVertexBuffer);
	mpVertexArray->AddVertexPointers(vertexElementCount, pVertexDataInfo);
    
    return CPUT_SUCCESS;
}

// https://sites.google.com/site/opengltutorialsbyaks/introduction-to-opengl-4-1---tutorial-05
void DebugOutputToFile(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, const char* message)
{
#ifndef CPUT_FOR_OGLES
    {
             char debSource[16], debType[20], debSev[5];
             
             if(source == GL_DEBUG_SOURCE_API_ARB)
                    strcpy(debSource, "OpenGL");
             else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
                    strcpy(debSource, "Windows");
             else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
                    strcpy(debSource, "Shader Compiler");
             else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
                    strcpy(debSource, "Third Party");
             else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)
                    strcpy(debSource, "Application");
             else if(source == GL_DEBUG_SOURCE_OTHER_ARB)
                    strcpy(debSource, "Other");
             if(type == GL_DEBUG_TYPE_ERROR_ARB)
                    strcpy(debType, "Error");
             else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
                    strcpy(debType, "Deprecated behavior");
             else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
                    strcpy(debType, "Undefined behavior");
             else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)
                    strcpy(debType, "Portability");
             else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
                    strcpy(debType, "Performance");
             else if(type == GL_DEBUG_TYPE_OTHER_ARB)
                    strcpy(debType, "Other");
             if(severity == GL_DEBUG_SEVERITY_HIGH_ARB)
                    strcpy(debSev, "High");
             else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
                    strcpy(debSev, "Medium");
             else if(severity == GL_DEBUG_SEVERITY_LOW_ARB)
                    strcpy(debSev, "Low");

             printf("Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);
    }
#endif
}

void CheckDebugLog()
{
#ifndef CPUT_FOR_OGLES
    // Be sure to check at run-time if given extension is supported !
    // Disabled for now. 
    return;

       unsigned int count = 10; // max. num. of messages that will be read from the log
       int bufsize = 2048;

       unsigned int* sources      = new unsigned int[count];
       unsigned int* types        = new unsigned int[count];
       unsigned int* ids   = new unsigned int[count];
       unsigned int* severities = new unsigned int[count];
       int* lengths = new int[count];
       char* messageLog = new char[bufsize];

       unsigned int retVal = glGetDebugMessageLogARB(count, bufsize, sources, types, ids, severities, lengths, messageLog);
       if(retVal > 0)
       {
             unsigned int pos = 0;
             for(unsigned int i=0; i<retVal; i++)
             {
                    DebugOutputToFile(sources[i], types[i], ids[i], severities[i], &messageLog[pos]);
                    pos += lengths[i];
              }
       }
       delete [] sources;
       delete [] types;
       delete [] ids;
       delete [] severities;
       delete [] lengths;
    delete [] messageLog;
#endif
}

void CPUTMeshOGL::Draw(CPUTRenderParameters &renderParams, CPUTModel *pModel)
{
    if(mVertexCount == 0 && mIndexCount == 0)
        return;
    mpVertexArray->Enable();
    if(mpIndexBuffer != NULL)
	{
		GL_CHECK(glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, NULL));
	}
	else
	{
		GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, mVertexCount));
	}
    mpVertexArray->Disable();
    
    glUseProgram(0);

    CheckDebugLog();
}

void CPUTMeshOGL::DrawPatches(CPUTRenderParameters &renderParams, CPUTModel *pModel)
{
    //todo - need to enable patch options - default is 3 control points.
    if(mVertexCount == 0 && mIndexCount == 0)
        return;
    mpVertexArray->Enable();
    if(mpIndexBuffer != NULL)
	{
		GL_CHECK(glDrawElements(GL_PATCHES, mIndexCount, GL_UNSIGNED_INT, NULL));
	}
	else
	{
		GL_CHECK(glDrawArrays(GL_PATCHES, 0, mVertexCount));
	}
    mpVertexArray->Disable();
    
    glUseProgram(0);

    CheckDebugLog();
}

//-----------------------------------------------------------------------------
void CPUTMeshOGL::BindVertexShaderLayout( CPUTMaterial *pMaterial, CPUTMaterial *pShadowCastMaterial )
{
}
void CPUTMeshOGL::SetIndexSubData( UINT offset, UINT size, void* pData)
{
    mpIndexBuffer->SetSubData(offset, size, pData);
}
void CPUTMeshOGL::SetVertexSubData( UINT offset, UINT size, void* pData)
{
    mpVertexBuffer->SetSubData(offset, size, pData);
}

