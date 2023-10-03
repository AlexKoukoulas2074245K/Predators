///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

Scene::Scene(const strutils::StringId& sceneName)
    : mSceneName(sceneName)
{
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<SceneObject> Scene::CreateSceneObject(const strutils::StringId sceneObjectName /* = strutils::StringId() */)
{
    mSceneObjects.emplace_back(std::make_shared<SceneObject>());
    mSceneObjects.back()->mName = sceneObjectName;
    return mSceneObjects.back();
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<SceneObject> Scene::FindSceneObject(const strutils::StringId& sceneObjectName)
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const std::shared_ptr<SceneObject>& sceneObject)
    {
        return sceneObject->mName == sceneObjectName;
    });
    
    return findIter != mSceneObjects.end() ? *findIter : nullptr;
}

///------------------------------------------------------------------------------------------------

void Scene::RemoveSceneObject(const strutils::StringId& sceneObjectName)
{
    auto findIter = std::find_if(mSceneObjects.begin(), mSceneObjects.end(), [&](const std::shared_ptr<SceneObject>& sceneObject)
    {
        return sceneObject->mName == sceneObjectName;
    });
    if (findIter != mSceneObjects.end()) mSceneObjects.erase(findIter);
}

///------------------------------------------------------------------------------------------------

[[nodiscard]] std::size_t Scene::GetSceneObjectCount() const { return mSceneObjects.size(); }

///------------------------------------------------------------------------------------------------

const std::vector<std::shared_ptr<SceneObject>>& Scene::GetSceneObjects() const { return mSceneObjects; }

///------------------------------------------------------------------------------------------------

rendering::Camera& Scene::GetCamera() { return mCamera; }

///------------------------------------------------------------------------------------------------

const strutils::StringId& Scene::GetName() const { return mSceneName; }

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
