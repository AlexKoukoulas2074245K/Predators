///------------------------------------------------------------------------------------------------
///  Game.h                                                                                          
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#ifndef Game_h
#define Game_h

///------------------------------------------------------------------------------------------------

#include <memory>

///------------------------------------------------------------------------------------------------

class GameActionEngine;
class Game final
{
public:
    Game(const int argc, char** argv);
    ~Game();
    
    void Init();
    void Update(const float dtMillis);
    void CreateDebugWidgets();
    void CreateDebugCards(const int cardCount);
private:
    std::unique_ptr<GameActionEngine> mActionEngine;
};

///------------------------------------------------------------------------------------------------

#endif /* Game_h */
