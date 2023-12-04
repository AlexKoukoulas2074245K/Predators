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
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

const std::string CardDestructionGameAction::CARD_INDICES_PARAM = "cardIndices";
const std::string CardDestructionGameAction::PLAYER_INDEX_PARAM = "playerIndex";
const std::string CardDestructionGameAction::IS_BOARD_CARD_PARAM = "isBoardCard";
const std::string CardDestructionGameAction::IS_TRAP_TRIGGER_PARAM = "isTrapTrigger";

const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_dissolve.vs";

static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId DISSOLVE_MAGNITUDE_UNIFORM_NAME = strutils::StringId("dissolve_magnitude");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");

static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const float CARD_DISSOLVE_SPEED = 0.001f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;

static const glm::vec2 CARD_DISSOLVE_EFFECT_MAG_RANGE = {7.0f, 14.0f};

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    CardDestructionGameAction::CARD_INDICES_PARAM,
    CardDestructionGameAction::PLAYER_INDEX_PARAM,
    CardDestructionGameAction::IS_BOARD_CARD_PARAM,
    CardDestructionGameAction::IS_TRAP_TRIGGER_PARAM,
};

///------------------------------------------------------------------------------------------------

void CardDestructionGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(CARD_INDICES_PARAM) != 0);
    assert(mExtraActionParams.count(PLAYER_INDEX_PARAM) != 0);
    assert(mExtraActionParams.count(IS_BOARD_CARD_PARAM) != 0);
    assert(mExtraActionParams.count(IS_TRAP_TRIGGER_PARAM) != 0);
    
    auto cardIndices = strutils::StringToVecOfStrings(mExtraActionParams.at(CARD_INDICES_PARAM));
    
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    bool isBoardCard = mExtraActionParams.at(IS_BOARD_CARD_PARAM) == "true";
    bool isTrapTrigger = mExtraActionParams.at(IS_TRAP_TRIGGER_PARAM) == "true";
    
    if (isBoardCard && !isTrapTrigger)
    {
        for (const auto& cardIndex: cardIndices)
        {
            if (isBoardCard)
            {
                mBoardState->GetPlayerStates()[attackingPayerIndex].mBoardCardIndicesToDestroy.insert(std::stoi(cardIndex));
            }
            else
            {
                mBoardState->GetPlayerStates()[attackingPayerIndex].mHeldCardIndicesToDestroy.insert(std::stoi(cardIndex));
            }
        }
    }
    else if (isBoardCard && isTrapTrigger)
    {
        mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerBoardCards.pop_back();
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
        cardSoWrapper->mSceneObject->mEffectTextureResourceIds[1] = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
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
    auto playerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    bool isBoardCard = mExtraActionParams.at(IS_BOARD_CARD_PARAM) == "true";
    bool isTrapTrigger = mExtraActionParams.at(IS_TRAP_TRIGGER_PARAM) == "true";
    
    bool finished = false;
    for (const auto& cardIndex: cardIndices)
    {
        auto cardIndexInt = std::stoi(cardIndex);
        auto cardSoWrapper = isBoardCard ?
            mGameSessionManager->GetBoardCardSoWrappers().at(playerIndex).at(cardIndexInt) :
            mGameSessionManager->GetHeldCardSoWrappers().at(playerIndex).at(cardIndexInt);
        cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
        
        if (cardSoWrapper->mSceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
        {
            if (isTrapTrigger)
            {
                events::EventSystem::GetInstance().DispatchEvent<events::ImmediateCardDestructionWithRepositionEvent>(cardIndexInt, true, playerIndex == game_constants::REMOTE_PLAYER_INDEX);
            }
            finished = true;
        }
    }
    
    if (finished)
    {
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
