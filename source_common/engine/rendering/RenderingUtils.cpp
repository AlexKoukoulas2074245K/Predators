///------------------------------------------------------------------------------------------------
///  RenderingUtils.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 31/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/CoreSystemsEngine.h>
#include <engine/rendering/RenderingUtils.h>
#include <engine/rendering/OpenGL.h>
#include <engine/rendering/IRenderer.h>
#include <engine/scene/Scene.h>
#include <engine/utils/PlatformMacros.h>

///------------------------------------------------------------------------------------------------

constexpr int NEW_TEXTURE_SIZE = 2048;

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

void CollateSceneObjectsIntoOne(const std::string& dynamicTextureResourceName, const glm::vec3& positionOffset, std::vector<std::shared_ptr<scene::SceneObject>>& sceneObjects, scene::Scene& scene)
{
    auto& resService = CoreSystemsEngine::GetInstance().GetResourceLoadingService();
    resources::ResourceId dynamicTextureResourceId = resService.HasLoadedResource(dynamicTextureResourceName, true) ? resService.GetResourceIdFromPath(dynamicTextureResourceName, true) : 0;
    
    if (!dynamicTextureResourceId)
    {
        GLint oldFrameBuffer;
        GLint oldRenderBuffer;
        GL_CALL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFrameBuffer));
        GL_CALL(glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderBuffer));
        
        GLuint frameBuffer, textureId;
        GL_CALL(glGenFramebuffers(1, &frameBuffer));
        GL_CALL(glGenTextures(1, &textureId));

        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));

        GL_CALL(glBindTexture(GL_TEXTURE_2D, textureId));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, NEW_TEXTURE_SIZE/2, NEW_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
        GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0));

        GLuint depthbuffer;
        GL_CALL(glGenRenderbuffers(1, &depthbuffer));
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthbuffer));
        GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, NEW_TEXTURE_SIZE/2, NEW_TEXTURE_SIZE));
        GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthbuffer));

        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        
        for (auto& sceneObject: sceneObjects)
        {
            sceneObject->mPosition -= positionOffset;
        }
        
        CoreSystemsEngine::GetInstance().GetRenderer().VRenderSceneObjectsToTexture(sceneObjects, scene.GetCamera());
        
        dynamicTextureResourceId = CoreSystemsEngine::GetInstance().GetResourceLoadingService().AddDynamicallyCreatedTextureResourceId
        (
            dynamicTextureResourceName,
            textureId,
            NEW_TEXTURE_SIZE,
            NEW_TEXTURE_SIZE
        );
        
        GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, oldFrameBuffer));
        GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, oldRenderBuffer));
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    }
    else
    {
        sceneObjects.front()->mPosition -= positionOffset;
    }
    
    assert(sceneObjects.size() > 1);
    
    for (auto iter = sceneObjects.begin() + 1; iter != sceneObjects.end();)
    {
        iter = sceneObjects.erase(iter);
    }
    
    sceneObjects.front()->mTextureResourceId = dynamicTextureResourceId;
}

///------------------------------------------------------------------------------------------------

}
