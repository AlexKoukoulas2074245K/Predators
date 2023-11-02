///------------------------------------------------------------------------------------------------
///  AnimatedStatCrystal.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/AnimatedStatCrystal.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObjectUtils.h>

///------------------------------------------------------------------------------------------------

static const std::string BASE_SCENE_OBJECT_NAME_POSTFIX = "_BASE";
static const std::string VALUE_SCENE_OBJECT_NAME_POSTFIX = "_VALUE";

static const glm::vec3 STAT_CRYSTAL_SCALE = {0.05f, 0.05f, 1.0f};
static const glm::vec3 STAT_CRYSTAL_VALUE_SCALE = {0.00015f, 0.00015f, 1.0f};
static const glm::vec3 STAT_CRYSTAL_VALUE_POSITION_OFFSET = {0.003, 0.002, 0.1f};
static const float MAX_VALUE_CHANGE_DELAY_SECS = 0.1f;

///------------------------------------------------------------------------------------------------

AnimatedStatCrystal::AnimatedStatCrystal(const glm::vec3& position, const std::string& textureFilename, const std::string& crystalName, const int& valueToTrack, scene::Scene& scene)
    : mValueToTrack(valueToTrack)
    , mDisplayedValue(valueToTrack)
    , mValueChangeDelaySecs(0.0f)
    , mScene(scene)
{
    auto crystalBaseSceneObject = scene.CreateSceneObject(strutils::StringId(crystalName + BASE_SCENE_OBJECT_NAME_POSTFIX));
    crystalBaseSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + textureFilename);
    crystalBaseSceneObject->mPosition = position;
    crystalBaseSceneObject->mScale = STAT_CRYSTAL_SCALE;
    
    auto crystalValueSceneObject = scene.CreateSceneObject(strutils::StringId(crystalName + VALUE_SCENE_OBJECT_NAME_POSTFIX));
    scene::TextSceneObjectData crystalValueTextData;
    crystalValueTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
    crystalValueSceneObject->mSceneObjectTypeData = std::move(crystalValueTextData);
    crystalValueSceneObject->mScale = STAT_CRYSTAL_VALUE_SCALE;
    crystalValueSceneObject->mPosition = crystalBaseSceneObject->mPosition + STAT_CRYSTAL_VALUE_POSITION_OFFSET;
    
    mSceneObjects.push_back(crystalBaseSceneObject);
    mSceneObjects.push_back(crystalValueSceneObject);
    
    // To init the text values
    Update(0.0f);
}

///------------------------------------------------------------------------------------------------

AnimatedStatCrystal::~AnimatedStatCrystal()
{
    for (auto sceneObject: mSceneObjects)
    {
        mScene.RemoveSceneObject(sceneObject->mName);
    }
}

///------------------------------------------------------------------------------------------------

AnimatedStatCrystalUpdateResult AnimatedStatCrystal::Update(const float dtMillis)
{
    AnimatedStatCrystalUpdateResult updateResult = AnimatedStatCrystalUpdateResult::ONGOING;
    
    if (mDisplayedValue != mValueToTrack)
    {
        mValueChangeDelaySecs -= dtMillis/1000.0f;
        if (mValueChangeDelaySecs <= 0.0f)
        {
            mValueChangeDelaySecs = MAX_VALUE_CHANGE_DELAY_SECS;
            
            if (mDisplayedValue < mValueToTrack) mDisplayedValue++;
            if (mDisplayedValue > mValueToTrack) mDisplayedValue--;
        }
    }
    else
    {
        updateResult = AnimatedStatCrystalUpdateResult::FINISHED;
    }
    
    auto baseCrystalSo = mSceneObjects.front();
    auto valueCrystalSo = mSceneObjects.back();
    
    std::get<scene::TextSceneObjectData>(valueCrystalSo->mSceneObjectTypeData).mText = std::to_string(mDisplayedValue);
    valueCrystalSo->mPosition = baseCrystalSo->mPosition + STAT_CRYSTAL_VALUE_POSITION_OFFSET;
    
    auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*valueCrystalSo);
    valueCrystalSo->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
    
    return updateResult;
}

///------------------------------------------------------------------------------------------------
