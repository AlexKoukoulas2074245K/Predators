///------------------------------------------------------------------------------------------------
///  GameConstants.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameConstants_h
#define GameConstants_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>

///------------------------------------------------------------------------------------------------

namespace constants
{
    inline const std::string TOP_PLAYER_CARD_NAME_PREFIX = "TOP_PLAYER_CARD_";
    inline const std::string BOT_PLAYER_CARD_NAME_PREFIX = "BOT_PLAYER_CARD_";
    
    inline const int IN_GAME_CARD_PUSH_THRESHOLD = 4;

    inline const float IN_GAME_CARD_SCALE = 0.1f;
    inline const float IN_GAME_CARD_WIDTH = 0.055f;
    inline const float IN_GAME_CARD_PUSH_VALUE = 0.003f;
    inline const float IN_GAME_PLAYER_HELD_CARD_Z = 0.2f;
    inline const float IN_GAME_TOP_PLAYER_HELD_CARD_Y = 0.1f;
    inline const float IN_GAME_BOT_PLAYER_HELD_CARD_Y = -0.1f;
    inline const float IN_GAME_DRAW_CARD_INIT_X = 0.274f;
    inline const float IN_GAME_DRAW_CARD_TOP_PLAYER_MID_POINT_Y = -0.075f;
    inline const float IN_GAME_DRAW_CARD_BOT_PLAYER_MID_POINT_Y = 0.075f;
    inline const float IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS = 0.75f;
    inline const float IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DURATION_SECS = 0.25f;
    inline const float IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DELAY_SECS = 0.5f;
}

///------------------------------------------------------------------------------------------------

#endif /* GameConstants_h */
