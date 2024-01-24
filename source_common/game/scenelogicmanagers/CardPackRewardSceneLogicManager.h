///------------------------------------------------------------------------------------------------
///  CardPackRewardSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 24/01/2024
///------------------------------------------------------------------------------------------------

#ifndef CardPackRewardSceneLogicManager_h
#define CardPackRewardSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class CardPackRewardSceneLogicManager final: public ISceneLogicManager, public events::IListener
{
public:
    CardPackRewardSceneLogicManager();
    ~CardPackRewardSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    std::shared_ptr<GuiObjectManager> VGetGuiObjectManager() override;
    
private:
    void RegisterForEvents();
    void OnWindowResize(const events::WindowResizeEvent& event);
    void UpdatePackVertices(const float dtMillis, std::shared_ptr<scene::Scene> scene);
    void PreparePackVertexVelocities(std::shared_ptr<scene::Scene> scene);
    void CardPackShakeStep(std::shared_ptr<scene::Scene> scene);
    void CreateCardRewards(std::shared_ptr<scene::Scene> scene);
    
private:
    enum class SceneState
    {
        PENDING_PACK_OPENING,
        PACK_SHAKING,
        PACK_EXPLODING
    };
    
    std::unique_ptr<AnimatedButton> mOpenButton;
    std::vector<glm::vec3> mCardPackVertexVelocities;
    SceneState mSceneState;
    int mCardPackShakeStepsRemaining;
};

///------------------------------------------------------------------------------------------------

#endif /* CardPackRewardSceneLogicManager_h */
