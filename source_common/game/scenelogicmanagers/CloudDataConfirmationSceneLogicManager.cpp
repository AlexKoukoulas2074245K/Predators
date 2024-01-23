///------------------------------------------------------------------------------------------------
///  CloudDataConfirmationSceneLogicManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 22/01/2024
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/SceneObjectUtils.h>
#include <fstream>
#include <game/AnimatedButton.h>
#include <game/Cards.h>
#include <game/events/EventSystem.h>
#include <game/DataRepository.h>
#include <game/scenelogicmanagers/CloudDataConfirmationSceneLogicManager.h>
#include <SDL_events.h>
#if defined(MACOS) || defined(MOBILE_FLOW)
#include <platform_utilities/AppleUtils.h>
#include <platform_utilities/CloudKitUtils.h>
#elif defined(WINDOWS)
#include <platform_utilities/WindowsUtils.h>
#endif

///------------------------------------------------------------------------------------------------

static const strutils::StringId USE_CLOUD_DATA_BUTTON_NAME = strutils::StringId("use_cloud_data_button");
static const strutils::StringId USE_LOCAL_DATA_BUTTON_NAME = strutils::StringId("use_local_data_button");
static const strutils::StringId CLOUD_DATA_DEVICE_NAME_AND_TIME_TEXT_SCENE_OBJECT_NAME = strutils::StringId("cloud_data_confirmation_text_1");

static const glm::vec3 BUTTON_SCALE = {0.00045, 0.00045, 0.00045};
static const glm::vec3 USE_CLOUD_DATA_BUTTON_POSITION = {-0.131f, -0.09f, 23.1f};
static const glm::vec3 USE_LOCAL_DATA_BUTTON_POSITION = {-0.151f, -0.175f, 23.1f};

static const float SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS = 0.25f;
static const float STAGGERED_ITEM_ALPHA_DELAY_SECS = 0.1f;

static const std::vector<strutils::StringId> APPLICABLE_SCENE_NAMES =
{
    game_constants::CLOUD_DATA_CONFIRMATION_SCENE
};

///------------------------------------------------------------------------------------------------

const std::vector<strutils::StringId>& CloudDataConfirmationSceneLogicManager::VGetApplicableSceneNames() const
{
    return APPLICABLE_SCENE_NAMES;
}

///------------------------------------------------------------------------------------------------

CloudDataConfirmationSceneLogicManager::CloudDataConfirmationSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

CloudDataConfirmationSceneLogicManager::~CloudDataConfirmationSceneLogicManager(){}

///------------------------------------------------------------------------------------------------

void CloudDataConfirmationSceneLogicManager::VInitSceneCamera(std::shared_ptr<scene::Scene>)
{
}

///------------------------------------------------------------------------------------------------

void CloudDataConfirmationSceneLogicManager::VInitScene(std::shared_ptr<scene::Scene> scene)
{
    mTransitioningToSubScene = false;
    
    // Update cloud data text and re-orient
    auto cloudDataText = DataRepository::GetInstance().GetCloudDataDeviceNameAndTime();
    auto cloudDataTextSceneObject = scene->FindSceneObject(CLOUD_DATA_DEVICE_NAME_AND_TIME_TEXT_SCENE_OBJECT_NAME);
    std::get<scene::TextSceneObjectData>(cloudDataTextSceneObject->mSceneObjectTypeData).mText = cloudDataText;
    auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*cloudDataTextSceneObject);
    auto textLength = boundingRect.topRight.x - boundingRect.bottomLeft.x;
    cloudDataTextSceneObject->mPosition.x -= textLength/2.0f;
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        USE_CLOUD_DATA_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Use Cloud Data",
        USE_CLOUD_DATA_BUTTON_NAME,
        [=]()
        {
            OnUseCloudDataButtonPressed();
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
            mTransitioningToSubScene = true;
        },
        *scene
    ));
    
    mAnimatedButtons.emplace_back(std::make_unique<AnimatedButton>
    (
        USE_LOCAL_DATA_BUTTON_POSITION,
        BUTTON_SCALE,
        game_constants::DEFAULT_FONT_NAME,
        "Keep Local Data",
        USE_LOCAL_DATA_BUTTON_NAME,
        [=]()
        {
            events::EventSystem::GetInstance().DispatchEvent<events::PopSceneModalEvent>();
            mTransitioningToSubScene = true;
        },
        *scene
    ));
    
    size_t sceneObjectIndex = 0;
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 1.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS, animation_flags::NONE, sceneObjectIndex++ * STAGGERED_ITEM_ALPHA_DELAY_SECS), [=]()
        {
            mTransitioningToSubScene = false;
        });
    }
}

///------------------------------------------------------------------------------------------------

void CloudDataConfirmationSceneLogicManager::VUpdate(const float dtMillis, std::shared_ptr<scene::Scene>)
{
    if (mTransitioningToSubScene)
    {
        return;
    }
    
    // Animated buttons
    for (auto& animatedButton: mAnimatedButtons)
    {
        animatedButton->Update(dtMillis);
    }
}

///------------------------------------------------------------------------------------------------

void CloudDataConfirmationSceneLogicManager::VDestroyScene(std::shared_ptr<scene::Scene> scene)
{
    for (auto sceneObject: scene->GetSceneObjects())
    {
        if (sceneObject->mName == game_constants::OVERLAY_SCENE_OBJECT_NAME)
        {
            continue;
        }
        
        CoreSystemsEngine::GetInstance().GetAnimationManager().StartAnimation(std::make_unique<rendering::TweenAlphaAnimation>(sceneObject, 0.0f, SUBSCENE_ITEM_FADE_IN_OUT_DURATION_SECS), [=]()
        {
            sceneObject->mInvisible = true;
        });
    }
}

///------------------------------------------------------------------------------------------------

std::shared_ptr<GuiObjectManager> CloudDataConfirmationSceneLogicManager::VGetGuiObjectManager()
{
    return nullptr;
}

///------------------------------------------------------------------------------------------------

void CloudDataConfirmationSceneLogicManager::OnUseCloudDataButtonPressed()
{
#if defined(MACOS) || defined(MOBILE_FLOW)
    auto checkAndReplacePersistentDataFile = [](const std::string& dataFileNameWithoutExtension)
    {
        std::string dataFileExtension = ".json";
        
        auto cloudFilePath = apple_utils::GetPersistentDataDirectoryPath() + "cloud_" + dataFileNameWithoutExtension + dataFileExtension;
        auto filePath = apple_utils::GetPersistentDataDirectoryPath() + dataFileNameWithoutExtension + dataFileExtension;
        
        std::ifstream cloudFile(cloudFilePath);
        if (cloudFile.is_open())
        {
            std::stringstream buffer;
            buffer << cloudFile.rdbuf();
            auto contents = buffer.str();
            
            std::ofstream dataFile(filePath);
            dataFile << contents;
            dataFile.close();
        }
        
        cloudFile.close();
        std::remove(cloudFilePath.c_str());
    };
    
    checkAndReplacePersistentDataFile("persistent");
    checkAndReplacePersistentDataFile("story");
    checkAndReplacePersistentDataFile("last_battle");
    
    DataRepository::GetInstance().ReloadProgressionDataFromFile();
#endif
}

///------------------------------------------------------------------------------------------------
