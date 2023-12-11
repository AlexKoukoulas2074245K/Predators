///------------------------------------------------------------------------------------------------
///  CardPlayedParticleEffectGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 23/11/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/CardPlayedParticleEffectGameAction.h>
#include <game/scenelogicmanagers/BattleSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId PARTICLE_SCENE_OBJECT_NAME = strutils::StringId("CARD_PLAYED_PARTICLE_EFFECT");

///------------------------------------------------------------------------------------------------

void CardPlayedParticleEffectGameAction::VSetNewGameState()
{
}

///------------------------------------------------------------------------------------------------

void CardPlayedParticleEffectGameAction::VInitAnimation()
{
    const auto& lastPlayedCardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers()[mBoardState->GetActivePlayerIndex()].back();
    auto scene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    assert(!lastPlayedCardSoWrapper->mCardData->mParticleEffect.isEmpty());
    
    CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition(lastPlayedCardSoWrapper->mCardData->mParticleEffect, lastPlayedCardSoWrapper->mSceneObject->mPosition, *scene, PARTICLE_SCENE_OBJECT_NAME);
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult CardPlayedParticleEffectGameAction::VUpdateAnimation(const float)
{
    auto scene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    if (scene->FindSceneObject(PARTICLE_SCENE_OBJECT_NAME))
    {
        return ActionAnimationUpdateResult::ONGOING;
    }
    else
    {
        return ActionAnimationUpdateResult::FINISHED;
    }
}

///------------------------------------------------------------------------------------------------

bool CardPlayedParticleEffectGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& CardPlayedParticleEffectGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
