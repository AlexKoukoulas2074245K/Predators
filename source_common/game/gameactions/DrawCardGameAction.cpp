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
#include <game/GameConstants.h>
#include <game/GameSessionManager.h>
#include <game/gameactions/DrawCardGameAction.h>

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    auto availableCardDataCount = static_cast<int>(CardDataRepository::GetInstance().GetCardDataCount());
    activePlayerState.mPlayerHeldCards.push_back(math::ControlledRandomInt() % availableCardDataCount);
}

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VInitAnimation()
{
    mPendingAnimations = 0;
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    
    auto dummyScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    const int cardCount = static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCards.size());
    bool opponentPlayerActive = mBoardState->GetActivePlayerIndex() == 0;
    
    for (int i = 0; i < cardCount; ++i)
    {
        auto finalCardPosition = card_utils::CalculateHeldCardPosition(i, cardCount, opponentPlayerActive);
        
        // The latest added card components need to be created from scratch
        if (i == cardCount - 1)
        {
            int cardId = mBoardState->GetActivePlayerState().mPlayerHeldCards.back();
            auto cardOpt = CardDataRepository::GetInstance().GetCard(cardId);
            assert(cardOpt);
            
            auto cardSoWrapper = card_utils::CreateCardSoWrapper(
                &cardOpt->get(), glm::vec3(game_constants::IN_GAME_DRAW_CARD_INIT_X + i * game_constants::IN_GAME_CARD_WIDTH/2,
                opponentPlayerActive ? game_constants::IN_GAME_TOP_PLAYER_HELD_CARD_Y : game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y, finalCardPosition.z),
                (opponentPlayerActive ? game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX) + std::to_string(i),
                (opponentPlayerActive ? CardOrientation::BACK_FACE : CardOrientation::FRONT_FACE), *dummyScene);
            
            cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
            mGameSessionManager->OnCardCreation(cardSoWrapper, opponentPlayerActive);
            
            auto midPos = cardSoWrapper->mSceneObjectComponents.front()->mPosition;
            midPos.x = math::Abs(cardSoWrapper->mSceneObjectComponents.front()->mPosition.x - finalCardPosition.x)/2.0f;
            midPos.y = opponentPlayerActive ? game_constants::IN_GAME_DRAW_CARD_TOP_PLAYER_MID_POINT_Y : game_constants::IN_GAME_DRAW_CARD_BOT_PLAYER_MID_POINT_Y;
            
            math::BezierCurve curve(std::vector<glm::vec3>{cardSoWrapper->mSceneObjectComponents.front()->mPosition, midPos, finalCardPosition});
            
            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(cardSoWrapper->mSceneObjectComponents, curve, game_constants::IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_Z_COMPONENT), [=]()
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
            auto cardSoWrapper = mGameSessionManager->GetHeldCardSoWrappers()[(opponentPlayerActive ? 0 : 1)][i];
            
            if (cardSoWrapper->mState != CardSoState::FREE_MOVING)
            {
                if (cardSoWrapper->mState != CardSoState::HIGHLIGHTED)
                {
                    cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                }
                
                animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(cardSoWrapper->mSceneObjectComponents, finalCardPosition, cardSoWrapper->mSceneObjectComponents[0]->mScale, game_constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DURATION_SECS, animation_flags::INITIAL_OFFSET_BASED_ADJUSTMENT | animation_flags::IGNORE_Y_COMPONENT, game_constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DELAY_SECS, math::QuadFunction), [=]()
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

const std::vector<std::string>& DrawCardGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
