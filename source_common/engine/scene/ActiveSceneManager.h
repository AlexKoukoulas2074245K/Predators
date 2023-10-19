///------------------------------------------------------------------------------------------------
///  ActiveSceneManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ActiveSceneManager_h
#define ActiveSceneManager_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

class Scene;
class ActiveSceneManager final
{
public:
    [[nodiscard]] std::shared_ptr<Scene> CreateScene(const strutils::StringId sceneName = strutils::StringId());
    [[nodiscard]] std::shared_ptr<Scene> FindScene(const strutils::StringId& sceneName) const;
    
    void SortSceneObjects(std::shared_ptr<Scene> scene);
    void RemoveScene(const strutils::StringId& sceneName);
    
    [[nodiscard]] std::size_t GetSceneCount() const;
    [[nodiscard]] const std::vector<std::shared_ptr<Scene>>& GetScenes() const;
    
private:
    std::vector<std::shared_ptr<Scene>> mScenes;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* ActiveSceneManager_h */
