///------------------------------------------------------------------------------------------------
///  TrapTriggeredAnimationGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/11/2023
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/TrapTriggeredAnimationGameAction.h>
#include <game/gameactions/CardDestructionGameAction.h>
#include <game/gameactions/CardEffectGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

const std::string TrapTriggeredAnimationGameAction::LAST_PLAYED_CARD_INDEX_PARAM = "lastPlayedCardIndex";

static const strutils::StringId CARD_DESTRUCTION_GAME_ACTION_NAME = strutils::StringId("CardDestructionGameAction");
static const float ANIMATION_STEP_DURATION = 0.75f;
static const float ANIMATION_MAX_ALPHA = 0.7f;
static const float ANIMATION_STEP_1_SCALE_FACTOR = 1.5f;
static const float ANIMATION_STEP_2_SCALE_FACTOR = 1.2f;
static const float ANIMATION_STEP_1_ROTATION_INCREMENT = math::PI/5;
static const float ANIMATION_STEP_2_ROTATION_INCREMENT = -math::PI/3;

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void TrapTriggeredAnimationGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    assert(!activePlayerState.mPlayerHeldCards.empty());
    assert(mExtraActionParams.count(LAST_PLAYED_CARD_INDEX_PARAM) != 0);
    
    mGameActionEngine->AddGameAction(CARD_DESTRUCTION_GAME_ACTION_NAME,
    {
        { CardDestructionGameAction::CARD_INDICES_PARAM, {"[" + std::to_string(activePlayerState.mPlayerBoardCards.size() - 1) + "]"}},
        { CardDestructionGameAction::PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex())},
        { CardDestructionGameAction::IS_BOARD_CARD_PARAM, "true"},
    });
}

///------------------------------------------------------------------------------------------------

void TrapTriggeredAnimationGameAction::VInitAnimation()
{
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    const auto lastPlayedCardIndex = std::stoi(mExtraActionParams.at(LAST_PLAYED_CARD_INDEX_PARAM));
        
    mAnimationState = ActionState::KILL_SO_ANIMATION_STEP_1;
    
    auto killEffectSceneObject = activeScene->FindSceneObject(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(killEffectSceneObject->mName);
    
    auto lastPlayedCardSoWrapper = mGameSessionManager->GetHeldCardSoWrappers()[mBoardState->GetActivePlayerIndex()].at(lastPlayedCardIndex);
    
    auto targetPosition = killEffectSceneObject->mPosition;
    targetPosition.z = lastPlayedCardSoWrapper->mSceneObject->mPosition.z + 0.1f;
    
    auto targetScale = killEffectSceneObject->mScale * ANIMATION_STEP_1_SCALE_FACTOR;
    auto targetRotation = killEffectSceneObject->mRotation;
    targetRotation.z += ANIMATION_STEP_1_ROTATION_INCREMENT;
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(killEffectSceneObject, targetPosition, targetScale, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        mAnimationState = ActionState::KILL_SO_ANIMATION_STEP_2;
    });
    animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(killEffectSceneObject, targetRotation, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::BounceFunction, math::TweeningMode::EASE_IN), [=](){});
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(killEffectSceneObject, ANIMATION_MAX_ALPHA, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult TrapTriggeredAnimationGameAction::VUpdateAnimation(const float)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto killEffectSceneObject = activeScene->FindSceneObject(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    
    switch (mAnimationState)
    {
        case ActionState::KILL_SO_ANIMATION_STEP_1:
        {
             
        } break;
        
        case ActionState::KILL_SO_ANIMATION_STEP_2:
        {
            auto targetScale = killEffectSceneObject->mScale * ANIMATION_STEP_2_SCALE_FACTOR;
            auto targetRotation = killEffectSceneObject->mRotation;
            targetRotation.z += ANIMATION_STEP_2_ROTATION_INCREMENT;
            
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(killEffectSceneObject, killEffectSceneObject->mPosition, targetScale, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
            {
                mAnimationState = ActionState::KILL_SO_ANIMATION_STEP_2;
            });
            
            animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(killEffectSceneObject, targetRotation, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::BounceFunction, math::TweeningMode::EASE_IN), [=]()
            {
                mAnimationState = ActionState::FINISHED;
            });
            
            mAnimationState = ActionState::KILL_SO_ANIMATION_STEP_3;
        } break;
        
        case ActionState::KILL_SO_ANIMATION_STEP_3:
        {
            
        } break;
            
        case ActionState::FINISHED:
        {
            events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, effects::board_modifier_masks::KILL_NEXT);
            return ActionAnimationUpdateResult::FINISHED;
        }
    }
    
    return ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool TrapTriggeredAnimationGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& TrapTriggeredAnimationGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
