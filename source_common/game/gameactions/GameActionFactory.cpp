///------------------------------------------------------------------------------------------------
///  GameActionFactory.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/gameactions/DrawCardGameAction.h>
#include <game/gameactions/GameActionFactory.h>
#include <game/gameactions/IdleGameAction.h>
#include <game/gameactions/NextPlayerGameAction.h>
#include <game/gameactions/PlayCardGameAction.h>

///------------------------------------------------------------------------------------------------

#define ACTION_CASE(name) if (actionName == strutils::StringId(#name)) { return std::make_unique<name>(); }

///------------------------------------------------------------------------------------------------

std::unique_ptr<BaseGameAction> GameActionFactory::CreateGameAction(const strutils::StringId& actionName)
{
    ACTION_CASE(IdleGameAction);
    ACTION_CASE(DrawCardGameAction);
    ACTION_CASE(NextPlayerGameAction);
    ACTION_CASE(PlayCardGameAction);
    assert(false && "Invalid game action name");
    return nullptr;
}

///------------------------------------------------------------------------------------------------
