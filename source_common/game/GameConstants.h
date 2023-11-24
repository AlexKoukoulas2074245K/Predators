///------------------------------------------------------------------------------------------------
///  GameConstants.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameConstants_h
#define GameConstants_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>

///------------------------------------------------------------------------------------------------

namespace game_constants
{
    // Resources
    inline const std::string DEFAULT_MESH_NAME = "quad.obj";
    inline const std::string DEFAULT_SHADER_NAME = "basic.vs";
    inline const std::string ACTION_HIGHLIGHTER_SHADER_NAME = "action_highlighter_perlin_noise.vs";
    inline const std::string BOARD_CARD_LOCATION_SHADER_NAME = "card_board_location_perlin_noise.vs";
    inline const std::string DEFAULT_TEXTURE_NAME = "debug.png";
    inline const std::string CARD_LOCATION_MASK_TEXTURE_NAME =  "card_location_mask.png";

    // SO Name Prefixes/Postfixes
    inline const std::string TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX = "TOP_PLAYER_HELD_CARD_";
    inline const std::string BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX = "BOT_PLAYER_HELD_CARD_";
    inline const std::string TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX = "TOP_PLAYER_BOARD_CARD_";
    inline const std::string BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX = "BOT_PLAYER_BOARD_CARD_";
    inline const std::string CARD_FREE_MOVING_ANIMATION_NAME_PRE_FIX = "FREE_MOVING_CARD_";
    inline const std::string BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX = "BOARD_SIDE_EFFECT_TOP_VALUE_";
    inline const std::string BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX = "BOARD_SIDE_EFFECT_BOT_VALUE_";
    inline const std::string CARD_BASE_SO_NAME_POST_FIX = "_CARD";

    // SO Names
    inline const strutils::StringId TURN_POINTER_SCENE_OBJECT_NAME = strutils::StringId("TURN_POINTER");
    inline const strutils::StringId TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME = strutils::StringId("TURN_POINTER_HIGHLIGHTER");
    inline const strutils::StringId BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME = strutils::StringId("BOARD_SIDE_EFFECT_TOP");
    inline const strutils::StringId BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME = strutils::StringId("BOARD_SIDE_EFFECT_BOT");
    inline const strutils::StringId KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME = strutils::StringId("KILL_SIDE_EFFECT_TOP");
    inline const strutils::StringId KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME = strutils::StringId("KILL_SIDE_EFFECT_BOT");

    // Fonts
    inline const strutils::StringId DEFAULT_FONT_NAME = strutils::StringId("font");
    inline const strutils::StringId DEFAULT_FONT_BLACK_NAME = strutils::StringId("font_black");
    inline const strutils::StringId FONT_PLACEHOLDER_DAMAGE_NAME = strutils::StringId("font_placeholder_damage");
    inline const strutils::StringId FONT_PLACEHOLDER_WEIGHT_NAME = strutils::StringId("font_placeholder_weight");

    // Uniforms
    inline const strutils::StringId CARD_HIGHLIGHTER_INVALID_ACTION_UNIFORM_NAME = strutils::StringId("invalid_action");
    inline const strutils::StringId CUSTOM_ALPHA_UNIFORM_NAME = strutils::StringId("custom_alpha");
    inline const strutils::StringId TIME_UNIFORM_NAME = strutils::StringId("time");
    inline const strutils::StringId PERLIN_TIME_SPEED_UNIFORM_NAME = strutils::StringId("time_speed");
    inline const strutils::StringId PERLIN_RESOLUTION_UNIFORM_NAME = strutils::StringId("perlin_resolution");
    inline const strutils::StringId PERLIN_CLARITY_UNIFORM_NAME = strutils::StringId("perlin_clarity");
    inline const strutils::StringId CARD_WEIGHT_INTERACTIVE_MODE_UNIFORM_NAME = strutils::StringId("weight_interactive_mode");
    inline const strutils::StringId CARD_DAMAGE_INTERACTIVE_MODE_UNIFORM_NAME = strutils::StringId("damage_interactive_mode");
    inline const strutils::StringId LIGHT_POS_X_UNIFORM_NAME = strutils::StringId("light_pos_x");
    inline const strutils::StringId IS_GOLDEN_CARD_UNIFORM_NAME = strutils::StringId("golden_card");
    inline const strutils::StringId IS_HELD_CARD_UNIFORM_NAME = strutils::StringId("held_card");

    // Card Family Names
    inline const strutils::StringId INSECTS_FAMILY_NAME = strutils::StringId("insects");

    // Scenes
    inline const strutils::StringId IN_GAME_BATTLE_SCENE = strutils::StringId("BATTLE");
    
    // General Game Constants
    inline const int REMOTE_PLAYER_INDEX = 0;
    inline const int LOCAL_PLAYER_INDEX = 1;
    inline const int MAX_BOARD_CARDS = 5;
    inline const int CARD_INTERACTIVE_MODE_DEFAULT = 0;
    inline const int CARD_INTERACTIVE_MODE_INTERACTIVE = 1;
    inline const int CARD_INTERACTIVE_MODE_NONINTERACTIVE = 2;
    inline const int IN_GAME_CARD_PUSH_THRESHOLD = 4;
    inline const int BOARD_SIDE_EFFECT_VALUE_SO_COUNT = 2;

    inline const float CARD_COMPONENT_Z_OFFSET = 0.1f;
    inline const float CARD_BOUNDING_RECT_X_MULTIPLIER = 0.5f;
    inline const float IN_GAME_PLAYED_CARD_Z = 0.1f;
    inline const float IN_GAME_HELD_CARD_Z = 0.3f;
    inline const float IN_GAME_CARD_BASE_SCALE = 0.1f;
    inline const float IN_GAME_CARD_PORTRAIT_SCALE = 0.025f;
    inline const float IN_GAME_CARD_PORTRAIT_Y_OFFSET = 0.008f;
    inline const float IN_GAME_CARD_PROPERTY_ICON_SCALE = 0.04f;
    inline const float IN_GAME_CARD_SPELL_PROPERTY_ICON_X_OFFSET = 0.000f;
    inline const float IN_GAME_CARD_SPELL_PROPERTY_ICON_Y_OFFSET = 0.035f;
    inline const float IN_GAME_CARD_PROPERTY_ICON_X_OFFSET = -0.015f;
    inline const float IN_GAME_CARD_PROPERTY_ICON_Y_OFFSET = 0.03f;
    inline const float IN_GAME_CARD_PROPERTY_X_OFFSET = -0.0158f;
    inline const float IN_GAME_CARD_PROPERTY_SCALE = 0.00015f;
    inline const float IN_GAME_CARD_PROPERTY_Y_OFFSET = 0.0285f;
    inline const float IN_GAME_CARD_NAME_X_OFFSET = 0.002f;
    inline const float IN_GAME_CARD_NAME_Y_OFFSET = -0.0145f;
    inline const float IN_GAME_CARD_NAME_SPELL_Y_OFFSET = -0.0125f;
    inline const float IN_GAME_CARD_NAME_SPELL_EFFECT_Y_OFFSET = -0.0205f;
    inline const float IN_GAME_CARD_NAME_SCALE = 0.00012f;
    inline const float IN_GAME_CARD_WIDTH = 0.055f;
    inline const float IN_GAME_CARD_ON_BOARD_WIDTH = 0.045f;
    inline const float IN_GAME_CARD_PUSH_VALUE = 0.003f;
    inline const float IN_GAME_TOP_PLAYER_HELD_CARD_Y = 0.132f;
    inline const float IN_GAME_BOT_PLAYER_HELD_CARD_Y = -0.132f;
    inline const float IN_GAME_TOP_PLAYER_BOARD_CARD_Y = 0.035f;
    inline const float IN_GAME_BOT_PLAYER_BOARD_CARD_Y = -0.035f;
    inline const float IN_GAME_BOT_PLAYER_SELECTED_CARD_Y_OFFSET = 0.039f;
    inline const float IN_GAME_DRAW_CARD_INIT_X = 0.274f;
    inline const float IN_GAME_DRAW_CARD_TOP_PLAYER_MID_POINT_Y = -0.075f;
    inline const float IN_GAME_DRAW_CARD_BOT_PLAYER_MID_POINT_Y = 0.075f;
    inline const float IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS = 0.75f;
    inline const float IN_GAME_HIGHLIGHTED_CARD_Z = 20.0f;
    inline const float IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS = 0.075f;
    inline const float IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DURATION_SECS = 0.25f;
    inline const float IN_GAME_DRAW_CARD_PUSH_EXISTING_CARDS_ANIMATION_DELAY_SECS = 0.5f;
    inline const float IN_GAME_MOBILE_ONLY_FREE_MOVING_CARD_Y_OFFSET = 0.05f;
    inline const float IN_GAME_PLAYED_CARD_SCALE_FACTOR = 0.666f;
    inline const float ACTION_HIGLIGHTER_Z_OFFSET = -0.05f;
    inline const float ACTION_HIGLIGHTER_PERLIN_TIME_SPEED = 12.595f;
    inline const float ACTION_HIGLIGHTER_PERLIN_RESOLUTION = 312.0f;
    inline const float ACTION_HIGLIGHTER_PERLIN_CLARITY = 5.23f;
    inline const float CARD_LOCATION_EFFECT_Z = 1.0f;
    inline const float CARD_LOCATION_EFFECT_TIME_SPEED = 1.0f;
    inline const float CARD_LOCATION_EFFECT_PERLIN_RESOLUTION = 70.0f;
    inline const float TURN_POINTER_ANIMATION_DURATION_SECS = 0.33f;
    inline const float KILL_SIDE_EFFECT_SCALE_UP_FACTOR = 1.5f;
    inline const float KILL_SIDE_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS = 1.0f;
    inline const float POISON_STACK_SHOW_HIDE_ANIMATION_DURATION_SECS = 1.0f;

    inline const glm::vec3 KILL_SIDE_EFFECT_SCALE = {1/20.0f, 1/20.0f, 1/20.0f};
    inline const glm::vec3 TURN_POINTER_HIGHLIGHTER_SCALE = {0.06f, 0.09f, 0.8f};
    inline const glm::vec3 CARD_HIGHLIGHTER_SCALE = {0.08f, 0.13f, 1.0f};
}

///------------------------------------------------------------------------------------------------

#endif /* GameConstants_h */
