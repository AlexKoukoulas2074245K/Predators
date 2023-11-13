///------------------------------------------------------------------------------------------------
///  CardDestructionGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
#include <game/events/EventSystem.h>
#include <game/gameactions/CardDestructionGameAction.h>
#include <game/gameactions/GameActionEngine.h>
#include <game/gameactions/GameOverGameAction.h>
#include <game/GameSessionManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Particles.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

const std::string CardDestructionGameAction::CARD_INDICES_PARAM = "cardIndices";
const std::string CardDestructionGameAction::PLAYER_INDEX_PARAM = "playerIndex";
const std::string CardDestructionGameAction::IS_BOARD_CARD_PARAM = "isBoardCard";

const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_dissolve.vs";

static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");

static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const float CARD_DISSOLVE_SPEED = 0.001f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;

static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE = {10.0f, 20.0f};

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    CardDestructionGameAction::CARD_INDICES_PARAM,
    CardDestructionGameAction::PLAYER_INDEX_PARAM,
    CardDestructionGameAction::IS_BOARD_CARD_PARAM
};

///------------------------------------------------------------------------------------------------

void CardDestructionGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(CARD_INDICES_PARAM) != 0);
    assert(mExtraActionParams.count(PLAYER_INDEX_PARAM) != 0);
    assert(mExtraActionParams.count(IS_BOARD_CARD_PARAM) != 0);
    
    auto cardIndices = strutils::StringToVecOfStrings(mExtraActionParams.at(CARD_INDICES_PARAM));
    
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    bool isBoardCard = mExtraActionParams.at(IS_BOARD_CARD_PARAM) == "true";
    
    auto& cards = isBoardCard ?
        mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerBoardCards:
        mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerHeldCards;
    
    assert(cards.size() >= cardIndices.size());
    
    std::vector<int> remainingCards;
    for (int i = 0; i < static_cast<int>(cards.size()); ++i)
    {
        if (std::find_if(cardIndices.cbegin(), cardIndices.cend(), [=](const std::string& index){ return std::stoi(index) == i; }) == cardIndices.end())
        {
            remainingCards.emplace_back(cards[i]);
        }
    }
    
    if (isBoardCard)
    {
        mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerBoardCards = remainingCards;
    }
    else
    {
        mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerHeldCards = remainingCards;
    }
}

///------------------------------------------------------------------------------------------------

void CardDestructionGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    
    auto cardIndices = strutils::StringToVecOfStrings(mExtraActionParams.at(CARD_INDICES_PARAM));
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    bool isBoardCard = mExtraActionParams.at(IS_BOARD_CARD_PARAM) == "true";
    
    for (const auto& cardIndex: cardIndices)
    {
        auto cardIndexInt = std::stoi(cardIndex);
        auto cardSoWrapper = isBoardCard ?
            mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndexInt) :
            mGameSessionManager->GetHeldCardSoWrappers().at(attackingPayerIndex).at(cardIndexInt);
        
        cardSoWrapper->mSceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_DISSOLVE_SHADER_FILE_NAME);
        cardSoWrapper->mSceneObject->mEffectTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_X_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.x;
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[CARD_ORIGIN_Y_UNIFORM_NAME] = cardSoWrapper->mSceneObject->mPosition.y;
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_MAGNITUDE_UNIFORM_NAME] = math::RandomFloat(CARD_DISSOLVE_EFFECT_MAG_RANGE.x, CARD_DISSOLVE_EFFECT_MAG_RANGE.y);
    }
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult CardDestructionGameAction::VUpdateAnimation(const float dtMillis)
{
    auto cardIndices = strutils::StringToVecOfStrings(mExtraActionParams.at(CARD_INDICES_PARAM));
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    bool isBoardCard = mExtraActionParams.at(IS_BOARD_CARD_PARAM) == "true";
    
    bool finished = false;
    for (const auto& cardIndex: cardIndices)
    {
        auto cardIndexInt = std::stoi(cardIndex);
        auto cardSoWrapper = isBoardCard ?
            mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(cardIndexInt) :
            mGameSessionManager->GetHeldCardSoWrappers().at(attackingPayerIndex).at(cardIndexInt);
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
      
        if (cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
        {
            finished = true;
        }
    }
    
    if (finished)
    {
        events::EventSystem::GetInstance().DispatchEvent<events::CardDestructionEvent>(cardIndices, isBoardCard, attackingPayerIndex == game_constants::REMOTE_PLAYER_INDEX);
        return ActionAnimationUpdateResult::FINISHED;
    }
    
    return ActionAnimationUpdateResult::ONGOING;
}

///------------------------------------------------------------------------------------------------

bool CardDestructionGameAction::VShouldBeSerialized() const
{
    return false;
}

///------------------------------------------------------------------------------------------------

const std::vector<std::string>& CardDestructionGameAction::VGetRequiredExtraParamNames() const
{
    return sRequiredExtraParamNames;
}
