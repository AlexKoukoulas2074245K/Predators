///------------------------------------------------------------------------------------------------
///  SettingsSceneLogicManager.cpp
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
#include <game/scenelogicmanagers/WheelOfFortuneSceneLogicManager.h>
#include <game/WheelOfFortuneController.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId WHEEL_OF_FORTUNE_SCENE_NAME = strutils::StringId("wheel_of_fortune_scene");
static const strutils::StringId SPIN_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("spin_button");

static const glm::vec3 SPIN_BUTTON_POSITION = {0.178f, 0.00f, 23.1f};
static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 COIN_REWARD_ORIGIN_POSITION = {-0.032f, -0.013f, 23.1f};

static const float FADE_IN_OUT_DURATION_SECS = 1.0f;

static const std::string REWARD_EXTRA_COINS_TEXTURE = "wheel_of_fortune_items/extra_coins.png";
static const std::string REWARD_EXTRA_HP_TEXTURE = "wheel_of_fortune_items/extra_hp.png";
static const std::string REWARD_REFILL_HP_TEXTURE = "wheel_of_fortune_items/refill_hp.png";

static const std::vector<std::string> WHEEL_REWARDS =
{
    REWARD_EXTRA_COINS_TEXTURE,
    REWARD_EXTRA_HP_TEXTURE,
    REWARD_EXTRA_COINS_TEXTURE,
    REWARD_REFILL_HP_TEXTURE,
    REWARD_EXTRA_COINS_TEXTURE,
    REWARD_REFILL_HP_TEXTURE
};

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    WHEEL_OF_FORTUNE_SCENE_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& WheelOfFortuneSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

WheelOfFortuneSceneLogicManager::WheelOfFortuneSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

WheelOfFortuneSceneLogicManager::~WheelOfFortuneSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mWheelController = std::make_unique<WheelOfFortuneController>(*scene, WHEEL_REWARDS, [=](const int itemIndex, const std::shared_ptr<scene::SceneObject> itemSceneObject){ OnWheelItemSelected(itemIndex, itemSceneObject); });
        
    mSpinButton = std::make_unique<AnimatedButton>
    (
        SPIN_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Spin!",
        SPIN_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            if (!mHasSpinnedWheel)
            {
                mWheelController->Spin();
                mHasSpinnedWheel = true;
                
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mSpinButton->GetSceneObject(), 0.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE), [=](){});
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
        
        sceneObject->mInvisible = false;
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE), [=](){});
    }
    
    mHasSpinnedWheel = false;
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    mWheelController->Update(dtMillis);
    
    if (!mHasSpinnedWheel)
    {
        mSpinButton->Update(dtMillis);
    }
    
    auto guiObjectManager = mGameSceneTransitionManager->GetSceneLogicManagerResponsibleForScene(mPreviousScene)->VGetGuiObjectManager();
    if (guiObjectManager)
    {
        guiObjectManager->Update(dtMillis, false);
    }
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
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

std::shared_ptr<GuiObjectManager> WheelOfFortuneSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneSceneLogicManager::OnWheelItemSelected(const int itemIndex, const std::shared_ptr<scene::SceneObject>)
{
    if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_COINS_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(50, COIN_REWARD_ORIGIN_POSITION);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_HP_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(50, COIN_REWARD_ORIGIN_POSITION);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_REFILL_HP_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(50, COIN_REWARD_ORIGIN_POSITION);
    }
}

///------------------------------------------------------------------------------------------------
