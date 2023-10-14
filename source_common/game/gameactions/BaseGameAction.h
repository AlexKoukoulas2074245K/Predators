///------------------------------------------------------------------------------------------------
///  BaseGameAction.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BaseGameAction_h
#define BaseGameAction_h

///------------------------------------------------------------------------------------------------

#include <game/gameactions/IGameAction.h>
#include <game/BoardState.h>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

class GameSessionManager;
class BaseGameAction: public IGameAction
{
    friend class GameActionEngine;
    
public:
    virtual ~BaseGameAction() = default;
    
    const strutils::StringId& VGetName() const override { return mName; }
    
protected:
    void SetName(const strutils::StringId& name) { mName = name; }
    void SetDependencies(BoardState* boardState, GameSessionManager* gameSessionManager) { mBoardState = boardState; mGameSessionManager = gameSessionManager; }
    void SetExtraActionParams(const std::unordered_map<std::string, std::string>& extraActionParams) { mExtraActionParams = extraActionParams; };
    
protected:
    std::unordered_map<std::string, std::string> mExtraActionParams;
    strutils::StringId mName = strutils::StringId();
    BoardState* mBoardState = nullptr;
    GameSessionManager* mGameSessionManager = nullptr;
};

///------------------------------------------------------------------------------------------------

#endif /* BaseGameAction_h */
