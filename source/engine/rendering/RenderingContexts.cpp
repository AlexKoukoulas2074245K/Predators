///------------------------------------------------------------------------------------------------
///  RenderingContexts.cpp
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 20/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/rendering/OpenGL.h>
#include <engine/rendering/OpenGLRenderer.h>
#include <engine/rendering/RenderingContexts.h>
#include <engine/utils/Logging.h>
#include <engine/utils/OSMessageBox.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

std::unique_ptr<IRenderingContext> RenderingContextHolder::sRenderingContext = nullptr;

///------------------------------------------------------------------------------------------------

SDL_Window& BaseRenderingContext::GetContextWindow() const { return *mWindow; }
glm::vec2 BaseRenderingContext::GetContextRenderableDimensions() const { int w,h; SDL_GL_GetDrawableSize(mWindow, &w, &h); return glm::vec2(w, h); }
IRenderer& BaseRenderingContext::GetRenderer() const { return *mRenderer; }

void BaseRenderingContext::SetContextWindow(SDL_Window* window) { mWindow = window; }
void BaseRenderingContext::SetContext(SDL_GLContext context) { mContext = context; }
void BaseRenderingContext::SetRenderer(std::unique_ptr<IRenderer> renderer) { mRenderer = std::move(renderer); }

///------------------------------------------------------------------------------------------------

bool MacRenderingContext::Init()
{
    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    // .. and window dimensions
    static constexpr int windowWidth = 1000;
    static constexpr int windowHeight = 600;
    
    // Set OpenGL desired attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    
    // Create window
    auto* window = SDL_CreateWindow("Predators", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE);
    
    if(!window)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    // Create OpenGL context
    auto* context = SDL_GL_CreateContext(window);
    if (!context || SDL_GL_MakeCurrent(window, context) != 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return false;
    }
    
    SDL_GL_SetSwapInterval(1);
    
    BaseRenderingContext::SetContextWindow(window);
    BaseRenderingContext::SetContext(context);
    BaseRenderingContext::SetRenderer(std::make_unique<OpenGLRenderer>());
    
    // Enable texture blending
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    // Enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glDepthFunc(GL_LESS));
    
    logging::Log(logging::LogType::INFO, "Vendor     : %s", GL_NO_CHECK_CALL(glGetString(GL_VENDOR)));
    logging::Log(logging::LogType::INFO, "Renderer   : %s", GL_NO_CHECK_CALL(glGetString(GL_RENDERER)));
    logging::Log(logging::LogType::INFO, "Version    : %s", GL_NO_CHECK_CALL(glGetString(GL_VERSION)));
    
    return true;
}

///------------------------------------------------------------------------------------------------

void RenderingContextFactory::CreateRenderingContext()
{
    std::unique_ptr<IRenderingContext> renderingContext = nullptr;
    
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  #ifdef _WIN64
    #error "Unknown Windows 64 platform"
  #else
    #error "Unknown Windows 32 platform"
  #endif
#elif __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_IPHONE_SIMULATOR
    #error "Unknown Apple platform"
  #elif TARGET_OS_MACCATALYST
    #error "Unknown Apple platform"
  #elif TARGET_OS_IPHONE
    #error "Unknown Apple platform"
  #elif TARGET_OS_MAC
    renderingContext = std::make_unique<MacRenderingContext>();
  #else
    #error "Unknown Apple platform"
  #endif
#else
  #error "Unknown compiler"
#endif
    
    assert(renderingContext->Init());
    RenderingContextHolder::SetRenderingContext(std::move(renderingContext));
}

///------------------------------------------------------------------------------------------------

}
///------------------------------------------------------------------------------------------------
