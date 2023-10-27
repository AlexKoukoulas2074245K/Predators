///------------------------------------------------------------------------------------------------
///  GameReplayEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <fstream>
#include <game/GameReplayEngine.h>
#include <game/gameactions/GameActionEngine.h>
#include <nlohmann/json.hpp>
#include <vector>

///------------------------------------------------------------------------------------------------

void GameReplayEngine::ReplayActionsFromGameFile(const std::string& filename, GameActionEngine* gameActionEngine)
{
    nlohmann::json gameJson;
    
#if !defined(NDEBUG)
    std::ifstream gameFile(filename);
    std::stringstream buffer;
    buffer << gameFile.rdbuf();
    gameJson = nlohmann::json::parse(buffer.str());
#else
    std::ifstream gameFile(filename, std::ios::binary);
    std::vector<std::uint8_t> contents((std::istreambuf_iterator<char>(gameFile)), std::istreambuf_iterator<char>());
    gameJson = nlohmann::json::from_bson(contents);
#endif
    
    //TODO: Handle SEED
    for (const auto& actionEntry: gameJson["actions"])
    {
        std::unordered_map<std::string, std::string> extraActionParams;
        if (actionEntry.count("extraActionParams") != 0)
        {
            auto extraActionParamsJson = actionEntry["extraActionParams"];
            for(auto it = extraActionParamsJson.begin(); it != extraActionParamsJson.end(); ++it)
            {
                extraActionParams[it.key()] = it.value();
            }
        }
        gameActionEngine->AddGameAction(strutils::StringId(actionEntry["name"]), extraActionParams);
    }
}

///------------------------------------------------------------------------------------------------
