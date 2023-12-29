///------------------------------------------------------------------------------------------------
///  ProgressionDataRepository.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ProgressionDataRepository_h
#define ProgressionDataRepository_h

///------------------------------------------------------------------------------------------------

#include <game/GameConstants.h>
#include <game/StoryMap.h>
#include <game/utils/ValueWithDelayedDisplay.h>
#include <engine/utils/MathUtils.h>
#include <vector>

///------------------------------------------------------------------------------------------------

enum class BattleControlType
{
    REPLAY,
    AI_TOP_BOT,
    AI_TOP_ONLY
};

///------------------------------------------------------------------------------------------------

enum class StoryMapSceneType
{
    STORY_MAP,
    EVENT
};

///------------------------------------------------------------------------------------------------

class ProgressionDataRepository final
{
public:
    static ProgressionDataRepository& GetInstance();
    ~ProgressionDataRepository() = default;
    
    ProgressionDataRepository(const ProgressionDataRepository&) = delete;
    ProgressionDataRepository(ProgressionDataRepository&&) = delete;
    const ProgressionDataRepository& operator = (const ProgressionDataRepository&) = delete;
    ProgressionDataRepository& operator = (ProgressionDataRepository&&) = delete;
    
    ValueWithDelayedDisplay<int>& CurrencyCoins();
    
    ValueWithDelayedDisplay<int>& StoryCurrentHealth();
    
    BattleControlType GetNextBattleControlType() const;
    void SetNextBattleControlType(const BattleControlType nextBattleControlType);
    
    StoryMapSceneType GetCurrentStoryMapSceneType() const;
    void SetCurrentStoryMapSceneType(const StoryMapSceneType currentStoryMapSceneType);
    
    const std::vector<int>& GetNextTopPlayerDeck() const;
    void SetNextTopPlayerDeck(const std::vector<int>& deck);
    
    const std::vector<int>& GetNextBotPlayerDeck() const;
    void SetNextBotPlayerDeck(const std::vector<int>& deck);
    
    
    const int& GetStoryMapGenerationSeed() const;
    void SetStoryMapGenerationSeed(const int storyMapGenerationSeed);
    
    const int& GetCurrentStoryMapNodeSeed() const;
    void SetCurrentStoryMapNodeSeed(const int currentStoryMapNodeSeed);
    
    const int& GetCurrentEventScreenIndex() const;
    void SetCurrentEventScreenIndex(const int currentEventScreenIndex);
    
    const glm::ivec2& GetCurrentStoryMapNodeCoord() const;
    void SetCurrentStoryMapNodeCoord(const glm::ivec2& currentStoryMapNodeCoord);
    
    const StoryMap::NodeData* GetSelectedStoryMapNodeData() const;
    void SetSelectedStoryMapNodeData(const StoryMap::NodeData* selectedStoryMapNodeData);
    
    const glm::vec3& GetSelectedStoryMapNodePosition() const;
    void SetSelectedStoryMapNodePosition(const glm::vec3& selectedStoryMapNodePosition);
    
private:
    ProgressionDataRepository() = default;
    
private:
    BattleControlType mNextBattleControlType;
    StoryMapSceneType mCurrentStoryMapSceneType;
    std::vector<int> mNextTopPlayerDeck;
    std::vector<int> mNextBotPlayerDeck;
    glm::vec3 mSelectedStoryMapNodePosition = {};
    glm::ivec2 mCurrentStoryMapNodeCoord = game_constants::STORY_MAP_INIT_COORD;
    const StoryMap::NodeData* mSelectedStoryMapNodeData = nullptr;
    ValueWithDelayedDisplay<int> mStoryCurrentHealth = 0;
    ValueWithDelayedDisplay<int> mCurrencyCoins = 0;
    int mStoryMapGenerationSeed = 0;
    int mCurrentStoryMapNodeSeed = 0;
    int mCurrentEventScreenIndex = 0;
};

///------------------------------------------------------------------------------------------------

#endif /* ProgressionDataRepository_h */
