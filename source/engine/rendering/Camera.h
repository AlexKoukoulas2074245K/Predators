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
    Camera();
    Camera(const float cameraLenseHeight);
    
    void RecalculateMatrices();
    
    float GetZoomFactor() const;
    float GetCameraLenseWidth() const;
    float GetCameraLenseHeight() const;
    const glm::vec3& GetPosition() const;
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjMatrix() const;
    
    void Shake();
    void Update(const float dtMillis);
    void SetZoomFactor(const float zoomFactor);
    void SetPosition(const glm::vec3& position);
    
public:
    static const glm::vec3 DEFAULT_CAMERA_POSITION;
    static const glm::vec3 DEFAULT_CAMERA_FRONT_VECTOR;
    static const glm::vec3 DEFAULT_CAMERA_UP_VECTOR;
    static const float DEVICE_INVARIABLE_ASPECT;
    static const float DEFAULT_CAMERA_ZNEAR;
    static const float DEFAULT_CAMERA_ZFAR;
    static const float DEFAULT_CAMERA_ZOOM_FACTOR;
    static const float SHAKE_DAMPING;
    static const float SHAKE_MAX_RADIUS;
    static const float SHAKE_MIN_RADIUS;
    
private:
    struct ShakeData
    {
        bool mIsShaking = false;
        glm::vec3 mPreShakePosition;
        float mShakeRadius = 0.0f;
        float mShakeRandomAngle = 0.0f;
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
