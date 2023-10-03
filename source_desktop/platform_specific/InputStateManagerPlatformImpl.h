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
#include <engine/CoreSystemsEngine.h>

///------------------------------------------------------------------------------------------------

namespace input
{

///------------------------------------------------------------------------------------------------

class InputStateManagerPlatformImpl final: public IInputStateManager
{
    friend struct CoreSystemsEngine::SystemsImpl;
public:
    void VProcessInputEvent(const SDL_Event& event, bool& shouldQuit, bool& windowSizeChange) override;
    const glm::vec2& VGetPointingPos() const override;
    void VUpdate(const float dtMillis) override;
    
private:
    InputStateManagerPlatformImpl() = default;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* InputStateManagerPlatformImpl_h */
