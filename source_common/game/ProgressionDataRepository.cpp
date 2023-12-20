///------------------------------------------------------------------------------------------------
///  ProgressionDataRepository.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/ProgressionDataRepository.h>

///------------------------------------------------------------------------------------------------

ProgressionDataRepository& ProgressionDataRepository::GetInstance()
{
    static ProgressionDataRepository instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

BattleControlType ProgressionDataRepository::GetNextBattleControlType() const
{
    return mNextBattleControlType;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleControlType(const BattleControlType nextBattleControlType)
{
    mNextBattleControlType = nextBattleControlType;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& ProgressionDataRepository::GetNextTopPlayerDeck() const
{
    return mNextTopPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextTopPlayerDeck(const std::vector<int>& deck)
{
    mNextTopPlayerDeck = deck;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& ProgressionDataRepository::GetNextBotPlayerDeck() const
{
    return mNextBotPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBotPlayerDeck(const std::vector<int>& deck)
{
    mNextBotPlayerDeck = deck;
}

///------------------------------------------------------------------------------------------------

int ProgressionDataRepository::GetCurrencyCoins() const
{
    return mCurrencyCoins;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrencyCoins(const int currencyCoins)
{
    mCurrencyCoins = currencyCoins;
}

///------------------------------------------------------------------------------------------------

const glm::ivec2& ProgressionDataRepository::GetCurrentStoryMapNodeCoord() const
{
    return mCurrentStoryMapNodeCoord;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentStoryMapNodeCoord(const glm::ivec2& currentStoryMapNodeCoord)
{
    mCurrentStoryMapNodeCoord = currentStoryMapNodeCoord;
}

///------------------------------------------------------------------------------------------------

const glm::vec3& ProgressionDataRepository::GetSelectedStoryMapNodePosition() const
{
    return mSelectedStoryMapNodePosition;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetSelectedStoryMapNodePosition(const glm::vec3& selectedStoryMapNodePosition)
{
    mSelectedStoryMapNodePosition = selectedStoryMapNodePosition;
}

///------------------------------------------------------------------------------------------------
