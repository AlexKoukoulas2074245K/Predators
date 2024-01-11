///------------------------------------------------------------------------------------------------
///  WheelOfFortuneController.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 08/01/2024                                                       
///------------------------------------------------------------------------------------------------

#include <game/WheelOfFortuneController.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

static const strutils::StringId WHEEL_BASE_SCENE_OBJECT_NAME = strutils::StringId("wheel_base");
static const strutils::StringId WHEEL_POINTER_SCENE_OBJECT_NAME = strutils::StringId("wheel_pointer");
static const strutils::StringId WHEEL_CENTER_SCENE_OBJECT_NAME = strutils::StringId("wheel_center");

static const std::string WHEEL_ITEM_SCENE_OBJECT_NAME_PREFIX = "wheel_item_";
static const std::string WHEEL_BASE_TEXTURE_FILE_NAME = "wheel_of_fortune.png";
static const std::string WHEEL_POINTER_TEXTURE_FILE_NAME = "wheel_of_fortune_pointer.png";
static const std::string WHEEL_CENTER_TEXTURE_FILE_NAME = "wheel_of_fortune_center.png";

static const glm::vec3 WHEEL_BASE_POSITION = {-0.05f, -0.05f, 23.1f};
static const glm::vec3 WHEEL_COMPONENTS_POSITION = {-0.05f, -0.05f, 23.2f};
static const glm::vec3 WHEEL_BASE_SCALE = {0.35f, 0.35f, 0.35f};

static const glm::vec2 WHEEL_ROTATION_MULTIPLIER_RANDOM_RANGE = {800.0f, 1200.0f};
static const float WHEEL_SPIN_ROTATION_DAMPING = 0.98f;
static const float WHEEL_MIN_ROTATION_SPEED = 0.00008f;
static const float WHEEL_INITIAL_SLOW_ROTATION_SPEED = 0.0002f;
static const float WHEEL_SPEED_DELTA_MILLIS = 16.6666f;

///------------------------------------------------------------------------------------------------

WheelOfFortuneController::WheelOfFortuneController(scene::Scene& scene, const std::vector<std::string>& itemTextures, std::function<void(const int, const std::shared_ptr<scene::SceneObject>)> onItemSelectedCallback)
    : mScene(scene)
    , mItems(itemTextures)
    , mOnItemSelectedCallback(onItemSelectedCallback)
    , mWheelRotationSpeed(0.0f)
    , mWheelRotation(0.0f)
{
    auto wheelBaseSceneObject = mScene.CreateSceneObject(strutils::StringId(WHEEL_BASE_SCENE_OBJECT_NAME));
    wheelBaseSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + WHEEL_BASE_TEXTURE_FILE_NAME);
    wheelBaseSceneObject->mPosition = WHEEL_BASE_POSITION;
    wheelBaseSceneObject->mScale = WHEEL_BASE_SCALE;
    wheelBaseSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    auto wheelPointerSceneObject = mScene.CreateSceneObject(strutils::StringId(WHEEL_POINTER_SCENE_OBJECT_NAME));
    wheelPointerSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + WHEEL_POINTER_TEXTURE_FILE_NAME);
    wheelPointerSceneObject->mPosition = WHEEL_COMPONENTS_POSITION;
    wheelPointerSceneObject->mScale = WHEEL_BASE_SCALE;
    wheelPointerSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    auto wheelCenterSceneObject = mScene.CreateSceneObject(strutils::StringId(WHEEL_CENTER_SCENE_OBJECT_NAME));
    wheelCenterSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + WHEEL_CENTER_TEXTURE_FILE_NAME);
    wheelCenterSceneObject->mPosition = WHEEL_COMPONENTS_POSITION;
    wheelCenterSceneObject->mScale = WHEEL_BASE_SCALE;
    wheelCenterSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    
    for (auto i = 0U; i < mItems.size(); ++i)
    {
        auto wheelCenterSceneObject = mScene.CreateSceneObject(strutils::StringId(WHEEL_ITEM_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)));
        wheelCenterSceneObject->mTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().LoadResource(resources::ResourceLoadingService::RES_TEXTURES_ROOT + mItems[i]);
        wheelCenterSceneObject->mPosition = WHEEL_COMPONENTS_POSITION;
        wheelCenterSceneObject->mScale = WHEEL_BASE_SCALE;
        wheelCenterSceneObject->mRotation.z -= i * math::PI/6;
        wheelCenterSceneObject->mShaderFloatUniformValues[game_constants::CUSTOM_ALPHA_UNIFORM_NAME] = 0.0f;
    }
    
    mState = WheelState::INITIAL_SLOW_ROTATION;
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneController::Spin()
{
    mWheelRotationSpeed = WHEEL_INITIAL_SLOW_ROTATION_SPEED * math::ControlledRandomFloat(WHEEL_ROTATION_MULTIPLIER_RANDOM_RANGE.s, WHEEL_ROTATION_MULTIPLIER_RANDOM_RANGE.t);
    mState = WheelState::SPINNING;
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneController::Update(const float)
{
    switch (mState)
    {
        case WheelState::INITIAL_SLOW_ROTATION:
        {
            mWheelRotationSpeed = 0.0f;
            
        } break;
            
        case WheelState::SPINNING:
        {
            mWheelRotationSpeed = mWheelRotationSpeed * WHEEL_SPIN_ROTATION_DAMPING;
            if (mWheelRotationSpeed < WHEEL_MIN_ROTATION_SPEED)
            {
                mWheelRotationSpeed = 0.0f;
                
                // Calculate pointee index
                auto sliceIndexFloat = (mWheelRotation + math::PI/12)/(-math::PI/6);
                auto itemIndex = static_cast<int>(sliceIndexFloat < 0.0f ? 0 : (mItems.size() - 1) - static_cast<int>(sliceIndexFloat));
                
                mOnItemSelectedCallback(itemIndex, mScene.FindSceneObject(strutils::StringId(WHEEL_ITEM_SCENE_OBJECT_NAME_PREFIX + std::to_string(itemIndex))));
                
                mState = WheelState::FINISHED;
            }
        } break;
            
        default: break;
    }
            
    mWheelRotation -= mWheelRotationSpeed * WHEEL_SPEED_DELTA_MILLIS;
    if (mWheelRotation < -math::PI * 2.0f)
    {
        mWheelRotation += math::PI * 2.0f;
    }
    
    ApplyRotationToItems();
}

///------------------------------------------------------------------------------------------------

std::vector<std::shared_ptr<scene::SceneObject>> WheelOfFortuneController::GetSceneObjects() const
{
    return mSceneObjects;
}

///------------------------------------------------------------------------------------------------

void WheelOfFortuneController::ApplyRotationToItems()
{
    mScene.FindSceneObject(WHEEL_BASE_SCENE_OBJECT_NAME)->mRotation.z = mWheelRotation;
    
    for (auto i = 0U; i < mItems.size(); ++i)
    {
        mScene.FindSceneObject(strutils::StringId(WHEEL_ITEM_SCENE_OBJECT_NAME_PREFIX + std::to_string(i)))->mRotation.z = -(i * math::PI/6) + mWheelRotation;
    }
}

///------------------------------------------------------------------------------------------------
