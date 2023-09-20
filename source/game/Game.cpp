///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/rendering/OpenGL.h>
#include <engine/rendering/RenderingContexts.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <engine/utils/OSMessageBox.h>
#include <game/Game.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

Game::Game(const int argc, char** argv)
    : mIsFinished(false)
{
    if (!InitSystems(argc, argv)) return;
    Run();
}

///------------------------------------------------------------------------------------------------

Game::~Game()
{
    SDL_Quit();
}

///------------------------------------------------------------------------------------------------

bool Game::InitSystems(const int argc, char** argv)
{
    // Log CWD
    if (argc > 0)
    {
        logging::Log(logging::LogType::INFO, "Initializing from CWD : %s", argv[0]);
    }
    
    rendering::RenderingContextFactory::CreateRenderingContext();
    
    resources::ResourceLoadingService::GetInstance();
    
    return true;
}

///------------------------------------------------------------------------------------------------

void Game::Run()
{
    SDL_Event e;
    
    auto lastFrameMillisSinceInit = 0.0f;
    auto secsAccumulator          = 0.0f;
    auto framesAccumulator        = 0LL;
    
    auto resId = resources::ResourceLoadingService::GetInstance().LoadResource("board.png");
    resId++;
    
    //While application is running
    while(!mIsFinished)
    {
        // Calculate frame delta
        const auto currentMillisSinceInit = static_cast<float>(SDL_GetTicks());  // the number of milliseconds since the SDL library
        const auto dtMillis = currentMillisSinceInit - lastFrameMillisSinceInit; // millis diff between current and last frame
        
        lastFrameMillisSinceInit = currentMillisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMillis * 0.001f; // dt in seconds;
        
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            switch (e.type)
            {
                case SDL_QUIT:
                case SDL_APP_TERMINATING:
                {
                    mIsFinished = true;
                } break;
                
                case SDL_KEYDOWN:
                {
                } break;
                case SDL_KEYUP:
                {
                } break;
            }
        }
       
        if (secsAccumulator > 1.0f)
        {
            logging::Log(logging::LogType::INFO, "FPS: %d", framesAccumulator);
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
        }
        
        // Cap inter-frame dt to [10.0f-20.0f]
        float propagatedDtMillis = math::Max(10.0f, math::Min(20.0f, dtMillis));
        (void)propagatedDtMillis;
        //scene.UpdateScene(propagatedDtMillis);
        
        // Set background color
        GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
        
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glEnable(GL_BLEND));
        
        // Clear buffers
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        GL_CALL(glDisable(GL_CULL_FACE));
        
        // Swap window buffers
        SDL_GL_SwapWindow(rendering::RenderingContextHolder::GetRenderingContext().GetContextWindow());
    }
}

///------------------------------------------------------------------------------------------------
