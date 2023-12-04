///------------------------------------------------------------------------------------------------
///  GameOverGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

const strutils::StringId VICTORIOUS_TEXT_SCENE_OBJECT_NAME = strutils::StringId("VICTORIOUS_PLAYER_TEXT");
const std::string GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM = "victoriousPlayerIndex";

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void GameOverGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(VICTORIOUS_PLAYER_INDEX_PARAM) != 0);
    logging::Log(logging::LogType::INFO, "%s", ("Player " + mExtraActionParams.at(VICTORIOUS_PLAYER_INDEX_PARAM) + " won!").c_str());
}

///------------------------------------------------------------------------------------------------

void GameOverGameAction::VInitAnimation()
{
    auto& scene = *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto victorTextSo = scene.CreateSceneObject(VICTORIOUS_TEXT_SCENE_OBJECT_NAME);
    
    scene::TextSceneObjectData damageTextData;
    damageTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
    damageTextData.mText = "Player " + mExtraActionParams.at(VICTORIOUS_PLAYER_INDEX_PARAM) + " won!";
    
    victorTextSo->mSceneObjectTypeData = std::move(damageTextData);
    victorTextSo->mScale = glm::vec3(game_constants::IN_GAME_CARD_PROPERTY_SCALE * 3);
    victorTextSo->mPosition = glm::vec3(-0.1f, 0.0f, 5.0f);
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult GameOverGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool GameOverGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& GameOverGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
