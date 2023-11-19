///------------------------------------------------------------------------------------------------
///  PostNextPlayerGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/11/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PostNextPlayerGameAction.h>


///------------------------------------------------------------------------------------------------

static const float TURN_POINTER_ANIMATION_DURATION_SECS = 0.66f;

///------------------------------------------------------------------------------------------------

void PostNextPlayerGameAction::VSetNewGameState()
{
    // Clear board modifiers
    mBoardState->GetInactivePlayerState().mPlayerBoardCardStatOverrides.clear();
    mBoardState->GetInactivePlayerState().mPlayerHeldCardStatOverrides.clear();
    mBoardState->GetInactivePlayerState().mBoardModifiers.mGlobalCardStatModifiers.clear();
    mBoardState->GetInactivePlayerState().mBoardModifiers.mBoardModifierMask = effects::board_modifier_masks::NONE;
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX, effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER);
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX, effects::board_modifier_masks::KILL_NEXT);
    events::EventSystem::GetInstance().DispatchEvent<events::WeightChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
}

///------------------------------------------------------------------------------------------------

void PostNextPlayerGameAction::VInitAnimation()
{
    mPendingAnimations = 1;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto turnPointerSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    bool localPlayerActive = mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX;
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(turnPointerSo, glm::vec3(0.0f, 0.0f, turnPointerSo->mRotation.z + (localPlayerActive ? math::PI/2 : -math::PI/2)), TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=]()
    {
        mPendingAnimations--;
        
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
        auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
        bool localPlayerActive = mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX;
        if (localPlayerActive)
        {
            auto turnPointerHighlighterSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(turnPointerHighlighterSo, 1.0f, TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), []()
            {
                events::EventSystem::GetInstance().DispatchEvent<events::LocalPlayerTurnStarted>();
            });
        }
    });
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PostNextPlayerGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool PostNextPlayerGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& PostNextPlayerGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
