///------------------------------------------------------------------------------------------------
///  HeroCardEntryGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/01/2024
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/CardHistoryEntryAdditionGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/HeroCardEntryGameAction.h>
#include <game/GameRuleEngine.h>
#include <game/scenelogicmanagers/BattleSceneLogicManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CARD_HISTORY_ENTRY_ADDITION_GAME_ACTION_NAME = strutils::StringId("CardHistoryEntryAdditionGameAction");
static const strutils::StringId CARD_PLAY_PARTICLE_NAME = strutils::StringId("card_play");
static const strutils::StringId TOP_PLAYER_HEALTH_CONTAINER_BASE = strutils::StringId("health_crystal_top_base");
static const strutils::StringId TOP_PLAYER_HEALTH_CONTAINER_VALUE = strutils::StringId("health_crystal_top_value");

static const glm::vec3 HEALTH_VALUE_TEXT_OFFSET = {0.001, 0.001, 0.02f};
static const glm::vec3 HEALTH_BASE_OFFSET = {-0.0005f, 0.015f, 0.12f};

static const float CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float CARD_PLAY_PARTICLE_EMITTER_Z = 0.01f;
static const float IN_GAME_PLAYED_CARD_ANIMATION_DURATION = 0.5f;
static const float HEALTH_CONTAINER_INIT_SCALE_FACTOR = 0.5f;
static const float HEALTH_CRYSTAL_ANIMATION_DELAY_SECS = 0.5f;

///------------------------------------------------------------------------------------------------

void HeroCardEntryGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX];
    
    assert(!ProgressionDataRepository::GetInstance().GetNextStoryOpponentTexturePath().empty());
    
    CardData heroCardData;
    heroCardData.mCardFamily = game_constants::DEMONS_GENERIC_FAMILY_NAME;
    heroCardData.mCardId = 0; // to be filled by CardDataRepository
    heroCardData.mCardName = ProgressionDataRepository::GetInstance().GetNextStoryOpponentName();
    heroCardData.mCardDamage = ProgressionDataRepository::GetInstance().GetNextStoryOpponentDamage();
    heroCardData.mCardWeight = ProgressionDataRepository::GetInstance().GetNextBattleTopPlayerWeightLimit();
    heroCardData.mCardShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::DEFAULT_SHADER_NAME);
    heroCardData.mCardTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(ProgressionDataRepository::GetInstance().GetNextStoryOpponentTexturePath());
    
    mHeroCardId = CardDataRepository::GetInstance().InsertDynamicCardData(heroCardData);
    
    activePlayerState.mGoldenCardIds.push_back(mHeroCardId);
    activePlayerState.mPlayerBoardCards.push_back(mHeroCardId);
    mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.resize(activePlayerState.mPlayerBoardCards.size() + 1);
    
    mGameActionEngine->AddGameAction(CARD_HISTORY_ENTRY_ADDITION_GAME_ACTION_NAME,
    {
        { CardHistoryEntryAdditionGameAction::PLAYER_INDEX_PARAM, std::to_string(game_constants::REMOTE_PLAYER_INDEX) },
        { CardHistoryEntryAdditionGameAction::CARD_INDEX_PARAM, std::to_string(activePlayerState.mPlayerBoardCards.size() - 1) },
        { CardHistoryEntryAdditionGameAction::ENTRY_TYPE_TEXTURE_FILE_NAME_PARAM, CardHistoryEntryAdditionGameAction::ENTRY_TYPE_TEXTURE_FILE_NAME_DEATH },
        { CardHistoryEntryAdditionGameAction::IS_TURN_COUNTER_PARAM, "false"}
    });
}

///------------------------------------------------------------------------------------------------

void HeroCardEntryGameAction::VInitAnimation()
{
    mAnimationState = AnimationState::ANIMATING_HERO_CARD;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto scene = sceneManager.FindScene(game_constants::BATTLE_SCENE);
    
    auto heroCardSoWrapper = card_utils::CreateCardSoWrapper
    (
        &CardDataRepository::GetInstance().GetCardData(mHeroCardId)->get(),
        glm::vec3(0.0f, 1.0f, 0.0f),
        game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX + std::to_string(mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mPlayerBoardCards.size() - 1),
        CardOrientation::FRONT_FACE,
        card_utils::GetCardRarity(mHeroCardId, game_constants::REMOTE_PLAYER_INDEX, *mBoardState),
        false,
        true,
        true,
        {},
        {},
        *sceneManager.FindScene(game_constants::BATTLE_SCENE)
    );
    
    // Animate played card to board
    events::EventSystem::GetInstance().DispatchEvent<events::HeroCardCreatedEvent>(heroCardSoWrapper);
    heroCardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers()[game_constants::REMOTE_PLAYER_INDEX].at(0);
    
    const auto& boardCards = mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mPlayerBoardCards;
    const auto& deadBoardCardIndices = mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mBoardCardIndicesToDestroy;
    const auto nonDeadBoardCardCount = card_utils::CalculateNonDeadCardsCount(boardCards, deadBoardCardIndices);
    
    auto targetPosition = card_utils::CalculateBoardCardPosition(nonDeadBoardCardCount - 1, nonDeadBoardCardCount, true);
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(heroCardSoWrapper->mSceneObject, targetPosition, heroCardSoWrapper->mSceneObject->mScale * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR, IN_GAME_PLAYED_CARD_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)->GetCamera().Shake(CARD_CAMERA_SHAKE_DURATION, CARD_CAMERA_SHAKE_STRENGTH);
        CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
        (
            CARD_PLAY_PARTICLE_NAME,
            glm::vec3(targetPosition.x, targetPosition.y, CARD_PLAY_PARTICLE_EMITTER_Z),
            *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)
        );
        
        heroCardSoWrapper->mSceneObject->mShaderBoolUniformValues[game_constants::IS_HELD_CARD_UNIFORM_NAME] = false;
        mAnimationState = AnimationState::INITIALIZE_HEALTH_CRYSTAL_ANIMATION;
    });
    
    auto topHealthContainerBase = scene->FindSceneObject(TOP_PLAYER_HEALTH_CONTAINER_BASE);
    auto topHealthContainerValue = scene->FindSceneObject(TOP_PLAYER_HEALTH_CONTAINER_VALUE);
    
    mTargetHealthCrystalBasePosition = topHealthContainerBase->mPosition;
    mTargetHealthCrystalBaseScale = topHealthContainerBase->mScale;
    
    topHealthContainerBase->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    topHealthContainerBase->mScale *= HEALTH_CONTAINER_INIT_SCALE_FACTOR;
    
    mTargetHealthCrystalValuePosition = topHealthContainerValue->mPosition;
    mTargetHealthCrystalValueScale = topHealthContainerValue->mScale;
    
    topHealthContainerValue->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    topHealthContainerValue->mScale *= HEALTH_CONTAINER_INIT_SCALE_FACTOR;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult HeroCardEntryGameAction::VUpdateAnimation(const float)
{
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto scene = sceneManager.FindScene(game_constants::BATTLE_SCENE);
    
    auto topHealthContainerBase = scene->FindSceneObject(TOP_PLAYER_HEALTH_CONTAINER_BASE);
    auto topHealthContainerValue = scene->FindSceneObject(TOP_PLAYER_HEALTH_CONTAINER_VALUE);
    
    switch (mAnimationState)
    {
        case AnimationState::ANIMATING_HERO_CARD:
        {
            auto heroCardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers()[game_constants::REMOTE_PLAYER_INDEX].at(0);
            topHealthContainerBase->mPosition = heroCardSoWrapper->mSceneObject->mPosition + HEALTH_BASE_OFFSET;
            topHealthContainerValue->mPosition = topHealthContainerBase->mPosition + HEALTH_VALUE_TEXT_OFFSET;
            
            auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*topHealthContainerValue);
            topHealthContainerValue->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
        } break;
            
        case AnimationState::INITIALIZE_HEALTH_CRYSTAL_ANIMATION:
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(topHealthContainerBase, mTargetHealthCrystalBasePosition, mTargetHealthCrystalBaseScale, 1.0f, animation_flags::NONE, HEALTH_CRYSTAL_ANIMATION_DELAY_SECS, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){ mAnimationState = AnimationState::COMPLETE; });
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(topHealthContainerValue, mTargetHealthCrystalValuePosition, mTargetHealthCrystalValueScale, 1.0f, animation_flags::NONE, HEALTH_CRYSTAL_ANIMATION_DELAY_SECS, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            
            mAnimationState = AnimationState::ANIMATING_HEALTH_CRYSTAL;
        } break;

        default: break;
    }
    
    return mAnimationState == AnimationState::COMPLETE ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool HeroCardEntryGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& HeroCardEntryGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> extraParams;
    return extraParams;
}

///------------------------------------------------------------------------------------------------
