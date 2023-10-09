///------------------------------------------------------------------------------------------------
///  DrawCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <game/gameactions/DrawCardGameAction.h>

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    activePlayerState.mPlayerHeldCards.push_back(math::ControlledRandomInt());
}

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VInitAnimation()
{
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult DrawCardGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------
