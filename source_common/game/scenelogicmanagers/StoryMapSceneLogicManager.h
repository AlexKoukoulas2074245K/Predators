///------------------------------------------------------------------------------------------------
///  StoryMapSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/12/2023
///------------------------------------------------------------------------------------------------

#ifndef StoryMapSceneLogicManager_h
#define StoryMapSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/scenelogicmanagers/ISceneLogicManager.h>

///------------------------------------------------------------------------------------------------

class StoryMapSceneLogicManager final: public ISceneLogicManager
{
public:
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    
private:
    void ResetSwipeData();

private:
    glm::vec3 mSwipeStartPos;
    glm::vec3 mSwipeCurrentPos;
    bool mHasStartedSwipe;
    float mSwipeDurationMillis = 0.0f;
    float mSwipeVelocityDelta = 0.0f;
    float mSwipeDelta = 0.0f;
};

///------------------------------------------------------------------------------------------------

#endif /* StoryMapSceneLogicManager_h */
