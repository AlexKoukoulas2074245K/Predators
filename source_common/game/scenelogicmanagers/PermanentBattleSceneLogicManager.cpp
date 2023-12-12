///------------------------------------------------------------------------------------------------
///  PermanentBattleSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <game/scenelogicmanagers/PermanentBattleSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId PERMANENT_BOARD_SCENE_NAME = strutils::StringId("permanent_board_scene");
static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    PERMANENT_BOARD_SCENE_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& PermanentBattleSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

void PermanentBattleSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
#if defined(MOBILE_FLOW)
    if (ios_utils::IsIPad())
    {
        scene->GetCamera().SetZoomFactor(120.0f);
    }
    else
    {
        scene->GetCamera().SetZoomFactor(130.0f);
    }
    
#else
    scene->GetCamera().SetZoomFactor(120.0f);
#endif
}

///------------------------------------------------------------------------------------------------

void PermanentBattleSceneLogicManager::VUpdate(const float, std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void PermanentBattleSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------
