///------------------------------------------------------------------------------------------------
///  BattleSerializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <fstream>
#include <game/utils/BattleSerializer.h>

///------------------------------------------------------------------------------------------------

BattleSerializer::BattleSerializer(const int gameSeed, const std::vector<int>& topPlayerDeck, const std::vector<int>& botPlayerDeck)
    : serial::BaseDataFileSerializer("last_battle", serial::DataFileType::PERSISTENCE_FILE_TYPE)
{
    mState["seed"] = gameSeed;
    mState["top_deck"] = topPlayerDeck;
    mState["bot_deck"] = botPlayerDeck;
    
    events::EventSystem::GetInstance().RegisterForEvent<events::SerializableGameActionEvent>(this, &BattleSerializer::OnSerializableGameActionEvent);
}

///------------------------------------------------------------------------------------------------

void BattleSerializer::OnSerializableGameActionEvent(const events::SerializableGameActionEvent& event)
{
    nlohmann::json actionJson;
    actionJson["name"] = event.mActionName.GetString();
    nlohmann::json actionExtraParamsJson;
    for (const auto& extraActionParam: event.mExtraActionParams)
    {
        actionExtraParamsJson[extraActionParam.first] = extraActionParam.second;
    }
    
    if (!actionExtraParamsJson.empty())
    {
        actionJson["extraActionParams"] = std::move(actionExtraParamsJson);
    }
    
    mState["actions"].push_back(std::move(actionJson));
}

///------------------------------------------------------------------------------------------------
