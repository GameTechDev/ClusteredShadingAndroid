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
#include "CPUTWindowX.h"

#include <unistd.h>

// static initializers
CPUT* CPUTWindowX::mCPUT=NULL;
bool CPUTWindowX::mbMaxMinFullScreen=false;

// Constructor
//-----------------------------------------------------------------------------
CPUTWindowX::CPUTWindowX() :
    mhInst(0),
    mhWnd(0),
    mAppClosedReturnCode(0)
{
    mAppTitle.clear();
}

// Destructor
//-----------------------------------------------------------------------------
CPUTWindowX::~CPUTWindowX()
{
    mAppTitle.clear();
}

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension)
 
{
 
  const char *start;
  const char *where, *terminator;
 
  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if ( where || *extension == '\0' )
    return false;
 
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for ( start = extList; ; ) {
    where = strstr( start, extension );
 
    if ( !where )
      break;
 
    terminator = where + strlen( extension );
 
    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return true;
 
    start = terminator;
  }
 
  return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}



void glErrorCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam)
{
//    DebugOutputToFile(source, type, id, severity, message);
    
    return;
}


extern void CheckOpenGLError(const char* stmt, const char* fname, int line);

#ifdef DEBUG
    #define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (false)
#else
    #define GL_CHECK(stmt) stmt
#endif

/* A simple function that will read a file into an allocated char pointer buffer */
char* filetobuf2(const char *file)
{
    FILE *fptr;
    long length;
    char *buf;
 
    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
 
    return buf; /* Return the buffer */
}
// Create window
// 
// Much of the Creation code was taken from the openGL wiki at:
// 
// http://www.opengl.org/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX) on 2/1/13
// 
//-----------------------------------------------------------------------------
CPUTResult CPUTWindowX::Create(CPUT* cput, const cString WindowTitle, const int windowWidth, const int windowHeight, int windowX, int windowY)
{
    CPUTResult result = CPUT_ERROR;
    
	pDisplay = XOpenDisplay(NULL);
    if (!pDisplay) {
        return result;
    }
    
    // Get a matching FB config
    int visual_attribs[] =
    {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };
    
    int glx_major, glx_minor;

    // FBConfigs were added in GLX version 1.3.
    if (!glXQueryVersion(pDisplay, &glx_major, &glx_minor)) {
        printf( "glXQueryVersion failed" );
        exit(1);
    }
    
    if (((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
        printf( "Invalid GLX version" );
        exit(1);
    }
    printf( "GLX version: %d.%d\n", glx_major, glx_minor );
    
    // Getting matching framebuffer configs
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(pDisplay, DefaultScreen( pDisplay ), visual_attribs, &fbcount );
    if ( !fbc )
    {
        printf( "Failed to retrieve a framebuffer config\n" );
        exit(1);
    }

    // Pick the FB config/visual with the most samples per pixel
    printf( "Getting XVisualInfos\n" );
    int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
    
    for ( int i = 0; i < fbcount; i++ )
    {
        XVisualInfo *vi = glXGetVisualFromFBConfig( pDisplay, fbc[i] );
        if (vi)
        {
            int samp_buf, samples;
            glXGetFBConfigAttrib( pDisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
            glXGetFBConfigAttrib( pDisplay, fbc[i], GLX_SAMPLES       , &samples  );
 
            printf( "  Matching fbconfig %d, visual ID 0lx%2lx: SAMPLE_BUFFERS = %d,"
                    " SAMPLES = %d\n", 
                    i, vi -> visualid, samp_buf, samples );
 
            if ( best_fbc < 0 || samp_buf && samples > best_num_samp ) {
                best_fbc = i, best_num_samp = samples;
            }
            if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp ) {
                worst_fbc = i, worst_num_samp = samples;
            }
        }
        XFree( vi );
    }
    
    GLXFBConfig bestFbc = fbc[ best_fbc ];
 
    // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
    XFree( fbc );
    
    // Get a visual
    XVisualInfo *vi = glXGetVisualFromFBConfig( pDisplay, bestFbc );
    printf( "Chosen visual ID = 0x%lx\n", vi->visualid );
 
    printf( "Creating colormap\n" );
    XSetWindowAttributes swa;
    swa.colormap = XCreateColormap(pDisplay,
                                          RootWindow(pDisplay, vi->screen), 
                                          vi->visual, AllocNone );
    swa.background_pixmap = None ;
    swa.border_pixel      = 0;
    swa.event_mask        = ExposureMask | Button1MotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask;
    
    printf( "Creating window\n" );
    win = XCreateWindow( pDisplay, RootWindow(pDisplay, vi->screen ), 
                        0, 0, windowWidth, windowHeight, 0, vi->depth, InputOutput, 
                        vi->visual, 
                        CWBorderPixel | CWColormap | CWEventMask, &swa );
    if ( !win )
    {
        printf( "Failed to create window.\n" );
        exit(1);
    }

    // Done with the visual info data
    XFree( vi );
 
    // register interest in the delete window message
    wmDeleteMessage = XInternAtom(pDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(pDisplay, win, &wmDeleteMessage, 1);
   
    XStoreName( pDisplay, win, "GL 4.0 Window" );
    
    printf( "Mapping window\n" );
    XMapWindow( pDisplay, win );
 
    // Get the default screen's GLX extension list
    const char *glxExts = glXQueryExtensionsString(pDisplay,
                                                   DefaultScreen( pDisplay ) );
 
    // NOTE: It is not necessary to create or make current to a context before
    // calling glXGetProcAddressARB
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
                                  glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
 
    ctx = 0;
    
    // Install an X error handler so the application won't exit if GL 3.0
    // context allocation fails.
    //
    // Note this error handler is global.  All display connections in all threads
    // of a process use the same error handler, so be sure to guard against other
    // threads issuing X commands while this code is running.
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) =
         XSetErrorHandler(&ctxErrorHandler);
 
    // Check for the GLX_ARB_create_context extension string and the function.
    // If either is not present, use GLX 1.3 context creation method.
    if (!isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
        !glXCreateContextAttribsARB )
    {
        printf("glXCreateContextAttribsARB() not found"
               " ... using old-style GLX context\n" );
        ctx = glXCreateNewContext( pDisplay, bestFbc, GLX_RGBA_TYPE, 0, True );
    }
    
    // If it does, try to get a GL 3.0 context!
    else
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_DEBUG_BIT_ARB,
            None
        };
 
        printf( "Creating context %d.%d\n",  4, 1);
        ctx = glXCreateContextAttribsARB(pDisplay, bestFbc, 0,
                                         True, context_attribs );
 
        // Sync to ensure any errors generated are processed.
        XSync( pDisplay, False );
        if ( !ctxErrorOccurred && ctx ) {
            printf( "Created GL 3.2 context\n" );
        }
        else
        {
            // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
            // When a context version below 3.0 is requested, implementations will
            // return the newest context version compatible with OpenGL versions less
            // than version 3.0.
            // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
            context_attribs[1] = 1;
            // GLX_CONTEXT_MINOR_VERSION_ARB = 0
            context_attribs[3] = 0;
 
            ctxErrorOccurred = false;
 
            printf( "Failed to create GL 3.0 context"
                    " ... using old-style GLX context\n" );
 //           ctx = glXCreateContextAttribsARB( pDisplay, bestFbc, 0, 
   //                                           True, context_attribs );
        }
    }
 
    // Sync to ensure any errors generated are processed.
    XSync( pDisplay, False );
 
    // Restore the original error handler
    XSetErrorHandler( oldHandler );
    
    if ( ctxErrorOccurred || !ctx )
    {
        printf( "Failed to create an OpenGL context\n" );
        exit(1);
    }
 
    // Verifying that context is a direct context
    if ( ! glXIsDirect ( pDisplay, ctx ) )
    {
        printf( "Indirect GLX rendering context obtained\n" );
    }
    else
    {
        printf( "Direct GLX rendering context obtained\n" );
    }
    
    printf( "Making context current\n" );
    glXMakeCurrent( pDisplay, win, ctx );

    glewExperimental=true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    
    // glewInit can generate an error which is apparently innocuous. Clear it out here.
    // http://www.opengl.org/wiki/OpenGL_Loading_Library on 2/20/13
    glGetError();
    
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    
    glClearColor ( 0, 0.5, 1, 1 );
    glClear ( GL_COLOR_BUFFER_BIT );
    glXSwapBuffers ( pDisplay, win );
    sleep( 1 );
 
    glClearColor ( 1, 0.5, 0, 1 );
    glClear ( GL_COLOR_BUFFER_BIT );
    glXSwapBuffers ( pDisplay, win );
 
    sleep( 1 );

    printf("GL Version: %s\n", glGetString(GL_VERSION));
    
    if (GLEW_ARB_vertex_array_object) {
        /* It is safe to use the ARB_vertex_program extension here. */
        printf("VAOs are supported\n");
    } else {
        printf("VAOs are NOT supported\n");    
    }
    
    if (GLEW_VERSION_4_0)
    {
        printf("OpenGL version 4.0 supported\n");
    } else {
        printf("OpenGL version 4.0 NOT supported\n");
    }
    
    mCPUT = (CPUT*) cput;
    
    /*
    if(mhWnd)
    {
        return CPUT_ERROR_WINDOW_ALREADY_EXISTS;
    }

    ASSERT( (windowX < GetSystemMetrics(SM_CXFULLSCREEN) && (windowX>=-1)), _L("You are attempting to create a window outside the desktop coordinates.  Check your CPUTWindowCreationParams"));
    ASSERT( (windowY < GetSystemMetrics(SM_CYFULLSCREEN) && (windowY>=-1)), _L("You are attempting to create a window outside the desktop coordinates.  Check your CPUTWindowCreationParams"));

    // get the hInstance of this executable
    mhInst = GetModuleHandle(NULL);
    if(NULL==mhInst)
    {
        return CPUT_ERROR_CANNOT_GET_WINDOW_INSTANCE;
    }

    // set up app title (if not specified)
    mAppTitle = WindowTitle;

    if(0==mAppTitle.compare(_L("")))
    {
        mAppTitle = _L("CPUT Sample");
    }

    // Register the Win32 class for this app
    WNDCLASS wc;
    if(TRUE == GetClassInfo(mhInst, mAppTitle.c_str(), &wc))
    {
        // point to the existing one
        mhInst = wc.hInstance;
    }
    else
    {
        // register a new windows class
        ATOM classID;
        classID = MyRegisterClass(mhInst);
        if(0==classID)
        {
			HandleWin32Error();
            return CPUT_ERROR_WINDOW_CANNOT_REGISTER_APP;
        }
    }


	// Perform Win32 instance initialization
    const int nCmdShow = SW_SHOWNORMAL;
	if (false == InitInstance(nCmdShow, windowWidth, windowHeight, windowX, windowY))
	{
		return CPUT_ERROR_CANNOT_GET_WINDOW_INSTANCE;
	}

    // store the CPUT pointer
    mCPUT = (CPUT*) cput;
*/
    return CPUT_SUCCESS;
}



// Destroy window
//-----------------------------------------------------------------------------
int CPUTWindowX::Destroy()
{
    glXMakeCurrent( pDisplay, 0, 0 );
    glXDestroyContext( pDisplay, ctx );
    ctx = NULL;

    XDestroyWindow(pDisplay, win);
    XCloseDisplay(pDisplay);
    
    pDisplay = NULL;    
    mCPUT = NULL;
    
    return true;
}


// Window return code on close
//-----------------------------------------------------------------------------
int CPUTWindowX::ReturnCode()
{
    return mAppClosedReturnCode;
}

//  Register window class
//-----------------------------------------------------------------------------
ATOM CPUTWindowX::MyRegisterClass(HINSTANCE hInstance)
{
    /*
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

    // load icon from resource file
    LPCTSTR iconPathName= L"CPUT.ico";
    UINT icon_flags = LR_LOADFROMFILE | LR_DEFAULTSIZE;
    HANDLE hIcon = LoadImage(hInstance, iconPathName, IMAGE_ICON, 0, 0, icon_flags);

    // set up RegisterClass struct
    wcex.style			= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= (HICON) hIcon; //LoadIcon(hInstance, iconSTR);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= mAppTitle.c_str();
	wcex.hIconSm		= NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // no small icon for now

    // register the window class
	return RegisterClassEx(&wcex);
     * */
}

CPUTResult CPUTWindowX::GetDimensions(int *pWidth, int *pHeight)
{
    XWindowAttributes windowAttributes;
    
    XGetWindowAttributes(pDisplay, win, &windowAttributes);
    
    *pWidth  = windowAttributes.width;
    *pHeight = windowAttributes.height;
    
    return CPUT_SUCCESS;
}

// InitInstance
// Saves the windows instance handle, creates, and displays the main program
// window
//-----------------------------------------------------------------------------
bool CPUTWindowX::InitInstance(int nCmdShow, int windowWidth, int windowHeight, int windowX, int windowY)
{
    /*
    // assure we have a valid hInstance
    ASSERT(NULL!=mhInst, _L(""));

   // zero sized windows means - you choose the size. :)
   if( (0==windowWidth) || (0==windowHeight) )
   {
       CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
       pServices->GetDesktopDimensions(&windowWidth, &windowHeight);

       // default window size 1280x720
       // but if screen is smaller than 1280x720, then pick 1/3 the screen size 
       // so that it doesn't appear off the edges
       if(1280>windowWidth)
       {
           windowWidth = (2*windowWidth)/3;
           windowHeight = (2*windowHeight)/3;
       }
       else
       {
        
        windowWidth=1280;
        windowHeight=720;
       }
   }

   // set up size structure
   RECT rc = { 0, 0, windowWidth, windowHeight };
   AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

   // if x = -1, then let windows decide where to put it
   if(-1==windowX)
   {
       windowX = CW_USEDEFAULT;
   }

   // create the window
   mhWnd = CreateWindow(mAppTitle.c_str(), mAppTitle.c_str(),
       WS_OVERLAPPEDWINDOW,
       windowX, //CW_USEDEFAULT,
       windowY, //CW_USEDEFAULT,
       rc.right - rc.left,
       rc.bottom - rc.top,
       NULL,
       NULL,
       mhInst,
       NULL);

   if (!mhWnd)
   {
      return FALSE;
   }

   ShowWindow(mhWnd, nCmdShow);
   UpdateWindow(mhWnd);

   // initialize the OS services with the hWND so you can make
   // reference to this object
   CPUTOSServices *pServices = CPUTOSServices::GetOSServices();
   pServices->SethWnd(mhWnd);
*/
   return true;
}


//
// WndProc
// Handles the main message loop's events/messages
//-----------------------------------------------------------------------------
LRESULT CPUTWindowX::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    /*
    CPUTEventHandledCode handledCode = CPUT_EVENT_UNHANDLED;
    LRESULT res;

    switch (message)
	{
	case WM_COMMAND:
        int     wmId, wmEvent;
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

        // handle any menu item events here
        // see reference code in file history for examples
		break;

    case WM_KEYDOWN:
        if(mCPUT)
        {
            CPUTKey key = ConvertSpecialKeyCode(wParam, lParam);
            if(KEY_NONE!=key)
            {
                handledCode = mCPUT->CPUTHandleKeyboardEvent( key );
            }
        }
        break;

    case WM_CHAR: // WM_KEYDOWN: gives you EVERY key - including shifts/etc
        if(mCPUT)
        {
            CPUTKey key = ConvertKeyCode(wParam, lParam);
            if(KEY_NONE!=key)
            {
                handledCode = mCPUT->CPUTHandleKeyboardEvent( key );
            }
        }
        break;

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
        // handle double-click events
        break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
        if(mCPUT)
        {
            CPUTMouseState state = ConvertMouseState(wParam);

            short xPos = LOWORD(lParam);
            short yPos = HIWORD(lParam);

            handledCode = mCPUT->CPUTHandleMouseEvent(xPos, yPos, 0, state);
        }
        break;

    case WM_MOUSEWHEEL:
        if(mCPUT)
        {
            // get mouse position
            short xPos = LOWORD(lParam);
            short yPos = HIWORD(lParam);

            // get wheel delta
            int wheel = GET_WHEEL_DELTA_WPARAM(wParam);  // one 'click'

            handledCode = mCPUT->CPUTHandleMouseEvent(xPos, yPos, wheel, CPUT_MOUSE_WHEEL);
        }
        return 0;
        break;

	case WM_PAINT:
	    PAINTSTRUCT ps;
	    HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_SIZING:
    case WM_MOVING:
    case WM_ERASEBKGND:
        // overriding this to do nothing avoids flicker and
        // the expense of re-creating tons of gfx contexts as it resizes
        break;

    //case WM_ACTIVATE:
        // check for maximize/minimize
      //  break;

    case WM_SIZE:
        int width, height;
        height = HIWORD(lParam);
        width  = LOWORD(lParam);
            
        RECT windowRect;
        if(0==GetClientRect(hWnd, &windowRect)) // this gets the client area inside the window frame *excluding* frames/menu bar/etc
            break;
        width = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;

        if(mCPUT)
        {
            // maximize/minimize effect
            if( (SIZE_MAXIMIZED == wParam) || (SIZE_MINIMIZED==wParam)  )
            {
                // resize for new max/min size
                mCPUT->ResizeWindow(width,height);
                mbMaxMinFullScreen = true;
            }
            else if(SIZE_RESTORED == wParam)
            {
                if(true == mbMaxMinFullScreen)
                {
                    // resize for new max/min size
                    mCPUT->ResizeWindow(width,height);
                    mbMaxMinFullScreen = false;
                }
                else
                {
                    // do a stretch-blit while actively sizing by just rendering to un-resized back buffer
                    mCPUT->ResizeWindowSoft(width, height);
                }
            }
        }
        break;
    
    case WM_EXITSIZEMOVE:
        // update the system's size and make callback
        if(mCPUT)
        {
            RECT windowRect;
            if(0==GetClientRect(hWnd, &windowRect)) // this gets the client area inside the window frame *excluding* frames/menu bar/etc
                break;

            width = windowRect.right - windowRect.left;
            height = windowRect.bottom - windowRect.top;
            mCPUT->ResizeWindow(width,height);
        }
        break;


    case WM_DESTROY:
        // time to shut down the system
        PostQuitMessage(0);
        break;

	default:
        // we don't handle it - pass it on thru to parent
        res = DefWindowProc(hWnd, message, wParam, lParam);
        return res;
	}

    // translate handled code
    if(CPUT_EVENT_HANDLED == handledCode)
    {
        return 1;
    }
*/
	return 0;
}


// Convert OS specific key events to CPUT events
//-----------------------------------------------------------------------------
CPUTKey CPUTWindowX::ConvertKeyCode(XKeyEvent *pKeyEvent)
{
    KeySym keySymbol;
    const int  textBufferSize = 100;
    char textBuffer[100];
    int count;
                        
    count = XLookupString(pKeyEvent, textBuffer, textBufferSize, &keySymbol, 0);
 
    switch(keySymbol)
    {
    case XK_a:
    case XK_A:
        return KEY_A;
    case XK_b:
    case XK_B:
        return KEY_B;
    case XK_c:
    case XK_C:
        return KEY_C;
    case XK_d:
    case XK_D:
        return KEY_D;
    case XK_e:
    case XK_E:
        return KEY_E;
    case XK_f:
    case XK_F:
        return KEY_F;
    case XK_g:
    case XK_G:
        return KEY_G;
    case XK_h:
    case XK_H:
        return KEY_H;
    case XK_i:
    case XK_I:
        return KEY_I;
    case XK_j:
    case XK_J:
        return KEY_J;
    case XK_k:
    case XK_K:
        return KEY_K;
    case XK_l:
    case XK_L:
        return KEY_L;
    case XK_m:
    case XK_M:
        return KEY_M;
    case XK_n:
    case XK_N:
        return KEY_N;
    case XK_o:
    case XK_O:
        return KEY_O;
    case XK_p:
    case XK_P:
        return KEY_P;
    case XK_q:
    case XK_Q:
        return KEY_Q;
    case XK_r:
    case XK_R:
        return KEY_R;
    case XK_s:
    case XK_S:
        return KEY_S;
    case XK_t:
    case XK_T:
        return KEY_T;
    case XK_u:
    case XK_U:
        return KEY_U;
    case XK_v:
    case XK_V:
        return KEY_V;
    case XK_w:
    case XK_W:
        return KEY_W;
    case XK_x:
    case XK_X:
        return KEY_X;
    case XK_y:
    case XK_Y:
        return KEY_Y;
    case XK_z:
    case XK_Z:
        return KEY_Z;


        // number keys
    case XK_1:
        return KEY_1;
    case XK_2:
        return KEY_2;
    case XK_3:
        return KEY_3;
    case XK_4:
        return KEY_4;
    case XK_5:
        return KEY_5;
    case XK_6:
        return KEY_6;
    case XK_7:
        return KEY_7;
    case XK_8:
        return KEY_8;
    case XK_9:
        return KEY_9;
    case XK_0:
        return KEY_0;


    // symbols
    case XK_space:
        return KEY_SPACE;
//    case XK_apostrophe:
  //      return KEY_BACKQUOTE;
    case XK_asciitilde:
        return KEY_TILDE;
    case XK_exclam:
        return KEY_EXCLAMATION;
    case XK_at:
        return KEY_AT;
    case XK_numbersign:
        return KEY_HASH;
    case XK_dollar:
        return KEY_$;
    case XK_percent:
        return KEY_PERCENT;
    case XK_asciicircum:
        return KEY_CARROT;
    case XK_ampersand:
        return KEY_ANDSIGN;
    case XK_asterisk:
        return KEY_STAR;
    case XK_parenleft:
        return KEY_OPENPAREN;
    case XK_parenright:
        return KEY_CLOSEPARN;
    case XK_underscore:
        return KEY__;
    case XK_minus:
        return KEY_MINUS;
    case XK_plus:
        return KEY_PLUS;
    case XK_bracketleft:
        return KEY_OPENBRACKET;
    case XK_bracketright:
        return KEY_CLOSEBRACKET;
    case XK_braceleft:
        return KEY_OPENBRACE;
    case XK_braceright:
        return KEY_CLOSEBRACE;
    case XK_backslash:
        return KEY_BACKSLASH;
    case XK_bar:
        return KEY_PIPE;
    case XK_semicolon:
        return KEY_SEMICOLON;
    case XK_colon:
        return KEY_COLON;
    case XK_apostrophe:
        return KEY_SINGLEQUOTE;
    case XK_quotedbl:
        return KEY_QUOTE;
    case XK_comma:
        return KEY_COMMA;
    case XK_period:
        return KEY_PERIOD;
    case XK_slash:
        return KEY_SLASH;
    case XK_less:
        return KEY_LESS;
    case XK_greater:
        return KEY_GREATER;
    case XK_question:
        return KEY_QUESTION;
    case XK_Shift_L:
        return KEY_SHIFT;
    case XK_Shift_R:
        return KEY_SHIFT;
    
    // special keys
    case XK_Escape:
        return KEY_ESCAPE;
        break;
    }

    return KEY_NONE;
}

// Convert extended key events to CPUT events
//-----------------------------------------------------------------------------
CPUTKey CPUTWindowX::ConvertSpecialKeyCode(WPARAM wParam, LPARAM lParam)
{
    /*
    UNREFERENCED_PARAMETER(lParam);
    switch(wParam)
    {
    // function keys
    case VK_F1:
        return KEY_F1;
    case VK_F2:
        return KEY_F2;
    case VK_F3:
        return KEY_F3;
    case VK_F4:
        return KEY_F4;
    case VK_F5:
        return KEY_F5;
    case VK_F6:
        return KEY_F6;
    case VK_F7:
        return KEY_F7;
    case VK_F8:
        return KEY_F8;
    case VK_F9:
        return KEY_F9;
    case VK_F10:
        return KEY_F10;
    case VK_F11:
        return KEY_F11;
    case VK_F12:
        return KEY_F12;


    // special keys
    case VK_HOME:
        return KEY_HOME;
    case VK_END:
        return KEY_END;
    case VK_PRIOR:
        return KEY_PAGEUP;
    case VK_NEXT:
        return KEY_PAGEDOWN;
    case VK_INSERT:
        return KEY_INSERT;
    case VK_DELETE:
        return KEY_DELETE;

    case VK_BACK:
        return KEY_BACKSPACE;
    case VK_TAB:
        return KEY_TAB;
    case VK_RETURN:
        return KEY_ENTER;

    case VK_PAUSE:
        return KEY_PAUSE;
    case VK_CAPITAL:
        return KEY_CAPSLOCK;
    case VK_ESCAPE:
        return KEY_ESCAPE;

    case VK_UP:
        return KEY_UP;
    case VK_DOWN:
        return KEY_DOWN;
    case VK_LEFT:
        return KEY_LEFT;
    case VK_RIGHT:
        return KEY_RIGHT;
    }
*/
    return KEY_NONE;
}

// Convert mouse state to CPUT state
//-----------------------------------------------------------------------------
CPUTMouseState CPUTWindowX::ConvertMouseState(XButtonEvent *pMouseEvent)
{
    
    CPUTMouseState eState = CPUT_MOUSE_NONE;
    
    if (pMouseEvent->button == Button1) {
        eState  = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_LEFT_DOWN));
    }
    
    if (pMouseEvent->button == Button2) {
        eState  = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_RIGHT_DOWN));
    }
/*
    if( wParam & MK_CONTROL)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_CTRL_DOWN));

    if( wParam & MK_SHIFT)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_SHIFT_DOWN));

    if( wParam & MK_LBUTTON)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_LEFT_DOWN));

    if( wParam & MK_MBUTTON)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_MIDDLE_DOWN));

    if( wParam & MK_RBUTTON)
        eState = (CPUTMouseState) (eState | static_cast<int>(CPUT_MOUSE_RIGHT_DOWN));
*/

    return eState;
}

void CPUTWindowX::Present()
{
    glXSwapBuffers ( pDisplay, win );
}

// Main message pump
//-----------------------------------------------------------------------------
int CPUTWindowX::StartMessageLoop()
{
    bool fRunning = true;
    XEvent event;
    while (fRunning) {
        if (XPending(pDisplay) > 0) {
            XNextEvent((pDisplay), &event);
            switch(event.type) {
                case KeyPress:
                    if(mCPUT) {
                        CPUTKey key = ConvertKeyCode(&event.xkey);
                        CPUTKeyState state = CPUT_KEY_DOWN;
                        if(key != KEY_NONE) {
                            CPUTEventHandledCode handledCode;
                            handledCode = mCPUT->CPUTHandleKeyboardEvent( key, state );
                        }
                    }
                    break;
                case KeyRelease:
                    if(mCPUT) {
                        CPUTKey key = ConvertKeyCode(&event.xkey);
                        CPUTKeyState state = CPUT_KEY_UP;
                        if(key != KEY_NONE) {
                            CPUTEventHandledCode handledCode;
                            handledCode = mCPUT->CPUTHandleKeyboardEvent( key, state );
                        }
                    }
                    break;
                case CreateNotify:
                    break;
                case DestroyNotify:
                    fRunning = false;
                    break;
                case ClientMessage:
                    if (event.xclient.data.l[0] == wmDeleteMessage) {
                        fRunning = false;
                    }
                    break;
                case ButtonPress:
                if (mCPUT) {
                        CPUTEventHandledCode handledCode;
                        CPUTMouseState state = ConvertMouseState(&event.xbutton);

                        short xPos = event.xbutton.x;
                        short yPos = event.xbutton.y;

                        handledCode = mCPUT->CPUTHandleMouseEvent(xPos, yPos, 0, state);
                    }
                    break;
                case ButtonRelease:
                
                    if (mCPUT) {
                        CPUTEventHandledCode handledCode;
                        CPUTMouseState state = ConvertMouseState(&event.xbutton);
                        state = CPUT_MOUSE_NONE;
                        short xPos = event.xbutton.x;
                        short yPos = event.xbutton.y;

                        handledCode = mCPUT->CPUTHandleMouseEvent(xPos, yPos, 0, state);
                    }
                    break;
                case MotionNotify:
                if (mCPUT) {
                    CPUTMouseState state = CPUT_MOUSE_NONE;
                        CPUTEventHandledCode handledCode;
                     //   CPUTMouseState state = ConvertMouseState(&event.xbutton);
                    if (event.xbutton.state & Button1Mask) {
                        state  = (CPUTMouseState) (state | static_cast<int>(CPUT_MOUSE_LEFT_DOWN));
                    }
    
                    if (event.xbutton.state & Button2Mask) {
                        state  = (CPUTMouseState) (state | static_cast<int>(CPUT_MOUSE_RIGHT_DOWN));
                    }

                        short xPos = event.xbutton.x;
                        short yPos = event.xbutton.y;

                        handledCode = mCPUT->CPUTHandleMouseEvent(xPos, yPos, 0, state);
                    }
                    break;
                default:
                    break;
            }
        } else {
            mCPUT->InnerExecutionLoop();
        }
    }
    
//     XDestroyWindow and XCloseDisplay as the next two commands immediately following your event loop, with an exit(0) af
    /*
	//
	// Message pump
	//
    MSG msg = { 0 };
	bool fRunning = true;
    while(fRunning)
    {
        // PeekMessage() is a passthru on no events
        // so it allows us to render while no events are present
        if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
			if (msg.message == WM_QUIT)
			{
				PostQuitMessage(0);
				fRunning = false;
			}
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        } else
		{
            // trigger render and other calls
            mCPUT->InnerExecutionLoop();
        }
    }
	
	//
	// Drain out the rest of the message queue.
	//
	while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	if (UnregisterClass(mAppTitle.c_str(), mhInst) == 0) {
		HandleWin32Error();
	}

	//
	// Set the window handle to NULL to indicate window shutdown is complete
	//
	mhWnd = NULL;

    // return code
    mAppClosedReturnCode =  (int) msg.wParam;
	return mAppClosedReturnCode;
     * */
     return 0;
}