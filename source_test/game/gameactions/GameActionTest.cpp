///------------------------------------------------------------------------------------------------
///  GameActionTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 29/09/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <engine/utils/Logging.h>
#include <game/gameactions/GameActionEngine.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");
static const strutils::StringId PLAY_CARD_GAME_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("NextPlayerGameAction");

///------------------------------------------------------------------------------------------------

TEST(GameActionTests, TestIdleGameActionExistsByDefault)
{
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS);
    
    EXPECT_EQ(engine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestPushedGameActionIsActive)
{
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    
    EXPECT_EQ(engine.GetActiveGameActionName(), DRAW_CARD_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestBoardStatePostDrawAction)
{
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.Update(0);
    
    EXPECT_EQ(engine.GetBoardState().GetActivePlayerState().mPlayerHeldCards.size(), 6);
    EXPECT_EQ(engine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestBoardStatePostDrawAndPlayAction)
{
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME);
    engine.Update(0);
    engine.Update(0);
    
    EXPECT_EQ(engine.GetBoardState().GetActivePlayerState().mPlayerHeldCards.size(), 5);
    EXPECT_EQ(engine.GetBoardState().GetActivePlayerState().mPlayerBoardCards.size(), 1);
    EXPECT_EQ(engine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST(GameActionTests, TestDrawPlayNextDrawPlayActionRound)
{
    GameActionEngine engine(GameActionEngine::EngineOperationMode::HEADLESS);
    
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    engine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME);
    engine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (engine.GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        engine.Update(0);
    }
    
    for (size_t i = 0; i < engine.GetBoardState().GetPlayerCount(); ++i)
    {
        EXPECT_EQ(engine.GetBoardState().GetPlayerStates().at(i).mPlayerHeldCards.size(), 5);
        EXPECT_EQ(engine.GetBoardState().GetPlayerStates().at(i).mPlayerBoardCards.size(), 1);
    }
    
    EXPECT_EQ(engine.GetBoardState().GetActivePlayerIndex(), 0);
}
