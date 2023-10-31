///------------------------------------------------------------------------------------------------
///  Scene.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/scene/Scene.h>

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
    auto newSceneObject = std::make_shared<SceneObject>();
    newSceneObject->mName = sceneObjectName;
    mSceneObjects.push_back(newSceneObject);
    return newSceneObject;
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

void Scene::RemoveAllSceneObjectsWithName(const strutils::StringId& sceneObjectName)
{
    while (true)
    {
        const auto sizeBefore = GetSceneObjectCount();
        RemoveSceneObject(sceneObjectName);
        if (GetSceneObjectCount() == sizeBefore)
        {
            break;
        }
    }
}

///------------------------------------------------------------------------------------------------

std::size_t Scene::GetSceneObjectCount() const { return mSceneObjects.size(); }

///------------------------------------------------------------------------------------------------

const std::vector<std::shared_ptr<SceneObject>>& Scene::GetSceneObjects() const { return mSceneObjects; }

///------------------------------------------------------------------------------------------------

std::vector<std::shared_ptr<SceneObject>>& Scene::GetSceneObjects() { return mSceneObjects;}

///------------------------------------------------------------------------------------------------

rendering::Camera& Scene::GetCamera() { return mCamera; }

///------------------------------------------------------------------------------------------------

const rendering::Camera& Scene::GetCamera() const { return mCamera; }

///------------------------------------------------------------------------------------------------

const strutils::StringId& Scene::GetName() const { return mSceneName; }

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
