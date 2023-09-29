///------------------------------------------------------------------------------------------------
///  BoardState.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/BoardState.h>
#include <sstream>

///------------------------------------------------------------------------------------------------

std::string BoardState::ToString() const
{
    std::stringstream ss;
    ss << "\n------------------------------------------------------------------------------\n";
    ss << "ACTV " << mActivePlayerIndex << "\n";
    ss << "------------------------------------------------------------------------------\n";
    ss << "HAND 0:  ";
    ss << "(";
    if (mPlayerStates.size() > 0)
    {
        for (int i = 0; i < mPlayerStates[0].mPlayerHeldCards.size(); ++i)
        {
            if (i != 0)
            {
                ss << ", ";
            }
            ss << mPlayerStates[0].mPlayerHeldCards[i];
        }
    }
    ss << ")\n";
    ss << "------------------------------------------------------------------------------\n";
    ss << "BOARD 0:  ";
    if (mPlayerStates.size() > 0)
    {
        for (int i = 0; i < mPlayerStates[0].mPlayerBoardCards.size(); ++i)
        {
            if (i != 0)
            {
                ss << ", ";
            }
            ss << mPlayerStates[0].mPlayerBoardCards[i];
        }
    }
    ss << "\n------------------------------------------------------------------------------\n";
    ss << "BOARD 1:  ";
    if (mPlayerStates.size() > 1)
    {
        for (int i = 0; i < mPlayerStates[1].mPlayerBoardCards.size(); ++i)
        {
            if (i != 0)
            {
                ss << ", ";
            }
            ss << mPlayerStates[1].mPlayerBoardCards[i];
        }
    }
    ss << "\n------------------------------------------------------------------------------\n";
    ss << "HAND 1:  ";
    ss << "(";
    if (mPlayerStates.size() > 1)
    {
        for (int i = 0; i < mPlayerStates[1].mPlayerHeldCards.size(); ++i)
        {
            if (i != 0)
            {
                ss << ", ";
            }
            ss << mPlayerStates[1].mPlayerHeldCards[i];
        }
    }
    ss << ")\n";
    ss << "------------------------------------------------------------------------------\n";
    
    return ss.str();
}

///------------------------------------------------------------------------------------------------

