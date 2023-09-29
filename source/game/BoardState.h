///------------------------------------------------------------------------------------------------
///  BoardState.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BoardState_h
#define BoardState_h

///------------------------------------------------------------------------------------------------

#include <vector>
#include <sstream>

///------------------------------------------------------------------------------------------------

struct PlayerState
{
    std::vector<int> mPlayerHeldCards;
    std::vector<int> mPlayerBoardCards;
    int mPlayerHealth = 0;
};

///------------------------------------------------------------------------------------------------

class BoardState
{
public:
    std::string ToString() const;
    const std::vector<PlayerState>& GetPlayerStates() const { return mPlayerStates; }
    std::vector<PlayerState>& GetPlayerStates() { return mPlayerStates; }
    const PlayerState& GetActivePlayerState() const { return mPlayerStates.at(mActivePlayerIndex); }
    PlayerState& GetActivePlayerState() { return mPlayerStates[mActivePlayerIndex]; }
    size_t GetActivePlayerIndex() const { return mActivePlayerIndex; }
    size_t& GetActivePlayerIndex() { return mActivePlayerIndex; }
    size_t GetPlayerCount() const { return mPlayerStates.size(); }
    
private:
    std::vector<PlayerState> mPlayerStates;
    size_t mActivePlayerIndex = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* BoardState_h */
