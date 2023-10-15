///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM = "lastPlayedCardIndex";

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerHeldCards.empty());
    assert(mExtraActionParams.count(LAST_PLAYED_CARD_INDEX_PARAM) != 0);
    
    auto lastPlayedCardIndex = std::stoi(mExtraActionParams.at(LAST_PLAYED_CARD_INDEX_PARAM));
    activePlayerState.mPlayerBoardCards.push_back(activePlayerState.mPlayerHeldCards[lastPlayedCardIndex]);
    activePlayerState.mPlayerHeldCards.erase(activePlayerState.mPlayerHeldCards.begin() + lastPlayedCardIndex);
    
    if (mGameSessionManager)
    {
        mLastPlayedCardSoWrapper = mGameSessionManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(lastPlayedCardIndex);
        mGameSessionManager->OnLastCardPlayedFinalized(lastPlayedCardIndex);
    }
}

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VInitAnimation()
{
    mPendingAnimations = 0;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();

    // Rename played card
    auto newComponentNames = card_utils::GetCardComponentSceneObjectNames((mBoardState->GetActivePlayerIndex() == 0 ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), CardOrientation::FRONT_FACE);
    for (size_t i = 0; i < mLastPlayedCardSoWrapper->mSceneObjectComponents.size(); ++i)
    {
        mLastPlayedCardSoWrapper->mSceneObjectComponents[i]->mName = newComponentNames[i];
    }
    
    // Animate played card to board
    auto targetPosition = card_utils::CalculateBoardCardPosition(static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size()), mBoardState->GetActivePlayerIndex() == 0);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(mLastPlayedCardSoWrapper->mSceneObjectComponents, targetPosition, mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mScale * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR, game_constants::IN_GAME_PLAYED_CARD_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
    {
        mPendingAnimations--;
        CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)->GetCamera().Shake(0.5f);
    });
    mPendingAnimations++;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PlayCardGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------
