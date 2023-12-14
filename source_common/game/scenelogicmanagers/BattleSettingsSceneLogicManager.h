///------------------------------------------------------------------------------------------------
///  BattleSettingsSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#ifndef BattleSettingsSceneLogicManager_h
#define BattleSettingsSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class BattleSettingsSceneLogicManager final: public ISceneLogicManager
{
public:
    BattleSettingsSceneLogicManager();
    ~BattleSettingsSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    
private:
    enum class SubSceneType
    {
        NONE,
        MAIN,
        QUIT_CONFIRMATION
    };
    
private:
    void InitSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene);
    void TransitionToSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene);
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    SubSceneType mActiveSubScene;
    bool mTransitioningToSubScene;
};

///------------------------------------------------------------------------------------------------

#endif /* MainMenuSceneLogicManager_h */
