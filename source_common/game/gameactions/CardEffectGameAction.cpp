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
#include <game/gameactions/CardDestructionGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CARD_DESTRUCTION_GAME_ACTION_NAME = strutils::StringId("CardDestructionGameAction");

// Effect components
static const std::string EFFECT_COMPONENT_DAMAGE = "DAMAGE";
static const std::string EFFECT_COMPONENT_FAMILY = "FAMILY";
static const std::unordered_set<std::string> STATIC_EFFECT_COMPONENT_NAMES =
{
    EFFECT_COMPONENT_DAMAGE,
    EFFECT_COMPONENT_FAMILY
};

// Resources
static const std::string CARD_EFFECT_PARTICLE_TEXTURE_FILE_NAME = "sparkles.png";
static const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_spell_dissolve.vs";
static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const std::string BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX = "card_effect_emitter_";

// Uniforms
static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");

static const strutils::StringId CARD_EFFECT_PARTICLE_EMITTER_NAME = strutils::StringId("card_effect_emitter");


static const float CARD_DISSOLVE_SPEED = 0.0005f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;
static const float CARD_EFFECT_PARTICLE_EMITTER_Z_OFFSET = 1.0f;
static const float CONTINUOUS_PARTICLE_GENERATION_DELAY_SECS = 0.002f;

static const int CARD_EFFECT_PARTICLE_COUNT = 100;

static const glm::vec2 CARD_EFFECT_PARTICLE_LIFETIME_RANGE = {0.7f, 1.3f};
static const glm::vec2 CARD_EFFECT_PARTICLE_X_OFFSET_RANGE = {-0.02f, 0.01f};
static const glm::vec2 CARD_EFFECT_PARTICLE_Y_OFFSET_RANGE = {-0.03f, 0.03f};
static const glm::vec2 CARD_EFFECT_PARTICLE_SIZE_RANGE     = {0.0075f, 0.0125f};

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
          
            if (effectCardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::BoardCardDestructionWithRepositionEvent>(static_cast<int>(boardCardIndex), mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                
                // Fade particle emitter on spell
                rendering::RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, CARD_EFFECT_PARTICLE_EMITTER_NAME, *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
                
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
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, originalScale * 1.5f, 0.25f, animation_flags::NONE, i * 0.5f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::CardBuffedEvent>(static_cast<int>(i), true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
                        auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(mAffectedBoardIndices.at(i));
                        
                        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, originalScale, 0.25f, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                        {
                            if(i == mAffectedBoardIndices.size() - 1)
                            {
                                for (const auto affectedIndex: mAffectedBoardIndices)
                                {
                                    rendering::RemoveParticleEmitterFlag(particle_flags::CONTINUOUS_PARTICLE_GENERATION, strutils::StringId(BUFFED_CARD_PARTICLE_EMITTER_NAME_PREFIX + std::to_string(affectedIndex)), *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE));
                                }
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
    mAffectedBoardCardsStatOffset = 0;
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
        
        // Modifier/Offset value component
        if (!STATIC_EFFECT_COMPONENT_NAMES.count(effectComponent))
        {
            mAffectedBoardCardsStatOffset = std::stoi(effectComponent);
        }
                
        // Stat Type component
        if (effectComponent == EFFECT_COMPONENT_DAMAGE)
        {
            mAffectedBoardCardsStatType = AffectedStatType::DAMAGE;
        }
    }
    
    for (int affectedIndex: mAffectedBoardIndices)
    {
        const auto& cardData = CardDataRepository::GetInstance().GetCardData(boardCards.at(affectedIndex))->get();
        const auto affectedStat = mAffectedBoardCardsStatType == AffectedStatType::DAMAGE ? CardStatType::DAMAGE : CardStatType::WEIGHT;
        
        mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.emplace_back();
        
        switch (affectedStat)
        {
            case CardStatType::DAMAGE: mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.back()[affectedStat] = cardData.mCardDamage + mAffectedBoardCardsStatOffset; break;
            case CardStatType::WEIGHT: mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.back()[affectedStat] = cardData.mCardWeight + mAffectedBoardCardsStatOffset; break;
        }
    }
}

///------------------------------------------------------------------------------------------------
