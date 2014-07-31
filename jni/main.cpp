/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include "ClusteredShadingSample.h"

#define GUI_LOCATION     "gui_assets/"
#define SYSTEM_LOCATION  "System/"

#define TEST_HOTKEY         50001
#define TEST_DOWN           50002
#define TEST_UP             50003
#define TEST_CLICK          50004
#define TEST_KEY_DOWN       50005
#define TEST_KEY_UP         50006
#define TEST_CLICK_2        50007
#define TEST_RADIO_1        50008
#define TEST_RADIO_2        50009
#define TEST_RADIO_3        50010
#define TEST_CLOSE_RADIO    50011
#define TEST_SHOW_RADIO     50012
#define SLIDER              50013
#define SLIDER_OFF          50015

StringMap EventMap[] = {
    { cString(_L("increment_slider")),  SLIDER},
    { cString(_L("slider_off")),        SLIDER_OFF}, 
    { cString(_L("test_mouse_click")),  TEST_CLICK},
    { cString(_L("test_mouse_down")),   TEST_DOWN},
    { cString(_L("test_mouse_up")),     TEST_UP},
    { cString(_L("test_key_down")),     TEST_KEY_DOWN},
    { cString(_L("test_key_up")),       TEST_KEY_UP},
    { cString(_L("test_mouse_click_2")),TEST_CLICK_2},
    { cString(_L("test_radio_1")),      TEST_RADIO_1},
    { cString(_L("test_radio_2")),      TEST_RADIO_2},
    { cString(_L("test_radio_3")),      TEST_RADIO_3},
    { cString(_L("test_close_radio")),  TEST_CLOSE_RADIO},
    { cString(_L("test_show_radio")),   TEST_SHOW_RADIO},
    { cString(_L("")), -1},
};


CPUTEventHandledCode ClusteredShadingSample :: HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    if (mpCameraController)
    {
        return mpCameraController->HandleKeyboardEvent(key, state);
    }
    return CPUT_EVENT_UNHANDLED;
}

CPUTKey ConvertToCPUTKey(int aKey)
{
    if ((aKey >= AKEYCODE_0) && (aKey <= AKEYCODE_9))
        return (CPUTKey)(KEY_0 + aKey - AKEYCODE_0);

    if ((aKey >= AKEYCODE_A) && (aKey <= AKEYCODE_Z))
        return (CPUTKey)(KEY_A + aKey - AKEYCODE_A);

    switch (aKey)
    {
    case AKEYCODE_HOME:
        return KEY_HOME;
    case AKEYCODE_STAR:
        return KEY_STAR;
    case AKEYCODE_POUND:
        return KEY_HASH;
    case AKEYCODE_COMMA:
        return KEY_COMMA;
    case AKEYCODE_PERIOD:
        return KEY_PERIOD;
    case AKEYCODE_ALT_LEFT:
        return KEY_LEFT_ALT;
    case AKEYCODE_ALT_RIGHT:
        return KEY_RIGHT_ALT;
    case AKEYCODE_SHIFT_LEFT:
        return KEY_LEFT_SHIFT;
    case AKEYCODE_SHIFT_RIGHT:
        return KEY_RIGHT_SHIFT;
    case AKEYCODE_TAB:
        return KEY_TAB;
    case AKEYCODE_SPACE:
        return KEY_SPACE;
    case AKEYCODE_ENTER:
        return KEY_ENTER;
    case AKEYCODE_DEL:
        return KEY_DELETE;
    case AKEYCODE_MINUS:
        return KEY_MINUS;
    case AKEYCODE_LEFT_BRACKET:
        return KEY_OPENBRACKET;
    case AKEYCODE_RIGHT_BRACKET:
        return KEY_CLOSEBRACKET;
    case AKEYCODE_BACKSLASH:
        return KEY_BACKSLASH;
    case AKEYCODE_SEMICOLON:
        return KEY_SEMICOLON;
    case AKEYCODE_APOSTROPHE:
        return KEY_SINGLEQUOTE;
    case AKEYCODE_SLASH:
        return KEY_SLASH;
    case AKEYCODE_AT:
        return KEY_AT;
    case AKEYCODE_PLUS:
        return KEY_PLUS;
    case AKEYCODE_PAGE_UP:
        return KEY_PAGEUP;
    case AKEYCODE_PAGE_DOWN:
        return KEY_PAGEDOWN;
    default:
    case AKEYCODE_SOFT_LEFT:
    case AKEYCODE_SOFT_RIGHT:
    case AKEYCODE_BACK:
    case AKEYCODE_CALL:
    case AKEYCODE_ENDCALL:
    case AKEYCODE_DPAD_UP:
    case AKEYCODE_DPAD_DOWN:
    case AKEYCODE_DPAD_LEFT:
    case AKEYCODE_DPAD_RIGHT:
    case AKEYCODE_DPAD_CENTER:
    case AKEYCODE_VOLUME_UP:
    case AKEYCODE_VOLUME_DOWN:
    case AKEYCODE_POWER:
    case AKEYCODE_CAMERA:
    case AKEYCODE_CLEAR:
    case AKEYCODE_SYM:
    case AKEYCODE_EXPLORER:
    case AKEYCODE_ENVELOPE:
    case AKEYCODE_GRAVE:
    case AKEYCODE_EQUALS:
    case AKEYCODE_NUM:
    case AKEYCODE_HEADSETHOOK:
    case AKEYCODE_FOCUS:
    case AKEYCODE_MENU:
    case AKEYCODE_NOTIFICATION:
    case AKEYCODE_SEARCH:
    case AKEYCODE_MEDIA_PLAY_PAUSE:
    case AKEYCODE_MEDIA_STOP:
    case AKEYCODE_MEDIA_NEXT:
    case AKEYCODE_MEDIA_PREVIOUS:
    case AKEYCODE_MEDIA_REWIND:
    case AKEYCODE_MEDIA_FAST_FORWARD:
    case AKEYCODE_MUTE:
    case AKEYCODE_PICTSYMBOLS:
    case AKEYCODE_SWITCH_CHARSET:
    case AKEYCODE_BUTTON_L1:
    case AKEYCODE_BUTTON_R1:
    case AKEYCODE_BUTTON_L2:
    case AKEYCODE_BUTTON_R2:
    case AKEYCODE_BUTTON_THUMBL:
    case AKEYCODE_BUTTON_THUMBR:
    case AKEYCODE_BUTTON_START:
    case AKEYCODE_BUTTON_SELECT:
    case AKEYCODE_BUTTON_MODE:
    case AKEYCODE_UNKNOWN:
        return KEY_NONE;
    }

}

CPUTKeyState ConvertToCPUTKeyState(int aAction)
{
    switch (aAction)
    {
    case AKEY_EVENT_ACTION_UP:
        return CPUT_KEY_UP;
    case AKEY_EVENT_ACTION_DOWN:
    default:
        return CPUT_KEY_DOWN;
    }
}

int32_t CPUT_OGL::cput_handle_input(struct android_app* app, AInputEvent* event)
{    
	int n;
    ClusteredShadingSample *pSample = (ClusteredShadingSample *)app->userData;
	int lEventType = AInputEvent_getType(event);
    static float drag_center_x = 0.0f, drag_center_y = 0.0f;
    static float dist_squared = 0.0f;
    static bool isPanning = false;
    
    static ndk_helper::Vec2 sLastMouse;

    switch (lEventType) 
	{
        case AINPUT_EVENT_TYPE_MOTION:
			{
                ndk_helper::GESTURE_STATE doubleTapState = pSample->mDoubletapDetector.Detect(event);
                ndk_helper::GESTURE_STATE dragState      = pSample->mDragDetector.Detect(event);
                ndk_helper::GESTURE_STATE pinchState     = pSample->mPinchDetector.Detect(event);

                if( doubleTapState == ndk_helper::GESTURE_STATE_ACTION )
                {
                    LOGI("DOUBLE TAP RECEIVED");
                }
                else
                {
                    //Handle drag state
                    if( dragState & ndk_helper::GESTURE_STATE_START )
                    {
                        if (isPanning == false) {
                            LOGI("GESTURE_STATE_START - drag");
                            pSample->mDragDetector.GetPointer( sLastMouse );
                            float x, y;
                            sLastMouse.Value(x, y);
                            LOGI("     TOUCH POINT: %f, %f", x, y);
                            pSample->CPUTHandleMouseEvent(x, y, 0.0f, CPUT_MOUSE_LEFT_DOWN, CPUT_EVENT_DOWN);
                        }
                    }
                    else if( dragState & ndk_helper::GESTURE_STATE_MOVE )
                    {
                        if (isPanning == false) {
                            LOGI("GESTURE_STATE_MOVE - drag");
                            float x, y;
                            pSample->mDragDetector.GetPointer( sLastMouse );
                            sLastMouse.Value(x, y);
                            pSample->CPUTHandleMouseEvent(x, y, 0.0f, CPUT_MOUSE_LEFT_DOWN, CPUT_EVENT_MOVE);
                        }
                    }
                    else if( dragState & ndk_helper::GESTURE_STATE_END )
                    {
                        float x, y;
                        sLastMouse.Value(x, y);
                        pSample->CPUTHandleMouseEvent(x, y, 0.0f, CPUT_MOUSE_NONE, CPUT_EVENT_UP);
                        pSample->HandleKeyboardEvent(KEY_A, CPUT_KEY_UP);
                        pSample->HandleKeyboardEvent(KEY_D, CPUT_KEY_UP);
                        pSample->HandleKeyboardEvent(KEY_E, CPUT_KEY_UP);
                        pSample->HandleKeyboardEvent(KEY_W, CPUT_KEY_UP);
                        pSample->HandleKeyboardEvent(KEY_S, CPUT_KEY_UP);
                        pSample->HandleKeyboardEvent(KEY_Q, CPUT_KEY_UP);
                        isPanning = false;
                        sLastMouse = ndk_helper::Vec2(-1.0, -1.0);
                        LOGI("GESTURE_STATE_END - drag");
                    }

                    //Handle pinch state
                    if( pinchState & ndk_helper::GESTURE_STATE_START )
                    {
                        if (isPanning == false) {
                            LOGI("GESTURE_STATE_START - pinch");
                            //Start new pinch
                            ndk_helper::Vec2 v1;
                            ndk_helper::Vec2 v2;
                            float x1, y1, x2, y2;
                            pSample->mPinchDetector.GetPointers( v1, v2 );
                            v1.Value(x1, y1);
                            v2.Value(x2, y2);
                            drag_center_x = (x1 + x2) / 2.0f;
                            drag_center_y = (y1 + y2) / 2.0f;
                            dist_squared = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));
                        }
                   }
                    else if( pinchState & ndk_helper::GESTURE_STATE_MOVE )
                    {
                        isPanning = true;

                        CPUTKey key = (CPUTKey)0;
                        CPUTKeyState state = (CPUTKeyState)0;

                        LOGI("GESTURE_STATE_MOVE - pinch");
                        
                        ndk_helper::Vec2 v1;
                        ndk_helper::Vec2 v2;
                        float x1, y1, x2, y2;
                        float new_center_x, new_center_y;
                        float new_dist_squared;
                        float delta_x, delta_y;
                        pSample->mPinchDetector.GetPointers( v1, v2 );
                        v1.Value(x1, y1);
                        v2.Value(x2, y2);
                            
                        new_center_x = (x1 + x2) / 2.0f;
                        new_center_y = (y1 + y2) / 2.0f;

                        new_dist_squared = ((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1));

                        delta_x = drag_center_x - new_center_x;
                        delta_y = drag_center_y - new_center_y;

                        //
                        // For each direction of movement, the opposite direction is cancelled (KEY_UP)
                        //

                        // Handle pinch and zoom actions
                        if (abs(new_dist_squared - dist_squared) > 1000.0f) {
                            if (new_dist_squared < dist_squared) {
                                pSample->HandleKeyboardEvent(KEY_S, CPUT_KEY_UP);
                                pSample->HandleKeyboardEvent(KEY_W, CPUT_KEY_DOWN);
                            } else {
                                pSample->HandleKeyboardEvent(KEY_W, CPUT_KEY_UP);
                                pSample->HandleKeyboardEvent(KEY_S, CPUT_KEY_DOWN);
                            } 
                        } else {
                            pSample->HandleKeyboardEvent(KEY_W, CPUT_KEY_UP);
                            pSample->HandleKeyboardEvent(KEY_S, CPUT_KEY_UP);
                        }

                        // handle left and right drag
                        if (delta_x >= 2.0f) {
                            pSample->HandleKeyboardEvent(KEY_A, CPUT_KEY_DOWN);
                            pSample->HandleKeyboardEvent(KEY_D, CPUT_KEY_UP);
                        } else if (delta_x <= -2.0f) {
                            pSample->HandleKeyboardEvent(KEY_D, CPUT_KEY_DOWN);
                            pSample->HandleKeyboardEvent(KEY_A, CPUT_KEY_UP);
                        } else if (delta_x < 2.0 && delta_x > -2.0) {
                            pSample->HandleKeyboardEvent(KEY_A, CPUT_KEY_UP);
                            pSample->HandleKeyboardEvent(KEY_D, CPUT_KEY_UP);
                        }

                        // handle up and down drag
                        if (delta_y >= 2.0f) {
                            pSample->HandleKeyboardEvent(KEY_Q, CPUT_KEY_UP);
                            pSample->HandleKeyboardEvent(KEY_E, CPUT_KEY_DOWN);
                        } else if (delta_y <= -2.0f) {
                            pSample->HandleKeyboardEvent(KEY_E, CPUT_KEY_UP);
                            pSample->HandleKeyboardEvent(KEY_Q, CPUT_KEY_DOWN);
                        } else if (delta_y < 2.0 && delta_y > -2.0) {
                            pSample->HandleKeyboardEvent(KEY_E, CPUT_KEY_UP);
                            pSample->HandleKeyboardEvent(KEY_Q, CPUT_KEY_UP);
                        }

                        // current values become old values for next frame
                        dist_squared = new_dist_squared;
                        drag_center_x = new_center_x;
                        drag_center_y = new_center_y;
                    }
                }
                
			}
		case AINPUT_EVENT_TYPE_KEY:
			{
				int aKey = AKeyEvent_getKeyCode(event);
				CPUTKey cputKey = ConvertToCPUTKey(aKey);
				int aAction = AKeyEvent_getAction(event);
				CPUTKeyState cputKeyState = ConvertToCPUTKeyState(aAction);
				pSample->CPUTHandleKeyboardEvent(cputKey, cputKeyState);
				return 1;
			}
		default:
			return 0;
    }
    
    return 0;
}
static void cput_handle_cmd(struct android_app* app, int32_t cmd)
{
    ClusteredShadingSample *pSample = (ClusteredShadingSample *)app->userData;

	switch (cmd)
    {
    case APP_CMD_SAVE_STATE:
        LOGI("APP_CMD_SAVE_STATE");
        break;
    case APP_CMD_INIT_WINDOW:
        LOGI("APP_CMD_INIT_WINDOW");
        if (!pSample->HasWindow())
        {
            LOGI("Creating window");
            CPUTResult result;

            // window and device parameters
            CPUTWindowCreationParams params;
            params.samples = 1;

            // create the window and device context
            result = pSample->CPUTCreateWindowAndContext(_L("CPUTWindow OpenGLES"), params);
            if (result != CPUT_SUCCESS)
                LOGI("Unable to create window");
        }
        else
        {
            LOGI("Window already created");
        }
        break;
    case APP_CMD_TERM_WINDOW:
		exit(0);
        LOGI("APP_CMD_TERM_WINDOW");
        // Need clear window create and destroy calls
        // The window is being hidden or closed, clean it up.
        if (pSample->HasWindow())
        {
            pSample->DeviceShutdown();
        }
        break;
    case APP_CMD_GAINED_FOCUS:
        LOGI("APP_CMD_GAINED_FOCUS");
        break;
    case APP_CMD_LOST_FOCUS:
        LOGI("APP_CMD_LOST_FOCUS");
        break;
	case APP_CMD_WINDOW_RESIZED:
        LOGI("APP_CMD_WINDOW_RESIZED");
        break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state)
{
    // Make sure glue isn't stripped.
    app_dummy();

    // create an instance of my sample
    ClusteredShadingSample *pSample = new ClusteredShadingSample();
    if (!pSample)
    {
        LOGI("Failed to allocate ClusteredShadingSample");
        return;
    }

     // Assign the sample back into the app state
    state->userData = pSample;
    state->onAppCmd = cput_handle_cmd;
    state->onInputEvent = CPUT_OGL::cput_handle_input;

	// Initialize the system and give it the base CPUT resource directory (location of GUI images/etc)
    // For now, we assume it's a relative directory from the executable directory.  Might make that resource
    // directory location an env variable/hardcoded later
    CPUTWindowAndroid::SetAppState(state);

    // start the main message loop
    pSample->CPUTMessageLoop();

    // cleanup resources
    SAFE_DELETE(pSample);
    pSample = NULL;

    state->userData = NULL;
}


//END_INCLUDE(all)
