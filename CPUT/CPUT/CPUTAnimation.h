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

#ifndef __CPUTANIMATION_H__
#define __CPUTANIMATION_H__

#include <string>
#include <fstream>
#include <exception>

#include "CPUTMath.h"
#include "CPUTRefCount.h"
#include "CPUTAssetLibrary.h"
#include <vector>
#include "CPUTSkeleton.h"


enum CPUTInterpolationType
{
    CPUT_CONSTANT_INTERPOLATION	    = 0x02,
    CPUT_LINEAR_INTERPOLATION		= 0x04,
    CPUT_CUBIC_INTERPOLATION		= 0x08
};

//Supported Animated Channels
enum CPUTAnimatedProperty
{
    TRANSLATE_X,
    TRANSLATE_Y,
    TRANSLATE_Z,

    ROTATE_X,
    ROTATE_Y,
    ROTATE_Z,

    SCALE_X,
    SCALE_Y,
    SCALE_Z
};

//Nesting CPUTKeyFrame, CPUTAnimationCurve and CPUTAnimationLayer as their
//implementation details do not not to be known outside of CPUTNodeAnimation
class CPUTNodeAnimation: public CPUTRefCount
{

    //Keyframe: contains value for a given state of a single Animated property
    //-----------------------------------------------------------------------
    class CPUTKeyFrame
    {
    public:
        float					mValue;                 //Value of the Keyframe
        float					mTime;                  //Keyframe's sample time
        CPUTInterpolationType	mInterpolationType;     //Interpolation used with Keyframe
        float					mCubicCoefficients[4];  //Coefficients used with cubic interpolation

        void					LoadKeyFrame(CPUTFileSystem::CPUTOSifstream& file);
    };

    //AnimationCurve: Collection of KeyFrames for any given Animated property
    //-----------------------------------------------------------------------
    class CPUTAnimationCurve
    {
        CPUTKeyFrame*			mpKeyFramesList;
        CPUTAnimatedProperty	mTransformType;
        UINT					mNumberOfKeyFrames;

    public:
        ~CPUTAnimationCurve()
        {
            if(mpKeyFramesList != NULL)
            {
                delete [] mpKeyFramesList;
            }
        }
        CPUTAnimationCurve():mpKeyFramesList(NULL),
            mTransformType(TRANSLATE_X),mNumberOfKeyFrames(0){}

        UINT	GetTransformType() const;
        void	LoadAnimationCurve(CPUTFileSystem::CPUTOSifstream& file);
        float	Interpolate(float sampleTime);
    };

    //AnimationLayer: Contains collection of Animated Curves.  Multiple Layers can be
    //blended together
    //-----------------------------------------------------------------------
    class CPUTAnimationLayer
    {
    public:
        float					mWeight;	//Weight of the layer, used for blending layers
        cString					mName;
        CPUTAnimationCurve*		mpCurvesList;
        UINT					mNumberOfCurves;

        ~CPUTAnimationLayer()
        {
            if(mpCurvesList != NULL)
            {
                delete [] mpCurvesList;
            }
        }
        CPUTAnimationLayer():mWeight(1.0),mName(_L("")),
            mpCurvesList(NULL),mNumberOfCurves(0){}

        UINT					GetNumberOfCurves() const { return mNumberOfCurves; }
        void					LoadAnimationLayer(CPUTFileSystem::CPUTOSifstream& file);
    };

    //NodeAnimation: Per-node Animations, composed of a collection of Animation Layers
    //-----------------------------------------------------------------------
    cString						mTarget;	//Name of the node associated with animation
    cString						mName;
    CPUTAnimationLayer*			mpLayersList;
    UINT						mNumberOfLayers;
    float						mDuration;	//Duration of the entire Node Animation
protected:
    ~CPUTNodeAnimation()
    {
        if(mpLayersList != NULL)
        {
            delete [] mpLayersList;
        }
    }

    //Hierarchy related variables
    int mId;
    int mParentId;
    CPUTNodeAnimation *mpParent;
    CPUTNodeAnimation *mpChild;
    CPUTNodeAnimation *mpSibling;
public:
    
    CPUTNodeAnimation():mTarget(_L("")),mName(_L("")),mpLayersList(NULL),
        mNumberOfLayers(0),mDuration(0.0f),mpParent(NULL),mpChild(NULL),mpSibling(NULL),
    mId(0),mParentId(0){}

    cString						GetTargetName() const;
    cString						GetName() const;
    void						LoadNodeAnimation(int &parentIndex,CPUTFileSystem::CPUTOSifstream& file);
    float4x4                    Interpolate(float sampleTime, bool isLoop = true);
    float4x4                    Interpolate(float sampleTime, CPUTJoint &joint, bool isLoop = true );
    bool						IsValidAnimation();

    void SetParent(CPUTNodeAnimation *pParent)
    {
        SAFE_RELEASE(mpParent);
        if(NULL!=pParent)
        {
            pParent->AddRef();
        }
        mpParent = pParent;
    }
    void AddChild(CPUTNodeAnimation *pChild)
    {
        ASSERT( NULL != pChild, _L("Can't add NULL NodeAnimation node.") );
        if( mpChild )
        {
            mpChild->AddSibling( pChild );
        }
        else
        {
            pChild->AddRef();
            mpChild = pChild;
        }
    }
    void AddSibling(CPUTNodeAnimation *pSibling)
    {
        ASSERT( NULL != pSibling, _L("Can't add NULL NodeAnimation node.") );

        if( mpSibling )
        {
            mpSibling->AddSibling( pSibling );
        }
        else
        {
            mpSibling = pSibling;
            pSibling->AddRef();
        }
    }
    
    CPUTNodeAnimation *GetChild()
    {
        return mpChild;
    }
    CPUTNodeAnimation *GetSibling()
    {
        return mpSibling;
    }
    int ReleaseRecursive()
    {
        // Release the parent
        SAFE_RELEASE(mpParent);

        int refCount;

        // Recursively release our children and siblings
        if( mpChild )
        {
            refCount = mpChild->ReleaseRecursive();
            if( !refCount )
            {
                mpChild = NULL;
            }
        }
        if( mpSibling )
        {
            refCount = mpSibling->ReleaseRecursive();
            if( !refCount )
            {
                mpSibling = NULL;
            }
        }
        return CPUTRefCount::Release();
    }
};

//AnimationSet: Contains all per-node animation for any given scene
class CPUTAnimation:public CPUTRefCount
{
    cString mName;
    CPUTNodeAnimation *mpRootAnimation; 
    std::vector<std::vector<CPUTNodeAnimation *> > mJointAnimationList;

    ~CPUTAnimation()
    {
        if(mpRootAnimation != NULL)
        {
            mpRootAnimation->ReleaseRecursive();
        }
        for(UINT i = 0; i < mJointAnimationList.size(); ++i)
        {
            for(UINT j = 0; j < mJointAnimationList[i].size(); ++j)
            {
                mJointAnimationList[i][j]->Release();
            }
        }
    }

public:
    CPUTAnimation(): mName(_L("")),mpRootAnimation(NULL){}

    CPUTNodeAnimation	 *GetRootAnimation() {return mpRootAnimation;}
    std::vector<CPUTNodeAnimation  *> *FindJointNodeAnimation(const cString &name)
    {
        for(UINT i = 0; i < mJointAnimationList.size();++i)
        {
            if(mJointAnimationList[i][0]->GetTargetName() == name)
                return &mJointAnimationList[i];
        }
        return NULL;
    }
    static CPUTAnimation *CreateAnimation(const cString &file);

};

#endif // __CPUTANIMATION_H__