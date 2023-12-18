///------------------------------------------------------------------------------------------------
///  AnimatedStatContainer.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/AnimatedButton.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObjectUtils.h>

///------------------------------------------------------------------------------------------------

static const float INTERACTION_ANIMATION_DURATION = 0.1f;
static const float INTERACTION_ANIMATION_SCALE_FACTOR = 0.5f;
static const float SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 28.5f;

///------------------------------------------------------------------------------------------------

AnimatedButton::AnimatedButton
(
    const glm::vec3& position,
    const glm::vec3& scale,
    const std::string& textureFilename,
    const strutils::StringId& buttonName,
    std::function<void()> onPressCallback,
    scene::Scene& scene,
    scene::SnapToEdgeBehavior snapToEdgeBehavior /* = scene::SnapToEdgeBehavior::NONE */
)
    : mScene(scene)
    , mOnPressCallback(onPressCallback)
    , mAnimating(false)
{
    mSceneObject = scene.CreateSceneObject(buttonName);
    
    mSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    mSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + textureFilename);
    mSceneObject->mPosition = position;
    mSceneObject->mScale = scale;
    mSceneObject->mSnapToEdgeBehavior = snapToEdgeBehavior;
    mSceneObject->mSnapToEdgeScaleOffsetFactor = mSceneObject->mScale.x * SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
}

///------------------------------------------------------------------------------------------------

AnimatedButton::AnimatedButton
(
    const glm::vec3& position,
    const glm::vec3& scale,
    const strutils::StringId& fontName,
    const std::string& text,
    const strutils::StringId& buttonName,
    std::function<void()> onPressCallback,
    scene::Scene& scene,
    scene::SnapToEdgeBehavior snapToEdgeBehavior /* = scene::SnapToEdgeBehavior::NONE */
)
    : mScene(scene)
    , mOnPressCallback(onPressCallback)
    , mAnimating(false)
{
    mSceneObject = scene.CreateSceneObject(buttonName);
    
    scene::TextSceneObjectData textData;
    textData.mFontName = fontName;
    textData.mText = text;
    
    mSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    mSceneObject->mSceneObjectTypeData = std::move(textData);
    mSceneObject->mPosition = position;
    mSceneObject->mScale = scale;
    mSceneObject->mSnapToEdgeBehavior = snapToEdgeBehavior;
    mSceneObject->mSnapToEdgeScaleOffsetFactor = mSceneObject->mScale.x;
}

///------------------------------------------------------------------------------------------------

void AnimatedButton::Update(const float)
{
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(mScene.GetCamera().GetViewMatrix(), mScene.GetCamera().GetProjMatrix());
    
    auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*mSceneObject);
    bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
    
    if (cursorInSceneObject && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && !mAnimating)
    {
        mAnimating = true;
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto originalScale = mSceneObject->mScale;
        animationManager.StartAnimation(std::make_unique<rendering::PulseAnimation>(mSceneObject, INTERACTION_ANIMATION_SCALE_FACTOR, INTERACTION_ANIMATION_DURATION), [=]()
        {
            mSceneObject->mScale = originalScale;
            mAnimating = false;
        });
        
        // Dummy animation to invoke callback mid-way pulse animation
        animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(mSceneObject, mSceneObject->mRotation, INTERACTION_ANIMATION_DURATION/2, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=](){ mOnPressCallback(); });
    }
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<scene::SceneObject> AnimatedButton::GetSceneObject() { return mSceneObject; }

///------------------------------------------------------------------------------------------------
