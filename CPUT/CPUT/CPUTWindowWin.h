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
#ifndef __WINDOWWIN_H__
#define __WINDOWWIN_H__

#include "CPUT.h"
#if defined( CPUT_FOR_OGL ) || defined( CPUT_FOR_OGLES )
#include <vector>
#include <string>
using namespace std;
#endif
#include "CPUTOSServices.h"
#include "CPUTResource.h" // win resource.h customized for CPUT
#include "CPUTWindow.h"

#include <windows.h>
#include <winuser.h> // for character codes
#include <cstringt.h> // for CString class
#include <atlstr.h> // CString class

// Forward declarations
class CPUT;

// OS-specific window class
//-----------------------------------------------------------------------------
class CPUTWindowWin : public CPUTWindow
{
public:
    // construction
    CPUTWindowWin();
    ~CPUTWindowWin();

    // Creates a graphics-context friendly window
    CPUTResult Create(CPUT* cput, const cString WindowTitle, CPUTWindowCreationParams windowParams);

    // Main windows message loop that handles and dispatches messages
    int StartMessageLoop();
    int Destroy();
    int ReturnCode();

    // return the HWND/Window handle for the created window
    HWND GetHWnd() { return mhWnd;};

    // screen/window dimensions
    void GetClientDimensions( int *pWidth, int *pHeight);
    void GetClientDimensions( int *pX, int *pY, int *pWidth, int *pHeight);
    void GetDesktopDimensions(int *pWidth, int *pHeight);
    bool IsWindowMaximized();
    bool IsWindowMinimized();
    bool DoesWindowHaveFocus();

    // Mouse capture - 'binds'/releases all mouse input to this window
    virtual void CaptureMouse();
    virtual void ReleaseMouse();

protected:

//#ifdef CPUT_OS_ANDROID
//    ANativeWindow*      window;                    // Native Activity window
//#endif
//#ifdef CPUT_OS_WINDOWS
    HINSTANCE           mhInst;                    // current instance
    HWND                mhWnd;                     // window handle

//#endif
    bool                fullscreen;                // Is in fullscreen mode?
    int                 mAppClosedReturnCode;      // windows OS return code
    cString             mAppTitle;                 // title put at top of window
    static CPUT*        mCPUT;                     // CPUT reference for callbacks

    static bool         mbMaxMinFullScreen;

    // All this could be moved to different context implementations

    // Window creation helper functions
    ATOM MyRegisterClass(HINSTANCE hInstance);
    BOOL InitInstance(int nCmdShow, int windowWidth, int windowHeight, int windowX, int windowY);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // CPUT conversion helper functions
    static CPUTMouseState ConvertMouseState(WPARAM wParam);
	static CPUTKey ConvertVirtualKeyToCPUTKey(WPARAM wParam);
    static CPUTKey ConvertCharacterToCPUTKey(WPARAM wParam);
};


#endif //#ifndef __WINDOWWIN_H__
