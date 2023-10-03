///------------------------------------------------------------------------------------------------
///  IInputStateManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023
///------------------------------------------------------------------------------------------------

#ifndef IInputStateManager_h
#define IInputStateManager_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

namespace input
{

///------------------------------------------------------------------------------------------------

class IInputStateManager
{
public:
    virtual ~IInputStateManager() = default;
    virtual void VProcessInputEvent(const SDL_Event& event, bool& shouldQuit, bool& windowSizeChange) = 0;
    virtual const glm::vec2& VGetPointingPos() const = 0;
    virtual void VUpdate(const float dtMillis) = 0;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* IInputStateManager_h */
