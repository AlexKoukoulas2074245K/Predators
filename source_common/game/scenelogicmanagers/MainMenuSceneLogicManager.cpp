///------------------------------------------------------------------------------------------------
///  MainMenuSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/MainMenuSceneLogicManager.h>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId MAIN_MENU_SCENE_NAME = strutils::StringId("main_menu_scene");
static const strutils::StringId PRACTICE_BATTLE_BUTTON_NAME = strutils::StringId("practice_battle_button");
static const strutils::StringId QUIT_BUTTON_NAME = strutils::StringId("quit_button");
static const strutils::StringId NORMAL_BATTLE_MODE_BUTTON_NAME = strutils::StringId("normal_battle_mode_button");
static const strutils::StringId AI_DEMO_BATTLE_MODE_BUTTON_NAME = strutils::StringId("ai_demo_battle_mode_button");
static const strutils::StringId START_BATTLE_BUTTON_NAME = strutils::StringId("start_battle_button");
static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId TITLE_SCENE_OBJECT_NAME = strutils::StringId("predators_title");

static const glm::vec3 BUTTON_SCALE = {0.0006f, 0.0006f, 0.0006f};
static const glm::vec3 PRACTICE_BATTLE_BUTTON_POSITION = {-0.139f, 0.02f, 0.1f};
static const glm::vec3 START_BATTLE_BUTTON_POSITION = {-0.147f, -0.147f, 0.1f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.164f, -0.147f, 0.1f};
static const glm::vec3 QUIT_BUTTON_POSITION = {-0.025f, -0.083f, 0.1f};

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.5f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    MAIN_MENU_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    TITLE_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& MainMenuSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

MainMenuSceneLogicManager::MainMenuSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

MainMenuSceneLogicManager::~MainMenuSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mActiveSubScene = SubSceneType::NONE;
    mTransitioningToSubScene = false;
    InitSubScene(SubSceneType::MAIN, scene);
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioningToSubScene)
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

void MainMenuSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::InitSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene)
{
    if (mActiveSubScene == subSceneType)
    {
        return;
    }
    
    mActiveSubScene = subSceneType;
    
    scene->RemoveAllSceneObjectsButTheOnesNamed(STATIC_SCENE_ELEMENTS);
    mAnimatedButtons.clear();
    
    switch (subSceneType)
    {
        case SubSceneType::MAIN:
        {
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                PRACTICE_BATTLE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Practice Battle",
                PRACTICE_BATTLE_BUTTON_NAME,
                [=](){ TransitionToSubScene(SubSceneType::PRACTICE_BATTLE, scene); },
                *scene
            ));
            
        #if defined(MOBILE_FLOW)
            (void)QUIT_BUTTON_NAME;
            (void)QUIT_BUTTON_POSITION;
        #else
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                QUIT_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Quit",
                QUIT_BUTTON_NAME,
                []()
                {
                    SDL_Event e;
                    e.type = SDL_QUIT;
                    SDL_PushEvent(&e);
                },
                *scene
            ));
        #endif
        } break;
            
        case SubSceneType::PRACTICE_BATTLE:
        {
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                BACK_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Back",
                BACK_BUTTON_NAME,
                [=](){ TransitionToSubScene(SubSceneType::MAIN, scene); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                START_BATTLE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Start Battle",
                START_BATTLE_BUTTON_NAME,
                [=](){ events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::IN_GAME_BATTLE_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE); },
                *scene
            ));
        } break;
            
        default: break;
    }
    
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }
        
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            mTransitioningToSubScene = false;
        });
    }
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::TransitionToSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene)
{
    mTransitioningToSubScene = true;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            InitSubScene(subSceneType, scene);
        });
    }
}

///------------------------------------------------------------------------------------------------
