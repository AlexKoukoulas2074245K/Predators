///------------------------------------------------------------------------------------------------
///  NextPlayerGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <game/GameConstants.h>
#include <game/gameactions/NextPlayerGameAction.h>

///------------------------------------------------------------------------------------------------

static const float TURN_POINTER_ANIMATION_DURATION_SECS = 1.0f;

///------------------------------------------------------------------------------------------------

void NextPlayerGameAction::VSetNewGameState()
{
    size_t& activePlayerIndex = mBoardState->GetActivePlayerIndex();
    activePlayerIndex = (activePlayerIndex + 1) % mBoardState->GetPlayerCount();
    
    mBoardState->GetTurnCounter()++;
    auto& targetPlayerState = mBoardState->GetPlayerStates()[mBoardState->GetTurnCounter() % mBoardState->GetPlayerCount()];
    targetPlayerState.mPlayerTotalWeightAmmo++;
    targetPlayerState.mPlayerCurrentWeightAmmo = targetPlayerState.mPlayerTotalWeightAmmo;
}

///------------------------------------------------------------------------------------------------

void NextPlayerGameAction::VInitAnimation()
{
    mPendingAnimations = 1;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto turnPointerSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(std::vector<std::shared_ptr<scene::SceneObject>>{turnPointerSo}, glm::vec3(0.0f, 0.0f, turnPointerSo->mRotation.z + math::PI), TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [&]()
    {
        mPendingAnimations--;
    });
    
    auto turnPointerHighlighterSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(std::vector<std::shared_ptr<scene::SceneObject>>{turnPointerHighlighterSo}, static_cast<float>(mBoardState->GetActivePlayerIndex()), TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult NextPlayerGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool NextPlayerGameAction::VShouldBeSerialized() const
{
    return true;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& NextPlayerGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
