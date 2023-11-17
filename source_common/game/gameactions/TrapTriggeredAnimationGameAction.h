///------------------------------------------------------------------------------------------------
///  TrapTriggeredAnimationGameAction.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/11/2023
///------------------------------------------------------------------------------------------------

#ifndef TrapTriggeredAnimationGameAction_h
#define TrapTriggeredAnimationGameAction_h

///------------------------------------------------------------------------------------------------

#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

class TrapTriggeredAnimationGameAction final: public BaseGameAction
{
public:
    static const std::string LAST_PLAYED_CARD_INDEX_PARAM;

public:
    void VSetNewGameState() override;
    
    void VInitAnimation() override;
    
    ActionAnimationUpdateResult VUpdateAnimation(const float dtMillis) override;
    
    bool VShouldBeSerialized() const override;
    
    const std::vector<std::string>& VGetRequiredExtraParamNames() const override;
private:
    enum class ActionState
    {
        KILL_SO_ANIMATION_STEP_1,
        KILL_SO_ANIMATION_STEP_2,
        KILL_SO_ANIMATION_STEP_3,
        FINISHED
    };
    
    ActionState mAnimationState;
};

///------------------------------------------------------------------------------------------------

#endif /* TrapTriggeredAnimationGameAction_h */
