///------------------------------------------------------------------------------------------------
///  CardDestructionGameAction.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/Cards.h>
#include <game/CardUtils.h>
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

const std::string CardDestructionGameAction::PLAYER_INDEX_PARAM = "playerIndex";
const std::string CARD_DISSOLVE_SHADER_FILE_NAME = "card_dissolve.vs";

static const strutils::StringId DISSOLVE_THRESHOLD_UNIFORM_NAME = strutils::StringId("dissolve_threshold");
static const strutils::StringId CARD_ORIGIN_X_UNIFORM_NAME = strutils::StringId("card_origin_x");
static const strutils::StringId CARD_ORIGIN_Y_UNIFORM_NAME = strutils::StringId("card_origin_y");

static const std::string DISSOLVE_TEXTURE_FILE_NAME = "dissolve.png";
static const float CARD_DISSOLVE_SPEED = 0.001f;
static const float MAX_CARD_DISSOLVE_VALUE = 1.2f;

///------------------------------------------------------------------------------------------------

static const std::vector<std::string> sRequiredExtraParamNames =
{
    CardDestructionGameAction::PLAYER_INDEX_PARAM
};

///------------------------------------------------------------------------------------------------

void CardDestructionGameAction::VSetNewGameState()
{
    assert(mExtraActionParams.count(PLAYER_INDEX_PARAM) != 0);
    
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    auto& attackingPlayerBoardCards = mBoardState->GetPlayerStates()[attackingPayerIndex].mPlayerBoardCards;
    
    assert(attackingPlayerBoardCards.size() > 0);
    
    attackingPlayerBoardCards.erase(attackingPlayerBoardCards.begin());
}

///------------------------------------------------------------------------------------------------

void CardDestructionGameAction::VInitAnimation()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    
    
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(0);
    for (auto& sceneObject: cardSoWrapper->mSceneObjectComponents)
    {
        sceneObject->mShaderResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + CARD_DISSOLVE_SHADER_FILE_NAME);
        sceneObject->mEffectTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + DISSOLVE_TEXTURE_FILE_NAME);
        sceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] = 0.0f;
        sceneObject->mShaderFloatUniformValues[CARD_ORIGIN_X_UNIFORM_NAME] = cardSoWrapper->mSceneObjectComponents.front()->mPosition.x;
        sceneObject->mShaderFloatUniformValues[CARD_ORIGIN_Y_UNIFORM_NAME] = cardSoWrapper->mSceneObjectComponents.front()->mPosition.y;
    }
}

///------------------------------------------------------------------------------------------------

ActionAnimationUpdateResult CardDestructionGameAction::VUpdateAnimation(const float dtMillis)
{
    auto attackingPayerIndex = std::stoi(mExtraActionParams.at(PLAYER_INDEX_PARAM));
    auto cardSoWrapper = mGameSessionManager->GetBoardCardSoWrappers().at(attackingPayerIndex).at(0);
    for (auto& sceneObject: cardSoWrapper->mSceneObjectComponents)
    {
        sceneObject->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] += dtMillis * CARD_DISSOLVE_SPEED;
    }
    
    if (cardSoWrapper->mSceneObjectComponents.front()->mShaderFloatUniformValues[DISSOLVE_THRESHOLD_UNIFORM_NAME] >= MAX_CARD_DISSOLVE_VALUE)
    {
        mGameSessionManager->OnBoardCardDestruction(0, attackingPayerIndex == game_constants::REMOTE_PLAYER_INDEX);
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
