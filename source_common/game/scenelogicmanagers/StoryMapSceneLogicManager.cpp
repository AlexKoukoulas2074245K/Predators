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
#include <game/AnimatedButton.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/scenelogicmanagers/StoryMapSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId SETTINGS_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("settings_button");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("background");

static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";

static const glm::vec2 MAP_SWIPE_X_BOUNDS = {-0.5f, 0.5f};
static const glm::vec2 MAP_SWIPE_Y_BOUNDS = {-0.5f, 0.5f};

static const glm::vec3 SETTINGS_BUTTON_POSITION = {0.145f, 0.091f, 0.1f};
static const glm::vec3 SETTINGS_BUTTON_SCALE = {0.06f, 0.06f, 0.06f};

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

StoryMapSceneLogicManager::StoryMapSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

StoryMapSceneLogicManager::~StoryMapSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    RegisterForEvents();
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        SETTINGS_BUTTON_POSITION,
        SETTINGS_BUTTON_SCALE,
        SETTINGS_ICON_TEXTURE_FILE_NAME,
        SETTINGS_BUTTON_SCENE_OBJECT_NAME,
        [=](){ OnSettingsButtonPressed(); },
        *scene,
        scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE
    ));
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    
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
            
            auto backgroundSceneObject = scene->FindSceneObject(BACKGROUND_SCENE_OBJECT_NAME);
            
            auto nextPosition = backgroundSceneObject->mPosition;
            
            nextPosition.x += targetDx;
            nextPosition.x = math::Max(MAP_SWIPE_X_BOUNDS.s, math::Min(MAP_SWIPE_X_BOUNDS.t, nextPosition.x));
            
            nextPosition.y += targetDy;
            nextPosition.y = math::Max(MAP_SWIPE_Y_BOUNDS.s, math::Min(MAP_SWIPE_Y_BOUNDS.t, nextPosition.y));
                
            backgroundSceneObject->mPosition = nextPosition;
            
            mSwipeCurrentPos = glm::vec3(worldTouchPos.x, worldTouchPos.y, 0.0f);
        }
    }
    else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
    {
        ResetSwipeData();
    }
    
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    eventSystem.RegisterForEvent<events::PopSceneModalEvent>(this, &StoryMapSceneLogicManager::OnPopSceneModal);
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &StoryMapSceneLogicManager::OnWindowResize);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::OnPopSceneModal(const events::PopSceneModalEvent&)
{
    ResetSwipeData();
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::STORY_MAP_SCENE)->RecalculatePositionOfEdgeSnappingSceneObjects();
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

void StoryMapSceneLogicManager::OnSettingsButtonPressed()
{
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(SETTINGS_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------
