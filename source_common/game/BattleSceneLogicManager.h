///------------------------------------------------------------------------------------------------
///  BattleSceneLogicManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BattleSceneLogicManager_h
#define BattleSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/ISceneLogicManager.h>
#include <game/events/EventSystem.h>
#include <game/ProgressionDataRepository.h>
#include <game/SwipeableContainer.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/MathUtils.h>
#include <memory>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class AnimatedStatContainer;
class BoardState;
class GameActionEngine;
class GameRuleEngine;
class GameSerializer;
class PlayerActionGenerationEngine;

///------------------------------------------------------------------------------------------------

struct CardSoWrapper;

///------------------------------------------------------------------------------------------------

class BattleSceneLogicManager final: public events::IListener, public ISceneLogicManager
{
public:
    BattleSceneLogicManager();
    ~BattleSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    
    const BoardState& GetBoardState() const;
    GameActionEngine& GetActionEngine();
    
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetHeldCardSoWrappers() const;
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetBoardCardSoWrappers() const;
    
private:
    void InitBattleScene(std::shared_ptr<scene::Scene> scene);
    void InitHistoryScene(std::shared_ptr<scene::Scene> scene);
    
    void HandleTouchInput(const float dtMillis);
    void UpdateMiscSceneObjects(const float dtMillis);
    void OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper);
    void CreateCardHighlighter();
    void CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText, const size_t cardIndex, std::shared_ptr<scene::Scene> scene);
    void DestroyCardHighlighterAtIndex(const int index);
    void DestroyCardTooltip(std::shared_ptr<scene::Scene> scene);
    void RegisterForEvents();
    void OnApplicationMovedToBackground(const events::ApplicationMovedToBackgroundEvent&);
    void OnWindowResize(const events::WindowResizeEvent&);
    void OnLocalPlayerTurnStarted(const events::LocalPlayerTurnStarted&);
    void OnEndOfTurnCardDestruction(const events::EndOfTurnCardDestructionEvent&);
    void OnImmediateCardDestructionWithReposition(const events::ImmediateCardDestructionWithRepositionEvent&);
    void OnCardCreation(const events::CardCreationEvent& event);
    void OnCardBuffedDebuffed(const events::CardBuffedDebuffedEvent& event);
    void OnHeldCardSwap(const events::HeldCardSwapEvent& event);
    void OnNewBoardCardCreated(const events::NewBoardCardCreatedEvent& event);
    void OnLastCardPlayedFinalized(const events::LastCardPlayedFinalizedEvent& event);
    void OnHealthChangeAnimationTrigger(const events::HealthChangeAnimationTriggerEvent&);
    void OnWeightChangeAnimationTrigger(const events::WeightChangeAnimationTriggerEvent&);
    void OnBoardSideCardEffectTriggered(const events::BoardSideCardEffectTriggeredEvent&);
    void OnBoardSideCardEffectEnded(const events::BoardSideCardEffectEndedEvent&);
    void OnForceSendCardBackToPosition(const events::ForceSendCardBackToPositionEvent&);
    void OnPoisonStackChangeChangeAnimationTrigger(const events::PoisonStackChangeChangeAnimationTriggerEvent&);
    void OnCardHistoryEntryAddition(const events::CardHistoryEntryAdditionEvent&);
    void OnHistoryButtonPressed();
    glm::vec3 CalculateBoardEffectPosition(const size_t effectIndex, const size_t effectsCount, bool forRemotePlayer);
    
private:
    enum class ProspectiveBoardCardsPushState
    {
        NONE, MAKE_SPACE_FOR_NEW_CARD, REVERT_TO_ORIGINAL_POSITION
    };
    
    struct CardHistoryEntry
    {
        std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjects;
        int mCardId;
        bool mForOpponent;
        bool mIsTurnCounter;
    };
    
private:
    std::unique_ptr<BoardState> mBoardState;
    std::unique_ptr<GameActionEngine> mActionEngine;
    std::unique_ptr<GameRuleEngine> mRuleEngine;
    std::unique_ptr<GameSerializer> mGameSerializer;
    std::unique_ptr<PlayerActionGenerationEngine> mPlayerActionGenerationEngine;
    std::unique_ptr<SwipeableContainer<CardHistoryEntry>> mCardHistoryContainer;
    std::shared_ptr<scene::Scene> mActiveScene;
    std::vector<std::unique_ptr<AnimatedButton>> mBattleSceneAnimatedButtons;
    std::vector<std::vector<std::shared_ptr<scene::SceneObject>>> mActiveIndividualCardBoardEffectSceneObjects;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerHeldCardSceneObjectWrappers;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerBoardCardSceneObjectWrappers;
    std::vector<std::pair<bool, std::unique_ptr<AnimatedStatContainer>>> mAnimatedStatContainers;
    std::vector<std::shared_ptr<CardSoWrapper>> mPendingCardsToBePlayed;
    ProspectiveBoardCardsPushState mPreviousProspectiveBoardCardsPushState;
    BattleControlType mCurrentBattleControlType;
    float mSecsCardHighlighted;
    bool mShouldShowCardLocationIndicator;
    bool mCanPlayNextCard;
    bool mCanIssueNextTurnInteraction;
    bool mCanInteractWithAnyHeldCard;
};

///------------------------------------------------------------------------------------------------

#endif /* BattleSceneLogicManager_h */
