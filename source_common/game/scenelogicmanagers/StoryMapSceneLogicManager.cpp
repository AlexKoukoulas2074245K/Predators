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
#include <engine/utils/PlatformMacros.h>
#include <game/AnimatedButton.h>
#include <game/AnimatedStatContainer.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/StoryMapSceneLogicManager.h>
#include <thread>

///------------------------------------------------------------------------------------------------

static const strutils::StringId VISIT_MAP_NODE_SCENE = strutils::StringId("visit_map_node_scene");
static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId SETTINGS_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("settings_button");
static const strutils::StringId COIN_STACK_SCENE_OBJECT_NAME = strutils::StringId("coin_stack");
static const strutils::StringId COIN_VALUE_TEXT_SCENE_OBJECT_NAME = strutils::StringId("coin_value_text");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("background");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";
static const std::string COIN_STACK_TEXTURE_FILE_NAME = "coin_stack.png";
static const std::string HEALTH_CRYSTAL_TEXTURE_FILE_NAME = "health_crystal.png";
static const std::string HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX = "health_crystal_";

static const glm::ivec2 STORY_NODE_MAP_DIMENSIONS = {10, 5};
static const glm::vec2 MAP_SWIPE_X_BOUNDS = {-0.5f, 0.5f};
static const glm::vec2 MAP_SWIPE_Y_BOUNDS = {-0.5f, 0.5f};

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
static const float DISTANCE_TO_TARGET_NODE_THRESHOLD = 0.1f;
static const float CAMERA_NOT_MOVED_THRESHOLD = 0.0001f;
static const float CAMERA_MOVING_TO_NODE_SPEED = 0.0005f;
static const float SELECTED_NODE_Z_OFFSET = 23.3f;
static const float FRESH_MAP_ANIMATION_TARGET_Y_OFFSET = -0.185f;

#if defined(NDEBUG) || defined(MOBILE_FLOW)
static const float FRESH_MAP_ANIMATION_SPEED = 0.1f;
#else
static const float FRESH_MAP_ANIMATION_SPEED = 3.0f;
#endif

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::STORY_MAP_SCENE
};

static const std::vector<strutils::StringId> GUI_SCENE_OBJECT_NAMES =
{
    COIN_STACK_SCENE_OBJECT_NAME,
    COIN_VALUE_TEXT_SCENE_OBJECT_NAME,
    SETTINGS_BUTTON_SCENE_OBJECT_NAME,
    strutils::StringId(HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX + "base"),
    strutils::StringId(HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX + "value")
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
    const auto& currentMapCoord = ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord();
    
    std::thread mapGenerationThread = std::thread([=]
    {
        mStoryMap = std::make_unique<StoryMap>(scene, STORY_NODE_MAP_DIMENSIONS, MapCoord(currentMapCoord.x, currentMapCoord.y));
        mStoryMap->GenerateMapNodes();
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
    
    mHealthStatContainer = std::make_unique<AnimatedStatContainer>(HEALTH_CRYSTAL_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX, ProgressionDataRepository::GetInstance().GetStoryCurrentHealth(), false, *scene, scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE, 2.0f);
    
    mHealthStatContainer->GetSceneObjects()[0]->mSnapToEdgeScaleOffsetFactor = HEALTH_CRYSTAL_BASE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    mHealthStatContainer->GetSceneObjects()[1]->mSnapToEdgeScaleOffsetFactor = HEALTH_CRYSTAL_VALUE_SNAP_TO_EDGE_OFFSET_SCALE_FACTOR;
    
    mSwipeCamera = scene->GetCamera();
    mScene = scene;
    
    ResetSwipeData();
    
    mMapUpdateState = MapUpdateState::NAVIGATING;
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    const auto& currentMapCoord = ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord();
    
    if (!mStoryMap->HasCreatedSceneObjects())
    {
        logging::Log(logging::LogType::INFO, "Finished Map Generation after %d attempts", mapGenerationAttempts);
        mStoryMap->CreateMapSceneObjects();
        
        if (currentMapCoord.x == game_constants::STORY_MAP_INIT_COORD.x && currentMapCoord.y == game_constants::STORY_MAP_INIT_COORD.y)
        {
            ProgressionDataRepository::GetInstance().SetStoryCurrentHealth(30);
            mHealthStatContainer->ForceSetDisplayedValue(ProgressionDataRepository::GetInstance().GetStoryCurrentHealth());
            
            mMapUpdateState = MapUpdateState::FRESH_MAP_ANIMATION;
            SetMapPositionTo(mStoryMap->GetMapData().at(MapCoord(game_constants::STORY_MAP_BOSS_COORD.x, game_constants::STORY_MAP_BOSS_COORD.y)).mPosition);
            
            mFreshMapCameraAnimationInitPosition = mScene->GetCamera().GetPosition();
            mCameraTargetPos = mStoryMap->GetMapData().at(MapCoord(game_constants::STORY_MAP_INIT_COORD.x, game_constants::STORY_MAP_INIT_COORD.y)).mPosition;
            mCameraTargetPos.y += FRESH_MAP_ANIMATION_TARGET_Y_OFFSET;
            mCameraTargetPos.z = mScene->GetCamera().GetPosition().z;
        }
        else
        {
            SetMapPositionTo(mStoryMap->GetMapData().at(MapCoord(currentMapCoord.x, currentMapCoord.y)).mPosition);
        }
    }
    
    mHealthStatContainer->Update(dtMillis);
    SetCoinValueText();
    
    switch (mMapUpdateState)
    {
        case MapUpdateState::NAVIGATING:
        {
            // Once cancelling a node visit
            if (mSelectedMapCoord)
            {
                // We need to reset the node components' z, but only after the overlay has fully gone away
                auto visitMapNodeScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(VISIT_MAP_NODE_SCENE);
                if (!visitMapNodeScene->FindSceneObject(game_constants::OVERLAY_SCENE_OBJECT_NAME))
                {
                    ResetSelectedMapNode();
                }
            }
            
            const auto& currentCoord = MapCoord(ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x, ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord().y);
            const auto& currentMapNode = mStoryMap->GetMapData().at(currentCoord);
                
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
                for (const auto& nodeMapData: mStoryMap->GetMapData())
                {
                    auto sceneObject = scene->FindSceneObject(strutils::StringId(nodeMapData.first.ToString()));
                    auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*sceneObject);
                    if (math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, touchPos))
                    {
                        // Will only navigate to active nodes
                        if (nodeMapData.first != currentCoord && !currentMapNode.mNodeLinks.count(nodeMapData.first))
                        {
                            continue;
                        }
                        
                        ResetSelectedMapNode();
                        
                        // Setup data for moving to target node
                        ProgressionDataRepository::GetInstance().SetSelectedStoryMapNodePosition(sceneObject->mPosition);
                        ProgressionDataRepository::GetInstance().SetSelectedStoryMapNodeData(&nodeMapData.second);
                        
                        mMapUpdateState = MapUpdateState::MOVING_TO_NODE;
                        mCameraTargetPos = sceneObject->mPosition;
                        mCameraTargetPos.z = mScene->GetCamera().GetPosition().z;
                        mSelectedMapCoord = std::make_unique<MapCoord>(nodeMapData.first);
                        for (auto mapNodeComponentSceneObject: scene->FindSceneObjectsWhoseNameStartsWith(mSelectedMapCoord->ToString()))
                        {
                            mapNodeComponentSceneObject->mPosition.z += SELECTED_NODE_Z_OFFSET;
                        }
                        
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
                    MoveMapBy(mSwipeCurrentPos - worldTouchPos);
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
            auto directionToTarget = mCameraTargetPos - initPosition;
            
            bool onlyMovingInOneDirection = (math::Abs(directionToTarget.x - mPreviousDirectionToTargetNode.x) <= CAMERA_NOT_MOVED_THRESHOLD || math::Abs(directionToTarget.y - mPreviousDirectionToTargetNode.y) <= CAMERA_NOT_MOVED_THRESHOLD);
            
            auto normalizedDirectionToTarget = glm::normalize(directionToTarget);
            auto targetVelocity = normalizedDirectionToTarget * dtMillis;
            
            targetVelocity *= onlyMovingInOneDirection ? 2 * CAMERA_MOVING_TO_NODE_SPEED : CAMERA_MOVING_TO_NODE_SPEED;
            
            MoveMapBy(targetVelocity);
            
            mPreviousDirectionToTargetNode = directionToTarget;
            auto currentDistanceToNode = glm::distance(mCameraTargetPos, mScene->GetCamera().GetPosition());
            
            if (currentDistanceToNode < DISTANCE_TO_TARGET_NODE_THRESHOLD ||
                glm::distance(initPosition, mScene->GetCamera().GetPosition()) < CAMERA_NOT_MOVED_THRESHOLD)
            {
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mScene->GetUpdateTimeSpeedFactor(), 0.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
                events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(VISIT_MAP_NODE_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                
                mMapUpdateState = MapUpdateState::NAVIGATING;
            }
        } break;
            
        case MapUpdateState::FRESH_MAP_ANIMATION:
        {
            auto initPosition = mScene->GetCamera().GetPosition();
            auto directionToTarget = mCameraTargetPos - initPosition;
            
            bool onlyMovingInOneDirection = (math::Abs(directionToTarget.x - mPreviousDirectionToTargetNode.x) <= CAMERA_NOT_MOVED_THRESHOLD || math::Abs(directionToTarget.y - mPreviousDirectionToTargetNode.y) <= CAMERA_NOT_MOVED_THRESHOLD);
            
            auto normalizedDirectionToTarget = glm::normalize(directionToTarget);
            auto targetVelocity = normalizedDirectionToTarget * dtMillis;
            
            targetVelocity *= onlyMovingInOneDirection ? 2 * CAMERA_MOVING_TO_NODE_SPEED : CAMERA_MOVING_TO_NODE_SPEED * (math::Max(FRESH_MAP_ANIMATION_SPEED, glm::length(initPosition - mFreshMapCameraAnimationInitPosition)/glm::length(mCameraTargetPos - mFreshMapCameraAnimationInitPosition)));
            
            MoveMapBy(targetVelocity);
            
            mPreviousDirectionToTargetNode = directionToTarget;
            auto currentDistanceToNode = glm::distance(mCameraTargetPos, mScene->GetCamera().GetPosition());
            
            if (currentDistanceToNode < DISTANCE_TO_TARGET_NODE_THRESHOLD ||
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
    mStoryMap->DestroyParticleEmitters();
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
    
    // Realign health stat container
    mHealthStatContainer->Update(0.0f);
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
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mScene->GetUpdateTimeSpeedFactor(), 0.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(SETTINGS_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::SetMapPositionTo(const glm::vec3& position)
{
    MoveMapBy(position - mScene->GetCamera().GetPosition());
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::MoveMapBy(const glm::vec3& delta)
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

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::ResetSelectedMapNode()
{
    if (mSelectedMapCoord)
    {
        for (auto mapNodeComponentSceneObject: mScene->FindSceneObjectsWhoseNameStartsWith(mSelectedMapCoord->ToString()))
        {
            mapNodeComponentSceneObject->mPosition.z -= SELECTED_NODE_Z_OFFSET;
        }
        mSelectedMapCoord = nullptr;
    }
}

///------------------------------------------------------------------------------------------------
