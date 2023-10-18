///------------------------------------------------------------------------------------------------
///  ParticleUpdater.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ParticleUpdater_h
#define ParticleUpdater_h

///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/StringUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }
namespace scene { struct ParticleEmitterObjectData; }

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class ParticleUpdater final
{
    friend struct CoreSystemsEngine::SystemsImpl;
public:
    void UpdateSceneParticles(const float dtMilis, scene::Scene& scene);
    
private:
    ParticleUpdater() = default;
    void SortParticles(scene::ParticleEmitterObjectData& particleEmitterData) const;
    
private:
    std::vector<strutils::StringId> mParticleEmitterNamesToDelete;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* ParticleUpdater_h */
