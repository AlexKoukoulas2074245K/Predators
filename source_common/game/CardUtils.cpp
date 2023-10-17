///------------------------------------------------------------------------------------------------
///  CardUtils.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 10/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
#include <game/CardUtils.h>

///------------------------------------------------------------------------------------------------

namespace card_utils
{

///------------------------------------------------------------------------------------------------

static const std::string CARD_FRAME_TEXTURE_FILE_NAME = "card_frame.png";
static const std::string CARD_BACK_TEXTURE_FILE_NAME = "card_back.png";
static const std::string CARD_DAMAGE_ICON_TEXTURE_FILE_NAME = "damage_icon.png";

static const float CARD_NAME_AREA_LENGTH = 0.042f;
static const float CARD_NAME_TEST_DEDUCT_INCREMENTS = 0.00001f;
static const float CARD_INDEX_Z_OFFSET = 1.0f;

///------------------------------------------------------------------------------------------------

glm::vec3 CalculateHeldCardPosition(const int cardIndex, const int playerCardCount, bool forOpponentPlayer)
{
    float cardBlockWidth = game_constants::IN_GAME_CARD_WIDTH * playerCardCount;
    float cardStartX = -cardBlockWidth/2.0f;
    
    auto targetX = cardStartX + cardIndex * game_constants::IN_GAME_CARD_WIDTH + game_constants::IN_GAME_CARD_WIDTH/2;
    if (playerCardCount > game_constants::IN_GAME_CARD_PUSH_THRESHOLD)
    {
        float pushX = (playerCardCount - game_constants::IN_GAME_CARD_PUSH_THRESHOLD) * game_constants::IN_GAME_CARD_PUSH_VALUE * (math::Abs(cardIndex - playerCardCount/2));
        bool oddCardCount = playerCardCount % 2 != 0;
        if ((oddCardCount && cardIndex != playerCardCount/2) || !oddCardCount)
        {
            targetX += (cardIndex < playerCardCount/2) ? pushX : -pushX;
        }
    }
    
    return glm::vec3(targetX, forOpponentPlayer ? game_constants::IN_GAME_TOP_PLAYER_HELD_CARD_Y : game_constants::IN_GAME_BOT_PLAYER_HELD_CARD_Y, game_constants::IN_GAME_HELD_CARD_Z + cardIndex * CARD_INDEX_Z_OFFSET);
}

///------------------------------------------------------------------------------------------------

glm::vec3 CalculateBoardCardPosition(const int cardIndex, const int playerCardCount, bool forOpponentPlayer)
{
    float cardBlockWidth = game_constants::IN_GAME_CARD_ON_BOARD_WIDTH * playerCardCount;
    float cardStartX = -cardBlockWidth/2.0f;
    
    auto targetX = cardStartX + cardIndex * game_constants::IN_GAME_CARD_ON_BOARD_WIDTH + game_constants::IN_GAME_CARD_ON_BOARD_WIDTH/2;
    if (playerCardCount > game_constants::IN_GAME_CARD_PUSH_THRESHOLD)
    {
        float pushX = (playerCardCount - game_constants::IN_GAME_CARD_PUSH_THRESHOLD) * game_constants::IN_GAME_CARD_PUSH_VALUE * (math::Abs(cardIndex - playerCardCount/2));
        bool oddCardCount = playerCardCount % 2 != 0;
        if ((oddCardCount && cardIndex != playerCardCount/2) || !oddCardCount)
        {
            targetX += (cardIndex < playerCardCount/2) ? pushX : -pushX;
        }
    }
    
    return glm::vec3(targetX, forOpponentPlayer ? game_constants::IN_GAME_TOP_PLAYER_BOARD_CARD_Y : game_constants::IN_GAME_BOT_PLAYER_BOARD_CARD_Y, game_constants::IN_GAME_PLAYED_CARD_Z);
}

///------------------------------------------------------------------------------------------------

std::vector<strutils::StringId> GetCardComponentSceneObjectNames(const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation)
{
    if (cardOrientation == CardOrientation::BACK_FACE)
    {
        return std::vector<strutils::StringId>
        {
            strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_BASE_SO_NAME_POST_FIX)
        };
    }
    else
    {
        return std::vector<strutils::StringId>
        {
            strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_BASE_SO_NAME_POST_FIX),
            strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_PORTRAIT_SO_NAME_POST_FIX),
            strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_DAMAGE_ICON_SO_NAME_POST_FIX),
            strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_DAMAGE_TEXT_SO_NAME_POST_FIX),
            strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_NAME_SO_NAME_POST_FIX),
        };
    }
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<CardSoWrapper> CreateCardSoWrapper(const CardData* cardData, const glm::vec3& position, const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation, scene::Scene& scene)
{
    auto cardSoWrapper = std::make_shared<CardSoWrapper>();
    auto& cardComponents = cardSoWrapper->mSceneObjectComponents;
    
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& resService = systemsEngine.GetResourceLoadingService();
    
    const auto& sceneObjectComponentNames = GetCardComponentSceneObjectNames(cardComponentsNamePrefix, cardOrientation);
    
    if (cardOrientation == CardOrientation::BACK_FACE)
    {
        // Create card back
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[0]));
        cardComponents.back()->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_BACK_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_SCALE;
        cardComponents.back()->mBoundingRectMultiplier.x = game_constants::CARD_BOUNDING_RECT_X_MULTIPLIER;
        cardComponents.back()->mPosition = position;
    }
    else
    {
        assert(cardData);
        
        // Create card base
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[0]));
        cardComponents.back()->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_FRAME_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_SCALE;
        cardComponents.back()->mBoundingRectMultiplier.x = game_constants::CARD_BOUNDING_RECT_X_MULTIPLIER;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mRotation.z = math::PI;
        
        // Create portrait
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[1]));
        cardComponents.back()->mTextureResourceId = cardData->mCardTextureResourceId;
        cardComponents.back()->mShaderResourceId = cardData->mCardShaderResourceId;
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_PORTRAIT_SCALE;
        cardComponents.back()->mBoundingRectMultiplier.x = game_constants::CARD_BOUNDING_RECT_X_MULTIPLIER;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_PORTRAIT_Y_OFFSET;
        cardComponents.back()->mPosition.z += game_constants::CARD_COMPONENT_Z_OFFSET;
        
        // Create damage icon
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[2]));
        cardComponents.back()->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_DAMAGE_ICON_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_DAMAGE_ICON_SCALE;
        cardComponents.back()->mBoundingRectMultiplier.x = game_constants::CARD_BOUNDING_RECT_X_MULTIPLIER;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.x += game_constants::IN_GAME_CARD_DAMAGE_ICON_X_OFFSET;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_DAMAGE_ICON_Y_OFFSET;
        cardComponents.back()->mPosition.z += 2 * game_constants::CARD_COMPONENT_Z_OFFSET;
        
        // Create damage
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[3]));
        scene::TextSceneObjectData damageTextData;
        damageTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        damageTextData.mText = std::to_string(cardData->mCardDamage);
        cardComponents.back()->mSceneObjectTypeData = std::move(damageTextData);
        cardComponents.back()->mScale = glm::vec3(game_constants::IN_GAME_CARD_DAMAGE_SCALE);
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.x += game_constants::IN_GAME_CARD_DAMAGE_ICON_X_OFFSET;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_DAMAGE_Y_OFFSET;
        cardComponents.back()->mPosition.z += 3 * game_constants::CARD_COMPONENT_Z_OFFSET;
        
        // Create card name
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[4]));
        scene::TextSceneObjectData cardNameTextData;
        cardNameTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        cardNameTextData.mText = cardData->mCardName;
        cardComponents.back()->mSceneObjectTypeData = std::move(cardNameTextData);
        
        float scaleDeduct = CARD_NAME_TEST_DEDUCT_INCREMENTS;
        float textLength = 0.0f;
        do
        {
            scaleDeduct += CARD_NAME_TEST_DEDUCT_INCREMENTS;
            cardComponents.back()->mScale = glm::vec3(game_constants::IN_GAME_CARD_NAME_SCALE - scaleDeduct);
            cardComponents.back()->mPosition = position + game_constants::IN_GAME_CARD_NAME_X_OFFSET;
            auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*cardComponents.back());
            textLength = boundingRect.topRight.x - boundingRect.bottomLeft.x;
            cardComponents.back()->mPosition.x -= textLength/2.0f;
        } while (textLength > CARD_NAME_AREA_LENGTH);
        
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_NAME_Y_OFFSET;
        cardComponents.back()->mPosition.z += game_constants::CARD_COMPONENT_Z_OFFSET;
    }
    
    cardSoWrapper->mCardData = cardData;
    
    return cardSoWrapper;
}

///------------------------------------------------------------------------------------------------

}
