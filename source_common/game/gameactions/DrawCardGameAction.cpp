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
#include <game/gameactions/DrawCardGameAction.h>

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    auto availableCardCount = static_cast<int>(CardRepository::GetInstance().GetCardCount());
    activePlayerState.mPlayerHeldCards.push_back(math::ControlledRandomInt() % availableCardCount);
}

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VInitAnimation()
{
    mPendingAnimations = 0;
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    
    auto dummyScene = activeSceneManager.FindScene(strutils::StringId("Dummy"));
    
    const int cardCount = static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCards.size());
    
    float cardBlockWidth = game_constants::IN_GAME_CARD_WIDTH * cardCount;
    float cardStartX = -cardBlockWidth/2.0f;
    
    bool topPlayerActive = mBoardState->GetActivePlayerIndex() == 0;
    
    for (int i = 0; i < cardCount; ++i)
    {
        // Calculate positioning vars
        auto targetX = cardStartX + i * game_constants::IN_GAME_CARD_WIDTH + game_constants::IN_GAME_CARD_WIDTH/2;
        if (cardCount > game_constants::IN_GAME_CARD_PUSH_THRESHOLD)
        {
            float pushX = (cardCount - game_constants::IN_GAME_CARD_PUSH_THRESHOLD) * game_constants::IN_GAME_CARD_PUSH_VALUE * (math::Abs(i - cardCount/2));
            bool oddCardCount = cardCount % 2 != 0;
            if ((oddCardCount && i != cardCount/2) || !oddCardCount)
            {
                targetX += (i < cardCount/2) ? pushX : -pushX;
            }
        }
        
        std::vector<std::shared_ptr<scene::SceneObject>> cardComponents;
        const auto& sceneObjectComponentNames = card_utils::GetCardComponentSceneObjectNames((topPlayerActive ? game_constants::TOP_PLAYER_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_CARD_SO_NAME_PREFIX) + std::to_string(i), topPlayerActive ? CardOrientation::BACK_FACE : CardOrientation::FRONT_FACE);
        
        auto cardAnimationName = strutils::StringId(game_constants::CARD_ANIMATION_PREFIX + std::to_string(i));
        
        // The latest added card components need to be created from scratch
        if (i == cardCount - 1)
        {
            int cardId = mBoardState->GetActivePlayerState().mPlayerHeldCards.back();
            auto cardOpt = CardRepository::GetInstance().GetCard(cardId);
            if (cardOpt)
            {
                if (topPlayerActive)
                {
                    cardComponents = card_utils::CreateCardComponentSceneObjects(&cardOpt->get(), glm::vec3(game_constants::IN_GAME_DRAW_CARD_INIT_X + i * game_constants::IN_GAME_CARD_WIDTH/2, game_constants::IN_GAME_TOP_PLAYER_HELD_CARD_Y, game_constants::IN_GAME_HELD_CARD_Z), game_constants::TOP_PLAYER_CARD_SO_NAME_PREFIX + std::to_string(i), CardOrientation::BACK_FACE, *dummyScene);
                }
                else
                {
                    auto card = cardOpt->get();
                    cardComponents = card_utils::CreateCardComponentSceneObjects(&card, glm::vec3(game_constants::IN_GAME_DRAW_CARD_INIT_X + i * game_constants::IN_GAME_CARD_WIDTH/2, game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y, game_constants::IN_GAME_HELD_CARD_Z), game_constants::BOT_PLAYER_CARD_SO_NAME_PREFIX + std::to_string(i), CardOrientation::FRONT_FACE, *dummyScene);
                }
            }
            
            auto midPos = cardComponents.front()->mPosition;
            midPos.x = math::Abs(cardComponents.front()->mPosition.x - targetX)/2.0f;
            midPos.y = topPlayerActive ? game_constants::IN_GAME_DRAW_CARD_TOP_PLAYER_MID_POINT_Y : game_constants::IN_GAME_DRAW_CARD_BOT_PLAYER_MID_POINT_Y;
            
            math::BezierCurve curve(std::vector<glm::vec3>{cardComponents.front()->mPosition, midPos, glm::vec3(targetX, cardComponents.front()->mPosition.y, game_constants::IN_GAME_HELD_CARD_Z)});

            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(cardComponents, curve, game_constants::IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS, true), [&](){ mPendingAnimations--; }, cardAnimationName);
            mPendingAnimations++;
        }
        
        // .. The rest can be looked up
        else
        {
            for (const auto& sceneObjectComponentName: sceneObjectComponentNames)
            {
                cardComponents.push_back(dummyScene->FindSceneObject(sceneObjectComponentName));
            }
            
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(cardComponents, glm::vec3(targetX, cardComponents.front()->mPosition.y, cardComponents.front()->mPosition.z), game_constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DURATION_SECS, true, game_constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DELAY_SECS, math::QuadFunction), [&](){ mPendingAnimations--; }, cardAnimationName);
            mPendingAnimations++;
        }
    }
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult DrawCardGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------
