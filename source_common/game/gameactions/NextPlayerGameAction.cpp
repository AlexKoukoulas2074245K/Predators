///------------------------------------------------------------------------------------------------
///  NextPlayerGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/gameactions/NextPlayerGameAction.h>

///------------------------------------------------------------------------------------------------

void NextPlayerGameAction::VSetNewGameState()
{
    size_t& activePlayerIndex = mBoardState->GetActivePlayerIndex();
    activePlayerIndex = (activePlayerIndex + 1) % mBoardState->GetPlayerCount();
}

///------------------------------------------------------------------------------------------------

void NextPlayerGameAction::VInitAnimation()
{
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult NextPlayerGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& NextPlayerGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
