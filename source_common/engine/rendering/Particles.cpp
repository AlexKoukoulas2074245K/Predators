///------------------------------------------------------------------------------------------------
///  Particles.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 17/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/OpenGL.h>
#include <engine/rendering/Particles.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>

#define IS_FLAG_SET(flag) ((particleEmitterData.mParticleFlags & flag) != 0)

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

static const std::vector<std::vector<float>> PARTICLE_VERTEX_POSITIONS =
{
//    {
//        0.0f, 0.0f, 0.0f,
//        1.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 1.0f,
//        1.0f, 0.0f, 1.0f
//    },
//
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    }
};

static const std::vector<float> PARTICLE_UVS =
{
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};

static int sParticleEmitterCount = 0;
static std::string PARTICLE_EMITTER_NAME_PREFIX = "PARTICLE_EMITTER_";
static std::string PARTICLE_SHADER_FILE_NAME = "generic_particle.vs";

///------------------------------------------------------------------------------------------------

void SpawnParticleAtIndex(const size_t index, const glm::vec3& sceneObjectPosition, scene::ParticleEmitterObjectData& particleEmitterData)
{
    const auto lifeTime = math::RandomFloat(particleEmitterData.mParticleLifetimeRangeSecs.s, particleEmitterData.mParticleLifetimeRangeSecs.t);
    const auto xOffset = math::RandomFloat(particleEmitterData.mParticlePositionXOffsetRange.s, particleEmitterData.mParticlePositionXOffsetRange.t);
    const auto yOffset = math::RandomFloat(particleEmitterData.mParticlePositionYOffsetRange.s, particleEmitterData.mParticlePositionYOffsetRange.t);
    const auto zOffset = math::RandomFloat(sceneObjectPosition.z - sceneObjectPosition.z * 0.1f, sceneObjectPosition.z + sceneObjectPosition.z * 0.1f);
    const auto size = math::RandomFloat(particleEmitterData.mParticleSizeRange.s, particleEmitterData.mParticleSizeRange.t);
    
    particleEmitterData.mParticleLifetimeSecs[index] = lifeTime;
    particleEmitterData.mParticlePositions[index] = sceneObjectPosition;
    particleEmitterData.mParticlePositions[index].x += xOffset;
    particleEmitterData.mParticlePositions[index].y += yOffset;
    particleEmitterData.mParticlePositions[index].z += zOffset;
    particleEmitterData.mParticleDirections[index] = glm::normalize(glm::vec3(xOffset, yOffset, 0.0f));
    particleEmitterData.mParticleSizes[index] = size;
    
}

///------------------------------------------------------------------------------------------------

void SpawnParticleAtIndex(const size_t index, scene::SceneObject& particleEmitterSceneObject)
{
    if (std::holds_alternative<scene::ParticleEmitterObjectData>(particleEmitterSceneObject.mSceneObjectTypeData))
    {
        SpawnParticleAtIndex(index, particleEmitterSceneObject.mPosition, std::get<scene::ParticleEmitterObjectData>(particleEmitterSceneObject.mSceneObjectTypeData));
    }
}

///------------------------------------------------------------------------------------------------

void SpawnParticlesAtFirstAvailableSlot(const size_t particlesToSpawnCount, scene::SceneObject& particleEmitterSceneObject)
{
    if (std::holds_alternative<scene::ParticleEmitterObjectData>(particleEmitterSceneObject.mSceneObjectTypeData))
    {
        const auto& particleEmitterData = std::get<scene::ParticleEmitterObjectData>(particleEmitterSceneObject.mSceneObjectTypeData);
        
        auto particlesToSpawn = particlesToSpawnCount;
        auto particleCount = particleEmitterData.mParticlePositions.size();
        
        for (size_t i = 0; i < particleCount && particlesToSpawn > 0; ++i)
        {
            if (particleEmitterData.mParticleLifetimeSecs[i] <= 0.0f)
            {
                SpawnParticleAtIndex(i, particleEmitterSceneObject);
                particlesToSpawn--;
            }
        }
    }
    
}

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
    const uint8_t particleFlags, /* = particle_flags::NONE */
    const strutils::StringId particleEmitterName /* = strutils::StringId() */
)
{
    auto particleSystemSo = scene.CreateSceneObject(particleEmitterName.isEmpty() ? strutils::StringId(PARTICLE_EMITTER_NAME_PREFIX + std::to_string(sParticleEmitterCount)) : particleEmitterName);
    particleSystemSo->mPosition = pos;
    particleSystemSo->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + particleTextureFilename);
    particleSystemSo->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + PARTICLE_SHADER_FILE_NAME);
    
    scene::ParticleEmitterObjectData particleEmitterData;
    particleEmitterData.mParticleCount = particleCount;
    particleEmitterData.mParticleFlags = particleFlags;
    
    assert(IS_FLAG_SET(particle_flags::PREFILLED) || IS_FLAG_SET(particle_flags::CONTINUOUS_PARTICLE_GENERATION));
    
    particleEmitterData.mParticleLifetimeSecs.resize(particleCount);
    particleEmitterData.mParticleDirections.resize(particleCount);
    particleEmitterData.mParticleSizes.resize(particleCount);
    particleEmitterData.mParticlePositions.resize(particleCount);
    
    particleEmitterData.mParticleLifetimeRangeSecs = particleLifetimeRangeSecs;
    particleEmitterData.mParticlePositionXOffsetRange = particlePositionXOffsetRange;
    particleEmitterData.mParticlePositionYOffsetRange = particlePositionYOffsetRange;
    particleEmitterData.mParticleSizeRange = particleSizeRange;
    
    for (size_t i = 0U; i < particleCount; ++i)
    {
        particleEmitterData.mParticleLifetimeSecs[i] = 0.0f;
        
        if (IS_FLAG_SET(particle_flags::PREFILLED))
        {
            SpawnParticleAtIndex(i, pos, particleEmitterData);
        }
    }
    
    GL_CALL(glGenVertexArrays(1, &particleEmitterData.mParticleVertexArrayObject));
    GL_CALL(glGenBuffers(1, &particleEmitterData.mParticleVertexBuffer));
    GL_CALL(glGenBuffers(1, &particleEmitterData.mParticleUVBuffer));
    GL_CALL(glGenBuffers(1, &particleEmitterData.mParticlePositionsBuffer));
    GL_CALL(glGenBuffers(1, &particleEmitterData.mParticleLifetimeSecsBuffer));
    GL_CALL(glGenBuffers(1, &particleEmitterData.mParticleSizesBuffer));
    
    GL_CALL(glBindVertexArray(particleEmitterData.mParticleVertexArrayObject));
    
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleVertexBuffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, PARTICLE_VERTEX_POSITIONS[0].size() * sizeof(float) , PARTICLE_VERTEX_POSITIONS[0].data(), GL_STATIC_DRAW));
    
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleUVBuffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, PARTICLE_UVS.size() * sizeof(float) , PARTICLE_UVS.data(), GL_STATIC_DRAW));

    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticlePositionsBuffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, particleCount * sizeof(glm::vec3), particleEmitterData.mParticlePositions.data(), GL_DYNAMIC_DRAW));
    
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleLifetimeSecsBuffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, particleCount * sizeof(float), particleEmitterData.mParticleLifetimeSecs.data(), GL_DYNAMIC_DRAW));
    
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleSizesBuffer));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, particleCount * sizeof(float), particleEmitterData.mParticleSizes.data(), GL_DYNAMIC_DRAW));
    
    particleSystemSo->mSceneObjectTypeData = std::move(particleEmitterData);
    
    sParticleEmitterCount++;
    
    return particleSystemSo;
}

///------------------------------------------------------------------------------------------------

}
