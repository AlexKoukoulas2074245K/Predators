///------------------------------------------------------------------------------------------------
///  EventSystem.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 01/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef EventSystem_h
#define EventSystem_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/TypeTraits.h>
#include <game/events/Events.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace events
{

///------------------------------------------------------------------------------------------------

class IListener
{
public:
    virtual ~IListener();
};

///------------------------------------------------------------------------------------------------

class EventSystem final
{
public:
    static EventSystem& GetInstance();
    
    template<typename EventType, class... Args>
    void DispatchEvent(Args&&... args)
    {
        RegisterEvent<EventType>();
        const auto& eventTypeId = GetTypeHash<EventType>();
        
        EventType event(std::forward<Args>(args)...);
        if (!mEventCallbacks<EventType>.empty())
        {
            for (auto callbackIter = mEventCallbacks<EventType>.begin(); callbackIter != mEventCallbacks<EventType>.end();)
            {
                if (mEventIdToDeadListeners[eventTypeId].count(callbackIter->first))
                {
                    mEventIdToDeadListeners[eventTypeId].erase(callbackIter->first);
                    callbackIter = mEventCallbacks<EventType>.erase(callbackIter);
                }
                else
                {
                    callbackIter->second(event);
                    callbackIter++;
                }
            }
        }
    };
    
    template<typename EventType, typename FunctionType>
    [[nodiscard]] std::unique_ptr<IListener> RegisterForEvent(FunctionType callback)
    {
        RegisterEvent<EventType>();
        
        auto listener = std::make_unique<IListener>();
        mEventCallbacks<EventType>.push_back(std::make_pair(listener.get(), callback));
        return listener;
    }
    
    template<typename EventType, typename InstanceType, typename FunctionType>
    void RegisterForEvent(InstanceType* listener, FunctionType callback)
    {
        RegisterEvent<EventType>();
        
        mEventCallbacks<EventType>.push_back(std::make_pair(listener, [listener, callback](const EventType& e){ (listener->*callback)(e); }));
    }
    
    template<typename EventType>
    void UnregisterForEvent(IListener* listener)
    {
        for (auto iter = mEventCallbacks<EventType>.begin(); iter != mEventCallbacks<EventType>.end();)
        {
            if (iter->first == static_cast<const void*>(listener))
            {
                iter = mEventCallbacks<EventType>.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }
    
    void UnregisterAllEventsForListener(const IListener* listener);
    
private:
    template<typename EventType>
    void RegisterEvent()
    {
        auto eventTypeId = GetTypeHash<EventType>();
        mEventIdToDeadListeners[eventTypeId];
    }
    
    EventSystem() = default;
    
private:
    template<typename EventType>
    static inline std::vector<std::pair<IListener*, std::function<void(const EventType&)>>> mEventCallbacks;
    
    std::unordered_map<size_t, std::unordered_set<const IListener*>> mEventIdToDeadListeners;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* EventSystem_h */
