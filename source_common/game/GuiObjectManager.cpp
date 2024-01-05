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

///------------------------------------------------------------------------------------------------

static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId PARTICLE_EMITTER_SCENE_OBJECT_NAME = strutils::StringId("particle_emitter");
static const strutils::StringId PARTICLE_EMITTER_DEFINITION_COIN_SMALL = strutils::StringId("coin_gain_small");
static const strutils::StringId PARTICLE_EMITTER_DEFINITION_COIN_LARGE = strutils::StringId("coin_gain_large");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "basic_custom_color.vs";
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
static const glm::vec3 COIN_INIT_POSITION_OFFSET = { 0.0f, 0.0f, 0.7f };
static const glm::vec3 COIN_TARGET_POSITION_OFFSET = { -0.02f, -0.01f, -22.5f };
static const glm::vec3 COIN_MID_POSITION_MIN = { 0.1f, -0.2f, 1.5f };
static const glm::vec3 COIN_MID_POSITION_MAX = { 0.3f, 0.2f, 1.5f };
static const glm::vec3 BATTLE_COIN_MID_POSITION_MIN = { 0.04f, -0.02f, 1.5f };
static const glm::vec3 BATTLE_COIN_MID_POSITION_MAX = { 0.14f, 0.1f, 1.5f };

static const float COIN_RESPAWN_TICK_SECS = 0.025f;
static const float SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 33.5f;
static const float COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.4f;
static const float COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 280.0f;
static const float HEALTH_CRYSTAL_BASE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.0f;
static const float HEALTH_CRYSTAL_VALUE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 260.0f;
static const float HEALTH_CRYSTAL_CONTAINER_CUSTOM_SCALE_FACTOR = 2.0f;
static const float BATTLE_SCENE_SCALE_FACTOR = 0.5f;
static const float COIN_ANIMATION_DURATION_SECS = 0.75f;

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
    
}

///------------------------------------------------------------------------------------------------

GuiObjectManager::~GuiObjectManager()
{
    
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::Update(const float dtMillis)
{
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
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
    coinValue = 123456;
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

void GuiObjectManager::AnimateCoinsToCoinStack(const glm::vec3& originPosition, const long long coinAmount)
{
    auto forBattleScene = mScene->GetName() == game_constants::BATTLE_SCENE;
    auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
    auto animatedNodePathParticleEmitterSceneObject = particleManager.CreateParticleEmitterAtPosition(forBattleScene ? PARTICLE_EMITTER_DEFINITION_COIN_SMALL : PARTICLE_EMITTER_DEFINITION_COIN_LARGE, glm::vec3(), *mScene, PARTICLE_EMITTER_SCENE_OBJECT_NAME, [=](float dtMillis, scene::ParticleEmitterObjectData& particleEmitterData)
    {
        auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        auto targetCoinPosition = mScene->FindSceneObject(game_constants::GUI_COIN_STACK_SCENE_OBJECT_NAME)->mPosition + COIN_TARGET_POSITION_OFFSET;

        auto& particleEmitterSceneObject = *mScene->FindSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME);
        
        static float timeAccum = 0.0f;
        timeAccum += dtMillis/1000.0f;
        
        if (timeAccum > COIN_RESPAWN_TICK_SECS && static_cast<int>(particleEmitterData.mTotalParticlesSpawned) < coinAmount)
        {
            timeAccum = 0.0f;
            int particleIndex = particleManager.SpawnParticleAtFirstAvailableSlot(particleEmitterSceneObject);
            
            particleEmitterData.mParticlePositions[particleIndex] = originPosition + COIN_INIT_POSITION_OFFSET;
            
            glm::vec3 coinMidPosition = glm::vec3(
                math::RandomFloat(forBattleScene ? BATTLE_COIN_MID_POSITION_MIN.x : COIN_MID_POSITION_MIN.x, forBattleScene ? BATTLE_COIN_MID_POSITION_MAX.x : COIN_MID_POSITION_MAX.x),
                math::RandomFloat(forBattleScene ? BATTLE_COIN_MID_POSITION_MIN.y : COIN_MID_POSITION_MIN.y, forBattleScene ? BATTLE_COIN_MID_POSITION_MAX.y : COIN_MID_POSITION_MAX.y),
                math::RandomFloat(COIN_MID_POSITION_MIN.z, COIN_MID_POSITION_MAX.z));
            math::BezierCurve curve({particleEmitterData.mParticlePositions[particleIndex], coinMidPosition, targetCoinPosition});
            
            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(particleEmitterData.mParticlePositions[particleIndex], curve, COIN_ANIMATION_DURATION_SECS), [=]()
            {
                std::get<scene::ParticleEmitterObjectData>(mScene->FindSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mParticleLifetimeSecs[particleIndex] = 0.0f;
                
                // Animation only coin change
                auto& coins = ProgressionDataRepository::GetInstance().CurrencyCoins();
                coins.SetDisplayedValue(coins.GetDisplayedValue() + 1);
            }, game_constants::COIN_FLYING_ANIMATION_NAME);
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
