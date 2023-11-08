///------------------------------------------------------------------------------------------------
///  Cards.cpp
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 10/10/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/DataFileResource.h>
#include <engine/resloading/TextureResource.h>
#include <engine/utils/OSMessageBox.h>
#include <game/Cards.h>
#include <nlohmann/json.hpp>

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

std::optional<std::reference_wrapper<const CardData>> CardDataRepository::GetCardData(const int cardId) const
{
    auto findIter = mCardDataMap.find(cardId);
    if (findIter != mCardDataMap.end())
    {
        return std::optional<std::reference_wrapper<const CardData>>{findIter->second};
    }
    
    ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, ("Cannot find card with id " + std::to_string(cardId)).c_str());
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void CardDataRepository::LoadCardData(bool loadAssets)
{
    auto cardsDefinitionJsonResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "card_data.json");
    auto& resourceService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    auto cardDataJson =  nlohmann::json::parse(CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetResource<resources::DataFileResource>(cardsDefinitionJsonResourceId).GetContents());
    
    for (const auto& cardFamily: cardDataJson["card_families"])
    {
        mCardFamilies.insert(strutils::StringId(cardFamily.get<std::string>()));
    }
    
    for (const auto& cardObject: cardDataJson["card_data"])
    {
        CardData cardData = {};
        cardData.mCardId = cardObject["id"].get<int>();
        cardData.mCardWeight = cardObject["weight"].get<int>();
        
        // Normal card
        if (cardObject.count("damage"))
        {
            cardData.mCardDamage = cardObject["damage"].get<int>();
        }
        // Spell card
        else
        {
            cardData.mCardEffect = cardObject["effect"].get<std::string>();
        }
        
        // Make sure card has a registered card family
        cardData.mCardFamily = strutils::StringId(cardObject["family"].get<std::string>());
        if (!mCardFamilies.count(cardData.mCardFamily))
        {
            ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, ("Cannot find family \"" + cardData.mCardFamily.GetString() + "\" for card with id=" + std::to_string(cardData.mCardId)).c_str());
        }
        
        cardData.mCardName = cardObject["name"].get<std::string>();
        
        if (loadAssets)
        {
            cardData.mCardTextureResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + cardObject["texture"].get<std::string>());
            cardData.mCardShaderResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + cardObject["shader"].get<std::string>());
        }
        
        mCardDataMap[cardData.mCardId] = cardData;
    }
}

///------------------------------------------------------------------------------------------------
