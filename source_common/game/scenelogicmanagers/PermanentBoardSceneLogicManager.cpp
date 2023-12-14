///------------------------------------------------------------------------------------------------
///  PermanentBoardSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <game/scenelogicmanagers/PermanentBoardSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId PERMANENT_BOARD_SCENE_NAME = strutils::StringId("permanent_board_scene");

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    PERMANENT_BOARD_SCENE_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& PermanentBoardSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

void PermanentBoardSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

void PermanentBoardSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void PermanentBoardSceneLogicManager::VUpdate(const float, std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void PermanentBoardSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------
