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
#include <game/BoardState.h>
#include <memory>
#include <queue>

///------------------------------------------------------------------------------------------------

class IGameAction;
class GameActionEngine final
{
public:
    enum class EngineOperationMode
    {
        ANIMATED, HEADLESS
    };
    
public:
    GameActionEngine(const EngineOperationMode operationMode);
    
    ~GameActionEngine();
    
    void Update(const float dtMillis);
    
    void AddGameAction(const strutils::StringId& actionName);
    
    const BoardState& GetBoardState() const;
    const strutils::StringId& GetActiveGameActionName() const;
    
private:
    void CreateAndPushGameAction(const strutils::StringId& actionName);
    
private:
    const EngineOperationMode mOperationMode;
    BoardState mBoardState;
    std::queue<std::unique_ptr<IGameAction>> mGameActions;
};

///------------------------------------------------------------------------------------------------

#endif /* GameActionEngine_h */
