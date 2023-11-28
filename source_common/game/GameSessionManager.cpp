///------------------------------------------------------------------------------------------------
///  GameSessionManager.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 11/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/AnimatedStatContainer.h>
#include <game/BoardState.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/GameConstants.h>
#include <game/GameReplayEngine.h>
#include <game/GameRuleEngine.h>
#include <game/GameSerializer.h>
#include <game/GameSessionManager.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayerActionGenerationEngine.h>
#include <game/utils/PersistenceUtils.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>

///------------------------------------------------------------------------------------------------

static constexpr int CARD_TOOLTIP_TEXT_ROWS_COUNT = 4;

static const strutils::StringId CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME = strutils::StringId("CARD_LOCATION_INDICATOR");
static const strutils::StringId CARD_TOOLTIP_SCENE_OBJECT_NAME = strutils::StringId("CARD_TOOLTIP");
static const strutils::StringId CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME = strutils::StringId("reveal_threshold");
static const strutils::StringId CARD_TOOLTIP_REVEAL_RGB_EXPONENT_UNIFORM_NAME = strutils::StringId("reveal_rgb_exponent");
static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId PLAY_CARD_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_ACTION_NAME = strutils::StringId("NextPlayerGameAction");
static const strutils::StringId CARD_EFFECT_GAME_ACTION_NAME = strutils::StringId("CardEffectGameAction");
static const strutils::StringId CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES [CARD_TOOLTIP_TEXT_ROWS_COUNT] =
{
    strutils::StringId("CARD_TOOLTIP_TEXT_0"),
    strutils::StringId("CARD_TOOLTIP_TEXT_1"),
    strutils::StringId("CARD_TOOLTIP_TEXT_2"),
    strutils::StringId("CARD_TOOLTIP_TEXT_3")
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
static const std::string KILL_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME = "trap_mask.png";
static const std::string BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME = "board_side_stat_effect.vs";
static const std::string CARD_TOOLTIP_TEXTURE_FILE_NAME = "tooltip.png";
static const std::string CARD_TOOLTIP_SHADER_FILE_NAME = "diagonal_reveal.vs";
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
static const glm::vec3 BOARD_SIDE_EFFECT_TOP_POSITION = { 0.0f, 0.044f, 0.01f};
static const glm::vec3 BOARD_SIDE_EFFECT_BOT_POSITION = { 0.0f, -0.044f, 0.01f};
static const glm::vec3 CARD_TOOLTIP_SCALE = {0.137f, 0.137f, 1/10.0f};
static const glm::vec3 CARD_TOOLTIP_OFFSET = {0.084f, 0.08f, 0.1f};
static const glm::vec3 CARD_TOOLTIP_TEXT_OFFSETS[CARD_TOOLTIP_TEXT_ROWS_COUNT] =
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

#if defined(MOBILE_FLOW)
static const float MOBILE_DISTANCE_FROM_CARD_LOCATION_INDICATOR = 0.003f;
#else
static const float DESKTOP_DISTANCE_FROM_CARD_LOCATION_INDICATOR = 0.003f;
#endif

///------------------------------------------------------------------------------------------------

GameSessionManager::GameSessionManager()
    : mPreviousProspectiveBoardCardsPushState(ProspectiveBoardCardsPushState::NONE)
    , mSecsCardHighlighted(0.0f)
    , mShouldShowCardLocationIndicator(false)
    , mCanPlayNextCard(false)
    , mCanIssueNextTurnInteraction(false)
    , mCanInteractWithAnyHeldCard(true)
{
    
}

///------------------------------------------------------------------------------------------------

GameSessionManager::~GameSessionManager(){}

///------------------------------------------------------------------------------------------------

void GameSessionManager::InitGameSession()
{
    RegisterForEvents();
    
    mBoardState = std::make_unique<BoardState>();
    mBoardState->GetPlayerStates().emplace_back();
    mBoardState->GetPlayerStates().emplace_back();
    
    mBoardState->GetPlayerStates()[game_constants::REMOTE_PLAYER_INDEX].mPlayerDeckCards = CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("insects"));
    mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mPlayerDeckCards = CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("rodents"));
    
    mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mGoldenCardIds = {19, 20, 21, 22};//CardDataRepository::GetInstance().GetCardIdsByFamily(strutils::StringId("rodents"));;
    
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    mPlayerHeldCardSceneObjectWrappers.emplace_back();
    
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    mPlayerBoardCardSceneObjectWrappers.emplace_back();
    
    mRuleEngine = std::make_unique<GameRuleEngine>(mBoardState.get());
    
    
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
    mPlayerActionGenerationEngine = std::make_unique<PlayerActionGenerationEngine>(mRuleEngine.get(), mActionEngine.get());
    
#if defined(REPLAY_FLOW)
    replayEngine.ReplayActions(mActionEngine.get());
#else
    mActionEngine->AddGameAction(strutils::StringId("NextPlayerGameAction"));
#endif
    
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Card Location Indicator
    auto cardLocationIndicatorSo = activeScene->CreateSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
    cardLocationIndicatorSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::CARD_LOCATION_MASK_TEXTURE_NAME);
    cardLocationIndicatorSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::BOARD_CARD_LOCATION_SHADER_NAME);
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_TIME_SPEED_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_TIME_SPEED;
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::PERLIN_RESOLUTION_UNIFORM_NAME] = game_constants::CARD_LOCATION_EFFECT_PERLIN_RESOLUTION;
    cardLocationIndicatorSo->mScale = glm::vec3(game_constants::IN_GAME_CARD_BASE_SCALE * game_constants::IN_GAME_PLAYED_CARD_SCALE_FACTOR);
    cardLocationIndicatorSo->mPosition.z = game_constants::CARD_LOCATION_EFFECT_Z;
    cardLocationIndicatorSo->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    cardLocationIndicatorSo->mInvisible = true;
    
    // Turn pointer
    auto turnPointerSo = activeScene->CreateSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    turnPointerSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + TURN_POINTER_TEXTURE_FILE_NAME);
    turnPointerSo->mPosition = TURN_POINTER_POSITION;
    turnPointerSo->mScale = TURN_POINTER_SCALE;
    turnPointerSo->mSnapToEdgeBehavior = scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE;
    
    // Turn pointer highlighter
    auto tunPointerHighlighterSo = activeScene->CreateSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
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
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::HEALTH_CRYSTAL_TOP_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_TOP_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[0].mPlayerHealth, false, *activeScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::HEALTH_CRYSTAL_BOT_POSITION, HEALTH_CRYSTAL_TEXTURE_FILE_NAME, HEALTH_CRYSTAL_BOT_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[1].mPlayerHealth, false, *activeScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::WEIGHT_CRYSTAL_TOP_POSITION, WEIGHT_CRYSTAL_TEXTURE_FILE_NAME, WEIGHT_CRYSTAL_TOP_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo, false, *activeScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::WEIGHT_CRYSTAL_BOT_POSITION, WEIGHT_CRYSTAL_TEXTURE_FILE_NAME, WEIGHT_CRYSTAL_BOT_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo, false, *activeScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::POISON_STACK_TOP_POSITION, POISON_STACK_TEXTURE_FILE_NAME, POISON_STACK_TOP_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[0].mPlayerPoisonStack, true, *activeScene)));
    mAnimatedStatContainers.emplace_back(std::make_pair(false, std::make_unique<AnimatedStatContainer>(game_constants::POISON_STACK_BOT_POSITION, POISON_STACK_TEXTURE_FILE_NAME, POISON_STACK_BOT_SCENE_OBJECT_NAME_PREFIX, mBoardState->GetPlayerStates()[1].mPlayerPoisonStack, true, *activeScene)));
    
    // Board Side Effect Top
    auto boardSideEffectTopSceneObject = activeScene->CreateSceneObject(game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME);
    boardSideEffectTopSceneObject->mScale = BOARD_SIDE_EFFECT_SCALE;
    boardSideEffectTopSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOARD_SIDE_EFFECT_REDUCTION_TEXTURE_FILE_NAME);
    boardSideEffectTopSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOARD_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME);
    boardSideEffectTopSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
    boardSideEffectTopSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    boardSideEffectTopSceneObject->mPosition = BOARD_SIDE_EFFECT_TOP_POSITION;
    boardSideEffectTopSceneObject->mInvisible = true;
    
    // Board Side Effect Bot
    auto boardSideEffectBotSceneObject = activeScene->CreateSceneObject(game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
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
            auto boardSideEffectTopValueSceneObject = activeScene->CreateSceneObject(strutils::StringId(game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX + std::to_string(i)));
            
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
            auto boardSideEffectBotValueSceneObject = activeScene->CreateSceneObject(strutils::StringId(game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX + std::to_string(i)));
            
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
    
    // Kill Side Effect Top
    auto killSideEffectTopSceneObject = activeScene->CreateSceneObject(game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME);
    killSideEffectTopSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + KILL_SIDE_EFFECT_TEXTURE_FILE_NAME);
    killSideEffectTopSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    killSideEffectTopSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + KILL_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME);
    killSideEffectTopSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
    killSideEffectTopSceneObject->mPosition = BOARD_SIDE_EFFECT_TOP_POSITION;
    killSideEffectTopSceneObject->mPosition.z += 0.1f;
    killSideEffectTopSceneObject->mScale = game_constants::KILL_SIDE_EFFECT_SCALE;
    killSideEffectTopSceneObject->mInvisible = true;
    animationManager.StartAnimation(std::make_unique<rendering::ContinuousPulseAnimation>(killSideEffectTopSceneObject, game_constants::KILL_SIDE_EFFECT_SCALE_UP_FACTOR, game_constants::KILL_SIDE_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS), [](){});
    
    // Kill Side Effect Bot
    auto killSideEffectBotSceneObject = activeScene->CreateSceneObject(game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    killSideEffectBotSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + KILL_SIDE_EFFECT_TEXTURE_FILE_NAME);
    killSideEffectBotSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    killSideEffectBotSceneObject->mEffectTextureResourceIds[0] = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + KILL_SIDE_EFFECT_MASK_TEXTURE_FILE_NAME);
    killSideEffectBotSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + BOARD_SIDE_STAT_EFFECT_SHADER_FILE_NAME);
    killSideEffectBotSceneObject->mPosition = BOARD_SIDE_EFFECT_BOT_POSITION;
    killSideEffectBotSceneObject->mPosition.z += 0.1f;
    killSideEffectBotSceneObject->mScale = game_constants::KILL_SIDE_EFFECT_SCALE;
    killSideEffectBotSceneObject->mInvisible = true;
    animationManager.StartAnimation(std::make_unique<rendering::ContinuousPulseAnimation>(killSideEffectBotSceneObject, game_constants::KILL_SIDE_EFFECT_SCALE_UP_FACTOR, game_constants::KILL_SIDE_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS), [](){});
    
    // Card Tooltips
    auto tooltipSceneObject = activeScene->CreateSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    tooltipSceneObject->mScale = CARD_TOOLTIP_SCALE;
    tooltipSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_TOOLTIP_TEXTURE_FILE_NAME);
    tooltipSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_TOOLTIP_SHADER_FILE_NAME);
    tooltipSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    tooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = 0.0f;
    tooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_RGB_EXPONENT_UNIFORM_NAME] = CARD_TOOLTIP_REVEAL_RGB_EXPONENT;
    tooltipSceneObject->mInvisible = true;
    
    for (auto i = 0; i < CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
    {
        auto tooltipTextSceneObject = activeScene->CreateSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
        scene::TextSceneObjectData tooltipTextData;
        tooltipTextData.mFontName = game_constants::DEFAULT_FONT_BLACK_NAME;
        tooltipTextSceneObject->mSceneObjectTypeData = std::move(tooltipTextData);
        tooltipTextSceneObject->mScale = glm::vec3(CARD_TOOLTIP_TEXT_FONT_SIZE);
        tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        tooltipTextSceneObject->mInvisible = true;
    }
    
    OnWindowResize({});
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::Update(const float dtMillis)
{
    if (mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME)
    {
        mCanInteractWithAnyHeldCard = true;
        mCanPlayNextCard = true;
        if (!mPendingCardsToBePlayed.empty())
        {
            mPendingCardsToBePlayed.erase(mPendingCardsToBePlayed.begin());
        }
    }
    
    if (mActionEngine->GetActiveGameActionName() == IDLE_GAME_ACTION_NAME &&
        mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX)
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
}

///------------------------------------------------------------------------------------------------

const BoardState& GameSessionManager::GetBoardState() const
{
    return *mBoardState;
}

///------------------------------------------------------------------------------------------------

GameActionEngine& GameSessionManager::GetActionEngine()
{
    return *mActionEngine;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GameSessionManager::GetHeldCardSoWrappers() const
{
    return mPlayerHeldCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::vector<std::shared_ptr<CardSoWrapper>>>& GameSessionManager::GetBoardCardSoWrappers() const
{
    return mPlayerBoardCardSceneObjectWrappers;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::HandleTouchInput(const float dtMillis)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(activeScene->GetCamera().GetViewMatrix(), activeScene->GetCamera().GetProjMatrix());
    
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
            if (mSecsCardHighlighted > CARD_TOOLTIP_CREATION_DELAY_SECS && activeScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME)->mInvisible)
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
                auto currentLocalPlayerBoardCardCount = static_cast<int>(mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX].size());
                auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
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
            auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false, activeScene->GetCamera());
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
                    auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false, activeScene->GetCamera());
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
                auto currentLocalPlayerBoardCardCount = static_cast<int>(mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX].size());
                auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
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
                 activeScene->FindSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i))) != nullptr)
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
                        auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, localPlayerCardCount, false, activeScene->GetCamera());
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
        
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(candidateHighlightIndices.front(), localPlayerCardCount, false, activeScene->GetCamera());
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
        auto turnPointerSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
        auto turnPointerHighlighterSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
        
        auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*turnPointerSo);
        bool cursorInSceneObject = math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos);
        
        if (cursorInSceneObject && inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON) && mCanIssueNextTurnInteraction)
        {
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

void GameSessionManager::UpdateMiscSceneObjects(const float dtMillis)
{
    static float time = 0.0f;
    time += dtMillis * 0.001f;
    
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Card Interactive Elements
    auto& localPlayerHeldCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    auto& localPlayerBoardCards = mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    auto& remotePlayerBoardCards = mPlayerBoardCardSceneObjectWrappers[game_constants::REMOTE_PLAYER_INDEX];
    
    for (size_t i = 0; i < localPlayerHeldCards.size(); ++i)
    {
        auto& cardSoWrapper = localPlayerHeldCards[i];
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
        
        if (mActionEngine->GetActiveGameActionName() != CARD_EFFECT_GAME_ACTION_NAME)
        {
            auto canCardBePlayed = mRuleEngine->CanCardBePlayed(cardSoWrapper->mCardData, i, game_constants::LOCAL_PLAYER_INDEX);
            cardSoWrapper->mSceneObject->mShaderIntUniformValues[game_constants::CARD_WEIGHT_INTERACTIVE_MODE_UNIFORM_NAME] = canCardBePlayed ? game_constants::CARD_INTERACTIVE_MODE_DEFAULT : game_constants::CARD_INTERACTIVE_MODE_NONINTERACTIVE;
            
            const auto& heldCardStatOverrides = mBoardState->GetPlayerStates()[game_constants::LOCAL_PLAYER_INDEX].mPlayerHeldCardStatOverrides;
            if (heldCardStatOverrides.size() > i)
            {
                int overriddenWeight = math::Max(0, heldCardStatOverrides[i].count(CardStatType::WEIGHT) ? heldCardStatOverrides[i].at(CardStatType::WEIGHT) : cardSoWrapper->mCardData->mCardWeight);
                if (canCardBePlayed && overriddenWeight < cardSoWrapper->mCardData->mCardWeight)
                {
                    cardSoWrapper->mSceneObject->mShaderIntUniformValues[game_constants::CARD_WEIGHT_INTERACTIVE_MODE_UNIFORM_NAME] = game_constants::CARD_INTERACTIVE_MODE_INTERACTIVE;
                }
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
        auto cardHighlighterObject = activeScene->FindSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
        if (cardHighlighterObject)
        {
            cardHighlighterObject->mInvisible = false;
            cardHighlighterObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = time;
            cardHighlighterObject->mPosition = localPlayerHeldCards[i]->mSceneObject->mPosition;
            cardHighlighterObject->mPosition.z += game_constants::ACTION_HIGLIGHTER_Z_OFFSET;
        }
    }
    
    // Turn pointer highlighter
    auto turnPointerSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_SCENE_OBJECT_NAME);
    auto turnPointerHighlighterSo = activeScene->FindSceneObject(game_constants::TURN_POINTER_HIGHLIGHTER_SCENE_OBJECT_NAME);
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
    auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
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
                prospectiveMakeSpaceRevertToPositionLambda(static_cast<int>(mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX].size() + 1));
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
                prospectiveMakeSpaceRevertToPositionLambda(static_cast<int>(mPlayerBoardCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX].size()));
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
    auto boardSideEffectTopSceneObject = activeScene->FindSceneObject(game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME);
    boardSideEffectTopSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = fmod(time/10, 1.0f);
    
    auto boardSideEffectBotSceneObject = activeScene->FindSceneObject(game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    boardSideEffectBotSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = fmod(time/10, 1.0f);
    
    auto killEffectTopSceneObject = activeScene->FindSceneObject(game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME);
    killEffectTopSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = 0.0f;
    
    auto killEffectBotSceneObject = activeScene->FindSceneObject(game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    killEffectBotSceneObject->mShaderFloatUniformValues[game_constants::TIME_UNIFORM_NAME] = 0.0f;
    
    // Card tooltip
    auto cardTooltipSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    
    cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_TOOLTIP_REVEAL_SPEED;
    if (cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] >= CARD_TOOLTIP_MAX_REVEAL_THRESHOLD)
    {
        cardTooltipSceneObject->mShaderFloatUniformValues[CARD_TOOLTIP_REVEAL_THRESHOLD_UNIFORM_NAME] = CARD_TOOLTIP_MAX_REVEAL_THRESHOLD;
        
        for (auto i = 0; i < CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
        {
            auto tooltipTextSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
            tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Min(1.0f, tooltipTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] +  dtMillis * CARD_TOOLTIP_TEXT_REVEAL_SPEED);
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnFreeMovingCardRelease(std::shared_ptr<CardSoWrapper> cardSoWrapper)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    auto cardIndex = std::find_if(localPlayerCards.begin(), localPlayerCards.end(), [=](const std::shared_ptr<CardSoWrapper>& otherCard)
    {
        return otherCard.get() == cardSoWrapper.get();
    }) - localPlayerCards.begin();
    
    DestroyCardHighlighterAtIndex(static_cast<int>(cardIndex));
    
    auto cardLocationIndicatorSo = activeScene->FindSceneObject(CARD_LOCATION_INDICATOR_SCENE_OBJECT_NAME);
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
        auto originalCardPosition = card_utils::CalculateHeldCardPosition(static_cast<int>(cardIndex), static_cast<int>(localPlayerCards.size()), false, activeScene->GetCamera());
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, originalCardPosition, cardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){cardSoWrapper->mState = CardSoState::IDLE; });
        cardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
    }

}

///------------------------------------------------------------------------------------------------

void GameSessionManager::CreateCardHighlighter()
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto& localPlayerCards = mPlayerHeldCardSceneObjectWrappers[game_constants::LOCAL_PLAYER_INDEX];
    for (size_t i = 0U; i < localPlayerCards.size(); ++i)
    {
        activeScene->RemoveSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
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
        auto cardHighlighterSo = activeScene->CreateSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(cardIndex)));
        
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

void GameSessionManager::CreateCardTooltip(const glm::vec3& cardOriginPostion, const std::string& tooltipText, const size_t cardIndex)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    const auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto tooltipSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    
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
        auto tooltipTextSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[1]);
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
            assert(i < CARD_TOOLTIP_TEXT_ROWS_COUNT);
            auto tooltipTextSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
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

void GameSessionManager::DestroyCardHighlighterAtIndex(const int index)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    auto cardHighlighterName = strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(index));
    activeScene->RemoveSceneObject(cardHighlighterName);
    
    DestroyCardTooltip();
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::DestroyCardTooltip()
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto tooltipSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_SCENE_OBJECT_NAME);
    tooltipSceneObject->mInvisible = true;
    
    for (auto i = 0; i < CARD_TOOLTIP_TEXT_ROWS_COUNT; ++i)
    {
        auto tooltipTextSceneObject = activeScene->FindSceneObject(CARD_TOOLTIP_TEXT_SCENE_OBJECT_NAMES[i]);
        tooltipTextSceneObject->mInvisible = true;
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::RegisterForEvents()
{
    auto& eventSystem = events::EventSystem::GetInstance();
    
    eventSystem.RegisterForEvent<events::ApplicationMovedToBackgroundEvent>(this, &GameSessionManager::OnApplicationMovedToBackground);
    eventSystem.RegisterForEvent<events::WindowResizeEvent>(this, &GameSessionManager::OnWindowResize);
    eventSystem.RegisterForEvent<events::LocalPlayerTurnStarted>(this, &GameSessionManager::OnLocalPlayerTurnStarted);
    eventSystem.RegisterForEvent<events::CardDestructionEvent>(this, &GameSessionManager::OnCardDestruction);
    eventSystem.RegisterForEvent<events::CardDestructionWithRepositionEvent>(this, &GameSessionManager::OnCardDestructionWithReposition);
    eventSystem.RegisterForEvent<events::CardCreationEvent>(this, &GameSessionManager::OnCardCreation);
    eventSystem.RegisterForEvent<events::CardBuffedDebuffedEvent>(this, &GameSessionManager::OnCardBuffedDebuffed);
    eventSystem.RegisterForEvent<events::HeldCardSwapEvent>(this, &GameSessionManager::OnHeldCardSwap);
    eventSystem.RegisterForEvent<events::LastCardPlayedFinalizedEvent>(this, &GameSessionManager::OnLastCardPlayedFinalized);
    eventSystem.RegisterForEvent<events::HealthChangeAnimationTriggerEvent>(this, &GameSessionManager::OnHealthChangeAnimationTriggerEvent);
    eventSystem.RegisterForEvent<events::WeightChangeAnimationTriggerEvent>(this, &GameSessionManager::OnWeightChangeAnimationTriggerEvent);
    eventSystem.RegisterForEvent<events::BoardSideCardEffectTriggeredEvent>(this, &GameSessionManager::OnBoardSideCardEffectTriggeredEvent);
    eventSystem.RegisterForEvent<events::BoardSideCardEffectEndedEvent>(this, &GameSessionManager::OnBoardSideCardEffectEndedEvent);
    eventSystem.RegisterForEvent<events::ForceSendCardBackToPositionEvent>(this, &GameSessionManager::OnForceSendCardBackToPositionEvent);
    eventSystem.RegisterForEvent<events::PoisonStackChangeChangeAnimationTriggerEvent>(this, &GameSessionManager::OnPoisonStackChangeChangeAnimationTriggerEvent);
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnApplicationMovedToBackground(const events::ApplicationMovedToBackgroundEvent&)
{
    mGameSerializer->FlushStateToFile();
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnWindowResize(const events::WindowResizeEvent&)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    // Correct position of held cards
    for (auto j = 0U; j < mPlayerHeldCardSceneObjectWrappers.size(); ++j)
    {
        for (auto i = 0U; i < mPlayerHeldCardSceneObjectWrappers[j].size(); ++i)
        {
            auto cardSoWrapper = mPlayerHeldCardSceneObjectWrappers[j][i];
            if (cardSoWrapper->mState == CardSoState::IDLE)
            {
                cardSoWrapper->mSceneObject->mPosition = card_utils::CalculateHeldCardPosition(i, static_cast<int>(mPlayerHeldCardSceneObjectWrappers[j].size()), j == game_constants::REMOTE_PLAYER_INDEX, activeScene->GetCamera());
            }
        }
    }
    
    // Correct position of other snap to edge scene objects
    activeScene->RecalculatePositionOfEdgeSnappingSceneObjects();
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnLocalPlayerTurnStarted(const events::LocalPlayerTurnStarted&)
{
    mCanIssueNextTurnInteraction = true;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnCardDestruction(const events::CardDestructionEvent& event)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
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
            activeScene->RemoveSceneObject(cardSoWrappers[i]->mSceneObject->mName);
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

void GameSessionManager::OnCardDestructionWithReposition(const events::CardDestructionWithRepositionEvent& event)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto& cardSoWrappers = event.mIsBoardCard ?
        mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)]:
        mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
    activeScene->RemoveSceneObject(cardSoWrappers[event.mCardIndex]->mSceneObject->mName);
    
    cardSoWrappers.erase(cardSoWrappers.begin() + event.mCardIndex);
    
    // Animate rest of the cards to position.
    const auto currentCardCount = static_cast<int>(cardSoWrappers.size());
    for (int i = 0; i < currentCardCount; ++i)
    {
        auto& currentCardSoWrapper = cardSoWrappers.at(i);
    
        auto originalCardPosition = event.mIsBoardCard ?
            card_utils::CalculateBoardCardPosition(i, currentCardCount, event.mForRemotePlayer) :
            card_utils::CalculateHeldCardPosition(i, currentCardCount, event.mForRemotePlayer, activeScene->GetCamera());
        
        animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnCardCreation(const events::CardCreationEvent& event)
{
    mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)].push_back(event.mCardSoWrapper);
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnCardBuffedDebuffed(const events::CardBuffedDebuffedEvent& event)
{
    auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    if (event.mBoardCard)
    {
        auto& boardSceneObjectWrappers = mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
        auto& cardSceneObjectWrapper = boardSceneObjectWrappers[event.mCardIndex];
        auto previousScale = cardSceneObjectWrapper->mSceneObject->mScale;
        
        activeScene->RemoveSceneObject(cardSceneObjectWrapper->mSceneObject->mName);
        
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
            *activeScene
        );
        boardSceneObjectWrappers[event.mCardIndex]->mSceneObject->mScale = previousScale;
    }
    else
    {
        auto& heldSceneObjectWrappers = mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
        auto& cardSceneObjectWrapper = heldSceneObjectWrappers[event.mCardIndex];
        auto previousScale = cardSceneObjectWrapper->mSceneObject->mScale;
        
        activeScene->RemoveSceneObject(cardSceneObjectWrapper->mSceneObject->mName);
        
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
            *activeScene
        );
        heldSceneObjectWrappers[event.mCardIndex]->mSceneObject->mScale = previousScale;
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnHeldCardSwap(const events::HeldCardSwapEvent& event)
{
    mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)][event.mCardIndex] = event.mCardSoWrapper;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnLastCardPlayedFinalized(const events::LastCardPlayedFinalizedEvent& event)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    activeScene->RemoveSceneObject(strutils::StringId(CARD_HIGHLIGHTER_SCENE_OBJECT_NAME_PREFIX + std::to_string(event.mCardIndex)));
    
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
            auto originalCardPosition = card_utils::CalculateHeldCardPosition(i, currentPlayerHeldCardCount, mBoardState->GetActivePlayerIndex() == 0, activeScene->GetCamera());
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){ currentCardSoWrapper->mState = CardSoState::IDLE; });
            currentCardSoWrapper->mState = CardSoState::MOVING_TO_SET_POSITION;
        }
    }
    
    const auto currentBoardCardCount = static_cast<int>(playerBoardCardSoWrappers.size());
    
    // Animate and rename board cards to position. Last one will be animated externally
    for (int i = 0; i < currentBoardCardCount; ++i)
    {
        auto& currentCardSoWrapper = playerBoardCardSoWrappers.at(i);
        currentCardSoWrapper->mSceneObject->mName = strutils::StringId((mBoardState->GetActivePlayerIndex() == game_constants::REMOTE_PLAYER_INDEX ? game_constants::TOP_PLAYER_BOARD_CARD_SO_NAME_PREFIX : game_constants::BOT_PLAYER_BOARD_CARD_SO_NAME_PREFIX) + std::to_string(i));
        
        if (i !=  currentBoardCardCount - 1)
        {
            auto originalCardPosition = card_utils::CalculateBoardCardPosition(i, currentBoardCardCount, mBoardState->GetActivePlayerIndex() == 0);
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(currentCardSoWrapper->mSceneObject, originalCardPosition, currentCardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnHealthChangeAnimationTriggerEvent(const events::HealthChangeAnimationTriggerEvent& event)
{
    mAnimatedStatContainers[event.mForRemotePlayer ? 0 : 1].first = true;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnWeightChangeAnimationTriggerEvent(const events::WeightChangeAnimationTriggerEvent& event)
{
    mAnimatedStatContainers[event.mForRemotePlayer ? 2 : 3].first = true;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnBoardSideCardEffectTriggeredEvent(const events::BoardSideCardEffectTriggeredEvent& event)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    const auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    std::shared_ptr<scene::SceneObject> sideEffectSceneObject = nullptr;
    float maxAlpha = 0.0f;
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER)
    {
        sideEffectSceneObject = activeScene->FindSceneObject(event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
        
        maxAlpha = 0.25f;
    }
    else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::KILL_NEXT)
    {
        sideEffectSceneObject = activeScene->FindSceneObject(event.mForRemotePlayer ? game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
        sideEffectSceneObject->mScale = game_constants::KILL_SIDE_EFFECT_SCALE;
        sideEffectSceneObject->mRotation = glm::vec3(0.0f);
        animationManager.StartAnimation(std::make_unique<rendering::ContinuousPulseAnimation>(sideEffectSceneObject, game_constants::KILL_SIDE_EFFECT_SCALE_UP_FACTOR, game_constants::KILL_SIDE_EFFECT_PULSE_ANIMATION_PULSE_DUARTION_SECS), [](){});
        maxAlpha = 0.25f;
    }
    
    sideEffectSceneObject->mInvisible = false;
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sideEffectSceneObject, maxAlpha, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
    
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER)
    {
        for (int i = 0; i < game_constants::BOARD_SIDE_EFFECT_VALUE_SO_COUNT; ++i)
        {
            auto boardSideEffectValueSceneObject = activeScene->FindSceneObject(strutils::StringId((event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX) + std::to_string(i)));
            boardSideEffectValueSceneObject->mInvisible = false;
            
            std::get<scene::TextSceneObjectData>(boardSideEffectValueSceneObject->mSceneObjectTypeData).mText = std::to_string(mBoardState->GetPlayerStates()[event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX].mBoardModifiers.mGlobalCardStatModifiers.at(CardStatType::DAMAGE));
            
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(boardSideEffectValueSceneObject, maxAlpha * 2, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnBoardSideCardEffectEndedEvent(const events::BoardSideCardEffectEndedEvent& event)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& animationManager = systemsEngine.GetAnimationManager();
    const auto& activeSceneManager = systemsEngine.GetActiveSceneManager();
    
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    
    std::shared_ptr<scene::SceneObject> sideEffectSceneObject = nullptr;
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER)
    {
        sideEffectSceneObject = activeScene->FindSceneObject(event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    }
    else if (event.mEffectBoardModifierMask == effects::board_modifier_masks::KILL_NEXT)
    {
        sideEffectSceneObject = activeScene->FindSceneObject(event.mForRemotePlayer ? game_constants::KILL_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME : game_constants::KILL_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME);
    }
    animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sideEffectSceneObject, 0.0f, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
    {
        sideEffectSceneObject->mInvisible = true;
    });
    
    if (event.mEffectBoardModifierMask == effects::board_modifier_masks::BOARD_SIDE_STAT_MODIFIER)
    {
        for (int i = 0; i < game_constants::BOARD_SIDE_EFFECT_VALUE_SO_COUNT; ++i)
        {
            auto boardSideEffectValueSceneObject = activeScene->FindSceneObject(strutils::StringId((event.mForRemotePlayer ? game_constants::BOARD_SIDE_EFFECT_TOP_SCENE_OBJECT_NAME_PRE_FIX : game_constants::BOARD_SIDE_EFFECT_BOT_SCENE_OBJECT_NAME_PRE_FIX) + std::to_string(i)));
            animationManager.StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(boardSideEffectValueSceneObject, 0.0f, BOARD_SIDE_EFFECT_SHOWING_HIDING_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [=]()
            {
                boardSideEffectValueSceneObject->mInvisible = true;
            });
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnForceSendCardBackToPositionEvent(const events::ForceSendCardBackToPositionEvent& event)
{
    const auto& activeSceneManager = CoreSystemsEngine::GetInstance().GetActiveSceneManager();
    auto activeScene = activeSceneManager.FindScene(game_constants::IN_GAME_BATTLE_SCENE);
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    
    auto& cardSoWrappers = event.mBoardCard ?
        mPlayerBoardCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)]:
        mPlayerHeldCardSceneObjectWrappers[(event.mForRemotePlayer ? game_constants::REMOTE_PLAYER_INDEX : game_constants::LOCAL_PLAYER_INDEX)];
    auto currentCardCount = static_cast<int>(cardSoWrappers.size());
    auto& cardSoWrapper = cardSoWrappers[event.mCardIdex];
    
    cardSoWrapper->mState = CardSoState::IDLE;
    
    auto originalCardPosition = event.mBoardCard ?
        card_utils::CalculateBoardCardPosition(event.mCardIdex, currentCardCount, event.mForRemotePlayer) :
        card_utils::CalculateHeldCardPosition(event.mCardIdex, currentCardCount, event.mForRemotePlayer, activeScene->GetCamera());
    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(cardSoWrapper->mSceneObject, originalCardPosition, cardSoWrapper->mSceneObject->mScale, CARD_SELECTION_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=](){});
    
    DestroyCardHighlighterAtIndex(event.mCardIdex);
    
    mCanInteractWithAnyHeldCard = false;
}

///------------------------------------------------------------------------------------------------

void GameSessionManager::OnPoisonStackChangeChangeAnimationTriggerEvent(const events::PoisonStackChangeChangeAnimationTriggerEvent& event)
{
    auto& affectedContainerEntry = mAnimatedStatContainers[event.mForRemotePlayer ? 4 : 5];
    affectedContainerEntry.first = true;
    
    for (auto& sceneObject: affectedContainerEntry.second->GetSceneObjects())
    {
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, event.mNewPoisonStackValue == 0 ? 0.0f : 1.0f, game_constants::POISON_STACK_SHOW_HIDE_ANIMATION_DURATION_SECS, animation_flags::NONE, 0.0f, math::LinearFunction, math::TweeningMode::EASE_IN), [](){});
    }
}

///------------------------------------------------------------------------------------------------
