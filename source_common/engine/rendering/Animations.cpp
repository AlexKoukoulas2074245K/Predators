///------------------------------------------------------------------------------------------------
///  Animations.cpp                                                                                        
///  engine/rendering                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Animations.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

BaseAnimation::BaseAnimation(const float secsDuration, const float secsDelay /* = 0.0f */)
    : mSecsDuration(secsDuration)
    , mSecsDelay(secsDelay)
    , mSecsAccumulator(0.0f)
    , mAnimationT(0.0f)
{
}

AnimationUpdateResult BaseAnimation::VUpdate(const float dtMillis)
{
    if (mSecsDelay > 0.0f)
    {
        mSecsDelay -= dtMillis/1000.0f;
    }
    else
    {
        mSecsAccumulator += dtMillis/1000.0f;
        if (mSecsAccumulator > mSecsDuration)
        {
            mSecsAccumulator = mSecsDuration;
            mAnimationT = 1.0f;
        }
        else
        {
            mAnimationT = mSecsAccumulator/mSecsDuration;
        }
    }
    
    return mAnimationT < 1.0f ? AnimationUpdateResult::ONGOING : AnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------

TweenAnimation::TweenAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetPosition, const float secsDuration, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mInitPosition(sceneObjectTargets.front()->mPosition)
    , mTargetPosition(targetPosition)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{
}

AnimationUpdateResult TweenAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (auto sceneObject: mSceneObjectTargets)
    {
        sceneObject->mPosition = math::Lerp(mInitPosition, mTargetPosition, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    }
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

BezierCurveAnimation::BezierCurveAnimation(const SceneObjectTargets& sceneObjectTargets, const math::BezierCurve& curve, const float secsDuration, const float secsDelay /* = 0.0f */)
    : BaseAnimation(secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mCurve(curve)
{
}

AnimationUpdateResult BezierCurveAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (auto sceneObject: mSceneObjectTargets)
    {
        sceneObject->mPosition = mCurve.ComputePointForT(mAnimationT);
    }
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
