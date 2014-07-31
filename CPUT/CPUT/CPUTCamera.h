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
#ifndef __CPUTCamera_H__
#define __CPUTCamera_H__

#include <memory.h>
#include "CPUT.h"
#include "CPUTRenderNode.h"
#include "CPUTConfigBlock.h"
#include "CPUTFrustum.h"
#include "CPUTOSServices.h"

enum CPUT_PROJECTION_MODE {
    CPUT_PERSPECTIVE,
    CPUT_ORTHOGRAPHIC
};

#define ASSERT_ORTHOGRAPHIC ASSERTA(mMode == CPUT_CAMERA_TYPE_ORTHOGRAPHIC, "Camera not orthographic.");
#define ASSERT_PERSPECTIVE  ASSERTA(mMode == CPUT_CAMERA_TYPE_PERSPECTIVE,  "Camera not perspective.");

//-----------------------------------------------------------------------------
class CPUTCamera:public CPUTRenderNode
{
protected:
    CPUT_PROJECTION_MODE mMode;
    float            mFov;                // the field of view in degrees
    float            mNearPlaneDistance;
    float            mFarPlaneDistance;
    float            mAspectRatio;        // width/height.  TODO: Support separate pixel and viewport aspect ratios
    float            mWidth;              // width and height for orthographic projection
    float            mHeight;
    float4x4         mView;
    float4x4         mProjection;

public:
    CPUTFrustum mFrustum;

    CPUTCamera( CPUT_PROJECTION_MODE mode = CPUT_PERSPECTIVE) : 
        mFov(75.0f * kPi/180.0f),
        mNearPlaneDistance(1.0f),
        mFarPlaneDistance(100.0f),
        mAspectRatio(16.0f/9.0f),
        mWidth(64),
        mHeight(64),
        mMode(mode)
    {
        // default maya position (roughly)
        SetPosition( 1.0f, 0.8f, 1.0f );
    }
    ~CPUTCamera() {}

    CPUT_NODE_TYPE GetNodeType() { return CPUT_NODE_CAMERA;};

    // We can't afford to Update() every time we're asked for the view matrix.
    // Caller needs to make sure to Update() before entering render loop.
    const float4x4  *GetViewMatrix(void)       const { return &mView; }
    const float4x4  *GetProjectionMatrix(void) const { return &mProjection; }
    CPUT_PROJECTION_MODE GetProjectionMode()       const { return mMode; }
    float            GetNearPlaneDistance()    const { return mNearPlaneDistance; }
    float            GetFarPlaneDistance()     const { return mFarPlaneDistance; }
    float            GetWidth()                const { ASSERT_ORTHOGRAPHIC; return mWidth; }
    float            GetHeight()               const { ASSERTA(mMode == CPUT_ORTHOGRAPHIC, "Camera not orthographic."); return mHeight; }
    float            GetAspectRatio()          const { return mAspectRatio; }
    float            GetFov()                  const { ASSERT_PERSPECTIVE; return mFov; }

    void             SetProjectionMatrix(const float4x4 &projection) { mProjection = projection; }
    void             SetNearPlaneDistance( float nearPlaneDistance ) { mNearPlaneDistance = nearPlaneDistance; }
    void             SetFarPlaneDistance(  float farPlaneDistance )  { mFarPlaneDistance  = farPlaneDistance; }
    void             SetWidth( float width)                          { ASSERT_ORTHOGRAPHIC; mWidth  = width;}
    void             SetHeight(float height)                         { ASSERT_ORTHOGRAPHIC; mHeight = height;};
    void             SetProjectionMode(CPUT_PROJECTION_MODE mode)    { mMode = mode; mFov = 0.0f; }
    void             SetAspectRatio(const float aspectRatio)         { mAspectRatio = aspectRatio; }
    void             SetFov(float fov)                               { ASSERT_PERSPECTIVE; mFov = fov; }

    void             LookAt( float xx, float yy, float zz );
    void             LookAt( const float3 &pp ) { LookAt( pp.x, pp.y, pp.z ); }
    void             Update( float deltaSeconds=0.0f );
    CPUTResult       LoadCamera(CPUTConfigBlock *pBlock, int *pParentID);
};

//-----------------------------------------------------------------------------
class CPUTCameraController : public CPUTEventHandler
{
protected:
    CPUTRenderNode *mpCamera;
    float           mfMoveSpeed;
    float           mfLookSpeed;
    int             mnPrevFrameX;
    int             mnPrevFrameY;
    CPUTMouseState  mPrevFrameState;
    CPUTKeyState    keyPressed[KEY_NUM_KEYS];

public:
    CPUTCameraController()
        : mpCamera(NULL)
        , mnPrevFrameX(0)
        , mnPrevFrameY(0)
        , mfMoveSpeed(1.0f)
        , mfLookSpeed(1.0f)
    {
        for (unsigned int i = 0; i < 100; i++) {
            keyPressed[i] = CPUT_KEY_UP;
        }
    }
    virtual         ~CPUTCameraController(){ SAFE_RELEASE(mpCamera);}
    void            SetCamera(CPUTRenderNode *pCamera)  { SAFE_RELEASE(mpCamera); mpCamera = pCamera; if(pCamera){pCamera->AddRef();} }
    CPUTRenderNode *GetCamera(void) const               { return mpCamera; }
    void            SetMoveSpeed(float speed)           { mfMoveSpeed = speed; }
    void            SetLookSpeed(float speed)           { mfLookSpeed = speed; }
    virtual void    Update(float deltaSeconds=0.0f) = 0;
};

//-----------------------------------------------------------------------------
class CPUTCameraControllerFPS : public CPUTCameraController
{
public:
    
    unsigned int keyPressedIndex;
    void Update( float deltaSeconds=0.0f);
    CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state);
    CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID message);
};

//-----------------------------------------------------------------------------
class CPUTCameraControllerArcBall : public CPUTCameraController
{
public:
    void Update( float deltaSeconds=0.0f ) {}
    CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key, CPUTKeyState state) { return CPUT_EVENT_UNHANDLED; }
    CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state, CPUTEventID  message);
};

#endif //#ifndef __CPUTCamera_H__
