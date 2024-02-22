///------------------------------------------------------------------------------------------------
///  MeteorCardSacrificeGameAction.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 22/02/2024
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/CardDestructionGameAction.h>
#include <game/GameConstants.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/MeteorCardSacrificeGameAction.h>
#include <game/gameactions/MeteorDamageGameAction.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId CARD_DESTRUCTION_GAME_ACTION_NAME = strutils::StringId("CardDestructionGameAction");
static const strutils::StringId METEOR_DAMAGE_GAME_ACTION_NAME = strutils::StringId("MeteorDamageGameAction");

///------------------------------------------------------------------------------------------------

void MeteorCardSacrificeGameAction::VSetNewGameState()
{
    auto& activePlayerState = mBoardState->GetActivePlayerState();
    
    // Find Dino to sacrifice
    auto randomHeldCardIndex = math::ControlledRandomInt() % activePlayerState.mPlayerHeldCards.size();
    auto cardData = CardDataRepository::GetInstance().GetCardData(activePlayerState.mPlayerHeldCards[randomHeldCardIndex], mBoardState->GetActivePlayerIndex());
    while (cardData.IsSpell() || cardData.mCardFamily != game_constants::DINOSAURS_FAMILY_NAME)
    {
        randomHeldCardIndex = math::ControlledRandomInt() % activePlayerState.mPlayerHeldCards.size();
        cardData = CardDataRepository::GetInstance().GetCardData(activePlayerState.mPlayerHeldCards[randomHeldCardIndex], mBoardState->GetActivePlayerIndex());
    }
    auto selectedCardIdToSacrifice = activePlayerState.mPlayerHeldCards[randomHeldCardIndex];
    
    // Erase spell from deck
    activePlayerState.mPlayerDeckCards.erase(std::remove(activePlayerState.mPlayerDeckCards.begin(), activePlayerState.mPlayerDeckCards.end(), selectedCardIdToSacrifice), activePlayerState.mPlayerDeckCards.end());
    if (activePlayerState.mPlayerDeckCards.empty())
    {
        activePlayerState.mPlayerDeckCards = { CardDataRepository::GetInstance().GetCardId(game_constants::EMPTY_DECK_TOKEN_CARD_NAME) };
    }
    
    // Find all held card indices for this card id
    std::vector<int> heldCardIndicesToDestroy;
    auto heldCardIter = activePlayerState.mPlayerHeldCards.begin();
    while ((heldCardIter = std::find(heldCardIter, activePlayerState.mPlayerHeldCards.end(), selectedCardIdToSacrifice)) != activePlayerState.mPlayerHeldCards.end())
    {
        heldCardIndicesToDestroy.push_back(static_cast<int>(std::distance(activePlayerState.mPlayerHeldCards.begin(), heldCardIter)));
        heldCardIter++;
    }
   
    mGameActionEngine->AddGameAction(CARD_DESTRUCTION_GAME_ACTION_NAME,
    {
        { CardDestructionGameAction::CARD_INDICES_PARAM, strutils::VecToString(heldCardIndicesToDestroy)},
        { CardDestructionGameAction::PLAYER_INDEX_PARAM, std::to_string(mBoardState->GetActivePlayerIndex())},
        { CardDestructionGameAction::IS_SINGLE_CARD_USED_COPY_PARAM, "true"},
        { CardDestructionGameAction::IS_BOARD_CARD_PARAM, "false"},
        { CardDestructionGameAction::IS_TRAP_TRIGGER_PARAM, "false"}
    });
    
    for (auto heldCardIter = activePlayerState.mPlayerHeldCards.begin(); heldCardIter != activePlayerState.mPlayerHeldCards.end();)
    {
        if (*heldCardIter == selectedCardIdToSacrifice)
        {
            heldCardIter = activePlayerState.mPlayerHeldCards.erase(heldCardIter);
        }
        else
        {
            heldCardIter++;
        }
    }
    
    events::EventSystem::GetInstance().DispatchEvent<events::BlockInteractionWithHeldCardsEvent>();
    
    mGameActionEngine->AddGameAction(METEOR_DAMAGE_GAME_ACTION_NAME,
    {
        { MeteorDamageGameAction::METEOR_DAMAGE_PARAM, std::to_string(cardData.mCardDamage * 2)}
    });
}

///------------------------------------------------------------------------------------------------

void MeteorCardSacrificeGameAction::VInitAnimation()
{
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult MeteorCardSacrificeGameAction::VUpdateAnimation(const float)
{
    return ActionAnimationUpdateResult::FINISHED;
}

///------------------------------------------------------------------------------------------------

bool MeteorCardSacrificeGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& MeteorCardSacrificeGameAction::VGetRequiredExtraParamNames() const
{
    static std::vector<std::string> v;
    return v;
}

///------------------------------------------------------------------------------------------------
