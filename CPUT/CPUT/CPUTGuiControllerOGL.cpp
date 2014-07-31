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
#include "CPUTGuiControllerOGL.h"
#include "CPUT_OGL.h" // for CPUTSetRasterizerState()
#include "CPUTTextureOGL.h"
#include "CPUTFont.h"
#include "CPUTAssetLibraryOGL.h"
#include "CPUTBufferOGL.h"
CPUTGuiControllerOGL* CPUTGuiControllerOGL::mguiController = NULL;

// chained constructor
//--------------------------------------------------------------------------------
CPUTGuiControllerOGL::CPUTGuiControllerOGL():
    CPUTGuiController(),
    mpTextMaterial(NULL),
    mpControlMaterial(NULL),
    mpConstantBufferVS(NULL),
    mModelViewMatrices(),
    mpUberBuffer(NULL),
    mpMirrorBuffer(NULL),
    mUberBufferIndex(0),
    mUberBufferMax(CPUT_GUI_BUFFER_SIZE),
    mpTextUberBuffer(NULL),
    mpTextMirrorBuffer(NULL),
    mTextUberBufferIndex(0),
    mpFocusedControlMirrorBuffer(NULL),
    mFocusedControlBufferIndex(0),
    mpFocusedControlBuffer(NULL),
    mpFocusedControlTextMirrorBuffer(NULL),
    mFocusedControlTextBufferIndex(0),
    mpFocusedControlTextBuffer(NULL)
{
    mpMirrorBuffer = new CPUTGUIVertex[CPUT_GUI_BUFFER_SIZE];
    mpTextMirrorBuffer = new  CPUTGUIVertex[CPUT_GUI_BUFFER_STRING_SIZE];

    mpFocusedControlMirrorBuffer = new CPUTGUIVertex[CPUT_GUI_BUFFER_SIZE];
    mpFocusedControlTextMirrorBuffer = new CPUTGUIVertex[CPUT_GUI_BUFFER_STRING_SIZE];
    
}

// destructor
//--------------------------------------------------------------------------------
CPUTGuiControllerOGL::~CPUTGuiControllerOGL()
{
    // delete all the controls under you
    ReleaseResources();
    DeleteAllControls();
    SAFE_RELEASE(mpTextMaterial);
    SAFE_RELEASE(mpControlMaterial);
    // FPS counter

    // delete arrays
    SAFE_DELETE_ARRAY(mpTextMirrorBuffer);
    SAFE_DELETE_ARRAY(mpMirrorBuffer);
    SAFE_DELETE_ARRAY(mpFocusedControlMirrorBuffer);
    SAFE_DELETE_ARRAY(mpFocusedControlTextMirrorBuffer);
}

// static getter
//--------------------------------------------------------------------------------
CPUTGuiControllerOGL* CPUTGuiControllerOGL::GetController()
{
    if(NULL==mguiController)
    {
        mguiController = new CPUTGuiControllerOGL();
    }
    return mguiController;
}


// Delete the controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerOGL::DeleteController()
{
    SAFE_DELETE(mguiController);

    return CPUT_SUCCESS;
}

//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerOGL::ReleaseResources()
{
    //Release all allocated resources
    SAFE_RELEASE(mpConstantBufferVS);

    // release the texture atlas+buffers
    SAFE_RELEASE(mpUberBuffer);
    
    SAFE_RELEASE(mpTextUberBuffer);
    
    SAFE_RELEASE(mpFocusedControlBuffer);
    SAFE_RELEASE(mpFocusedControlTextBuffer);    

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
CPUTResult CPUTGuiControllerOGL::Initialize(cString &ResourceDirectory)
{
    SetResourceDirectory(ResourceDirectory);
    return RegisterGUIResources();
}


// Control creation

// Create a button control and add it to the GUI layout controller
//--------------------------------------------------------------------------------
CPUTResult CPUTGuiControllerOGL::CreateButton(const cString ButtonText, CPUTControlID controlID, CPUTControlID panelID, CPUTButton **ppButton)
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
CPUTResult CPUTGuiControllerOGL::CreateSlider(const cString SliderText, CPUTControlID controlID, CPUTControlID panelID, CPUTSlider **ppSlider, float scale)
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
CPUTResult CPUTGuiControllerOGL::CreateCheckbox(const cString CheckboxText, CPUTControlID controlID, CPUTControlID panelID, CPUTCheckbox **ppCheckbox, float scale)
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
CPUTResult CPUTGuiControllerOGL::CreateDropdown(const cString SelectionText, CPUTControlID controlID, CPUTControlID panelID, CPUTDropdown **ppDropdown)
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
CPUTResult CPUTGuiControllerOGL::CreateText(const cString Text, CPUTControlID controlID, CPUTControlID panelID, CPUTText **ppStatic)
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
CPUTResult CPUTGuiControllerOGL::DeleteControl(CPUTControlID controlID)
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
void CPUTGuiControllerOGL::DrawFPS(bool drawfps)
{
    //mbDrawFPS = drawfps;
}

// GetFPS - Returns the last frame's FPS value
//--------------------------------------------------------------------------------
float CPUTGuiControllerOGL::GetFPS()
{
    return 0;//mLastFPS;
}

// Draw - must be positioned after all the controls are defined
//--------------------------------------------------------------------------------
void CPUTGuiControllerOGL::Draw()
{
    HEAPCHECK;

    static double timeSinceLastFPSUpdate = 0;
    static int    framesSinceLastFPSUpdate = 0;
    static double avgFPS = 0;
    static double minFrameTime = 999;
    static double maxFrameTime = 0;
    static CPUT_RECT windowRect = {0, 0, 0, 0};

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
            mRecalculateLayout = false;
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
        UpdateUberBuffers();

        // Clear dirty flag on uberbuffer
        mUberBufferDirty = false;

    }
    HEAPCHECK

    if( mpConstantBufferVS )
    {
		// set up orthographic display
		int windowWidth, windowHeight;
		float znear = 0.1f;
		float zfar = 100.0f;
		float4x4 m;

		pWindow->GetClientDimensions( &windowWidth, &windowHeight );
		m = float4x4OrthographicOffCenterLH(0, (float)windowWidth, (float)windowHeight, 0, znear, zfar);
    
	    GUIConstantBufferVS ConstantBufferMatrices;
	    ConstantBufferMatrices.Projection = m;
        m = float4x4Identity();
	    ConstantBufferMatrices.Model = m; 
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, mpConstantBufferVS->GetBufferID()));
        GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GUIConstantBufferVS), &ConstantBufferMatrices));
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

    CPUTRenderParameters params;
    mpControlMaterial->GetMaterialEffects()[0]->SetRenderStates(params);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

	// draw the control graphics
	CPUTRenderParameters p;
	mpUberBuffer->Draw(p, NULL);
    
    mpTextMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    // draw text things here
    // draw the FPS counter
	mpTextUberBuffer->Draw(p, NULL);

    mpControlMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    mpFocusedControlBuffer->Draw(p, NULL);
    
	// draw focused control
	mpTextMaterial->GetMaterialEffects()[0]->SetRenderStates(params);
    mpFocusedControlTextBuffer->Draw(p, NULL);
    
	glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
	GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    HEAPCHECK;
}

//
//------------------------------------------------------------------------
CPUTResult CPUTGuiControllerOGL::UpdateUberBuffers()
{

	// Update geometry to draw the control graphics
    ASSERT(CPUT_GUI_VERTEX_BUFFER_SIZE > mUberBufferIndex, _L("CPUT GUI: Too many controls for default-sized uber-buffer.  Increase CPUT_GUI_VERTEX_BUFFER_SIZE"));
    //why +1?
    mpUberBuffer->SetVertexSubData(0, sizeof(CPUTGUIVertex)*mUberBufferIndex+1, mpMirrorBuffer); 
    mpUberBuffer->SetNumVertices(mUberBufferIndex);
    mpTextUberBuffer->SetVertexSubData(0, sizeof(CPUTGUIVertex)*mTextUberBufferIndex+1, mpTextMirrorBuffer);
    mpTextUberBuffer->SetNumVertices(mTextUberBufferIndex);
    mpFocusedControlBuffer->SetVertexSubData(0, sizeof(CPUTGUIVertex)*mFocusedControlBufferIndex+1, mpFocusedControlMirrorBuffer);
    mpFocusedControlBuffer->SetNumVertices(mFocusedControlBufferIndex);
    mpFocusedControlTextBuffer->SetVertexSubData(0, sizeof(CPUTGUIVertex)*mFocusedControlTextBufferIndex+1, mpFocusedControlTextMirrorBuffer);
    mpFocusedControlTextBuffer->SetNumVertices(mFocusedControlTextBufferIndex);
    
    return CPUT_SUCCESS;
}


// Load and register all the resources needed by the GUI system
//-----------------------------------------------------------------------------
CPUTResult CPUTGuiControllerOGL::RegisterGUIResources()
{
    CPUTResult result = CPUT_SUCCESS;

	cString name = _L("$cbGUIValues");
    mpConstantBufferVS = new CPUTBufferOGL(name);
    GLuint id = mpConstantBufferVS->GetBufferID();
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, mpConstantBufferVS->GetBufferID()));
    GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, sizeof(GUIConstantBufferVS), NULL, GL_DYNAMIC_DRAW)); // NULL to just init buffer size
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    DEBUG_PRINT(_L("bind gui constant buffers: %d\n"), id);
//    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, mpConstantBufferVS->GetBufferID(), mpConstantBufferVS->GetBufferID()));
    DEBUG_PRINT(_L("completed bind gui constant buffers\n"));
    CPUTAssetLibrary::GetAssetLibrary()->AddConstantBuffer(_L(""), name, _L(""), mpConstantBufferVS);

    CPUTAssetLibraryOGL *pAssetLibrary = NULL;
    pAssetLibrary = (CPUTAssetLibraryOGL*)CPUTAssetLibraryOGL::GetAssetLibrary();

    // Get the resource directory
    cString mediaDirectory;
    mediaDirectory = pAssetLibrary->GetMediaDirectoryName();

    mpTextMaterial = pAssetLibrary->GetMaterial(_L("guimaterial_text"));
    mpControlMaterial = pAssetLibrary->GetMaterial(_L("guimaterial_control"));

    CPUTBufferElementInfo pGUIVertex[3] = {
        { "POSITION", 0, 0, CPUT_F32, 3, 3*sizeof(float), 0 },            
        { "TEXCOORD", 0, 1, CPUT_F32, 2, 2*sizeof(float), 3*sizeof(float)},
        { "COLOR",    0, 2, CPUT_F32, 4, 4*sizeof(float), 5*sizeof(float)},
    };

    unsigned int maxSize = std::max(CPUT_GUI_BUFFER_STRING_SIZE, CPUT_GUI_BUFFER_SIZE);
    maxSize = std::max(maxSize, CPUT_GUI_VERTEX_BUFFER_SIZE);    
    maxSize *= sizeof( CPUTGUIVertex );

    mpUberBuffer = new CPUTMeshOGL();
    mpUberBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, mpMirrorBuffer, NULL, 0, NULL);

    mpTextUberBuffer = new CPUTMeshOGL();
    mpTextUberBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, NULL, NULL, 0, NULL);

    mpFocusedControlBuffer = new CPUTMeshOGL();
	mpFocusedControlBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, mpFocusedControlMirrorBuffer, NULL, 0, NULL);

    mpFocusedControlTextBuffer = new CPUTMeshOGL();
	mpFocusedControlTextBuffer->CreateNativeResources(NULL, 0, 3, pGUIVertex, CPUT_GUI_VERTEX_BUFFER_SIZE, mpFocusedControlTextMirrorBuffer, NULL, 0, NULL);

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
    DEBUG_PRINT(_L("Exit RegisterGUIResources()\n"));
    return CPUT_SUCCESS;
}

// Re-calculates all the positions of the controls based on their sizes
// to have a 'pretty' layout
//--------------------------------------------------------------------------------
void CPUTGuiControllerOGL::RecalculateLayout()
{
    // if we have no valid panel, just return
    if(CPUT_CONTROL_ID_INVALID == mActiveControlPanelSlotID)
    {
        return;
    }

    // if we don't want the auto-layout feature, just return
    if(false == mbAutoLayout)
    {
        return;
    }

    // get window size
    CPUT_RECT windowRect;
    pWindow->GetClientDimensions(&windowRect.x, &windowRect.y, &windowRect.width, &windowRect.height);

	// Build columns of controls right to left
    int x,y;
    x=0; y=0;

    // walk list of controls, counting up their *heights*, until the
    // column is full.  While counting, keep track of the *widest*
    int width, height;
    const int GUI_WINDOW_PADDING = 5;

    int numberOfControls = (int) mControlPanelIDList[mActiveControlPanelSlotID]->mControlList.size();
    int indexStart=0;
    int indexEnd=0;
    int columnX = 0;
    int columnNumber = 1;
    while(indexEnd < numberOfControls)
    {
        int columnWidth=0;
        y=0;
        // figure out which controls belong in this column + column width
        while( indexEnd < numberOfControls )
        {
            if(mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[indexEnd]->IsVisible() &&
                mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[indexEnd]->IsAutoArranged())
            {
                mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[indexEnd]->GetDimensions(width, height);
                if( y + height + GUI_WINDOW_PADDING < (windowRect.height-2*GUI_WINDOW_PADDING))
                {
                    y = y + height + GUI_WINDOW_PADDING;
                    if(columnWidth < width)
                    {
                        columnWidth = width;
                    }
                    indexEnd++;
                }
                else
                {
                    // if the window is now so small it won't fit a whole control, just
                    // draw one anyway and it'll just have to be clipped
                    if(indexEnd == indexStart)
                    {
                        columnWidth = width;
                        indexEnd++;
                    }
                    break;
                }
            }
            else
            {
                indexEnd++;
            }
        }
        

        // ok, now re-position each control with x at widest, and y at proper height
        y=GUI_WINDOW_PADDING;
        for(int i=indexStart; i<indexEnd; i++)
        {
            if(mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[i]->IsVisible() &&
                mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[i]->IsAutoArranged())
            {
                mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[i]->GetDimensions(width, height);
                x = windowRect.width - columnX - columnWidth - (columnNumber*GUI_WINDOW_PADDING);
                mControlPanelIDList[mActiveControlPanelSlotID]->mControlList[i]->SetPosition(x,y);
				y = y + height + GUI_WINDOW_PADDING;
            }
        }
        indexStart = indexEnd;
        columnX+=columnWidth;
        columnNumber++;
    }
        
    mRecalculateLayout = false;
}
