///------------------------------------------------------------------------------------------------
///  GameActionEngine.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameActionFactory.h>
#include <game/gameactions/BaseGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");

///------------------------------------------------------------------------------------------------

GameActionEngine::GameActionEngine(const EngineOperationMode operationMode)
    : mOperationMode(operationMode)
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

void GameActionEngine::Update(const float)
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

void GameActionEngine::CreateAndPushGameAction(const strutils::StringId& actionName)
{
    auto action = GameActionFactory::CreateGameAction(actionName);
    action->SetName(actionName);
    action->SetBoardState(&mBoardState);
    mGameActions.push(std::move(action));
}

///------------------------------------------------------------------------------------------------
