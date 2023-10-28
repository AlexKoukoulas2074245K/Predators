///------------------------------------------------------------------------------------------------
///  CardAttackGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/CardAttackGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string CardAttackGameAction::CARD_INDEX_PARAM = "cardIndex";
const std::string CardAttackGameAction::PLAYER_INDEX_PARAM = "playerIndex";

static const strutils::StringId GAME_OVER_GAME_ACTION_NAME = strutils::StringId("GameOverGameAction");
static const float ATTACKING_CARD_ANIMATION_Y_OFFSET = 0.16f;
//static const float ATTACKING_CARD_ANIMATION_DURATION_SECS = 0.5f;

static const std::string ATTACKING_CARD_PARTICLE_TEXTURE_FILE_NAME = "smoke.png";

static const float ATTACKING_CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float ATTACKING_CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float ATTACKING_CARD_PARTICLE_EMITTER_Z = 0.01f;
static const float ATTACKING_CARD_SHORT_ANIMATION_DURATION = 0.25f;
static const float ATTACKING_CARD_LONG_ANIMATION_DURATION = 0.4f;
static const float ATTACKING_CARD_ANIMATION_ELEVATED_Z = 20.0f;

static const int ATTACKING_CARD_PARTICLE_COUNT = 20;

static const glm::vec2 ATTACKING_CARD_PARTICLE_LIFETIME_RANGE = {0.5f, 1.0f};
static const glm::vec2 ATTACKING_CARD_PARTICLE_X_OFFSET_RANGE = {-0.04f, -0.02f};
static const glm::vec2 ATTACKING_CARD_PARTICLE_Y_OFFSET_RANGE = {-0.05f, -0.01f};
static const glm::vec2 ATTACKING_CARD_PARTICLE_SIZE_RANGE     = {0.03f, 0.06f};

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
    
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    activePlayerState.mPlayerHealth -= attackingCardData->get().mCardDamage;
    
    if (activePlayerState.mPlayerHealth <= 0.0f)
    {
        mGameActionEngine->AddGameAction(GAME_OVER_GAME_ACTION_NAME, {{ GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM, std::to_string(attackingPayerIndex)}});
    }
    else
    {
        //TODO: handle card destruction game action
    }
    
}

///------------------------------------------------------------------------------------------------

void CardAttackGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    
    auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    
    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndex);
    
    mOriginalCardPosition = cardSoWrapper->mSceneObjectComponents[0]->mPosition;
    mOriginalCardScale = cardSoWrapper->mSceneObjectComponents[0]->mScale;
    
    // Enlargement animation
    {
        auto targetScale = mOriginalCardScale * 1.2f;
        auto targetPos = cardSoWrapper->mSceneObjectComponents[0]->mPosition;
        targetPos.z += ATTACKING_CARD_ANIMATION_ELEVATED_Z;
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObjectComponents, targetPos, targetScale, ATTACKING_CARD_SHORT_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
        {
            mPendingAnimations--;
            
            // Move to target position animation
            {
                auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
                auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
                auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndex);
                
                auto targetPos = cardSoWrapper->mSceneObjectComponents[0]->mPosition;
                targetPos.y += attackingPayerIndex == game_constants::LOCAL_PLAYER_INDEX ? ATTACKING_CARD_ANIMATION_Y_OFFSET : - ATTACKING_CARD_ANIMATION_Y_OFFSET;
                
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObjectComponents, targetPos, cardSoWrapper->mSceneObjectComponents.front()->mScale, ATTACKING_CARD_SHORT_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
                {
                    mPendingAnimations--;
                    
                    CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE)->GetCamera().Shake(ATTACKING_CARD_CAMERA_SHAKE_DURATION, ATTACKING_CARD_CAMERA_SHAKE_STRENGTH);
                    
                    auto cardIndex = std::stoi(mExtraActionParams.at(CARD_INDEX_PARAM));
                    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
                    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndex);
                    
                    rendering::CreateParticleEmitterAtPosition
                    (
                        glm::vec3(cardSoWrapper->mSceneObjectComponents.front()->mPosition.x, cardSoWrapper->mSceneObjectComponents.front()->mPosition.y, ATTACKING_CARD_PARTICLE_EMITTER_Z), // pos
                        ATTACKING_CARD_PARTICLE_LIFETIME_RANGE,         // particleLifetimeRange
                        ATTACKING_CARD_PARTICLE_X_OFFSET_RANGE,         // particlePositionXOffsetRange
                        ATTACKING_CARD_PARTICLE_Y_OFFSET_RANGE,         // particlePositionYOffsetRange
                        ATTACKING_CARD_PARTICLE_SIZE_RANGE,             // particleSizeRange
                        ATTACKING_CARD_PARTICLE_COUNT,                  // particleCount
                        ATTACKING_CARD_PARTICLE_TEXTURE_FILE_NAME,      // particleTextureFilename
                        *CoreSystemsEngine::GetInstance().GetActiveSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE), // scene
                        particle_flags::PREFILLED                  // particleFlags
                     );
                    
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObjectComponents, mOriginalCardPosition, mOriginalCardScale, ATTACKING_CARD_LONG_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [&]()
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
