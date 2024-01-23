///------------------------------------------------------------------------------------------------
///  DataRepository.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/DataRepository.h>
#include <game/utils/StoryDeserializer.h>
#include <game/utils/StorySerializer.h>
#include <game/utils/PersistentAccountDataSerializer.h>
#include <game/utils/PersistentAccountDataDeserializer.h>

///------------------------------------------------------------------------------------------------

DataRepository& DataRepository::GetInstance()
{
    static DataRepository instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

DataRepository::DataRepository()
    : mStoryCurrentHealth(0)
    , mCurrencyCoins(0)
{
    mPersistentDataSerializer = std::make_unique<PersistentAccountDataSerializer>();
    mStoryDataSerializer = std::make_unique<StorySerializer>();
    
    // Persistent Account data initialization
    mUnlockedCardIds = CardDataRepository::GetInstance().GetFreshAccountUnlockedCardIds();
    mCurrencyCoins = ValueWithDelayedDisplay<long long>(0, 0, [=](const long long& newValue) { mPersistentDataSerializer->GetState()["currency_coins"] = newValue; });
    ResetStoryData();
    
    mPersistentDataDeserializer = std::make_unique<PersistentAccountDataDeserializer>(*this);
    mStoryDataDeserializer = std::make_unique<StoryDeserializer>(*this);
}

///------------------------------------------------------------------------------------------------

void DataRepository::ResetStoryData()
{
    // Stroy data initialization
    mStoryDataSerializer->GetState().clear();
    
    mStoryPlayerCardStatModifiers.clear();
    
    mStoryCurrentHealth = ValueWithDelayedDisplay<int>(game_constants::STORY_DEFAULT_MAX_HEALTH, game_constants::STORY_DEFAULT_MAX_HEALTH, [=](const int& newValue) { mStoryDataSerializer->GetState()["current_story_health"] = newValue; });
    
    mCurrentShopBoughtProductCoordinates.clear();
    mCurrentStoryPlayerDeck.clear();
    mNextTopPlayerDeck.clear();
    mNextBotPlayerDeck.clear();
    mNextStoryOpponentTexturePath.clear();
    mNextStoryOpponentName.clear();
    
    mSelectedStoryMapNodePosition = {};
    mCurrentStoryMapNodeCoord = game_constants::STORY_MAP_INIT_COORD;
    mCurrentStoryMapNodeType = StoryMap::NodeType::NORMAL_ENCOUNTER;
    mCurrentCardLibraryBehaviorType = CardLibraryBehaviorType::NORMAL_BROWSING;
    mSelectedStoryMapNodeData = nullptr;
    
    mStoryMaxHealth = game_constants::STORY_DEFAULT_MAX_HEALTH;
    mStoryMapGenerationSeed = 0;
    mCurrentStoryMapNodeSeed = 0;
    mCurrentEventScreenIndex = 0;
    mCurrentEventIndex = 0;
    mNextBattleTopPlayerHealth = 0;
    mNextBattleBotPlayerHealth = 0;
    mNextBattleTopPlayerInitWeight = 0;
    mNextBattleBotPlayerInitWeight = game_constants::BOT_PLAYER_DEFAULT_WEIGHT - 1;
    mNextBattleTopPlayerWeightLimit = 0;
    mNextBattleBotPlayerWeightLimit = 0;
    mNextStoryOpponentDamage = 0;
    mCurrentStorySecondsPlayed = 0;
    
    mIsCurrentlyPlayingStoryMode = false;
    
    SetNextBotPlayerDeck(CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::RODENTS_FAMILY_NAME));
    SetCurrentStoryPlayerDeck(CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::RODENTS_FAMILY_NAME));
}

///------------------------------------------------------------------------------------------------

void DataRepository::ReloadProgressionDataFromFile()
{
    ResetStoryData();
    mPersistentDataSerializer->GetState().clear();
    
    mPersistentDataDeserializer = std::make_unique<PersistentAccountDataDeserializer>(*this);
    mStoryDataDeserializer = std::make_unique<StoryDeserializer>(*this);
}

///------------------------------------------------------------------------------------------------

void DataRepository::FlushStateToFile()
{
    mStoryDataSerializer->FlushStateToFile();
    mPersistentDataSerializer->FlushStateToFile();
}

///------------------------------------------------------------------------------------------------

const std::unordered_map<CardStatType, int>& DataRepository::GetStoryPlayerCardStatModifiers() const
{
    return mStoryPlayerCardStatModifiers;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetStoryPlayerCardStatModifier(const CardStatType statType, int statModifier)
{
    mStoryPlayerCardStatModifiers[statType] = statModifier;
    
    nlohmann::json storyPlayerCardStatModifiersJson;
    for (auto& cardStatModifierEntry: mStoryPlayerCardStatModifiers)
    {
        storyPlayerCardStatModifiersJson[std::to_string(static_cast<int>(cardStatModifierEntry.first))] = cardStatModifierEntry.second;
    }
    mStoryDataSerializer->GetState()["story_player_card_stat_modifiers"] = storyPlayerCardStatModifiersJson;
}

///------------------------------------------------------------------------------------------------

QuickPlayData* DataRepository::GetQuickPlayData() const
{
    return mQuickPlayData.get();
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetQuickPlayData(std::unique_ptr<QuickPlayData> quickPlayData)
{
    mQuickPlayData = std::move(quickPlayData);
}

///------------------------------------------------------------------------------------------------

ValueWithDelayedDisplay<long long>& DataRepository::CurrencyCoins()
{
    return mCurrencyCoins;
}

///------------------------------------------------------------------------------------------------

ValueWithDelayedDisplay<int>& DataRepository::StoryCurrentHealth()
{
    return mStoryCurrentHealth;
}

///------------------------------------------------------------------------------------------------

BattleControlType DataRepository::GetNextBattleControlType() const
{
    return mNextBattleControlType;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleControlType(const BattleControlType nextBattleControlType)
{
    mNextBattleControlType = nextBattleControlType;
}

///------------------------------------------------------------------------------------------------

StoryMapSceneType DataRepository::GetCurrentStoryMapSceneType() const
{
    return mCurrentStoryMapSceneType;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentStoryMapSceneType(const StoryMapSceneType currentStoryMapSceneType)
{
    mCurrentStoryMapSceneType = currentStoryMapSceneType;
    mStoryDataSerializer->GetState()["current_story_map_scene_type"] = static_cast<int>(currentStoryMapSceneType);
}

///------------------------------------------------------------------------------------------------

BattleSubSceneType DataRepository::GetCurrentBattleSubSceneType() const
{
    return mCurrentBattleSubSceneType;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentBattleSubSceneType(const BattleSubSceneType currentBattleSubSceneType)
{
    mCurrentBattleSubSceneType = currentBattleSubSceneType;
    mStoryDataSerializer->GetState()["current_battle_sub_scene_type"] = static_cast<int>(mCurrentBattleSubSceneType);
}

///------------------------------------------------------------------------------------------------

CardLibraryBehaviorType DataRepository::GetCurrentCardLibraryBehaviorType() const
{
    return mCurrentCardLibraryBehaviorType;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentCardLibraryBehaviorType(const CardLibraryBehaviorType currentCardLibraryBehaviorType)
{
    mCurrentCardLibraryBehaviorType = currentCardLibraryBehaviorType;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetGamesFinishedCount() const
{
    return mGamesFinishedCount;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetGamesFinishedCount(const int gamesFinishedCount)
{
    mGamesFinishedCount = gamesFinishedCount;
    mPersistentDataSerializer->GetState()["games_finished_count"] = mGamesFinishedCount;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetCurrentEventScreenIndex() const
{
    return mCurrentEventScreenIndex;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentEventScreenIndex(const int currentEventScreenIndex)
{
    mCurrentEventScreenIndex = currentEventScreenIndex;
    mStoryDataSerializer->GetState()["current_event_screen"] = currentEventScreenIndex;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetCurrentEventIndex() const
{
    return mCurrentEventIndex;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentEventIndex(const int currentEventIndex)
{
    mCurrentEventIndex = currentEventIndex;
    mStoryDataSerializer->GetState()["current_event"] = currentEventIndex;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& DataRepository::GetUnlockedCardIds() const
{
    return mUnlockedCardIds;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetUnlockedCardIds(const std::vector<int>& unlockedCardIds)
{
    mUnlockedCardIds = unlockedCardIds;
    mPersistentDataSerializer->GetState()["unlocked_card_ids"] = mUnlockedCardIds;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& DataRepository::GetCurrentStoryPlayerDeck() const
{
    return mCurrentStoryPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentStoryPlayerDeck(const std::vector<int>& deck)
{
    mCurrentStoryPlayerDeck = deck;
    mStoryDataSerializer->GetState()["current_story_player_deck"] = deck;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& DataRepository::GetNextTopPlayerDeck() const
{
    return mNextTopPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextTopPlayerDeck(const std::vector<int>& deck)
{
    mNextTopPlayerDeck = deck;
    mStoryDataSerializer->GetState()["next_top_player_deck"] = deck;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& DataRepository::GetNextBotPlayerDeck() const
{
    return mNextBotPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBotPlayerDeck(const std::vector<int>& deck)
{
    mNextBotPlayerDeck = deck;
    mStoryDataSerializer->GetState()["next_bot_player_deck"] = deck;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetStoryMaxHealth() const
{
    return mStoryMaxHealth;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetStoryMaxHealth(const int storyMaxHealth)
{
    mStoryMaxHealth = storyMaxHealth;
    mStoryDataSerializer->GetState()["story_max_health"] = mStoryMaxHealth;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetStoryMapGenerationSeed() const
{
    return mStoryMapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetStoryMapGenerationSeed(const int storyMapGenerationSeed)
{
    mStoryMapGenerationSeed = storyMapGenerationSeed;
    mStoryDataSerializer->GetState()["story_seed"] = storyMapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetCurrentStoryMapNodeSeed() const
{
    return mCurrentStoryMapNodeSeed;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentStoryMapNodeSeed(const int currentStoryMapNodeSeed)
{
    mCurrentStoryMapNodeSeed = currentStoryMapNodeSeed;
    mStoryDataSerializer->GetState()["current_story_map_node_seed"] = currentStoryMapNodeSeed;
}

///------------------------------------------------------------------------------------------------

StoryMap::NodeType DataRepository::GetCurrentStoryMapNodeType() const
{
    return mCurrentStoryMapNodeType;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentStoryMapNodeType(const StoryMap::NodeType currentStoryMapNodeType)
{
    mCurrentStoryMapNodeType = currentStoryMapNodeType;
    mStoryDataSerializer->GetState()["current_story_map_node_type"] = static_cast<int>(mCurrentStoryMapNodeType);
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextBattleTopPlayerHealth() const
{
    return mNextBattleTopPlayerHealth;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleTopPlayerHealth(const int nextBattleTopPlayerHealth)
{
    mNextBattleTopPlayerHealth = nextBattleTopPlayerHealth;
    mStoryDataSerializer->GetState()["next_battle_top_health"] = nextBattleTopPlayerHealth;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextBattleBotPlayerHealth() const
{
    return mNextBattleBotPlayerHealth;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleBotPlayerHealth(const int nextBattleBotPlayerHealth)
{
    mNextBattleBotPlayerHealth = nextBattleBotPlayerHealth;
    mStoryDataSerializer->GetState()["next_battle_bot_health"] = nextBattleBotPlayerHealth;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextBattleTopPlayerInitWeight() const
{
    return mNextBattleTopPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleTopPlayerInitWeight(const int nextBattleTopPlayerInitWeight)
{
    mNextBattleTopPlayerInitWeight = nextBattleTopPlayerInitWeight;
    mStoryDataSerializer->GetState()["next_battle_top_init_weight"] = nextBattleTopPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextBattleBotPlayerInitWeight() const
{
    return mNextBattleBotPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleBotPlayerInitWeight(const int nextBattleBotPlayerInitWeight)
{
    mNextBattleBotPlayerInitWeight = nextBattleBotPlayerInitWeight;
    mStoryDataSerializer->GetState()["next_battle_bot_init_weight"] = nextBattleBotPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextBattleTopPlayerWeightLimit() const
{
    return mNextBattleTopPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleTopPlayerWeightLimit(const int nextBattleTopPlayerWeightLimit)
{
    mNextBattleTopPlayerWeightLimit = nextBattleTopPlayerWeightLimit;
    mStoryDataSerializer->GetState()["next_battle_top_weight_limit"] = nextBattleTopPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextBattleBotPlayerWeightLimit() const
{
    return mNextBattleBotPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextBattleBotPlayerWeightLimit(const int nextBattleBotPlayerWeightLimit)
{
    mNextBattleBotPlayerWeightLimit = nextBattleBotPlayerWeightLimit;
    mStoryDataSerializer->GetState()["next_battle_bot_weight_limit"] = nextBattleBotPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetNextStoryOpponentDamage() const
{
    return mNextStoryOpponentDamage;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextStoryOpponentDamage(const int nextStoryOpponentDamage)
{
    mNextStoryOpponentDamage = nextStoryOpponentDamage;
    mStoryDataSerializer->GetState()["next_story_opponent_damage"] = nextStoryOpponentDamage;
}

///------------------------------------------------------------------------------------------------

const int& DataRepository::GetCurrentStorySecondsPlayed() const
{
    return mCurrentStorySecondsPlayed;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentStorySecondPlayed(const int currentStorySecondsPlayed)
{
    mCurrentStorySecondsPlayed = currentStorySecondsPlayed;
    mStoryDataSerializer->GetState()["current_story_seconds_played"] = mCurrentStorySecondsPlayed;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::pair<int, int>>& DataRepository::GetCurrentShopBoughtProductCoordinates() const
{
    return mCurrentShopBoughtProductCoordinates;
}

///------------------------------------------------------------------------------------------------

void DataRepository::ClearShopBoughtProductCoordinates()
{
    mCurrentShopBoughtProductCoordinates.clear();
    mStoryDataSerializer->GetState()["current_shop_bought_product_coordinates"].clear();
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetShopBoughtProductCoordinates(const std::vector<std::pair<int, int>>& shopBoughtProductCoordinates)
{
    mCurrentShopBoughtProductCoordinates = shopBoughtProductCoordinates;
    mStoryDataSerializer->GetState()["current_shop_bought_product_coordinates"] = mCurrentShopBoughtProductCoordinates;
}

///------------------------------------------------------------------------------------------------

void DataRepository::AddShopBoughtProductCoordinates(const std::pair<int, int>& shopBoughtProductCoordinates)
{
    mCurrentShopBoughtProductCoordinates.push_back(shopBoughtProductCoordinates);
    mStoryDataSerializer->GetState()["current_shop_bought_product_coordinates"] = mCurrentShopBoughtProductCoordinates;
}

///------------------------------------------------------------------------------------------------

const glm::ivec2& DataRepository::GetCurrentStoryMapNodeCoord() const
{
    return mCurrentStoryMapNodeCoord;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCurrentStoryMapNodeCoord(const glm::ivec2& currentStoryMapNodeCoord)
{
    mCurrentStoryMapNodeCoord = currentStoryMapNodeCoord;
    
    nlohmann::json currentStoryMapCoordJson;
    currentStoryMapCoordJson["col"] = currentStoryMapNodeCoord.x;
    currentStoryMapCoordJson["row"] = currentStoryMapNodeCoord.y;
    mStoryDataSerializer->GetState()["current_story_map_node_coord"] = currentStoryMapCoordJson;
}

///------------------------------------------------------------------------------------------------

const StoryMap::NodeData* DataRepository::GetSelectedStoryMapNodeData() const
{
    return mSelectedStoryMapNodeData;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetSelectedStoryMapNodeData(const StoryMap::NodeData* selectedStoryMapNodeData)
{
    mSelectedStoryMapNodeData = selectedStoryMapNodeData;
}

///------------------------------------------------------------------------------------------------

const glm::vec3& DataRepository::GetSelectedStoryMapNodePosition() const
{
    return mSelectedStoryMapNodePosition;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetSelectedStoryMapNodePosition(const glm::vec3& selectedStoryMapNodePosition)
{
    mSelectedStoryMapNodePosition = selectedStoryMapNodePosition;
}

///------------------------------------------------------------------------------------------------

const std::string& DataRepository::GetNextStoryOpponentTexturePath() const
{
    return mNextStoryOpponentTexturePath;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextStoryOpponentTexturePath(const std::string& nextStoryOpponentTexturePath)
{
    mNextStoryOpponentTexturePath = nextStoryOpponentTexturePath;
    mStoryDataSerializer->GetState()["next_story_opponent_path"] = nextStoryOpponentTexturePath;
}

///------------------------------------------------------------------------------------------------

const std::string& DataRepository::GetNextStoryOpponentName() const
{
    return mNextStoryOpponentName;
}

///------------------------------------------------------------------------------------------------

const std::string& DataRepository::GetCloudDataDeviceNameAndTime() const
{
    return mCloudDataDeviceAndTime;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetCloudDataDeviceNameAndTime(const std::string& cloudDataDeviceNameAndTime)
{
    mCloudDataDeviceAndTime = cloudDataDeviceNameAndTime;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetNextStoryOpponentName(const std::string& nextStoryOpponentName)
{
    mNextStoryOpponentName = nextStoryOpponentName;
    mStoryDataSerializer->GetState()["next_story_opponent_name"] = nextStoryOpponentName;
}

///------------------------------------------------------------------------------------------------

const bool& DataRepository::IsCurrentlyPlayingStoryMode() const
{
    return mIsCurrentlyPlayingStoryMode;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetIsCurrentlyPlayingStoryMode(const bool isCurrentlyPlayingStoryMode)
{
    mIsCurrentlyPlayingStoryMode = isCurrentlyPlayingStoryMode;
}

///------------------------------------------------------------------------------------------------

const bool& DataRepository::ForeignProgressionDataFound() const
{
    return mForeignProgressionDataFound;
}

///------------------------------------------------------------------------------------------------

void DataRepository::SetForeignProgressionDataFound(const bool foreignProgressionDataFound)
{
    mForeignProgressionDataFound = foreignProgressionDataFound;
}

///------------------------------------------------------------------------------------------------
