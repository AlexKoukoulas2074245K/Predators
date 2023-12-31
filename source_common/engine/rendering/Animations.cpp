///------------------------------------------------------------------------------------------------
///  Animations.cpp                                                                                        
///  engine/rendering                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Animations.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>

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
    else if (mSecsDuration > 0.0f)
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
    
    return (mAnimationT < 1.0f || mSecsDuration < 0.0f) ? AnimationUpdateResult::ONGOING : AnimationUpdateResult::FINISHED;
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
    assert(!IS_FLAG_SET(animation_flags::ANIMATE_CONTINUOUSLY));
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

std::shared_ptr<scene::SceneObject> TweenPositionScaleAnimation::VGetSceneObject()
{
    return mSceneObjectTarget;
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
    assert(!IS_FLAG_SET(animation_flags::ANIMATE_CONTINUOUSLY));
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

std::shared_ptr<scene::SceneObject> TweenRotationAnimation::VGetSceneObject()
{
    return mSceneObjectTarget;
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
    assert(!IS_FLAG_SET(animation_flags::ANIMATE_CONTINUOUSLY));
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

std::shared_ptr<scene::SceneObject> TweenAlphaAnimation::VGetSceneObject()
{
    return mSceneObjectTarget;
}

///------------------------------------------------------------------------------------------------

TweenValueAnimation::TweenValueAnimation(float& value, const float targetValue, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mValue(value)
    , mInitValue(value)
    , mTargetValue(targetValue)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
{
    assert(!IS_FLAG_SET(animation_flags::ANIMATE_CONTINUOUSLY));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT));
}

AnimationUpdateResult TweenValueAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    mValue = math::Lerp(mInitValue, mTargetValue, math::TweenValue(mAnimationT, mTweeningFunc, mTweeningMode));
    return animationUpdateResult;
}

std::shared_ptr<scene::SceneObject> TweenValueAnimation::VGetSceneObject()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

PulseAnimation::PulseAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const float scaleFactor, const float secsPulseDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> tweeningFunc /* = math::LinearFunction */, const math::TweeningMode tweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, (animationFlags & animation_flags::ANIMATE_CONTINUOUSLY) != 0 ? -1.0f : secsPulseDuration * 2.0f, secsDelay)
    , mSceneObjectTarget(sceneObjectTarget)
    , mSecsPulseDuration(secsPulseDuration)
    , mInitScale(sceneObjectTarget->mScale)
    , mTargetScale(sceneObjectTarget->mScale * scaleFactor)
    , mTweeningFunc(tweeningFunc)
    , mTweeningMode(tweeningMode)
    , mSecsPulseAccum(0.0f)
    , mScalingUp(true)
{
    assert(!IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT));
}

AnimationUpdateResult PulseAnimation::VUpdate(const float dtMillis)
{
    mSecsPulseAccum += dtMillis/1000.0f;
    if (mSecsPulseAccum >= mSecsPulseDuration)
    {
        mSecsPulseAccum -= mSecsPulseDuration;
        mScalingUp = !mScalingUp;
    }
    
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    if (mScalingUp)
    {
        mSceneObjectTarget->mScale = math::Lerp(mInitScale, mTargetScale, math::TweenValue(mSecsPulseAccum/mSecsPulseDuration, mTweeningFunc, mTweeningMode));
    }
    else
    {
        mSceneObjectTarget->mScale = math::Lerp(mTargetScale, mInitScale, math::TweenValue(mSecsPulseAccum/mSecsPulseDuration, mTweeningFunc, mTweeningMode));
    }
    
    if (animationUpdateResult == AnimationUpdateResult::FINISHED)
    {
        mSceneObjectTarget->mScale = mInitScale;
    }
    return animationUpdateResult;
}

std::shared_ptr<scene::SceneObject> PulseAnimation::VGetSceneObject()
{
    return mSceneObjectTarget;
}

///------------------------------------------------------------------------------------------------

BouncePositionAnimation::BouncePositionAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const glm::vec3& positionOffsetSpeed, const float secsBounceDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */, const std::function<float(const float)> /* = math::LinearFunction */, const math::TweeningMode /* = math::TweeningMode::EASE_IN */)
    : BaseAnimation(animationFlags, (animationFlags & animation_flags::ANIMATE_CONTINUOUSLY) != 0 ? -1.0f : secsBounceDuration * 2.0f, secsDelay)
    , mSceneObjectTarget(sceneObjectTarget)
    , mSecsBounceDuration(secsBounceDuration)
    , mInitPosition(sceneObjectTarget->mPosition)
    , mPositionOffsetSpeed(positionOffsetSpeed)
    , mSecsBounceAccum(secsDelay)
    , mMovingUp(true)
{
    assert(!IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT));
    assert(!IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT));
}

AnimationUpdateResult BouncePositionAnimation::VUpdate(const float dtMillis)
{
    mSecsBounceAccum += dtMillis/1000.0f;
    if (mSecsBounceAccum >= mSecsBounceDuration)
    {
        mSecsBounceAccum -= mSecsBounceDuration;
        mMovingUp = !mMovingUp;
    }
    
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    if (mMovingUp)
    {
        mSceneObjectTarget->mPosition += mPositionOffsetSpeed * dtMillis;
    }
    else
    {
        mSceneObjectTarget->mPosition -= mPositionOffsetSpeed * dtMillis;
    }
    
    if (animationUpdateResult == AnimationUpdateResult::FINISHED)
    {
        mSceneObjectTarget->mPosition = mInitPosition;
    }
    return animationUpdateResult;
}

std::shared_ptr<scene::SceneObject> BouncePositionAnimation::VGetSceneObject()
{
    return mSceneObjectTarget;
}

///------------------------------------------------------------------------------------------------

BezierCurveAnimation::BezierCurveAnimation(glm::vec3& sceneObjectPosition, const math::BezierCurve& curve, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */)
    : BaseAnimation(animationFlags, secsDuration, secsDelay)
    , mSceneObjectPosition(sceneObjectPosition)
    , mCurve(curve)
{
    assert(!IS_FLAG_SET(animation_flags::ANIMATE_CONTINUOUSLY));
}

BezierCurveAnimation::BezierCurveAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const math::BezierCurve& curve, const float secsDuration, const uint8_t animationFlags /* = animation_flags::NONE */, const float secsDelay /* = 0.0f */)
    : BezierCurveAnimation(sceneObjectTarget->mPosition, curve, secsDuration, animationFlags, secsDelay)
{
}

AnimationUpdateResult BezierCurveAnimation::VUpdate(const float dtMillis)
{
    auto animationUpdateResult = BaseAnimation::VUpdate(dtMillis);
    
    float x = mSceneObjectPosition.x;
    float z = mSceneObjectPosition.z;
    float y = mSceneObjectPosition.y;
    
    mSceneObjectPosition = mCurve.ComputePointForT(mAnimationT);
    mSceneObjectPosition.z = IS_FLAG_SET(animation_flags::IGNORE_Z_COMPONENT) ? z : mSceneObjectPosition.z;
    mSceneObjectPosition.x = IS_FLAG_SET(animation_flags::IGNORE_X_COMPONENT) ? x : mSceneObjectPosition.x;
    mSceneObjectPosition.y = IS_FLAG_SET(animation_flags::IGNORE_Y_COMPONENT) ? y : mSceneObjectPosition.y;
    
    return animationUpdateResult;
}

std::shared_ptr<scene::SceneObject> BezierCurveAnimation::VGetSceneObject()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
