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
#include <unordered_set>
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

enum class CardRarity
{
    NORMAL,
    GOLDEN
};

///------------------------------------------------------------------------------------------------

enum class CardStatType
{
    DAMAGE,
    WEIGHT
};

///------------------------------------------------------------------------------------------------

using CardStatOverrides = std::unordered_map<CardStatType, int>;

///------------------------------------------------------------------------------------------------

struct CardData
{
    bool IsSpell() const { return !mCardEffect.empty(); }
    
    int mCardId;
    int mCardDamage;
    int mCardWeight;
    std::string mCardName;
    std::string mCardEffect;
    std::string mCardEffectTooltip;
    strutils::StringId mCardFamily;
    strutils::StringId mParticleEffect;
    resources::ResourceId mCardTextureResourceId;
    resources::ResourceId mCardShaderResourceId;
};

///------------------------------------------------------------------------------------------------

struct CardSoWrapper
{
    CardSoState mState = CardSoState::IDLE;
    CardData mCardData;
    std::shared_ptr<scene::SceneObject> mSceneObject;
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
    std::vector<int> GetAllCardIds() const;
    std::vector<int> GetAllNonSpellCardIds() const;
    std::vector<int> GetCardIdsByFamily(const strutils::StringId& family) const;
    std::vector<int> GetStoryStartingFamilyCards(const strutils::StringId& family) const;
    std::vector<int> GetFreshAccountUnlockedCardIds() const;
    std::vector<int> GetStoryUnlockedCardRewardsPool() const;
    
    CardData GetCardData(const int cardId, const size_t forPlayerIndex) const;
    const std::unordered_set<strutils::StringId, strutils::StringIdHasher>& GetCardFamilies() const;
    strutils::StringId GuessCurrentStoryDeckFamily() const;
    
    void CleanDeckFromTempIds(std::vector<int>& deck);
    void ClearCardData();
    void LoadCardData(bool loadCardAssets);
    int InsertDynamicCardData(const CardData& cardData);
    
private:
    CardDataRepository() = default;
    
private:
    std::unordered_map<int, CardData> mCardDataMap;
    std::unordered_set<strutils::StringId, strutils::StringIdHasher> mCardFamilies;
};

///------------------------------------------------------------------------------------------------

#endif /* Cards_h */
