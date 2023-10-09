///------------------------------------------------------------------------------------------------
///  GameActionEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameActionFactory.h>
#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");

///------------------------------------------------------------------------------------------------

GameActionEngine::GameActionEngine(const EngineOperationMode operationMode)
    : mOperationMode(operationMode)
    , mActiveActionHasSetState(false)
    , mLoggingActionTransitions(true)
{
    GameActionFactory::RegisterGameActions();
    
    mBoardState.GetPlayerStates().emplace_back();
    mBoardState.GetPlayerStates().back().mPlayerHeldCards = {1,2,3,4,5};
    mBoardState.GetPlayerStates().emplace_back();
    mBoardState.GetPlayerStates().back().mPlayerHeldCards = {1,2,3,4,5};
    
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
                LogActionTransition("Setting state of action " + mGameActions.front()->VGetName().GetString());
                mGameActions.front()->VSetNewGameState();
                mActiveActionHasSetState = true;
            }
            
            if (mGameActions.front()->VUpdateAnimation(dtMillis) == ActionAnimationUpdateResult::FINISHED)
            {
                LogActionTransition("Removing post finished animation action " + mGameActions.front()->VGetName().GetString());
                mGameActions.pop();
            }
            
            if (mGameActions.empty())
            {
                CreateAndPushGameAction(IDLE_GAME_ACTION_NAME);
            }
            else
            {
                mGameActions.front()->VInitAnimation();
                LogActionTransition("Initialized animation of action " + mGameActions.front()->VGetName().GetString());
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

void GameActionEngine::AddGameAction(const strutils::StringId& actionName)
{
    bool firstNonIdleAction = false;
    if (GetActiveGameActionName() == IDLE_GAME_ACTION_NAME)
    {
        mGameActions.pop();
        firstNonIdleAction = true;
    }
    
    CreateAndPushGameAction(actionName);
    if (firstNonIdleAction)
    {
        mGameActions.front()->VInitAnimation();
        LogActionTransition("Initialized animation of action " + actionName.GetString());
    }
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
