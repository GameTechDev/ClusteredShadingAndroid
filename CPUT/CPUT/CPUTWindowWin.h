/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
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
