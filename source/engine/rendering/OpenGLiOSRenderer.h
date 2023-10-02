///------------------------------------------------------------------------------------------------
///  OpenGLiOSRenderer.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 02/10/2023
///------------------------------------------------------------------------------------------------

#ifndef OpenGLiOSRenderer_h
#define OpenGLiOSRenderer_h

///------------------------------------------------------------------------------------------------

#include <engine/rendering/IRenderer.h>
#include <memory>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class OpenGLiOSRenderer final: public IRenderer
{
public:
    void BeginRenderPass() override;
    void RenderScene(scene::Scene& scene) override;
    void EndRenderPass() override;
    void SpecialEventHandling(SDL_Event& event) override;
    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* OpenGLiOSRenderer_h */
