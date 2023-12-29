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

ValueWithDelayedDisplay<int>& ProgressionDataRepository::CurrencyCoins()
{
    return mCurrencyCoins;
}

///------------------------------------------------------------------------------------------------

ValueWithDelayedDisplay<int>& ProgressionDataRepository::StoryCurrentHealth()
{
    return mStoryCurrentHealth;
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

StoryMapSceneType ProgressionDataRepository::GetCurrentStoryMapSceneType() const
{
    return mCurrentStoryMapSceneType;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentStoryMapSceneType(const StoryMapSceneType currentStoryMapSceneType)
{
    mCurrentStoryMapSceneType = currentStoryMapSceneType;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetCurrentEventScreenIndex() const
{
    return mCurrentEventScreenIndex;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentEventScreenIndex(const int currentEventScreenIndex)
{
    mCurrentEventScreenIndex = currentEventScreenIndex;
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

const int& ProgressionDataRepository::GetStoryMapGenerationSeed() const
{
    return mStoryMapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetStoryMapGenerationSeed(const int storyMapGenerationSeed)
{
    mStoryMapGenerationSeed = storyMapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetCurrentStoryMapNodeSeed() const
{
    return mCurrentStoryMapNodeSeed;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentStoryMapNodeSeed(const int currentStoryMapNodeSeed)
{
    mCurrentStoryMapNodeSeed = currentStoryMapNodeSeed;
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

const StoryMap::NodeData* ProgressionDataRepository::GetSelectedStoryMapNodeData() const
{
    return mSelectedStoryMapNodeData;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetSelectedStoryMapNodeData(const StoryMap::NodeData* selectedStoryMapNodeData)
{
    mSelectedStoryMapNodeData = selectedStoryMapNodeData;
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
