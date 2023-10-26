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

TweenPositionScaleAnimation::TweenPositionScaleAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetPosition, const glm::vec3& targetScale, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{
    float scaleRatioX = targetScale.x/sceneObjectTargets.front()->mScale.x;
    float scaleRatioY = targetScale.y/sceneObjectTargets.front()->mScale.y;
        
    for (auto sceneObject: sceneObjectTargets)
    {
        glm::vec3 sceneObjectOffset(0.0f);
        if (IS_FLAG_SET(animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT))
        {
            sceneObjectOffset = sceneObject->mPosition - sceneObjectTargets.front()->mPosition;
        }

        mInitPositions.emplace_back(sceneObject->mPosition);
        mTargetPositions.emplace_back(targetPosition + (glm::vec3(sceneObjectOffset.x * scaleRatioX, sceneObjectOffset.y * scaleRatioY, sceneObjectOffset.z)));
        
        mInitScales.emplace_back(sceneObject->mScale);
        mTargetScales.emplace_back(glm::vec3(sceneObject->mScale.x * scaleRatioX, sceneObject->mScale.y * scaleRatioY, 1.0f));
    }
}

AnimationUpdateResult TweenPositionScaleAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (size_t i = 0; i < mSceneObjectTargets.size(); ++i)
    {
        auto sceneObject = mSceneObjectTargets.at(i);
        
        float x = sceneObject->mPosition.x;
        float z = sceneObject->mPosition.z;
        float y = sceneObject->mPosition.y;
        
        sceneObject->mPosition = math::Lerp(mInitPositions[i], mTargetPositions[i], math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
        sceneObject->mPosition.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : sceneObject->mPosition.z;
        sceneObject->mPosition.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : sceneObject->mPosition.x;
        sceneObject->mPosition.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : sceneObject->mPosition.y;
        
        sceneObject->mScale = math::Lerp(mInitScales[i], mTargetScales[i], math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    }
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

TweenRotationAnimation::TweenRotationAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetRotation, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mInitRotation(sceneObjectTargets.front()->mRotation)
    , mTargetRotation(targetRotation)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{    
    for (auto sceneObject: sceneObjectTargets)
    {
        if (IS_FLAG_SET(animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT))
        {
            mSceneObjectRotationOffsets.emplace_back(sceneObject->mRotation - sceneObjectTargets.front()->mRotation);
        }
        else
        {
            mSceneObjectRotationOffsets.emplace_back(glm::vec3(0.0f));
        }
    }
}

AnimationUpdateResult TweenRotationAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (size_t i = 0; i < mSceneObjectTargets.size(); ++i)
    {
        auto sceneObject = mSceneObjectTargets.at(i);
        auto offset = mSceneObjectRotationOffsets.at(i);
        
        float x = sceneObject->mPosition.x;
        float z = sceneObject->mPosition.z;
        float y = sceneObject->mPosition.y;
        
        sceneObject->mRotation = math::Lerp(mInitRotation, mTargetRotation, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
        sceneObject->mRotation.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : sceneObject->mRotation.z + offset.z;
        sceneObject->mRotation.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : sceneObject->mRotation.x + offset.x;
        sceneObject->mRotation.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : sceneObject->mRotation.y + offset.y;
    }
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

TweenAlphaAnimation::TweenAlphaAnimation(const SceneObjectTargets& sceneObjectTargets, const float targetAlpha, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTargets(sceneObjectTargets)
    , mInitAlpha(sceneObjectTargets.front()->mShaderFloatUniformValues.at(game_constants::CUSTOM_ALPHA_UNIFORM_NAME))
    , mTargetAlpha(targetAlpha)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{
    assert(!IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT));
    
    for (auto sceneObject: sceneObjectTargets)
    {
        if (IS_FLAG_SET(animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT))
        {
            mSceneObjectAlphaOffsets.emplace_back(sceneObject->mShaderFloatUniformValues.at(game_constants::CUSTOM_ALPHA_UNIFORM_NAME) - sceneObjectTargets.front()->mShaderFloatUniformValues.at(game_constants::CUSTOM_ALPHA_UNIFORM_NAME));
        }
        else
        {
            mSceneObjectAlphaOffsets.emplace_back(0.0f);
        }
    }
}

AnimationUpdateResult TweenAlphaAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (size_t i = 0; i < mSceneObjectTargets.size(); ++i)
    {
        auto sceneObject = mSceneObjectTargets.at(i);
        auto offset = mSceneObjectAlphaOffsets.at(i);
        
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Lerp(mInitAlpha, mTargetAlpha, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode)) + offset;
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
            mSceneObjectPositionOffsets.emplace_back(sceneObject->mPosition - sceneObjectTargets.front()->mPosition);
        }
        else
        {
            mSceneObjectPositionOffsets.emplace_back(glm::vec3(0.0f));
        }
    }
}

AnimationUpdateResult BezierCurveAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    for (size_t i = 0; i < mSceneObjectTargets.size(); ++i)
    {
        auto sceneObject = mSceneObjectTargets.at(i);
        auto offset = mSceneObjectPositionOffsets.at(i);
        
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
