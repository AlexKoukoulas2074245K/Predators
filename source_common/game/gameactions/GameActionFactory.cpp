///------------------------------------------------------------------------------------------------
///  GameActionFactory.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/gameactions/CardAttackGameAction.h>
#include <game/gameactions/CardEffectGameAction.h>
#include <game/gameactions/CardDestructionGameAction.h>
#include <game/gameactions/CardPlayedParticleEffectGameAction.h>
#include <game/gameactions/DrawCardGameAction.h>
#include <game/gameactions/GameActionFactory.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/gameactions/GoldenCardPlayedEffectGameAction.h>
#include <game/gameactions/IdleGameAction.h>
#include <game/gameactions/InsectDuplicationGameAction.h>
#include <game/gameactions/NextPlayerGameAction.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/gameactions/PoisonStackApplicationGameAction.h>
#include <game/gameactions/PostNextPlayerGameAction.h>
#include <game/gameactions/RodentsDigAnimationGameAction.h>
#include <game/gameactions/TrapTriggeredAnimationGameAction.h>
#include <game/gameactions/NextDinoDamageDoublingGameAction.h>
#include <game/gameactions/CardBuffedDebuffedAnimationGameAction.h>
#include <algorithm>
#include <vector>

///------------------------------------------------------------------------------------------------

#define REGISTER_ACTION(name) REGISTERED_ACTION_NAMES.push_back(strutils::StringId(#name))
#define ACTION_CASE(name) if (actionName == strutils::StringId(#name)) { return std::make_unique<name>(); }

///------------------------------------------------------------------------------------------------

static std::vector<strutils::StringId> REGISTERED_ACTION_NAMES;

///------------------------------------------------------------------------------------------------

void GameActionFactory::RegisterGameActions()
{
    REGISTERED_ACTION_NAMES.clear();
    
    REGISTER_ACTION(IdleGameAction);
    REGISTER_ACTION(CardAttackGameAction);
    REGISTER_ACTION(CardEffectGameAction);
    REGISTER_ACTION(DrawCardGameAction);
    REGISTER_ACTION(GameOverGameAction);
    REGISTER_ACTION(CardPlayedParticleEffectGameAction);
    REGISTER_ACTION(NextPlayerGameAction);
    REGISTER_ACTION(PlayCardGameAction);
    REGISTER_ACTION(CardDestructionGameAction);
    REGISTER_ACTION(PostNextPlayerGameAction);
    REGISTER_ACTION(TrapTriggeredAnimationGameAction);
    REGISTER_ACTION(GoldenCardPlayedEffectGameAction);
    REGISTER_ACTION(PoisonStackApplicationGameAction);
    REGISTER_ACTION(RodentsDigAnimationGameAction);
    REGISTER_ACTION(InsectDuplicationGameAction);
    REGISTER_ACTION(NextDinoDamageDoublingGameAction);
    REGISTER_ACTION(CardBuffedDebuffedAnimationGameAction);
    std::sort(REGISTERED_ACTION_NAMES.begin(), REGISTERED_ACTION_NAMES.end(), [&](const strutils::StringId& lhs, const strutils::StringId& rhs)
    {
        return lhs.GetString() < rhs.GetString();
    });
}

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& GameActionFactory::GetRegisteredActions()
{
    return REGISTERED_ACTION_NAMES;
}

///------------------------------------------------------------------------------------------------

std::unique_ptr<BaseGameAction> GameActionFactory::CreateGameAction(const strutils::StringId& actionName)
{
    ACTION_CASE(IdleGameAction);
    ACTION_CASE(CardAttackGameAction);
    ACTION_CASE(CardEffectGameAction);
    ACTION_CASE(CardDestructionGameAction);
    ACTION_CASE(DrawCardGameAction);
    ACTION_CASE(GameOverGameAction);
    ACTION_CASE(CardPlayedParticleEffectGameAction);
    ACTION_CASE(NextPlayerGameAction);
    ACTION_CASE(PlayCardGameAction);
    ACTION_CASE(PostNextPlayerGameAction);
    ACTION_CASE(TrapTriggeredAnimationGameAction);
    ACTION_CASE(GoldenCardPlayedEffectGameAction);
    ACTION_CASE(PoisonStackApplicationGameAction);
    ACTION_CASE(RodentsDigAnimationGameAction);
    ACTION_CASE(InsectDuplicationGameAction);
    ACTION_CASE(NextDinoDamageDoublingGameAction);
    ACTION_CASE(CardBuffedDebuffedAnimationGameAction);
    assert(false && "Invalid game action name");
    return nullptr;
}

///------------------------------------------------------------------------------------------------
