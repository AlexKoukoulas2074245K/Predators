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
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

class Scene final
{
public:
    [[nodiscard]] std::shared_ptr<SceneObject> CreateSceneObject();
    [[nodiscard]] std::shared_ptr<SceneObject> FindSceneObject(const strutils::StringId& sceneObjectName);
    void RemoveSceneObject(const strutils::StringId& sceneObjectName);
    
    [[nodiscard]] std::size_t GetSceneObjectCount() const;
    [[nodiscard]] const std::vector<std::shared_ptr<SceneObject>>& GetSceneObjects() const;
    [[nodiscard]] rendering::Camera& GetCamera();
    
private:
    std::vector<std::shared_ptr<SceneObject>> mSceneObjects;
    rendering::Camera mCamera;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Scene_h */
