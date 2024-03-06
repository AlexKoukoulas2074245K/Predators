///------------------------------------------------------------------------------------------------
///  TutorialManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 06/03/2024
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/resloading/DataFileResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/BaseDataFileDeserializer.h>
#include <engine/utils/Logging.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneManager.h>
#include <engine/scene/SceneObject.h>
#include <game/DataRepository.h>
#include <game/TutorialManager.h>
#include <nlohmann/json.hpp>

///------------------------------------------------------------------------------------------------

static constexpr int TUTORIAL_TEXT_ROWS_COUNT = 7;

static const strutils::StringId TUTORIAL_BASE_SCENE_OBJECT_NAME = strutils::StringId("tutorial_base");
static const strutils::StringId TUTORIAL_REVEAL_THRESHOLD_UNIFORM_NAME = strutils::StringId("reveal_threshold");
static const strutils::StringId TUTORIAL_REVEAL_RGB_EXPONENT_UNIFORM_NAME = strutils::StringId("reveal_rgb_exponent");
static const strutils::StringId TUTORIAL_TEXT_SCENE_OBJECT_NAMES [TUTORIAL_TEXT_ROWS_COUNT] =
{
    strutils::StringId("tutorial_text_0"),
    strutils::StringId("tutorial_text_1"),
    strutils::StringId("tutorial_text_2"),
    strutils::StringId("tutorial_text_3"),
    strutils::StringId("tutorial_text_4"),
    strutils::StringId("tutorial_text_5"),
    strutils::StringId("tutorial_text_6"),
};

static const std::string TUTORIAL_TEXTURE_FILE_NAME = "tutorial.png";
static const std::string TUTORIAL_SHADER_FILE_NAME = "diagonal_reveal.vs";

static const glm::vec3 TUTORIAL_BASE_POSITION = {0.0f, 0.0f, 27.0f};
static const glm::vec3 TUTORIAL_TEXT_SCALE = {0.00032f, 0.00032f, 0.00032f};
static const glm::vec3 TUTORIAL_BASE_SCALE = {0.4f, 0.4f, 0.4f};
static const glm::vec3 TUTORIAL_TEXT_OFFSETS[TUTORIAL_TEXT_ROWS_COUNT] =
{
    { -0.042f, 0.137f, 0.1f},
    { -0.13f, 0.097f, 0.1f },
    { -0.13f, 0.063f, 0.1f },
    { -0.13f, 0.029f, 0.1f },
    { -0.13f, -0.005f, 0.1f },
    { -0.13f, -0.039f, 0.1f },
    { -0.13f, -0.073f, 0.1f },
};

static const float TUTORIAL_MAX_REVEAL_THRESHOLD = 2.5f;
static const float TUTORIAL_REVEAL_SPEED = 1.0f/200.0f;
static const float TUTORIAL_TEXT_REVEAL_SPEED = 1.0f/500.0f;

///------------------------------------------------------------------------------------------------

TutorialManager::TutorialManager()
{
    events::EventSystem::GetInstance().RegisterForEvent<events::TutorialTriggerEvent>(this, &TutorialManager::OnTutorialTrigger);
}

///------------------------------------------------------------------------------------------------

const std::unordered_map<strutils::StringId, TutorialDefinition, strutils::StringIdHasher>& TutorialManager::GetTutorialDefinitions() const
{
    return mTutorialDefinitions;
}

///------------------------------------------------------------------------------------------------

bool TutorialManager::HasAnyActiveTutorial() const
{
    return !mActiveTutorials.empty();
}

///------------------------------------------------------------------------------------------------

bool TutorialManager::IsTutorialActive(const strutils::StringId& tutorialName) const
{
    return std::find(mActiveTutorials.cbegin(), mActiveTutorials.cend(), tutorialName) != mActiveTutorials.cend();
}

///------------------------------------------------------------------------------------------------

void TutorialManager::LoadTutorialDefinitions()
{
    auto& systemsEngine = CoreSystemsEngine::GetInstance();
    auto tutorialDefinitionJsonResourceId = systemsEngine.GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_DATA_ROOT + "tutorial_definitions.json", resources::DONT_RELOAD);
    const auto tutorialsJson =  nlohmann::json::parse(systemsEngine.GetResourceLoadingService().GetResource<resources::DataFileResource>(tutorialDefinitionJsonResourceId).GetContents());
    
    for (const auto& tutorialDefinitionObject: tutorialsJson["tutorial_definitions"])
    {
        strutils::StringId tutorialName = strutils::StringId(tutorialDefinitionObject["name"].get<std::string>());
        std::string tutorialDescription = tutorialDefinitionObject["description"].get<std::string>();
        
        mTutorialDefinitions.emplace(std::make_pair(tutorialName, TutorialDefinition(tutorialName, tutorialDescription)));
    }
}

///------------------------------------------------------------------------------------------------

void TutorialManager::Update(const float dtMillis)
{
    if (!mActiveTutorials.empty())
    {
        // Tutorial active but not created yet. Create it.
        if (mTutorialSceneObjects.empty())
        {
            CreateTutorial();
        }
        // Tutorial active and created. Update it
        else
        {
            UpdateActiveTutorial(dtMillis);
        }
    }
}

///------------------------------------------------------------------------------------------------

void TutorialManager::CreateTutorial()
{
    const auto& tutorialDefinition = mTutorialDefinitions.at(mActiveTutorials.front());
    
    auto seenTutorials = DataRepository::GetInstance().GetSeenTutorials();
    seenTutorials.push_back(mActiveTutorials.front());
    DataRepository::GetInstance().SetSeenTutorials(seenTutorials);
    DataRepository::GetInstance().FlushStateToFile();

    auto tutorialScene = CoreSystemsEngine::GetInstance().GetSceneManager().CreateScene(game_constants::TUTORIAL_SCENE_NAME);
    tutorialScene->SetLoaded(true);
    
    auto tutorialSceneObject = tutorialScene->CreateSceneObject(TUTORIAL_BASE_SCENE_OBJECT_NAME);
    
    tutorialSceneObject->mPosition = TUTORIAL_BASE_POSITION;
    tutorialSceneObject->mScale = TUTORIAL_BASE_SCALE;
    tutorialSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + TUTORIAL_TEXTURE_FILE_NAME);
    tutorialSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + TUTORIAL_SHADER_FILE_NAME);
    
    tutorialSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 1.0f;
    tutorialSceneObject->mShaderFloatUniformValues[TUTORIAL_REVEAL_THRESHOLD_UNIFORM_NAME] = 0.0f;
    tutorialSceneObject->mShaderFloatUniformValues[TUTORIAL_REVEAL_RGB_EXPONENT_UNIFORM_NAME] =  1.127f;
    
    mTutorialSceneObjects.push_back(tutorialSceneObject);
    
    auto tutorialTextRows = strutils::StringSplit(tutorialDefinition.mTutorialDescription, '$');
    tutorialTextRows.insert(tutorialTextRows.begin(), "Tutorial");
    
    for (auto i = 0U; i < tutorialTextRows.size(); ++i)
    {
        assert(i < TUTORIAL_TEXT_ROWS_COUNT);
        auto tutorialTextSceneObject = tutorialScene->CreateSceneObject(TUTORIAL_TEXT_SCENE_OBJECT_NAMES[i]);
        tutorialTextSceneObject->mScale = TUTORIAL_TEXT_SCALE;
        tutorialTextSceneObject->mPosition = tutorialSceneObject->mPosition;
        tutorialTextSceneObject->mPosition += TUTORIAL_TEXT_OFFSETS[i];
        tutorialTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
        
        scene::TextSceneObjectData textData;
        textData.mFontName = game_constants::DEFAULT_FONT_BLACK_NAME;
        textData.mText = tutorialTextRows[i];
        tutorialTextSceneObject->mSceneObjectTypeData = std::move(textData);
        
        mTutorialSceneObjects.push_back(tutorialTextSceneObject);
    }
}

///------------------------------------------------------------------------------------------------

void TutorialManager::UpdateActiveTutorial(const float dtMillis)
{
    mTutorialSceneObjects[0]->mShaderFloatUniformValues[TUTORIAL_REVEAL_THRESHOLD_UNIFORM_NAME] += dtMillis * TUTORIAL_REVEAL_SPEED;
    if (mTutorialSceneObjects[0]->mShaderFloatUniformValues[TUTORIAL_REVEAL_THRESHOLD_UNIFORM_NAME] >= TUTORIAL_MAX_REVEAL_THRESHOLD)
    {
        mTutorialSceneObjects[0]->mShaderFloatUniformValues[TUTORIAL_REVEAL_THRESHOLD_UNIFORM_NAME] = TUTORIAL_MAX_REVEAL_THRESHOLD;
        
        for (auto i = 1U; i < mTutorialSceneObjects.size(); ++i)
        {
            auto tutorialTextSceneObject = mTutorialSceneObjects[i];
            tutorialTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = math::Min(1.0f, tutorialTextSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] +  dtMillis * TUTORIAL_TEXT_REVEAL_SPEED);
        }
    }
}

///------------------------------------------------------------------------------------------------

void TutorialManager::OnTutorialTrigger(const events::TutorialTriggerEvent& event)
{
    // Tutorials not active
    if (!DataRepository::GetInstance().AreTutorialsEnabled())
    {
        return;
    }
    
    // Tutorial seen already
    const auto& seenTutorials = DataRepository::GetInstance().GetSeenTutorials();
    if (std::find(seenTutorials.cbegin(), seenTutorials.cend(), event.mTutorialName) != seenTutorials.cend())
    {
        return;
    }
    
    // Tutorial already queued up
    if (IsTutorialActive(event.mTutorialName))
    {
        return;
    }
    
    // Tutorial definition not found
    if (mTutorialDefinitions.count(event.mTutorialName) == 0)
    {
        logging::Log(logging::LogType::ERROR, "Tried to surface unknown tutorial %s", event.mTutorialName.GetString().c_str());
        assert(false);
        return;
    }
    
    mActiveTutorials.push_back(event.mTutorialName);
}

///------------------------------------------------------------------------------------------------
