///------------------------------------------------------------------------------------------------
///  BoardState.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BoardState_h
#define BoardState_h

///------------------------------------------------------------------------------------------------

#include <game/CardEffectComponents.h>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

enum class CardStatType
{
    DAMAGE,
    WEIGHT
};

///------------------------------------------------------------------------------------------------

using CardStatOverrides = std::unordered_map<CardStatType, int>;

///------------------------------------------------------------------------------------------------

struct BoardModifiers
{
    effects::EffectBoardModifierMask mBoardModifierMask = effects::board_modifier_masks::NONE;
    CardStatOverrides mGlobalCardStatModifiers;
};

///------------------------------------------------------------------------------------------------

struct PlayerState
{
    std::vector<int> mPlayerDeckCards;
    std::vector<int> mPlayerHeldCards;
    std::vector<int> mPlayerBoardCards;
    std::vector<CardStatOverrides> mPlayerBoardCardStatOverrides;
    std::vector<CardStatOverrides> mPlayerHeldCardStatOverrides;
    std::vector<int> mGoldenCardIds;
    BoardModifiers mBoardModifiers;
    int mPlayerHealth = 30;
    int mPlayerPoisonStack = 0;
    int mPlayerTotalWeightAmmo = 0;
    int mPlayerCurrentWeightAmmo = 0;
};

///------------------------------------------------------------------------------------------------

class BoardState
{
public:
    const std::vector<PlayerState>& GetPlayerStates() const { return mPlayerStates; }
    std::vector<PlayerState>& GetPlayerStates() { return mPlayerStates; }
    const PlayerState& GetActivePlayerState() const { return mActivePlayerIndex == -1 ? mPlayerStates.at(1) : mPlayerStates.at(mActivePlayerIndex); }
    PlayerState& GetActivePlayerState() { return mActivePlayerIndex == -1 ? mPlayerStates[1] : mPlayerStates[mActivePlayerIndex]; }
    const PlayerState& GetInactivePlayerState() const { return mActivePlayerIndex == -1 ? mPlayerStates.at(0) : mPlayerStates.at((mActivePlayerIndex + 1) % GetPlayerCount()); }
    PlayerState& GetInactivePlayerState() { return mActivePlayerIndex == -1 ? mPlayerStates[0] : mPlayerStates[(mActivePlayerIndex + 1) % GetPlayerCount()]; }
    int GetActivePlayerIndex() const { return mActivePlayerIndex; }
    int& GetActivePlayerIndex() { return mActivePlayerIndex; }
    int& GetTurnCounter() { return mTurnCounter; }
    int GetTurnCounter() const { return mTurnCounter; }
    size_t GetPlayerCount() const { return mPlayerStates.size(); }
    
private:
    std::vector<PlayerState> mPlayerStates;
    int mActivePlayerIndex = -1;
    int mTurnCounter = -1;
};

///------------------------------------------------------------------------------------------------

#endif /* BoardState_h */
