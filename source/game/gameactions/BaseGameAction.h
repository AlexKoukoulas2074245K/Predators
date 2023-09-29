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

///------------------------------------------------------------------------------------------------

class BaseGameAction: public IGameAction
{
    friend class GameActionEngine;
    
public:
    virtual ~BaseGameAction() = default;
    
    const strutils::StringId& VGetName() const override { return mName; }
    
protected:
    void SetName(const strutils::StringId& name) { mName = name; }
    void SetBoardState(BoardState* boardState) { mBoardState = boardState; }
    
protected:
    strutils::StringId mName = strutils::StringId();
    BoardState* mBoardState = nullptr;
};

///------------------------------------------------------------------------------------------------

#endif /* BaseGameAction_h */
