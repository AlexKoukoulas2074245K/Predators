///------------------------------------------------------------------------------------------------
///  VisitMapNodeSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/ProgressionDataRepository.h>
#include <game/scenelogicmanagers/VisitMapNodeSceneLogicManager.h>
#include <SDL_events.h>

///------------------------------------------------------------------------------------------------

static const std::string CUSTOM_COLOR_SHADER_FILE_NAME = "basic_custom_color.vs";

static const strutils::StringId VISIT_MAP_NODE_SCENE_NAME = strutils::StringId("visit_map_node_scene");
static const strutils::StringId NODE_DESCRIPTION_TEXT_SCENE_OBJECT_NAME = strutils::StringId("node_description_text");
static const strutils::StringId VISIT_BUTTON_NAME = strutils::StringId("visit_button");
static const strutils::StringId BACK_BUTTON_NAME = strutils::StringId("back_button");

static const glm::vec3 BUTTON_SCALE = {0.0005f, 0.0005f, 0.0005f};
static const glm::vec3 WHITE_NODE_DESC_COLOR = {0.96f, 0.96f, 0.96f};
static const glm::vec3 RED_NODE_DESC_COLOR = {0.86f, 0.1f, 0.1f};
static const glm::vec3 PURPLE_NODE_DESC_COLOR = {0.66f, 0.35f, 1.0f};
static const glm::vec3 ORANGE_NODE_DESC_COLOR = {0.96f, 0.47f, 0.25f};

static const glm::vec2 NODE_DESC_MIN_MAX_X_OFFSETS = {-0.1f, -0.23f};
static const glm::vec2 NODE_DESC_MIN_MAX_Y_OFFSETS = {0.14f, -0.11f};

static const float VISIT_BUTTON_HOR_DISTANCE_FROM_NODE = 0.1f;
static const float VISIT_BUTTON_Y_OFFSET_FROM_NODE = 0.05f;

static const float BACK_BUTTON_HOR_DISTANCE_FROM_NODE = 0.1f;
static const float BACK_BUTTON_Y_OFFSET_FROM_NODE = -0.03f;
static const float BUTTON_Z = 24.0f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;
static const float FADE_IN_OUT_DURATION_SECS = 0.25f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    VISIT_MAP_NODE_SCENE_NAME
};

static const std::unordered_set<strutils::StringId, strutils::StringIdHasher> STATIC_SCENE_ELEMENTS =
{
    game_constants::OVERLAY_SCENE_OBJECT_NAME
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& VisitMapNodeSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

VisitMapNodeSceneLogicManager::VisitMapNodeSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

VisitMapNodeSceneLogicManager::~VisitMapNodeSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene> scene)
{
    scene->GetCamera().SetPosition(CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mPreviousScene)->GetCamera().GetPosition());
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mTransitioning = false;
    
    scene->RemoveAllSceneObjectsButTheOnesNamed(STATIC_SCENE_ELEMENTS);
    
    mAnimatedButtons.clear();
    
    auto& targetNodePosition = ProgressionDataRepository::GetInstance().GetSelectedStoryMapNodePosition();
    auto& previousSceneCameraPosition = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(mPreviousScene)->GetCamera().GetPosition();
    
    // Don't visit Tent node
    if (ProgressionDataRepository::GetInstance().GetSelectedStoryMapNodeData()->mCoords != ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord())
    {
        auto visitButtonPosition = targetNodePosition;
        visitButtonPosition.x += (targetNodePosition.x < previousSceneCameraPosition.x ? VISIT_BUTTON_HOR_DISTANCE_FROM_NODE : -1.5f * VISIT_BUTTON_HOR_DISTANCE_FROM_NODE);
        visitButtonPosition.y += VISIT_BUTTON_Y_OFFSET_FROM_NODE;
        visitButtonPosition.z = BUTTON_Z;
        mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
        (
            visitButtonPosition,
            BUTTON_SCALE,
            game_constants::DEFAULT_FONT_NAME,
            "Visit",
            VISIT_BUTTON_NAME,
            [=]()
            {
                mTransitioning = true;
                InitializeNodeVisitData();
            },
            *scene
        ));
    }
    
    
    auto backButtonPosition = targetNodePosition;
    backButtonPosition.x += (targetNodePosition.x < previousSceneCameraPosition.x ? BACK_BUTTON_HOR_DISTANCE_FROM_NODE : -1.5f * BACK_BUTTON_HOR_DISTANCE_FROM_NODE);
    backButtonPosition.y += BACK_BUTTON_Y_OFFSET_FROM_NODE;
    backButtonPosition.z = BUTTON_Z;
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        backButtonPosition,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Back",
        BACK_BUTTON_NAME,
        [=]()
        {
            mTransitioning = true;
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
        },
        *scene
    ));
    
    auto nodeDescriptionSceneObject = scene->CreateSceneObject(NODE_DESCRIPTION_TEXT_SCENE_OBJECT_NAME);
    nodeDescriptionSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CUSTOM_COLOR_SHADER_FILE_NAME);
    
    scene::TextSceneObjectData textDataNodeDescription;
    textDataNodeDescription.mFontName = game_constants::DEFAULT_FONT_NAME;
    
    const auto effectiveNodeType = ProgressionDataRepository::GetInstance().GetSelectedStoryMapNodeData()->mCoords == ProgressionDataRepository::GetInstance().GetCurrentStoryMapNodeCoord() ? StoryMap::NodeType::STARTING_LOCATION : ProgressionDataRepository::GetInstance().GetSelectedStoryMapNodeData()->mNodeType;
    switch(effectiveNodeType)
    {
        case StoryMap::NodeType::NORMAL_ENCOUNTER:
        {
            nodeDescriptionSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = WHITE_NODE_DESC_COLOR;
            textDataNodeDescription.mText = "Normal Encounter";
        } break;
        
        case StoryMap::NodeType::ELITE_ENCOUNTER:
        {
            textDataNodeDescription.mText = "Elite Encounter";
            nodeDescriptionSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = ORANGE_NODE_DESC_COLOR;
        } break;
        
        case StoryMap::NodeType::EVENT:
        {
            textDataNodeDescription.mText = "Random Event";
            nodeDescriptionSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = PURPLE_NODE_DESC_COLOR;
        } break;
        
        case StoryMap::NodeType::BOSS_ENCOUNTER:
        {
            textDataNodeDescription.mText = "Boss Encounter";
            nodeDescriptionSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = RED_NODE_DESC_COLOR;
        } break;
        
        case StoryMap::NodeType::SHOP:
        {
            textDataNodeDescription.mText = "Merchant Encounter";
            nodeDescriptionSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = PURPLE_NODE_DESC_COLOR;
        } break;
        
        case StoryMap::NodeType::STARTING_LOCATION:
        {
            textDataNodeDescription.mText = "Your Tent!";
            nodeDescriptionSceneObject->mShaderVec3UniformValues[game_constants::CUSTOM_COLOR_UNIFORM_NAME] = WHITE_NODE_DESC_COLOR;
        }break;
        default: break;
    }
    
    nodeDescriptionSceneObject->mSceneObjectTypeData = std::move(textDataNodeDescription);
    nodeDescriptionSceneObject->mPosition = targetNodePosition;
    nodeDescriptionSceneObject->mPosition.x += (targetNodePosition.x < previousSceneCameraPosition.x ? NODE_DESC_MIN_MAX_X_OFFSETS.s : NODE_DESC_MIN_MAX_X_OFFSETS.t);
    nodeDescriptionSceneObject->mPosition.y += (targetNodePosition.y < previousSceneCameraPosition.y ? NODE_DESC_MIN_MAX_Y_OFFSETS.s : NODE_DESC_MIN_MAX_Y_OFFSETS.t);
    nodeDescriptionSceneObject->mPosition.z = BUTTON_Z;
    nodeDescriptionSceneObject->mScale = BUTTON_SCALE;
    
    size_t sceneObjectIndex = 0;
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
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=]()
        {
        });
    }
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioning)
    {
        return;
    }
    
    // Animated buttons
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, FADE_IN_OUT_DURATION_SECS), [=]()
        {
            sceneObject->mInvisible = true;
        });
    }
    
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto previousScene = sceneManager.FindScene(mPreviousScene);
    
    animationManager.StopAnimation(game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
    animationManager.StartAnimation(std::make_unique<rendering::TweenValueAnimation>(previousScene->GetUpdateTimeSpeedFactor(), 1.0f, game_constants::SCENE_SPEED_DILATION_ANIMATION_DURATION_SECS), [](){}, game_constants::SCENE_SPEED_DILATION_ANIMATION_NAME);
}

///------------------------------------------------------------------------------------------------

void VisitMapNodeSceneLogicManager::InitializeNodeVisitData()
{
    auto* selectedNodeData = ProgressionDataRepository::GetInstance().GetSelectedStoryMapNodeData();
    
    assert(selectedNodeData);
    assert(selectedNodeData->mNodeRandomSeed != 0);
    
    ProgressionDataRepository::GetInstance().SetCurrentStoryMapNodeSeed(selectedNodeData->mNodeRandomSeed);
    ProgressionDataRepository::GetInstance().SetCurrentStoryMapNodeCoord(selectedNodeData->mCoords);
    
    std::vector<int> opponentDeckBuilder;
    
    switch (selectedNodeData->mNodeType)
    {
        case StoryMap::NodeType::EVENT:
        {
            ProgressionDataRepository::GetInstance().SetCurrentEventScreenIndex(0);
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::EVENT_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
        } break;
        
        case StoryMap::NodeType::BOSS_ENCOUNTER:
        {
            auto eliteCards = CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::DEMONS_HARD_FAMILY_NAME);
            opponentDeckBuilder.insert(opponentDeckBuilder.end(), eliteCards.begin(), eliteCards.end());
        } // Intentional fallthrough
        case StoryMap::NodeType::ELITE_ENCOUNTER:
        {
            if (selectedNodeData->mCoords.x >= game_constants::STORY_NODE_MAP_DIMENSIONS.x/2)
            {
                auto mediumCards = CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::DEMONS_MEDIUM_FAMILY_NAME);
                opponentDeckBuilder.insert(opponentDeckBuilder.end(), mediumCards.begin(), mediumCards.end());
            }
        } // Intentional fallthrough
        case StoryMap::NodeType::NORMAL_ENCOUNTER:
        {
            auto normalCards = CardDataRepository::GetInstance().GetCardIdsByFamily(game_constants::DEMONS_NORMAL_FAMILY_NAME);
            opponentDeckBuilder.insert(opponentDeckBuilder.end(), normalCards.begin(), normalCards.end());
            
            // Populate opponent deck and battle control type
            ProgressionDataRepository::GetInstance().SetNextTopPlayerDeck(opponentDeckBuilder);
            ProgressionDataRepository::GetInstance().SetNextBattleControlType(BattleControlType::AI_TOP_ONLY);
            
            // Populate opponent hero card name & texture
            auto storyMapScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::STORY_MAP_SCENE);
            auto nodePortraitSceneObject = storyMapScene->FindSceneObject(strutils::StringId(MapCoord(selectedNodeData->mCoords.x, selectedNodeData->mCoords.y).ToString() + game_constants::STORY_MAP_NODE_PORTRAIT_SO_NAME_POST_FIX));
            auto nodeHealthTextSceneObject = storyMapScene->FindSceneObject(strutils::StringId(MapCoord(selectedNodeData->mCoords.x, selectedNodeData->mCoords.y).ToString() + game_constants::STORY_MAP_NODE_HEALTH_TEXT_SO_NAME_POST_FIX));
            auto nodeDamageTextSceneObject = storyMapScene->FindSceneObject(strutils::StringId(MapCoord(selectedNodeData->mCoords.x, selectedNodeData->mCoords.y).ToString() + game_constants::STORY_MAP_NODE_DAMAGE_TEXT_SO_NAME_POST_FIX));
            auto nodeWeightTextSceneObject = storyMapScene->FindSceneObject(strutils::StringId(MapCoord(selectedNodeData->mCoords.x, selectedNodeData->mCoords.y).ToString() + game_constants::STORY_MAP_NODE_WEIGHT_TEXT_SO_NAME_POST_FIX));
            auto nodeNameTextSceneObject = storyMapScene->FindSceneObject(strutils::StringId(MapCoord(selectedNodeData->mCoords.x, selectedNodeData->mCoords.y).ToString() + game_constants::STORY_MAP_NODE_TEXT_SO_NAME_POST_FIX));
            
            ProgressionDataRepository::GetInstance().SetNextStoryOpponentTexturePath(CoreSystemsEngine::GetInstance().GetResourceLoadingService().GetResourcePath(nodePortraitSceneObject->mTextureResourceId));
            ProgressionDataRepository::GetInstance().SetNextStoryOpponentName(std::get<scene::TextSceneObjectData>(nodeNameTextSceneObject->mSceneObjectTypeData).mText);
            
            // Populate opponent stats
            ProgressionDataRepository::GetInstance().SetNextStoryOpponentDamage(std::stoi(std::get<scene::TextSceneObjectData>(nodeDamageTextSceneObject->mSceneObjectTypeData).mText));
            ProgressionDataRepository::GetInstance().SetNextBattleTopPlayerHealth(std::stoi(std::get<scene::TextSceneObjectData>(nodeHealthTextSceneObject->mSceneObjectTypeData).mText));
            ProgressionDataRepository::GetInstance().SetNextBattleTopPlayerInitWeight(std::stoi(std::get<scene::TextSceneObjectData>(nodeWeightTextSceneObject->mSceneObjectTypeData).mText) - 1);
            ProgressionDataRepository::GetInstance().SetNextBattleTopPlayerWeightLimit(std::stoi(std::get<scene::TextSceneObjectData>(nodeWeightTextSceneObject->mSceneObjectTypeData).mText));
            
            // Populate local player stats
            ProgressionDataRepository::GetInstance().SetNextBotPlayerDeck(ProgressionDataRepository::GetInstance().GetCurrentStoryPlayerDeck());
            ProgressionDataRepository::GetInstance().SetNextBattleBotPlayerHealth(ProgressionDataRepository::GetInstance().StoryCurrentHealth().GetValue());
            ProgressionDataRepository::GetInstance().SetNextBattleBotPlayerInitWeight(game_constants::BOT_PLAYER_DEFAULT_WEIGHT - 1);
            ProgressionDataRepository::GetInstance().SetNextBattleBotPlayerWeightLimit(game_constants::BOT_PLAYER_DEFAULT_WEIGHT_LIMIT);
            
            events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(game_constants::BATTLE_SCENE, SceneChangeType::CONCRETE_SCENE_ASYNC_LOADING, PreviousSceneDestructionType::DESTROY_PREVIOUS_SCENE);
        } break;
            
        default:
        {
            assert(false);
        } break;
    }
    
    ProgressionDataRepository::GetInstance().FlushStateToFile();
}

///------------------------------------------------------------------------------------------------
