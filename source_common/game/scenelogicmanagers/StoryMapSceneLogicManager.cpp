///------------------------------------------------------------------------------------------------
///  StoryMapSceneLogicManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/utils/Logging.h>
#include <game/GameConstants.h>
#include <game/scenelogicmanagers/StoryMapSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const float MAP_SWIPE_INTERPOLATION_DURATION_SECS = 0.025f;
static const glm::vec2 MAP_SWIPE_X_BOUNDS = {-0.5f, 0.5f};
static const glm::vec2 MAP_SWIPE_Y_BOUNDS = {-0.5f, 0.5f};

///------------------------------------------------------------------------------------------------

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::STORY_MAP_SCENE
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& StoryMapSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(scene->GetCamera().GetViewMatrix(), scene->GetCamera().GetProjMatrix());
    
    if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
    {
        mHasStartedSwipe = true;
        mSwipeStartPos = glm::vec3(worldTouchPos.x, worldTouchPos.y, 0.0f);
        mSwipeCurrentPos = mSwipeStartPos;
        mSwipeDurationMillis = 0.0f;
    }
    else if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
    {
        if (mHasStartedSwipe)
        {
            mSwipeDurationMillis += dtMillis;
            
            float targetDx = (worldTouchPos.x - mSwipeCurrentPos.x);
            float targetDy = (worldTouchPos.y - mSwipeCurrentPos.y);
            
            for (auto& sceneObject: scene->GetSceneObjects())
            {
                auto nextPosition = sceneObject->mPosition;
                
                nextPosition.x += targetDx;
                nextPosition.x = math::Max(MAP_SWIPE_X_BOUNDS.s, math::Min(MAP_SWIPE_X_BOUNDS.t, nextPosition.x));
                
                nextPosition.y += targetDy;
                nextPosition.y = math::Max(MAP_SWIPE_Y_BOUNDS.s, math::Min(MAP_SWIPE_Y_BOUNDS.t, nextPosition.y));
                
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(sceneObject, nextPosition, sceneObject->mScale, MAP_SWIPE_INTERPOLATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            }
             mSwipeCurrentPos = glm::vec3(worldTouchPos.x, worldTouchPos.y, 0.0f);
        }
    }
    else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
    {
        ResetSwipeData();
    }
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::ResetSwipeData()
{
    mHasStartedSwipe = false;
    mSwipeDurationMillis = 0.0f;
    mSwipeVelocityDelta = 0.0f;
    mSwipeDelta = 0.0f;
}
    
///------------------------------------------------------------------------------------------------

