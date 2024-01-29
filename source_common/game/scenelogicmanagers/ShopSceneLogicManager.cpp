///------------------------------------------------------------------------------------------------
///  ShopSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/01/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/DataFileResource.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/utils/BaseDataFileDeserializer.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/CardTooltipController.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/GuiObjectManager.h>
#include <game/DataRepository.h>
#include <game/ProductIds.h>
#include <game/scenelogicmanagers/ShopSceneLogicManager.h>
#if defined(MACOS) || defined(MOBILE_FLOW)
#include <platform_utilities/AppleUtils.h>
#include <platform_utilities/CloudKitUtils.h>
#elif defined(WINDOWS)
#include <platform_utilities/WindowsUtils.h>
#endif

///------------------------------------------------------------------------------------------------

static constexpr int SHELF_COUNT = 3;
static constexpr int SHELF_ITEM_COUNT = 3;
static constexpr std::pair<int, int> COINS_TO_LIFE_RATE = std::make_pair(100, 30);
static constexpr std::pair<int, int> CARD_DELETION_PRODUCT_COORDS = std::make_pair(2, 2);

static const strutils::StringId SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("selected_product_overlay");
static const strutils::StringId SHELVES_SCENE_OBJECT_NAME = strutils::StringId("shelves");
static const strutils::StringId CANT_BUY_PRODUCT_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("cant_buy_product_overlay");
static const strutils::StringId CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("cant_buy_product_confirmation_button");
static const strutils::StringId CONTINUE_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("continue_button");
static const strutils::StringId CANT_BUY_PRODUCT_TEXT_0_SCENE_OBJECT_NAME = strutils::StringId("cant_buy_product_text_0");
static const strutils::StringId CANT_BUY_PRODUCT_TEXT_1_SCENE_OBJECT_NAME = strutils::StringId("cant_buy_product_text_1");
static const strutils::StringId SELECT_CARD_FOR_DELETION_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("select_card_for_deletion_button");
static const strutils::StringId BUY_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("buy_button");
static const strutils::StringId CANCEL_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("cancel_button");
static const strutils::StringId DEFEAT_SCENE_NAME = strutils::StringId("defeat_scene");
static const strutils::StringId COINS_S_PRODUCT_NAME = strutils::StringId("coins_s");
static const strutils::StringId COINS_M_PRODUCT_NAME = strutils::StringId("coins_m");
static const strutils::StringId COINS_L_PRODUCT_NAME = strutils::StringId("coins_l");
static const strutils::StringId DAMAGE_GAIN_PRODUCT_NAME = strutils::StringId("damage_gain_+1");
static const strutils::StringId WEIGHT_GAIN_PRODUCT_NAME = strutils::StringId("weight_gain_+1");
static const strutils::StringId COINS_TO_LIFE_PRODUCT_NAME = strutils::StringId("coins_to_life");
static const strutils::StringId LIFE_TO_COINS_PRODUCT_NAME = strutils::StringId("life_to_coins");
static const strutils::StringId CARD_DELETION_PRODUCT_NAME = strutils::StringId("card_deletion");
static const strutils::StringId GUI_HEALTH_CRYSTAL_BASE_SCENE_OBJECT_NAME = strutils::StringId("health_crystal_base");
static const strutils::StringId GUI_HEALTH_CRYSTAL_VALUE_SCENE_OBJECT_NAME = strutils::StringId("health_crystal_value");
static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId ORIGIN_X_UNIFORM_NAME = strutils::StringId("origin_x");
static const strutils::StringId ORIGIN_Y_UNIFORM_NAME = strutils::StringId("origin_y");

static const std::string DISSOLVE_SHADER_FILE_NAME = "generic_dissolve.vs";
static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const std::string SHELVES_STORY_SHOP_TEXTURE_FILE_NAME = "shelves_story_shop.png";
static const std::string SHELVES_PERMA_SHOP_TEXTURE_FILE_NAME = "shelves_perma_shop.png";
static const std::string BASIC_CUSTOM_COLOR_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string PRICE_TAG_TEXTURE_FILE_NAME_PREFIX = "shop_items/price_tag_digits_";
static const std::string PRODUCT_NAME_PREFIX = "product_";
static const std::string CANT_BUY_PRODUCT_COIN_CASE_TEXT = "You don't have sufficient coins";
static const std::string CANT_BUY_PRODUCT_HEALTH_CASE_TEXT = "You don't have sufficient health";
static const std::string CANT_BUY_PRODUCT_FULL_HEALTH_CASE_TEXT = "You're health is Full. No need";
static const std::string CANT_BUY_PRODUCT_CASE_TEXT = "to buy this product!";
static const std::string CANT_USE_SERVICE_CASE_TEXT = "to use this service!";

static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 SELECT_CARD_FOR_DELETION_BUTTON_SCALE = {0.0003f, 0.0003f, 0.0003f};
static const glm::vec3 CONTINUE_BUTTON_POSITION = {0.0f, -0.1f, 0.3f};
static const glm::vec3 CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_POSITION = {-0.09f, -0.125f, 20.1f};
static const glm::vec3 BUY_BUTTON_POSITION = {-0.225f, 0.05f, 6.0f};
static const glm::vec3 SELECT_CARD_FOR_DELETION_BUTTON_POSITION = {-0.305f, 0.04f, 6.0f};
static const glm::vec3 CANCEL_BUTTON_POSITION = {-0.25f, -0.05f, 6.0f};
static const glm::vec3 COIN_RED_VALUE_TEXT_COLOR = {0.80f, 0.11f, 0.11f};
static const glm::vec3 COIN_NORMAL_VALUE_TEXT_COLOR = {0.80f, 0.71f, 0.11f};
static const glm::vec3 GENERIC_PRODUCT_SCALE = {0.125f, 0.125f, 0.125f};
static const glm::vec3 CARD_PRODUCT_SCALE = {-0.125f, 0.125f, 0.125f};
static const glm::vec3 PRODUCT_POSITION_OFFSET = {0.0f, 0.0f, 0.4f};
static const glm::vec3 PRODUCT_PRICE_TAG_POSITION_OFFSET = {0.0f, -0.0175f, 0.5f};
static const glm::vec3 PRODUCT_PRICE_TAG_TEXT_POSITION_OFFSET = {0.0f, -0.0165f, 0.6f};
static const glm::vec3 PRICE_TAG_SCALE = {0.1f, 0.1f, 0.1};
static const glm::vec3 PRICE_TAG_TEXT_SCALE = {0.000185f, 0.000185f, 0.000185f};
static const glm::vec3 SELECTED_PRODUCT_TARGET_POSITION = {0.0f, 0.0f, 12.0f};
static const glm::vec3 SHELF_ITEM_TARGET_BASE_POSITIONS[SHELF_COUNT] =
{
    { 0.0f, 0.175f, 0.0f },
    { 0.0f, 0.04f, 0.0f },
    { 0.0f, -0.09f, 0.0f }
};
static const glm::vec3 CARD_TOOLTIP_POSITION_OFFSET = {0.0f, 0.1f, 0.0f};
static const glm::vec3 CARD_TOOLTIP_BASE_SCALE = {0.274f, 0.274f, 1/10.0f};

static const glm::vec2 PRODUCT_GROUP_MIN_MAX_BOUNCE_SPEED = {0.0000015f, 0.0000045f};
static const glm::vec2 PRODUCT_GROUP_MIN_MAX_ANIMATION_DELAY_SECS = {0.0f, 1.0f};
static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE = {3.0f, 6.0f};
static const glm::vec2 CARD_BOUGHT_ANIMATION_MIN_MAX_OFFSETS = {-0.3f, 0.3f};

static const float PRODUCT_BOUNCE_ANIMATION_DURATION_SECS = 1.0f;
static const float CONTINUE_BUTTON_SNAP_TO_EDGE_FACTOR = 950000.0f;
static const float FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float HIGHLIGHTED_PRODUCT_SCALE_FACTOR = 1.25f;
static const float SELECTED_PRODUCT_SCALE_FACTOR = 2.0f;
static const float PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS = 0.35f;
static const float STAGGERED_FADE_IN_SECS = 0.1f;
static const float SELECTED_PRODUCT_OVERLAY_MAX_ALPHA = 0.9f;
static const float CARD_DISSOLVE_SPEED = 0.0005f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;
static const float ANIMATED_COIN_VALUE_DURATION_SECS = 1.5f;
static const float CARD_BOUGHT_ANIMATION_DURATION_SECS = 1.0f;
static const float CARD_BOUGHT_ANIMATION_MIN_ALPHA = 0.3f;
static const float CARD_BOUGHT_ANIMATION_LIBRARY_ICON_PULSE_FACTOR = 1.25f;
static const float CARD_BOUGHT_ANIMATION_LIBRARY_ICON_PULSE_DURATION_SECS = 0.1f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::SHOP_SCENE
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    strutils::StringId("shop_title"),
    strutils::StringId("shelves"),
    strutils::StringId("background_overlay"),
    strutils::StringId("background")
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& ShopSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

ShopSceneLogicManager::ShopSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

ShopSceneLogicManager::~ShopSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    CardDataRepository::GetInstance().LoadCardData(true);
    LoadProductData();
    
    mScene = scene;
    DestroyCardTooltip();
    mGuiManager = std::make_shared<GuiObjectManager>(scene);
    
    if (DataRepository::GetInstance().GetCurrentShopBehaviorType() == ShopBehaviorType::PERMA_SHOP)
    {
        scene->FindSceneObject(game_constants::GUI_SETTINGS_BUTTON_SCENE_OBJECT_NAME)->mInvisible = true;
        scene->FindSceneObject(game_constants::GUI_STORY_CARDS_BUTTON_SCENE_OBJECT_NAME)->mInvisible = true;
        scene->FindSceneObject(GUI_HEALTH_CRYSTAL_BASE_SCENE_OBJECT_NAME)->mInvisible = true;
        scene->FindSceneObject(GUI_HEALTH_CRYSTAL_VALUE_SCENE_OBJECT_NAME)->mInvisible = true;
    }
    
    RegisterForEvents();
    
    math::SetControlSeed(DataRepository::GetInstance().GetCurrentStoryMapNodeSeed());
    DataRepository::GetInstance().SetCurrentStoryMapSceneType(StoryMapSceneType::SHOP);
    
    scene->FindSceneObject(SHELVES_SCENE_OBJECT_NAME)->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + (DataRepository::GetInstance().GetCurrentShopBehaviorType() == ShopBehaviorType::STORY_SHOP ? SHELVES_STORY_SHOP_TEXTURE_FILE_NAME : SHELVES_PERMA_SHOP_TEXTURE_FILE_NAME));
    
    mSceneState = SceneState::CREATING_DYNAMIC_OBJECTS;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
    {
        for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            mProducts[shelfIndex][shelfItemIndex]->mSceneObjects.front()->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
        }
    }
    
    switch (mSceneState)
    {
        case SceneState::CREATING_DYNAMIC_OBJECTS:
        {
            CreateDynamicSceneObjects();
            HandleAlreadyBoughtProducts();
            FadeInDynamicSceneObjects();

            OnWindowResize(events::WindowResizeEvent{});
            mSceneState = SceneState::BROWSING_SHOP;
        } break;
            
        case SceneState::BROWSING_SHOP:
        {
            CheckProductsFinishedFadingIn();
            
            mGuiManager->Update(dtMillis);
            
            if (!mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME)->mInvisible)
            {
                return;
            }
            
            for (auto& animatedButton: mAnimatedButtons)
            {
                animatedButton->Update(dtMillis);
            }
            
            auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
            auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(mScene->GetCamera().GetViewMatrix(), mScene->GetCamera().GetProjMatrix());
            
            for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
            {
                for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
                {
                    if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
                    {
                        continue;
                    }
                    
                    auto& product = mProducts[shelfIndex][shelfItemIndex];
                    
                    auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*product->mSceneObjects.front());
                    
                    bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
                    if (cursorInSceneObject && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && mItemsFinishedFadingIn)
                    {
                        // Product highlighting
                        if (!product->mHighlighted)
                        {
                            product->mHighlighted = true;
                            HighlightProduct(shelfIndex, shelfItemIndex);
                        }
                        
                        SelectProduct(shelfIndex, shelfItemIndex);
                        
                        mSceneState = SceneState::SELECTED_PRODUCT;
                    }
                    
                    #if !defined(MOBILE_FLOW)
                    if (cursorInSceneObject && !product->mHighlighted)
                    {
                        product->mHighlighted = true;
                        HighlightProduct(shelfIndex, shelfItemIndex);
                    }
                    else if (!cursorInSceneObject && product->mHighlighted)
                    {
                        product->mHighlighted = false;
                        DehighlightProduct(shelfIndex, shelfItemIndex);
                    }
                    #endif
                }
            }
        } break;
          
        case SceneState::SELECTED_PRODUCT:
        {
            if (mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME)->mInvisible)
            {
                return;
            }
            
            for (auto& animatedButton: mAnimatedButtons)
            {
                if (animatedButton->GetSceneObject()->mName == CONTINUE_BUTTON_SCENE_OBJECT_NAME)
                {
                    continue;
                }
                
                animatedButton->Update(dtMillis);
            }
            
            if (mCardTooltipController)
            {
                mCardTooltipController->Update(dtMillis);
            }
        } break;
        
        case SceneState::CANT_BUY_PRODUCT_CONFIRMATION:
        {
            if (mScene->FindSceneObject(CANT_BUY_PRODUCT_OVERLAY_SCENE_OBJECT_NAME)->mInvisible)
            {
                return;
            }
            
            for (auto& animatedButton: mAnimatedButtons)
            {
                if (animatedButton->GetSceneObject()->mName == CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_SCENE_OBJECT_NAME)
                {
                    animatedButton->Update(dtMillis);
                    break;
                }
            }
        } break;
            
        case SceneState::BUYING_NON_CARD_PRODUCT:
        {
            size_t productShelfIndex, productShelfItemIndex;
            FindHighlightedProduct(productShelfIndex, productShelfItemIndex);
            auto& product = mProducts[productShelfIndex][productShelfItemIndex];
            product->mSceneObjects.front()->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
            
            if (product->mSceneObjects.front()->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
            {
                product->mSceneObjects.front()->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = MAX_CARD_DISSOLVE_VALUE;
            }
        } // Intentional Fallthrough
        case SceneState::BUYING_CARD_PRODUCT:
        {
            mGuiManager->Update(dtMillis);
            
            if (mAnimatingCoinValue)
            {
                DataRepository::GetInstance().CurrencyCoins().SetDisplayedValue(static_cast<long long>(mCoinAnimationValue));
            }
        } break;
            
        case SceneState::FINISHING_PRODUCT_PURCHASE:
        {
            mGuiManager->Update(dtMillis);
            
            auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
            auto selectedProductOverlaySceneObject = mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME);
            animationManager.StopAllAnimationsPlayingForSceneObject(selectedProductOverlaySceneObject->mName);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME), 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ selectedProductOverlaySceneObject->mInvisible = true; });
            
            mSceneState = SceneState::BROWSING_SHOP;
        }
        default: break;
    }
    
    UpdateProductPriceTags();
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    mAnimatedButtons.clear();
    mProducts.clear();
    mGuiManager = nullptr;
    DestroyCardTooltip();
    events::EventSystem::GetInstance().UnregisterAllEventsForListener(this);
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> ShopSceneLogicManager::VGetGuiObjectManager()
{
    return mGuiManager;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &ShopSceneLogicManager::OnWindowResize);
    eventSystem.RegisterForEvent<events::CardDeletionAnimationFinishedEvent>(this, &ShopSceneLogicManager::OnCardDeletionAnimationFinished);
    eventSystem.RegisterForEvent<events::GuiRewardAnimationFinishedEvent>(this, &ShopSceneLogicManager::OnGuiRewardAnimationFinished);
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::SHOP_SCENE)->RecalculatePositionOfEdgeSnappingSceneObjects();
    
    // Realign gui
    mGuiManager->OnWindowResize();
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::OnCardDeletionAnimationFinished(const events::CardDeletionAnimationFinishedEvent&)
{
    DeselectProduct(CARD_DELETION_PRODUCT_COORDS.first, CARD_DELETION_PRODUCT_COORDS.second);
    HandleAlreadyBoughtProducts();
    mSceneState = SceneState::BROWSING_SHOP;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::OnGuiRewardAnimationFinished(const events::GuiRewardAnimationFinishedEvent&)
{
    HandleAlreadyBoughtProducts();
    mSceneState = SceneState::FINISHING_PRODUCT_PURCHASE;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::CreateDynamicSceneObjects()
{
    CreateProducts();
    
    mAnimatedButtons.clear();
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        CONTINUE_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Continue",
        CONTINUE_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            if (mGuiManager)
            {
                mGuiManager->StopRewardAnimation();
            }
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>((DataRepository::GetInstance().GetCurrentShopBehaviorType() == ShopBehaviorType::STORY_SHOP ? game_constants::STORY_MAP_SCENE : game_constants::MAIN_MENU_SCENE), SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
            mSceneState = SceneState::LEAVING_SHOP;
        },
        *mScene,
        scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE,
        CONTINUE_BUTTON_SNAP_TO_EDGE_FACTOR
    ));
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        BUY_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Buy",
        BUY_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            size_t productShelfIndex, productShelfItemIndex;
            FindHighlightedProduct(productShelfIndex, productShelfItemIndex);
            OnBuyProductAttempt(productShelfIndex, productShelfItemIndex);
        },
        *mScene
    ));
    mAnimatedButtons.back()->GetSceneObject()->mInvisible = true;
    mAnimatedButtons.back()->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        SELECT_CARD_FOR_DELETION_BUTTON_POSITION,
        SELECT_CARD_FOR_DELETION_BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Select Card to Delete",
        SELECT_CARD_FOR_DELETION_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            size_t productShelfIndex, productShelfItemIndex;
            FindHighlightedProduct(productShelfIndex, productShelfItemIndex);
            OnBuyProductAttempt(productShelfIndex, productShelfItemIndex);
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
            size_t productShelfIndex, productShelfItemIndex;
            FindHighlightedProduct(productShelfIndex, productShelfItemIndex);
            DeselectProduct(productShelfIndex, productShelfItemIndex);
            mSceneState = SceneState::BROWSING_SHOP;
        },
        *mScene
    ));
    mAnimatedButtons.back()->GetSceneObject()->mInvisible = true;
    mAnimatedButtons.back()->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Continue",
        CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_SCENE_OBJECT_NAME,
        [=]()
        {
            OnCantBuyProductConfirmationButtonPressed();
        },
        *mScene
    ));
    mAnimatedButtons.back()->GetSceneObject()->mInvisible = true;
    mAnimatedButtons.back()->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::FadeInDynamicSceneObjects()
{
    size_t sceneObjectCounter = 0U;
    for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
    {
        for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            auto& product = mProducts[shelfIndex][shelfItemIndex];
            for (auto sceneObjectIndex = 0U; sceneObjectIndex < product->mSceneObjects.size(); ++sceneObjectIndex)
            {
                auto sceneObject = product->mSceneObjects[sceneObjectIndex];
                sceneObject->mInvisible = false;
                sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectCounter++ * STAGGERED_FADE_IN_SECS), [=](){});
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::CreateProducts()
{
    mProducts.clear();
    mProducts.resize(SHELF_COUNT);
    for (size_t row = 0; row < SHELF_COUNT; ++row)
    {
        for (size_t col = 0; col < SHELF_ITEM_COUNT; ++col)
        {
            mProducts[row].push_back(nullptr);
        }
    }
    
    if (DataRepository::GetInstance().GetCurrentShopBehaviorType() == ShopBehaviorType::STORY_SHOP)
    {
        // First Shelf
        mProducts[0][0] = std::make_unique<ProductInstance>(DAMAGE_GAIN_PRODUCT_NAME);
        mProducts[0][1] = std::make_unique<ProductInstance>(WEIGHT_GAIN_PRODUCT_NAME);
        
        // Second Shelf
        const auto& cardRewardsPool = CardDataRepository::GetInstance().GetStoryUnlockedCardRewardsPool();
        for (size_t col = 0; col < SHELF_ITEM_COUNT; ++col)
        {
            auto randomCardIndex = static_cast<int>(math::ControlledRandomInt() % cardRewardsPool.size());
            auto cardId = cardRewardsPool[randomCardIndex];
            const auto& cardData = CardDataRepository::GetInstance().GetCardData(cardId, true);
            auto productDefinitionName = strutils::StringId("card_" + std::to_string(cardId));
            
            auto cardPrice = cardData.IsSpell() ? 100 : 50;
            mProductDefinitions.emplace(std::make_pair(productDefinitionName, ProductDefinition(productDefinitionName, cardId, cardData.mCardEffectTooltip, cardPrice, true)));
            mProducts[1][col] = std::make_unique<ProductInstance>(productDefinitionName);
        }
        
        // Third Shelf
        mProducts[2][0] = std::make_unique<ProductInstance>(COINS_TO_LIFE_PRODUCT_NAME);
        mProducts[2][1] = std::make_unique<ProductInstance>(LIFE_TO_COINS_PRODUCT_NAME);
        mProducts[2][2] = std::make_unique<ProductInstance>(CARD_DELETION_PRODUCT_NAME);
    }
    else if (DataRepository::GetInstance().GetCurrentShopBehaviorType() == ShopBehaviorType::PERMA_SHOP)
    {
        // Second Shelf
        mProducts[1][0] = std::make_unique<ProductInstance>(COINS_S_PRODUCT_NAME);
        mProducts[1][1] = std::make_unique<ProductInstance>(COINS_M_PRODUCT_NAME);
        mProducts[1][2] = std::make_unique<ProductInstance>(COINS_L_PRODUCT_NAME);
    }
    
    for (int shelfIndex = 0; shelfIndex < SHELF_COUNT; ++shelfIndex)
    {
        for (int shelfItemIndex = 0; shelfItemIndex < SHELF_ITEM_COUNT; ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            auto& product = mProducts[shelfIndex][shelfItemIndex];
            const auto& productDefinition = mProductDefinitions.at(product->mProductName);
            
            // Generic Product
            if (std::holds_alternative<std::string>(productDefinition.mProductTexturePathOrCardId))
            {
                auto shelfItemSceneObject = mScene->CreateSceneObject(strutils::StringId(PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex)));
                shelfItemSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::get<std::string>(productDefinition.mProductTexturePathOrCardId));
                shelfItemSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_POSITION_OFFSET;
                shelfItemSceneObject->mScale = GENERIC_PRODUCT_SCALE;
                shelfItemSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                shelfItemSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
                shelfItemSceneObject->mSnapToEdgeScaleOffsetFactor = 0.4f + 1.2f * shelfItemIndex;
                
                product->mSceneObjects.push_back(shelfItemSceneObject);
            }
            // Card Product
            else
            {
                auto cardId = std::get<int>(productDefinition.mProductTexturePathOrCardId);
                auto cardData = CardDataRepository::GetInstance().GetCardData(cardId, game_constants::LOCAL_PLAYER_INDEX);
                const auto& cardIdToGoldenCardEnabledMap = DataRepository::GetInstance().GetGoldenCardIdMap();
                bool isGoldenCard = cardIdToGoldenCardEnabledMap.count(cardId) && cardIdToGoldenCardEnabledMap.at(cardId);
                
                auto cardSoWrapper = card_utils::CreateCardSoWrapper(&cardData, glm::vec3(), PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex), CardOrientation::FRONT_FACE, isGoldenCard ? CardRarity::GOLDEN : CardRarity::NORMAL, false, false, true, {}, {}, *mScene);

                cardSoWrapper->mSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_POSITION_OFFSET;
                cardSoWrapper->mSceneObject->mScale = CARD_PRODUCT_SCALE;
                cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                cardSoWrapper->mSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
                cardSoWrapper->mSceneObject->mSnapToEdgeScaleOffsetFactor = -0.4f - 1.2f * shelfItemIndex;
                
                product->mSceneObjects.push_back(cardSoWrapper->mSceneObject);
            }
            
            if (productDefinition.mPrice > 0)
            {
                auto priceTagSceneObject = mScene->CreateSceneObject(strutils::StringId(PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex) + "_tag"));
                priceTagSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PRICE_TAG_TEXTURE_FILE_NAME_PREFIX + std::to_string(std::to_string(productDefinition.mPrice).size()) + ".png");
                priceTagSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_PRICE_TAG_POSITION_OFFSET;
                priceTagSceneObject->mScale = PRICE_TAG_SCALE;
                
                priceTagSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                priceTagSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
                priceTagSceneObject->mSnapToEdgeScaleOffsetFactor = 1.1f + 1.5f * shelfItemIndex;
                product->mSceneObjects.push_back(priceTagSceneObject);
                
                scene::TextSceneObjectData priceTextData;
                priceTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
                priceTextData.mText = "|" + std::to_string(productDefinition.mPrice);
                
                if
                (
                    product->mProductName == COINS_S_PRODUCT_NAME ||
                    product->mProductName == COINS_M_PRODUCT_NAME ||
                    product->mProductName == COINS_L_PRODUCT_NAME
                )
                {
#if defined(MACOS) || defined(MOBILE_FLOW)
                    priceTextData.mText = apple_utils::GetProductPrice(product->mProductName.GetString());
#endif
                }
                
                auto priceTextSceneObject = mScene->CreateSceneObject(strutils::StringId(PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex) + "_price_text"));
                priceTextSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_PRICE_TAG_TEXT_POSITION_OFFSET;
                priceTextSceneObject->mSceneObjectTypeData = std::move(priceTextData);
                priceTextSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BASIC_CUSTOM_COLOR_SHADER_FILE_NAME);
                priceTextSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = productDefinition.mPrice > DataRepository::GetInstance().CurrencyCoins().GetValue() ? COIN_RED_VALUE_TEXT_COLOR : COIN_NORMAL_VALUE_TEXT_COLOR;
                priceTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                priceTextSceneObject->mScale = PRICE_TAG_TEXT_SCALE;
                priceTextSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
                priceTextSceneObject->mSnapToEdgeScaleOffsetFactor = 830.1f + 810.5f * shelfItemIndex;
                product->mSceneObjects.push_back(priceTextSceneObject);
            }
            
            // Animation bounce data to be applied to all of this product's scene objects
            auto itemGroupBounceSpeed = glm::vec3(0.0f, math::RandomFloat(PRODUCT_GROUP_MIN_MAX_BOUNCE_SPEED.s, PRODUCT_GROUP_MIN_MAX_BOUNCE_SPEED.t), 0.0f);
            auto itemGroupBounceDelay = math::RandomFloat(PRODUCT_GROUP_MIN_MAX_ANIMATION_DELAY_SECS.s, PRODUCT_GROUP_MIN_MAX_ANIMATION_DELAY_SECS.t);
            
            // Animate all scene objects for this product
            for (auto& sceneObject: mProducts[shelfIndex][shelfItemIndex]->mSceneObjects)
            {
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(sceneObject, itemGroupBounceSpeed, PRODUCT_BOUNCE_ANIMATION_DURATION_SECS, animation_flags::ANIMATE_CONTINUOUSLY, itemGroupBounceDelay), [](){});
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::HandleAlreadyBoughtProducts()
{
    const auto& alreadyBoughtProductCoords = DataRepository::GetInstance().GetCurrentShopBoughtProductCoordinates();
    for (const auto& boughtProductCoord: alreadyBoughtProductCoords)
    {
        auto& productInstance = mProducts[boughtProductCoord.first][boughtProductCoord.second];
        if (productInstance != nullptr)
        {
            for (auto sceneObject: productInstance->mSceneObjects)
            {
                mScene->RemoveSceneObject(sceneObject->mName);
            }
            mProducts[boughtProductCoord.first][boughtProductCoord.second] = nullptr;
        }
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::HighlightProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    const auto& productDefinition = mProductDefinitions.at(product->mProductName);
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, product->mSceneObjects[0]->mPosition, (std::holds_alternative<int>(productDefinition.mProductTexturePathOrCardId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE) * HIGHLIGHTED_PRODUCT_SCALE_FACTOR, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::DehighlightProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    const auto& productDefinition = mProductDefinitions.at(product->mProductName);
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, product->mSceneObjects[0]->mPosition, (std::holds_alternative<int>(productDefinition.mProductTexturePathOrCardId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE), PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::SelectProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    const auto& productDefinition = mProductDefinitions.at(product->mProductName);
    
    for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
    {
        for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            if (mProducts[shelfIndex][shelfItemIndex] == product)
            {
                for (auto& sceneObject: mProducts[shelfIndex][shelfItemIndex]->mSceneObjects)
                {
                    sceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::NONE;
                }
            }
            
            for (auto& sceneObject: mProducts[shelfIndex][shelfItemIndex]->mSceneObjects)
            {
                animationManager.StopAllAnimationsPlayingForSceneObject(sceneObject->mName);
            }
        }
    }
    
    if (product->mProductName == CARD_DELETION_PRODUCT_NAME)
    {
        // Fade in buy button
        auto selectCardForDeletionButtonSceneObject = mScene->FindSceneObject(SELECT_CARD_FOR_DELETION_BUTTON_SCENE_OBJECT_NAME);
        selectCardForDeletionButtonSceneObject->mInvisible = false;
        animationManager.StopAllAnimationsPlayingForSceneObject(selectCardForDeletionButtonSceneObject->mName);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(selectCardForDeletionButtonSceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
    }
    else
    {
        // Fade in buy button
        auto buyButtonSceneObject = mScene->FindSceneObject(BUY_BUTTON_SCENE_OBJECT_NAME);
        buyButtonSceneObject->mInvisible = false;
        animationManager.StopAllAnimationsPlayingForSceneObject(buyButtonSceneObject->mName);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(buyButtonSceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
    }

    // Fade in cancel button
    auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
    cancelButtonSceneObject->mInvisible = false;
    animationManager.StopAllAnimationsPlayingForSceneObject(cancelButtonSceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
    
    // Fade in selected product overlay
    auto selectedProductOverlaySceneObject = mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME);
    selectedProductOverlaySceneObject->mInvisible = false;
    animationManager.StopAllAnimationsPlayingForSceneObject(selectedProductOverlaySceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(selectedProductOverlaySceneObject, SELECTED_PRODUCT_OVERLAY_MAX_ALPHA, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
    
    // Animate product (and related scene objects to target position)
    mSelectedProductInitialPosition = product->mSceneObjects.front()->mPosition;
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, SELECTED_PRODUCT_TARGET_POSITION, (std::holds_alternative<int>(productDefinition.mProductTexturePathOrCardId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE) * SELECTED_PRODUCT_SCALE_FACTOR, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=]()
    {
        // Create card tooltip if necessary
        auto& product = mProducts[productShelfIndex][productShelfItemIndex];
        const auto& productDefinition = mProductDefinitions.at(product->mProductName);
        
        if (product->mProductName != COINS_S_PRODUCT_NAME &&
            product->mProductName != COINS_M_PRODUCT_NAME &&
            product->mProductName != COINS_L_PRODUCT_NAME)
        {
            if (!productDefinition.mDescription.empty())
            {
                CreateCardTooltip(SELECTED_PRODUCT_TARGET_POSITION, productDefinition.mDescription);
            }
        }
        
        product->mSceneObjects.front()->mShaderFloatUniformValues[game_constants::LIGHT_POS_X_UNIFORM_NAME] = game_constants::GOLDEN_CARD_LIGHT_POS_MIN_MAX_X.s;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(product->mSceneObjects.front()->mShaderFloatUniformValues[game_constants::LIGHT_POS_X_UNIFORM_NAME], game_constants::GOLDEN_CARD_LIGHT_POS_MIN_MAX_X.t, 1.0f), [](){});
    });
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::DeselectProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    DestroyCardTooltip();
    
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    const auto& productDefinition = mProductDefinitions.at(product->mProductName);
    product->mHighlighted = false;
    
    for (auto& sceneObject: product->mSceneObjects)
    {
        animationManager.StopAllAnimationsPlayingForSceneObject(sceneObject->mName);
        sceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
    }
    
    if (product->mProductName == CARD_DELETION_PRODUCT_NAME)
    {
        // Fade out select card for deletion button
        auto selectCardForDeletionButtonSceneObject = mScene->FindSceneObject(SELECT_CARD_FOR_DELETION_BUTTON_SCENE_OBJECT_NAME);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(selectCardForDeletionButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ selectCardForDeletionButtonSceneObject->mInvisible = true; });
    }
    else
    {
        // Fade out buy button
        auto buyButtonSceneObject = mScene->FindSceneObject(BUY_BUTTON_SCENE_OBJECT_NAME);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(buyButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ buyButtonSceneObject->mInvisible = true; });
    }
    
    // Fade out cancel button
    auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cancelButtonSceneObject->mInvisible = true; });
    
    // Fade in selected product overlay
    auto selectedProductOverlaySceneObject = mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(selectedProductOverlaySceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME), 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ selectedProductOverlaySceneObject->mInvisible = true; });
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, mSelectedProductInitialPosition, (std::holds_alternative<int>(productDefinition.mProductTexturePathOrCardId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE), PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=]()
    {
        for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
        {
            for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
            {
                if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
                {
                    continue;
                }
                
                // Animation bounce data to be applied to all of this product's scene objects
                auto itemGroupBounceSpeed = glm::vec3(0.0f, math::RandomFloat(PRODUCT_GROUP_MIN_MAX_BOUNCE_SPEED.s, PRODUCT_GROUP_MIN_MAX_BOUNCE_SPEED.t), 0.0f);
                auto itemGroupBounceDelay = math::RandomFloat(PRODUCT_GROUP_MIN_MAX_ANIMATION_DELAY_SECS.s, PRODUCT_GROUP_MIN_MAX_ANIMATION_DELAY_SECS.t);
                
                // Animate all scene objects for this product
                for (auto& sceneObject: mProducts[shelfIndex][shelfItemIndex]->mSceneObjects)
                {
                    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(sceneObject, itemGroupBounceSpeed, PRODUCT_BOUNCE_ANIMATION_DURATION_SECS, animation_flags::ANIMATE_CONTINUOUSLY, itemGroupBounceDelay), [](){});
                }
            }
        }
    });
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText)
{
    bool shouldBeHorFlipped = cardOriginPostion.x > 0.0f;
    
    mCardTooltipController = std::make_unique<CardTooltipController>
    (
        cardOriginPostion + CARD_TOOLTIP_POSITION_OFFSET,
        CARD_TOOLTIP_BASE_SCALE,
        tooltipText,
        false,
        shouldBeHorFlipped,
        false,
        *mScene
    );
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::DestroyCardTooltip()
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

void ShopSceneLogicManager::LoadProductData()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto productDefinitionJsonResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "shop_product_data.json", resources::DONT_RELOAD);
    const auto particlesJson =  nlohmann::json::parse(systemsEngine.GetResourceLoadingService().GetResource<resources::DataFileResource>(productDefinitionJsonResourceId).GetContents());
    
    for (const auto& shopDefinitionObject: particlesJson["shop_product_data"])
    {
        strutils::StringId productName = strutils::StringId(shopDefinitionObject["name"].get<std::string>());
        int productPrice = shopDefinitionObject["price"].get<int>();
        std::string productTexturePath = shopDefinitionObject["texture_path"].get<std::string>();
        std::string productDescription = shopDefinitionObject["description"].get<std::string>();
        bool isProductSingleUse = shopDefinitionObject["is_single_use"].get<bool>();
        
        mProductDefinitions.emplace(std::make_pair(productName, ProductDefinition(productName, productTexturePath, productDescription, productPrice, isProductSingleUse)));
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::OnBuyProductAttempt(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    const auto& productDefinition = mProductDefinitions.at(product->mProductName);
    
    auto currentCoinsValue = DataRepository::GetInstance().CurrencyCoins().GetValue();
    auto currentHealthValue = DataRepository::GetInstance().StoryCurrentHealth().GetValue();
    
    // Insufficient funds/health case
    if (productDefinition.mPrice > currentCoinsValue ||
        (product->mProductName == COINS_TO_LIFE_PRODUCT_NAME && COINS_TO_LIFE_RATE.first > currentCoinsValue) ||
        (product->mProductName == LIFE_TO_COINS_PRODUCT_NAME && COINS_TO_LIFE_RATE.second >= currentHealthValue) ||
        (product->mProductName == COINS_TO_LIFE_PRODUCT_NAME && DataRepository::GetInstance().StoryCurrentHealth().GetValue() == DataRepository::GetInstance().GetStoryMaxHealth()))
    {
        // Fade in can't buy product confirmation button
        auto cantBuyProductConfirmationButtonSceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_SCENE_OBJECT_NAME);
        cantBuyProductConfirmationButtonSceneObject->mInvisible = false;
        animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductConfirmationButtonSceneObject->mName);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductConfirmationButtonSceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
        
        // Fade in can't buy product text 0
        auto cantBuyProductText0SceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_TEXT_0_SCENE_OBJECT_NAME);
        std::get<scene::TextSceneObjectData>(cantBuyProductText0SceneObject->mSceneObjectTypeData).mText = product->mProductName == LIFE_TO_COINS_PRODUCT_NAME ?
            CANT_BUY_PRODUCT_HEALTH_CASE_TEXT:
            CANT_BUY_PRODUCT_COIN_CASE_TEXT;
        
        if (product->mProductName == COINS_TO_LIFE_PRODUCT_NAME && DataRepository::GetInstance().StoryCurrentHealth().GetValue() == DataRepository::GetInstance().GetStoryMaxHealth())
        {
            std::get<scene::TextSceneObjectData>(cantBuyProductText0SceneObject->mSceneObjectTypeData).mText = CANT_BUY_PRODUCT_FULL_HEALTH_CASE_TEXT;
        }
        
        cantBuyProductText0SceneObject->mInvisible = false;
        animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductText0SceneObject->mName);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductText0SceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
        
        // Fade in can't buy product text 1
        auto cantBuyProductText1SceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_TEXT_1_SCENE_OBJECT_NAME);
        std::get<scene::TextSceneObjectData>(cantBuyProductText1SceneObject->mSceneObjectTypeData).mText =
            (product->mProductName == LIFE_TO_COINS_PRODUCT_NAME ||
             product->mProductName == COINS_TO_LIFE_PRODUCT_NAME ||
             product->mProductName == CARD_DELETION_PRODUCT_NAME) ?
            CANT_USE_SERVICE_CASE_TEXT:
            CANT_BUY_PRODUCT_CASE_TEXT;
        
        cantBuyProductText1SceneObject->mInvisible = false;
        animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductText1SceneObject->mName);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductText1SceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
        
        // Fade in selected product overlay
        auto cantBuyProductOverlaySceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_OVERLAY_SCENE_OBJECT_NAME);
        cantBuyProductOverlaySceneObject->mInvisible = false;
        animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductOverlaySceneObject->mName);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductOverlaySceneObject, SELECTED_PRODUCT_OVERLAY_MAX_ALPHA, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
        
        mSceneState = SceneState::CANT_BUY_PRODUCT_CONFIRMATION;
    }
    // Product/Service is purchased
    else
    {
        mAnimatingCoinValue = false;
        
        if (product->mProductName == CARD_DELETION_PRODUCT_NAME)
        {
            // Card deletion follows a unique flow for card deletion
            DataRepository::GetInstance().SetCurrentCardLibraryBehaviorType(CardLibraryBehaviorType::BROWSING_FOR_DELETION);
            CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mScene->GetUpdateTimeSpeedFactor(), 0.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::CARD_LIBRARY_SCENE, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
            return;
        }
        else if (product->mProductName == WEIGHT_GAIN_PRODUCT_NAME)
        {
            events::EventSystem::GetInstance().DispatchEvent<events::ExtraWeightRewardEvent>();
        }
        else if (product->mProductName == DAMAGE_GAIN_PRODUCT_NAME)
        {
            events::EventSystem::GetInstance().DispatchEvent<events::ExtraDamageRewardEvent>();
        }
        else if (product->mProductName == LIFE_TO_COINS_PRODUCT_NAME)
        {
            auto& storyCurrenteHealth = DataRepository::GetInstance().StoryCurrentHealth();
            storyCurrenteHealth.SetDisplayedValue(storyCurrenteHealth.GetValue() - COINS_TO_LIFE_RATE.second);
            storyCurrenteHealth.SetValue(storyCurrenteHealth.GetValue() - COINS_TO_LIFE_RATE.second);
            events::EventSystem::GetInstance().DispatchEvent<events::CoinRewardEvent>(COINS_TO_LIFE_RATE.first, product->mSceneObjects.front()->mPosition);
        }
        else if (product->mProductName == COINS_TO_LIFE_PRODUCT_NAME)
        {
            ChangeAndAnimateCoinValueReduction(COINS_TO_LIFE_RATE.first);
            
            auto& storyCurrentHealth = DataRepository::GetInstance().StoryCurrentHealth();
            auto healthRestored = math::Min(DataRepository::GetInstance().GetStoryMaxHealth(), storyCurrentHealth.GetValue() + COINS_TO_LIFE_RATE.second) - storyCurrentHealth.GetValue();
            events::EventSystem::GetInstance().DispatchEvent<events::HealthRefillRewardEvent>(healthRestored, product->mSceneObjects.front()->mPosition);
        }
        
        if (productDefinition.mPrice > 0)
        {
            ChangeAndAnimateCoinValueReduction(productDefinition.mPrice);
        }
        
        // Fade out tag and price scene objects
        for (auto i = 1U; i < product->mSceneObjects.size(); ++i)
        {
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(product->mSceneObjects[i], 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ mProducts[productShelfIndex][productShelfItemIndex]->mSceneObjects[i]->mInvisible = true; });
        }
        
        if (std::holds_alternative<int>(productDefinition.mProductTexturePathOrCardId))
        {
            // Add card to player's deck
            auto currentPlayerDeck = DataRepository::GetInstance().GetCurrentStoryPlayerDeck();
            currentPlayerDeck.push_back(std::get<int>(productDefinition.mProductTexturePathOrCardId));
            DataRepository::GetInstance().SetCurrentStoryPlayerDeck(currentPlayerDeck);
            
            AnimateBoughtCardToLibrary(productShelfIndex, productShelfItemIndex);
            
            mSceneState = SceneState::BUYING_CARD_PRODUCT;
        }
        else
        {
            product->mSceneObjects.front()->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + DISSOLVE_SHADER_FILE_NAME);
            product->mSceneObjects.front()->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
            product->mSceneObjects.front()->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
            product->mSceneObjects.front()->mShaderFloatUniformValues[ORIGIN_X_UNIFORM_NAME] = product->mSceneObjects.front()->mPosition.x;
            product->mSceneObjects.front()->mShaderFloatUniformValues[ORIGIN_Y_UNIFORM_NAME] = product->mSceneObjects.front()->mPosition.y;
            product->mSceneObjects.front()->mShaderFloatUniformValues[DISSOLVE_MAGNITUDE_UNIFORM_NAME] = math::RandomFloat(CARD_DISSOLVE_EFFECT_MAG_RANGE.x, CARD_DISSOLVE_EFFECT_MAG_RANGE.y);
            
            mSceneState = SceneState::BUYING_NON_CARD_PRODUCT;
        }
        
        // Commit change to data repository and flush
        DataRepository::GetInstance().AddShopBoughtProductCoordinates(std::make_pair(productShelfIndex, productShelfItemIndex));
        DataRepository::GetInstance().FlushStateToFile();
        
        DestroyCardTooltip();
        
        if (product->mProductName == CARD_DELETION_PRODUCT_NAME)
        {
            // Fade out select card for deletion button
            auto selectCardForDeletionButtonSceneObject = mScene->FindSceneObject(SELECT_CARD_FOR_DELETION_BUTTON_SCENE_OBJECT_NAME);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(selectCardForDeletionButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ selectCardForDeletionButtonSceneObject->mInvisible = true; });
        }
        else
        {
            // Fade out buy button
            auto buyButtonSceneObject = mScene->FindSceneObject(BUY_BUTTON_SCENE_OBJECT_NAME);
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(buyButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ buyButtonSceneObject->mInvisible = true; });
        }
        
        // Fade out cancel button
        auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
        animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cancelButtonSceneObject->mInvisible = true; });
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::FindHighlightedProduct(size_t& productShelfIndex, size_t& productShelfItemIndex)
{
    for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
    {
        for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            if (mProducts[shelfIndex][shelfItemIndex]->mHighlighted)
            {
                productShelfIndex = shelfIndex;
                productShelfItemIndex = shelfItemIndex;
                return;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::OnCantBuyProductConfirmationButtonPressed()
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    // Fade out selected product overlay
    auto cantBuyProductOverlaySceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_OVERLAY_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductOverlaySceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductOverlaySceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cantBuyProductOverlaySceneObject->mInvisible = true; });
    
    // Fade out can't buy product text 0
    auto cantBuyProductText0SceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_TEXT_0_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductText0SceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductText0SceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cantBuyProductText0SceneObject->mInvisible = true; });
    
    // Fade out can't buy product text 1
    auto cantBuyProductText1SceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_TEXT_1_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(cantBuyProductText1SceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductText1SceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cantBuyProductText1SceneObject->mInvisible = true; });
    
    // Fade out cant buy product confirmation button
    auto cantBuyProductButtonSceneObject = mScene->FindSceneObject(CANT_BUY_PRODUCT_CONFIRMATION_BUTTON_SCENE_OBJECT_NAME);
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cantBuyProductButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cantBuyProductButtonSceneObject->mInvisible = true; });
    mSceneState = SceneState::SELECTED_PRODUCT;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::ChangeAndAnimateCoinValueReduction(long long coinValueReduction)
{
    auto& storyCurrencyCoins = DataRepository::GetInstance().CurrencyCoins();
    storyCurrencyCoins.SetValue(storyCurrencyCoins.GetValue() - coinValueReduction);
    
    mCoinAnimationValue = storyCurrencyCoins.GetDisplayedValue();
    mAnimatingCoinValue = true;
    
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(mCoinAnimationValue, static_cast<float>(storyCurrencyCoins.GetValue()), ANIMATED_COIN_VALUE_DURATION_SECS), [=](){ mAnimatingCoinValue = false; });
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::AnimateBoughtCardToLibrary(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    
    // Calculate bezier points for card animation
    auto cardLibraryIconPosition = mScene->FindSceneObject(game_constants::GUI_STORY_CARDS_BUTTON_SCENE_OBJECT_NAME)->mPosition;
    glm::vec3 midPosition = glm::vec3(SELECTED_PRODUCT_TARGET_POSITION + cardLibraryIconPosition)/2.0f;
    midPosition.y += math::RandomSign() == 1 ? CARD_BOUGHT_ANIMATION_MIN_MAX_OFFSETS.t : CARD_BOUGHT_ANIMATION_MIN_MAX_OFFSETS.s ;
    math::BezierCurve curve({SELECTED_PRODUCT_TARGET_POSITION, midPosition, cardLibraryIconPosition});
    
    // Animate bought card to card library icon
    animationManager.StartAnimation(std::make_unique<rendering::BezierCurveAnimation>(product->mSceneObjects.front()->mPosition, curve, CARD_BOUGHT_ANIMATION_DURATION_SECS), [=](){ mSceneState = SceneState::FINISHING_PRODUCT_PURCHASE; });
    
    // And its alpha
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(product->mSceneObjects.front(), CARD_BOUGHT_ANIMATION_MIN_ALPHA, CARD_BOUGHT_ANIMATION_DURATION_SECS), [=](){ mProducts[productShelfIndex][productShelfItemIndex]->mSceneObjects[0]->mInvisible = true; });
    
    // And its scale
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(product->mSceneObjects.front(), glm::vec3(), CARD_PRODUCT_SCALE, CARD_BOUGHT_ANIMATION_DURATION_SECS, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
    {
        events::EventSystem::GetInstance().DispatchEvent<events::GuiRewardAnimationFinishedEvent>();
   
        // And pulse card library icon
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto cardLibraryIconSceneObject = mScene->FindSceneObject(game_constants::GUI_STORY_CARDS_BUTTON_SCENE_OBJECT_NAME);
        auto originalScale = cardLibraryIconSceneObject->mScale;
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardLibraryIconSceneObject, cardLibraryIconSceneObject->mPosition, originalScale * CARD_BOUGHT_ANIMATION_LIBRARY_ICON_PULSE_FACTOR, CARD_BOUGHT_ANIMATION_LIBRARY_ICON_PULSE_DURATION_SECS, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
        {
            auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardLibraryIconSceneObject, cardLibraryIconSceneObject->mPosition, originalScale, CARD_BOUGHT_ANIMATION_LIBRARY_ICON_PULSE_DURATION_SECS, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
            {
                cardLibraryIconSceneObject->mScale = originalScale;
            });
        });
    });
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::UpdateProductPriceTags()
{
    for (int shelfIndex = 0; shelfIndex < SHELF_COUNT; ++shelfIndex)
    {
        for (int shelfItemIndex = 0; shelfItemIndex < SHELF_ITEM_COUNT; ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            auto& product = mProducts[shelfIndex][shelfItemIndex];
            const auto& productDefinition = mProductDefinitions.at(product->mProductName);
            
            if (product->mProductName == COINS_S_PRODUCT_NAME ||
                product->mProductName == COINS_M_PRODUCT_NAME ||
                product->mProductName == COINS_L_PRODUCT_NAME)
            {
                product->mSceneObjects[2]->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = COIN_NORMAL_VALUE_TEXT_COLOR;
                continue;
            }
            
            if (productDefinition.mPrice > 0)
            {
                product->mSceneObjects[2]->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = productDefinition.mPrice > DataRepository::GetInstance().CurrencyCoins().GetValue() ? COIN_RED_VALUE_TEXT_COLOR : COIN_NORMAL_VALUE_TEXT_COLOR;
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::CheckProductsFinishedFadingIn()
{
    mItemsFinishedFadingIn = true;
    for (int shelfIndex = 0; shelfIndex < SHELF_COUNT; ++shelfIndex)
    {
        for (int shelfItemIndex = 0; shelfItemIndex < SHELF_ITEM_COUNT; ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            auto& product = mProducts[shelfIndex][shelfItemIndex];
            
            for (auto sceneObject: product->mSceneObjects)
            {
                if (sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] < 1.0f)
                {
                    mItemsFinishedFadingIn = false;
                    return;
                }
            }
        }
    }
}

///------------------------------------------------------------------------------------------------
