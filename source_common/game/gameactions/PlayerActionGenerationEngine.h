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
struct CardData;
class PlayerActionGenerationEngine final
{
public:
    PlayerActionGenerationEngine(GameRuleEngine* gameRuleEngine, GameActionEngine* gameActionEngine);
    
    void DecideAndPushNextActions(BoardState* currentBoardState);

private:
    bool ShouldWaitForFurtherActionsAfterPlayingCard(const CardData& cardData) const;
    
private:
    GameRuleEngine* mGameRuleEngine;
    GameActionEngine* mGameActionEngine;
};

///------------------------------------------------------------------------------------------------

#endif /* PlayerActionGenerationEngine_h */
