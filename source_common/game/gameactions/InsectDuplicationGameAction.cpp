///------------------------------------------------------------------------------------------------
///  InsectDuplicationGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 30/11/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/InsectDuplicationGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

static const float DUPLICATED_CARD_Z_OFFSET = -0.01f;
static const float DUPLICATED_CARD_INIT_SCALE_FACTOR = 0.01f;
static const float DUPLICATION_ANIMATION_SECS_DURATION = 2.0f;

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
};

///------------------------------------------------------------------------------------------------

void InsectDuplicationGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerBoardCards.empty());
    
    activePlayerState.mPlayerBoardCards.push_back(activePlayerState.mPlayerBoardCards.back());
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, false, effects::board_modifier_masks::DUPLICATE_NEXT_INSECT);
}

///------------------------------------------------------------------------------------------------

void InsectDuplicationGameAction::VInitAnimation()
{
    mFinished = false;
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto activeScene = systemsEngine.GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto lastPlayedCardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).back();
    auto lastPlayedCardIndex = mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 2;
    auto sourcePosition = lastPlayedCardSoWrapper->mSceneObject->mPosition;
    
    auto newCardSoWrapper = card_utils::CreateCardSoWrapper
    (
     lastPlayedCardSoWrapper->mCardData,
        sourcePosition,
        (mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1),
        CardOrientation::FRONT_FACE,
        card_utils::GetCardRarity(lastPlayedCardSoWrapper->mCardData->mCardId, mBoardState->GetActivePlayerIndex(), *mBoardState),
        true,
        mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX,
        true,
        (mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.size() > lastPlayedCardIndex ? mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.at(lastPlayedCardIndex) : CardStatOverrides()), // held card stat overrides have moved to board card stat overrides from the setstate above
        mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers,
        *activeScene
    );
    newCardSoWrapper->mSceneObject->mPosition.z += DUPLICATED_CARD_Z_OFFSET;
    newCardSoWrapper->mSceneObject->mScale *= DUPLICATED_CARD_INIT_SCALE_FACTOR;
    
    events::EventSystem::GetInstance().DispatchEvent<events::NewBoardCardCreatedEvent>(newCardSoWrapper, static_cast<int>(lastPlayedCardIndex), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
    
    const auto& boardCards = mBoardState->GetActivePlayerState().mPlayerBoardCards;
    const auto& deadBoardCardIndices = mBoardState->GetActivePlayerState().mBoardCardIndicesToDestroy;
    const auto nonDeadBoardCardCount = card_utils::CalculateNonDeadCardsCount(boardCards, deadBoardCardIndices);
    
    auto targetPosition = card_utils::CalculateBoardCardPosition(nonDeadBoardCardCount - 1, nonDeadBoardCardCount, mBoardState->GetActivePlayerIndex() == 0);
    
    systemsEngine.GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(newCardSoWrapper->mSceneObject, targetPosition, lastPlayedCardSoWrapper->mSceneObject->mScale, DUPLICATION_ANIMATION_SECS_DURATION, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [&]()
    {
        mFinished = true;
    });
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult InsectDuplicationGameAction::VUpdateAnimation(const float)
{
    return mFinished ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool InsectDuplicationGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& InsectDuplicationGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
