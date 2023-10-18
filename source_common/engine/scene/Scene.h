///------------------------------------------------------------------------------------------------
///  Scene.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Scene_h
#define Scene_h

///------------------------------------------------------------------------------------------------

#include <engine/rendering/Camera.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/StringUtils.h>
#include <memory>
#include <set>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

struct SceneObjectComparator
{
public:
    bool operator () (const std::shared_ptr<scene::SceneObject>& lhs, const std::shared_ptr<scene::SceneObject>& rhs) const
    {
        return lhs->mPosition.z > rhs->mPosition.z;
    }
};

///------------------------------------------------------------------------------------------------

struct SceneObject;
class Scene final
{
public:
    Scene(const strutils::StringId& sceneName);
    
    [[nodiscard]] std::shared_ptr<SceneObject> CreateSceneObject(const strutils::StringId sceneObjectName = strutils::StringId());
    [[nodiscard]] std::shared_ptr<SceneObject> FindSceneObject(const strutils::StringId& sceneObjectName);
    
    void RemoveSceneObject(const strutils::StringId& sceneObjectName);
    void RemoveAllSceneObjectsWithName(const strutils::StringId& sceneObjectName);
    
    [[nodiscard]] std::size_t GetSceneObjectCount() const;
    [[nodiscard]] const std::multiset<std::shared_ptr<SceneObject>, SceneObjectComparator>& GetSceneObjects() const;
    [[nodiscard]] std::multiset<std::shared_ptr<SceneObject>, SceneObjectComparator>& GetSceneObjects();
    [[nodiscard]] rendering::Camera& GetCamera();
    [[nodiscard]] const strutils::StringId& GetName() const;
    
private:
    const strutils::StringId mSceneName;
    std::multiset<std::shared_ptr<SceneObject>, SceneObjectComparator> mSceneObjects;
    rendering::Camera mCamera;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
