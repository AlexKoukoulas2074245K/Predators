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
    const auto& storyJson = GetState();
    
    if (storyJson.count("currency_coins"))
    {
        auto currency = storyJson["currency_coins"].get<long long>();
        dataRepository.CurrencyCoins().SetDisplayedValue(currency);
        dataRepository.CurrencyCoins().SetValue(currency);
    }
    
    if (storyJson.count("unlocked_card_ids"))
    {
        dataRepository.SetUnlockedCardIds(storyJson["unlocked_card_ids"].get<std::vector<int>>());
    }
}

///------------------------------------------------------------------------------------------------
