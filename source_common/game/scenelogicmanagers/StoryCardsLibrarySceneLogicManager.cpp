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
#include <game/CardUtils.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/StoryCardsLibrarySceneLogicManager.h>

///------------------------------------------------------------------------------------------------

static const std::string CARD_ENTRY_SHADER = "card_library_entry.vs";
static const std::string CARD_TOOLTIP_TEXTURE_FILE_NAME = "tooltip.png";
static const std::string CARD_TOOLTIP_SHADER_FILE_NAME = "diagonal_reveal.vs";

static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");
static const strutils::StringId CARD_CONTAINER_SCENE_OBJECT_NAME = strutils::StringId("card_container");
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

static const glm::vec3 BUTTON_SCALE = {0.0004f, 0.0004f, 0.0004f};
static const glm::vec3 BACK_BUTTON_POSITION = {0.0f, -0.1f, 23.2f};
static const glm::vec3 CARD_ENTRY_SCALE = glm::vec3(-0.273f, 0.2512f, 2.0f);
static const glm::vec3 CONTAINER_ITEM_ENTRY_SCALE = glm::vec3(0.124f, 0.212f, 2.0f);
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

static const glm::vec2 CARD_ENTRY_CUTOFF_VALUES = {-0.208f, 0.158f};
static const glm::vec2 CARD_CONTAINER_CUTOFF_VALUES = {-0.15f, 0.15f};

static const math::Rectangle CARD_CONTAINER_BOUNDS = {{-0.305f, -0.22f}, {0.305f, 0.15f}};

static const float ITEMS_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;
static const float BACK_BUTTON_SNAP_TO_EDGE_FACTOR = 950000.0f;
static const float CARD_ENTRY_Z = 23.2f;
static const float CARD_TOOLTIP_MAX_REVEAL_THRESHOLD = 2.0f;
static const float CARD_TOOLTIP_REVEAL_SPEED = 1.0f/200.0f;
static const float CARD_TOOLTIP_TEXT_REVEAL_SPEED = 1.0f/500.0f;
static const float CARD_TOOLTIP_FLIPPED_X_OFFSET = -0.17f;
static const float CARD_TOOLTIP_FLIPPED_Y_OFFSET = -0.25f;
static const float CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET = -0.007f;
static const float CARD_TOOLTIP_TEXT_FLIPPED_Y_OFFSET = -0.014f;

static constexpr int MIN_CONTAINER_ENTRIES_TO_ANIMATE = 5;

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
            sceneObject->mName == CARD_TOOLTIP_SCENE_OBJECT_NAME ||
            sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[0] ||
            sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[1] ||
            sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[2] ||
            sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[3]
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
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioning)
    {
        return;
    }
    
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
                
                auto cardData = CardDataRepository::GetInstance().GetCardData(interactedElementEntry.mCardSoWrapper->mCardData.mCardId, game_constants::LOCAL_PLAYER_INDEX);
                
                DestroyCardTooltip();
                
                if (cardData.IsSpell())
                {
                    sToolTipPointeePosY = interactedElementEntry.mSceneObjects.front()->mPosition.y;
                    
                    CreateCardTooltip(interactedElementEntry.mSceneObjects.front()->mPosition + CARD_TOOLTIP_POSITION_OFFSET, cardData.mCardEffectTooltip);
                }
            }
        }
        
        // Card tooltip
        if (sToolTipIndex != -1)
        {
            auto interactedElementEntry = mCardContainer->GetItems()[sToolTipIndex];
            if (math::Abs(interactedElementEntry.mSceneObjects.front()->mPosition.y - sToolTipPointeePosY) > 0.01f)
            {
                sToolTipIndex = -1;
                DestroyCardTooltip();
            }
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
    }
    
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void StoryCardsLibrarySceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, ITEMS_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            if
            (
                sceneObject->mName == CARD_TOOLTIP_SCENE_OBJECT_NAME ||
                sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[0] ||
                sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[1] ||
                sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[2] ||
                sceneObject->mName == CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[3]
            )
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

void StoryCardsLibrarySceneLogicManager::DestroyCardTooltip()
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
