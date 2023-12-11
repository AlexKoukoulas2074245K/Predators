///------------------------------------------------------------------------------------------------
///  LoadingSceneLogicManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/SceneManager.h>
#include <game/scenelogicmanagers/LoadingSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId LOADING_SCENE_NAME = strutils::StringId("loading_scene");
static const strutils::StringId LOADING_SCENE_BACKGROUND_SCENE_OBJECT_NAME = strutils::StringId("loading_background");
static const strutils::StringId LOADING_PROGRESS_TEXT_SCENE_OBJECT_NAME = strutils::StringId("loading_text");

static const float LOADING_PROGRESS_TEXT_PULSE_SCALE_FACTOR = 1.05f;
static const float LOADING_PROGRESS_TEXT_INTER_PULSE_DURATION_SECS = 1.0f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    LOADING_SCENE_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& LoadingSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

void LoadingSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mTotalLoadingJobCount = -1;
    SetLoadingProgress(0);
    
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::PulseAnimation>(scene->FindSceneObject(LOADING_PROGRESS_TEXT_SCENE_OBJECT_NAME), LOADING_PROGRESS_TEXT_PULSE_SCALE_FACTOR, LOADING_PROGRESS_TEXT_INTER_PULSE_DURATION_SECS, animation_flags::ANIMATE_CONTINUOUSLY), [](){});
}

///------------------------------------------------------------------------------------------------

void LoadingSceneLogicManager::VUpdate(const float, std::shared_ptr<scene::Scene>)
{
    if (mTotalLoadingJobCount == -1)
    {
        mTotalLoadingJobCount = CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetOustandingLoadingJobCount();
    }
    
    auto outstandingLoadingJobCount = CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetOustandingLoadingJobCount();
    SetLoadingProgress(static_cast<int>((mTotalLoadingJobCount - outstandingLoadingJobCount)/static_cast<float>(mTotalLoadingJobCount) * 100.0f));
}

///------------------------------------------------------------------------------------------------

void LoadingSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    SetLoadingProgress(100);
}

///------------------------------------------------------------------------------------------------

void LoadingSceneLogicManager::SetLoadingProgress(const int progressPercent)
{
    auto loadingScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(LOADING_SCENE_NAME);
    auto loadingProgressSceneObject = loadingScene->FindSceneObject(LOADING_PROGRESS_TEXT_SCENE_OBJECT_NAME);
    std::get<scene::TextSceneObjectData>(loadingProgressSceneObject->mSceneObjectTypeData).mText = "Loading Progress: " + std::to_string(progressPercent) + "%";
}

///------------------------------------------------------------------------------------------------
