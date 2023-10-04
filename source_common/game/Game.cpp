///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/Camera.h>
#include <engine/rendering/Fonts.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <game/Game.h>
#include <game/gameactions/GameActionEngine.h>

///------------------------------------------------------------------------------------------------

Game::Game(const int argc, char** argv)
{
    if (argc > 0)
    {
        logging::Log(logging::LogType::INFO, "Initializing from CWD : %s", argv[0]);
    }
    
    CoreSystemsEngine::GetInstance().Start([&](){ Init(); }, [&](const float dtMillis){ Update(dtMillis); });
}

///------------------------------------------------------------------------------------------------

void Game::Init()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    systemsEngine.GetFontRepository().LoadFont("font", resources::ResourceReloadMode::DONT_RELOAD);
    
    auto dummyScene = systemsEngine.GetActiveSceneManager().CreateScene(strutils::StringId("Dummy"));
    auto boardSceneObject = dummyScene->CreateSceneObject(strutils::StringId("Board"));
    boardSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    boardSceneObject->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "board.png");
    boardSceneObject->mMeshResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    
    auto cardFrameSceneObject = dummyScene->CreateSceneObject(strutils::StringId("CardFrame"));
    cardFrameSceneObject->mScale.x = cardFrameSceneObject->mScale.y = 0.1f;
    cardFrameSceneObject->mPosition.z = 0.2f;
    cardFrameSceneObject->mPosition.y = 0.1f;
    cardFrameSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
    cardFrameSceneObject->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "card_frame.png");
    cardFrameSceneObject->mMeshResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    
    
    auto uiScene = systemsEngine.GetActiveSceneManager().CreateScene(strutils::StringId("UI"));
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
        auto fontRowSceneObject = uiScene->CreateSceneObject();
        
        scene::TextSceneObjectData textData;
        textData.mFontName = strutils::StringId("font");
        textData.mText = texts[i];
        
        fontRowSceneObject->mSceneObjectTypeData = std::move(textData);
        
        fontRowSceneObject->mPosition = glm::vec3(-0.4f, yCursors[i], 0.1f);
        fontRowSceneObject->mScale = glm::vec3(0.00058f);
        fontRowSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
        fontRowSceneObject->mMeshResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    }
    
    GameActionEngine gameActionEngine(GameActionEngine::EngineOperationMode::HEADLESS);
    gameActionEngine.AddGameAction(strutils::StringId("DrawCardGameAction"));
}

///------------------------------------------------------------------------------------------------

void Game::Update(const float)
{
}

///------------------------------------------------------------------------------------------------
