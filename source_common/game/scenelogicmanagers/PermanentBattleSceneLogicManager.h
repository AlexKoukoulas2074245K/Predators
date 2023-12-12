///------------------------------------------------------------------------------------------------
///  PermanentBattleSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#ifndef PermanentBattleSceneLogicManager_h
#define PermanentBattleSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/scenelogicmanagers/ISceneLogicManager.h>

///------------------------------------------------------------------------------------------------

class PermanentBattleSceneLogicManager final: public ISceneLogicManager
{
public:
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
};

///------------------------------------------------------------------------------------------------

#endif /* PermanentBattleSceneLogicManager_h */
