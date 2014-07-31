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
#include "CPUTWindowAndroid.h"
#include "CPUT_OGL.h"

#include <unistd.h>

#include <android_native_app_glue.h>

// static initializers
CPUT* CPUTWindowAndroid::mCPUT=NULL;
bool CPUTWindowAndroid::mbMaxMinFullScreen=false;


android_app*    CPUTWindowAndroid::mpAppState = NULL;

// Constructor
//-----------------------------------------------------------------------------
CPUTWindowAndroid::CPUTWindowAndroid()
{
    mWidth = 0;
    mHeight = 0;
}

// Destructor
//-----------------------------------------------------------------------------
CPUTWindowAndroid::~CPUTWindowAndroid()
{
    mWidth = 0;
    mHeight = 0;
}


// Create window
// 
//-----------------------------------------------------------------------------
CPUTResult CPUTWindowAndroid::Create(CPUT* cput, const cString WindowTitle, CPUTWindowCreationParams windowParams)
{
    ASSERT(mpAppState, "mpAppState not set in startup code");
    ASSERT(cput, "cput not set in startup code");

    mCPUT = cput;
    DEBUG_PRINT("CPUTWindowAndroid::Create\n");

    return CPUT_SUCCESS;
}



// Destroy window
//-----------------------------------------------------------------------------
int CPUTWindowAndroid::Destroy()
{
    
    mCPUT = NULL;
    
    DEBUG_PRINT("CPUTWindowAndroid::Destroy\n");

    return true;
}

NativeWindowType CPUTWindowAndroid::GetHWnd()
{ 
	return mpAppState->window;
}




// start main message loop
//-----------------------------------------------------------------------------
int CPUTWindowAndroid::StartMessageLoop()
{
    // We might not have a window yet, so can't call it to start the message loop
    // It must be done here.

	 DEBUG_PRINT("CPUTWindowAndroid::StartMessageLoop\n");
    // We can create event queues here if we are interested in Sensors etc.
    // They would drop out of the source->process code and require special processing
    while (1)
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
//        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) 
#warning "Need to not poll if we don't have focus"
        while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) 
        {
            // Process this event.
            if (source != NULL) 
            {
                source->process(mpAppState, source);
            }
            
            if (mpAppState->destroyRequested)
            {
                DEBUG_PRINT("Destroy Requested");
                return 0;
            }
        }
        
		if(mCPUT)
			mCPUT->InnerExecutionLoop();
    }
}