///------------------------------------------------------------------------------------------------
///  PlayerActionGenerationEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
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
        return cardRepository.GetCardData(lhs)->get().mCardDamage >
               cardRepository.GetCardData(rhs)->get().mCardDamage;
    });

    // Play every card possible (from highest weights to lowest)
    for (auto iter = currentHeldCardsCopySorted.cbegin(); iter != currentHeldCardsCopySorted.cend();)
    {
        const auto* cardData = &cardRepository.GetCardData(*iter)->get();
        if (mGameRuleEngine->CanCardBePlayed(cardData, boardStateCopy.GetActivePlayerIndex(), &boardStateCopy))
        {
            // Find index of card in original vector
            auto originalHeldCardIter = std::find(currentHeldCards.cbegin(), currentHeldCards.cend(), cardData->mCardId);
            assert(originalHeldCardIter != currentHeldCards.cend());
            
            mGameActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, std::to_string(originalHeldCardIter - currentHeldCards.cbegin())}});
            
            // Simulate card play effects on copy of board state
            boardStateCopy.GetActivePlayerState().mPlayerCurrentWeightAmmo -= cardData->mCardWeight;
            currentBoardCards.push_back(cardData->mCardId);
            
            currentHeldCards.erase(originalHeldCardIter);
            iter = currentHeldCardsCopySorted.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    mGameActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
}

///------------------------------------------------------------------------------------------------
