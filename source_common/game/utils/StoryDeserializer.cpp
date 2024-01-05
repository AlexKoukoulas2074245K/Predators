///------------------------------------------------------------------------------------------------
///  StoryDeserializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/01/2024
///------------------------------------------------------------------------------------------------

#include <fstream>
#include <game/utils/StoryDeserializer.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/ProgressionDataRepository.h>
#include <vector>

///------------------------------------------------------------------------------------------------

StoryDeserializer::StoryDeserializer(ProgressionDataRepository& progressionDataRepository)
    : serial::BaseDataFileDeserializer("story", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM)
{
    const auto& storyJson = GetState();
    
    if (storyJson.count("current_story_health"))
    {
        auto storyHealth = storyJson["current_story_health"].get<int>();
        progressionDataRepository.StoryCurrentHealth().SetDisplayedValue(storyHealth);
        progressionDataRepository.StoryCurrentHealth().SetValue(storyHealth);
    }
    
    if (storyJson.count("current_story_player_deck"))
    {
        progressionDataRepository.SetCurrentStoryPlayerDeck(storyJson["current_story_player_deck"].get<std::vector<int>>());
    }
    
    if (storyJson.count("next_top_player_deck"))
    {
        progressionDataRepository.SetNextTopPlayerDeck(storyJson["next_top_player_deck"].get<std::vector<int>>());
    }
    
    if (storyJson.count("next_bot_player_deck"))
    {
        progressionDataRepository.SetNextBotPlayerDeck(storyJson["next_bot_player_deck"].get<std::vector<int>>());
    }
    
    if (storyJson.count("current_story_map_scene_type"))
    {
        progressionDataRepository.SetCurrentStoryMapSceneType(static_cast<StoryMapSceneType>(storyJson["current_story_map_scene_type"].get<int>()));
    }
    
    if (storyJson.count("current_event_screen"))
    {
        progressionDataRepository.SetCurrentEventScreenIndex(storyJson["current_event_screen"].get<int>());
    }
    
    if (storyJson.count("story_seed"))
    {
        progressionDataRepository.SetStoryMapGenerationSeed(storyJson["story_seed"].get<int>());
    }
    
    if (storyJson.count("current_story_map_node_seed"))
    {
        progressionDataRepository.SetCurrentStoryMapNodeSeed(storyJson["current_story_map_node_seed"].get<int>());
    }
    
    if (storyJson.count("next_battle_top_health"))
    {
        progressionDataRepository.SetNextBattleTopPlayerHealth(storyJson["next_battle_top_health"].get<int>());
    }
    
    if (storyJson.count("next_battle_bot_health"))
    {
        progressionDataRepository.SetNextBattleBotPlayerHealth(storyJson["next_battle_bot_health"].get<int>());
    }
    
    if (storyJson.count("next_battle_top_init_weight"))
    {
        progressionDataRepository.SetNextBattleTopPlayerInitWeight(storyJson["next_battle_top_init_weight"].get<int>());
    }
    
    if (storyJson.count("next_battle_bot_init_weight"))
    {
        progressionDataRepository.SetNextBattleBotPlayerInitWeight(storyJson["next_battle_bot_init_weight"].get<int>());
    }
    
    if (storyJson.count("next_battle_top_weight_limit"))
    {
        progressionDataRepository.SetNextBattleTopPlayerWeightLimit(storyJson["next_battle_top_weight_limit"].get<int>());
    }
    
    if (storyJson.count("next_battle_bot_weight_limit"))
    {
        progressionDataRepository.SetNextBattleBotPlayerWeightLimit(storyJson["next_battle_bot_weight_limit"].get<int>());
    }
    
    if (storyJson.count("next_story_opponent_damage"))
    {
        progressionDataRepository.SetNextStoryOpponentDamage(storyJson["next_story_opponent_damage"].get<int>());
    }
    
    if (storyJson.count("current_story_map_node_coord"))
    {
        progressionDataRepository.SetCurrentStoryMapNodeCoord(glm::ivec2(storyJson["current_story_map_node_coord"]["col"].get<int>(), storyJson["current_story_map_node_coord"]["row"].get<int>()));
    }
    
    if (storyJson.count("next_story_opponent_path"))
    {
        progressionDataRepository.SetNextStoryOpponentTexturePath(storyJson["next_story_opponent_path"].get<std::string>());
    }
    
    if (storyJson.count("next_story_opponent_name"))
    {
        progressionDataRepository.SetNextStoryOpponentName(storyJson["next_story_opponent_name"].get<std::string>());
    }
}

///------------------------------------------------------------------------------------------------
