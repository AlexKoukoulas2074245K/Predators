///------------------------------------------------------------------------------------------------
///  NextPlayerGameAction.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef NextPlayerGameAction_h
#define NextPlayerGameAction_h

///------------------------------------------------------------------------------------------------

#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

class NextPlayerGameAction final: public BaseGameAction
{
public:
    void VSetNewGameState() override;
};

///------------------------------------------------------------------------------------------------

#endif /* NextPlayerGameAction_h */