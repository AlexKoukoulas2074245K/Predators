///------------------------------------------------------------------------------------------------
///  GameActionTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 29/09/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <engine/utils/Logging.h>
#include <game/BoardState.h>
#include <game/Cards.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayCardGameAction.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");
static const strutils::StringId PLAY_CARD_GAME_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("NextPlayerGameAction");

///------------------------------------------------------------------------------------------------

class GameActionTests : public testing::Test
{
protected:
    GameActionTests()
        : mBoardState()
        , mActionEngine(GameActionEngine::EngineOperationMode::HEADLESS, 0, &mBoardState, nullptr)
    {
        
    }
    
    void SetUp() override
    {
        CardDataRepository::GetInstance().LoadCardData();
        mBoardState.GetPlayerStates().emplace_back();
        mBoardState.GetPlayerStates().emplace_back();
    }
    
protected:
    BoardState mBoardState;
    GameActionEngine mActionEngine;
};

///------------------------------------------------------------------------------------------------

TEST_F(GameActionTests, TestIdleGameActionExistsByDefault)
{
    EXPECT_EQ(mActionEngine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestPushedGameActionIsActive)
{
    mActionEngine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    
    EXPECT_EQ(mActionEngine.GetActiveGameActionName(), DRAW_CARD_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestBoardStatePostDrawAction)
{
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.Update(0);
    
    mActionEngine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    mActionEngine.Update(0);
    
    EXPECT_EQ(mBoardState.GetActivePlayerState().mPlayerHeldCards.size(), 1);
    EXPECT_EQ(mActionEngine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestBoardStatePostDrawAndPlayAction)
{
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    mActionEngine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine.Update(0);
    mActionEngine.Update(0);
    mActionEngine.Update(0);
    
    EXPECT_EQ(mBoardState.GetActivePlayerState().mPlayerHeldCards.size(), 0);
    EXPECT_EQ(mBoardState.GetActivePlayerState().mPlayerBoardCards.size(), 1);
    EXPECT_EQ(mActionEngine.GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestDrawPlayNextDrawPlayActionRound)
{
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    mActionEngine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    mActionEngine.AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine.GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine.Update(0);
    }
    
    for (size_t i = 0; i < mBoardState.GetPlayerCount(); ++i)
    {
        EXPECT_EQ(mBoardState.GetPlayerStates().at(i).mPlayerHeldCards.size(), 0);
        EXPECT_EQ(mBoardState.GetPlayerStates().at(i).mPlayerBoardCards.size(), 1);
    }
    
    EXPECT_EQ(mBoardState.GetActivePlayerIndex(), 0);
}

TEST_F(GameActionTests, TestWeightAmmoIncrements)
{
    for (size_t i = 0; i < mBoardState.GetPlayerCount(); ++i)
    {
        EXPECT_EQ(mBoardState.GetPlayerStates().at(i).mPlayerTotalWeightAmmo, 0);
        EXPECT_EQ(mBoardState.GetPlayerStates().at(i).mPlayerCurrentWeightAmmo, 0);
    }
    
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.Update(0);
    
    EXPECT_EQ(mBoardState.GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 1);
    EXPECT_EQ(mBoardState.GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 0);
    
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.Update(0);
    
    EXPECT_EQ(mBoardState.GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 1);
    EXPECT_EQ(mBoardState.GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 1);
    
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.Update(0);
    
    EXPECT_EQ(mBoardState.GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 2);
    EXPECT_EQ(mBoardState.GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 1);
    
    mActionEngine.AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine.Update(0);
    
    EXPECT_EQ(mBoardState.GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 2);
    EXPECT_EQ(mBoardState.GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 2);
}
