///------------------------------------------------------------------------------------------------
///  AnimatedStatContainer.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef AnimatedStatContainer_h
#define AnimatedStatContainer_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }
namespace scene { struct SceneObject; }

///------------------------------------------------------------------------------------------------

enum class AnimatedStatContainerUpdateResult
{
    FINISHED, ONGOING
};

///------------------------------------------------------------------------------------------------

class AnimatedStatContainer final
{
public:
    AnimatedStatContainer
    (
        const glm::vec3& position,
        const std::string& textureFilename,
        const std::string& crystalName,
        const int& valueToTrack,
        const bool startHidden,
        scene::Scene& scene
    );
    ~AnimatedStatContainer();
    
    AnimatedStatContainerUpdateResult Update(const float dtMillis);
    
    std::vector<std::shared_ptr<scene::SceneObject>>& GetSceneObjects();
    
private:
    const int& mValueToTrack;
    int mDisplayedValue;
    float mValueChangeDelaySecs;
    scene::Scene& mScene;
    std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjects;
    bool mFinishedAnimating;
};

///------------------------------------------------------------------------------------------------

#endif /* AnimatedStatContainer_h */
