///------------------------------------------------------------------------------------------------
///  GameSessionManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameSessionManager_h
#define GameSessionManager_h

///------------------------------------------------------------------------------------------------

#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

class BoardState;
class GameActionEngine;

///------------------------------------------------------------------------------------------------

struct CardSoWrapper;

///------------------------------------------------------------------------------------------------

class GameSessionManager final
{
public:
    GameSessionManager();
    ~GameSessionManager();
    
    void InitGameSession();
    void Update(const float dtMillis);
    
    const BoardState& GetBoardState() const;
    GameActionEngine& GetActionEngine();
    
    void OnCardCreation(std::shared_ptr<CardSoWrapper>, const bool forOpponentPlayer);
    void OnLastCardPlayedFinalized();
    
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetHeldCardSoWrappers() const;
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetBoardCardSoWrappers() const;
    
    std::shared_ptr<CardSoWrapper> GetLastPlayedCardSceneObjectWrapper() const;
    int GetLastPlayedCardIndex() const;
    
private:
    void HandleTouchInput(const float dtMillis);
    void OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper);
    
private:
    std::unique_ptr<BoardState> mBoardState;
    std::unique_ptr<GameActionEngine> mActionEngine;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerHeldCardSceneObjectWrappers;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerBoardCardSceneObjectWrappers;
    std::shared_ptr<CardSoWrapper> mLastPlayedCardSoWrapper;
    int mLastPlayedCardIndex;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSessionManager_h */
