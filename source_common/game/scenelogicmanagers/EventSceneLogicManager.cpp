///------------------------------------------------------------------------------------------------
///  EventSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/GuiObjectManager.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/EventSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId VISIT_BUTTON_NAME = strutils::StringId("visit_button");
static const strutils::StringId EVENT_PORTRAIT_SCENE_OBJECT_NAME = strutils::StringId("event_portrait");
static const strutils::StringId EVENT_DESCRIPTION_SCENE_OBJECT_NAME = strutils::StringId("event_description");
static const strutils::StringId EVENT_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("event_button");

static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 EVENT_DESCRIPTION_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 EVENT_PORTRAIT_SCALE = {0.4f, 0.4f, 0.4f};
static const glm::vec3 EVENT_PORTRAIT_POSITION = {-0.1f, 0.0f, 1.0f};

static const float EVENT_SCREEN_FADE_IN_OUT_DURATION_SECS = 0.5f;
static const float EVENT_SCREEN_ITEM_Z = 1.0f;
static const float EVENT_PORTRAIT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR = 0.09f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::EVENT_SCENE
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    strutils::StringId("background_overlay"),
    strutils::StringId("background")
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& EventSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

EventSceneLogicManager::EventSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

EventSceneLogicManager::~EventSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mScene = scene;
    
    mTransitioning = false;
    
    mCurrentEventButtons.clear();
    mGuiManager = std::make_unique<GuiObjectManager>(scene);
    
    mCurrentEventIndex = 0;
    mCurrentEventScreenIndex = -1;
    
    RegisterForEvents();
    SelectRandomStoryEvent();
    CreateEventScreen(ProgressionDataRepository::GetInstance().GetCurrentEventScreenIndex());
    
    ProgressionDataRepository::GetInstance().SetCurrentStoryMapSceneType(StoryMapSceneType::EVENT);
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioning)
    {
        return;
    }
    
    mGuiManager->Update(dtMillis);
    
    for (auto& animatedButton: mCurrentEventButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &EventSceneLogicManager::OnWindowResize);
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::EVENT_SCENE)->RecalculatePositionOfEdgeSnappingSceneObjects();
    
    // Realign health stat container
    mGuiManager->OnWindowResize();
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::SelectRandomStoryEvent()
{
    auto currentNodeSeed = ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeSeed();
    math::SetControlSeed(currentNodeSeed);
    
    mRegisteredStoryEvents.clear();
    mRegisteredStoryEvents.emplace_back
    (
        StoryRandomEventData
        ({
            StoryRandomEventScreenData("events/gold_cart.png", {"You found a Gold cart!", "It probably contains a ", "reasonable amount of gold"},
            {
                StoryRandomEventButtonData("Collect the Gold", 1),
                StoryRandomEventButtonData("Ignore Cart", 2),
            }),
            StoryRandomEventScreenData("events/gold_cart.png", {"You collected " + std::to_string(math::ControlledRandomInt(50, 100)) + " gold!"},
            {
                StoryRandomEventButtonData("Ok", 3)
            }),
            StoryRandomEventScreenData("events/gold_cart.png", {"You got suspicious and", "ignored the gold cart.."},
            {
                StoryRandomEventButtonData("Ok", 3)
            })
        })
    );
    
    mCurrentEventIndex = math::ControlledRandomInt(0, static_cast<int>(mRegisteredStoryEvents.size()) - 1);
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::TransitionToEventScreen(const int screenIndex)
{
    mTransitioning = true;
    
    if (screenIndex >= static_cast<int>(mRegisteredStoryEvents[mCurrentEventIndex].mEventScreens.size()))
    {
        events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::STORY_MAP_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
        return;
    }
    
    for (auto sceneObject: mScene->GetSceneObjects())
    {
        bool applicableSceneObject = false;
        
        if (sceneObject->mName == EVENT_DESCRIPTION_SCENE_OBJECT_NAME || sceneObject->mName == EVENT_BUTTON_SCENE_OBJECT_NAME)
        {
            applicableSceneObject =  true;
        }
        else if (sceneObject->mName == EVENT_PORTRAIT_SCENE_OBJECT_NAME)
        {
            if (mRegisteredStoryEvents[mCurrentEventIndex].mEventScreens[mCurrentEventScreenIndex].mEventScreenPortraitTextureFilename != mRegisteredStoryEvents[mCurrentEventIndex].mEventScreens[screenIndex].mEventScreenPortraitTextureFilename)
            {
                applicableSceneObject = true;
            }
        }
        
        if (applicableSceneObject)
        {
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, EVENT_SCREEN_FADE_IN_OUT_DURATION_SECS), [=]()
            {
                CreateEventScreen(screenIndex);
            });
        }
    }
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::CreateEventScreen(const int screenIndex)
{
    if (mCurrentEventScreenIndex == screenIndex)
    {
        return;
    }
    mCurrentEventScreenIndex = screenIndex;
    ProgressionDataRepository::GetInstance().SetCurrentEventScreenIndex(mCurrentEventScreenIndex);
    
    mCurrentEventButtons.clear();
    
    mScene->RemoveAllSceneObjectsWithName(strutils::StringId(EVENT_DESCRIPTION_SCENE_OBJECT_NAME));
    mScene->RemoveAllSceneObjectsWithName(strutils::StringId(EVENT_BUTTON_SCENE_OBJECT_NAME));
    
    const auto& screenData = mRegisteredStoryEvents[mCurrentEventIndex].mEventScreens[mCurrentEventScreenIndex];
    
    auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    
    // Event portrait
    auto existingEventPortraitSceneObject = mScene->FindSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME);
    if (existingEventPortraitSceneObject == nullptr || existingEventPortraitSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.1)
    {
        mScene->RemoveSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME);
        auto eventPortraitSceneObject = mScene->CreateSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME);
        eventPortraitSceneObject->mPosition = EVENT_PORTRAIT_POSITION;
        eventPortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + screenData.mEventScreenPortraitTextureFilename);
        eventPortraitSceneObject->mScale = EVENT_PORTRAIT_SCALE;
        eventPortraitSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        eventPortraitSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
        eventPortraitSceneObject->mSnapToEdgeScaleOffsetFactor = EVENT_PORTRAIT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR;
        mScene->RecalculatePositionOfEdgeSnappingSceneObject(eventPortraitSceneObject, mScene->GetCamera().CalculateFrustum());
    }
    
    // Event screen description text
    int descriptionRowIndex = 0;
    for (const auto& descriptionRow: screenData.mEventScreenDescriptionSentences)
    {
        scene::TextSceneObjectData textData;
        textData.mFontName = game_constants::DEFAULT_FONT_NAME;
        textData.mText = descriptionRow;
        
        auto descriptionRowSceneObject = mScene->CreateSceneObject(EVENT_DESCRIPTION_SCENE_OBJECT_NAME);
        descriptionRowSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        descriptionRowSceneObject->mSceneObjectTypeData = std::move(textData);
        descriptionRowSceneObject->mPosition = { -0.06f, 0.15f - descriptionRowIndex * 0.05, EVENT_SCREEN_ITEM_Z };
        descriptionRowSceneObject->mScale = EVENT_DESCRIPTION_TEXT_SCALE;
        descriptionRowIndex++;
    }
    
    // Event screen buttons
    int screenButtonIndex = 0;
    for (const auto& screenButton: screenData.mEventScreenButtons)
    {
        mCurrentEventButtons.emplace_back(std::make_unique<AnimatedButton>
        (
            glm::vec3(0.0f, -0.07f - screenButtonIndex * 0.08f, EVENT_SCREEN_ITEM_Z),
            BUTTON_SCALE,
            game_constants::DEFAULT_FONT_NAME,
            screenButton.mButtonText,
            strutils::StringId(EVENT_BUTTON_SCENE_OBJECT_NAME),
            [=]()
            {
                TransitionToEventScreen(screenButton.mNextScreenIndex);
            },
            *mScene
        ));
        screenButtonIndex++;
    }
    
    for (auto sceneObject: mScene->GetSceneObjects())
    {
        bool applicableSceneObject = false;
        
        if (sceneObject->mName == EVENT_DESCRIPTION_SCENE_OBJECT_NAME || sceneObject->mName == EVENT_BUTTON_SCENE_OBJECT_NAME)
        {
            applicableSceneObject =  true;
        }
        else if (sceneObject->mName == EVENT_PORTRAIT_SCENE_OBJECT_NAME)
        {
            if (sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
            {
                applicableSceneObject = true;
            }
        }
        
        if (applicableSceneObject)
        {
            sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, EVENT_SCREEN_FADE_IN_OUT_DURATION_SECS), [=]()
            {
                mTransitioning = false;
            });
        }
    }
}

///------------------------------------------------------------------------------------------------
