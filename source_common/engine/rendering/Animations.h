///------------------------------------------------------------------------------------------------
///  Animations.h                                                                                          
///  engine/rendering                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Animations_h
#define Animations_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <functional>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { struct SceneObject; }

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
    BaseAnimation(const float secsDuration, const float secsDelay = 0.0f);
    virtual ~BaseAnimation() = default;
    virtual AnimationUpdateResult VUpdate(const float dtMillis);
    
protected:
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
    TweenAnimation(const SceneObjectTargets& sceneObjectTargets, const glm::vec3& targetPosition, const float secsDuration, const bool offsetsBasedAdjustment, const float secsDelay = 0.0f, const std::function<float(const float)> tweeningFunc = math::LinearFunction , const math::TweeningMode tweeningMode = math::TweeningMode::EASE_IN);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    SceneObjectTargets mSceneObjectTargets;
    const glm::vec3 mInitPosition;
    const glm::vec3 mTargetPosition;
    const std::function<float(const float)> mTweeningFunc;
    const math::TweeningMode mTweeningMode;
    std::vector<glm::vec3> mSceneObjectOffsets;
};

///------------------------------------------------------------------------------------------------

class BezierCurveAnimation final: public BaseAnimation
{
public:
    BezierCurveAnimation(const SceneObjectTargets& targets, const math::BezierCurve& curve, const float secsDuration, const bool offsetsBasedAdjustment, const float secsDelay = 0.0f);
    AnimationUpdateResult VUpdate(const float dtMillis);
    
private:
    SceneObjectTargets mSceneObjectTargets;
    math::BezierCurve mCurve;
    std::vector<glm::vec3> mSceneObjectOffsets;
    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Animations_h */
