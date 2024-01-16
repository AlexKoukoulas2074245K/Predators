///------------------------------------------------------------------------------------------------
///  CardSelectionRewardSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/01/2024
///------------------------------------------------------------------------------------------------

#ifndef CardSelectionRewardSceneLogicManager_h
#define CardSelectionRewardSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class CardTooltipController;
class CardSelectionRewardSceneLogicManager final: public ISceneLogicManager
{
public:
    CardSelectionRewardSceneLogicManager();
    ~CardSelectionRewardSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    std::shared_ptr<GuiObjectManager> VGetGuiObjectManager() override;
    
private:
    void CreateCardRewards(std::shared_ptr<scene::Scene> scene);
    void CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText, const size_t cardIndex, std::shared_ptr<scene::Scene> scene);
    void DestroyCardTooltip(std::shared_ptr<scene::Scene> scene);
    
private:
    enum class SceneState
    {
        PENDING_PRESENTATION,
        PENDING_CARD_SELECTION,
        PENDING_CARD_SELECTION_CONFIRMATION
    };
    
    std::vector<std::shared_ptr<CardSoWrapper>> mCardRewards;
    std::unique_ptr<AnimatedButton> mConfirmationButton;
    std::unique_ptr<CardTooltipController> mCardTooltipController;
    SceneState mSceneState;
    float mInitialSurfacingDelaySecs;
};

///------------------------------------------------------------------------------------------------

#endif /* CardSelectionRewardSceneLogicManager_h */
