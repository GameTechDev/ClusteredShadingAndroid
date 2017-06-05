/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or imlied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "CPUTGuiControllerDX11.h"
#include "CPUT_DX11.h" // for CPUTSetRasterizerState()
#include "CPUTTextureDX11.h"
#include "CPUTFont.h"
#include "CPUTBufferDX11.h"


CPUTGuiControllerDX11* CPUTGuiControllerDX11::mguiController = NULL;

// chained constructor
//--------------------------------------------------------------------------------
CPUTGuiControllerDX11::CPUTGuiControllerDX11():CPUTGuiController(),
    mpGUIVertexShader(NULL),
    mpGUIPixelShader(NULL),
    mpConstantBufferVS(NULL),
    mpVertexLayout(NULL),

    // texture atlas+uber buffer
    mpMirrorBuffer(NULL),
    mUberBufferIndex(0),
    mUberBufferMax(CPUT_GUI_BUFFER_SIZE),
    mpTextMirrorBuffer(NULL),
    mTextUberBufferIndex(0),

    mFocusedControlBufferIndex(0),
    mFocusedControlTextBufferIndex(0),
    
    mpUberBuffer(NULL),
    mpTextUberBuffer(NULL),
    mpFocusedControlBuffer(NULL),
    mpFocusedControlTextBuffer(NULL)
{
    mpMirrorBuffer = new CPUTGUIVertex[CPUT_GUI_BUFFER_SIZE];
    mpTextMirrorBuffer = new  CPUTGUIVertex[CPUT_GUI_BUFFER_STRING_SIZE];

    mpFocusedControlMirrorBuffer = new CPUTGUIVertex[CPUT_GUI_BUFFER_SIZE];
    mpFocusedControlTextMirrorBuffer = new CPUTGUIVertex[CPUT_GUI_BUFFER_STRING_SIZE];
}

// destructor
//--------------------------------------------------------------------------------
CPUTGuiControllerDX11::~CPUTGuiControllerDX11()
{
    // delete all the controls under you
    ReleaseResources();
    DeleteAllControls();

    // delete arrays
    SAFE_DELETE_ARRAY(mpTextMirrorBuffer);
    SAFE_DELETE_ARRAY(mpMirrorBuffer);
    SAFE_DELETE_ARRAY(mpFocusedControlMirrorBuffer);
    SAFE_DELETE_ARRAY(mpFocusedControlTextMirrorBuffer);
}

// static getter
//--------------------------------------------------------------------------------
CPUTGuiControllerDX11* CPUTGuiControllerDX11::GetController()
{
    if(NULL==mguiController)
    {
        mguiController = new CPUTGuiControllerDX11();
    }
    return mguiController;
}


// Delete the controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::DeleteController()
{
    SAFE_DELETE(mguiController);

    return CPUT_SUCCESS;
}

//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::ReleaseResources()
{
    //Release all allocated resources
    SAFE_RELEASE(mpGUIVertexShader);
    SAFE_RELEASE(mpGUIPixelShader);
    SAFE_RELEASE(mpVertexLayout);
    SAFE_RELEASE(mpConstantBufferVS);

    // release the texture atlas+buffers
    
    SAFE_RELEASE(mpUberBuffer);
    SAFE_RELEASE(mpTextUberBuffer);
    SAFE_RELEASE(mpFocusedControlBuffer);
    SAFE_RELEASE(mpFocusedControlTextBuffer);

    SAFE_RELEASE(mpTextMaterial);
    SAFE_RELEASE(mpControlMaterial);
    
    SAFE_RELEASE(mpConstantBufferVS2);

    // tell all controls to unregister all their static resources
    CPUTText::UnRegisterStaticResources();
    CPUTButton::UnRegisterStaticResources();
    CPUTCheckbox::UnRegisterStaticResources();
    CPUTSlider::UnRegisterStaticResources();
    CPUTDropdown::UnRegisterStaticResources();

    return CPUT_SUCCESS;
}

// Initialize the GUI controller and all it's static resources
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::Initialize(ID3D11DeviceContext *pImmediateContext, cString &ResourceDirectory)
{
    // All the GUI resource files
    cString VertexShader =          _L(".//shader//GUIShaderDX.vs");
    cString PixelShader =          _L(".//shader//GUIShaderDX.ps");
    cString DefaultFontFile =       _L(".//fonts//font_arial_12.dds");
    cString ControlAtlasTexture =   _L(".//controls//atlas.dds");
    cString RenderStateFile =       _L(".//Shader//GUIRenderState.rs");

    // store the resource directory to be used by the GUI manager
    SetResourceDirectory(ResourceDirectory);

    return RegisterGUIResources(pImmediateContext, VertexShader, PixelShader, RenderStateFile, DefaultFontFile, ControlAtlasTexture);
}


// Control creation

// Create a button control and add it to the GUI layout controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::CreateButton(const cString ButtonText, CPUTControlID controlID, CPUTControlID panelID, CPUTButton **ppButton)
{
    // create the control
    CPUTButton *pButton = new CPUTButton(ButtonText, controlID, mpFont);
    ASSERT(NULL != pButton, _L("Failed to create control.") );

    // return control if requested
    if(NULL!=ppButton)
    {
        *ppButton = pButton;
    }

    // add control to the gui manager
    return this->AddControl(pButton, panelID);

}

// Create a slider control and add it to the GUI layout controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::CreateSlider(const cString SliderText, CPUTControlID controlID, CPUTControlID panelID, CPUTSlider **ppSlider, float scale)
{
    // create the control
    CPUTSlider *pSlider = new CPUTSlider(SliderText, controlID, mpFont, scale);
    ASSERT(NULL!=pSlider, _L("Failed creating slider") );

    // return control if requested
    if(NULL!=ppSlider)
    {
        *ppSlider = pSlider;
    }
    
    // add control to the gui manager
    return this->AddControl(pSlider, panelID);
}

// Create a checkbox control and add it to the GUI layout controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::CreateCheckbox(const cString CheckboxText, CPUTControlID controlID, CPUTControlID panelID, CPUTCheckbox **ppCheckbox, float scale)
{
    // create the control
    CPUTCheckbox *pCheckbox = new CPUTCheckbox(CheckboxText, controlID, mpFont, scale);
    ASSERT(NULL!=pCheckbox, _L("Failed creating checkbox") );

    // return control if requested
    if(NULL!=ppCheckbox)
    {
        *ppCheckbox = pCheckbox;
    }

    // add control to the gui manager
    return this->AddControl(pCheckbox, panelID);
}

// Create a dropdown control and add it to the GUI layout controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::CreateDropdown(const cString SelectionText, CPUTControlID controlID, CPUTControlID panelID, CPUTDropdown **ppDropdown)
{
    // create the control
    CPUTDropdown *pDropdown = new CPUTDropdown(SelectionText, controlID, mpFont);
    ASSERT(NULL!=pDropdown, _L("Failed creating control") );

    // return control if requested
    if(NULL!=ppDropdown)
    {
        *ppDropdown = pDropdown;
    }

    // add control to the gui manager
    CPUTResult result;
    result = this->AddControl(pDropdown, panelID);

    return result;
}

// Create a text item (static text)
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::CreateText(const cString Text, CPUTControlID controlID, CPUTControlID panelID, CPUTText **ppStatic)
{
    // create the control
    CPUTText *pStatic=NULL;
    pStatic = new CPUTText(Text, controlID, mpFont);
    ASSERT(NULL!=pStatic, _L("Failed creating static") );
    if(NULL != ppStatic)
    {
        *ppStatic = pStatic;
    }

    // add control to the gui manager
    return this->AddControl(pStatic, panelID);
}

// Deletes a control from the GUI manager
// Will delete all instances of the control no matter which panel(s) it is in and then
// deallocates the memory for the control
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::DeleteControl(CPUTControlID controlID)
{
    // look thruogh all the panels and delete the item with this controlID
    // for each panel
    std::vector <CPUTControl*> pDeleteList;

    for(UINT i=0; i<mControlPanelIDList.size(); i++)
    {
        // walk list of controls
        for(UINT j=0; j<mControlPanelIDList[i]->mControlList.size(); j++)
        {
            if(controlID == mControlPanelIDList[i]->mControlList[j]->GetControlID())
            {
                // found an instance of the control we wish to delete
                // see if it's in the list already
                bool bFound = false;
                for(UINT x=0; x<pDeleteList.size(); x++)
                {
                    if( mControlPanelIDList[i]->mControlList[j] ==  pDeleteList[x] )
                    {
                        bFound = true;
                        break;
                    }
                }

                if(!bFound)
                {
                    // store for deleting
                    pDeleteList.push_back( mControlPanelIDList[i]->mControlList[j] );
                }

                // remove the control from the container list
                mControlPanelIDList[i]->mControlList.erase( mControlPanelIDList[i]->mControlList.begin() + j );
            }
        }
    }

    // delete the control(s) we found with this id
    for(UINT i=0; i<pDeleteList.size(); i++)
    {
        SAFE_DELETE( pDeleteList[i] );
    }

    // force a resize event to recalculate new control locations now that some might have been deleted
    this->Resize();

    // don't worry about cleaning up std::vector list itself, it'll do so when it falls out of scope
    return CPUT_SUCCESS;
}

// DrawFPS - Should the GUI draw the FPS counter in the upper-left?
//--------------------------------------------------------------------------------
void CPUTGuiControllerDX11::DrawFPS(bool drawfps)
{
}

// GetFPS - Returns the last frame's FPS value
//--------------------------------------------------------------------------------
float CPUTGuiControllerDX11::GetFPS()
{
    return 0.0f;
}

// Draw - must be positioned after all the controls are defined
//--------------------------------------------------------------------------------
void CPUTGuiControllerDX11::Draw(ID3D11DeviceContext *pImmediateContext)
{
    HEAPCHECK;

    if( 0 == GetNumberOfControlsInPanel())
    {
        return;
    }

    // check and see if any of the controls resized themselves
    int ControlCount=GetNumberOfControlsInPanel();
    bool ResizingNeeded = false;
    for(int ii=0; ii<ControlCount; ii++)
    {
        CPUTControl *pControl = mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[ii];
        if(true == pControl->ControlResizedItself())
        {
            ResizingNeeded = true;
            pControl->ControlResizingHandled();
        }
    }

    // if any of the controls resized, then re-do the autoarrangment
    if(true == ResizingNeeded)
    {
        this->Resize();
    }

    // Now check to see if any controls' graphics are dirty
    for(int ii=0; ii<ControlCount; ii++)
    {
        CPUTControl *pControl = mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[ii];
        if(true == pControl->ControlGraphicsDirty())
        {
            mUberBufferDirty = true;
            break;
        }        
    }

    // if any of the controls have announced they are dirty, then rebuild the mirror buffer and update the GFX buffer
    if(mUberBufferDirty)
    {
        
        // if a resize was flagged, do it now.  
        if(mRecalculateLayout)
        {
            RecalculateLayout();
        }


        // 'clear' the buffer by resetting the pointer to the head
        mUberBufferIndex = 0;
        mTextUberBufferIndex = 0;
        mFocusedControlBufferIndex = 0;
        mFocusedControlTextBufferIndex = 0;

        int ii=0;
        while(ii<GetNumberOfControlsInPanel())
        {
            CPUTControl *pControl = mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[ii];

            // don't draw the focus control - draw it last so it stays on 'top'
            if(mpFocusControl != pControl)
            {
                switch(pControl->GetType())
                {
                case CPUT_BUTTON:
                    ((CPUTButton*)pControl)->DrawIntoBuffer(mpMirrorBuffer, &mUberBufferIndex, mUberBufferMax, mpTextMirrorBuffer, &mTextUberBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);                    
                    break;
                case CPUT_CHECKBOX:
                    ((CPUTCheckbox*)pControl)->DrawIntoBuffer(mpMirrorBuffer, &mUberBufferIndex, mUberBufferMax, mpTextMirrorBuffer, &mTextUberBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                    break;
                case CPUT_SLIDER:
                    ((CPUTSlider*)pControl)->DrawIntoBuffer(mpMirrorBuffer, &mUberBufferIndex, mUberBufferMax, mpTextMirrorBuffer, &mTextUberBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                    break;
                case CPUT_DROPDOWN:
                    ((CPUTDropdown*)pControl)->DrawIntoBuffer(mpMirrorBuffer, &mUberBufferIndex, mUberBufferMax, mpTextMirrorBuffer, &mTextUberBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                    break;

                case CPUT_STATIC:
                    ((CPUTText*)pControl)->DrawIntoBuffer(mpTextMirrorBuffer, &mTextUberBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                    break;
                }
            }
            ii++;
            HEAPCHECK
        }

        // do the 'focused' control last so it stays on top (i.e. dropdowns)
        if(mpFocusControl)
        {
            switch(mpFocusControl->GetType())
            {
            case CPUT_BUTTON:
                ((CPUTButton*)mpFocusControl)->DrawIntoBuffer(mpFocusedControlMirrorBuffer, &mFocusedControlBufferIndex, mUberBufferMax, mpFocusedControlTextMirrorBuffer, &mFocusedControlTextBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);                    
                break;
            case CPUT_CHECKBOX:
                ((CPUTCheckbox*)mpFocusControl)->DrawIntoBuffer(mpFocusedControlMirrorBuffer, &mFocusedControlBufferIndex, mUberBufferMax, mpFocusedControlTextMirrorBuffer, &mFocusedControlTextBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                break;
            case CPUT_SLIDER:
                ((CPUTSlider*)mpFocusControl)->DrawIntoBuffer(mpFocusedControlMirrorBuffer, &mFocusedControlBufferIndex, mUberBufferMax, mpFocusedControlTextMirrorBuffer, &mFocusedControlTextBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                break;
            case CPUT_DROPDOWN:
                ((CPUTDropdown*)mpFocusControl)->DrawIntoBuffer(mpFocusedControlMirrorBuffer, &mFocusedControlBufferIndex, mUberBufferMax, mpFocusedControlTextMirrorBuffer, &mFocusedControlTextBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                break;
            case CPUT_STATIC:
                ((CPUTText*)mpFocusControl)->DrawIntoBuffer(mpFocusedControlMirrorBuffer, &mFocusedControlTextBufferIndex, CPUT_GUI_BUFFER_STRING_SIZE);
                break;
            }
        }
        
                
        // update the uber-buffers with the control graphics
        UpdateUberBuffers(pImmediateContext);

        // Clear dirty flag on uberbuffer
        mUberBufferDirty = false;

    }
    HEAPCHECK    

    // set up orthographic display
    int windowWidth, windowHeight;
    GUIConstantBufferVS ConstantBufferMatrices;
    float znear = 0.1f;
    float zfar = 100.0f;
    DirectX::XMMATRIX m;

    pWindow->GetClientDimensions( &windowWidth, &windowHeight );
    m = DirectX::XMMatrixOrthographicOffCenterLH(0, (float)windowWidth, (float)windowHeight, 0, znear, zfar);
    ConstantBufferMatrices.Projection = DirectX::XMMatrixTranspose( m );

    m = DirectX::XMMatrixIdentity();
    ConstantBufferMatrices.Model = DirectX::XMMatrixTranspose( m );

    CPUTRenderParametersDX params;
    CPUTRenderParametersDX p;
    p.mpContext = params.mpContext = pImmediateContext;

    mpControlMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    pImmediateContext->UpdateSubresource( mpConstantBufferVS, 0, NULL, &ConstantBufferMatrices, 0, 0 );
    pImmediateContext->VSSetConstantBuffers( 0, 1, &mpConstantBufferVS );
    mpUberBuffer->Draw(p, mpVertexLayout);


    mpTextMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    mpTextUberBuffer->Draw(p, mpVertexLayout);

    mpControlMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    mpFocusedControlBuffer->Draw(p, mpVertexLayout);

    mpTextMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    mpFocusedControlTextBuffer->Draw(p, mpVertexLayout);

    HEAPCHECK;
}

//
//------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::UpdateUberBuffers(ID3D11DeviceContext *pImmediateContext )
{
    ASSERT(pImmediateContext, _L("CPUTGuiControllerDX11::UpdateUberBuffers - Context pointer is NULL"));

    // get the device
    ID3D11Device *pD3dDevice = NULL;
    pImmediateContext->GetDevice(&pD3dDevice);
    

    // Update geometry to draw the control graphics
    ASSERT(CPUT_GUI_VERTEX_BUFFER_SIZE > mUberBufferIndex, _L("CPUT GUI: Too many controls for default-sized uber-buffer.  Increase CPUT_GUI_VERTEX_BUFFER_SIZE"));
    pImmediateContext->UpdateSubresource(mpUberBuffer->GetVertexBuffer(), 0, NULL, (void *)mpMirrorBuffer, sizeof(CPUTGUIVertex)*mUberBufferIndex+1, 0);
    mpUberBuffer->SetNumVertices(mUberBufferIndex);

    // Update geometry to draw the controls' text
    ASSERT(CPUT_GUI_BUFFER_STRING_SIZE > mTextUberBufferIndex, _L("CPUT GUI: Too many strings for default-sized uber-buffer.  Increase CPUT_GUI_BUFFER_STRING_SIZE"));
    pImmediateContext->UpdateSubresource(mpTextUberBuffer->GetVertexBuffer(), 0, NULL, (void*) mpTextMirrorBuffer, sizeof( CPUTGUIVertex )*(mTextUberBufferIndex+1), 0);
    mpTextUberBuffer->SetNumVertices(mTextUberBufferIndex);

    // register the focused control's graphics
    ASSERT(CPUT_GUI_VERTEX_BUFFER_SIZE > mUberBufferIndex, _L("CPUT GUI: Too many controls for default-sized uber-buffer.  Increase CPUT_GUI_VERTEX_BUFFER_SIZE"));
    pImmediateContext->UpdateSubresource(mpFocusedControlBuffer->GetVertexBuffer(), 0, NULL, (void*) mpFocusedControlMirrorBuffer, sizeof( CPUTGUIVertex )*(mFocusedControlBufferIndex+1), 0);
    mpFocusedControlBuffer->SetNumVertices(mFocusedControlBufferIndex);

    //register the focused control's text
    ASSERT(CPUT_GUI_BUFFER_STRING_SIZE > mFocusedControlTextBufferIndex, _L("CPUT GUI: Too many strings for default-sized uber-buffer.  Increase CPUT_GUI_BUFFER_STRING_SIZE"));
    pImmediateContext->UpdateSubresource(mpFocusedControlTextBuffer->GetVertexBuffer(), 0, NULL, (void*) mpFocusedControlTextMirrorBuffer, sizeof( CPUTGUIVertex )*(mFocusedControlTextBufferIndex+1), 0);
    mpFocusedControlTextBuffer->SetNumVertices(mFocusedControlTextBufferIndex);

    // release the device pointer
    SAFE_RELEASE(pD3dDevice);    

    return CPUT_SUCCESS;

}

// Load and register all the resources needed by the GUI system
//-----------------------------------------------------------------------------
CPUTResult CPUTGuiControllerDX11::RegisterGUIResources(ID3D11DeviceContext *pImmediateContext, cString VertexShaderFilename, cString PixelShaderFilename, cString RenderStateFile, cString DefaultFontFilename, cString ControlAtlasTexture)
{
    if(NULL==pImmediateContext)
    {
        return CPUT_ERROR_INVALID_PARAMETER;
    }

    {
        D3D11_BUFFER_DESC bd = {0};
        bd.ByteWidth = sizeof(GUIConstantBufferVS);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        ID3D11Buffer *pGUIConstantBuffer;

        ID3D11Device *pDevice = CPUT_DX11::GetDevice();
        HRESULT hr = pDevice->CreateBuffer( &bd, NULL, &pGUIConstantBuffer );
        ASSERT( !FAILED( hr ), _L("Error creating constant buffer." ));
        CPUTSetDebugName( pGUIConstantBuffer, _L("GUI VS Constant buffer" ));
        cString name = _L("$cbGUIValues");
        CPUTMaterial::mGlobalProperties.AddValue( _L("cbGUIValues"), _L("$cbGUIValues") );        
        mpConstantBufferVS2 = new CPUTBufferDX11(name, pGUIConstantBuffer);
        CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer( name, _L(""), _L(""), mpConstantBufferVS2);
        SAFE_RELEASE(pGUIConstantBuffer);
    }

    CPUTResult result;
    HRESULT hr;
    ID3D11Device *pD3dDevice = NULL;
    CPUTAssetLibraryDX11 *pAssetLibrary = NULL;
    std::string ErrorMessage;

    // Get the services/resource pointers we need
    pImmediateContext->GetDevice(&pD3dDevice);
    pAssetLibrary = (CPUTAssetLibraryDX11*)CPUTAssetLibraryDX11::GetAssetLibrary();

    // Get the resource directory
    cString mediaDirectory;
    cString baseAssetDirectoryName;
    mediaDirectory = pAssetLibrary->GetMediaDirectoryName();
    baseAssetDirectoryName = pAssetLibrary->GetAssetDirectoryName();

    // Load the GUI Vertex Shader
    result = pAssetLibrary->GetVertexShader(baseAssetDirectoryName + VertexShaderFilename, _L("VS"), _L("vs_4_0"), &mpGUIVertexShader, true);
    CPUTSetDebugName( mpGUIVertexShader->GetNativeVertexShader(), _L("GUIVertexShader"));
    if(CPUTFAILED(result))
    {
        ASSERT(CPUTSUCCESS(result), _L("Error loading the vertex shader needed for the CPUT GUI system."));
    }
    ID3DBlob *pVertexShaderBlob = mpGUIVertexShader->GetBlob();

    // Load the GUI Pixel Shader
    result = pAssetLibrary->GetPixelShader(baseAssetDirectoryName + PixelShaderFilename, _L("PS"), _L("ps_4_0"), &mpGUIPixelShader, true);
    CPUTSetDebugName( mpGUIPixelShader->GetNativePixelShader(), _L("GUIPixelShader"));
    if(CPUTFAILED(result))
    {
        ASSERT(CPUTSUCCESS(result), _L("Error loading the pixel shader needed for the CPUT GUI system."));
    }

    // 4. Create the vertex layout description for all the GUI controls we'll draw
    // set vertex shader as active so we can configure it
    ID3D11VertexShader *pVertexShader = mpGUIVertexShader->GetNativeVertexShader();
    pImmediateContext->VSSetShader( pVertexShader, NULL, 0 );

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },            
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = pD3dDevice->CreateInputLayout( layout, numElements, pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), &mpVertexLayout );
    ASSERT( SUCCEEDED(hr), _L("Error creating CPUT GUI system input layout") );
    CPUTSetDebugName( mpVertexLayout, _L("CPUT GUI InputLayout object"));
    
    // 5. create the vertex shader constant buffer pointers
    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(GUIConstantBufferVS);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = pD3dDevice->CreateBuffer( &bd, NULL, &mpConstantBufferVS );
    ASSERT( SUCCEEDED(hr), _L("Error creating constant buffer VS") );
    CPUTSetDebugName( mpConstantBufferVS, _L("GUI ConstantBuffer"));
      
    mpTextMaterial = pAssetLibrary->GetMaterial(_L("guimaterial_text"));
    mpControlMaterial = pAssetLibrary->GetMaterial(_L("guimaterial_control"));

    CPUTBufferElementInfo pGUIVertex[3] = {
        { "POSITION", 0, 0, CPUT_F32, 3, 3*sizeof(float), 0 },            
        { "TEXCOORD", 0, 1, CPUT_F32, 2, 2*sizeof(float), 3*sizeof(float)},
        { "COLOR",    0, 2, CPUT_F32, 4, 4*sizeof(float), 5*sizeof(float)},
    };
    
    // 7. Set up the DirectX uber-buffers that the controls draw into
    unsigned int maxSize = std::max(CPUT_GUI_BUFFER_STRING_SIZE, CPUT_GUI_BUFFER_SIZE);
    maxSize = std::max(maxSize, CPUT_GUI_VERTEX_BUFFER_SIZE);    
    maxSize *= sizeof( CPUTGUIVertex );

    mpUberBuffer = new CPUTMeshDX11();
    mpUberBuffer->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);
    mpUberBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, mpMirrorBuffer, NULL, 0, NULL);
 
    mpTextUberBuffer = new CPUTMeshDX11();
    mpTextUberBuffer->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);
    mpTextUberBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, NULL, NULL, 0, NULL);

    mpFocusedControlBuffer = new CPUTMeshDX11();
    mpFocusedControlBuffer->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);
	mpFocusedControlBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, mpFocusedControlMirrorBuffer, NULL, 0, NULL);

    mpFocusedControlTextBuffer = new CPUTMeshDX11();
    mpFocusedControlTextBuffer->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);
	mpFocusedControlTextBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, mpFocusedControlTextMirrorBuffer, NULL, 0, NULL);

 

    char *pZeroedBuffer= new char[maxSize];
    memset(pZeroedBuffer, 0, maxSize);

    // set up buffer description
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( CPUTGUIVertex ) * CPUT_GUI_VERTEX_BUFFER_SIZE; //mUberBufferIndex;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    // initialization data (all 0's for now)
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = mpMirrorBuffer;
    
    // no longer need the device - release it.
    SAFE_RELEASE(pD3dDevice);
    SAFE_DELETE_ARRAY(pZeroedBuffer);    


    // 8. Register all GUI sub-resources
    // Walk all the controls/fonts and have them register all their required static resources
    // Returning errors if you couldn't find your resources
    result = CPUTText::RegisterStaticResources();
    if(CPUTFAILED(result))
    {
        return result;
    }
    result = CPUTButton::RegisterStaticResources();
    if(CPUTFAILED(result))
    {
        return result;
    }
    result = CPUTCheckbox::RegisterStaticResources();
    if(CPUTFAILED(result))
    {
        return result;
    }
    result = CPUTSlider::RegisterStaticResources();
    if(CPUTFAILED(result))
    {
        return result;
    }
    result = CPUTDropdown::RegisterStaticResources();
    if(CPUTFAILED(result))
    {
        return result;
    }

    return CPUT_SUCCESS;
}
