///------------------------------------------------------------------------------------------------
///  ParticleUpdater.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 18/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Particles.h>
#include <engine/rendering/ParticleUpdater.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <numeric>

#define IS_FLAG_SET(flag) ((particleEmitterData.mParticleFlags & flag) != 0)

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

// Prob needs to be configurable
static const float PARTICLE_ENLARGEMENT_SPEED = 0.00001f;

///------------------------------------------------------------------------------------------------

void ParticleUpdater::UpdateSceneParticles(const float dtMillis, scene::Scene& scene)
{
    mParticleEmitterNamesToDelete.clear();
    for (auto& sceneObject: scene.GetSceneObjects())
    {
        if (std::holds_alternative<scene::ParticleEmitterObjectData>(sceneObject->mSceneObjectTypeData))
        {
            auto& particleEmitterData = std::get<scene::ParticleEmitterObjectData>(sceneObject->mSceneObjectTypeData);
            size_t deadParticles = 0;
            for (size_t i = 0; i < particleEmitterData.mParticleCount; ++i)
            {
                // subtract from the particles lifetime
                particleEmitterData.mParticleLifetimeSecs[i] -= dtMillis/1000.0f;
                
                // if the lifetime is below add to the count of finished particles
                if (particleEmitterData.mParticleLifetimeSecs[i] <= 0.0f )
                {
                    if (IS_FLAG_SET(particle_flags::CONTINUOUS_PARTICLE_GENERATION))
                    {
                        SpawnParticleAtIndex(i, sceneObject->mPosition, particleEmitterData);
                    }
                    else
                    {
                        particleEmitterData.mParticleLifetimeSecs[i] = 0.0f;
                        deadParticles++;
                    }
                }
                
                // move the particle up depending on the delta time
                particleEmitterData.mParticleSizes[i] += PARTICLE_ENLARGEMENT_SPEED * dtMillis;
            }
            
            if (deadParticles == particleEmitterData.mParticleCount)
            {
                mParticleEmitterNamesToDelete.push_back(sceneObject->mName);
            }
            else
            {
                SortParticles(particleEmitterData);
            }
        }
    }
    
    for (const auto& particleEmitterName: mParticleEmitterNamesToDelete)
    {
        scene.RemoveSceneObject(particleEmitterName);
    }
}

///------------------------------------------------------------------------------------------------

void ParticleUpdater::SortParticles(scene::ParticleEmitterObjectData& particleEmitterData) const
{
    // Create permutation index vector for final positions
    const auto particleCount = particleEmitterData.mParticleCount;
    
    std::vector<std::size_t> indexVec(particleCount);
    std::iota(indexVec.begin(), indexVec.end(), 0);
    std::sort(indexVec.begin(), indexVec.end(), [&](const size_t i, const size_t j)
    {
        return particleEmitterData.mParticlePositions[i].z < particleEmitterData.mParticlePositions[j].z;
    });
    
    // Create corrected vectors
    std::vector<glm::vec3> correctedPositions(particleCount);
    std::vector<glm::vec3> correctedDirections(particleCount);
    std::vector<float> correctedLifetimes(particleCount);
    std::vector<float> correctedSizes(particleCount);
    
    for (size_t i = 0U; i < particleCount; ++i)
    {
        correctedPositions[i]  = particleEmitterData.mParticlePositions[indexVec[i]];
        correctedDirections[i] = particleEmitterData.mParticleDirections[indexVec[i]];
        correctedLifetimes[i]  = particleEmitterData.mParticleLifetimeSecs[indexVec[i]];
        correctedSizes[i]      = particleEmitterData.mParticleSizes[indexVec[i]];
    }
    
    particleEmitterData.mParticlePositions = std::move(correctedPositions);
    particleEmitterData.mParticleDirections = std::move(correctedDirections);
    particleEmitterData.mParticleLifetimeSecs = std::move(correctedLifetimes);
    particleEmitterData.mParticleSizes     = std::move(correctedSizes);
}

///------------------------------------------------------------------------------------------------

}
