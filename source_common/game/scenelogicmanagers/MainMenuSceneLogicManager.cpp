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

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    MAIN_MENU_SCENE_NAME
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
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        glm::vec3(-0.045f, 0.02f, 0.1f),
        glm::vec3(0.0006f, 0.0006f, 0.0006f),
        game_constants::DEFAULT_FONT_NAME,
        "Battle",
        strutils::StringId("battle_button_name"),
        [](){ events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::IN_GAME_BATTLE_SCENE, true, false, true); },
        *scene
    ));
    
#if !defined(MOBILE_FLOW)
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        glm::vec3(-0.025f, -0.083f, 0.1f),
        glm::vec3(0.0006f, 0.0006f, 0.0006f),
        game_constants::DEFAULT_FONT_NAME,
        "Quit",
        strutils::StringId("quit_button_name"),
        []()
        {
            SDL_Event e;
            e.type = SDL_QUIT;
            SDL_PushEvent(&e);
        },
        *scene
    ));
#endif
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
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
