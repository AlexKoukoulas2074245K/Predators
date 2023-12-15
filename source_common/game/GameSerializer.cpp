///------------------------------------------------------------------------------------------------
///  GameSerializer.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <game/utils/PersistenceUtils.h>
#include <filesystem>
#include <fstream>
#include <game/GameSerializer.h>
#include <nlohmann/json.hpp>

//#define TEST_BINARY_FLOW

///------------------------------------------------------------------------------------------------

#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
static const std::string GAME_FILE_NAME = "game.json";
#else
static const std::string GAME_FILE_NAME = "game.bin";
#endif

static nlohmann::json sGameState;
static std::ofstream sFile;

///------------------------------------------------------------------------------------------------

GameSerializer::GameSerializer(const int gameSeed, const std::vector<int>& topPlayerDeck, const std::vector<int>& botPlayerDeck)
{
#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
    std::ifstream existingFile(persistence_utils::GetProgressDirectoryPath() + GAME_FILE_NAME);
#else
    std::ifstream existingFile(persistence_utils::GetProgressDirectoryPath() + GAME_FILE_NAME, std::ios::binary);
#endif
    
    if (existingFile.is_open())
    {
        std::stringstream buffer;
        buffer << existingFile.rdbuf();
        logging::Log(logging::LogType::INFO, "Read existing game json file %s", buffer.str().c_str());
    }
    sGameState["seed"] = gameSeed;
    sGameState["top_deck"] = topPlayerDeck;
    sGameState["bot_deck"] = botPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void GameSerializer::FlushStateToFile()
{
    if (sFile.is_open())
    {
        logging::Log(logging::LogType::INFO, "Writing game state to %s %s", (persistence_utils::GetProgressDirectoryPath() + GAME_FILE_NAME).c_str(), sGameState.dump(4).c_str());
        auto checksumString = "&" + std::to_string(strutils::StringId(sGameState.dump(4)).GetStringId());
        
    #if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
        sFile << sGameState.dump(4);
        sFile << checksumString;
    #else
        const auto binVec = nlohmann::json::to_bson(sGameState);
        sFile.write(reinterpret_cast<const char*>(&binVec[0]), binVec.size() * sizeof(std::uint8_t));
        sFile.write(reinterpret_cast<const char*>(&checksumString[0]), checksumString.size() * sizeof(char));
    #endif
        sFile.close();
    }
}

///------------------------------------------------------------------------------------------------

void GameSerializer::OnGameAction(const strutils::StringId& gameActionName, const ExtraActionParams& extraActionParams)
{
    if (!sFile.is_open())
    {
#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
    #if defined(DESKTOP_FLOW)
        std::filesystem::create_directory(persistence_utils::GetProgressDirectoryPath());
    #endif
        sFile.open(persistence_utils::GetProgressDirectoryPath() + GAME_FILE_NAME);
#else
        sFile.open(persistence_utils::GetProgressDirectoryPath() + GAME_FILE_NAME, std::ios::binary);
#endif
    }
    
    nlohmann::json actionJson;
    actionJson["name"] = gameActionName.GetString();
    nlohmann::json actionExtraParamsJson;
    for (const auto& extraActionParam: extraActionParams)
    {
        actionExtraParamsJson[extraActionParam.first] = extraActionParam.second;
    }
    
    if (!actionExtraParamsJson.empty())
    {
        actionJson["extraActionParams"] = std::move(actionExtraParamsJson);
    }
    
    sGameState["actions"].push_back(std::move(actionJson));
}

///------------------------------------------------------------------------------------------------
