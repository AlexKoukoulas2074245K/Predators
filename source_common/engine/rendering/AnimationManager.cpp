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

void AnimationManager::StartAnimation(std::unique_ptr<IAnimation> animation, std::function<void()> onCompleteCallback, const strutils::StringId animationName /* = strutils::StringId() */)
{
    if (mAnimationContainerLocked)
    {
        mAnimationsToAdd.emplace_back(AnimationEntry{ std::move(animation), onCompleteCallback, animationName });
    }
    else
    {
        mAnimations.emplace_back(AnimationEntry{ std::move(animation), onCompleteCallback, animationName });
    }
}

///------------------------------------------------------------------------------------------------

void AnimationManager::StopAnimation(const strutils::StringId& animationName)
{
    if (mAnimationContainerLocked)
    {
        mAnimationNamesToRemove.emplace_back(animationName);
    }
    else
    {
        auto findIter = std::find_if(mAnimations.cbegin(), mAnimations.cend(), [&](const AnimationEntry& entry) { return entry.mAnimationName == animationName; });
        if (findIter != mAnimations.cend())
        {
            mAnimations.erase(findIter);
        }
    }
}

///------------------------------------------------------------------------------------------------

void AnimationManager::Update(const float dtMillis)
{
    mAnimationContainerLocked = true;
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
    mAnimationContainerLocked = false;
    
    for (const auto& animationName: mAnimationNamesToRemove)
    {
        auto findIter = std::find_if(mAnimations.cbegin(), mAnimations.cend(), [&](const AnimationEntry& entry) { return entry.mAnimationName == animationName; });
        if (findIter != mAnimations.cend())
        {
            mAnimations.erase(findIter);
        }
    }
    
    for (auto& animationEntry: mAnimationsToAdd)
    {
        mAnimations.emplace_back(std::move(animationEntry));
    }
    
    mAnimationsToAdd.clear();
    mAnimationNamesToRemove.clear();
}

///------------------------------------------------------------------------------------------------

bool AnimationManager::IsAnimationPlaying(const strutils::StringId& animationName) const
{
    auto findIter = std::find_if(mAnimations.cbegin(), mAnimations.cend(), [&](const AnimationEntry& entry) { return entry.mAnimationName == animationName; });
    return findIter != mAnimations.cend();
}

///------------------------------------------------------------------------------------------------

int AnimationManager::GetAnimationsPlayingCount() const
{
    return static_cast<int>(mAnimations.size());
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
