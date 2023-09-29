///------------------------------------------------------------------------------------------------
///  GameActionFactory.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameActionFactory_h
#define GameActionFactory_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class BaseGameAction;
class GameActionFactory final
{
    friend class GameActionEngine;
    
private:
    GameActionFactory() = delete;
    
    static std::unique_ptr<BaseGameAction> CreateGameAction(const strutils::StringId& actionName);
};

///------------------------------------------------------------------------------------------------

#endif /* GameActionFactory_h */
