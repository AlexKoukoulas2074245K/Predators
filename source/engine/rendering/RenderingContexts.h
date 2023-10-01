///------------------------------------------------------------------------------------------------
///  RenderingContexts.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 20/09/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef RenderingContexts_h
#define RenderingContexts_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/MathUtils.h>
#include <engine/rendering/IRenderer.h>
#include <memory>

///------------------------------------------------------------------------------------------------

struct SDL_Window;
using SDL_GLContext = void*;

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class IRenderingContext
{
public:
    virtual ~IRenderingContext() = default;
    virtual bool VInit() = 0;
    virtual IRenderer& VGetRenderer() const = 0;
    virtual SDL_Window& VGetContextWindow() const = 0;
    virtual glm::vec2 VGetContextRenderableDimensions() const = 0;
};

///------------------------------------------------------------------------------------------------

class BaseRenderingContext : public IRenderingContext
{
public:
    virtual ~BaseRenderingContext() = default;
    SDL_Window& VGetContextWindow() const override;
    IRenderer& VGetRenderer() const override;
    glm::vec2 VGetContextRenderableDimensions() const override;
    
protected:
    void SetContextWindow(SDL_Window* window);
    void SetContext(SDL_GLContext context);
    void SetRenderer(std::unique_ptr<IRenderer> renderer);
    
protected:
    SDL_Window* mWindow;
    SDL_GLContext mContext;
    std::unique_ptr<IRenderer> mRenderer;
};

///------------------------------------------------------------------------------------------------

class EmptyRenderingContext final: public BaseRenderingContext
{
    bool VInit() override;
};

///------------------------------------------------------------------------------------------------

class MacRenderingContext final : public BaseRenderingContext
{
public:
    bool VInit() override;
};

///------------------------------------------------------------------------------------------------


class WindowsRenderingContext final : public BaseRenderingContext
{
public:
    bool VInit() override;
};

///------------------------------------------------------------------------------------------------

class RenderingContextFactory final
{
public:
    static void CreateRenderingContext();
    
private:
    RenderingContextFactory() = default;
};

///------------------------------------------------------------------------------------------------

class RenderingContextHolder final
{
    friend class RenderingContextFactory;
    
public:
    static IRenderingContext& GetRenderingContext() { return *sRenderingContext; }
    
private:
    static void SetRenderingContext(std::unique_ptr<IRenderingContext> renderingContext) { sRenderingContext = std::move(renderingContext); }
    
    RenderingContextHolder() = default;
    
private:
    static std::unique_ptr<IRenderingContext> sRenderingContext;
    
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* RenderingContexts_h */
