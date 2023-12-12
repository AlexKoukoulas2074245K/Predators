///------------------------------------------------------------------------------------------------
///  MainMenuSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#ifndef MainMenuSceneLogicManager_h
#define MainMenuSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class MainMenuSceneLogicManager final: public ISceneLogicManager
{
public:
    MainMenuSceneLogicManager();
    ~MainMenuSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
};

///------------------------------------------------------------------------------------------------

#endif /* MainMenuSceneLogicManager_h */
