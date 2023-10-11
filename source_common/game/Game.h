///------------------------------------------------------------------------------------------------
///  Game.h                                                                                          
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#ifndef Game_h
#define Game_h

///------------------------------------------------------------------------------------------------

#include <game/GameSessionManager.h>
#include <memory>

///------------------------------------------------------------------------------------------------

class Game final
{
public:
    Game(const int argc, char** argv);
    ~Game();
    
    void Init();
    void Update(const float dtMillis);
    void CreateDebugWidgets();
    
private:
    GameSessionManager mGameSessionManager;
};

///------------------------------------------------------------------------------------------------

#endif /* Game_h */
