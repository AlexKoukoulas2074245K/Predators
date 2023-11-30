///------------------------------------------------------------------------------------------------
///  NextDinoDamageDoublingGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/11/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/NextDinoDamageDoublingGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

static const float CARD_SCALE_ANIMATION_DURATION_SECS = 1.0f;
static const float CARD_SCALE_FACTOR = 2.5f;

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
};

///------------------------------------------------------------------------------------------------

void NextDinoDamageDoublingGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerBoardCards.empty());
    
    auto& playerBoardCardStatOverrides = activePlayerState.mPlayerBoardCardStatOverrides;
    const auto& playerBoardCards = activePlayerState.mPlayerBoardCards;
    
    if (playerBoardCardStatOverrides.size() > playerBoardCards.size() - 1 && playerBoardCardStatOverrides[playerBoardCards.size() -1].count(CardStatType::DAMAGE))
    {
        playerBoardCardStatOverrides[playerBoardCards.size() - 1][CardStatType::DAMAGE] *= 2;
    }
    else
    {
        playerBoardCardStatOverrides.resize(playerBoardCards.size());
        playerBoardCardStatOverrides[playerBoardCards.size() -1][CardStatType::DAMAGE] = CardDataRepository::GetInstance().GetCardData(playerBoardCards.back())->get().mCardDamage * 2;
    }
    
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, false, effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE);
}

///------------------------------------------------------------------------------------------------

void NextDinoDamageDoublingGameAction::VInitAnimation()
{
    mFinished = false;
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto affectedCardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers()[mBoardState->GetActivePlayerIndex()].back();
    auto originalScale = affectedCardSoWrapper->mSceneObject->mScale;
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(affectedCardSoWrapper->mSceneObject, affectedCardSoWrapper->mSceneObject->mPosition, originalScale * CARD_SCALE_FACTOR, CARD_SCALE_ANIMATION_DURATION_SECS/2, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CardBuffedDebuffedEvent>(static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
        
        auto affectedCardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers()[mBoardState->GetActivePlayerIndex()].back();
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(affectedCardSoWrapper->mSceneObject, affectedCardSoWrapper->mSceneObject->mPosition, originalScale, CARD_SCALE_ANIMATION_DURATION_SECS/2, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
        {
            mFinished = true;
        });
    });
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult NextDinoDamageDoublingGameAction::VUpdateAnimation(const float)
{
    return mFinished ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool NextDinoDamageDoublingGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& NextDinoDamageDoublingGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
