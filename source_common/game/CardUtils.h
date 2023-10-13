///------------------------------------------------------------------------------------------------
///  CardUtils.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 10/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef CardUtils_h
#define CardUtils_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>
#include <game/Cards.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }
namespace scene { struct SceneObject; }
struct CardData;

///------------------------------------------------------------------------------------------------

namespace card_utils
{

///------------------------------------------------------------------------------------------------

glm::vec3 CalculateHeldCardPosition(const int cardIndex, const int playerCardCount, bool forOpponentPlayer);

///------------------------------------------------------------------------------------------------

glm::vec3 CalculateBoardCardPosition(const int cardIndex, const int playerCardCount, bool forOpponentPlayer);

///------------------------------------------------------------------------------------------------

std::vector<strutils::StringId> GetCardComponentSceneObjectNames(const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation);

///------------------------------------------------------------------------------------------------

std::shared_ptr<CardSoWrapper> CreateCardSoWrapper(const CardData* card, const glm::vec3& position, const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation, scene::Scene& scene);

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* CardUtils_h */
