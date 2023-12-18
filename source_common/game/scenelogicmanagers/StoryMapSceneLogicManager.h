///------------------------------------------------------------------------------------------------
///  StoryMapSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/12/2023
///------------------------------------------------------------------------------------------------

#ifndef StoryMapSceneLogicManager_h
#define StoryMapSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class StoryMapSceneLogicManager final: public ISceneLogicManager, public events::IListener
{
public:
    StoryMapSceneLogicManager();
    ~StoryMapSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    
private:
    void RegisterForEvents();
    void OnPopSceneModal(const events::PopSceneModalEvent& event);
    void OnWindowResize(const events::WindowResizeEvent& event);
    void ResetSwipeData();
    void OnSettingsButtonPressed();
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    
    glm::vec3 mSwipeStartPos;
    glm::vec3 mSwipeCurrentPos;
    bool mHasStartedSwipe;
    float mSwipeDurationMillis = 0.0f;
    float mSwipeVelocityDelta = 0.0f;
    float mSwipeDelta = 0.0f;
};

///------------------------------------------------------------------------------------------------

#endif /* StoryMapSceneLogicManager_h */
