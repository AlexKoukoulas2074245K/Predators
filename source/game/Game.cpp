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
#include <game/gameactions/GameActionEngine.h>
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
    rendering::FontRepository::GetInstance().LoadFont("font", resources::ResourceReloadMode::DONT_RELOAD);
    
    scene::Scene dummyScene;
    auto boardSceneObject = dummyScene.CreateSceneObject();
    boardSceneObject->mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    boardSceneObject->mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "board.png");
    boardSceneObject->mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    
    auto cardFrameSceneObject = dummyScene.CreateSceneObject();
    cardFrameSceneObject->mScale.x = cardFrameSceneObject->mScale.y = 0.1f;
    cardFrameSceneObject->mPosition.z = 0.2f;
    cardFrameSceneObject->mPosition.y = 0.1f;
    cardFrameSceneObject->mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    cardFrameSceneObject->mTextureResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "card_frame.png");
    cardFrameSceneObject->mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    
    
    scene::Scene uiScene;
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
        auto fontRowSceneObject = uiScene.CreateSceneObject();
        
        scene::TextSceneObjectData textData;
        textData.mFontName = strutils::StringId("font");
        textData.mText = texts[i];
        
        fontRowSceneObject->mSceneObjectTypeData = std::move(textData);
        
        fontRowSceneObject->mPosition = glm::vec3(-0.4f, yCursors[i], 0.1f);
        fontRowSceneObject->mScale = glm::vec3(0.00058f);
        fontRowSceneObject->mShaderResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
        fontRowSceneObject->mMeshResourceId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    }
    
    int move = 0;
    int cam = 0;
    
    GameActionEngine gameActionEngine(GameActionEngine::EngineOperationMode::HEADLESS);
    gameActionEngine.AddGameAction(strutils::StringId("DrawCardGameAction"));
    
    //While application is running
    SDL_Event event;
    
    auto lastFrameMillisSinceInit = 0.0f;
    auto secsAccumulator          = 0.0f;
    auto framesAccumulator        = 0LL;
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
        while(SDL_PollEvent(&event) != 0)
        {
            //User requests quit
            switch (event.type)
            {
                case SDL_QUIT:
                case SDL_APP_TERMINATING:
                {
                    mIsFinished = true;
                } break;
                
                case SDL_KEYDOWN:
                {
                    static bool xDownPreviously = false;
                    if (event.key.keysym.sym == SDLK_w)
                    {
                        cam = 1;
                    }
                    else if (event.key.keysym.sym == SDLK_d)
                    {
                        cam = 2;
                    }
                    else if (event.key.keysym.sym == SDLK_s)
                    {
                        cam = 3;
                    }
                    else if (event.key.keysym.sym == SDLK_a)
                    {
                        cam = 4;
                    }
                    else if (event.key.keysym.sym == SDLK_x && !xDownPreviously)
                    {
                        dummyScene.GetCamera().Shake();
                        xDownPreviously = true;
                    }
                    else
                    {
                        xDownPreviously = false;
                    }
                    
                } break;
                    
                case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    dummyScene.GetCamera().RecalculateMatrices();
                    uiScene.GetCamera().RecalculateMatrices();
                }
                break;
                    
                case SDL_KEYUP:
                {
                } break;
                    
                case SDL_MOUSEWHEEL:
                {
                    if (event.wheel.y > 0) move = 1;
                } break;
            }
            
            rendering::RenderingContextHolder::GetRenderingContext().VGetRenderer().SpecialEventHandling(event);
        }
       
        if (secsAccumulator > 1.0f)
        {
            logging::Log(logging::LogType::INFO, "FPS: %d", framesAccumulator);
            framesAccumulator = 0;
            secsAccumulator = 0.0f;
            
            resources::ResourceLoadingService::GetInstance().ReloadMarkedResourcesFromDisk();
            rendering::FontRepository::GetInstance().ReloadMarkedFontsFromDisk();
        }
        
        if (move == 1)
        {
            dummyScene.GetCamera().SetZoomFactor(dummyScene.GetCamera().GetZoomFactor() + 0.05f * dtMillis);
            auto& rot = boardSceneObject->mRotation.z;
            
            rot += 0.001f * dtMillis;
            if (rot > 1.567f)
            {
                rot = 1.567f;
                move = 0;
            }
        }
        
        switch (cam)
        {
            case 1: dummyScene.GetCamera().SetPosition(glm::vec3(dummyScene.GetCamera().GetPosition().x, dummyScene.GetCamera().GetPosition().y + 0.0001f * dtMillis, dummyScene.GetCamera().GetPosition().z)); break;
            case 2: dummyScene.GetCamera().SetPosition(glm::vec3(dummyScene.GetCamera().GetPosition().x + 0.0001f * dtMillis, dummyScene.GetCamera().GetPosition().y, dummyScene.GetCamera().GetPosition().z)); break;
            case 3: dummyScene.GetCamera().SetPosition(glm::vec3(dummyScene.GetCamera().GetPosition().x, dummyScene.GetCamera().GetPosition().y - 0.0001f * dtMillis, dummyScene.GetCamera().GetPosition().z)); break;
            case 4: dummyScene.GetCamera().SetPosition(glm::vec3(dummyScene.GetCamera().GetPosition().x - 0.0001f * dtMillis, dummyScene.GetCamera().GetPosition().y, dummyScene.GetCamera().GetPosition().z)); break;
            default: break;
        }
        
        dummyScene.GetCamera().Update(dtMillis);
        
        auto& renderer = rendering::RenderingContextHolder::GetRenderingContext().VGetRenderer();
        renderer.BeginRenderPass();
        renderer.RenderScene(dummyScene);
        renderer.RenderScene(uiScene);
        renderer.EndRenderPass();
    }
}

///------------------------------------------------------------------------------------------------
