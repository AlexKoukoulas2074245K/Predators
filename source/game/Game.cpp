///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Camera.h>
#include <engine/rendering/Fonts.h>
#include <engine/rendering/OpenGL.h>
#include <engine/rendering/RenderingContexts.h>
#include <engine/resloading/DataFileResource.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/ShaderResource.h>
#include <engine/resloading/TextureResource.h>
#include <engine/scene/Scene.h>
#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <engine/utils/OSMessageBox.h>
#include <functional>
#include <game/Game.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------



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
    
    // Create appropriate rendering context for current platform
    rendering::RenderingContextFactory::CreateRenderingContext();
    
    // Initialize resource loaders
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
    
    scene::Scene dummyScene;
    dummyScene.mSceneObjects.emplace_back();
    dummyScene.mSceneObjects.back().mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    dummyScene.mSceneObjects.back().mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "board.png");
    dummyScene.mSceneObjects.back().mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    
    std::string texts[6] =
    {
        "AbCdEfGhIjKlMnOpQrStUvWxYz",
        "-----------------------------------------------",
        "ZaBcDeFgHiJkLmNoPqRsTuVwXy",
        "-----------------------------------------------",
        "1234567890!@£$%^&*()-=_+{}",
        "-----------------------------------------------",
    };
    
    float yCursors[6] =
    {
        0.1f,
        0.088f,
        0.0f,
        -0.01f,
        -0.1f,
        -0.11f
    };
    
    for (int i = 0; i < 6; ++i)
    {
        dummyScene.mSceneObjects.emplace_back();
        
        scene::TextSceneObjectData textData;
        textData.mTextScale = 0.00058f;
        textData.mTextPosition = glm::vec3(-0.4f, yCursors[i], 0.1f);
        textData.mFontName = strutils::StringId("font");
        textData.mText = texts[i];
        
        dummyScene.mSceneObjects.back().mSceneObjectTypeData = std::move(textData);
        dummyScene.mSceneObjects.back().mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
        dummyScene.mSceneObjects.back().mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    }
    
    rendering::FontRepository::GetInstance().LoadFont("font");
    
    int move = 0;
    int cam = 0;
    //While application is running
    while(!mIsFinished)
    {
        // Calculate frame delta
        const auto currentMillisSinceInit = static_cast<float>(SDL_GetTicks());  // the number of milliseconds since the SDL library
        const auto dtMillis = currentMillisSinceInit - lastFrameMillisSinceInit; // millis diff between current and last frame
        
        lastFrameMillisSinceInit = currentMillisSinceInit;
        framesAccumulator++;
        secsAccumulator += dtMillis * 0.001f; // dt in seconds;
        
        cam = 0;
        
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
                    static bool xDownPreviously = false;
                    if (e.key.keysym.sym == SDLK_w)
                    {
                        cam = 1;
                    }
                    else if (e.key.keysym.sym == SDLK_d)
                    {
                        cam = 2;
                    }
                    else if (e.key.keysym.sym == SDLK_s)
                    {
                        cam = 3;
                    }
                    else if (e.key.keysym.sym == SDLK_a)
                    {
                        cam = 4;
                    }
                    else if (e.key.keysym.sym == SDLK_x && !xDownPreviously)
                    {
                        dummyScene.mCamera.Shake();
                        xDownPreviously = true;
                    }
                    else
                    {
                        xDownPreviously = false;
                    }
                    
                } break;
                    
                case SDL_WINDOWEVENT:
                if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    dummyScene.mCamera.RecalculateMatrices();
                }
                break;
                    
                case SDL_KEYUP:
                {
                } break;
                    
                case SDL_MOUSEWHEEL:
                {
                    if (e.wheel.y > 0) move = 1;
                } break;
            }
        }
       
        if (secsAccumulator > 1.0f)
        {
            logging::Log(logging::LogType::INFO, "FPS: %d", framesAccumulator);
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
            
            resources::ResourceLoadingService::GetInstance().UnloadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "font.png");
            resources::ResourceLoadingService::GetInstance().UnloadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "font.json");
            rendering::FontRepository::GetInstance().LoadFont("font");
        }
        
        if (move == 1)
        {
            dummyScene.mCamera.SetZoomFactor(dummyScene.mCamera.GetZoomFactor() + 0.05f * dtMillis);
            auto& rot = std::get<scene::DefaultSceneObjectData>(dummyScene.mSceneObjects.front().mSceneObjectTypeData).mRotation.z;
            
            rot += 0.001f * dtMillis;
            if (rot > 1.567f)
            {
                rot = 1.567f;
                move = 0;
            }
        }
        
        switch (cam)
        {
            case 1: dummyScene.mCamera.SetPosition(glm::vec3(dummyScene.mCamera.GetPosition().x, dummyScene.mCamera.GetPosition().y + 0.0001f * dtMillis, dummyScene.mCamera.GetPosition().z)); break;
            case 2: dummyScene.mCamera.SetPosition(glm::vec3(dummyScene.mCamera.GetPosition().x + 0.0001f * dtMillis, dummyScene.mCamera.GetPosition().y, dummyScene.mCamera.GetPosition().z)); break;
            case 3: dummyScene.mCamera.SetPosition(glm::vec3(dummyScene.mCamera.GetPosition().x, dummyScene.mCamera.GetPosition().y - 0.0001f * dtMillis, dummyScene.mCamera.GetPosition().z)); break;
            case 4: dummyScene.mCamera.SetPosition(glm::vec3(dummyScene.mCamera.GetPosition().x - 0.0001f * dtMillis, dummyScene.mCamera.GetPosition().y, dummyScene.mCamera.GetPosition().z)); break;
            default: break;
        }
        
        dummyScene.mCamera.Update(dtMillis);
        
        auto& renderer = rendering::RenderingContextHolder::GetRenderingContext().GetRenderer();
        renderer.BeginRenderPass();
        renderer.RenderScene(dummyScene);
        renderer.EndRenderPass();
    }
}

///------------------------------------------------------------------------------------------------
