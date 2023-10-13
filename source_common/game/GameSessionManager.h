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
    const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GetCardSoWrappers() const;
    
private:
    void HandleTouchInput();
    
private:
    std::unique_ptr<BoardState> mBoardState;
    std::unique_ptr<GameActionEngine> mActionEngine;
    std::vector<std::vector<std::shared_ptr<CardSoWrapper>>> mPlayerCardSceneObjectWrappers;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSessionManager_h */
