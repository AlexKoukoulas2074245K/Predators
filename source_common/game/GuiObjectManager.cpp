///------------------------------------------------------------------------------------------------
///  GuiObjectManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/AnimatedButton.h>
#include <game/AnimatedStatContainer.h>
#include <game/GuiObjectManager.h>
#include <game/GameConstants.h>
#include <game/events/EventSystem.h>
#include <game/ProgressionDataRepository.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId PARTICLE_EMITTER_SCENE_OBJECT_NAME = strutils::StringId("stat_particle_emitter");
static const strutils::StringId PARTICLE_EMITTER_DEFINITION_COIN_SMALL = strutils::StringId("coin_gain_small");
static const strutils::StringId PARTICLE_EMITTER_DEFINITION_COIN_LARGE = strutils::StringId("coin_gain_large");
static const strutils::StringId PARTICLE_EMITTER_DEFINITION_HEALTH_SMALL = strutils::StringId("health_gain_small");
static const strutils::StringId PARTICLE_EMITTER_DEFINITION_HEALTH_LARGE = strutils::StringId("health_gain_large");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "animated_stat_container_value_object.vs";
static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";
static const std::string COIN_STACK_TEXTURE_FILE_NAME = "coin_stack.png";
static const std::string HEALTH_CRYSTAL_TEXTURE_FILE_NAME = "health_icon.png";
static const std::string HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX = "health_crystal_";

static const glm::vec3 BATTLE_SCENE_SETTINGS_BUTTON_POSITION = {0.145f, 0.09f, 24.0f};
static const glm::vec3 SETTINGS_BUTTON_POSITION = {0.145f, 0.161f, 24.0f};
static const glm::vec3 SETTINGS_BUTTON_SCALE = {0.06f, 0.06f, 0.06f};
static const glm::vec3 COIN_STACK_POSITION = {0.145f, 0.101f, 24.0f};
static const glm::vec3 BATTLE_SCENE_COIN_STACK_POSITION = {0.145f, 0.06f, 24.0f};
static const glm::vec3 COIN_STACK_SCALE = {0.08f, 0.08f, 0.08f};
static const glm::vec3 COIN_VALUE_TEXT_POSITION = {0.155f, 0.105f, 24.0f};
static const glm::vec3 BATTLE_SCENE_COIN_VALUE_TEXT_POSITION = {0.155f, 0.06f, 24.0f};
static const glm::vec3 COIN_VALUE_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 COIN_VALUE_TEXT_COLOR = {0.80f, 0.71f, 0.11f};
static const glm::vec3 BATTLE_SCENE_HEALTH_CRYSTAL_POSITION = {0.145f, 0.02f, 24.0f};
static const glm::vec3 HEALTH_CRYSTAL_POSITION = {0.145f, 0.04f, 24.0f};
static const glm::vec3 STAT_PARTICLE_INIT_POSITION_OFFSET = { 0.0f, 0.0f, 0.7f };
static const glm::vec3 STAT_PARTICLE_TARGET_POSITION_OFFSET = { -0.02f, -0.01f, -0.001f };
static const glm::vec3 STAT_PARTICLE_MID_POSITION_MIN = { 0.1f, -0.2f, 0.01f };
static const glm::vec3 STAT_PARTICLE_MID_POSITION_MAX = { 0.3f, 0.2f, 0.02f };
static const glm::vec3 BATTLE_STAT_PARTICLE_MID_POSITION_MIN = { 0.04f, -0.02f, 0.01f };
static const glm::vec3 BATTLE_STAT_PARTICLE_MID_POSITION_MAX = { 0.14f, 0.1f, 0.02f };

static const float COIN_PARTICLE_RESPAWN_TICK_SECS = 0.025f;
static const float HEALTH_PARTICLE_RESPAWN_TICK_SECS = 0.25f;
static const float SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 33.5f;
static const float COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.4f;
static const float COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 280.0f;
static const float HEALTH_CRYSTAL_BASE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.0f;
static const float HEALTH_CRYSTAL_VALUE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 260.0f;
static const float HEALTH_CRYSTAL_CONTAINER_CUSTOM_SCALE_FACTOR = 2.0f;
static const float BATTLE_SCENE_SCALE_FACTOR = 0.5f;
static const float STAT_PARTICLE_ANIMATION_DURATION_SECS = 0.75f;

///------------------------------------------------------------------------------------------------

GuiObjectManager::GuiObjectManager(std::shared_ptr<scene::Scene> scene)
    : mScene(scene)
{
    // Sync any desynced values with delayed displays.
    // Might not be the best place to do this.
    ProgressionDataRepository::GetInstance().CurrencyCoins().SetDisplayedValue(ProgressionDataRepository::GetInstance().CurrencyCoins().GetValue());
    ProgressionDataRepository::GetInstance().StoryCurrentHealth().SetDisplayedValue(ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue());
    
    auto forBattleScene = scene->GetName() == game_constants::BATTLE_SCENE;
    auto extraScaleFactor = forBattleScene ? BATTLE_SCENE_SCALE_FACTOR : 1.0f;
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        forBattleScene ? BATTLE_SCENE_SETTINGS_BUTTON_POSITION : SETTINGS_BUTTON_POSITION,
        extraScaleFactor * SETTINGS_BUTTON_SCALE,
        SETTINGS_ICON_TEXTURE_FILE_NAME,
        game_constants::GUI_SETTINGS_BUTTON_SCENE_OBJECT_NAME,
        [=](){ OnSettingsButtonPressed(); },
        *scene,
        scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE,
        SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR / extraScaleFactor
    ));
    
    auto coinStackSceneObject = scene->CreateSceneObject(game_constants::GUI_COIN_STACK_SCENE_OBJECT_NAME);
    coinStackSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    coinStackSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + COIN_STACK_TEXTURE_FILE_NAME);
    coinStackSceneObject->mPosition = forBattleScene ? BATTLE_SCENE_COIN_STACK_POSITION : COIN_STACK_POSITION;
    coinStackSceneObject->mScale = extraScaleFactor * COIN_STACK_SCALE;
    coinStackSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    coinStackSceneObject->mSnapToEdgeScaleOffsetFactor = COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    
    scene::TextSceneObjectData coinValueText;
    coinValueText.mFontName = game_constants::DEFAULT_FONT_NAME;
    coinValueText.mText = std::to_string(ProgressionDataRepository::GetInstance().CurrencyCoins().GetValue());
    auto coinValueTextSceneObject = scene->CreateSceneObject(game_constants::GUI_COIN_VALUE_TEXT_SCENE_OBJECT_NAME);
    coinValueTextSceneObject->mSceneObjectTypeData = std::move(coinValueText);
    coinValueTextSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + COIN_VALUE_TEXT_SHADER_FILE_NAME);
    coinValueTextSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = COIN_VALUE_TEXT_COLOR;
    coinValueTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    coinValueTextSceneObject->mPosition = forBattleScene ? BATTLE_SCENE_COIN_VALUE_TEXT_POSITION : COIN_VALUE_TEXT_POSITION;
    coinValueTextSceneObject->mScale = extraScaleFactor * COIN_VALUE_TEXT_SCALE;
    coinValueTextSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    coinValueTextSceneObject->mSnapToEdgeScaleOffsetFactor = COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    
    mHealthStatContainer = std::make_unique<AnimatedStatContainer>(forBattleScene ? BATTLE_SCENE_HEALTH_CRYSTAL_POSITION : HEALTH_CRYSTAL_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX, ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetDisplayedValue(), forBattleScene, *scene, scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE, extraScaleFactor * HEALTH_CRYSTAL_CONTAINER_CUSTOM_SCALE_FACTOR);
    mHealthStatContainer->ForceSetDisplayedValue(ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue());
    
    mHealthStatContainer->GetSceneObjects()[0]->mSnapToEdgeScaleOffsetFactor = HEALTH_CRYSTAL_BASE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    mHealthStatContainer->GetSceneObjects()[1]->mSnapToEdgeScaleOffsetFactor = HEALTH_CRYSTAL_VALUE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    
    Update(0.0f);
    
    events::EventSystem::GetInstance().RegisterForEvent<events::CoinRewardEvent>(this, &GuiObjectManager::OnCoinReward);
    events::EventSystem::GetInstance().RegisterForEvent<events::HealthRefillRewardEvent>(this, &GuiObjectManager::OnHealthRefillReward);
}

///------------------------------------------------------------------------------------------------

GuiObjectManager::~GuiObjectManager()
{
    
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::Update(const float dtMillis, const bool allowButtonInput /* = true */)
{
    if (allowButtonInput)
    {
        for (auto& animatedButton: mAnimatedButtons)
        {
            animatedButton->Update(dtMillis);
        }
    }
    
    auto& currentHealth = ProgressionDataRepository::GetInstance().StoryCurrentHealth();
    currentHealth.SetValue(math::Max(0, currentHealth.GetValue()));
    currentHealth.SetDisplayedValue(math::Max(0, currentHealth.GetDisplayedValue()));
    
    mHealthStatContainer->Update(dtMillis);
    SetCoinValueText();
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::SetCoinValueText()
{
    auto coinValue = ProgressionDataRepository::GetInstance().CurrencyCoins().GetDisplayedValue();

    if (coinValue < 1000)
    {
        std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(game_constants::GUI_COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue);
    }
    else if (coinValue < 1000000)
    {
        std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(game_constants::GUI_COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue/1000) + "." + std::to_string((coinValue % 1000)/100) + "k";
    }
    else
    {
        std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(game_constants::GUI_COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue/1000000) + "." + std::to_string((coinValue % 1000000)/100000) + "m";
    }
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::OnWindowResize()
{
    mHealthStatContainer->Update(0.0f);
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::ForceSetStoryHealthValue(const int storyHealthValue)
{
    mHealthStatContainer->ForceSetDisplayedValue(storyHealthValue);
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::AnimateStatParticlesToGui(const glm::vec3& originPosition, const StatParticleType statParticleType, const long long statAmount)
{
    auto forBattleScene = mScene->GetName() == game_constants::BATTLE_SCENE;
    auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
    mScene->RemoveSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME);
    
    auto particleDefinition = strutils::StringId();
    switch (statParticleType)
    {
        case StatParticleType::COINS: particleDefinition = forBattleScene ? PARTICLE_EMITTER_DEFINITION_COIN_SMALL : PARTICLE_EMITTER_DEFINITION_COIN_LARGE; break;
        case StatParticleType::HEALTH: particleDefinition = forBattleScene ? PARTICLE_EMITTER_DEFINITION_HEALTH_SMALL : PARTICLE_EMITTER_DEFINITION_HEALTH_LARGE; break;
    }
    
    auto animatedNodePathParticleEmitterSceneObject = particleManager.CreateParticleEmitterAtPosition(particleDefinition, glm::vec3(), *mScene, PARTICLE_EMITTER_SCENE_OBJECT_NAME, [=](float dtMillis, scene::ParticleEmitterObjectData& particleEmitterData)
    {
        auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        auto targetRespawnSecs = 0.0f;
        auto targetPosition = STAT_PARTICLE_TARGET_POSITION_OFFSET;
        switch (statParticleType)
        {
            case StatParticleType::COINS:
            {
                targetPosition += mScene->FindSceneObject(game_constants::GUI_COIN_STACK_SCENE_OBJECT_NAME)->mPosition;
                targetRespawnSecs = COIN_PARTICLE_RESPAWN_TICK_SECS;
            } break;
                
            case StatParticleType::HEALTH:
            {
                targetPosition += mHealthStatContainer->GetSceneObjects().front()->mPosition;
                targetRespawnSecs = HEALTH_PARTICLE_RESPAWN_TICK_SECS;
            } break;
        }
        
        auto& particleEmitterSceneObject = *mScene->FindSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME);
        
        static float timeAccum = 0.0f;
        timeAccum += dtMillis/1000.0f;

        if (timeAccum > targetRespawnSecs && static_cast<int>(particleEmitterData.mTotalParticlesSpawned) < statAmount)
        {
            timeAccum = 0.0f;
            int particleIndex = particleManager.SpawnParticleAtFirstAvailableSlot(particleEmitterSceneObject);
            
            particleEmitterData.mParticlePositions[particleIndex] = originPosition + STAT_PARTICLE_INIT_POSITION_OFFSET;
            
            glm::vec3 midPosition = glm::vec3(
                math::RandomFloat(forBattleScene ? BATTLE_STAT_PARTICLE_MID_POSITION_MIN.x : STAT_PARTICLE_MID_POSITION_MIN.x, forBattleScene ? BATTLE_STAT_PARTICLE_MID_POSITION_MAX.x : STAT_PARTICLE_MID_POSITION_MAX.x),
                math::RandomFloat(forBattleScene ? BATTLE_STAT_PARTICLE_MID_POSITION_MIN.y : STAT_PARTICLE_MID_POSITION_MIN.y, forBattleScene ? BATTLE_STAT_PARTICLE_MID_POSITION_MAX.y : STAT_PARTICLE_MID_POSITION_MAX.y),
                (particleEmitterData.mParticlePositions[particleIndex].z + targetPosition.z)/2.0f + math::RandomFloat(STAT_PARTICLE_MID_POSITION_MIN.z, STAT_PARTICLE_MID_POSITION_MAX.z));
            math::BezierCurve curve({particleEmitterData.mParticlePositions[particleIndex], midPosition, targetPosition});
            
            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(particleEmitterData.mParticlePositions[particleIndex], curve, STAT_PARTICLE_ANIMATION_DURATION_SECS), [=]()
            {
                std::get<scene::ParticleEmitterObjectData>(mScene->FindSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mParticleLifetimeSecs[particleIndex] = 0.0f;
                
                switch (statParticleType)
                {
                    case StatParticleType::COINS:
                    {
                        // Animation only coin change
                        auto& coins = ProgressionDataRepository::GetInstance().CurrencyCoins();
                        coins.SetDisplayedValue(coins.GetDisplayedValue() + 1);
                    } break;
                        
                    case StatParticleType::HEALTH:
                    {
                        // Animation only health change
                        auto& health = ProgressionDataRepository::GetInstance().StoryCurrentHealth();
                        health.SetDisplayedValue(health.GetDisplayedValue() + 1);
                    } break;
                }
            }, game_constants::STAT_PARTICLE_FLYING_ANIMATION_NAME);
        }
    });
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::OnSettingsButtonPressed()
{
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mScene->GetUpdateTimeSpeedFactor(), 0.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(SETTINGS_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::OnCoinReward(const events::CoinRewardEvent& event)
{
    ProgressionDataRepository::GetInstance().CurrencyCoins().SetValue(ProgressionDataRepository::GetInstance().CurrencyCoins().GetValue() + event.mCoinAmount);
    AnimateStatParticlesToGui(event.mAnimationOriginPosition, StatParticleType::COINS, event.mCoinAmount);
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::OnHealthRefillReward(const events::HealthRefillRewardEvent& event)
{
    for (auto sceneObject: mHealthStatContainer->GetSceneObjects())
    {
        sceneObject->mInvisible = false;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, 0.5f), [=](){});
    }
    
    ProgressionDataRepository::GetInstance().StoryCurrentHealth().SetValue(ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue() + event.mHealthAmount);
    AnimateStatParticlesToGui(event.mAnimationOriginPosition, StatParticleType::HEALTH, event.mHealthAmount);
}

///------------------------------------------------------------------------------------------------
