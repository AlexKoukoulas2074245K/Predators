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
#include <game/StoryNodeMap.h>
#include <engine/rendering/Camera.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class StoryNodeMap;
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
    void SetCoinValueText();
    void OnPopSceneModal(const events::PopSceneModalEvent& event);
    void OnWindowResize(const events::WindowResizeEvent& event);
    void ResetSwipeData();
    void OnSettingsButtonPressed();
    void MoveWorldBy(const glm::vec3& delta);
    void MoveGUIBy(const glm::vec3& delta);
    void ResetSelectedMapNode();
    
private:
    enum class MapUpdateState
    {
        NAVIGATING,
        MOVING_TO_NODE
    };
    
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::unique_ptr<StoryNodeMap> mStoryNodeMap;
    std::unique_ptr<MapCoord> mSelectedMapCoord;
    std::shared_ptr<scene::Scene> mScene;
    rendering::Camera mSwipeCamera;
    glm::vec3 mSwipeCurrentPos;
    glm::vec3 mCameraTargetPos;
    MapUpdateState mMapUpdateState;
    bool mHasStartedSwipe;
};

///------------------------------------------------------------------------------------------------

#endif /* StoryMapSceneLogicManager_h */
