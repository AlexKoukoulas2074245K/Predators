///------------------------------------------------------------------------------------------------
///  RemotePlayerActionEngine.cpp                                                                                        
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
#include <game/gameactions/RemotePlayerActionEngine.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId PLAY_CARD_GAME_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("NextPlayerGameAction");

///------------------------------------------------------------------------------------------------

RemotePlayerActionEngine::RemotePlayerActionEngine(GameRuleEngine* gameRuleEngine, GameActionEngine* gameActionEngine)
    : mGameRuleEngine(gameRuleEngine)
    , mGameActionEngine(gameActionEngine)
{
    
}

///------------------------------------------------------------------------------------------------

void RemotePlayerActionEngine::DecideAndPushNextActions(BoardState* currentBoardState)
{
    assert(currentBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
    
    BoardState boardStateCopy = *currentBoardState;
    auto& currentHeldCards = boardStateCopy.GetActivePlayerState().mPlayerHeldCards;
    auto& currentBoardCards = boardStateCopy.GetActivePlayerState().mPlayerBoardCards;
    
    auto currentHeldCardsCopySorted = currentHeldCards;
    
    // Sort all held cards by descending weight
    const auto& cardRepository = CardDataRepository::GetInstance();
    std::sort(currentHeldCardsCopySorted.begin(), currentHeldCardsCopySorted.end(), [&](const int& lhs, const int& rhs)
    {
        return cardRepository.GetCardData(lhs)->get().mCardWeight >
               cardRepository.GetCardData(rhs)->get().mCardWeight;
    });
    
    // Play every card possible (from highest weights to lowest)
    for (auto iter = currentHeldCardsCopySorted.cbegin(); iter != currentHeldCardsCopySorted.cend();)
    {
        const auto* cardData = &cardRepository.GetCardData(*iter)->get();
        if (mGameRuleEngine->CanCardBePlayed(cardData, game_constants::REMOTE_PLAYER_INDEX, &boardStateCopy))
        {
            // Find index of card in original vector
            auto originalHeldCardIter = std::find(currentHeldCards.cbegin(), currentHeldCards.cend(), currentHeldCardsCopySorted.front());
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
