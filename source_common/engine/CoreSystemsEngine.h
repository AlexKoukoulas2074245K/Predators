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
namespace rendering { class IRenderer; }
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
    
    void Start(std::function<void()> clientInitFunction, std::function<void(const float)> clientUpdateFunction);
    
    rendering::IRenderer& GetRenderer();
    input::IInputStateManager& GetInputStateManager();
    scene::ActiveSceneManager& GetActiveSceneManager();
    
    SDL_Window& GetContextWindow() const;
    glm::vec2 GetContextRenderableDimensions() const;
    void SpecialEventHandling(SDL_Event& event);
    
private:
    CoreSystemsEngine() = default;
    void Initialize();
    
private:
    static bool mInitialized;
    
private:
    SDL_Window* mWindow = nullptr;
    SDL_GLContext mContext = nullptr;
    std::unique_ptr<rendering::IRenderer> mRenderer;
    std::unique_ptr<input::IInputStateManager> mInputStateManager;
    std::unique_ptr<scene::ActiveSceneManager> mActiveSceneManager;
};

///------------------------------------------------------------------------------------------------

#endif /* CoreSystemsEngine_h */
