///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/gameactions/PlayCardGameAction.h>

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerHeldCards.empty());
    activePlayerState.mPlayerBoardCards.push_back(activePlayerState.mPlayerHeldCards.back());
    activePlayerState.mPlayerHeldCards.pop_back();
}

///------------------------------------------------------------------------------------------------
