///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Camera.h>
#include <engine/rendering/OpenGL.h>
#include <engine/rendering/RenderingContexts.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/ShaderResource.h>
#include <engine/resloading/TextureResource.h>
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
    
    auto shaderResId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    auto textureResId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "board.png");
    auto meshResId = resources::ResourceLoadingService::GetInstance().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    
    Camera camera;
    float rot = 0.0f;
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
                        camera.Shake();
                        xDownPreviously = true;
                    }
                    else
                    {
                        xDownPreviously = false;
                    }
                    
                } break;
                    
                case SDL_WINDOWEVENT:
                if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    camera.RecalculateMatrices();
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
        }
        
        if (move == 1)
        {
            camera.SetZoomFactor(camera.GetZoomFactor() + 0.05f * dtMillis);
            rot += 0.001f * dtMillis;
            
            if (rot > 1.567f)
            {
                rot = 1.567f;
                move = 0;
            }
        }
        
        switch (cam)
        {
            case 1: camera.SetPosition(glm::vec3(camera.GetPosition().x, camera.GetPosition().y + 0.0001f * dtMillis, camera.GetPosition().z)); break;
            case 2: camera.SetPosition(glm::vec3(camera.GetPosition().x + 0.0001f * dtMillis, camera.GetPosition().y, camera.GetPosition().z)); break;
            case 3: camera.SetPosition(glm::vec3(camera.GetPosition().x, camera.GetPosition().y - 0.0001f * dtMillis, camera.GetPosition().z)); break;
            case 4: camera.SetPosition(glm::vec3(camera.GetPosition().x - 0.0001f * dtMillis, camera.GetPosition().y, camera.GetPosition().z)); break;
            default: break;
        }
        //scene.UpdateScene(propagatedDtMillis);
        camera.Update(dtMillis);
        
        auto windowDimensions = rendering::RenderingContextHolder::GetRenderingContext().GetContextRenderableDimensions();
        
        // Set View Port
        GL_CALL(glViewport(0, 0, windowDimensions.x, windowDimensions.y));
        
        // Set background color
        GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
        
        GL_CALL(glEnable(GL_DEPTH_TEST));
        GL_CALL(glEnable(GL_BLEND));
        
        // Clear buffers
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        GL_CALL(glDisable(GL_CULL_FACE));
        
        
        auto& resService = resources::ResourceLoadingService::GetInstance();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(shaderResId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(meshResId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(textureResId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
                
        glm::mat4 world(1.0f);
    
        world = glm::translate(world, glm::zero<glm::vec3>());
//        world = glm::rotate(world, so.mRotation.x, math::X_AXIS);
//        world = glm::rotate(world, so.mRotation.y, math::Y_AXIS);
        world = glm::rotate(world, rot, math::Z_AXIS);
        world = glm::scale(world, glm::one<glm::vec3>());
        
        currentShader->SetMatrix4fv(strutils::StringId("world"), world);
        currentShader->SetMatrix4fv(strutils::StringId("view"), camera.GetViewMatrix());
        currentShader->SetMatrix4fv(strutils::StringId("proj"), camera.GetProjMatrix());
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));

        // Swap window buffers
        SDL_GL_SwapWindow(rendering::RenderingContextHolder::GetRenderingContext().GetContextWindow());
    }
}

///------------------------------------------------------------------------------------------------
