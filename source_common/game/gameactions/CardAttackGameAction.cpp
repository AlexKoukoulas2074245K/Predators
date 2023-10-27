///------------------------------------------------------------------------------------------------
///  CardAttackGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/CardAttackGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string CardAttackGameAction::CARD_INDEX_PARAM = "cardIndex";
const std::string CardAttackGameAction::PLAYER_INDEX_PARAM = "playerIndex";

static const float ATTACKING_CARD_ANIMATION_Y_OFFSET = 0.2f;
static const float ATTACKING_CARD_ANIMATION_DURATION_SECS = 0.5f;
//static const std::string CARD_PLAY_PARTICLE_TEXTURE_FILE_NAME = "smoke.png";
//
//static const float CARD_CAMERA_SHAKE_DURATION = 0.25f;
//static const float CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
//static const float CARD_PLAY_PARTICLE_EMITTER_Z = 0.01f;
//
//static const int CARD_PLAY_PARTICLE_COUNT = 20;
//
//static const glm::vec2 CARD_PLAY_PARTICLE_LIFETIME_RANGE = {0.5f, 1.0f};
//static const glm::vec2 CARD_PLAY_PARTICLE_X_OFFSET_RANGE = {-0.04f, -0.02f};
//static const glm::vec2 CARD_PLAY_PARTICLE_Y_OFFSET_RANGE = {-0.05f, -0.01f};
//static const glm::vec2 CARD_PLAY_PARTICLE_SIZE_RANGE     = {0.03f, 0.06f};

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
        //TODO: handle death
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
    
    auto secondPos = cardSoWrapper->mSceneObjectComponents.front()->mPosition;
    secondPos.y += attackingPayerIndex == game_constants::LOCAL_PLAYER_INDEX ? ATTACKING_CARD_ANIMATION_Y_OFFSET : -ATTACKING_CARD_ANIMATION_Y_OFFSET;
    
    auto thirdPos = cardSoWrapper->mSceneObjectComponents.front()->mPosition;
    thirdPos.y = (secondPos.y - cardSoWrapper->mSceneObjectComponents.front()->mPosition.y)/2.0f;
    
    math::BezierCurve curve(std::vector<glm::vec3>{cardSoWrapper->mSceneObjectComponents.front()->mPosition, secondPos, thirdPos, cardSoWrapper->mSceneObjectComponents.front()->mPosition});
    
    animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(cardSoWrapper->mSceneObjectComponents, curve, ATTACKING_CARD_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT), [=]()
    {
        mPendingAnimations--;
    });
    mPendingAnimations = 1;
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
