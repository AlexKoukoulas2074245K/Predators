///------------------------------------------------------------------------------------------------
///  BattleDeserializer.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BattleDeserializer_h
#define BattleDeserializer_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <engine/utils/BaseDataFileDeserializer.h>

///------------------------------------------------------------------------------------------------

class GameActionEngine;
class BattleDeserializer final: public serial::BaseDataFileDeserializer
{
public:
    BattleDeserializer();
    
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

#endif /* BattleDeserializer_h */
