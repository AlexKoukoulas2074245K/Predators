///------------------------------------------------------------------------------------------------
///  CardAttackGameAction.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023
///------------------------------------------------------------------------------------------------

#ifndef CardAttackGameAction_h
#define CardAttackGameAction_h

///------------------------------------------------------------------------------------------------

#include <game/gameactions/BaseGameAction.h>
#include <memory>

///------------------------------------------------------------------------------------------------


class CardAttackGameAction final: public BaseGameAction
{
public:
    static const std::string CARD_INDEX_PARAM;
    static const std::string PLAYER_INDEX_PARAM;
    
public:
    void VSetNewGameState() override;
    
    void VInitAnimation() override;
    
    ActionAnimationUpdateResult VUpdateAnimation(const float dtMillis) override;
    
    bool VShouldBeSerialized() const override;
    
    const std::vector<std::string>& VGetRequiredExtraParamNames() const override;
private:
    int mPendingAnimations;
};

///------------------------------------------------------------------------------------------------

#endif /* CardAttackGameAction_h */
