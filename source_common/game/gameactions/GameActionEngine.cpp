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

GameActionEngine::GameActionEngine(const EngineOperationMode operationMode, const int gameSeed)
    : mOperationMode(operationMode)
    , mGameSeed(gameSeed)
    , mActiveActionHasSetState(false)
    , mLoggingActionTransitions(true)
{
    math::SetControlSeed(mGameSeed);
    
    GameActionFactory::RegisterGameActions();
    
    mBoardState.GetPlayerStates().emplace_back();
    mBoardState.GetPlayerStates().emplace_back();
    mBoardState.GetActivePlayerIndex() = 1;
    
    CreateAndPushGameAction(IDLE_GAME_ACTION_NAME);
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
                CreateAndPushGameAction(IDLE_GAME_ACTION_NAME);
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
                CreateAndPushGameAction(IDLE_GAME_ACTION_NAME);
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::AddGameAction(const strutils::StringId& actionName)
{
    if (GetActiveGameActionName() == IDLE_GAME_ACTION_NAME)
    {
        mGameActions.pop();
    }
    
    CreateAndPushGameAction(actionName);
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::SetLoggingActionTransitions(const bool logActionTransitions)
{
    mLoggingActionTransitions = logActionTransitions;
}

///------------------------------------------------------------------------------------------------

const BoardState& GameActionEngine::GetBoardState() const
{
    return mBoardState;
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

void GameActionEngine::CreateAndPushGameAction(const strutils::StringId& actionName)
{
    auto action = GameActionFactory::CreateGameAction(actionName);
    action->SetName(actionName);
    action->SetBoardState(&mBoardState);
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
