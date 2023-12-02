///------------------------------------------------------------------------------------------------
///  GameSessionManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameSessionManager_h
#define GameSessionManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
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

class GameSessionManager final: public events::IListener
{
public:
    GameSessionManager();
    ~GameSessionManager();
    
    void InitGameSession();
    void Update(const float dtMillis);
    
    const BoardState& GetBoardState() const;
    GameActionEngine& GetActionEngine();
    
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetHeldCardSoWrappers() const;
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetBoardCardSoWrappers() const;
    
    int GetLastPlayedCardIndex() const;
    
private:
    void HandleTouchInput(const float dtMillis);
    void UpdateMiscSceneObjects(const float dtMillis);
    void OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper);
    void CreateCardHighlighter();
    void CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText, const size_t cardIndex);
    void DestroyCardHighlighterAtIndex(const int index);
    void DestroyCardTooltip();
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
    void OnHistoryButtonPressed();
    glm::vec3 CalculateBoardEffectPosition(const size_t effectIndex, const size_t effectsCount, bool forRemotePlayer);
    
private:
    enum class ProspectiveBoardCardsPushState
    {
        NONE, MAKE_SPACE_FOR_NEW_CARD, REVERT_TO_ORIGINAL_POSITION
    };
    
private:
    std::unique_ptr<BoardState> mBoardState;
    std::unique_ptr<GameActionEngine> mActionEngine;
    std::unique_ptr<GameRuleEngine> mRuleEngine;
    std::unique_ptr<GameSerializer> mGameSerializer;
    std::unique_ptr<PlayerActionGenerationEngine> mPlayerActionGenerationEngine;
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::vector<std::vector<std::shared_ptr<scene::SceneObject>>> mActiveIndividualCardBoardEffectSceneObjects;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerHeldCardSceneObjectWrappers;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerBoardCardSceneObjectWrappers;
    std::vector<std::pair<bool, std::unique_ptr<AnimatedStatContainer>>> mAnimatedStatContainers;
    std::vector<std::shared_ptr<CardSoWrapper>> mPendingCardsToBePlayed;
    ProspectiveBoardCardsPushState mPreviousProspectiveBoardCardsPushState;
    float mSecsCardHighlighted;
    bool mShouldShowCardLocationIndicator;
    bool mCanPlayNextCard;
    bool mCanIssueNextTurnInteraction;
    bool mCanInteractWithAnyHeldCard;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSessionManager_h */
