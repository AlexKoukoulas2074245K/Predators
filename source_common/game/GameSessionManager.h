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

class AnimatedStatCrystal;
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
    void HandleTouchInput();
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
    void OnCardDestruction(const events::CardDestructionEvent&);
    void OnCardDestructionWithReposition(const events::CardDestructionWithRepositionEvent&);
    void OnCardCreation(const events::CardCreationEvent& event);
    void OnCardBuffedDebuffed(const events::CardBuffedDebuffedEvent& event);
    void OnHeldCardSwap(const events::HeldCardSwapEvent& event);
    void OnLastCardPlayedFinalized(const events::LastCardPlayedFinalizedEvent& event);
    void OnHealthChangeAnimationTriggerEvent(const events::HealthChangeAnimationTriggerEvent&);
    void OnWeightChangeAnimationTriggerEvent(const events::WeightChangeAnimationTriggerEvent&);
    void OnBoardSideCardEffectTriggeredEvent(const events::BoardSideCardEffectTriggeredEvent&);
    void OnBoardSideCardEffectEndedEvent(const events::BoardSideCardEffectEndedEvent&);

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
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerHeldCardSceneObjectWrappers;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerBoardCardSceneObjectWrappers;
    std::vector<std::pair<bool, std::unique_ptr<AnimatedStatCrystal>>> mStatCrystals;
    std::vector<std::shared_ptr<CardSoWrapper>> mPendingCardsToBePlayed;
    ProspectiveBoardCardsPushState mPreviousProspectiveBoardCardsPushState;
    bool mShouldShowCardLocationIndicator;
    bool mCanPlayNextCard;
    bool mCanIssueNextTurnInteraction;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSessionManager_h */
