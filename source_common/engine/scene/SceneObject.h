///------------------------------------------------------------------------------------------------
///  SceneObject.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneObject_h
#define SceneObject_h

///------------------------------------------------------------------------------------------------

#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>
#include <game/GameConstants.h>
#include <unordered_map>
#include <variant>

///------------------------------------------------------------------------------------------------

namespace resources { using ResourceId = size_t; }

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

struct DefaultSceneObjectData
{
};

///------------------------------------------------------------------------------------------------

struct TextSceneObjectData
{
    std::string mText;
    strutils::StringId mFontName;
};

///------------------------------------------------------------------------------------------------

struct ParticleEmitterObjectData
{
    size_t mParticleCount;
    uint8_t mParticleFlags;
    
    std::vector<glm::vec3> mParticlePositions;
    std::vector<glm::vec3> mParticleDirections;
    std::vector<float> mParticleLifetimeSecs;
    std::vector<float> mParticleSizes;
    
    glm::vec2 mParticleLifetimeRangeSecs;
    glm::vec2 mParticlePositionXOffsetRange;
    glm::vec2 mParticlePositionYOffsetRange;
    glm::vec2 mParticleSizeRange;
    
    unsigned int mParticleVertexArrayObject;
    unsigned int mParticleVertexBuffer;
    unsigned int mParticleUVBuffer;
    unsigned int mParticlePositionsBuffer;
    unsigned int mParticleLifetimeSecsBuffer;
    unsigned int mParticleSizesBuffer;
};

///------------------------------------------------------------------------------------------------

struct SceneObject
{
    strutils::StringId mName = strutils::StringId();
    std::variant<DefaultSceneObjectData, TextSceneObjectData, ParticleEmitterObjectData> mSceneObjectTypeData;
    std::unordered_map<strutils::StringId, float, strutils::StringIdHasher> mShaderFloatUniformValues;
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mScale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 mBoundingRectMultiplier = glm::vec3(1.0f, 1.0f, 1.0f);
    resources::ResourceId mMeshResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_MESHES_ROOT + game_constants::DEFAULT_MESH_NAME);
    resources::ResourceId mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + game_constants::DEFAULT_TEXTURE_NAME);
    resources::ResourceId mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + game_constants::DEFAULT_SHADER_NAME);
    resources::ResourceId mEffectTextureResourceId = 0;
    bool mInvisible = false;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObject_h */
