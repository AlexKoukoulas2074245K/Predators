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

std::vector<std::shared_ptr<scene::SceneObject>> CreateCardComponentSceneObjects(const Card* card, const glm::vec3& position, const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation, scene::Scene& scene)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto& resService = systemsEngine.GetResourceLoadingService();
    
    const auto& sceneObjectComponentNames = GetCardComponentSceneObjectNames(cardComponentsNamePrefix, cardOrientation);
    std::vector<std::shared_ptr<scene::SceneObject>> cardComponents;
    
    if (cardOrientation == CardOrientation::BACK_FACE)
    {
        // Create card back
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[0]));
        cardComponents.back()->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_BACK_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_SCALE;
        cardComponents.back()->mPosition = position;
    }
    else
    {
        assert(card);
        
        // Create card base
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[0]));
        cardComponents.back()->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_FRAME_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_SCALE;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mRotation.z = math::PI;
        
        // Create portrait
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[1]));
        cardComponents.back()->mTextureResourceId = card->mCardTextureResourceId;
        cardComponents.back()->mShaderResourceId = card->mCardShaderResourceId;
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_PORTRAIT_SCALE;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_PORTRAIT_Y_OFFSET;
        cardComponents.back()->mPosition.z += game_constants::CARD_COMPONENT_Z_OFFSET;
        
        // Create damage icon
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[2]));
        cardComponents.back()->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_DAMAGE_ICON_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_DAMAGE_ICON_SCALE;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.x += game_constants::IN_GAME_CARD_DAMAGE_ICON_X_OFFSET;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_DAMAGE_ICON_Y_OFFSET;
        cardComponents.back()->mPosition.z += 2 * game_constants::CARD_COMPONENT_Z_OFFSET;
        
        // Create damage
        cardComponents.push_back(scene.CreateSceneObject(sceneObjectComponentNames[3]));
        scene::TextSceneObjectData damageTextData;
        damageTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        damageTextData.mText = std::to_string(card->mCardDamage);
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
        cardNameTextData.mText = card->mCardName;
        cardComponents.back()->mSceneObjectTypeData = std::move(cardNameTextData);
        
        float scaleDeduct = CARD_NAME_TEST_DEDUCT_INCREMENTS;
        float textLength = 0.0f;
        do
        {
            scaleDeduct += CARD_NAME_TEST_DEDUCT_INCREMENTS;
            cardComponents.back()->mScale = glm::vec3(game_constants::IN_GAME_CARD_NAME_SCALE - scaleDeduct);
            cardComponents.back()->mPosition = position + game_constants::IN_GAME_CARD_NAME_X_OFFSET;
            auto boundingRect = scene_object_utils::GetTextSceneObjectBoundingRect(*cardComponents.back());
            textLength = boundingRect.topRight.x - boundingRect.bottomLeft.x;
            cardComponents.back()->mPosition.x -= textLength/2.0f;
        } while (textLength > CARD_NAME_AREA_LENGTH);
        
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_NAME_Y_OFFSET;
        cardComponents.back()->mPosition.z += game_constants::CARD_COMPONENT_Z_OFFSET;
    }
 
    return cardComponents;
}

///------------------------------------------------------------------------------------------------

}
