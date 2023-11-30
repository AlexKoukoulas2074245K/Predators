///------------------------------------------------------------------------------------------------
///  PlayerActionGenerationEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
#include <game/CardEffectComponents.h>
#include <game/Cards.h>
#include <game/GameConstants.h>
#include <game/GameRuleEngine.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/gameactions/PlayerActionGenerationEngine.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId PLAY_CARD_GAME_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("NextPlayerGameAction");

///------------------------------------------------------------------------------------------------

PlayerActionGenerationEngine::PlayerActionGenerationEngine(GameRuleEngine* gameRuleEngine, GameActionEngine* gameActionEngine)
    : mGameRuleEngine(gameRuleEngine)
    , mGameActionEngine(gameActionEngine)
{
    
}

///------------------------------------------------------------------------------------------------

void PlayerActionGenerationEngine::DecideAndPushNextActions(BoardState* currentBoardState)
{
    BoardState boardStateCopy = *currentBoardState;
    auto& currentHeldCards = boardStateCopy.GetActivePlayerState().mPlayerHeldCards;
    auto& currentBoardCards = boardStateCopy.GetActivePlayerState().mPlayerBoardCards;
    
    auto currentHeldCardsCopySorted = currentHeldCards;
    
    // Sort all held cards by descending damage
    const auto& cardRepository = CardDataRepository::GetInstance();
    std::sort(currentHeldCardsCopySorted.begin(), currentHeldCardsCopySorted.end(), [&](const int& lhs, const int& rhs)
    {
        auto& cardDataLhs = cardRepository.GetCardData(lhs)->get();
        auto& cardDataRhs = cardRepository.GetCardData(rhs)->get();
        
        bool shouldWaitForFurtherActionsAfterPlayingLhs = ShouldWaitForFurtherActionsAfterPlayingCard(cardDataLhs);
        bool shouldWaitForFurtherActionsAfterPlayingRhs = ShouldWaitForFurtherActionsAfterPlayingCard(cardDataRhs);
        
        if (shouldWaitForFurtherActionsAfterPlayingLhs && shouldWaitForFurtherActionsAfterPlayingRhs)
        {
            return lhs < rhs;
        }
        else if (shouldWaitForFurtherActionsAfterPlayingLhs && !shouldWaitForFurtherActionsAfterPlayingRhs)
        {
            return true;
        }
        else if (!shouldWaitForFurtherActionsAfterPlayingLhs && shouldWaitForFurtherActionsAfterPlayingRhs)
        {
            return false;
        }
        
        return cardDataLhs.mCardDamage >
               cardDataRhs.mCardDamage;
    });

    // Play every card possible (from highest weights to lowest)
    bool shouldWaitForFurtherActions = false;
    for (auto iter = currentHeldCardsCopySorted.cbegin(); iter != currentHeldCardsCopySorted.cend();)
    {
        const auto* cardData = &cardRepository.GetCardData(*iter)->get();
        // Find index of card in original vector
        auto originalHeldCardIter = std::find(currentHeldCards.cbegin(), currentHeldCards.cend(), cardData->mCardId);
        const auto cardIndex = originalHeldCardIter - currentHeldCards.cbegin();
        
        if (mGameRuleEngine->CanCardBePlayed(cardData, cardIndex, boardStateCopy.GetActivePlayerIndex(), &boardStateCopy))
        {
            assert(originalHeldCardIter != currentHeldCards.cend());
            
            mGameActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, std::to_string(cardIndex)}});
            
            // Simulate card play effects on copy of board state
            auto cardWeight = cardData->mCardWeight;
            const auto& cardStatOverrides = boardStateCopy.GetActivePlayerState().mPlayerHeldCardStatOverrides;
            if (static_cast<int>(cardStatOverrides.size()) > cardIndex)
            {
                cardWeight = math::Max(0, cardStatOverrides[cardIndex].count(CardStatType::WEIGHT) ? cardStatOverrides[cardIndex].at(CardStatType::WEIGHT) : cardData->mCardWeight);
            }
            
            boardStateCopy.GetActivePlayerState().mPlayerCurrentWeightAmmo -= cardWeight;
            currentBoardCards.push_back(cardData->mCardId);
            
            currentHeldCards.erase(originalHeldCardIter);
            iter = currentHeldCardsCopySorted.erase(iter);
            
            shouldWaitForFurtherActions = ShouldWaitForFurtherActionsAfterPlayingCard(*cardData);
            if (shouldWaitForFurtherActions)
            {
                break;
            }
        }
        else
        {
            iter++;
        }
    }
    
    if (!shouldWaitForFurtherActions)
    {
        mGameActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    }
}

///------------------------------------------------------------------------------------------------

bool PlayerActionGenerationEngine::ShouldWaitForFurtherActionsAfterPlayingCard(const CardData& cardData) const
{
    if (cardData.IsSpell() && strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_DRAW))
        return true;
    else if
    (
        cardData.IsSpell() &&
        strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_WEIGHT) &&
        strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_FAMILY) &&
        strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_HELD) &&
        !strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_BOARD)
    ) return true;
    else if
    (
        cardData.IsSpell() &&
        strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_CLEAR_EFFECTS)
    ) return true;
    else if
    (
        cardData.IsSpell() &&
        strutils::StringContains(cardData.mCardEffect, effects::EFFECT_COMPONENT_DUPLICATE_INSECT)
    ) return true;
        
    return false;
}

///------------------------------------------------------------------------------------------------
