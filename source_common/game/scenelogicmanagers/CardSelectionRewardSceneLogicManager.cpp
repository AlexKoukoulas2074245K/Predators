///------------------------------------------------------------------------------------------------
///  CardSelectionRewardSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/GuiObjectManager.h>
#include <game/events/EventSystem.h>
#include <game/GameSceneTransitionManager.h>
#include <game/scenelogicmanagers/CardSelectionRewardSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CARD_SELECTION_REWARD_SCENE_NAME = strutils::StringId("card_selection_reward_scene");
static const strutils::StringId OK_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("ok_button");
static const strutils::StringId CARD_SELECTION_TITLE_SCENE_OBJECT_NAME = strutils::StringId("card_selection_title");

static const glm::vec3 OK_BUTTON_POSITION = {0.155f, -0.038f, 23.1f};
static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};

static const float FADE_IN_OUT_DURATION_SECS = 1.0f;
static const float INITIAL_SURFACING_DELAY_SECS = 1.0f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    CARD_SELECTION_REWARD_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    CARD_SELECTION_TITLE_SCENE_OBJECT_NAME,
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& CardSelectionRewardSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

CardSelectionRewardSceneLogicManager::CardSelectionRewardSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

CardSelectionRewardSceneLogicManager::~CardSelectionRewardSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mHasConfirmedSelection = false;
    mHasPresentedSceneObjects = false;
    mInitialSurfacingDelaySecs = INITIAL_SURFACING_DELAY_SECS;
    
    mOkButton = std::make_unique<AnimatedButton>
    (
        OK_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "OK",
        OK_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            if (!mHasConfirmedSelection)
            {
                mHasConfirmedSelection = true;
                events::EventSystem::GetInstance().DispatchEvent<events::StoryBattleFinishedEvent>();
            }
        },
        *scene
    );
    
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        sceneObject->mInvisible = true;
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    }
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> scene)
{
    if (!mHasPresentedSceneObjects)
    {
        mInitialSurfacingDelaySecs -= dtMillis/1000.0f;
        if (mInitialSurfacingDelaySecs <= 0.0f)
        {
            for (auto sceneObject: scene->GetSceneObjects())
            {
                if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
                {
                    continue;
                }
                
                sceneObject->mInvisible = false;
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE), [=](){});
            }
            
            mHasPresentedSceneObjects = true;
        }
    }
    
    mOkButton->Update(dtMillis);
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, FADE_IN_OUT_DURATION_SECS), [=]()
        {
            sceneObject->mInvisible = true;
        });
    }
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> CardSelectionRewardSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------
