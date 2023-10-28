///------------------------------------------------------------------------------------------------
///  GameRuleEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 26/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
#include <game/Cards.h>
#include <game/GameRuleEngine.h>
#include <game/GameConstants.h>

///------------------------------------------------------------------------------------------------

GameRuleEngine::GameRuleEngine(BoardState* boardState)
    : mBoardState(boardState)
{
}

///------------------------------------------------------------------------------------------------

bool GameRuleEngine::CanCardBePlayed(const CardData* cardData, const size_t forPlayerIndex, BoardState* customBoardStateOverride /* = nullptr */) const
{
    auto* boardStateToUse = customBoardStateOverride ? customBoardStateOverride : mBoardState;
    auto& activePlayerState = boardStateToUse->GetPlayerStates()[forPlayerIndex];
    return activePlayerState.mPlayerCurrentWeightAmmo >= cardData->mCardWeight && activePlayerState.mPlayerBoardCards.size() < game_constants::MAX_BOARD_CARDS;
}

///------------------------------------------------------------------------------------------------
