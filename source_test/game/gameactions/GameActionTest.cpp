///------------------------------------------------------------------------------------------------
///  GameActionTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 29/09/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <engine/utils/Logging.h>
#include <game/BoardState.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayCardGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");
static const strutils::StringId PLAY_CARD_GAME_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("NextPlayerGameAction");

///------------------------------------------------------------------------------------------------

TEST(GameActionTests, TestIdleGameActionExistsByDefault)
{
    BoardState boardState;
    boardState.GetPlayerStates().emplace_back();
    boardState.GetPlayerStates().emplace_back();
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS, 0, &boardState, nullptr);
    
    EXPECT_EQ(engine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestPushedGameActionIsActive)
{
    BoardState boardState;
    boardState.GetPlayerStates().emplace_back();
    boardState.GetPlayerStates().emplace_back();
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS, 0, &boardState, nullptr);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    
    EXPECT_EQ(engine.GetActiveGameActionName(), DRAW_CARD_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestBoardStatePostDrawAction)
{
    BoardState boardState;
    boardState.GetPlayerStates().emplace_back();
    boardState.GetPlayerStates().emplace_back();
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS, 0, &boardState, nullptr);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.Update(0);
    
    EXPECT_EQ(boardState.GetActivePlayerState().mPlayerHeldCards.size(), 1);
    EXPECT_EQ(engine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestBoardStatePostDrawAndPlayAction)
{
    BoardState boardState;
    boardState.GetPlayerStates().emplace_back();
    boardState.GetPlayerStates().emplace_back();
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS, 0, &boardState, nullptr);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    engine.Update(0);
    engine.Update(0);
    
    EXPECT_EQ(boardState.GetActivePlayerState().mPlayerHeldCards.size(), 0);
    EXPECT_EQ(boardState.GetActivePlayerState().mPlayerBoardCards.size(), 1);
    EXPECT_EQ(engine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestDrawPlayNextDrawPlayActionRound)
{
    BoardState boardState;
    boardState.GetPlayerStates().emplace_back();
    boardState.GetPlayerStates().emplace_back();
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS, 0, &boardState, nullptr);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    engine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    engine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (engine.GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        engine.Update(0);
    }
    
    for (size_t i = 0; i < boardState.GetPlayerCount(); ++i)
    {
        EXPECT_EQ(boardState.GetPlayerStates().at(i).mPlayerHeldCards.size(), 0);
        EXPECT_EQ(boardState.GetPlayerStates().at(i).mPlayerBoardCards.size(), 1);
    }
    
    EXPECT_EQ(boardState.GetActivePlayerIndex(), 0);
}
