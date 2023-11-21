///------------------------------------------------------------------------------------------------
///  ParticleManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ParticleManager_h
#define ParticleManager_h

///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/StringUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }
namespace scene { struct SceneObject; }
namespace scene { struct ParticleEmitterObjectData; }

///------------------------------------------------------------------------------------------------

inline const float DEFAULT_PARTICLE_ENLARGEMENT_SPEED = 0.00001f;

///------------------------------------------------------------------------------------------------

namespace particle_flags
{
    static constexpr uint8_t NONE                           = 0x0;
    static constexpr uint8_t PREFILLED                      = 0x1;
    static constexpr uint8_t CONTINUOUS_PARTICLE_GENERATION = 0x2;
    static constexpr uint8_t ENLARGE_OVER_TIME              = 0x4;
}

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class ParticleManager final
{
    friend struct CoreSystemsEngine::SystemsImpl;
    
public:
    void UpdateSceneParticles(const float dtMilis, scene::Scene& scene);

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
        const strutils::StringId particleEmitterSceneObjectName = strutils::StringId(),
        const float particleEnlargementSpeed = DEFAULT_PARTICLE_ENLARGEMENT_SPEED,
        const float particleGenerationDelaySecs = 0.0f
     );
    void RemoveParticleEmitterFlag(const uint8_t flag, const strutils::StringId particleEmitterSceneObjectName, scene::Scene& scene);

private:
    ParticleManager() = default;
    void SortParticles(scene::ParticleEmitterObjectData& particleEmitterData) const;
    void SpawnParticleAtIndex(const size_t index, const glm::vec3& sceneObjectPosition, scene::ParticleEmitterObjectData& particleEmitterObjectData);
    void SpawnParticleAtIndex(const size_t index, scene::SceneObject& particleEmitterSceneObject);
    void SpawnParticlesAtFirstAvailableSlot(const size_t particlesToSpawnCount, scene::SceneObject& particleEmitterSceneObject);
    
private:
    std::vector<std::shared_ptr<scene::SceneObject>> mParticleEmittersToDelete;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* ParticleManager_h */
