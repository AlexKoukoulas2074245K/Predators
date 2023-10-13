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
#include <memory>

///------------------------------------------------------------------------------------------------

struct CardSoWrapper;
class PlayCardGameAction final: public BaseGameAction
{
public:
    void VSetNewGameState() override;
    
    void VInitAnimation() override;
    
    ActionAnimationUpdateResult VUpdateAnimation(const float dtMillis) override;
    
private:
    std::shared_ptr<CardSoWrapper> mLastPlayedCardSoWrapper;
};

///------------------------------------------------------------------------------------------------

#endif /* PlayCardGameAction_h */
