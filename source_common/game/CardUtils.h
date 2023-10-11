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
struct Card;

///------------------------------------------------------------------------------------------------

namespace card_utils
{

///------------------------------------------------------------------------------------------------

std::vector<strutils::StringId> GetCardComponentSceneObjectNames(const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation);

///------------------------------------------------------------------------------------------------

std::vector<std::shared_ptr<scene::SceneObject>> CreateCardComponentSceneObjects(const Card* card, const glm::vec3& position, const std::string& cardComponentsNamePrefix, const CardOrientation cardOrientation, scene::Scene& scene);

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* CardUtils_h */
