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
#ifndef __CPUTGUICONTROLLEROGL_H__
#define __CPUTGUICONTROLLEROGL_H__

#include "CPUTGuiController.h"
//#include "CPUTTimerWin.h"

#include "CPUTButton.h"
#include "CPUTText.h"
#include "CPUTCheckbox.h"
#include "CPUTSlider.h"
#include "CPUTDropdown.h"
#include "CPUTMaterialEffectOGL.h"
//#include "CPUTWindowWin.h"
#include "CPUTMath.h"
#include "CPUTBufferOGL.h"

//#define SAVE_RESTORE_DS_HS_GS_SHADER_STATE

// forward declarations
class CPUT_OGL;
class CPUTButton;
class CPUTSlider;
class CPUTCheckbox;
class CPUTDropdown;
class CPUTText;
class CPUTTextureOGL;

const unsigned int CPUT_GUI_BUFFER_SIZE = 5000;         // size (in number of verticies) for all GUI control graphics
const unsigned int CPUT_GUI_BUFFER_STRING_SIZE = 5000;  // size (in number of verticies) for all GUI string graphics
const unsigned int CPUT_GUI_VERTEX_BUFFER_SIZE = 5000;
const CPUTControlID ID_CPUT_GUI_FPS_COUNTER = 400000201;        // pick very random number for FPS counter string ID


// the GUI controller class that dispatches the rendering calls to all the buttons
//--------------------------------------------------------------------------------
class CPUTGuiControllerOGL:public CPUTGuiController
{ 
    struct GUIConstantBufferVS
    {
        float4x4 Projection;
        float4x4 Model;
    };


public:
    static CPUTGuiControllerOGL *GetController();
    static CPUTResult DeleteController();

    // initialization
    CPUTResult Initialize(cString &ResourceDirectory);
    CPUTResult ReleaseResources();
    

    // Control creation/deletion 'helpers'
    CPUTResult CreateButton(const cString pButtonText, CPUTControlID controlID, CPUTControlID panelID, CPUTButton **ppButton=NULL);
    CPUTResult CreateSlider(const cString pSliderText, CPUTControlID controlID, CPUTControlID panelID, CPUTSlider **ppSlider=NULL, float scale = 1.0f);
    CPUTResult CreateCheckbox(const cString pCheckboxText, CPUTControlID controlID, CPUTControlID panelID, CPUTCheckbox **ppCheckbox=NULL, float scale = 1.0f);
    CPUTResult CreateDropdown(const cString pSelectionText, CPUTControlID controlID, CPUTControlID panelID, CPUTDropdown **ppDropdown=NULL);
    CPUTResult CreateText(const cString Text,  CPUTControlID controlID, CPUTControlID panelID, CPUTText **ppStatic=NULL);    
    CPUTResult DeleteControl(CPUTControlID controlID);

    // draw routines    
    void  Draw();
    void  DrawFPS(bool drawfps);
    float GetFPS();
    void  RecalculateLayout(); // forces a recalculation of control positions



private:
    static CPUTGuiControllerOGL *mguiController; // singleton object

    CPUTMaterial *mpTextMaterial;
    CPUTMaterial *mpControlMaterial;

    CPUTBufferOGL        *mpConstantBufferVS;
    GUIConstantBufferVS   mModelViewMatrices;

    CPUTMeshOGL                *mpUberBuffer;
    CPUTGUIVertex              *mpMirrorBuffer;
    UINT                        mUberBufferIndex;
    UINT                        mUberBufferMax;


    // Font atlas
    CPUTMeshOGL                *mpTextUberBuffer;
    CPUTGUIVertex              *mpTextMirrorBuffer;
    UINT                        mTextUberBufferIndex;

    // Focused control buffers
    CPUTGUIVertex              *mpFocusedControlMirrorBuffer;
    UINT                        mFocusedControlBufferIndex;
    CPUTMeshOGL                *mpFocusedControlBuffer;
    CPUTGUIVertex              *mpFocusedControlTextMirrorBuffer;
    UINT                        mFocusedControlTextBufferIndex;
    CPUTMeshOGL                *mpFocusedControlTextBuffer;


    /*
    // FPS
    bool                        mbDrawFPS;
    float                       mLastFPS;
    CPUTText                   *mpFPSCounter;
    // FPS control buffers
    CPUTGUIVertex              *mpFPSMirrorBuffer;
    UINT                        mFPSBufferIndex;
    CPUTBufferOGL              *mpFPSDirectXBuffer;
    CPUTTimerWin               *mpFPSTimer;

    // render state
    //CPUTRenderStateBlockOGL   *mpGUIRenderStateBlock;
    */
    CPUTResult UpdateUberBuffers();

    // helper functions
    CPUTGuiControllerOGL();    // singleton
    ~CPUTGuiControllerOGL();
    CPUTResult RegisterGUIResources();
};

#endif // #ifndef __CPUTGUICONTROLLEROGL_H__
