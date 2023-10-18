///------------------------------------------------------------------------------------------------
///  Particles.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef Particles_h
#define Particles_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>
#include <memory>

///------------------------------------------------------------------------------------------------

namespace scene { struct SceneObject; }
namespace scene { class Scene; }
namespace scene { struct ParticleEmitterObjectData; }

///------------------------------------------------------------------------------------------------

namespace particle_flags
{
    static constexpr uint8_t NONE                           = 0x0;
    static constexpr uint8_t PREFILLED                      = 0x1;
    static constexpr uint8_t CONTINUOUS_PARTICLE_GENERATION = 0x2;
}

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

void SpawnParticleAtIndex(const size_t index, const glm::vec3& sceneObjectPosition, scene::ParticleEmitterObjectData& particleEmitterObjectData);

///------------------------------------------------------------------------------------------------

void SpawnParticleAtIndex(const size_t index, scene::SceneObject& particleEmitterSceneObject);

///------------------------------------------------------------------------------------------------

void SpawnParticlesAtFirstAvailableSlot(const int particlesToSpawnCount, scene::SceneObject& particleEmitterSceneObject);

///------------------------------------------------------------------------------------------------

std::shared_ptr<scene::SceneObject> CreateParticleEmitterAtPosition
(
    const glm::vec3& pos,
    const glm::vec2& particleLifetimeRangeSecs,
    const glm::vec2& particlePositionXOffsetRange,
    const glm::vec2& particlePositionYOffsetRange,
    const glm::vec2& particleSizeRange,
    const size_t particleCount,
    const std::string& particleTextureFilename,
    scene::Scene& scene,
    const uint8_t particleFlags = particle_flags::NONE,
    const strutils::StringId particleEmitterName = strutils::StringId()
 );

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Particles_h */
