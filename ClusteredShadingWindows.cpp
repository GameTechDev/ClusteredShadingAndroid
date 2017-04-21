/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#include "ClusteredShadingSample.h"

const cString WINDOW_TITLE = _L("CPUTWindow OpenGL 4.0");

// Windows entry point. Calls standard main()
//-----------------------------------------------------------------------------


int main( int argc, char **argv );
#ifdef CPUT_OS_WINDOWS
#include <stdlib.h>
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Prevent unused parameter compiler warnings
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    return main(__argc, __argv);
}
#endif
// Application entry point.  Execution begins here.
//-----------------------------------------------------------------------------
int main( int argc, char **argv )
{
#ifdef DEBUG
    // tell VS to report leaks at any exit of the program
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    //http://msdn.microsoft.com/en-us/library/x98tx3cf%28v=vs.100%29.aspx
	//Add a watch for “{,,msvcr100d.dll}_crtBreakAlloc” to the watch window
	//Set the value of the watch to the memory allocation number reported by your sample at exit.
	//Note that the “msvcr100d.dll” is for MSVC2010.  Other versions of MSVC use different versions of this dl; you’ll need to specify the appropriate version.

#endif
    CPUTResult result=CPUT_SUCCESS;
    int returnCode=0;

    // create an instance of my sample
    ClusteredShadingSample *pSample = new ClusteredShadingSample();

    // window and device parameters
    CPUTWindowCreationParams params;
    params.samples = 1;
    
    // parse out the parameter settings or reset them to defaults if not specified
    std::string AssetFilename;

    result = pSample->CPUTCreateWindowAndContext(WINDOW_TITLE, params);
    ASSERT( CPUTSUCCESS(result), _L("CPUT Error creating window and context.") );

    returnCode = pSample->CPUTMessageLoop();
    pSample->ReleaseResources();
    pSample->DeviceShutdown();

    // cleanup resources
    SAFE_DELETE(pSample);

    return returnCode;
}




// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode ClusteredShadingSample::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    static bool panelToggle = false;
    CPUTEventHandledCode    handled = CPUT_EVENT_UNHANDLED;
    cString fileName;

    switch(key)
    {
    case KEY_ESCAPE:
        handled = CPUT_EVENT_HANDLED;
        DestroyWindow(mpWindow->GetHWnd());
        break;
    case KEY_L:
        {
            static int cameraObjectIndex = 0;
            CPUTRenderNode *pCameraList[] = { mpCamera, mpShadowCamera };
            cameraObjectIndex = (++cameraObjectIndex) % (sizeof(pCameraList)/sizeof(*pCameraList));
            CPUTRenderNode *pCamera = pCameraList[cameraObjectIndex];
            mpCameraController->SetCamera( pCamera );
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    }

    // pass it to the camera controller
    if(handled == CPUT_EVENT_UNHANDLED)
    {
        if (mpCameraController) {
            handled = mpCameraController->HandleKeyboardEvent(key, state);
        }
    }
    return handled;
}
