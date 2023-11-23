///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/TrapTriggeredAnimationGameAction.h>
#include <game/gameactions/CardEffectGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/GameRuleEngine.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM = "lastPlayedCardIndex";

static const strutils::StringId CARD_EFFECT_GAME_ACTION_NAME = strutils::StringId("CardEffectGameAction");
static const strutils::StringId TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME = strutils::StringId("TrapTriggeredAnimationGameAction");
static const strutils::StringId GOLDEN_CARD_PLAYED_EFFECT_GAME_ACTION_NAME = strutils::StringId("GoldenCardPlayedEffectGameAction");
static const strutils::StringId CARD_PLAY_PARTICLE_NAME = strutils::StringId("card_play");

static const float CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float CARD_PLAY_PARTICLE_EMITTER_Z = 0.01f;
static const float IN_GAME_PLAYED_CARD_ANIMATION_DURATION = 0.5f;


///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerHeldCards.empty());
    assert(mExtraActionParams.count(LAST_PLAYED_CARD_INDEX_PARAM) != 0);
    
    auto lastPlayedCardIndex = std::stoi(mExtraActionParams.at(LAST_PLAYED_CARD_INDEX_PARAM));
    auto cardId = activePlayerState.mPlayerHeldCards[lastPlayedCardIndex];
    auto cardData = CardDataRepository::GetInstance().GetCardData(cardId);
    
    assert(cardData.has_value());
    
    // Tried to overplay?
    mAborted = mGameRuleEngine && !mGameRuleEngine->CanCardBePlayed(&cardData->get(), lastPlayedCardIndex, mBoardState->GetActivePlayerIndex());
    if (mAborted)
    {
        return;
    }
    
    auto cardWeight = cardData->get().mCardWeight;
    const auto& cardStatOverrides = activePlayerState.mPlayerHeldCardStatOverrides;
    
    if (static_cast<int>(cardStatOverrides.size()) > lastPlayedCardIndex)
    {
        cardWeight = math::Max(0, cardStatOverrides[lastPlayedCardIndex].count(CardStatType::WEIGHT) ? cardStatOverrides[lastPlayedCardIndex].at(CardStatType::WEIGHT) : cardData->get().mCardWeight);
    }
    
    // Transfer held card stat override to the new board position
    if (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.size()) > lastPlayedCardIndex)
    {
        if (!mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[lastPlayedCardIndex].empty())
        {
            mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.emplace_back(mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[lastPlayedCardIndex]);
        }
        
        mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.erase(mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.begin() + lastPlayedCardIndex);
    }
    
    activePlayerState.mPlayerBoardCards.push_back(cardId);
    activePlayerState.mPlayerHeldCards.erase(activePlayerState.mPlayerHeldCards.begin() + lastPlayedCardIndex);
    activePlayerState.mPlayerCurrentWeightAmmo -= cardWeight;
    
    if (card_utils::GetCardRarity(cardId, mBoardState->GetActivePlayerIndex(), *mBoardState) == CardRarity::GOLDEN)
    {
        mGameActionEngine->AddGameAction(GOLDEN_CARD_PLAYED_EFFECT_GAME_ACTION_NAME);
    }
    
    if (cardData->get().IsSpell())
    {
        mGameActionEngine->AddGameAction(CARD_EFFECT_GAME_ACTION_NAME);
    }
    else // normal card
    {
        if ((activePlayerState.mBoardModifiers.mBoardModifierMask & effects::board_modifier_masks::KILL_NEXT) != 0)
        {
            mGameActionEngine->AddGameAction(TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME,
            {
                { TrapTriggeredAnimationGameAction::TRAP_TRIGGER_TYPE_PARAM, TrapTriggeredAnimationGameAction::TRAP_TRIGGER_TYPE_KILL }
            });
            activePlayerState.mBoardModifiers.mBoardModifierMask &= (~effects::board_modifier_masks::KILL_NEXT);
        }
        else if ((activePlayerState.mBoardModifiers.mBoardModifierMask & effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER) != 0)
        {
            mGameActionEngine->AddGameAction(TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME,
            {
                { TrapTriggeredAnimationGameAction::TRAP_TRIGGER_TYPE_PARAM, TrapTriggeredAnimationGameAction::TRAP_TRIGGER_TYPE_DEBUFF }
            });
        }
    }
}

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VInitAnimation()
{
    mPendingAnimations = 0;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    const auto lastPlayedCardIndex = std::stoi(mExtraActionParams.at(LAST_PLAYED_CARD_INDEX_PARAM));
    
    auto lastPlayedCardSoWrapper = mGameSessionManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(lastPlayedCardIndex);
    
    if (mAborted)
    {
        return;
    }
    
    // For remote plays, the front face card components also need to be created
    if (mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX)
    {
        activeScene->RemoveSceneObject(lastPlayedCardSoWrapper->mSceneObject->mName);
        lastPlayedCardSoWrapper = card_utils::CreateCardSoWrapper
        (
            lastPlayedCardSoWrapper->mCardData,
            lastPlayedCardSoWrapper->mSceneObject->mPosition,
            game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX + std::to_string(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1),
            CardOrientation::FRONT_FACE,
            card_utils::GetCardRarity(lastPlayedCardSoWrapper->mCardData->mCardId, mBoardState->GetActivePlayerIndex(), *mBoardState),
            false,
            true,
            true,
            (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.size()) > lastPlayedCardIndex ? mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.at(lastPlayedCardIndex) : CardStatOverrides()), // held card stat overrides have moved to board card stat overrides from the setstate above
            {},
            *activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE)
         );
        events::EventSystem::GetInstance().DispatchEvent<events::HeldCardSwapEvent>(lastPlayedCardSoWrapper, lastPlayedCardIndex, true);
    }
    
    events::EventSystem::GetInstance().DispatchEvent<events::LastCardPlayedFinalizedEvent>(lastPlayedCardIndex);
    
    // Rename played card components
    lastPlayedCardSoWrapper->mSceneObject->mName = strutils::StringId((mBoardState->GetActivePlayerIndex() == 0 ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1));
    
    // Animate played card to board
    auto targetPosition = card_utils::CalculateBoardCardPosition(static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size()), mBoardState->GetActivePlayerIndex() == 0);
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(lastPlayedCardSoWrapper->mSceneObject, targetPosition, lastPlayedCardSoWrapper->mSceneObject->mScale * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR, IN_GAME_PLAYED_CARD_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        mPendingAnimations--;
        CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)->GetCamera().Shake(CARD_CAMERA_SHAKE_DURATION, CARD_CAMERA_SHAKE_STRENGTH);
        
        events::EventSystem::GetInstance().DispatchEvent<events::WeightChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
        
        CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
        (
            CARD_PLAY_PARTICLE_NAME,
            glm::vec3(targetPosition.x, targetPosition.y, CARD_PLAY_PARTICLE_EMITTER_Z),
            *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)
         );
        
        lastPlayedCardSoWrapper->mSceneObject->mShaderBoolUniformValues[game_constants::IS_HELD_CARD_UNIFORM_NAME] = false;
    });
    mPendingAnimations++;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PlayCardGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool PlayCardGameAction::VShouldBeSerialized() const
{
    return true;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& PlayCardGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
