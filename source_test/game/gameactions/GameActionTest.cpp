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
#include <game/GameRuleEngine.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/PlayCardGameAction.h>
#include <game/gameactions/PlayerActionGenerationEngine.h>
#include <memory>
#include <set>

///------------------------------------------------------------------------------------------------

static const strutils::StringId IDLE_GAME_ACTION_NAME = strutils::StringId("IdleGameAction");
static const strutils::StringId DRAW_CARD_GAME_ACTION_NAME = strutils::StringId("DrawCardGameAction");
static const strutils::StringId PLAY_CARD_GAME_ACTION_NAME = strutils::StringId("PlayCardGameAction");
static const strutils::StringId NEXT_PLAYER_GAME_ACTION_NAME = strutils::StringId("NextPlayerGameAction");
static const strutils::StringId GAME_OVER_GAME_ACTION_NAME = strutils::StringId("GameOverGameAction");

///------------------------------------------------------------------------------------------------

class GameActionTests : public testing::Test
{
protected:
    void Init()
    {
        mBoardState = std::make_unique<BoardState>();
        mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::HEADLESS, math::RandomInt(), mBoardState.get(), nullptr, nullptr, nullptr);
        mGameRuleEngine = std::make_unique<GameRuleEngine>(mBoardState.get());
        mPlayerActionGenerationEngine = std::make_unique<PlayerActionGenerationEngine>(mGameRuleEngine.get(), mActionEngine.get());
        mBoardState->GetPlayerStates().emplace_back();
        mBoardState->GetPlayerStates().back().mPlayerDeckCards = CardDataRepository::GetInstance().GetAllCardIds();
        mBoardState->GetPlayerStates().emplace_back();
        mBoardState->GetPlayerStates().back().mPlayerDeckCards = CardDataRepository::GetInstance().GetAllCardIds();
    }
    
    void SetUp() override
    {
        CardDataRepository::GetInstance().LoadCardData(false);
        Init();
    }
    
protected:
    std::unique_ptr<BoardState> mBoardState;
    std::unique_ptr<GameActionEngine> mActionEngine;
    std::unique_ptr<GameRuleEngine> mGameRuleEngine;
    std::unique_ptr<PlayerActionGenerationEngine> mPlayerActionGenerationEngine;
};

///------------------------------------------------------------------------------------------------

TEST_F(GameActionTests, TestIdleGameActionExistsByDefault)
{
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestPushedGameActionIsActive)
{
    mActionEngine->AddGameAction(DRAW_CARD_GAME_ACTION_NAME);
    
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), DRAW_CARD_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestBoardStatePostDrawAction)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 2);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestBoardStatePostDrawAndPlayAction)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 1);
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerBoardCards.size(), 1);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestDrawPlayNextDrawPlayActionRound)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerHeldCards.size(), 2);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerBoardCards.size(), 0);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerHeldCards.size(), 3);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerBoardCards.size(), 0);
    
    EXPECT_EQ(mBoardState->GetActivePlayerIndex(), 0);
}

TEST_F(GameActionTests, TestWeightAmmoIncrements)
{
    for (size_t i = 0; i < mBoardState->GetPlayerCount(); ++i)
    {
        EXPECT_EQ(mBoardState->GetPlayerStates().at(i).mPlayerTotalWeightAmmo, 0);
        EXPECT_EQ(mBoardState->GetPlayerStates().at(i).mPlayerCurrentWeightAmmo, 0);
    }
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 1);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 0);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 1);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 1);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 2);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 1);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 2);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 2);
}

TEST_F(GameActionTests, TestPlayerActionGenerationEngine)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {3, 9, 3, 11, 4};
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = 6;
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = 6;
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get());
    
    while (mActionEngine->GetActiveGameActionName() != NEXT_PLAYER_GAME_ACTION_NAME)
    {
        mActionEngine->Update(0);
    }
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 2);
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerBoardCards.size(), 3);
}

TEST_F(GameActionTests, BattleSimulation)
{
    constexpr int GAME_COUNT = 10000;
    
    std::stringstream statistics;
    int gamesTopPlayerWonCounter = 0;
    int turnCounter = 0;
    int weightAmmoCounter = 0;
    std::vector<std::pair<int, int>> winnerGameCountsAndCardIds;
    std::vector<std::pair<int, int>> looserGameCountsAndCardIds;
    std::set<int> uniquePlayedCardIds[2];
    
    for (int i = 0; i < GAME_COUNT; ++i)
    {
        uniquePlayedCardIds[0].clear();
        uniquePlayedCardIds[1].clear();
        
        Init();
        mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
        while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME && mActionEngine->GetActiveGameActionName() != GAME_OVER_GAME_ACTION_NAME)
        {
            mActionEngine->Update(0);
        }
        
        while (mActionEngine->GetActiveGameActionName() != GAME_OVER_GAME_ACTION_NAME)
        {
            mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get());
            while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME && mActionEngine->GetActiveGameActionName() != GAME_OVER_GAME_ACTION_NAME)
            {
                mActionEngine->Update(0);
                
                for (const auto cardId: mBoardState->GetPlayerStates()[0].mPlayerBoardCards)
                {
                    uniquePlayedCardIds[0].insert(cardId);
                }
                
                for (const auto cardId: mBoardState->GetPlayerStates()[1].mPlayerBoardCards)
                {
                    uniquePlayedCardIds[1].insert(cardId);
                }
            }
        }
        
        assert(mBoardState->GetPlayerStates()[0].mPlayerHealth > 0 || mBoardState->GetPlayerStates()[1].mPlayerHealth > 0);
        if (mBoardState->GetPlayerStates()[0].mPlayerHealth > 0)
        {
            gamesTopPlayerWonCounter++;
        }
        
        auto winnerPlayerIndex = mBoardState->GetPlayerStates()[0].mPlayerHealth > 0 ? 0 : 1;
        auto looserPlayerIndex = mBoardState->GetPlayerStates()[0].mPlayerHealth <= 0 ? 0 : 1;
        
        for (auto cardId: uniquePlayedCardIds[winnerPlayerIndex])
        {
            auto foundInWinnerGamesIter = std::find_if(winnerGameCountsAndCardIds.begin(), winnerGameCountsAndCardIds.end(), [&](const std::pair<int,int>& entry)
            {
                return cardId == entry.second;
            });
            
            if (foundInWinnerGamesIter != winnerGameCountsAndCardIds.end())
            {
                foundInWinnerGamesIter->first++;
            }
            else
            {
                winnerGameCountsAndCardIds.emplace_back(std::make_pair(1, cardId));
            }
        }
        
        for (const auto cardId: uniquePlayedCardIds[looserPlayerIndex])
        {
            auto foundInLosserGamesIter = std::find_if(looserGameCountsAndCardIds.begin(), looserGameCountsAndCardIds.end(), [&](const std::pair<int,int>& entry)
            {
                return cardId == entry.second;
            });
            
            if (foundInLosserGamesIter != looserGameCountsAndCardIds.end())
            {
                foundInLosserGamesIter->first++;
            }
            else
            {
                looserGameCountsAndCardIds.emplace_back(std::make_pair(1, cardId));
            }
        }
        
        turnCounter += mBoardState->GetTurnCounter();
        weightAmmoCounter += mBoardState->GetPlayerStates()[winnerPlayerIndex].mPlayerTotalWeightAmmo;
    }
    
    std::sort(winnerGameCountsAndCardIds.begin(), winnerGameCountsAndCardIds.end(), [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
    {
        return lhs.first > rhs.first;
    });
    
    std::sort(looserGameCountsAndCardIds.begin(), looserGameCountsAndCardIds.end(), [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
    {
        return lhs.first > rhs.first;
    });
    
    statistics << "Games won: Top=" << 100.0f * gamesTopPlayerWonCounter/static_cast<float>(GAME_COUNT) << "%  Bot=" << 100.0f * (GAME_COUNT - gamesTopPlayerWonCounter)/static_cast<float>(GAME_COUNT) << "%\n";
    statistics << "Average weight ammo per game on victory: " << weightAmmoCounter/static_cast<float>(GAME_COUNT) << "\n";
    statistics << "Average turns per game: " << turnCounter/static_cast<float>(GAME_COUNT) << "\n";
    
    statistics << "Card pressence in won games: \n";
    auto cardStatRowPopulationLambda = [&](const std::pair<int, int> entry)
    {
        const auto& cardData = CardDataRepository::GetInstance().GetCardData(entry.second)->get();
        std::stringstream row;
        row << "\t" << "ID=" << cardData.mCardId << ", " << "d=" << cardData.mCardDamage << ", w=" << cardData.mCardWeight;
        row << std::setw(35 - static_cast<int>(row.str().size()));
        row << cardData.mCardName ;
        row << std::setw(43 - static_cast<int>(row.str().size()));
        row << std::setprecision(2);
        row << " in " << 100.0f * entry.first/static_cast<float>(GAME_COUNT) << "%";
        row << std::setw(55 - static_cast<int>(row.str().size()));
        row << " of games (" << entry.first << " out of " << GAME_COUNT << " games) \n";
        statistics << row.str();
    };
    
    for (const auto& entry: winnerGameCountsAndCardIds)
    {
        cardStatRowPopulationLambda(entry);
    }
    
    statistics << "\nCard pressence in lost games: \n";
    for (const auto& entry: looserGameCountsAndCardIds)
    {
        cardStatRowPopulationLambda(entry);
    }
    
    logging::Log(logging::LogType::INFO, "Game Stats: \n%s", statistics.str().c_str());
        
//        std::stringstream ss;
//        ss << "Turn Counter: " << mBoardState->GetTurnCounter() << "\n";
//        ss << "Health Top: " << mBoardState->GetPlayerStates()[0].mPlayerHealth << "\n";
//        ss << "Weight Top: " << mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo << "\n";
//        ss << "Board Top: ";
//        for (size_t i = 0; i < mBoardState->GetPlayerStates()[0].mPlayerBoardCards.size(); ++i)
//        {
//            ss << mBoardState->GetPlayerStates()[0].mPlayerBoardCards[i] << ", ";
//        }
//        ss << "\n";
//
//        ss << "Board Bottom: ";
//        for (size_t i = 0; i < mBoardState->GetPlayerStates()[1].mPlayerBoardCards.size(); ++i)
//        {
//            ss << mBoardState->GetPlayerStates()[1].mPlayerBoardCards[i] << ", ";
//        }
//        ss << "\n";
//        ss << "Health Bottom: " << mBoardState->GetPlayerStates()[1].mPlayerHealth << "\n";
//        ss << "Weight Bottom: " << mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo << "\n";
//
//        ss << "Player " << (mBoardState->GetPlayerStates()[1].mPlayerHealth <= 0 ? "0" : "1") << " won!";
}
