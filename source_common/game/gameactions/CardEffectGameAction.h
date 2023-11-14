///------------------------------------------------------------------------------------------------
///  CardEffectGameAction.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/11/2023
///------------------------------------------------------------------------------------------------

#ifndef CardEffectGameAction_h
#define CardEffectGameAction_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <game/gameactions/BaseGameAction.h>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------


class CardEffectGameAction final: public BaseGameAction
{
public:
    void VSetNewGameState() override;
    
    void VInitAnimation() override;
    
    ActionAnimationUpdateResult VUpdateAnimation(const float dtMillis) override;
    
    bool VShouldBeSerialized() const override;
    
    const std::vector<std::string>& VGetRequiredExtraParamNames() const override;
    
private:
    void HandleCardEffect(const std::string& effect);
    
private:
    enum class ActionState
    {
        EFFECT_CARD_ANIMATION,
        AFFECTED_CARDS_SPARKLE_ANIMATION,
        AFFECTED_CARDS_SCALE_ANIMATION,
        FINISHED
    };
    ActionState mActionState;
    
    enum class AffectedStatType
    {
        NONE,
        DAMAGE,
        WEIGHT
    };
    
    static const std::unordered_map<AffectedStatType, CardStatType> sAffectedStatTypeToCardStatType;
    
    AffectedStatType mAffectedBoardCardsStatType;
    int mEffectValue;
    bool mAffectingNextPlayer;
    float mAnimationDelayCounterSecs;
    std::vector<int> mAffectedBoardIndices;
};

///------------------------------------------------------------------------------------------------

#endif /* CardEffectGameAction_h */
