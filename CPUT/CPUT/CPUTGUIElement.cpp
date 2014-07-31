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
#include "CPUTGUIElement.h"
#include "CPUTFont.h"
#ifdef CPUT_FOR_DX11
#include "CPUTBufferDX11.h"
#include "CPUTRenderParamsDX.h"
#else
#include "CPUTBufferOGL.h"
#endif

CPUTKey MapKey(cString str);

const cString WIDTH = _L("width");
const cString HEIGHT = _L("height");
const cString POS_X = _L("posX");
const cString POS_Y = _L("posY");
const cString PARENT_RELATIVE = _L("parentrelative");
const cString ASSET_TYPE = _L("type");
const cString TYPE_TEXT = _L("text");
const cString MATERIAL = _L("material");
const cString TEXT_MATERIAL = _L("textmaterial");
const cString PARENT = _L("parent");
const cString NAME = _L("name");
const cString FOREGROUND_COLOR = _L("foregroundcolor");
const cString BACKGROUND_COLOR = _L("backgroundcolor");
const cString FOREGROUND_HIGHLIGHT_COLOR = _L("foregroundhighlightcolor");
const cString BACKGROUND_HIGHLIGHT_COLOR = _L("backgroundhighlightcolor");
const cString GUI_CONSTANT_BUFFER = _L("$cbGUIConstants");
const cString TEXT = _L("text");
const cString HIGHLIGHT = _L("highlight");
const cString MOUSE_CLICK = _L("click");
const cString MOUSE_UP = _L("up");
const cString MOUSE_DOWN = _L("down");
const cString MOUSE_IN = _L("in");
const cString MOUSE_OUT = _L("out");
const cString HOTKEY = _L("hotkey");
const cString HOTKEY_DOWN = _L("keydown");
const cString HOTKEY_UP = _L("keyup");

const cString VISIBLE = _L("visible");

CPUTGUIElement::CPUTGUIElement() :
    mpFont(NULL),
    mpSprite(NULL),
    mParentRelative(true),
    mRelX(0), mRelY(0),
    mWidth(0), mHeight(0),
    mPosX(0), mPosY(0),
    mTextX(0), mTextY(0),
    mVisible(true),
    mEnabled(true),
    mHighlighted(false),
    mForegroundColor(1.0),
    mBackgroundColor(0.0),
    mForegroundHighlightColor(0.0),
    mBackgroundHighlightColor(1.0),
    mConstants(),
    mpConstants(NULL),
    mpTextMesh(NULL),
    mpTextMaterial(NULL),
    mpMaterial(NULL),
    mEventHandler(false),
    mButtonDown(false),
    mCursorIn(false),
    mpHandler(NULL)
{
#ifdef CPUT_FOR_DX11
    mpConstants = (CPUTBufferDX11*)CPUTAssetLibrary::GetAssetLibrary()->GetConstantBuffer(GUI_CONSTANT_BUFFER);
#else
    mpConstants = (CPUTBufferOGL*)CPUTAssetLibrary::GetAssetLibrary()->GetConstantBuffer(GUI_CONSTANT_BUFFER);
#endif
}
CPUTGUIElement::~CPUTGUIElement()
{
    SAFE_RELEASE(mpFont);
    SAFE_DELETE(mpSprite);
    SAFE_RELEASE(mpConstants);
    SAFE_DELETE(mpTextMesh);
    SAFE_RELEASE(mpTextMaterial);
    SAFE_RELEASE(mpMaterial);
}

void CPUTGUIElement::Enable(bool enable)
{
    mEnabled = enable;
}
bool CPUTGUIElement::Enabled()
{
    return mEnabled;
}

void CPUTGUIElement::Visible(bool visible)
{
    mVisible = visible;
}
bool CPUTGUIElement::Visible()
{
    return mVisible;
}

void CPUTGUIElement::Highlight(bool highlight)
{
    mHighlighted = highlight;
}
bool CPUTGUIElement::Highlighted()
{
    return mHighlighted;
}

bool CPUTGUIElement::Load(CPUTConfigBlock* pBlock, int* pParent)
{
    CPUTConfigEntry* pEntry = NULL;

    pEntry = pBlock->GetValueByName(NAME);
    if (pEntry->IsValid())
        mName= pEntry->ValueAsString();

    pEntry = pBlock->GetValueByName(PARENT);
    if(pEntry->IsValid())
        *pParent = pEntry->ValueAsInt();

    pEntry = pBlock->GetValueByName(WIDTH);
    if(pEntry->IsValid())
        mWidth = pEntry->ValueAsInt();

    pEntry = pBlock->GetValueByName(HEIGHT);
    if(pEntry->IsValid())
        mHeight = pEntry->ValueAsInt();
    
    pEntry = pBlock->GetValueByName(POS_X);
    if(pEntry->IsValid())
        mRelX = pEntry->ValueAsInt();

    pEntry = pBlock->GetValueByName(POS_Y);
    if(pEntry->IsValid())
        mRelY = pEntry->ValueAsInt();
    
    pEntry = pBlock->GetValueByName(PARENT_RELATIVE);
    if(pEntry->IsValid())
        mParentRelative = pEntry->ValueAsBool();

    pEntry = pBlock->GetValueByName(VISIBLE);
    if(pEntry->IsValid())
        mVisible = pEntry->ValueAsBool();

    pEntry = pBlock->GetValueByName(FOREGROUND_COLOR);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsFloatArray((float*)&mForegroundColor, 4);
        mForegroundHighlightColor = mForegroundColor;
    }

    pEntry = pBlock->GetValueByName(BACKGROUND_COLOR);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsFloatArray((float*)&mBackgroundColor, 4);
        mBackgroundHighlightColor = mBackgroundColor;
    }
    pEntry = pBlock->GetValueByName(FOREGROUND_HIGHLIGHT_COLOR);
    if(pEntry->IsValid())
        pEntry->ValueAsFloatArray((float*)&mForegroundHighlightColor, 4);

    pEntry = pBlock->GetValueByName(BACKGROUND_HIGHLIGHT_COLOR);
    if(pEntry->IsValid())
        pEntry->ValueAsFloatArray((float*)&mBackgroundHighlightColor, 4);

    pEntry = pBlock->GetValueByName(MATERIAL);
    if(pEntry->IsValid())
    {
        cString materialName = pEntry->ValueAsString();
        mpSprite = CPUTSprite::CreateSprite(0.0, 0.0, 1.0, 1.0, materialName);
    }

    pEntry = pBlock->GetValueByName(TEXT_MATERIAL);
    if(pEntry->IsValid())
    {
        cString materialName = pEntry->ValueAsString();
        mpTextMaterial = CPUTAssetLibrary::GetAssetLibrary()->GetMaterial(materialName);
    }
    const cString FONT = _L("font");
    pEntry = pBlock->GetValueByName(FONT);
    if(pEntry->IsValid())
    {
        cString fontName;
        pEntry->ValueAsString(&fontName);
        mpFont = (CPUTFont*)CPUTAssetLibrary::GetAssetLibrary()->GetFontByName(fontName);
        if(mpFont == NULL)
        {
            DEBUG_PRINT(_L("Failed to load font: %s"), fontName.c_str());
        }
    }
    pEntry = pBlock->GetValueByName(TEXT);
    cString string;
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&string);
        SetText(string);
    }   

    pEntry = pBlock->GetValueByName(HIGHLIGHT);
    if(pEntry->IsValid())
    {
        mHighlighted = pEntry->ValueAsBool();
    }   

    pEntry = pBlock->GetValueByName(MOUSE_CLICK);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mClick.key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(MOUSE_UP);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mUp.key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(MOUSE_DOWN);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mDown.key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(MOUSE_OUT);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mOut.key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(MOUSE_IN);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mIn.key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(HOTKEY);
    if(pEntry->IsValid())
    {
        cString key;
        pEntry->ValueAsString(&key);
        mHotkey = MapKey(key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(HOTKEY_UP);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mKeyUp.key);
        mEventHandler = true;
    }
    pEntry = pBlock->GetValueByName(HOTKEY_DOWN);
    if(pEntry->IsValid())
    {
        pEntry->ValueAsString(&mKeyDown.key);
        mEventHandler = true;
    }

    mParentMatrix = float4x4Translation((float)mPosX, (float)mPosY, 0.0);
    return true;
}
int GetEventValue(StringMap eventMap[], cString event)
{
    int i = 0; 
    while(eventMap[i].value > 0)
    {
        if(event.compare(eventMap[i].key) == 0)
        {
            DEBUG_PRINT(_L("Found Event: %s\n"), event.c_str());
            return eventMap[i].value;

        }
        i++;
    }
    return -1;
}
void CPUTGUIElement::SetEvents(StringMap eventMap[], CPUTCallbackHandler *pHandler)
{
    if(mEventHandler)
    {
        mKeyDown.value = GetEventValue(eventMap, mKeyDown.key);
        mKeyUp.value = GetEventValue(eventMap, mKeyUp.key);
        mClick.value = GetEventValue(eventMap, mClick.key);
        mDown.value = GetEventValue(eventMap, mDown.key);
        mUp.value = GetEventValue(eventMap, mUp.key);
        mIn.value = GetEventValue(eventMap, mIn.key);
        mOut.value = GetEventValue(eventMap, mOut.key);
        mpHandler = pHandler;
    }
}


void CPUTGUIElement::SetText(cString string)
{
    DEBUG_PRINT(_L("GUIElement SetText: %s"), string.c_str());
    mText = string;
    if(mpFont && mpTextMaterial)
    {
        DEBUG_PRINT(_L("\t have font and material"));
        if(!mpTextMesh)
        {
#ifdef CPUT_FOR_DX11
            mpTextMesh = new CPUTMeshDX11();
#else
            mpTextMesh = new CPUTMeshOGL();
#endif
        }
        unsigned int numCharacters = (unsigned int) string.size();
        unsigned int numVertices = numCharacters * 6;
        CPUTGUIVertex* pVB = new CPUTGUIVertex[numVertices];

        CPUTBufferElementInfo pGUIVertex[3] = {
            { "POSITION", 0, 0, CPUT_F32, 3, 3*sizeof(float), 0 },            
            { "TEXCOORD", 0, 1, CPUT_F32, 2, 2*sizeof(float), 3*sizeof(float)},
            { "COLOR",    0, 2, CPUT_F32, 4, 4*sizeof(float), 5*sizeof(float)},
        };
        mpFont->LayoutText(NULL, &mTextWidth, &mTextHeight, mText, 0, 0);
        mTextX = (mWidth-mTextWidth)/2;
        mTextY = (mHeight-mTextHeight)/2;
        mpFont->LayoutText(pVB, &mTextWidth, &mTextHeight, mText, 0, 0);
        mpTextMesh->CreateNativeResources(NULL, 1, 3, pGUIVertex, numVertices, pVB, NULL, 0, NULL);
        
#ifdef CPUT_FOR_DX11
        mpTextMesh->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);
        
        D3D11_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },            
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        UINT numElements = ARRAYSIZE( layout );
        ID3DBlob* pBlob = ((CPUTMaterialEffectDX11*)mpTextMaterial->GetMaterialEffects()[0])->GetVertexShader()->GetBlob();
        
        CPUT_DX11::GetDevice()->CreateInputLayout( layout, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &mpVertexLayout );
        CPUTSetDebugName( mpVertexLayout, "CPUT GUI InputLayout object");
#else
#endif
        SAFE_DELETE_ARRAY(pVB);
    }
}

void CPUTGUIElement::RenderRecursive(CPUTRenderParameters &renderParams, int materialIndex)
{
    if(mVisible)
    {
        Render(renderParams, materialIndex);

        if(mpChild)
        {
            mpChild->RenderRecursive(renderParams, materialIndex);
            CPUTRenderNode *pNode = mpChild->GetSibling();
            while(pNode)
            {
                pNode->RenderRecursive(renderParams, materialIndex);
                pNode = pNode->GetSibling();
            }
        }
    }
}

void CPUTGUIElement::Render(CPUTRenderParameters &params, int materialIndex)
{
    if(mVisible)
    {
        if(mpSprite)
        {
            float4x4 world = float4x4Translation((float)mPosX, (float)mPosY, 0.0);
            float4x4 projection(*params.mpCamera->GetProjectionMatrix());
            mConstants.wvpMatrix = world*projection;
            mConstants.position.x = (float)mPosX;
            mConstants.position.y = (float)mPosY;
            mConstants.position.z = (float)mWidth;
            mConstants.position.w = (float)mHeight;
            if(mHighlighted)
                mConstants.backgroundcolor = mBackgroundHighlightColor;
            else
                mConstants.backgroundcolor = mBackgroundColor;
            mConstants.foregroundcolor = mForegroundColor;
            mConstants.resolution.x = (float)params.mWidth;
            mConstants.resolution.y = (float)params.mHeight;
            mConstants.mousePosition = mMousePosition;
#ifdef CPUT_FOR_DX11
            ID3D11DeviceContext *pDeviceContext = CPUT_DX11::GetContext();
            ID3D11Buffer *pBuffer = mpConstants->GetNativeBuffer();
            D3D11_MAPPED_SUBRESOURCE mapInfo;
            pDeviceContext->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo );
            GUIConstants *pCb = (GUIConstants*)mapInfo.pData;
            memcpy(pCb, &mConstants, sizeof(GUIConstants));
            pDeviceContext->Unmap(pBuffer, 0);
#else
            mpConstants->SetSubData(0, sizeof(GUIConstants), &mConstants);
#endif
            mpSprite->DrawSprite(params);
        }
        if(mpTextMesh)
        {
            float4x4 world = float4x4Translation((float)(mTextX+mPosX), (float)(mTextY+mPosY), 0.0);
            float4x4 projection(*params.mpCamera->GetProjectionMatrix());
            mConstants.wvpMatrix = world*projection;
            mConstants.position.x = (float)mTextX+mPosX;
            mConstants.position.y = (float)mTextY+mPosY;
            mConstants.position.z = (float)mWidth;
            mConstants.position.w = (float)mHeight;
            mConstants.backgroundcolor = mBackgroundColor;
            if(mHighlighted)
                mConstants.foregroundcolor = mForegroundHighlightColor;
            else
                mConstants.foregroundcolor = mForegroundColor;
            mConstants.resolution.x = (float)params.mWidth;
            mConstants.resolution.y = (float)params.mHeight;
#ifdef CPUT_FOR_DX11
            ID3D11DeviceContext *pDeviceContext = CPUT_DX11::GetContext();
            ID3D11Buffer *pBuffer = mpConstants->GetNativeBuffer();
            D3D11_MAPPED_SUBRESOURCE mapInfo;
            pDeviceContext->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapInfo );
            GUIConstants *pCb = (GUIConstants*)mapInfo.pData;
            memcpy(pCb, &mConstants, sizeof(GUIConstants));
            pDeviceContext->Unmap(pBuffer, 0);
            mpTextMaterial->GetMaterialEffects()[mpTextMaterial->GetCurrentEffect()]->SetRenderStates(params);
            mpTextMesh->Draw(params, mpVertexLayout);
#else
            mpConstants->SetSubData(0, sizeof(GUIConstants), &mConstants);
            mpTextMaterial->GetMaterialEffects()[mpTextMaterial->GetCurrentEffect()]->SetRenderStates(params);
            mpTextMesh->Draw(params, NULL);

#endif
            
        }
    }
}

void CPUTGUIElement::Resize(int x, int y, int width, int height, int windowWidth, int windowHeight)
{
    if(mParentRelative)
    {
        mPosX = mRelX + x;
        mPosY = mRelY + y;
        if(mRelX < 0)
            mPosX += width;
        if(mRelY < 0)
            mPosY += height;
    }
    else
    {
        mPosX = mRelX;
        mPosY = mRelY;
        if(mRelX < 0)
            mPosX += windowWidth;
        if(mRelY < 0)
            mPosY += windowHeight;
    }

    float4x4 m = float4x4Translation((float)mPosX, (float)mPosY, 0);
    SetParentMatrix(m);
    CPUTRenderNode* pNode = mpChild;
    if(pNode)
    {
        ((CPUTGUIElement*)pNode)->Resize(mPosX, mPosY, mWidth, mHeight, windowWidth, windowHeight);
    }
    pNode = mpSibling;
    while(pNode)
    {
        ((CPUTGUIElement*)pNode)->Resize(x, y, width, height, windowWidth, windowHeight);
        pNode = pNode->GetSibling();
    }
}

CPUTEventHandledCode SendEvent(int eventID, CPUTCallbackHandler* pHandler,CPUTGUIElement* pGUI)
{
    if(eventID > 0)
    {
        pHandler->HandleGUIElementEvent((CPUTEventID)eventID, 0, pGUI);
        return CPUT_EVENT_HANDLED;
    }
    return CPUT_EVENT_UNHANDLED;
}

CPUTEventHandledCode CPUTGUIElement::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    if(key == mHotkey)
    {
        if(state == CPUT_KEY_UP)
        {
            return SendEvent(mKeyUp.value, mpHandler, this);
        }
        else if(state == CPUT_KEY_DOWN)
        {
            return SendEvent(mKeyDown.value, mpHandler, this);
        }
    }
    return CPUTEventHandledCode::CPUT_EVENT_UNHANDLED;        
}

float2 CPUTGUIElement::GetRelativeMousePosition()
{
    return mMousePosition;
}

bool CPUTGUIElement::UpdateCursor(int x, int y)
{
    if(mButtonDown)
    {
        mMousePosition.x = ((float)(x - mPosX))/mWidth;
        mMousePosition.y = ((float)(y - mPosY))/mHeight;
    }
    return mPosX < x && x < mPosX + mWidth && mPosY < y && y < mPosY + mHeight;
}

CPUTEventHandledCode CPUTGUIElement::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message)
{
    if(!mpHandler)
        return CPUT_EVENT_UNHANDLED;
    
    bool previousIn = mCursorIn;
    mCursorIn = UpdateCursor(x, y);
    
    if(mButtonDown)
    {
        if(state == CPUTMouseState::CPUT_MOUSE_NONE)
        {
            mButtonDown = false;
            if(mCursorIn && mClick.value >= 0)
            {
                DEBUG_PRINT("mClick");
                return SendEvent(mClick.value, mpHandler, this);
            }
            else
            {
                DEBUG_PRINT("mUp");
                return SendEvent(mUp.value, mpHandler, this);
            }
        }
        else if(!mCursorIn && previousIn)
        {
            DEBUG_PRINT("mOut");
            return SendEvent(mOut.value, mpHandler, this);
        }
        else if(mCursorIn && !previousIn)
        {
            DEBUG_PRINT("mIn");
            return SendEvent(mIn.value, mpHandler, this);
        }
        return CPUT_EVENT_HANDLED;
    }
    else //mButtonDown == false
    {   
        if(mCursorIn && message == CPUT_EVENT_DOWN)
        {
            if(state == CPUT_MOUSE_LEFT_DOWN)
            {
                mButtonDown = true;
                DEBUG_PRINT("mDown");
                SendEvent(mDown.value, mpHandler, this);
                return CPUT_EVENT_HANDLED;
            }
        }
    }
    return CPUT_EVENT_UNHANDLED;
};
    
void CPUTGUIElement::SetEvents(CPUTRenderNode *pRoot, StringMap *pEventMap, CPUTCallbackHandler *pHandler)
{
    if(pRoot == NULL)
        return;
    CPUTRenderNode *pNode = pRoot;
    do
    {
        if(pNode->GetNodeType() == CPUTRenderNode::CPUT_NODE_TYPE::CPUT_NODE_GUI_ELEMENT)
        {
            ((CPUTGUIElement*)pNode)->SetEvents(pEventMap, pHandler);
        }
        pNode = pNode->GetNext(pRoot);
    } while(pNode  != pRoot);
}

CPUTEventHandledCode CPUTGUIElement::HandleKeyboardEvent(CPUTRenderNode *pRoot, CPUTKey key, CPUTKeyState state)
{
    if(pRoot == NULL)
        return CPUT_EVENT_UNHANDLED;;
    CPUTRenderNode* pNode = pRoot;

    do 
    {
        if(pNode->GetNodeType() == CPUTRenderNode::CPUT_NODE_TYPE::CPUT_NODE_GUI_ELEMENT)
        {
            if(CPUT_EVENT_HANDLED == ((CPUTGUIElement*)pNode)->HandleKeyboardEvent(key, state))
                return CPUT_EVENT_HANDLED;
        }
        pNode = pNode->GetNext(pRoot);
    } while(pNode  != pRoot);
    return CPUT_EVENT_UNHANDLED;
}

CPUTEventHandledCode CPUTGUIElement::HandleMouseEvent(CPUTRenderNode *pRoot, int x, int y , int wheel, CPUTMouseState state, CPUTEventID message)
{
    if(pRoot == NULL)
        return CPUT_EVENT_UNHANDLED;;
    CPUTRenderNode* pNode = pRoot;

    do 
    {
        if(pNode->GetNodeType() == CPUTRenderNode::CPUT_NODE_TYPE::CPUT_NODE_GUI_ELEMENT)
        {
            CPUTGUIElement* pElement = (CPUTGUIElement*)pNode;
            if(CPUT_EVENT_HANDLED == ((CPUTGUIElement*)pNode)->HandleMouseEvent(x, y, wheel, state, message))
                return CPUT_EVENT_HANDLED;
        }
        pNode = pNode->GetNext(pRoot);
    } while(pNode  != pRoot);
    return CPUT_EVENT_UNHANDLED;
}

void CPUTGUIElement::Resize(CPUTRenderNode *pRoot, int width, int height)
{
    CPUTRenderNode *pNode = pRoot;
    while(pNode->GetNodeType() != CPUT_NODE_GUI_ELEMENT)    
        pNode = pNode->GetNext(pRoot);

    if(pNode)
    {
        ((CPUTGUIElement*)pNode)->Resize(0, 0, width, height, width, height);
    }
}

struct CPUTKeyMapEntry
{
    cString name;
    CPUTKey value;
};

CPUTKeyMapEntry CPUTKeyMap[] =
{
    // caps keys
    {_L("KEY_A"),KEY_A},
    {_L("KEY_B"),KEY_B},
    {_L("KEY_C"),KEY_C},
    {_L("KEY_D"),KEY_D},
    {_L("KEY_E"),KEY_E},
    {_L("KEY_F"),KEY_F},
    {_L("KEY_G"),KEY_G},
    {_L("KEY_H"),KEY_H},
    {_L("KEY_I"),KEY_I},
    {_L("KEY_J"),KEY_J},
    {_L("KEY_K"),KEY_K},
    {_L("KEY_L"),KEY_L},
    {_L("KEY_M"),KEY_M},
    {_L("KEY_N"),KEY_N},
    {_L("KEY_O"),KEY_O},
    {_L("KEY_P"),KEY_P},
    {_L("KEY_Q"),KEY_Q},
    {_L("KEY_R"),KEY_R},
    {_L("KEY_S"),KEY_S},
    {_L("KEY_T"),KEY_T},
    {_L("KEY_U"),KEY_U},
    {_L("KEY_V"),KEY_V},
    {_L("KEY_W"),KEY_W},
    {_L("KEY_X"),KEY_X},
    {_L("KEY_Y"),KEY_Y},
    {_L("KEY_Z"),KEY_Z},

    // number keys
    {_L("KEY_1"),KEY_1},
    {_L("KEY_2"),KEY_2},
    {_L("KEY_3"),KEY_3},
    {_L("KEY_4"),KEY_4},
    {_L("KEY_5"),KEY_5},
    {_L("KEY_6"),KEY_6},
    {_L("KEY_7"),KEY_7},
    {_L("KEY_8"),KEY_8},
    {_L("KEY_9"),KEY_9},
    {_L("KEY_0"),KEY_0},

    // symbols
    {_L("KEY_SPACE"), KEY_SPACE},
    {_L("KEY_BACKQUOTE"), KEY_BACKQUOTE},
    {_L("KEY_TILDE"), KEY_TILDE},
    {_L("KEY_EXCLAMATION"), KEY_EXCLAMATION},
    {_L("KEY_AT"), KEY_AT},
    {_L("KEY_HASH"), KEY_HASH},
    {_L("KEY_$"), KEY_$},
    {_L("KEY_PERCENT"), KEY_PERCENT},
    {_L("KEY_CARROT"), KEY_CARROT},
    {_L("KEY_ANDSIGN"), KEY_ANDSIGN},
    {_L("KEY_STAR"), KEY_STAR},
    {_L("KEY_OPENPAREN"), KEY_OPENPAREN},
    {_L("KEY_CLOSEPARN"), KEY_CLOSEPARN},
    {_L("KEY__)"), KEY__},
    {_L("KEY_MINUS"), KEY_MINUS},
    {_L("KEY_PLUS"), KEY_PLUS},

    {_L("KEY_OPENBRACKET"), KEY_OPENBRACKET},
    {_L("KEY_CLOSEBRACKET"), KEY_CLOSEBRACKET},
    {_L("KEY_OPENBRACE"), KEY_OPENBRACE},
    {_L("KEY_CLOSEBRACE"), KEY_CLOSEBRACE},
    {_L("KEY_BACKSLASH"), KEY_BACKSLASH},
    {_L("KEY_PIPE"), KEY_PIPE},
    {_L("KEY_SEMICOLON"), KEY_SEMICOLON},
    {_L("KEY_COLON"), KEY_COLON},
    {_L("KEY_SINGLEQUOTE"), KEY_SINGLEQUOTE},
    {_L("KEY_QUOTE"), KEY_QUOTE},
    {_L("KEY_COMMA"), KEY_COMMA},
    {_L("KEY_PERIOD"), KEY_PERIOD},
    {_L("KEY_SLASH"), KEY_SLASH},
    {_L("KEY_LESS"), KEY_LESS},
    {_L("KEY_GREATER"), KEY_GREATER},
    {_L("KEY_QUESTION"), KEY_QUESTION},

    // function keys
    {_L("KEY_F1"),KEY_F1},
    {_L("KEY_F2"),KEY_F2},
    {_L("KEY_F3"),KEY_F3},
    {_L("KEY_F4"),KEY_F4},
    {_L("KEY_F5"),KEY_F5},
    {_L("KEY_F6"),KEY_F6},
    {_L("KEY_F7"),KEY_F7},
    {_L("KEY_F8"),KEY_F8},
    {_L("KEY_F9"),KEY_F9},
    {_L("KEY_F10"),KEY_F10},
    {_L("KEY_F11"),KEY_F11},
    {_L("KEY_F12"),KEY_F12},

    // special keys
    {_L("KEY_HOME"),KEY_HOME},
    {_L("KEY_END"),KEY_END},
    {_L("KEY_INSERT"), KEY_INSERT},
    {_L("KEY_DELETE"),KEY_DELETE},
    {_L("KEY_PAGEUP"),KEY_PAGEUP},
    {_L("KEY_PAGEDOWN"),KEY_PAGEDOWN},

    {_L("KEY_UP"),KEY_UP},
    {_L("KEY_DOWN"),KEY_DOWN},
    {_L("KEY_LEFT"),KEY_LEFT},
    {_L("KEY_RIGHT"),KEY_RIGHT},
    
    {_L("KEY_BACKSPACE"),KEY_BACKSPACE},
    {_L("KEY_ENTER"),KEY_ENTER},
    {_L("KEY_TAB"),KEY_TAB},
    {_L("KEY_PAUSE"),KEY_PAUSE},
    {_L("KEY_CAPSLOCK"),KEY_CAPSLOCK},
    {_L("KEY_ESCAPE"), KEY_ESCAPE},

    // control keys
    {_L("KEY_SHIFT"),KEY_SHIFT},
    {_L("KEY_LEFT_SHIFT"),KEY_LEFT_SHIFT},
    {_L("KEY_RIGHT_SHIFT"),KEY_RIGHT_SHIFT},
    {_L("KEY_CTRL"),KEY_CTRL},
    {_L("KEY_LEFT_CTRL"),KEY_LEFT_CTRL},
    {_L("KEY_RIGHT_CTRL"),KEY_RIGHT_CTRL},
    {_L("KEY_LEFT_ALT"),KEY_LEFT_ALT},
    {_L("KEY_RIGHT_ALT"),KEY_RIGHT_ALT},

    {_L(""), KEY_NUM_KEYS},
};

CPUTKey MapKey(cString strKey)
{
    for(int i = 0; i < KEY_NUM_KEYS; i++)
    {
        if( 0 == strKey.compare(CPUTKeyMap[i].name))
            return CPUTKeyMap[i].value;
    }
    DEBUG_PRINT(_L("Unknown Key: %s"), strKey.c_str());
    return KEY_NUM_KEYS;
}
