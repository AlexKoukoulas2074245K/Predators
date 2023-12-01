///------------------------------------------------------------------------------------------------
///  CardEffectComponents.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 15/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef CardEffectComponents_h
#define CardEffectComponents_h

///------------------------------------------------------------------------------------------------

#include <string>
#include <unordered_set>

///------------------------------------------------------------------------------------------------

namespace effects
{

///------------------------------------------------------------------------------------------------
// Effect types
using EffectBoardModifierMask = uint8_t;
namespace board_modifier_masks
{
    static constexpr EffectBoardModifierMask NONE = 0x0;
    static constexpr EffectBoardModifierMask KILL_NEXT = 0x1;
    static constexpr EffectBoardModifierMask BOARD_SIDE_DEBUFF = 0x2;
    static constexpr EffectBoardModifierMask DUPLICATE_NEXT_INSECT = 0x4;
    static constexpr EffectBoardModifierMask DOUBLE_NEXT_DINO_DAMAGE = 0x8;
};


// Effect components
inline const std::string EFFECT_COMPONENT_DAMAGE                  = "DAMAGE";
inline const std::string EFFECT_COMPONENT_WEIGHT                  = "WEIGHT";
inline const std::string EFFECT_COMPONENT_FAMILY                  = "FAMILY";
inline const std::string EFFECT_COMPONENT_ENEMY_BOARD_DEBUFF      = "ENEMY_BOARD_DEBUFF";
inline const std::string EFFECT_COMPONENT_DRAW                    = "DRAW";
inline const std::string EFFECT_COMPONENT_KILL                    = "KILL";
inline const std::string EFFECT_COMPONENT_BOARD                   = "BOARD";
inline const std::string EFFECT_COMPONENT_HELD                    = "HELD";
inline const std::string EFFECT_COMPONENT_DUPLICATE_INSECT        = "DUPLICATE_NEXT_INSECT";
inline const std::string EFFECT_COMPONENT_CLEAR_EFFECTS           = "CLEAR_EFFECTS";
inline const std::string EFFECT_COMPONENT_DOUBLE_NEXT_DINO_DAMAGE = "DOUBLE_NEXT_DINO_DAMAGE";

inline const std::unordered_set<std::string> STATIC_EFFECT_COMPONENT_NAMES =
{
    EFFECT_COMPONENT_DRAW,
    EFFECT_COMPONENT_DAMAGE,
    EFFECT_COMPONENT_WEIGHT,
    EFFECT_COMPONENT_FAMILY,
    EFFECT_COMPONENT_ENEMY_BOARD_DEBUFF,
    EFFECT_COMPONENT_KILL,
    EFFECT_COMPONENT_BOARD,
    EFFECT_COMPONENT_HELD,
    EFFECT_COMPONENT_CLEAR_EFFECTS,
    EFFECT_COMPONENT_DUPLICATE_INSECT,
    EFFECT_COMPONENT_DOUBLE_NEXT_DINO_DAMAGE
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* CardEffectComponents_h */
