///------------------------------------------------------------------------------------------------
///  EventSystem.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef EventSystem_h
#define EventSystem_h

///------------------------------------------------------------------------------------------------

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace events
{

///------------------------------------------------------------------------------------------------

class IListener;

///------------------------------------------------------------------------------------------------

struct IEvent
{
    virtual ~IEvent() = default;
};

///------------------------------------------------------------------------------------------------

class EventSystem final
{
public:
    static EventSystem& GetInstance();
    
    template<class EventType, class... Args>
    void DispatchEvent(Args&&... args)
    {
        EventType event(std::forward<Args>(args)...);
        auto className = typeid(EventType).name();
        if (mEventListenerMap.count(className))
        {
            for (auto callbackEntry: mEventListenerMap.at(className))
            {
                callbackEntry.second(event);
            }
        }
    };
    
    template<class EventType>
    void RegisterForEvent(const IListener* listener, std::function<void(const IEvent&)> callback)
    {
        auto className = typeid(EventType).name();
        mEventListenerMap[className].push_back(std::make_pair(listener, callback));
    }
    
    template<class EventType>
    void UnregisterForEvent(const IListener* listener)
    {
        auto className = typeid(EventType).name();
        auto listenerEventEntryIter = std::find_if(mEventListenerMap[className].begin(), mEventListenerMap[className].end(), [=](const std::pair<const IListener*, std::function<void(const IEvent&)>>& entry)
        {
            return entry.first == listener;
        });
        
        if (listenerEventEntryIter != mEventListenerMap[className].end())
        {
            mEventListenerMap[className].erase(listenerEventEntryIter);
        }
    }
    
    void UnregisterAllEventsForListener(const IListener* listener);
    
private:
    EventSystem() = default;
    
private:
    std::unordered_map<std::string, std::vector<std::pair<const IListener*, std::function<void(const IEvent&)>>>> mEventListenerMap;
};

///------------------------------------------------------------------------------------------------

class IListener
{
public:
    virtual ~IListener();
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* EventSystem_h */
