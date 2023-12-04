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

static const strutils::StringId OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("OVERLAY");
static const strutils::StringId SCENE_TRANSITION_ANIMATION_NAME = strutils::StringId("SCENE_TRANSITION_ANIMATION");

static const std::string OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const float OVERLAY_SCALE = 10.0f;
static const float OVERLAY_Z = 23.0f;

///------------------------------------------------------------------------------------------------

GameSceneTransitionManager::GameSceneTransitionManager()
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
    
    if (!CoreSystemsEngine::GetInstance().GetAnimationManager().IsAnimationPlaying(SCENE_TRANSITION_ANIMATION_NAME))
    {
        mActiveSceneStack.top().mActiveSceneLogicManager->VUpdate(dtMillis, CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mActiveSceneStack.top().mActiveSceneName));
    }
    
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
    for (auto scene: CoreSystemsEngine::GetInstance().GetSceneManager().GetScenes())
    {
        if
        (
            scene->GetName() != mActiveSceneStack.top().mActiveSceneName &&
            scene->GetSceneObjects().size() > 0 &&
            scene->GetSceneObjects().back()->mPosition.z >= OVERLAY_Z &&
            scene->GetSceneObjects().back()->mName != OVERLAY_SCENE_OBJECT_NAME
        )
        {
            logging::Log(logging::LogType::WARNING, "Found scene object: %s with exceeding Z: %.6f",
                scene->GetSceneObjects().back()->mName.GetString().c_str(),
                scene->GetSceneObjects().back()->mPosition.z);
        }
    }
#endif
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::ChangeToScene
(
    const strutils::StringId& sceneName,
    const bool isModal,
    const float targetTransitionDurationSecs /* = 0.0f */,
    const float maxTransitionDarkeningAlpha /* = 0.0f */
)
{
    // Destroy logic manager only when transitioning to a completely new scene
    if (!isModal && !mActiveSceneStack.empty())
    {
        DestroyActiveSceneLogicManager();
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
    
    // With darkening transition
    if (targetTransitionDurationSecs > 0.0f)
    {
        // Create scene from scratch if non-existent
        auto newScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(sceneName);
        if (!newScene)
        {
            newScene = CoreSystemsEngine::GetInstance().GetSceneManager().CreateScene(sceneName);
        }
        
        // Create and setup overlay object for transition
        auto overlaySceneObject = newScene->CreateSceneObject(OVERLAY_SCENE_OBJECT_NAME);
        overlaySceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        overlaySceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + OVERLAY_TEXTURE_FILE_NAME);
        overlaySceneObject->mScale *= OVERLAY_SCALE;
        overlaySceneObject->mPosition.z = OVERLAY_Z;
        
        // Start darkening transition animation
        auto newSceneNameCopy = sceneName;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(overlaySceneObject, maxTransitionDarkeningAlpha, targetTransitionDurationSecs, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
        {
            mActiveSceneStack.push({nextActiveSceneLogicManager, newSceneNameCopy});
            InitializeActiveSceneLogicManager();
        }, SCENE_TRANSITION_ANIMATION_NAME);
    }
    // Without darkening transition
    else
    {
        mActiveSceneStack.push({nextActiveSceneLogicManager, sceneName});
        InitializeActiveSceneLogicManager();
    }
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::PopModalScene
(
    const float targetTransitionDurationSecs /* = 0.0f */,
    const float maxTransitionDarkeningAlpha /* = 0.0f */
)
{
    assert(!mActiveSceneStack.empty());
    
    auto activeScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mActiveSceneStack.top().mActiveSceneName);
    auto overlaySceneObject = activeScene->FindSceneObject(OVERLAY_SCENE_OBJECT_NAME);
    
    // Destroy active scene and pop from stack
    DestroyActiveSceneLogicManager();
    mActiveSceneStack.pop();
    
    if (targetTransitionDurationSecs > 0.0f)
    {
        // If darkening transition is requested, destroy the overlay object at the end
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(overlaySceneObject, maxTransitionDarkeningAlpha, targetTransitionDurationSecs, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
        {
            activeScene->RemoveSceneObject(OVERLAY_SCENE_OBJECT_NAME);
        });
    }
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::InitializeActiveSceneLogicManager()
{
    auto sceneLogicManagerEntry = std::find_if(mRegisteredSceneLogicManagers.begin(), mRegisteredSceneLogicManagers.end(), [=](const SceneLogicManagerEntry& entry)
    {
        return entry.mSceneLogicManager.get() == mActiveSceneStack.top().mActiveSceneLogicManager;
    });
    
    auto activeSceneName = mActiveSceneStack.top().mActiveSceneName;
    assert(sceneLogicManagerEntry != mRegisteredSceneLogicManagers.end());
    if (!sceneLogicManagerEntry->mSceneInitStatusMap.at(activeSceneName))
    {
        mActiveSceneStack.top().mActiveSceneLogicManager->VInitScene(CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(activeSceneName));
        events::EventSystem::GetInstance().DispatchEvent<events::WindowResizeEvent>();
        sceneLogicManagerEntry->mSceneInitStatusMap[activeSceneName] = true;
    }
}

///------------------------------------------------------------------------------------------------

void GameSceneTransitionManager::DestroyActiveSceneLogicManager()
{
    auto activeSceneName = mActiveSceneStack.top().mActiveSceneName;
    mActiveSceneStack.top().mActiveSceneLogicManager->VDestroyScene(CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(activeSceneName));
    auto sceneLogicManagerEntry = std::find_if(mRegisteredSceneLogicManagers.begin(), mRegisteredSceneLogicManagers.end(), [=](const SceneLogicManagerEntry& entry)
    {
        return entry.mSceneLogicManager.get() == mActiveSceneStack.top().mActiveSceneLogicManager;
    });
    assert(sceneLogicManagerEntry != mRegisteredSceneLogicManagers.end());
    sceneLogicManagerEntry->mSceneInitStatusMap[activeSceneName] = false;
}

///------------------------------------------------------------------------------------------------
