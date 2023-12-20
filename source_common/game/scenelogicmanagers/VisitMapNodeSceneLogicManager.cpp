///------------------------------------------------------------------------------------------------
///  VisitMapNodeSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/VisitMapNodeSceneLogicManager.h>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId VISIT_MAP_NODE_SCENE_NAME = strutils::StringId("visit_map_node_scene");
static const strutils::StringId VISIT_BUTTON_NAME = strutils::StringId("visit_button");
static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");

static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};

static const float VISIT_BUTTON_HOR_DISTANCE_FROM_NODE = 0.1f;
static const float VISIT_BUTTON_Y_OFFSET_FROM_NODE = 0.05f;

static const float BACK_BUTTON_HOR_DISTANCE_FROM_NODE = 0.1f;
static const float BACK_BUTTON_Y_OFFSET_FROM_NODE = -0.03f;
static const float BUTTON_Z = 24.0f;

static const float FADE_IN_OUT_DURATION_SECS = 0.5f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    VISIT_MAP_NODE_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& VisitMapNodeSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

VisitMapNodeSceneLogicManager::VisitMapNodeSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

VisitMapNodeSceneLogicManager::~VisitMapNodeSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene> scene)
{
    scene->GetCamera().SetPosition(CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mPreviousScene)->GetCamera().GetPosition());
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mTransitioning = false;
    
    scene->RemoveAllSceneObjectsButTheOnesNamed(STATIC_SCENE_ELEMENTS);
    mAnimatedButtons.clear();
    
    auto& targetNodePosition = ProgressionDataRepository::GetInstance().GetSelectedStoryMapNodePosition();
    auto& previousSceneCameraPosition = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mPreviousScene)->GetCamera().GetPosition();
    
    auto visitButtonPosition = targetNodePosition;
    visitButtonPosition.x += (targetNodePosition.x < previousSceneCameraPosition.x ? VISIT_BUTTON_HOR_DISTANCE_FROM_NODE : -1.5f * VISIT_BUTTON_HOR_DISTANCE_FROM_NODE);
    visitButtonPosition.y += VISIT_BUTTON_Y_OFFSET_FROM_NODE;
    visitButtonPosition.z = BUTTON_Z;
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        visitButtonPosition,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Visit",
        VISIT_BUTTON_NAME,
        [=]()
        {
            mTransitioning = true;
        },
        *scene
    ));
    
    auto backButtonPosition = targetNodePosition;
    backButtonPosition.x += (targetNodePosition.x < previousSceneCameraPosition.x ? BACK_BUTTON_HOR_DISTANCE_FROM_NODE : -1.5f * BACK_BUTTON_HOR_DISTANCE_FROM_NODE);
    backButtonPosition.y += BACK_BUTTON_Y_OFFSET_FROM_NODE;
    backButtonPosition.z = BUTTON_Z;
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        backButtonPosition,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Back",
        BACK_BUTTON_NAME,
        [=]()
        {
            mTransitioning = true;
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
        },
        *scene
    ));
    
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        sceneObject->mInvisible = false;
        
        if (!STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS), [=]()
        {
        });
    }
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioning)
    {
        return;
    }
    
    // Animated buttons
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, FADE_IN_OUT_DURATION_SECS), [=]()
        {
            sceneObject->mInvisible = true;
        });
    }
    
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto previousScene = sceneManager.FindScene(mPreviousScene);
    
    animationManager.StopAnimation(game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenValueAnimation>(previousScene->GetUpdateTimeSpeedFactor(), 1.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
}

///------------------------------------------------------------------------------------------------
