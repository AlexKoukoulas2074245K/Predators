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
    
    if (persistentDataJson.count("new_card_ids"))
    {
        dataRepository.SetNewCardIds(persistentDataJson["new_card_ids"].get<std::vector<int>>());
    }
    
    if (persistentDataJson.count("seen_opponent_spell_card_ids"))
    {
        dataRepository.SetSeenOpponentSpellCardIds(persistentDataJson["seen_opponent_spell_card_ids"].get<std::vector<int>>());
    }
    
    if (persistentDataJson.count("successful_transaction_ids"))
    {
        dataRepository.SetSuccessfulTransactionIds(persistentDataJson["successful_transaction_ids"].get<std::vector<std::string>>());
    }
    
    if (persistentDataJson.count("gift_codes_claimed"))
    {
        dataRepository.SetGiftCodesClaimed(persistentDataJson["gift_codes_claimed"].get<std::vector<std::string>>());
    }
    
    if (persistentDataJson.count("audio_enabled"))
    {
        dataRepository.SetAudioEnabled(persistentDataJson["audio_enabled"].get<bool>());
    }
    
    if (persistentDataJson.count("golden_card_id_map"))
    {
        dataRepository.ClearGoldenCardIdMap();
        
        if (!persistentDataJson["golden_card_id_map"].is_null())
        {
            for (auto entryIter = persistentDataJson["golden_card_id_map"].begin(); entryIter != persistentDataJson["golden_card_id_map"].end(); ++entryIter)
            {
                dataRepository.SetGoldenCardMapEntry(std::stoi(entryIter.key()), entryIter.value().get<bool>());
            }
        }
    }
    
    if (persistentDataJson.count("pending_card_packs"))
    {
        while (!dataRepository.GetPendingCardPacks().empty())
        {
            dataRepository.PopFrontPendingCardPack();
        }
        
        if (!persistentDataJson["pending_card_packs"].is_null())
        {
            for (auto entryIter = persistentDataJson["pending_card_packs"].begin(); entryIter != persistentDataJson["pending_card_packs"].end(); ++entryIter)
            {
                dataRepository.AddPendingCardPack(static_cast<CardPackType>(std::stoi(entryIter.value().get<std::string>())));
            }
        }
    }
}

///------------------------------------------------------------------------------------------------
