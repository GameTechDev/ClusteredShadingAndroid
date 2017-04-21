/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "CPUTCamera.h"
#include "CPUTFrustum.h"

//-----------------------------------------------------------------------------
CPUTResult CPUTCamera::LoadCamera(CPUTConfigBlock *pBlock, int *pParentID)
{
    // TODO: Have render node load common properties.
    CPUTResult result = CPUT_SUCCESS;

    mName = pBlock->GetValueByName(_L("name"))->ValueAsString();
    *pParentID = pBlock->GetValueByName(_L("parent"))->ValueAsInt();

    mFov = pBlock->GetValueByName(_L("FieldOfView"))->ValueAsFloat();
    mFov = DegToRad(mFov);
    mNearPlaneDistance = pBlock->GetValueByName(_L("NearPlane"))->ValueAsFloat();
    mFarPlaneDistance = pBlock->GetValueByName(_L("FarPlane"))->ValueAsFloat();

    LoadParentMatrixFromParameterBlock( pBlock );

    return result;
}

//-----------------------------------------------------------------------------
void CPUTCamera::LookAt( const float xx, const float yy, const float zz )
{
    float3 pos;
    GetPosition( &pos);

    float3 lookPoint(xx, yy, zz);
    float3 look  = (lookPoint - pos).normalize();
    float3 right = cross3(float3(0.0f,1.0f,0.0f), look).normalize(); // TODO: simplify algebraically
    float3 up    = cross3(look, right);
    
    mParentMatrix = float4x4(
        right.x, right.y, right.z, 0.0f,
           up.x,    up.y,    up.z, 0.0f,
         look.x,  look.y,  look.z, 0.0f,
          pos.x,   pos.y,   pos.z, 1.0f
    );
}

//-----------------------------------------------------------------------------
void CPUTCamera::Update( float deltaSeconds ) {
    // TODO: Do only if required (i.e. if dirty)
    if( mMode == CPUT_PERSPECTIVE )
    {
        mProjection = float4x4PerspectiveFovLH( mFov, mAspectRatio, mFarPlaneDistance, mNearPlaneDistance );
    }
    else if ( mMode == CPUT_ORTHOGRAPHIC )
    {
        mProjection = float4x4OrthographicLH(mWidth, mHeight, mFarPlaneDistance, mNearPlaneDistance);
    }
    mView = inverse(*GetWorldMatrix());
    mFrustum.InitializeFrustum(this);
};

#define KEY_DOWN(vk) ((GetAsyncKeyState(vk) & 0x8000)?1:0)
//-----------------------------------------------------------------------------
void CPUTCameraControllerFPS::Update(float deltaSeconds)
{
    if( !mpCamera )
    {
        return;
    }
    float speed = mfMoveSpeed * deltaSeconds;
    float4x4 *pParentMatrix = mpCamera->GetParentMatrix();

    float3 vRight(pParentMatrix->getXAxis());
    float3 vUp(pParentMatrix->getYAxis());
    float3 vLook(pParentMatrix->getZAxis());
    float3 vPositionDelta(0.0f);
    int    rotateX = 0;
    int    rotateY = 0;
    bool   bRotate = false;
    if (keyPressed[KEY_SHIFT] == CPUT_KEY_DOWN) {
        speed *= 10.0f;
    }
    if (keyPressed[KEY_CTRL] == CPUT_KEY_DOWN) {
        speed *= 0.5;
    }
    // Added the ability to toggle on/off rotation mode
    if (keyPressed[KEY_SPACE] == CPUT_KEY_DOWN) {
        bRotate = true;
    }
    if(keyPressed[KEY_W] == CPUT_KEY_DOWN) {
        vPositionDelta +=  vLook *  speed;
        rotateY = -1;
    }
    if(keyPressed[KEY_A] == CPUT_KEY_DOWN) {
        vPositionDelta += vRight * -speed;
        rotateX = -1;
    }
    if(keyPressed[KEY_S] == CPUT_KEY_DOWN) {
        vPositionDelta +=  vLook * -speed;
        rotateY = 1;
    }
    if(keyPressed[KEY_D] == CPUT_KEY_DOWN) {
        vPositionDelta += vRight *  speed;
        rotateX = 1;
    }
    if(keyPressed[KEY_E] == CPUT_KEY_DOWN) {
        vPositionDelta +=    vUp *  speed;
    }
    if(keyPressed[KEY_Q] == CPUT_KEY_DOWN) {
        vPositionDelta +=    vUp * -speed;
    }
    if (bRotate && (rotateX || rotateY))
    {
        // this lets you rotate the camera with the keyboard if you don't have a mouse. like if you only have one
        // usb slot available on a mobile.
        float nDeltaX = (float)(rotateX) * speed * 10.0f;
        float nDeltaY = (float)(rotateY) * speed * 10.0f;

        float4x4 rotationX = float4x4RotationX(nDeltaY*mfLookSpeed);
        float4x4 rotationY = float4x4RotationY(nDeltaX*mfLookSpeed);

        float3 position = mpCamera->GetPosition();
        mpCamera->SetPosition(0.0f, 0.0f, 0.0f); // Rotate about camera center
        float4x4 parent      = *mpCamera->GetParentMatrix();
        float4x4 orientation = rotationX  *parent * rotationY;
        orientation.orthonormalize();
        mpCamera->SetParentMatrix( orientation );
        mpCamera->SetPosition( position.x, position.y, position.z ); // Move back to original position
    }
    else
    {
        float x,y,z;
        mpCamera->GetPosition( &x, &y, &z );
        mpCamera->SetPosition( x+vPositionDelta.x, y+vPositionDelta.y, z+vPositionDelta.z );
    }
    mpCamera->Update();

    //Compute animation as an offset from the current position
    mpCamera->mPosition += float3( vPositionDelta.x, vPositionDelta.y, vPositionDelta.z );
    //END
    return;
}

CPUTEventHandledCode CPUTCameraControllerFPS::HandleKeyboardEvent(CPUTKey key, CPUTKeyState state)
{
    keyPressed[key] = state;
    return CPUT_EVENT_HANDLED;
}

//-----------------------------------------------------------------------------
CPUTEventHandledCode CPUTCameraControllerFPS::HandleMouseEvent(
    int x,
    int y,
    int wheel,
    CPUTMouseState state,
    CPUTEventID message
)
{
    if( !mpCamera )
    {
        return CPUT_EVENT_UNHANDLED;
    }
    if(state & CPUT_MOUSE_LEFT_DOWN)
    {
        float3 position = mpCamera->GetPosition();

        if(!(mPrevFrameState & CPUT_MOUSE_LEFT_DOWN)) // Mouse was just clicked
        {
            mnPrevFrameX = x;
            mnPrevFrameY = y;
        }

        float nDeltaX = (float)(x-mnPrevFrameX);
        float nDeltaY = (float)(y-mnPrevFrameY);

        //Compute animation as an offset from the current basis
        float degreeY = nDeltaX*mfLookSpeed;
        float degreeX = nDeltaY*mfLookSpeed;
        mpCamera->mRotation += float3(degreeX,degreeY,0);

        float4x4 rotationX = float4x4RotationX(degreeX);
        float4x4 rotationY = float4x4RotationY(degreeY);

        mpCamera->SetPosition(0.0f, 0.0f, 0.0f); // Rotate about camera center
        float4x4 parent      = *mpCamera->GetParentMatrix();
        float4x4 orientation = rotationX  *parent * rotationY;
        orientation.orthonormalize();
        mpCamera->SetParentMatrix( orientation );
        mpCamera->SetPosition( position.x, position.y, position.z ); // Move back to original position
        mpCamera->Update();

        mnPrevFrameX = x;
        mnPrevFrameY = y;
        mPrevFrameState = state;
        return CPUT_EVENT_HANDLED;
    } else
    {
        mPrevFrameState = state;
        return CPUT_EVENT_UNHANDLED;
    }
}

//-----------------------------------------------------------------------------
CPUTEventHandledCode CPUTCameraControllerArcBall::HandleMouseEvent(
    int x,
    int y,
    int wheel,
    CPUTMouseState state, 
    CPUTEventID message
)
{
    // TODO: We want move-in-x to orbit light in view space, not object space.
    if( !mpCamera )
    {
        return CPUT_EVENT_UNHANDLED;
    }
    if(state & CPUT_MOUSE_RIGHT_DOWN) // TODO: How to make this flexible?  Want to choose which mouse button has effect.
    {
        float4x4  rotationX, rotationY;

        if(!(mPrevFrameState & CPUT_MOUSE_RIGHT_DOWN)) // Mouse was just clicked
        {
            mnPrevFrameX = x;
            mnPrevFrameY = y;
        }
        int nDeltaX = x-mnPrevFrameX;
        int nDeltaY = y-mnPrevFrameY;

        rotationY = float4x4RotationX(nDeltaY*mfLookSpeed);
        rotationX = float4x4RotationY(nDeltaX*mfLookSpeed);
        float4x4 orientation = *mpCamera->GetParentMatrix() * rotationY * rotationX;

        orientation.orthonormalize();
        mpCamera->SetParentMatrix( orientation );

        mnPrevFrameX = x;
        mnPrevFrameY = y;
        mPrevFrameState = state;
        return CPUT_EVENT_HANDLED;
    } else
    {
        mPrevFrameState = state;
        return CPUT_EVENT_UNHANDLED;
    }
}
