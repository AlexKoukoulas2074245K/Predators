///------------------------------------------------------------------------------------------------
///  EventSystem.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/11/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>

///------------------------------------------------------------------------------------------------

namespace events
{

///------------------------------------------------------------------------------------------------

EventSystem& EventSystem::GetInstance()
{
    static EventSystem instance;
    return instance;
}

///------------------------------------------------------------------------------------------------

void EventSystem::UnregisterAllEventsForListener(const IListener *listener)
{
    for (auto& eventEntry: mEventListenerMap)
    {
        for (auto iter = eventEntry.second.begin(); iter != eventEntry.second.end(); )
        {
            if (iter->first == listener)
            {
                iter = eventEntry.second.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

IListener::~IListener()
{
    EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

}
