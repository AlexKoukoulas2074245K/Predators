///------------------------------------------------------------------------------------------------
///  BattleSerializer.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BattleSerializer_h
#define BattleSerializer_h

///------------------------------------------------------------------------------------------------

#include <game/BaseDataFileSerializer.h>
#include <game/events/EventSystem.h>
#include <engine/utils/StringUtils.h>
#include <vector>

///------------------------------------------------------------------------------------------------

class BattleSerializer final: public BaseDataFileSerializer, public events::IListener
{
public:
    BattleSerializer(const int gameSeed, const std::vector<int>& topPlayerDeck, const std::vector<int>& botPlayerDeck);

private:
    void OnSerializableGameActionEvent(const events::SerializableGameActionEvent& event);
};

///------------------------------------------------------------------------------------------------

#endif /* BattleSerializer_h */
