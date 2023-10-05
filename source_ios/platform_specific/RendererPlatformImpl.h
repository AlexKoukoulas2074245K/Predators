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
#include <engine/CoreSystemsEngine.h>
#include <functional>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class RendererPlatformImpl final: public IRenderer
{
    friend struct CoreSystemsEngine::SystemsImpl;    
public:
    void VBeginRenderPass() override;
    void VRenderScene(scene::Scene& scene) override;
    void VEndRenderPass() override;
    
private:
    RendererPlatformImpl() = default;
    
private:
    std::vector<std::reference_wrapper<scene::Scene>> mCachedScenes;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* RendererPlatformImpl_h */
