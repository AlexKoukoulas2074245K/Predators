///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
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
#include <game/gameactions/GameActionFactory.h>

///------------------------------------------------------------------------------------------------

Game::Game(const int argc, char** argv)
{
    if (argc > 0)
    {
        logging::Log(logging::LogType::INFO, "Initializing from CWD : %s", argv[0]);
    }
    
    CoreSystemsEngine::GetInstance().Start([&](){ Init(); }, [&](const float dtMillis){ Update(dtMillis); }, [&](){ CreateDebugWidgets(); });
}

///------------------------------------------------------------------------------------------------

Game::~Game()
{
    
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
    
#if __APPLE__
    #include <TargetConditionals.h>
    #if defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_IPHONE)
        dummyScene->GetCamera().SetZoomFactor(dummyScene->GetCamera().GetZoomFactor()*2);
    #endif
#endif
    
    boardSceneObject->mRotation.z = math::PI/2.0f;
    dummyScene->GetCamera().SetZoomFactor(150.0f);
    
//    auto uiScene = systemsEngine.GetActiveSceneManager().CreateScene(strutils::StringId("UI"));
//    std::string texts[6] =
//    {
//        "AbCdEfGhIjKlMnOpQrStUvWxYz",
//        "-----------------------------------------------",
//        "ZaBcDeFgHiJkLmNoPqRsTuVwXy",
//        "-----------------------------------------------",
//        "1234567890!@£$%^&*()-=_+{}",
//        "-----------------------------------------------",
//    };
//
//    float yCursors[6] =
//    {
//        0.1f,
//        0.088f,
//        0.0f,
//        -0.01f,
//        -0.1f,
//        -0.11f
//    };
//
//    for (int i = 0; i < 6; ++i)
//    {
//        auto fontRowSceneObject = uiScene->CreateSceneObject();
//
//        scene::TextSceneObjectData textData;
//        textData.mFontName = strutils::StringId("font");
//        textData.mText = texts[i];
//
//        fontRowSceneObject->mSceneObjectTypeData = std::move(textData);
//
//        fontRowSceneObject->mPosition = glm::vec3(-0.4f, yCursors[i], 0.1f);
//        fontRowSceneObject->mScale = glm::vec3(0.00058f);
//        fontRowSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
//        fontRowSceneObject->mMeshResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
//    }
//
    CreateDebugCards(1);
    mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::ANIMATED);
}

///------------------------------------------------------------------------------------------------

void Game::Update(const float dtMillis)
{
    if (CoreSystemsEngine::GetInstance().GetInputStateManager().VButtonTapped(input::Button::MAIN_BUTTON))
    {
        auto& systemsEngine = CoreSystemsEngine::GetInstance();
        auto dummyScene = systemsEngine.GetActiveSceneManager().FindScene(strutils::StringId("Dummy"));
        
        auto existingCards = dummyScene->FindAllSceneObjectsWithNamePrefixedBy("CardFrame");
        assert(existingCards.size() > 0);
        
        auto nextCount = existingCards.size() + 1;
        CreateDebugCards(static_cast<int>(nextCount));
    }
    mActionEngine->Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

void Game::CreateDebugCards(const int cardCount)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    
    auto dummyScene = systemsEngine.GetActiveSceneManager().FindScene(strutils::StringId("Dummy"));
    
    auto existingCards = dummyScene->FindAllSceneObjectsWithNamePrefixedBy("CardFrame");
    for (auto existingCard: existingCards)
    {
        dummyScene->RemoveSceneObject(existingCard->mName);
    }
    
    float cardWidth = 0.055f;
    
    float cardBlockWidth = cardWidth * cardCount;
    float cardStartX = -cardBlockWidth/2.0f;
    
    for (int i = 0; i < cardCount; ++i)
    {
        auto cardFrameSceneObject = dummyScene->CreateSceneObject(strutils::StringId(std::string("CardFrame") + std::to_string(i)));
        cardFrameSceneObject->mScale.x = cardFrameSceneObject->mScale.y = 0.1f;
        auto targetPosition = cardStartX + i * cardWidth + cardWidth/2;
        
        if (cardCount > 4)
        {
            float pushX = (cardCount - 4) * 0.003f * (math::Abs(i - cardCount/2));
            bool oddCardCount = cardCount % 2 != 0;
            if ((oddCardCount && i != cardCount/2) || !oddCardCount)
            {
                targetPosition += (i < cardCount/2) ? pushX : -pushX;
            }
        }
        
        cardFrameSceneObject->mPosition.x = 0.274f + i * cardWidth/2;
        cardFrameSceneObject->mPosition.y = -0.1f; // -0.064 selected
        cardFrameSceneObject->mPosition.z = 0.2f;
        
        auto midPos = cardFrameSceneObject->mPosition;
        midPos.x = math::Abs(cardFrameSceneObject->mPosition.x - targetPosition)/2.0f;
        midPos.y = 0.1f;
        
        math::BezierCurve curve(std::vector<glm::vec3>{cardFrameSceneObject->mPosition, midPos, glm::vec3(targetPosition, -0.1f, 0.2f)});

        animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(std::vector<std::shared_ptr<scene::SceneObject>>{cardFrameSceneObject}, curve, 1.0f, 0.5f * i), [](){ logging::Log(logging::LogType::INFO, "Finished"); });
//        animationManager.StartAnimation(std::make_unique<rendering::TweenAnimation>(std::vector<std::shared_ptr<scene::SceneObject>>{cardFrameSceneObject}, glm::vec3(targetPosition, -0.1f, 0.2f), 2.0f, math::BounceFunction), [](){ logging::Log(logging::LogType::INFO, "Finished"); });
        
        cardFrameSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + "basic.vs");
        cardFrameSceneObject->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "card_frame.png");
        cardFrameSceneObject->mMeshResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + "quad.obj");
    }
}

///------------------------------------------------------------------------------------------------

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define CREATE_DEBUG_WIDGETS
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #undef CREATE_DEBUG_WIDGETS
    #else
        #define CREATE_DEBUG_WIDGETS
    #endif
#endif

#if (!defined(NDEBUG)) && (defined(CREATE_DEBUG_WIDGETS))
#include <imgui/backends/imgui_impl_sdl2.h>
void Game::CreateDebugWidgets()
{
    // Create game configs
    static bool printGameActionTransitions = false;
    printGameActionTransitions = mActionEngine->LoggingActionTransitions();
    ImGui::Begin("Game Runtime");
    ImGui::SeparatorText("General");
    ImGui::Checkbox("Print Action Transitions", &printGameActionTransitions);
    mActionEngine->SetLoggingActionTransitions(printGameActionTransitions);
    ImGui::End();
    
    // Create action generator
    ImGui::Begin("Action Generator");
    const auto& actions = GameActionFactory::GetRegisteredActions();
    
    static size_t currentIndex = 0;
    static std::string activePlayerIndex = "";
    static std::string topPlayerHealth = "";
    static std::string topPlayerHand = "";
    static std::string topPlayerBoard = "";
    static std::string botPlayerHealth = "";
    static std::string botPlayerBoard = "";
    static std::string botPlayerHand = "";
    
    if (ImGui::BeginCombo(" ", actions.at(currentIndex).GetString().c_str()))
    {
        for (size_t n = 0U; n < actions.size(); n++)
        {
            const bool isSelected = (currentIndex == n);
            if (ImGui::Selectable(actions.at(n).GetString().c_str(), isSelected))
                currentIndex = n;
            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3 / 7.0f, 0.8f, 0.8f));
    if (ImGui::Button("Create"))
    {
        mActionEngine->AddGameAction(strutils::StringId(actions.at(currentIndex)));
        mActionEngine->Update(1);
        const auto& boardState = mActionEngine->GetBoardState();
        activePlayerIndex = std::to_string(boardState.GetActivePlayerIndex());
        topPlayerHealth = std::to_string(boardState.GetPlayerStates().front().mPlayerHealth);
        topPlayerHand = strutils::VecToString(boardState.GetPlayerStates().front().mPlayerHeldCards);
        topPlayerBoard = strutils::VecToString(boardState.GetPlayerStates().front().mPlayerBoardCards);
        botPlayerBoard = strutils::VecToString(boardState.GetPlayerStates().back().mPlayerBoardCards);
        botPlayerHand = strutils::VecToString(boardState.GetPlayerStates().back().mPlayerHeldCards);
        botPlayerHealth = std::to_string(boardState.GetPlayerStates().back().mPlayerHealth);
    }
    ImGui::PopStyleColor(3);
    ImGui::SeparatorText("Output");
    ImGui::TextWrapped("Active Player %s", activePlayerIndex.c_str());
    ImGui::SeparatorText("Top Player Health");
    ImGui::TextWrapped("%s", topPlayerHealth.c_str());
    ImGui::SeparatorText("Top Player Hand");
    ImGui::TextWrapped("%s", topPlayerHand.c_str());
    ImGui::SeparatorText("Top Player Board");
    ImGui::TextWrapped("%s", topPlayerBoard.c_str());
    ImGui::SeparatorText("Bot Player Board");
    ImGui::TextWrapped("%s", botPlayerBoard.c_str());
    ImGui::SeparatorText("Bot Player Hand");
    ImGui::TextWrapped("%s", botPlayerHand.c_str());
    ImGui::SeparatorText("Bot Player Health");
    ImGui::TextWrapped("%s", botPlayerHealth.c_str());
    ImGui::End();
}
#else
void Game::CreateDebugWidgets()
{
}
#endif

///------------------------------------------------------------------------------------------------
