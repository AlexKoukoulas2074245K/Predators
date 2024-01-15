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
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/GuiObjectManager.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/ShopSceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static constexpr int SHELF_COUNT = 3;
static constexpr int SHELF_ITEM_COUNT = 3;

static const strutils::StringId SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("selected_product_overlay");
static const strutils::StringId CONTINUE_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("continue_button");
static const strutils::StringId BUY_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("buy_button");
static const strutils::StringId CANCEL_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("cancel_button");
static const strutils::StringId DEFEAT_SCENE_NAME = strutils::StringId("defeat_scene");
static const strutils::StringId DAMAGE_GAIN_UNIQUE_ID = strutils::StringId("shop_items/damage_gain_icon.png");
static const strutils::StringId WEIGHT_GAIN_UNIQUE_ID = strutils::StringId("shop_items/weight_gain_icon.png");
static const strutils::StringId COINS_TO_LIFE_UNIQUE_ID = strutils::StringId("shop_items/coins_to_life_icon.png");
static const strutils::StringId LIFE_TO_COINS_UNIQUE_ID = strutils::StringId("shop_items/life_to_coins_icon.png");
static const strutils::StringId CARD_DELETION_UNIQUE_ID = strutils::StringId("shop_items/delete_card_icon.png");

static const std::string BASIC_CUSTOM_COLOR_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string PRICE_TAG_TEXTURE_FILE_NAME_PREFIX = "shop_items/price_tag_digits_";
static const std::string PRODUCT_NAME_PREFIX = "product_";

static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 CONTINUE_BUTTON_POSITION = {0.0f, -0.1f, 0.3f};
static const glm::vec3 BUY_BUTTON_POSITION = {-0.225f, 0.05f, 6.0f};
static const glm::vec3 CANCEL_BUTTON_POSITION = {-0.25f, -0.05f, 6.0f};
static const glm::vec3 COIN_VALUE_TEXT_COLOR = {0.80f, 0.71f, 0.11f};
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

static const glm::vec2 PRODUCT_GROUP_MIN_MAX_BOUNCE_SPEED = {0.0000015f, 0.0000045f};
static const glm::vec2 PRODUCT_GROUP_MIN_MAX_ANIMATION_DELAY_SECS = {0.0f, 1.0f};

static const float PRODUCT_BOUNCE_ANIMATION_DURATION_SECS = 1.0f;
static const float CONTINUE_BUTTON_SNAP_TO_EDGE_FACTOR = 950000.0f;
static const float FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float HIGHLIGHTED_PRODUCT_SCALE_FACTOR = 1.25f;
static const float SELECTED_PRODUCT_SCALE_FACTOR = 2.0f;
static const float PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS = 0.35f;
static const float STAGGERED_FADE_IN_SECS = 0.1f;
static const float SELECTED_PRODUCT_OVERLAY_MAX_ALPHA = 0.95f;

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
    
    mScene = scene;
    mGuiManager = std::make_shared<GuiObjectManager>(scene);
    
    RegisterForEvents();
    
    math::SetControlSeed(ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeSeed());
    ProgressionDataRepository::GetInstance().SetCurrentStoryMapSceneType(StoryMapSceneType::SHOP);
    
    mSceneState = SceneState::CREATING_DYNAMIC_OBJECTS;
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    switch (mSceneState)
    {
        case SceneState::CREATING_DYNAMIC_OBJECTS:
        {
            CreateDynamicSceneObjects();
            FadeInDynamicSceneObjects();

            OnWindowResize(events::WindowResizeEvent{});
            mSceneState = SceneState::BROWSING_SHOP;
        } break;
            
        case SceneState::BROWSING_SHOP:
        {
            if (!mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME)->mInvisible)
            {
                return;
            }
            
            mGuiManager->Update(dtMillis);
            
            auto& progressionHealth = ProgressionDataRepository::GetInstance().StoryCurrentHealth();
            if (progressionHealth.GetDisplayedValue() <= 0)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(DEFEAT_SCENE_NAME, SceneChangeType::MODAL_SCENE, PreviousSceneDestructionType::RETAIN_PREVIOUS_SCENE);
                mSceneState = SceneState::LEAVING_SHOP;
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
        } break;
            
        default: break;
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene>)
{
    mAnimatedButtons.clear();
    mProducts.clear();
    mGuiManager = nullptr;
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
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::SHOP_SCENE)->RecalculatePositionOfEdgeSnappingSceneObjects();
    
    // Realign gui
    mGuiManager->OnWindowResize();
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
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::STORY_MAP_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
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
                        DeselectProduct(shelfIndex, shelfItemIndex);
                    }
                }
            }
            
            mSceneState = SceneState::BROWSING_SHOP;
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
    mItemsFinishedFadingIn = false;
    for (auto shelfIndex = 0U; shelfIndex < mProducts.size(); ++shelfIndex)
    {
        for (auto shelfItemIndex = 0U; shelfItemIndex < mProducts[shelfIndex].size(); ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            auto& product = mProducts[shelfIndex][shelfItemIndex];
            auto productSceneObjectCount = product->mSceneObjects.size();
            for (auto sceneObjectIndex = 0U; sceneObjectIndex < product->mSceneObjects.size(); ++sceneObjectIndex)
            {
                auto sceneObject = product->mSceneObjects[sceneObjectIndex];
                sceneObject->mInvisible = false;
                sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectCounter++ * STAGGERED_FADE_IN_SECS), [=]()
                {
                    if (shelfIndex == SHELF_COUNT - 1 && shelfItemIndex == SHELF_ITEM_COUNT - 1 && sceneObjectIndex == productSceneObjectCount - 1)
                    {
                        mItemsFinishedFadingIn = true;
                    }
                });
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
    
    // First Shelf
    mProducts[0][0] = std::make_unique<Product>(DAMAGE_GAIN_UNIQUE_ID, 500, true);
    mProducts[0][1] = std::make_unique<Product>(WEIGHT_GAIN_UNIQUE_ID, 200, true);
    
    // Second Shelf
    const auto& cardRewardsPool = CardDataRepository::GetInstance().GetStoryUnlockedCardRewardsPool();
    for (size_t col = 0; col < SHELF_ITEM_COUNT; ++col)
    {
        auto randomCardIndex = static_cast<int>(math::ControlledRandomInt() % cardRewardsPool.size());
        mProducts[1][col] = std::make_unique<Product>(cardRewardsPool[randomCardIndex], 50, true);
    }
    
    // Thid Shelf
    mProducts[2][0] = std::make_unique<Product>(COINS_TO_LIFE_UNIQUE_ID, 0, true);
    mProducts[2][1] = std::make_unique<Product>(LIFE_TO_COINS_UNIQUE_ID, 0, true);
    mProducts[2][2] = std::make_unique<Product>(CARD_DELETION_UNIQUE_ID, 100, true);
    
    for (int shelfIndex = 0; shelfIndex < SHELF_COUNT; ++shelfIndex)
    {
        for (int shelfItemIndex = 0; shelfItemIndex < SHELF_ITEM_COUNT; ++shelfItemIndex)
        {
            if (mProducts[shelfIndex][shelfItemIndex] == nullptr)
            {
                continue;
            }
            
            auto& product = mProducts[shelfIndex][shelfItemIndex];
            
            // Generic Product
            if (std::holds_alternative<strutils::StringId>(product->mProductId))
            {
                auto shelfItemSceneObject = mScene->CreateSceneObject(strutils::StringId(PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex)));
                shelfItemSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::get<strutils::StringId>(product->mProductId).GetString());
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
                auto cardId = std::get<int>(product->mProductId);
                auto cardData = CardDataRepository::GetInstance().GetCardData(cardId, game_constants::LOCAL_PLAYER_INDEX);
                auto cardSoWrapper = card_utils::CreateCardSoWrapper(&cardData, glm::vec3(), PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex), CardOrientation::FRONT_FACE, CardRarity::NORMAL, false, false, true, {}, {}, *mScene);

                cardSoWrapper->mSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_POSITION_OFFSET;
                cardSoWrapper->mSceneObject->mScale = CARD_PRODUCT_SCALE;
                cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                cardSoWrapper->mSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
                cardSoWrapper->mSceneObject->mSnapToEdgeScaleOffsetFactor = -0.4f - 1.2f * shelfItemIndex;
                
                product->mSceneObjects.push_back(cardSoWrapper->mSceneObject);
            }
            
            if (product->mPrice > 0)
            {
                auto priceTagSceneObject = mScene->CreateSceneObject(strutils::StringId(PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex) + "_tag"));
                priceTagSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + PRICE_TAG_TEXTURE_FILE_NAME_PREFIX + std::to_string(std::to_string(product->mPrice).size()) + ".png");
                priceTagSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_PRICE_TAG_POSITION_OFFSET;
                priceTagSceneObject->mScale = PRICE_TAG_SCALE;
                priceTagSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                priceTagSceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
                priceTagSceneObject->mSnapToEdgeScaleOffsetFactor = 1.1f + 1.5f * shelfItemIndex;
                product->mSceneObjects.push_back(priceTagSceneObject);
                
                scene::TextSceneObjectData priceTextData;
                priceTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
                priceTextData.mText = std::to_string(product->mPrice) + "$";
                
                auto priceTextSceneObject = mScene->CreateSceneObject(strutils::StringId(PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex) + "_price_text"));
                priceTextSceneObject->mPosition = SHELF_ITEM_TARGET_BASE_POSITIONS[shelfIndex] + PRODUCT_PRICE_TAG_TEXT_POSITION_OFFSET;
                priceTextSceneObject->mSceneObjectTypeData = std::move(priceTextData);
                priceTextSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BASIC_CUSTOM_COLOR_SHADER_FILE_NAME);
                priceTextSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = COIN_VALUE_TEXT_COLOR;
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

void ShopSceneLogicManager::HighlightProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, product->mSceneObjects[0]->mPosition, (std::holds_alternative<int>(product->mProductId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE) * HIGHLIGHTED_PRODUCT_SCALE_FACTOR, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::DehighlightProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, product->mSceneObjects[0]->mPosition, (std::holds_alternative<int>(product->mProductId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE), PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [=](){});
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::SelectProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    
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
    
    // Fade in buy button
    auto buyButtonSceneObject = mScene->FindSceneObject(BUY_BUTTON_SCENE_OBJECT_NAME);
    buyButtonSceneObject->mInvisible = false;
    animationManager.StopAllAnimationsPlayingForSceneObject(buyButtonSceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(buyButtonSceneObject, 1.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});

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
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, SELECTED_PRODUCT_TARGET_POSITION, (std::holds_alternative<int>(product->mProductId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE) * SELECTED_PRODUCT_SCALE_FACTOR, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){});
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::DeselectProduct(const size_t productShelfIndex, const size_t productShelfItemIndex)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& product = mProducts[productShelfIndex][productShelfItemIndex];
    product->mHighlighted = false;
    
    for (auto& sceneObject: product->mSceneObjects)
    {
        animationManager.StopAllAnimationsPlayingForSceneObject(sceneObject->mName);
        sceneObject->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE;
    }
    
    // Fade out buy button
    auto buyButtonSceneObject = mScene->FindSceneObject(BUY_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(buyButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ buyButtonSceneObject->mInvisible = true; });
    
    // Fade out cancel button
    auto cancelButtonSceneObject = mScene->FindSceneObject(CANCEL_BUTTON_SCENE_OBJECT_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(cancelButtonSceneObject, 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ cancelButtonSceneObject->mInvisible = true; });
    
    // Fade in selected product overlay
    auto selectedProductOverlaySceneObject = mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME);
    animationManager.StopAllAnimationsPlayingForSceneObject(selectedProductOverlaySceneObject->mName);
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(mScene->FindSceneObject(SELECTED_PRODUCT_OVERLAY_SCENE_OBJECT_NAME), 0.0f, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=](){ selectedProductOverlaySceneObject->mInvisible = true; });
    
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, mSelectedProductInitialPosition, (std::holds_alternative<int>(product->mProductId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE), PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=]()
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
