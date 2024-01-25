///------------------------------------------------------------------------------------------------
///  PersistentAccountDataDeserializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/01/2024
///------------------------------------------------------------------------------------------------

#include <fstream>
#include <game/utils/PersistentAccountDataDeserializer.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/DataRepository.h>
#include <vector>

///------------------------------------------------------------------------------------------------

PersistentAccountDataDeserializer::PersistentAccountDataDeserializer(DataRepository& dataRepository)
    : serial::BaseDataFileDeserializer("persistent", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM)
{
    const auto& persistentDataJson = GetState();
    
    if (persistentDataJson.count("currency_coins"))
    {
        auto currency = persistentDataJson["currency_coins"].get<long long>();
        dataRepository.CurrencyCoins().SetDisplayedValue(currency);
        dataRepository.CurrencyCoins().SetValue(currency);
    }
    
    if (persistentDataJson.count("next_card_pack_seed"))
    {
        dataRepository.SetNextCardPackSeed(persistentDataJson["next_card_pack_seed"].get<int>());
    }
    
    if (persistentDataJson.count("games_finished_count"))
    {
        dataRepository.SetGamesFinishedCount(persistentDataJson["games_finished_count"].get<int>());
    }
    
    if (persistentDataJson.count("unlocked_card_ids"))
    {
        dataRepository.SetUnlockedCardIds(persistentDataJson["unlocked_card_ids"].get<std::vector<int>>());
    }
    
    if (persistentDataJson.count("golden_card_id_map") && !persistentDataJson["golden_card_id_map"].is_null())
    {
        for (auto entryIter = persistentDataJson["golden_card_id_map"].begin(); entryIter != persistentDataJson["golden_card_id_map"].end(); ++entryIter)
        {
            dataRepository.SetGoldenCardMapEntry(std::stoi(entryIter.key()), entryIter.value().get<bool>());
        }
    }
}

///------------------------------------------------------------------------------------------------
