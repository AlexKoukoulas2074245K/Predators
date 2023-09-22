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
#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <engine/utils/OSMessageBox.h>
#include <functional>
#include <game/Game.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

inline const strutils::StringId MIN_U_UNIFORM_NAME = strutils::StringId("min_u");
inline const strutils::StringId MIN_V_UNIFORM_NAME = strutils::StringId("min_v");
inline const strutils::StringId MAX_U_UNIFORM_NAME = strutils::StringId("max_u");
inline const strutils::StringId MAX_V_UNIFORM_NAME = strutils::StringId("max_v");

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
    
    rendering::FontRepository::GetInstance().LoadFont("font");
    
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
            
            resources::ResourceLoadingService::GetInstance().UnloadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "font.json");
            rendering::FontRepository::GetInstance().LoadFont("font");
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
        
        currentShader->SetBool(strutils::StringId("texture_sheet"), false);
        currentShader->SetMatrix4fv(strutils::StringId("world"), world);
        currentShader->SetMatrix4fv(strutils::StringId("view"), camera.GetViewMatrix());
        currentShader->SetMatrix4fv(strutils::StringId("proj"), camera.GetProjMatrix());
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
        
        auto fontOpt = rendering::FontRepository::GetInstance().GetFont(strutils::StringId("font"));
        if (fontOpt)
        {
            const auto& font = fontOpt->get();
            
            GL_CALL(glUseProgram(currentShader->GetProgramId()));
            GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
            
            auto* currentTexture = &(resService.GetResource<resources::TextureResource>(font.mFontTextureResourceId));
            GL_CALL(glActiveTexture(GL_TEXTURE0));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
            
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
            
            for (int j = 0; j < 6; ++j)
            {
                auto FindGlyphLambda = [](char c, const rendering::Font& f){ auto glyphIter = f.mGlyphs.find(c); return glyphIter == f.mGlyphs.cend() ? f.mGlyphs.at(' ') : glyphIter->second; };
                
                float xCursor = -0.4f;
                
                for (size_t i = 0; i < texts[j].size(); ++i)
                {
                    const auto& glyph = FindGlyphLambda(texts[j][i], font);
                    
                    glm::vec3 textScale = glm::vec3(0.00058f, 0.00058f, 0.00058f);
                    
                    float targetX = xCursor;
                    float targetY = yCursors[j] - glyph.mYOffsetPixels * textScale.y * 0.5f;
                    
                    world = glm::mat4(1.0f);
                    world = glm::translate(world, glm::vec3(targetX, targetY, 0.1f));
                    world = glm::scale(world, glm::vec3(glyph.mWidthPixels * textScale.x, glyph.mHeightPixels * textScale.y, 1.0f));
                    
                    currentShader->SetBool(strutils::StringId("texture_sheet"), true);
                    currentShader->SetFloat(MIN_U_UNIFORM_NAME, glyph.minU);
                    currentShader->SetFloat(MIN_V_UNIFORM_NAME, glyph.minV);
                    currentShader->SetFloat(MAX_U_UNIFORM_NAME, glyph.maxU);
                    currentShader->SetFloat(MAX_V_UNIFORM_NAME, glyph.maxV);
                    currentShader->SetMatrix4fv(strutils::StringId("world"), world);
                    currentShader->SetMatrix4fv(strutils::StringId("view"), camera.GetViewMatrix());
                    currentShader->SetMatrix4fv(strutils::StringId("proj"), camera.GetProjMatrix());
                    
                    GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
                    
                    if (i != texts[j].size() - 1)
                    {
                        // Since each glyph is rendered with its center as the origin, we advance
                        // half this glyph's width + half the next glyph's width ahead
                        const auto& nextGlyph = FindGlyphLambda(texts[j][i + 1], font);
                        xCursor += (glyph.mWidthPixels * textScale.x) * 0.5f + (nextGlyph.mWidthPixels * textScale.x) * 0.5f;
                        xCursor += glyph.mAdvancePixels * textScale.x;
                    }
                }
            }
        }
        
        // Swap window buffers
        SDL_GL_SwapWindow(rendering::RenderingContextHolder::GetRenderingContext().GetContextWindow());
    }
}

///------------------------------------------------------------------------------------------------
