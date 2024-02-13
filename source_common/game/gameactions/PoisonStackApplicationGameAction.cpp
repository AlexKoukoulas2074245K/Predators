///------------------------------------------------------------------------------------------------
///  PoisonStackApplicationGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/11/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/scenelogicmanagers/BattleSceneLogicManager.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PoisonStackApplicationGameAction.h>
#include <game/gameactions/GameOverGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId GAME_OVER_GAME_ACTION_NAME = strutils::StringId("GameOverGameAction");

static const strutils::StringId POISON_GAS_PARTICLE_NAME = strutils::StringId("poison_smoke");
static const float DURATION_SECS_PER_STACK = 0.2f;
static const float POISON_SMOKE_Z_OFFSET = -0.09f;

///------------------------------------------------------------------------------------------------

void PoisonStackApplicationGameAction::VSetNewGameState()
{
    mPendingDurationSecs = 0.0f;
    
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    if (activePlayerState.mPlayerPoisonStack == 0)
    {
        return;
    }
    
    mPendingDurationSecs = activePlayerState.mPlayerPoisonStack * DURATION_SECS_PER_STACK;
    
    int damage = activePlayerState.mPlayerPoisonStack;
    mAmountOfArmorDamaged = 0;
    mAmountOfHealthDamaged = 0;
    
    if (damage > 0)
    {
        if (activePlayerState.mPlayerCurrentArmor > 0)
        {
            int startingArmorValue = activePlayerState.mPlayerCurrentArmor;
            activePlayerState.mPlayerCurrentArmor = math::Max(0, activePlayerState.mPlayerCurrentArmor - damage);
            damage = math::Max(0, damage - startingArmorValue);
            mAmountOfArmorDamaged = math::Min(startingArmorValue, activePlayerState.mPlayerPoisonStack);
        }
        
        if (damage > 0 && activePlayerState.mPlayerCurrentArmor <= 0)
        {
            activePlayerState.mPlayerHealth -= damage;
            mAmountOfHealthDamaged = damage;
        }
    }
    
    activePlayerState.mPlayerPoisonStack = 0;
    
    events::EventSystem::GetInstance().DispatchEvent<events::PoisonStackChangeChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, 0);
    
    if (activePlayerState.mPlayerHealth <= 0)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::HealthChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
        mPendingDurationSecs = 0.0f;
        activePlayerState.mPlayerHealth = 0;
        mGameActionEngine->AddGameAction(GAME_OVER_GAME_ACTION_NAME,
        {
            { GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::LOCAL_PLAYER_INDEX : game_constants::REMOTE_PLAYER_INDEX) }
        });
    }
}

///------------------------------------------------------------------------------------------------

void PoisonStackApplicationGameAction::VInitAnimation()
{
    if (mPendingDurationSecs <= 0)
    {
        return;
    }
    
    if (mAmountOfArmorDamaged > 0)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::ArmorChangeChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX, mBoardState->GetActivePlayerState().mPlayerCurrentArmor);
        
        if (mAmountOfHealthDamaged > 0)
        {
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TimeDelayAnimation>(game_constants::PER_ARMOR_DROPPED_DELAY_ANIMATION_DURATION_SECS * mAmountOfArmorDamaged), [&]()
            {
                events::EventSystem::GetInstance().DispatchEvent<events::HealthChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
            });
        }
    }
    else
    {
        events::EventSystem::GetInstance().DispatchEvent<events::HealthChangeAnimationTriggerEvent>(mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX);
    }
    
    auto targetPosition =
        mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ?
        game_constants::HEALTH_CRYSTAL_TOP_POSITION :
        game_constants::HEALTH_CRYSTAL_BOT_POSITION;
    targetPosition.z += POISON_SMOKE_Z_OFFSET;
    
    CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
    (
        POISON_GAS_PARTICLE_NAME,
        targetPosition,
        *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)
    );
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult PoisonStackApplicationGameAction::VUpdateAnimation(const float dtMillis)
{
    mPendingDurationSecs -= dtMillis/1000.0f;
    return mPendingDurationSecs <= 0.0f ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool PoisonStackApplicationGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& PoisonStackApplicationGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
