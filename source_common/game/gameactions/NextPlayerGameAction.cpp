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
