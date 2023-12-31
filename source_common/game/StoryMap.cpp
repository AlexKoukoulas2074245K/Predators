///------------------------------------------------------------------------------------------------
///  StoryMap.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 19/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/GameConstants.h>
#include <game/ProgressionDataRepository.h>
#include <game/StoryMap.h>
#include <game/utils/DemonNameGenerator.h>
#include <engine/CoreSystemsEngine.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/ParticleManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <unordered_set>
#include <unordered_map>

///------------------------------------------------------------------------------------------------

static const std::unordered_map<StoryMap::NodeType, std::string> MAP_NODE_TYPES_TO_PORTRAIT_TEXTURES =
{
    { StoryMap::NodeType::NORMAL_ENCOUNTER, "map_node_normal.png" },
    { StoryMap::NodeType::ELITE_ENCOUNTER, "map_node_elite.png" },
    { StoryMap::NodeType::BOSS_ENCOUNTER, "map_node_boss.png" },
    { StoryMap::NodeType::EVENT, "map_node_misc.png" },
    { StoryMap::NodeType::SHOP, "map_node_misc.png" },
    { StoryMap::NodeType::STARTING_LOCATION, "teepee.png" },
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
static const std::string ENCOUNTER_STAT_HEALTH_ICON_TEXTURE_FILE_NAME = "health_icon.png";
static const std::string ENCOUNTER_STAT_DAMAGE_ICON_TEXTURE_FILE_NAME = "health_crystal.png";
static const std::string ENCOUNTER_STAT_WEIGHT_ICON_TEXTURE_FILE_NAME = "weight_crystal.png";

static const glm::vec3 FIRST_NODE_POSITION = { -1.0f, -0.83f, 0.1f };
static const glm::vec3 LAST_NODE_POSITION = { 0.6f, 0.73f, 0.1f };
static const glm::vec3 NODE_PORTRAIT_POSITION_OFFSET = {0.00f, 0.01f, 0.08f};
static const glm::vec3 PORTRAIT_TEXT_SCALE = {0.00017f, 0.00017f, 0.00017f};
static const glm::vec3 PORTRAIT_PRIMARY_TEXT_POSITION_OFFSET = {0.005f, -0.03f, 0.1f};
static const glm::vec3 PORTRAIT_SECONDARY_TEXT_POSITION_OFFSET = {-0.009f, -0.05f, 0.1f};
static const glm::vec3 ENCOUNTER_STAT_TEXT_SCALE = {0.00022f, 0.00022, 0.00022f};
static const glm::vec3 ENCOUNTER_STAT_TEXT_POSITION_OFFSET = {0.004f, 0.003f, 0.05f};
static const glm::vec3 ENCOUNTER_STAT_ICON_SCALE = {0.072f, 0.072f, 0.072f};
static const glm::vec3 ENCOUNTER_STAT_HEALTH_ICON_POSITION_OFFSET = {0.00f, 0.07f, 0.12f};
static const glm::vec3 ENCOUNTER_STAT_DAMAGE_ICON_POSITION_OFFSET = {-0.04f, 0.05f, 0.12f};
static const glm::vec3 ENCOUNTER_STAT_WEIGHT_ICON_POSITION_OFFSET = {0.04f, 0.05f, 0.12f};

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
static const float ELITE_STAT_FACTOR = 1.5f;
static const float BOSS_STAT_FACTOR = 3.0f;

static const int MAP_PATH_SEGMENTS_FACTOR = 30;
static const int MAP_GENERATION_PASSES = 5;

#if defined(NDEBUG) || defined(MOBILE_FLOW)
static const float NODES_CLOSE_ENOUGH_THRESHOLD = 0.050f;
static const float NODES_CLOSE_ENOUGH_TO_EDGE_NODES_THRESHOLD = 0.075f;
static const int MAX_MAP_GENERATION_ATTEMPTS = 100000;
static const glm::vec2 VERTICAL_MAP_EDGE = {-0.95f, 0.95f};
#else
static const float NODES_CLOSE_ENOUGH_THRESHOLD = 0.030f;
static const float NODES_CLOSE_ENOUGH_TO_EDGE_NODES_THRESHOLD = 0.075f;
static const int MAX_MAP_GENERATION_ATTEMPTS = 50000;
static const glm::vec2 VERTICAL_MAP_EDGE = {-0.95f, 0.95f};
#endif

int mapGenerationAttempts = 0;

///------------------------------------------------------------------------------------------------

StoryMap::StoryMap(std::shared_ptr<scene::Scene> scene, const glm::ivec2& mapDimensions, const MapCoord& currentMapCoord)
    : mScene(scene)
    , mMapDimensions(mapDimensions)
    , mCurrentMapCoord(currentMapCoord)
    , mMapGenerationAttemptsRemaining(MAX_MAP_GENERATION_ATTEMPTS)
    , mHasCreatedSceneObjects(false)
{
}

///------------------------------------------------------------------------------------------------

void StoryMap::GenerateMapNodes()
{
    GenerateMapData();
}

///------------------------------------------------------------------------------------------------

bool StoryMap::HasCreatedSceneObjects() const
{
    return mHasCreatedSceneObjects;
}

///------------------------------------------------------------------------------------------------

const std::map<MapCoord, StoryMap::NodeData>& StoryMap::GetMapData() const
{
    return mMapData;
}

///------------------------------------------------------------------------------------------------

const glm::ivec2& StoryMap::GetMapDimensions() const
{
    return mMapDimensions;
}

///------------------------------------------------------------------------------------------------

void StoryMap::GenerateMapData()
{
    mapGenerationAttempts = 0;
    auto currentGenerationSeed = ProgressionDataRepository::GetInstance().GetStoryMapGenerationSeed();
    if (currentGenerationSeed == 0)
    {
        // New map will be generated
        auto newGenerationSeed = math::RandomInt();
        math::SetControlSeed(newGenerationSeed);
    }
    else
    {
        // Same map as before will be generated here.
        math::SetControlSeed(currentGenerationSeed);
        mMapGenerationAttemptsRemaining = 1;
    }
    
    CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddArtificialLoadingJobCount(mMapGenerationAttemptsRemaining);
    
    do
    {
        mMapGenerationAttemptsRemaining--;
        CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddArtificialLoadingJobCount(-1);
        
        mapGenerationAttempts++;
        mMapData.clear();
        
        ProgressionDataRepository::GetInstance().SetStoryMapGenerationSeed(math::GetControlSeed());
        
        for (int i = 0; i < MAP_GENERATION_PASSES; ++i)
        {
            auto currentCoordinate = MapCoord(0, mMapDimensions.y/2);
            mMapData[currentCoordinate].mPosition = GenerateNodePositionForCoord(currentCoordinate);
            mMapData[currentCoordinate].mNodeType = SelectNodeTypeForCoord(currentCoordinate);
            mMapData[currentCoordinate].mNodeRandomSeed = math::ControlledRandomInt();
            mMapData[currentCoordinate].mCoords = { currentCoordinate.mCol, currentCoordinate.mRow };
            
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
                mMapData[currentCoordinate].mNodeRandomSeed = math::ControlledRandomInt();
                mMapData[currentCoordinate].mCoords = { currentCoordinate.mCol, currentCoordinate.mRow };
            }
        }
    } while (FoundCloseEnoughNodes() && mMapGenerationAttemptsRemaining > 0);
    
    CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddArtificialLoadingJobCount(-mMapGenerationAttemptsRemaining);
}

///------------------------------------------------------------------------------------------------

void StoryMap::DestroyParticleEmitters()
{
    mScene->RemoveSceneObject(STATIC_NODE_PATH_PARTICLE_EMITTER_NAME);
    mScene->RemoveSceneObject(ANIMATED_NODE_PATH_PARTICLE_EMITTER_NAME);
}

///------------------------------------------------------------------------------------------------

bool StoryMap::FoundCloseEnoughNodes() const
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
        
        if (mapNodeEntry.second.mPosition.y < VERTICAL_MAP_EDGE.s || mapNodeEntry.second.mPosition.y > VERTICAL_MAP_EDGE.t)
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

void StoryMap::CreateMapSceneObjects()
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
        auto effectiveNodeType = mapNodeEntry.second.mNodeType;
        if (mapNodeEntry.first == mCurrentMapCoord)
        {
            effectiveNodeType = NodeType::STARTING_LOCATION;
        }
        
        auto nodeSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString()));
        nodeSceneObject->mPosition = mapNodeEntry.second.mPosition;
        nodeSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
        nodeSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
        nodeSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + MAP_NODE_TYPES_TO_PORTRAIT_TEXTURES.at(effectiveNodeType));
        nodeSceneObject->mBoundingRectMultiplier.x = game_constants::CARD_BOUNDING_RECT_X_MULTIPLIER;
        nodeSceneObject->mScale = glm::vec3(NODE_SCALE);
        
        auto nodePortraitSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_PORTRAIT_SO_NAME_POST_FIX));
        nodePortraitSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
        nodePortraitSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
        nodePortraitSceneObject->mPosition = mapNodeEntry.second.mPosition;
        nodePortraitSceneObject->mScale = glm::vec3(NODE_PORTRAIT_SCALE);
        nodePortraitSceneObject->mPosition += NODE_PORTRAIT_POSITION_OFFSET;
        
        // Starting location does not have a portrait texture
        if (effectiveNodeType == NodeType::STARTING_LOCATION)
        {
            nodePortraitSceneObject->mInvisible = true;
        }
        
        std::vector<std::shared_ptr<scene::SceneObject>> textSceneObjects;
        textSceneObjects.emplace_back(mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_TEXT_SO_NAME_POST_FIX)));
        textSceneObjects.emplace_back(mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_SECONDARY_TEXT_SO_NAME_POST_FIX)));
        
        textSceneObjects[0]->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
        textSceneObjects[1]->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
        
        scene::TextSceneObjectData primaryTextData;
        scene::TextSceneObjectData secondaryTextData;
        
        primaryTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        secondaryTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
        
        bool isEncounterNode = effectiveNodeType == NodeType::BOSS_ENCOUNTER || effectiveNodeType == NodeType::ELITE_ENCOUNTER || effectiveNodeType == NodeType::NORMAL_ENCOUNTER;
        
        switch (effectiveNodeType)
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
        
        std::shared_ptr<scene::SceneObject> nodeHealthIconSceneObject = nullptr;
        std::shared_ptr<scene::SceneObject> nodeHealthTextSceneObject = nullptr;
        std::shared_ptr<scene::SceneObject> nodeDamageIconSceneObject = nullptr;
        std::shared_ptr<scene::SceneObject> nodeDamageTextSceneObject = nullptr;
        std::shared_ptr<scene::SceneObject> nodeWeightIconSceneObject = nullptr;
        std::shared_ptr<scene::SceneObject> nodeWeightTextSceneObject = nullptr;
        
        if (isEncounterNode)
        {
            // Stat range builders
            auto defaultHealthRange = glm::vec2(5.0f + mapNodeEntry.first.mCol, 10.0f + mapNodeEntry.first.mCol);
            auto defaultDamageRange = glm::vec2(0.0f + mapNodeEntry.first.mCol, 1.0f + mapNodeEntry.first.mCol);
            auto defaultWeightRange = glm::vec2(2.0f + mapNodeEntry.first.mCol, 3.0f + mapNodeEntry.first.mCol);
            
            if (effectiveNodeType == NodeType::ELITE_ENCOUNTER)
            {
                defaultHealthRange *= ELITE_STAT_FACTOR;
                defaultDamageRange *= ELITE_STAT_FACTOR;
                defaultWeightRange *= ELITE_STAT_FACTOR;
            }
            
            if (effectiveNodeType == NodeType::BOSS_ENCOUNTER)
            {
                defaultHealthRange *= BOSS_STAT_FACTOR;
                defaultDamageRange *= BOSS_STAT_FACTOR;
                defaultWeightRange *= BOSS_STAT_FACTOR;
            }
            
            // Final stat values
            auto nodeOpponentHealth = math::ControlledRandomFloat(defaultHealthRange.s, defaultHealthRange.t);
            auto nodeOpponentDamage = math::ControlledRandomFloat(defaultDamageRange.s, defaultDamageRange.t);
            auto nodeOpponentWeight = math::ControlledRandomFloat(defaultWeightRange.s, defaultWeightRange.t);
            
            // Health Icon
            nodeHealthIconSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_HEALTH_ICON_SO_NAME_POST_FIX));
            nodeHealthIconSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + ENCOUNTER_STAT_HEALTH_ICON_TEXTURE_FILE_NAME);
            nodeHealthIconSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
            nodeHealthIconSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
            nodeHealthIconSceneObject->mPosition = mapNodeEntry.second.mPosition;
            nodeHealthIconSceneObject->mScale = ENCOUNTER_STAT_ICON_SCALE;
            nodeHealthIconSceneObject->mPosition += ENCOUNTER_STAT_HEALTH_ICON_POSITION_OFFSET;
            
            // Health Text
            scene::TextSceneObjectData healthIconTextData;
            healthIconTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
            healthIconTextData.mText = std::to_string(static_cast<int>(nodeOpponentHealth));
            
            nodeHealthTextSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_HEALTH_TEXT_SO_NAME_POST_FIX));
            nodeHealthTextSceneObject->mSceneObjectTypeData = std::move(healthIconTextData);
            nodeHealthTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
            nodeHealthTextSceneObject->mScale = ENCOUNTER_STAT_TEXT_SCALE;
            nodeHealthTextSceneObject->mPosition = nodeHealthIconSceneObject->mPosition + ENCOUNTER_STAT_TEXT_POSITION_OFFSET;
            
            boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*nodeHealthTextSceneObject);
            nodeHealthTextSceneObject->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
            
            // Damage Icon
            nodeDamageIconSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_DAMAGE_ICON_SO_NAME_POST_FIX));
            nodeDamageIconSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + ENCOUNTER_STAT_DAMAGE_ICON_TEXTURE_FILE_NAME);
            nodeDamageIconSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
            nodeDamageIconSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
            nodeDamageIconSceneObject->mPosition = mapNodeEntry.second.mPosition;
            nodeDamageIconSceneObject->mScale = ENCOUNTER_STAT_ICON_SCALE;
            nodeDamageIconSceneObject->mPosition += ENCOUNTER_STAT_DAMAGE_ICON_POSITION_OFFSET;
            
            // Damage Text
            scene::TextSceneObjectData damageIconTextData;
            damageIconTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
            damageIconTextData.mText = std::to_string(static_cast<int>(nodeOpponentDamage));
            
            nodeDamageTextSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_DAMAGE_TEXT_SO_NAME_POST_FIX));
            nodeDamageTextSceneObject->mSceneObjectTypeData = std::move(damageIconTextData);
            nodeDamageTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
            nodeDamageTextSceneObject->mScale = ENCOUNTER_STAT_TEXT_SCALE;
            nodeDamageTextSceneObject->mPosition = nodeDamageIconSceneObject->mPosition + ENCOUNTER_STAT_TEXT_POSITION_OFFSET;
            
            boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*nodeDamageTextSceneObject);
            nodeDamageTextSceneObject->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
            
            // Weight Icon
            nodeWeightIconSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_WEIGHT_ICON_SO_NAME_POST_FIX));
            nodeWeightIconSceneObject->mTextureResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + ENCOUNTER_STAT_WEIGHT_ICON_TEXTURE_FILE_NAME);
            nodeWeightIconSceneObject->mShaderResourceId = resService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + STORY_MAP_NODE_SHADER_FILE_NAME);
            nodeWeightIconSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = mapNodeEntry.first == mCurrentMapCoord;
            nodeWeightIconSceneObject->mPosition = mapNodeEntry.second.mPosition;
            nodeWeightIconSceneObject->mScale = ENCOUNTER_STAT_ICON_SCALE;
            nodeWeightIconSceneObject->mPosition += ENCOUNTER_STAT_WEIGHT_ICON_POSITION_OFFSET;
            
            // Weight Text
            scene::TextSceneObjectData weightIconTextData;
            weightIconTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
            weightIconTextData.mText = std::to_string(static_cast<int>(nodeOpponentWeight));
            
            nodeWeightTextSceneObject = mScene->CreateSceneObject(strutils::StringId(mapNodeEntry.first.ToString() + game_constants::STORY_MAP_NODE_WEIGHT_TEXT_SO_NAME_POST_FIX));
            nodeWeightTextSceneObject->mSceneObjectTypeData = std::move(weightIconTextData);
            nodeWeightTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = INACTIVE_NODE_TEXT_ALPHA;
            nodeWeightTextSceneObject->mScale = ENCOUNTER_STAT_TEXT_SCALE;
            nodeWeightTextSceneObject->mPosition = nodeWeightIconSceneObject->mPosition + ENCOUNTER_STAT_TEXT_POSITION_OFFSET;
            
            boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*nodeWeightTextSceneObject);
            nodeWeightTextSceneObject->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
        }
        
        // Add also pulsing animation if node is active
        if (mMapData.at(mCurrentMapCoord).mNodeLinks.count(mapNodeEntry.first))
        {
            nodeSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
            nodePortraitSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
            
            auto randomDelaySecsOffset = math::RandomFloat(0.0f, 1.0f);
            auto randomBounceYSpeed = math::RandomFloat(SELECTABLE_NODE_BOUNCE_SPEED_Y - SELECTABLE_NODE_BOUNCE_SPEED_Y * PORTRAIT_BOUNCE_NOISE_FACTOR, SELECTABLE_NODE_BOUNCE_SPEED_Y + SELECTABLE_NODE_BOUNCE_SPEED_Y * PORTRAIT_BOUNCE_NOISE_FACTOR);
            animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodePortraitSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            
            
            if (isEncounterNode)
            {
                nodeHealthIconSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeHealthIconSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
                
                nodeHealthTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeHealthTextSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
                
                nodeDamageIconSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeDamageIconSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
                
                nodeDamageTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeDamageTextSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
                
                nodeWeightIconSceneObject->mShaderBoolUniformValues[IS_NODE_ACTIVE_UNIFORM_NAME] = true;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeWeightIconSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
                
                nodeWeightTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(nodeWeightTextSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            }
            
            for (auto textSceneObject: textSceneObjects)
            {
                textSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
                animationManager.StartAnimation(std::make_unique<rendering::BouncePositionAnimation>(textSceneObject, glm::vec3(0.0f, randomBounceYSpeed, 0.0f), 1.0f, animation_flags::ANIMATE_CONTINUOUSLY, randomDelaySecsOffset), [](){});
            }
        }
        
        // Make all previous nodes invisible
        if (mapNodeEntry.first.mCol <= mCurrentMapCoord.mCol && mapNodeEntry.first != mCurrentMapCoord) 
        {
            nodeSceneObject->mInvisible = true;
            nodePortraitSceneObject->mInvisible = true;
            
            if (isEncounterNode)
            {
                nodeHealthIconSceneObject->mInvisible = true;
                nodeHealthTextSceneObject->mInvisible = true;
                nodeDamageIconSceneObject->mInvisible = true;
                nodeDamageTextSceneObject->mInvisible = true;
                nodeWeightIconSceneObject->mInvisible = true;
                nodeWeightTextSceneObject->mInvisible = true;
            }
            
            for (auto textSceneObject: textSceneObjects)
            {
                textSceneObject->mInvisible = true;
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
                    particleEmitterData.mParticleSizes[i] = MAX_NODE_PATH_SCALE;
                    particleEmitterData.mParticleAngles[i] = -1.0f;
                }
            }
            else
            {
                particleEmitterData.mParticleSizes[i] -= dtMillis * NODE_PATH_SCALE_SPEED;
                if (particleEmitterData.mParticleSizes[i] < MIN_NODE_PATH_SCALE)
                {
                    particleEmitterData.mParticleSizes[i] = MIN_NODE_PATH_SCALE;
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
            particleEmitterData.mParticleSizes[i] = MIN_NODE_PATH_SCALE;
        }
        
    });
    
    for (const auto& mapNodeEntry: mMapData)
    {
        if (mapNodeEntry.first.mCol <= mCurrentMapCoord.mCol && mapNodeEntry.first != mCurrentMapCoord)
        {
            continue;
        }
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
                logging::Log(logging::LogType::INFO, "Creating particle of size %.6f", particleEmitterData.mParticleSizes[indexSpawnedAt]);
                particleEmitterData.mParticleAngles[indexSpawnedAt] = 1.0f; // signifies > 0.0f -> scale up, < 0.0f -> scale down
                particleEmitterData.mParticlePositions[indexSpawnedAt] = mMapData.at(mapNodeEntry.first).mPosition + dirToNext * (i/static_cast<float>(pathSegments));
                particleEmitterData.mParticlePositions[indexSpawnedAt].z = NODE_PATH_POSITION_Z + indexSpawnedAt * NODE_PATH_Z_SEPARATOR;
            }
        }
    }
    
    mHasCreatedSceneObjects = true;
}

///------------------------------------------------------------------------------------------------

bool StoryMap::DetectedCrossedEdge(const MapCoord& currentCoord, const MapCoord& targetTestCoord) const
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

glm::vec3 StoryMap::GenerateNodePositionForCoord(const MapCoord& mapCoord) const
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
        
        auto t = 0.03f + mapCoord.mCol/static_cast<float>(mMapDimensions.x);
        
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

StoryMap::NodeType StoryMap::SelectNodeTypeForCoord(const MapCoord& mapCoord) const
{
    // Forced single entry point and starting coord case
    if (mapCoord == MapCoord(0, mMapDimensions.y/2))
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
        
        if (mapCoord.mCol == 1)
        {
            // Elite fights can't be at first availables nodes to move to
            availableNodeTypes.erase(NodeType::ELITE_ENCOUNTER);
        }
        
        // Remove any node types from the immediate previous links.
        for (const auto& mapEntry: mMapData)
        {
            if (mapEntry.second.mNodeLinks.count(mapCoord) && availableNodeTypes.size() > 2)
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

MapCoord StoryMap::RandomlySelectNextMapCoord(const MapCoord& mapCoord) const
{
    auto randRow = math::Max(math::Min(mMapDimensions.y - 1, mapCoord.mRow + math::ControlledRandomInt(-1, 1)), 0);
    return mapCoord.mCol == mMapDimensions.x - 2 ? MapCoord(mMapDimensions.x - 1, mMapDimensions.y/2) : MapCoord(mapCoord.mCol + 1, randRow);
}

///------------------------------------------------------------------------------------------------
