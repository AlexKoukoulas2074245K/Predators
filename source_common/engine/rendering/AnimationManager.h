///------------------------------------------------------------------------------------------------
///  AnimationManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023
///------------------------------------------------------------------------------------------------

#ifndef AnimationManager_h
#define AnimationManager_h

///------------------------------------------------------------------------------------------------

#include <engine/rendering/Animations.h>
#include <engine/utils/StringUtils.h>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

class AnimationManager final
{
public:
    void StartAnimation(std::unique_ptr<IAnimation> animation, std::function<void()> onCompleteCallback, const strutils::StringId animationName = strutils::StringId());
    void Update(const float dtMillis);
    
    bool IsAnimationPlaying(const strutils::StringId& animationName) const;
private:
    struct AnimationEntry
    {
        std::unique_ptr<IAnimation> mAnimation;
        std::function<void()> mCompletionCallback;
        strutils::StringId mAnimationName;
    };
    
    std::vector<AnimationEntry> mAnimations;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* AnimationManager_h */
