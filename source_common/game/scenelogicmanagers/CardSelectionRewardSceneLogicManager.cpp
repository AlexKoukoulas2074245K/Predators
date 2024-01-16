///------------------------------------------------------------------------------------------------
///  CardSelectionRewardSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/SceneObjectUtils.h>
#include <game/AnimatedButton.h>
#include <game/CardUtils.h>
#include <game/CardTooltipController.h>
#include <game/GuiObjectManager.h>
#include <game/events/EventSystem.h>
#include <game/GameSceneTransitionManager.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/CardSelectionRewardSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CARD_SELECTION_REWARD_SCENE_NAME = strutils::StringId("card_selection_reward_scene");
static const strutils::StringId REWARD_HIGHLIGHTER_SCENE_OBJECT_NAME = strutils::StringId("reward_highlighter");
static const strutils::StringId CONFIRMATION_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("confirmation_button");
static const strutils::StringId CARD_SELECTION_TITLE_SCENE_OBJECT_NAME = strutils::StringId("card_selection_title");
static const strutils::StringId DARKEN_UNIFORM_NAME = strutils::StringId("darken");

static const std::string CARD_REWARD_SCENE_OBJECT_NAME_PREFIX = "card_reward_";
static const std::string CARD_REWARD_SHADER_FILE_NAME = "card_reward.vs";

static const glm::vec3 CONFIRMATION_BUTTON_POSITION = {-0.10f, -0.18f, 23.1f};
static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 CARD_REWARD_DEFAULT_SCALE = glm::vec3(-0.273f, 0.2512f, 2.0f);
static const glm::vec3 CARD_HIGHLIGHTER_SCALE = glm::vec3(0.08f, 0.13f, 1.0f) * 2.35f;
static const glm::vec3 CARD_REWARD_EXPANDED_SCALE = 1.25f * CARD_REWARD_DEFAULT_SCALE;
static const glm::vec3 CARD_TOOLTIP_POSITION_OFFSET = {0.0f, 0.1f, 0.0f};
static const glm::vec3 CARD_TOOLTIP_BASE_SCALE = {0.274f, 0.274f, 1/10.0f};

static const float FADE_IN_OUT_DURATION_SECS = 0.5f;
static const float INITIAL_SURFACING_DELAY_SECS = 1.0f;
static const float CARD_HIGHLIGHTER_X_OFFSET = -0.003f;
static const float CARD_HIGHLIGHT_ANIMATION_DURATION_SECS = 0.5f;
static const float CARD_REWARD_SURFACE_DELAY_SECS = 0.5f;

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
    mCardRewards.clear();
    mCardTooltipController = nullptr;
    mSceneState = SceneState::PENDING_PRESENTATION;
    mInitialSurfacingDelaySecs = INITIAL_SURFACING_DELAY_SECS;
    
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
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    switch (mSceneState)
    {
        case SceneState::PENDING_PRESENTATION:
        {
            mInitialSurfacingDelaySecs -= dtMillis/1000.0f;
            if (mInitialSurfacingDelaySecs <= 0.0f)
            {
                if (!ProgressionDataRepository::GetInstance().GetNextStoryOpponentName().empty())
                {
                    ProgressionDataRepository::GetInstance().SetCurrentBattleSubSceneType(BattleSubSceneType::CARD_SELECTION);
                    ProgressionDataRepository::GetInstance().SetCurrentStoryMapNodeSeed(math::GetControlSeed());
                    ProgressionDataRepository::GetInstance().FlushStateToFile();
                }
                
                for (auto sceneObject: scene->GetSceneObjects())
                {
                    if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
                    {
                        continue;
                    }
                    
                    sceneObject->mInvisible = false;
                    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE), [=](){});
                }
                
                CreateCardRewards(scene);
                mSceneState = SceneState::PENDING_CARD_SELECTION;
            }
        } break;
            
        case SceneState::PENDING_CARD_SELECTION:
        {
            auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
            auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
            
            auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(scene->GetCamera().GetViewMatrix(), scene->GetCamera().GetProjMatrix());
            
            for (auto i = 0U; i < mCardRewards.size(); ++i)
            {
                auto cardSoWrapper = mCardRewards[i];
                auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*cardSoWrapper->mSceneObject);
                
                bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
                if (cursorInSceneObject && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
                {
                    if (cardSoWrapper->mState == CardSoState::IDLE)
                    {
                        cardSoWrapper->mState = CardSoState::HIGHLIGHTED;
                        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, CARD_REWARD_EXPANDED_SCALE, 0.5f, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
                    }
                    
                    for (auto j = 0U; j < mCardRewards.size(); ++j)
                    {
                        if (j == i)
                        {
                            continue;
                        }
                        
                        mCardRewards[j]->mSceneObject->mShaderBoolUniformValues[DARKEN_UNIFORM_NAME] = true;
                    }
                    
                    auto cardHighlighterSo = scene->CreateSceneObject(REWARD_HIGHLIGHTER_SCENE_OBJECT_NAME);
                    cardHighlighterSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::ACTION_HIGHLIGHTER_SHADER_NAME);
                    cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_TIME_SPEED;
                    cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_RESOLUTION;
                    cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_CLARITY_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_CLARITY;
                    cardHighlighterSo->mShaderBoolUniformValues[game_constants::CARD_HIGHLIGHTER_INVALID_ACTION_UNIFORM_NAME] = false;
                    cardHighlighterSo->mPosition = cardSoWrapper->mSceneObject->mPosition;
                    cardHighlighterSo->mPosition.x += CARD_HIGHLIGHTER_X_OFFSET;
                    cardHighlighterSo->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
                    cardHighlighterSo->mScale = CARD_HIGHLIGHTER_SCALE;
                    
                    mConfirmationButton = std::make_unique<AnimatedButton>
                    (
                        CONFIRMATION_BUTTON_POSITION,
                        BUTTON_SCALE,
                        game_constants::DEFAULT_FONT_NAME,
                        "Confirm",
                        CONFIRMATION_BUTTON_SCENE_OBJECT_NAME,
                        [=]()
                        {
                            for (auto& cardReward: mCardRewards)
                            {
                                if (cardReward->mState == CardSoState::HIGHLIGHTED)
                                {
                                    auto currentPlayerDeck = ProgressionDataRepository::GetInstance().GetCurrentStoryPlayerDeck();
                                    currentPlayerDeck.push_back(cardReward->mCardData.mCardId);
                                    ProgressionDataRepository::GetInstance().SetCurrentStoryPlayerDeck(currentPlayerDeck);
                                    break;
                                }
                            }
                            events::EventSystem::GetInstance().DispatchEvent<events::StoryBattleFinishedEvent>();
                        },
                        *scene
                    );
                    
                    if (cardSoWrapper->mCardData.IsSpell())
                    {
                        CreateCardTooltip(cardSoWrapper->mSceneObject->mPosition, cardSoWrapper->mCardData.mCardEffectTooltip, i, scene);
                    }
                    
                    mSceneState = SceneState::PENDING_CARD_SELECTION_CONFIRMATION;
                }
                
#if !defined(MOBILE_FLOW)
                if (cursorInSceneObject && cardSoWrapper->mState == CardSoState::IDLE)
                {
                    cardSoWrapper->mState = CardSoState::HIGHLIGHTED;
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, CARD_REWARD_EXPANDED_SCALE, CARD_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
                }
                else if (!cursorInSceneObject && cardSoWrapper->mState == CardSoState::HIGHLIGHTED)
                {
                    cardSoWrapper->mState = CardSoState::IDLE;
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, cardSoWrapper->mSceneObject->mPosition, CARD_REWARD_DEFAULT_SCALE, CARD_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
                }
#endif
            }
            
        } break;
            
        case SceneState::PENDING_CARD_SELECTION_CONFIRMATION:
        {
            mConfirmationButton->Update(dtMillis);
            
            if (mCardTooltipController)
            {
                mCardTooltipController->Update(dtMillis);
            }
            
            auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
            auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(scene->GetCamera().GetViewMatrix(), scene->GetCamera().GetProjMatrix());
            
            auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*mConfirmationButton->GetSceneObject());
            bool cursorInConfirmationButton = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
            
            if (!cursorInConfirmationButton && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
            {
                // Confirmation button was not pressed at this point
                for (auto i = 0U; i < mCardRewards.size(); ++i)
                {
                    mCardRewards[i]->mSceneObject->mShaderBoolUniformValues[DARKEN_UNIFORM_NAME] = false;
                    
#if defined(MOBILE_FLOW)
                    mCardRewards[i]->mState = CardSoState::IDLE;
                    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(mCardRewards[i]->mSceneObject, mCardRewards[i]->mSceneObject->mPosition, CARD_REWARD_DEFAULT_SCALE, CARD_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
#endif
                }
                
                DestroyCardTooltip(scene);
                scene->RemoveSceneObject(mConfirmationButton->GetSceneObject()->mName);
                mConfirmationButton = nullptr;
                scene->RemoveSceneObject(REWARD_HIGHLIGHTER_SCENE_OBJECT_NAME);
                mSceneState = SceneState::PENDING_CARD_SELECTION;
            }
        } break;
    }
    
    auto cardHighlighterObject = scene->FindSceneObject(strutils::StringId(REWARD_HIGHLIGHTER_SCENE_OBJECT_NAME));
    if (cardHighlighterObject)
    {
        cardHighlighterObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
    }
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    DestroyCardTooltip(scene);
    mCardRewards.clear();
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> CardSelectionRewardSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::CreateCardRewards(std::shared_ptr<scene::Scene> scene)
{
    const auto& cardRewardsPool = CardDataRepository::GetInstance().GetStoryUnlockedCardRewardsPool();
    for (size_t i = 0; i < 3; ++i)
    {
        auto randomCardIndex = math::ControlledRandomInt() % cardRewardsPool.size();
        auto cardData = CardDataRepository::GetInstance().GetCardData(cardRewardsPool[randomCardIndex], game_constants::LOCAL_PLAYER_INDEX);
        mCardRewards.push_back(card_utils::CreateCardSoWrapper(&cardData, glm::vec3(-0.18f + 0.15 * i, -0.0f, 23.2f), CARD_REWARD_SCENE_OBJECT_NAME_PREFIX + std::to_string(i), CardOrientation::FRONT_FACE, CardRarity::NORMAL, false, false, true, {}, {}, *scene));
        mCardRewards.back()->mSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        mCardRewards.back()->mSceneObject->mScale = CARD_REWARD_DEFAULT_SCALE;
        mCardRewards.back()->mSceneObject->mShaderBoolUniformValues[DARKEN_UNIFORM_NAME] = false;
        mCardRewards.back()->mSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_REWARD_SHADER_FILE_NAME);
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mCardRewards.back()->mSceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, CARD_REWARD_SURFACE_DELAY_SECS + i * CARD_REWARD_SURFACE_DELAY_SECS), [=](){});
    }
    
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText, const size_t cardIndex, std::shared_ptr<scene::Scene> scene)
{
    bool shouldBeHorFlipped = cardIndex > 1;
    mCardTooltipController = std::make_unique<CardTooltipController>
    (
        cardOriginPostion + CARD_TOOLTIP_POSITION_OFFSET,
        CARD_TOOLTIP_BASE_SCALE,
        tooltipText,
        false,
        shouldBeHorFlipped,
        false,
        *scene
    );
}

///------------------------------------------------------------------------------------------------

void CardSelectionRewardSceneLogicManager::DestroyCardTooltip(std::shared_ptr<scene::Scene> scene)
{
    if (mCardTooltipController)
    {
        for (auto sceneObject: mCardTooltipController->GetSceneObjects())
        {
            scene->RemoveSceneObject(sceneObject->mName);
        }
    }
    
    mCardTooltipController = nullptr;
}

///------------------------------------------------------------------------------------------------
