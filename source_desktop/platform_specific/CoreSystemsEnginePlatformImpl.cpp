///------------------------------------------------------------------------------------------------
///  CoreSystemsEnginePlatformImpl.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Fonts.h>
#include <engine/rendering/OpenGL.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/utils/Logging.h>
#include <engine/utils/OSMessageBox.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <platform_specific/RendererPlatformImpl.h>
#include <platform_specific/InputStateManagerPlatformImpl.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

static constexpr int DEFAULT_WINDOW_WIDTH  = 1688;
static constexpr int DEFAULT_WINDOW_HEIGHT = 780;
static constexpr int MIN_WINDOW_WIDTH      = 844;
static constexpr int MIN_WINDOW_HEIGHT     = 390;

///------------------------------------------------------------------------------------------------

bool CoreSystemsEngine::mInitialized = false;

///------------------------------------------------------------------------------------------------

static bool sPrintFPS = false;
static void CreateEngineDebugWidgets();

///------------------------------------------------------------------------------------------------

struct CoreSystemsEngine::SystemsImpl
{
    rendering::RendererPlatformImpl mRenderer;
    rendering::FontRepository mFontRepository;
    input::InputStateManagerPlatformImpl mInputStateManager;
    scene::ActiveSceneManager mActiveSceneManager;
    resources::ResourceLoadingService mResourceLoadingService;
};

///------------------------------------------------------------------------------------------------

CoreSystemsEngine& CoreSystemsEngine::GetInstance()
{
    static CoreSystemsEngine instance;
    if (!instance.mInitialized) instance.Initialize();
    return instance;
}

///------------------------------------------------------------------------------------------------

CoreSystemsEngine::~CoreSystemsEngine()
{
}

///------------------------------------------------------------------------------------------------

void CoreSystemsEngine::Initialize()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return;
    }

    // Create window
    mWindow = SDL_CreateWindow("Predators", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_RESIZABLE);

    // Set minimum window size
    SDL_SetWindowMinimumSize(mWindow, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);

    if (!mWindow)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return;
    }
  
#if __APPLE__
    // Set OpenGL desired attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
#endif
    
    // Create OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext || SDL_GL_MakeCurrent(mWindow, mContext) != 0)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "SDL could not initialize!", SDL_GetError());
        return;
    }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    if (glewInit() != GLEW_OK)
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "GLEW could not initialize!", "GLEW Fatal Error");
        return;
    }
#endif
    
    // Vsync
    SDL_GL_SetSwapInterval(1);

    // Systems Initialization
    mSystems = std::make_unique<SystemsImpl>();
    mSystems->mResourceLoadingService.Initialize();
    
    // Enable texture blending
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Enable depth test
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glDepthFunc(GL_LESS));

    logging::Log(logging::LogType::INFO, "Vendor     : %s", GL_NO_CHECK_CALL(glGetString(GL_VENDOR)));
    logging::Log(logging::LogType::INFO, "Renderer   : %s", GL_NO_CHECK_CALL(glGetString(GL_RENDERER)));
    logging::Log(logging::LogType::INFO, "Version    : %s", GL_NO_CHECK_CALL(glGetString(GL_VERSION)));
    logging::Log(logging::LogType::INFO, "Version    : %s", GL_NO_CHECK_CALL(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(mWindow, mContext);
    ImGui_ImplOpenGL3_Init();
    
    mInitialized = true;
}

///------------------------------------------------------------------------------------------------

void CoreSystemsEngine::Start(std::function<void()> clientInitFunction, std::function<void(const float)> clientUpdateFunction, std::function<void()> clientCreateDebugWidgetsFunction)
{
    clientInitFunction();
    
    //While application is running
    SDL_Event event;
    auto lastFrameMillisSinceInit = 0.0f;
    auto secsAccumulator          = 0.0f;
    auto framesAccumulator        = 0LL;
    
    bool shouldQuit = false;
    
    while(!shouldQuit)
    {
        bool windowSizeChanged = false;
        
        // Calculate frame delta
        const auto currentMillisSinceInit = static_cast<float>(SDL_GetTicks());  // the number of milliseconds since the SDL library
        const auto dtMillis = currentMillisSinceInit - lastFrameMillisSinceInit; // millis diff between current and last frame
        
        lastFrameMillisSinceInit = currentMillisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMillis * 0.001f; // dt in seconds;
        
        //Handle events on queue
        while(SDL_PollEvent(&event) != 0)
        {
            mSystems->mInputStateManager.VProcessInputEvent(event, shouldQuit, windowSizeChanged);
        }
        
        if (windowSizeChanged)
        {
            for (auto& scene: mSystems->mActiveSceneManager.GetScenes())
            {
                scene->GetCamera().RecalculateMatrices();
            }
        }
        
        if (secsAccumulator > 1.0f)
        {
            if (sPrintFPS)
            {
                logging::Log(logging::LogType::INFO, "FPS: %d", framesAccumulator);
            }
            
            framesAccumulator = 0;
            secsAccumulator -= 1.0f;
            
            mSystems->mResourceLoadingService.ReloadMarkedResourcesFromDisk();
            mSystems->mFontRepository.ReloadMarkedFontsFromDisk();
        }

        clientUpdateFunction(dtMillis);
        
        mSystems->mInputStateManager.VUpdate(dtMillis);
        mSystems->mRenderer.VBeginRenderPass();
        
        clientCreateDebugWidgetsFunction();
        CreateEngineDebugWidgets();
        
        for (auto& scene: mSystems->mActiveSceneManager.GetScenes())
        {
            mSystems->mRenderer.VRenderScene(*scene);
        }
        
        mSystems->mRenderer.VEndRenderPass();
    }
}

///------------------------------------------------------------------------------------------------

rendering::IRenderer& CoreSystemsEngine::GetRenderer()
{
    return mSystems->mRenderer;
}

///------------------------------------------------------------------------------------------------

rendering::FontRepository& CoreSystemsEngine::GetFontRepository()
{
    return mSystems->mFontRepository;
}

///------------------------------------------------------------------------------------------------

input::IInputStateManager& CoreSystemsEngine::GetInputStateManager()
{
    return mSystems->mInputStateManager;
}

///------------------------------------------------------------------------------------------------

scene::ActiveSceneManager& CoreSystemsEngine::GetActiveSceneManager()
{
    return mSystems->mActiveSceneManager;
}

///------------------------------------------------------------------------------------------------

resources::ResourceLoadingService& CoreSystemsEngine::GetResourceLoadingService()
{
    return mSystems->mResourceLoadingService;
}

///------------------------------------------------------------------------------------------------

SDL_Window& CoreSystemsEngine::GetContextWindow() const
{
    return *mWindow;
}

///------------------------------------------------------------------------------------------------

glm::vec2 CoreSystemsEngine::GetContextRenderableDimensions() const
{
    int w,h; SDL_GL_GetDrawableSize(mWindow, &w, &h); return glm::vec2(w, h);
}

///------------------------------------------------------------------------------------------------

void CoreSystemsEngine::SpecialEventHandling(SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

///------------------------------------------------------------------------------------------------

void CreateEngineDebugWidgets()
{
    // Create runtime configs
    ImGui::Begin("Runtime Config");
    ImGui::SeparatorText("General");
    ImGui::Checkbox("Print FPS", &sPrintFPS);
    ImGui::End();
}

///------------------------------------------------------------------------------------------------
