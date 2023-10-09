///------------------------------------------------------------------------------------------------
///  PlayCardGameAction.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef PlayCardGameAction_h
#define PlayCardGameAction_h

///------------------------------------------------------------------------------------------------

#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

class PlayCardGameAction final: public BaseGameAction
{
public:
    void VSetNewGameState() override;
    
    void VInitAnimation() override;
    
    ActionAnimationUpdateResult VUpdateAnimation(const float dtMillis) override;
};

///------------------------------------------------------------------------------------------------

#endif /* PlayCardGameAction_h */
