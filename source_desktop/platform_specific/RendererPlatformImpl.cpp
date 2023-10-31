///------------------------------------------------------------------------------------------------
///  RendererPlatformImpl.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/rendering/Fonts.h>
#include <engine/rendering/OpenGL.h>
#include <engine/resloading/MeshResource.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/resloading/ShaderResource.h>
#include <engine/resloading/TextureResource.h>
#include <engine/scene/Scene.h>
#include <engine/scene/SceneObject.h>
#include <engine/utils/Logging.h>
#include <engine/utils/StringUtils.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <platform_specific/RendererPlatformImpl.h>
#include <SDL.h>
#include <unordered_map>

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
static const strutils::StringId CUSTOM_ALPHA_UNIFORM_NAME = strutils::StringId("custom_alpha");

static const glm::ivec4 RENDER_TO_TEXTURE_VIEWPORT = {-768, -512, 2048, 2048};
static const glm::vec4 RENDER_TO_TEXTURE_CLEAR_COLOR = {1.0f, 1.0f, 1.0f, 0.0f};


///------------------------------------------------------------------------------------------------

static int sDrawCallCounter = 0;
static int sParticleCounter = 0;

struct SceneObjectDebugOverrideData
{
    bool mOverrideVisibility = false;
};

static std::unordered_map<strutils::StringId, std::unique_ptr<SceneObjectDebugOverrideData>, strutils::StringIdHasher> sSceneObjectOverrideData;

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
        
        for (size_t i = 0; i < currentShader->GetUniformSamplerNames().size(); ++i)
        {
            currentShader->SetInt(currentShader->GetUniformSamplerNames().at(i), static_cast<int>(i));
        }
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(mSceneObject.mMeshResourceId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        if (mSceneObject.mEffectTextureResourceId != 0 && mSceneObject.mEffectTextureResourceId != 2)
        {
            auto* currentEffectTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mEffectTextureResourceId));
            GL_CALL(glActiveTexture(GL_TEXTURE1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, currentEffectTexture->GetGLTextureId()));
        }
        
        glm::mat4 world(1.0f);
        world = glm::translate(world, mSceneObject.mPosition);
        world = glm::rotate(world, mSceneObject.mRotation.x, math::X_AXIS);
        world = glm::rotate(world, mSceneObject.mRotation.y, math::Y_AXIS);
        world = glm::rotate(world, mSceneObject.mRotation.z, math::Z_AXIS);
        world = glm::scale(world, mSceneObject.mScale);
        
        currentShader->SetFloat(CUSTOM_ALPHA_UNIFORM_NAME, 1.0f);
        currentShader->SetBool(IS_TEXTURE_SHEET_UNIFORM_NAME, false);
        currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
        currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, mCamera.GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, mCamera.GetProjMatrix());
        
        for (const auto& floatEntry: mSceneObject.mShaderFloatUniformValues) currentShader->SetFloat(floatEntry.first, floatEntry.second);
       
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
        if (sSceneObjectOverrideData.at(mSceneObject.mName)->mOverrideVisibility)
        {
            currentShader->SetFloat(strutils::StringId(CUSTOM_ALPHA_UNIFORM_NAME), 1.0f);
        }
#endif
        
        GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
        sDrawCallCounter++;
    }
    
    void operator()(scene::TextSceneObjectData sceneObjectTypeData)
    {
        auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(mSceneObject.mShaderResourceId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        for (size_t i = 0; i < currentShader->GetUniformSamplerNames().size(); ++i)
        {
            currentShader->SetInt(currentShader->GetUniformSamplerNames().at(i), static_cast<int>(i));
        }
        
        auto* currentMesh = &(resService.GetResource<resources::MeshResource>(mSceneObject.mMeshResourceId));
        GL_CALL(glBindVertexArray(currentMesh->GetVertexArrayObject()));
        
        auto fontOpt = CoreSystemsEngine::GetInstance().GetFontRepository().GetFont(sceneObjectTypeData.mFontName);
        assert(fontOpt);
        const auto& font = fontOpt->get();
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(font.mFontTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        if (mSceneObject.mEffectTextureResourceId != 0)
        {
            auto* currentEffectTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mEffectTextureResourceId));
            GL_CALL(glActiveTexture(GL_TEXTURE1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, currentEffectTexture->GetGLTextureId()));
        }
        
        float xCursor = mSceneObject.mPosition.x;
        
        for (size_t i = 0; i < sceneObjectTypeData.mText.size(); ++i)
        {
            const auto& glyph = font.FindGlyph(sceneObjectTypeData.mText[i]);
            
            float targetX = xCursor;
            float targetY = mSceneObject.mPosition.y - glyph.mYOffsetPixels * mSceneObject.mScale.y * 0.5f;
            
            glm::mat4 world(1.0f);
            world = glm::translate(world, glm::vec3(targetX, targetY, mSceneObject.mPosition.z));
            world = glm::scale(world, glm::vec3(glyph.mWidthPixels * mSceneObject.mScale.x, glyph.mHeightPixels * mSceneObject.mScale.y, 1.0f));
            
            currentShader->SetFloat(CUSTOM_ALPHA_UNIFORM_NAME, 1.0f);
            currentShader->SetBool(IS_TEXTURE_SHEET_UNIFORM_NAME, true);
            currentShader->SetFloat(MIN_U_UNIFORM_NAME, glyph.minU);
            currentShader->SetFloat(MIN_V_UNIFORM_NAME, glyph.minV);
            currentShader->SetFloat(MAX_U_UNIFORM_NAME, glyph.maxU);
            currentShader->SetFloat(MAX_V_UNIFORM_NAME, glyph.maxV);
            currentShader->SetMatrix4fv(WORLD_MATRIX_UNIFORM_NAME, world);
            currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, mCamera.GetViewMatrix());
            currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, mCamera.GetProjMatrix());
            
            for (const auto& floatEntry: mSceneObject.mShaderFloatUniformValues) currentShader->SetFloat(floatEntry.first, floatEntry.second);
           
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
            if (sSceneObjectOverrideData.at(mSceneObject.mName)->mOverrideVisibility)
            {
                currentShader->SetFloat(strutils::StringId(CUSTOM_ALPHA_UNIFORM_NAME), 1.0f);
            }
#endif
            GL_CALL(glDrawElements(GL_TRIANGLES, currentMesh->GetElementCount(), GL_UNSIGNED_SHORT, (void*)0));
            sDrawCallCounter++;
            
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
    
    void operator()(scene::ParticleEmitterObjectData particleEmitterData)
    {
        auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
        
        auto* currentShader = &(resService.GetResource<resources::ShaderResource>(mSceneObject.mShaderResourceId));
        GL_CALL(glUseProgram(currentShader->GetProgramId()));
        
        for (size_t i = 0; i < currentShader->GetUniformSamplerNames().size(); ++i)
        {
            currentShader->SetInt(currentShader->GetUniformSamplerNames().at(i), static_cast<int>(i));
        }
        
        auto* currentTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mTextureResourceId));
        GL_CALL(glActiveTexture(GL_TEXTURE0));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, currentTexture->GetGLTextureId()));
        
        if (mSceneObject.mEffectTextureResourceId != 0)
        {
            auto* currentEffectTexture = &(resService.GetResource<resources::TextureResource>(mSceneObject.mEffectTextureResourceId));
            GL_CALL(glActiveTexture(GL_TEXTURE1));
            GL_CALL(glBindTexture(GL_TEXTURE_2D, currentEffectTexture->GetGLTextureId()));
        }
        
        currentShader->SetFloat(CUSTOM_ALPHA_UNIFORM_NAME, 1.0f);
        currentShader->SetMatrix4fv(VIEW_MATRIX_UNIFORM_NAME, mCamera.GetViewMatrix());
        currentShader->SetMatrix4fv(PROJ_MATRIX_UNIFORM_NAME, mCamera.GetProjMatrix());
        
        for (const auto& floatEntry: mSceneObject.mShaderFloatUniformValues) currentShader->SetFloat(floatEntry.first, floatEntry.second);
       
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
        if (sSceneObjectOverrideData.at(mSceneObject.mName)->mOverrideVisibility)
        {
            currentShader->SetFloat(strutils::StringId(CUSTOM_ALPHA_UNIFORM_NAME), 1.0f);
        }
#endif
        GL_CALL(glBindVertexArray(particleEmitterData.mParticleVertexArrayObject));
        
        GL_CALL(glEnableVertexAttribArray(0));
        GL_CALL(glEnableVertexAttribArray(1));
        GL_CALL(glEnableVertexAttribArray(2));
        GL_CALL(glEnableVertexAttribArray(3));
        GL_CALL(glEnableVertexAttribArray(4));
        
        // update the position buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticlePositionsBuffer));
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, particleEmitterData.mParticlePositions.size() * sizeof(glm::vec3), particleEmitterData.mParticlePositions.data()));
        
        // update the lifetime buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleLifetimeSecsBuffer));
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, particleEmitterData.mParticlePositions.size() * sizeof(float), particleEmitterData.mParticleLifetimeSecs.data()));
        
        // update the scale buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleSizesBuffer));
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, particleEmitterData.mParticleSizes.size() * sizeof(float), particleEmitterData.mParticleSizes.data()));
        
        // vertex buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER , particleEmitterData.mParticleVertexBuffer));
        GL_CALL(glVertexAttribPointer(0, 3 , GL_FLOAT, GL_FALSE , 0 , nullptr));
        
        // uv buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER , particleEmitterData.mParticleUVBuffer));
        GL_CALL(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE , 0 , nullptr));
        
        // position buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticlePositionsBuffer));
        GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE , 0 , nullptr));
        GL_CALL(glVertexAttribDivisor(2, 1));
        
        // lifetime buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleLifetimeSecsBuffer));
        GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE , 0 , nullptr));
        GL_CALL(glVertexAttribDivisor(3, 1));
        
        // size buffer
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, particleEmitterData.mParticleSizesBuffer));
        GL_CALL(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE , 0 , nullptr));
        GL_CALL(glVertexAttribDivisor(4, 1));
        
        // draw triangles
        GL_CALL(glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<int>(particleEmitterData.mParticlePositions.size())));
        
        GL_CALL(glDisableVertexAttribArray(0));
        GL_CALL(glDisableVertexAttribArray(1));
        GL_CALL(glDisableVertexAttribArray(2));
        GL_CALL(glDisableVertexAttribArray(3));
        GL_CALL(glDisableVertexAttribArray(4));
        
        GL_CALL(glBindVertexArray(0));
        
        sParticleCounter += particleEmitterData.mParticleCount;
        sDrawCallCounter++;
    }

private:
    const scene::SceneObject& mSceneObject;
    const Camera& mCamera;
};

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VBeginRenderPass()
{
    sDrawCallCounter = 0;
    sParticleCounter = 0;
    
    // Set View Port
    int w, h;
    SDL_GL_GetDrawableSize(&CoreSystemsEngine::GetInstance().GetContextWindow(), &w, &h);
    GL_CALL(glViewport(0, 0, w, h));
    
    // Set background color
    GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
    
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    
    // Clear buffers
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    GL_CALL(glDisable(GL_CULL_FACE));
    
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
    // Imgui start-of-frame calls
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
#endif
}

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VRenderScene(scene::Scene& scene)
{
    mCachedScenes.push_back(scene);

#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
    for (auto iter = sSceneObjectOverrideData.begin(); iter != sSceneObjectOverrideData.end();)
    {
        if (scene.FindSceneObject(iter->first) == nullptr)
        {
            iter = sSceneObjectOverrideData.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
#else
    (void)sSceneObjectOverrideData;
#endif
    
    for (const auto& sceneObject: scene.GetSceneObjects())
    {
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
        if (sSceneObjectOverrideData.count(sceneObject->mName) == 0)
        {
            sSceneObjectOverrideData[sceneObject->mName] = std::make_unique<SceneObjectDebugOverrideData>();
        }
        
        if (!sSceneObjectOverrideData.at(sceneObject->mName)->mOverrideVisibility && sceneObject->mInvisible) continue;
#else
        if (sceneObject->mInvisible) continue;
#endif

        std::visit(SceneObjectTypeRendererVisitor(*sceneObject, scene.GetCamera()), sceneObject->mSceneObjectTypeData);
    }
}

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VRenderSceneObjectsToTexture(const std::vector<std::shared_ptr<scene::SceneObject>>& sceneObjects, const rendering::Camera& camera)
{
    // Set custom viewport
    GL_CALL(glViewport(RENDER_TO_TEXTURE_VIEWPORT.x, RENDER_TO_TEXTURE_VIEWPORT.y, RENDER_TO_TEXTURE_VIEWPORT.z, RENDER_TO_TEXTURE_VIEWPORT.w));
    
    // Set background color
    GL_CALL(glClearColor(RENDER_TO_TEXTURE_CLEAR_COLOR.r, RENDER_TO_TEXTURE_CLEAR_COLOR.g, RENDER_TO_TEXTURE_CLEAR_COLOR.b, RENDER_TO_TEXTURE_CLEAR_COLOR.a));
    
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    
    // Clear buffers
    GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    
    GL_CALL(glDisable(GL_CULL_FACE));
    
    for (auto sceneObject: sceneObjects)
    {
        if (sSceneObjectOverrideData.count(sceneObject->mName) == 0)
        {
            sSceneObjectOverrideData[sceneObject->mName] = std::make_unique<SceneObjectDebugOverrideData>();
        }
        
        std::visit(SceneObjectTypeRendererVisitor(*sceneObject, camera), sceneObject->mSceneObjectTypeData);
    }
}

///------------------------------------------------------------------------------------------------

void RendererPlatformImpl::VEndRenderPass()
{
#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
    // Create all custom GUIs
    CreateIMGuiWidgets();
    mCachedScenes.clear();
    
    // Imgui end-of-frame calls
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
    
    // Swap window buffers
    SDL_GL_SwapWindow(&CoreSystemsEngine::GetInstance().GetContextWindow());
}

///------------------------------------------------------------------------------------------------

#if (!defined(NDEBUG)) || defined(IMGUI_IN_RELEASE)
static std::unordered_map<strutils::StringId, glm::vec2, strutils::StringIdHasher> sUniformMinMaxValues;

class SceneObjectDataIMGuiVisitor
{
public:
    void operator()(scene::DefaultSceneObjectData)
    {
        ImGui::Text("SO Type: Default");
    }
    void operator()(scene::TextSceneObjectData textData)
    {
        ImGui::Text("SO Type: Text");
        ImGui::Text("Text: %s", textData.mText.c_str());
    }
    void operator()(scene::ParticleEmitterObjectData)
    {
        ImGui::Text("SO Type: Particle Emitter");
    }
};

static SceneObjectDataIMGuiVisitor imguiVisitor;

void RendererPlatformImpl::CreateIMGuiWidgets()
{
    ImGui::ShowDemoWindow();
    
    auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    
    ImGui::Begin("Rendering", nullptr, GLOBAL_WINDOW_LOCKING);
    ImGui::Text("Draw Calls %d", sDrawCallCounter);
    ImGui::Text("Particle Count %d", sParticleCounter);
    ImGui::Text("Anims Live %d", CoreSystemsEngine::GetInstance().GetAnimationManager().GetAnimationsPlayingCount());
    
    ImGui::End();
    
    // Create scene data viewer
    for (auto& sceneRef: mCachedScenes)
    {
        auto viewerName = strutils::StringId("Scene Data Viewer (" + sceneRef.get().GetName().GetString() + ")");
        
        ImGui::Begin(viewerName.GetString().c_str(), nullptr, GLOBAL_WINDOW_LOCKING);
        
        // Camera properties
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None))
        {
            static glm::vec3 cameraPos(0.0f);
            cameraPos = sceneRef.get().GetCamera().GetPosition();
            if(ImGui::SliderFloat("camX", &cameraPos.x , -0.5f, 0.5f) ||
               ImGui::SliderFloat("camY", &cameraPos.y, -0.5f, 0.5f) ||
               ImGui::SliderFloat("camZ", &cameraPos.z, -0.5f, 0.5f))
            {
                sceneRef.get().GetCamera().SetPosition(cameraPos);
            }
            
            static float cameraZoomFactor(0.0f);
            cameraZoomFactor = sceneRef.get().GetCamera().GetZoomFactor();
            if (ImGui::SliderFloat("zoom", &cameraZoomFactor, 10.0f, 200.0f))
            {
                sceneRef.get().GetCamera().SetZoomFactor(cameraZoomFactor);
            }
        }
        
        // SO Properties
        size_t i = 0;
        for (auto sceneObject: sceneRef.get().GetSceneObjects())
        {
            auto sceneObjectName = sceneObject->mName.isEmpty() ? strutils::StringId("SO: " + std::to_string(i)) : strutils::StringId("SO: " + sceneObject->mName.GetString());
            
            if (ImGui::CollapsingHeader(sceneObjectName.GetString().c_str(), ImGuiTreeNodeFlags_None))
            {
                ImGui::PushID(sceneObjectName.GetString().c_str());
                std::visit(imguiVisitor, sceneObject->mSceneObjectTypeData);
                ImGui::Text("Mesh: %s", resService.GetResourcePath(sceneObject->mMeshResourceId).c_str());
                ImGui::Text("Shader: %s", resService.GetResourcePath(sceneObject->mShaderResourceId).c_str());
                ImGui::Text("Texture: %s", resService.GetResourcePath(sceneObject->mTextureResourceId).c_str());
                ImGui::Checkbox("Override Visibility", &(sSceneObjectOverrideData.at(sceneObject->mName)->mOverrideVisibility));
                ImGui::SliderFloat("x", &sceneObject->mPosition.x, -0.5f, 0.5f);
                ImGui::SliderFloat("y", &sceneObject->mPosition.y, -0.5f, 0.5f);
                ImGui::SliderFloat("z", &sceneObject->mPosition.z, -0.5f, 0.5f);
                ImGui::SliderFloat("rx", &sceneObject->mRotation.x, -3.14f, 3.14f);
                ImGui::SliderFloat("ry", &sceneObject->mRotation.y, -3.14f, 3.14f);
                ImGui::SliderFloat("rz", &sceneObject->mRotation.z, -3.14f, 3.14f);
                ImGui::SliderFloat("sx", &sceneObject->mScale.x, 0.00001f, 1.0f);
                ImGui::SliderFloat("sy", &sceneObject->mScale.y, 0.00001f, 1.0f);
                ImGui::SliderFloat("sz", &sceneObject->mScale.z, 0.00001f, 1.0f);
                ImGui::SeparatorText("Uniforms (floats)");
                for (auto& uniformFloatEntry: sceneObject->mShaderFloatUniformValues)
                {
                    if (sUniformMinMaxValues.count(uniformFloatEntry.first) == 0)
                    {
                        sUniformMinMaxValues[uniformFloatEntry.first] = glm::vec2(uniformFloatEntry.second/100.0f, uniformFloatEntry.second*10.0f);
                    }
                    
                    auto uniformMinMaxValues = sUniformMinMaxValues.at(uniformFloatEntry.first);
                    ImGui::SliderFloat(uniformFloatEntry.first.GetString().c_str(), &uniformFloatEntry.second, uniformMinMaxValues.x, uniformMinMaxValues.y);
                }
                ImGui::PopID();
            }
            i++;
        }
        ImGui::End();
    }
}
#else
void RendererPlatformImpl::CreateIMGuiWidgets()
{
}
#endif

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
