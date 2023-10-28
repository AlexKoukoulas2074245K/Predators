///------------------------------------------------------------------------------------------------
///  PlayerActionGenerationEngine.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef PlayerActionGenerationEngine_h
#define PlayerActionGenerationEngine_h

///------------------------------------------------------------------------------------------------

class GameRuleEngine;
class GameActionEngine;
class BoardState;
class PlayerActionGenerationEngine final
{
public:
    PlayerActionGenerationEngine(GameRuleEngine* gameRuleEngine, GameActionEngine* gameActionEngine);
    
    void DecideAndPushNextActions(BoardState* currentBoardState);
    
private:
    GameRuleEngine* mGameRuleEngine;
    GameActionEngine* mGameActionEngine;
};

///------------------------------------------------------------------------------------------------

#endif /* PlayerActionGenerationEngine_h */
