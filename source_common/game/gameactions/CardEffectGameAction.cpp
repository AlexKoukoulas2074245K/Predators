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
#include <game/gameactions/CardBuffedDebuffedAnimationGameAction.h>
#include <game/gameactions/CardEffectGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/BattleSceneLogicManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::unordered_map<CardEffectGameAction::AffectedStatType, CardStatType> CardEffectGameAction::sAffectedStatTypeToCardStatType =
{
    {AffectedStatType::DAMAGE, CardStatType::DAMAGE},
    {AffectedStatType::WEIGHT, CardStatType::WEIGHT}
};

// Follow up game actions
static const strutils::StringId CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME = strutils::StringId("CardBuffedDebuffedAnimationGameAction");

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
static const float CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET = 21.0f;
static const float CARD_SCALE_UP_FACTOR = 1.5f;
static const float CARD_SCALE_DOWN_FACTOR = 0.5f;

static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE = {10.0f, 18.0f};

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
    auto cardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(mBoardState->GetActivePlayerState().mPlayerBoardCards.size());
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
        *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
        CARD_EFFECT_PARTICLE_EMITTER_NAME
    );
    
    // Force release all held/moving cards back to position
    for (const auto& affectedCardEntry: mAffectedCards)
    {
        if (!affectedCardEntry.mIsBoardCard)
        {
            events::EventSystem::GetInstance().DispatchEvent<events::ForceSendCardBackToPositionEvent>(static_cast<int>(affectedCardEntry.mCardIndex), affectedCardEntry.mIsBoardCard, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
        }
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
            const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
            const auto& scene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
            
            const auto& boardCards = mBoardState->GetActivePlayerState().mPlayerBoardCards;
            const auto& deadBoardCardIndices = mBoardState->GetActivePlayerState().mBoardCardIndicesToDestroy;
            
            const auto& heldCards = mBoardState->GetActivePlayerState().mPlayerHeldCards;
            const auto& deadHeldCardIndices = mBoardState->GetActivePlayerState().mHeldCardIndicesToDestroy;
            
            auto boardCardIndex = boardCards.size();
            auto effectCardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(boardCardIndex);
            effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
          
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE/2)
            {
                // Fade particle emitter on spell
                CoreSystemsEngine::GetInstance().GetParticleManager().RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, CARD_EFFECT_PARTICLE_EMITTER_NAME, *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
            }
            
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::ImmediateCardDestructionWithRepositionEvent>(static_cast<int>(boardCardIndex), true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                
                // Create particle emitters on affected cards
                for (size_t i = 0U; i < mAffectedCards.size(); ++i)
                {
                    auto& affectedCardEntry = mAffectedCards[i];
                    
                    auto cardSoWrapper = affectedCardEntry.mIsBoardCard ?
                        mBattleSceneLogicManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(affectedCardEntry.mCardIndex):
                        mBattleSceneLogicManager->GetHeldCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(affectedCardEntry.mCardIndex);
                    
                    auto targetPosition = affectedCardEntry.mIsBoardCard ?
                        card_utils::CalculateBoardCardPosition(affectedCardEntry.mCardIndex, card_utils::CalculateNonDeadCardsCount(boardCards, deadBoardCardIndices), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX):
                        card_utils::CalculateHeldCardPosition(affectedCardEntry.mCardIndex, card_utils::CalculateNonDeadCardsCount(heldCards, deadHeldCardIndices), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, scene->GetCamera());
                    
                    CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
                    (
                        CARD_SPELL_EFFECT_PARTICLE_NAME,
                        glm::vec3(targetPosition.x, targetPosition.y, CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET), // pos
                        *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
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
                case effects::board_modifier_masks::BOARD_SIDE_DEBUFF:
                case effects::board_modifier_masks::DOUBLE_POISON_ATTACKS:
                {
                    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectTriggeredEvent>(mBoardState->GetActivePlayerIndex() != game_constants::REMOTE_PLAYER_INDEX, mCardBoardEffectMask);
                } break;
                    
                case effects::board_modifier_masks::DUPLICATE_NEXT_INSECT:
                case effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE:
                case effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION:
                {
                    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectTriggeredEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, mCardBoardEffectMask);
                }
            }
            
            if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_CLEAR_EFFECTS) != mEffectComponents.cend())
            {
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::BOARD_SIDE_DEBUFF);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::KILL_NEXT);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::DUPLICATE_NEXT_INSECT);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE);
                events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true,  effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION);
            }
            
            if (mAffectedCards.empty())
            {
                mActionState = ActionState::FINISHED;
            }
            
            mAnimationDelayCounterSecs += dtMillis/1000.0f;
            if (mAnimationDelayCounterSecs > 1.0f)
            {
                mAnimationDelayCounterSecs = 0.0f;
                mActionState = ActionState::FINISHED;
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
            if ((mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask & effects::board_modifier_masks::BOARD_SIDE_DEBUFF) != 0)
            {
                for (auto i = 0U; i < boardCards.size() - 1; ++i)
                {
                    mGameActionEngine->AddGameAction(CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME,
                    {
                        { CardBuffedDebuffedAnimationGameAction::CARD_INDEX_PARAM, std::to_string(i)},
                        { CardBuffedDebuffedAnimationGameAction::PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex())},
                        { CardBuffedDebuffedAnimationGameAction::IS_BOARD_CARD_PARAM, "true" },
                        { CardBuffedDebuffedAnimationGameAction::SCALE_FACTOR_PARAM, std::to_string(CARD_SCALE_UP_FACTOR) }
                    });
                }
            }
            else if ((mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask & effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION) != 0)
            {
                for (auto i = 0U; i < heldCards.size(); ++i)
                {
                    if (!CardDataRepository::GetInstance().GetCardData(heldCards[i])->get().IsSpell())
                    {
                        mGameActionEngine->AddGameAction(CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME,
                        {
                            { CardBuffedDebuffedAnimationGameAction::CARD_INDEX_PARAM, std::to_string(i)},
                            { CardBuffedDebuffedAnimationGameAction::PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex())},
                            { CardBuffedDebuffedAnimationGameAction::IS_BOARD_CARD_PARAM, "false" },
                            { CardBuffedDebuffedAnimationGameAction::SCALE_FACTOR_PARAM, std::to_string(CARD_SCALE_DOWN_FACTOR) }
                        });
                    }
                }
            }
            
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
        
        // Doubling Poison Attacks component
        else if (effectComponent == effects::EFFECT_COMPONENT_DOUBLE_POISON_ATTACKS)
        {
            mBoardState->GetInactivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::DOUBLE_POISON_ATTACKS;
            mCardBoardEffectMask = effects::board_modifier_masks::DOUBLE_POISON_ATTACKS;
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
        mBoardState->GetInactivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::BOARD_SIDE_DEBUFF;
        mCardBoardEffectMask = effects::board_modifier_masks::BOARD_SIDE_DEBUFF;
    }
    
    // Continual weight reduction component
    else if (std::find(mEffectComponents.cbegin(), mEffectComponents.cend(), effects::EFFECT_COMPONENT_PERMANENT_CONTINUAL_WEIGHT_REDUCTION) != mEffectComponents.cend())
    {
        if (mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers.count(sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType)) == 0)
        {
            mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers[sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType)] = 0;
        }
        
        mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers[sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType)]--;
        mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask |= effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION;
        mCardBoardEffectMask = effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION;
    }
    
    // Create/Modify board card stat overrides
    int particleEmitterIndex = 0;
    for (auto affectedBoardCardIter = affectedBoardCardIndices.begin(); affectedBoardCardIter != affectedBoardCardIndices.end();)
    {
        const auto affectedStat = sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType);
        auto cardData = CardDataRepository::GetInstance().GetCardData(mBoardState->GetActivePlayerState().mPlayerBoardCards.at(*affectedBoardCardIter))->get();
        auto currentValue = mAffectedBoardCardsStatType == AffectedStatType::DAMAGE ? cardData.mCardDamage : cardData.mCardWeight;
        
        if (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.size()) <= *affectedBoardCardIter)
        {
            mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.resize(*affectedBoardCardIter + 1);
        }
        
        if (mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[*affectedBoardCardIter].count(affectedStat) == 0)
        {
            mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[*affectedBoardCardIter][affectedStat] = 0;
        }
        else
        {
            currentValue = mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[*affectedBoardCardIter][affectedStat];
        }
        
        mGameActionEngine->AddGameAction(CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME,
        {
            { CardBuffedDebuffedAnimationGameAction::CARD_INDEX_PARAM, std::to_string(*affectedBoardCardIter)},
            { CardBuffedDebuffedAnimationGameAction::PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex())},
            { CardBuffedDebuffedAnimationGameAction::IS_BOARD_CARD_PARAM, "true" },
            { CardBuffedDebuffedAnimationGameAction::SCALE_FACTOR_PARAM, std::to_string(CARD_SCALE_UP_FACTOR) },
            { CardBuffedDebuffedAnimationGameAction::PARTICLE_EMITTER_NAME_TO_REMOVE_PARAM, BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(particleEmitterIndex++) }
        });
        
        mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[*affectedBoardCardIter][affectedStat] = currentValue + mEffectValue;
        affectedBoardCardIter++;
    }
    
    // Create/Modify held card stat overrides
    for (auto affectedHeldCardIter = affectedHeldCardIndices.begin(); affectedHeldCardIter != affectedHeldCardIndices.end();)
    {
        if (mCardBoardEffectMask != effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION)
        {
            const auto affectedStat = sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType);
            auto cardData = CardDataRepository::GetInstance().GetCardData(mBoardState->GetActivePlayerState().mPlayerHeldCards.at(*affectedHeldCardIter))->get();
            auto currentValue = mAffectedBoardCardsStatType == AffectedStatType::DAMAGE ? cardData.mCardDamage : cardData.mCardWeight;
            
            if (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.size()) <= *affectedHeldCardIter)
            {
                mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.resize(*affectedHeldCardIter + 1);
            }
            
            if (mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[*affectedHeldCardIter].count(affectedStat) == 0)
            {
                mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[*affectedHeldCardIter][affectedStat] = 0;
            }
            else
            {
                currentValue = mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[*affectedHeldCardIter][affectedStat];
            }
            
            mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides[*affectedHeldCardIter][affectedStat] = currentValue + mEffectValue;
        }
        
        // Skip animation for held cards for opponent
        if (mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX)
        {
            affectedHeldCardIter = affectedHeldCardIndices.erase(affectedHeldCardIter);
            continue;
        }
        
        mGameActionEngine->AddGameAction(CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME,
        {
            { CardBuffedDebuffedAnimationGameAction::CARD_INDEX_PARAM, std::to_string(*affectedHeldCardIter)},
            { CardBuffedDebuffedAnimationGameAction::PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex())},
            { CardBuffedDebuffedAnimationGameAction::IS_BOARD_CARD_PARAM, "false" },
            { CardBuffedDebuffedAnimationGameAction::SCALE_FACTOR_PARAM, std::to_string(CARD_SCALE_UP_FACTOR) },
            { CardBuffedDebuffedAnimationGameAction::PARTICLE_EMITTER_NAME_TO_REMOVE_PARAM, BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(particleEmitterIndex++) }
        });
        
        affectedHeldCardIter++;
    }
    
    // For non-headless behavior
    if (mBattleSceneLogicManager)
    {
        for (auto i = 0; i < static_cast<int>(affectedBoardCardIndices.size()); ++i)
        {
            mAffectedCards.push_back({mBattleSceneLogicManager->GetBoardCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(affectedBoardCardIndices.at(i)), affectedBoardCardIndices.at(i), true});
        }
        
        for (auto i = 0; i < static_cast<int>(affectedHeldCardIndices.size()); ++i)
        {
            mAffectedCards.push_back({mBattleSceneLogicManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(affectedHeldCardIndices.at(i)), affectedHeldCardIndices.at(i), false});
        }
    }
    //
}

///------------------------------------------------------------------------------------------------
