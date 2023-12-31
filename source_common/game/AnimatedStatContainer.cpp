///------------------------------------------------------------------------------------------------
///  AnimatedStatContainer.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/11/2023                                                       
///------------------------------------------------------------------------------------------------

#include <game/AnimatedStatContainer.h>
#include <engine/rendering/Animations.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObjectUtils.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

static const std::string BASE_SCENE_OBJECT_NAME_POSTFIX = "base";
static const std::string VALUE_SCENE_OBJECT_NAME_POSTFIX = "value";
static const std::string HEALTH_STAT_CONTAINER_BASE_OBJECT_SHADER = "animated_stat_container_base_object.vs";

static const glm::vec3 STAT_CRYSTAL_SCALE = {0.05f, 0.05f, 1.0f};
static const glm::vec3 STAT_CRYSTAL_VALUE_SCALE = {0.00013f, 0.00013f, 1.0f};
static const glm::vec3 STAT_CRYSTAL_VALUE_POSITION_OFFSET = {0.003, 0.002, 0.02f};
static const float MAX_VALUE_CHANGE_DELAY_SECS = 0.2f;

///------------------------------------------------------------------------------------------------

AnimatedStatContainer::AnimatedStatContainer
(
    const glm::vec3& position,
    const std::string& textureFilename,
    const std::string& crystalName,
    const int& valueToTrack,
    const bool startHidden,
    scene::Scene& scene,
    scene::SnapToEdgeBehavior snapToEdgeBehavior /* = scene::SnapToEdgeBehavior::NONE */,
    const float customScaleFactor /* = 1.0f */
)
    : mValueToTrack(valueToTrack)
    , mScaleFactor(customScaleFactor)
    , mDisplayedValue(valueToTrack)
    , mValueChangeDelaySecs(0.0f)
    , mFinishedAnimating(false)
{
    auto crystalBaseSceneObject = scene.CreateSceneObject(strutils::StringId(crystalName + BASE_SCENE_OBJECT_NAME_POSTFIX));
    crystalBaseSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + textureFilename);
    crystalBaseSceneObject->mShaderResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_SHADERS_ROOT + HEALTH_STAT_CONTAINER_BASE_OBJECT_SHADER);
    crystalBaseSceneObject->mPosition = position;
    crystalBaseSceneObject->mScale = STAT_CRYSTAL_SCALE * mScaleFactor;
    crystalBaseSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = startHidden ? 0.0f : 1.0f;
    crystalBaseSceneObject->mInvisible = startHidden;
    crystalBaseSceneObject->mSnapToEdgeBehavior = snapToEdgeBehavior;
    
    auto crystalValueSceneObject = scene.CreateSceneObject(strutils::StringId(crystalName + VALUE_SCENE_OBJECT_NAME_POSTFIX));
    scene::TextSceneObjectData crystalValueTextData;
    crystalValueTextData.mFontName = game_constants::DEFAULT_FONT_NAME;
    crystalValueSceneObject->mSceneObjectTypeData = std::move(crystalValueTextData);
    crystalValueSceneObject->mScale = STAT_CRYSTAL_VALUE_SCALE * mScaleFactor; 
    crystalValueSceneObject->mPosition = crystalBaseSceneObject->mPosition + STAT_CRYSTAL_VALUE_POSITION_OFFSET;
    crystalValueSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = startHidden ? 0.0f : 1.0f;
    crystalValueSceneObject->mInvisible = startHidden;
    crystalValueSceneObject->mSnapToEdgeBehavior = snapToEdgeBehavior;
    
    mSceneObjects.push_back(crystalBaseSceneObject);
    mSceneObjects.push_back(crystalValueSceneObject);
    
    // To init the text values
    Update(0.0f);
}

///------------------------------------------------------------------------------------------------

AnimatedStatContainer::~AnimatedStatContainer()
{
}

///------------------------------------------------------------------------------------------------

AnimatedStatContainerUpdateResult AnimatedStatContainer::Update(const float dtMillis)
{
    AnimatedStatContainerUpdateResult updateResult = AnimatedStatContainerUpdateResult::ONGOING;
    
    auto baseCrystalSo = mSceneObjects.front();
    auto valueCrystalSo = mSceneObjects.back();
    
    if (mDisplayedValue != mValueToTrack)
    {
        mValueChangeDelaySecs -= dtMillis/1000.0f;
        if (mValueChangeDelaySecs <= 0.0f)
        {
            mValueChangeDelaySecs = MAX_VALUE_CHANGE_DELAY_SECS;
            
            if (mDisplayedValue < mValueToTrack) mDisplayedValue++;
            if (mDisplayedValue > mValueToTrack) mDisplayedValue--;
            
            mFinishedAnimating = false;
            auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
            auto originalValueScale = valueCrystalSo->mScale;
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(valueCrystalSo, valueCrystalSo->mPosition, originalValueScale * 1.5f, MAX_VALUE_CHANGE_DELAY_SECS/3, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
            {
                auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(mSceneObjects.back(), mSceneObjects.back()->mPosition, originalValueScale, MAX_VALUE_CHANGE_DELAY_SECS/3, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                {
                    mFinishedAnimating = true;
                    mSceneObjects.back()->mScale = STAT_CRYSTAL_VALUE_SCALE * mScaleFactor;
                });
            });
            
            auto originalGemScale = baseCrystalSo->mScale;
            animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(baseCrystalSo, baseCrystalSo->mPosition, originalGemScale * 1.5f, MAX_VALUE_CHANGE_DELAY_SECS/3, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
            {
                auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
                animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(mSceneObjects.front(), mSceneObjects.front()->mPosition, originalGemScale, MAX_VALUE_CHANGE_DELAY_SECS/3, animation_flags::IGNORE_X_COMPONENT | animation_flags::IGNORE_Y_COMPONENT | animation_flags::IGNORE_Z_COMPONENT, 0.0f, math::LinearFunction, math::TweeningMode::EASE_OUT), [=]()
                {
                    mFinishedAnimating = true;
                    mSceneObjects.front()->mScale = STAT_CRYSTAL_SCALE * mScaleFactor;
                });
            });
        }
    }
    else if (mFinishedAnimating)
    {
        updateResult = AnimatedStatContainerUpdateResult::FINISHED;
    }
    
    std::get<scene::TextSceneObjectData>(valueCrystalSo->mSceneObjectTypeData).mText = std::to_string(mDisplayedValue);
    valueCrystalSo->mPosition = baseCrystalSo->mPosition + STAT_CRYSTAL_VALUE_POSITION_OFFSET;
    
    auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*valueCrystalSo);
    valueCrystalSo->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
    
    
    return updateResult;
}

///------------------------------------------------------------------------------------------------

std::vector<std::shared_ptr<scene::SceneObject>>& AnimatedStatContainer::GetSceneObjects()
{
    return mSceneObjects;
}

///------------------------------------------------------------------------------------------------

void AnimatedStatContainer::ForceSetDisplayedValue(const int displayedValue)
{
    mDisplayedValue = displayedValue;
    
    auto baseCrystalSo = mSceneObjects.front();
    auto valueCrystalSo = mSceneObjects.back();
    
    std::get<scene::TextSceneObjectData>(valueCrystalSo->mSceneObjectTypeData).mText = std::to_string(mDisplayedValue);
    valueCrystalSo->mPosition = baseCrystalSo->mPosition + STAT_CRYSTAL_VALUE_POSITION_OFFSET;
    
    auto boundingRect = scene_object_utils::GetSceneObjectBoundingRect(*valueCrystalSo);
    valueCrystalSo->mPosition.x -= (boundingRect.topRight.x - boundingRect.bottomLeft.x)/2.0f;
}

///------------------------------------------------------------------------------------------------
