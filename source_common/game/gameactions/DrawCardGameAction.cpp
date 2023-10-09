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
#include <game/GameConstants.h>
#include <game/gameactions/DrawCardGameAction.h>

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    activePlayerState.mPlayerHeldCards.push_back(math::ControlledRandomInt());
}

///------------------------------------------------------------------------------------------------

void DrawCardGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    
    auto dummyScene = activeSceneManager.FindScene(strutils::StringId("Dummy"));
    
    const int cardCount = static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCards.size());
    
    float cardBlockWidth = constants::IN_GAME_CARD_WIDTH * cardCount;
    float cardStartX = -cardBlockWidth/2.0f;
    
    for (int i = 0; i < cardCount; ++i)
    {
        auto cardName = (mBoardState->GetActivePlayerIndex() == 0 ? constants::TOP_PLAYER_CARD_NAME_PREFIX : constants::BOT_PLAYER_CARD_NAME_PREFIX) + std::to_string(i);
        auto cardFrameSceneObject = (i == cardCount - 1) ? dummyScene->CreateSceneObject(strutils::StringId(cardName)) : dummyScene->FindSceneObject(strutils::StringId(cardName));
        
        cardFrameSceneObject->mScale.x = cardFrameSceneObject->mScale.y = constants::IN_GAME_CARD_SCALE;
        auto targetX = cardStartX + i * constants::IN_GAME_CARD_WIDTH + constants::IN_GAME_CARD_WIDTH/2;
        
        if (cardCount > constants::IN_GAME_CARD_PUSH_THRESHOLD)
        {
            float pushX = (cardCount - constants::IN_GAME_CARD_PUSH_THRESHOLD) * constants::IN_GAME_CARD_PUSH_VALUE * (math::Abs(i - cardCount/2));
            bool oddCardCount = cardCount % 2 != 0;
            if ((oddCardCount && i != cardCount/2) || !oddCardCount)
            {
                targetX += (i < cardCount/2) ? pushX : -pushX;
            }
        }
        
        if (i == cardCount - 1)
        {
            cardFrameSceneObject->mPosition.y = mBoardState->GetActivePlayerIndex() == 0 ? constants::IN_GAME_TOP_PLAYER_HELD_CARD_Y : constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y;
            cardFrameSceneObject->mPosition.z = constants::IN_GAME_PLAYER_HELD_CARD_Z;
            cardFrameSceneObject->mRotation.z = math::PI;
            
            cardFrameSceneObject->mPosition.x = constants::IN_GAME_DRAW_CARD_INIT_X + i * constants::IN_GAME_CARD_WIDTH/2;
            
            auto midPos = cardFrameSceneObject->mPosition;
            midPos.x = math::Abs(cardFrameSceneObject->mPosition.x - targetX)/2.0f;
            midPos.y = mBoardState->GetActivePlayerIndex() == 0 ? constants::IN_GAME_DRAW_CARD_TOP_PLAYER_MID_POINT_Y : constants::IN_GAME_DRAW_CARD_BOT_PLAYER_MID_POINT_Y;
            
            math::BezierCurve curve(std::vector<glm::vec3>{cardFrameSceneObject->mPosition, midPos, glm::vec3(targetX, cardFrameSceneObject->mPosition.y, constants::IN_GAME_PLAYER_HELD_CARD_Z)});

            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(std::vector<std::shared_ptr<scene::SceneObject>>{cardFrameSceneObject}, curve, constants::IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS), [&](){ mPendingAnimations--; });
            
            cardFrameSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
            cardFrameSceneObject->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT +  (mBoardState->GetActivePlayerIndex() == 0 ? "card_back.png" : "card_frame.png"));
            cardFrameSceneObject->mMeshResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
        }
        else
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(std::vector<std::shared_ptr<scene::SceneObject>>{cardFrameSceneObject}, glm::vec3(targetX, cardFrameSceneObject->mPosition.y, cardFrameSceneObject->mPosition.z), constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DURATION_SECS, constants::IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DELAY_SECS, math::QuadFunction), [&](){ mPendingAnimations--; });
        }
    }
    
    mPendingAnimations = static_cast<int>(cardCount);
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult DrawCardGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------
