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

static const float CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float CARD_PLAY_PARTICLE_EMITTER_Z = 0.01f;
static const float IN_GAME_PLAYED_CARD_ANIMATION_DURATION = 0.5f;

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
    mPendingAnimations = 0;
    
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
        mPendingAnimations--;
        CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)->GetCamera().Shake(CARD_CAMERA_SHAKE_DURATION, CARD_CAMERA_SHAKE_STRENGTH);
        CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
        (
            CARD_PLAY_PARTICLE_NAME,
            glm::vec3(targetPosition.x, targetPosition.y, CARD_PLAY_PARTICLE_EMITTER_Z),
            *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)
        );
        
        heroCardSoWrapper->mSceneObject->mShaderBoolUniformValues[game_constants::IS_HELD_CARD_UNIFORM_NAME] = false;
    });
    mPendingAnimations++;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult HeroCardEntryGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
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
