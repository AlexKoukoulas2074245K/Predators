///------------------------------------------------------------------------------------------------
///  CardEffectGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/11/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/CardEffectGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------
// Effect components
static const std::string EFFECT_COMPONENT_DAMAGE = "DAMAGE";
static const std::string EFFECT_COMPONENT_FAMILY = "FAMILY";
static const std::string EFFECT_COMPONENT_ENEMIES = "ENEMIES";
static const std::string EFFECT_COMPONENT_DRAW   = "DRAW";
static const std::unordered_set<std::string> STATIC_EFFECT_COMPONENT_NAMES =
{
    EFFECT_COMPONENT_DRAW,
    EFFECT_COMPONENT_DAMAGE,
    EFFECT_COMPONENT_FAMILY,
    EFFECT_COMPONENT_ENEMIES
};

const std::unordered_map<CardEffectGameAction::AffectedStatType, CardStatType> CardEffectGameAction::sAffectedStatTypeToCardStatType =
{
    {AffectedStatType::DAMAGE, CardStatType::DAMAGE},
    {AffectedStatType::WEIGHT, CardStatType::WEIGHT}
};

// Resources
static const std::string CARD_EFFECT_PARTICLE_TEXTURE_FILE_NAME = "sparkles.png";
static const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_spell_dissolve.vs";
static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const std::string BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX = "card_effect_emitter_";

// Uniforms
static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");

static const strutils::StringId CARD_EFFECT_PARTICLE_EMITTER_NAME = strutils::StringId("card_effect_emitter");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");

static const float CARD_DISSOLVE_SPEED = 0.001f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;
static const float CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET = 22.0f;
static const float CONTINUOUS_PARTICLE_GENERATION_DELAY_SECS = 0.002f;
static const float CARD_SCALE_ANIMATION_DURATION_SECS = 0.6f;

static const int CARD_EFFECT_PARTICLE_COUNT = 100;

static const glm::vec2 CARD_EFFECT_PARTICLE_LIFETIME_RANGE = {0.7f, 1.3f};
static const glm::vec2 CARD_EFFECT_PARTICLE_X_OFFSET_RANGE = {-0.02f, 0.01f};
static const glm::vec2 CARD_EFFECT_PARTICLE_Y_OFFSET_RANGE = {-0.03f, 0.03f};
static const glm::vec2 CARD_EFFECT_PARTICLE_SIZE_RANGE     = {0.0075f, 0.0125f};
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
    
    activePlayerState.mPlayerBoardCards.pop_back();
}

///------------------------------------------------------------------------------------------------

void CardEffectGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(mBoardState->GetActivePlayerState().mPlayerBoardCards.size());
    cardSoWrapper->mSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_DISSOLVE_SHADER_FILE_NAME);
    cardSoWrapper->mSceneObject->mEffectTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_X_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.x;
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_Y_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.y;
    cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_MAGNITUDE_UNIFORM_NAME] = math::RandomFloat(CARD_DISSOLVE_EFFECT_MAG_RANGE.x, CARD_DISSOLVE_EFFECT_MAG_RANGE.y);
    
    rendering::CreateParticleEmitterAtPosition
    (
        glm::vec3(cardSoWrapper->mSceneObject->mPosition.x, cardSoWrapper->mSceneObject->mPosition.y, CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET), // pos
        CARD_EFFECT_PARTICLE_LIFETIME_RANGE,            // particleLifetimeRange
        CARD_EFFECT_PARTICLE_X_OFFSET_RANGE,            // particlePositionXOffsetRange
        CARD_EFFECT_PARTICLE_Y_OFFSET_RANGE,            // particlePositionYOffsetRange
        CARD_EFFECT_PARTICLE_SIZE_RANGE,                // particleSizeRange
        CARD_EFFECT_PARTICLE_COUNT,                     // particleCount
        CARD_EFFECT_PARTICLE_TEXTURE_FILE_NAME,         // particleTextureFilename
        *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
        particle_flags::CONTINUOUS_PARTICLE_GENERATION | particle_flags::ENLARGE_OVER_TIME,  // particleFlags
        CARD_EFFECT_PARTICLE_EMITTER_NAME,
        CONTINUOUS_PARTICLE_GENERATION_DELAY_SECS
    );
    
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
            const auto& boardCards = mBoardState->GetActivePlayerState().mPlayerBoardCards;
            auto boardCardIndex = boardCards.size();
            auto effectCardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(boardCardIndex);
            effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
          
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE/2)
            {
                // Fade particle emitter on spell
                rendering::RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, CARD_EFFECT_PARTICLE_EMITTER_NAME, *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
            }
            
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::CardDestructionWithRepositionEvent>(static_cast<int>(boardCardIndex), true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                
                // Create particle emitters on affected cards
                for (const auto affectedIndex: mAffectedBoardIndices)
                {
                    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(affectedIndex);
                    auto targetPosition = card_utils::CalculateBoardCardPosition(affectedIndex, static_cast<int>(boardCards.size()), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                    
                    rendering::CreateParticleEmitterAtPosition
                    (
                        glm::vec3(targetPosition.x, targetPosition.y, CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET), // pos
                        CARD_EFFECT_PARTICLE_LIFETIME_RANGE,            // particleLifetimeRange
                        CARD_EFFECT_PARTICLE_X_OFFSET_RANGE,            // particlePositionXOffsetRange
                        CARD_EFFECT_PARTICLE_Y_OFFSET_RANGE,            // particlePositionYOffsetRange
                        CARD_EFFECT_PARTICLE_SIZE_RANGE,                // particleSizeRange
                        CARD_EFFECT_PARTICLE_COUNT,                     // particleCount
                        CARD_EFFECT_PARTICLE_TEXTURE_FILE_NAME,         // particleTextureFilename
                        *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
                        particle_flags::CONTINUOUS_PARTICLE_GENERATION | particle_flags::ENLARGE_OVER_TIME,  // particleFlags
                        strutils::StringId(BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(affectedIndex)),
                        CONTINUOUS_PARTICLE_GENERATION_DELAY_SECS
                    );
                }
                
                mActionState = ActionState::AFFECTED_CARDS_SPARKLE_ANIMATION;
            }
        } break;
            
        case ActionState::AFFECTED_CARDS_SPARKLE_ANIMATION:
        {
            if (mAffectingNextPlayer)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::CardEffectNextTurnTriggeredEvent>(mBoardState->GetActivePlayerIndex() != game_constants::REMOTE_PLAYER_INDEX);
            }
            
            if (mAffectedBoardIndices.empty())
            {
                mActionState = ActionState::FINISHED;
            }
            
            mAnimationDelayCounterSecs += dtMillis/1000.0f;
            if (mAnimationDelayCounterSecs > 1.0f)
            {
                mAnimationDelayCounterSecs = 0.0f;
                
                auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                
                // Create particle emitters on affected cards
                for (size_t i = 0; i < mAffectedBoardIndices.size(); ++i)
                {
                    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(mAffectedBoardIndices.at(i));
                    
                    auto originalScale = cardSoWrapper->mSceneObject->mScale;
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, originalScale * 1.5f, CARD_SCALE_ANIMATION_DURATION_SECS/2, animation_flags::NONE, i * CARD_SCALE_ANIMATION_DURATION_SECS, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                    {
                        for (const auto affectedIndex: mAffectedBoardIndices)
                        {
                            rendering::RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, strutils::StringId(BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(affectedIndex)), *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
                        }
                        
                        events::EventSystem::GetInstance().DispatchEvent<events::CardBuffedEvent>(static_cast<int>(i), true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                        auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(mAffectedBoardIndices.at(i));
                        
                        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, originalScale, CARD_SCALE_ANIMATION_DURATION_SECS/2, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                        {
                            if(i == mAffectedBoardIndices.size() - 1)
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
    mAffectedBoardCardsStatType = AffectedStatType::NONE;
    mEffectValue = 0;
    mAffectedBoardIndices.clear();
    
    const auto effectComponents = strutils::StringSplit(effect, ' ');
    const auto& boardCards = mBoardState->GetActivePlayerState().mPlayerBoardCards;
    const auto effectCardFamily = CardDataRepository::GetInstance().GetCardData(boardCards.back())->get().mCardFamily;
    
    for (const auto& effectComponent: effectComponents)
    {
        // Collection component
        if (effectComponent == EFFECT_COMPONENT_FAMILY)
        {
            for (int i = 0; i < static_cast<int>(boardCards.size()) - 1; ++i)
            {
                auto cardData = CardDataRepository::GetInstance().GetCardData(boardCards[i]);
                assert(cardData);
                
                if (!cardData->get().IsSpell() && cardData->get().mCardFamily == effectCardFamily)
                {
                    mAffectedBoardIndices.push_back(i);
                }
            }
        }
        
        // Stat Type component
        if (effectComponent == EFFECT_COMPONENT_DAMAGE)
        {
            mAffectedBoardCardsStatType = AffectedStatType::DAMAGE;
        }
        
        // Modifier/Offset value component
        if (!STATIC_EFFECT_COMPONENT_NAMES.count(effectComponent))
        {
            mEffectValue = std::stoi(effectComponent);
        }
    }
    
    // Draw effect
    if (std::find(effectComponents.cbegin(), effectComponents.cend(), EFFECT_COMPONENT_DRAW) != effectComponents.cend())
    {
        for (auto i = 0; i < mEffectValue; ++i)
        {
            mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
        }
    }
    
    // Next turn effect
    mAffectingNextPlayer = false;
    if (std::find(effectComponents.cbegin(), effectComponents.cend(), EFFECT_COMPONENT_ENEMIES) != effectComponents.cend())
    {
        mAffectingNextPlayer = true;
        mBoardState->GetInactivePlayerState().mGlobalBoardCardStatModifiers[sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType)] += mEffectValue;
    }
    
    // Current turn effect
    for (int affectedIndex: mAffectedBoardIndices)
    {
        const auto& cardData = CardDataRepository::GetInstance().GetCardData(boardCards.at(affectedIndex))->get();
        const auto affectedStat = sAffectedStatTypeToCardStatType.at(mAffectedBoardCardsStatType);
        auto currentValue = mAffectedBoardCardsStatType == AffectedStatType::DAMAGE ? cardData.mCardDamage : cardData.mCardWeight;
        
        if (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.size()) >= affectedIndex + 1)
        {
            currentValue = mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides[affectedIndex][affectedStat];
        }
        else
        {
            mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.emplace_back();
        }
        
        mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.back()[affectedStat] = currentValue + mEffectValue;
    }
}

///------------------------------------------------------------------------------------------------