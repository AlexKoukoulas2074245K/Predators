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

CardRepository& CardRepository::GetInstance()
{
    static CardRepository cardRepository;
    return cardRepository;
}

///------------------------------------------------------------------------------------------------

size_t CardRepository::GetCardCount() const
{
    return mCardMap.size();
}

///------------------------------------------------------------------------------------------------

std::optional<std::reference_wrapper<const Card>> CardRepository::GetCard(const int cardId) const
{
    auto findIter = mCardMap.find(cardId);
    if (findIter != mCardMap.end())
    {
        return std::optional<std::reference_wrapper<const Card>>{findIter->second};
    }
    
    ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, ("Cannot find card with id " + std::to_string(cardId)).c_str());
    return std::nullopt;
}

///------------------------------------------------------------------------------------------------

void CardRepository::LoadCards()
{
    auto cardsDefinitionJsonResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "cards.json");
    auto& resourceService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    auto fontJson =  nlohmann::json::parse(CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetResource<resources::DataFileResource>(cardsDefinitionJsonResourceId).GetContents());
    
    for (const auto& cardObject: fontJson["cards"])
    {
        Card card;
        card.mCardId = cardObject["id"].get<int>();
        card.mCardDamage = cardObject["damage"].get<int>();
        card.mCardName = cardObject["name"].get<std::string>();
        card.mCardTextureResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + cardObject["texture"].get<std::string>());
        card.mCardShaderResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + cardObject["shader"].get<std::string>());
        
        mCardMap[card.mCardId] = card;
    }
}

///------------------------------------------------------------------------------------------------
