///------------------------------------------------------------------------------------------------
///  ISceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ISceneLogicManager_h
#define ISceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>

///------------------------------------------------------------------------------------------------

class ISceneLogicManager
{
public:
    virtual ~ISceneLogicManager() = default;
    
    virtual const std::vector<strutils::StringId>& VGetApplicableSceneNames() const = 0;
    virtual void VInitScene(const strutils::StringId& sceneName) = 0;
    virtual void VUpdate(const float dtMillis, const strutils::StringId& activeSceneName) = 0;
    virtual void VDestroyScene(const strutils::StringId& sceneName) = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* ISceneLogicManager_h */
