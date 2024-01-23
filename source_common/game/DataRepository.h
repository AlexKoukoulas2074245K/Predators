///------------------------------------------------------------------------------------------------
///  DataRepository.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef DataRepository_h
#define DataRepository_h

///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
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
    BATTLE,
    SHOP
};

///------------------------------------------------------------------------------------------------

enum class BattleSubSceneType
{
    BATTLE,
    WHEEL,
    CARD_SELECTION
};

///------------------------------------------------------------------------------------------------

enum class CardLibraryBehaviorType
{
    NORMAL_BROWSING,
    BROWSING_FOR_DELETION
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
class DataRepository final
{
public:
    static DataRepository& GetInstance();
    ~DataRepository() = default;
    
    DataRepository(const DataRepository&) = delete;
    DataRepository(DataRepository&&) = delete;
    const DataRepository& operator = (const DataRepository&) = delete;
    DataRepository& operator = (DataRepository&&) = delete;
    
    void ResetStoryData();
    void ReloadProgressionDataFromFile();
    void FlushStateToFile();
    
    const std::unordered_map<CardStatType, int>& GetStoryPlayerCardStatModifiers() const;
    void SetStoryPlayerCardStatModifier(const CardStatType statType, int statModifier);
    
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
    
    CardLibraryBehaviorType GetCurrentCardLibraryBehaviorType() const;
    void SetCurrentCardLibraryBehaviorType(const CardLibraryBehaviorType currentCardLibraryBehaviorType);
    
    const std::vector<int>& GetUnlockedCardIds() const;
    void SetUnlockedCardIds(const std::vector<int>& unlockedCardIds);
    
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
    
    const int& GetCurrentEventIndex() const;
    void SetCurrentEventIndex(const int currentEventIndex);
        
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
    
    const std::vector<std::pair<int, int>>& GetCurrentShopBoughtProductCoordinates() const;
    void ClearShopBoughtProductCoordinates();
    void SetShopBoughtProductCoordinates(const std::vector<std::pair<int, int>>& shopBoughtProductCoordinates);
    void AddShopBoughtProductCoordinates(const std::pair<int, int>& shopBoughtProductCoordinates);
    
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
    
    const std::string& GetCloudDataDeviceNameAndTime() const;
    void SetCloudDataDeviceNameAndTime(const std::string& cloudDataDeviceNameAndTime);
    
    const bool& IsCurrentlyPlayingStoryMode() const;
    void SetIsCurrentlyPlayingStoryMode(const bool isCurrentlyPlayingStoryMode);
    
    const bool& ForeignProgressionDataFound() const;
    void SetForeignProgressionDataFound(const bool foreignProgressionDataFound);
private:
    DataRepository();
    
private:
    std::unordered_map<CardStatType, int> mStoryPlayerCardStatModifiers;
    std::unique_ptr<PersistentAccountDataDeserializer> mPersistentDataDeserializer;
    std::unique_ptr<PersistentAccountDataSerializer> mPersistentDataSerializer;
    std::unique_ptr<StoryDeserializer> mStoryDataDeserializer;
    std::unique_ptr<StorySerializer> mStoryDataSerializer;
    std::unique_ptr<QuickPlayData> mQuickPlayData;
    BattleControlType mNextBattleControlType;
    StoryMapSceneType mCurrentStoryMapSceneType;
    BattleSubSceneType mCurrentBattleSubSceneType;
    CardLibraryBehaviorType mCurrentCardLibraryBehaviorType;
    std::vector<int> mUnlockedCardIds;
    std::vector<int> mCurrentStoryPlayerDeck;
    std::vector<int> mNextTopPlayerDeck;
    std::vector<int> mNextBotPlayerDeck;
    std::vector<std::pair<int, int>> mCurrentShopBoughtProductCoordinates;
    std::string mNextStoryOpponentTexturePath;
    std::string mNextStoryOpponentName;
    std::string mCloudDataDeviceAndTime;
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
    int mCurrentEventIndex = 0;
    int mNextBattleTopPlayerHealth = 0;
    int mNextBattleBotPlayerHealth = 0;
    int mNextBattleTopPlayerInitWeight = 0;
    int mNextBattleBotPlayerInitWeight = 0;
    int mNextBattleTopPlayerWeightLimit = 0;
    int mNextBattleBotPlayerWeightLimit = 0;
    int mNextStoryOpponentDamage = 0;
    int mCurrentStorySecondsPlayed = 0;
    bool mIsCurrentlyPlayingStoryMode = false;
    bool mForeignProgressionDataFound = false;
};

///------------------------------------------------------------------------------------------------

#endif /* DataRepository_h */