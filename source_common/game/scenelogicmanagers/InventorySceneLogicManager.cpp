///------------------------------------------------------------------------------------------------
///  InventorySceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/02/2024
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/CardTooltipController.h>
#include <game/CardUtils.h>
#include <game/GameSceneTransitionManager.h>
#include <game/GuiObjectManager.h>
#include <game/DataRepository.h>
#include <game/scenelogicmanagers/InventorySceneLogicManager.h>
#include <game/ProductRepository.h>

///------------------------------------------------------------------------------------------------

static const std::string ARTIFACT_ITEM_ENTRY_SHADER = "artifact_container_entry.vs";
static const std::string ARTIFACT_TEXT_ITEM_ENTRY_SHADER = "artifact_text_container_entry.vs";
static const std::string MUTATION_ITEM_ENTRY_SHADER = "mutation_container_entry.vs";

static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId ARTIFACTS_TITLE_SCENE_OBJECT_NAME = strutils::StringId("inventory_artifacts_title");
static const strutils::StringId MUTATIONS_TITLE_SCENE_OBJECT_NAME = strutils::StringId("inventory_mutations_title");
static const strutils::StringId ARTIFACT_ITEM_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("artifact_item_container");
static const strutils::StringId MUTATION_ITEM_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("mutation_item_container");

static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.0f, -0.2f, 23.2f};
static const glm::vec3 ITEM_ENTRY_SCALE = glm::vec3(0.273f/1.5f, 0.2512f/1.5f, 2.0f);
static const glm::vec3 ITEM_TOOLTIP_POSITION_OFFSET = {0.0f, 0.1f, 0.0f};
static const glm::vec3 ITEM_TOOLTIP_BASE_SCALE = {0.274f, 0.274f, 1/10.0f};
static const glm::vec3 ARTIFACT_CONTAINER_ITEM_ENTRY_SCALE = {0.173f, 0.142f, 2.0f};
static const glm::vec3 MUTATION_CONTAINER_ITEM_ENTRY_SCALE = {0.34f, 0.142f, 2.0f};
static const glm::vec3 ARTIFACT_TEXT_SCALE = {0.00025f, 0.00025f, 0.00025f};
static const glm::vec3 ARTIFACT_NAME_TEXT_OFFSET = glm::vec3(-0.06f, 0.05f, 0.1f);
static const glm::vec3 ARTIFACT_COUNT_TEXT_OFFSET = glm::vec3(-0.06f, 0.0f, 0.1f);

static const glm::vec2 MUTATION_ITEM_ENTRY_CUTOFF_VALUES = {-0.27f, 0.2f};
static const glm::vec2 MUTATION_ITEM_CONTAINER_CUTOFF_VALUES = {-0.15f, 0.15f};
static const glm::vec2 ARTIFACT_ITEM_ENTRY_CUTOFF_VALUES = {-0.047f, 0.183f};
static const glm::vec2 ARTIFACT_ITEM_CONTAINER_CUTOFF_VALUES = {0.076, 0.093f};
static const glm::vec2 NO_MUTATIONS_ARTIFACT_ITEM_ENTRY_CUTOFF_VALUES = {-0.185f, 0.183f};
static const glm::vec2 NO_MUTATIONS_ARTIFACT_ITEM_CONTAINER_CUTOFF_VALUES = {0.076, 0.093f};

static const math::Rectangle ARTIFACT_ITEM_CONTAINER_BOUNDS = {{-0.305f, -0.0525f}, {0.305f, 0.182f}};
static const math::Rectangle NO_MUTATIONS_ARTIFACT_ITEM_CONTAINER_BOUNDS = {{-0.305f, -0.250f}, {0.305f, 0.182f}};
static const math::Rectangle MUTATION_ITEM_CONTAINER_BOUNDS = {{-0.382f, -0.250f}, {0.305f, -0.08f}};

static const float ITEMS_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.05f;
static const float BACK_BUTTON_SNAP_TO_EDGE_FACTOR = 950000.0f;
static const float ITEM_ENTRY_Z = 23.2f;

static const int MIN_CONTAINER_ENTRIES_TO_ANIMATE = 4;
static const int NO_MUTATIONS_MIN_CONTAINER_ENTRIES_TO_ANIMATE = 10;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::INVENTORY_SCENE
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& InventorySceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

InventorySceneLogicManager::InventorySceneLogicManager(){}

///------------------------------------------------------------------------------------------------

InventorySceneLogicManager::~InventorySceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mScene = scene;

    mArtifactsItemContainer = nullptr;
    mMutationsItemContainer = nullptr;
    mSelectedItemIndex = -1;
    
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
    mAnimatedButtons.back()->GetSceneObject()->mInvisible = true;
    mAnimatedButtons.back()->GetSceneObject()->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    CreateItemEntriesAndContainer();
    
    // Staggered Item Presentation
    size_t sceneObjectIndex = 0;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (STATIC_SCENE_ELEMENTS.count(sceneObject->mName))
        {
            continue;
        }

        if (sceneObject->mName != MUTATIONS_TITLE_SCENE_OBJECT_NAME)
        {
            sceneObject->mInvisible = false;
        }
        
        sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, ITEMS_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=](){});
    }
    
    events::EventSystem::GetInstance().RegisterForEvent<events::WindowResizeEvent>(this, &InventorySceneLogicManager::OnWindowResize);
    mTransitioning = false;
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioning)
    {
        return;
    }
    
    UpdateItemContainer(dtMillis, mArtifactsItemContainer);
    UpdateItemContainer(dtMillis, mMutationsItemContainer);
    
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
    
    if (mItemTooltipController)
    {
        mItemTooltipController->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    DestroyItemTooltip();
    
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, ITEMS_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            if (sceneObject->mName == ARTIFACTS_TITLE_SCENE_OBJECT_NAME || sceneObject->mName == MUTATIONS_TITLE_SCENE_OBJECT_NAME)
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

std::shared_ptr<GuiObjectManager> InventorySceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::UpdateItemContainer(const float dtMillis, std::unique_ptr<SwipeableContainer<ItemEntry>>& itemContainer)
{
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    for (auto i = 0; i < itemContainer->GetItems().size(); ++i)
    {
        for (auto& sceneObject: itemContainer->GetItems()[i].mSceneObjects)
        {
            sceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time + i;
        }
    }
    
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    
    if (itemContainer)
    {
        static int sToolTipIndices[2] = {-1, -1};
        static float sToolTipPointeePosYCoords[2] = {0.0f, 0.0f};
        
        int& sToolTipIndex = itemContainer == mArtifactsItemContainer ? sToolTipIndices[0] : sToolTipIndices[1];
        float& sToolTipPointeePosY = itemContainer == mArtifactsItemContainer ? sToolTipPointeePosYCoords[0] : sToolTipPointeePosYCoords[1];

        const auto& itemContainerUpdateResult = itemContainer->Update(dtMillis);
        
        if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && itemContainerUpdateResult.mInteractionType != InteractionType::NONE)
        {
            mSelectedItemIndex = -1;
            DestroyItemTooltip();
        }
        
        if (itemContainerUpdateResult.mInteractionType == InteractionType::INTERACTED_WITH_ELEMENTS)
        {
            if (sToolTipIndex != itemContainerUpdateResult.mInteractedElementIndex)
            {
                sToolTipIndex = itemContainerUpdateResult.mInteractedElementIndex;
                auto interactedElementEntry = itemContainer->GetItems()[itemContainerUpdateResult.mInteractedElementIndex];
                
                sToolTipPointeePosY = interactedElementEntry.mSceneObjects.front()->mPosition.y;
                
                auto productDescription = ProductRepository::GetInstance().GetProductDefinition(interactedElementEntry.mArtifactOrMutationName).mDescription;
                CreateItemTooltip(interactedElementEntry.mSceneObjects.front()->mPosition, productDescription);
            }
        }
        
        if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
        {
            mSelectedItemIndex = -1;
        }
        
        if (sToolTipIndex != -1)
        {
            auto interactedElementEntry = itemContainer->GetItems()[sToolTipIndex];
            if (math::Abs(interactedElementEntry.mSceneObjects.front()->mPosition.y - sToolTipPointeePosY) > 0.01f)
            {
                sToolTipIndex = -1;
                DestroyItemTooltip();
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    mScene->RecalculatePositionOfEdgeSnappingSceneObjects();
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::CreateItemEntriesAndContainer()
{
    // Clean up existing container
    bool containerExists = mArtifactsItemContainer != nullptr;
    if (containerExists)
    {
        for (const auto& containerItem: mArtifactsItemContainer->GetItems())
        {
            for (const auto& sceneObject: containerItem.mSceneObjects)
            {
                CoreSystemsEngine::GetInstance().GetAnimationManager().StopAllAnimationsPlayingForSceneObject(sceneObject->mName);
                mScene->RemoveSceneObject(sceneObject->mName);
            }
        }
        
        for (const auto& containerItem: mMutationsItemContainer->GetItems())
        {
            for (const auto& sceneObject: containerItem.mSceneObjects)
            {
                CoreSystemsEngine::GetInstance().GetAnimationManager().StopAllAnimationsPlayingForSceneObject(sceneObject->mName);
                mScene->RemoveSceneObject(sceneObject->mName);
            }
        }
        
        mArtifactsItemContainer = nullptr;
        mMutationsItemContainer = nullptr;
    }
    
    // Item Containers
    mMutationsItemContainer = std::make_unique<SwipeableContainer<ItemEntry>>
    (
        ContainerType::HORIZONTAL_LINE,
        MUTATION_CONTAINER_ITEM_ENTRY_SCALE,
        MUTATION_ITEM_CONTAINER_BOUNDS,
        MUTATION_ITEM_CONTAINER_CUTOFF_VALUES,
        MUTATION_ITEM_CONTAINER_SCENE_OBJECT_NAME,
        ITEM_ENTRY_Z,
        *mScene,
        MIN_CONTAINER_ENTRIES_TO_ANIMATE
    );
    
    // Create artifact entries
    //TODO: mutations here
    auto mutationCount = 0;
    for (const auto& artifactEntry: DataRepository::GetInstance().GetCurrentStoryArtifacts())
    {
        mutationCount += artifactEntry.second;
        const auto& artifactItemProduct = ProductRepository::GetInstance().GetProductDefinition(artifactEntry.first);

        auto artifactSceneObject = mScene->CreateSceneObject();
        artifactSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + MUTATION_ITEM_ENTRY_SHADER);
        artifactSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::get<std::string>(artifactItemProduct.mProductTexturePathOrCardId));
        artifactSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_X_UNIFORM_NAME] = MUTATION_ITEM_ENTRY_CUTOFF_VALUES.s;
        artifactSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_X_UNIFORM_NAME] = MUTATION_ITEM_ENTRY_CUTOFF_VALUES.t;
        artifactSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        artifactSceneObject->mScale = ITEM_ENTRY_SCALE;

        ItemEntry itemEntry;
        itemEntry.mArtifactOrMutationName = artifactEntry.first;
        itemEntry.mSceneObjects.push_back(artifactSceneObject);

        mMutationsItemContainer->AddItem(std::move(itemEntry), EntryAdditionStrategy::ADD_ON_THE_BACK);
    }
    
    mArtifactsItemContainer = std::make_unique<SwipeableContainer<ItemEntry>>
    (
        ContainerType::VERTICAL_MATRIX,
        ARTIFACT_CONTAINER_ITEM_ENTRY_SCALE,
        mutationCount == 0 ? NO_MUTATIONS_ARTIFACT_ITEM_CONTAINER_BOUNDS : ARTIFACT_ITEM_CONTAINER_BOUNDS,
        mutationCount == 0 ? NO_MUTATIONS_ARTIFACT_ITEM_CONTAINER_CUTOFF_VALUES : ARTIFACT_ITEM_CONTAINER_CUTOFF_VALUES,
        ARTIFACT_ITEM_CONTAINER_SCENE_OBJECT_NAME,
        ITEM_ENTRY_Z,
        *mScene,
        mutationCount == 0 ? NO_MUTATIONS_MIN_CONTAINER_ENTRIES_TO_ANIMATE : MIN_CONTAINER_ENTRIES_TO_ANIMATE
    );
    
    auto artifactCount = 0;
    auto shaderCutoffValues = mutationCount == 0 ? NO_MUTATIONS_ARTIFACT_ITEM_ENTRY_CUTOFF_VALUES : ARTIFACT_ITEM_ENTRY_CUTOFF_VALUES;
    const auto& artifactEntries = DataRepository::GetInstance().GetCurrentStoryArtifacts();
    for (const auto& artifactEntry: artifactEntries)
    {
        artifactCount += artifactEntry.second;
        const auto& artifactItemProduct = ProductRepository::GetInstance().GetProductDefinition(artifactEntry.first);
        
        auto artifactSceneObject = mScene->CreateSceneObject();
        artifactSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + ARTIFACT_ITEM_ENTRY_SHADER);
        artifactSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + std::get<std::string>(artifactItemProduct.mProductTexturePathOrCardId));
        artifactSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_Y_UNIFORM_NAME] = shaderCutoffValues.s;
        artifactSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_Y_UNIFORM_NAME] = shaderCutoffValues.t;
        artifactSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        artifactSceneObject->mScale = ITEM_ENTRY_SCALE;
        
        auto artifactCountTextSceneObject = mScene->CreateSceneObject();
        artifactCountTextSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + ARTIFACT_TEXT_ITEM_ENTRY_SHADER);
        
        scene::TextSceneObjectData artifactCountText;
        artifactCountText.mText = std::to_string(artifactEntry.second) + " x";
        artifactCountText.mFontName = game_constants::DEFAULT_FONT_NAME;
        
        artifactCountTextSceneObject->mSceneObjectTypeData = std::move(artifactCountText);
        artifactCountTextSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_Y_UNIFORM_NAME] = shaderCutoffValues.s;
        artifactCountTextSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_Y_UNIFORM_NAME] = shaderCutoffValues.t;
        artifactCountTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        artifactCountTextSceneObject->mScale = ARTIFACT_TEXT_SCALE;
        artifactCountTextSceneObject->mPosition += ARTIFACT_COUNT_TEXT_OFFSET;
        
        auto artifactNameTextSceneObject = mScene->CreateSceneObject();
        artifactNameTextSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + ARTIFACT_TEXT_ITEM_ENTRY_SHADER);
        
        scene::TextSceneObjectData artifactNameText;
        artifactNameText.mText = artifactItemProduct.mStoryRareItemName;
        artifactNameText.mFontName = game_constants::DEFAULT_FONT_NAME;
        
        artifactNameTextSceneObject->mSceneObjectTypeData = std::move(artifactNameText);
        artifactNameTextSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MIN_Y_UNIFORM_NAME] = shaderCutoffValues.s;
        artifactNameTextSceneObject->mShaderFloatUniformValues[game_constants::CUTOFF_MAX_Y_UNIFORM_NAME] = shaderCutoffValues.t;
        artifactNameTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        artifactNameTextSceneObject->mScale = ARTIFACT_TEXT_SCALE;
        artifactNameTextSceneObject->mPosition += ARTIFACT_NAME_TEXT_OFFSET;
        
        ItemEntry itemEntry;
        itemEntry.mArtifactOrMutationName = artifactEntry.first;
        itemEntry.mSceneObjects.push_back(artifactSceneObject);
        itemEntry.mSceneObjects.push_back(artifactCountTextSceneObject);
        itemEntry.mSceneObjects.push_back(artifactNameTextSceneObject);
        
        mArtifactsItemContainer->AddItem(std::move(itemEntry), EntryAdditionStrategy::ADD_ON_THE_BACK);
    }
    
    // Toggle mutations title off if necessary
    mScene->FindSceneObject(MUTATIONS_TITLE_SCENE_OBJECT_NAME)->mInvisible = mutationCount == 0;
    
    // Append item count to containers
    std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(ARTIFACTS_TITLE_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = "Artifacts (" + std::to_string(artifactCount) + ")";
    std::get<scene::TextSceneObjectData>(mScene->FindSceneObject(MUTATIONS_TITLE_SCENE_OBJECT_NAME)->mSceneObjectTypeData).mText = "Mutations (" + std::to_string(mutationCount) + ")";
    
    // If container doesn't exist the staggered fade in will happen automatically at the end of VInitScene
    if (containerExists)
    {
        // Staggered Item Presentation
        size_t sceneObjectIndex = 0;
        for (const auto& containerItems: mArtifactsItemContainer->GetItems())
        {
            for (auto& sceneObject: containerItems.mSceneObjects)
            {
                sceneObject->mInvisible = false;
                sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, ITEMS_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=](){});
            }
        }
        
        for (const auto& containerItems: mMutationsItemContainer->GetItems())
        {
            for (auto& sceneObject: containerItems.mSceneObjects)
            {
                sceneObject->mInvisible = false;
                sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
                CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, ITEMS_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=](){});
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::CreateItemTooltip(const glm::vec3& itemOriginPostion, const std::string& tooltipText)
{
    bool shouldBeHorFlipped = itemOriginPostion.x > 0.0f;
    bool shouldBeVerFlipped = itemOriginPostion.y > 0.0f;
    
    mItemTooltipController = std::make_unique<CardTooltipController>
    (
        itemOriginPostion + ITEM_TOOLTIP_POSITION_OFFSET,
        ITEM_TOOLTIP_BASE_SCALE,
        tooltipText,
        false,
        shouldBeHorFlipped,
        shouldBeVerFlipped,
        *mScene
    );
}

///------------------------------------------------------------------------------------------------

void InventorySceneLogicManager::DestroyItemTooltip()
{
    if (mItemTooltipController)
    {
        for (auto sceneObject: mItemTooltipController->GetSceneObjects())
        {
            mScene->RemoveSceneObject(sceneObject->mName);
        }
    }
    
    mItemTooltipController = nullptr;
}

///------------------------------------------------------------------------------------------------

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <imgui/backends/imgui_impl_sdl2.h>
    #define CREATE_DEBUG_WIDGETS
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
        #undef CREATE_DEBUG_WIDGETS
    #else
        #include <imgui/backends/imgui_impl_sdl2.h>
        #define CREATE_DEBUG_WIDGETS
    #endif
#endif

#if ((!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)) && (defined(CREATE_DEBUG_WIDGETS))
void InventorySceneLogicManager::VCreateDebugWidgets()
{
//    bool interactedWithSlider = false;
//
//    ImGui::Begin("Swipeable Container", nullptr, GLOBAL_IMGUI_WINDOW_FLAGS);
//
//    ImGui::SeparatorText("Shader Cutoff Values");
//    interactedWithSlider |= ImGui::SliderFloat("Shader Cutoff Min", &MUTATION_ITEM_ENTRY_CUTOFF_VALUES.x, -1.0f, 1.0f);
//    interactedWithSlider |= ImGui::SliderFloat("Shader Cutoff Max", &MUTATION_ITEM_ENTRY_CUTOFF_VALUES.y, -1.0f, 1.0f);
//
//    ImGui::SeparatorText("Container Cutoff Values");
//    interactedWithSlider |= ImGui::SliderFloat("Container Cutoff Min", &MUTATION_ITEM_CONTAINER_CUTOFF_VALUES.x, -1.0f, 1.0f);
//    interactedWithSlider |= ImGui::SliderFloat("Container Cutoff Max", &MUTATION_ITEM_CONTAINER_CUTOFF_VALUES.y, -1.0f, 1.0f);
//
//    ImGui::SeparatorText("Container Hor Bounds");
//    interactedWithSlider |= ImGui::SliderFloat("Container Hor Min", &MUTATION_ITEM_CONTAINER_BOUNDS.bottomLeft.x, -1.0f, 1.0f);
//    interactedWithSlider |= ImGui::SliderFloat("Container Hor Max", &MUTATION_ITEM_CONTAINER_BOUNDS.topRight.x, -1.0f, 1.0f);
//
//    ImGui::SeparatorText("Container Ver Bounds");
//    interactedWithSlider |= ImGui::SliderFloat("Container Ver Min", &MUTATION_ITEM_CONTAINER_BOUNDS.bottomLeft.y, -1.0f, 1.0f);
//    interactedWithSlider |= ImGui::SliderFloat("Container Ver Max", &MUTATION_ITEM_CONTAINER_BOUNDS.topRight.y, -1.0f, 1.0f);
//
//    ImGui::End();
//
//    if (interactedWithSlider)
//    {
//        CreateItemEntriesAndContainer();
//    }
}
#else
void InventorySceneLogicManager::VCreateDebugWidgets()
{
}
#endif

///------------------------------------------------------------------------------------------------
