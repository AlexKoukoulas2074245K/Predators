///------------------------------------------------------------------------------------------------
///  AnimatedStatCrystal.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef AnimatedStatCrystal_h
#define AnimatedStatCrystal_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }
namespace scene { struct SceneObject; }

///------------------------------------------------------------------------------------------------

enum class AnimatedStatCrystalUpdateResult
{
    FINISHED, ONGOING
};

///------------------------------------------------------------------------------------------------

class AnimatedStatCrystal final
{
public:
    AnimatedStatCrystal(const glm::vec3& position, const std::string& textureFilename, const std::string& crystalName, const int& valueToTrack, scene::Scene& scene);
    ~AnimatedStatCrystal();
    
    AnimatedStatCrystalUpdateResult Update(const float dtMillis);
    
private:
    const int& mValueToTrack;
    int mDisplayedValue;
    float mValueChangeDelaySecs;
    scene::Scene& mScene;
    std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjects;
};

///------------------------------------------------------------------------------------------------

#endif /* AnimatedStatCrystal_h */
