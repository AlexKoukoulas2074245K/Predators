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
#include <game/gameactions/PlayCardGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

#if __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #define IOS_FLOW
    #endif
#endif

///------------------------------------------------------------------------------------------------

static const strutils::StringId BATTLE_ICON_SCENE_OBJECT_NAME = strutils::StringId("BATTLE_ICON");
static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");

static const std::string BATTLE_ICON_TEXTURE_FILE_NAME = "battle_icon.png";

static const float CARD_SELECTION_ANIMATION_DURATION = 0.2f;
static const float BATTLE_ICON_TARGET_ALPHA = 0.6f;
inline const float IN_GAME_BATTLE_ICON_SCALE = 0.1f;
inline const float IN_GAME_BATTLE_ICON_Z = 10.0f;
inline const float IN_GAME_BATTLE_ICON_ALPHA_SPEED = 0.003f;

#if defined(IOS_FLOW)
static const float BATTLE_ICON_PROMPT_MOBILE_DISTANCE_FROM_CENTER_THRESHOLD = 0.05f;
#else
static const float BATTLE_ICON_PROMPT_DISTANCE_FROM_CENTER_THRESHOLD = 0.07f;
#endif

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
    
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    
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
    mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "4" }});
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "2" }});
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto battleIcon = activeScene->CreateSceneObject(BATTLE_ICON_SCENE_OBJECT_NAME);
    battleIcon->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BATTLE_ICON_TEXTURE_FILE_NAME);
    battleIcon->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CUSTOM_ALPHA_SHADER_NAME);
    battleIcon->mScale.x = battleIcon->mScale.y = IN_GAME_BATTLE_ICON_SCALE;
    battleIcon->mPosition.z = IN_GAME_BATTLE_ICON_Z;
    battleIcon->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::Update(const float dtMillis)
{
    HandleTouchInput(dtMillis);
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
    if (!forOpponentPlayer)
    {
        cardSoWrapper->mCanBeHighlighted = true;
    }
    
    mPlayerHeldCardSceneObjectWrappers[(forOpponentPlayer ? 0 : 1)].push_back(cardSoWrapper);
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnLastCardPlayedFinalized(int cardIndex)
{
    auto& playerHeldCardSoWrappers = mPlayerHeldCardSceneObjectWrappers[mBoardState->GetActivePlayerIndex()];
    auto& playerBoardCardSoWrappers = mPlayerBoardCardSceneObjectWrappers[mBoardState->GetActivePlayerIndex()];
    
    playerBoardCardSoWrappers.push_back(playerHeldCardSoWrappers[cardIndex]);
    playerHeldCardSoWrappers.erase(playerHeldCardSoWrappers.begin() + cardIndex);
    
    const auto currentPlayerHeldCardCount = static_cast<int>(playerHeldCardSoWrappers.size());
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    for (int i = 0; i < currentPlayerHeldCardCount; ++i)
    {
        auto& currentCardSoWrapper = playerHeldCardSoWrappers.at(i);
        
        // Rename held cards for different indices
        auto newComponentNames = card_utils::GetCardComponentSceneObjectNames((mBoardState->GetActivePlayerIndex() == 0 ? game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX) + std::to_string(i), CardOrientation::FRONT_FACE);
        for (size_t j = 0; j < currentCardSoWrapper->mSceneObjectComponents.size(); ++j)
        {
            currentCardSoWrapper->mSceneObjectComponents[j]->mName = newComponentNames[j];
        }
        
        // Reposition held cards for different indices
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, currentPlayerHeldCardCount, mBoardState->GetActivePlayerIndex() == 0);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){ currentCardSoWrapper->mState = CardSoState::IDLE; });
        currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
    }
    
    const auto currentBoardCardCount = static_cast<int>(playerBoardCardSoWrappers.size());
    
    // Last one will be animated externally
    for (int i = 0; i < currentBoardCardCount - 1; ++i)
    {
        auto& currentCardSoWrapper = playerBoardCardSoWrappers.at(i);
        auto originalCardPosition = card_utils::CalculateBoardCardPosition(i, currentBoardCardCount, mBoardState->GetActivePlayerIndex() == 0);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
    }
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GameSessionManager::GetHeldCardSoWrappers() const
{
    return mPlayerHeldCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GameSessionManager::GetBoardCardSoWrappers() const
{
    return mPlayerBoardCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::HandleTouchInput(const float dtMillis)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(activeScene->GetCamera().GetViewMatrix(), activeScene->GetCamera().GetProjMatrix());
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[1];
    const auto localPlayerCardCount = static_cast<int>(localPlayerCards.size());
    
    std::vector<int> candidateHighlightIndices;
    
    bool battleIconFadingIn = false;
    
    for (int i = 0; i < localPlayerCardCount; ++i)
    {
        auto& currentCardSoWrapper = localPlayerCards.at(i);
        
        bool otherHighlightedCardExists = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper){ return cardSoWrapper.get() != currentCardSoWrapper.get() && cardSoWrapper->mState == CardSoState::HIGHLIGHTED; }) != localPlayerCards.cend();
        
        auto cardBaseSceneObject = currentCardSoWrapper->mSceneObjectComponents[0];
        auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*cardBaseSceneObject);
        
        bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
        
        if (!cursorInSceneObject)
        {
            currentCardSoWrapper->mCanBeHighlighted = true;
        }
        
#if defined(IOS_FLOW)
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && (currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED || currentCardSoWrapper->mState == CardSoState::FREE_MOVING))
        {
            currentCardSoWrapper->mState = CardSoState::FREE_MOVING;
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, glm::vec3(worldTouchPos.x, worldTouchPos.y + game_constants::IN_GAME_MOBILE_ONLY_FREE_MOVING_CARD_Y_OFFSET, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), currentCardSoWrapper->mSceneObjectComponents[0]->mScale, game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            
            auto distanceFromCenter = math::Abs(currentCardSoWrapper->mSceneObjectComponents.front()->mPosition.y);
            battleIconFadingIn = distanceFromCenter <= BATTLE_ICON_PROMPT_MOBILE_DISTANCE_FROM_CENTER_THRESHOLD;
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
                    OnFreeMovingCardRelease(currentCardSoWrapper);
                } break;
                case CardSoState::HIGHLIGHTED:
                {
                    if (!cursorInSceneObject)
                    {
                        auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false);
                        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                        currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                    }
                } break;
                    
                default: break;
            }
        }
        
#else
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && currentCardSoWrapper->mState == CardSoState::FREE_MOVING)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, glm::vec3(worldTouchPos.x, worldTouchPos.y, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), currentCardSoWrapper->mSceneObjectComponents[0]->mScale, game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            
            auto distanceFromCenter = math::Abs(currentCardSoWrapper->mSceneObjectComponents.front()->mPosition.y);
            battleIconFadingIn = distanceFromCenter <= BATTLE_ICON_PROMPT_DISTANCE_FROM_CENTER_THRESHOLD;
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
                    OnFreeMovingCardRelease(currentCardSoWrapper);
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
                        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
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
        
        if (currentCardSoWrapper->mCanBeHighlighted)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            currentCardSoWrapper->mCanBeHighlighted = false;
        }
        
        currentCardSoWrapper->mState = CardSoState::HIGHLIGHTED;
        
    }
    
    
    // Battle Icon
    battleIconFadingIn &= mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME;
    battleIconFadingIn &= mBoardState->GetActivePlayerIndex() == 1;
    auto battleIconSceneObject = activeScene->FindSceneObject(BATTLE_ICON_SCENE_OBJECT_NAME);
    if (battleIconFadingIn)
    {
        battleIconSceneObject->mInvisible = false;
        battleIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * IN_GAME_BATTLE_ICON_ALPHA_SPEED;
        if (battleIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= BATTLE_ICON_TARGET_ALPHA)
        {
            battleIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = BATTLE_ICON_TARGET_ALPHA;
        }
    }
    else
    {
        battleIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * IN_GAME_BATTLE_ICON_ALPHA_SPEED;
        if (battleIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
        {
            battleIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            battleIconSceneObject->mInvisible = true;
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper)
{
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[1];
    auto cardIndex = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [=](const std::shared_ptr<CardSoWrapper>& otherCard)
    {
        return otherCard.get() == cardSoWrapper.get();
    }) - localPlayerCards.begin();
    
    auto distanceFromCenter = math::Abs(cardSoWrapper->mSceneObjectComponents.front()->mPosition.y);
    
#if defined(IOS_FLOW)
    bool inBoardDropThreshold = distanceFromCenter <= BATTLE_ICON_PROMPT_MOBILE_DISTANCE_FROM_CENTER_THRESHOLD;
#else
    bool inBoardDropThreshold = distanceFromCenter <= BATTLE_ICON_PROMPT_DISTANCE_FROM_CENTER_THRESHOLD;
#endif
    
    if (inBoardDropThreshold && mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME && mBoardState->GetActivePlayerIndex() == 1)
    {
        mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, std::to_string(cardIndex)}});
    }
    else
    {
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(static_cast<int>(cardIndex), static_cast<int>(localPlayerCards.size()), false);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(cardSoWrapper->mSceneObjectComponents, originalCardPosition, cardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){cardSoWrapper->mState = CardSoState::IDLE; });
        cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
    }
}

///------------------------------------------------------------------------------------------------
