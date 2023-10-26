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
    static constexpr uint8_t INITIAL_OFFSET_BASED_ADJUSTMENT = 0x1;
    static constexpr uint8_t IGNORE_Z_COMPONENT              = 0x2;
    static constexpr uint8_t IGNORE_X_COMPONENT              = 0x4;
    static constexpr uint8_t IGNORE_Y_COMPONENT              = 0x8;
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

using SceneObjectTargets = std::vector<std::shared_ptr<scene::SceneObject>>;

///------------------------------------------------------------------------------------------------

class TweenAnimation final: public BaseAnimation
{
public:
    TweenAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetPosition, const glm::vec3& targetScale, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f, const std::function<float(const float)> tweeningFunc = math::LinearFunction , const math::TweeningMode tweeningMode = math::TweeningMode::EASE_IN);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    SceneObjectTargets mSceneObjectTargets;
    const std::function<float(const float)> mTweeningFunc;
    const math::TweeningMode mTweeningMode;
    std::vector<glm::vec3> mInitScales;
    std::vector<glm::vec3> mTargetScales;
    std::vector<glm::vec3> mInitPositions;
    std::vector<glm::vec3> mTargetPositions;
};

///------------------------------------------------------------------------------------------------

class TweenRotationAnimation final: public BaseAnimation
{
public:
    TweenRotationAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetPosition, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f, const std::function<float(const float)> tweeningFunc = math::LinearFunction , const math::TweeningMode tweeningMode = math::TweeningMode::EASE_IN);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    SceneObjectTargets mSceneObjectTargets;
    const glm::vec3 mInitRotation;
    const glm::vec3 mTargetRotation;
    const std::function<float(const float)> mTweeningFunc;
    const math::TweeningMode mTweeningMode;
    std::vector<glm::vec3> mSceneObjectRotationOffsets;
};

///------------------------------------------------------------------------------------------------

class BezierCurveAnimation final: public BaseAnimation
{
public:
    BezierCurveAnimation(const SceneObjectTargets& targets, const math::BezierCurve& curve, const float secsDuration, const uint8_t animationFlags = animation_flags::NONE, const float secsDelay = 0.0f);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    SceneObjectTargets mSceneObjectTargets;
    math::BezierCurve mCurve;
    std::vector<glm::vec3> mSceneObjectPositionOffsets;
    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Animations_h */
