///------------------------------------------------------------------------------------------------
///  InputStateManagerPlatformImpl.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <platform_specific/InputStateManagerPlatformImpl.h>

///------------------------------------------------------------------------------------------------

namespace input
{

///------------------------------------------------------------------------------------------------

const glm::vec2& InputStateManagerPlatformImpl::VGetPointingPos() const
{
    return mPointingPos;
}

///------------------------------------------------------------------------------------------------

bool InputStateManagerPlatformImpl::VIsTouchInputPlatform() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

bool InputStateManagerPlatformImpl::VButtonPressed(const Button button) const
{
    return (mCurrentFrameButtonState & (1 << static_cast<uint8_t>(button))) != 0;
}

///------------------------------------------------------------------------------------------------

bool InputStateManagerPlatformImpl::VButtonTapped(const Button button) const
{
    return VButtonPressed(button) && (mPreviousFrameButtonState & (1 << static_cast<uint8_t>(button))) == 0;
}

///------------------------------------------------------------------------------------------------

void InputStateManagerPlatformImpl::VProcessInputEvent(const SDL_Event& event, bool& shouldQuit, bool& windowSizeChange)
{
    const auto& renderableDimensions = CoreSystemsEngine::GetInstance().GetContextRenderableDimensions();
    shouldQuit = windowSizeChange = false;
    
    //User requests quit
    switch (event.type)
    {
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
        {
            shouldQuit = true;
        } break;
        
        case SDL_WINDOWEVENT:
        {
            if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                windowSizeChange = true;
            }
        }
        break;
        
        case SDL_MOUSEBUTTONDOWN:
        {
            mCurrentFrameButtonState |= (1 << event.button.button);
        } break;
            
        case SDL_MOUSEBUTTONUP:
        {
            mCurrentFrameButtonState ^= (1 << event.button.button);
        } break;
            
        case SDL_MOUSEMOTION:
        {
            mPointingPos = glm::vec2(event.motion.x/renderableDimensions.x, event.motion.y/renderableDimensions.y);
        } break;
            
        case SDL_MOUSEWHEEL:
        {
        } break;
    }
    
    ImGui_ImplSDL2_ProcessEvent(&event);
}

///------------------------------------------------------------------------------------------------

void InputStateManagerPlatformImpl::VUpdate(const float)
{
    mPreviousFrameButtonState = mCurrentFrameButtonState;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
