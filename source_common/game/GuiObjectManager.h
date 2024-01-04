///------------------------------------------------------------------------------------------------
///  GuiObjectManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GuiObjectManager_h
#define GuiObjectManager_h

///------------------------------------------------------------------------------------------------

#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }

class AnimatedStatContainer;
class AnimatedButton;
class GuiObjectManager final
{
public:
    GuiObjectManager(std::shared_ptr<scene::Scene> scene, const bool forBattleScene = false);
    ~GuiObjectManager();
    
    void Update(const float dtMillis);
    void OnWindowResize();
    void ForceSetStoryHealthValue(const int storyHealthValue);
    
private:
    void SetCoinValueText();
    void OnSettingsButtonPressed();
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::unique_ptr<AnimatedStatContainer> mHealthStatContainer;
    std::shared_ptr<scene::Scene> mScene;
};

///------------------------------------------------------------------------------------------------

#endif /* GuiObjectManager_h */
