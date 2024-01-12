///------------------------------------------------------------------------------------------------
///  Cards.cpp
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 10/10/2023
///------------------------------------------------------------------------------------------------

#include <algorithm>
#include <engine/CoreSystemsEngine.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/DataFileResource.h>
#include <engine/resloading/TextureResource.h>
#include <engine/utils/BaseDataFileDeserializer.h>
#include <engine/utils/OSMessageBox.h>
#include <game/Cards.h>
#include <game/GameConstants.h>
#include <game/ProgressionDataRepository.h>
#include <nlohmann/json.hpp>

///------------------------------------------------------------------------------------------------

static const std::vector<int> FRESH_ACCOUNT_UNLOCKED_CARDS =
{
    // All family story starting cards
    17, 14, 3, 16, 4, 15, 8, 9, 7, 10, 2, 12,
    
    // Rest of available cards
    0, 13, 6, 1, 11, 5, 18, 20, 21, 27, 28
};

static const std::unordered_map<strutils::StringId, std::vector<int>, strutils::StringIdHasher> FAMILY_STORY_STARTING_CARDS =
{
    { game_constants::DINOSAURS_FAMILY_NAME, {17, 14, 3, 16}},
    { game_constants::RODENTS_FAMILY_NAME, {4, 15, 8, 9}},
    { game_constants::INSECTS_FAMILY_NAME, {7, 10, 2, 12}},
};

///------------------------------------------------------------------------------------------------

CardDataRepository& CardDataRepository::GetInstance()
{
    static CardDataRepository CardDataRepository;
    return CardDataRepository;
}

///------------------------------------------------------------------------------------------------

size_t CardDataRepository::GetCardDataCount() const
{
    return mCardDataMap.size();
}

///------------------------------------------------------------------------------------------------

std::vector<int> CardDataRepository::GetAllCardIds() const
{
    std::vector<int> resultCardIds;
    for (const auto& [id, data]: mCardDataMap)
    {
        resultCardIds.emplace_back(data.mCardId);
    }
    return resultCardIds;
}

///------------------------------------------------------------------------------------------------

std::vector<int> CardDataRepository::GetAllNonSpellCardIds() const
{
    std::vector<int> resultCardIds;
    for (const auto& [id, data]: mCardDataMap)
    {
        if (!data.IsSpell())
        {
            resultCardIds.emplace_back(data.mCardId);
        }
    }
    return resultCardIds;
}

///------------------------------------------------------------------------------------------------

std::vector<int> CardDataRepository::GetCardIdsByFamily(const strutils::StringId& family) const
{
    std::vector<int> resultCardIds;
    for (const auto& [id, data]: mCardDataMap)
    {
        if (data.mCardFamily == family)
        {
            resultCardIds.emplace_back(data.mCardId);
        }
    }
    return resultCardIds;
}

///------------------------------------------------------------------------------------------------

std::vector<int> CardDataRepository::GetStoryStartingFamilyCards(const strutils::StringId& family) const
{
    return FAMILY_STORY_STARTING_CARDS.at(family);
}

///------------------------------------------------------------------------------------------------

std::vector<int> CardDataRepository::GetFreshAccountUnlockedCardIds() const
{
    return FRESH_ACCOUNT_UNLOCKED_CARDS;
}

///------------------------------------------------------------------------------------------------

CardData CardDataRepository::GetCardData(const int cardId, const size_t forPlayerIndex) const
{
    auto findIter = mCardDataMap.find(cardId);
    if (findIter != mCardDataMap.end())
    {
        CardData cardData = findIter->second;
        
        if (!ProgressionDataRepository::GetInstance().GetQuickPlayData())
        {
            if (forPlayerIndex == game_constants::LOCAL_PLAYER_INDEX)
            {
                const auto& storyCardStatModifiers = ProgressionDataRepository::GetInstance().GetStoryPlayerCardStatModifiers();
                if (storyCardStatModifiers.count(CardStatType::DAMAGE))
                {
                    cardData.mCardDamage += storyCardStatModifiers.at(CardStatType::DAMAGE);
                }
                if (storyCardStatModifiers.count(CardStatType::WEIGHT))
                {
                    cardData.mCardWeight += storyCardStatModifiers.at(CardStatType::WEIGHT);
                }
            }
        }
        
        return cardData;
    }
    
    ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, ("Cannot find card with id " + std::to_string(cardId)).c_str());
    return CardData();
}

///------------------------------------------------------------------------------------------------

const std::unordered_set<strutils::StringId, strutils::StringIdHasher>& CardDataRepository::GetCardFamilies() const
{
    return mCardFamilies;
}

///------------------------------------------------------------------------------------------------

strutils::StringId CardDataRepository::GuessCurrentStoryDeckFamily() const
{
    auto currentStoryDeck = ProgressionDataRepository::GetInstance().GetCurrentStoryPlayerDeck();
    std::sort(currentStoryDeck.begin(), currentStoryDeck.end());
    
    for (const auto& cardFamily: mCardFamilies)
    {
        auto allFamilyCards = GetCardIdsByFamily(cardFamily);
        std::sort(allFamilyCards.begin(), allFamilyCards.end());
        
        std::vector<int> intersection;
        std::set_intersection(currentStoryDeck.begin(), currentStoryDeck.end(), allFamilyCards.begin(), allFamilyCards.end(), std::back_inserter(intersection));
        
        if (!intersection.empty())
        {
            return cardFamily;
        }
    }
    
    assert(false);
    return game_constants::RODENTS_FAMILY_NAME;
}

///------------------------------------------------------------------------------------------------

void CardDataRepository::CleanDeckFromTempIds(std::vector<int>& deck)
{
    for (auto iter = deck.begin(); iter != deck.end();)
    {
        if (!mCardDataMap.count(*iter))
        {
            iter = deck.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

///------------------------------------------------------------------------------------------------

void CardDataRepository::ClearCardData()
{
    mCardFamilies.clear();
    mCardDataMap.clear();
}

///------------------------------------------------------------------------------------------------

void CardDataRepository::LoadCardData(bool loadCardAssets)
{
    auto& resourceService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    
#if !defined(NDEBUG)
    auto cardsDefinitionJsonResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "card_data.json");
    const auto cardDataJson =  nlohmann::json::parse(resourceService.GetResource<resources::DataFileResource>(cardsDefinitionJsonResourceId).GetContents());
#else
    const auto cardDataJson = serial::BaseDataFileDeserializer("card_data", serial::DataFileType::ASSET_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM).GetState();
#endif
    
    for (const auto& cardFamily: cardDataJson["card_families"])
    {
        mCardFamilies.insert(strutils::StringId(cardFamily.get<std::string>()));
    }
    
    std::unordered_set<int> cardIdsSeenThisLoad;
    for (const auto& cardObject: cardDataJson["card_data"])
    {
        CardData cardData = {};
        cardData.mCardId = cardObject["id"].get<int>();
        cardData.mCardWeight = cardObject["weight"].get<int>();
        
        assert(cardIdsSeenThisLoad.count(cardData.mCardId) == 0);
        
        // Normal card
        if (cardObject.count("damage"))
        {
            cardData.mCardDamage = cardObject["damage"].get<int>();
        }
        // Spell card
        else
        {
            cardData.mCardEffect = cardObject["effect"].get<std::string>();
            cardData.mCardEffectTooltip = cardObject["tooltip"].get<std::string>();
            
            assert(strutils::StringSplit(cardData.mCardEffectTooltip, '$').size() <= game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT);
        }
        
        // Optional particle effect
        if (cardObject.count("particle_effect"))
        {
            cardData.mParticleEffect = strutils::StringId(cardObject["particle_effect"].get<std::string>());
        }
        
        // Make sure card has a registered card family
        cardData.mCardFamily = strutils::StringId(cardObject["family"].get<std::string>());
        if (cardData.mCardFamily != game_constants::DEMONS_GENERIC_FAMILY_NAME && !mCardFamilies.count(cardData.mCardFamily))
        {
            ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, ("Cannot find family \"" + cardData.mCardFamily.GetString() + "\" for card with id=" + std::to_string(cardData.mCardId)).c_str());
        }
        
        cardData.mCardName = cardObject["name"].get<std::string>();
        
        if (loadCardAssets)
        {
            cardData.mCardTextureResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + cardObject["texture"].get<std::string>());
            cardData.mCardShaderResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + cardObject["shader"].get<std::string>());
        }
        
        cardIdsSeenThisLoad.insert(cardData.mCardId);
        mCardDataMap[cardData.mCardId] = cardData;
    }
}

///------------------------------------------------------------------------------------------------

int CardDataRepository::InsertDynamicCardData(const CardData& cardData)
{
    auto allCardIds = GetAllCardIds();
    std::sort(allCardIds.begin(), allCardIds.end());
    
    auto newCardId = allCardIds.back() + 1;
    mCardDataMap[newCardId] = cardData;
    mCardDataMap[newCardId].mCardId = newCardId;
    
    return newCardId;
}

///------------------------------------------------------------------------------------------------
