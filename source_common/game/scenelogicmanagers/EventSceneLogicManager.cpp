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
static const strutils::StringId PARTICLE_EMITTER_SCENE_OBJECT_NAME = strutils::StringId("particle_emitter");

static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 EVENT_DESCRIPTION_TEXT_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 EVENT_PORTRAIT_SCALE = {0.4f, 0.4f, 0.4f};
static const glm::vec3 EVENT_PORTRAIT_POSITION = {-0.1f, 0.0f, 0.8f};

static const float EVENT_SCREEN_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float EVENT_SCREEN_ITEM_Z = 1.0f;
static const float EVENT_PORTRAIT_ALPHA = 0.75f;
static const float EVENT_PORTRAIT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR = 0.09f;
static const float EVENT_DESCRIPTION_TEXT_SNAP_TO_EDGE_SCALE_OFFSET_FACTOR = 1500.0f;
static const float EVENT_BUTTON_SNAP_TO_EDGE_OFFSET_FACTOR = 1100.0f;

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
    mGuiManager->Update(dtMillis);
    
    if (mTransitioning)
    {
        return;
    }
    
    for (auto& animatedButton: mCurrentEventButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void EventSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
    mScene->RemoveSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME);
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
    
    ///------------------------------------------------------------------------------------------------
    /// Gold cart event
    auto goldToGain = math::ControlledRandomInt(30, 100) + 20 * ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord().x;
    mRegisteredStoryEvents.emplace_back
    (
        StoryRandomEventData
        ({
            StoryRandomEventScreenData("events/gold_cart.png", {"", "", "You found a Gold cart!"},
            {
                StoryRandomEventButtonData("Collect the Gold", 1, [=]()
                {
                    // Permanent gold change internally
                    auto& progressionCoins = ProgressionDataRepository::GetInstance().CurrencyCoins();
                    progressionCoins.SetValue(progressionCoins.GetValue() + goldToGain);
                    
                    auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
                    
                    auto animatedNodePathParticleEmitterSceneObject = particleManager.CreateParticleEmitterAtPosition(strutils::StringId("coin_gain"), glm::vec3(), *mScene, PARTICLE_EMITTER_SCENE_OBJECT_NAME, [=](float dtMillis, scene::ParticleEmitterObjectData& particleEmitterData)
                    {
                        static const glm::vec3 COIN_INIT_POSITION_OFFSET = { 0.0f, 0.0f, 0.7f };
                        static const glm::vec3 COIN_TARGET_POSITION_OFFSET = { -0.02f, -0.01f, -22.5f };
                        static const glm::vec3 COIN_MID_POSITION_MIN = { 0.1f, -0.2f, 1.5f };
                        static const glm::vec3 COIN_MID_POSITION_MAX = { 0.3f, 0.2f, 1.5f };
                        static const float COIN_RESPAWN_TICK_SECS = 0.025f;
                        
                        auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
                        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                        
                        auto targetCoinPosition = mScene->FindSceneObject(game_constants::GUI_COIN_STACK_SCENE_OBJECT_NAME)->mPosition + COIN_TARGET_POSITION_OFFSET;
            
                        auto& particleEmitterSceneObject = *mScene->FindSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME);
                        
                        static float timeAccum = 0.0f;
                        timeAccum += dtMillis/1000.0f;
                        
                        if (timeAccum > COIN_RESPAWN_TICK_SECS && static_cast<int>(particleEmitterData.mTotalParticlesSpawned) < goldToGain)
                        {
                            timeAccum = 0.0f;
                            int particleIndex = particleManager.SpawnParticleAtFirstAvailableSlot(particleEmitterSceneObject);
                            
                            particleEmitterData.mParticlePositions[particleIndex] = mScene->FindSceneObject(EVENT_PORTRAIT_SCENE_OBJECT_NAME)->mPosition + COIN_INIT_POSITION_OFFSET;
                            
                            math::BezierCurve curve(std::vector<glm::vec3>{particleEmitterData.mParticlePositions[particleIndex], glm::vec3(math::RandomFloat(COIN_MID_POSITION_MIN.x, COIN_MID_POSITION_MAX.x), math::RandomFloat(COIN_MID_POSITION_MIN.y, COIN_MID_POSITION_MAX.y), math::RandomFloat(COIN_MID_POSITION_MIN.z, COIN_MID_POSITION_MAX.z)), targetCoinPosition});
                            
                            animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(particleEmitterData.mParticlePositions[particleIndex], curve, game_constants::IN_GAME_DRAW_CARD_ANIMATION_DURATION_SECS), [=]()
                            {
                                std::get<scene::ParticleEmitterObjectData>(mScene->FindSceneObject(PARTICLE_EMITTER_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mParticleLifetimeSecs[particleIndex] = 0.0f;
                                
                                // Animation only gold change
                                auto& coins = ProgressionDataRepository::GetInstance().CurrencyCoins();
                                coins.SetDisplayedValue(coins.GetDisplayedValue() + 1);
                            });
                        }
                    });
                }),
                StoryRandomEventButtonData("Ignore Cart", 2)
            }),
            StoryRandomEventScreenData("events/gold_cart.png", {"", "", "You collected " + std::to_string(goldToGain) + " gold!"},
            {
                StoryRandomEventButtonData("Ok", 3)
            }),
            StoryRandomEventScreenData("events/gold_cart.png", {"", "You got suspicious and", "ignored the gold cart.."},
            {
                StoryRandomEventButtonData("Ok", 3)
            })
        })
    );
    
    ///------------------------------------------------------------------------------------------------
    /// Lava Trap event
    auto guaranteedHpLoss = math::ControlledRandomInt(1, 2);
    auto randomHpLoss = math::ControlledRandomInt(5, 10);
    auto failedJump = math::ControlledRandomInt(1, 3) == 1;
    
    mRegisteredStoryEvents.emplace_back
    (
        StoryRandomEventData
        ({
            StoryRandomEventScreenData("events/lava_trap.png", {"You approach a steep cliff", "overlooking a river of lava.", "You can either try jumping,", "risking a fall, or go back", "stepping on the hot ground."},
            {
                StoryRandomEventButtonData("Jump  (33% -" + std::to_string(randomHpLoss) + "*)", failedJump ? 1 : 2, [=]()
                {
                    if (failedJump)
                    {
                        auto& progressionHealth = ProgressionDataRepository::GetInstance().StoryCurrentHealth();
                        progressionHealth.SetValue(progressionHealth.GetValue() - randomHpLoss);
                        progressionHealth.SetDisplayedValue(progressionHealth.GetDisplayedValue() - randomHpLoss);
                        
                        mScene->GetCamera().Shake(1.0f, 0.05f);
                    }
                }),
                StoryRandomEventButtonData("Go around  (100% -" + std::to_string(guaranteedHpLoss) + "*)", 3, [=]()
                {
                    auto& progressionHealth = ProgressionDataRepository::GetInstance().StoryCurrentHealth();
                    progressionHealth.SetValue(progressionHealth.GetValue() - guaranteedHpLoss);
                    progressionHealth.SetDisplayedValue(progressionHealth.GetDisplayedValue() - guaranteedHpLoss);
                    
                    mScene->GetCamera().Shake(0.4f, 0.002f);
                })
            }),
            StoryRandomEventScreenData("events/lava_trap.png", {"", "You failed the jump, fell", "and got severely damaged.."},
            {
                StoryRandomEventButtonData("Ok", 4)
            }),
            StoryRandomEventScreenData("events/lava_trap.png", {"", "You successfully jumped", "over the clif without", "a scratch!"},
            {
                StoryRandomEventButtonData("Ok", 4)
            }),
            StoryRandomEventScreenData("events/lava_trap.png", {"", "You decided to back,", "stepping on the hot ground..."},
            {
                StoryRandomEventButtonData("Ok", 4)
            }),
        })
    );
    
    //mCurrentEventIndex = 1;
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
        descriptionRowSceneObject->mPosition = { -0.06f, 0.20f - descriptionRowIndex * 0.05, EVENT_SCREEN_ITEM_Z };
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
}

///------------------------------------------------------------------------------------------------
