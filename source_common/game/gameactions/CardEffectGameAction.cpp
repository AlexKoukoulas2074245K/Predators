///------------------------------------------------------------------------------------------------
///  CardEffectGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/11/2023
///------------------------------------------------------------------------------------------------

#include <game/CardEffectComponents.h>
#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/CardEffectGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::unordered_map<CardEffectGameAction::AffectedStatType, CardStatType> CardEffectGameAction::sAffectedStatTypeToCardStatType =
{
    {AffectedStatType::DAMAGE, CardStatType::DAMAGE},
    {AffectedStatType::WEIGHT, CardStatType::WEIGHT}
};

// Resources
static const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_spell_dissolve.vs";
static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const std::string BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX = "card_effect_emitter_";

// Uniforms
static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");
static const strutils::StringId CARD_SPELL_EFFECT_PARTICLE_NAME = strutils::StringId("card_spell_effect");
static const strutils::StringId CARD_EFFECT_PARTICLE_EMITTER_NAME = strutils::StringId("card_effect_emitter");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");

static const float CARD_DISSOLVE_SPEED = 0.001f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;
static const float CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET = 22.0f;
static const float CARD_SCALE_ANIMATION_DURATION_SECS = 0.6f;

static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE      = {10.0f, 18.0f};

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
};

///------------------------------------------------------------------------------------------------

void CardEffectGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    auto cardId = activePlayerState.mPlayerBoardCards.back();
    const auto& cardEffectData = CardDataRepository::GetInstance().GetCardData(cardId);
    
    assert(cardEffectData);
    HandleCardEffect(cardEffectData->get().mCardEffect);
    
    // shouldn't really happen
    if (activePlayerState.mPlayerBoardCardStatOverrides.size() == activePlayerState.mPlayerBoardCards.size())
    {
        activePlayerState.mPlayerBoardCardStatOverrides.pop_back();
    }
    
    activePlayerState.mPlayerBoardCards.pop_back();
}

///------------------------------------------------------------------------------------------------

void CardEffectGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(mBoardState->GetActivePlayerState().mPlayerBoardCards.size());
    cardSoWrapper->mSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_DISSOLVE_SHADER_FILE_NAME);
    cardSoWrapper->mSceneObject->mEffectTextureResourceIds[1] = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_X_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.x;
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_Y_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.y;
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_MAGNITUDE_UNIFORM_NAME] = math::RandomFloat(CARD_DISSOLVE_EFFECT_MAG_RANGE.x, CARD_DISSOLVE_EFFECT_MAG_RANGE.y);
    
    systemsEngine.GetParticleManager().CreateParticleEmitterAtPosition
    (
        CARD_SPELL_EFFECT_PARTICLE_NAME,
        glm::vec3(cardSoWrapper->mSceneObject->mPosition.x, cardSoWrapper->mSceneObject->mPosition.y, CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET), // pos
        *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
        CARD_EFFECT_PARTICLE_EMITTER_NAME
    );
    
    // Force release all held/moving cards back to position
    for (const auto& affectedCardEntry: mAffectedCards)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::ForceSendCardBackToPositionEvent>(static_cast<int>(affectedCardEntry.mCardIndex), affectedCardEntry.mIsBoardCard, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
    }
    
    
    mActionState = ActionState::EFFECT_CARD_ANIMATION;
    mAnimationDelayCounterSecs = 0.0f;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult CardEffectGameAction::VUpdateAnimation(const float dtMillis)
{ 
    switch (mActionState)
    {
        case ActionState::EFFECT_CARD_ANIMATION:
        {
            const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
            const auto& activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
            
            const auto& boardCards = mBoardState->GetActivePlayerState().mPlayerBoardCards;
            const auto& deadBoardCardIndices = mBoardState->GetActivePlayerState().mBoardCardIndicesToDestroy;
            
            const auto& heldCards = mBoardState->GetActivePlayerState().mPlayerHeldCards;
            const auto& deadHeldCardIndices = mBoardState->GetActivePlayerState().mHeldCardIndicesToDestroy;
            
            auto boardCardIndex = boardCards.size();
            auto effectCardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(boardCardIndex);
            effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
          
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE/2)
            {
                // Fade particle emitter on spell
                CoreSystemsEngine::GetInstance().GetParticleManager().RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, CARD_EFFECT_PARTICLE_EMITTER_NAME, *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
            }
            
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::ImmediateCardDestructionWithRepositionEvent>(static_cast<int>(boardCardIndex), true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                
                // Create particle emitters on affected cards
                for (size_t i = 0U; i < mAffectedCards.size(); ++i)
                {
                    auto& affectedCardEntry = mAffectedCards[i];
                    
                    auto cardSoWrapper = affectedCardEntry.mIsBoardCard ?
                        mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(affectedCardEntry.mCardIndex):
                        mGameSessionManager->GetHeldCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(affectedCardEntry.mCardIndex);
                    
                    auto targetPosition = affectedCardEntry.mIsBoardCard ?
                        card_utils::CalculateBoardCardPosition(affectedCardEntry.mCardIndex, card_utils::CalculateNonDeadCardsCount(boardCards, deadBoardCardIndices), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX):
                        card_utils::CalculateHeldCardPosition(affectedCardEntry.mCardIndex, card_utils::CalculateNonDeadCardsCount(heldCards, deadHeldCardIndices), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, activeScene->GetCamera());
                    
                    CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
                    (
                        CARD_SPELL_EFFECT_PARTICLE_NAME,
                        glm::vec3(targetPosition.x, targetPosition.y, CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET), // pos
                        *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
                        strutils::StringId(BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(i))
                    );
                }
                
                mActionState = ActionState::AFFECTED_CARDS_SPARKLE_ANIMATION;
            }
        } break;
            
        case ActionState::AFFECTED_CARDS_SPARKLE_ANIMATION:
        {
            switch (mCardBoardEffectMask)
            {
                case effects::board_modifier_masks::KILL_NEXT:
                case effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER:
                {
                    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectTriggeredEvent>(mBoardState->GetActivePlayerIndex() != game_constants::REMOTE_PLAYER_INDEX, mCardBoardEffectMask);
                } break;
                    
                case effects::board_modifier_masks::DUPLICATE_NEXT_INSECT:
                case effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE:
                {
                    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectTriggeredEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, mCardBoardEffectMask);
                }
            }
            
            if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_CLEAR_EFFECTS) != mEffectComponents.cend())
            {
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::KILL_NEXT);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::DUPLICATE_NEXT_INSECT);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE);
            }
            
            if (mAffectedCards.empty())
            {
                mActionState = ActionState::FINISHED;
            }
            
            mAnimationDelayCounterSecs += dtMillis/1000.0f;
            if (mAnimationDelayCounterSecs > 1.0f)
            {
                mAnimationDelayCounterSecs = 0.0f;
                
                auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                
                // Create particle emitters on affected cards
                for (size_t i = 0; i < mAffectedCards.size(); ++i)
                {
                    auto& affectedCardEntry = mAffectedCards.at(i);
                    
                    auto originalScale = affectedCardEntry.mCardSoWrapper->mSceneObject->mScale;
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(affectedCardEntry.mCardSoWrapper->mSceneObject, affectedCardEntry.mCardSoWrapper->mSceneObject->mPosition, originalScale * 1.5f, CARD_SCALE_ANIMATION_DURATION_SECS/2, animation_flags::NONE, i * CARD_SCALE_ANIMATION_DURATION_SECS, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                    {
                        CoreSystemsEngine::GetInstance().GetParticleManager().RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, strutils::StringId(BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(i)), *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
                        
                        events::EventSystem::GetInstance().DispatchEvent<events::CardBuffedDebuffedEvent>(static_cast<int>(mAffectedCards[i].mCardIndex), mAffectedCards[i].mIsBoardCard, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                        
                        mAffectedCards[i].mCardSoWrapper = mAffectedCards[i].mIsBoardCard ?
                            mGameSessionManager->GetBoardCardSoWrappers()[mBoardState->GetActivePlayerIndex()][mAffectedCards[i].mCardIndex]:
                            mGameSessionManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()][mAffectedCards[i].mCardIndex];
                        
                        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(mAffectedCards[i].mCardSoWrapper->mSceneObject, mAffectedCards[i].mCardSoWrapper->mSceneObject->mPosition, originalScale, CARD_SCALE_ANIMATION_DURATION_SECS/2, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                        {
                            if (i == mAffectedCards.size() - 1)
                            {
                                mActionState = ActionState::FINISHED;
                            }
                        });
                    });
                }
                
                mActionState = ActionState::AFFECTED_CARDS_SCALE_ANIMATION;
            }
        } break;
            
        default: break;
    }
    
   return mActionState == ActionState::FINISHED ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool CardEffectGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& CardEffectGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------

void CardEffectGameAction::HandleCardEffect(const std::string& effect)
{
    mCardBoardEffectMask = effects::board_modifier_masks::NONE;
    mAffectedBoardCardsStatType = AffectedStatType::NONE;
    mEffectValue = 0;
    mAffectedCards.clear();
    
    mEffectComponents = strutils::StringSplit(effect, ' ');
    const auto& boardCards = mBoardState->GetActivePlayerState().mPlayerBoardCards;
    const auto& heldCards = mBoardState->GetActivePlayerState().mPlayerHeldCards;
    const auto effectCardFamily = CardDataRepository::GetInstance().GetCardData(boardCards.back())->get().mCardFamily;
    bool mAffectingFamilyOnly = false;
    
    std::vector<int> affectedBoardCardIndices;
    std::vector<int> affectedHeldCardIndices;
    
    for (const auto& effectComponent: mEffectComponents)
    {
        // Collection component
        if (effectComponent == effects::EFFECT_COMPONENT_FAMILY)
        {
            mAffectingFamilyOnly = true;
        }
        
        // Stat Type component
        else if (effectComponent == effects::EFFECT_COMPONENT_DAMAGE)
        {
            mAffectedBoardCardsStatType = AffectedStatType::DAMAGE;
        }
        else if (effectComponent == effects::EFFECT_COMPONENT_WEIGHT)
        {
            mAffectedBoardCardsStatType = AffectedStatType::WEIGHT;
        }
        
        // Clear effects component
        else if (effectComponent == effects::EFFECT_COMPONENT_CLEAR_EFFECTS)
        {
            mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers.clear();
            mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask = effects::board_modifier_masks::NONE;
        }
        
        // Kill component
        else if (effectComponent == effects::EFFECT_COMPONENT_KILL)
        {
            mBoardState->GetInactivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::KILL_NEXT;
            mCardBoardEffectMask = effects::board_modifier_masks::KILL_NEXT;
        }
        
        // Insect Duplication component
        else if (effectComponent == effects::EFFECT_COMPONENT_DUPLICATE_INSECT)
        {
            mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::DUPLICATE_NEXT_INSECT;
            mCardBoardEffectMask = effects::board_modifier_masks::DUPLICATE_NEXT_INSECT;
        }
        
        // Doubling Dino Damage component
        else if (effectComponent == effects::EFFECT_COMPONENT_DOUBLE_NEXT_DINO_DAMAGE)
        {
            mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE;
            mCardBoardEffectMask = effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE;
        }
        
        // Modifier/Offset value component
        else if (!effects::STATIC_EFFECT_COMPONENT_NAMES.count(effectComponent))
        {
            mEffectValue = std::stoi(effectComponent);
        }
    }
    
    // Board effect
    if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_BOARD) != mEffectComponents.cend())
    {
        for (int i = 0; i < static_cast<int>(boardCards.size()) - 1; ++i)
        {
            auto cardData = CardDataRepository::GetInstance().GetCardData(boardCards[i]);
            assert(cardData);
            
            if (mAffectingFamilyOnly && cardData->get().mCardFamily != effectCardFamily)
            {
                continue;
            }
            
            if (!cardData->get().IsSpell())
            {
                affectedBoardCardIndices.emplace_back(i);
            }
        }
    }
    
    // Held Cards effect
    if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_HELD) != mEffectComponents.cend())
    {
        for (int i = 0; i < static_cast<int>(heldCards.size()); ++i)
        {
            auto cardData = CardDataRepository::GetInstance().GetCardData(heldCards[i]);
            assert(cardData);
            
            if (mAffectingFamilyOnly && cardData->get().mCardFamily != effectCardFamily)
            {
                continue;
            }
            
            if (!cardData->get().IsSpell())
            {
                affectedHeldCardIndices.emplace_back(i);
            }
        }
    }
    
    // Draw effect
    if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_DRAW) != mEffectComponents.cend())
    {
        for (auto i = 0; i < mEffectValue; ++i)
        {
            mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
        }
    }
    
    // Next turn effect
    if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_ENEMY_BOARD_DEBUFF) != mEffectComponents.cend())
    {
        mBoardState->GetInactivePlayerState().mBoardModifiers.mGlobalCardStatModifiers[sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType)] += mEffectValue;
        mBoardState->GetInactivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER;
        mCardBoardEffectMask = effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER;
    }
    
    // Create/Modify board card stat overrides
    for (auto affectedBoardCardIter = affectedBoardCardIndices.begin(); affectedBoardCardIter != affectedBoardCardIndices.end();)
    {
        const auto affectedStat = sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType);
        auto cardData = CardDataRepository::GetInstance().GetCardData(mBoardState->GetActivePlayerState().mPlayerBoardCards.at(*affectedBoardCardIter))->get();
        auto currentValue = mAffectedBoardCardsStatType == AffectedStatType::DAMAGE ? cardData.mCardDamage : cardData.mCardWeight;
        
        if (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.size()) > *affectedBoardCardIter)
        {
            currentValue = mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[*affectedBoardCardIter][affectedStat];
        }
        else
        {
            mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.resize(*affectedBoardCardIter + 1);
        }
        
        mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[*affectedBoardCardIter][affectedStat] = currentValue + mEffectValue;
        affectedBoardCardIter++;
    }
    
    // Create/Modify held card stat overrides
    for (auto affectedHeldCardIter = affectedHeldCardIndices.begin(); affectedHeldCardIter != affectedHeldCardIndices.end();)
    {
        const auto affectedStat = sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType);
        auto cardData = CardDataRepository::GetInstance().GetCardData(mBoardState->GetActivePlayerState().mPlayerHeldCards.at(*affectedHeldCardIter))->get();
        auto currentValue = mAffectedBoardCardsStatType == AffectedStatType::DAMAGE ? cardData.mCardDamage : cardData.mCardWeight;
        
        if (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.size()) > *affectedHeldCardIter)
        {
            currentValue = mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[*affectedHeldCardIter][affectedStat];
        }
        else
        {
            mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.resize(*affectedHeldCardIter + 1);
        }
        
        mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[*affectedHeldCardIter][affectedStat] = currentValue + mEffectValue;
        
        // Skip animation for held cards for opponent
        if (mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX)
        {
            affectedHeldCardIter = affectedHeldCardIndices.erase(affectedHeldCardIter);
            continue;
        }
        
        affectedHeldCardIter++;
    }
    
    // For non-headless behavior
    if (mGameSessionManager)
    {
        for (auto i = 0; i < static_cast<int>(affectedBoardCardIndices.size()); ++i)
        {
            mAffectedCards.push_back({mGameSessionManager->GetBoardCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(affectedBoardCardIndices.at(i)), affectedBoardCardIndices.at(i), true});
        }
        
        for (auto i = 0; i < static_cast<int>(affectedHeldCardIndices.size()); ++i)
        {
            mAffectedCards.push_back({mGameSessionManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(affectedHeldCardIndices.at(i)), affectedHeldCardIndices.at(i), false});
        }
    }
    //
}

///------------------------------------------------------------------------------------------------
