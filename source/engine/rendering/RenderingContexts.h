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
    virtual bool Init() = 0;
    virtual SDL_Window* GetContextWindow() const = 0;
    virtual const glm::vec2& GetContextRenderableDimensions() const = 0;
};

///------------------------------------------------------------------------------------------------

class BaseRenderingContext : public IRenderingContext
{
public:
    virtual ~BaseRenderingContext() = default;
    SDL_Window* GetContextWindow() const override { return mWindow; }
    const glm::vec2& GetContextRenderableDimensions() const override { return mRenderableDimensions; }
    
protected:
    void SetContextWindow(SDL_Window* window) { mWindow = window; }
    void SetContext(SDL_GLContext context) { mContext = context; }
    void SetContextRenderableDimensions(const glm::vec2& renderableDimensions) { mRenderableDimensions = renderableDimensions; }
    
protected:
    SDL_Window* mWindow;
    SDL_GLContext mContext;
    glm::vec2 mRenderableDimensions;
};

///------------------------------------------------------------------------------------------------

class MacRenderingContext : public BaseRenderingContext
{
public:
    bool Init() override;
};

///------------------------------------------------------------------------------------------------

class RenderingContextFactory
{
public:
    static void CreateRenderingContext();
    
private:
    RenderingContextFactory() = default;
};

///------------------------------------------------------------------------------------------------

class RenderingContextHolder
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
