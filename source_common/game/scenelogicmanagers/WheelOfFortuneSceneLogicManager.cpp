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
#include <game/ProgressionDataRepository.h>
#include <game/WheelOfFortuneController.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId WHEEL_OF_FORTUNE_SCENE_NAME = strutils::StringId("wheel_of_fortune_scene");
static const strutils::StringId SPIN_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("spin_button");
static const strutils::StringId CONTINUE_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("continue_button");
static const strutils::StringId WHEEL_OF_FORTUNE_TITLE_SCENE_OBJECT_NAME = strutils::StringId("wheel_of_fortune_title");

static const glm::vec3 BUTTON_POSITION = {0.155f, -0.038f, 23.1f};
static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 REWARD_ORIGIN_POSITION = {-0.032f, -0.034f, 23.1f};

static const int EXTRA_HP_REWARD_VALUE = 10;

static const float FADE_IN_OUT_DURATION_SECS = 1.0f;

static const std::string REWARD_EXTRA_15_COINS_TEXTURE = "wheel_of_fortune_items/extra_15_coins.png";
static const std::string REWARD_EXTRA_50_COINS_TEXTURE = "wheel_of_fortune_items/extra_50_coins.png";
static const std::string REWARD_EXTRA_100_COINS_TEXTURE = "wheel_of_fortune_items/extra_100_coins.png";
static const std::string REWARD_EXTRA_HP_TEXTURE = "wheel_of_fortune_items/extra_hp.png";
static const std::string REWARD_REFILL_HP_TEXTURE = "wheel_of_fortune_items/refill_hp.png";
static const std::string REWARD_EXTRA_WEIGHT_TEXTURE = "wheel_of_fortune_items/extra_weight.png";
static const std::string REWARD_EXTRA_DAMAGE_TEXTURE = "wheel_of_fortune_items/extra_damage.png";

static const std::vector<std::string> WHEEL_REWARDS =
{
    REWARD_EXTRA_15_COINS_TEXTURE,
    REWARD_EXTRA_HP_TEXTURE,
    REWARD_EXTRA_50_COINS_TEXTURE,
    REWARD_EXTRA_15_COINS_TEXTURE,
    REWARD_EXTRA_DAMAGE_TEXTURE,
    REWARD_EXTRA_HP_TEXTURE,
    REWARD_EXTRA_100_COINS_TEXTURE,
    REWARD_EXTRA_HP_TEXTURE,
    REWARD_EXTRA_15_COINS_TEXTURE,
    REWARD_REFILL_HP_TEXTURE,
    REWARD_EXTRA_50_COINS_TEXTURE,
    REWARD_EXTRA_WEIGHT_TEXTURE
};

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    WHEEL_OF_FORTUNE_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    WHEEL_OF_FORTUNE_TITLE_SCENE_OBJECT_NAME,
    game_constants::OVERLAY_SCENE_OBJECT_NAME
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
    mScene = scene;
    
    if (!ProgressionDataRepository::GetInstance().GetNextStoryOpponentName().empty())
    {
        ProgressionDataRepository::GetInstance().SetCurrentBattleSubSceneType(BattleSubSceneType::WHEEL);
        ProgressionDataRepository::GetInstance().SetCurrentStoryMapNodeSeed(math::GetControlSeed());
        ProgressionDataRepository::GetInstance().FlushStateToFile();
    }
    
    mWheelController = std::make_unique<WheelOfFortuneController>(*scene, WHEEL_REWARDS, [=](const int itemIndex, const std::shared_ptr<scene::SceneObject> itemSceneObject){ OnWheelItemSelected(itemIndex, itemSceneObject); });
        
    mContinueButton = nullptr;
    mSpinButton = std::make_unique<AnimatedButton>
    (
        BUTTON_POSITION,
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
                
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mSpinButton->GetSceneObject(), 0.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE), [=]()
                {
                    mSpinButton->GetSceneObject()->mInvisible = true;
                });
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
        
        if (!STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
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
    
    if (mContinueButton)
    {
        mContinueButton->Update(dtMillis);
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
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
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
    if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_15_COINS_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(15, REWARD_ORIGIN_POSITION);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_50_COINS_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(50, REWARD_ORIGIN_POSITION);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_100_COINS_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(100, REWARD_ORIGIN_POSITION);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_HP_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::MaxHealthGainRewardEvent>(EXTRA_HP_REWARD_VALUE);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_REFILL_HP_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::HealthRefillRewardEvent>(ProgressionDataRepository::GetInstance().GetStoryMaxHealth() - ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue(), REWARD_ORIGIN_POSITION);
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_DAMAGE_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::ExtraDamageRewardEvent>();
    }
    else if (WHEEL_REWARDS.at(itemIndex) == REWARD_EXTRA_WEIGHT_TEXTURE)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::ExtraWeightRewardEvent>();
    }
    
    if (!ProgressionDataRepository::GetInstance().GetNextStoryOpponentName().empty())
    {
        ProgressionDataRepository::GetInstance().SetCurrentBattleSubSceneType(BattleSubSceneType::CARD_SELECTION);
        ProgressionDataRepository::GetInstance().SetCurrentStoryMapNodeSeed(math::GetControlSeed());
        ProgressionDataRepository::GetInstance().FlushStateToFile();
    }
    
    mContinueButton = std::make_unique<AnimatedButton>
    (
        BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Continue",
        CONTINUE_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            auto guiObjectManager = mGameSceneTransitionManager->GetSceneLogicManagerResponsibleForScene(mPreviousScene)->VGetGuiObjectManager();
            if (guiObjectManager)
            {
                guiObjectManager->StopRewardAnimation();
            }
            
            ProgressionDataRepository::GetInstance().StoryCurrentHealth().SetDisplayedValue(ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue());
            guiObjectManager->ForceSetStoryHealthValue(ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue());
            
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
        },
        *mScene
    );
}

///------------------------------------------------------------------------------------------------
