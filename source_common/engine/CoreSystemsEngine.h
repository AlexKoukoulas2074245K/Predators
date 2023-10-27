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
#include <functional>
#include <memory>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

namespace input { class IInputStateManager; }
namespace rendering { class AnimationManager; }
namespace rendering { class IRenderer; }
namespace rendering { class FontRepository; }
namespace resources { class ResourceLoadingService; }
namespace scene { class ActiveSceneManager; }

struct SDL_Window;
using SDL_GLContext = void*;

///------------------------------------------------------------------------------------------------

class CoreSystemsEngine final
{
public:
    static CoreSystemsEngine& GetInstance();
    ~CoreSystemsEngine();
    
    CoreSystemsEngine(const CoreSystemsEngine&) = delete;
    CoreSystemsEngine(CoreSystemsEngine&&) = delete;
    const CoreSystemsEngine& operator = (const CoreSystemsEngine&) = delete;
    CoreSystemsEngine& operator = (CoreSystemsEngine&&) = delete;
    
    void Start(std::function<void()> clientInitFunction, std::function<void(const float)> clientUpdateFunction, std::function<void()> clientApplicationMovedToBackgroundFunction, std::function<void()> clientCreateDebugWidgetsFunction);
    
    rendering::AnimationManager& GetAnimationManager();
    rendering::IRenderer& GetRenderer();
    rendering::FontRepository& GetFontRepository();
    input::IInputStateManager& GetInputStateManager();
    scene::ActiveSceneManager& GetActiveSceneManager();
    resources::ResourceLoadingService& GetResourceLoadingService();
    
    SDL_Window& GetContextWindow() const;
    glm::vec2 GetContextRenderableDimensions() const;
    void SpecialEventHandling(SDL_Event& event);
    
    // Public so that subsystems have visibility
    // of this before befriending 
    struct SystemsImpl;
private:
    CoreSystemsEngine() = default;
    void Initialize();
    
private:
    static bool mInitialized;
    
private:
    SDL_Window* mWindow = nullptr;
    SDL_GLContext mContext = nullptr;
    
    std::unique_ptr<SystemsImpl> mSystems;
};

///------------------------------------------------------------------------------------------------

#endif /* CoreSystemsEngine_h */
