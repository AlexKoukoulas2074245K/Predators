///------------------------------------------------------------------------------------------------
///  StoryNodeMap.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 19/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/GameConstants.h>
#include <game/ProgressionDataRepository.h>
#include <game/StoryNodeMap.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <unordered_set>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

static const std::unordered_map<StoryNodeMap::NodeType, std::string> MAP_NODE_TYPES_TO_PORTRAIT_TEXTURES =
{
    { StoryNodeMap::NodeType::NORMAL_ENCOUNTER, "map_node_normal.png" },
    { StoryNodeMap::NodeType::ELITE_ENCOUNTER, "map_node_elite.png" },
    { StoryNodeMap::NodeType::BOSS_ENCOUNTER, "map_node_boss.png" },
    { StoryNodeMap::NodeType::EVENT, "map_node_misc.png" },
    { StoryNodeMap::NodeType::SHOP, "map_node_misc.png" },
    { StoryNodeMap::NodeType::STARTING_LOCATION, "teepee.png" },
};

static const strutils::StringId ANIMATED_NODE_PATH_PARTICLE_EMITTER_NAME = strutils::StringId("node_path_animated");
static const strutils::StringId STATIC_NODE_PATH_PARTICLE_EMITTER_NAME = strutils::StringId("node_path_static");

static const std::string NODE_PATH_TEXTURE_FILE_NAME = "trap_mask.png";

static const glm::vec3 FIRST_NODE_POSITION = { -0.8f, -0.63f, 0.1f };
static const glm::vec3 LAST_NODE_POSITION = { 0.46f, 0.53f, 0.1f };

#if defined(NDEBUG)
static const float NODES_CLOSE_ENOUGH_THRESHOLD = 0.025f;
static const float NODES_CLOSE_ENOUGH_TO_EDGE_NODES_THRESHOLD = 0.075f;
#else
static const float NODES_CLOSE_ENOUGH_THRESHOLD = 0.025f;
static const float NODES_CLOSE_ENOUGH_TO_EDGE_NODES_THRESHOLD = 0.075f;
#endif
static const float NODE_GENERATION_POSITION_NOISE = 0.1f;
static const float NODE_POSITION_Z = 0.1f;
static const float NODE_PATH_POSITION_Z = 0.01f;
static const float NODE_SCALE = 0.15f;
static const float NODE_PATH_SCALE = 0.015f;
static const float MAX_NODE_PATH_SCALE = 0.04f;
static const float MIN_NODE_PATH_SCALE = 0.025f;
static const float NODE_PATH_INIT_SCALE_SEPARATOR = 0.002f;
static const float NODE_PATH_Z_SEPARATOR = 0.0001f;
static const float NODE_PATH_SCALE_SPEED = 0.00003f;
static const float INACTIVE_NODE_PATH_LIFETIME_SECS = 0.85f;
static const float SELECTABLE_NODE_BOUNCE_SPEED_Y = 0.000005f;

static const int MAP_PATH_SEGMENTS_FACTOR = 30;
static const int MAP_GENERATION_PASSES = 5;
static const int MAX_MAP_GENERATION_ATTEMPTS = 100000;

int mapGenerationAttempts = 0;

///------------------------------------------------------------------------------------------------

StoryNodeMap::StoryNodeMap(std::shared_ptr<scene::Scene> scene, const int mapGenerationSeed, const glm::ivec2& mapDimensions, const MapCoord& currentMapCoord, const bool singleEntryPoint)
    : mScene(scene)
    , mMapDimensions(mapDimensions)
    , mCurrentMapCoord(currentMapCoord)
    , mMapGenerationSeed(mapGenerationSeed)
    , mHasSingleEntryPoint(singleEntryPoint)
    , mMapGenerationAttemptsRemaining(MAX_MAP_GENERATION_ATTEMPTS)
    , mHasCreatedSceneObjects(false)
{
}

///------------------------------------------------------------------------------------------------

void StoryNodeMap::GenerateMapNodes()
{
    CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddArtificialLoadingJobCount(mMapGenerationAttemptsRemaining);
    math::SetControlSeed(mMapGenerationSeed);
    GenerateMapData();
    CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddArtificialLoadingJobCount(-mMapGenerationAttemptsRemaining);
}

///------------------------------------------------------------------------------------------------

int StoryNodeMap::GetCurrentGenerationSeed() const
{
    return mMapGenerationSeed;
}

///------------------------------------------------------------------------------------------------

bool StoryNodeMap::HasCreatedSceneObjects() const
{
    return mHasCreatedSceneObjects;
}

///------------------------------------------------------------------------------------------------

const std::map<MapCoord, StoryNodeMap::NodeData>& StoryNodeMap::GetMapData() const
{
    return mMapData;
}

///------------------------------------------------------------------------------------------------

const glm::ivec2& StoryNodeMap::GetMapDimensions() const
{
    return mMapDimensions;
}

///------------------------------------------------------------------------------------------------

void StoryNodeMap::GenerateMapData()
{
    do
    {
        mMapGenerationAttemptsRemaining--;
        CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddArtificialLoadingJobCount(-1);
        
        mapGenerationAttempts++;
        mMapData.clear();
        
        for (int i = 0; i < MAP_GENERATION_PASSES; ++i)
        {
            auto currentCoordinate = mHasSingleEntryPoint ? MapCoord(0, mMapDimensions.y/2) : MapCoord(0, math::ControlledRandomInt(0, mMapDimensions.y - 1));
            mMapData[currentCoordinate].mPosition = GenerateNodePositionForCoord(currentCoordinate);
            mMapData[currentCoordinate].mNodeType = SelectNodeTypeForCoord(currentCoordinate);
            
            for (int col = 1; col < mMapDimensions.x; ++col)
            {
                MapCoord targetCoord = RandomlySelectNextMapCoord(currentCoordinate);
                
                while (DetectedCrossedEdge(currentCoordinate, targetCoord))
                {
                    targetCoord = RandomlySelectNextMapCoord(currentCoordinate);
                }
                
                mMapData[currentCoordinate].mNodeLinks.insert(targetCoord);
                currentCoordinate = targetCoord;
                mMapData[currentCoordinate].mPosition = GenerateNodePositionForCoord(currentCoordinate);
                mMapData[currentCoordinate].mNodeType = SelectNodeTypeForCoord(currentCoordinate);
            }
        }
    } while (FoundCloseEnoughNodes() && mMapGenerationAttemptsRemaining > 0);
}

///------------------------------------------------------------------------------------------------

void StoryNodeMap::DestroyParticleEmitters()
{
    mScene->RemoveSceneObject(STATIC_NODE_PATH_PARTICLE_EMITTER_NAME);
    mScene->RemoveSceneObject(ANIMATED_NODE_PATH_PARTICLE_EMITTER_NAME);
}

///------------------------------------------------------------------------------------------------

bool StoryNodeMap::FoundCloseEnoughNodes() const
{
    for (auto& mapNodeEntry: mMapData)
    {
        if (mapNodeEntry.first.mCol == 0 || mapNodeEntry.first.mCol == mMapDimensions.x - 1)
        {
            continue;
        }
        
        if (math::Distance2(mMapData.at(MapCoord(0, 2)).mPosition, mapNodeEntry.second.mPosition) < NODES_CLOSE_ENOUGH_TO_EDGE_NODES_THRESHOLD)
        {
            return true;
        }
        
        if (math::Distance2(mMapData.at(MapCoord(mMapDimensions.x - 1, 2)).mPosition, mapNodeEntry.second.mPosition) < NODES_CLOSE_ENOUGH_TO_EDGE_NODES_THRESHOLD)
        {
            return true;
        }
        
        for (const auto& otherMapNodeEntry: mMapData)
        {
            if (otherMapNodeEntry.first == mapNodeEntry.first)
            {
                continue;
            }
            
            if (math::Distance2(otherMapNodeEntry.second.mPosition, mapNodeEntry.second.mPosition) < NODES_CLOSE_ENOUGH_THRESHOLD)
            {
                return true;
            }
        }
    }
    
    return false;
}

///------------------------------------------------------------------------------------------------

void StoryNodeMap::CreateMapSceneObjects()
{
    auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
    auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    
    // All node meshes
    for (const auto& mapNodeEntry: mMapData)
    {
        auto nodeSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString()));
        nodeSceneObject->mPosition = mapNodeEntry.second.mPosition;
        nodeSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + MAP_NODE_TYPES_TO_PORTRAIT_TEXTURES.at(mapNodeEntry.second.mNodeType));
        nodeSceneObject->mScale = glm::vec3(NODE_SCALE);
        
        auto nodeTextSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + "_text"));
        scene::TextSceneObjectData textData;
        textData.mFontName = game_constants::DEFAULT_FONT_NAME;
        
        switch (mapNodeEntry.second.mNodeType)
        {
            case NodeType::STARTING_LOCATION:
            {
                textData.mText = "Teepee";
            } break;
                
            case NodeType::ELITE_ENCOUNTER:
            {
                textData.mText = "Elite";
            } break;
                
            case NodeType::NORMAL_ENCOUNTER:
            {
                textData.mText = "Normal";
            } break;
            
            case NodeType::EVENT:
            {
                textData.mText = "Event";
            } break;
                
            case NodeType::SHOP:
            {
                textData.mText = "Shop";
            } break;
                
            case NodeType::BOSS_ENCOUNTER:
            {
                textData.mText = "Boss";
            } break;
                
            default: break;
        }
        
        nodeTextSceneObject->mSceneObjectTypeData = std::move(textData);
        nodeTextSceneObject->mScale = {0.0002f, 0.0002f, 0.0002f};
        nodeTextSceneObject->mPosition = mapNodeEntry.second.mPosition;
        nodeTextSceneObject->mPosition.x -= 0.02f;
        nodeTextSceneObject->mPosition.z += 0.1f;
        
        // Add also pulsing animation if node is active
        if (mMapData.at(mCurrentMapCoord).mNodeLinks.count(mapNodeEntry.first))
        {
            auto randomDelaySecsOffset = math::RandomFloat(0.0f, 1.0f);
            animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeTextSceneObject, glm::vec3(0.0f, SELECTABLE_NODE_BOUNCE_SPEED_Y, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeSceneObject, glm::vec3(0.0f, SELECTABLE_NODE_BOUNCE_SPEED_Y, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
        }
    }
    
    auto& particleManager = CoreSystemsEngine::GetInstance().GetParticleManager();
    auto animatedNodePathParticleEmitterSceneObject = particleManager.CreateParticleEmitterAtPosition(ANIMATED_NODE_PATH_PARTICLE_EMITTER_NAME, glm::vec3(), *mScene, ANIMATED_NODE_PATH_PARTICLE_EMITTER_NAME, [](float dtMillis, scene::ParticleEmitterObjectData& particleEmitterData)
    {
        for (size_t i = 0; i < particleEmitterData.mParticleCount; ++i)
        {
            if (particleEmitterData.mParticleAngles[i] > 0.0f)
            {
                particleEmitterData.mParticleSizes[i] += dtMillis * NODE_PATH_SCALE_SPEED;
                if (particleEmitterData.mParticleSizes[i] > MAX_NODE_PATH_SCALE)
                {
                    particleEmitterData.mParticleAngles[i] = -1.0f;
                }
            }
            else
            {
                particleEmitterData.mParticleSizes[i] -= dtMillis * NODE_PATH_SCALE_SPEED;
                if (particleEmitterData.mParticleSizes[i] < MIN_NODE_PATH_SCALE)
                {
                    particleEmitterData.mParticleAngles[i] = 1.0f;
                }
            }
        }
    });
    auto staticNodePathParticleEmitterSceneObject = particleManager.CreateParticleEmitterAtPosition(STATIC_NODE_PATH_PARTICLE_EMITTER_NAME, glm::vec3(), *mScene, STATIC_NODE_PATH_PARTICLE_EMITTER_NAME, [](float, scene::ParticleEmitterObjectData& particleEmitterData)
    {
        for (size_t i = 0; i < particleEmitterData.mParticleCount; ++i)
        {
            particleEmitterData.mParticleLifetimeSecs[i] = INACTIVE_NODE_PATH_LIFETIME_SECS;
        }
        
    });
    
    for (const auto& mapNodeEntry: mMapData)
    {
        for (const auto& linkedCoord: mapNodeEntry.second.mNodeLinks)
        {
            bool isPartOfEligiblePath = mapNodeEntry.first == mCurrentMapCoord;
            glm::vec3 dirToNext = mMapData.at(linkedCoord).mPosition - mMapData.at(mapNodeEntry.first).mPosition;
            dirToNext.z = 0.0f;
            
            auto pathSegments = static_cast<int>(MAP_PATH_SEGMENTS_FACTOR * glm::length(dirToNext));
            for (int i = 0; i < pathSegments; ++i)
            {
                
                auto& emitterToUse = isPartOfEligiblePath ? animatedNodePathParticleEmitterSceneObject : staticNodePathParticleEmitterSceneObject;
                auto indexSpawnedAt = particleManager.SpawnParticleAtFirstAvailableSlot(*emitterToUse);
                assert(indexSpawnedAt != -1);
                if (indexSpawnedAt == -1)
                {
                    continue;
                }
                
                auto& particleEmitterData = std::get<scene::ParticleEmitterObjectData>(emitterToUse->mSceneObjectTypeData);
                particleEmitterData.mParticleSizes[indexSpawnedAt] = isPartOfEligiblePath ? NODE_PATH_SCALE + (pathSegments - i) * NODE_PATH_INIT_SCALE_SEPARATOR : MIN_NODE_PATH_SCALE;
                particleEmitterData.mParticleAngles[indexSpawnedAt] = 1.0f; // signifies > 0.0f -> scale up, < 0.0f -> scale down
                particleEmitterData.mParticlePositions[indexSpawnedAt] = mMapData.at(mapNodeEntry.first).mPosition + dirToNext * (i/static_cast<float>(pathSegments));
                particleEmitterData.mParticlePositions[indexSpawnedAt].z = NODE_PATH_POSITION_Z + indexSpawnedAt * NODE_PATH_Z_SEPARATOR;
            }
        }
    }
    
    mHasCreatedSceneObjects = true;
}

///------------------------------------------------------------------------------------------------

bool StoryNodeMap::DetectedCrossedEdge(const MapCoord& currentCoord, const MapCoord& targetTestCoord) const
{
    bool currentCoordHasTopNeighbor = currentCoord.mRow > 0;
    bool currentCoordHasBotNeighbor = currentCoord.mRow < mMapDimensions.y - 1;
    bool targetCoordHasTopNeighbor = targetTestCoord.mRow > 0;
    bool targetCoordHasBotNeighbor = targetTestCoord.mRow < mMapDimensions.y - 1;
    
    if (currentCoordHasTopNeighbor && targetCoordHasBotNeighbor)
    {
        MapCoord currentTopNeighbor(currentCoord.mCol, currentCoord.mRow - 1);
        if (mMapData.count(currentTopNeighbor) && mMapData.at(currentTopNeighbor).mNodeLinks.count(MapCoord(targetTestCoord.mCol, targetTestCoord.mRow + 1))) return true;
    }
    if (currentCoordHasBotNeighbor && targetCoordHasTopNeighbor)
    {
        MapCoord currentBotNeighbor(currentCoord.mCol, currentCoord.mRow + 1);
        if (mMapData.count(currentBotNeighbor) && mMapData.at(currentBotNeighbor).mNodeLinks.count(MapCoord(targetTestCoord.mCol, targetTestCoord.mRow - 1))) return true;
    }
    
    return false;
}

///------------------------------------------------------------------------------------------------

glm::vec3 StoryNodeMap::GenerateNodePositionForCoord(const MapCoord& mapCoord) const
{
    if (mapCoord.mCol == 0)
    {
        return FIRST_NODE_POSITION;
    }
    else if (mapCoord.mCol == mMapDimensions.x - 1)
    {
        return LAST_NODE_POSITION;
    }
    else
    {
        auto lastToFirstDirection = LAST_NODE_POSITION - FIRST_NODE_POSITION;
        lastToFirstDirection.z = 0.0f;
        
        auto t = 0.05f + mapCoord.mCol/static_cast<float>(mMapDimensions.x);
        
        auto lineOriginPosition = FIRST_NODE_POSITION + t * lastToFirstDirection;
        
        glm::vec3 resultPosition = lineOriginPosition + glm::vec3
        (
            0.1f + 0.2f * (mapCoord.mRow - static_cast<float>(mMapDimensions.y/2)),
            -0.15f * (mapCoord.mRow - static_cast<float>(mMapDimensions.y/2)),
            NODE_POSITION_Z
        );
        
        resultPosition.x += math::ControlledRandomFloat(-NODE_GENERATION_POSITION_NOISE, NODE_GENERATION_POSITION_NOISE);
        resultPosition.y += math::ControlledRandomFloat(-NODE_GENERATION_POSITION_NOISE, NODE_GENERATION_POSITION_NOISE);
        return resultPosition;
    }
}

///------------------------------------------------------------------------------------------------

StoryNodeMap::NodeType StoryNodeMap::SelectNodeTypeForCoord(const MapCoord& mapCoord) const
{
    // Forced single entry point and starting coord case
    if (mHasSingleEntryPoint && mapCoord == MapCoord(0, mMapDimensions.y/2))
    {
        return NodeType::STARTING_LOCATION;
    }
    // Last map coord
    else if (mapCoord == MapCoord(mMapDimensions.x - 1, mMapDimensions.y/2))
    {
        return NodeType::BOSS_ENCOUNTER;
    }
    else if (mapCoord.mCol == mMapDimensions.x - 2)
    {
        return NodeType::SHOP;
    }
    else
    {
        // Generate list of node types to pick from
        std::unordered_set<NodeType> availableNodeTypes;
        for (int i = 0; i < static_cast<int>(NodeType::COUNT); ++i)
        {
            availableNodeTypes.insert(static_cast<NodeType>(i));
        }
        
        // Only first node is a starting location
        availableNodeTypes.erase(NodeType::STARTING_LOCATION);
        
        // Only last node can have a boss encounter
        availableNodeTypes.erase(NodeType::BOSS_ENCOUNTER);
        
        // Second node can not have a shop
        if (mapCoord.mCol == 1)
        {
            availableNodeTypes.erase(NodeType::SHOP);
        }
         
        // Remove any node types from the immediate previous links except if they are
        // normal encounters or events
        for (const auto& mapEntry: mMapData)
        {
            
            if (mapEntry.second.mNodeLinks.count(mapCoord))
            {
                availableNodeTypes.erase(mapEntry.second.mNodeType);
            }
        }
        
        // Select at random from the remaining node types.
        // Unfortunately because it's a set I can't just pick begin() + random index
        auto randomIndex = math::ControlledRandomInt(0, static_cast<int>(availableNodeTypes.size()) - 1);
        for (const auto& nodeType: availableNodeTypes)
        {
            if (randomIndex-- == 0) return nodeType;
        }
    }
    
    return NodeType::NORMAL_ENCOUNTER;
}

///------------------------------------------------------------------------------------------------

MapCoord StoryNodeMap::RandomlySelectNextMapCoord(const MapCoord& mapCoord) const
{
    auto randRow = math::Max(math::Min(mMapDimensions.y - 1, mapCoord.mRow + math::ControlledRandomInt(-1, 1)), 0);
    return mapCoord.mCol == mMapDimensions.x - 2 ? MapCoord(mMapDimensions.x - 1, mMapDimensions.y/2) : MapCoord(mapCoord.mCol + 1, randRow);
}

///------------------------------------------------------------------------------------------------
