///------------------------------------------------------------------------------------------------
///  GameActionEngine.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameActionEngine_h
#define GameActionEngine_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <memory>
#include <queue>

///------------------------------------------------------------------------------------------------

class BoardState;
class IGameAction;
class GameSessionManager;
class GameActionEngine final
{
public:
    enum class EngineOperationMode
    {
        ANIMATED, HEADLESS
    };
    
public:
    GameActionEngine(const EngineOperationMode operationMode, const int gameSeed, BoardState* boardState, GameSessionManager* gameSessionManager);
    
    ~GameActionEngine();
    
    void Update(const float dtMillis);
    
    void AddGameAction(const strutils::StringId& actionName);
    
    void SetLoggingActionTransitions(const bool logActionTransitions);
    
    const strutils::StringId& GetActiveGameActionName() const;
    bool LoggingActionTransitions() const;
    
private:
    void CreateAndPushGameAction(const strutils::StringId& actionName);
    void LogActionTransition(const std::string& actionTransition);
    
private:
    const EngineOperationMode mOperationMode;
    const int mGameSeed;
    BoardState* mBoardState;
    GameSessionManager* mGameSessionManager;
    std::queue<std::unique_ptr<IGameAction>> mGameActions;
    bool mActiveActionHasSetState;
    bool mLoggingActionTransitions;
};

///------------------------------------------------------------------------------------------------

#endif /* GameActionEngine_h */
