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
#include "CPUT_OGL.h"
#include "CPUTRenderStateBlockOGL.h"
#include "CPUTGuiControllerOGL.h"
//#include "CPUTBufferOGL.h"
//#include "CPUTTextureOGL.h"

#ifdef CPUT_FOR_OGLES
#define CPUT_GLSL_VERSION "#version 300 es \n"
#else
#ifdef CPUT_SUPPORT_IMAGE_STORE
#define CPUT_GLSL_VERSION "#version 420 \n"
#else
#define CPUT_GLSL_VERSION "#version 400 \n"
#endif
#endif


// static initializers
CPUT_OGL *gpSample;

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
#ifdef CPUT_OS_ANDROID
        LOGW("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
#else
        DEBUG_PRINT(_L("OpenGL error %08x, at %s:%i - for %s\n"), err, fname, line, stmt);
#endif
        abort();
    }
}

char* CPUT_OGL::GLSL_VERSION = NULL;
CPUT_SHADER_MACRO* CPUT_OGL::DEFAULT_MACROS = NULL;

CPUT_OGL::CPUT_OGL() :
        mpWindow(NULL),
        mbShutdown(false),
        mpPerFrameConstantBuffer(NULL),
        mpPerModelConstantBuffer(NULL)
    {
        GLSL_VERSION = CPUT_GLSL_VERSION;
        DEFAULT_MACROS = NULL;

        gpSample = this;
#if defined CPUT_OS_LINUX
        mpTimer = (CPUTTimer*) new CPUTTimerLinux();
#elif defined CPUT_OS_ANDROID
        mpTimer = (CPUTTimer*) new CPUTTimerLinux();
#elif defined CPUT_OS_WINDOWS
        mpTimer = (CPUTTimer*) new CPUTTimerWin();
#else
#error "No OS defined"
#endif
    }
 


// Destructor
//-----------------------------------------------------------------------------
CPUT_OGL::~CPUT_OGL()
{
    // all previous shutdown tasks should have happened in CPUTShutdown()

	SAFE_DELETE(mpTimer);
    
    // destroy the window
    if(mpWindow)
    {
        delete mpWindow;
        mpWindow = NULL;
    }

}

// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode CPUT_OGL::CPUTHandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    CPUTEventHandledCode handleCode;
    
    // dispatch event to GUI to handle GUI triggers (if any) #### no GUI for now
    //CPUTEventHandledCode handleCode = CPUTGuiController::GetController()->HandleKeyboardEvent(key);

    // dispatch event to users HandleMouseEvent() method
    HEAPCHECK;
    handleCode = HandleKeyboardEvent(key, state);
    HEAPCHECK;

    return handleCode;
}

// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode CPUT_OGL::CPUTHandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID  message)
{
//    assert(false);
    CPUTEventHandledCode handleCode = CPUT_EVENT_UNHANDLED;
    
    // dispatch event to GUI to handle GUI triggers (if any)
    handleCode = CPUTGuiControllerOGL::GetController()->HandleMouseEvent(x,y,wheel,state, message);

    // dispatch event to users HandleMouseEvent() method if it wasn't consumed by the GUI
    if(CPUT_EVENT_HANDLED != handleCode)
    {
        HEAPCHECK;
        handleCode = HandleMouseEvent(x,y,wheel,state, message);
        HEAPCHECK;
    }

    return handleCode;
}


// Call appropriate OS create window call
//-----------------------------------------------------------------------------
CPUTResult CPUT_OGL::MakeWindow(const cString WindowTitle, CPUTWindowCreationParams windowParams)
{
    CPUTResult result;

    HEAPCHECK;

    // if we have a window, destroy it
    if(mpWindow)
    {
        delete mpWindow;
        mpWindow = NULL;
    }

    HEAPCHECK;

	// As OpenGL is multiplatform be sure to call proper function on proper OS to create window for now.
#ifdef CPUT_OS_WINDOWS
	mpWindow = new CPUTWindowWin();
#elif defined CPUT_OS_LINUX
    mpWindow = new CPUTWindowX();
#elif defined CPUT_OS_ANDROID
    mpWindow = new CPUTWindowAndroid();
#else
#error "Need OS Support"
#endif

    result = mpWindow->Create((CPUT*)this, WindowTitle, windowParams);

    HEAPCHECK;

    return result;
}


// Return the current GUI controller
//-----------------------------------------------------------------------------
CPUTGuiController* CPUT_OGL::CPUTGetGuiController()
{
    return CPUTGuiControllerOGL::GetController();
}

bool CPUT_OGL::supportExtension(const string name)
{
	vector<string>::iterator it;

	for(it=extensions.begin() ; it < extensions.end(); it++) {

		if(name == (*it))
		{
	        DEBUG_PRINT(_L("%s == %s"), name.c_str(),(*it).c_str());
			return true;
		}
    }
    return false;
}
/*
CPUTResult CPUT_OGL::CreateOGLContext(CPUTContextCreation ContextParams )
{




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
    CPUTRenderStateBlock *pBlock = new CPUTRenderStateBlockOGL();
//    pBlock->CreateNativeResources();
    CPUTRenderStateBlock::SetDefaultRenderStateBlock( pBlock );
    
    cString name = _L("$cbPerFrameValues");
    mpPerFrameConstantBuffer = new CPUTBufferOGL(name);
    GLuint id = mpPerFrameConstantBuffer->GetBufferID();
#ifndef CPUT_FOR_OGLES2
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, mpPerFrameConstantBuffer->GetBufferID()));
    GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, sizeof(CPUTFrameConstantBuffer), NULL, GL_DYNAMIC_DRAW)); // NULL to just init buffer size
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    DEBUG_PRINT(_L("bind per frame buffer buffer %d\n"), id);
    GL_CHECK(ES3_COMPAT(glBindBufferBase(GL_UNIFORM_BUFFER, id, id)));
    DEBUG_PRINT(_L("completed - bind buffer %d\n"), id);
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    
#else
#warning "Need to do something with uniform buffers here"
#endif
    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer(_L(""), name, _L(""), mpPerFrameConstantBuffer);
    
    return CPUT_SUCCESS;
}
*/

// This function tests a created DirectX context for specific features required for
// the framework, and possibly sample.  If your sample has specific hw features
// you wish to check for at creation time, you can add them here and have them
// tested at startup time.  If no contexts support your desired features, then
// the system will revert to the DX reference rasterizer, or barring that, 
// pop up a dialog and exit.
//-----------------------------------------------------------------------------
bool CPUT_OGL::TestContextForRequiredFeatures()
{

    return true;
}

// Default creation routine for making the back/stencil buffers
//-----------------------------------------------------------------------------
CPUTResult CPUT_OGL::CreateContext()
{
    int width,height;
    mpWindow->GetClientDimensions(&width, &height);

    // Set default viewport
    glViewport( 0, 0, width, height );
    return CPUT_SUCCESS;
}



// Toggle the fullscreen mode
// This routine keeps the current desktop resolution.  DougB suggested allowing
// one to go fullscreen in a different resolution
//-----------------------------------------------------------------------------
CPUTResult CPUT_OGL::CPUTToggleFullScreenMode()
{    
    // get the current fullscreen state
    bool bIsFullscreen = CPUTGetFullscreenState();
     
    // toggle the state
    bIsFullscreen = !bIsFullscreen;

    // set the fullscreen state
 //   HRESULT hr = mpSwapChain->SetFullscreenState(bIsFullscreen, NULL);
    ASSERT( SUCCEEDED(CPUT_ERROR), _L("Failed toggling full screen mode.") );

    // trigger resize event so that all buffers can resize
    int x,y,width,height;
    mpWindow->GetClientDimensions(&x, &y, &width, &height);
    ResizeWindow(width,height);

    // trigger a fullscreen mode change call if the sample has decided to handle the mode change
    FullscreenModeChange( bIsFullscreen );

    return CPUT_SUCCESS;
}

// Set the fullscreen mode to a desired state
//-----------------------------------------------------------------------------
void CPUT_OGL::CPUTSetFullscreenState(bool bIsFullscreen)
{
    // get the current fullscreen state
    bool bCurrentFullscreenState = CPUTGetFullscreenState();
    if((bool)bCurrentFullscreenState == bIsFullscreen)
    {
        // no need to call expensive state change, full screen state is already
        // in desired state
        return;
    }

    // set the fullscreen state
//    HRESULT hr = mpSwapChain->SetFullscreenState(bIsFullscreen, NULL);
//   ASSERT( SUCCEEDED(hr), _L("Failed toggling full screen mode.") );

    // trigger resize event so that all buffers can resize
    int x,y,width,height;
    mpWindow->GetClientDimensions(&x, &y, &width, &height);
    ResizeWindow(width,height);



    // trigger a fullscreen mode change call if the sample has decided to handle the mode change
    FullscreenModeChange( bIsFullscreen );
}

// Get a bool indicating whether the system is in full screen mode or not
//-----------------------------------------------------------------------------
bool CPUT_OGL::CPUTGetFullscreenState()
{
	/*
    // get the current fullscreen state
    BOOL bCurrentlyFullscreen;
    IDXGIOutput *pSwapTarget=NULL;
    mpSwapChain->GetFullscreenState(&bCurrentlyFullscreen, &pSwapTarget);
    SAFE_RELEASE(pSwapTarget);
    if(TRUE == bCurrentlyFullscreen )
    {
        return true;
    }
	 * */
    return false;
}


// incoming resize event to be handled and translated
//-----------------------------------------------------------------------------
void CPUT_OGL::ResizeWindow(UINT width, UINT height)
{
    DEBUG_PRINT( _L("ResizeWindow") );

	DEBUG_PRINT(_L("Width %d Height %d"),width, height);
	
	CPUTGuiControllerOGL::GetController()->Resize();
	
    // set the viewport
    glViewport( 0, 0, width, height );

}

// 'soft' resize - just stretch-blit
//-----------------------------------------------------------------------------
void CPUT_OGL::ResizeWindowSoft(UINT width, UINT height)
{
    UNREFERENCED_PARAMETER(width);
    UNREFERENCED_PARAMETER(height);
    // trigger the GUI manager to resize
 //   CPUTGuiControllerDX11::GetController()->Resize();

    InnerExecutionLoop();
}

//-----------------------------------------------------------------------------
void CPUT_OGL::UpdatePerFrameConstantBuffer( CPUTRenderParameters &renderParams, double totalSeconds )
{
    //NOTE: Issue with using the value of the resultant Uniform Block in shader
    if( mpPerFrameConstantBuffer )
    {
        float4x4 view, projection, inverseView, viewProjection;
        float4 eyePosition, lightDir;
        if( renderParams.mpCamera )
        {
            view = *renderParams.mpCamera->GetViewMatrix();
            projection = *renderParams.mpCamera->GetProjectionMatrix();
            inverseView = inverse(*renderParams.mpCamera->GetViewMatrix());
            eyePosition = float4(renderParams.mpCamera->GetPosition(), 0.0f);        
            viewProjection = view * projection;
        }
        if( renderParams.mpShadowCamera )
        {
            lightDir = float4(normalize(renderParams.mpShadowCamera->GetLook()), 0);
        }

        CPUTFrameConstantBuffer cb;

        cb.View           = view;
        cb.InverseView    = inverseView;
        cb.Projection     = projection;
        cb.ViewProjection = viewProjection;
        cb.AmbientColor   = float4(mAmbientColor, 0.0f);
        cb.LightColor     = float4(mLightColor, 0.0f);
        cb.LightDirection = lightDir;
        cb.EyePosition    = eyePosition;
        cb.TotalSeconds   = float4((float)totalSeconds);

#ifndef CPUT_FOR_OGLES2
		GLuint BufferID = mpPerFrameConstantBuffer->GetBufferID();
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER,BufferID ));
        GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CPUTFrameConstantBuffer), &cb));
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
#else
#warning "Need to do something with uniform buffers here"
#endif
        
    }
}

// Call the user's Render() callback (if it exists)
//-----------------------------------------------------------------------------
void CPUT_OGL::InnerExecutionLoop()
{
#ifdef CPUT_GPA_INSTRUMENTATION
    D3DPERF_BeginEvent(D3DCOLOR(0xff0000), L"CPUT User's Render() ");
#endif
    if (!mbShutdown)
    {
        double deltaSeconds = mpTimer->GetElapsedTime();
        Update(deltaSeconds);
        Present(); // Note: Presenting immediately before Rendering minimizes CPU stalls (i.e., execute Update() before Present() stalls)

        double totalSeconds = mpTimer->GetTotalTime();
        //CPUTMaterialDX11::ResetStateTracking();
        Render(deltaSeconds);
        //if(!CPUTOSServices::GetOSServices()->DoesWindowHaveFocus())
        //{
          //  Sleep(100);
        //}
    }
    else
    {
#ifndef _DEBUG
        exit(0);
#endif
     //   Present(); // Need to present, or will leak all references held by previous Render()!
        ShutdownAndDestroy();
    }

#ifdef CPUT_GPA_INSTRUMENTATION
    D3DPERF_EndEvent();
#endif
}

// draw all the GUI controls
//-----------------------------------------------------------------------------
void CPUT_OGL::CPUTDrawGUI()
{
#ifdef CPUT_GPA_INSTRUMENTATION
    D3DPERF_BeginEvent(D3DCOLOR(0xff0000), L"CPUT Draw GUI");
#endif

    // draw all the Gui controls
    HEAPCHECK;
        CPUTGuiControllerOGL::GetController()->Draw();
    HEAPCHECK;

#ifdef CPUT_GPA_INSTRUMENTATION
        D3DPERF_EndEvent();
#endif
}

// Parse the command line for the parameters
// Only parameters that are specified on the command line are updated, if there
// are no parameters for a value, the previous WindowParams settings passed in
// are preserved
//-----------------------------------------------------------------------------
CPUTResult CPUT_OGL::CPUTParseCommandLine(cString commandLine, CPUTWindowCreationParams *pWindowParams, cString *pFilename)
{
    ASSERT( (NULL!=pWindowParams), _L("Required command line parsing parameter is NULL"));
    ASSERT( (NULL!=pFilename), _L("Required command line parsing parameter is NULL"));
   
    // there are no command line parameters, just return
    if(0==commandLine.size())
    {
        return CPUT_SUCCESS;
    }
 
    return CPUT_SUCCESS;
}




// Create a window context
//-----------------------------------------------------------------------------
CPUTResult CPUT_OGL::CPUTCreateWindowAndContext(const cString WindowTitle, CPUTWindowCreationParams windowParams)
{
    CPUTResult result = CPUT_SUCCESS;

    HEAPCHECK;

    // We shouldn't destroy old window if it already exist, 
    // Framework user should do this by himself to be aware
    // of what he is doing.
    if( mpWindow )
    {
        return CPUT_ERROR_WINDOW_ALREADY_EXISTS;
    }

    result = MakeWindow(WindowTitle, windowParams);
    if(CPUTFAILED(result))
    {
        return result;
    }


    HEAPCHECK;

    // create the GL context
    result = CreateOGLContext(windowParams.deviceParams);
    if(CPUTFAILED(result))
    {
        return result;
    }


    HEAPCHECK;

    result = CreateContext();

    CPUTRenderStateBlock *pBlock = new CPUTRenderStateBlockOGL();
    CPUTRenderStateBlock::SetDefaultRenderStateBlock( pBlock );

    cString name = _L("$cbPerFrameValues");
    mpPerFrameConstantBuffer = new CPUTBufferOGL(name);
    GLuint id = mpPerFrameConstantBuffer->GetBufferID();
#ifndef CPUT_FOR_OGLES2
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, mpPerFrameConstantBuffer->GetBufferID()));
    GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, sizeof(CPUTFrameConstantBuffer), NULL, GL_DYNAMIC_DRAW)); // NULL to just init buffer size
    DEBUG_PRINT(_L("bind per frame buffer buffer %d\n"), id);
//FIXME: constant buffer binding
    GL_CHECK(ES3_COMPAT(glBindBufferBase(GL_UNIFORM_BUFFER, id, id)));
    DEBUG_PRINT(_L("completed - bind buffer %d\n"), id);
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
#else
#warning "Need to do something with uniform buffers here"
#endif
    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer(_L(""), name, _L(""), mpPerFrameConstantBuffer);
    
    name = _L("$cbPerModelValues");
    mpPerModelConstantBuffer = new CPUTBufferOGL(name, GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(CPUTModelConstantBuffer), NULL);
    
    id = mpPerModelConstantBuffer->GetBufferID();
#ifndef CPUT_FOR_OGLES2
    DEBUG_PRINT(_L("Bind per model values %d"), id);
    GL_CHECK(ES3_COMPAT(glBindBufferBase(GL_UNIFORM_BUFFER, id, id)));
    DEBUG_PRINT(_L("Completed bind per model values"));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
#else
#warning "Need to do something with uniform buffers here"
#endif
    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer(_L(""), name, _L(""), mpPerModelConstantBuffer);

    name = _L("$cbGUIConstants");
    CPUTBuffer* pBuffer = new CPUTBufferOGL(name, GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW, sizeof(GUIConstants), NULL);
    
    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer(_L(""), name, _L(""), pBuffer);
    SAFE_RELEASE(pBuffer);
	// Add our programatic (and global) material parameters
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerFrameValues"), _L("$cbPerFrameValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerModelValues"), _L("$cbPerModelValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbGUIValues"), _L("$cbGUIValues") );
    HEAPCHECK;

    // Trigger a post-create user callback event
    Create();
    HEAPCHECK;

    //
    // Start the timer after everything is initialized and assets have been loaded
    //
    mpTimer->StartTimer();

    int x,y,width,height;
    mpWindow->GetClientDimensions(&x, &y, &width, &height);

    ResizeWindow(width,height);

    return result;
}


// Pop up a message box with specified title/text
//-----------------------------------------------------------------------------
void CPUT_OGL::DrawLoadingFrame()
{
    DEBUG_PRINT(_L("DrawLoadingFrame()"));
    // fill first frame with clear values so render order later is ok
    const float srgbClearColor[] = { 0.0993f, 0.0993f, 0.0993f, 1.0f };
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    CPUTGuiControllerOGL *pGUIController = CPUTGuiControllerOGL::GetController();
    CPUTText *pText = NULL;
    pGUIController->CreateText(_L("Just a moment, now loading..."), 999, 0, &pText);

    pGUIController->EnableAutoLayout(false);
    int textWidth, textHeight;
    pText->GetDimensions(textWidth, textHeight);
    int width,height;
    mpWindow->GetClientDimensions(&width, &height);
    pText->SetPosition(width/2-textWidth/2, height/2);

    pGUIController->Draw();
    pGUIController->DeleteAllControls();
    pGUIController->EnableAutoLayout(true);
    
    // present loading screen
    Present();
    
}

// Pop up a message box with specified title/text
//-----------------------------------------------------------------------------
void CPUT_OGL::CPUTMessageBox(const cString DialogBoxTitle, const cString DialogMessage)
{
    CPUTOSServices::OpenMessageBox(DialogBoxTitle.c_str(), DialogMessage.c_str());
}

// start main message loop
//-----------------------------------------------------------------------------
int CPUT_OGL::CPUTMessageLoop()
{
    return mpWindow->StartMessageLoop();
}

// Window is closing. Shut the system to shut down now, not later.
//-----------------------------------------------------------------------------
void CPUT_OGL::DeviceShutdown()
{
	/*
    if(mpSwapChain)
    {
        // DX requires setting fullscreenstate to false before exit.
        mpSwapChain->SetFullscreenState(false, NULL);
    }
     */
    if (mbShutdown == false) {
        mbShutdown = true;
        ShutdownAndDestroy();
    }
}

// Shutdown the CPUT system
// Destroy all 'global' resource handling objects, all asset handlers,
// the DX context, and everything EXCEPT the window
//-----------------------------------------------------------------------------
void CPUT_OGL::Shutdown()
{
    // release the lock on the mouse (if there was one)
//    CPUTOSServices::GetOSServices()->ReleaseMouse();
    mbShutdown = true;
}

// Frees all resources and removes all assets from asset library
//-----------------------------------------------------------------------------
void CPUT_OGL::RestartCPUT()
{
    //
    // Clear out all CPUT resources
    //
//    CPUTInputLayoutCacheDX11::GetInputLayoutCache()->ClearLayoutCache();
    CPUTAssetLibrary::GetAssetLibrary()->ReleaseAllLibraryLists();
//	CPUTGuiControllerDX11::GetController()->DeleteAllControls();
//	CPUTGuiControllerDX11::GetController()->ReleaseResources();

    //
    // Clear out all DX resources and contexts
    //
    DestroyOGLContext();

    //
    // Signal the window to close
    //
  //  mpWindow->Destroy();

    //
    // Clear out the timer
    //
    mpTimer->StopTimer();
    mpTimer->ResetTimer();
    
    HEAPCHECK;
}
// Actually destroy all 'global' resource handling objects, all asset handlers,
// the DX context, and everything EXCEPT the window
//-----------------------------------------------------------------------------
void CPUT_OGL::ShutdownAndDestroy()
{
    // make sure no more rendering can happen
    mbShutdown = true;

    // call the user's OnShutdown code
    Shutdown();
	
    CPUTAssetLibrary::DeleteAssetLibrary();
    CPUTGuiControllerOGL::DeleteController();
	CPUTRenderStateBlock::SetDefaultRenderStateBlock( NULL );
	
    SAFE_RELEASE(mpPerFrameConstantBuffer);
    SAFE_RELEASE(mpPerModelConstantBuffer);

    DestroyOGLContext();

    // Tell the window layer to post a close-window message to OS
    // and stop the message pump
    mpWindow->Destroy();

    HEAPCHECK;
}

//-----------------------------------------------------------------------------
void CPUTSetDebugName( void *pResource, std::wstring name )
{
#ifdef _DEBUG
    //char pCharString[CPUT_MAX_STRING_LENGTH];
    const wchar_t *pWideString = name.c_str();
    //UINT ii;
	/*
    UINT length = min( (UINT)name.length(), (CPUT_MAX_STRING_LENGTH-1));
    for(ii=0; ii<length; ii++)
    {
        pCharString[ii] = (char)pWideString[ii];
    }
    pCharString[ii] = 0; // Force NULL termination
    ((ID3D11DeviceChild*)pResource)->SetPrivateData( WKPDID_D3DDebugObjectName, (UINT)name.length(), pCharString );
	 * */
#endif // _DEBUG
}
