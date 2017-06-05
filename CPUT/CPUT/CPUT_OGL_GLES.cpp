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
#include "CPUTRenderStateBlockOGL.h"
#include "CPUTGuiControllerOGL.h"
//#include "CPUTBufferOGL.h"
//#include "CPUTTextureOGL.h"

#ifdef CPUT_OS_WINDOWS
#define OSSleep Sleep
#else
#include <unistd.h>
#define OSSleep sleep
#endif
#ifdef CPUT_FOR_OGLES3_COMPAT
CPUTOglES3CompatFuncPtrs gOGLESCompatFPtrs;
#endif

PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = NULL;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture = NULL;
PFNGLPATCHPARAMETERIPROC glPatchParameteri = NULL;
PFNGLPATCHPARAMETERFVPROC glPatchParameterfv = NULL;

CPUTResult CPUT_OGL::DestroyOGLContext(void)
{
     // Disabling and deleting all rendering contexts
	if(display!= EGL_NO_DISPLAY)
	{
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if(context != EGL_NO_CONTEXT)
		{
			eglDestroyContext(display,context);
		}
		if(surface != EGL_NO_SURFACE)
		{
			eglDestroySurface(display,surface);
		}
		eglTerminate(display);
	}
	return CPUT_SUCCESS;
}


//-----------------------------------------------------------------------------
CPUTResult CPUT_OGL::CreateOGLContext(CPUTContextCreation ContextParams )
{
    CPUTResult result = CPUT_ERROR;
    
    
    // Get a matching FB config
    const EGLint attribs[] =
    {
        EGL_SURFACE_TYPE,     EGL_WINDOW_BIT,
        EGL_BLUE_SIZE,        8,
        EGL_GREEN_SIZE,       8,
        EGL_RED_SIZE,         8,
        EGL_ALPHA_SIZE,       8,
        EGL_DEPTH_SIZE,       24,
        EGL_STENCIL_SIZE,     8,
        //EGL_SAMPLE_BUFFERS  , 1,
        //EGL_SAMPLES         , 4,
        EGL_NONE
    };

#ifndef EGL_CONTEXT_MINOR_VERSION_KHR
	#define EGL_CONTEXT_MINOR_VERSION_KHR 0x30FB
#endif

#ifndef EGL_CONTEXT_MAJOR_VERSION_KHR
	#define EGL_CONTEXT_MAJOR_VERSION_KHR EGL_CONTEXT_CLIENT_VERSION
#endif



    const EGLint contextAttribs[] =
    {
#ifdef CPUT_FOR_OGLES2
        EGL_CONTEXT_CLIENT_VERSION, 2,
#elif defined CPUT_FOR_OGLES3
    #if defined CPUT_FOR_OGLES3_1
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
        EGL_CONTEXT_MINOR_VERSION_KHR, 1,
    #else
        EGL_CONTEXT_CLIENT_VERSION, 3,
    #endif
#else
#error "No ES version specified"
#endif
        EGL_NONE
    };
    
    EGLint format;
    EGLint numConfigs;
    EGLConfig config;
    EGLBoolean success = EGL_TRUE;
    
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) 
    {
        return result;
    }
    
    success = eglInitialize(display, 0, 0);
    if (!success)
    {
        DEBUG_PRINT_ALWAYS((_L("Failed to initialise EGL")));
        return result;
    }
    
    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    success = eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    if (!success)
    {
        DEBUG_PRINT_ALWAYS((_L("Failed to choose config")));
        return result;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

#ifdef CPUT_ANDROID
    ANativeWindow_setBuffersGeometry(mpWindow->GetHWnd(), 0, 0, format);
#endif
    surface = eglCreateWindowSurface(display, config, mpWindow->GetHWnd(), NULL);

    if (surface == EGL_NO_SURFACE)
    {
        DEBUG_PRINT_ALWAYS((_L("Failed to create EGLSurface")));
        return result;
    }
      
	DEBUG_PRINT(_L("contextAttribs: %x %x\n"), contextAttribs[0],contextAttribs[1]);
	DEBUG_PRINT(_L("contextAttribs: %x %x\n"), contextAttribs[2],contextAttribs[3]);

    context = eglCreateContext(display, config, NULL, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        DEBUG_PRINT_ALWAYS((_L("Failed to create EGLContext")));
        return result;
    }
    
    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) 
    {
        DEBUG_PRINT_ALWAYS((_L("Failed to eglMakeCurrent")));
        return result;
    }

 
    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);
	mpWindow->SetClientDimensions(width, height);
    
    glClearColor ( 0, 0.5, 1, 1 );
    glClear ( GL_COLOR_BUFFER_BIT );
    eglSwapBuffers( display, surface);
    OSSleep( 1 );
 
    glClearColor ( 1, 0.5, 0, 1 );
    glClear ( GL_COLOR_BUFFER_BIT );
    eglSwapBuffers( display, surface);
 
    OSSleep( 1 );

    DEBUG_PRINT((_L("GL Version: %s\n")), glGetString(GL_VERSION));
    
   // Creates table of supported extensions strings
    extensions.clear();
    string tmp;
    sint32 begin, end;
    tmp   = string( (char*)glGetString( GL_EXTENSIONS ) );

    begin = 0;
    end   = tmp.find( ' ', 0 );

    DEBUG_PRINT(_L("Checking Extensions"));
    
    while( end != string::npos )
    {
        DEBUG_PRINT((_L("extension %s")), tmp.substr( begin, end-begin ).c_str());
        extensions.insert( extensions.end(), tmp.substr( begin, end-begin ) );
        begin = end + 1;
        end   = tmp.find( ' ', begin );

    }

	if(supportExtension("GL_INTEL_tessellation"))
	{
		glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)eglGetProcAddress("glPatchParameteri");
        DEBUG_PRINT(_L("%s = %p"), "glPatchParameteri",(void*)glPatchParameteri);
		glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC)eglGetProcAddress("glPatchParameterfv");
        DEBUG_PRINT(_L("%s = %p"), "glPatchParameterfv",(void*)glPatchParameterfv);
	}
	//if(supportExtension("GL_INTEL_compute_shader"))
	{
		glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)eglGetProcAddress("glDispatchCompute");
        DEBUG_PRINT(_L("%s = %p"), "glDispatchCompute",(void*)glDispatchCompute);
		glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)eglGetProcAddress("glBindImageTexture");
        DEBUG_PRINT(_L("%s = %p"), "glBindImageTexture",(void*)glBindImageTexture);
	}

//	if(supportExtension("GL_INTEL_shader_image_load_store"))
//	{
//		glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)eglGetProcAddress("glBindImageTexture");
//        DEBUG_PRINT(_L("%s = %p"), "glBindImageTexture",(void*)glBindImageTexture);
//	}



#ifdef CPUT_FOR_OGLES3_COMPAT
    // Set up the ES3 compat functions
    memset((void *)&gOGLESCompatFPtrs, 0, sizeof(CPUTOglES3CompatFuncPtrs));

#define ES3_FUNC(a, b, c) \
    gOGLESCompatFPtrs.b = (CPUT_PASTE(CPUTOglES3CompatFuncPtrs::FPtrType_, b)) eglGetProcAddress(CPUT_STRINGIFY(b)); \
    DEBUG_PRINT("%s = %p", CPUT_STRINGIFY(b), gOGLESCompatFPtrs.b); \
    if (! gOGLESCompatFPtrs.b) \
        return CPUT_ERROR;
    
#include "CPUTOGLES3Compat.h"
    
#undef ES3_FUNC
#endif  

    return CPUT_SUCCESS;
}