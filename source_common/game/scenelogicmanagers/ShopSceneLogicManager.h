///------------------------------------------------------------------------------------------------
///  ShopSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 14/01/2023
///------------------------------------------------------------------------------------------------

#ifndef ShopSceneLogicManager_h
#define ShopSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>
#include <functional>
#include <variant>
#include <vector>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class GuiObjectManager;
class ShopSceneLogicManager final: public ISceneLogicManager, public events::IListener
{    
public:
    ShopSceneLogicManager();
    ~ShopSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    std::shared_ptr<GuiObjectManager> VGetGuiObjectManager() override;
    
private:
    void RegisterForEvents();
    void OnWindowResize(const events::WindowResizeEvent& event);
    void CreateDynamicSceneObjects();
    void CreateProducts();
    void HighlightProduct(const size_t productShelfIndex, const size_t productShelfItemIndex);
    void DehighlightProduct(const size_t productShelfIndex, const size_t productShelfItemIndex);
    
private:
    struct Product
    {
        Product(const std::variant<strutils::StringId, int>& productId, const int price, const bool singleUse)
            : mProductId(productId)
            , mPrice(price)
            , mSingleUse(singleUse)
            , mHighlighted(false)
        {
        }
        
        const std::variant<strutils::StringId, int> mProductId;
        const int mPrice;
        const bool mSingleUse;
        std::vector<std::shared_ptr<scene::SceneObject>> mSceneObjects;
        bool mHighlighted;
    };
    
    enum class SceneState
    {
        CREATING_DYNAMIC_OBJECTS,
        BROWSING_SHOP,
        LEAVING_SHOP
    };
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::vector<std::vector<std::unique_ptr<Product>>> mProducts;
    std::shared_ptr<GuiObjectManager> mGuiManager;
    std::shared_ptr<scene::Scene> mScene;
    SceneState mSceneState;
};

///------------------------------------------------------------------------------------------------

#endif /* ShopSceneLogicManager_h */
