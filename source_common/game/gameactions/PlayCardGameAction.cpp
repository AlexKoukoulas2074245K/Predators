///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/gameactions/PlayCardGameAction.h>
#include <game/GameSessionManager.h>

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerHeldCards.empty());
    
    if (mGameSessionManager)
    {
        activePlayerState.mPlayerBoardCards.push_back(activePlayerState.mPlayerHeldCards[mGameSessionManager->GetLastPlayedCardIndex()]);
        activePlayerState.mPlayerHeldCards.erase(activePlayerState.mPlayerHeldCards.begin() + mGameSessionManager->GetLastPlayedCardIndex());
        mGameSessionManager->OnLastCardPlayedFinalized();
    }
    else
    {
        activePlayerState.mPlayerBoardCards.push_back(activePlayerState.mPlayerHeldCards.back());
        activePlayerState.mPlayerHeldCards.pop_back();
    }
}

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VInitAnimation()
{
    
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PlayCardGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------
