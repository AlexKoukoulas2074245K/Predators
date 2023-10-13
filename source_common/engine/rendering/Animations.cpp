///------------------------------------------------------------------------------------------------
///  Animations.cpp                                                                                        
///  engine/rendering                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Animations.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

#define IS_FLAG_SET(flag) ((mAnimationFlags & flag) != 0)

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

BaseAnimation::BaseAnimation(const uint8_t animationFlags, const float secsDuration, const float secsDelay /* = 0.0f */)
    : mAnimationFlags(animationFlags)
    , mSecsDuration(secsDuration)
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

TweenAnimation::TweenAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetPosition, const glm::vec3& targetScale, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mInitPosition(sceneObjectTargets.front()->mPosition)
    , mTargetPosition(targetPosition)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{
    float mScaleRatioX = targetScale.x/sceneObjectTargets.front()->mScale.y;
    float mScaleRatioY = targetScale.x/sceneObjectTargets.front()->mScale.y;
        
    for (auto sceneObject: sceneObjectTargets)
    {
        if (IS_FLAG_SET(animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT))
        {
            mSceneObjectOffsets.emplace_back(sceneObject->mPosition - sceneObjectTargets.front()->mPosition);
        }
        else
        {
            mSceneObjectOffsets.emplace_back(glm::vec3(0.0f));
        }
        
        mSceneObjectOffsets.back().x *= mScaleRatioX;
        mSceneObjectOffsets.back().y *= mScaleRatioY;
        
        mInitScales.emplace_back(sceneObject->mScale);
        mTargetScales.emplace_back(glm::vec3(sceneObject->mScale.x * mScaleRatioX, sceneObject->mScale.y * mScaleRatioY, 1.0f));
    }
}

AnimationUpdateResult TweenAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (size_t i = 0; i < mSceneObjectTargets.size(); ++i)
    {
        auto sceneObject = mSceneObjectTargets.at(i);
        auto offset = mSceneObjectOffsets.at(i);
        
        float x = sceneObject->mPosition.x;
        float z = sceneObject->mPosition.z;
        float y = sceneObject->mPosition.y;
        
        sceneObject->mPosition = math::Lerp(mInitPosition, mTargetPosition, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
        sceneObject->mPosition.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : sceneObject->mPosition.z + offset.z;
        sceneObject->mPosition.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : sceneObject->mPosition.x + offset.x;
        sceneObject->mPosition.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : sceneObject->mPosition.y + offset.y;
        
        sceneObject->mScale = math::Lerp(mInitScales[i], mTargetScales[i], math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    }
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

BezierCurveAnimation::BezierCurveAnimation(const SceneObjectTargets& sceneObjectTargets, const math::BezierCurve& curve, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mCurve(curve)
{
    for (auto sceneObject: sceneObjectTargets)
    {
        if (IS_FLAG_SET(animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT))
        {
            mSceneObjectOffsets.emplace_back(sceneObject->mPosition - sceneObjectTargets.front()->mPosition);
        }
        else
        {
            mSceneObjectOffsets.emplace_back(glm::vec3(0.0f));
        }
    }
}

AnimationUpdateResult BezierCurveAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (size_t i = 0; i < mSceneObjectTargets.size(); ++i)
    {
        auto sceneObject = mSceneObjectTargets.at(i);
        auto offset = mSceneObjectOffsets.at(i);
        
        float x = sceneObject->mPosition.x;
        float z = sceneObject->mPosition.z;
        float y = sceneObject->mPosition.y;
        
        sceneObject->mPosition = mCurve.ComputePointForT(mAnimationT);
        sceneObject->mPosition.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : sceneObject->mPosition.z + offset.z;
        sceneObject->mPosition.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : sceneObject->mPosition.x + offset.x;
        sceneObject->mPosition.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : sceneObject->mPosition.y + offset.y;
    }
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
