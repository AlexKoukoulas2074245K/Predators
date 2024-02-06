///------------------------------------------------------------------------------------------------
///  MainMenuSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 12/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/BaseDataFileDeserializer.h>
#include <engine/utils/Date.h>
#include <engine/utils/Logging.h>
#include <engine/scene/SceneManager.h>
#include <fstream>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/MainMenuSceneLogicManager.h>
#include <game/DataRepository.h>
#include <game/ProductIds.h>
#include <game/ProductRepository.h>
#include <game/utils/GiftingUtils.h>
#include <SDL_events.h>
#if defined(MACOS) || defined(MOBILE_FLOW)
#include <platform_utilities/AppleUtils.h>
#include <platform_utilities/CloudKitUtils.h>
#elif defined(WINDOWS)
#include <platform_utilities/WindowsUtils.h>
#endif

///------------------------------------------------------------------------------------------------

static const std::string SELECTABLE_BUTTON_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string DECK_ENTRY_SHADER = "card_family_selection_swipe_entry.vs";
static const std::string DECK_ENTRY_MASK_TEXTURE_FILE_NAME = "trap_mask.png";

static const strutils::StringId GIFT_CODE_CLAIM_SCENE = strutils::StringId("gift_code_claim_scene");
static const strutils::StringId BOARD_SCENE_OBJECT_NAME = strutils::StringId("board");
static const strutils::StringId STORY_MODE_BUTTON_NAME = strutils::StringId("story_mode_button");
static const strutils::StringId CARD_LIBRARY_BUTTON_NAME = strutils::StringId("card_library_button");
static const strutils::StringId SHOP_BUTTON_NAME = strutils::StringId("shop_button");
static const strutils::StringId CONTINUE_STORY_BUTTON_NAME = strutils::StringId("continue_story_button");
static const strutils::StringId NEW_STORY_BUTTON_NAME = strutils::StringId("new_story_button");
static const strutils::StringId EXTRAS_BUTTON_NAME = strutils::StringId("extras_button");
static const strutils::StringId QUIT_BUTTON_NAME = strutils::StringId("quit_button");
static const strutils::StringId NORMAL_BATTLE_MODE_BUTTON_NAME = strutils::StringId("normal_battle_mode_button");
static const strutils::StringId AI_DEMO_BATTLE_MODE_BUTTON_NAME = strutils::StringId("ai_demo_battle_mode_button");
static const strutils::StringId REPLAY_BATTLE_MODE_BUTTON_NAME = strutils::StringId("replay_battle_mode_button");
static const strutils::StringId ENTER_GIFT_CODE_BUTTON_NAME = strutils::StringId("enter_gift_code_button");
static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId TITLE_SCENE_OBJECT_NAME = strutils::StringId("predators_title");
static const strutils::StringId TOP_DECK_TEXT_SCENE_OBJECT_NAME = strutils::StringId("top_deck_text");
static const strutils::StringId BOT_DECK_TEXT_SCENE_OBJECT_NAME = strutils::StringId("bot_deck_text");
static const strutils::StringId STORY_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("story_deck_container");
static const strutils::StringId TOP_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("top_deck_container");
static const strutils::StringId BOT_DECK_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("bot_deck_container");
static const strutils::StringId NEW_STORY_CONFIRMATION_BUTTON_NAME = strutils::StringId("new_story_confirmation");
static const strutils::StringId NEW_STORY_CANCELLATION_BUTTON_NAME = strutils::StringId("new_story_cancellation");
static const strutils::StringId NEW_STORY_CONFIRMATION_TEXT_TOP_NAME = strutils::StringId("new_story_confirmation_text_top");
static const strutils::StringId NEW_STORY_CONFIRMATION_TEXT_MIDDLE_NAME = strutils::StringId("new_story_confirmation_text_middle");
static const strutils::StringId NEW_STORY_CONFIRMATION_TEXT_BOT_NAME = strutils::StringId("new_story_confirmation_text_bot");
static const strutils::StringId STORY_DECK_SELECTION_PROMPT_SCENE_OBJECT_NAME = strutils::StringId("story_deck_selection_prompt");
static const strutils::StringId START_NEW_STORY_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("start_new_story_button");
static const strutils::StringId STORY_HEALTH_REFILL_PRODUCT_NAME = strutils::StringId("story_health_refill");
static const strutils::StringId NORMAL_PACK_PRODUCT_NAME = strutils::StringId("normal_card_pack");
static const strutils::StringId GOLDEN_PACK_PRODUCT_NAME = strutils::StringId("golden_card_pack");
static const strutils::StringId COINS_S_PRODUCT_NAME = strutils::StringId("coins_s");
static const strutils::StringId COINS_M_PRODUCT_NAME = strutils::StringId("coins_m");
static const strutils::StringId COINS_L_PRODUCT_NAME = strutils::StringId("coins_l");

static const glm::vec2 STORY_DECK_ENTRY_CUTOFF_VALUES = {-0.25f, 0.15f};
static const glm::vec2 STORY_DECK_SELECTION_CONTAINER_CUTOFF_VALUES = {-0.1f, 0.1f};

static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 STORY_MODE_BUTTON_POSITION = {0.0f, 0.09f, 0.1f};
static const glm::vec3 CONTINUE_STORY_BUTTON_POSITION = {-0.142f, 0.09f, 0.1f};
static const glm::vec3 NO_PROGRESS_NEW_STORY_BUTTON_POSITION = {-0.091f, 0.06f, 0.1f};
static const glm::vec3 NEW_STORY_BUTTON_POSITION = {-0.091f, 0.00f, 0.1f};
static const glm::vec3 CARD_LIBRARY_BUTTON_POSITION = {0.0f, 0.02f, 0.1f};
static const glm::vec3 SHOP_BUTTON_POSITION = {0.0f, -0.05f, 0.1f};
static const glm::vec3 EXTRAS_BUTTON_POSITION = {0.0f, -0.110f, 0.1f};
static const glm::vec3 QUIT_BUTTON_POSITION = {0.0f, -0.180f, 0.1f};

static const glm::vec3 ENTER_GIFT_CODE_BUTTON_POSITION = {-0.135f, 0.085f, 0.1f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.082f, -0.173f, 0.1f};
static const glm::vec3 DESELECTED_BUTTON_COLOR = { 1.0f, 1.0f, 1.0f};
static const glm::vec3 SELECTED_BUTTON_COLOR = {0.0f, 0.66f, 0.66f};
static const glm::vec3 NEW_STORY_CONFIRMATION_BUTTON_POSITION = {-0.132f, -0.103f, 23.1f};
static const glm::vec3 NEW_STORY_CANCELLATION_BUTTON_POSITION = {0.036f, -0.103f, 23.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_TEXT_TOP_POSITION = {-0.267f, 0.09f, 23.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_TEXT_MIDDLE_POSITION = {-0.282f, 0.039f, 23.1f};
static const glm::vec3 NEW_STORY_CONFIRMATION_TEXT_BOT_POSITION = {-0.205f, -0.012f, 23.1f};
static const glm::vec3 NEW_STORY_DECK_SELECTION_TEXT_POSITION = {-0.169f, 0.115f, 0.1f};
static const glm::vec3 START_NEW_STORY_BUTTON_POSITION = {-0.058f, -0.145f, 23.1f};

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float DECK_SWIPEABLE_ENTRY_SCALE = 0.075f;
static const float STORY_DECK_SELECTION_ENTRY_SCALE = 0.115f;
static const float DECK_ENTRY_ALPHA = 0.5f;
static const float DECK_ENTRY_Z = 0.1f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;
static const float DECK_SELECTED_MAX_SCALE_FACTOR = 1.15f;
static const float DECK_SELECTED_MIN_SCALE_FACTOR = 0.65f;
static const float DECK_SELECTION_ANIMATION_DURATION_SECS = 0.4f;

static const math::Rectangle STORY_DECK_SELECTION_CONTAINER_TOP_BOUNDS = {{-0.25f, -0.08f}, {0.2f, 0.01f}};

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

static const std::unordered_map<StoryMapSceneType, strutils::StringId> STORY_MAP_SCENE_TYPE_TO_SCENE_NAME =
{
    { StoryMapSceneType::STORY_MAP, game_constants::STORY_MAP_SCENE },
    { StoryMapSceneType::EVENT, game_constants::EVENT_SCENE },
    { StoryMapSceneType::BATTLE, game_constants::BATTLE_SCENE },
    { StoryMapSceneType::SHOP, game_constants::SHOP_SCENE }
};

///------------------------------------------------------------------------------------------------

static bool sEmptyProgression = false;
void CheckForEmptyProgression()
{
    serial::BaseDataFileDeserializer persistentDataFileChecker("persistent", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM);
    sEmptyProgression = persistentDataFileChecker.GetState().empty();
}

///------------------------------------------------------------------------------------------------

#if defined(MACOS) || defined(MOBILE_FLOW)
void OnCloudQueryCompleted(cloudkit_utils::QueryResultData resultData)
{
    if (!resultData.mSuccessfullyQueriedAtLeastOneFileField)
    {
        return;
    }
    
    auto writeDataStringToTempFile = [](const std::string& tempFileNameWithoutExtension, const std::string& data)
    {
        if (!data.empty())
        {
            std::string dataFileExtension = ".json";
            
            auto filePath = apple_utils::GetPersistentDataDirectoryPath() + tempFileNameWithoutExtension + dataFileExtension;
            std::remove(filePath.c_str());
            
            std::ofstream file(filePath);
            if (file.is_open())
            {
                file << data;
                file.close();
            }
        }
    };
    
    auto localDeviceId = apple_utils::GetDeviceId();
    
    auto checkForDeviceIdInconsistency = [=](const std::string& targetDataFileNameWithoutExtension, const serial::BaseDataFileDeserializer& dataFileDeserializer)
    {
        if
        (
            targetDataFileNameWithoutExtension == "persistent" &&
            dataFileDeserializer.GetState().count("device_id") &&
            dataFileDeserializer.GetState().count("device_name") &&
            dataFileDeserializer.GetState().count("timestamp")
        )
        {
            auto deviceId = dataFileDeserializer.GetState().at("device_id").get<std::string>();
            
            using namespace date;
            std::stringstream s;
            s << std::chrono::system_clock::time_point(std::chrono::seconds(dataFileDeserializer.GetState().at("timestamp").get<long>()));
            
            const auto& localSuccessfulTransactions = DataRepository::GetInstance().GetSuccessfulTransactionIds();
            std::vector<std::string> cloudSuccessfulTransactions;
            if (dataFileDeserializer.GetState().count("successful_transaction_ids"))
            {
                cloudSuccessfulTransactions = dataFileDeserializer.GetState()["successful_transaction_ids"].get<std::vector<std::string>>();
            }
            
            // If local progression is ahead in terms of transactions we decline the cloud data
            if (localSuccessfulTransactions.size() > cloudSuccessfulTransactions.size())
            {
                DataRepository::GetInstance().SetForeignProgressionDataFound(ForeignCloudDataFoundType::NONE);
            }
            // If cloud progression is ahead in terms of transactions we mandate the cloud data
            else if (localSuccessfulTransactions.size() < cloudSuccessfulTransactions.size())
            {
                DataRepository::GetInstance().SetForeignProgressionDataFound(ForeignCloudDataFoundType::MANDATORY);
            }
            // Otherwise if the transaction vectors are the same let the user choose
            else
            {
                if (deviceId != localDeviceId)
                {
                    DataRepository::GetInstance().SetForeignProgressionDataFound(ForeignCloudDataFoundType::OPTIONAL);
                }
                else
                {
                    DataRepository::GetInstance().SetForeignProgressionDataFound(ForeignCloudDataFoundType::NONE);
                }
            }
            
            DataRepository::GetInstance().SetCloudDataDeviceNameAndTime("(From " + dataFileDeserializer.GetState().at("device_name").get<std::string>() + " at " + strutils::StringSplit(s.str(), '.')[0] + ")");
            
        }
    };
    
    writeDataStringToTempFile("cloud_persistent", resultData.mPersistentProgressRawString);
    writeDataStringToTempFile("cloud_story", resultData.mStoryProgressRawString);
    writeDataStringToTempFile("cloud_last_battle", resultData.mLastBattleRawString);
    
    checkForDeviceIdInconsistency("persistent", serial::BaseDataFileDeserializer("cloud_persistent", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM));
    checkForDeviceIdInconsistency("story", serial::BaseDataFileDeserializer("cloud_story", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM));
    checkForDeviceIdInconsistency("last_battle", serial::BaseDataFileDeserializer("cloud_last_battle", serial::DataFileType::PERSISTENCE_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::DO_NOT_WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM));
}
#endif

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
    CheckForEmptyProgression();
#if defined(MACOS) || defined(MOBILE_FLOW)
    cloudkit_utils::QueryPlayerProgress([=](cloudkit_utils::QueryResultData resultData){ OnCloudQueryCompleted(resultData); });
    apple_utils::LoadStoreProducts({ product_ids::STORY_HEALTH_REFILL, product_ids::COINS_S, product_ids::COINS_M, product_ids::COINS_L });
#endif
    
    DataRepository::GetInstance().SetQuickPlayData(nullptr);
    DataRepository::GetInstance().SetIsCurrentlyPlayingStoryMode(false);
    
    mQuickPlayData = std::make_unique<QuickPlayData>();
    
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
    if (mTransitioningToSubScene || DataRepository::GetInstance().GetForeignProgressionDataFound() != ForeignCloudDataFoundType::NONE)
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
        if (containerUpdateResult.mInteractedElementId != -1 && mQuickPlayData->mBattleControlType != BattleControlType::REPLAY)
        {
            DeckSelected(containerUpdateResult.mInteractedElementId, true);
        }
    }
    
    if (mCardFamilyContainerBot)
    {
        auto containerUpdateResult = mCardFamilyContainerBot->Update(dtMillis);
        if (containerUpdateResult.mInteractedElementId != -1 && (mQuickPlayData->mBattleControlType != BattleControlType::REPLAY || mActiveSubScene == SubSceneType::NEW_STORY_DECK_SELECTION))
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

std::shared_ptr<GuiObjectManager> MainMenuSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
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
    mDeckSelectionSceneObjects.clear();
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
                CARD_LIBRARY_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Card Library",
                CARD_LIBRARY_BUTTON_NAME,
                [=]()
                {
                    DataRepository::GetInstance().SetCurrentCardLibraryBehaviorType(CardLibraryBehaviorType::CARD_LIBRARY);
                    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::CARD_LIBRARY_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                SHOP_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Shop",
                SHOP_BUTTON_NAME,
                [=]()
                {
                    if (IsDisconnected())
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::DISCONNECTED_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                    }
                    else
                    {
                        DataRepository::GetInstance().SetCurrentShopBehaviorType(ShopBehaviorType::PERMA_SHOP);
                        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::SHOP_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
                    }
                },
                *scene
            ));
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                EXTRAS_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Extras",
                EXTRAS_BUTTON_NAME,
                [=](){ TransitionToSubScene(SubSceneType::EXTRAS, scene); },
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
            
            for (auto& animatedButton: mAnimatedButtons)
            {
                auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*animatedButton->GetSceneObject());
                auto textLength = boundingRect.topRight.x - boundingRect.bottomLeft.x;
                animatedButton->GetSceneObject()->mPosition.x -= textLength/2.0f;
            }
        } break;
           
        case SubSceneType::STORY_MODE:
        {
            bool progressExists = DataRepository::GetInstance().GetStoryMapGenerationSeed() != 0;
            if (progressExists)
            {
                mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
                (
                    CONTINUE_STORY_BUTTON_POSITION,
                    BUTTON_SCALE,
                    game_constants::DEFAULT_FONT_NAME,
                    "Continue Story",
                    CONTINUE_STORY_BUTTON_NAME,
                    [=](){
                        DataRepository::GetInstance().SetIsCurrentlyPlayingStoryMode(true);
                        DataRepository::GetInstance().SetCurrentShopBehaviorType(ShopBehaviorType::STORY_SHOP);
                        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(STORY_MAP_SCENE_TYPE_TO_SCENE_NAME.at(DataRepository::GetInstance().GetCurrentStoryMapSceneType()), SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE); },
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
                    [=]()
                    {
                        DataRepository::GetInstance().ResetStoryData();
                        DataRepository::GetInstance().FlushStateToFile();
                        TransitionToSubScene(SubSceneType::NEW_STORY_DECK_SELECTION, scene);
                    },
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
                [=]()
                {
                    DataRepository::GetInstance().ResetStoryData();
                    DataRepository::GetInstance().FlushStateToFile();
                    TransitionToSubScene(SubSceneType::NEW_STORY_DECK_SELECTION, scene);
                },
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
        
        case SubSceneType::NEW_STORY_DECK_SELECTION:
        {
            scene::TextSceneObjectData textDataDeckSelectionPrompt;
            textDataDeckSelectionPrompt.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataDeckSelectionPrompt.mText = "Select Story Deck";
            auto deckSelectionTextSceneObject = scene->CreateSceneObject(STORY_DECK_SELECTION_PROMPT_SCENE_OBJECT_NAME);
            deckSelectionTextSceneObject->mSceneObjectTypeData = std::move(textDataDeckSelectionPrompt);
            deckSelectionTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            deckSelectionTextSceneObject->mPosition = NEW_STORY_DECK_SELECTION_TEXT_POSITION;
            deckSelectionTextSceneObject->mScale = BUTTON_SCALE;
            
            mCardFamilyContainerBot = std::make_unique<SwipeableContainer<CardFamilyEntry>>
            (
                ContainerType::HORIZONTAL_LINE,
                glm::vec3(STORY_DECK_SELECTION_ENTRY_SCALE * 2),
                STORY_DECK_SELECTION_CONTAINER_TOP_BOUNDS,
                STORY_DECK_SELECTION_CONTAINER_CUTOFF_VALUES,
                STORY_DECK_CONTAINER_SCENE_OBJECT_NAME,
                DECK_ENTRY_Z,
                *scene,
                MIN_DECK_ENTRIES_TO_SCROLL
            );
            
            for (const auto& cardFamilyEntry: game_constants::CARD_FAMILY_NAMES_TO_TEXTURES)
            {
                {
                    auto cardFamilyEntrySceneObject = scene->CreateSceneObject();
                    cardFamilyEntrySceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + DECK_ENTRY_SHADER);
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_X_UNIFORM_NAME] = STORY_DECK_ENTRY_CUTOFF_VALUES.s;
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_X_UNIFORM_NAME] = STORY_DECK_ENTRY_CUTOFF_VALUES.t;
                    cardFamilyEntrySceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = DECK_ENTRY_ALPHA;
                    cardFamilyEntrySceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DECK_ENTRY_MASK_TEXTURE_FILE_NAME);
                    cardFamilyEntrySceneObject->mScale = glm::vec3(STORY_DECK_SELECTION_ENTRY_SCALE);
                    cardFamilyEntrySceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + cardFamilyEntry.second);
                    
                    CardFamilyEntry cardEntry;
                    cardEntry.mCardFamilyName = cardFamilyEntry.first;
                    cardEntry.mSceneObjects.emplace_back(cardFamilyEntrySceneObject);
                    mCardFamilyContainerBot->AddItem(std::move(cardEntry), EntryAdditionStrategy::ADD_ON_THE_BACK);
                }
            }
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                START_NEW_STORY_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Start",
                START_NEW_STORY_BUTTON_SCENE_OBJECT_NAME,
                [=]()
                {
                    DataRepository::GetInstance().SetCurrentStoryPlayerDeck(mQuickPlayData->mBotPlayerDeck);
                    DataRepository::GetInstance().FlushStateToFile();
                    StartNewStory();
                },
                *scene
            ));
            
            DeckSelected(0, false);
        } break;
            
        case SubSceneType::EXTRAS:
        {
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                ENTER_GIFT_CODE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Enter Gift Code",
                ENTER_GIFT_CODE_BUTTON_NAME,
                [=]()
                {
                    if (IsDisconnected())
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::DISCONNECTED_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                    }
                    else
                    {
                        OnEnterGiftCodeButtonPressed();
                    }
                },
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
        } break;
            
        default: break;
    }
    
    size_t sceneObjectIndex = 0;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }
        
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=]()
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
        mQuickPlayData->mBattleControlType = BATTLE_MODE_BUTTON_NAMES_TO_BATTLE_CONTROL_TYPE.at(buttonName);
        
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
        auto targetScale = glm::vec3((mActiveSubScene == SubSceneType::NEW_STORY_DECK_SELECTION ? STORY_DECK_SELECTION_ENTRY_SCALE : DECK_SWIPEABLE_ENTRY_SCALE) * (selectedDeckIndex == i ? DECK_SELECTED_MAX_SCALE_FACTOR : DECK_SELECTED_MIN_SCALE_FACTOR));
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(sceneObject, sceneObject->mPosition, targetScale, DECK_SELECTION_ANIMATION_DURATION_SECS, animation_flags::IGNORE_X_COMPONENT, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
    }
    
    if (mActiveSubScene == SubSceneType::NEW_STORY_DECK_SELECTION)
    {
        mQuickPlayData->mBotPlayerDeck = CardDataRepository::GetInstance().GetStoryStartingFamilyCards(respectiveDeckContainer->GetItems()[selectedDeckIndex].mCardFamilyName);
    }
    else
    {
        if (forTopPlayer)
        {
            mQuickPlayData->mTopPlayerDeck = CardDataRepository::GetInstance().GetCardIdsByFamily(respectiveDeckContainer->GetItems()[selectedDeckIndex].mCardFamilyName);
        }
        else
        {
            mQuickPlayData->mBotPlayerDeck = CardDataRepository::GetInstance().GetCardIdsByFamily(respectiveDeckContainer->GetItems()[selectedDeckIndex].mCardFamilyName);
        }
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

void MainMenuSceneLogicManager::StartNewStory()
{
    DataRepository::GetInstance().SetIsCurrentlyPlayingStoryMode(true);
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::STORY_MAP_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
    DataRepository::GetInstance().FlushStateToFile();
}

///------------------------------------------------------------------------------------------------

void MainMenuSceneLogicManager::OnEnterGiftCodeButtonPressed()
{
#if defined(MACOS) || defined(MOBILE_FLOW)
    apple_utils::GetMessageBoxTextInput([](const std::string& giftCodeEntered)
    {
        strutils::StringId resultProductName;
        gift_utils::ClaimGiftCode(giftCodeEntered, resultProductName);
        
        if (DataRepository::GetInstance().GetCurrentGiftCodeClaimedResultType() == GiftCodeClaimedResultType::SUCCESS)
        {
            const auto& productDefinition = ProductRepository::GetInstance().GetProductDefinition(resultProductName);
            if (resultProductName == STORY_HEALTH_REFILL_PRODUCT_NAME)
            {
                DataRepository::GetInstance().StoryCurrentHealth().SetValue(DataRepository::GetInstance().GetStoryMaxHealth());
            }
            else if (resultProductName == NORMAL_PACK_PRODUCT_NAME)
            {
                DataRepository::GetInstance().AddPendingCardPack(CardPackType::NORMAL);
            }
            else if (resultProductName == GOLDEN_PACK_PRODUCT_NAME)
            {
                DataRepository::GetInstance().AddPendingCardPack(CardPackType::GOLDEN);
            }
            else if (resultProductName == COINS_S_PRODUCT_NAME || resultProductName == COINS_M_PRODUCT_NAME || resultProductName == COINS_L_PRODUCT_NAME)
            {
                DataRepository::GetInstance().CurrencyCoins().SetValue(DataRepository::GetInstance().CurrencyCoins().GetValue() + productDefinition.mPrice);
            }
            
            DataRepository::GetInstance().FlushStateToFile();
        }
        
        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(GIFT_CODE_CLAIM_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
    });
#endif
}

///------------------------------------------------------------------------------------------------

bool MainMenuSceneLogicManager::IsDisconnected() const
{
#if defined(MACOS) || defined(MOBILE_FLOW)
    return !apple_utils::IsConnectedToTheInternet();
#else
    return !window_utils::IsConnectedToTheInternet();
#endif
}

///------------------------------------------------------------------------------------------------
