///------------------------------------------------------------------------------------------------
///  DefeatSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/01/2024
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/DefeatSceneLogicManager.h>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId DEFEAT_SCENE_NAME = strutils::StringId("defeat_scene");
static const strutils::StringId CONTINUE_BUTTON_NAME = strutils::StringId("continue_button");
static const strutils::StringId BACK_TO_MAIN_MENU_BUTTON_NAME = strutils::StringId("back_to_main_menu_button");
static const strutils::StringId DEFEAT_TEXT_SCENE_OBJECT_NAME = strutils::StringId("defeat_text");
static const strutils::StringId QUIT_CONFIRMATION_BUTTON_NAME = strutils::StringId("quit_confirmation");
static const strutils::StringId QUIT_CANCELLATION_BUTTON_NAME = strutils::StringId("quit_cancellation");
static const strutils::StringId DEFEAT_INTRO_TEXT_TOP_NAME = strutils::StringId("defeat_intro_text_top");
static const strutils::StringId DEFEAT_INTRO_TEXT_BOT_NAME = strutils::StringId("defeat_intro_text_bot");
static const strutils::StringId DEFEAT_RESULTS_TEXT_TOP_NAME = strutils::StringId("defeat_results_text_top");
static const strutils::StringId DEFEAT_RESULTS_TEXT_BOT_NAME = strutils::StringId("defeat_results_text_bot");

static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 CONTINUE_BUTTON_POSITION = {-0.091f, -0.092f, 23.1f};
static const glm::vec3 BACK_TO_MAIN_MENU_BUTTON_POSITION = {-0.152f, -0.083f, 23.1f};
static const glm::vec3 DEFEAT_INTRO_TEXT_TOP_POSITION = {-0.25f, 0.07f, 23.1f};
static const glm::vec3 DEFEAT_INTRO_TEXT_BOT_POSITION = {-0.20f, 0.019f, 23.1f};
static const glm::vec3 DEFEAT_RESULTS_TEXT_TOP_POSITION = {-0.186f, 0.07f, 23.1f};
static const glm::vec3 DEFEAT_RESULTS_TEXT_BOT_POSITION = {-0.159f, 0.019f, 23.1f};

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    DEFEAT_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    DEFEAT_TEXT_SCENE_OBJECT_NAME,
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& DefeatSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

DefeatSceneLogicManager::DefeatSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

DefeatSceneLogicManager::~DefeatSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void DefeatSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void DefeatSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mActiveSubScene = SubSceneType::NONE;
    mTransitioningToSubScene = false;
    InitSubScene(SubSceneType::INTRO, scene);
}

///------------------------------------------------------------------------------------------------

void DefeatSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioningToSubScene)
    {
        return;
    }
    
    // Animated buttons
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void DefeatSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            sceneObject->mInvisible = true;
        });
    }
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> DefeatSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void DefeatSceneLogicManager::InitSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene)
{
    if (mActiveSubScene == subSceneType)
    {
        return;
    }
    
    mActiveSubScene = subSceneType;
    
    scene->RemoveAllSceneObjectsButTheOnesNamed(STATIC_SCENE_ELEMENTS);
    mAnimatedButtons.clear();
    
    switch (subSceneType)
    {
        case SubSceneType::INTRO:
        {
            scene::TextSceneObjectData textDataDefeatIntroTop;
            textDataDefeatIntroTop.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataDefeatIntroTop.mText = "Your journey has come to an end!";
            auto textDefeatIntroTopSceneObject = scene->CreateSceneObject(DEFEAT_INTRO_TEXT_TOP_NAME);
            textDefeatIntroTopSceneObject->mSceneObjectTypeData = std::move(textDataDefeatIntroTop);
            textDefeatIntroTopSceneObject->mPosition = DEFEAT_INTRO_TEXT_TOP_POSITION;
            textDefeatIntroTopSceneObject->mScale = BUTTON_SCALE;
            
            scene::TextSceneObjectData textDataDefeatIntroBot;
            textDataDefeatIntroBot.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataDefeatIntroBot.mText = "Continue to see the results.";
            auto textDefeatIntroBotSceneObject = scene->CreateSceneObject(DEFEAT_INTRO_TEXT_BOT_NAME);
            textDefeatIntroBotSceneObject->mSceneObjectTypeData = std::move(textDataDefeatIntroBot);
            textDefeatIntroBotSceneObject->mPosition = DEFEAT_INTRO_TEXT_BOT_POSITION;
            textDefeatIntroBotSceneObject->mScale = BUTTON_SCALE;
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                CONTINUE_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Continue",
                CONTINUE_BUTTON_NAME,
                [=]() { TransitionToSubScene(SubSceneType::RESULTS, scene); },
                *scene
            ));
        } break;
            
        case SubSceneType::RESULTS:
        {
            scene::TextSceneObjectData textDataDefeatResultsTop;
            textDataDefeatResultsTop.mFontName = game_constants::DEFAULT_FONT_NAME;
            textDataDefeatResultsTop.mText = "Highest level achieved: " + std::to_string(ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x) + "";
            auto textDefeatResultsTopSceneObject = scene->CreateSceneObject(DEFEAT_RESULTS_TEXT_TOP_NAME);
            textDefeatResultsTopSceneObject->mSceneObjectTypeData = std::move(textDataDefeatResultsTop);
            textDefeatResultsTopSceneObject->mPosition = DEFEAT_RESULTS_TEXT_TOP_POSITION;
            textDefeatResultsTopSceneObject->mScale = BUTTON_SCALE;
            
            scene::TextSceneObjectData textDataDefeatResultsBot;
            textDataDefeatResultsBot.mFontName = game_constants::DEFAULT_FONT_NAME;
            
            auto timePreformatted = strutils::GetHoursMinutesStringFromSeconds(ProgressionDataRepository::GetInstance().GetCurrentStorySecondsPlayed());
            auto timeComponents = strutils::StringSplit(timePreformatted, ':');
            textDataDefeatResultsBot.mText = "Time played: " + timeComponents[0] + "h " + timeComponents[1] + "m";
            
            auto textDefeatResultBotSceneObject = scene->CreateSceneObject(DEFEAT_RESULTS_TEXT_BOT_NAME);
            textDefeatResultBotSceneObject->mSceneObjectTypeData = std::move(textDataDefeatResultsBot);
            textDefeatResultBotSceneObject->mPosition = DEFEAT_RESULTS_TEXT_BOT_POSITION;
            textDefeatResultBotSceneObject->mScale = BUTTON_SCALE;
            
            mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
            (
                BACK_TO_MAIN_MENU_BUTTON_POSITION,
                BUTTON_SCALE,
                game_constants::DEFAULT_FONT_NAME,
                "Back to Main Menu",
                BACK_TO_MAIN_MENU_BUTTON_NAME,
                [=]()
                {
                    ProgressionDataRepository::GetInstance().ResetStoryData();
                    ProgressionDataRepository::GetInstance().FlushStateToFile();
                    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::MAIN_MENU_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
                },
                *scene
            ));
        } break;
            
        default: break;
    }
    
    size_t sceneObjectIndex = 0;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        sceneObject->mInvisible = false;
        
        if (!STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=]()
        {
            mTransitioningToSubScene = false;
        });
    }
}

///------------------------------------------------------------------------------------------------

void DefeatSceneLogicManager::TransitionToSubScene(const SubSceneType subSceneType, std::shared_ptr<scene::Scene> scene)
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
