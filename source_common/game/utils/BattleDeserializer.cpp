///------------------------------------------------------------------------------------------------
///  BattleDeserializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <fstream>
#include <game/utils/BattleDeserializer.h>
#include <game/gameactions/GameActionEngine.h>
#include <vector>

///------------------------------------------------------------------------------------------------

BattleDeserializer::BattleDeserializer()
    : serial::BaseDataFileDeserializer("last_battle", serial::DataFileType::PERSISTENCE_FILE_TYPE)
{
    mGameFileSeed = mState["seed"].get<int>();
    mTopPlayerDeck = mState["top_deck"].get<std::vector<int>>();
    mBotPlayerDeck = mState["bot_deck"].get<std::vector<int>>();
}

///------------------------------------------------------------------------------------------------

int BattleDeserializer::GetGameFileSeed() const
{
    return mGameFileSeed;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& BattleDeserializer::GetTopPlayerDeck() const
{
    return mTopPlayerDeck;
}

///------------------------------------------------------------------------------------------------

const std::vector<int>& BattleDeserializer::GetBotPlayerDeck() const
{
    return mBotPlayerDeck;
}

///------------------------------------------------------------------------------------------------

void BattleDeserializer::ReplayActions(GameActionEngine* gameActionEngine)
{
    for (const auto& actionEntry: mState["actions"])
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

