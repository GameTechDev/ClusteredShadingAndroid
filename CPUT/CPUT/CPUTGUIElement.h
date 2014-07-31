//--------------------------------------------------------------------------------------
// Copyright 2014 Intel Corporation
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
#ifndef __CPUTGUIELEMENT_H__
#define __CPUTGUIELEMENT_H__

#include <cstring>
#include "CPUTEventHandler.h"
#include "CPUTSprite.h"
#include "CPUTRenderNode.h"

// forward declarations
class CPUTButton;
class CPUTSlider;
class CPUTCheckbox;
class CPUTDropdown;
class CPUTText;
class CPUTFont;
#ifdef CPUT_FOR_DX11
class CPUTBufferDX11;
class CPUTMeshDX11;
#else 
class CPUTBufferOGL;
class CPUTMeshOGL;
#endif


struct StringMap
{
    cString key;
    int value;
};

struct GUIConstants
{
    float4x4 wvpMatrix;
    float4 position;
    float4 backgroundcolor;
    float4 foregroundcolor;
    float4 resolution;
    float2 mousePosition;
};
class CPUTGUIElement : public CPUTRenderNode, CPUTEventHandler
{
public:
    //Static helper funcitons for iterating through GUI elements
    static void SetEvents(CPUTRenderNode *pRoot, StringMap *pEventMap, CPUTCallbackHandler *pHandler);
    static CPUTEventHandledCode HandleKeyboardEvent(CPUTRenderNode *pRoot, CPUTKey key, CPUTKeyState state);
    static CPUTEventHandledCode HandleMouseEvent(CPUTRenderNode *pRoot, int x, int y , int wheel, CPUTMouseState state, CPUTEventID message);
    static void Resize(CPUTRenderNode *pRoot, int width, int height);

    CPUTGUIElement* GetNext(CPUTRenderNode* pRoot);
    CPUT_NODE_TYPE GetNodeType() { return CPUT_NODE_GUI_ELEMENT;};

    bool Load(CPUTConfigBlock* pBlock, int *parent);
    void Render(CPUTRenderParameters &params, int materialIndex);
    void RenderRecursive(CPUTRenderParameters &params, int materialIndex);
    void Resize(int x, int y, int width, int height, int windowWidth, int windowHeight);
    bool CreateResources();
    void Enable(bool enable);
    bool Enabled();
    void Visible(bool visible);
    bool Visible();
    void Highlight(bool highlight);
    bool Highlighted();
    
    CPUTGUIElement();
    ~CPUTGUIElement();
    void SetText(cString text);
    //void PositionText();
    //void SetFont(CPUTFont font);
    
private:
    CPUTFont *mpFont;
    CPUTSprite* mpSprite;
    
    bool mParentRelative;
    int mRelX, mRelY;

    int mWidth, mHeight, mPosX, mPosY;
    int mTextWidth, mTextHeight;
    int mTextX, mTextY;
    bool mVisible;
    bool mEnabled;
    bool mHighlighted;
    float4 mForegroundColor;
    float4 mBackgroundColor;
    float4 mForegroundHighlightColor;
    float4 mBackgroundHighlightColor;

    GUIConstants mConstants;
    cString mText;
    CPUTMaterial *mpTextMaterial;
    CPUTMaterial* mpMaterial;

#ifdef CPUT_FOR_DX11
    CPUTMeshDX11 *mpTextMesh;
    CPUTBufferDX11 *mpConstants;
    ID3D11InputLayout* mpVertexLayout;
#else
    CPUTBufferOGL *mpConstants;
    CPUTMeshOGL *mpTextMesh;
#endif
public:
    //Event Handling
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state);
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);
    //implement pure virtual control methods so this object can be passed to the event handler callback
    virtual void GetPosition(int &x, int &y){};
    virtual void SetPosition(int x, int y){};
    virtual void GetDimensions(int &width, int &height){};
    virtual bool ContainsPoint(int x, int y){return false;};
    float2 GetRelativeMousePosition();
    void SetEvents(StringMap eventMap[], CPUTCallbackHandler *pHandler);
    bool UpdateCursor(int x, int y);
private:
    bool mEventHandler;
    float2 mMousePosition;
    CPUTKey mHotkey;
    bool mButtonDown;
    bool mCursorIn;
    StringMap mKeyUp;
    StringMap mKeyDown;
    StringMap mClick;
    StringMap mDown;
    StringMap mUp;
    StringMap mIn;
    StringMap mOut;

    CPUTCallbackHandler *mpHandler;

    
};
#endif //#ifndef __CPUTGUIELEMENT_H__
