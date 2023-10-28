///------------------------------------------------------------------------------------------------
///  RemotePlayerActionEngine.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef RemotePlayerActionEngine_h
#define RemotePlayerActionEngine_h

///------------------------------------------------------------------------------------------------

class GameRuleEngine;
class GameActionEngine;
class BoardState;
class RemotePlayerActionEngine final
{
public:
    RemotePlayerActionEngine(GameRuleEngine* gameRuleEngine, GameActionEngine* gameActionEngine);
    
    void DecideAndPushNextActions(BoardState* currentBoardState);
    
private:
    GameRuleEngine* mGameRuleEngine;
    GameActionEngine* mGameActionEngine;
};

///------------------------------------------------------------------------------------------------

#endif /* RemotePlayerActionEngine_h */
