///------------------------------------------------------------------------------------------------
///  StoryCardsSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/01/2024
///------------------------------------------------------------------------------------------------

#ifndef StoryCardsSceneLogicManager_h
#define StoryCardsSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class StoryCardsSceneLogicManager final: public ISceneLogicManager, public events::IListener
{
public:
    StoryCardsSceneLogicManager();
    ~StoryCardsSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    std::shared_ptr<GuiObjectManager> VGetGuiObjectManager() override;
    
private:
    void OnWindowResize(const events::WindowResizeEvent&);
    
private:
    std::shared_ptr<scene::Scene> mScene;
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    bool mTransitioning;
};

///------------------------------------------------------------------------------------------------

#endif /* StoryCardsSceneLogicManager_h */
