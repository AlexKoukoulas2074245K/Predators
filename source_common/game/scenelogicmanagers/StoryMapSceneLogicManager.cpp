///------------------------------------------------------------------------------------------------
///  StoryMapSceneLogicManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <game/AnimatedButton.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/StoryMapSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId SETTINGS_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("settings_button");
static const strutils::StringId COIN_STACK_SCENE_OBJECT_NAME = strutils::StringId("coin_stack");
static const strutils::StringId COIN_VALUE_TEXT_SCENE_OBJECT_NAME = strutils::StringId("coin_value_text");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("background");

static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";
static const std::string COIN_STACK_TEXTURE_FILE_NAME = "coin_stack.png";

static const glm::vec2 MAP_SWIPE_X_BOUNDS = {-0.5f, 0.5f};
static const glm::vec2 MAP_SWIPE_Y_BOUNDS = {-0.5f, 0.5f};

static const glm::vec3 SETTINGS_BUTTON_POSITION = {0.145f, 0.181f, 0.1f};
static const glm::vec3 SETTINGS_BUTTON_SCALE = {0.06f, 0.06f, 0.06f};
static const glm::vec3 COIN_STACK_POSITION = {0.145f, 0.101f, 0.1f};
static const glm::vec3 COIN_STACK_SCALE = {0.08f, 0.08f, 0.08f};
static const glm::vec3 COIN_VALUE_TEXT_POSITION = {0.155f, 0.105f, 0.1f};
static const glm::vec3 COIN_VALUE_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 COIN_VALUE_TEXT_COLOR = {0.80f, 0.71f, 0.11f};


static const float SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 31.5f;
static const float COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.3f;
static const float COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 260.0f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::STORY_MAP_SCENE
};

static const std::vector<strutils::StringId> GUI_SCENE_OBJECT_NAMES =
{
    COIN_STACK_SCENE_OBJECT_NAME,
    SETTINGS_BUTTON_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& StoryMapSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

StoryMapSceneLogicManager::StoryMapSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

StoryMapSceneLogicManager::~StoryMapSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    RegisterForEvents();
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
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(scene->GetCamera().GetViewMatrix(), scene->GetCamera().GetProjMatrix());
    
    if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
    {
        bool tappedGuiSceneObject = false;
        for (const auto& guiSceneObjectName: GUI_SCENE_OBJECT_NAMES)
        {
            auto sceneObject = scene->FindSceneObject(guiSceneObjectName);
            auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*sceneObject);
            if (math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos))
            {
                tappedGuiSceneObject = true;
                break;
            }
        }
        
        if (tappedGuiSceneObject)
        {
            ResetSwipeData();
        }
        else
        {
            mHasStartedSwipe = true;
            mSwipeStartPos = glm::vec3(worldTouchPos.x, worldTouchPos.y, 0.0f);
            mSwipeCurrentPos = mSwipeStartPos;
            mSwipeDurationMillis = 0.0f;
        }
    }
    else if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
    {
        if (mHasStartedSwipe)
        {
            mSwipeDurationMillis += dtMillis;
            
            float targetDx = (worldTouchPos.x - mSwipeCurrentPos.x);
            float targetDy = (worldTouchPos.y - mSwipeCurrentPos.y);
            
            auto backgroundSceneObject = scene->FindSceneObject(BACKGROUND_SCENE_OBJECT_NAME);
            
            auto nextPosition = backgroundSceneObject->mPosition;
            
            nextPosition.x += targetDx;
            nextPosition.x = math::Max(MAP_SWIPE_X_BOUNDS.s, math::Min(MAP_SWIPE_X_BOUNDS.t, nextPosition.x));
            
            nextPosition.y += targetDy;
            nextPosition.y = math::Max(MAP_SWIPE_Y_BOUNDS.s, math::Min(MAP_SWIPE_Y_BOUNDS.t, nextPosition.y));
                
            backgroundSceneObject->mPosition = nextPosition;
            
            mSwipeCurrentPos = glm::vec3(worldTouchPos.x, worldTouchPos.y, 0.0f);
        }
    }
    else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
    {
        ResetSwipeData();
    }
    
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
    
    SetCoinValueText();
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    eventSystem.RegisterForEvent<events::PopSceneModalEvent>(this, &StoryMapSceneLogicManager::OnPopSceneModal);
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &StoryMapSceneLogicManager::OnWindowResize);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::SetCoinValueText()
{
    auto scene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::STORY_MAP_SCENE);
    auto coinValue = ProgressionDataRepository::GetInstance().GetCurrencyCoins();
    
    if (coinValue < 1000)
    {
        std::get<scene::TextSceneObjectData>(scene->FindSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue);
    }
    else if (coinValue < 1000000)
    {
        std::get<scene::TextSceneObjectData>(scene->FindSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue/1000) + "." + std::to_string((coinValue % 1000)/100) + "k";
    }
    else
    {
        std::get<scene::TextSceneObjectData>(scene->FindSceneObject(COIN_VALUE_TEXT_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = std::to_string(coinValue/1000000) + "." + std::to_string((coinValue % 1000000)/100000) + "m";
    }
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::OnPopSceneModal(const events::PopSceneModalEvent&)
{
    ResetSwipeData();
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::STORY_MAP_SCENE)->RecalculatePositionOfEdgeSnappingSceneObjects();
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::ResetSwipeData()
{
    mHasStartedSwipe = false;
    mSwipeDurationMillis = 0.0f;
    mSwipeVelocityDelta = 0.0f;
    mSwipeDelta = 0.0f;
}
    
///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::OnSettingsButtonPressed()
{
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(SETTINGS_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------
