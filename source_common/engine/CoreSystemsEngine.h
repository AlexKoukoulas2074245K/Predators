///------------------------------------------------------------------------------------------------
///  CoreSystemsEngine.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023
///------------------------------------------------------------------------------------------------

#ifndef CoreSystemsEngine_h
#define CoreSystemsEngine_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/rendering/IRenderer.h>
#include <memory>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

struct SDL_Window;
using SDL_GLContext = void*;

///------------------------------------------------------------------------------------------------

class CoreSystemsEngine final
{
public:
    static CoreSystemsEngine& GetInstance();
    
    CoreSystemsEngine(const CoreSystemsEngine&) = delete;
    CoreSystemsEngine(CoreSystemsEngine&&) = delete;
    const CoreSystemsEngine& operator = (const CoreSystemsEngine&) = delete;
    CoreSystemsEngine& operator = (CoreSystemsEngine&&) = delete;
    
    rendering::IRenderer& VGetRenderer() const;
    SDL_Window& VGetContextWindow() const;
    glm::vec2 VGetContextRenderableDimensions() const;
    void SpecialEventHandling(SDL_Event& event);
    
private:
    CoreSystemsEngine() = default;
    void Initialize();
    
private:
    static bool mInitialized;
    
private:
    SDL_Window* mWindow = nullptr;
    SDL_GLContext mContext = nullptr;
    std::unique_ptr<rendering::IRenderer> mRenderer = nullptr;
};

///------------------------------------------------------------------------------------------------

#endif /* CoreSystemsEngine_h */
