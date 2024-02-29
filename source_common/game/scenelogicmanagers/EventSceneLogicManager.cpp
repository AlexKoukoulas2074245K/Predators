///------------------------------------------------------------------------------------------------
///  EventSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <engine/sound/SoundManager.h>
#include <game/AnimatedButton.h>
#include <game/ArtifactProductIds.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/GuiObjectManager.h>
#include <game/DataRepository.h>
#include <game/scenelogicmanagers/EventSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId EVENT_PORTRAIT_SCENE_OBJECT_NAME = strutils::StringId("event_portrait");
static const strutils::StringId EVENT_DESCRIPTION_SCENE_OBJECT_NAME = strutils::StringId("event_description");
static const strutils::StringId EVENT_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("event_button");
static const strutils::StringId DEFEAT_SCENE_NAME = strutils::StringId("defeat_scene");
static const strutils::StringId ANIMATED_STAT_CONTAINER_ANIMATION_NAME = strutils::StringId("animated_stat_container_animation");
static const strutils::StringId GUARDIAN_ANGEL_ICON_SCENE_OBJECT_NAME = strutils::StringId("guardian_angel_icon");

static const std::string VICTORY_SFX = "sfx_victory";
static const std::string GUARDIAN_ANGEL_ICON_SHADER_FILE_NAME = "rare_item.vs";
static const std::string GUARDIAN_ANGEL_ICON_TEXTURE_FILE_NAME = "rare_item_rewards/guardian_angel.png";

static const glm::vec3 GUARDIAN_ANGEL_ICON_INIT_SCALE = {0.001f, 0.001f, 0.001f};
static const glm::vec3 GUARDIAN_ANGEL_ICON_END_SCALE = {0.4f, 0.4f, 0.4f};
static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 EVENT_DESCRIPTION_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 EVENT_PORTRAIT_SCALE = {0.4f, 0.4f, 0.4f};
static const glm::vec3 EVENT_PORTRAIT_POSITION = {-0.1f, 0.0f, 0.8f};

static const float EVENT_SCREEN_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float EVENT_SCREEN_ITEM_Z = 1.0f;
static const float EVENT_PORTRAIT_ALPHA = 0.75f;
static const float EVENT_PORTRAIT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR = 0.09f;
static const float EVENT_DESCRIPTION_TEXT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR = 1500.0f;
static const float EVENT_BUTTON_SNAP_TO_EDGE_OFFSET_FACTOR = 1500.0f;
static const float ANIMATION_STEP_DURATION = 2.0f;
static const float ANIMATION_MAX_ALPHA = 0.6f;
static const float GUARDIAN_ANGEL_ICON_Z = 20.0f;

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
    mGuiManager = std::make_shared<GuiObjectManager>(scene);
    
    mCurrentEventIndex = 0;
    mCurrentEventScreenIndex = -1;
    
    RegisterForEvents();
    SelectRandomStoryEvent();
    CreateEventScreen(DataRepository::GetInstance().GetCurrentEventScreenIndex());
    
    DataRepository::GetInstance().SetCurrentStoryMapSceneType(StoryMapSceneType::EVENT);
    CoreSystemsEngine::GetInstance().GetSoundManager().PreloadSfx(VICTORY_SFX);
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (!mScene->GetCamera().IsShaking())
    {
        mGuiManager->Update(dtMillis);
    }
    
    if (mTransitioning)
    {
        return;
    }
    
    if (mGuiManager->GetStoryHealthContainerCurrentValue() != DataRepository::GetInstance().StoryCurrentHealth().GetDisplayedValue())
    {
        return;
    }
    
    if (mGuiManager->GetStoryHealthContainerCurrentValue() <= 0)
    {
        // Resurrection case
        if (DataRepository::GetInstance().GetStoryArtifactCount(artifacts::GUARDIAN_ANGEL) > 0)
        {
            // Commit health values
            auto& progressionHealth = DataRepository::GetInstance().StoryCurrentHealth();
            progressionHealth.SetValue(DataRepository::GetInstance().GetStoryMaxHealth()/2);
            progressionHealth.SetDisplayedValue(progressionHealth.GetValue());
            
            // And artifact changes
            auto currentStoryArtifacts = DataRepository::GetInstance().GetCurrentStoryArtifacts();
            currentStoryArtifacts.erase(std::remove_if(currentStoryArtifacts.begin(), currentStoryArtifacts.end(), [](const std::pair<strutils::StringId, int>& artifactEntry)
            {
                return artifactEntry.first == artifacts::GUARDIAN_ANGEL;
            }), currentStoryArtifacts.end());
            DataRepository::GetInstance().SetCurrentStoryArtifacts(currentStoryArtifacts);
            DataRepository::GetInstance().FlushStateToFile();
            
            // Play Sound
            CoreSystemsEngine::GetInstance().GetSoundManager().PlaySound(VICTORY_SFX);
            
            // And animate icon
            auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
            auto guardianAngelIconSceneObject = mScene->CreateSceneObject(GUARDIAN_ANGEL_ICON_SCENE_OBJECT_NAME);
            guardianAngelIconSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = ANIMATION_MAX_ALPHA;
            guardianAngelIconSceneObject->mPosition.z = GUARDIAN_ANGEL_ICON_Z;
            guardianAngelIconSceneObject->mScale = GUARDIAN_ANGEL_ICON_INIT_SCALE;
            guardianAngelIconSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + GUARDIAN_ANGEL_ICON_SHADER_FILE_NAME);
            guardianAngelIconSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + GUARDIAN_ANGEL_ICON_TEXTURE_FILE_NAME);
            
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(guardianAngelIconSceneObject, guardianAngelIconSceneObject->mPosition, GUARDIAN_ANGEL_ICON_END_SCALE, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(guardianAngelIconSceneObject, 0.0f, ANIMATION_STEP_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
            {
                mScene->RemoveSceneObject(GUARDIAN_ANGEL_ICON_SCENE_OBJECT_NAME);
            });
        }
        else
        {
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(DEFEAT_SCENE_NAME, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
            mTransitioning = true;
            return;
        }
    }
    
    if (!CoreSystemsEngine::GetInstance().GetAnimationManager().IsAnimationPlaying(ANIMATED_STAT_CONTAINER_ANIMATION_NAME))
    {
        for (auto& animatedButton: mCurrentEventButtons)
        {
            animatedButton->Update(dtMillis);
        }
    }
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    mGuiManager->StopRewardAnimation();
    mGuiManager = nullptr;
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> EventSceneLogicManager::VGetGuiObjectManager()
{
    return mGuiManager;
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
    mGuiManager->OnWindowResize();
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::SelectRandomStoryEvent()
{
    auto currentNodeSeed = DataRepository::GetInstance().GetCurrentStoryMapNodeSeed();
    math::SetControlSeed(currentNodeSeed);
    
    mRegisteredStoryEvents.clear();
    
    ///------------------------------------------------------------------------------------------------
    /// Gold Coin cart event
    {
        auto coinsToGain = math::ControlledRandomInt(15, 30) + 8 * (DataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x + (DataRepository::GetInstance().GetCurrentStoryMapType() == StoryMapType::NORMAL_MAP ? game_constants::TUTORIAL_NODE_MAP_DIMENSIONS.s : 0));
        
        auto greedyGoblinCount = DataRepository::GetInstance().GetStoryArtifactCount(artifacts::GREEDY_GOBLIN);
        if (greedyGoblinCount > 0)
        {
            coinsToGain *= 2 * greedyGoblinCount;
        }
        
        mRegisteredStoryEvents.emplace_back
        (
            StoryRandomEventData
            ({
                StoryRandomEventScreenData("events/gold_coin_cart.png", {"", "You found a cart full of", "gold coins!"},
                {
                    StoryRandomEventButtonData("Collect " + std::to_string(coinsToGain) + " Gold Coins", 1, [=]()
                    {
                        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(coinsToGain, mScene->FindSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME)->mPosition);
                    }),
                    StoryRandomEventButtonData("Ignore Cart", 2)
                }),
                StoryRandomEventScreenData("events/gold_coin_cart.png", {"", "You collected " + std::to_string(coinsToGain) + " gold coins!"},
                {
                    StoryRandomEventButtonData("Ok", 3)
                }),
                StoryRandomEventScreenData("events/gold_coin_cart.png", {"", "You got suspicious and", "ignored the gold coin cart..."},
                {
                    StoryRandomEventButtonData("Ok", 3)
                })
            }, [](){ return true; })
        );
    }
    
    ///------------------------------------------------------------------------------------------------
    /// Lava Trap event
    {
        auto guaranteedHpLoss = math::ControlledRandomInt(1, 2) + (DataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x + (DataRepository::GetInstance().GetCurrentStoryMapType() == StoryMapType::NORMAL_MAP ? game_constants::TUTORIAL_NODE_MAP_DIMENSIONS.s : 0))/2;
        auto randomHpLoss = math::ControlledRandomInt(5, 15) + (DataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x + (DataRepository::GetInstance().GetCurrentStoryMapType() == StoryMapType::NORMAL_MAP ? game_constants::TUTORIAL_NODE_MAP_DIMENSIONS.s : 0));
        auto failedJump = math::ControlledRandomInt(1, 3) == 1;
        
        mRegisteredStoryEvents.emplace_back
        (
            StoryRandomEventData
            ({
                StoryRandomEventScreenData("events/lava_trap.png", {"", "You approach a steep cliff", "overlooking a river of lava."},
                {
                    StoryRandomEventButtonData("Continue", 1)
                }),
                StoryRandomEventScreenData("events/lava_trap.png", {"You can either try jumping,", "risking a fall, or retrace your", "steps, circle down and", "around (closer to the lava),", "stepping on the hot ground."},
                {
                    StoryRandomEventButtonData("Risk the Jump  (33% -" + std::to_string(randomHpLoss) + "*)", failedJump ? 2 : 3, [=]()
                    {
                        if (failedJump)
                        {
                            auto& progressionHealth = DataRepository::GetInstance().StoryCurrentHealth();
                            progressionHealth.SetValue(progressionHealth.GetValue() - randomHpLoss);
                            progressionHealth.SetDisplayedValue(progressionHealth.GetDisplayedValue() - randomHpLoss);
                            
                            mScene->GetCamera().Shake(1.0f, 0.05f);
                        }
                    }),
                    StoryRandomEventButtonData("Go down and around  (100% -" + std::to_string(guaranteedHpLoss) + "*)", 4, [=]()
                    {
                        auto& progressionHealth = DataRepository::GetInstance().StoryCurrentHealth();
                        progressionHealth.SetValue(progressionHealth.GetValue() - guaranteedHpLoss);
                        progressionHealth.SetDisplayedValue(progressionHealth.GetDisplayedValue() - guaranteedHpLoss);
                        
                        mScene->GetCamera().Shake(0.4f, 0.002f);
                    })
                }),
                StoryRandomEventScreenData("events/lava_trap.png", {"", "You failed the jump, fell", "and got severely damaged.."},
                {
                    StoryRandomEventButtonData("Ok", 5)
                }),
                StoryRandomEventScreenData("events/lava_trap.png", {"", "You successfully jumped", "over the clif without", "a scratch!"},
                {
                    StoryRandomEventButtonData("Ok", 5)
                }),
                StoryRandomEventScreenData("events/lava_trap.png", {"", "You decided to circle around,", "stepping on the hot ground..."},
                {
                    StoryRandomEventButtonData("Ok", 5)
                }),
            }, [](){ return true; })
        );
    }
    
    ///------------------------------------------------------------------------------------------------
    /// Mysterious Spring event
    {
        auto guaranteedHpGain = math::ControlledRandomInt(10, 15);
        auto randomHpLoss = math::ControlledRandomInt(5, 10);
        auto failedMaxDrink = math::ControlledRandomInt(1, 2) == 1;
        
        mRegisteredStoryEvents.emplace_back
        (
            StoryRandomEventData
            ({
                StoryRandomEventScreenData("events/mysterious_spring.png", {"You approach a moonlit", "forest glade. An animated", "emerald water spring", "catches your eye."},
                {
                    StoryRandomEventButtonData("Continue", 1, [=](){})
                }),
                StoryRandomEventScreenData("events/mysterious_spring.png", {"It looks safe to drink, however", "many skulls are littered", "throughout the lake. How", "much water should you drink?"},
                {
                    StoryRandomEventButtonData("LOADS  (50% Full* or 50% -" + std::to_string(randomHpLoss) + "*)", failedMaxDrink ? 2 : 3, [=]()
                    {
                        if (failedMaxDrink)
                        {
                            auto& progressionHealth = DataRepository::GetInstance().StoryCurrentHealth();
                            progressionHealth.SetValue(progressionHealth.GetValue() - randomHpLoss);
                            progressionHealth.SetDisplayedValue(progressionHealth.GetDisplayedValue() - randomHpLoss);
                            
                            mScene->GetCamera().Shake(1.0f, 0.05f);
                        }
                        else
                        {
                            events::EventSystem::GetInstance().DispatchEvent<events::HealthRefillRewardEvent>(DataRepository::GetInstance().GetStoryMaxHealth() - DataRepository::GetInstance().StoryCurrentHealth().GetValue(), mScene->FindSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME)->mPosition);
                        }
                    }),
                    StoryRandomEventButtonData("Just a sip  (100% +" + std::to_string(guaranteedHpGain) + "*)", 4, [=]()
                    {
                        auto& storyCurrentHealth = DataRepository::GetInstance().StoryCurrentHealth();
                        auto healthRestored = math::Min(DataRepository::GetInstance().GetStoryMaxHealth(), storyCurrentHealth.GetValue() + guaranteedHpGain) - storyCurrentHealth.GetValue();
                        events::EventSystem::GetInstance().DispatchEvent<events::HealthRefillRewardEvent>(healthRestored, mScene->FindSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME)->mPosition);
                    })
                }),
                StoryRandomEventScreenData("events/mysterious_spring.png", {"", "You drank greedily, only to", "soon realize that the spring", "was poisoned!"},
                {
                    StoryRandomEventButtonData("Ok", 5)
                }),
                StoryRandomEventScreenData("events/mysterious_spring.png", {"You drank greedily. As much", "as you could. A serene aura", "surrounded you and made", " you feel exceptionally", " refreshed!"},
                {
                    StoryRandomEventButtonData("Ok", 5)
                }),
                StoryRandomEventScreenData("events/mysterious_spring.png", {"", "You decided to a quick,", "safe sip and felt", "slightly refreshed."},
                {
                    StoryRandomEventButtonData("Ok", 5)
                }),
            }, [](){ return DataRepository::GetInstance().StoryCurrentHealth().GetValue() < 0.9f * DataRepository::GetInstance().GetStoryMaxHealth(); })
        );
    }
    
    for (auto i = 0; i < mRegisteredStoryEvents.size(); ++i)
    {
        logging::Log(logging::LogType::INFO, "Event %d applicable=%s", i, mRegisteredStoryEvents[i].mApplicabilityFunction() ? "true" : "false");
    }
    
    auto eventIndexSelectionRandInt = math::ControlledRandomInt(0, static_cast<int>(mRegisteredStoryEvents.size()) - 1);
    mCurrentEventIndex = DataRepository::GetInstance().GetCurrentEventIndex();
    if (mCurrentEventIndex == -1)
    {
        mCurrentEventIndex = eventIndexSelectionRandInt;
        while (!mRegisteredStoryEvents[mCurrentEventIndex].mApplicabilityFunction())
        {
            mCurrentEventIndex = (mCurrentEventIndex + 1) % mRegisteredStoryEvents.size();
        }
        DataRepository::GetInstance().SetCurrentEventIndex(mCurrentEventIndex);
    }
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
        descriptionRowSceneObject->mPosition = { -0.06f, 0.20f - descriptionRowIndex * 0.045, EVENT_SCREEN_ITEM_Z };
        descriptionRowSceneObject->mScale = EVENT_DESCRIPTION_TEXT_SCALE;
        descriptionRowSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
        descriptionRowSceneObject->mSnapToEdgeScaleOffsetFactor = EVENT_DESCRIPTION_TEXT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR;
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
                if (screenButton.mOnClickCallback)
                {
                    screenButton.mOnClickCallback();
                }
                DataRepository::GetInstance().SetCurrentEventScreenIndex(screenButton.mNextScreenIndex);
                DataRepository::GetInstance().FlushStateToFile();
                TransitionToEventScreen(screenButton.mNextScreenIndex);
            },
            *mScene,
            scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE,
            EVENT_BUTTON_SNAP_TO_EDGE_OFFSET_FACTOR/BUTTON_SCALE.x
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
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, sceneObject->mName == EVENT_PORTRAIT_SCENE_OBJECT_NAME ? EVENT_PORTRAIT_ALPHA : 1.0f, EVENT_SCREEN_FADE_IN_OUT_DURATION_SECS), [=]()
            {
                mTransitioning = false;
            });
        }
    }

    OnWindowResize(events::WindowResizeEvent());
}

///------------------------------------------------------------------------------------------------
