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

void EventSystem::UnregisterAllEventsForListener(const IListener* listener)
{
    mDeadListeners.emplace_back(listener);
}

///------------------------------------------------------------------------------------------------

IListener::~IListener()
{
    EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

}
