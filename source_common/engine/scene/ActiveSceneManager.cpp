///------------------------------------------------------------------------------------------------
///  ActiveSceneManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/scene/Scene.h>
#include <engine/scene/ActiveSceneManager.h>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

std::shared_ptr<Scene> ActiveSceneManager::CreateScene(const strutils::StringId sceneName /* = strutils::StringId() */)
{
    mScenes.emplace_back(std::make_shared<Scene>(sceneName));
    return mScenes.back();
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<Scene> ActiveSceneManager::FindScene(const strutils::StringId& sceneName) const
{
    auto findIter = std::find_if(mScenes.begin(), mScenes.end(), [&](const std::shared_ptr<Scene>& scene)
                                 {
        return scene->GetName() == sceneName;
    });
    
    return findIter != mScenes.end() ? *findIter : nullptr;
}

///------------------------------------------------------------------------------------------------

void ActiveSceneManager::SortSceneObjects(std::shared_ptr<Scene> scene)
{
    auto& sceneObjects = scene->GetSceneObjects();
    std::sort(sceneObjects.begin(), sceneObjects.end(), [&](const std::shared_ptr<scene::SceneObject>& lhs, const std::shared_ptr<scene::SceneObject>& rhs)
    {
        return lhs->mPosition.z < rhs->mPosition.z;
    });
}

///------------------------------------------------------------------------------------------------

void ActiveSceneManager::RemoveScene(const strutils::StringId& sceneName)
{
    auto findIter = std::find_if(mScenes.begin(), mScenes.end(), [&](const std::shared_ptr<Scene>& scene)
                                 {
        return scene->GetName() == sceneName;
    });
    if (findIter != mScenes.end()) mScenes.erase(findIter);
}

///------------------------------------------------------------------------------------------------

[[nodiscard]] std::size_t ActiveSceneManager::GetSceneCount() const { return mScenes.size(); }

///------------------------------------------------------------------------------------------------

const std::vector<std::shared_ptr<Scene>>& ActiveSceneManager::GetScenes() const { return mScenes; }

///------------------------------------------------------------------------------------------------

}
