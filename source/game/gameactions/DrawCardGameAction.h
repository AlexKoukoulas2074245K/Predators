///------------------------------------------------------------------------------------------------
///  DrawCardGameAction.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef DrawCardGameAction_h
#define DrawCardGameAction_h

///------------------------------------------------------------------------------------------------

#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

class DrawCardGameAction final: public BaseGameAction
{
public:
    void VSetNewGameState() override;
};

///------------------------------------------------------------------------------------------------

#endif /* DrawCardGameAction_h */
