///------------------------------------------------------------------------------------------------
///  GameOverGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/scenelogicmanagers/BattleSceneLogicManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

const strutils::StringId VICTORIOUS_TEXT_SCENE_OBJECT_NAME = strutils::StringId("victorious_player_text");
const std::string GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM = "victoriousPlayerIndex";

static const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_dissolve.vs";
static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";

static const strutils::StringId CARD_SELECTION_REWARD_SCENE_NAME = strutils::StringId("card_selection_reward_scene");
static const strutils::StringId WHEEL_OF_FORTUNE_SCENE_NAME = strutils::StringId("wheel_of_fortune_scene");
static const strutils::StringId DEFEAT_SCENE_NAME = strutils::StringId("defeat_scene");
static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");
static const strutils::StringId HERO_CARD_DESTRUCTION_PARTICLE_NAME = strutils::StringId("hero_card_destruction");

static const float CARD_CAMERA_SHAKE_DURATION = 0.25f;
static const float CARD_CAMERA_SHAKE_STRENGTH = 0.005f;
static const float CARD_DISSOLVE_SPEED = 0.0006f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;
static const float EXPLOSION_DELAY_SECS = 0.8f;

static const int MAX_EXPLOSIONS = 5;

static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE = {10.0f, 18.0f};

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    GameOverGameAction::VICTORIOUS_PLAYER_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void GameOverGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(VICTORIOUS_PLAYER_INDEX_PARAM) != 0);
    logging::Log(logging::LogType::INFO, "%s", ("Player " + mExtraActionParams.at(VICTORIOUS_PLAYER_INDEX_PARAM) + " won!").c_str());
}

///------------------------------------------------------------------------------------------------

void GameOverGameAction::VInitAnimation()
{
    auto& scene = *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE);
    
    if (!ProgressionDataRepository::GetInstance().GetNextStoryOpponentName().empty() && !ProgressionDataRepository::GetInstance().GetQuickPlayData())
    {
        if (std::stoi(mExtraActionParams.at(VICTORIOUS_PLAYER_INDEX_PARAM)) == game_constants::LOCAL_PLAYER_INDEX)
        {
            mExplosionDelaySecs = EXPLOSION_DELAY_SECS;
            mAnimationState = AnimationState::EXPLOSIONS;
            mExplosionCounter = 0;
        }
        else
        {
            mAnimationState = AnimationState::DEFEAT;
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(scene.GetUpdateTimeSpeedFactor(), 0.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(DEFEAT_SCENE_NAME, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
        }
    }
    else
    {
        auto victorTextSo = scene.CreateSceneObject(VICTORIOUS_TEXT_SCENE_OBJECT_NAME);
        
        scene::TextSceneObjectData damageTextData;
        damageTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        damageTextData.mText = "Player " + mExtraActionParams.at(VICTORIOUS_PLAYER_INDEX_PARAM) + " won!";
        
        victorTextSo->mSceneObjectTypeData = std::move(damageTextData);
        victorTextSo->mScale = glm::vec3(game_constants::IN_GAME_CARD_PROPERTY_SCALE * 3);
        victorTextSo->mPosition = glm::vec3(-0.1f, 0.0f, 5.0f);
    }
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult GameOverGameAction::VUpdateAnimation(const float dtMillis)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    
    if (!ProgressionDataRepository::GetInstance().GetNextStoryOpponentName().empty())
    {
        switch (mAnimationState)
        {
            case AnimationState::EXPLOSIONS:
            {
                auto cardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers()[game_constants::REMOTE_PLAYER_INDEX][0];
                
                mExplosionDelaySecs -= dtMillis/1000.0f;
                if (mExplosionDelaySecs <= 0.0f)
                {
                    mExplosionDelaySecs = EXPLOSION_DELAY_SECS - (mExplosionCounter * 0.1f);
                    
                    if (mExplosionCounter++ <= MAX_EXPLOSIONS)
                    {
                        auto particleEmitterPosition = cardSoWrapper->mSceneObject->mPosition;
                        particleEmitterPosition.x += math::RandomFloat(-0.01f, 0.01f);
                        particleEmitterPosition.y += math::RandomFloat(-0.01f, 0.01f);
                        particleEmitterPosition.z += math::RandomFloat(1.0f, 3.0f);
                        
                        CoreSystemsEngine::GetInstance().GetParticleManager().CreateParticleEmitterAtPosition
                        (
                             HERO_CARD_DESTRUCTION_PARTICLE_NAME,
                             particleEmitterPosition,
                             *CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)
                        );
                        
                        CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::BATTLE_SCENE)->GetCamera().Shake(CARD_CAMERA_SHAKE_DURATION, CARD_CAMERA_SHAKE_STRENGTH);
                    }
                    else
                    {
                        cardSoWrapper->mSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_DISSOLVE_SHADER_FILE_NAME);
                        cardSoWrapper->mSceneObject->mEffectTextureResourceIds[1] = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
                        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
                        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_X_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.x;
                        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_Y_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.y;
                        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_MAGNITUDE_UNIFORM_NAME] = math::RandomFloat(CARD_DISSOLVE_EFFECT_MAG_RANGE.x, CARD_DISSOLVE_EFFECT_MAG_RANGE.y);
                        mAnimationState = AnimationState::DISSOLVE;
                        
                        events::EventSystem::GetInstance().DispatchEvent<events::StoryBattleWonEvent>();
                    }
                }
            } break;
                
            case AnimationState::DISSOLVE:
            {
                auto cardSoWrapper = mBattleSceneLogicManager->GetBoardCardSoWrappers()[game_constants::REMOTE_PLAYER_INDEX][0];
                cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = math::Min(cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] + dtMillis * CARD_DISSOLVE_SPEED, MAX_CARD_DISSOLVE_VALUE);
                
                if (cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE && !CoreSystemsEngine::GetInstance().GetAnimationManager().IsAnimationPlaying(game_constants::STAT_PARTICLE_FLYING_ANIMATION_NAME))
                {
                    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(CARD_SELECTION_REWARD_SCENE_NAME, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                    
                    if (ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeType() == StoryMap::NodeType::ELITE_ENCOUNTER)
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(WHEEL_OF_FORTUNE_SCENE_NAME, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                    }
                    
                    return ActionAnimationUpdateResult::FINISHED;
                }
            } break;
                
            default: break;
        }
    }
    return mAnimationState == AnimationState::FINISHED ? ActionAnimationUpdateResult::FINISHED : ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool GameOverGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& GameOverGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}

///------------------------------------------------------------------------------------------------
