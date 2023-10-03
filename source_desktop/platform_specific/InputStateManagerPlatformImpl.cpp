///------------------------------------------------------------------------------------------------
///  InputStateManagerPlatformImpl.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <imgui/backends/imgui_impl_sdl2.h>
#include <platform_specific/InputStateManagerPlatformImpl.h>

///------------------------------------------------------------------------------------------------

namespace input
{

///------------------------------------------------------------------------------------------------

void InputStateManagerPlatformImpl::VProcessInputEvent(const SDL_Event& event, bool& shouldQuit, bool& windowSizeChange)
{
    shouldQuit = windowSizeChange = false;
    
    //User requests quit
    switch (event.type)
    {
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
        {
            shouldQuit = true;
        } break;
        
        case SDL_KEYDOWN:
        {
        } break;
            
        case SDL_WINDOWEVENT:
        {
            if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                windowSizeChange = true;
            }
        }
        break;
            
        case SDL_KEYUP:
        {
        } break;
            
        case SDL_MOUSEWHEEL:
        {
        } break;
    }
    
    ImGui_ImplSDL2_ProcessEvent(&event);
}

///------------------------------------------------------------------------------------------------

const glm::vec2& InputStateManagerPlatformImpl::VGetPointingPos() const
{
    static glm::vec2 test(0.0f);
    return test;
}

///------------------------------------------------------------------------------------------------

void InputStateManagerPlatformImpl::VUpdate(const float)
{
    
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
