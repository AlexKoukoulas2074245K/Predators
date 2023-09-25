///------------------------------------------------------------------------------------------------
///  OpenGLRenderer.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/rendering/Fonts.h>
#include <engine/rendering/OpenGL.h>
#include <engine/rendering/OpenGLRenderer.h>
#include <engine/rendering/RenderingContexts.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/ShaderResource.h>
#include <engine/resloading/TextureResource.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/StringUtils.h>
#include <SDL.h>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

static const strutils::StringId WORLD_MATRIX_UNIFORM_NAME = strutils::StringId("world");
static const strutils::StringId VIEW_MATRIX_UNIFORM_NAME  = strutils::StringId("view");
static const strutils::StringId PROJ_MATRIX_UNIFORM_NAME  = strutils::StringId("proj");
inline const strutils::StringId MIN_U_UNIFORM_NAME = strutils::StringId("min_u");
inline const strutils::StringId MIN_V_UNIFORM_NAME = strutils::StringId("min_v");
inline const strutils::StringId MAX_U_UNIFORM_NAME = strutils::StringId("max_u");
inline const strutils::StringId MAX_V_UNIFORM_NAME = strutils::StringId("max_v");
static const strutils::StringId ACTIVE_LIGHT_COUNT_UNIFORM_NAME = strutils::StringId("active_light_count");
static const strutils::StringId AMBIENT_LIGHT_COLOR_UNIFORM_NAME = strutils::StringId("ambient_light_color");
static const strutils::StringId POINT_LIGHT_COLORS_UNIFORM_NAME = strutils::StringId("point_light_colors");
static const strutils::StringId POINT_LIGHT_POSITIONS_UNIFORM_NAME = strutils::StringId("point_light_positions");
static const strutils::StringId POINT_LIGHT_POWERS_UNIFORM_NAME = strutils::StringId("point_light_powers");
static const strutils::StringId IS_TEXTURE_SHEET_UNIFORM_NAME = strutils::StringId("texture_sheet");

///------------------------------------------------------------------------------------------------

class SceneObjectTypeRendererVisitor
{
public:
    SceneObjectTypeRendererVisitor(scene::SceneObject& sceneObject, Camera& camera)
        : mSceneObject(sceneObject)
        , mCamera(camera)
    {
    }
    
    void operator()(scene::DefaultSceneObjectData sceneObjectTypeData)
    {
        auto& resService = resources::ResourceLoadingService::GetInstance();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(mSceneObject.mShaderResourceId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(mSceneObject.mMeshResourceId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        glm::mat4 world(1.0f);
        world = glm::translate(world, sceneObjectTypeData.mPosition);
        world = glm::rotate(world, sceneObjectTypeData.mRotation.x, math::X_AXIS);
        world = glm::rotate(world, sceneObjectTypeData.mRotation.y, math::Y_AXIS);
        world = glm::rotate(world, sceneObjectTypeData.mRotation.z, math::Z_AXIS);
        world = glm::scale(world, sceneObjectTypeData.mScale);
        
        currentShader->SetBool(IS_TEXTURE_SHEET_UNIFORM_NAME, false);
        currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
        currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, mCamera.GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, mCamera.GetProjMatrix());
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
    }
    
    void operator()(scene::TextSceneObjectData sceneObjectTypeData)
    {
        auto& resService = resources::ResourceLoadingService::GetInstance();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(mSceneObject.mShaderResourceId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(mSceneObject.mMeshResourceId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto fontOpt = rendering::FontRepository::GetInstance().GetFont(sceneObjectTypeData.mFontName);
        assert(fontOpt);
        const auto& font = fontOpt->get();
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(font.mFontTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        float xCursor = sceneObjectTypeData.mTextPosition.x;
        
        for (size_t i = 0; i < sceneObjectTypeData.mText.size(); ++i)
        {
            const auto& glyph = font.FindGlyph(sceneObjectTypeData.mText[i]);
            
            glm::vec3 textScale = glm::vec3(0.00058f, 0.00058f, 0.00058f);
            
            float targetX = xCursor;
            float targetY = sceneObjectTypeData.mTextPosition.y - glyph.mYOffsetPixels * textScale.y * 0.5f;
            
            glm::mat4 world(1.0f);
            world = glm::translate(world, glm::vec3(targetX, targetY, 0.1f));
            world = glm::scale(world, glm::vec3(glyph.mWidthPixels * textScale.x, glyph.mHeightPixels * textScale.y, 1.0f));
            
            currentShader->SetBool(IS_TEXTURE_SHEET_UNIFORM_NAME, true);
            currentShader->SetFloat(MIN_U_UNIFORM_NAME, glyph.minU);
            currentShader->SetFloat(MIN_V_UNIFORM_NAME, glyph.minV);
            currentShader->SetFloat(MAX_U_UNIFORM_NAME, glyph.maxU);
            currentShader->SetFloat(MAX_V_UNIFORM_NAME, glyph.maxV);
            currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
            currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, mCamera.GetViewMatrix());
            currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, mCamera.GetProjMatrix());
            
            GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
            
            if (i != sceneObjectTypeData.mText.size() - 1)
            {
                // Since each glyph is rendered with its center as the origin, we advance
                // half this glyph's width + half the next glyph's width ahead
                const auto& nextGlyph = font.FindGlyph(sceneObjectTypeData.mText[i + 1]);
                xCursor += (glyph.mWidthPixels * textScale.x) * 0.5f + (nextGlyph.mWidthPixels * textScale.x) * 0.5f;
                xCursor += glyph.mAdvancePixels * textScale.x;
            }
        }
    }
    
private:
    scene::SceneObject& mSceneObject;
    Camera mCamera;
};

///------------------------------------------------------------------------------------------------

void OpenGLRenderer::BeginRenderPass()
{
    auto windowDimensions = rendering::RenderingContextHolder::GetRenderingContext().GetContextRenderableDimensions();
    
    // Set View Port
    GL_CALL(glViewport(0, 0, windowDimensions.x, windowDimensions.y));
    
    // Set background color
    GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    
    // Clear buffers
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    GL_CALL(glDisable(GL_CULL_FACE));
}

///------------------------------------------------------------------------------------------------

void OpenGLRenderer::RenderScene(scene::Scene& scene)
{
    for (auto& sceneObject: scene.mSceneObjects)
    {
        std::visit(SceneObjectTypeRendererVisitor(sceneObject, scene.mCamera), sceneObject.mSceneObjectTypeData);
    }
}

///------------------------------------------------------------------------------------------------

void OpenGLRenderer::EndRenderPass()
{
    // Swap window buffers
    SDL_GL_SwapWindow(&rendering::RenderingContextHolder::GetRenderingContext().GetContextWindow());
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
