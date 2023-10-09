///------------------------------------------------------------------------------------------------
///  AnimationManager.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 09/10/2023
///------------------------------------------------------------------------------------------------

#include <engine/rendering/AnimationManager.h>

///------------------------------------------------------------------------------------------------

namespace rendering
{

///------------------------------------------------------------------------------------------------

void AnimationManager::StartAnimation(std::unique_ptr<IAnimation> animation, std::function<void()> onCompleteCallback)
{
    mAnimations.emplace_back(AnimationEntry{ std::move(animation), onCompleteCallback });
}

///------------------------------------------------------------------------------------------------

void AnimationManager::Update(const float dtMillis)
{
    for(auto iter = mAnimations.begin(); iter != mAnimations.end();)
    {
       if (iter->mAnimation->VUpdate(dtMillis) == AnimationUpdateResult::FINISHED)
       {
           iter->mCompletionCallback();
           iter = mAnimations.erase(iter);
       }
       else
       {
          ++iter;
       }
    }
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
