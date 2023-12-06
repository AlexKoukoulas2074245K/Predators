///------------------------------------------------------------------------------------------------
///  CardHistoryEntryAdditionGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 06/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/BattleSceneLogicManager.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/CardHistoryEntryAdditionGameAction.h>

///------------------------------------------------------------------------------------------------

const std::string CardHistoryEntryAdditionGameAction::CARD_INDEX_PARAM = "cardIndex";
const std::string CardHistoryEntryAdditionGameAction::PLAYER_INDEX_PARAM = "playerIndex";

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    CardHistoryEntryAdditionGameAction::PLAYER_INDEX_PARAM,
    CardHistoryEntryAdditionGameAction::CARD_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void CardHistoryEntryAdditionGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(PLAYER_INDEX_PARAM));
    assert(mExtraActionParams.count(CARD_INDEX_PARAM));
}

///------------------------------------------------------------------------------------------------

void CardHistoryEntryAdditionGameAction::VInitAnimation()
{
    events::EventSystem::GetInstance().DispatchEvent<events::HistoryEntryAdditionEvent>(std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM)) == game_constants::REMOTE_PLAYER_INDEX, std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM)));
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult CardHistoryEntryAdditionGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------

bool CardHistoryEntryAdditionGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& CardHistoryEntryAdditionGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
