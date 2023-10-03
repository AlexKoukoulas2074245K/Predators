///------------------------------------------------------------------------------------------------
///  IGameAction.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef IGameAction_h
#define IGameAction_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>

///------------------------------------------------------------------------------------------------

class IGameAction
{
public:
    virtual ~IGameAction() = default;
    
    virtual const strutils::StringId& VGetName() const = 0;
    
    // To be called directly by the engine. This
    // needs to set the final board/game state post this action
    // (before the animations actually run) for game integrity purposes.
    virtual void VSetNewGameState() = 0;    
};

///------------------------------------------------------------------------------------------------

#endif /* IGameAction_h */
