///------------------------------------------------------------------------------------------------
///  Cards.h
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 10/10/2023
///------------------------------------------------------------------------------------------------

#ifndef Cards_h
#define Cards_h

///------------------------------------------------------------------------------------------------

#include <engine/resloading/ResourceLoadingService.h>
#include <optional>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

struct Card
{
    int mCardId;
    int mCardDamage;
    std::string mCardName;
    resources::ResourceId mCardTextureResourceId;
    resources::ResourceId mCardShaderResourceId;
};

///------------------------------------------------------------------------------------------------

class CardRepository final
{
public:
    static CardRepository& GetInstance();
    
    ~CardRepository() = default;
    CardRepository(const CardRepository&) = delete;
    CardRepository(CardRepository&&) = delete;
    const CardRepository& operator = (const CardRepository&) = delete;
    CardRepository& operator = (CardRepository&&) = delete;
    
    size_t GetCardCount() const;
    std::optional<std::reference_wrapper<const Card>> GetCard(const int cardId) const;
    void LoadCards();
    
private:
    CardRepository() = default;
    
private:
    std::unordered_map<int, Card> mCardMap;
};

///------------------------------------------------------------------------------------------------

#endif /* Cards_h */
