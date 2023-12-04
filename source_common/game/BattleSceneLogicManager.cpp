///------------------------------------------------------------------------------------------------
///  BattleSceneLogicManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/AnimatedButton.h>
#include <game/AnimatedStatContainer.h>
#include <game/BoardState.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/GameReplayEngine.h>
#include <game/GameRuleEngine.h>
#include <game/GameSerializer.h>
#include <game/BattleSceneLogicManager.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayerActionGenerationEngine.h>
#include <game/utils/PersistenceUtils.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId HISTORY_SCENE = strutils::StringId("battle_history_scene");
static const strutils::StringId HISTORY_TROLLEY_SCENE_OBJECT_NAME = strutils::StringId("HISTORY_TROLLEY");
static const strutils::StringId CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME = strutils::StringId("CARD_LOCATION_INDICATOR");
static const strutils::StringId CARD_TOOLTIP_SCENE_OBJECT_NAME = strutils::StringId("CARD_TOOLTIP");
static const strutils::StringId HISTORY_BUTTON_SCENE_OBJECT_NAME = strutils::StringId("HISTORY_ICON");
static const strutils::StringId HISTORY_OVERLAY_SCENE_OBJECT_NAME = strutils::StringId("HISTORY_OVERLAY");
static const strutils::StringId BATTLE_SCENE_SPEED_DILATION_ANIMATION_NAME = strutils::StringId("SCENE_SPEED_DILATION_ANIMATION");
static const strutils::StringId CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME = strutils::StringId("reveal_threshold");
static const strutils::StringId CARD_TOOLTIP_REVEAL_RGB_EXPONENT_UNIFORM_NAME = strutils::StringId("reveal_rgb_exponent");
static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId PLAY_CARD_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_ACTION_NAME = strutils::StringId("NextPlayerGameAction");
static const strutils::StringId CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME = strutils::StringId("CardBuffedDebuffedAnimationGameAction");
static const strutils::StringId CARD_EFFECT_GAME_ACTION_NAME = strutils::StringId("CardEffectGameAction");
static const strutils::StringId CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES [game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT] =
{
    strutils::StringId("CARD_TOOLTIP_TEXT_0"),
    strutils::StringId("CARD_TOOLTIP_TEXT_1"),
    strutils::StringId("CARD_TOOLTIP_TEXT_2"),
    strutils::StringId("CARD_TOOLTIP_TEXT_3")
};
static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::IN_GAME_BATTLE_SCENE,
    HISTORY_SCENE
};

static const std::string MAKE_SPACE_REVERT_TO_POSITION_ANIMATION_NAME_PREFIX = "MAKE_SPACE_REVERT_";
static const std::string BATTLE_ICON_TEXTURE_FILE_NAME = "battle_icon.png";
static const std::string TURN_POINTER_TEXTURE_FILE_NAME = "turn_pointer.png";
static const std::string HEALTH_CRYSTAL_TEXTURE_FILE_NAME = "health_crystal.png";
static const std::string WEIGHT_CRYSTAL_TEXTURE_FILE_NAME = "weight_crystal.png";
static const std::string POISON_STACK_TEXTURE_FILE_NAME = "poison_splatter.png";
static const std::string BOARD_SIDE_EFFECT_REDUCTION_TEXTURE_FILE_NAME = "board_side_reduction.png";
static const std::string BOARD_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME = "board_side_mask.png";
static const std::string KILL_SIDE_EFFECT_TEXTURE_FILE_NAME = "trap.png";
static const std::string INSECT_DUPLICATION_EFFECT_TEXTURE_FILE_NAME = "insect_duplication.png";
static const std::string NEXT_DINO_DAMAGE_DOUBLING_EFFECT_TEXTURE_FILE_NAME = "mighty_roar.png";
static const std::string DOUBLE_POISON_ATTACKS_EFFECT_TEXTURE_FILE_NAME = "poison_smoke.png";
static const std::string PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_TEXTURE_FILE_NAME = "impending_doom.png";
static const std::string INDIVIDUAL_CARD_BOARD_EFFECT_MASK_TEXTURE_FILE_NAME = "trap_mask.png";
static const std::string BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME = "board_side_stat_effect.vs";
static const std::string CARD_TOOLTIP_TEXTURE_FILE_NAME = "tooltip.png";
static const std::string CARD_TOOLTIP_SHADER_FILE_NAME = "diagonal_reveal.vs";
static const std::string HISTORY_ICON_TEXTURE_FILE_NAME = "history_icon.png";
static const std::string HISTORY_OVERLAY_TEXTURE_FILE_NAME = "overlay.png";
static const std::string CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX = "HIGHLIGHTER_CARD_";
static const std::string HEALTH_CRYSTAL_TOP_SCENE_OBJECT_NAME_PREFIX = "HEALTH_CRYSTAL_TOP_";
static const std::string HEALTH_CRYSTAL_BOT_SCENE_OBJECT_NAME_PREFIX = "HEALTH_CRYSTAL_BOT_";
static const std::string WEIGHT_CRYSTAL_TOP_SCENE_OBJECT_NAME_PREFIX = "WEIGHT_CRYSTAL_TOP_";
static const std::string WEIGHT_CRYSTAL_BOT_SCENE_OBJECT_NAME_PREFIX = "WEIGHT_CRYSTAL_BOT_";
static const std::string POISON_STACK_TOP_SCENE_OBJECT_NAME_PREFIX = "POISON_STACK_TOP_";
static const std::string POISON_STACK_BOT_SCENE_OBJECT_NAME_PREFIX = "POISON_STACK_BOT_";

static const glm::vec3 TURN_POINTER_POSITION = {0.2f, 0.0f, 0.1f};
static const glm::vec3 TURN_POINTER_SCALE = {0.08f, 0.08f, 0.08f};
static const glm::vec3 BOARD_SIDE_EFFECT_SCALE = {0.372f, 0.346f, 1.0f};
static const glm::vec3 BOARD_SIDE_EFFECT_TOP_POSITION = { 0.0f, 0.044f, 1.0f};
static const glm::vec3 BOARD_SIDE_EFFECT_BOT_POSITION = { 0.0f, -0.044f, 1.0f};
static const glm::vec3 CARD_TOOLTIP_SCALE = {0.137f, 0.137f, 1/10.0f};
static const glm::vec3 CARD_TOOLTIP_OFFSET = {0.084f, 0.08f, 0.1f};
static const glm::vec3 HISTORY_BUTTON_POSITION = {-0.155f, 0.05f, 10.0f};
static const glm::vec3 HISTORY_BUTTON_SCALE = {0.03f, 0.03f, 0.03f};

static const glm::vec3 CARD_TOOLTIP_TEXT_OFFSETS[game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT] =
{
    { -0.033f, 0.029f, 0.1f },
    { -0.051f, 0.014f, 0.1f },
    { -0.036f, -0.000f, 0.1f },
    { -0.03f, -0.014f, 0.1f }
};

static const float BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS = 0.5f;
static const float CARD_SELECTION_ANIMATION_DURATION = 0.15f;
static const float CARD_LOCATION_EFFECT_MIN_TARGET_ALPHA = 0.25f;
static const float CARD_LOCATION_EFFECT_MAX_TARGET_ALPHA = 1.0f;
static const float CARD_LOCATION_EFFECT_ALPHA_SPEED = 0.003f;
static const float CARD_TOOLTIP_TEXT_FONT_SIZE = 0.00016f;
static const float CARD_TOOLTIP_MAX_REVEAL_THRESHOLD = 2.0f;
static const float CARD_TOOLTIP_REVEAL_RGB_EXPONENT = 1.127f;
static const float CARD_TOOLTIP_REVEAL_SPEED = 1.0f/200.0f;
static const float CARD_TOOLTIP_TEXT_REVEAL_SPEED = 1.0f/500.0f;
static const float CARD_TOOLTIP_FLIPPED_X_OFFSET = -0.17f;
static const float CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET = -0.007f;
static const float CARD_TOOLTIP_CREATION_DELAY_SECS = 0.5f;
static const float BOARD_SIDE_EFFECT_VALUE_LEFT_X = -0.075f;
static const float BOARD_SIDE_EFFECT_VALUE_RIGHT_X = 0.045f;
static const float BOARD_SIDE_EFFECT_VALUE_Z_OFFSET = 0.01f;
static const float BOARD_SIDE_EFFECT_VALUE_SCALE = 0.0003f;
static const float INDIVIDUAL_CARD_BOARD_EFFECT_BASE_Z = 1.1f;
static const float INDIVIDUAL_CARD_BOARD_EFFECT_Z_INCREMENT = 0.01f;
static const float BOARD_EFFECT_MAX_ALPHA = 0.25f;
static const float TURN_POINTER_INTERACTOR_SCALE_FACTOR = 0.5f;
static const float TURN_POINTER_INTERACTION_PULSE_DURATION = 0.1f;
static const float OVERLAY_SCENE_SPEED_ANIMATION_TARGET_DURATION = 0.5f;
static const float HISTORY_MODAL_MAX_ALPHA = 0.75f;

#if defined(MOBILE_FLOW)
static const float MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR = 0.003f;
#else
static const float DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR = 0.003f;
#endif

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& BattleSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

BattleSceneLogicManager::BattleSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

BattleSceneLogicManager::~BattleSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    if (scene->GetName() == game_constants::IN_GAME_BATTLE_SCENE)
    {
        InitBattleScene();
    }
    else if (scene->GetName() == HISTORY_SCENE)
    {
        InitHistoryScene();
    }
}

void BattleSceneLogicManager::InitBattleScene()
{
    RegisterForEvents();
    
    mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::NONE;
    mSecsCardHighlighted = 0.0f;
    mShouldShowCardLocationIndicator = false;
    mCanPlayNextCard = false;
    mCanIssueNextTurnInteraction = false;
    mCanInteractWithAnyHeldCard = true;
    
    mBoardState = std::make_unique<BoardState>();
    mBoardState->GetPlayerStates().emplace_back();
    mBoardState->GetPlayerStates().emplace_back();
    
    mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mPlayerHealth = game_constants::TOP_PLAYER_DEFAULT_HEALTH;
    mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mPlayerHealth = game_constants::BOT_PLAYER_DEFAULT_HEALTH;
    
    mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mPlayerDeckCards = CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("rodents"));
    mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mPlayerDeckCards = CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("dinosaurs"));
    
    //mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mGoldenCardIds = CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("rodents"));
    //mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mGoldenCardIds = CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("rodents"));;
    
    mActiveIndividualCardBoardEffectSceneObjects.emplace_back();
    mActiveIndividualCardBoardEffectSceneObjects.emplace_back();
    
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    
    mRuleEngine = std::make_unique<GameRuleEngine>(mBoardState.get());

#define AUTO_PLAY
//#define REPLAY_FLOW
    
#if defined(REPLAY_FLOW)
    GameReplayEngine replayEngine(persistence_utils::GetProgressDirectoryPath() + "game");
    auto seed = replayEngine.GetGameFileSeed();
#else
    //auto seed = 51329216;
    auto seed = math::RandomInt();
#endif
    
    mGameSerializer = std::make_unique<GameSerializer>(seed);
    mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::ANIMATED, seed, mBoardState.get(), this, mRuleEngine.get(), mGameSerializer.get());
    mPlayerActionGenerationEngine = std::make_unique<PlayerActionGenerationEngine>(mRuleEngine.get(), mActionEngine.get(), PlayerActionGenerationEngine::ActionGenerationType::OPTIMISED);
    
#if defined(REPLAY_FLOW)
    replayEngine.ReplayActions(mActionEngine.get());
#else
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
#endif
    
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Card Location Indicator
    auto cardLocationIndicatorSo = battleScene->CreateSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    cardLocationIndicatorSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CARD_LOCATION_MASK_TEXTURE_NAME);
    cardLocationIndicatorSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BOARD_CARD_LOCATION_SHADER_NAME);
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_TIME_SPEED;
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_PERLIN_RESOLUTION;
    cardLocationIndicatorSo->mScale = glm::vec3(game_constants::IN_GAME_CARD_BASE_SCALE * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR);
    cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    cardLocationIndicatorSo->mInvisible = true;
    
    // Turn pointer
    auto turnPointerSo = battleScene->CreateSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    turnPointerSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + TURN_POINTER_TEXTURE_FILE_NAME);
    turnPointerSo->mPosition = TURN_POINTER_POSITION;
    turnPointerSo->mScale = TURN_POINTER_SCALE;
    turnPointerSo->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    
    // Turn pointer highlighter
    auto tunPointerHighlighterSo = battleScene->CreateSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
    tunPointerHighlighterSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::ACTION_HIGHLIGHTER_SHADER_NAME);
    tunPointerHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_TIME_SPEED;
    tunPointerHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_RESOLUTION;
    tunPointerHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_CLARITY_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_CLARITY;
    tunPointerHighlighterSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    tunPointerHighlighterSo->mPosition = turnPointerSo->mPosition;
    tunPointerHighlighterSo->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
    tunPointerHighlighterSo->mScale = game_constants::TURN_POINTER_HIGHLIGHTER_SCALE;
    tunPointerHighlighterSo->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    
    // Stat Containers
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::HEALTH_CRYSTAL_TOP_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_TOP_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[0].mPlayerHealth, false, *battleScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::HEALTH_CRYSTAL_BOT_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_BOT_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[1].mPlayerHealth, false, *battleScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::WEIGHT_CRYSTAL_TOP_POSITION, WEIGHT_CRYSTAL_TEXTURE_FILE_NAME, WEIGHT_CRYSTAL_TOP_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo, false, *battleScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::WEIGHT_CRYSTAL_BOT_POSITION, WEIGHT_CRYSTAL_TEXTURE_FILE_NAME, WEIGHT_CRYSTAL_BOT_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo, false, *battleScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::POISON_STACK_TOP_POSITION, POISON_STACK_TEXTURE_FILE_NAME, POISON_STACK_TOP_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[0].mPlayerPoisonStack, true, *battleScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::POISON_STACK_BOT_POSITION, POISON_STACK_TEXTURE_FILE_NAME, POISON_STACK_BOT_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[1].mPlayerPoisonStack, true, *battleScene)));
    
    // Board Side Effect Top
    auto boardSideEffectTopSceneObject = battleScene->CreateSceneObject(game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME);
    boardSideEffectTopSceneObject->mScale = BOARD_SIDE_EFFECT_SCALE;
    boardSideEffectTopSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOARD_SIDE_EFFECT_REDUCTION_TEXTURE_FILE_NAME);
    boardSideEffectTopSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOARD_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME);
    boardSideEffectTopSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
    boardSideEffectTopSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    boardSideEffectTopSceneObject->mPosition = BOARD_SIDE_EFFECT_TOP_POSITION;
    boardSideEffectTopSceneObject->mInvisible = true;
    
    // Board Side Effect Bot
    auto boardSideEffectBotSceneObject = battleScene->CreateSceneObject(game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    boardSideEffectBotSceneObject->mScale = BOARD_SIDE_EFFECT_SCALE;
    boardSideEffectBotSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOARD_SIDE_EFFECT_REDUCTION_TEXTURE_FILE_NAME);
    boardSideEffectBotSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOARD_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME);
    boardSideEffectBotSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
    boardSideEffectBotSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    boardSideEffectBotSceneObject->mPosition = BOARD_SIDE_EFFECT_BOT_POSITION;
    boardSideEffectBotSceneObject->mInvisible = true;
    
    for (int i = 0; i < game_constants::BOARD_SIDE_EFFECT_VALUE_SO_COUNT; ++i)
    {
        {
            auto boardSideEffectTopValueSceneObject = battleScene->CreateSceneObject(strutils::StringId(game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX + std::to_string(i)));
            
            scene::TextSceneObjectData effectValueTextData;
            effectValueTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
            effectValueTextData.mText = std::to_string(0);
            
            boardSideEffectTopValueSceneObject->mSceneObjectTypeData = std::move(effectValueTextData);
            boardSideEffectTopValueSceneObject->mScale = glm::vec3(BOARD_SIDE_EFFECT_VALUE_SCALE);
            boardSideEffectTopValueSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            boardSideEffectTopValueSceneObject->mPosition = boardSideEffectTopSceneObject->mPosition;
            boardSideEffectTopValueSceneObject->mPosition.x = i == 0 ? BOARD_SIDE_EFFECT_VALUE_LEFT_X : BOARD_SIDE_EFFECT_VALUE_RIGHT_X;
            boardSideEffectTopValueSceneObject->mPosition.z += BOARD_SIDE_EFFECT_VALUE_Z_OFFSET;
            boardSideEffectTopValueSceneObject->mInvisible = true;
        }
        
        {
            auto boardSideEffectBotValueSceneObject = battleScene->CreateSceneObject(strutils::StringId(game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX + std::to_string(i)));
            
            scene::TextSceneObjectData effectValueTextData;
            effectValueTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
            effectValueTextData.mText = std::to_string(0);
            
            boardSideEffectBotValueSceneObject->mSceneObjectTypeData = std::move(effectValueTextData);
            boardSideEffectBotValueSceneObject->mScale = glm::vec3(BOARD_SIDE_EFFECT_VALUE_SCALE);
            boardSideEffectBotValueSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            boardSideEffectBotValueSceneObject->mPosition = boardSideEffectBotSceneObject->mPosition;
            boardSideEffectBotValueSceneObject->mPosition.x = i == 0 ? BOARD_SIDE_EFFECT_VALUE_LEFT_X : BOARD_SIDE_EFFECT_VALUE_RIGHT_X;
            boardSideEffectBotValueSceneObject->mPosition.z += BOARD_SIDE_EFFECT_VALUE_Z_OFFSET;
            boardSideEffectBotValueSceneObject->mInvisible = true;
        }
    }
    
    auto individualCardBoardEffectCreation = [=]
    (
        const strutils::StringId& topSceneObjectName,
        const strutils::StringId& botSceneObjectName,
        const std::string& textureFilename
    )
    {
        auto effectTopSceneObject = battleScene->CreateSceneObject(topSceneObjectName);
        effectTopSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + textureFilename);
        effectTopSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        effectTopSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + INDIVIDUAL_CARD_BOARD_EFFECT_MASK_TEXTURE_FILE_NAME);
        effectTopSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
        effectTopSceneObject->mPosition = BOARD_SIDE_EFFECT_TOP_POSITION;
        effectTopSceneObject->mScale = game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_SCALE;
        effectTopSceneObject->mInvisible = true;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::PulseAnimation>(effectTopSceneObject, game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_SCALE_UP_FACTOR, game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS, animation_flags::ANIMATE_CONTINUOUSLY), [](){});
        
        auto effectBotSceneObject = battleScene->CreateSceneObject(botSceneObjectName);
        effectBotSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + textureFilename);
        effectBotSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        effectBotSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + INDIVIDUAL_CARD_BOARD_EFFECT_MASK_TEXTURE_FILE_NAME);
        effectBotSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
        effectBotSceneObject->mPosition = BOARD_SIDE_EFFECT_BOT_POSITION;
        effectBotSceneObject->mScale = game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_SCALE;
        effectBotSceneObject->mInvisible = true;
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::PulseAnimation>(effectBotSceneObject, game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_SCALE_UP_FACTOR, game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS, animation_flags::ANIMATE_CONTINUOUSLY), [](){});
    };
    
    // Kill Side Effects
    individualCardBoardEffectCreation(game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME, game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME, KILL_SIDE_EFFECT_TEXTURE_FILE_NAME);
    
    // Insect Duplication Effects
    individualCardBoardEffectCreation(game_constants::INSECT_DUPLICATION_EFFECT_TOP_SCENE_OBJECT_NAME, game_constants::INSECT_DUPLICATION_EFFECT_BOT_SCENE_OBJECT_NAME, INSECT_DUPLICATION_EFFECT_TEXTURE_FILE_NAME);
    
    // Double Dino Damage Effects
    individualCardBoardEffectCreation(game_constants::NEXT_DINO_DAMAGE_DOUBLING_EFFECT_TOP_SCENE_OBJECT_NAME, game_constants::NEXT_DINO_DAMAGE_DOUBLING_EFFECT_BOT_SCENE_OBJECT_NAME, NEXT_DINO_DAMAGE_DOUBLING_EFFECT_TEXTURE_FILE_NAME);
    
    // Double Poison Attacks Effects
    individualCardBoardEffectCreation(game_constants::DOUBLE_POISON_ATTACKS_EFFECT_TOP_SCENE_OBJECT_NAME, game_constants::DOUBLE_POISON_ATTACKS_EFFECT_BOT_SCENE_OBJECT_NAME, DOUBLE_POISON_ATTACKS_EFFECT_TEXTURE_FILE_NAME);
    
    // Permanent Continual Weight Reduction Effects
    individualCardBoardEffectCreation(game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_TOP_SCENE_OBJECT_NAME, game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_BOT_SCENE_OBJECT_NAME, PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_TEXTURE_FILE_NAME);

    // Card Tooltips
    auto tooltipSceneObject = battleScene->CreateSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    tooltipSceneObject->mScale = CARD_TOOLTIP_SCALE;
    tooltipSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_TOOLTIP_TEXTURE_FILE_NAME);
    tooltipSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_TOOLTIP_SHADER_FILE_NAME);
    tooltipSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    tooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = 0.0f;
    tooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_RGB_EXPONENT_UNIFORM_NAME] = CARD_TOOLTIP_REVEAL_RGB_EXPONENT;
    tooltipSceneObject->mInvisible = true;
    
    for (auto i = 0; i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
    {
        auto tooltipTextSceneObject = battleScene->CreateSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
        scene::TextSceneObjectData tooltipTextData;
        tooltipTextData.mFontName = game_constants::DEFAULT_FONT_BLACK_NAME;
        tooltipTextSceneObject->mSceneObjectTypeData = std::move(tooltipTextData);
        tooltipTextSceneObject->mScale = glm::vec3(CARD_TOOLTIP_TEXT_FONT_SIZE);
        tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        tooltipTextSceneObject->mInvisible = true;
    }
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        HISTORY_BUTTON_POSITION,
        HISTORY_BUTTON_SCALE,
        HISTORY_ICON_TEXTURE_FILE_NAME,
        HISTORY_BUTTON_SCENE_OBJECT_NAME,
        [=](){ OnHistoryButtonPressed(); },
        *battleScene
    ));
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::InitHistoryScene()
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto historyScene = sceneManager.FindScene(HISTORY_SCENE);
    
    auto turnPointerSo = historyScene->CreateSceneObject(HISTORY_TROLLEY_SCENE_OBJECT_NAME);
    turnPointerSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + TURN_POINTER_TEXTURE_FILE_NAME);
    turnPointerSo->mPosition.z = 24.0f;
    turnPointerSo->mScale = TURN_POINTER_SCALE;
    turnPointerSo->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::NONE;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene)
{
    if (activeScene->GetName() == game_constants::IN_GAME_BATTLE_SCENE)
    {
#if defined(AUTO_PLAY)
        if (mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME)
#else
        if (mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME && mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX)
#endif
        {
            mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get());
        }
        
        if (mBoardState->GetActivePlayerIndex() == game_constants::LOCAL_PLAYER_INDEX)
        {
            HandleTouchInput(dtMillis);
        }
        
        UpdateMiscSceneObjects(dtMillis);
        
        auto foundActiveStatContainer = std::find_if(mAnimatedStatContainers.cbegin(), mAnimatedStatContainers.cend(), [](const std::pair<bool, std::unique_ptr<AnimatedStatContainer>>& statContainerEntry)
        {
            return statContainerEntry.first;
        }) != mAnimatedStatContainers.cend();
        
        if (!foundActiveStatContainer)
        {
            mActionEngine->Update(dtMillis);
        }
        
        if (mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME)
        {
            mCanInteractWithAnyHeldCard = true;
            mCanPlayNextCard = true;
            if (!mPendingCardsToBePlayed.empty())
            {
                mPendingCardsToBePlayed.erase(mPendingCardsToBePlayed.begin());
            }
        }
    }
    else if (activeScene->GetName() == HISTORY_SCENE)
    {
        const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
        const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
        
        //auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(historyScene->GetCamera().GetViewMatrix(), historyScene->GetCamera().GetProjMatrix());
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
        {
            animationManager.StopAnimation(BATTLE_SCENE_SPEED_DILATION_ANIMATION_NAME);
            animationManager.StartAnimation(std::make_unique<rendering::TweenValueAnimation>(battleScene->GetUpdateTimeSpeedFactor(), 1.0f, OVERLAY_SCENE_SPEED_ANIMATION_TARGET_DURATION), [](){}, BATTLE_SCENE_SPEED_DILATION_ANIMATION_NAME);
            
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>(OVERLAY_SCENE_SPEED_ANIMATION_TARGET_DURATION);
        }
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    if (scene->GetName() == HISTORY_SCENE)
    {
        CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(HISTORY_SCENE)->RemoveSceneObject(HISTORY_TROLLEY_SCENE_OBJECT_NAME);
    }
}

///------------------------------------------------------------------------------------------------

const BoardState& BattleSceneLogicManager::GetBoardState() const
{
    return *mBoardState;
}

///------------------------------------------------------------------------------------------------

GameActionEngine& BattleSceneLogicManager::GetActionEngine()
{
    return *mActionEngine;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& BattleSceneLogicManager::GetHeldCardSoWrappers() const
{
    return mPlayerHeldCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& BattleSceneLogicManager::GetBoardCardSoWrappers() const
{
    return mPlayerBoardCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::HandleTouchInput(const float dtMillis)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(battleScene->GetCamera().GetViewMatrix(), battleScene->GetCamera().GetProjMatrix());
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    const auto localPlayerCardCount = static_cast<int>(localPlayerCards.size());
    
    std::vector<int> candidateHighlightIndices;
    mShouldShowCardLocationIndicator = false;
    bool freeMovingCardThisFrame = false;
    for (int i = 0; i < localPlayerCardCount; ++i)
    {
        auto& currentCardSoWrapper = localPlayerCards.at(i);
        
        if (currentCardSoWrapper->mState == CardSoState::FREE_MOVING)
        {
            DestroyCardTooltip();
        }
        
        bool otherHighlightedCardExists = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper){ return cardSoWrapper.get() != currentCardSoWrapper.get() && cardSoWrapper->mState == CardSoState::HIGHLIGHTED; }) != localPlayerCards.cend();
        
        auto cardBaseSceneObject = currentCardSoWrapper->mSceneObject;
        auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*cardBaseSceneObject);
        
        bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
        
        // Check for card tooltip creation
        if (cursorInSceneObject && currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED)
        {
            mSecsCardHighlighted += dtMillis/1000.0f;
            if (mSecsCardHighlighted > CARD_TOOLTIP_CREATION_DELAY_SECS && battleScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME)->mInvisible)
            {
                if (currentCardSoWrapper->mCardData->IsSpell())
                {
                    CreateCardTooltip(currentCardSoWrapper->mSceneObject->mPosition, currentCardSoWrapper->mCardData->mCardEffectTooltip, i);
                }
            }
        }
        
#if defined(MOBILE_FLOW)
        static std::unique_ptr<glm::vec2> selectedCardInitialTouchPosition = nullptr;
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) &&
            mRuleEngine->CanCardBePlayed(currentCardSoWrapper->mCardData, i, game_constants::LOCAL_PLAYER_INDEX) &&
            ((currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED && glm::distance(worldTouchPos, *selectedCardInitialTouchPosition) > 0.005f) || currentCardSoWrapper->mState == CardSoState::FREE_MOVING) &&
            !freeMovingCardThisFrame)
        {
            currentCardSoWrapper->mState = CardSoState::FREE_MOVING;
            
            if (std::find(mPendingCardsToBePlayed.begin(), mPendingCardsToBePlayed.end(), currentCardSoWrapper) == mPendingCardsToBePlayed.end())
            {
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, glm::vec3(worldTouchPos.x, worldTouchPos.y + game_constants::IN_GAME_MOBILE_ONLY_FREE_MOVING_CARD_Y_OFFSET, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), currentCardSoWrapper->mSceneObject->mScale, game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
                
                const auto& currentLocalPlayerBoardCards = mBoardState->GetPlayerStates().at(game_constants::LOCAL_PLAYER_INDEX).mPlayerBoardCards;
                const auto& currentLocalPlayerDeadBoardCardIndices = mBoardState->GetPlayerStates().at(game_constants::LOCAL_PLAYER_INDEX).mBoardCardIndicesToDestroy;
                const auto& currentLocalPlayerBoardCardCount = card_utils::CalculateNonDeadCardsCount(currentLocalPlayerBoardCards, currentLocalPlayerDeadBoardCardIndices);
                
                auto cardLocationIndicatorSo = battleScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
                cardLocationIndicatorSo->mPosition = card_utils::CalculateBoardCardPosition(currentLocalPlayerBoardCardCount, currentLocalPlayerBoardCardCount + 1, false);
                cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
                mShouldShowCardLocationIndicator = true;
                std::vector<std::string> cardNames;
                for (const auto& soWrapper: mPendingCardsToBePlayed)
                {
                    cardNames.push_back(soWrapper->mCardData->mCardName);
                }
                freeMovingCardThisFrame = true;
            }
        }
        else if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && cursorInSceneObject && !otherHighlightedCardExists && mCanInteractWithAnyHeldCard)
        {
            auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false, battleScene->GetCamera());
            if (currentCardSoWrapper->mSceneObject->mPosition.y <= originalCardPosition.y)
            {
                selectedCardInitialTouchPosition = std::make_unique<glm::vec2>(worldTouchPos);
                candidateHighlightIndices.push_back(i);
            }
        }
        else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
        {
            switch (currentCardSoWrapper->mState)
            {
                case CardSoState::FREE_MOVING:
                {
                    OnFreeMovingCardRelease(currentCardSoWrapper);
                } break;
                    
                case CardSoState::HIGHLIGHTED:
                {
                    auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false, battleScene->GetCamera());
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                    currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                    DestroyCardHighlighterAtIndex(i);
                    mSecsCardHighlighted = 0.0f;
                } break;
                    
                default: break;
            }
        }
        
#else
        if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) &&
            currentCardSoWrapper->mState == CardSoState::FREE_MOVING &&
            !freeMovingCardThisFrame)
        {
            if (std::find(mPendingCardsToBePlayed.begin(), mPendingCardsToBePlayed.end(), currentCardSoWrapper) == mPendingCardsToBePlayed.end())
            {
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, glm::vec3(worldTouchPos.x, worldTouchPos.y, game_constants::IN_GAME_HIGHLIGHTED_CARD_Z), currentCardSoWrapper->mSceneObject->mScale, game_constants::IN_GAME_CARD_FREE_MOVEMENT_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [](){});
                
                const auto& currentLocalPlayerBoardCards = mBoardState->GetPlayerStates().at(game_constants::LOCAL_PLAYER_INDEX).mPlayerBoardCards;
                const auto& currentLocalPlayerDeadBoardCardIndices = mBoardState->GetPlayerStates().at(game_constants::LOCAL_PLAYER_INDEX).mBoardCardIndicesToDestroy;
                const auto& currentLocalPlayerBoardCardCount = card_utils::CalculateNonDeadCardsCount(currentLocalPlayerBoardCards, currentLocalPlayerDeadBoardCardIndices);
                
                auto cardLocationIndicatorSo = battleScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
                cardLocationIndicatorSo->mPosition = card_utils::CalculateBoardCardPosition(currentLocalPlayerBoardCardCount, currentLocalPlayerBoardCardCount + 1, false);
                cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
                mShouldShowCardLocationIndicator = true;
                freeMovingCardThisFrame = true;
            }
        }
        else if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) &&
                 cursorInSceneObject &&
                 !otherHighlightedCardExists &&
                 currentCardSoWrapper->mState == CardSoState::HIGHLIGHTED &&
                 mRuleEngine->CanCardBePlayed(currentCardSoWrapper->mCardData, i, game_constants::LOCAL_PLAYER_INDEX) &&
                 battleScene->FindSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i))) != nullptr)
        {
            currentCardSoWrapper->mState = CardSoState::FREE_MOVING;
        }
        else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON))
        {
            switch (currentCardSoWrapper->mState)
            {
                case CardSoState::FREE_MOVING:
                {
                    OnFreeMovingCardRelease(currentCardSoWrapper);
                } break;
                
                case CardSoState::IDLE:
                {
                    if (cursorInSceneObject && !otherHighlightedCardExists && mCanInteractWithAnyHeldCard)
                    {
                        candidateHighlightIndices.push_back(i);
                    }
                } break;
                    
                case CardSoState::HIGHLIGHTED:
                {
                    if (!cursorInSceneObject)
                    {
                        auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false, battleScene->GetCamera());
                        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){currentCardSoWrapper->mState = CardSoState::IDLE; });
                        currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
                        DestroyCardHighlighterAtIndex(i);
                        mSecsCardHighlighted = 0.0f;
                    }
                } break;
                    
                default: break;
            }
        }
#endif
    }
    
    // Select based candidate card to highlight based on distance from cursor
    std::sort(candidateHighlightIndices.begin(), candidateHighlightIndices.end(), [&](const int& lhs, const int& rhs)
    {
        return math::Abs(localPlayerCards[lhs]->mSceneObject->mPosition.x - worldTouchPos.x) <
               math::Abs(localPlayerCards[rhs]->mSceneObject->mPosition.x - worldTouchPos.x);
    });
    
    if (!candidateHighlightIndices.empty() && localPlayerCards.size() == mBoardState->GetPlayerStates()[1].mPlayerHeldCards.size())
    {
        auto currentCardSoWrapper = localPlayerCards[candidateHighlightIndices.front()];
        
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(candidateHighlightIndices.front(), localPlayerCardCount, false, battleScene->GetCamera());
        originalCardPosition.y += game_constants::IN_GAME_BOT_PLAYER_SELECTED_CARD_Y_OFFSET;
        originalCardPosition.z = game_constants::IN_GAME_HIGHLIGHTED_CARD_Z;
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::IGNORE_X_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
        {
            CreateCardHighlighter();
        });
        
        currentCardSoWrapper->mState = CardSoState::HIGHLIGHTED;
        
    }
    
    // Check for turn pointer interaction
    bool freeMovingCardExists = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper){ return cardSoWrapper->mState == CardSoState::FREE_MOVING; }) != localPlayerCards.cend();
    if (!freeMovingCardExists && mBoardState->GetActivePlayerIndex() == 1)
    {
        auto turnPointerSo = battleScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
        auto turnPointerHighlighterSo = battleScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
        
        auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*turnPointerSo);
        bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
        
        if (cursorInSceneObject && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && mCanIssueNextTurnInteraction)
        {
            animationManager.StartAnimation(std::make_unique<rendering::PulseAnimation>(turnPointerSo, TURN_POINTER_INTERACTOR_SCALE_FACTOR, TURN_POINTER_INTERACTION_PULSE_DURATION), [](){});
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(turnPointerHighlighterSo, 0.0f, game_constants::TURN_POINTER_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
            mActionEngine->AddGameAction(NEXT_PLAYER_ACTION_NAME);
            mCanIssueNextTurnInteraction = false;
        }
    }
    
    // Make sure that later pending cards appear in front of earlier ones
    if (mPendingCardsToBePlayed.size() > 1U)
    {
        for (auto i = 1U; i < mPendingCardsToBePlayed.size(); ++i)
        {
            mPendingCardsToBePlayed[i]->mSceneObject->mPosition.z = mPendingCardsToBePlayed.front()->mSceneObject->mPosition.z + i * 0.1f;
        }
    }
    
    // Additional constraints on showing the card location indicator
    mShouldShowCardLocationIndicator &= mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME;
    mShouldShowCardLocationIndicator &= mBoardState->GetActivePlayerIndex() == 1;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::UpdateMiscSceneObjects(const float dtMillis)
{
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Card Interactive Elements
    auto& localPlayerHeldCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    auto& localPlayerBoardCards = mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    auto& remotePlayerBoardCards = mPlayerBoardCardSceneObjectWrappers[game_constants::REMOTE_PLAYER_INDEX];
    
    for (size_t i = 0; i < localPlayerHeldCards.size(); ++i)
    {
        auto& cardSoWrapper = localPlayerHeldCards[i];
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
        
        if (mActionEngine->GetActiveGameActionName() != CARD_BUFFED_DEBUFFED_ANIMATION_GAME_ACTION_NAME &&
            mActionEngine->GetActiveGameActionName() != CARD_EFFECT_GAME_ACTION_NAME)
        {
            auto canCardBePlayed = mRuleEngine->CanCardBePlayed(cardSoWrapper->mCardData, i, game_constants::LOCAL_PLAYER_INDEX);
            cardSoWrapper->mSceneObject->mShaderIntUniformValues[game_constants::CARD_WEIGHT_INTERACTIVE_MODE_UNIFORM_NAME] = canCardBePlayed ? game_constants::CARD_INTERACTIVE_MODE_DEFAULT : game_constants::CARD_INTERACTIVE_MODE_NONINTERACTIVE;
            
            if (cardSoWrapper->mCardData->IsSpell())
            {
                continue;
            }
            
            const auto& heldCardStatOverrides = mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mPlayerHeldCardStatOverrides;
            int overriddenWeight = cardSoWrapper->mCardData->mCardWeight;
            if (heldCardStatOverrides.size() > i)
            {
                overriddenWeight = math::Max(0, heldCardStatOverrides[i].count(CardStatType::WEIGHT) ? heldCardStatOverrides[i].at(CardStatType::WEIGHT) : cardSoWrapper->mCardData->mCardWeight);
            }
            if (mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mBoardModifiers.mGlobalCardStatModifiers.count(CardStatType::WEIGHT))
            {
                overriddenWeight = math::Max(0, overriddenWeight + mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mBoardModifiers.mGlobalCardStatModifiers.at(CardStatType::WEIGHT));
            }
            
            if (canCardBePlayed && overriddenWeight < cardSoWrapper->mCardData->mCardWeight)
            {
                cardSoWrapper->mSceneObject->mShaderIntUniformValues[game_constants::CARD_WEIGHT_INTERACTIVE_MODE_UNIFORM_NAME] = game_constants::CARD_INTERACTIVE_MODE_INTERACTIVE;
            }
        }
    }
    for (auto& cardSoWrapper: localPlayerBoardCards)
    {
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
    }
    for (auto& cardSoWrapper: remotePlayerBoardCards)
    {
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
    }
    
    // Action Highlighters
    for (size_t i = 0U; i < localPlayerHeldCards.size(); ++i)
    {
        auto cardHighlighterObject = battleScene->FindSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
        if (cardHighlighterObject)
        {
            cardHighlighterObject->mInvisible = false;
            cardHighlighterObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
            cardHighlighterObject->mPosition = localPlayerHeldCards[i]->mSceneObject->mPosition;
            cardHighlighterObject->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
        }
    }
    
    // Turn pointer highlighter
    auto turnPointerSo = battleScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    auto turnPointerHighlighterSo = battleScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
    turnPointerHighlighterSo->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
    turnPointerHighlighterSo->mShaderBoolUniformValues[game_constants::CARD_HIGHLIGHTER_INVALID_ACTION_UNIFORM_NAME] = false;
    turnPointerHighlighterSo->mPosition = turnPointerSo->mPosition;
    turnPointerHighlighterSo->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
    
    // Lambda to make space/revert to original position board cards
    auto prospectiveMakeSpaceRevertToPositionLambda = [&](int prospectiveCardCount)
    {
        auto& boardCardSoWrappers = mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        const auto currentBoardCardCount = static_cast<int>(boardCardSoWrappers.size());
        
        for (int i = 0; i < currentBoardCardCount; ++i)
        {
            auto animationName = strutils::StringId(MAKE_SPACE_REVERT_TO_POSITION_ANIMATION_NAME_PREFIX + std::to_string(i));
            auto& currentCardSoWrapper = boardCardSoWrappers.at(i);
            auto originalCardPosition = card_utils::CalculateBoardCardPosition(i, prospectiveCardCount, false);
            animationManager.StopAnimation(animationName);
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){}, animationName);
        }
    };
    
    // Card Location
    auto cardLocationIndicatorSo = battleScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    auto currentSoWrapperIter = std::find_if(localPlayerHeldCards.begin(), localPlayerHeldCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper){ return cardSoWrapper->mState == CardSoState::FREE_MOVING; });
    
    if (mShouldShowCardLocationIndicator && currentSoWrapperIter != localPlayerHeldCards.end())
    {
        cardLocationIndicatorSo->mInvisible = false;
        cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
        
        auto distanceFromCardLocationSo = math::Distance2IgnoreZ((*currentSoWrapperIter)->mSceneObject->mPosition, cardLocationIndicatorSo->mPosition);
#if defined(MOBILE_FLOW)
        bool inBoardDropThreshold = distanceFromCardLocationSo <= MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#else
        bool inBoardDropThreshold = distanceFromCardLocationSo <= DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#endif
        
        const auto& currentLocalPlayerBoardCards = mBoardState->GetPlayerStates().at(game_constants::LOCAL_PLAYER_INDEX).mPlayerBoardCards;
        const auto& currentLocalPlayerDeadBoardCardIndices = mBoardState->GetPlayerStates().at(game_constants::LOCAL_PLAYER_INDEX).mBoardCardIndicesToDestroy;
        const auto& currentLocalPlayerBoardCardCount = card_utils::CalculateNonDeadCardsCount(currentLocalPlayerBoardCards, currentLocalPlayerDeadBoardCardIndices);
        
        // If in drop threshold we lerp to max target location alpha
        if (inBoardDropThreshold)
        {
            cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
            if (cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] >= CARD_LOCATION_EFFECT_MAX_TARGET_ALPHA)
            {
                cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = CARD_LOCATION_EFFECT_MAX_TARGET_ALPHA;
            }
            
            if (mPreviousProspectiveBoardCardsPushState == ProspectiveBoardCardsPushState::MAKE_SPACE_FOR_NEW_CARD)
            {
                prospectiveMakeSpaceRevertToPositionLambda(currentLocalPlayerBoardCardCount + 1);
            }
            mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::MAKE_SPACE_FOR_NEW_CARD;
        }
        else
        {
            // Else if not, we constrain the alpha to the min target
            if (math::Abs(cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] - CARD_LOCATION_EFFECT_MIN_TARGET_ALPHA) > dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED)
            {
                if (cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] > CARD_LOCATION_EFFECT_MIN_TARGET_ALPHA)
                {
                    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
                }
                else
                {
                    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] += dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
                }
            }
            
            if (mPreviousProspectiveBoardCardsPushState != ProspectiveBoardCardsPushState::REVERT_TO_ORIGINAL_POSITION)
            {
                prospectiveMakeSpaceRevertToPositionLambda(currentLocalPlayerBoardCardCount);
            }
            mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::REVERT_TO_ORIGINAL_POSITION;
        }
    }
    else
    {
        cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] -= dtMillis * CARD_LOCATION_EFFECT_ALPHA_SPEED;
        if (cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] <= 0.0f)
        {
            cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            cardLocationIndicatorSo->mInvisible = true;
        }
        
        mPreviousProspectiveBoardCardsPushState = ProspectiveBoardCardsPushState::NONE;
    }
    
    // Stat Crystal Values
    for (auto& statContainerEntry: mAnimatedStatContainers)
    {
        if (statContainerEntry.first)
        {
            statContainerEntry.first = statContainerEntry.second->Update(dtMillis) == AnimatedStatContainerUpdateResult::ONGOING;
        }
    }
    
    // Board side effects
    auto boardSideEffectTopSceneObject = battleScene->FindSceneObject(game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME);
    boardSideEffectTopSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = fmod(time/10, 1.0f);
    
    auto boardSideEffectBotSceneObject = battleScene->FindSceneObject(game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    boardSideEffectBotSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = fmod(time/10, 1.0f);
    
    for (const auto& activePlayerIndividualCardEffectSceneObjects: mActiveIndividualCardBoardEffectSceneObjects)
    {
        for (auto& effectSceneObject: activePlayerIndividualCardEffectSceneObjects)
        {
            effectSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = 0.0f;
        }
    }
    
    // Card tooltip
    auto cardTooltipSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    
    cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_TOOLTIP_REVEAL_SPEED;
    if (cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] >= CARD_TOOLTIP_MAX_REVEAL_THRESHOLD)
    {
        cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = CARD_TOOLTIP_MAX_REVEAL_THRESHOLD;
        
        for (auto i = 0; i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
        {
            auto tooltipTextSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
            tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Min(1.0f, tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] +  dtMillis * CARD_TOOLTIP_TEXT_REVEAL_SPEED);
        }
    }
    
    // Animated buttons
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    auto cardIndex = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [=](const std::shared_ptr<CardSoWrapper>& otherCard)
    {
        return otherCard.get() == cardSoWrapper.get();
    }) - localPlayerCards.begin();
    
    DestroyCardHighlighterAtIndex(static_cast<int>(cardIndex));
    
    auto cardLocationIndicatorSo = battleScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    auto distanceFromCardLocationSo = math::Distance2IgnoreZ(cardSoWrapper->mSceneObject->mPosition, cardLocationIndicatorSo->mPosition);
    
#if defined(MOBILE_FLOW)
    bool inBoardDropThreshold = distanceFromCardLocationSo <= MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#else
    bool inBoardDropThreshold = distanceFromCardLocationSo <= DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR;
#endif
    
    if (inBoardDropThreshold &&
        (mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME || mActionEngine->GetActionCount() <= 2) &&
        mBoardState->GetActivePlayerIndex() == 1 &&
        mRuleEngine->CanCardBePlayed(cardSoWrapper->mCardData, cardIndex, game_constants::LOCAL_PLAYER_INDEX))
    {
        bool inPendingCardsToBePlayed = std::find(mPendingCardsToBePlayed.begin(), mPendingCardsToBePlayed.end(), cardSoWrapper) != mPendingCardsToBePlayed.end();
        if (mCanPlayNextCard && !inPendingCardsToBePlayed)
        {
            mActionEngine->AddGameAction(PLAY_CARD_ACTION_NAME, {{PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, std::to_string(cardIndex)}});
            mCanPlayNextCard = false;
        }
        else
        {
            if (!inPendingCardsToBePlayed)
            {
                mPendingCardsToBePlayed.push_back(cardSoWrapper);
            }
        }
    }
    else if (!inBoardDropThreshold || mCanPlayNextCard)
    {
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(static_cast<int>(cardIndex), static_cast<int>(localPlayerCards.size()), false, battleScene->GetCamera());
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, originalCardPosition, cardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){cardSoWrapper->mState = CardSoState::IDLE; });
        cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
    }

}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::CreateCardHighlighter()
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    for (size_t i = 0U; i < localPlayerCards.size(); ++i)
    {
        battleScene->RemoveSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
    }

    auto highlightedCardIter = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [&](const std::shared_ptr<CardSoWrapper>& cardSoWrapper)
    {
#if defined(MOBILE_FLOW)
        return cardSoWrapper->mState == CardSoState::HIGHLIGHTED || cardSoWrapper->mState == CardSoState::FREE_MOVING;
#else
        return cardSoWrapper->mState == CardSoState::HIGHLIGHTED;
#endif
    });
    if (highlightedCardIter != localPlayerCards.cend())
    {
        auto cardIndex = highlightedCardIter - localPlayerCards.cbegin();
        auto cardHighlighterSo = battleScene->CreateSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(cardIndex)));
        
        cardHighlighterSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::ACTION_HIGHLIGHTER_SHADER_NAME);
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_TIME_SPEED;
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_RESOLUTION;
        cardHighlighterSo->mShaderFloatUniformValues[game_constants::PERLIN_CLARITY_UNIFORM_NAME] = game_constants::ACTION_HIGLIGHTER_PERLIN_CLARITY;
        cardHighlighterSo->mShaderBoolUniformValues[game_constants::CARD_HIGHLIGHTER_INVALID_ACTION_UNIFORM_NAME] = !mRuleEngine->CanCardBePlayed((*highlightedCardIter)->mCardData, cardIndex, game_constants::LOCAL_PLAYER_INDEX);
        cardHighlighterSo->mPosition = (*highlightedCardIter)->mSceneObject->mPosition;
        cardHighlighterSo->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
        cardHighlighterSo->mScale = game_constants::CARD_HIGHLIGHTER_SCALE;
        cardHighlighterSo->mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText, const size_t cardIndex)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    const auto& sceneManager = systemsEngine.GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto tooltipSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    
    bool shouldBeFlipped = cardIndex >= mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX].size()/2 && cardIndex != 0;
    tooltipSceneObject->mPosition = cardOriginPostion + CARD_TOOLTIP_OFFSET;
    tooltipSceneObject->mPosition.x += shouldBeFlipped ? CARD_TOOLTIP_FLIPPED_X_OFFSET : 0.0f;
    tooltipSceneObject->mInvisible = false;
    tooltipSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    tooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = 0.0f;
    tooltipSceneObject->mScale.x = shouldBeFlipped ? -CARD_TOOLTIP_SCALE.x : CARD_TOOLTIP_SCALE.x;
    
    auto tooltipTextRows = strutils::StringSplit(tooltipText, '$');
    
    if (tooltipTextRows.size() == 1)
    {
        auto tooltipTextSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[1]);
        tooltipTextSceneObject->mPosition = tooltipSceneObject->mPosition;
        tooltipTextSceneObject->mPosition += CARD_TOOLTIP_TEXT_OFFSETS[1];
        tooltipTextSceneObject->mPosition.x += shouldBeFlipped ? CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET : 0.0f;
        tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        std::get<scene::TextSceneObjectData>(tooltipTextSceneObject->mSceneObjectTypeData).mText = tooltipTextRows[0];
        tooltipTextSceneObject->mInvisible = false;
    }
    else
    {
        for (auto i = 0U; i < tooltipTextRows.size(); ++i)
        {
            assert(i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT);
            auto tooltipTextSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
            tooltipTextSceneObject->mPosition = tooltipSceneObject->mPosition;
            tooltipTextSceneObject->mPosition += CARD_TOOLTIP_TEXT_OFFSETS[i];
            tooltipTextSceneObject->mPosition.x += shouldBeFlipped ? CARD_TOOLTIP_TEXT_FLIPPED_X_OFFSET : 0.0f;
            tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
            std::get<scene::TextSceneObjectData>(tooltipTextSceneObject->mSceneObjectTypeData).mText = tooltipTextRows[i];
            tooltipTextSceneObject->mInvisible = false;
        }
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::DestroyCardHighlighterAtIndex(const int index)
{
    mSecsCardHighlighted = 0.0f;
    
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto cardHighlighterName = strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(index));
    battleScene->RemoveSceneObject(cardHighlighterName);
    
    DestroyCardTooltip();
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::DestroyCardTooltip()
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto tooltipSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    tooltipSceneObject->mInvisible = true;
    
    for (auto i = 0; i < game_constants::CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
    {
        auto tooltipTextSceneObject = battleScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
        tooltipTextSceneObject->mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    
    eventSystem.RegisterForEvent<events::ApplicationMovedToBackgroundEvent>(this, &BattleSceneLogicManager::OnApplicationMovedToBackground);
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &BattleSceneLogicManager::OnWindowResize);
    eventSystem.RegisterForEvent<events::LocalPlayerTurnStarted>(this, &BattleSceneLogicManager::OnLocalPlayerTurnStarted);
    eventSystem.RegisterForEvent<events::EndOfTurnCardDestructionEvent>(this, &BattleSceneLogicManager::OnEndOfTurnCardDestruction);
    eventSystem.RegisterForEvent<events::ImmediateCardDestructionWithRepositionEvent>(this, &BattleSceneLogicManager::OnImmediateCardDestructionWithReposition);
    eventSystem.RegisterForEvent<events::CardCreationEvent>(this, &BattleSceneLogicManager::OnCardCreation);
    eventSystem.RegisterForEvent<events::CardBuffedDebuffedEvent>(this, &BattleSceneLogicManager::OnCardBuffedDebuffed);
    eventSystem.RegisterForEvent<events::HeldCardSwapEvent>(this, &BattleSceneLogicManager::OnHeldCardSwap);
    eventSystem.RegisterForEvent<events::NewBoardCardCreatedEvent>(this, &BattleSceneLogicManager::OnNewBoardCardCreated);
    eventSystem.RegisterForEvent<events::LastCardPlayedFinalizedEvent>(this, &BattleSceneLogicManager::OnLastCardPlayedFinalized);
    eventSystem.RegisterForEvent<events::HealthChangeAnimationTriggerEvent>(this, &BattleSceneLogicManager::OnHealthChangeAnimationTrigger);
    eventSystem.RegisterForEvent<events::WeightChangeAnimationTriggerEvent>(this, &BattleSceneLogicManager::OnWeightChangeAnimationTrigger);
    eventSystem.RegisterForEvent<events::BoardSideCardEffectTriggeredEvent>(this, &BattleSceneLogicManager::OnBoardSideCardEffectTriggered);
    eventSystem.RegisterForEvent<events::BoardSideCardEffectEndedEvent>(this, &BattleSceneLogicManager::OnBoardSideCardEffectEnded);
    eventSystem.RegisterForEvent<events::ForceSendCardBackToPositionEvent>(this, &BattleSceneLogicManager::OnForceSendCardBackToPosition);
    eventSystem.RegisterForEvent<events::PoisonStackChangeChangeAnimationTriggerEvent>(this, &BattleSceneLogicManager::OnPoisonStackChangeChangeAnimationTrigger);
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnApplicationMovedToBackground(const events::ApplicationMovedToBackgroundEvent&)
{
    mGameSerializer->FlushStateToFile();
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnWindowResize(const events::WindowResizeEvent&)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Correct position of held cards
    for (auto j = 0U; j < mPlayerHeldCardSceneObjectWrappers.size(); ++j)
    {
        for (auto i = 0U; i < mPlayerHeldCardSceneObjectWrappers[j].size(); ++i)
        {
            auto cardSoWrapper = mPlayerHeldCardSceneObjectWrappers[j][i];
            if (cardSoWrapper->mState == CardSoState::IDLE)
            {
                cardSoWrapper->mSceneObject->mPosition = card_utils::CalculateHeldCardPosition(i, static_cast<int>(mPlayerHeldCardSceneObjectWrappers[j].size()), j == game_constants::REMOTE_PLAYER_INDEX, battleScene->GetCamera());
            }
        }
    }
    
    // Correct position of other snap to edge scene objects
    battleScene->RecalculatePositionOfEdgeSnappingSceneObjects();
    
    // Fix position of child->parent objects
    auto turnPointerSo = battleScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    auto turnPointerHighlighterSo = battleScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
    turnPointerHighlighterSo->mPosition = turnPointerSo->mPosition;
    turnPointerHighlighterSo->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnLocalPlayerTurnStarted(const events::LocalPlayerTurnStarted&)
{
    mCanIssueNextTurnInteraction = true;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnEndOfTurnCardDestruction(const events::EndOfTurnCardDestructionEvent& event)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& cardSoWrappers = event.mIsBoardCard ?
        mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)]:
        mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
    
    std::vector<std::shared_ptr<CardSoWrapper>> remainingCards;
    for (int i = 0; i < static_cast<int>(cardSoWrappers.size()); ++i)
    {
        if (std::find_if(event.mCardIndices.cbegin(), event.mCardIndices.cend(), [=](const std::string& index){ return std::stoi(index) == i; }) == event.mCardIndices.end())
        {
            remainingCards.emplace_back(cardSoWrappers[i]);
        }
        else
        {
            battleScene->RemoveSceneObject(cardSoWrappers[i]->mSceneObject->mName);
        }
    }
    
    if (event.mIsBoardCard)
    {
        mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)] = remainingCards;
    }
    else
    {
        mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)] = remainingCards;
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnImmediateCardDestructionWithReposition(const events::ImmediateCardDestructionWithRepositionEvent& event)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    const auto& cards = event.mIsBoardCard ?
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mPlayerBoardCards:
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mPlayerHeldCards;
    const auto& cardIndicesToDestroy = event.mIsBoardCard ?
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mBoardCardIndicesToDestroy:
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mHeldCardIndicesToDestroy;
    auto currentCardCount = card_utils::CalculateNonDeadCardsCount(cards, cardIndicesToDestroy);
    
    auto& cardSoWrappers = event.mIsBoardCard ?
        mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)]:
        mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
    battleScene->RemoveSceneObject(cardSoWrappers[event.mCardIndex]->mSceneObject->mName);
    
    cardSoWrappers.erase(cardSoWrappers.begin() + event.mCardIndex);
    
    // Animate rest of the cards to position.
    for (int i = 0; i < currentCardCount; ++i)
    {
        auto& currentCardSoWrapper = cardSoWrappers.at(i);
        
        currentCardSoWrapper->mSceneObject->mName = event.mIsBoardCard ?
            strutils::StringId((mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(i)):
            strutils::StringId((mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX) + std::to_string(i));
        
        auto originalCardPosition = event.mIsBoardCard ?
            card_utils::CalculateBoardCardPosition(i, currentCardCount, event.mForRemotePlayer) :
            card_utils::CalculateHeldCardPosition(i, currentCardCount, event.mForRemotePlayer, battleScene->GetCamera());
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnCardCreation(const events::CardCreationEvent& event)
{
    mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].push_back(event.mCardSoWrapper);
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnCardBuffedDebuffed(const events::CardBuffedDebuffedEvent& event)
{
    auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    if (event.mBoardCard)
    {
        auto& boardSceneObjectWrappers = mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
        auto& cardSceneObjectWrapper = boardSceneObjectWrappers[event.mCardIndex];
        auto previousScale = cardSceneObjectWrapper->mSceneObject->mScale;
        
        battleScene->RemoveSceneObject(cardSceneObjectWrapper->mSceneObject->mName);
        
        boardSceneObjectWrappers[event.mCardIndex] = card_utils::CreateCardSoWrapper
        (
            cardSceneObjectWrapper->mCardData,
            cardSceneObjectWrapper->mSceneObject->mPosition,
            (event.mForRemotePlayer ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(event.mCardIndex),
            CardOrientation::FRONT_FACE,
            card_utils::GetCardRarity(cardSceneObjectWrapper->mCardData->mCardId, mBoardState->GetActivePlayerIndex(), *mBoardState),
            true,
            event.mForRemotePlayer,
            true,
            (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.size()) > event.mCardIndex ? mBoardState->GetActivePlayerState().mPlayerBoardCardStatOverrides.at(event.mCardIndex) : CardStatOverrides()),
            mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers,
            *battleScene
        );
        boardSceneObjectWrappers[event.mCardIndex]->mSceneObject->mScale = previousScale;
    }
    else
    {
        auto& heldSceneObjectWrappers = mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
        auto& cardSceneObjectWrapper = heldSceneObjectWrappers[event.mCardIndex];
        auto previousScale = cardSceneObjectWrapper->mSceneObject->mScale;
        
        battleScene->RemoveSceneObject(cardSceneObjectWrapper->mSceneObject->mName);
        
        heldSceneObjectWrappers[event.mCardIndex] = card_utils::CreateCardSoWrapper
        (
            cardSceneObjectWrapper->mCardData,
            cardSceneObjectWrapper->mSceneObject->mPosition,
            (event.mForRemotePlayer ? game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX) + std::to_string(event.mCardIndex),
            CardOrientation::FRONT_FACE,
            card_utils::GetCardRarity(cardSceneObjectWrapper->mCardData->mCardId, mBoardState->GetActivePlayerIndex(), *mBoardState),
            false,
            event.mForRemotePlayer,
            mRuleEngine->CanCardBePlayed(heldSceneObjectWrappers[event.mCardIndex]->mCardData, event.mCardIndex, game_constants::LOCAL_PLAYER_INDEX),
            (static_cast<int>(mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.size()) > event.mCardIndex ? mBoardState->GetActivePlayerState().mPlayerHeldCardStatOverrides.at(event.mCardIndex) : CardStatOverrides()),
            mBoardState->GetActivePlayerState().mBoardModifiers.mGlobalCardStatModifiers,
            *battleScene
        );
        heldSceneObjectWrappers[event.mCardIndex]->mSceneObject->mScale = previousScale;
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnHeldCardSwap(const events::HeldCardSwapEvent& event)
{
    mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)][event.mCardIndex] = event.mCardSoWrapper;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnNewBoardCardCreated(const events::NewBoardCardCreatedEvent& event)
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& playerBoardCardSoWrappers = mPlayerBoardCardSceneObjectWrappers[event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX];
    
    playerBoardCardSoWrappers.push_back(event.mCardSoWrapper);
    
    const auto& boardCards = mBoardState->GetPlayerStates().at(mBoardState->GetActivePlayerIndex()).mPlayerBoardCards;
    const auto& deadBoardCardIndices = mBoardState->GetPlayerStates().at(mBoardState->GetActivePlayerIndex()).mBoardCardIndicesToDestroy;
    const auto& boardCardCount = card_utils::CalculateNonDeadCardsCount(boardCards, deadBoardCardIndices);
    
    // Animate and rename board cards to position. Last one will be animated externally
    for (int i = 0; i < boardCardCount; ++i)
    {
        auto& currentCardSoWrapper = playerBoardCardSoWrappers.at(i);
        if (i !=  boardCardCount - 1)
        {
            auto originalCardPosition = card_utils::CalculateBoardCardPosition(i, boardCardCount, mBoardState->GetActivePlayerIndex() == 0);
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
        }
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnLastCardPlayedFinalized(const events::LastCardPlayedFinalizedEvent& event)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    battleScene->RemoveSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(event.mCardIndex)));
    
    auto& playerHeldCardSoWrappers = mPlayerHeldCardSceneObjectWrappers[mBoardState->GetActivePlayerIndex()];
    auto& playerBoardCardSoWrappers = mPlayerBoardCardSceneObjectWrappers[mBoardState->GetActivePlayerIndex()];
    
    playerBoardCardSoWrappers.push_back(playerHeldCardSoWrappers[event.mCardIndex]);
    playerHeldCardSoWrappers.erase(playerHeldCardSoWrappers.begin() + event.mCardIndex);
    
    const auto currentPlayerHeldCardCount = static_cast<int>(playerHeldCardSoWrappers.size());
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    for (int i = 0; i < currentPlayerHeldCardCount; ++i)
    {
        auto& currentCardSoWrapper = playerHeldCardSoWrappers.at(i);
        
        // Rename held cards for different indices
        currentCardSoWrapper->mSceneObject->mName = strutils::StringId((mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::TOP_PLAYER_HELD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_HELD_CARD_SO_NAME_PREFIX) + std::to_string(i));
        
        // Reposition held cards for different indices
        if (currentCardSoWrapper->mState != CardSoState::FREE_MOVING)
        {
            auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, currentPlayerHeldCardCount, mBoardState->GetActivePlayerIndex() == 0, battleScene->GetCamera());
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){ currentCardSoWrapper->mState = CardSoState::IDLE; });
            currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
        }
    }
    
    const auto& boardCards = mBoardState->GetPlayerStates().at(mBoardState->GetActivePlayerIndex()).mPlayerBoardCards;
    const auto& deadBoardCardIndices = mBoardState->GetPlayerStates().at(mBoardState->GetActivePlayerIndex()).mBoardCardIndicesToDestroy;
    const auto& boardCardCount = card_utils::CalculateNonDeadCardsCount(boardCards, deadBoardCardIndices);
    
    // Animate and rename board cards to position. Last one will be animated externally
    for (int i = 0; i < boardCardCount; ++i)
    {
        auto& currentCardSoWrapper = playerBoardCardSoWrappers.at(i);
        currentCardSoWrapper->mSceneObject->mName = strutils::StringId((mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(i));
        
        if (i !=  boardCardCount - 1)
        {
            auto originalCardPosition = card_utils::CalculateBoardCardPosition(i, boardCardCount, mBoardState->GetActivePlayerIndex() == 0);
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
        }
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnHealthChangeAnimationTrigger(const events::HealthChangeAnimationTriggerEvent& event)
{
    mAnimatedStatContainers[event.mForRemotePlayer ? 0 : 1].first = true;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnWeightChangeAnimationTrigger(const events::WeightChangeAnimationTriggerEvent& event)
{
    mAnimatedStatContainers[event.mForRemotePlayer ? 2 : 3].first = true;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnBoardSideCardEffectTriggered(const events::BoardSideCardEffectTriggeredEvent& event)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    const auto& sceneManager = systemsEngine.GetSceneManager();
    
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    std::shared_ptr<scene::SceneObject> sideEffectSceneObject = nullptr;
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_DEBUFF)
    {
        sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    }
    else
    {
        sideEffectSceneObject = nullptr;
        if (event.mEffectBoardModifierMask == effects::board_modifier_masks::KILL_NEXT)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::DUPLICATE_NEXT_INSECT)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::INSECT_DUPLICATION_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::INSECT_DUPLICATION_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::NEXT_DINO_DAMAGE_DOUBLING_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::NEXT_DINO_DAMAGE_DOUBLING_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::DOUBLE_POISON_ATTACKS)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::DOUBLE_POISON_ATTACKS_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::DOUBLE_POISON_ATTACKS_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        
        assert(sideEffectSceneObject);
        
        if (!sideEffectSceneObject->mInvisible)
        {
            // effect already showing (and not additive like the the side stat modifier above)
            return;
        }
        
        sideEffectSceneObject->mScale = game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_SCALE;
        sideEffectSceneObject->mRotation = glm::vec3(0.0f);
        animationManager.StartAnimation(std::make_unique<rendering::PulseAnimation>(sideEffectSceneObject, game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_SCALE_UP_FACTOR, game_constants::INDIVIDUAL_CARD_BOARD_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS, animation_flags::ANIMATE_CONTINUOUSLY), [](){});
        
        auto& activeEffects = mActiveIndividualCardBoardEffectSceneObjects[event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX];
        if (std::find(activeEffects.cbegin(), activeEffects.cend(), sideEffectSceneObject) == activeEffects.cend())
        {
            activeEffects.push_back(sideEffectSceneObject);
            
            for (auto i = 0U; i < activeEffects.size(); ++i)
            {
                const auto& targetPosition = CalculateBoardEffectPosition(i, activeEffects.size(), event.mForRemotePlayer);
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(activeEffects[i], targetPosition, activeEffects[i]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
            }
        }
    }
    
    sideEffectSceneObject->mInvisible = false;
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sideEffectSceneObject, BOARD_EFFECT_MAX_ALPHA, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
    
    
    // Update text specifically for board side stat modifier
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_DEBUFF)
    {
        for (int i = 0; i < game_constants::BOARD_SIDE_EFFECT_VALUE_SO_COUNT; ++i)
        {
            auto boardSideEffectValueSceneObject = battleScene->FindSceneObject(strutils::StringId((event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX) + std::to_string(i)));
            boardSideEffectValueSceneObject->mInvisible = false;
            
            std::get<scene::TextSceneObjectData>(boardSideEffectValueSceneObject->mSceneObjectTypeData).mText = std::to_string(mBoardState->GetPlayerStates()[event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX].mBoardModifiers.mGlobalCardStatModifiers.at(CardStatType::DAMAGE));
            
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(boardSideEffectValueSceneObject, BOARD_EFFECT_MAX_ALPHA * 2, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
        }
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnBoardSideCardEffectEnded(const events::BoardSideCardEffectEndedEvent& event)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    const auto& sceneManager = systemsEngine.GetSceneManager();
    
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    std::shared_ptr<scene::SceneObject> sideEffectSceneObject = nullptr;
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_DEBUFF)
    {
        sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    }
    else
    {
        sideEffectSceneObject = nullptr;
        if (event.mEffectBoardModifierMask == effects::board_modifier_masks::KILL_NEXT)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::DUPLICATE_NEXT_INSECT)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::INSECT_DUPLICATION_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::INSECT_DUPLICATION_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::DOUBLE_NEXT_DINO_DAMAGE)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::NEXT_DINO_DAMAGE_DOUBLING_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::NEXT_DINO_DAMAGE_DOUBLING_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::DOUBLE_POISON_ATTACKS)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::DOUBLE_POISON_ATTACKS_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::DOUBLE_POISON_ATTACKS_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::PERMANENT_CONTINUAL_WEIGHT_REDUCTION)
        {
            sideEffectSceneObject = battleScene->FindSceneObject(event.mForRemotePlayer ? game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_BOT_SCENE_OBJECT_NAME);
        }
        
        assert(sideEffectSceneObject);
        
        auto& activeEffects = mActiveIndividualCardBoardEffectSceneObjects[event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX];
        if (!activeEffects.empty())
        {
            auto foundIter = std::find(activeEffects.cbegin(), activeEffects.cend(), sideEffectSceneObject);
            if (foundIter != activeEffects.end())
            {
                activeEffects.erase(std::find(activeEffects.cbegin(), activeEffects.cend(), sideEffectSceneObject));
            }
            
            for (auto i = 0U; i < activeEffects.size(); ++i)
            {
                if (!event.mMassClear || (activeEffects[i]->mName == game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_TOP_SCENE_OBJECT_NAME || activeEffects[i]->mName == game_constants::PERMANENT_CONTINUAL_WEIGHT_REDUCTION_EFFECT_BOT_SCENE_OBJECT_NAME))
                {
                    const auto& targetPosition = CalculateBoardEffectPosition(i, activeEffects.size(), event.mForRemotePlayer);
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(activeEffects[i], targetPosition, activeEffects[i]->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
                }
            }
        }
    }
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sideEffectSceneObject, 0.0f, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
    {
        sideEffectSceneObject->mInvisible = true;
    });
    
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_DEBUFF)
    {
        for (int i = 0; i < game_constants::BOARD_SIDE_EFFECT_VALUE_SO_COUNT; ++i)
        {
            auto boardSideEffectValueSceneObject = battleScene->FindSceneObject(strutils::StringId((event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX) + std::to_string(i)));
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(boardSideEffectValueSceneObject, 0.0f, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
            {
                boardSideEffectValueSceneObject->mInvisible = true;
            });
        }
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnForceSendCardBackToPosition(const events::ForceSendCardBackToPositionEvent& event)
{
    const auto& sceneManager = CoreSystemsEngine::GetInstance().GetSceneManager();
    auto battleScene = sceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    const auto& cards = event.mBoardCard ?
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mPlayerBoardCards:
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mPlayerHeldCards;
    const auto& cardIndicesToDestroy = event.mBoardCard ?
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mBoardCardIndicesToDestroy:
        mBoardState->GetPlayerStates()[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].mHeldCardIndicesToDestroy;
    auto currentCardCount = card_utils::CalculateNonDeadCardsCount(cards, cardIndicesToDestroy);
    
    auto& cardSoWrappers = event.mBoardCard ?
        mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)]:
        mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
    
    auto& cardSoWrapper = cardSoWrappers[event.mCardIdex];
    
    cardSoWrapper->mState = CardSoState::IDLE;
    
    auto originalCardPosition = event.mBoardCard ?
        card_utils::CalculateBoardCardPosition(event.mCardIdex, currentCardCount, event.mForRemotePlayer) :
        card_utils::CalculateHeldCardPosition(event.mCardIdex, currentCardCount, event.mForRemotePlayer, battleScene->GetCamera());
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, originalCardPosition, cardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
    
    DestroyCardHighlighterAtIndex(event.mCardIdex);
    
    mCanInteractWithAnyHeldCard = false;
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnPoisonStackChangeChangeAnimationTrigger(const events::PoisonStackChangeChangeAnimationTriggerEvent& event)
{
    auto& affectedContainerEntry = mAnimatedStatContainers[event.mForRemotePlayer ? 4 : 5];
    affectedContainerEntry.first = true;
    auto newPoisonStackValue = event.mNewPoisonStackValue;
    
    for (auto sceneObject: affectedContainerEntry.second->GetSceneObjects())
    {
        if (newPoisonStackValue != 0)
        {
            sceneObject->mInvisible = false;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, newPoisonStackValue == 0 ? 0.0f : 1.0f, game_constants::POISON_STACK_SHOW_HIDE_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
        {
            if (newPoisonStackValue == 0)
            {
                sceneObject->mInvisible = true;
            }
        });
    }
}

///------------------------------------------------------------------------------------------------

void BattleSceneLogicManager::OnHistoryButtonPressed()
{
    auto battleScene = CoreSystemsEngine::GetInstance().GetSceneManager().FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenValueAnimation>(battleScene->GetUpdateTimeSpeedFactor(), 0.0f, OVERLAY_SCENE_SPEED_ANIMATION_TARGET_DURATION), [](){}, BATTLE_SCENE_SPEED_DILATION_ANIMATION_NAME);
    
    events::EventSystem::GetInstance().DispatchEvent<events::SceneChangeEvent>(HISTORY_SCENE, true, OVERLAY_SCENE_SPEED_ANIMATION_TARGET_DURATION, HISTORY_MODAL_MAX_ALPHA);
}

///------------------------------------------------------------------------------------------------

glm::vec3 BattleSceneLogicManager::CalculateBoardEffectPosition(const size_t effectIndex, const size_t effectsCount, bool forRemotePlayer)
{
    float cardBlockWidth = game_constants::IN_GAME_CARD_ON_BOARD_WIDTH * effectsCount;
    float cardStartX = -cardBlockWidth/2.0f;
    
    auto targetX = cardStartX + effectIndex * game_constants::IN_GAME_CARD_ON_BOARD_WIDTH + game_constants::IN_GAME_CARD_ON_BOARD_WIDTH/2;
    if (effectsCount > game_constants::IN_GAME_CARD_PUSH_THRESHOLD)
    {
        float pushX = (effectsCount - game_constants::IN_GAME_CARD_PUSH_THRESHOLD) * game_constants::IN_GAME_CARD_PUSH_VALUE * (math::Abs(effectIndex - effectsCount/2));
        bool oddCardCount = effectsCount % 2 != 0;
        if ((oddCardCount && effectIndex != effectsCount/2) || !oddCardCount)
        {
            targetX += (effectIndex < effectsCount/2) ? pushX : -pushX;
        }
    }
    
    return glm::vec3(targetX, forRemotePlayer ? BOARD_SIDE_EFFECT_TOP_POSITION.y : BOARD_SIDE_EFFECT_BOT_POSITION.y, INDIVIDUAL_CARD_BOARD_EFFECT_BASE_Z + effectIndex * INDIVIDUAL_CARD_BOARD_EFFECT_Z_INCREMENT);
}

///------------------------------------------------------------------------------------------------
