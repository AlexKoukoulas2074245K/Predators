///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerHeldCards.empty());
    
    if (mGameSessionManager)
    {
        activePlayerState.mPlayerBoardCards.push_back(activePlayerState.mPlayerHeldCards[mGameSessionManager->GetLastPlayedCardIndex()]);
        activePlayerState.mPlayerHeldCards.erase(activePlayerState.mPlayerHeldCards.begin() + mGameSessionManager->GetLastPlayedCardIndex());
        mLastPlayedCardSoWrapper = mGameSessionManager->GetLastPlayedCardSceneObjectWrapper();
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
//    mLastPlayedCardSoWrapper->mSceneObjectComponents[1]->mPosition.y = mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mPosition.y + 0.007f;
//    mLastPlayedCardSoWrapper->mSceneObjectComponents[2]->mPosition.x = mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mPosition.x - 0.007f;
//    mLastPlayedCardSoWrapper->mSceneObjectComponents[2]->mPosition.y = mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mPosition.y + 0.0175f;
//
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(mLastPlayedCardSoWrapper->mSceneObjectComponents, glm::vec3(0.0f, 0.0f, 0.04f), mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mScale * 0.666f, 0.5f, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PlayCardGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------
