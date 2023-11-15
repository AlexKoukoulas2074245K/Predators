///------------------------------------------------------------------------------------------------
///  DrawCardGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/MathUtils.h>
#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/GameRuleEngine.h>
#include <game/GameSessionManager.h>
#include <game/gameactions/DrawCardGameAction.h>

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    auto availableCardDataCount = static_cast<int>(activePlayerState.mPlayerDeckCards.size());
    auto randomCardIndex = math::ControlledRandomInt() % availableCardDataCount;
    
    activePlayerState.mPlayerHeldCards.push_back(activePlayerState.mPlayerDeckCards.at(randomCardIndex));
}

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VInitAnimation()
{
    mPendingAnimations = 0;
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    const int cardCount = static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCards.size());
    bool remotePlayerActive = mBoardState->GetActivePlayerIndex() == 0;
    
    for (int i = 0; i < cardCount; ++i)
    {
        auto finalCardPosition = card_utils::CalculateHeldCardPosition(i, cardCount, remotePlayerActive);
        
        // The latest added card components need to be created from scratch
        if (i == cardCount - 1)
        {
            int cardId = mBoardState->GetActivePlayerState().mPlayerHeldCards.back();
            auto cardOpt = CardDataRepository::GetInstance().GetCardData(cardId);
            assert(cardOpt);
            
            auto cardSoWrapper = card_utils::CreateCardSoWrapper(
                &cardOpt->get(), glm::vec3(game_constants::IN_GAME_DRAW_CARD_INIT_X + i * game_constants::IN_GAME_CARD_WIDTH/2,
                remotePlayerActive ? game_constants::IN_GAME_TOP_PLAYER_HELD_CARD_Y : game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y, finalCardPosition.z),
                (remotePlayerActive ? game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX) + std::to_string(i), (remotePlayerActive ? CardOrientation::BACK_FACE : CardOrientation::FRONT_FACE), remotePlayerActive, mGameRuleEngine->CanCardBePlayed(&cardOpt->get(), mBoardState->GetActivePlayerIndex()), {}, mBoardState->GetActivePlayerState().mGlobalBoardCardStatModifiers, *activeScene);
            
            cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
            events::EventSystem::GetInstance().DispatchEvent<events::CardCreationEvent>(cardSoWrapper, remotePlayerActive);
            
            auto midPos = cardSoWrapper->mSceneObject->mPosition;
            midPos.x = math::Abs(cardSoWrapper->mSceneObject->mPosition.x - finalCardPosition.x)/2.0f;
            midPos.y = remotePlayerActive ? game_constants::IN_GAME_DRAW_CARD_TOP_PLAYER_MID_POINT_Y : game_constants::IN_GAME_DRAW_CARD_BOT_PLAYER_MID_POINT_Y;
            
            math::BezierCurve curve(std::vector<glm::vec3>{cardSoWrapper->mSceneObject->mPosition, midPos, finalCardPosition});
            
            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(cardSoWrapper->mSceneObject, curve, game_constants::IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS, animation_flags::IGNORE_Z_COMPONENT), [=]()
            {
                mPendingAnimations--;
                if (cardSoWrapper->mState != CardSoState::FREE_MOVING)
                {
                    cardSoWrapper->mState = CardSoState::IDLE;
                }
            });
            mPendingAnimations++;
        }
        // .. The rest can be looked up
        else
        {
            auto cardSoWrapper = mGameSessionManager->GetHeldCardSoWrappers()[(remotePlayerActive ? 0 : 1)][i];
            
            if (cardSoWrapper->mState != CardSoState::FREE_MOVING)
            {
                if (cardSoWrapper->mState != CardSoState::HIGHLIGHTED)
                {
                    cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                }
                
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, finalCardPosition, cardSoWrapper->mSceneObject->mScale, game_constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DURATION_SECS, animation_flags::IGNORE_Y_COMPONENT, game_constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DELAY_SECS, math::QuadFunction), [=]()
                {
                    mPendingAnimations--;
                    if (cardSoWrapper->mState != CardSoState::FREE_MOVING && cardSoWrapper->mState != CardSoState::HIGHLIGHTED)
                    {
                        cardSoWrapper->mState = CardSoState::IDLE;
                    }
                });
                mPendingAnimations++;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult DrawCardGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool DrawCardGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& DrawCardGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
