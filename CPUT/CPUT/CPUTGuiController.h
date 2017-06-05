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
#ifndef __CPUTGUICONTROLLER_H__
#define __CPUTGUICONTROLLER_H__



#include <stdio.h>
#include <stdlib.h> // for RAND_MAX
#include <vector>
#include <cstring>

#include "CPUTEventHandler.h"
#include "CPUTControl.h"
#include "CPUTOSServices.h"
#include "CPUTWindow.h"

// forward declarations
class CPUTButton;
class CPUTSlider;
class CPUTCheckbox;
class CPUTDropdown;
class CPUTText;
class CPUTFont;

// definition of the vertex used in the GUI shader
struct CPUTGUIVertex
{
    float3 Pos;
    float2 UV;
    CPUTColor4 Color;
};

class CPUTGuiController:public CPUTEventHandler
{
public:
    CPUTGuiController();
    virtual ~CPUTGuiController();

    // get GUI controller base class
    static CPUTGuiController *GetController();

    //CPUTEventHandler members
    virtual CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state) {UNREFERENCED_PARAMETER(key); return CPUT_EVENT_UNHANDLED;}
    virtual CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);

    // members
    void EnableAutoLayout(bool UseAutoLayout);
    CPUTResult SetResourceDirectory(const cString ResourceDirectory);     // sets the resource directory to use when loading GUI resources

    virtual CPUTResult CreateButton(const cString pButtonText, CPUTControlID controlID, CPUTControlID panelID, CPUTButton **ppButton=NULL) = 0;
    virtual CPUTResult CreateSlider(const cString pSliderText, CPUTControlID controlID, CPUTControlID panelID, CPUTSlider **ppSlider=NULL, float scale = 1.0f) = 0;
    virtual CPUTResult CreateCheckbox(const cString pCheckboxText, CPUTControlID controlID, CPUTControlID panelID, CPUTCheckbox **ppCheckbox=NULL, float scale = 1.0f) = 0;
    virtual CPUTResult CreateDropdown(const cString pSelectionText, CPUTControlID controlID, CPUTControlID panelID, CPUTDropdown **ppDropdown=NULL) = 0;
    virtual CPUTResult CreateText(const cString Text,  CPUTControlID controlID, CPUTControlID panelID, CPUTText **ppStatic=NULL) = 0;    
    virtual CPUTResult DeleteControl(CPUTControlID controlID) = 0;
    virtual void DrawFPS(bool drawfps) = 0;

    // Panels
    CPUTResult      AddControl(CPUTControl *pControl, CPUTControlID panelID);                                // adds a control to the specified panel
    CPUTResult      FindControl(CPUTControlID controlID, CPUTControl **ppControl, CPUTControlID *pPanelID);  // search all panels to find a control and its panelID
    CPUTControl    *GetControl(CPUTControlID controlID, CPUTResult *pResult = NULL);            // search all panels to find a control
    CPUTResult      SetActivePanel(CPUTControlID panelID);                                      // sets the actively displayed panel
    CPUTControlID   GetActivePanelID();                                                         // returns the ID of the active panel
    CPUTResult      GetActiveControlList(std::vector<CPUTControl*> *ppControlList);
    CPUTResult      RemoveControlFromPanel(CPUTControlID controlID, CPUTControlID panelID=-1);  // removes specified control from the panel (does not delete the control)
    CPUTResult      DeletePanel(CPUTControlID panelID);                                         // removes panel and deletes all controls associated with it
    void            DeleteAllControls();                                                        // deletes all controls and all panels
    int             GetNumberOfControlsInPanel(CPUTControlID panelID=-1);                       // returns the number of controls in a specific panel
    bool            IsControlInPanel(CPUTControlID controlID, CPUTControlID panelID=-1);        // Is specified control in panel?
    bool            IsRecalculatingLayout() {return mRecalculateLayout;}
    void			SetWindow(CPUTWindow *pWin) { pWindow = pWin; }
    void            SetFont(CPUTFont *pFont) { mpFont = pFont; }

	// drawing/callbacks
    void Resize();  
    void SetCallback(CPUTCallbackHandler *pHandler, bool ForceAll=false);       // sets the event handler callback on all registered controls
    void ControlIsDirty() {mUberBufferDirty = true;}
    void RecalculateLayout();
protected:
    cString     mResourceDirectory;
    bool        mUberBufferDirty;
    bool        mRecalculateLayout;
    
    struct Panel
    {
        CPUTControlID mpanelID;
        CPUTControl *mpFocusControl;
        std::vector<CPUTControl*> mControlList;
    };

    // list of panels which have lists of controls associated with it
    std::vector<Panel*>  mControlPanelIDList;

    // the active panel list
    CPUTControlID  mActiveControlPanelSlotID;
    CPUTCallbackHandler *mpHandler;

    // active control
    CPUTControl *mpFocusControl;

    bool mbAutoLayout;
    CPUTFont                *mpFont;
    CPUTWindow              *pWindow;

private:
    bool mbRebuildDrawList;
    // helper functions
    UINT FindPanelIDIndex(CPUTControlID panelID);

};



#endif //#ifndef __CPUTGUICONTROLLER_H__
