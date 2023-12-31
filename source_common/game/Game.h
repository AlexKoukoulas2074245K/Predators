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
#include <game/events/EventSystem.h>

///------------------------------------------------------------------------------------------------

class GameSceneTransitionManager;
class Game final
{
public:
    Game(const int argc, char** argv);
    ~Game();
    
    void Init();
    void Update(const float dtMillis);
    void ApplicationMovedToBackground();
    void WindowResize();
    void OnOneSecondElapsed();
    void CreateDebugWidgets();
    
    std::unique_ptr<GameSceneTransitionManager> mGameSceneTransitionManager;
    std::unique_ptr<events::IListener> mSceneChangeEventListener;
    std::unique_ptr<events::IListener> mPopModalSceneEventListener;
};

///------------------------------------------------------------------------------------------------

#endif /* Game_h */
