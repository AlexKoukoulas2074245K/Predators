///------------------------------------------------------------------------------------------------
///  GameSceneTransitionManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/GameConstants.h>
#include <game/GameSceneTransitionManager.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/utils/Logging.h>
#include <game/events/EventSystem.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId OVERLAY_DARKENING_ANIMATION_NAME = strutils::StringId("overlay_darkening_animation");
static const strutils::StringId LOADING_SCENE_NAME = strutils::StringId("loading_scene");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";

static const float LOADING_SCENE_FADE_IN_OUT_DURATION_SECS = 0.5f;
static const float OVERLAY_ANIMATION_TARGET_DURATION_SECS = 0.5f;
static const float OVERLAY_SCALE = 10.0f;
static const float OVERLAY_Z = 23.0f;
static const float MODAL_MAX_ALPHA = 0.75f;
static const float LOADING_SCENE_FADE_IN_SPEED = 0.002f;

///------------------------------------------------------------------------------------------------

GameSceneTransitionManager::GameSceneTransitionManager()
    : mFirstTimeLoadingScreenMaxAlpha(true)
{
}

///------------------------------------------------------------------------------------------------

ISceneLogicManager* GameSceneTransitionManager::GetActiveSceneLogicManager()
{
    return mActiveSceneStack.top().mActiveSceneLogicManager;
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::Update(const float dtMillis)
{
    assert(!mActiveSceneStack.empty());
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    
    if (CoreSystemsEngine::GetInstance().GetAnimationManager().IsAnimationPlaying(OVERLAY_DARKENING_ANIMATION_NAME))
    {
        return;
    }
    
    auto outstandingLoadingJobCount = CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetOustandingLoadingJobCount();
    auto activeScene = sceneManager.FindScene(mActiveSceneStack.top().mActiveSceneName);
    if (activeScene->GetName() == LOADING_SCENE_NAME && outstandingLoadingJobCount == 0)
    {
        CoreSystemsEngine::GetInstance().GetResourceLoadingService().SetAsyncLoading(false);
        
        for (auto sceneObject: activeScene->GetSceneObjects())
        {
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, LOADING_SCENE_FADE_IN_OUT_DURATION_SECS), [=]()
            {
                CoreSystemsEngine::GetInstance().GetSceneManager().RemoveScene(LOADING_SCENE_NAME);
            });
        }
        
        DestroyActiveSceneLogicManager();
        mActiveSceneStack.pop();
        sceneManager.FindScene(mActiveSceneStack.top().mActiveSceneName)->SetLoaded(true);
        return;
    }
    else if (activeScene->GetName() == LOADING_SCENE_NAME && outstandingLoadingJobCount != 0)
    {
        if (mFirstTimeLoadingScreenMaxAlpha)
        {
            for (auto sceneObject: activeScene->GetSceneObjects())
            {
                sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
            }
            mFirstTimeLoadingScreenMaxAlpha = false;
        }
        else
        {
            for (auto sceneObject: activeScene->GetSceneObjects())
            {
                if (sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] < 1.0f)
                {
                    sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * LOADING_SCENE_FADE_IN_SPEED;
                }
            }
        }
    }
    
    if (activeScene->IsLoaded())
    {
        mActiveSceneStack.top().mActiveSceneLogicManager->VUpdate(dtMillis, activeScene);
    }
    
    
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
//    for (auto scene: CoreSystemsEngine::GetInstance().GetSceneManager().GetScenes())
//    {
//        if
//        (
//            scene->GetName() != mActiveSceneStack.top().mActiveSceneName &&
//            scene->GetSceneObjects().size() > 0 &&
//            scene->GetSceneObjects().back()->mPosition.z >= OVERLAY_Z &&
//            scene->GetSceneObjects().back()->mName != game_constants::OVERLAY_SCENE_OBJECT_NAME
//        )
//        {
//            logging::Log(logging::LogType::WARNING, "Found scene object: %s with exceeding Z: %.6f",
//                scene->GetSceneObjects().back()->mName.GetString().c_str(),
//                scene->GetSceneObjects().back()->mPosition.z);
//        }
//    }
#endif
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::ChangeToScene
(
    const strutils::StringId& sceneName,
    const SceneChangeType sceneChangeType,
    const PreviousSceneDestructionType previousSceneDestructionType
)
{
    assert(sceneChangeType != SceneChangeType::MODAL_SCENE || previousSceneDestructionType != PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
    
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    
    if (sceneChangeType != SceneChangeType::MODAL_SCENE && !mActiveSceneStack.empty())
    {
        // Destroy logic manager only when transitioning to a completely new scene
        DestroyActiveSceneLogicManager();
        
        // If we additionally want to completely wipe the previous scene, we first fade it's elements out
        if (previousSceneDestructionType == PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE)
        {
            auto sceneToDestroy = sceneManager.FindScene(mActiveSceneStack.top().mActiveSceneName);
            for (auto sceneObject: sceneToDestroy->GetSceneObjects())
            {
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, LOADING_SCENE_FADE_IN_OUT_DURATION_SECS), [=]()
                {
                    CoreSystemsEngine::GetInstance().GetSceneManager().RemoveScene(sceneToDestroy->GetName());
                });
            }
        }
        
        // Erase from active scene stack
        mActiveSceneStack.pop();
    }
    
    // Select the applicable logic manager for the given scene name
    ISceneLogicManager* nextActiveSceneLogicManager = nullptr;
    for (const auto& sceneLogicManagerEntry: mRegisteredSceneLogicManagers)
    {
        const auto& applicableSceneNames = sceneLogicManagerEntry.mSceneLogicManager->VGetApplicableSceneNames();
        if (std::find(applicableSceneNames.cbegin(), applicableSceneNames.cend(), sceneName) != applicableSceneNames.cend())
        {
            assert(nextActiveSceneLogicManager == nullptr && ("Found more than one applicable scene logic managers for scene: " + sceneName.GetString()).c_str());
            nextActiveSceneLogicManager = sceneLogicManagerEntry.mSceneLogicManager.get();
        }
    }
    
    // Create scene from scratch if non-existent
    auto newScene = sceneManager.FindScene(sceneName);
    if (!newScene)
    {
        newScene = sceneManager.CreateScene(sceneName);
    }
    
    // With darkening transition
    if (sceneChangeType == SceneChangeType::MODAL_SCENE)
    {
        // Create and setup overlay object for transition
        auto overlaySceneObject = newScene->CreateSceneObject(game_constants::OVERLAY_SCENE_OBJECT_NAME);
        overlaySceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        overlaySceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + OVERLAY_TEXTURE_FILE_NAME);
        overlaySceneObject->mScale *= OVERLAY_SCALE;
        overlaySceneObject->mPosition.z = OVERLAY_Z;
        
        // Start darkening transition animation
        auto newSceneNameCopy = sceneName;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(overlaySceneObject, MODAL_MAX_ALPHA, OVERLAY_ANIMATION_TARGET_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
        {
            mActiveSceneStack.push({nextActiveSceneLogicManager, newSceneNameCopy});
            InitializeActiveSceneLogicManager(sceneChangeType);
        }, OVERLAY_DARKENING_ANIMATION_NAME);
    }
    // Without darkening transition
    else
    {
        if (sceneChangeType == SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING)
        {
            // We first do a (recursive) call to the ChangeToScene to load the loading scene
            ChangeToScene(LOADING_SCENE_NAME, SceneChangeType::CONCRETE_SCENE_SYNC_LOADING, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
            
            // Enable async resource loading
            CoreSystemsEngine::GetInstance().GetResourceLoadingService().SetAsyncLoading(true);
            
            // Save the top entry on the stack (at this point it will be the loading scene entry).
            auto frontEntry = mActiveSceneStack.top();
            
            // Pop and push the next active scene logic managers that will be active
            mActiveSceneStack.pop();
            mActiveSceneStack.push({nextActiveSceneLogicManager, sceneName});
            
            // .. and initialize it (will load everything asynchronously at this point.
            InitializeActiveSceneLogicManager(sceneChangeType);
            
            // Finally push the loading scene entry on top to be updateable whilst the
            // rest of the resources are loading in the background.
            mActiveSceneStack.push(frontEntry);
        }
        else
        {
            mActiveSceneStack.push({nextActiveSceneLogicManager, sceneName});
            InitializeActiveSceneLogicManager(sceneChangeType);
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::PopModalScene()
{
    assert(!mActiveSceneStack.empty());
    
    auto activeScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mActiveSceneStack.top().mActiveSceneName);
    auto overlaySceneObject = activeScene->FindSceneObject(game_constants::OVERLAY_SCENE_OBJECT_NAME);
    
    // Destroy active scene and pop from stack
    DestroyActiveSceneLogicManager();
    mActiveSceneStack.pop();
    
    // If darkening transition is requested, destroy the overlay object at the end
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(overlaySceneObject, 0.0f, OVERLAY_ANIMATION_TARGET_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
    {
        activeScene->RemoveSceneObject(game_constants::OVERLAY_SCENE_OBJECT_NAME);
    });
}

///------------------------------------------------------------------------------------------------

const std::vector<GameSceneTransitionManager::SceneLogicManagerEntry>& GameSceneTransitionManager::GetRegisteredSceneLogicManagers() const
{
    return mRegisteredSceneLogicManagers;
}

///------------------------------------------------------------------------------------------------

const std::stack<GameSceneTransitionManager::ActiveSceneEntry> GameSceneTransitionManager::GetActiveSceneStack() const
{
    return mActiveSceneStack;
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::InitializeActiveSceneLogicManager(const SceneChangeType sceneChangeType)
{
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    
    auto sceneLogicManagerEntry = std::find_if(mRegisteredSceneLogicManagers.begin(), mRegisteredSceneLogicManagers.end(), [=](const SceneLogicManagerEntry& entry)
    {
        return entry.mSceneLogicManager.get() == mActiveSceneStack.top().mActiveSceneLogicManager;
    });
    
    auto activeSceneName = mActiveSceneStack.top().mActiveSceneName;
    assert(sceneLogicManagerEntry != mRegisteredSceneLogicManagers.end());
    if (!sceneLogicManagerEntry->mSceneInitStatusMap.at(activeSceneName))
    {
        auto scene = sceneManager.FindScene(activeSceneName);
        mActiveSceneStack.top().mActiveSceneLogicManager->VInitSceneCamera(scene);
        sceneManager.LoadPredefinedObjectsFromDescriptorForScene(scene);
        mActiveSceneStack.top().mActiveSceneLogicManager->VInitScene(scene);
        events::EventSystem::GetInstance().DispatchEvent<events::WindowResizeEvent>();
        sceneLogicManagerEntry->mSceneInitStatusMap[activeSceneName] = true;
        
        if (sceneChangeType != SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING)
        {
            scene->SetLoaded(true);
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::DestroyActiveSceneLogicManager()
{
    auto activeSceneName = mActiveSceneStack.top().mActiveSceneName;
    auto sceneLogicManagerEntry = std::find_if(mRegisteredSceneLogicManagers.begin(), mRegisteredSceneLogicManagers.end(), [=](const SceneLogicManagerEntry& entry)
    {
        return entry.mSceneLogicManager.get() == mActiveSceneStack.top().mActiveSceneLogicManager;
    });
    assert(sceneLogicManagerEntry != mRegisteredSceneLogicManagers.end());
    
    if (sceneLogicManagerEntry->mSceneInitStatusMap[activeSceneName])
    {
        mActiveSceneStack.top().mActiveSceneLogicManager->VDestroyScene(CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(activeSceneName));
        sceneLogicManagerEntry->mSceneInitStatusMap[activeSceneName] = false;
    }
}

///------------------------------------------------------------------------------------------------
