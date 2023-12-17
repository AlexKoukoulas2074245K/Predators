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
#include <game/GameConstants.h>
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

#define GET_CARD_DAMAGE(id) CardDataRepository::GetInstance().GetCardData(id)->get().mCardDamage
#define GET_CARD_WEIGHT(id) CardDataRepository::GetInstance().GetCardData(id)->get().mCardWeight

///------------------------------------------------------------------------------------------------

static constexpr int TEST_DEFAULT_PLAYER_HEALTH = 30;

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
    
    void SimulateBattle(strutils::StringId topDeckFamilyName = strutils::StringId(), strutils::StringId botDeckFamilyName = strutils::StringId());
    
    void Init(const PlayerActionGenerationEngine::ActionGenerationType actionGenerationType, const CardCollectionType& cardCollectionType, const bool useRuleEngine)
    {
        mBoardState = std::make_unique<BoardState>();
        mGameRuleEngine = std::make_unique<GameRuleEngine>(mBoardState.get());
        mActionEngine = std::make_unique<GameActionEngine>(GameActionEngine::EngineOperationMode::HEADLESS, math::RandomInt(), mBoardState.get(), nullptr, useRuleEngine ? mGameRuleEngine.get() : nullptr);
        
        mPlayerActionGenerationEngine = std::make_unique<PlayerActionGenerationEngine>(mGameRuleEngine.get(), mActionEngine.get(), actionGenerationType);
        mBoardState->GetPlayerStates().emplace_back();
        mBoardState->GetPlayerStates().back().mPlayerDeckCards = cardCollectionType == CardCollectionType::ALL_NON_SPELL_CARDS ? CardDataRepository::GetInstance().GetAllNonSpellCardIds() : CardDataRepository::GetInstance().GetAllCardIds();
        mBoardState->GetPlayerStates().back().mPlayerHealth = TEST_DEFAULT_PLAYER_HEALTH;
        mBoardState->GetPlayerStates().emplace_back();
        mBoardState->GetPlayerStates().back().mPlayerDeckCards = cardCollectionType == CardCollectionType::ALL_NON_SPELL_CARDS ? CardDataRepository::GetInstance().GetAllNonSpellCardIds() : CardDataRepository::GetInstance().GetAllCardIds();
        mBoardState->GetPlayerStates().back().mPlayerHealth = TEST_DEFAULT_PLAYER_HEALTH;
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
        Init(PlayerActionGenerationEngine::ActionGenerationType::FULLY_DETERMINISTIC, CardCollectionType::ALL_NON_SPELL_CARDS, false);
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
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 4);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestBoardStatePostDrawAndPlayAction)
{
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 3);
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerBoardCards.size(), 1);
    EXPECT_EQ(mActionEngine->GetActiveGameActionName(), IDLE_GAME_ACTION_NAME);
}

TEST_F(GameActionTests, TestDrawPlayNextDrawPlayActionRound)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {24};
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {24};
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }});
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates().at(0).mPlayerHeldCards.size(), 4);
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
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(11) + GET_CARD_WEIGHT(3) + GET_CARD_WEIGHT(4);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(11) + GET_CARD_WEIGHT(3) + GET_CARD_WEIGHT(4);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get());
    UpdateUntilActionOrIdle(NEXT_PLAYER_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerHeldCards.size(), 2); // Brachiosaurus can't be played
    EXPECT_EQ(mBoardState->GetActivePlayerState().mPlayerBoardCards.size(), 3); // Mantis, Bunny, Guinea Pig are played
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
    UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
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
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH);
    mActionEngine->Update(0);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH); // No damage is inflicted since bunny goes down to 0 attack
}

TEST_F(GameActionTests, TestNetAndFluffAttackCombinedEffects)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {21}; // Top player has a deck of nets
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {19, 0}; // Bot player has a deck of Beavers(3,3) and fluff attack
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Net is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(0);
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(0);
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {19,0};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Fluff Attack is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Beaver is played
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - GET_CARD_DAMAGE(0)); // Beaver original attack = 3. Net - 2. Fluff Attack + 2. Final attack = 3.
}

TEST_F(GameActionTests, TestDoubleFluffAttackFollowedByBunny)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {4}; // Top player has a deck of bunnies
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {4, 19}; // Bot player has a deck of Beavers(3,3) and fluff attack
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bunny is played by top player
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(4);
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(4);
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {4, 19, 19};  // Bot player has 2 fluff attacks and a bunny
    
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }}); // First Fluff Attack is played
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "1" }}); // Second Fluff Attack is played
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }}); // Bunny is played
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    
    UpdateUntilActionOrIdle(CARD_ATTACK_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - (GET_CARD_DAMAGE(4) + 2 + 2)); // Bunny original attack = 1. Fluff Attack + 2. Fluff Attack + 2. Final attack = 5.
}

TEST_F(GameActionTests, TestDoubleNetAndFluffAttackCombinedEffects)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {21}; // Top player has a deck of nets
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {19, 0}; // Bot player has a deck of Beavers(3,3) and fluff attack
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(21) + GET_CARD_WEIGHT(21);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(21) + GET_CARD_WEIGHT(21);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // 2 Nets are played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(0);
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(0);
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {19,0};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Fluff Attack is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Beaver is played
   
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH);
    
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - (GET_CARD_DAMAGE(0) - 2 - 2 + 2)); // Beaver original attack = 3. Net - 2. Net - 2. Fluff Attack + 2. Final attack = 1.
}

TEST_F(GameActionTests, TestFeatheryDinoEffect)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {23, 17}; // Top player has a deck of Feathery Dino and Triceratops
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(17) - 2 + GET_CARD_WEIGHT(23);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(17) - 2 + GET_CARD_WEIGHT(23);;
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {23, 17}; // Top player has a hand of Feathery Dino and Triceratops
    
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }}); // Feathery Dino is Played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    mActionEngine->AddGameAction(PLAY_CARD_GAME_ACTION_NAME, {{ PlayCardGameAction::LAST_PLAYED_CARD_INDEX_PARAM, "0" }}); // Triceratops is Played (with reduced weight cost)
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH);
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - GET_CARD_DAMAGE(17)); // Triceratops attacks
}

TEST_F(GameActionTests, TestBearTrapEffectFollowedByGustOfWind)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {22}; // Top player has a deck of bear traps
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {24, 4}; // Bot player has a deck of Gusts of Wind and Bunnies
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bear trap is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(24) + GET_CARD_WEIGHT(4);
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(24) + GET_CARD_WEIGHT(4);
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {24, 4};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Gust of Wind is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bunny is played
    UpdateUntilActionOrIdle(CARD_DESTRUCTION_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - GET_CARD_DAMAGE(4)); // Bunny is not killed due to Gust of Windw clearing the bear trap and attacks
}

TEST_F(GameActionTests, TestInsectDuplicationEffect)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {25, 1}; // Top player has a deck of Insect Duplications and Bees
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(25) + GET_CARD_WEIGHT(1);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(25) + GET_CARD_WEIGHT(1);
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {25, 1};
    
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Insect Duplication is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bee is played
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - (GET_CARD_DAMAGE(1) + 1 + GET_CARD_DAMAGE(1) + 1)); // 2 Bees attack instead of 1
}

TEST_F(GameActionTests, TestToxicWaveAndInsectDuplicationEffect)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {25, 27, 1}; // Top player has a deck of Insect Duplications, Toxic Wave and Bees
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(25) + GET_CARD_WEIGHT(27) + GET_CARD_WEIGHT(1);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(25) + GET_CARD_WEIGHT(27) + GET_CARD_WEIGHT(1);
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {25, 27, 1};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Insect Duplication (or Toxic Wave) is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Toxic Wave (or Insect Duplication) is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Bee is played
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - (GET_CARD_DAMAGE(1) + 2 + GET_CARD_DAMAGE(1) + 2)); // 2 Bees attack instead of 1 and apply double the poison each
}

TEST_F(GameActionTests, TestMightyDinoRoarEffect)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {26, 5}; // Top player has a deck of Mighty Dino Roars (w=2) and  Dilophosaurus (d=5,w=4)
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = 12;
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = 12;
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {26, 5, 5};
    
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Mighty Dino Roar is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // 2 Dilophosaurus' are played
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, 15); // First dilophosaurus has double attack, the second one has normal attack
}

TEST_F(GameActionTests, TestDinoMultiBuff)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {23, 28, 5}; // Top player has a deck of Feathery Dinos, Metal Claws and Dilophosaurus (d=5,w=4)
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(23) + GET_CARD_WEIGHT(28) + GET_CARD_WEIGHT(5) - 2;
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(23) + GET_CARD_WEIGHT(28) + GET_CARD_WEIGHT(5) - 2;
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {23, 28, 5};
    
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Feathery Dino is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Metal Claws are played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Dilophosaurus is played
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, TEST_DEFAULT_PLAYER_HEALTH - (GET_CARD_DAMAGE(5) + 1)); // Dilophosaurus can be played due to reduced weight cost and also has +1 attack due to Metal Claws
}

TEST_F(GameActionTests, TestImpendingDoomAndFeatheryDinoEffects)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {29, 23, 5}; // Top player has a deck of Impending Doom, Feathery Dino and Dilophosaurus (d=5,w=4)
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(29);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(29);
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {29};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Impending Doom is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(23);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(23);
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {23, 5};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Feathery Dino is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Dilophosaurus is played (-2 from impending doom, -2 from feathery dino
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerHealth, 30 - GET_CARD_DAMAGE(5)); // Dilophosaurus can be played due to reduced weight cost and also has +2 attack due to Metal Claws
}

TEST_F(GameActionTests, TestBuffedDugOutRodentsHaveCorrectModifiersPostClearingNetWithGustOfWind)
{
    mBoardState->GetPlayerStates()[0].mPlayerDeckCards = {21}; // Top player has a deck of Nets
    mBoardState->GetPlayerStates()[1].mPlayerDeckCards = {19, 4, 15, 24}; // Bot player has a deck of Fluff Attacks, Bunnies, Squirrels and Gusts of Winds
    
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    do
    {
        mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME); // Skip top player's turn
        UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
        
        mBoardState->GetPlayerStates()[0].mPlayerHealth = TEST_DEFAULT_PLAYER_HEALTH;
        mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(4) + GET_CARD_WEIGHT(15) + GET_CARD_WEIGHT(24);
        mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(19) + GET_CARD_WEIGHT(4) + GET_CARD_WEIGHT(15) + GET_CARD_WEIGHT(24);
        mBoardState->GetPlayerStates()[1].mPlayerBoardCards = {};
        mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {4, 15, 19};
        
        mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Fluff Attack is played
        UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
        mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Squirrel and Bunny are played
        
        UpdateUntilActionOrIdle(DRAW_CARD_GAME_ACTION_NAME);
        
        if (mBoardState->GetPlayerStates()[1].mPlayerBoardCards.size() == 2 && mBoardState->GetPlayerStates()[0].mPlayerHealth == (TEST_DEFAULT_PLAYER_HEALTH - (GET_CARD_DAMAGE(4) + 2 + GET_CARD_DAMAGE(15) + 2))) // We want both rodents to have dug
        {
            break;
        }
    } while (true);
    
    
    mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(21);
    mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(21);
    mBoardState->GetPlayerStates()[0].mPlayerHeldCards = {21};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Net is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = GET_CARD_WEIGHT(24);
    mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = GET_CARD_WEIGHT(24);
    mBoardState->GetPlayerStates()[1].mPlayerHeldCards = {24};
    
    mPlayerActionGenerationEngine->DecideAndPushNextActions(mBoardState.get()); // Gust of wind is played
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerBoardCardStatOverrides.size(), 2U);
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerBoardCardStatOverrides[0].at(CardStatType::DAMAGE), GET_CARD_DAMAGE(15) + 2); // Position and value of overrides is maintained
    EXPECT_EQ(mBoardState->GetPlayerStates()[1].mPlayerBoardCardStatOverrides[1].at(CardStatType::DAMAGE), GET_CARD_DAMAGE(4) + 2); // Position and value of overrides is maintained
    
    auto currentHealth = mBoardState->GetPlayerStates()[0].mPlayerHealth;
    mActionEngine->AddGameAction(NEXT_PLAYER_GAME_ACTION_NAME);
    UpdateUntilActionOrIdle(IDLE_GAME_ACTION_NAME);
    EXPECT_EQ(mBoardState->GetPlayerStates()[0].mPlayerHealth, currentHealth - (GET_CARD_DAMAGE(15) + 2 + GET_CARD_DAMAGE(4) + 2)); // 23 - 4 -3 = 16 (gust of wind cleared net)
}

int BATTLE_SIMULATION_ITERATIONS = 1000;

void GameActionTests::SimulateBattle(strutils::StringId topDeckFamilyName /*= strutils::StringId()*/, strutils::StringId botDeckFamilyName /*= strutils::StringId()*/)
{
    const int PROGRESS_INCREMENTS = BATTLE_SIMULATION_ITERATIONS/100;
    
    std::stringstream statistics;
    int gamesTopPlayerWonCounter = 0;
    int turnCounter = 0;
    int weightAmmoCounter = 0;
    bool mFamilyBattles = !topDeckFamilyName.isEmpty() || !botDeckFamilyName.isEmpty();
    std::vector<std::pair<int, int>> winnerGameCountsAndCardIds;
    std::vector<std::pair<int, int>> looserGameCountsAndCardIds;
    std::vector<std::pair<float, int>> powerLevelAndCardIds;
    std::set<int> uniquePlayedCardIds[2];
    
    std::cout << "            0%  5%  10%  15%  20%  25%  30%  35%  40%  45%  50%  55%  60%  65%  70%  75%  80%  85%  90%  95%  100%\n";
    std::cout << "Progress:   [";
    
    for (int i = 0; i < BATTLE_SIMULATION_ITERATIONS; ++i)
    {
        if (i % PROGRESS_INCREMENTS == 0)
        {
            std::cout << "#" << std::flush;
        }
        
        uniquePlayedCardIds[0].clear();
        uniquePlayedCardIds[1].clear();
        
        Init(PlayerActionGenerationEngine::ActionGenerationType::OPTIMISED, CardCollectionType::ALL_CARDS, true);
        mBoardState->GetPlayerStates()[0].mPlayerHealth = game_constants::TOP_PLAYER_DEFAULT_HEALTH;
        mBoardState->GetPlayerStates()[1].mPlayerHealth = game_constants::TOP_PLAYER_DEFAULT_HEALTH;
        
        mBoardState->GetPlayerStates()[0].mPlayerTotalWeightAmmo = game_constants::TOP_PLAYER_DEFAULT_WEIGHT;
        mBoardState->GetPlayerStates()[0].mPlayerCurrentWeightAmmo = game_constants::TOP_PLAYER_DEFAULT_WEIGHT;
        
        mBoardState->GetPlayerStates()[1].mPlayerTotalWeightAmmo = game_constants::BOT_PLAYER_DEFAULT_WEIGHT;
        mBoardState->GetPlayerStates()[1].mPlayerCurrentWeightAmmo = game_constants::BOT_PLAYER_DEFAULT_WEIGHT;
        
        if (mFamilyBattles)
        {
            mBoardState->GetPlayerStates()[0].mPlayerDeckCards = CardDataRepository::GetInstance().GetCardIdsByFamily(topDeckFamilyName);
            mBoardState->GetPlayerStates()[1].mPlayerDeckCards = CardDataRepository::GetInstance().GetCardIdsByFamily(botDeckFamilyName);
        }
        
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
    
    std::cout << "#]\n";
    
    std::sort(winnerGameCountsAndCardIds.begin(), winnerGameCountsAndCardIds.end(), [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
    {
        return lhs.first > rhs.first;
    });
    
    std::sort(looserGameCountsAndCardIds.begin(), looserGameCountsAndCardIds.end(), [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
    {
        return lhs.first > rhs.first;
    });
    
    if (mFamilyBattles)
    {
        statistics << "Total Games: " << BATTLE_SIMULATION_ITERATIONS << "\n";
        statistics << "Games won: Top=" << 100.0f * gamesTopPlayerWonCounter/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "%  Bot=" << 100.0f * (BATTLE_SIMULATION_ITERATIONS - gamesTopPlayerWonCounter)/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "%\n";
        statistics << "Average weight ammo per game on victory: " << weightAmmoCounter/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "\n";
        statistics << "Average turns per game: " << turnCounter/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "\n";
        logging::Log(logging::LogType::INFO, "Card Family battle: %s vs %s:\n%s", topDeckFamilyName.GetString().c_str(), botDeckFamilyName.GetString().c_str(), statistics.str().c_str());
    }
    else
    {
        statistics << "Total Games: " << BATTLE_SIMULATION_ITERATIONS << "\n";
        statistics << "Games won: Top=" << 100.0f * gamesTopPlayerWonCounter/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "%  Bot=" << 100.0f * (BATTLE_SIMULATION_ITERATIONS - gamesTopPlayerWonCounter)/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "%\n";
        statistics << "Average weight ammo per game on victory: " << weightAmmoCounter/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "\n";
        statistics << "Average turns per game: " << turnCounter/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "\n";
        
        
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
            row << " in " << 100.0f * entry.first/static_cast<float>(BATTLE_SIMULATION_ITERATIONS) << "%";
            row << std::setw(55 - static_cast<int>(row.str().size()));
            row << " of games (" << entry.first << " out of " << BATTLE_SIMULATION_ITERATIONS << " games) \n";
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
            float powerLevel = (entry.first / static_cast<float>(BATTLE_SIMULATION_ITERATIONS)) * 100.0f;
            
            auto foundInLostGamesContainer = std::find_if(looserGameCountsAndCardIds.cbegin(), looserGameCountsAndCardIds.cend(), [=](const std::pair<int, int>& looseEntry)
            {
                return looseEntry.second == entry.second;
            });
            if (foundInLostGamesContainer != looserGameCountsAndCardIds.cend())
            {
                powerLevel -= (foundInLostGamesContainer->first / static_cast<float>(BATTLE_SIMULATION_ITERATIONS) * 100.0f);
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
}

TEST_F(GameActionTests, BattleSimulation)
{
    // Simulate battles with all cards
    SimulateBattle();
    
    // Create family battle combinations
    const auto& cardFamiliesSet = CardDataRepository::GetInstance().GetCardFamilies();
    std::vector<strutils::StringId> cardFamilies(cardFamiliesSet.begin(), cardFamiliesSet.end());
    std::unordered_map<strutils::StringId, std::vector<strutils::StringId>, strutils::StringIdHasher> cardFamilyBattleCombinations;
    
    for (auto i = 0U; i < cardFamilies.size(); ++i)
    {
        for (auto j = 0U; j < cardFamilies.size(); ++j)
        {
            const auto& lhsFamily = cardFamilies[i];
            const auto& rhsFamily = cardFamilies[j];
            
            if (i != j)
            {
                cardFamilyBattleCombinations[lhsFamily].push_back(rhsFamily);
                if (cardFamilyBattleCombinations.count(rhsFamily) != 0)
                {
                    auto& existingRhsBattles = cardFamilyBattleCombinations[rhsFamily];
                    existingRhsBattles.erase(std::find(existingRhsBattles.begin(), existingRhsBattles.end(), lhsFamily));
                }
            }
        }
    }
    
    // Simulate battles for card family vs card family
    for (const auto& battleCombinationEntry: cardFamilyBattleCombinations)
    {
        for (const auto& rhsFamily: battleCombinationEntry.second)
        {
            SimulateBattle(battleCombinationEntry.first, rhsFamily);
            SimulateBattle(rhsFamily, battleCombinationEntry.first);
        }
    }
}
