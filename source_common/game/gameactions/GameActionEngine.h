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
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class BoardState;
class IGameAction;
class GameSessionManager;
class GameSerializer;
class GameActionEngine final
{
public:
    enum class EngineOperationMode
    {
        ANIMATED, HEADLESS
    };
    
    using ExtraActionParams = std::unordered_map<std::string, std::string>;
    
public:
    GameActionEngine(const EngineOperationMode operationMode, const int gameSeed, BoardState* boardState, GameSessionManager* gameSessionManager, GameSerializer* gameSerializer);
    
    ~GameActionEngine();
    
    void Update(const float dtMillis);
    
    void AddGameAction(const strutils::StringId& actionName, const ExtraActionParams extraActionParams = {});
    
    void SetLoggingActionTransitions(const bool logActionTransitions);
    
    const strutils::StringId& GetActiveGameActionName() const;
    bool LoggingActionTransitions() const;
    
private:
    void CreateAndPushGameAction(const strutils::StringId& actionName, const ExtraActionParams& extraActionParams);
    void LogActionTransition(const std::string& actionTransition);
    void ReadjustActionQueue(const size_t sizeBeforeNewState);
    
private:
    const EngineOperationMode mOperationMode;
    const int mGameSeed;
    BoardState* mBoardState;
    GameSessionManager* mGameSessionManager;
    GameSerializer* mGameSerializer;
    std::queue<std::unique_ptr<IGameAction>> mGameActions;
    bool mActiveActionHasSetState;
    bool mLoggingActionTransitions;
};

///------------------------------------------------------------------------------------------------

#endif /* GameActionEngine_h */
