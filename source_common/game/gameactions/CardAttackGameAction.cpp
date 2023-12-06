///------------------------------------------------------------------------------------------------
///  CardAttackGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/CardAttackGameAction.h>
#include <game/gameactions/CardDestructionGameAction.h>
#include <game/gameactions/CardHistoryEntryAdditionGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/gameactions/RodentsDigAnimationGameAction.h>
#include <game/BattleSceneLogicManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string CardAttackGameAction::CARD_INDEX_PARAM = "cardIndex";
const std::string CardAttackGameAction::PLAYER_INDEX_PARAM = "playerIndex";

static const strutils::StringId GAME_OVER_GAME_ACTION_NAME = strutils::StringId("GameOverGameAction");
static const strutils::StringId CARD_DESTRUCTION_GAME_ACTION_NAME = strutils::StringId("CardDestructionGameAction");
static const strutils::StringId CARD_HISTORY_ENTRY_ADDITION_GAME_ACTION_NAME = strutils::StringId("CardHistoryEntryAdditionGameAction");
static const strutils::StringId RODENTS_DIG_ANIMATION_GAME_ACTION_NAME = strutils::StringId("RodentsDigAnimationGameAction");
static const strutils::StringId ATTACKING_CARD_PARTICLE_NAME = strutils::StringId("card_attack");

static const float ATTACKING_CARD_ANIMATION_Y_OFFSET = 0.16f;
static const float ATTACKING_CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float ATTACKING_CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float ATTACKING_CARD_PARTICLE_EMITTER_Z = 0.01f;
static const float ATTACKING_CARD_SHORT_ANIMATION_DURATION = 0.25f;
static const float ATTACKING_CARD_LONG_ANIMATION_DURATION = 0.4f;
static const float ATTACKING_CARD_ANIMATION_ELEVATED_Z = 20.0f;

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    CardAttackGameAction::CARD_INDEX_PARAM,
    CardAttackGameAction::PLAYER_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void CardAttackGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(CARD_INDEX_PARAM) != 0);
    assert(mExtraActionParams.count(PLAYER_INDEX_PARAM) != 0);
    
    auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    auto& attackingPlayerBoardCards = mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerBoardCards;
    const auto& attackingCardData = CardDataRepository::GetInstance().GetCardData(attackingPlayerBoardCards[cardIndex]);
    
    assert(attackingCardData);
    
    // Card has been destroyed in between this action's creation and it's invocation of setting state here
    if (mBoardState->GetPlayerStates()[attackingPayerIndex].mBoardCardIndicesToDestroy.count(cardIndex))
    {
        return;
    }
    
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    auto& attackingPlayerOverrides = mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerBoardCardStatOverrides;
    
    auto damage = attackingCardData->get().mCardDamage;
    
    if (!attackingPlayerOverrides.empty() && static_cast<int>(attackingPlayerOverrides.size()) > cardIndex && attackingPlayerOverrides[cardIndex].count(CardStatType::DAMAGE))
    {
        damage = math::Max(0, attackingPlayerOverrides[cardIndex].at(CardStatType::DAMAGE));
    }
    
    if (mBoardState->GetPlayerStates()[attackingPayerIndex].mBoardModifiers.mGlobalCardStatModifiers.count(CardStatType::DAMAGE))
    {
        damage = math::Max(0, damage + mBoardState->GetPlayerStates()[attackingPayerIndex].mBoardModifiers.mGlobalCardStatModifiers.at(CardStatType::DAMAGE));
    }
    
    if (attackingCardData->get().mCardFamily == game_constants::INSECTS_FAMILY_NAME)
    {
        activePlayerState.mPlayerPoisonStack++;
        
        if ((mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask & effects::board_modifier_masks::DOUBLE_POISON_ATTACKS) != 0)
        {
            activePlayerState.mPlayerPoisonStack++;
        }
    }
    
    activePlayerState.mPlayerHealth -= damage;
    mPendingDamage = damage;
    
    mGameActionEngine->AddGameAction(CARD_HISTORY_ENTRY_ADDITION_GAME_ACTION_NAME,
    {
        { CardHistoryEntryAdditionGameAction::PLAYER_INDEX_PARAM, std::to_string(attackingPayerIndex) },
        { CardHistoryEntryAdditionGameAction::CARD_INDEX_PARAM, std::to_string(cardIndex) },
        { CardHistoryEntryAdditionGameAction::ENTRY_TYPE_TEXTURE_FILE_NAME_PARAM, CardHistoryEntryAdditionGameAction::ENTRY_TYPE_TEXTURE_FILE_NAME_BATTLE },
    });
    
    if (activePlayerState.mPlayerHealth <= 0.0f)
    {
        activePlayerState.mPlayerHealth = 0.0f;
        mGameActionEngine->AddGameAction(GAME_OVER_GAME_ACTION_NAME,
        {
            { GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM, std::to_string(attackingPayerIndex)}
        });
    }
    else
    {
        // Check for rodents respawn flow
        if (attackingCardData->get().mCardFamily == game_constants::RODENTS_FAMILY_NAME)
        {
            if (math::ControlledRandomFloat() <= game_constants::RODENTS_RESPAWN_CHANCE)
            {
                mGameActionEngine->AddGameAction(RODENTS_DIG_ANIMATION_GAME_ACTION_NAME,
                {
                    { RodentsDigAnimationGameAction::CARD_INDEX_PARAM, std::to_string(cardIndex) },
                    { RodentsDigAnimationGameAction::PLAYER_INDEX_PARAM, std::to_string(attackingPayerIndex) }
                });
                return;
            }
        }
        
        mGameActionEngine->AddGameAction(CARD_DESTRUCTION_GAME_ACTION_NAME,
        {
            { CardDestructionGameAction::CARD_INDICES_PARAM, {"[" + std::to_string(cardIndex) + "]"}},
            { CardDestructionGameAction::PLAYER_INDEX_PARAM, std::to_string(attackingPayerIndex)},
            { CardDestructionGameAction::IS_BOARD_CARD_PARAM, "true"},
            { CardDestructionGameAction::IS_TRAP_TRIGGER_PARAM, "false"},
        });
    }
}

///------------------------------------------------------------------------------------------------

void CardAttackGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    
    auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    
    mPendingAnimations = 0;
    
    // Card has been destroyed in between this action's creation and it's invocation of setting state here
    if (mBoardState->GetPlayerStates()[attackingPayerIndex].mBoardCardIndicesToDestroy.count(cardIndex))
    {
        return;
    }
    
    auto cardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndex);
    
    mOriginalCardPosition = cardSoWrapper->mSceneObject->mPosition;
    mOriginalCardScale = cardSoWrapper->mSceneObject->mScale;
    
    // Enlargement animation
    {
        auto targetScale = mOriginalCardScale * 1.2f;
        auto targetPos = cardSoWrapper->mSceneObject->mPosition;
        targetPos.z += ATTACKING_CARD_ANIMATION_ELEVATED_Z;
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, targetPos, targetScale, ATTACKING_CARD_SHORT_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
        {
            mPendingAnimations--;
            
            // Move to target position animation
            {
                auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
                auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
                auto cardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndex);
                
                auto targetPos = cardSoWrapper->mSceneObject->mPosition;
                targetPos.y += attackingPayerIndex == game_constants::LOCAL_PLAYER_INDEX ? ATTACKING_CARD_ANIMATION_Y_OFFSET : - ATTACKING_CARD_ANIMATION_Y_OFFSET;
                
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, targetPos, cardSoWrapper->mSceneObject->mScale, ATTACKING_CARD_SHORT_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
                {
                    mPendingAnimations--;
                    
                    auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
                    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
                    
                    if (mPendingDamage != 0)
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::HealthChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                    }
                    
                    auto cardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndex);
                    
                    if (!cardSoWrapper->mCardData->IsSpell() && cardSoWrapper->mCardData->mCardFamily == game_constants::INSECTS_FAMILY_NAME)
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::PoisonStackChangeChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, mBoardState->GetActivePlayerState().mPlayerPoisonStack);
                    }
                    
                    systemsEngine.GetParticleManager().CreateParticleEmitterAtPosition
                    (
                        ATTACKING_CARD_PARTICLE_NAME,
                        glm::vec3(cardSoWrapper->mSceneObject->mPosition.x, cardSoWrapper->mSceneObject->mPosition.y, ATTACKING_CARD_PARTICLE_EMITTER_Z),
                        *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)
                     );
                    
                    auto cameraShakeDuration = ATTACKING_CARD_CAMERA_SHAKE_DURATION * (1.0f + 0.05f * std::powf(mPendingDamage, 2));
                    auto cameraShakeStrength = ATTACKING_CARD_CAMERA_SHAKE_STRENGTH * (1.0f + 0.05f * std::powf(mPendingDamage, 2));
                    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)->GetCamera().Shake(cameraShakeDuration, cameraShakeStrength);
                    
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, mOriginalCardPosition, mOriginalCardScale, ATTACKING_CARD_LONG_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
                    {
                        mPendingAnimations--;
                    });
                });
            }
        });
    }
    
    mPendingAnimations = 3;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult CardAttackGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool CardAttackGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& CardAttackGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
