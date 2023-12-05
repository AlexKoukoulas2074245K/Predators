///------------------------------------------------------------------------------------------------
///  Camera.cpp                                                                                        
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 20/09/2023
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Camera.h>
#include <engine/utils/Logging.h>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

const glm::vec3 Camera::DEFAULT_CAMERA_POSITION     = {0.0f, -0.0087f, -5.0f};
const glm::vec3 Camera::DEFAULT_CAMERA_FRONT_VECTOR = {0.0f, 0.0f, -1.0f};
const glm::vec3 Camera::DEFAULT_CAMERA_UP_VECTOR    = {0.0f, 1.0f, 0.0f};

const float Camera::DEVICE_INVARIABLE_ASPECT = 0.46f;

const float Camera::DEFAULT_CAMERA_ZNEAR       = -50.0f;
const float Camera::DEFAULT_CAMERA_ZFAR        = 50.0f;
const float Camera::DEFAULT_CAMERA_ZOOM_FACTOR = 60.0f;

const float Camera::DEFAULT_SHAKE_STRENGTH_RADIUS = 0.05f;
const float Camera::SHAKE_MIN_RADIUS = 0.00001f;

///------------------------------------------------------------------------------------------------

Camera::Camera()
: Camera(30.0f)
{
    
}

///------------------------------------------------------------------------------------------------

Camera::Camera(const float cameraLenseHeight)
    : mZoomFactor(DEFAULT_CAMERA_ZOOM_FACTOR)
    , mTargetAspectRatio(CoreSystemsEngine::GetInstance().GetDefaultAspectRatio())
    , mPosition(DEFAULT_CAMERA_POSITION)
{
    mCameraLenseWidth = cameraLenseHeight * DEVICE_INVARIABLE_ASPECT;
    mCameraLenseHeight = cameraLenseHeight;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------

void Camera::RecalculateMatrices()
{
    float previousZoomFactor = mZoomFactor;
    const auto& windowDimensions = CoreSystemsEngine::GetInstance().GetContextRenderableDimensions();
    const auto& currentAspect = static_cast<float>(windowDimensions.x)/windowDimensions.y;
    const auto& currentToDefaultAspectRatio = (currentAspect/mTargetAspectRatio + 1.0f)/2.0f;
    mZoomFactor = previousZoomFactor * currentToDefaultAspectRatio;
    //logging::Log(logging::LogType::INFO, "Recalculating Matrices for %.3f, %.3f (AR %.6f)", windowDimensions.x, windowDimensions.y, windowDimensions.x/windowDimensions.y);
    
    float aspect = windowDimensions.x/windowDimensions.y;
    mView = glm::lookAt(mPosition, mPosition + DEFAULT_CAMERA_FRONT_VECTOR, DEFAULT_CAMERA_UP_VECTOR);
    mProj = glm::ortho((-mCameraLenseWidth/(DEVICE_INVARIABLE_ASPECT/aspect))/2.0f/mZoomFactor, (mCameraLenseWidth/((DEVICE_INVARIABLE_ASPECT/aspect)))/2.0f/mZoomFactor, -mCameraLenseHeight/2.0f/mZoomFactor, mCameraLenseHeight/2.0f/mZoomFactor, DEFAULT_CAMERA_ZNEAR, DEFAULT_CAMERA_ZFAR);
    
    mTargetAspectRatio = currentAspect;
}

///------------------------------------------------------------------------------------------------

float Camera::GetZoomFactor() const
{
    return mZoomFactor;
}

///------------------------------------------------------------------------------------------------

float Camera::GetCameraLenseWidth() const
{
    return mCameraLenseWidth;
}

///------------------------------------------------------------------------------------------------

float Camera::GetCameraLenseHeight() const
{
    return mCameraLenseHeight;
}

///------------------------------------------------------------------------------------------------

const glm::vec3& Camera::GetPosition() const
{
    return mPosition;
}

///------------------------------------------------------------------------------------------------

const glm::mat4& Camera::GetViewMatrix() const
{
    return mView;
}

///------------------------------------------------------------------------------------------------

const glm::mat4& Camera::GetProjMatrix() const
{
    return mProj;
}

///------------------------------------------------------------------------------------------------

math::Frustum Camera::CalculateFrustum() const
{
    math::Frustum cameraFrustum;
    auto viewProjectionMatrix = mProj * mView;

    // Extract rows from combined view projection matrix
    const auto rowX = glm::row(viewProjectionMatrix, 0);
    const auto rowY = glm::row(viewProjectionMatrix, 1);
    const auto rowZ = glm::row(viewProjectionMatrix, 2);
    const auto rowW = glm::row(viewProjectionMatrix, 3);

    // Calculate planes
    cameraFrustum[0] = glm::normalize(rowW + rowX);
    cameraFrustum[1] = glm::normalize(rowW - rowX);
    cameraFrustum[2] = glm::normalize(rowW + rowY);
    cameraFrustum[3] = glm::normalize(rowW - rowY);
    cameraFrustum[4] = glm::normalize(rowW + rowZ);
    cameraFrustum[5] = glm::normalize(rowW - rowZ);

    // Normalize planes
    for (auto i = 0U; i < math::FRUSTUM_SIDES; ++i)
    {
        glm::vec3 planeNormal(cameraFrustum[i].x, cameraFrustum[i].y, cameraFrustum[i].z);
        const auto length = glm::length(planeNormal);
        cameraFrustum[i] = -cameraFrustum[i] / length;
    }

    return cameraFrustum;
}

///------------------------------------------------------------------------------------------------

void Camera::Shake(const float targetDurationSecs, const float shakeStrengthRadius /* = DEFAULT_SHAKE_STRENGTH_RADIUS */, const float shakeInterTremmorDelaySecs /* = 0.0f */)
{
    if (mShakeData.mShakeCurrentRadius <= SHAKE_MIN_RADIUS)
    {
        mShakeData.mPreShakePosition = mPosition;
        mShakeData.mShakeTimeAccumulatorMillis = 0.0f;
        mShakeData.mShakeTargetDurationMillis = targetDurationSecs * 1000.0f;
        mShakeData.mShakeStrengthRadius = shakeStrengthRadius;
        mShakeData.mShakeCurrentRadius = shakeStrengthRadius;
        mShakeData.mShakeInterTremmorAccumMillis = 0.0;
        mShakeData.mShakeInterTremmorDelayMillis = shakeInterTremmorDelaySecs * 1000.0f;
        
        mShakeData.mShakeRandomAngle = math::RandomFloat(0.0f, 2.0f * math::PI);
        auto offset = glm::vec2(math::Sinf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeCurrentRadius, math::Cosf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeCurrentRadius);
        
        SetPosition(glm::vec3(mShakeData.mPreShakePosition.x + offset.x, mShakeData.mPreShakePosition.y + offset.y, GetPosition().z));
    }
}

///------------------------------------------------------------------------------------------------

void Camera::Update(const float dtMillis)
{
    if (mShakeData.mShakeCurrentRadius > SHAKE_MIN_RADIUS)
    {
        if (mShakeData.mShakeInterTremmorDelayMillis > 0.0f)
        {
            mShakeData.mShakeInterTremmorAccumMillis += dtMillis;
            if (mShakeData.mShakeInterTremmorAccumMillis < mShakeData.mShakeInterTremmorDelayMillis)
            {
                return;
            }
            else
            {
                mShakeData.mShakeInterTremmorAccumMillis -= mShakeData.mShakeInterTremmorDelayMillis;
            }
        }
        
        mShakeData.mShakeCurrentRadius = mShakeData.mShakeStrengthRadius * (1.0f - (mShakeData.mShakeTimeAccumulatorMillis/mShakeData.mShakeTargetDurationMillis));
        mShakeData.mShakeTimeAccumulatorMillis += dtMillis;
        
        if (mShakeData.mShakeCurrentRadius <= SHAKE_MIN_RADIUS)
        {
            mShakeData.mShakeCurrentRadius = SHAKE_MIN_RADIUS;
            SetPosition(mShakeData.mPreShakePosition);
        }
        else
        {
            mShakeData.mShakeRandomAngle = math::RandomFloat(0.0f, 2.0f * math::PI);
            auto offset = glm::vec2(math::Sinf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeCurrentRadius, math::Cosf(mShakeData.mShakeRandomAngle) * mShakeData.mShakeCurrentRadius);
            
            SetPosition(glm::vec3(mShakeData.mPreShakePosition.x + offset.x, mShakeData.mPreShakePosition.y + offset.y, GetPosition().z));
        }
    }
}

///------------------------------------------------------------------------------------------------

void Camera::StopShake()
{
    mShakeData.mShakeCurrentRadius = 0.0f;
}

///------------------------------------------------------------------------------------------------

void Camera::SetZoomFactor(const float zoomFactor)
{
    mZoomFactor = zoomFactor;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------

void Camera::SetPosition(const glm::vec3& position)
{
    mPosition = position;
    RecalculateMatrices();
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
