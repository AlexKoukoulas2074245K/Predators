///------------------------------------------------------------------------------------------------
///  StoryMapSceneLogicManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/MeshResource.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <game/AnimatedButton.h>
#include <game/AnimatedStatContainer.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/GuiObjectManager.h>
#include <game/DataRepository.h>
#include <game/scenelogicmanagers/StoryMapSceneLogicManager.h>
#include <thread>

///------------------------------------------------------------------------------------------------

static const strutils::StringId VISIT_MAP_NODE_SCENE = strutils::StringId("visit_map_node_scene");
static const strutils::StringId SETTINGS_SCENE = strutils::StringId("settings_scene");
static const strutils::StringId BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("background");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const std::string COIN_VALUE_TEXT_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string SETTINGS_ICON_TEXTURE_FILE_NAME = "settings_button_icon.png";
static const std::string COIN_STACK_TEXTURE_FILE_NAME = "coin_stack.png";
static const std::string HEALTH_CRYSTAL_TEXTURE_FILE_NAME = "health_crystal.png";
static const std::string HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX = "health_crystal_";

static const glm::vec2 MAP_SWIPE_X_BOUNDS = {-0.78f, 0.78f};
static const glm::vec2 MAP_SWIPE_Y_BOUNDS = {-0.78f, 0.78f};

static const float DISTANCE_TO_TARGET_NODE_THRESHOLD = 0.01f;
static const float CAMERA_NOT_MOVED_THRESHOLD = 0.0001f;
static const float CAMERA_MOVING_TO_NODE_SPEED = 0.0005f;
static const float SELECTED_NODE_Z_OFFSET = 23.3f;
static const float FRESH_MAP_ANIMATION_TARGET_Y_OFFSET = -0.19f;
static const float SWIPE_VELOCITY_DAMPING = 0.8f;
static const float SWIPE_VELOCITY_INTEGRATION_SPEED = 0.05f;
static const float SWIPE_VELOCITY_MIN_MAGNITUDE_TO_START_MOVING = 0.0001f;
static const float MAX_CAMERA_DISTANCE_TO_REGISTER_NODE_TAP = 0.01f;

#if defined(NDEBUG) || defined(MOBILE_FLOW)
static const float FRESH_MAP_ANIMATION_SPEED = 0.2f;
#else
static const float FRESH_MAP_ANIMATION_SPEED = 3.0f;
#endif

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::STORY_MAP_SCENE
};

static const std::vector<strutils::StringId> GUI_SCENE_OBJECT_NAMES =
{
    game_constants::GUI_COIN_STACK_SCENE_OBJECT_NAME,
    game_constants::GUI_COIN_VALUE_TEXT_SCENE_OBJECT_NAME,
    game_constants::GUI_SETTINGS_BUTTON_SCENE_OBJECT_NAME,
    game_constants::GUI_STORY_CARDS_BUTTON_SCENE_OBJECT_NAME,
    strutils::StringId(HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX + "base"),
    strutils::StringId(HEALTH_CRYSTAL_SCENE_OBJECT_NAME_PREFIX + "value")
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
    if (DataRepository::GetInstance().GetStoryMapGenerationSeed() == 0)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::LoadingProgressPrefixTextOverrideEvent>("Generating New Story: ");
    }
    
    const auto& currentMapCoord = DataRepository::GetInstance().GetCurrentStoryMapNodeCoord();
    std::thread mapGenerationThread = std::thread([=]
    {
        mStoryMap = std::make_unique<StoryMap>(scene, game_constants::STORY_NODE_MAP_DIMENSIONS, MapCoord(currentMapCoord.x, currentMapCoord.y));
        mStoryMap->GenerateMapNodes();
    });
    mapGenerationThread.detach();
    
    RegisterForEvents();
    
    mGuiManager = std::make_shared<GuiObjectManager>(scene);
    
    mSwipeCamera = scene->GetCamera();
    mScene = scene;
    
    ResetSwipeData();
    
    DataRepository::GetInstance().SetCurrentStoryMapSceneType(StoryMapSceneType::STORY_MAP);
    DataRepository::GetInstance().FlushStateToFile();
    
    mExcludedSceneObjectsFromFrustumCulling.clear();
    
    mMapUpdateState = MapUpdateState::NAVIGATING;
    mSelectedMapCoord = nullptr;
    mTappedMapNodeData = nullptr;
    mSwipeVelocity = glm::vec3(0.0f);
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    const auto& currentMapCoord = DataRepository::GetInstance().GetCurrentStoryMapNodeCoord();
    
    if (!mStoryMap->HasCreatedSceneObjects())
    {
        const auto& mapGenerationInfo = mStoryMap->GetMapGenerationInfo();
        logging::Log(logging::LogType::INFO, "Finished Map Generation after %d attempts", mapGenerationInfo.mMapGenerationAttempts);
        logging::Log(logging::LogType::INFO, "Close To Start Node Errors %d", mapGenerationInfo.mCloseToStartingNodeErrors);
        logging::Log(logging::LogType::INFO, "Close To Boss Node Errors %d", mapGenerationInfo.mCloseToBossNodeErrors);
        logging::Log(logging::LogType::INFO, "Close To North Edge Errors %d", mapGenerationInfo.mCloseToNorthEdgeErrors);
        logging::Log(logging::LogType::INFO, "Close To South Edge Errors %d", mapGenerationInfo.mCloseToSouthEdgeErrors);
        logging::Log(logging::LogType::INFO, "Close To Other Nodes Errors %d", mapGenerationInfo.mCloseToOtherNodesErrors);
        mStoryMap->CreateMapSceneObjects();
        
        for (const auto& sceneObject: scene->GetSceneObjects())
        {
            if (sceneObject->mInvisible || std::holds_alternative<scene::ParticleEmitterObjectData>(sceneObject->mSceneObjectTypeData))
            {
                mExcludedSceneObjectsFromFrustumCulling.insert(sceneObject);
            }
        }
        
        // First time entering map initialisation
        if (currentMapCoord.x == game_constants::STORY_MAP_INIT_COORD.x && currentMapCoord.y == game_constants::STORY_MAP_INIT_COORD.y)
        {
            mGuiManager->ForceSetStoryHealthValue(DataRepository::GetInstance().StoryCurrentHealth().GetValue());
            
            mMapUpdateState = MapUpdateState::FRESH_MAP_ANIMATION;
            SetMapPositionTo(mStoryMap->GetMapData().at(MapCoord(game_constants::STORY_MAP_BOSS_COORD.x, game_constants::STORY_MAP_BOSS_COORD.y)).mPosition);
            
            mFreshMapCameraAnimationInitPosition = mScene->GetCamera().GetPosition();
            mCameraTargetPos = mStoryMap->GetMapData().at(MapCoord(game_constants::STORY_MAP_INIT_COORD.x, game_constants::STORY_MAP_INIT_COORD.y)).mPosition;
            mCameraTargetPos.y += FRESH_MAP_ANIMATION_TARGET_Y_OFFSET;
            
            mCameraTargetPos.x = math::Max(MAP_SWIPE_X_BOUNDS.s, math::Min(MAP_SWIPE_X_BOUNDS.t, mCameraTargetPos.x));
            mCameraTargetPos.y = math::Max(MAP_SWIPE_Y_BOUNDS.s, math::Min(MAP_SWIPE_Y_BOUNDS.t, mCameraTargetPos.y));
            
            mCameraTargetPos.z = mScene->GetCamera().GetPosition().z;
        }
        // Subsequent map enters. Set position to average between current map coord and active nodes
        else
        {
            auto& mapNodeData = mStoryMap->GetMapData().at(MapCoord(currentMapCoord.x, currentMapCoord.y));
            glm::vec3 positionAccum = mapNodeData.mPosition;
            int positionInfluenceCount = 1;
            
            for (const auto& link: mapNodeData.mNodeLinks)
            {
                positionAccum += mStoryMap->GetMapData().at(MapCoord(link.mCol, link.mRow)).mPosition;
                positionInfluenceCount++;
            }
            
            SetMapPositionTo(positionAccum/static_cast<float>(positionInfluenceCount));
        }
    }
    
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
            
            auto guiInteractionResult = mGuiManager->Update(dtMillis);
            bool interactedWithGui = guiInteractionResult == GuiUpdateInteractionResult::CLICKED_GUI_BUTTONS;
            
            const auto& currentCoord = MapCoord(DataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x, DataRepository::GetInstance().GetCurrentStoryMapNodeCoord().y);
            const auto& currentMapNode = mStoryMap->GetMapData().at(currentCoord);
                
            const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
            auto touchPos = inputStateManager.VGetPointingPosInWorldSpace(mSwipeCamera.GetViewMatrix(), mSwipeCamera.GetProjMatrix());
            auto worldTouchPos = glm::vec3(touchPos.x, touchPos.y, 0.0f);
            
            if (!interactedWithGui && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
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
                        
                        // Node registered as tapped, nothing will happen unless
                        // we release the touch and the camera hasn't moved much from it
                        mTappedMapNodeData = std::make_shared<StoryMap::NodeData>(nodeMapData.second);
                        mTappedNodeInitCameraPosition = mScene->GetCamera().GetPosition();
                        break;
                    }
                }
                
                mSwipeVelocity = glm::vec3(0.0f);
                if (tappedGuiSceneObject)
                {
                    ResetSwipeData();
                }
                else
                {
                    mSwipeCurrentPos = worldTouchPos;
                    mHasStartedSwipe = true;
                }
            }
            else if (!interactedWithGui && inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
            {
                if (mHasStartedSwipe)
                {
                    const auto deltaMotion = mSwipeCurrentPos - worldTouchPos;
                    if (glm::length(deltaMotion) < 1.0f)
                    {
                        mSwipeVelocity = deltaMotion;
                    }
                    
                    mSwipeCurrentPos = worldTouchPos;
                }
            }
            else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
            {
                if (!interactedWithGui)
                {
                    ResetSwipeData();
                }
                
                // Only once the touch is released and we the camera is sufficiently close
                // to the originally tapped node do we actually move to it.
                if (mTappedMapNodeData && glm::distance(mTappedNodeInitCameraPosition, mScene->GetCamera().GetPosition()) < MAX_CAMERA_DISTANCE_TO_REGISTER_NODE_TAP)
                {
                    MapCoord targetMapCoord(mTappedMapNodeData->mCoords.x, mTappedMapNodeData->mCoords.y);
                    mSwipeVelocity = glm::vec3(0.0f);
                    ResetSwipeData();
                    
                    ResetSelectedMapNode();

                    // Setup data for moving to target node
                    DataRepository::GetInstance().SetSelectedStoryMapNodePosition(mTappedMapNodeData->mPosition);
                    DataRepository::GetInstance().SetSelectedStoryMapNodeData(&mStoryMap->GetMapData().at(targetMapCoord));

                    mMapUpdateState = MapUpdateState::MOVING_TO_NODE;
                    mCameraTargetPos = mTappedMapNodeData->mPosition;
                    mCameraTargetPos.x = math::Max(MAP_SWIPE_X_BOUNDS.s, math::Min(MAP_SWIPE_X_BOUNDS.t, mCameraTargetPos.x));
                    mCameraTargetPos.y = math::Max(MAP_SWIPE_Y_BOUNDS.s, math::Min(MAP_SWIPE_Y_BOUNDS.t, mCameraTargetPos.y));
                    mCameraTargetPos.z = mScene->GetCamera().GetPosition().z;
                    mSelectedMapCoord = std::make_unique<MapCoord>(mTappedMapNodeData->mCoords.x, mTappedMapNodeData->mCoords.y);
                    for (auto mapNodeComponentSceneObject: scene->FindSceneObjectsWhoseNameStartsWith(mSelectedMapCoord->ToString()))
                    {
                        mapNodeComponentSceneObject->mPosition.z += SELECTED_NODE_Z_OFFSET;
                    }
                }
                        
                mTappedMapNodeData = nullptr;
            }
            
            // Move map and integrate velocity
            if (glm::length(mSwipeVelocity) > SWIPE_VELOCITY_MIN_MAGNITUDE_TO_START_MOVING)
            {
                MoveMapBy(mSwipeVelocity * dtMillis * SWIPE_VELOCITY_INTEGRATION_SPEED);
                mSwipeVelocity.x *= SWIPE_VELOCITY_DAMPING;
                mSwipeVelocity.y *= SWIPE_VELOCITY_DAMPING;
            }
            else
            {
                mSwipeVelocity = glm::vec3(0.0f);
            }
        } break;
            
        case MapUpdateState::MOVING_TO_NODE:
        {
            auto initPosition = mScene->GetCamera().GetPosition();
            auto directionToTarget = mCameraTargetPos - initPosition;
            
            bool alreadyArrivedAtTarget =
                math::Abs(directionToTarget.x) < DISTANCE_TO_TARGET_NODE_THRESHOLD &&
                math::Abs(directionToTarget.y) < DISTANCE_TO_TARGET_NODE_THRESHOLD &&
                math::Abs(directionToTarget.z) < DISTANCE_TO_TARGET_NODE_THRESHOLD;
            
            auto currentDistanceToNode = 0.0f;
            
            if (!alreadyArrivedAtTarget)
            {
                bool onlyMovingInOneDirection = (math::Abs(directionToTarget.x - mPreviousDirectionToTargetNode.x) <= CAMERA_NOT_MOVED_THRESHOLD || math::Abs(directionToTarget.y - mPreviousDirectionToTargetNode.y) <= CAMERA_NOT_MOVED_THRESHOLD);
                
                auto normalizedDirectionToTarget = glm::normalize(directionToTarget);
                auto targetVelocity = normalizedDirectionToTarget * dtMillis;
                
                targetVelocity *= onlyMovingInOneDirection ? 2 * CAMERA_MOVING_TO_NODE_SPEED : CAMERA_MOVING_TO_NODE_SPEED;
                
                MoveMapBy(targetVelocity);
                
                mPreviousDirectionToTargetNode = directionToTarget;
                currentDistanceToNode = glm::distance(mCameraTargetPos, mScene->GetCamera().GetPosition());
                
                alreadyArrivedAtTarget = currentDistanceToNode <= glm::length(targetVelocity);
            }
            
            if (alreadyArrivedAtTarget || currentDistanceToNode < DISTANCE_TO_TARGET_NODE_THRESHOLD ||
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
    
    const auto& currentFrustum = mScene->GetCamera().CalculateFrustum();
    for (auto& sceneObject: mScene->GetSceneObjects())
    {
        if (mExcludedSceneObjectsFromFrustumCulling.count(sceneObject))
        {
            continue;
        }
        
        auto sceneObjectMeshDimensions = CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetResource<resources::MeshResource>(sceneObject->mMeshResourceId).GetDimensions();
        if (std::holds_alternative<scene::TextSceneObjectData>(sceneObject->mSceneObjectTypeData))
        {
            sceneObjectMeshDimensions *= 1000.0f;
        }
        
        int breachedSideIndex = 0;
        sceneObject->mInvisible = !math::IsMeshAtLeastPartlyInsideFrustum(sceneObject->mPosition, sceneObject->mScale, sceneObjectMeshDimensions, currentFrustum, breachedSideIndex);
    }
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    mGuiManager = nullptr;
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
    mExcludedSceneObjectsFromFrustumCulling.clear();
    mStoryMap->DestroyParticleEmitters();
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> StoryMapSceneLogicManager::VGetGuiObjectManager()
{
    return mGuiManager;
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    eventSystem.RegisterForEvent<events::PopSceneModalEvent>(this, &StoryMapSceneLogicManager::OnPopSceneModal);
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &StoryMapSceneLogicManager::OnWindowResize);
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
    mGuiManager->OnWindowResize();
}

///------------------------------------------------------------------------------------------------

void StoryMapSceneLogicManager::ResetSwipeData()
{
    mSwipeCamera.SetPosition(mScene->GetCamera().GetPosition());
    mHasStartedSwipe = false;
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
