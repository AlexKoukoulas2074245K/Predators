///------------------------------------------------------------------------------------------------
///  GameSessionManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
#include <game/GameConstants.h>
#include <game/GameSessionManager.h>
#include <game/gameactions/GameActionEngine.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

static const float CARD_SELECTION_ANIMATION_DURATION = 0.2f;

///------------------------------------------------------------------------------------------------

GameSessionManager::GameSessionManager(){}

///------------------------------------------------------------------------------------------------

GameSessionManager::~GameSessionManager(){}

///------------------------------------------------------------------------------------------------

void GameSessionManager::InitGameSession()
{
    mBoardState = std::make_unique<BoardState>();
    mBoardState->GetPlayerStates().emplace_back();
    mBoardState->GetPlayerStates().emplace_back();
    mBoardState->GetActivePlayerIndex() = 0;
    
    mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::ANIMATED, 0, mBoardState.get());
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::Update(const float dtMillis)
{
    HandleTouchInput();
    mActionEngine->Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

const BoardState& GameSessionManager::GetBoardState() const
{
    return *mBoardState;
}

///------------------------------------------------------------------------------------------------

GameActionEngine& GameSessionManager::GetActionEngine()
{
    return *mActionEngine;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::HandleTouchInput()
{
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto activeScene = activeSceneManager.FindScene(strutils::StringId("Dummy"));
    
    for (size_t i = 0; i < mBoardState->GetPlayerStates()[1].mPlayerHeldCards.size(); ++i)
    {
        auto currentCardComponents = activeScene->FindAllSceneObjectsWithNamePrefixedBy(game_constants::BOT_PLAYER_CARD_SO_NAME_PREFIX + std::to_string(i));
        auto worldTouchPos = CoreSystemsEngine::GetInstance().GetInputStateManager().VGetPointingPosInWorldSpace(activeScene->GetCamera().GetViewMatrix(), activeScene->GetCamera().GetProjMatrix());
        
        auto cardBaseSceneObject = *currentCardComponents[0];
        auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*currentCardComponents[0]);
        
        bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
        auto cardAnimationName = strutils::StringId(game_constants::CARD_ANIMATION_PREFIX + std::to_string(i));
         
        if (cursorInSceneObject && !animationManager.IsAnimationPlaying(cardAnimationName))
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardComponents, glm::vec3(cardBaseSceneObject.mPosition.x, game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y + game_constants::IN_GAME_BOT_PLAYER_SELECTED_CARD_Y_OFFSET, cardBaseSceneObject.mPosition.z), CARD_SELECTION_ANIMATION_DURATION, true, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){}, cardAnimationName);
        }
        else if (!cursorInSceneObject && !animationManager.IsAnimationPlaying(cardAnimationName) && cardBaseSceneObject.mPosition.y > game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardComponents, glm::vec3(currentCardComponents[0]->mPosition.x, game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y, currentCardComponents[0]->mPosition.z), CARD_SELECTION_ANIMATION_DURATION, true, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){}, cardAnimationName);
        }
    }
}

///------------------------------------------------------------------------------------------------
