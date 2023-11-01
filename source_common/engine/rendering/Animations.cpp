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

TweenPositionScaleAnimation::TweenPositionScaleAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const glm::vec3& targetPosition, const glm::vec3& targetScale, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTarget(sceneObjectTarget)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
    , mInitPosition(mSceneObjectTarget->mPosition)
    , mTargetPosition(targetPosition)
    , mInitScale(mSceneObjectTarget->mScale)
    , mTargetScale(glm::vec3(mInitScale.x * (targetScale.x/mSceneObjectTarget->mScale.x), mInitScale.y * (targetScale.y/mSceneObjectTarget->mScale.y), mInitScale.z))
{
}

AnimationUpdateResult TweenPositionScaleAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    float x = mSceneObjectTarget->mPosition.x;
    float z = mSceneObjectTarget->mPosition.z;
    float y = mSceneObjectTarget->mPosition.y;
    
    mSceneObjectTarget->mPosition = math::Lerp(mInitPosition, mTargetPosition, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    mSceneObjectTarget->mPosition.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : mSceneObjectTarget->mPosition.z;
    mSceneObjectTarget->mPosition.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : mSceneObjectTarget->mPosition.x;
    mSceneObjectTarget->mPosition.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : mSceneObjectTarget->mPosition.y;
    
    mSceneObjectTarget->mScale = math::Lerp(mInitScale, mTargetScale, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

TweenRotationAnimation::TweenRotationAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const glm::vec3& targetRotation, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTarget(sceneObjectTarget)
    , mInitRotation(sceneObjectTarget->mRotation)
    , mTargetRotation(targetRotation)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{    
}

AnimationUpdateResult TweenRotationAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    float x = mSceneObjectTarget->mPosition.x;
    float z = mSceneObjectTarget->mPosition.z;
    float y = mSceneObjectTarget->mPosition.y;
    
    mSceneObjectTarget->mRotation = math::Lerp(mInitRotation, mTargetRotation, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    mSceneObjectTarget->mRotation.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : mSceneObjectTarget->mRotation.z;
    mSceneObjectTarget->mRotation.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : mSceneObjectTarget->mRotation.x;
    mSceneObjectTarget->mRotation.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : mSceneObjectTarget->mRotation.y;
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

TweenAlphaAnimation::TweenAlphaAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const float targetAlpha, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTarget(sceneObjectTarget)
    , mInitAlpha(sceneObjectTarget->mShaderFloatUniformValues.at(game_constants::CUSTOM_ALPHA_UNIFORM_NAME))
    , mTargetAlpha(targetAlpha)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{
    assert(!IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT));
}

AnimationUpdateResult TweenAlphaAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    mSceneObjectTarget->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Lerp(mInitAlpha, mTargetAlpha, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

BezierCurveAnimation::BezierCurveAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const math::BezierCurve& curve, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectTarget(sceneObjectTarget)
    , mCurve(curve)
{
}

AnimationUpdateResult BezierCurveAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    float x = mSceneObjectTarget->mPosition.x;
    float z = mSceneObjectTarget->mPosition.z;
    float y = mSceneObjectTarget->mPosition.y;
    
    mSceneObjectTarget->mPosition = mCurve.ComputePointForT(mAnimationT);
    mSceneObjectTarget->mPosition.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : mSceneObjectTarget->mPosition.z;
    mSceneObjectTarget->mPosition.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : mSceneObjectTarget->mPosition.x;
    mSceneObjectTarget->mPosition.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : mSceneObjectTarget->mPosition.y;
    
    return animationUpdateResult;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
