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
#include <game/BoardState.h>
#include <game/Game.h>
#include <game/GameConstants.h>
#include <game/Cards.h>
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
    CardDataRepository::GetInstance().LoadCardData();
    
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    systemsEngine.GetFontRepository().LoadFont("font", resources::ResourceReloadMode::DONT_RELOAD);
    
    auto dummyScene = systemsEngine.GetActiveSceneManager().CreateScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto boardSceneObject = dummyScene->CreateSceneObject(strutils::StringId("Board"));
    boardSceneObject->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + "board.png");
    
    boardSceneObject->mRotation.z = math::PI/2.0f;
    dummyScene->GetCamera().SetZoomFactor(120.0f);
    
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

    mGameSessionManager.InitGameSession();
}

///------------------------------------------------------------------------------------------------

void Game::Update(const float dtMillis)
{
//    auto& systemsEngine = CoreSystemsEngine::GetInstance();
//    auto dummyScene = systemsEngine.GetActiveSceneManager().FindScene(strutils::StringId("Dummy"));
//    auto worldTouchPos = systemsEngine.GetInputStateManager().VGetPointingPosInWorldSpace(dummyScene->GetCamera().GetViewMatrix(), dummyScene->GetCamera().GetProjMatrix());
//    logging::Log(logging::LogType::INFO, "World pos %.3f, %.3f", worldTouchPos.x, worldTouchPos.y);
    mGameSessionManager.Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <imgui/backends/imgui_impl_sdl2.h>
    #define CREATE_DEBUG_WIDGETS
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #undef CREATE_DEBUG_WIDGETS
    #else
        #include <imgui/backends/imgui_impl_sdl2.h>
        #define CREATE_DEBUG_WIDGETS
    #endif
#endif

#if ((!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)) && (defined(CREATE_DEBUG_WIDGETS))
void Game::CreateDebugWidgets()
{
    // Create game configs
    static bool printGameActionTransitions = false;
    printGameActionTransitions = mGameSessionManager.GetActionEngine().LoggingActionTransitions();
    ImGui::Begin("Game Runtime", nullptr, GLOBAL_WINDOW_LOCKING);
    ImGui::SeparatorText("General");
    ImGui::Checkbox("Print Action Transitions", &printGameActionTransitions);
    mGameSessionManager.GetActionEngine().SetLoggingActionTransitions(printGameActionTransitions);
    ImGui::End();
    
    // Create Card State Viewer
    ImGui::Begin("Card State Viewer", nullptr, GLOBAL_WINDOW_LOCKING);
    static std::string cardStateName;
    const auto& localPlayerCardSoWrappers = mGameSessionManager.GetCardSoWrappers();
    for (size_t i = 0; i < localPlayerCardSoWrappers.size(); ++i)
    {
        ImGui::SeparatorText(i == 0 ? "Opponent Player" : "Local Player");
        for (size_t j = 0; j < localPlayerCardSoWrappers[i].size(); ++j)
        {
            switch (localPlayerCardSoWrappers[i][j]->mState)
            {
                case CardSoState::MOVING_TO_SET_POSITION: cardStateName = "MOVING_TO_SET_POSITION"; break;
                case CardSoState::IDLE:                   cardStateName = "IDLE"; break;
                case CardSoState::HIGHLIGHTED:            cardStateName = "HIGHLIGHTED"; break;
                case CardSoState::FREE_MOVING:            cardStateName = "FREE_MOVING"; break;
            }
            
            ImGui::Text("%d Card State: %s", static_cast<int>(j), cardStateName.c_str());
        }
    }
    
    ImGui::End();
    
    // Create action generator
    ImGui::Begin("Action Generator", nullptr, GLOBAL_WINDOW_LOCKING);
    const auto& actions = GameActionFactory::GetRegisteredActions();
    
    static size_t currentIndex = 0;
    static std::string activePlayerIndex = "";
    static std::string opponentPlayerHealth = "";
    static std::string opponentPlayerHand = "";
    static std::string opponentPlayerBoard = "";
    static std::string localPlayerHealth = "";
    static std::string localPlayerBoard = "";
    static std::string localPlayerHand = "";
    
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
        mGameSessionManager.GetActionEngine().AddGameAction(strutils::StringId(actions.at(currentIndex)));
        mGameSessionManager.GetActionEngine().Update(1);
    }
    
    const auto& boardState = mGameSessionManager.GetBoardState();
    activePlayerIndex = std::to_string(boardState.GetActivePlayerIndex());
    opponentPlayerHealth = std::to_string(boardState.GetPlayerStates().front().mPlayerHealth);
    opponentPlayerHand = strutils::VecToString(boardState.GetPlayerStates().front().mPlayerHeldCards);
    opponentPlayerBoard = strutils::VecToString(boardState.GetPlayerStates().front().mPlayerBoardCards);
    localPlayerBoard = strutils::VecToString(boardState.GetPlayerStates().back().mPlayerBoardCards);
    localPlayerHand = strutils::VecToString(boardState.GetPlayerStates().back().mPlayerHeldCards);
    localPlayerHealth = std::to_string(boardState.GetPlayerStates().back().mPlayerHealth);
    
    ImGui::PopStyleColor(3);
    ImGui::SeparatorText("Output");
    ImGui::TextWrapped("Active Player %s", activePlayerIndex.c_str());
    ImGui::SeparatorText("Opponent Player Health");
    ImGui::TextWrapped("%s", opponentPlayerHealth.c_str());
    ImGui::SeparatorText("Opponent Player Hand");
    ImGui::TextWrapped("%s", opponentPlayerHand.c_str());
    ImGui::SeparatorText("Opponent Player Board");
    ImGui::TextWrapped("%s", opponentPlayerBoard.c_str());
    ImGui::SeparatorText("Local Player Board");
    ImGui::TextWrapped("%s", localPlayerBoard.c_str());
    ImGui::SeparatorText("Local Player Hand");
    ImGui::TextWrapped("%s", localPlayerHand.c_str());
    ImGui::SeparatorText("Local Player Health");
    ImGui::TextWrapped("%s", localPlayerHealth.c_str());
    ImGui::End();
}
#else
void Game::CreateDebugWidgets()
{
}
#endif

///------------------------------------------------------------------------------------------------
