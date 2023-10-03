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
#include <engine/utils/StringUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

struct SceneObject;
class Scene final
{
public:
    Scene(const strutils::StringId& sceneName);
    
    [[nodiscard]] std::shared_ptr<SceneObject> CreateSceneObject(const strutils::StringId sceneObjectName = strutils::StringId());
    [[nodiscard]] std::shared_ptr<SceneObject> FindSceneObject(const strutils::StringId& sceneObjectName);
    
    void RemoveSceneObject(const strutils::StringId& sceneObjectName);
    
    [[nodiscard]] std::size_t GetSceneObjectCount() const;
    [[nodiscard]] const std::vector<std::shared_ptr<SceneObject>>& GetSceneObjects() const;
    [[nodiscard]] rendering::Camera& GetCamera();
    [[nodiscard]] const strutils::StringId& GetName() const;
    
private:
    const strutils::StringId mSceneName;
    std::vector<std::shared_ptr<SceneObject>> mSceneObjects;
    rendering::Camera mCamera;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
