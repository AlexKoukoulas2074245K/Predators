///------------------------------------------------------------------------------------------------
///  GameReplayEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/OSMessageBox.h>
#include <fstream>
#include <game/GameReplayEngine.h>
#include <game/gameactions/GameActionEngine.h>
#include <nlohmann/json.hpp>
#include <vector>

//#define TEST_BINARY_FLOW

///------------------------------------------------------------------------------------------------

static nlohmann::json sGameJson;

///------------------------------------------------------------------------------------------------

GameReplayEngine::GameReplayEngine(const std::string& filenameNoExtension)
{
#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
    auto gameFileName = filenameNoExtension + ".json";
    std::ifstream gameFile(gameFileName);
    if (gameFile.is_open())
    {
        std::stringstream buffer;
        buffer << gameFile.rdbuf();
        auto contents = buffer.str();
        if (contents.size() > 1)
        {
            sGameJson = nlohmann::json::parse(buffer.str());
#else
    auto gameFileName = filenameNoExtension + ".bin";
    std::ifstream gameFile(gameFileName, std::ios::binary);
    if (gameFile.is_open())
    {
        std::vector<std::uint8_t> contents((std::istreambuf_iterator<char>(gameFile)), std::istreambuf_iterator<char>());
        if (contents.size() > 1)
        {
            sGameJson = nlohmann::json::from_bson(contents);
#endif
            mGameFileSeed = static_cast<int>(sGameJson["seed"]);
        }
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "File not found", ("Game File " + gameFileName + " not found.").c_str());
    }
}

///------------------------------------------------------------------------------------------------

int GameReplayEngine::GetGameFileSeed() const
{
    return mGameFileSeed;
}

///------------------------------------------------------------------------------------------------

void GameReplayEngine::ReplayActions(GameActionEngine* gameActionEngine)
{
    for (const auto& actionEntry: sGameJson["actions"])
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
