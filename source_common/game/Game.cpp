///------------------------------------------------------------------------------------------------
///  Game.cpp                                                                                        
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <bitset>
#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Camera.h>
#include <engine/rendering/Fonts.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <engine/utils/OSMessageBox.h>
#include <engine/utils/PlatformMacros.h>
#include <game/BoardState.h>
#include <game/Game.h>
#include <game/GameConstants.h>
#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/GameSceneTransitionManager.h>
#include <game/ProgressionDataRepository.h>
#include <game/gameactions/BaseGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameActionFactory.h>
#include <game/scenelogicmanagers/BattleSceneLogicManager.h>
#include <game/scenelogicmanagers/CardSelectionRewardSceneLogicManager.h>
#include <game/scenelogicmanagers/DefeatSceneLogicManager.h>
#include <game/scenelogicmanagers/EventSceneLogicManager.h>
#include <game/scenelogicmanagers/LoadingSceneLogicManager.h>
#include <game/scenelogicmanagers/MainMenuSceneLogicManager.h>
#include <game/scenelogicmanagers/SettingsSceneLogicManager.h>
#include <game/scenelogicmanagers/ShopSceneLogicManager.h>
#include <game/scenelogicmanagers/StoryCardsLibrarySceneLogicManager.h>
#include <game/scenelogicmanagers/StoryMapSceneLogicManager.h>
#include <game/scenelogicmanagers/VisitMapNodeSceneLogicManager.h>
#include <game/scenelogicmanagers/WheelOfFortuneSceneLogicManager.h>
#include <engine/utils/PersistenceUtils.h>
#if defined(MOBILE_FLOW)
#include <platform_specific/IOSUtils.h>
#endif

///------------------------------------------------------------------------------------------------

static const strutils::StringId MAIN_MENU_SCENE = strutils::StringId("main_menu_scene");

///------------------------------------------------------------------------------------------------

Game::Game(const int argc, char** argv)
{
    if (argc > 0)
    {
        logging::Log(logging::LogType::INFO, "Initializing from CWD : %s", argv[0]);
    }
    
    CoreSystemsEngine::GetInstance().Start([&](){ Init(); }, [&](const float dtMillis){ Update(dtMillis); }, [&](){ ApplicationMovedToBackground(); }, [&](){ WindowResize(); }, [&](){ CreateDebugWidgets(); }, [&](){ OnOneSecondElapsed(); });
}

///------------------------------------------------------------------------------------------------

Game::~Game(){}

///------------------------------------------------------------------------------------------------

void Game::Init()
{
    ProgressionDataRepository::GetInstance();
    
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    systemsEngine.GetFontRepository().LoadFont(game_constants::DEFAULT_FONT_NAME.GetString(), resources::ResourceReloadMode::DONT_RELOAD);
    systemsEngine.GetFontRepository().LoadFont(game_constants::DEFAULT_FONT_BLACK_NAME.GetString(), resources::ResourceReloadMode::DONT_RELOAD);
    systemsEngine.GetFontRepository().LoadFont(game_constants::FONT_PLACEHOLDER_DAMAGE_NAME.GetString(), resources::ResourceReloadMode::DONT_RELOAD);
    systemsEngine.GetFontRepository().LoadFont(game_constants::FONT_PLACEHOLDER_WEIGHT_NAME.GetString(), resources::ResourceReloadMode::DONT_RELOAD);
    
    auto& eventSystem = events::EventSystem::GetInstance();
    mSceneChangeEventListener = eventSystem.RegisterForEvent<events::SceneChangeEvent>([=](const events::SceneChangeEvent& event)
    {
        mGameSceneTransitionManager->ChangeToScene(event.mNewSceneName, event.mSceneChangeType, event.mPreviousSceneDestructionType);
    });
    
    mPopModalSceneEventListener = eventSystem.RegisterForEvent<events::PopSceneModalEvent>([=](const events::PopSceneModalEvent&)
    {
        mGameSceneTransitionManager->PopModalScene();
    });
    
    mGameSceneTransitionManager = std::make_unique<GameSceneTransitionManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<BattleSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<CardSelectionRewardSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<DefeatSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<EventSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<SettingsSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<LoadingSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<MainMenuSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<ShopSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<StoryCardsLibrarySceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<StoryMapSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<VisitMapNodeSceneLogicManager>();
    mGameSceneTransitionManager->RegisterSceneLogicManager<WheelOfFortuneSceneLogicManager>();
    
#if defined(MOBILE_FLOW)
    if (ios_utils::IsIPad())
    {
        game_constants::GAME_BOARD_BASED_SCENE_ZOOM_FACTOR = 120.0f;
    }
    else
    {
        game_constants::GAME_BOARD_BASED_SCENE_ZOOM_FACTOR = 130.0f;
    }
#else
    game_constants::GAME_BOARD_BASED_SCENE_ZOOM_FACTOR = 120.0f;
#endif
    
    mGameSceneTransitionManager->ChangeToScene(MAIN_MENU_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------

void Game::Update(const float dtMillis)
{
//    auto& systemsEngine = CoreSystemsEngine::GetInstance();
//    auto scene = systemsEngine.GetSceneManager().FindScene(game_constants::MAIN_MENU_SCENE);
//
//    if (systemsEngine.GetInputStateManager().VButtonTapped(input::Button::MAIN_BUTTON))
//    {
//        auto touchPos = systemsEngine.GetInputStateManager().VGetPointingPosInWorldSpace(scene->GetCamera().GetViewMatrix(), scene->GetCamera().GetProjMatrix());
//        auto& systemsEngine = CoreSystemsEngine::GetInstance();
//        //auto scene = systemsEngine.GetSceneManager().FindScene(game_constants::M);
//
//        //auto targetPosition = game_constants::HEALTH_CRYSTAL_TOP_POSITION;
//        static bool large = false;
//        large = !large;
//
//        auto animatedNodePathParticleEmitterSceneObject = systemsEngine.GetParticleManager().CreateParticleEmitterAtPosition(large ? strutils::StringId("health_gain_large") :  strutils::StringId("health_gain_small"), glm::vec3(touchPos.x, touchPos.y, 0.1f), *systemsEngine.GetSceneManager().FindScene(game_constants::MAIN_MENU_SCENE), large ? strutils::StringId("test_large") : strutils::StringId("test_small"), [=](float dtMillis, scene::ParticleEmitterObjectData& particleEmitterData)
//        {
//            auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
//            auto scene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::MAIN_MENU_SCENE);
//
//            auto targetRespawnSecs = 0.2f;
//            auto particleEmitterSceneObject = scene->FindSceneObject(strutils::StringId("test_large"));
//
//            static float timeAccum = 0.0f;
//            timeAccum += dtMillis/1000.0f;
//
//            if (timeAccum > targetRespawnSecs)
//            {
//                timeAccum = 0.0f;
//                auto newParticleIndex = particleManager.SpawnParticleAtFirstAvailableSlot(*particleEmitterSceneObject);
//
//                particleEmitterData.mParticleLifetimeSecs[newParticleIndex] = math::RandomFloat(0.01f, 0.1f);
//                particleEmitterData.mParticleVelocities[newParticleIndex].y = 0.0001f;
//                particleEmitterData.mParticleAngles[newParticleIndex] = 1.0f;
//            }
//
//            for (auto i = 0U; i < particleEmitterData.mParticleCount; ++i)
//            {
//                if (particleEmitterData.mParticleLifetimeSecs[i] <= 0.0f)
//                {
//                    continue;
//                }
//
//                if (particleEmitterData.mParticleAngles[i] > 0.0f)
//                {
//                    particleEmitterData.mParticleLifetimeSecs[i] += dtMillis * 0.001f;
//                    if (particleEmitterData.mParticleLifetimeSecs[i] > 1.0f)
//                    {
//                        particleEmitterData.mParticleAngles[i] = -1.0f;
//                    }
//                }
//                else
//                {
//                    particleEmitterData.mParticleLifetimeSecs[i] = math::Max(0.01f, particleEmitterData.mParticleLifetimeSecs[i] - dtMillis * 0.001f);
//                }
//
//                particleEmitterData.mParticlePositions[i] += particleEmitterData.mParticleVelocities[i] * dtMillis;
//            }
//        });
//    }
    
    mGameSceneTransitionManager->Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

void Game::ApplicationMovedToBackground()
{
    ProgressionDataRepository::GetInstance().FlushStateToFile();
    events::EventSystem::GetInstance().DispatchEvent<events::ApplicationMovedToBackgroundEvent>();
}

///------------------------------------------------------------------------------------------------

void Game::OnOneSecondElapsed()
{
    if (ProgressionDataRepository::GetInstance().IsCurrentlyPlayingStoryMode())
    {
        ProgressionDataRepository::GetInstance().SetCurrentStorySecondPlayed(ProgressionDataRepository::GetInstance().GetCurrentStorySecondsPlayed() + 1);
    }
}

///------------------------------------------------------------------------------------------------

void Game::WindowResize()
{
    events::EventSystem::GetInstance().DispatchEvent<events::WindowResizeEvent>();
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
static void CreateImGuiCardVecEntry(const std::string& cardIdPrefix, std::string& cardVec, const std::vector<CardStatOverrides>& cardOverrides, const effects::EffectBoardModifierMask boardModifierMask, const bool forRemotePlayer)
{
    cardVec.erase(cardVec.begin());
    cardVec.erase(cardVec.end() - 1);
    auto splitByNewLine = strutils::StringSplit(cardVec, ',');
    ImGui::Text("[");
    ImGui::SameLine();
    for (size_t i = 0; i < splitByNewLine.size(); ++i)
    {
        auto cardData = CardDataRepository::GetInstance().GetCardData(std::stoi(splitByNewLine[i]), forRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX);
        ImGui::PushID((cardIdPrefix + std::to_string(i)).c_str());
        ImGui::Text((i == 0 ? "%s" : ",%s"), splitByNewLine[i].c_str());
        
        if (cardData.IsSpell())
        {
            ImGui::SetItemTooltip("(Name: %s, Family: %s, Effect: %s, Weight: %d)",
                                  cardData.mCardName.c_str(),
                                  cardData.mCardFamily.GetString().c_str(),
                                  cardData.mCardEffect.c_str(),
                                  cardData.mCardWeight);
        }
        else
        {
            ImGui::SetItemTooltip("(Name: %s, Family: %s, Damage: %d, Weight: %d)",
                                  cardData.mCardName.c_str(),
                                  cardData.mCardFamily.GetString().c_str(),
                                  cardData.mCardDamage,
                                  cardData.mCardWeight);
        }
        
        ImGui::PopID();
        ImGui::SameLine();
    }
    ImGui::Text("]");
    ImGui::NewLine();
    
    if (!cardOverrides.empty())
    {
        std::stringstream overridesString;
        overridesString << "[";
        for (size_t i = 0; i < cardOverrides.size(); ++i)
        {
            if (i != 0)
            {
                overridesString << ", ";
            }
            
            overridesString << i << ":{";
            bool hasSeenFirstInnerEntry = false;
            for (const auto& statOverrideEntry: cardOverrides[i])
            {
                if (hasSeenFirstInnerEntry)
                {
                    overridesString << ", ";
                }
                else
                {
                    hasSeenFirstInnerEntry = true;
                }
                
                switch (statOverrideEntry.first)
                {
                    case CardStatType::DAMAGE: overridesString << "DAMAGE="; break;
                    case CardStatType::WEIGHT: overridesString << "WEIGHT="; break;
                }
                
                overridesString << statOverrideEntry.second;
            }
            
            overridesString << "}";
        }
        overridesString << "}";
        ImGui::Text("Overrides: %s", overridesString.str().c_str());
    }
    
    if (boardModifierMask > 0)
    {
        std::stringstream maskString;
        maskString << std::bitset<8>(boardModifierMask);
        ImGui::Text("Board Modifier Mask: %s", maskString.str().c_str());
    }
}

void Game::CreateDebugWidgets()
{
    // Create game configs
    static bool printGameActionTransitions = false;
    
    ImGui::Begin("Scene Transitions", nullptr, GLOBAL_IMGUI_WINDOW_FLAGS);
    ImGui::SeparatorText("Active Scene Stack");
    auto activeSceneStack = mGameSceneTransitionManager->GetActiveSceneStack();
    while (!activeSceneStack.empty())
    {
        const auto& topEntry = activeSceneStack.top();
        ImGui::Text("%s: %s", typeid(*topEntry.mActiveSceneLogicManager).name(), topEntry.mActiveSceneName.GetString().c_str());
        activeSceneStack.pop();
    }
    
    ImGui::SeparatorText("Scene Logic Managers");
    const auto& registeredSceneManagers = mGameSceneTransitionManager->GetRegisteredSceneLogicManagers();
    for (const auto& sceneManagerEntry: registeredSceneManagers)
    {
        auto* sceneLogicManager = sceneManagerEntry.mSceneLogicManager.get();
        std::stringstream initStatusStr;
        for (const auto& initStatusEntry: sceneManagerEntry.mSceneInitStatusMap)
        {
            if (!initStatusStr.str().empty())
            {
                initStatusStr << ", ";
            }
            initStatusStr << initStatusEntry.first.GetString() << ":" << (initStatusEntry.second ? "true" : "false");
        }
        ImGui::Text("%s: [%s]", typeid(*sceneLogicManager).name(), initStatusStr.str().c_str());
    }
    ImGui::End();
    
    // Battle specific ImGui windows
    auto* activeSceneLogicManager = mGameSceneTransitionManager->GetActiveSceneLogicManager();
    if (!dynamic_cast<BattleSceneLogicManager*>(activeSceneLogicManager))
    {
        return;
    }
    
    auto& battleSceneLogicManager = *(dynamic_cast<BattleSceneLogicManager*>(activeSceneLogicManager));
    printGameActionTransitions = battleSceneLogicManager.GetActionEngine().LoggingActionTransitions();
    ImGui::Begin("Game Runtime", nullptr, GLOBAL_IMGUI_WINDOW_FLAGS);
    ImGui::SeparatorText("General");
    ImGui::Checkbox("Print Action Transitions", &printGameActionTransitions);
    battleSceneLogicManager.GetActionEngine().SetLoggingActionTransitions(printGameActionTransitions);
    ImGui::End();
    
    // Create Card State Viewer
    ImGui::Begin("Card State Viewer", nullptr, GLOBAL_IMGUI_WINDOW_FLAGS);
    static std::string cardStateName;
    const auto& localPlayerCardSoWrappers = battleSceneLogicManager.GetHeldCardSoWrappers();
    for (size_t i = 0; i < localPlayerCardSoWrappers.size(); ++i)
    {
        ImGui::SeparatorText(i == 0 ? "Remote Player" : "Local Player");
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
    ImGui::Begin("Action Generator", nullptr, GLOBAL_IMGUI_WINDOW_FLAGS);
    const auto& actions = GameActionFactory::GetRegisteredActions();
    
    static size_t currentIndex = 0;
    static std::string activePlayerIndex = "";
    static std::string remotePlayerStats = "";
    static std::string remotePlayerHand = "";
    static std::string remotePlayerBoard = "";
    static std::string localPlayerStats = "";
    static std::string localPlayerBoard = "";
    static std::string localPlayerHand = "";
    static std::unordered_map<std::string, std::string> actionExtraParams;
    
    if (ImGui::BeginCombo(" ", actions.at(currentIndex).GetString().c_str()))
    {
        for (size_t n = 0U; n < actions.size(); n++)
        {
            const bool isSelected = (currentIndex == n);
            if (ImGui::Selectable(actions.at(n).GetString().c_str(), isSelected))
            {
                currentIndex = n;
                actionExtraParams.clear();
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3 / 7.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3 / 7.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3 / 7.0f, 0.8f, 0.8f));
    if (ImGui::Button("Create"))
    {
        battleSceneLogicManager.GetActionEngine().AddGameAction(strutils::StringId(actions.at(currentIndex)), actionExtraParams);
        battleSceneLogicManager.GetActionEngine().Update(1);
    }
    
    // Hacky way of getting all action required params
    auto requiredGameActionExtraParams = GameActionFactory::CreateGameAction(actions.at(currentIndex))->VGetRequiredExtraParamNames();
    for (size_t i = 0; i < requiredGameActionExtraParams.size(); ++i)
    {
        const auto& requiredExtraParam = requiredGameActionExtraParams.at(i);
        ImGui::PushID(("RequiredExtraParam" + std::to_string(i)).c_str());
        if (actionExtraParams.count(requiredExtraParam) == 0)
        {
            actionExtraParams[requiredExtraParam].resize(128);
        }
        
        ImGui::Text("%s", requiredExtraParam.c_str());
        ImGui::SameLine();
        ImGui::InputText("##hidelabel", &actionExtraParams[requiredExtraParam][0], actionExtraParams[requiredExtraParam].size());
        ImGui::PopID();
    }
    
    const auto& boardState = battleSceneLogicManager.GetBoardState();
    activePlayerIndex = std::to_string(boardState.GetActivePlayerIndex());
    remotePlayerStats = "Health: " + std::to_string(boardState.GetPlayerStates().front().mPlayerHealth) +
                       " | Total Weight Ammo: " + std::to_string(boardState.GetPlayerStates().front().mPlayerTotalWeightAmmo) +
                       " | Current Weight Ammo: " + std::to_string(boardState.GetPlayerStates().front().mPlayerCurrentWeightAmmo);
    remotePlayerHand = strutils::VecToString(boardState.GetPlayerStates().front().mPlayerHeldCards);
    remotePlayerBoard = strutils::VecToString(boardState.GetPlayerStates().front().mPlayerBoardCards);
    localPlayerBoard = strutils::VecToString(boardState.GetPlayerStates().back().mPlayerBoardCards);
    localPlayerHand = strutils::VecToString(boardState.GetPlayerStates().back().mPlayerHeldCards);
    localPlayerStats = "Health: " + std::to_string(boardState.GetPlayerStates().back().mPlayerHealth) +
                    " | Total Weight Ammo: " + std::to_string(boardState.GetPlayerStates().back().mPlayerTotalWeightAmmo) +
                    " | Current Weight Ammo: " + std::to_string(boardState.GetPlayerStates().back().mPlayerCurrentWeightAmmo);
    
    ImGui::PopStyleColor(3);
    ImGui::SeparatorText("Output");
    ImGui::TextWrapped("Turn Counter %d", boardState.GetTurnCounter());
    ImGui::TextWrapped("Active Player %s", activePlayerIndex.c_str());
    ImGui::SeparatorText("Remote Player Stats");
    ImGui::TextWrapped("%s", remotePlayerStats.c_str());
    ImGui::SeparatorText("Remote Player Hand");
    CreateImGuiCardVecEntry("RemotePlayerHand", remotePlayerHand, boardState.GetPlayerStates()[0].mPlayerHeldCardStatOverrides, 0, true);
    ImGui::SeparatorText("Remote Player Board");
    CreateImGuiCardVecEntry("RemotePlayerBoard", remotePlayerBoard, boardState.GetPlayerStates()[0].mPlayerBoardCardStatOverrides, boardState.GetPlayerStates()[0].mBoardModifiers.mBoardModifierMask, true);
    ImGui::SeparatorText("Local Player Board");
    CreateImGuiCardVecEntry("LocalPlayerBoard", localPlayerBoard, boardState.GetPlayerStates()[1].mPlayerBoardCardStatOverrides, boardState.GetPlayerStates()[1].mBoardModifiers.mBoardModifierMask, false);
    ImGui::SeparatorText("Local Player Hand");
    CreateImGuiCardVecEntry("LocalPlayerHand", localPlayerHand, boardState.GetPlayerStates()[1].mPlayerHeldCardStatOverrides, 0, false);
    ImGui::SeparatorText("Local Player Stats");
    ImGui::TextWrapped("%s", localPlayerStats.c_str());
    ImGui::End();
}
#else
void Game::CreateDebugWidgets()
{
}
#endif

///------------------------------------------------------------------------------------------------
