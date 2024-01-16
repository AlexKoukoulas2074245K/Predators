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
static const strutils::StringId DAMAGE_GAIN_PRODUCT_NAME = strutils::StringId("damage_gain_+1");
static const strutils::StringId WEIGHT_GAIN_PRODUCT_NAME = strutils::StringId("weight_gain_+1");
static const strutils::StringId COINS_TO_LIFE_PRODUCT_NAME = strutils::StringId("coins_to_life");
static const strutils::StringId LIFE_TO_COINS_PRODUCT_NAME = strutils::StringId("life_to_coins");
static const strutils::StringId CARD_DELETION_PRODUCT_NAME = strutils::StringId("card_deletion");
static const strutils::StringId CARD_TOOLTIP_SCENE_OBJECT_NAME = strutils::StringId("card_tooltip");
static const strutils::StringId CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME = strutils::StringId("reveal_threshold");
static const strutils::StringId CARD_TOOLTIP_REVEAL_RGB_EXPONENT_UNIFORM_NAME = strutils::StringId("reveal_rgb_exponent");
static const strutils::StringId CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES [game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT] =
{
    strutils::StringId("card_tooltip_text_0"),
    strutils::StringId("card_tooltip_text_1"),
    strutils::StringId("card_tooltip_text_2"),
    strutils::StringId("card_tooltip_text_3")
};

static const std::string BASIC_CUSTOM_COLOR_SHADER_FILE_NAME = "basic_custom_color.vs";
static const std::string PRICE_TAG_TEXTURE_FILE_NAME_PREFIX = "shop_items/price_tag_digits_";
static const std::string PRODUCT_NAME_PREFIX = "product_";
static const std::string CARD_TOOLTIP_TEXTURE_FILE_NAME = "tooltip.png";
static const std::string CARD_TOOLTIP_SHADER_FILE_NAME = "diagonal_reveal.vs";

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
static const glm::vec3 CARD_TOOLTIP_POSITION_OFFSET = {0.0f, 0.1f, 0.0f};
static const glm::vec3 CARD_TOOLTIP_BASE_SCALE = {0.274f, 0.274f, 1/10.0f};
static const glm::vec3 CARD_TOOLTIP_BASE_OFFSET = {0.06f, 0.033f, 0.2f};
static const glm::vec3 CARD_TOOLTIP_TEXT_OFFSETS[game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT] =
{
    { -0.033f, 0.029f, 0.1f },
    { -0.051f, 0.014f, 0.1f },
    { -0.036f, -0.000f, 0.1f },
    { -0.03f, -0.014f, 0.1f }
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
static const float CARD_TOOLTIP_MAX_REVEAL_THRESHOLD = 2.0f;
static const float CARD_TOOLTIP_REVEAL_SPEED = 1.0f/200.0f;
static const float CARD_TOOLTIP_TEXT_REVEAL_SPEED = 1.0f/500.0f;
static const float CARD_TOOLTIP_FLIPPED_X_OFFSET = -0.17f;
static const float CARD_TOOLTIP_FLIPPED_Y_OFFSET = -0.25f;
static const float CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET = -0.007f;
static const float CARD_TOOLTIP_TEXT_FLIPPED_Y_OFFSET = -0.014f;

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
            
            auto cardTooltipSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
            cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_TOOLTIP_REVEAL_SPEED;
            if (cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] >= CARD_TOOLTIP_MAX_REVEAL_THRESHOLD)
            {
                cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = CARD_TOOLTIP_MAX_REVEAL_THRESHOLD;
                
                for (auto i = 0; i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
                {
                    auto tooltipTextSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
                    tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Min(1.0f, tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] +  dtMillis * CARD_TOOLTIP_TEXT_REVEAL_SPEED);
                }
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
    
    // Thid Shelf
    mProducts[2][0] = std::make_unique<ProductInstance>(COINS_TO_LIFE_PRODUCT_NAME);
    mProducts[2][1] = std::make_unique<ProductInstance>(LIFE_TO_COINS_PRODUCT_NAME);
    mProducts[2][2] = std::make_unique<ProductInstance>(CARD_DELETION_PRODUCT_NAME);
    
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
                auto cardSoWrapper = card_utils::CreateCardSoWrapper(&cardData, glm::vec3(), PRODUCT_NAME_PREFIX + std::to_string(shelfIndex) + "_" + std::to_string(shelfItemIndex), CardOrientation::FRONT_FACE, CardRarity::NORMAL, false, false, true, {}, {}, *mScene);

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
                priceTextData.mText = std::to_string(productDefinition.mPrice) + "$";
                
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
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleGroupAnimation>(product->mSceneObjects, SELECTED_PRODUCT_TARGET_POSITION, (std::holds_alternative<int>(productDefinition.mProductTexturePathOrCardId) ? CARD_PRODUCT_SCALE : GENERIC_PRODUCT_SCALE) * SELECTED_PRODUCT_SCALE_FACTOR, PRODUCT_HIGHLIGHT_ANIMATION_DURATION_SECS), [=]()
    {
        // Create card tooltip if necessary
        auto& product = mProducts[productShelfIndex][productShelfItemIndex];
        const auto& productDefinition = mProductDefinitions.at(product->mProductName);
        
        if (!productDefinition.mDescription.empty())
        {
            CreateCardTooltip(SELECTED_PRODUCT_TARGET_POSITION + CARD_TOOLTIP_POSITION_OFFSET, productDefinition.mDescription);
        }
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
    auto tooltipSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    bool shouldBeHorFlipped = cardOriginPostion.x > 0.0f;
    bool shouldBeVerFlipped = cardOriginPostion.y > 0.1f;
    
    tooltipSceneObject->mPosition = cardOriginPostion + CARD_TOOLTIP_BASE_OFFSET;
    tooltipSceneObject->mPosition.x += shouldBeHorFlipped ? CARD_TOOLTIP_FLIPPED_X_OFFSET : 0.046f;
    tooltipSceneObject->mPosition.y += shouldBeVerFlipped ? CARD_TOOLTIP_FLIPPED_Y_OFFSET : 0.0f;
    
    tooltipSceneObject->mInvisible = false;
    tooltipSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    tooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = 0.0f;
    tooltipSceneObject->mScale.x = shouldBeHorFlipped ? -CARD_TOOLTIP_BASE_SCALE.x : CARD_TOOLTIP_BASE_SCALE.x;
    tooltipSceneObject->mScale.y = shouldBeVerFlipped ? -CARD_TOOLTIP_BASE_SCALE.y : CARD_TOOLTIP_BASE_SCALE.y;
    
    auto tooltipTextRows = strutils::StringSplit(tooltipText, '$');
    
    if (tooltipTextRows.size() == 1)
    {
        auto tooltipTextSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[1]);
        tooltipTextSceneObject->mPosition = tooltipSceneObject->mPosition;
        tooltipTextSceneObject->mPosition += 2.0f * CARD_TOOLTIP_TEXT_OFFSETS[1];
        tooltipTextSceneObject->mPosition.x += shouldBeHorFlipped ? (2.0f * CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET) : 0.0f;
        tooltipTextSceneObject->mPosition.y += shouldBeVerFlipped ? (2.0f * CARD_TOOLTIP_TEXT_FLIPPED_Y_OFFSET) : 0.0f;
        tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        std::get<scene::TextSceneObjectData>(tooltipTextSceneObject->mSceneObjectTypeData).mText = tooltipTextRows[0];
        tooltipTextSceneObject->mInvisible = false;
    }
    else
    {
        for (auto i = 0U; i < tooltipTextRows.size(); ++i)
        {
            assert(i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT);
            auto tooltipTextSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
            tooltipTextSceneObject->mPosition = tooltipSceneObject->mPosition;
            tooltipTextSceneObject->mPosition += 2.0f * CARD_TOOLTIP_TEXT_OFFSETS[i];
            tooltipTextSceneObject->mPosition.x += shouldBeHorFlipped ? (2.0f * CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET) : 0.0f;
            tooltipTextSceneObject->mPosition.y += shouldBeVerFlipped ? (2.0f * CARD_TOOLTIP_TEXT_FLIPPED_Y_OFFSET) : 0.0f;
            tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            std::get<scene::TextSceneObjectData>(tooltipTextSceneObject->mSceneObjectTypeData).mText = tooltipTextRows[i];
            tooltipTextSceneObject->mInvisible = false;
        }
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::DestroyCardTooltip()
{
    auto tooltipSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    tooltipSceneObject->mInvisible = true;

    for (auto i = 0; i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
    {
        auto tooltipTextSceneObject = mScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
        tooltipTextSceneObject->mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void ShopSceneLogicManager::LoadProductData()
{
      auto& systemsEngine = CoreSystemsEngine::GetInstance();
      
  #if !defined(NDEBUG)
      auto productDefinitionJsonResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "shop_product_data.json", resources::DONT_RELOAD);
      const auto particlesJson =  nlohmann::json::parse(systemsEngine.GetResourceLoadingService().GetResource<resources::DataFileResource>(productDefinitionJsonResourceId).GetContents());
  #else
      const auto particlesJson = serial::BaseDataFileDeserializer("shop_product_data", serial::DataFileType::ASSET_FILE_TYPE, serial::WarnOnFileNotFoundBehavior::WARN, serial::CheckSumValidationBehavior::VALIDATE_CHECKSUM).GetState();
  #endif
    
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
