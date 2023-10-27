///------------------------------------------------------------------------------------------------
///  GameSerializer.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameSerializer_h
#define GameSerializer_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

using ExtraActionParams = std::unordered_map<std::string, std::string>;
class GameSerializer final
{
public:
    GameSerializer(const int gameSeed);
    
    void FlushStateToFile();
    void OnGameAction(const strutils::StringId& gameActionName, const ExtraActionParams& extraActionParams);
};

///------------------------------------------------------------------------------------------------

#endif /* GameSerializer_h */
