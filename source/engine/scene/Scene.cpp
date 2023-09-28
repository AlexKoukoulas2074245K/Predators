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

std::shared_ptr<SceneObject> Scene::CreateSceneObject()
{
    mSceneObjects.emplace_back(std::make_shared<SceneObject>());
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

}

///------------------------------------------------------------------------------------------------
