///------------------------------------------------------------------------------------------------
///  OpenGLDesktopRenderer.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 25/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef OpenGLDesktopRenderer_h
#define OpenGLDesktopRenderer_h

///------------------------------------------------------------------------------------------------

#include <engine/rendering/IRenderer.h>
#include <functional>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class OpenGLDesktopRenderer final: public IRenderer
{
public:
    void BeginRenderPass() override;
    void RenderScene(scene::Scene& scene) override;
    void EndRenderPass() override;
    void SpecialEventHandling(SDL_Event& event) override;
    
private:
    void CreateIMGuiWidgets();
    
private:
    std::vector<std::reference_wrapper<scene::Scene>> mCachedScenes;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* OpenGLDesktopRenderer_h */
