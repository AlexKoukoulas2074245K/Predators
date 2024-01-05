///------------------------------------------------------------------------------------------------
///  PersistentAccountDataDeserializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/01/2024
///------------------------------------------------------------------------------------------------

#include <fstream>
#include <game/utils/PersistentAccountDataDeserializer.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/ProgressionDataRepository.h>
#include <vector>

///------------------------------------------------------------------------------------------------

PersistentAccountDataDeserializer::PersistentAccountDataDeserializer(ProgressionDataRepository& progressionDataRepository)
    : serial::BaseDataFileDeserializer("persistent", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM)
{
    const auto& storyJson = GetState();
    
    if (storyJson.count("currency_coins"))
    {
        auto currency = storyJson["currency_coins"].get<long long>();
        progressionDataRepository.CurrencyCoins().SetDisplayedValue(currency);
        progressionDataRepository.CurrencyCoins().SetValue(currency);
    }
}

///------------------------------------------------------------------------------------------------
