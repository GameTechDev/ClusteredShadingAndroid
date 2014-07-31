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
#ifndef __CPUT_DX11_H__
#define __CPUT_DX11_H__

#include <stdio.h>

// include base headers we'll need
#include "CPUTWindowWin.h"
#include "CPUT.h"
#include "CPUTMath.h"
#include "CPUTEventHandler.h"
#include "CPUTGuiControllerDX11.h"
#include "CPUTGUIElement.h"

// CPUT objects
#include "CPUTMeshDX11.h"
#include "CPUTModelDX11.h"
#include "CPUTAssetLibraryDX11.h"
#include "CPUTCamera.h"
#include "CPUTLight.h"
#include "CPUTMaterialEffectDX11.h"

// Types of message boxes you can create
enum CPUT_MESSAGE_BOX_TYPE
{
    CPUT_MB_OK = MB_OK | MB_ICONINFORMATION,
    CPUT_MB_ERROR = MB_OK | MB_ICONEXCLAMATION,
    CPUT_MB_WARNING = MB_OK | MB_ICONWARNING
};

// DirectX 11 CPUT layer
//-----------------------------------------------------------------------------
class CPUT_DX11;
extern CPUT_DX11 *gpSample;

class CPUT_DX11:public CPUT
{
protected:
    static ID3D11Device *mpD3dDevice;
    static ID3D11DeviceContext *mpContext;

public:
    static ID3D11Device *GetDevice();
    static ID3D11DeviceContext *GetContext() { return mpContext; }

protected:
    CPUTWindowWin             *mpWindow;
    bool                       mbShutdown;

    D3D_DRIVER_TYPE            mdriverType;
    D3D_FEATURE_LEVEL          mfeatureLevel;
    IDXGISwapChain            *mpSwapChain;
    UINT                       mSwapChainBufferCount;
    ID3D11RenderTargetView    *mpBackBufferRTV;
    ID3D11ShaderResourceView  *mpBackBufferSRV;
    ID3D11UnorderedAccessView *mpBackBufferUAV;
    DXGI_FORMAT                mSwapChainFormat;

    ID3D11Texture2D           *mpDepthStencilBuffer;
    ID3D11DepthStencilState   *mpDepthStencilState;
    ID3D11DepthStencilView    *mpDepthStencilView; // was in protected
    ID3D11ShaderResourceView  *mpDepthStencilSRV;

    UINT                       mSyncInterval; // used for vsync
    CPUTBufferDX11            *mpPerFrameConstantBuffer;
    CPUTBufferDX11            *mpPerModelConstantBuffer;

public:
    CPUT_DX11():mpWindow(NULL),
        mpSwapChain(NULL),
        mSwapChainBufferCount(1),
        mpBackBufferRTV(NULL),
        mpBackBufferSRV(NULL),
        mpBackBufferUAV(NULL),
        mpDepthStencilBuffer(NULL),
        mpDepthStencilState(NULL),
        mpDepthStencilView(NULL),
        mpDepthStencilSRV(NULL),
        mSwapChainFormat(DXGI_FORMAT_UNKNOWN),
        mbShutdown(false),
        mSyncInterval(1),    // start with vsync on
        mpPerFrameConstantBuffer(NULL)
    {
        mpTimer = (CPUTTimer*) new CPUTTimerWin();
        gpSample = this;
    }
    virtual ~CPUT_DX11();

    // context creation/destruction routines
    CPUTResult CPUTParseCommandLine(std::wstring commandLine, CPUTWindowCreationParams *pWindowParams, std::wstring *pFilename, std::wstring *pSceneFilename);
    CPUTResult CPUTParseCommandLine(std::string commandLine, CPUTWindowCreationParams *pWindowParams, std::string *pFilename, std::string *pSceneFilename);
    D3D_FEATURE_LEVEL GetFeatureLevel() { return mfeatureLevel; }

    int CPUTMessageLoop();
    CPUTResult CPUTCreateWindowAndContext(const cString WindowTitle, CPUTWindowCreationParams windowParams);

    // CPUT interfaces
    virtual void ResizeWindow(UINT width, UINT height);
    virtual void ResizeWindowSoft(UINT width, UINT height);
    void DeviceShutdown();
    void RestartCPUT();

    void UpdatePerFrameConstantBuffer( CPUTRenderParameters &renderParams, double totalSeconds );
    void InnerExecutionLoop();

    // events
    virtual void Update(double deltaSeconds) {}
    virtual void Present() { if(mpSwapChain) { mpSwapChain->Present( mSyncInterval, 0 );} }
    virtual void Render(double deltaSeconds) = 0;
    virtual void Create()=0;
    virtual void Shutdown();
    virtual void FullscreenModeChange(bool bFullscreen) {UNREFERENCED_PARAMETER(bFullscreen);}
    virtual void ReleaseSwapChain() {}
    virtual CPUTResult CreateContext();
    

    // GUI
    void CPUTDrawGUI();

    // Event Handling
    CPUTEventHandledCode CPUTHandleKeyboardEvent(CPUTKey key, CPUTKeyState state);
    CPUTEventHandledCode CPUTHandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTMouseMessage message );

    // Utility functions for the sample developer
    CPUTResult CPUTToggleFullScreenMode();
    void CPUTSetFullscreenState(bool bIsFullscreen);
    bool CPUTGetFullscreenState();
    CPUTGuiControllerDX11* CPUTGetGuiController();

    // Message boxes
    void CPUTMessageBox(const cString DialogBoxTitle, const cString DialogMessage);

protected:
    // private helper functions
    bool TestContextForRequiredFeatures();
    void ShutdownAndDestroy();
    virtual CPUTResult CreateDXContext(CPUTContextCreation ContextParams);   // allow user to override DirectX context creation
    virtual CPUTResult DestroyDXContext();  // allow user to override DirectX context destruction
    CPUTResult         MakeWindow(const cString WindowTitle, CPUTWindowCreationParams windowParams);
    CPUTResult         CreateAndBindDepthBuffer(int width, int height);
    void               DrawLoadingFrame();

    // TODO: Put this somewhere else
    bool               FindMatchingInputSlot(const char *pInputSlotName, const ID3DBlob *pVertexShaderBlob);
};

#endif //#ifndef __CPUT_DX11_H__
