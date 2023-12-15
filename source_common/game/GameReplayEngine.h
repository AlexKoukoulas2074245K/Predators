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
    GameReplayEngine(const std::string& filenameNoExtension);
    
    int GetGameFileSeed() const;
    const std::vector<int>& GetTopPlayerDeck() const;
    const std::vector<int>& GetBotPlayerDeck() const;
    
    void ReplayActions(GameActionEngine* gameActionEngine);
    
private:
    int mGameFileSeed;
    std::vector<int> mTopPlayerDeck;
    std::vector<int> mBotPlayerDeck;
};

///------------------------------------------------------------------------------------------------

#endif /* GameReplayEngine_h */
