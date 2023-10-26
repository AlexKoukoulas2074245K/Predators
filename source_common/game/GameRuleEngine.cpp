///------------------------------------------------------------------------------------------------
///  GameRuleEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 26/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
#include <game/Cards.h>
#include <game/GameRuleEngine.h>

///------------------------------------------------------------------------------------------------

GameRuleEngine::GameRuleEngine(BoardState* boardState)
    : mBoardState(boardState)
{
}

///------------------------------------------------------------------------------------------------

bool GameRuleEngine::CanCardBePlayed(const CardData* cardData, const size_t forPlayerIndex) const
{
//    auto& activePlayerState = mBoardState->GetPlayerStates()[forPlayerIndex];
//    return activePlayerState.mPlayerCurrentWeightAmmo >= cardData->mCardWeight;
    (void)cardData;
    (void)forPlayerIndex;
    (void)mBoardState;
    return true;
}

///------------------------------------------------------------------------------------------------
