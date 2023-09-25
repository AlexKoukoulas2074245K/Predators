///------------------------------------------------------------------------------------------------
///  SceneObject.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SceneObject_h
#define SceneObject_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>
#include <variant>

///------------------------------------------------------------------------------------------------

namespace resources { using ResourceId = size_t; }

///------------------------------------------------------------------------------------------------

namespace scene
{

///------------------------------------------------------------------------------------------------

struct DefaultSceneObjectData
{
    glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mScale = glm::vec3(1.0f, 1.0f, 1.0f);
};

///------------------------------------------------------------------------------------------------

struct TextSceneObjectData
{
    std::string mText;
    strutils::StringId mFontName;
    glm::vec3 mTextPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mTextRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    float mTextScale;
};

///------------------------------------------------------------------------------------------------


struct SceneObject
{
    std::variant<DefaultSceneObjectData, TextSceneObjectData> mSceneObjectTypeData;
    resources::ResourceId mMeshResourceId;
    resources::ResourceId mTextureResourceId;
    resources::ResourceId mShaderResourceId;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* SceneObject_h */
