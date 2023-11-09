///------------------------------------------------------------------------------------------------
///  Events.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Events_h
#define Events_h

///------------------------------------------------------------------------------------------------

#include <game/Cards.h>

///------------------------------------------------------------------------------------------------

namespace events
{

///------------------------------------------------------------------------------------------------

class ApplicationMovedToBackgroundEvent final
{
    
};

///------------------------------------------------------------------------------------------------

class BoardCardDestructionEvent final
{
public:
    BoardCardDestructionEvent(const int cardIndex, const bool forRemotePlayer)
        : mCardIndex(cardIndex)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const int mCardIndex;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class BoardCardDestructionWithRepositionEvent final
{
public:
    BoardCardDestructionWithRepositionEvent(const int cardIndex, const bool forRemotePlayer)
        : mCardIndex(cardIndex)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const int mCardIndex;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class CardCreationEvent final
{
public:
    CardCreationEvent(std::shared_ptr<CardSoWrapper> cardSoWrapper, const bool forRemotePlayer)
        : mCardSoWrapper(cardSoWrapper)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const std::shared_ptr<CardSoWrapper> mCardSoWrapper;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class CardBuffedEvent final
{
public:
    CardBuffedEvent(const int cardIndex, const bool boardCard, const bool forRemotePlayer)
        : mCardIdex(cardIndex)
        , mBoardCard(boardCard)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const int mCardIdex;
    const bool mBoardCard;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class HeldCardSwapEvent final
{
public:
    HeldCardSwapEvent(const std::shared_ptr<CardSoWrapper> cardSoWrapper, const int cardIndex, const bool forRemotePlayer)
        : mCardSoWrapper(cardSoWrapper)
        , mCardIndex(cardIndex)
        , mForRemotePlayer(forRemotePlayer)
    {
    }
    
    const std::shared_ptr<CardSoWrapper> mCardSoWrapper;
    const int mCardIndex;
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class LastCardPlayedFinalizedEvent final
{
public:
    LastCardPlayedFinalizedEvent(const int cardIndex)
        : mCardIndex(cardIndex)
    {
    }
    
    const int mCardIndex;
};

///------------------------------------------------------------------------------------------------

class WeightChangeAnimationTriggerEvent final
{
public:
    WeightChangeAnimationTriggerEvent(const bool forRemotePlayer)
        : mForRemotePlayer(forRemotePlayer)
    {
        
    }
    
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

class HealthChangeAnimationTriggerEvent final
{
public:
    HealthChangeAnimationTriggerEvent(const bool forRemotePlayer)
        : mForRemotePlayer(forRemotePlayer)
    {
        
    }
    
    const bool mForRemotePlayer;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Events_h */
