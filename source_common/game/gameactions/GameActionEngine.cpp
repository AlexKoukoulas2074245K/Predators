///------------------------------------------------------------------------------------------------
///  GameActionEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameActionFactory.h>
#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");

///------------------------------------------------------------------------------------------------

GameActionEngine::GameActionEngine(const EngineOperationMode operationMode, const int gameSeed, BoardState* boardState, GameSessionManager* gameSessionManager)
    : mOperationMode(operationMode)
    , mGameSeed(gameSeed)
    , mBoardState(boardState)
    , mGameSessionManager(gameSessionManager)
    , mActiveActionHasSetState(false)
    , mLoggingActionTransitions(true)
{
    math::SetControlSeed(mGameSeed);
    
    GameActionFactory::RegisterGameActions();
    
    CreateAndPushGameAction(IDLE_GAME_ACTION_NAME, {});
}

///------------------------------------------------------------------------------------------------

GameActionEngine::~GameActionEngine()
{    
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::Update(const float dtMillis)
{
    if (mOperationMode == EngineOperationMode::HEADLESS)
    {
        if (GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
        {
            mGameActions.front()->VSetNewGameState();
            mGameActions.pop();
            
            if (mGameActions.empty())
            {
                CreateAndPushGameAction(IDLE_GAME_ACTION_NAME, {});
            }
        }
    }
    else if (mOperationMode == EngineOperationMode::ANIMATED)
    {
        if (GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
        {
            if (!mActiveActionHasSetState)
            {
                LogActionTransition("Setting state and initializing animation of action " + mGameActions.front()->VGetName().GetString());
                mGameActions.front()->VSetNewGameState();
                mGameActions.front()->VInitAnimation();
                mActiveActionHasSetState = true;
            }
            
            if (mGameActions.front()->VUpdateAnimation(dtMillis) == ActionAnimationUpdateResult::FINISHED)
            {
                LogActionTransition("Removing post finished animation action " + mGameActions.front()->VGetName().GetString());
                mGameActions.pop();
                mActiveActionHasSetState = false;
            }
            
            if (mGameActions.empty())
            {
                CreateAndPushGameAction(IDLE_GAME_ACTION_NAME, {});
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::AddGameAction(const strutils::StringId& actionName, const std::unordered_map<std::string, std::string> extraActionParams /* = {} */)
{
    if (GetActiveGameActionName() == IDLE_GAME_ACTION_NAME)
    {
        mGameActions.pop();
    }
    
    CreateAndPushGameAction(actionName, extraActionParams);
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::SetLoggingActionTransitions(const bool logActionTransitions)
{
    mLoggingActionTransitions = logActionTransitions;
}

///------------------------------------------------------------------------------------------------

const strutils::StringId& GameActionEngine::GetActiveGameActionName() const
{
    return mGameActions.front()->VGetName();
}

///------------------------------------------------------------------------------------------------

bool GameActionEngine::LoggingActionTransitions() const
{
    return mLoggingActionTransitions;
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::CreateAndPushGameAction(const strutils::StringId& actionName, const ExtraActionParams& extraActionParams)
{
    auto action = GameActionFactory::CreateGameAction(actionName);
    action->SetName(actionName);
    action->SetDependencies(mBoardState, mGameSessionManager);
    action->SetExtraActionParams(extraActionParams);
    mGameActions.push(std::move(action));
    
    LogActionTransition("Pushed action " + actionName.GetString());
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::LogActionTransition(const std::string& actionTransition)
{
    if (mLoggingActionTransitions)
    {
        logging::Log(logging::LogType::INFO, "%s", actionTransition.c_str());
    }
}

///------------------------------------------------------------------------------------------------
