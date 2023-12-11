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
#include <engine/scene/Scene.h>

///------------------------------------------------------------------------------------------------

class ISceneLogicManager
{
public:
    virtual ~ISceneLogicManager() = default;
    
    virtual const std::vector<strutils::StringId>& VGetApplicableSceneNames() const = 0;
    virtual void VInitScene(std::shared_ptr<scene::Scene> scene) = 0;
    virtual void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) = 0;
    virtual void VDestroyScene(std::shared_ptr<scene::Scene> scene) = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* ISceneLogicManager_h */
