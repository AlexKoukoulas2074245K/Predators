///------------------------------------------------------------------------------------------------
///  InputStateManagerPlatformImpl.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023
///------------------------------------------------------------------------------------------------

#ifndef InputStateManagerPlatformImpl_h
#define InputStateManagerPlatformImpl_h

///------------------------------------------------------------------------------------------------

#include <engine/input/IInputStateManager.h>

///------------------------------------------------------------------------------------------------

namespace input
{

///------------------------------------------------------------------------------------------------

class InputStateManagerPlatformImpl final: public IInputStateManager
{
public:
    void VProcessInputEvent(const SDL_Event& event, bool& shouldQuit, bool& windowSizeChange) override;
    const glm::vec2& VGetPointingPos() const override;
    void VUpdate(const float dtMillis) override;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* InputStateManagerPlatformImpl_h */
