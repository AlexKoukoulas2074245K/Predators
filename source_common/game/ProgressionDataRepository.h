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
    EVENT,
    BATTLE
};

///------------------------------------------------------------------------------------------------

enum class BattleSubSceneType
{
    BATTLE,
    WHEEL,
    CARD_SELECTION
};

///------------------------------------------------------------------------------------------------

struct QuickPlayData
{
    BattleControlType mBattleControlType;
    std::vector<int> mTopPlayerDeck;
    std::vector<int> mBotPlayerDeck;
};

///------------------------------------------------------------------------------------------------

class StoryDeserializer;
class StorySerializer;
class PersistentAccountDataSerializer;
class PersistentAccountDataDeserializer;
class ProgressionDataRepository final
{
public:
    static ProgressionDataRepository& GetInstance();
    ~ProgressionDataRepository() = default;
    
    ProgressionDataRepository(const ProgressionDataRepository&) = delete;
    ProgressionDataRepository(ProgressionDataRepository&&) = delete;
    const ProgressionDataRepository& operator = (const ProgressionDataRepository&) = delete;
    ProgressionDataRepository& operator = (ProgressionDataRepository&&) = delete;
    
    void ResetStoryData();
    void FlushStateToFile();
    
    QuickPlayData* GetQuickPlayData() const;
    void SetQuickPlayData(std::unique_ptr<QuickPlayData> quickPlayData);
    
    ValueWithDelayedDisplay<long long>& CurrencyCoins();
    ValueWithDelayedDisplay<int>& StoryCurrentHealth();
    
    BattleControlType GetNextBattleControlType() const;
    void SetNextBattleControlType(const BattleControlType nextBattleControlType);
    
    StoryMapSceneType GetCurrentStoryMapSceneType() const;
    void SetCurrentStoryMapSceneType(const StoryMapSceneType currentStoryMapSceneType);
    
    BattleSubSceneType GetCurrentBattleSubSceneType() const;
    void SetCurrentBattleSubSceneType(const BattleSubSceneType currentBattleSubSceneType);
    
    const std::vector<int>& GetCurrentStoryPlayerDeck() const;
    void SetCurrentStoryPlayerDeck(const std::vector<int>& deck);
    
    const std::vector<int>& GetNextTopPlayerDeck() const;
    void SetNextTopPlayerDeck(const std::vector<int>& deck);
    
    const std::vector<int>& GetNextBotPlayerDeck() const;
    void SetNextBotPlayerDeck(const std::vector<int>& deck);
    
    const int& GetStoryMaxHealth() const;
    void SetStoryMaxHealth(const int storyMaxHealth);
    
    const int& GetStoryMapGenerationSeed() const;
    void SetStoryMapGenerationSeed(const int storyMapGenerationSeed);
    
    const int& GetCurrentStoryMapNodeSeed() const;
    void SetCurrentStoryMapNodeSeed(const int currentStoryMapNodeSeed);
    
    StoryMap::NodeType GetCurrentStoryMapNodeType() const;
    void SetCurrentStoryMapNodeType(const StoryMap::NodeType currentStoryMapNodeType);
    
    const int& GetCurrentEventScreenIndex() const;
    void SetCurrentEventScreenIndex(const int currentEventScreenIndex);
    
    const int& GetNextBattleTopPlayerHealth() const;
    void SetNextBattleTopPlayerHealth(const int nextBattleTopPlayerHealth);
    
    const int& GetNextBattleBotPlayerHealth() const;
    void SetNextBattleBotPlayerHealth(const int nextBattleBotPlayerHealth);
    
    const int& GetNextBattleTopPlayerInitWeight() const;
    void SetNextBattleTopPlayerInitWeight(const int nextBattleTopPlayerInitWeight);
    
    const int& GetNextBattleBotPlayerInitWeight() const;
    void SetNextBattleBotPlayerInitWeight(const int nextBattleBotPlayerInitWeight);
    
    const int& GetNextBattleTopPlayerWeightLimit() const;
    void SetNextBattleTopPlayerWeightLimit(const int nextBattleTopPlayerWeightLimit);
    
    const int& GetNextBattleBotPlayerWeightLimit() const;
    void SetNextBattleBotPlayerWeightLimit(const int nextBattleBotPlayerWeightLimit);
    
    const int& GetNextStoryOpponentDamage() const;
    void SetNextStoryOpponentDamage(const int nextStoryOpponentDamage);
    
    const int& GetCurrentStorySecondsPlayed() const;
    void SetCurrentStorySecondPlayed(const int currentStorySecondsPlayed);
    
    const glm::ivec2& GetCurrentStoryMapNodeCoord() const;
    void SetCurrentStoryMapNodeCoord(const glm::ivec2& currentStoryMapNodeCoord);
    
    const StoryMap::NodeData* GetSelectedStoryMapNodeData() const;
    void SetSelectedStoryMapNodeData(const StoryMap::NodeData* selectedStoryMapNodeData);
    
    const glm::vec3& GetSelectedStoryMapNodePosition() const;
    void SetSelectedStoryMapNodePosition(const glm::vec3& selectedStoryMapNodePosition);
    
    const std::string& GetNextStoryOpponentTexturePath() const;
    void SetNextStoryOpponentTexturePath(const std::string& nextStoryOpponentTexturePath);
    
    const std::string& GetNextStoryOpponentName() const;
    void SetNextStoryOpponentName(const std::string& nextStoryOpponentName);
    
    const bool& IsCurrentlyPlayingStoryMode() const;
    void SetIsCurrentlyPlayingStoryMode(const bool isCurrentlyPlayingStoryMode);
    
private:
    ProgressionDataRepository();
    
private:
    std::unique_ptr<PersistentAccountDataDeserializer> mPersistentDataDeserializer;
    std::unique_ptr<PersistentAccountDataSerializer> mPersistentDataSerializer;
    std::unique_ptr<StoryDeserializer> mStoryDataDeserializer;
    std::unique_ptr<StorySerializer> mStoryDataSerializer;
    std::unique_ptr<QuickPlayData> mQuickPlayData;
    BattleControlType mNextBattleControlType;
    StoryMapSceneType mCurrentStoryMapSceneType;
    BattleSubSceneType mCurrentBattleSubSceneType;
    std::vector<int> mCurrentStoryPlayerDeck;
    std::vector<int> mNextTopPlayerDeck;
    std::vector<int> mNextBotPlayerDeck;
    std::string mNextStoryOpponentTexturePath;
    std::string mNextStoryOpponentName;
    glm::vec3 mSelectedStoryMapNodePosition = {};
    glm::ivec2 mCurrentStoryMapNodeCoord = game_constants::STORY_MAP_INIT_COORD;
    StoryMap::NodeType mCurrentStoryMapNodeType = StoryMap::NodeType::NORMAL_ENCOUNTER;
    const StoryMap::NodeData* mSelectedStoryMapNodeData = nullptr;
    ValueWithDelayedDisplay<int> mStoryCurrentHealth;
    ValueWithDelayedDisplay<long long> mCurrencyCoins;
    int mStoryMaxHealth = 0;
    int mStoryMapGenerationSeed = 0;
    int mCurrentStoryMapNodeSeed = 0;
    int mCurrentEventScreenIndex = 0;
    int mNextBattleTopPlayerHealth = 0;
    int mNextBattleBotPlayerHealth = 0;
    int mNextBattleTopPlayerInitWeight = 0;
    int mNextBattleBotPlayerInitWeight = 0;
    int mNextBattleTopPlayerWeightLimit = 0;
    int mNextBattleBotPlayerWeightLimit = 0;
    int mNextStoryOpponentDamage = 0;
    int mCurrentStorySecondsPlayed = 0;
    bool mIsCurrentlyPlayingStoryMode = false;
};

///------------------------------------------------------------------------------------------------

#endif /* ProgressionDataRepository_h */
