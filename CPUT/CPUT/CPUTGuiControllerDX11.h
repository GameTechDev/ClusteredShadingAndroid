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
#ifndef __CPUTGUICONTROLLERDX11_H__
#define __CPUTGUICONTROLLERDX11_H__

#include "CPUTGuiController.h"
#include "CPUTTimerWin.h"

#include "CPUTButton.h"
#include "CPUTText.h"
#include "CPUTCheckbox.h"
#include "CPUTSlider.h"
#include "CPUTDropdown.h"
#include "CPUTVertexShaderDX11.h"
#include "CPUTPixelShaderDX11.h"
#include "CPUTRenderStateBlockDX11.h"
#include "CPUTWindowWin.h"
#include "CPUTMeshDX11.h"
//#define SAVE_RESTORE_DS_HS_GS_SHADER_STATE

// forward declarations
class CPUT_DX11;
class CPUTButton;
class CPUTSlider;
class CPUTCheckbox;
class CPUTDropdown;
class CPUTText;
class CPUTTextureDX11;
class CPUTFont;

const unsigned int CPUT_GUI_BUFFER_SIZE = 5000;         // size (in number of verticies) for all GUI control graphics
const unsigned int CPUT_GUI_BUFFER_STRING_SIZE = 5000;  // size (in number of verticies) for all GUI string graphics
const unsigned int CPUT_GUI_VERTEX_BUFFER_SIZE = 5000;
const CPUTControlID ID_CPUT_GUI_FPS_COUNTER = 4000000201;        // pick very random number for FPS counter string ID

#include <d3d11.h>
#include <DirectXMath.h> // for xmmatrix/et al


// the GUI controller class that dispatches the rendering calls to all the buttons
//--------------------------------------------------------------------------------
class CPUTGuiControllerDX11:public CPUTGuiController
{ 
    struct GUIConstantBufferVS
    {
        DirectX::XMMATRIX Projection;
        DirectX::XMMATRIX Model;
    };


public:
    static CPUTGuiControllerDX11 *GetController();
    static CPUTResult DeleteController();

    // initialization
    CPUTResult Initialize(ID3D11DeviceContext *pImmediateContext, cString &ResourceDirectory);
    CPUTResult ReleaseResources();
    

    // Control creation/deletion 'helpers'
    CPUTResult CreateButton(const cString pButtonText, CPUTControlID controlID, CPUTControlID panelID, CPUTButton **ppButton=NULL);
    CPUTResult CreateSlider(const cString pSliderText, CPUTControlID controlID, CPUTControlID panelID, CPUTSlider **ppSlider=NULL, float scale = 1.0f);
    CPUTResult CreateCheckbox(const cString pCheckboxText, CPUTControlID controlID, CPUTControlID panelID, CPUTCheckbox **ppCheckbox=NULL, float scale = 1.0f);
    CPUTResult CreateDropdown(const cString pSelectionText, CPUTControlID controlID, CPUTControlID panelID, CPUTDropdown **ppDropdown=NULL);
    CPUTResult CreateText(const cString Text,  CPUTControlID controlID, CPUTControlID panelID, CPUTText **ppStatic=NULL);    
    CPUTResult DeleteControl(CPUTControlID controlID);

    // draw routines    
    void Draw(ID3D11DeviceContext *pImmediateContext);
    void DrawFPS(bool drawfps);
    float GetFPS();
//    void RecalculateLayout(); // forces a recalculation of control positions


private:
    static CPUTGuiControllerDX11 *mguiController; // singleton object

    CPUTMaterial *mpTextMaterial;
    CPUTMaterial *mpControlMaterial;


    // DirectX state objects for GUI drawing
    CPUTVertexShaderDX11 *mpGUIVertexShader;
    CPUTPixelShaderDX11  *mpGUIPixelShader;
    ID3D11InputLayout    *mpVertexLayout;
    ID3D11Buffer         *mpConstantBufferVS;
    CPUTBufferDX11       *mpConstantBufferVS2;
    GUIConstantBufferVS   mModelViewMatrices;

    // Texture atlas
    CPUTMeshDX11               *mpUberBuffer;
    CPUTGUIVertex              *mpMirrorBuffer;
    UINT                        mUberBufferIndex;
    UINT                        mUberBufferMax;

    // Font atlas
    CPUTMeshDX11               *mpTextUberBuffer;
    CPUTGUIVertex              *mpTextMirrorBuffer;
    UINT                        mTextUberBufferIndex;

    // Focused control buffers
    CPUTGUIVertex              *mpFocusedControlMirrorBuffer;
    UINT                        mFocusedControlBufferIndex;
    CPUTMeshDX11               *mpFocusedControlBuffer;
    CPUTGUIVertex              *mpFocusedControlTextMirrorBuffer;
    UINT                        mFocusedControlTextBufferIndex;
    CPUTMeshDX11               *mpFocusedControlTextBuffer;

    // render state
    CPUTResult UpdateUberBuffers(ID3D11DeviceContext *pImmediateContext );


    // helper functions
    CPUTGuiControllerDX11();    // singleton
    ~CPUTGuiControllerDX11();
    CPUTResult RegisterGUIResources(ID3D11DeviceContext *pImmediateContext, cString VertexShaderFilename, cString RenderStateFile, cString PixelShaderFilename, cString DefaultFontFilename, cString ControlTextureAtlas);
};




#endif // #ifndef __CPUTGUICONTROLLERDX11_H__
