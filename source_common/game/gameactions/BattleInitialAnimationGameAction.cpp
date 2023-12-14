///------------------------------------------------------------------------------------------------
///  BattleInitialAnimationGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/BattleInitialAnimationGameAction.h>
#include <game/scenelogicmanagers/BattleSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId PERMANENT_BOARD_SCENE = strutils::StringId("permanent_board_scene");
static const strutils::StringId BOARD_SCENE_OBJECT_NAME = strutils::StringId("board");

static const glm::vec3 BOARD_TARGET_POSITION = {-0.013f, 0.003f, 0.0f };
static const glm::vec3 BOARD_TARGET_ROTATION = {0.00f, 0.000f, math::PI/2 };

static const float BOARD_ANIMATION_DURATION_SECS = 1.0f;
static const float BOARD_ITEMS_FADE_IN_DURATION_SECS = 0.5f;

///------------------------------------------------------------------------------------------------

void BattleInitialAnimationGameAction::VSetNewGameState()
{
}

///------------------------------------------------------------------------------------------------

void BattleInitialAnimationGameAction::VInitAnimation()
{
    auto permanentBoardScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(PERMANENT_BOARD_SCENE);
    auto boardSceneObject = permanentBoardScene->FindSceneObject(BOARD_SCENE_OBJECT_NAME);
    
    boardSceneObject->mPosition = game_constants::GAME_BOARD_INIT_POSITION;
    boardSceneObject->mRotation = game_constants::GAME_BOARD_INIT_ROTATION;
    
    permanentBoardScene->GetCamera().SetZoomFactor(game_constants::GAME_BOARD_BASED_SCENE_ZOOM_FACTOR);
    
    // Animate board to target position
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(boardSceneObject, BOARD_TARGET_POSITION, boardSceneObject->mScale, BOARD_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        mPendingAnimations--;
    });
    mPendingAnimations++;
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(boardSceneObject, BOARD_TARGET_ROTATION, BOARD_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        mPendingAnimations--;
    });
    mPendingAnimations++;
    
    // Fade in board scene objects with a delay matching the duration of the board animation
    auto battleScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    for (auto& sceneObject: battleScene->GetSceneObjects())
    {
        // Only fade in normally visible elements
        if (sceneObject->mInvisible || (sceneObject->mShaderFloatUniformValues.count(game_constants::CUSTOM_ALPHA_UNIFORM_NAME) && sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f))
        {
            continue;
        }
        
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, BOARD_ITEMS_FADE_IN_DURATION_SECS, animation_flags::NONE, BOARD_ANIMATION_DURATION_SECS), [=]()
        {
            mPendingAnimations--;
        });
        mPendingAnimations++;
    }
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult BattleInitialAnimationGameAction::VUpdateAnimation(const float)
{
    
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool BattleInitialAnimationGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& BattleInitialAnimationGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
