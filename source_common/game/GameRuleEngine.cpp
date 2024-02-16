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
#include <game/CardEffectComponents.h>

///------------------------------------------------------------------------------------------------

GameRuleEngine::GameRuleEngine(BoardState* boardState)
    : mBoardState(boardState)
{
}

///------------------------------------------------------------------------------------------------

bool GameRuleEngine::CanCardBePlayed(const CardData* cardData, const size_t cardIndex, const size_t forPlayerIndex, BoardState* customBoardStateOverride /* = nullptr */) const
{
    auto* boardStateToUse = customBoardStateOverride ? customBoardStateOverride : mBoardState;
    auto& activePlayerState = boardStateToUse->GetPlayerStates()[forPlayerIndex];
    
    auto cardWeight = cardData->mCardWeight;
    const auto& cardStatOverrides = activePlayerState.mPlayerHeldCardStatOverrides;
    
    if (cardStatOverrides.size() > cardIndex)
    {
        cardWeight = math::Max(0, cardStatOverrides[cardIndex].count(CardStatType::WEIGHT) ? cardStatOverrides[cardIndex].at(CardStatType::WEIGHT) : cardData->mCardWeight);
    }
    
    if (!cardData->IsSpell() && activePlayerState.mBoardModifiers.mGlobalCardStatModifiers.count(CardStatType::WEIGHT))
    {
        cardWeight = math::Max(0, cardWeight + activePlayerState.mBoardModifiers.mGlobalCardStatModifiers.at(CardStatType::WEIGHT));
    }
    
    if (cardData->mCardEffect == effects::EFFECT_COMPONENT_INSECT_MEGASWARM && activePlayerState.mPlayerBoardCards.size() > 1)
    {
        return false;
    }
    
    return activePlayerState.mPlayerCurrentWeightAmmo >= cardWeight && activePlayerState.mPlayerBoardCards.size() < game_constants::MAX_BOARD_CARDS;
}

///------------------------------------------------------------------------------------------------
