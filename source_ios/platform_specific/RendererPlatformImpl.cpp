///------------------------------------------------------------------------------------------------
///  RendererPlatformImpl.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/Fonts.h>
#include <engine/rendering/OpenGL.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/ShaderResource.h>
#include <engine/resloading/TextureResource.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/StringUtils.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <platform_specific/RendererPlatformImpl.h>
#include <SDL.h>

//#define IMGUI_IN_RELEASE

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

static const strutils::StringId WORLD_MATRIX_UNIFORM_NAME = strutils::StringId("world");
static const strutils::StringId VIEW_MATRIX_UNIFORM_NAME  = strutils::StringId("view");
static const strutils::StringId PROJ_MATRIX_UNIFORM_NAME  = strutils::StringId("proj");
static const strutils::StringId MIN_U_UNIFORM_NAME = strutils::StringId("min_u");
static const strutils::StringId MIN_V_UNIFORM_NAME = strutils::StringId("min_v");
static const strutils::StringId MAX_U_UNIFORM_NAME = strutils::StringId("max_u");
static const strutils::StringId MAX_V_UNIFORM_NAME = strutils::StringId("max_v");
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
    SceneObjectTypeRendererVisitor(const scene::SceneObject& sceneObject, const Camera& camera)
    : mSceneObject(sceneObject)
    , mCamera(camera)
    {
    }
    
    void operator()(scene::DefaultSceneObjectData)
    {
        auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(mSceneObject.mShaderResourceId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(mSceneObject.mMeshResourceId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        glm::mat4 world(1.0f);
        world = glm::translate(world, mSceneObject.mPosition);
        world = glm::rotate(world, mSceneObject.mRotation.x, math::X_AXIS);
        world = glm::rotate(world, mSceneObject.mRotation.y, math::Y_AXIS);
        world = glm::rotate(world, mSceneObject.mRotation.z, math::Z_AXIS);
        world = glm::scale(world, mSceneObject.mScale);
        
        currentShader->SetBool(IS_TEXTURE_SHEET_UNIFORM_NAME, false);
        currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
        currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, mCamera.GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, mCamera.GetProjMatrix());
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
    }
    
    void operator()(scene::TextSceneObjectData sceneObjectTypeData)
    {
        auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(mSceneObject.mShaderResourceId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(mSceneObject.mMeshResourceId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto fontOpt = CoreSystemsEngine::GetInstance().GetFontRepository().GetFont(sceneObjectTypeData.mFontName);
        assert(fontOpt);
        const auto& font = fontOpt->get();
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(font.mFontTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        float xCursor = mSceneObject.mPosition.x;
        
        for (size_t i = 0; i < sceneObjectTypeData.mText.size(); ++i)
        {
            const auto& glyph = font.FindGlyph(sceneObjectTypeData.mText[i]);
            
            float targetX = xCursor;
            float targetY = mSceneObject.mPosition.y - glyph.mYOffsetPixels * mSceneObject.mScale.y * 0.5f;
            
            glm::mat4 world(1.0f);
            world = glm::translate(world, glm::vec3(targetX, targetY, 0.1f));
            world = glm::scale(world, glm::vec3(glyph.mWidthPixels * mSceneObject.mScale.x, glyph.mHeightPixels * mSceneObject.mScale.y, 1.0f));
            
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
                xCursor += (glyph.mWidthPixels * mSceneObject.mScale.x) * 0.5f + (nextGlyph.mWidthPixels * mSceneObject.mScale.x) * 0.5f;
                xCursor += glyph.mAdvancePixels * mSceneObject.mScale.x;
            }
        }
    }
    
private:
    const scene::SceneObject& mSceneObject;
    const Camera& mCamera;
};

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VBeginRenderPass()
{
    auto windowDimensions = CoreSystemsEngine::GetInstance().GetContextRenderableDimensions();
    
    // Set View Port
    GL_CALL(glViewport(0, 0, static_cast<int>(windowDimensions.x), static_cast<int>(windowDimensions.y)));
    
    // Set background color
    GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    
    // Clear buffers
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    GL_CALL(glDisable(GL_CULL_FACE));
}

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VRenderScene(scene::Scene& scene)
{
    mCachedScenes.push_back(scene);
    for (const auto& sceneObject: scene.GetSceneObjects())
    {
        std::visit(SceneObjectTypeRendererVisitor(*sceneObject, scene.GetCamera()), sceneObject->mSceneObjectTypeData);
    }
}

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VEndRenderPass()
{
    // Swap window buffers
    SDL_GL_SwapWindow(&CoreSystemsEngine::GetInstance().GetContextWindow());
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------