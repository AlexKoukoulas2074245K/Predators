///------------------------------------------------------------------------------------------------
///  SceneManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/resloading/DataFileResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneManager.h>
#include <engine/utils/BaseDataFileDeserializer.h>
#include <fstream>
#include <nlohmann/json.hpp>

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

static const std::string SCENE_DESCRIPTORS_PATH = "scene_descriptors/";
static const std::unordered_map<std::string, scene::SnapToEdgeBehavior> STRING_TO_SNAP_TO_EDGE_BEHAVIOR_MAP =
{
    { "none", scene::SnapToEdgeBehavior::NONE },
    { "snap_to_left_edge", scene::SnapToEdgeBehavior::SNAP_TO_LEFT_EDGE },
    { "snap_to_right_edge", scene::SnapToEdgeBehavior::SNAP_TO_RIGHT_EDGE },
    { "snap_to_top_edge", scene::SnapToEdgeBehavior::SNAP_TO_TOP_EDGE },
    { "snap_to_bot_edge", scene::SnapToEdgeBehavior::SNAP_TO_BOT_EDGE }
};

///------------------------------------------------------------------------------------------------

std::shared_ptr<Scene> SceneManager::CreateScene(const strutils::StringId sceneName /* = strutils::StringId() */)
{
    mScenes.emplace_back(std::make_shared<Scene>(sceneName));
    return mScenes.back();
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<Scene> SceneManager::FindScene(const strutils::StringId& sceneName) const
{
    auto findIter = std::find_if(mScenes.begin(), mScenes.end(), [&](const std::shared_ptr<Scene>& scene)
    {
        return scene->GetName() == sceneName;
    });
    
    return findIter != mScenes.end() ? *findIter : nullptr;
}

///------------------------------------------------------------------------------------------------

void SceneManager::LoadPredefinedObjectsFromDescriptorForScene(std::shared_ptr<Scene> scene)
{
    if (scene->HasLoadedPredefinedObjects())
    {
        return;
    }
    
    scene->SetHasLoadedPredefinedObjects(true);
    
    auto sceneDescriptorPath = resources::ResourceLoadingService::RES_DATA_ROOT + SCENE_DESCRIPTORS_PATH + scene->GetName().GetString() + ".json";
    std::ifstream testFile(sceneDescriptorPath);
    if (!testFile.is_open())
    {
        return;
    }
    
    auto& resourceService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    
#if !defined(NDEBUG)
    auto sceneDescriptorJsonResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(sceneDescriptorPath);
    const auto sceneDescriptorJson =  nlohmann::json::parse(resourceService.GetResource<resources::DataFileResource>(sceneDescriptorJsonResourceId).GetContents());
#else
    const auto sceneDescriptorJson = serial::BaseDataFileDeserializer(SCENE_DESCRIPTORS_PATH + scene->GetName().GetString(), serial::DataFileType::ASSET_FILE_TYPE).GetState();
#endif
    
    for (const auto& childSceneJson: sceneDescriptorJson["children_scenes"])
    {
        const auto& childSceneName = strutils::StringId(childSceneJson.get<std::string>());
        auto childScene = FindScene(childSceneName);
        if (!childScene)
        {
            childScene = CoreSystemsEngine::GetInstance().GetSceneManager().CreateScene(childSceneName);
        }
        
        LoadPredefinedObjectsFromDescriptorForScene(childScene);
    }
    
    for (const auto& sceneObjectJson: sceneDescriptorJson["scene_objects"])
    {
        auto sceneObjectName = strutils::StringId(sceneObjectJson["name"].get<std::string>());
        assert (!scene->FindSceneObject(sceneObjectName));
        auto sceneObject = scene->CreateSceneObject(strutils::StringId(sceneObjectName));
        
        if (sceneObjectJson.count("is_background"))
        {
            sceneObject->mIsBackground = sceneObjectJson["is_background"].get<bool>();
        }
        
        if (sceneObjectJson.count("texture"))
        {
            sceneObject->mTextureResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + sceneObjectJson["texture"].get<std::string>());
        }
        
        if (sceneObjectJson.count("effect_textures"))
        {
            int i = 0;
            for (const auto& effectTextureJson: sceneObjectJson["effect_textures"])
            {
                sceneObject->mEffectTextureResourceIds[i++] = resourceService.LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + effectTextureJson.get<std::string>());
            }
        }
        
        if (sceneObjectJson.count("shader"))
        {
            sceneObject->mShaderResourceId = resourceService.LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + sceneObjectJson["shader"].get<std::string>());
        }
        
        if (sceneObjectJson.count("position"))
        {
            sceneObject->mPosition = glm::vec3
            (
                sceneObjectJson["position"]["x"].get<float>(),
                sceneObjectJson["position"]["y"].get<float>(),
                sceneObjectJson["position"]["z"].get<float>()
            );
        }
        
        if (sceneObjectJson.count("scale"))
        {
            sceneObject->mScale = glm::vec3
            (
                sceneObjectJson["scale"]["x"].get<float>(),
                sceneObjectJson["scale"]["y"].get<float>(),
                sceneObjectJson["scale"]["z"].get<float>()
            );
        }
        
        if (sceneObjectJson.count("rotation"))
        {
            sceneObject->mRotation = glm::vec3
            (
                sceneObjectJson["rotation"]["x"].get<float>(),
                sceneObjectJson["rotation"]["y"].get<float>(),
                sceneObjectJson["rotation"]["z"].get<float>()
            );
        }
        
        if (sceneObjectJson.count("alpha"))
        {
            sceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = sceneObjectJson["alpha"].get<float>();
        }
        
        if (sceneObjectJson.count("invisible"))
        {
            sceneObject->mInvisible = sceneObjectJson["invisible"].get<bool>();
        }
        
        if (sceneObjectJson.count("snap_to_edge"))
        {
            sceneObject->mSnapToEdgeBehavior = STRING_TO_SNAP_TO_EDGE_BEHAVIOR_MAP.at(sceneObjectJson["snap_to_edge"].get<std::string>());
        }
        
        if (sceneObjectJson.count("uniform_floats"))
        {
            for (const auto& uniformFloatJson: sceneObjectJson["uniform_floats"])
            {
                sceneObject->mShaderFloatUniformValues[strutils::StringId(uniformFloatJson["name"].get<std::string>())] = uniformFloatJson["value"].get<float>();
            }
        }
        
        scene::TextSceneObjectData textData;
        if (sceneObjectJson.count("font"))
        {
            textData.mFontName = strutils::StringId(sceneObjectJson["font"].get<std::string>());
        }
        if (sceneObjectJson.count("text"))
        {
            textData.mText = sceneObjectJson["text"].get<std::string>();
        }
        
        if (!textData.mText.empty() || !textData.mFontName.isEmpty())
        {
            sceneObject->mSceneObjectTypeData = std::move(textData);
        }
    }
}

///------------------------------------------------------------------------------------------------

void SceneManager::SortSceneObjects(std::shared_ptr<Scene> scene)
{
    auto& sceneObjects = scene->GetSceneObjects();
    std::sort(sceneObjects.begin(), sceneObjects.end(), [&](const std::shared_ptr<scene::SceneObject>& lhs, const std::shared_ptr<scene::SceneObject>& rhs)
    {
        return lhs->mPosition.z < rhs->mPosition.z;
    });
}

///------------------------------------------------------------------------------------------------

void SceneManager::RemoveScene(const strutils::StringId& sceneName)
{
    auto findIter = std::find_if(mScenes.begin(), mScenes.end(), [&](const std::shared_ptr<Scene>& scene)
    {
        return scene->GetName() == sceneName;
    });
    if (findIter != mScenes.end())
    {
        for (auto& sceneObject: (*findIter)->GetSceneObjects())
        {
            sceneObject->mScene = nullptr;
        }
        mScenes.erase(findIter);
    }
}

///------------------------------------------------------------------------------------------------

[[nodiscard]] std::size_t SceneManager::GetSceneCount() const { return mScenes.size(); }

///------------------------------------------------------------------------------------------------

const std::vector<std::shared_ptr<Scene>>& SceneManager::GetScenes() const { return mScenes; }

///------------------------------------------------------------------------------------------------

}
