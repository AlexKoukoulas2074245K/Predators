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

static const strutils::StringId CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME = strutils::StringId("CARD_LOCATION_INDICATOR");
static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId PLAY_CARD_ACTION_NAME = strutils::StringId("PlayCardGameAction");

static const std::string MAKE_SPACE_REVERT_TO_POSITION_ANIMATION_NAME_PREFIX = "MAKE_SPACE_REVERT_";
static const std::string BATTLE_ICON_TEXTURE_FILE_NAME = "battle_icon.png";
static const std::string CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX = "HIGHLIGHTER_CARD_";

static const float CARD_SELECTION_ANIMATION_DURATION = 0.2f;
static const float CARD_LOCATION_EFFECT_MIN_TARGET_ALPHA = 0.25f;
static const float CARD_LOCATION_EFFECT_MAX_TARGET_ALPHA = 1.0f;
static const float CARD_LOCATION_EFFECT_ALPHA_SPEED = 0.003f;

#if defined(IOS_FLOW)
static const float MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR = 0.004f;
#else
static const float DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR = 0.004f;
#endif

///------------------------------------------------------------------------------------------------

GameSessionManager::GameSessionManager()
    : mPreviousProspectiveBoardCardsPushState(ProspectiveBoardCardsPushState::NONE)
    , mShouldShowCardLocationIndicator(false)
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
    
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    
    mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::ANIMATED, 0, mBoardState.get(), this);
    
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
    
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
//    mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "4" }});
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "4" }});
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("DrawCardGameAction"));
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }});
    //mActionEngine->AddGameAction(strutils::StringId("PlayCardGameAction"), {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "2" }});
    
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto cardLocationIndicatorSo = activeScene->CreateSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    cardLocationIndicatorSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CARD_LOCATION_MASK_TEXTURE_NAME);
    cardLocationIndicatorSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BOARD_CARD_LOCATION_SHADER_NAME);
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_TIME_SPEED;
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_X_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_PERLIN_RESOLUTION;
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_Y_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_PERLIN_RESOLUTION;
    cardLocationIndicatorSo->mScale = glm::vec3(game_constants::IN_GAME_CARD_BASE_SCALE * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR);
    cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
    
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    cardLocationIndicatorSo->mInvisible = true;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::Update(const float dtMillis)
{
    HandleTouchInput();
    UpdateMiscSceneObjects(dtMillis);
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
    mPlayerHeldCardSceneObjectWrappers[(forOpponentPlayer ? 0 : 1)].push_back(cardSoWrapper);
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnHeldCardSwap(std::shared_ptr<CardSoWrapper> cardSoWrapper, const int cardIndex, const bool forOpponentPlayer)
{
    mPlayerHeldCardSceneObjectWrappers[(forOpponentPlayer ? 0 : 1)][cardIndex] = cardSoWrapper;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnLastCardPlayedFinalized(const int cardIndex)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    activeScene->RemoveSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(cardIndex)));
    
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
        if (currentCardSoWrapper->mState != CardSoState::FREE_MOVING)
        {
            auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, currentPlayerHeldCardCount, mBoardState->GetActivePlayerIndex() == 0);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){ currentCardSoWrapper->mState = CardSoState::IDLE; });
            currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
        }
    }
    
    const auto currentBoardCardCount = static_cast<int>(playerBoardCardSoWrappers.size());
    
    // Animate board cards to position. Last one will be animated externally
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

void GameSessionManager::HandleTouchInput()
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(activeScene->GetCamera().GetViewMatrix(), activeScene->GetCamera().GetProjMatrix());
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[1];
    const auto localPlayerCardCount = static_cast<int>(localPlayerCards.size());
    
    std::vector<int> candidateHighlightIndices;
    mShouldShowCardLocationIndicator = false;
    
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
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, glm::vec3(worldTouchPos.x, worldTouchPos.y + game_constants::IN_GAME_MOBILE_ONLY_FREE_MOVING_CARD_Y_OFFSET, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), currentCardSoWrapper->mSceneObjectComponents[0]->mScale, game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            auto currentLocalPlayerBoardCardCount = static_cast<int>(mPlayerBoardCardSceneObjectWrappers[1].size());
            auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
            cardLocationIndicatorSo->mPosition = card_utils::CalculateBoardCardPosition(currentLocalPlayerBoardCardCount, currentLocalPlayerBoardCardCount + 1, false);
            cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
            mShouldShowCardLocationIndicator = true;
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
                        //DestroyCardHighlighterAtIndex(i);
                    }
                } break;
                    
                default: break;
            }
        }
        
#else
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && currentCardSoWrapper->mState == CardSoState::FREE_MOVING)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, glm::vec3(worldTouchPos.x, worldTouchPos.y, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), currentCardSoWrapper->mSceneObjectComponents[0]->mScale, game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
            auto currentLocalPlayerBoardCardCount = static_cast<int>(mPlayerBoardCardSceneObjectWrappers[1].size());
            auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
            cardLocationIndicatorSo->mPosition = card_utils::CalculateBoardCardPosition(currentLocalPlayerBoardCardCount, currentLocalPlayerBoardCardCount + 1, false);
            cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
            mShouldShowCardLocationIndicator = true;
        }
        else if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) &&
                 cursorInSceneObject &&
                 !otherHighlightedCardExists &&
                 currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED &&
                 activeScene->FindSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i))) != nullptr)
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
                        DestroyCardHighlighterAtIndex(i);
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
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
        {
            CreateCardHighlighter();
        });
        
        currentCardSoWrapper->mState = CardSoState::HIGHLIGHTED;
    }
    
    // Additional constraints on showing the card location indicator
    mShouldShowCardLocationIndicator &= mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME;
    mShouldShowCardLocationIndicator &= mBoardState->GetActivePlayerIndex() == 1;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::UpdateMiscSceneObjects(const float dtMillis)
{
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Card Highlighters
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[1];
    for (size_t i = 0U; i < localPlayerCards.size(); ++i)
    {
        auto cardHighlighterObject = activeScene->FindSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
        if (cardHighlighterObject)
        {
            cardHighlighterObject->mInvisible = false;
            cardHighlighterObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
            cardHighlighterObject->mPosition = localPlayerCards[i]->mSceneObjectComponents[0]->mPosition;
            cardHighlighterObject->mPosition.z += game_constants::CARD_HIGLIGHTER_Z_OFFSET;
        }
    }
    
    // Lambda to make space/revert to original position board cards
    auto prospectiveMakeSpaceRevertToPositionLambda = [&](int prospectiveCardCount)
    {
        auto& boardCardSoWrappers = mPlayerBoardCardSceneObjectWrappers[1];
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        const auto currentBoardCardCount = static_cast<int>(boardCardSoWrappers.size());
        
        for (int i = 0; i < currentBoardCardCount; ++i)
        {
            auto animationName = strutils::StringId(MAKE_SPACE_REVERT_TO_POSITION_ANIMATION_NAME_PREFIX + std::to_string(i));
            auto& currentCardSoWrapper = boardCardSoWrappers.at(i);
            auto originalCardPosition = card_utils::CalculateBoardCardPosition(i, prospectiveCardCount, false);
            animationManager.StopAnimation(animationName);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(currentCardSoWrapper->mSceneObjectComponents, originalCardPosition, currentCardSoWrapper->mSceneObjectComponents[0]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){}, animationName);
        }
    };
    
    // Card Location
    auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    auto currentSoWrapperIter = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper){ return cardSoWrapper->mState == CardSoState::FREE_MOVING; });
    
    if (mShouldShowCardLocationIndicator && currentSoWrapperIter != localPlayerCards.end())
    {
        cardLocationIndicatorSo->mInvisible = false;
        cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
        
        auto distanceFromCardLocationSo = math::Distance2IgnoreZ((*currentSoWrapperIter)->mSceneObjectComponents.front()->mPosition, cardLocationIndicatorSo->mPosition);
#if defined(IOS_FLOW)
        bool inBoardDropThreshold = distanceFromCardLocationSo <= MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#else
        bool inBoardDropThreshold = distanceFromCardLocationSo <= DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#endif
        
        // If in drop threshold we lerp to max target location alpha
        if (inBoardDropThreshold)
        {
            cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
            if (cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= CARD_LOCATION_EFFECT_MAX_TARGET_ALPHA)
            {
                cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = CARD_LOCATION_EFFECT_MAX_TARGET_ALPHA;
            }
            
            if (mPreviousProspectiveBoardCardsPushState == ProspectiveBoardCardsPushState::MAKE_SPACE_FOR_NEW_CARD)
            {
                prospectiveMakeSpaceRevertToPositionLambda(static_cast<int>(mPlayerBoardCardSceneObjectWrappers[1].size() + 1));
            }
            mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::MAKE_SPACE_FOR_NEW_CARD;
        }
        else
        {
            // Else if not, we constrain the alpha to the min target
            if (math::Abs(cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] - CARD_LOCATION_EFFECT_MIN_TARGET_ALPHA) > dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED)
            {
                if (cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] > CARD_LOCATION_EFFECT_MIN_TARGET_ALPHA)
                {
                    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
                }
                else
                {
                    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
                }
            }
            
            if (mPreviousProspectiveBoardCardsPushState != ProspectiveBoardCardsPushState::REVERT_TO_ORIGINAL_POSITION)
            {
                prospectiveMakeSpaceRevertToPositionLambda(static_cast<int>(mPlayerBoardCardSceneObjectWrappers[1].size()));
            }
            mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::REVERT_TO_ORIGINAL_POSITION;
        }
    }
    else
    {
        cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
        if (cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
        {
            cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            cardLocationIndicatorSo->mInvisible = true;
        }
        
        mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::NONE;
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[1];
    auto cardIndex = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [=](const std::shared_ptr<CardSoWrapper>& otherCard)
    {
        return otherCard.get() == cardSoWrapper.get();
    }) - localPlayerCards.begin();
    
    DestroyCardHighlighterAtIndex(static_cast<int>(cardIndex));
    
    auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    auto distanceFromCardLocationSo = math::Distance2IgnoreZ(cardSoWrapper->mSceneObjectComponents.front()->mPosition, cardLocationIndicatorSo->mPosition);
    
#if defined(IOS_FLOW)
    bool inBoardDropThreshold = distanceFromCardLocationSo <= MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#else
    bool inBoardDropThreshold = distanceFromCardLocationSo <= DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#endif
    
    if (inBoardDropThreshold && mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME && mBoardState->GetActivePlayerIndex() == 1)
    {
        mActionEngine->AddGameAction(PLAY_CARD_ACTION_NAME, {{PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, std::to_string(cardIndex)}});
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

void GameSessionManager::CreateCardHighlighter()
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[1];
    for (size_t i = 0U; i < localPlayerCards.size(); ++i)
    {
        activeScene->RemoveSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
    }

    auto highlightedCardIter = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper)
    {
#if defined(IOS_FLOW)
        return cardSoWrapper->mState == CardSoState::FREE_MOVING;
#else
        return cardSoWrapper->mState == CardSoState::HIGHLIGHTED;
#endif
    });
    if (highlightedCardIter != localPlayerCards.cend())
    {
        auto cardIndex = highlightedCardIter - localPlayerCards.cbegin();
        auto cardHighlighterSo = activeScene->CreateSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(cardIndex)));
        
        cardHighlighterSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::CARD_HIGHLIGHTER_SHADER_NAME);
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::CARD_HIGLIGHTER_PERLIN_TIME_SPEED;
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_X_UNIFORM_NAME] = game_constants::CARD_HIGLIGHTER_PERLIN_RESOLUTION;
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_Y_UNIFORM_NAME] = game_constants::CARD_HIGLIGHTER_PERLIN_RESOLUTION;
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_CLARITY_UNIFORM_NAME] = game_constants::CARD_HIGLIGHTER_PERLIN_CLARITY;
        cardHighlighterSo->mPosition = (*highlightedCardIter)->mSceneObjectComponents[0]->mPosition;
        cardHighlighterSo->mPosition.z += game_constants::CARD_HIGLIGHTER_Z_OFFSET;
        cardHighlighterSo->mScale = game_constants::CARD_HIGHLIGHTER_SCALE;
        cardHighlighterSo->mInvisible = true;

    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::DestroyCardHighlighterAtIndex(const int index)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto cardHighlighterName = strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(index));
    activeScene->RemoveSceneObject(cardHighlighterName);
}

///------------------------------------------------------------------------------------------------
