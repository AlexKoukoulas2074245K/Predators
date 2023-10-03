///------------------------------------------------------------------------------------------------
///  RendererPlatformImpl.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 03/10/2023
///------------------------------------------------------------------------------------------------

#ifndef RendererPlatformImpl_h
#define RendererPlatformImpl_h

///------------------------------------------------------------------------------------------------

#include <engine/rendering/IRenderer.h>
#include <functional>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class RendererPlatformImpl final: public IRenderer
{
public:
    void BeginRenderPass() override;
    void RenderScene(scene::Scene& scene) override;
    void EndRenderPass() override;
    
private:
    void CreateIMGuiWidgets();
    
private:
    std::vector<std::reference_wrapper<scene::Scene>> mCachedScenes;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* RendererPlatformImpl_h */
