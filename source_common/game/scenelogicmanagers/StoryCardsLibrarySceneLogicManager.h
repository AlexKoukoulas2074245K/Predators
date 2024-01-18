///------------------------------------------------------------------------------------------------
///  StoryCardsLibrarySceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/01/2024
///------------------------------------------------------------------------------------------------

#ifndef StoryCardsLibrarySceneLogicManager_h
#define StoryCardsLibrarySceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <game/SwipeableContainer.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class CardTooltipController;
class StoryCardsLibrarySceneLogicManager final: public ISceneLogicManager, public events::IListener
{
public:
    StoryCardsLibrarySceneLogicManager();
    ~StoryCardsLibrarySceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    std::shared_ptr<GuiObjectManager> VGetGuiObjectManager() override;
    
private:
    void OnWindowResize(const events::WindowResizeEvent&);
    void CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText);
    void DestroyCardTooltip();
    void SelectCard(int selectedCardIndex);
    void DeleteCard();
    void DeselectCard();
    
private:
    enum class SceneState
    {
        BROWSING_CARDS,
        SELECTED_CARD_FOR_DELETION,
        DISSOLVING_DELETED_CARD
    };
    
    struct CardEntry
    {
        std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjects;
        std::shared_ptr<CardSoWrapper> mCardSoWrapper;
    };
    
    std::shared_ptr<scene::Scene> mScene;
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::unique_ptr<SwipeableContainer<CardEntry>> mCardContainer;
    std::unique_ptr<CardTooltipController> mCardTooltipController;
    glm::vec3 mSelectedCardInitialPosition;
    SceneState mSceneState;
    float mCoinAnimationValue;
    int mSelectedCardIndex;
    bool mTransitioning;
    bool mAnimatingCoinValue;
};

///------------------------------------------------------------------------------------------------

#endif /* StoryCardsLibrarySceneLogicManager_h */
