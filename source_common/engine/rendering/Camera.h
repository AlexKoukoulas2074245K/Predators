///------------------------------------------------------------------------------------------------
///  Camera.h                                                                                          
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 20/09/2023
///------------------------------------------------------------------------------------------------

#ifndef Camera_h
#define Camera_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class Camera final
{
public:
    static const float DEFAULT_SHAKE_STRENGTH_RADIUS;
    
public:
    Camera();
    Camera(const float cameraLenseHeight);
    
    void RecalculateMatrices();
    
    float GetZoomFactor() const;
    float GetCameraLenseWidth() const;
    float GetCameraLenseHeight() const;
    const glm::vec3& GetPosition() const;
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjMatrix() const;
    
    ///-----------------------------------------------------------------------------------------------
    /// Performs a camera shake.
    /// @param[in] durationSecs the target (to be exceeded if shakeInterTremmorDelaySecs is set to a value > 0) seconds the shake will run for
    /// @param[in] shakeStrengthRadius (optional) sets the starting radius offset for the camera shake. defaults to 0.05f
    /// @param[in] shakeInterTremmorDelaySecs (optional) specifies the delay in between shake tremmors. Will also naturally lengthen the shake duration
    /// specified by durationSecs. Sane values are around 0.01f - 0.1f
    void Shake(const float targetDurationSecs, const float shakeStrengthRadius = DEFAULT_SHAKE_STRENGTH_RADIUS, const float shakeInterTremmorDelaySecs = 0.0f);
    
    void Update(const float dtMillis);
    void SetZoomFactor(const float zoomFactor);
    void SetPosition(const glm::vec3& position);
    
private:
    static const glm::vec3 DEFAULT_CAMERA_POSITION;
    static const glm::vec3 DEFAULT_CAMERA_FRONT_VECTOR;
    static const glm::vec3 DEFAULT_CAMERA_UP_VECTOR;
    static const float DEVICE_INVARIABLE_ASPECT;
    static const float DEFAULT_CAMERA_ZNEAR;
    static const float DEFAULT_CAMERA_ZFAR;
    static const float DEFAULT_CAMERA_ZOOM_FACTOR;
    static const float SHAKE_MIN_RADIUS;
    
private:
    struct ShakeData
    {
        glm::vec3 mPreShakePosition;
        float mShakeCurrentRadius = 0.0f;
        float mShakeStrengthRadius = 0.0f;
        float mShakeRandomAngle = 0.0f;
        float mShakeTargetDurationMillis = 0.0f;
        float mShakeTimeAccumulatorMillis = 0.0f;
        float mShakeInterTremmorDelayMillis = 0.0f;
        float mShakeInterTremmorAccumMillis = 0.0f;
    };
    
    ShakeData mShakeData;
    float mZoomFactor;
    float mCameraLenseWidth;
    float mCameraLenseHeight;
    glm::vec3 mPosition;
    glm::mat4 mView;
    glm::mat4 mProj;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* Camera_h */
