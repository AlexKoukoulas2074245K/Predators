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
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId SETTINGS_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("settings_button");
static const strutils::StringId COIN_STACK_SCENE_OBJECT_NAME = strutils::StringId("coin_stack");
static const strutils::StringId COIN_VALUE_TEXT_SCENE_OBJECT_NAME = strutils::StringId("coin_value_text");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";
static const std::string COIN_STACK_TEXTURE_FILE_NAME = "coin_stack.png";
static const std::string HEALTH_CRYSTAL_TEXTURE_FILE_NAME = "health_crystal.png";
static const std::string HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX = "health_crystal_";

static const glm::vec3 SETTINGS_BUTTON_POSITION = {0.145f, 0.181f, 24.0f};
static const glm::vec3 SETTINGS_BUTTON_SCALE = {0.06f, 0.06f, 0.06f};
static const glm::vec3 COIN_STACK_POSITION = {0.145f, 0.101f, 24.0f};
static const glm::vec3 COIN_STACK_SCALE = {0.08f, 0.08f, 0.08f};
static const glm::vec3 COIN_VALUE_TEXT_POSITION = {0.155f, 0.105f, 24.0f};
static const glm::vec3 COIN_VALUE_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 COIN_VALUE_TEXT_COLOR = {0.80f, 0.71f, 0.11f};
static const glm::vec3 HEALTH_CRYSTAL_POSITION = {0.145f, 0.02f, 24.0f};

static const float SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 31.5f;
static const float COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.3f;
static const float COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 260.0f;
static const float HEALTH_CRYSTAL_BASE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 0.95f;
static const float HEALTH_CRYSTAL_VALUE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 260.0f;
static const float HEALTH_CRYSTAL_CONTAINER_CUSTOM_SCALE_FACTOR = 2.0f;

///------------------------------------------------------------------------------------------------

GuiObjectManager::GuiObjectManager(std::shared_ptr<scene::Scene> scene)
    : mScene(scene)
{
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        SETTINGS_BUTTON_POSITION,
        SETTINGS_BUTTON_SCALE,
        SETTINGS_ICON_TEXTURE_FILE_NAME,
        SETTINGS_BUTTON_SCENE_OBJECT_NAME,
        [=](){ OnSettingsButtonPressed(); },
        *scene,
        scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE,
        SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR
    ));
    
    auto coinStackSceneObject = scene->CreateSceneObject(COIN_STACK_SCENE_OBJECT_NAME);
    coinStackSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    coinStackSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + COIN_STACK_TEXTURE_FILE_NAME);
    coinStackSceneObject->mPosition = COIN_STACK_POSITION;
    coinStackSceneObject->mScale = COIN_STACK_SCALE;
    coinStackSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    coinStackSceneObject->mSnapToEdgeScaleOffsetFactor = COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    
    scene::TextSceneObjectData coinValueText;
    coinValueText.mFontName = game_constants::DEFAULT_FONT_NAME;
    coinValueText.mText = std::to_string(ProgressionDataRepository::GetInstance().GetCurrencyCoins());
    auto coinValueTextSceneObject = scene->CreateSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME);
    coinValueTextSceneObject->mSceneObjectTypeData = std::move(coinValueText);
    coinValueTextSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + COIN_VALUE_TEXT_SHADER_FILE_NAME);
    coinValueTextSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = COIN_VALUE_TEXT_COLOR;
    coinValueTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    coinValueTextSceneObject->mPosition = COIN_VALUE_TEXT_POSITION;
    coinValueTextSceneObject->mScale = COIN_VALUE_TEXT_SCALE;
    coinValueTextSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    coinValueTextSceneObject->mSnapToEdgeScaleOffsetFactor = COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    
    mHealthStatContainer = std::make_unique<AnimatedStatContainer>(HEALTH_CRYSTAL_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX, ProgressionDataRepository::GetInstance().GetStoryCurrentHealth(), false, *scene, scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE, HEALTH_CRYSTAL_CONTAINER_CUSTOM_SCALE_FACTOR);
    
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
    
    mHealthStatContainer->Update(dtMillis);
    SetCoinValueText();
}

///------------------------------------------------------------------------------------------------

void GuiObjectManager::SetCoinValueText()
{
    auto coinValue = ProgressionDataRepository::GetInstance().GetCurrencyCoins();
    
    if (coinValue < 1000)
    {
        std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue);
    }
    else if (coinValue < 1000000)
    {
        std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue/1000) + "." + std::to_string((coinValue % 1000)/100) + "k";
    }
    else
    {
        std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue/1000000) + "." + std::to_string((coinValue % 1000000)/100000) + "m";
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

void GuiObjectManager::OnSettingsButtonPressed()
{
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mScene->GetUpdateTimeSpeedFactor(), 0.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(SETTINGS_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}



///------------------------------------------------------------------------------------------------
