///------------------------------------------------------------------------------------------------
///  PostNextPlayerGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/11/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/GameSessionManager.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PostNextPlayerGameAction.h>


///------------------------------------------------------------------------------------------------

static const float TURN_POINTER_ANIMATION_DURATION_SECS = 0.66f;
static const float DORMANT_CARDS_REEMERGE_ANIMATION_DURATION_SECS = 0.5f;

///------------------------------------------------------------------------------------------------

void PostNextPlayerGameAction::VSetNewGameState()
{
    std::vector<int> remainingBoardCards;
    std::vector<CardStatOverrides> remainingBoardCardStatOverrides;
    
    auto& boardCards = mBoardState->GetInactivePlayerState().mPlayerBoardCards;
    auto& boardCardIndicesToDestroy = mBoardState->GetInactivePlayerState().mBoardCardIndicesToDestroy;
    
    for (int i = static_cast<int>(boardCards.size()) - 1; i >= 0; --i)
    {
        if (boardCardIndicesToDestroy.count(i) == 0)
        {
            remainingBoardCards.insert(remainingBoardCards.begin(), boardCards[i]);
            
            if (static_cast<int>(mBoardState->GetInactivePlayerState().mPlayerBoardCardStatOverrides.size()) > i)
            {
                remainingBoardCardStatOverrides.insert(remainingBoardCardStatOverrides.begin(), mBoardState->GetInactivePlayerState().mPlayerBoardCardStatOverrides[i]);
            }
        }
        else
        {
            std::vector<std::string> idx = { std::to_string(i) };
            events::EventSystem::GetInstance().DispatchEvent<events::EndOfTurnCardDestructionEvent>(idx, true, mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX);
        }
    }
    
    auto& heldCards = mBoardState->GetInactivePlayerState().mPlayerHeldCards;
    
    for (int i = static_cast<int>(heldCards.size()) - 1; i >= 0; --i)
    {
        std::vector<std::string> idx = { std::to_string(i) };
        events::EventSystem::GetInstance().DispatchEvent<events::EndOfTurnCardDestructionEvent>(idx, false, mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX);
    }
    
    mBoardState->GetInactivePlayerState().mPlayerBoardCards = remainingBoardCards;
    mBoardState->GetInactivePlayerState().mPlayerBoardCardStatOverrides = remainingBoardCardStatOverrides;
    mBoardState->GetInactivePlayerState().mPlayerHeldCards.clear();
    mBoardState->GetInactivePlayerState().mPlayerHeldCardStatOverrides.clear();
    mBoardState->GetInactivePlayerState().mBoardModifiers.mGlobalCardStatModifiers.clear();
    mBoardState->GetInactivePlayerState().mBoardModifiers.mBoardModifierMask = effects::board_modifier_masks::NONE;
    mBoardState->GetInactivePlayerState().mBoardCardIndicesToDestroy.clear();
    mBoardState->GetInactivePlayerState().mHeldCardIndicesToDestroy.clear();
    mBoardState->GetActivePlayerState().mBoardModifiers.mBoardModifierMask &= ~(effects::board_modifier_masks::DOUBLE_POISON_ATTACKS);
    
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX, true, effects::board_modifier_masks::BOARD_SIDE_DEBUFF);
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX, true, effects::board_modifier_masks::KILL_NEXT);
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX, true, effects::board_modifier_masks::DUPLICATE_NEXT_INSECT);
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX, true, effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE);
    events::EventSystem::GetInstance().DispatchEvent<events::BoardSideCardEffectEndedEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, true, effects::board_modifier_masks::DOUBLE_POISON_ATTACKS);
    
    events::EventSystem::GetInstance().DispatchEvent<events::WeightChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
}

///------------------------------------------------------------------------------------------------

void PostNextPlayerGameAction::VInitAnimation()
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Any surviving board cards for player whose turn has ended need to be repositioned at this point
    for (auto i = 0U; i < mBoardState->GetInactivePlayerState().mPlayerBoardCards.size(); ++i)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::ForceSendCardBackToPositionEvent>(i, true, mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX);
    }
    
    // .. and any surviving board cards from previous turn of the active player need to re-emerge out again
    for (auto i = 0U; i < mBoardState->GetActivePlayerState().mPlayerBoardCards.size(); ++i)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CardBuffedDebuffedEvent>(i, true, mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
        auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(mBoardState->GetActivePlayerIndex()).at(i);
        
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::DORMANT_CARD_VALUE_UNIFORM_NAME] = 1.0f;
        animationManager.StartAnimation(std::make_unique<rendering::TweenValueAnimation>(cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::DORMANT_CARD_VALUE_UNIFORM_NAME], 0.0f, DORMANT_CARDS_REEMERGE_ANIMATION_DURATION_SECS),[=](){});
    }
    
    mPendingAnimations = 1;
    
    auto turnPointerSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    bool localPlayerActive = mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX;
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenRotationAnimation>(turnPointerSo, glm::vec3(0.0f, 0.0f, turnPointerSo->mRotation.z + (localPlayerActive ? math::PI/2 : -math::PI/2)), TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=]()
    {
        mPendingAnimations--;
        
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
        auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
        bool localPlayerActive = mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX;
        if (localPlayerActive)
        {
            auto turnPointerHighlighterSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(turnPointerHighlighterSo, 1.0f, TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), []()
            {
                events::EventSystem::GetInstance().DispatchEvent<events::LocalPlayerTurnStarted>();
            });
        }
    });
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PostNextPlayerGameAction::VUpdateAnimation(const float)
{
    return mPendingAnimations == 0 ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool PostNextPlayerGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& PostNextPlayerGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
