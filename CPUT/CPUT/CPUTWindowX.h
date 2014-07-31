//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
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
#ifndef CPUTWINDOWX_H
#define CPUTWINDOWX_H

#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "CPUT.h"
#include "CPUTOSServices.h"
//#include "CPUTResource.h" // win resource.h customized for CPUT

 
#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
 
#define HWND int
#define ATOM int
#define LPARAM int
#define WPARAM int
#define HINSTANCE int
#define LRESULT int


// Forward declarations
class CPUT;

// OS-specific window class
//-----------------------------------------------------------------------------
class CPUTWindowX
{
public:
    // construction
    CPUTWindowX();
    ~CPUTWindowX();

    // Creates a graphics-context friendly window
    CPUTResult Create(CPUT* cput, const cString WindowTitle, const int windowWidth, const int windowHeight, int windowX, int windowY);
    CPUTResult GetDimensions(int *pWidth, int *pHeight);
    void GetClientDimensions(int *pX, int *pY, int *pWidth, int *pHeight) {
        *pX = 0; *pY = 0;  GetDimensions(pWidth, pHeight);
    };
    void Present();
      
    // Main windows message loop that handles and dispatches messages
    int StartMessageLoop();
    int Destroy();
    int ReturnCode();

    // return the HWND/Window handle for the created window
    HWND GetHWnd() { return mhWnd;};

protected:
    Display            *pDisplay;
    Window              win;
    Atom                wmDeleteMessage;
    GLXContext          ctx;
    HINSTANCE           mhInst;                    // current instance
    HWND                mhWnd;                     // window handle
    int                 mAppClosedReturnCode;      // windows OS return code
    cString             mAppTitle;                 // title put at top of window
    static CPUT*        mCPUT;                     // CPUT reference for callbacks

    static bool         mbMaxMinFullScreen;

    // Window creation helper functions
    ATOM MyRegisterClass(HINSTANCE hInstance);
    bool InitInstance(int nCmdShow, int windowWidth, int windowHeight, int windowX, int windowY);
    static LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // CPUT conversion helper functions
    static CPUTMouseState ConvertMouseState(XButtonEvent *pMouseEvent);
    static CPUTKey ConvertKeyCode(XKeyEvent *pKeyEvent);
    static CPUTKey ConvertSpecialKeyCode(WPARAM wParam, LPARAM lParam);
};


#endif // CPUTWINDOWX_H
