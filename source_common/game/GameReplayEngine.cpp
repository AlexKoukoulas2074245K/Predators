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

template<class T>
bool ValidateChecksum(T& contentsContainer)
{
    std::string checkSumString;
    
    while (!contentsContainer.empty())
    {
        if (contentsContainer.back() == '&')
        {
            break;
        }
        checkSumString = char(contentsContainer.back()) + checkSumString;
        contentsContainer.pop_back();
    }
    
    // Newline on debug builds
#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
    checkSumString.pop_back();
#endif
    
    contentsContainer.pop_back();
    
    if (contentsContainer.empty())
    {
        return false;
    }
    
#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
    if (checkSumString == std::to_string(strutils::StringId(nlohmann::json::parse(contentsContainer).dump(4)).GetStringId()))
    {
        return true;
    }
#else
    if (checkSumString == std::to_string(strutils::StringId(nlohmann::json::from_bson(contentsContainer).dump(4)).GetStringId()))
    {
        return true;
    }
#endif
    
    return false;
}

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
        
        if (!ValidateChecksum(contents))
        {
            ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Corrupted file", ("Game File " + gameFileName + " is corrupted.").c_str());
            return;
        }
        
        if (contents.size() > 1)
        {
            sGameJson = nlohmann::json::parse(contents);
#else
    auto gameFileName = filenameNoExtension + ".bin";
    std::ifstream gameFile(gameFileName, std::ios::binary);
    if (gameFile.is_open())
    {
        std::vector<std::uint8_t> contents((std::istreambuf_iterator<char>(gameFile)), std::istreambuf_iterator<char>());
        
        if (!ValidateChecksum(contents))
        {
            ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Corrupted file", ("Game File " + gameFileName + " is corrupted.").c_str());
            return;
        }
        
        if (contents.size() > 1)
        {
            sGameJson = nlohmann::json::from_bson(contents);
#endif
            mGameFileSeed = sGameJson["seed"].get<int>();
            mTopPlayerDeck = sGameJson["top_deck"].get<std::vector<int>>();
            mBotPlayerDeck = sGameJson["bot_deck"].get<std::vector<int>>();
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

const std::vector<int>& GameReplayEngine::GetTopPlayerDeck() const
{
    return mTopPlayerDeck;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& GameReplayEngine::GetBotPlayerDeck() const
{
    return mBotPlayerDeck;
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

