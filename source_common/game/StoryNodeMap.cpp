///------------------------------------------------------------------------------------------------
///  StoryNodeMap.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 19/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/GameConstants.h>
#include <game/ProgressionDataRepository.h>
#include <game/StoryNodeMap.h>
#include <game/utils/DemonNameGenerator.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
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

static const std::vector<std::string> EASY_FIGHT_TEXTURES =
{
    "story_cards/baby_demon.png",
    "story_cards/small_imp.png",
    "story_cards/youngster_imp_puppy.png",
    "story_cards/red_youngster_imp_puppy.png"
};

static const std::vector<std::string> MEDIUM_FIGHT_TEXTURES =
{
    "story_cards/young_adult_blue_demon.png",
    "story_cards/red_young_adult_demon.png"
};

static const std::vector<std::string> HARD_FIGHT_TEXTURES =
{
    "story_cards/elite_demon_0.png",
    "story_cards/elite_demon_1.png",
    "story_cards/elite_demon_2.png",
    "story_cards/elite_demon_3.png",
    "story_cards/elite_demon_4.png",
    "story_cards/elite_demon_5.png",
    "story_cards/elite_demon_6.png"
};

static const std::vector<std::string> BOSS_FIGHT_TEXTURES =
{
    "story_cards/demon_boss_0.png",
    "story_cards/demon_boss_1.png",
    "story_cards/demon_boss_2.png",
    "story_cards/demon_boss_3.png",
    "story_cards/demon_boss_4.png",
    "story_cards/demon_boss_5.png",
    "story_cards/demon_boss_6.png",
    "story_cards/demon_boss_7.png"
};

static const strutils::StringId ANIMATED_NODE_PATH_PARTICLE_EMITTER_NAME = strutils::StringId("node_path_animated");
static const strutils::StringId STATIC_NODE_PATH_PARTICLE_EMITTER_NAME = strutils::StringId("node_path_static");
static const strutils::StringId IS_NODE_ACTIVE_UNIFORM_NAME = strutils::StringId("is_active");

static const std::string STORY_MAP_NODE_SHADER_FILE_NAME = "story_map_node.vs";
static const std::string SHOP_TEXTURE_FILE_NAME = "story_cards/shop.png";
static const std::string EVENT_TEXTURE_FILE_NAME = "story_cards/event.png";
static const std::string NODE_PATH_TEXTURE_FILE_NAME = "trap_mask.png";

static const glm::vec3 FIRST_NODE_POSITION = { -0.8f, -0.63f, 0.1f };
static const glm::vec3 LAST_NODE_POSITION = { 0.46f, 0.53f, 0.1f };
static const glm::vec3 NODE_PORTRAIT_POSITION_OFFSET = {0.00f, 0.01f, 0.08f};
static const glm::vec3 PORTRAIT_TEXT_SCALE = {0.00017f, 0.00017f, 0.00017f};
static const glm::vec3 PORTRAIT_PRIMARY_TEXT_POSITION_OFFSET = {0.005f, -0.03f, 0.1f};
static const glm::vec3 PORTRAIT_SECONDARY_TEXT_POSITION_OFFSET = {-0.009f, -0.05f, 0.1f};

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
static const float NODE_SCALE = 0.18f;
static const float NODE_PORTRAIT_SCALE = 0.072f;
static const float NODE_PATH_SCALE = 0.015f;
static const float MAX_NODE_PATH_SCALE = 0.04f;
static const float MIN_NODE_PATH_SCALE = 0.025f;
static const float NODE_PATH_INIT_SCALE_SEPARATOR = 0.002f;
static const float NODE_PATH_Z_SEPARATOR = 0.0001f;
static const float NODE_PATH_SCALE_SPEED = 0.00003f;
static const float INACTIVE_NODE_PATH_LIFETIME_SECS = 0.85f;
static const float SELECTABLE_NODE_BOUNCE_SPEED_Y = 0.000005f;
static const float PORTRAIT_BOUNCE_NOISE_FACTOR = 0.2f;
static const float INACTIVE_NODE_TEXT_ALPHA = 0.5f;

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
    
    // Generate all encounter names and sort them by name length
    std::vector<std::string> generatedDemonNames;
    for (auto& mapNodeEntry: mMapData)
    {
        if (mapNodeEntry.second.mNodeType == NodeType::NORMAL_ENCOUNTER || mapNodeEntry.second.mNodeType == NodeType::ELITE_ENCOUNTER || mapNodeEntry.second.mNodeType == NodeType::BOSS_ENCOUNTER)
        {
            generatedDemonNames.emplace_back(GenerateControlledRandomDemonName());
        }
    }
    std::sort(generatedDemonNames.begin(), generatedDemonNames.end(), [](const std::string& lhs, const std::string& rhs)
    {
        return lhs.size() < rhs.size();
    });
    
    // All node meshes
    for (const auto& mapNodeEntry: mMapData)
    {
        auto nodeSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString()));
        nodeSceneObject->mPosition = mapNodeEntry.second.mPosition;
        nodeSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
        nodeSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
        nodeSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + MAP_NODE_TYPES_TO_PORTRAIT_TEXTURES.at(mapNodeEntry.second.mNodeType));
        nodeSceneObject->mScale = glm::vec3(NODE_SCALE);
        
        auto nodePortraitSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + "_portrait"));
        nodePortraitSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
        nodePortraitSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
        nodePortraitSceneObject->mPosition = mapNodeEntry.second.mPosition;
        nodePortraitSceneObject->mScale = glm::vec3(NODE_PORTRAIT_SCALE);
        nodePortraitSceneObject->mPosition += NODE_PORTRAIT_POSITION_OFFSET;
        
        // Starting location does not have a portrait texture
        if (mapNodeEntry.second.mNodeType == NodeType::STARTING_LOCATION)
        {
            nodePortraitSceneObject->mInvisible = true;
        }
        
        std::vector<std::shared_ptr<scene::SceneObject>> textSceneObjects;
        textSceneObjects.emplace_back(mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + "_text")));
        textSceneObjects.emplace_back(mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + "_text_secondary")));
        
        textSceneObjects[0]->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
        textSceneObjects[1]->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
        
        scene::TextSceneObjectData primaryTextData;
        scene::TextSceneObjectData secondaryTextData;
        
        primaryTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        secondaryTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        
        switch (mapNodeEntry.second.mNodeType)
        {
            case NodeType::STARTING_LOCATION:
            {
            } break;
                
            case NodeType::ELITE_ENCOUNTER:
            {
                primaryTextData.mText = generatedDemonNames.front();
                generatedDemonNames.erase(generatedDemonNames.begin());
                
                if (mapNodeEntry.first.mCol < mMapDimensions.x/2)
                {
                    nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + MEDIUM_FIGHT_TEXTURES.at(math::ControlledRandomInt(0, static_cast<int>(MEDIUM_FIGHT_TEXTURES.size()) - 1)));
                }
                else
                {
                    nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + HARD_FIGHT_TEXTURES.at(math::ControlledRandomInt(0, static_cast<int>(HARD_FIGHT_TEXTURES.size()) - 1)));
                }
                
                secondaryTextData.mText = "Elite";
            } break;
                
            case NodeType::NORMAL_ENCOUNTER:
            {
                primaryTextData.mText = generatedDemonNames.front();
                generatedDemonNames.erase(generatedDemonNames.begin());
                
                if (mapNodeEntry.first.mCol < mMapDimensions.x/2)
                {
                    nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + EASY_FIGHT_TEXTURES.at(math::ControlledRandomInt(0, static_cast<int>(EASY_FIGHT_TEXTURES.size()) - 1)));
                }
                else
                {
                    nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + MEDIUM_FIGHT_TEXTURES.at(math::ControlledRandomInt(0, static_cast<int>(MEDIUM_FIGHT_TEXTURES.size()) - 1)));
                }
            } break;
            
            case NodeType::EVENT:
            {
                primaryTextData.mText = "Event";
                nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + EVENT_TEXTURE_FILE_NAME);
            } break;
                
            case NodeType::SHOP:
            {
                primaryTextData.mText = "DemoBob's";
                secondaryTextData.mText = "Shop";
                nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + SHOP_TEXTURE_FILE_NAME);
            } break;
                
            case NodeType::BOSS_ENCOUNTER:
            {
                primaryTextData.mText = generatedDemonNames.front();
                generatedDemonNames.erase(generatedDemonNames.begin());
                nodePortraitSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + BOSS_FIGHT_TEXTURES.at(math::ControlledRandomInt(0, static_cast<int>(BOSS_FIGHT_TEXTURES.size()) - 1)));
            } break;
                
            default: break;
        }
        
        textSceneObjects[0]->mScale = PORTRAIT_TEXT_SCALE;
        textSceneObjects[0]->mPosition = mapNodeEntry.second.mPosition;
        textSceneObjects[0]->mSceneObjectTypeData = std::move(primaryTextData);
        
        auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*textSceneObjects[0]);
        auto boundingRectWidth = boundingRect.topRight.x - boundingRect.bottomLeft.x;
        
        textSceneObjects[0]->mPosition += PORTRAIT_PRIMARY_TEXT_POSITION_OFFSET;
        textSceneObjects[0]->mPosition.x -= boundingRectWidth/2;
        
        textSceneObjects[1]->mScale = PORTRAIT_TEXT_SCALE;
        textSceneObjects[1]->mSceneObjectTypeData = std::move(secondaryTextData);
        textSceneObjects[1]->mPosition = mapNodeEntry.second.mPosition;
        textSceneObjects[1]->mPosition += PORTRAIT_SECONDARY_TEXT_POSITION_OFFSET;
        
        // Add also pulsing animation if node is active
        if (mMapData.at(mCurrentMapCoord).mNodeLinks.count(mapNodeEntry.first))
        {
            nodeSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
            nodePortraitSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
            
            auto randomDelaySecsOffset = math::RandomFloat(0.0f, 1.0f);
            auto randomBounceYSpeed = math::RandomFloat(SELECTABLE_NODE_BOUNCE_SPEED_Y - SELECTABLE_NODE_BOUNCE_SPEED_Y * PORTRAIT_BOUNCE_NOISE_FACTOR, SELECTABLE_NODE_BOUNCE_SPEED_Y + SELECTABLE_NODE_BOUNCE_SPEED_Y * PORTRAIT_BOUNCE_NOISE_FACTOR);
            animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodePortraitSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            
            for (auto textSceneObject: textSceneObjects)
            {
                textSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(textSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            }
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
        
        // Shop only at penultimate node and via event
        availableNodeTypes.erase(NodeType::SHOP);
        
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
