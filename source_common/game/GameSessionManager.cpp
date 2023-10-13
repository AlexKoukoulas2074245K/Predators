///------------------------------------------------------------------------------------------------
///  GameSessionManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
#include <game/CardUtils.h>
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

GameSessionManager::GameSessionManager()
{
    
}

///------------------------------------------------------------------------------------------------

GameSessionManager::~GameSessionManager(){}

///------------------------------------------------------------------------------------------------

void GameSessionManager::InitGameSession()
{
    mBoardState = std::make_unique<BoardState>();
    mBoardState->GetPlayerStates().emplace_back();
    mBoardState->GetPlayerStates().emplace_back();
    mBoardState->GetActivePlayerIndex() = 0;
    
    mPlayerCardSceneObjectWrappers.emplace_back();
    mPlayerCardSceneObjectWrappers.emplace_back();
    
    mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::ANIMATED, 0, mBoardState.get(), this);
    
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
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

void GameSessionManager::OnCardCreation(std::shared_ptr<CardSoWrapper> cardSoWrapper, const bool forOpponentPlayer)
{
    mPlayerCardSceneObjectWrappers[(forOpponentPlayer ? 0 : 1)].push_back(cardSoWrapper);
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GameSessionManager::GetCardSoWrappers() const
{
    return mPlayerCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

#if __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #define IOS_FLOW
    #endif
#endif

void GameSessionManager::HandleTouchInput()
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto activeScene = activeSceneManager.FindScene(strutils::StringId("Dummy"));
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(activeScene->GetCamera().GetViewMatrix(), activeScene->GetCamera().GetProjMatrix());
    
    auto& localPlayerCards = mPlayerCardSceneObjectWrappers[1];
    const auto localPlayerCardCount = static_cast<int>(localPlayerCards.size());
    
    std::vector<int> candidateHighlightIndices;
    
    for (int i = 0; i < localPlayerCardCount; ++i)
    {
        auto& currentCardSoWrapper = localPlayerCards.at(i);
        
        bool otherHighlightedCardExists = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper){ return cardSoWrapper.get() != currentCardSoWrapper.get() && cardSoWrapper->mState == CardSoState::HIGHLIGHTED; }) != localPlayerCards.cend();
        
        auto cardBaseSceneObject = currentCardSoWrapper->mSceneObjectComponents[0];
        auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*cardBaseSceneObject);
        
        bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);

#if defined(IOS_FLOW)
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && (currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED || currentCardSoWrapper->mState == CardSoState::FREE_MOVING))
        {
            currentCardSoWrapper->mState = CardSoState::FREE_MOVING;
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, glm::vec3(worldTouchPos.x, worldTouchPos.y + game_constants::IN_GAME_MOBILE_ONLY_FREE_MOVING_CARD_Y_OFFSET, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
        }
        else if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && cursorInSceneObject && !otherHighlightedCardExists)
        {
            candidateHighlightIndices.push_back(i);
        }
        else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
        {
            switch (currentCardSoWrapper->mState)
            {
                case CardSoState::FREE_MOVING:
                {
                    auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false);
                    animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                    currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                } break;
                case CardSoState::HIGHLIGHTED:
                {
                    if (!cursorInSceneObject)
                    {
                        auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false);
                        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                        currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                    }
                } break;
                    
                default: break;
            }
        }
        
#else
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && currentCardSoWrapper->mState == CardSoState::FREE_MOVING)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, glm::vec3(worldTouchPos.x, worldTouchPos.y, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
        }
        else if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && cursorInSceneObject && !otherHighlightedCardExists && currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED)
        {
            currentCardSoWrapper->mState = CardSoState::FREE_MOVING;
        }
        else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
        {
            switch (currentCardSoWrapper->mState)
            {
                case CardSoState::FREE_MOVING:
                {
                    auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false);
                    animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                    currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                } break;
                    
                case CardSoState::IDLE:
                {
                    if (cursorInSceneObject && !otherHighlightedCardExists)
                    {
                        candidateHighlightIndices.push_back(i);
                    }
                } break;
                case CardSoState::HIGHLIGHTED:
                {
                    if (!cursorInSceneObject)
                    {
                        auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false);
                        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                        currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                    }
                } break;
                    
                default: break;
            }
        }
#endif
    }
    
    // Select based candidate card to highlight based on distance from cursor
    std::sort(candidateHighlightIndices.begin(), candidateHighlightIndices.end(), [&](const int& lhs, const int& rhs)
    {
        return math::Abs(localPlayerCards[lhs]->mSceneObjectComponents[0]->mPosition.x - worldTouchPos.x) <
               math::Abs(localPlayerCards[rhs]->mSceneObjectComponents[0]->mPosition.x - worldTouchPos.x);
    });
    
    if (!candidateHighlightIndices.empty())
    {
        auto currentCardSoWrapper = localPlayerCards[candidateHighlightIndices.front()];
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(candidateHighlightIndices.front(), localPlayerCardCount, false);
        originalCardPosition.y += game_constants::IN_GAME_BOT_PLAYER_SELECTED_CARD_Y_OFFSET;
        originalCardPosition.z = game_constants::IN_GAME_HIGHLIGHTED_CARD_Z;
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
        currentCardSoWrapper->mState = CardSoState::HIGHLIGHTED;
    }
}

///------------------------------------------------------------------------------------------------
