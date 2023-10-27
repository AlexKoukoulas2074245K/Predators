///------------------------------------------------------------------------------------------------
///  GameReplayEngine.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameReplayEngine_h
#define GameReplayEngine_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>

///------------------------------------------------------------------------------------------------

class GameActionEngine;
class GameReplayEngine final
{
public:
    void ReplayActionsFromGameFile(const std::string& filename, GameActionEngine* gameActionEngine);
};

///------------------------------------------------------------------------------------------------

#endif /* GameReplayEngine_h */
