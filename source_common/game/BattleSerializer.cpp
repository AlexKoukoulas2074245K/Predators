///------------------------------------------------------------------------------------------------
///  BattleSerializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <game/utils/PersistenceUtils.h>
#include <filesystem>
#include <fstream>
#include <game/BattleSerializer.h>

///------------------------------------------------------------------------------------------------

BattleSerializer::BattleSerializer(const int gameSeed, const std::vector<int>& topPlayerDeck, const std::vector<int>& botPlayerDeck)
    : BaseDataFileSerializer("game")
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
