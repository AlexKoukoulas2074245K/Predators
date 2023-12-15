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
#include <game/SwipeableContainer.h>
#include <memory>
#include <unordered_set>

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
    enum class SubSceneType
    {
        NONE,
        MAIN,
        PRACTICE_BATTLE
    };
    
    struct CardFamilyEntry
    {
        std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjects;
        strutils::StringId mCardFamilyName;
    };
    
private:
    void InitSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene);
    void TransitionToSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene);
    void BattleModeSelected(const strutils::StringId& buttonName);
    void DeckSelected(const int selectedDeckIndex, const bool forTopPlayer);
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::unique_ptr<SwipeableContainer<CardFamilyEntry>> mCardFamilyContainerTop;
    std::unique_ptr<SwipeableContainer<CardFamilyEntry>> mCardFamilyContainerBot;
    std::vector<std::shared_ptr<scene::SceneObject>> mDeckSelectionSceneObjects;
    SubSceneType mActiveSubScene;
    bool mTransitioningToSubScene;
    bool mNeedToSetBoardPositionAndZoomFactor;
};

///------------------------------------------------------------------------------------------------

#endif /* MainMenuSceneLogicManager_h */
