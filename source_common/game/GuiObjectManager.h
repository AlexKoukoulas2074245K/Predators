///------------------------------------------------------------------------------------------------
///  GuiObjectManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GuiObjectManager_h
#define GuiObjectManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
#include <engine/utils/MathUtils.h>
#include <memory>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace scene { class Scene; }

class AnimatedStatContainer;
class AnimatedButton;
class GuiObjectManager final: public events::IListener
{
public:
    GuiObjectManager(std::shared_ptr<scene::Scene> scene);
    ~GuiObjectManager();
    
    void Update(const float dtMillis, const bool allowButtonInput = true);
    void OnWindowResize();
    void ForceSetStoryHealthValue(const int storyHealthValue);
    
private:
    enum class StatParticleType
    {
        COINS,
        HEALTH
    };
    
    enum class StatGainParticleType
    {
        MAX_HEALTH,
        DAMAGE,
        WEIGHT
    };
    
    void AnimateStatParticlesToGui(const glm::vec3& originPosition, const StatParticleType statParticleType, const long long coinAmount);
    void AnimateStatGainParticles(const glm::vec3& originPosition, const StatGainParticleType statGainParticleType);
    void SetCoinValueText();
    void OnSettingsButtonPressed();
    void OnCoinReward(const events::CoinRewardEvent&);
    void OnHealthRefillReward(const events::HealthRefillRewardEvent&);
    void OnMaxHealthGainReward(const events::MaxHealthGainRewardEvent&);
    void OnExtraDamageReward(const events::ExtraDamageRewardEvent&);
    void OnExtraWeightReward(const events::ExtraWeightRewardEvent&);
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mAnimatedButtons;
    std::unique_ptr<AnimatedStatContainer> mHealthStatContainer;
    std::shared_ptr<scene::Scene> mScene;
    float mRewardAnimationSecsLeft;
};

///------------------------------------------------------------------------------------------------

#endif /* GuiObjectManager_h */
