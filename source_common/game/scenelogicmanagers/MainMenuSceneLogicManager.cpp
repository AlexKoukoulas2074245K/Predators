///------------------------------------------------------------------------------------------------
///  MainMenuSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/Logging.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/MainMenuSceneLogicManager.h>
#include <game/ProgressionDataRepository.h>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

static const std::string SELECTABLE_BUTTON_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string DECK_ENTRY_SHADER = "card_family_selection_swipe_entry.vs";
static const std::string DECK_ENTRY_MASK_TEXTURE_FILE_NAME = "trap_mask.png";

static const strutils::StringId PERMANENT_BOARD_SCENE = strutils::StringId("permanent_board_scene");
static const strutils::StringId BOARD_SCENE_OBJECT_NAME = strutils::StringId("board");
static const strutils::StringId QUICK_BATTLE_BUTTON_NAME = strutils::StringId("quick_battle_button");
static const strutils::StringId QUIT_BUTTON_NAME = strutils::StringId("quit_button");
static const strutils::StringId NORMAL_BATTLE_MODE_BUTTON_NAME = strutils::StringId("normal_battle_mode_button");
static const strutils::StringId AI_DEMO_BATTLE_MODE_BUTTON_NAME = strutils::StringId("ai_demo_battle_mode_button");
static const strutils::StringId REPLAY_BATTLE_MODE_BUTTON_NAME = strutils::StringId("replay_battle_mode_button");
static const strutils::StringId START_BATTLE_BUTTON_NAME = strutils::StringId("start_battle_button");
static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId TITLE_SCENE_OBJECT_NAME = strutils::StringId("predators_title");
static const strutils::StringId CUSTOM_COLOR_UNIFORM_NAME = strutils::StringId("custom_color");
static const strutils::StringId TOP_DECK_TEXT_SCENE_OBJECT_NAME = strutils::StringId("top_deck_text");
static const strutils::StringId BOT_DECK_TEXT_SCENE_OBJECT_NAME = strutils::StringId("bot_deck_text");
static const strutils::StringId TOP_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("top_deck_container");
static const strutils::StringId BOT_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("bot_deck_container");

static const glm::vec2 DECK_ENTRY_CUTOFF_VALUES = {-0.01f, 0.25f};
static const glm::vec2 DECK_CONTAINER_CUTOFF_VALUES = {0.05f, 0.15f};
static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 QUICK_BATTLE_BUTTON_POSITION = {-0.109f, 0.02f, 0.1f};
static const glm::vec3 NORMAL_BATTLE_MODE_BUTTON_POSITION = {-0.254f, 0.086f, 0.1f};
static const glm::vec3 AI_DEMO_BATTLE_MODE_BUTTON_POSITION = {-0.07f, 0.086f, 0.1f};
static const glm::vec3 REPLAY_BATTLE_MODE_BUTTON_POSITION = {0.136f, 0.086f, 0.1f};
static const glm::vec3 START_BATTLE_BUTTON_POSITION = {-0.198f, -0.173f, 0.1f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.082f, -0.173f, 0.1f};
static const glm::vec3 QUIT_BUTTON_POSITION = {-0.022f, -0.083f, 0.1f};
static const glm::vec3 DESELECTED_BUTTON_COLOR = { 1.0f, 1.0f, 1.0f};
static const glm::vec3 SELECTED_BUTTON_COLOR = {0.0f, 0.66f, 0.66f};
static const glm::vec3 TOP_DECK_TEXT_POSITION = {-0.254f, 0.01f, 0.1f};
static const glm::vec3 BOT_DECK_TEXT_POSITION = {-0.250f, -0.068f, 0.1f};

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.5f;
static const float DECK_SWIPEABLE_ENTRY_SCALE = 0.075f;
static const float DECK_ENTRY_ALPHA = 0.5f;
static const float DECK_ENTRY_Z = 0.1f;
static const float INITIAL_CAMERA_ZOOM_FACTOR_OFFSET = 54.065f;

static const math::Rectangle DECK_SELECTION_CONTAINER_TOP_BOUNDS = {{-0.005f, -0.03f}, {0.24f, 0.04f}};
static const math::Rectangle DECK_SELECTION_CONTAINER_BOT_BOUNDS = {{-0.005f, -0.11f}, {0.24f, -0.04f}};

static const int MIN_DECK_ENTRIES_TO_SCROLL = 4;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::MAIN_MENU_SCENE
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    TITLE_SCENE_OBJECT_NAME
};

static const std::unordered_map<strutils::StringId, BattleControlType, strutils::StringIdHasher> BATTLE_MODE_BUTTON_NAMES_TO_BATTLE_CONTROL_TYPE =
{
    { NORMAL_BATTLE_MODE_BUTTON_NAME, BattleControlType::AI_TOP_ONLY },
    { AI_DEMO_BATTLE_MODE_BUTTON_NAME, BattleControlType::AI_TOP_BOT },
    { REPLAY_BATTLE_MODE_BUTTON_NAME, BattleControlType::REPLAY }
};

static const std::unordered_map<strutils::StringId, std::string, strutils::StringIdHasher> CARD_FAMILY_NAMES_TO_SELECTION_TEXTURES =
{
    { game_constants::INSECTS_FAMILY_NAME, "insect_duplication.png" },
    { game_constants::RODENTS_FAMILY_NAME, "rodents_attack.png" },
    { game_constants::DINOSAURS_FAMILY_NAME, "mighty_roar.png" }
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& MainMenuSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

MainMenuSceneLogicManager::MainMenuSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

MainMenuSceneLogicManager::~MainMenuSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    CardDataRepository::GetInstance().LoadCardData(true);
    mActiveSubScene = SubSceneType::NONE;
    mTransitioningToSubScene = false;
    mNeedToSetBoardPositionAndZoomFactor = true;
    InitSubScene(SubSceneType::MAIN, scene);
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioningToSubScene)
    {
        return;
    }
    
    if (mNeedToSetBoardPositionAndZoomFactor)
    {
        auto permanentBoardScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(PERMANENT_BOARD_SCENE);
        auto boardSceneObject = permanentBoardScene->FindSceneObject(BOARD_SCENE_OBJECT_NAME);
        
        boardSceneObject->mPosition = game_constants::GAME_BOARD_INIT_POSITION;
        boardSceneObject->mRotation = game_constants::GAME_BOARD_INIT_ROTATION;
        
        permanentBoardScene->GetCamera().SetZoomFactor(game_constants::GAME_BOARD_BASED_SCENE_ZOOM_FACTOR + INITIAL_CAMERA_ZOOM_FACTOR_OFFSET);
        
        mNeedToSetBoardPositionAndZoomFactor = false;
    }
    
    // Animated buttons
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
    
    if (mCardFamilyContainerTop)
    {
        auto containerUpdateResult = mCardFamilyContainerTop->Update(dtMillis);
        if (containerUpdateResult.mInteractedElementId != -1 &&  ProgressionDataRepository::GetInstance().GetNextBattleControlType() != BattleControlType::REPLAY)
        {
            DeckSelected(containerUpdateResult.mInteractedElementId, true);
        }
    }
    
    if (mCardFamilyContainerBot)
    {
        auto containerUpdateResult = mCardFamilyContainerBot->Update(dtMillis);
        if (containerUpdateResult.mInteractedElementId != -1 &&  ProgressionDataRepository::GetInstance().GetNextBattleControlType() != BattleControlType::REPLAY)
        {
            DeckSelected(containerUpdateResult.mInteractedElementId, false);
        }
    }
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::InitSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene)
{
    if (mActiveSubScene == subSceneType)
    {
        return;
    }
    
    mActiveSubScene = subSceneType;
    
    scene->RemoveAllSceneObjectsButTheOnesNamed(STATIC_SCENE_ELEMENTS);
    mAnimatedButtons.clear();
    mCardFamilyContainerTop = nullptr;
    mCardFamilyContainerBot = nullptr;
    
    switch (subSceneType)
    {
        case SubSceneType::MAIN:
        {
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                QUICK_BATTLE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Quick Battle",
                QUICK_BATTLE_BUTTON_NAME,
                [=](){ TransitionToSubScene(SubSceneType::QUICK_BATTLE, scene); },
                *scene
            ));
            
        #if defined(MOBILE_FLOW)
            (void)QUIT_BUTTON_NAME;
            (void)QUIT_BUTTON_POSITION;
        #else
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                QUIT_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Quit",
                QUIT_BUTTON_NAME,
                []()
                {
                    SDL_Event e;
                    e.type = SDL_QUIT;
                    SDL_PushEvent(&e);
                },
                *scene
            ));
        #endif
        } break;
            
        case SubSceneType::QUICK_BATTLE:
        {
            scene::TextSceneObjectData textDataTop;
            textDataTop.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataTop.mText = "Top Deck";
            auto topDeckTextSceneObject = scene->CreateSceneObject(TOP_DECK_TEXT_SCENE_OBJECT_NAME);
            topDeckTextSceneObject->mSceneObjectTypeData = std::move(textDataTop);
            topDeckTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            topDeckTextSceneObject->mPosition = TOP_DECK_TEXT_POSITION;
            topDeckTextSceneObject->mScale = BUTTON_SCALE;
            
            scene::TextSceneObjectData textDataBot;
            textDataBot.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataBot.mText = "Bottom Deck";
            auto botDeckTextSceneObject = scene->CreateSceneObject(BOT_DECK_TEXT_SCENE_OBJECT_NAME);
            botDeckTextSceneObject->mSceneObjectTypeData = std::move(textDataBot);
            botDeckTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            botDeckTextSceneObject->mPosition = BOT_DECK_TEXT_POSITION;
            botDeckTextSceneObject->mScale = BUTTON_SCALE;
            
            mCardFamilyContainerTop = std::make_unique<SwipeableContainer<CardFamilyEntry>>
            (
                SwipeDirection::HORIZONTAL,
                glm::vec3(DECK_SWIPEABLE_ENTRY_SCALE * 2),
                DECK_SELECTION_CONTAINER_TOP_BOUNDS,
                DECK_CONTAINER_CUTOFF_VALUES,
                TOP_DECK_CONTAINER_SCENE_OBJECT_NAME,
                DECK_ENTRY_Z,
                *scene,
                MIN_DECK_ENTRIES_TO_SCROLL
            );
            mCardFamilyContainerBot = std::make_unique<SwipeableContainer<CardFamilyEntry>>
            (
                SwipeDirection::HORIZONTAL,
                glm::vec3(DECK_SWIPEABLE_ENTRY_SCALE * 2),
                DECK_SELECTION_CONTAINER_BOT_BOUNDS,
                DECK_CONTAINER_CUTOFF_VALUES,
                BOT_DECK_CONTAINER_SCENE_OBJECT_NAME,
                DECK_ENTRY_Z,
                *scene,
                MIN_DECK_ENTRIES_TO_SCROLL
            );
            
            for (const auto& cardFamilyEntry: CARD_FAMILY_NAMES_TO_SELECTION_TEXTURES)
            {
                {
                    auto cardFamilyEntrySceneObject = scene->CreateSceneObject();
                    cardFamilyEntrySceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + DECK_ENTRY_SHADER);
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_X_UNIFORM_NAME] = DECK_ENTRY_CUTOFF_VALUES.s;
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_X_UNIFORM_NAME] = DECK_ENTRY_CUTOFF_VALUES.t;
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = DECK_ENTRY_ALPHA;
                    cardFamilyEntrySceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DECK_ENTRY_MASK_TEXTURE_FILE_NAME);
                    cardFamilyEntrySceneObject->mScale = glm::vec3(DECK_SWIPEABLE_ENTRY_SCALE);
                    cardFamilyEntrySceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + cardFamilyEntry.second);
                    
                    CardFamilyEntry topEntry;
                    topEntry.mCardFamilyName = cardFamilyEntry.first;
                    topEntry.mSceneObjects.emplace_back(cardFamilyEntrySceneObject);
                    mCardFamilyContainerTop->AddItem(std::move(topEntry), true);
                }
                
                {
                    auto cardFamilyEntrySceneObject = scene->CreateSceneObject();
                    cardFamilyEntrySceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + DECK_ENTRY_SHADER);
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_X_UNIFORM_NAME] = DECK_ENTRY_CUTOFF_VALUES.s;
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_X_UNIFORM_NAME] = DECK_ENTRY_CUTOFF_VALUES.t;
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = DECK_ENTRY_ALPHA;
                    cardFamilyEntrySceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DECK_ENTRY_MASK_TEXTURE_FILE_NAME);
                    cardFamilyEntrySceneObject->mScale = glm::vec3(DECK_SWIPEABLE_ENTRY_SCALE);
                    cardFamilyEntrySceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + cardFamilyEntry.second);
                    
                    CardFamilyEntry botEntry;
                    botEntry.mCardFamilyName = cardFamilyEntry.first;
                    botEntry.mSceneObjects.emplace_back(cardFamilyEntrySceneObject);
                    mCardFamilyContainerBot->AddItem(std::move(botEntry), true);
                }
            }
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                NORMAL_BATTLE_MODE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Normal",
                NORMAL_BATTLE_MODE_BUTTON_NAME,
                [=](){ BattleModeSelected(NORMAL_BATTLE_MODE_BUTTON_NAME); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                AI_DEMO_BATTLE_MODE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "AI Demo",
                AI_DEMO_BATTLE_MODE_BUTTON_NAME,
                [=](){ BattleModeSelected(AI_DEMO_BATTLE_MODE_BUTTON_NAME); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                REPLAY_BATTLE_MODE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Replay",
                REPLAY_BATTLE_MODE_BUTTON_NAME,
                [=](){ BattleModeSelected(REPLAY_BATTLE_MODE_BUTTON_NAME); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                BACK_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Back",
                BACK_BUTTON_NAME,
                [=](){ TransitionToSubScene(SubSceneType::MAIN, scene); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                START_BATTLE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Start Battle",
                START_BATTLE_BUTTON_NAME,
                [=](){ events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::IN_GAME_BATTLE_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE); },
                *scene
            ));
            
            DeckSelected(0, true);
            DeckSelected(0, false);
            BattleModeSelected(NORMAL_BATTLE_MODE_BUTTON_NAME);
            
            mDeckSelectionSceneObjects.clear();
            mDeckSelectionSceneObjects.push_back(topDeckTextSceneObject);
            mDeckSelectionSceneObjects.push_back(botDeckTextSceneObject);
            
            for (auto& topCardFamilyEntry: mCardFamilyContainerTop->GetItems())
            {
                mDeckSelectionSceneObjects.push_back(topCardFamilyEntry.mSceneObjects.front());
            }
            
            for (auto& botCardFamilyEntry: mCardFamilyContainerBot->GetItems())
            {
                mDeckSelectionSceneObjects.push_back(botCardFamilyEntry.mSceneObjects.front());
            }
            
        } break;
            
        default: break;
    }
    
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }
        
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            mTransitioningToSubScene = false;
        });
    }
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::TransitionToSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene)
{
    mTransitioningToSubScene = true;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            InitSubScene(subSceneType, scene);
        });
    }
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::BattleModeSelected(const strutils::StringId& buttonName)
{
    auto& resourceLoadingService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    auto scene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::MAIN_MENU_SCENE);
    
    scene->FindSceneObject(NORMAL_BATTLE_MODE_BUTTON_NAME)->mShaderResourceId = resourceLoadingService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + SELECTABLE_BUTTON_SHADER_FILE_NAME);
    scene->FindSceneObject(REPLAY_BATTLE_MODE_BUTTON_NAME)->mShaderResourceId = resourceLoadingService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + SELECTABLE_BUTTON_SHADER_FILE_NAME);
    scene->FindSceneObject(AI_DEMO_BATTLE_MODE_BUTTON_NAME)->mShaderResourceId = resourceLoadingService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + SELECTABLE_BUTTON_SHADER_FILE_NAME);
    
    scene->FindSceneObject(NORMAL_BATTLE_MODE_BUTTON_NAME)->mShaderVec3UniformValues[CUSTOM_COLOR_UNIFORM_NAME] = DESELECTED_BUTTON_COLOR;
    scene->FindSceneObject(REPLAY_BATTLE_MODE_BUTTON_NAME)->mShaderVec3UniformValues[CUSTOM_COLOR_UNIFORM_NAME] = DESELECTED_BUTTON_COLOR;
    scene->FindSceneObject(AI_DEMO_BATTLE_MODE_BUTTON_NAME)->mShaderVec3UniformValues[CUSTOM_COLOR_UNIFORM_NAME] = DESELECTED_BUTTON_COLOR;
    
    if (!buttonName.isEmpty())
    {
        scene->FindSceneObject(buttonName)->mShaderVec3UniformValues[CUSTOM_COLOR_UNIFORM_NAME] = SELECTED_BUTTON_COLOR;
        ProgressionDataRepository::GetInstance().SetNextBattleControlType(BATTLE_MODE_BUTTON_NAMES_TO_BATTLE_CONTROL_TYPE.at(buttonName));
        
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        for (auto& deckSelectionSceneObject: mDeckSelectionSceneObjects)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(deckSelectionSceneObject, buttonName == REPLAY_BATTLE_MODE_BUTTON_NAME ? 0.0f : 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [](){});
        }
    }
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::DeckSelected(const int selectedDeckIndex, const bool forTopPlayer)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& respectiveDeckContainer = forTopPlayer ? mCardFamilyContainerTop : mCardFamilyContainerBot;
    
    for (auto i = 0; i < static_cast<int>(respectiveDeckContainer->GetItems().size()); ++i)
    {
        auto& sceneObject = respectiveDeckContainer->GetItems()[i].mSceneObjects.front();
        auto targetScale = glm::vec3(DECK_SWIPEABLE_ENTRY_SCALE * (selectedDeckIndex == i ? 1.15f : 0.65f));
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(sceneObject, sceneObject->mPosition, targetScale, 0.4f, animation_flags::IGNORE_X_COMPONENT, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
    }
    
    if (forTopPlayer)
    {
        ProgressionDataRepository::GetInstance().SetNextTopPlayerDeck(CardDataRepository::GetInstance().GetCardIdsByFamily(respectiveDeckContainer->GetItems()[selectedDeckIndex].mCardFamilyName));
    }
    else
    {
        ProgressionDataRepository::GetInstance().SetNextBotPlayerDeck(CardDataRepository::GetInstance().GetCardIdsByFamily(respectiveDeckContainer->GetItems()[selectedDeckIndex].mCardFamilyName));
    }
}

///------------------------------------------------------------------------------------------------
