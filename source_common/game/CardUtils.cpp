///------------------------------------------------------------------------------------------------
///  CardUtils.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 10/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <game/CardUtils.h>

///------------------------------------------------------------------------------------------------

namespace card_utils
{

///------------------------------------------------------------------------------------------------

static const std::string CARD_FRAME_TEXTURE_FILE_NAME = "card_frame.png";
static const std::string CARD_BACK_TEXTURE_FILE_NAME = "card_back.png";

///------------------------------------------------------------------------------------------------

std::vector<std::shared_ptr<scene::SceneObject>> CreateCardComponentSceneObjects(const Card* card, const glm::vec3& position, const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation, scene::Scene& scene)
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    std::vector<std::shared_ptr<scene::SceneObject>> cardComponents;
    
    if (cardOrientation == CardOrientation::BACK_FACE)
    {
        // Create card back
        cardComponents.push_back(scene.CreateSceneObject(strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_BASE_SO_NAME_POST_FIX)));
        cardComponents.back()->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_BACK_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_SCALE;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mRotation.z = math::PI;
    }
    else
    {
        assert(card);
        
        // Create card base
        cardComponents.push_back(scene.CreateSceneObject(strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_BASE_SO_NAME_POST_FIX)));
        cardComponents.back()->mTextureResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + CARD_FRAME_TEXTURE_FILE_NAME);
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_SCALE;
        cardComponents.back()->mPosition = position;
        
        // Create portrait
        cardComponents.push_back(scene.CreateSceneObject(strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_PORTRAIT_SO_NAME_POST_FIX)));
        cardComponents.back()->mTextureResourceId = card->mCardTextureResourceId;
        cardComponents.back()->mShaderResourceId = card->mCardShaderResourceId;
        cardComponents.back()->mScale.x = cardComponents.back()->mScale.y = game_constants::IN_GAME_CARD_PORTRAIT_SCALE;
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_PORTRAIT_Y_OFFSET;
        cardComponents.back()->mPosition.z += game_constants::CARD_COMPONENT_Z_OFFSET;
        
        // Create damage
        cardComponents.push_back(scene.CreateSceneObject(strutils::StringId(cardComponentsNamePrefix + game_constants::CARD_DAMAGE_SO_NAME_POST_FIX)));
        scene::TextSceneObjectData textData;
        textData.mFontName = game_constants::DEFAULT_FONT_NAME;
        textData.mText = std::to_string(card->mCardDamage);
        cardComponents.back()->mSceneObjectTypeData = std::move(textData);
        cardComponents.back()->mScale = glm::vec3(game_constants::IN_GAME_CARD_DAMAGE_SCALE);
        cardComponents.back()->mPosition = position;
        cardComponents.back()->mPosition.y += game_constants::IN_GAME_CARD_DAMAGE_Y_OFFSET;
        cardComponents.back()->mPosition.z += game_constants::CARD_COMPONENT_Z_OFFSET;
    }
 
    return cardComponents;
}

///------------------------------------------------------------------------------------------------

}
