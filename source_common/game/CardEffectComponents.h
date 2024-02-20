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
using EffectBoardModifierMask = uint32_t;
namespace board_modifier_masks
{
    static constexpr EffectBoardModifierMask NONE                                 = 0x0;
    static constexpr EffectBoardModifierMask KILL_NEXT                            = 0x1;
    static constexpr EffectBoardModifierMask BOARD_SIDE_DEBUFF                    = 0x2;
    static constexpr EffectBoardModifierMask DUPLICATE_NEXT_INSECT                = 0x4;
    static constexpr EffectBoardModifierMask DOUBLE_NEXT_DINO_DAMAGE              = 0x8;
    static constexpr EffectBoardModifierMask DOUBLE_POISON_ATTACKS                = 0x10;
    static constexpr EffectBoardModifierMask PERMANENT_CONTINUAL_WEIGHT_REDUCTION = 0x20;
    static constexpr EffectBoardModifierMask DIG_NO_FAIL                          = 0x40;
    static constexpr EffectBoardModifierMask RODENT_LIFESTEAL                     = 0x80;
    static constexpr EffectBoardModifierMask HEAL_NEXT_DINO_DAMAGE                = 0x100;
    static constexpr EffectBoardModifierMask INSECT_VIRUS                         = 0x200;
    static constexpr EffectBoardModifierMask DEMON_KILL_NEXT                      = 0x400;
};


// Effect components
inline const std::string EFFECT_COMPONENT_DAMAGE                               = "DAMAGE";
inline const std::string EFFECT_COMPONENT_WEIGHT                               = "WEIGHT";
inline const std::string EFFECT_COMPONENT_FAMILY                               = "FAMILY";
inline const std::string EFFECT_COMPONENT_ENEMY_BOARD_DEBUFF                   = "ENEMY_BOARD_DEBUFF";
inline const std::string EFFECT_COMPONENT_DRAW                                 = "DRAW";
inline const std::string EFFECT_COMPONENT_GAIN_1_WEIGHT                        = "GAIN_1_WEIGHT";
inline const std::string EFFECT_COMPONENT_CARD_TOKEN                           = "CARD_TOKEN";
inline const std::string EFFECT_COMPONENT_KILL                                 = "KILL";
inline const std::string EFFECT_COMPONENT_DEMON_KILL                           = "DEMON_KILL";
inline const std::string EFFECT_COMPONENT_BOARD                                = "BOARD";
inline const std::string EFFECT_COMPONENT_HELD                                 = "HELD";
inline const std::string EFFECT_COMPONENT_DUPLICATE_INSECT                     = "DUPLICATE_NEXT_INSECT";
inline const std::string EFFECT_COMPONENT_CLEAR_EFFECTS                        = "CLEAR_EFFECTS";
inline const std::string EFFECT_COMPONENT_DOUBLE_NEXT_DINO_DAMAGE              = "DOUBLE_NEXT_DINO_DAMAGE";
inline const std::string EFFECT_COMPONENT_DOUBLE_POISON_ATTACKS                = "DOUBLE_POISON_ATTACKS";
inline const std::string EFFECT_COMPONENT_PERMANENT_CONTINUAL_WEIGHT_REDUCTION = "PERMANENT_CONTINUAL_WEIGHT_REDUCTION";
inline const std::string EFFECT_COMPONENT_DIG_NO_FAIL                          = "DIG_NO_FAIL";
inline const std::string EFFECT_COMPONENT_DRAW_RANDOM_SPELL                    = "DRAW_RANDOM_SPELL";
inline const std::string EFFECT_COMPONENT_ARMOR                                = "ARMOR";
inline const std::string EFFECT_COMPONENT_TOXIC_BOMB                           = "TOXIC_BOMB";
inline const std::string EFFECT_COMPONENT_RODENT_LIFESTEAL_ON_ATTACKS          = "RODENT_LIFESTEAL_ON_ATTACKS";
inline const std::string EFFECT_COMPONENT_HEAL_NEXT_DINO_DAMAGE                = "HEAL_NEXT_DINO_DAMAGE";
inline const std::string EFFECT_COMPONENT_INSECT_MEGASWARM                     = "INSECT_MEGASWARM";
inline const std::string EFFECT_COMPONENT_INSECT_VIRUS                         = "INSECT_VIRUS";

inline const std::unordered_set<std::string> STATIC_EFFECT_COMPONENT_NAMES =
{
    EFFECT_COMPONENT_DRAW,
    EFFECT_COMPONENT_DAMAGE,
    EFFECT_COMPONENT_WEIGHT,
    EFFECT_COMPONENT_FAMILY,
    EFFECT_COMPONENT_ENEMY_BOARD_DEBUFF,
    EFFECT_COMPONENT_KILL,
    EFFECT_COMPONENT_DEMON_KILL,
    EFFECT_COMPONENT_BOARD,
    EFFECT_COMPONENT_HELD,
    EFFECT_COMPONENT_CLEAR_EFFECTS,
    EFFECT_COMPONENT_DUPLICATE_INSECT,
    EFFECT_COMPONENT_DOUBLE_NEXT_DINO_DAMAGE,
    EFFECT_COMPONENT_DOUBLE_POISON_ATTACKS,
    EFFECT_COMPONENT_PERMANENT_CONTINUAL_WEIGHT_REDUCTION,
    EFFECT_COMPONENT_GAIN_1_WEIGHT,
    EFFECT_COMPONENT_DIG_NO_FAIL,
    EFFECT_COMPONENT_DRAW_RANDOM_SPELL,
    EFFECT_COMPONENT_CARD_TOKEN,
    EFFECT_COMPONENT_ARMOR,
    EFFECT_COMPONENT_TOXIC_BOMB,
    EFFECT_COMPONENT_RODENT_LIFESTEAL_ON_ATTACKS,
    EFFECT_COMPONENT_HEAL_NEXT_DINO_DAMAGE,
    EFFECT_COMPONENT_INSECT_MEGASWARM,
    EFFECT_COMPONENT_INSECT_VIRUS,
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* CardEffectComponents_h */
