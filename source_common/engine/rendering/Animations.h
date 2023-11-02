///------------------------------------------------------------------------------------------------
///  Animations.h                                                                                          
///  engine/rendering                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Animations_h
#define Animations_h

///------------------------------------------------------------------------------------------------

#include <cstdint>
#include <engine/utils/MathUtils.h>
#include <functional>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { struct SceneObject; }

///------------------------------------------------------------------------------------------------

namespace animation_flags
{
    static constexpr uint8_t NONE                            = 0x0;
    static constexpr uint8_t IGNORE_Z_COMPONENT              = 0x1;
    static constexpr uint8_t IGNORE_X_COMPONENT              = 0x2;
    static constexpr uint8_t IGNORE_Y_COMPONENT              = 0x4;
}

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

enum class AnimationUpdateResult
{
    ONGOING,
    FINISHED
};

///------------------------------------------------------------------------------------------------

class IAnimation
{
public:
    virtual ~IAnimation() = default;
    virtual AnimationUpdateResult VUpdate(const float dtMillis) = 0;
};

///------------------------------------------------------------------------------------------------

class BaseAnimation: public IAnimation
{
public:
    BaseAnimation(const uint8_t animationFlags, const float secsDuration, const float secsDelay = 0.0f);
    virtual ~BaseAnimation() = default;
    virtual AnimationUpdateResult VUpdate(const float dtMillis);
    
protected:
    const uint8_t mAnimationFlags;
    const float mSecsDuration;
    float mSecsDelay;
    float mSecsAccumulator;
    float mAnimationT;
};

///------------------------------------------------------------------------------------------------

class TweenPositionScaleAnimation final: public BaseAnimation
{
public:
    TweenPositionScaleAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const glm::vec3& targetPosition, const glm::vec3& targetScale, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f, const std::function<float(const float)> tweeningFunc = math::LinearFunction , const math::TweeningMode tweeningMode = math::TweeningMode::EASE_IN);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    std::shared_ptr<scene::SceneObject> mSceneObjectTarget;
    const std::function<float(const float)> mTweeningFunc;
    const math::TweeningMode mTweeningMode;
    const glm::vec3 mInitPosition;
    const glm::vec3 mTargetPosition;
    const glm::vec3 mInitScale;
    const glm::vec3 mTargetScale;
    
};

///------------------------------------------------------------------------------------------------

class TweenRotationAnimation final: public BaseAnimation
{
public:
    TweenRotationAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const glm::vec3& targetRotation, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f, const std::function<float(const float)> tweeningFunc = math::LinearFunction , const math::TweeningMode tweeningMode = math::TweeningMode::EASE_IN);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    std::shared_ptr<scene::SceneObject> mSceneObjectTarget;
    const glm::vec3 mInitRotation;
    const glm::vec3 mTargetRotation;
    const std::function<float(const float)> mTweeningFunc;
    const math::TweeningMode mTweeningMode;
};

///------------------------------------------------------------------------------------------------

// Expects the custom_alpha float uniform to have been set prior to the creation of this animation type
class TweenAlphaAnimation final: public BaseAnimation
{
public:
    TweenAlphaAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const float targetAlpha, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f, const std::function<float(const float)> tweeningFunc = math::LinearFunction , const math::TweeningMode tweeningMode = math::TweeningMode::EASE_IN);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    std::shared_ptr<scene::SceneObject> mSceneObjectTarget;
    const float mInitAlpha;
    const float mTargetAlpha;
    const std::function<float(const float)> mTweeningFunc;
    const math::TweeningMode mTweeningMode;
};

///------------------------------------------------------------------------------------------------

class BezierCurveAnimation final: public BaseAnimation
{
public:
    BezierCurveAnimation(std::shared_ptr<scene::SceneObject> sceneObjectTarget, const math::BezierCurve& curve, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    std::shared_ptr<scene::SceneObject> mSceneObjectTarget;
    math::BezierCurve mCurve;
    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Animations_h */
