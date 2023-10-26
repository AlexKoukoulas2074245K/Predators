///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM = "lastPlayedCardIndex";

static const std::string CARD_PLAY_PARTICLE_TEXTURE_FILE_NAME = "smoke.png";

static const float CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float CARD_PLAY_PARTICLE_EMITTER_Z = 0.01f;

static const int CARD_PLAY_PARTICLE_COUNT = 20;

static const glm::vec2 CARD_PLAY_PARTICLE_LIFETIME_RANGE = {0.5f, 1.0f};
static const glm::vec2 CARD_PLAY_PARTICLE_X_OFFSET_RANGE = {-0.04f, -0.02f};
static const glm::vec2 CARD_PLAY_PARTICLE_Y_OFFSET_RANGE = {-0.05f, -0.01f};
static const glm::vec2 CARD_PLAY_PARTICLE_SIZE_RANGE     = {0.03f, 0.06f};

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
    
    activePlayerState.mPlayerBoardCards.push_back(cardId);
    activePlayerState.mPlayerHeldCards.erase(activePlayerState.mPlayerHeldCards.begin() + lastPlayedCardIndex);
    activePlayerState.mPlayerCurrentWeightAmmo -= CardDataRepository::GetInstance().GetCardData(cardId)->get().mCardWeight;
}

///------------------------------------------------------------------------------------------------

void PlayCardGameAction::VInitAnimation()
{
    mPendingAnimations = 0;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    const auto lastPlayedCardIndex = std::stoi(mExtraActionParams.at(LAST_PLAYED_CARD_INDEX_PARAM));
    
    mLastPlayedCardSoWrapper = mGameSessionManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(lastPlayedCardIndex);
    
    // For opponent plays, the front face card components also need to be created
    if (mBoardState->GetActivePlayerIndex() == 0)
    {
        activeScene->RemoveSceneObject(mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mName);
        mLastPlayedCardSoWrapper = card_utils::CreateCardSoWrapper(mLastPlayedCardSoWrapper->mCardData, mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mPosition, game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX + std::to_string(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), CardOrientation::FRONT_FACE, *activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE));
        mGameSessionManager->OnHeldCardSwap(mLastPlayedCardSoWrapper, lastPlayedCardIndex, true);
    }
    
    mGameSessionManager->OnLastCardPlayedFinalized(lastPlayedCardIndex);
    
    
    // Rename played card components
    auto newComponentNames = card_utils::GetCardComponentSceneObjectNames((mBoardState->GetActivePlayerIndex() == 0 ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), CardOrientation::FRONT_FACE);
    for (size_t i = 0; i < mLastPlayedCardSoWrapper->mSceneObjectComponents.size(); ++i)
    {
        mLastPlayedCardSoWrapper->mSceneObjectComponents[i]->mName = newComponentNames[i];
    }
    
    // Animate played card to board
    auto targetPosition = card_utils::CalculateBoardCardPosition(static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size() - 1), static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCards.size()), mBoardState->GetActivePlayerIndex() == 0);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(mLastPlayedCardSoWrapper->mSceneObjectComponents, targetPosition, mLastPlayedCardSoWrapper->mSceneObjectComponents[0]->mScale * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR, game_constants::IN_GAME_PLAYED_CARD_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        mPendingAnimations--;
        CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)->GetCamera().Shake(CARD_CAMERA_SHAKE_DURATION, CARD_CAMERA_SHAKE_STRENGTH);
        
        rendering::CreateParticleEmitterAtPosition
        (
            glm::vec3(targetPosition.x, targetPosition.y, CARD_PLAY_PARTICLE_EMITTER_Z), // pos
            CARD_PLAY_PARTICLE_LIFETIME_RANGE,         // particleLifetimeRange
            CARD_PLAY_PARTICLE_X_OFFSET_RANGE,         // particlePositionXOffsetRange
            CARD_PLAY_PARTICLE_Y_OFFSET_RANGE,         // particlePositionYOffsetRange
            CARD_PLAY_PARTICLE_SIZE_RANGE,             // particleSizeRange
            CARD_PLAY_PARTICLE_COUNT,                  // particleCount
            CARD_PLAY_PARTICLE_TEXTURE_FILE_NAME,      // particleTextureFilename
            *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
            particle_flags::PREFILLED                  // particleFlags
         );
    });
    mPendingAnimations++;
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PlayCardGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& PlayCardGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
