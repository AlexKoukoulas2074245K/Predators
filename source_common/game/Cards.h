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
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { struct SceneObject; }

///------------------------------------------------------------------------------------------------

enum class CardOrientation
{
    FRONT_FACE, BACK_FACE
};

///------------------------------------------------------------------------------------------------

enum class CardSoState
{
    MOVING_TO_SET_POSITION,
    IDLE,
    HIGHLIGHTED,
    FREE_MOVING
};

///------------------------------------------------------------------------------------------------

struct CardData
{
    int mCardId;
    int mCardDamage;
    std::string mCardName;
    resources::ResourceId mCardTextureResourceId;
    resources::ResourceId mCardShaderResourceId;
};

///------------------------------------------------------------------------------------------------

struct CardSoWrapper
{
    CardSoState mState = CardSoState::IDLE;
    const CardData* mCardData = nullptr;
    std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjectComponents;
};

///------------------------------------------------------------------------------------------------

class CardDataRepository final
{
public:
    static CardDataRepository& GetInstance();
    
    ~CardDataRepository() = default;
    CardDataRepository(const CardDataRepository&) = delete;
    CardDataRepository(CardDataRepository&&) = delete;
    const CardDataRepository& operator = (const CardDataRepository&) = delete;
    CardDataRepository& operator = (CardDataRepository&&) = delete;
    
    size_t GetCardDataCount() const;
    std::optional<std::reference_wrapper<const CardData>> GetCard(const int cardId) const;
    void LoadCardData();
    
private:
    CardDataRepository() = default;
    
private:
    std::unordered_map<int, CardData> mCardDataMap;
};

///------------------------------------------------------------------------------------------------

#endif /* Cards_h */
