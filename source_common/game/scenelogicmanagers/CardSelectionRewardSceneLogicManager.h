///------------------------------------------------------------------------------------------------
///  CardSelectionRewardSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/01/2024
///------------------------------------------------------------------------------------------------

#ifndef CardSelectionRewardSceneLogicManager_h
#define CardSelectionRewardSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
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
    std::unique_ptr<AnimatedButton> mOkButton;
    bool mHasConfirmedSelection;
    bool mHasPresentedSceneObjects;
    float mInitialSurfacingDelaySecs;
};

///------------------------------------------------------------------------------------------------

#endif /* CardSelectionRewardSceneLogicManager_h */
