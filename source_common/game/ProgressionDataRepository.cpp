///------------------------------------------------------------------------------------------------
///  ProgressionDataRepository.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/ProgressionDataRepository.h>
#include <game/utils/StoryDeserializer.h>
#include <game/utils/StorySerializer.h>
#include <game/utils/PersistentAccountDataSerializer.h>
#include <game/utils/PersistentAccountDataDeserializer.h>

///------------------------------------------------------------------------------------------------

ProgressionDataRepository& ProgressionDataRepository::GetInstance()
{
    static ProgressionDataRepository instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

ProgressionDataRepository::ProgressionDataRepository()
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

void ProgressionDataRepository::ResetStoryData()
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

void ProgressionDataRepository::FlushStateToFile()
{
    mStoryDataSerializer->FlushStateToFile();
    mPersistentDataSerializer->FlushStateToFile();
}

///------------------------------------------------------------------------------------------------

const std::unordered_map<CardStatType, int>& ProgressionDataRepository::GetStoryPlayerCardStatModifiers() const
{
    return mStoryPlayerCardStatModifiers;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetStoryPlayerCardStatModifier(const CardStatType statType, int statModifier)
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

QuickPlayData* ProgressionDataRepository::GetQuickPlayData() const
{
    return mQuickPlayData.get();
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetQuickPlayData(std::unique_ptr<QuickPlayData> quickPlayData)
{
    mQuickPlayData = std::move(quickPlayData);
}

///------------------------------------------------------------------------------------------------

ValueWithDelayedDisplay<long long>& ProgressionDataRepository::CurrencyCoins()
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
    mStoryDataSerializer->GetState()["current_story_map_scene_type"] = static_cast<int>(currentStoryMapSceneType);
}

///------------------------------------------------------------------------------------------------

BattleSubSceneType ProgressionDataRepository::GetCurrentBattleSubSceneType() const
{
    return mCurrentBattleSubSceneType;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentBattleSubSceneType(const BattleSubSceneType currentBattleSubSceneType)
{
    mCurrentBattleSubSceneType = currentBattleSubSceneType;
    mStoryDataSerializer->GetState()["current_battle_sub_scene_type"] = static_cast<int>(mCurrentBattleSubSceneType);
}

///------------------------------------------------------------------------------------------------

CardLibraryBehaviorType ProgressionDataRepository::GetCurrentCardLibraryBehaviorType() const
{
    return mCurrentCardLibraryBehaviorType;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentCardLibraryBehaviorType(const CardLibraryBehaviorType currentCardLibraryBehaviorType)
{
    mCurrentCardLibraryBehaviorType = currentCardLibraryBehaviorType;
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
    mStoryDataSerializer->GetState()["current_event_screen"] = currentEventScreenIndex;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetCurrentEventIndex() const
{
    return mCurrentEventIndex;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentEventIndex(const int currentEventIndex)
{
    mCurrentEventIndex = currentEventIndex;
    mStoryDataSerializer->GetState()["current_event"] = currentEventIndex;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& ProgressionDataRepository::GetUnlockedCardIds() const
{
    return mUnlockedCardIds;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetUnlockedCardIds(const std::vector<int>& unlockedCardIds)
{
    mUnlockedCardIds = unlockedCardIds;
    mPersistentDataSerializer->GetState()["unlocked_card_ids"] = mUnlockedCardIds;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& ProgressionDataRepository::GetCurrentStoryPlayerDeck() const
{
    return mCurrentStoryPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentStoryPlayerDeck(const std::vector<int>& deck)
{
    mCurrentStoryPlayerDeck = deck;
    mStoryDataSerializer->GetState()["current_story_player_deck"] = deck;
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
    mStoryDataSerializer->GetState()["next_top_player_deck"] = deck;
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
    mStoryDataSerializer->GetState()["next_bot_player_deck"] = deck;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetStoryMaxHealth() const
{
    return mStoryMaxHealth;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetStoryMaxHealth(const int storyMaxHealth)
{
    mStoryMaxHealth = storyMaxHealth;
    mStoryDataSerializer->GetState()["story_max_health"] = mStoryMaxHealth;
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
    mStoryDataSerializer->GetState()["story_seed"] = storyMapGenerationSeed;
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
    mStoryDataSerializer->GetState()["current_story_map_node_seed"] = currentStoryMapNodeSeed;
}

///------------------------------------------------------------------------------------------------

StoryMap::NodeType ProgressionDataRepository::GetCurrentStoryMapNodeType() const
{
    return mCurrentStoryMapNodeType;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentStoryMapNodeType(const StoryMap::NodeType currentStoryMapNodeType)
{
    mCurrentStoryMapNodeType = currentStoryMapNodeType;
    mStoryDataSerializer->GetState()["current_story_map_node_type"] = static_cast<int>(mCurrentStoryMapNodeType);
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextBattleTopPlayerHealth() const
{
    return mNextBattleTopPlayerHealth;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleTopPlayerHealth(const int nextBattleTopPlayerHealth)
{
    mNextBattleTopPlayerHealth = nextBattleTopPlayerHealth;
    mStoryDataSerializer->GetState()["next_battle_top_health"] = nextBattleTopPlayerHealth;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextBattleBotPlayerHealth() const
{
    return mNextBattleBotPlayerHealth;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleBotPlayerHealth(const int nextBattleBotPlayerHealth)
{
    mNextBattleBotPlayerHealth = nextBattleBotPlayerHealth;
    mStoryDataSerializer->GetState()["next_battle_bot_health"] = nextBattleBotPlayerHealth;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextBattleTopPlayerInitWeight() const
{
    return mNextBattleTopPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleTopPlayerInitWeight(const int nextBattleTopPlayerInitWeight)
{
    mNextBattleTopPlayerInitWeight = nextBattleTopPlayerInitWeight;
    mStoryDataSerializer->GetState()["next_battle_top_init_weight"] = nextBattleTopPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextBattleBotPlayerInitWeight() const
{
    return mNextBattleBotPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleBotPlayerInitWeight(const int nextBattleBotPlayerInitWeight)
{
    mNextBattleBotPlayerInitWeight = nextBattleBotPlayerInitWeight;
    mStoryDataSerializer->GetState()["next_battle_bot_init_weight"] = nextBattleBotPlayerInitWeight;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextBattleTopPlayerWeightLimit() const
{
    return mNextBattleTopPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleTopPlayerWeightLimit(const int nextBattleTopPlayerWeightLimit)
{
    mNextBattleTopPlayerWeightLimit = nextBattleTopPlayerWeightLimit;
    mStoryDataSerializer->GetState()["next_battle_top_weight_limit"] = nextBattleTopPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextBattleBotPlayerWeightLimit() const
{
    return mNextBattleBotPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextBattleBotPlayerWeightLimit(const int nextBattleBotPlayerWeightLimit)
{
    mNextBattleBotPlayerWeightLimit = nextBattleBotPlayerWeightLimit;
    mStoryDataSerializer->GetState()["next_battle_bot_weight_limit"] = nextBattleBotPlayerWeightLimit;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetNextStoryOpponentDamage() const
{
    return mNextStoryOpponentDamage;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextStoryOpponentDamage(const int nextStoryOpponentDamage)
{
    mNextStoryOpponentDamage = nextStoryOpponentDamage;
    mStoryDataSerializer->GetState()["next_story_opponent_damage"] = nextStoryOpponentDamage;
}

///------------------------------------------------------------------------------------------------

const int& ProgressionDataRepository::GetCurrentStorySecondsPlayed() const
{
    return mCurrentStorySecondsPlayed;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetCurrentStorySecondPlayed(const int currentStorySecondsPlayed)
{
    mCurrentStorySecondsPlayed = currentStorySecondsPlayed;
    mStoryDataSerializer->GetState()["current_story_seconds_played"] = mCurrentStorySecondsPlayed;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::pair<int, int>>& ProgressionDataRepository::GetCurrentShopBoughtProductCoordinates() const
{
    return mCurrentShopBoughtProductCoordinates;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::ClearShopBoughtProductCoordinates()
{
    mCurrentShopBoughtProductCoordinates.clear();
    mStoryDataSerializer->GetState()["current_shop_bought_product_coordinates"].clear();
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetShopBoughtProductCoordinates(const std::vector<std::pair<int, int>>& shopBoughtProductCoordinates)
{
    mCurrentShopBoughtProductCoordinates = shopBoughtProductCoordinates;
    mStoryDataSerializer->GetState()["current_shop_bought_product_coordinates"] = mCurrentShopBoughtProductCoordinates;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::AddShopBoughtProductCoordinates(const std::pair<int, int>& shopBoughtProductCoordinates)
{
    mCurrentShopBoughtProductCoordinates.push_back(shopBoughtProductCoordinates);
    mStoryDataSerializer->GetState()["current_shop_bought_product_coordinates"] = mCurrentShopBoughtProductCoordinates;
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
    
    nlohmann::json currentStoryMapCoordJson;
    currentStoryMapCoordJson["col"] = currentStoryMapNodeCoord.x;
    currentStoryMapCoordJson["row"] = currentStoryMapNodeCoord.y;
    mStoryDataSerializer->GetState()["current_story_map_node_coord"] = currentStoryMapCoordJson;
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

const std::string& ProgressionDataRepository::GetNextStoryOpponentTexturePath() const
{
    return mNextStoryOpponentTexturePath;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextStoryOpponentTexturePath(const std::string& nextStoryOpponentTexturePath)
{
    mNextStoryOpponentTexturePath = nextStoryOpponentTexturePath;
    mStoryDataSerializer->GetState()["next_story_opponent_path"] = nextStoryOpponentTexturePath;
}

///------------------------------------------------------------------------------------------------

const std::string& ProgressionDataRepository::GetNextStoryOpponentName() const
{
    return mNextStoryOpponentName;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetNextStoryOpponentName(const std::string& nextStoryOpponentName)
{
    mNextStoryOpponentName = nextStoryOpponentName;
    mStoryDataSerializer->GetState()["next_story_opponent_name"] = nextStoryOpponentName;
}

///------------------------------------------------------------------------------------------------

const bool& ProgressionDataRepository::IsCurrentlyPlayingStoryMode() const
{
    return mIsCurrentlyPlayingStoryMode;
}

///------------------------------------------------------------------------------------------------

void ProgressionDataRepository::SetIsCurrentlyPlayingStoryMode(const bool isCurrentlyPlayingStoryMode)
{
    mIsCurrentlyPlayingStoryMode = isCurrentlyPlayingStoryMode;
}

///------------------------------------------------------------------------------------------------
