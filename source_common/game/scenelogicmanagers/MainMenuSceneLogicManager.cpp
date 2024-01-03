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

static const strutils::StringId BOARD_SCENE_OBJECT_NAME = strutils::StringId("board");
static const strutils::StringId STORY_MODE_BUTTON_NAME = strutils::StringId("story_mode_button");
static const strutils::StringId CONTINUE_STORY_BUTTON_NAME = strutils::StringId("continue_story_button");
static const strutils::StringId NEW_STORY_BUTTON_NAME = strutils::StringId("new_story_button");
static const strutils::StringId QUICK_BATTLE_BUTTON_NAME = strutils::StringId("quick_battle_button");
static const strutils::StringId QUIT_BUTTON_NAME = strutils::StringId("quit_button");
static const strutils::StringId NORMAL_BATTLE_MODE_BUTTON_NAME = strutils::StringId("normal_battle_mode_button");
static const strutils::StringId AI_DEMO_BATTLE_MODE_BUTTON_NAME = strutils::StringId("ai_demo_battle_mode_button");
static const strutils::StringId REPLAY_BATTLE_MODE_BUTTON_NAME = strutils::StringId("replay_battle_mode_button");
static const strutils::StringId START_BATTLE_BUTTON_NAME = strutils::StringId("start_battle_button");
static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId TITLE_SCENE_OBJECT_NAME = strutils::StringId("predators_title");
static const strutils::StringId TOP_DECK_TEXT_SCENE_OBJECT_NAME = strutils::StringId("top_deck_text");
static const strutils::StringId BOT_DECK_TEXT_SCENE_OBJECT_NAME = strutils::StringId("bot_deck_text");
static const strutils::StringId TOP_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("top_deck_container");
static const strutils::StringId BOT_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("bot_deck_container");
static const strutils::StringId NEW_STORY_CONFIRMATION_BUTTON_NAME = strutils::StringId("new_story_confirmation");
static const strutils::StringId NEW_STORY_CANCELLATION_BUTTON_NAME = strutils::StringId("new_story_cancellation");
static const strutils::StringId NEW_STORY_CONFIRMATION_TEXT_TOP_NAME = strutils::StringId("new_story_confirmation_text_top");
static const strutils::StringId NEW_STORY_CONFIRMATION_TEXT_MIDDLE_NAME = strutils::StringId("new_story_confirmation_text_middle");
static const strutils::StringId NEW_STORY_CONFIRMATION_TEXT_BOT_NAME = strutils::StringId("new_story_confirmation_text_bot");

static const glm::vec2 DECK_ENTRY_CUTOFF_VALUES = {-0.01f, 0.25f};
static const glm::vec2 DECK_CONTAINER_CUTOFF_VALUES = {0.05f, 0.15f};
static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 STORY_MODE_BUTTON_POSITION = {-0.109f, 0.09f, 0.1f};
static const glm::vec3 CONTINUE_STORY_BUTTON_POSITION = {-0.142f, 0.09f, 0.1f};
static const glm::vec3 NO_PROGRESS_NEW_STORY_BUTTON_POSITION = {-0.091f, 0.06f, 0.1f};
static const glm::vec3 NEW_STORY_BUTTON_POSITION = {-0.091f, 0.00f, 0.1f};
static const glm::vec3 QUICK_BATTLE_BUTTON_POSITION = {-0.109f, -0.003f, 0.1f};
static const glm::vec3 QUIT_BUTTON_POSITION = {-0.033f, -0.093f, 0.1f};
static const glm::vec3 NORMAL_BATTLE_MODE_BUTTON_POSITION = {-0.254f, 0.086f, 0.1f};
static const glm::vec3 AI_DEMO_BATTLE_MODE_BUTTON_POSITION = {-0.07f, 0.086f, 0.1f};
static const glm::vec3 REPLAY_BATTLE_MODE_BUTTON_POSITION = {0.136f, 0.086f, 0.1f};
static const glm::vec3 START_BATTLE_BUTTON_POSITION = {-0.198f, -0.173f, 0.1f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.082f, -0.173f, 0.1f};
static const glm::vec3 DESELECTED_BUTTON_COLOR = { 1.0f, 1.0f, 1.0f};
static const glm::vec3 SELECTED_BUTTON_COLOR = {0.0f, 0.66f, 0.66f};
static const glm::vec3 TOP_DECK_TEXT_POSITION = {-0.254f, 0.01f, 0.1f};
static const glm::vec3 BOT_DECK_TEXT_POSITION = {-0.250f, -0.068f, 0.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_BUTTON_POSITION = {-0.132f, -0.103f, 23.1f};
static const glm::vec3 NEW_STORY_CANCELLATION_BUTTON_POSITION = {0.036f, -0.103f, 23.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_TEXT_TOP_POSITION = {-0.267f, 0.09f, 23.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_TEXT_MIDDLE_POSITION = {-0.282f, 0.039f, 23.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_TEXT_BOT_POSITION = {-0.205f, -0.012f, 23.1f};

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float DECK_SWIPEABLE_ENTRY_SCALE = 0.075f;
static const float DECK_ENTRY_ALPHA = 0.5f;
static const float DECK_ENTRY_Z = 0.1f;
//static const float INITIAL_CAMERA_ZOOM_FACTOR_OFFSET = 54.065f;
static const float DECK_SELECTED_MAX_SCALE_FACTOR = 1.15f;
static const float DECK_SELECTED_MIN_SCALE_FACTOR = 0.65f;
static const float DECK_SELECTION_ANIMATION_DURATION_SECS = 0.4f;

static const math::Rectangle DECK_SELECTION_CONTAINER_TOP_BOUNDS = {{-0.005f, -0.03f}, {0.24f, 0.04f}};
static const math::Rectangle DECK_SELECTION_CONTAINER_BOT_BOUNDS = {{-0.005f, -0.11f}, {0.24f, -0.04f}};

static const int MIN_DECK_ENTRIES_TO_SCROLL = 4;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::MAIN_MENU_SCENE
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    TITLE_SCENE_OBJECT_NAME,
    BOARD_SCENE_OBJECT_NAME
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

static const std::unordered_map<StoryMapSceneType, strutils::StringId> STORY_MAP_SCENE_TYPE_TO_SCENE_NAME =
{
    { StoryMapSceneType::STORY_MAP, game_constants::STORY_MAP_SCENE },
    { StoryMapSceneType::EVENT, game_constants::EVENT_SCENE },
    { StoryMapSceneType::BATTLE, game_constants::BATTLE_SCENE }
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
    mPreviousSubSceneStack = std::stack<SubSceneType>();
    mActiveSubScene = SubSceneType::NONE;
    mTransitioningToSubScene = false;
    mNeedToSetBoardPositionAndZoomFactor = true;
    mShouldPushToPreviousSceneStack = true;
    InitSubScene(SubSceneType::MAIN, scene);
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    if (mTransitioningToSubScene)
    {
        return;
    }
    
    if (mNeedToSetBoardPositionAndZoomFactor)
    {
        auto boardSceneObject = scene->FindSceneObject(BOARD_SCENE_OBJECT_NAME);
        
        boardSceneObject->mPosition = game_constants::GAME_BOARD_INIT_POSITION;
        boardSceneObject->mRotation = game_constants::GAME_BOARD_INIT_ROTATION;
        
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
        if (containerUpdateResult.mInteractedElementId != -1 && ProgressionDataRepository::GetInstance().GetNextBattleControlType() != BattleControlType::REPLAY)
        {
            DeckSelected(containerUpdateResult.mInteractedElementId, true);
        }
    }
    
    if (mCardFamilyContainerBot)
    {
        auto containerUpdateResult = mCardFamilyContainerBot->Update(dtMillis);
        if (containerUpdateResult.mInteractedElementId != -1 && ProgressionDataRepository::GetInstance().GetNextBattleControlType() != BattleControlType::REPLAY)
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
    
    if (!mShouldPushToPreviousSceneStack)
    {
        mShouldPushToPreviousSceneStack = true;
    }
    else
    {
        mPreviousSubSceneStack.push(mActiveSubScene);
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
                STORY_MODE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Story Mode",
                STORY_MODE_BUTTON_NAME,
                [=](){ TransitionToSubScene(SubSceneType::STORY_MODE, scene); },
                *scene
            ));
            
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
           
        case SubSceneType::STORY_MODE:
        {
            bool progressExists = ProgressionDataRepository::GetInstance().GetStoryMapGenerationSeed() != 0;
            if (progressExists)
            {
                mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
                (
                    CONTINUE_STORY_BUTTON_POSITION,
                    BUTTON_SCALE,
                    game_constants::DEFAULT_FONT_NAME,
                    "Continue Story",
                    CONTINUE_STORY_BUTTON_NAME,
                    [=](){ events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(STORY_MAP_SCENE_TYPE_TO_SCENE_NAME.at(ProgressionDataRepository::GetInstance().GetCurrentStoryMapSceneType()), SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE); },
                    *scene
                ));
                
                mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
                (
                    NEW_STORY_BUTTON_POSITION,
                    BUTTON_SCALE,
                    game_constants::DEFAULT_FONT_NAME,
                    "New Story",
                    NEW_STORY_BUTTON_NAME,
                    [=](){ TransitionToSubScene(SubSceneType::NEW_STORY_CONFIRMATION, scene); },
                    *scene
                ));
            }
            else
            {
                mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
                (
                    NO_PROGRESS_NEW_STORY_BUTTON_POSITION,
                    BUTTON_SCALE,
                    game_constants::DEFAULT_FONT_NAME,
                    "New Story",
                    NEW_STORY_BUTTON_NAME,
                    [=](){ InitializeNewStoryData(); },
                    *scene
                ));
            }
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                BACK_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Back",
                BACK_BUTTON_NAME,
                [=](){ GoToPreviousSubScene(scene); },
                *scene
            ));
        } break;
            
        case SubSceneType::NEW_STORY_CONFIRMATION:
        {
            scene::TextSceneObjectData textDataNewStoryTop;
            textDataNewStoryTop.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataNewStoryTop.mText = "Are you sure you want to start";
            auto textNewStoryTopSceneObject = scene->CreateSceneObject(NEW_STORY_CONFIRMATION_TEXT_TOP_NAME);
            textNewStoryTopSceneObject->mSceneObjectTypeData = std::move(textDataNewStoryTop);
            textNewStoryTopSceneObject->mPosition = NEW_STORY_CONFIRMATION_TEXT_TOP_POSITION;
            textNewStoryTopSceneObject->mScale = BUTTON_SCALE;
            
            scene::TextSceneObjectData textDataNewStoryMid;
            textDataNewStoryMid.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataNewStoryMid.mText = "a new story? Your active story";
            auto textNewStoryMidSceneObject = scene->CreateSceneObject(NEW_STORY_CONFIRMATION_TEXT_MIDDLE_NAME);
            textNewStoryMidSceneObject->mSceneObjectTypeData = std::move(textDataNewStoryMid);
            textNewStoryMidSceneObject->mPosition = NEW_STORY_CONFIRMATION_TEXT_MIDDLE_POSITION;
            textNewStoryMidSceneObject->mScale = BUTTON_SCALE;
            
            scene::TextSceneObjectData textDataNewStoryBot;
            textDataNewStoryBot.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataNewStoryBot.mText = " progress will be lost.";
            auto textNewStoryBotSceneObject = scene->CreateSceneObject(NEW_STORY_CONFIRMATION_TEXT_BOT_NAME);
            textNewStoryBotSceneObject->mSceneObjectTypeData = std::move(textDataNewStoryBot);
            textNewStoryBotSceneObject->mPosition = NEW_STORY_CONFIRMATION_TEXT_BOT_POSITION;
            textNewStoryBotSceneObject->mScale = BUTTON_SCALE;
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                NEW_STORY_CONFIRMATION_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Yes",
                NEW_STORY_CONFIRMATION_BUTTON_NAME,
                [=](){ InitializeNewStoryData(); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                NEW_STORY_CANCELLATION_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Cancel",
                NEW_STORY_CANCELLATION_BUTTON_NAME,
                [=]() { GoToPreviousSubScene(scene); },
                *scene
            ));
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
                [=](){ GoToPreviousSubScene(scene); },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                START_BATTLE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Start Battle",
                START_BATTLE_BUTTON_NAME,
                [=](){ events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::BATTLE_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE); },
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
    
    scene->FindSceneObject(NORMAL_BATTLE_MODE_BUTTON_NAME)->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = DESELECTED_BUTTON_COLOR;
    scene->FindSceneObject(REPLAY_BATTLE_MODE_BUTTON_NAME)->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = DESELECTED_BUTTON_COLOR;
    scene->FindSceneObject(AI_DEMO_BATTLE_MODE_BUTTON_NAME)->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = DESELECTED_BUTTON_COLOR;
    
    if (!buttonName.isEmpty())
    {
        scene->FindSceneObject(buttonName)->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = SELECTED_BUTTON_COLOR;
        ProgressionDataRepository::GetInstance().SetNextBattleControlType(BATTLE_MODE_BUTTON_NAMES_TO_BATTLE_CONTROL_TYPE.at(buttonName));
        ProgressionDataRepository::GetInstance().SetNextBattleTopPlayerHealth(game_constants::TOP_PLAYER_DEFAULT_HEALTH);
        ProgressionDataRepository::GetInstance().SetNextBattleBotPlayerHealth(game_constants::BOT_PLAYER_DEFAULT_HEALTH);
        ProgressionDataRepository::GetInstance().SetNextBattleTopPlayerInitWeight(game_constants::TOP_PLAYER_DEFAULT_WEIGHT);
        ProgressionDataRepository::GetInstance().SetNextBattleBotPlayerInitWeight(game_constants::BOT_PLAYER_DEFAULT_WEIGHT);
        ProgressionDataRepository::GetInstance().SetNextBattleTopPlayerWeightLimit(game_constants::TOP_PLAYER_DEFAULT_WEIGHT_LIMIT);
        ProgressionDataRepository::GetInstance().SetNextBattleBotPlayerWeightLimit(game_constants::BOT_PLAYER_DEFAULT_WEIGHT_LIMIT);
        ProgressionDataRepository::GetInstance().SetNextStoryOpponentTexturePath("");
        ProgressionDataRepository::GetInstance().SetNextStoryOpponentName("");
        
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
        auto targetScale = glm::vec3(DECK_SWIPEABLE_ENTRY_SCALE * (selectedDeckIndex == i ? DECK_SELECTED_MAX_SCALE_FACTOR : DECK_SELECTED_MIN_SCALE_FACTOR));
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(sceneObject, sceneObject->mPosition, targetScale, DECK_SELECTION_ANIMATION_DURATION_SECS, animation_flags::IGNORE_X_COMPONENT, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
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

void MainMenuSceneLogicManager::GoToPreviousSubScene(std::shared_ptr<scene::Scene> mainScene)
{
    auto previousSubScene = mPreviousSubSceneStack.top();
    mPreviousSubSceneStack.pop();
    mShouldPushToPreviousSceneStack = false;
    TransitionToSubScene(previousSubScene, mainScene);
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::InitializeNewStoryData()
{
    ProgressionDataRepository::GetInstance().ResetStoryData();
    
    ProgressionDataRepository::GetInstance().SetNextBotPlayerDeck(CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::DINOSAURS_FAMILY_NAME));
    ProgressionDataRepository::GetInstance().SetCurrentStoryPlayerDeck(CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::DINOSAURS_FAMILY_NAME));
    
    ProgressionDataRepository::GetInstance().SetStoryMapGenerationSeed(0);
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::STORY_MAP_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
}

///------------------------------------------------------------------------------------------------
