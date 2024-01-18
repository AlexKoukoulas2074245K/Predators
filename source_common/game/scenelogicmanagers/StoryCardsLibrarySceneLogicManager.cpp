///------------------------------------------------------------------------------------------------
///  StoryCardsLibrarySceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 13/01/2024
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
#include <game/CardTooltipController.h>
#include <game/CardUtils.h>
#include <game/GameSceneTransitionManager.h>
#include <game/GuiObjectManager.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/StoryCardsLibrarySceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const std::string CARD_ENTRY_SHADER = "card_library_entry.vs";
static const std::string TITLE_NORMAL_BROWSING = "Story Card Library";
static const std::string TITLE_BROWSING_FOR_DELETION = "Select Card To Delete";
static const std::string DISSOLVE_SHADER_FILE_NAME = "card_dissolve.vs";
static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";

static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId STORY_CARDS_TITLE_SCENE_OBJECT_NAME = strutils::StringId("story_cards_title");
static const strutils::StringId CARD_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("card_container");
static const strutils::StringId CARD_DELETION_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("card_deletion_overlay");
static const strutils::StringId DELETE_CARD_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("delete_card_button");
static const strutils::StringId CANCEL_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("cancel_button");
static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");

static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 DELETE_CARD_BUTTON_POSITION = {-0.225f, 0.05f, 23.9f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.0f, -0.1f, 23.2f};
static const glm::vec3 CANCEL_BUTTON_POSITION = {-0.231f, -0.05f, 23.9f};
static const glm::vec3 CARD_ENTRY_SCALE = glm::vec3(-0.273f, 0.2512f, 2.0f);
static const glm::vec3 CONTAINER_ITEM_ENTRY_SCALE = glm::vec3(0.124f, 0.212f, 2.0f);
static const glm::vec3 CARD_TOOLTIP_POSITION_OFFSET = {0.0f, 0.1f, 0.0f};
static const glm::vec3 CARD_TOOLTIP_BASE_SCALE = {0.274f, 0.274f, 1/10.0f};
static const glm::vec3 SELECTED_CARD_TARGET_POSITION = {0.0f, 0.0f, 26.5f};

static const glm::vec2 CARD_ENTRY_CUTOFF_VALUES = {-0.208f, 0.158f};
static const glm::vec2 CARD_CONTAINER_CUTOFF_VALUES = {-0.15f, 0.15f};
static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE = {3.0f, 6.0f};

static const math::Rectangle CARD_CONTAINER_BOUNDS = {{-0.305f, -0.22f}, {0.305f, 0.15f}};

static const float ITEMS_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;
static const float BACK_BUTTON_SNAP_TO_EDGE_FACTOR = 950000.0f;
static const float CARD_ENTRY_Z = 23.2f;
static const float SELECTED_CARD_ANIMATION_DURATION_SECS = 0.35f;
static const float SELECTED_CARD_OVERLAY_MAX_ALPHA = 0.9f;
static const float SELECTED_CARD_SCALE_FACTOR = 1.0f;
static const float CARD_DISSOLVE_SPEED = 0.0005f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;
static const float ANIMATED_COIN_VALUE_DURATION_SECS = 1.5f;

static constexpr std::pair<int, int> CARD_DELETION_PRODUCT_COORDS = std::make_pair(2, 2);
static constexpr int MIN_CONTAINER_ENTRIES_TO_ANIMATE = 5;
static constexpr int CARD_DELETION_SERVICE_PRICE = 100;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::STORY_CARDS_LIBRARY_SCENE
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& StoryCardsLibrarySceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

StoryCardsLibrarySceneLogicManager::StoryCardsLibrarySceneLogicManager(){}

///------------------------------------------------------------------------------------------------

StoryCardsLibrarySceneLogicManager::~StoryCardsLibrarySceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mScene = scene;
    CardDataRepository::GetInstance().LoadCardData(true);
    mCardTooltipController = nullptr;
    mSelectedCardIndex = -1;
    mCoinAnimationValue = 0.0f;
    mAnimatingCoinValue = false;
    
    switch (ProgressionDataRepository::GetInstance().GetCurrentCardLibraryBehaviorType())
    {
        case CardLibraryBehaviorType::NORMAL_BROWSING:
        {
            std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(STORY_CARDS_TITLE_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = TITLE_NORMAL_BROWSING;
        } break;
            
        case CardLibraryBehaviorType::BROWSING_FOR_DELETION:
        {
            std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(STORY_CARDS_TITLE_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = TITLE_BROWSING_FOR_DELETION;
        } break;
    }
    
    mAnimatedButtons.clear();
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        BACK_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Back",
        BACK_BUTTON_NAME,
        [=]()
        {
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
            mTransitioning = true;
        },
        *scene,
        scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE,
        BACK_BUTTON_SNAP_TO_EDGE_FACTOR
    ));
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        DELETE_CARD_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Delete",
        DELETE_CARD_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            DeleteCard();
        },
        *mScene
    ));
    mAnimatedButtons.back()->GetSceneObject()->mInvisible = true;
    mAnimatedButtons.back()->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        CANCEL_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Cancel",
        CANCEL_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            DeselectCard();
        },
        *mScene
    ));
    mAnimatedButtons.back()->GetSceneObject()->mInvisible = true;
    mAnimatedButtons.back()->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    mCardContainer = std::make_unique<SwipeableContainer<CardEntry>>
    (
        ContainerType::VERTICAL_MATRIX,
        CONTAINER_ITEM_ENTRY_SCALE,
        CARD_CONTAINER_BOUNDS,
        CARD_CONTAINER_CUTOFF_VALUES,
        CARD_CONTAINER_SCENE_OBJECT_NAME,
        CARD_ENTRY_Z,
        *scene,
        MIN_CONTAINER_ENTRIES_TO_ANIMATE
    );
    
    for (const auto& cardId: ProgressionDataRepository::GetInstance().GetCurrentStoryPlayerDeck())
    {
        CardData cardData = CardDataRepository::GetInstance().GetCardData(cardId, game_constants::LOCAL_PLAYER_INDEX);
        
        auto cardSoWrapper = card_utils::CreateCardSoWrapper(&cardData, glm::vec3(), "", CardOrientation::FRONT_FACE, CardRarity::NORMAL, false, false, true, {}, {}, *scene);
        cardSoWrapper->mSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_ENTRY_SHADER);
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_Y_UNIFORM_NAME] = CARD_ENTRY_CUTOFF_VALUES.s;
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_Y_UNIFORM_NAME] = CARD_ENTRY_CUTOFF_VALUES.t;
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        cardSoWrapper->mSceneObject->mScale = CARD_ENTRY_SCALE;
        
        CardEntry cardEntry;
        cardEntry.mCardSoWrapper = cardSoWrapper;
        cardEntry.mSceneObjects.emplace_back(cardSoWrapper->mSceneObject);
        mCardContainer->AddItem(std::move(cardEntry), EntryAdditionStrategy::ADD_ON_THE_BACK);
    }
    
    size_t sceneObjectIndex = 0;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }
        
        if
        (
            sceneObject->mName == CARD_DELETION_OVERLAY_SCENE_OBJECT_NAME ||
            sceneObject->mName == DELETE_CARD_BUTTON_SCENE_OBJECT_NAME ||
            sceneObject->mName == CANCEL_BUTTON_SCENE_OBJECT_NAME
        )
        {
            continue;
        }
        
        sceneObject->mInvisible = false;
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, ITEMS_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=](){});
    }
    
    events::EventSystem::GetInstance().RegisterForEvent<events::WindowResizeEvent>(this, &StoryCardsLibrarySceneLogicManager::OnWindowResize);
    mTransitioning = false;
    mSceneState = SceneState::BROWSING_CARDS;
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioning)
    {
        return;
    }
    
    switch (mSceneState)
    {
        case SceneState::BROWSING_CARDS:
        {
            if (mCardContainer)
            {
                static int sToolTipIndex = -1;
                static float sToolTipPointeePosY = 0.0f;
                
                const auto& cardHistoryContainerUpdateResult = mCardContainer->Update(dtMillis);
                if (cardHistoryContainerUpdateResult.mInteractionType == InteractionType::INTERACTED_WITH_ELEMENTS)
                {
                    if (sToolTipIndex != cardHistoryContainerUpdateResult.mInteractedElementId)
                    {
                        sToolTipIndex = cardHistoryContainerUpdateResult.mInteractedElementId;
                        auto interactedElementEntry = mCardContainer->GetItems()[cardHistoryContainerUpdateResult.mInteractedElementId];
                        
                        switch (ProgressionDataRepository::GetInstance().GetCurrentCardLibraryBehaviorType())
                        {
                            case CardLibraryBehaviorType::NORMAL_BROWSING:
                            {
                                auto cardData = CardDataRepository::GetInstance().GetCardData(interactedElementEntry.mCardSoWrapper->mCardData.mCardId, game_constants::LOCAL_PLAYER_INDEX);
                                
                                DestroyCardTooltip();
                                
                                if (cardData.IsSpell())
                                {
                                    sToolTipPointeePosY = interactedElementEntry.mSceneObjects.front()->mPosition.y;
                                    
                                    CreateCardTooltip(interactedElementEntry.mSceneObjects.front()->mPosition, cardData.mCardEffectTooltip);
                                }
                            } break;
                                
                            case CardLibraryBehaviorType::BROWSING_FOR_DELETION:
                            {
                                SelectCard(cardHistoryContainerUpdateResult.mInteractedElementId);
                            } break;
                        }
                    }
                }
                
                if (sToolTipIndex != -1)
                {
                    auto interactedElementEntry = mCardContainer->GetItems()[sToolTipIndex];
                    if (math::Abs(interactedElementEntry.mSceneObjects.front()->mPosition.y - sToolTipPointeePosY) > 0.01f)
                    {
                        sToolTipIndex = -1;
                        DestroyCardTooltip();
                    }
                }
            }
            
            for (auto& animatedButton: mAnimatedButtons)
            {
                animatedButton->Update(dtMillis);
            }
        } break;
            
        case SceneState::SELECTED_CARD_FOR_DELETION:
        {
            for (auto& animatedButton: mAnimatedButtons)
            {
                if (animatedButton->GetSceneObject()->mName == BACK_BUTTON_NAME)
                {
                    continue;
                }
                
                animatedButton->Update(dtMillis);
            }
        } break;
            
        case SceneState::DISSOLVING_DELETED_CARD:
        {
            auto interactedElementEntry = mCardContainer->GetItems()[mSelectedCardIndex];
            auto selectedSceneObject = interactedElementEntry.mSceneObjects.front();
            
            selectedSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
            
            if (selectedSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
            {
                selectedSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = MAX_CARD_DISSOLVE_VALUE;
                events::EventSystem::GetInstance().DispatchEvent<events::CardDeletionAnimationFinishedEvent>();
                events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
                mTransitioning = true;
            }
            
            if (mAnimatingCoinValue)
            {
                ProgressionDataRepository::GetInstance().CurrencyCoins().SetDisplayedValue(static_cast<long long>(mCoinAnimationValue));
            }
            
            auto guiObjectManager = mGameSceneTransitionManager->GetSceneLogicManagerResponsibleForScene(mPreviousScene)->VGetGuiObjectManager();
            if (guiObjectManager)
            {
                guiObjectManager->Update(dtMillis);
            }
        }
    }
    
    
    if (mCardTooltipController)
    {
        mCardTooltipController->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    DestroyCardTooltip();
    
    for (auto sceneObject: scene->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, ITEMS_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            if (sceneObject->mName == STORY_CARDS_TITLE_SCENE_OBJECT_NAME ||
                sceneObject->mName == CARD_DELETION_OVERLAY_SCENE_OBJECT_NAME)
            {
                sceneObject->mInvisible = true;
                return;
            }
            
            scene->RemoveSceneObject(sceneObject->mName);
        });
    }
    
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto previousScene = sceneManager.FindScene(mPreviousScene);
    
    animationManager.StopAnimation(game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenValueAnimation>(previousScene->GetUpdateTimeSpeedFactor(), 1.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> StoryCardsLibrarySceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    mScene->RecalculatePositionOfEdgeSnappingSceneObjects();
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText)
{
    bool shouldBeHorFlipped = cardOriginPostion.x > 0.0f;
    bool shouldBeVerFlipped = cardOriginPostion.y > 0.0f;
    
    mCardTooltipController = std::make_unique<CardTooltipController>
    (
        cardOriginPostion + CARD_TOOLTIP_POSITION_OFFSET,
        CARD_TOOLTIP_BASE_SCALE,
        tooltipText,
        false,
        shouldBeHorFlipped,
        shouldBeVerFlipped,
        *mScene
    );
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::DestroyCardTooltip()
{
    if (mCardTooltipController)
    {
        for (auto sceneObject: mCardTooltipController->GetSceneObjects())
        {
            mScene->RemoveSceneObject(sceneObject->mName);
        }
    }
    
    mCardTooltipController = nullptr;
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::SelectCard(int selectedCardIndex)
{
    mSelectedCardIndex = selectedCardIndex;
    auto card = mCardContainer->GetItems()[mSelectedCardIndex].mCardSoWrapper;
    auto cardSceneObject = mCardContainer->GetItems()[mSelectedCardIndex].mSceneObjects.front();
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
   
    // Fade in delete button
    auto deleteCardButtonSceneObject = mScene->FindSceneObject(DELETE_CARD_BUTTON_SCENE_OBJECT_NAME);
    deleteCardButtonSceneObject->mInvisible = false;
    animationManager.StopAllAnimationsPlayingForSceneObject(deleteCardButtonSceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(deleteCardButtonSceneObject, 1.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){});

    // Fade in cancel button
    auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
    cancelButtonSceneObject->mInvisible = false;
    animationManager.StopAllAnimationsPlayingForSceneObject(cancelButtonSceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 1.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){});
    
    // Fade in selected product overlay
    auto cardDeletionOverlaySceneObject = mScene->FindSceneObject(CARD_DELETION_OVERLAY_SCENE_OBJECT_NAME);
    cardDeletionOverlaySceneObject->mInvisible = false;
    animationManager.StopAllAnimationsPlayingForSceneObject(cardDeletionOverlaySceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cardDeletionOverlaySceneObject, SELECTED_CARD_OVERLAY_MAX_ALPHA, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){});
    
    // Animate product (and related scene objects to target position)
    mSelectedCardInitialPosition = cardSceneObject->mPosition;
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(card->mSceneObject, SELECTED_CARD_TARGET_POSITION, CARD_ENTRY_SCALE * SELECTED_CARD_SCALE_FACTOR, SELECTED_CARD_ANIMATION_DURATION_SECS), [=]()
    {
        if (card->mCardData.IsSpell())
        {
            CreateCardTooltip(SELECTED_CARD_TARGET_POSITION, card->mCardData.mCardEffectTooltip);
        }
    });
    
    mSceneState = SceneState::SELECTED_CARD_FOR_DELETION;
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::DeleteCard()
{
    auto cardSceneObject = mCardContainer->GetItems()[mSelectedCardIndex].mSceneObjects.front();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    animationManager.StopAllAnimationsPlayingForSceneObject(cardSceneObject->mName);
    
    // Fade out delete card button
    auto deleteCardButtonSceneObject = mScene->FindSceneObject(DELETE_CARD_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(deleteCardButtonSceneObject, 0.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){ deleteCardButtonSceneObject->mInvisible = true; });
    
    // Fade out cancel button
    auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 0.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){ cancelButtonSceneObject->mInvisible = true; });
    
    cardSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + DISSOLVE_SHADER_FILE_NAME);
    cardSceneObject->mEffectTextureResourceIds[1] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
    cardSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
    cardSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_X_UNIFORM_NAME] = cardSceneObject->mPosition.x;
    cardSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_Y_UNIFORM_NAME] = cardSceneObject->mPosition.y;
    cardSceneObject->mShaderFloatUniformValues[DISSOLVE_MAGNITUDE_UNIFORM_NAME] = math::RandomFloat(CARD_DISSOLVE_EFFECT_MAG_RANGE.x, CARD_DISSOLVE_EFFECT_MAG_RANGE.y);
    
    auto playerDeck = ProgressionDataRepository::GetInstance().GetCurrentStoryPlayerDeck();
    playerDeck.erase(playerDeck.begin() + mSelectedCardIndex);
    ProgressionDataRepository::GetInstance().SetCurrentStoryPlayerDeck(playerDeck);
    
    ProgressionDataRepository::GetInstance().AddShopBoughtProductCoordinates(CARD_DELETION_PRODUCT_COORDS);
    
    auto& storyCurrencyCoins = ProgressionDataRepository::GetInstance().CurrencyCoins();
    storyCurrencyCoins.SetValue(storyCurrencyCoins.GetValue() - CARD_DELETION_SERVICE_PRICE);
    
    mCoinAnimationValue = storyCurrencyCoins.GetDisplayedValue();
    mAnimatingCoinValue = true;
    
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mCoinAnimationValue, static_cast<float>(storyCurrencyCoins.GetValue()), ANIMATED_COIN_VALUE_DURATION_SECS), [=](){ mAnimatingCoinValue = false; });
    
    ProgressionDataRepository::GetInstance().FlushStateToFile();
    
    mSceneState = SceneState::DISSOLVING_DELETED_CARD;
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::DeselectCard()
{
    DestroyCardTooltip();
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto cardSceneObject = mCardContainer->GetItems()[mSelectedCardIndex].mSceneObjects.front();
    animationManager.StopAllAnimationsPlayingForSceneObject(cardSceneObject->mName);
    
    // Fade out delete card button
    auto deleteCardButtonSceneObject = mScene->FindSceneObject(DELETE_CARD_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(deleteCardButtonSceneObject, 0.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){ deleteCardButtonSceneObject->mInvisible = true; });
    
    // Fade out cancel button
    auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 0.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){ cancelButtonSceneObject->mInvisible = true; });
    
    // Fade in selected card overlay
    auto cardDeletionOverlaySceneObject = mScene->FindSceneObject(CARD_DELETION_OVERLAY_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(cardDeletionOverlaySceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mScene->FindSceneObject(CARD_DELETION_OVERLAY_SCENE_OBJECT_NAME), 0.0f, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){ cardDeletionOverlaySceneObject->mInvisible = true; });
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSceneObject, mSelectedCardInitialPosition, CARD_ENTRY_SCALE, SELECTED_CARD_ANIMATION_DURATION_SECS), [=](){ mSceneState = SceneState::BROWSING_CARDS; });
    
    mSelectedCardIndex = -1;
}

///------------------------------------------------------------------------------------------------
