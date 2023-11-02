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
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/gameactions/CardAttackGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/NextPlayerGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CARD_ATTACK_GAME_ACTION_NAME = strutils::StringId("CardAttackGameAction");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");
static const strutils::StringId POST_NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("PostNextPlayerGameAction");

///------------------------------------------------------------------------------------------------

void NextPlayerGameAction::VSetNewGameState()
{
    int& activePlayerIndex = mBoardState->GetActivePlayerIndex();
    const auto previousPlayerIndex = activePlayerIndex;
    activePlayerIndex = (activePlayerIndex + 1) % mBoardState->GetPlayerCount();
    
    mBoardState->GetTurnCounter()++;
    auto& targetPlayerState = mBoardState->GetPlayerStates()[mBoardState->GetTurnCounter() % mBoardState->GetPlayerCount()];
    targetPlayerState.mPlayerTotalWeightAmmo++;
    targetPlayerState.mPlayerCurrentWeightAmmo = targetPlayerState.mPlayerTotalWeightAmmo;
    
    // Potentially generate card attack actions for player whose turn was just ended
    if (previousPlayerIndex != -1)
    {
        auto& boardCards = mBoardState->GetPlayerStates()[previousPlayerIndex].mPlayerBoardCards;
        for (size_t i = 0; i < boardCards.size(); ++i)
        {
            mGameActionEngine->AddGameAction(CARD_ATTACK_GAME_ACTION_NAME,
            {
                { CardAttackGameAction::PLAYER_INDEX_PARAM, std::to_string(previousPlayerIndex) },
                { CardAttackGameAction::CARD_INDEX_PARAM, "0" }
            });
        }
    }
    
    mGameActionEngine->AddGameAction(POST_NEXT_PLAYER_GAME_ACTION_NAME);
    
    mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    
    // First time top player gets 2 cards in total
    if (previousPlayerIndex == -1)
    {
        mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    }
    // First time bot player gets 4 cards in total
    else if (previousPlayerIndex == 0 && mBoardState->GetActivePlayerState().mPlayerTotalWeightAmmo == 1)
    {
        mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
        mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
        mGameActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    }
}

///------------------------------------------------------------------------------------------------

void NextPlayerGameAction::VInitAnimation()
{
    mPendingAnimations = 1;
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto turnPointerSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    bool localPlayerActive = mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX;
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(turnPointerSo, glm::vec3(0.0f, 0.0f, turnPointerSo->mRotation.z + (localPlayerActive ? math::PI/2 : -math::PI/2)), game_constants::TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=]()
    {
        mPendingAnimations--;
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
        auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
        
        auto turnPointerHighlighterSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(turnPointerHighlighterSo, 0.0f, game_constants::TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
    });
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
