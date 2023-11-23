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
static const strutils::StringId CARD_ATTACK_GAME_ACTION_NAME = strutils::StringId("CardAttackGameAction");
static const strutils::StringId TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME = strutils::StringId("TrapTriggeredAnimationGameAction");
static const strutils::StringId CARD_DESTRUCTION_GAME_ACTION_NAME = strutils::StringId("CardDestructionGameAction");
static const strutils::StringId CARD_EFFECT_GAME_ACTION_NAME = strutils::StringId("CardEffectGameAction");

///------------------------------------------------------------------------------------------------

class GameActionTests : public testing::Test
{
protected:
    enum class CardCollectionType
    {
        ALL_CARDS, ALL_NON_SPELL_CARDS
    };
    
protected:
    GameActionTests()
    {
        CardDataRepository::GetInstance().LoadCardData(false);
    }
    
    void Init(const CardCollectionType& cardCollectionType, const bool useRuleEngine)
    {
        mBoardState = std::make_unique<BoardState>();
        mGameRuleEngine = std::make_unique<GameRuleEngine>(mBoardState.get());
        mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::HEADLESS, math::RandomInt(), mBoardState.get(), nullptr, useRuleEngine ? mGameRuleEngine.get() : nullptr, nullptr);
        
        mPlayerActionGenerationEngine = std::make_unique<PlayerActionGenerationEngine>(mGameRuleEngine.get(), mActionEngine.get());
        mBoardState->GetPlayerStates().emplace_back();
        mBoardState->GetPlayerStates().back().mPlayerDeckCards = cardCollectionType == CardCollectionType::ALL_NON_SPELL_CARDS ? CardDataRepository::GetInstance().GetAllNonSpellCardIds() : CardDataRepository::GetInstance().GetAllCardIds();
        mBoardState->GetPlayerStates().emplace_back();
        mBoardState->GetPlayerStates().back().mPlayerDeckCards = cardCollectionType == CardCollectionType::ALL_NON_SPELL_CARDS ? CardDataRepository::GetInstance().GetAllNonSpellCardIds() : CardDataRepository::GetInstance().GetAllCardIds();
    }
    
    void UpdateUntilActionOrIdle(const strutils::StringId& actionName)
    {
        while (mActionEngine->GetActiveGameActionName() != IDLE_GAME_ACTION_NAME && mActionEngine->GetActiveGameActionName() != actionName)
        {
            mActionEngine->Update(0);
        }
    }
    
    void SetUp() override
    {
        Init(CardCollectionType::ALL_NON_SPELL_CARDS, false);
    }
    
    void TearDown() override
    {
        CardDataRepository::GetInstance().ClearCardData();
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
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 3);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestBoardStatePostDrawAndPlayAction)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 2);
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
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerHeldCards.size(), 3);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerBoardCards.size(), 0);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerHeldCards.size(), 0);
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
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 1);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 0);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 1);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 1);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 2);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 1);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerTotalWeightAmmo, 2);
    EXPECT_EQ(mBoardState->GetPlayerStates().at(1).mPlayerCurrentWeightAmmo, 2);
}

TEST_F(GameActionTests, TestPlayerActionGenerationEngine)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {3, 9, 3, 11, 4};
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = 6;
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = 6;
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get());
    UpdateUntilActionOrIdle(NEXT_PLAYER_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 2);
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerBoardCards.size(), 3);
}

TEST_F(GameActionTests, TestBearTrapEffect)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {22}; // Top player has a deck of bear traps
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {4}; // Bot player has a deck of bunnies
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bear trap is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bunny is played
    
    UpdateUntilActionOrIdle(TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME); // Make sure the next stop is at TrapTriggerAnimationGameAction (not IdleGameAction)
    mActionEngine->Update(0);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerBoardCards.size(), 1);
    mActionEngine->Update(0);
    mActionEngine->Update(0);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerBoardCards.size(), 0); // Bunny is destroyed before end of turn
}

TEST_F(GameActionTests, TestNetEffect)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {21}; // Top player has a deck of nets traps
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {4}; // Bot player has a deck of bunnies
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Net is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bunny is played
    
    UpdateUntilActionOrIdle(TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), TRAP_TRIGGERED_ANIMATION_GAME_ACTION_NAME); // Make sure the next stop is at TrapTriggerAnimationGameAction (not IdleGameAction)
    mActionEngine->Update(0);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerBoardCards.size(), 1);
    mActionEngine->Update(0);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 30);
    mActionEngine->Update(0);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 30); // No damage is inflicted since bunny goes down to 0 attack
}

TEST_F(GameActionTests, TestNetAndFluffAttackCombinedEffects)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {21}; // Top player has a deck of nets
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {19, 0}; // Bot player has a deck of Beavers(3,3) and fluff attack
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Net is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = 5;
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = 5;
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {19,0};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Beaver and Fluff Attack are played
    
    UpdateUntilActionOrIdle(CARD_EFFECT_GAME_ACTION_NAME);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), CARD_EFFECT_GAME_ACTION_NAME); // Make sure the next stop is at Card Effect (for fluff attack) (not IdleGameAction)
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 30);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 27); // Beaver original attack = 3. Net - 2. Fluff Attack + 2. Final attack = 3.
}

TEST_F(GameActionTests, TestDoubleFluffAttackFollowedByBunnyStats)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {4}; // Top player has a deck of bunnies
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {4, 19}; // Bot player has a deck of Beavers(3,3) and fluff attack
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bunny is played by top player
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = 5;
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = 5;
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {4, 19, 19};  // Bot player has 2 fluff attacks and a bunny
    
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }}); // First Fluff Attack is played
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }}); // Second Fluff Attack is played
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }}); // Bunny is played
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    UpdateUntilActionOrIdle(CARD_ATTACK_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 30);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 25); // Bunny original attack = 1. Fluff Attack + 2. Fluff Attack + 2. Final attack = 5.
}

TEST_F(GameActionTests, TestDoubleNetAndFluffAttackCombinedEffects)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {21}; // Top player has a deck of nets
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {19, 0}; // Bot player has a deck of Beavers(3,3) and fluff attack
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = 2;
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = 2;
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // 2 Nets are played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = 5;
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = 5;
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {19,0};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Beaver and Fluff Attack are played
    
    UpdateUntilActionOrIdle(CARD_EFFECT_GAME_ACTION_NAME);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), CARD_EFFECT_GAME_ACTION_NAME); // Make sure the next stop is at Card Effect (for fluff attack) (not IdleGameAction)
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 30);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 29); // Beaver original attack = 3. Net - 2. Net - 2. Fluff Attack + 2. Final attack = 1.
}

TEST_F(GameActionTests, TestFeatheryDinoEffect)
{
    Init(CardCollectionType::ALL_CARDS, true);
    
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {23, 17}; // Top player has a deck of Feathery Dino and Triceratops
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = 8;
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = 8;
    
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {23, 17}; // Top player has a hand of Feathery Dino and Triceratops
    
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }}); // Feathery Dino is Played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }}); // Triceratops is Played (with reduced weight cost)
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, 30);
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, 21); // Triceratops attacks
}

TEST_F(GameActionTests, TestBearTrapEffectFollowedByGustOfWind)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {22}; // Top player has a deck of bear traps
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {24, 4}; // Bot player has a deck of Gusts of Wind and Bunnies
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bear trap is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = 2;
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = 2;
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {24, 4};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Gust of Wind is played
    
    UpdateUntilActionOrIdle(CARD_DESTRUCTION_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, 29); // Bunny is not killed due to Gust of Windw clearing the bear trap and attacks
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
    std::vector<std::pair<float, int>> powerLevelAndCardIds;
    std::set<int> uniquePlayedCardIds[2];
    
    for (int i = 0; i < GAME_COUNT; ++i)
    {
        uniquePlayedCardIds[0].clear();
        uniquePlayedCardIds[1].clear();
        
        Init(CardCollectionType::ALL_CARDS, true);
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
    
    statistics << "\nCard power score: \n"; // won games - lost games
    for (const auto& entry: winnerGameCountsAndCardIds)
    {
        float powerLevel = (entry.first / static_cast<float>(GAME_COUNT)) * 100.0f;
        
        auto foundInLostGamesContainer = std::find_if(looserGameCountsAndCardIds.cbegin(), looserGameCountsAndCardIds.cend(), [=](const std::pair<int, int>& looseEntry)
        {
            return looseEntry.second == entry.second;
        });
        if (foundInLostGamesContainer != looserGameCountsAndCardIds.cend())
        {
            powerLevel -= (foundInLostGamesContainer->first / static_cast<float>(GAME_COUNT) * 100.0f);
        }
        
        powerLevelAndCardIds.push_back(std::make_pair(powerLevel, entry.second));
    }
    
    std::sort(powerLevelAndCardIds.begin(), powerLevelAndCardIds.end(), [](const std::pair<float, int>& lhs, const std::pair<float, int>& rhs)
    {
        return lhs.first > rhs.first;
    });
    
    for (const auto& entry: powerLevelAndCardIds)
    {
        const auto& cardData = CardDataRepository::GetInstance().GetCardData(entry.second)->get();
        std::stringstream row;
        row << "\t" << "ID=" << cardData.mCardId << ", " << "d=" << cardData.mCardDamage << ", w=" << cardData.mCardWeight;
        row << std::setw(35 - static_cast<int>(row.str().size()));
        row << cardData.mCardName ;
        row << std::setw(43 - static_cast<int>(row.str().size()));
        row << std::setprecision(2);
        row << " power " << entry.first << "%\n";
        statistics << row.str();
    }
    
    logging::Log(logging::LogType::INFO, "Game Stats: \n%s", statistics.str().c_str());
}
