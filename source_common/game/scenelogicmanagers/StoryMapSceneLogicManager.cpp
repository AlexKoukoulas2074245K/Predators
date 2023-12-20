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
#include <game/StoryNodeMap.h>
#include <thread>

///------------------------------------------------------------------------------------------------

static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId SETTINGS_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("settings_button");
static const strutils::StringId COIN_STACK_SCENE_OBJECT_NAME = strutils::StringId("coin_stack");
static const strutils::StringId COIN_VALUE_TEXT_SCENE_OBJECT_NAME = strutils::StringId("coin_value_text");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("background");

static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";
static const std::string COIN_STACK_TEXTURE_FILE_NAME = "coin_stack.png";

static const glm::ivec2 STORY_NODE_MAP_DIMENSIONS = {10, 5};
static const glm::vec2 MAP_SWIPE_X_BOUNDS = {-0.5f, 0.5f};
static const glm::vec2 MAP_SWIPE_Y_BOUNDS = {-0.5f, 0.5f};

static const glm::vec3 SETTINGS_BUTTON_POSITION = {0.145f, 0.181f, 10.0f};
static const glm::vec3 SETTINGS_BUTTON_SCALE = {0.06f, 0.06f, 0.06f};
static const glm::vec3 COIN_STACK_POSITION = {0.145f, 0.101f, 10.0f};
static const glm::vec3 COIN_STACK_SCALE = {0.08f, 0.08f, 0.08f};
static const glm::vec3 COIN_VALUE_TEXT_POSITION = {0.155f, 0.105f, 10.0f};
static const glm::vec3 COIN_VALUE_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 COIN_VALUE_TEXT_COLOR = {0.80f, 0.71f, 0.11f};

static const float SETTINGS_BUTTON_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 31.5f;
static const float COIN_STACK_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 1.3f;
static const float COIN_VALUE_TEXT_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR = 260.0f;
static const float DISTANCE_TO_TARGET_NODE_THRESHOLD = 0.1f;
static const float CAMERA_NOT_MOVED_THRESHOLD = 0.0001f;
static const float CAMERA_MOVING_TO_NODE_SPEED = 0.0005f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::STORY_MAP_SCENE
};

static const std::vector<strutils::StringId> GUI_SCENE_OBJECT_NAMES =
{
    COIN_STACK_SCENE_OBJECT_NAME,
    COIN_VALUE_TEXT_SCENE_OBJECT_NAME,
    SETTINGS_BUTTON_SCENE_OBJECT_NAME
};

extern int mapGenerationAttempts;

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
    std::thread mapGenerationThread = std::thread([=]
    {
        const auto& currentMapCoord = ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord();
        mStoryNodeMap = std::make_unique<StoryNodeMap>(scene, math::RandomInt(), STORY_NODE_MAP_DIMENSIONS, MapCoord(currentMapCoord.x, currentMapCoord.y), true);
        mStoryNodeMap->GenerateMapNodes();
    });
    mapGenerationThread.detach();
    
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
    
    mSwipeCamera = scene->GetCamera();
    mScene = scene;
    
    ResetSwipeData();
    mMapUpdateState = MapUpdateState::NAVIGATING;
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    if (!mStoryNodeMap->HasCreatedSceneObjects())
    {
        logging::Log(logging::LogType::INFO, "Finished Map Generation after %d attempts", mapGenerationAttempts);
        mStoryNodeMap->CreateMapSceneObjects();
    }
    
    SetCoinValueText();
    
    switch (mMapUpdateState)
    {
        case MapUpdateState::NAVIGATING:
        {
            const auto& currentCoord = MapCoord(ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x, ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord().y);
            const auto& currentMapNode = mStoryNodeMap->GetMapData().at(currentCoord);
                
            const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
            auto touchPos = inputStateManager.VGetPointingPosInWorldSpace(mSwipeCamera.GetViewMatrix(), mSwipeCamera.GetProjMatrix());
            auto worldTouchPos = glm::vec3(touchPos.x, touchPos.y, 0.0f);
            
            if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
            {
                bool tappedGuiSceneObject = false;
                for (const auto& guiSceneObjectName: GUI_SCENE_OBJECT_NAMES)
                {
                    auto sceneObject = scene->FindSceneObject(guiSceneObjectName);
                    auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*sceneObject);
                    if (math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, touchPos))
                    {
                        tappedGuiSceneObject = true;
                        break;
                    }
                }
                    
                bool tappedNodeObject = false;
                for (const auto& nodeMapData: mStoryNodeMap->GetMapData())
                {
                    // Will only navigate to active nodes
                    if (nodeMapData.first != currentCoord && !currentMapNode.mNodeLinks.count(nodeMapData.first))
                    {
                        continue;
                    }
                    
                    auto sceneObject = scene->FindSceneObject(strutils::StringId(nodeMapData.first.ToString()));
                    auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*sceneObject);
                    if (math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, touchPos))
                    {
                        mMapUpdateState = MapUpdateState::MOVING_TO_NODE;
                        mCameraTargetPos = sceneObject->mPosition;
                        mCameraTargetPos.z = mScene->GetCamera().GetPosition().z;
                        tappedNodeObject = true;
                        break;
                    }
                }
                
                if (tappedGuiSceneObject || tappedNodeObject)
                {
                    ResetSwipeData();
                }
                else
                {
                    mSwipeCurrentPos = worldTouchPos;
                    mHasStartedSwipe = true;
                }
            }
            else if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
            {
                if (mHasStartedSwipe)
                {
                    MoveWorldBy(mSwipeCurrentPos - worldTouchPos);
                    mSwipeCurrentPos = worldTouchPos;
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
        } break;
            
        case MapUpdateState::MOVING_TO_NODE:
        {
            auto initPosition = mScene->GetCamera().GetPosition();
            auto normalizedDirectionToTarget = glm::normalize(mCameraTargetPos - initPosition);
            
            MoveWorldBy(normalizedDirectionToTarget * dtMillis * CAMERA_MOVING_TO_NODE_SPEED);
            
            if (glm::distance(mCameraTargetPos, mScene->GetCamera().GetPosition()) < DISTANCE_TO_TARGET_NODE_THRESHOLD ||
                glm::distance(initPosition, mScene->GetCamera().GetPosition()) < CAMERA_NOT_MOVED_THRESHOLD)
            {
                mMapUpdateState = MapUpdateState::NAVIGATING;
            }
        } break;
    }
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
    mStoryNodeMap->DestroyParticleEmitters();
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
    mSwipeCamera.SetPosition(mScene->GetCamera().GetPosition());
    mHasStartedSwipe = false;
}
    
///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::OnSettingsButtonPressed()
{
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(SETTINGS_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::MoveWorldBy(const glm::vec3& delta)
{
    const auto cameraInitialPosition = mScene->GetCamera().GetPosition();
    auto cameraTargetPosition = cameraInitialPosition;
    
    cameraTargetPosition.x += delta.x;
    cameraTargetPosition.x = math::Max(MAP_SWIPE_X_BOUNDS.s, math::Min(MAP_SWIPE_X_BOUNDS.t, cameraTargetPosition.x));
    
    cameraTargetPosition.y += delta.y;
    cameraTargetPosition.y = math::Max(MAP_SWIPE_Y_BOUNDS.s, math::Min(MAP_SWIPE_Y_BOUNDS.t, cameraTargetPosition.y));

    mScene->GetCamera().SetPosition(cameraTargetPosition);
    
    MoveGUIBy(cameraTargetPosition - cameraInitialPosition);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::MoveGUIBy(const glm::vec3& delta)
{
    for (auto& sceneObject: mScene->GetSceneObjects())
    {
        if (std::find(GUI_SCENE_OBJECT_NAMES.cbegin(), GUI_SCENE_OBJECT_NAMES.cend(), sceneObject->mName) != GUI_SCENE_OBJECT_NAMES.cend())
        {
            sceneObject->mPosition += delta;
        }
    }
}
