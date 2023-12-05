///------------------------------------------------------------------------------------------------
///  SwipeableContainer.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 05/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef SwipeableContainer_h
#define SwipeableContainer_h

///------------------------------------------------------------------------------------------------

#include <engine/scene/SceneObjectUtils.h>
#include <engine/input/IInputStateManager.h>
#include <engine/rendering/AnimationManager.h>
#include <engine/scene/Scene.h>
#include <engine/utils/Logging.h>
#include <engine/utils/MathUtils.h>
#include <engine/utils/StringUtils.h>

#include <vector>

///------------------------------------------------------------------------------------------------

inline const strutils::StringId RUBBER_BANDING_ANIMATION_NAME = strutils::StringId("RUBBER_BANDING_ANIMATION");
inline const int MIN_ITEMS_TO_ANIMATE = 4;
inline const float CARD_VELOCITY_DAMPING = 0.85f;
inline const float OVERSWIPE_DAMPING = 100.0f;
inline const float SWIPE_DELTA_DIRECTION_CHANGE_NOISE_THRESHOLD = 0.00001f;
inline const float RUBBER_BANDING_ANIMATION_DURATION = 0.3f;

///------------------------------------------------------------------------------------------------

enum class SwipeDirection
{
    HORIZONTAL, VERTICAL
};

///------------------------------------------------------------------------------------------------

enum class InteractionType
{
    NONE,
    INTERACTED_WITH_ELEMENTS,
    INTERACTED_WITH_CONTAINER_AREA
};

///------------------------------------------------------------------------------------------------

struct UpdateResult
{
    InteractionType mInteractionType;
    int mInteractedElementId;
};

///------------------------------------------------------------------------------------------------

template <typename ContainerEntryT>
class SwipeableContainer final
{
public:
    SwipeableContainer
    (
        const SwipeDirection validSwipeDirection,
        const math::Rectangle& containerBounds,
        const glm::vec2& containerCutoffValues,
        const strutils::StringId& containerName,
        const float containerItemsZ,
        scene::Scene& scene
     )
        : mValidSwipeDirection(validSwipeDirection)
        , mContainerBounds(containerBounds)
        , mContainerCutoffValues(containerCutoffValues)
        , mContainerName(containerName)
        , mContainerItemsZ(containerItemsZ)
        , mScene(scene)
    {
    }
    
    void AddItem(ContainerEntryT&& item, bool atTheBack)
    {
        if (atTheBack)
        {
            item.mSceneObject->mName = strutils::StringId(mContainerName.GetString() + "_" + std::to_string(mItems.size()));
            mItems.emplace_back(item);
        }
        else
        {
            mItems.insert(mItems.begin(), item);
            for (int i = 0; i < static_cast<int>(mItems.size()); ++i)
            {
                auto item = mItems[i];
                item.mSceneObject->mName = strutils::StringId(mContainerName.GetString() + "_" + std::to_string(i));
            }
        }
        
        ResetItemPositions();
        ResetSwipeData();
    }
    
    void ResetItemPositions()
    {
        for (int i = 0; i < static_cast<int>(mItems.size()); ++i)
        {
            ResetPositionForItem(i, mItems[i]);
        }
    }
    
    void ResetPositionForItem(int itemIndex, ContainerEntryT& item)
    {
        switch (mValidSwipeDirection)
        {
            case SwipeDirection::HORIZONTAL:
            {
                item.mSceneObject->mPosition =
                {
                    (mContainerBounds.bottomLeft.x + mContainerBounds.topRight.x)/2.0f + itemIndex * item.mSceneObject->mScale.x/2.0f,
                    (mContainerBounds.bottomLeft.y + mContainerBounds.topRight.y)/2.0f,
                    mContainerItemsZ
                };
            } break;
            
            case SwipeDirection::VERTICAL:
            {
                item.mSceneObject->mPosition =
                {
                    (mContainerBounds.bottomLeft.x + mContainerBounds.topRight.x)/2.0f,
                    (mContainerBounds.bottomLeft.y + mContainerBounds.topRight.y)/2.0f + itemIndex * item.mSceneObject->mScale.y/2.0f,
                    mContainerItemsZ
                };
            }
        }
    }
    
    UpdateResult Update(const float dtMillis)
    {
        UpdateResult updateResult;
        updateResult.mInteractionType = InteractionType::NONE;
        updateResult.mInteractedElementId = -1;
        
        const auto& inputStateManager = CoreSystemsEngine::GetInstance().GetInputStateManager();
        auto& animationManager = CoreSystemsEngine::GetInstance().GetAnimationManager();
        
        auto worldTouchPos = inputStateManager.VGetPointingPosInWorldSpace(mScene.GetCamera().GetViewMatrix(), mScene.GetCamera().GetProjMatrix());
        
        auto firstSceneObject = mScene.FindSceneObject(strutils::StringId(mContainerName.GetString() + "_" + std::to_string(0)));
        auto lastSceneObject = mScene.FindSceneObject(strutils::StringId(mContainerName.GetString() + "_" + std::to_string(mItems.size() - 1)));
        
        mSwipeVelocityDelta *= CARD_VELOCITY_DAMPING;
        
        if (inputStateManager.VButtonTapped(input::Button::MAIN_BUTTON))
        {
            bool touchInVisibleContainerArea = math::IsPointInsideRectangle(mContainerBounds.bottomLeft, mContainerBounds.topRight, worldTouchPos);
            if (touchInVisibleContainerArea)
            {
                mHasStartedSwipe = true;
                mSwipeStartPos = glm::vec3(worldTouchPos.x, worldTouchPos.y, 0.0f);
                mSwipeCurrentPos = mSwipeStartPos;
                mSwipeDurationMillis = 0.0f;
                updateResult.mInteractionType = InteractionType::INTERACTED_WITH_CONTAINER_AREA;
                
                for (int i = 0; i < static_cast<int>(mItems.size()); ++i)
                {
                    auto sceneObjectRect = scene_object_utils::GetSceneObjectBoundingRect(*mItems[i].mSceneObject);
                    if (math::IsPointInsideRectangle(sceneObjectRect.bottomLeft, sceneObjectRect.topRight, worldTouchPos))
                    {
                        updateResult.mInteractionType = InteractionType::INTERACTED_WITH_ELEMENTS;
                        updateResult.mInteractedElementId = i;
                    }
                }
            }
            else if (!touchInVisibleContainerArea || animationManager.IsAnimationPlaying(RUBBER_BANDING_ANIMATION_NAME))
            {
                ResetSwipeData();
            }
        }
        else if (inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && mItems.size() > MIN_ITEMS_TO_ANIMATE)
        {
            if (mHasStartedSwipe && !animationManager.IsAnimationPlaying(RUBBER_BANDING_ANIMATION_NAME) && firstSceneObject != nullptr && lastSceneObject != nullptr)
            {
                mSwipeDurationMillis += dtMillis;
                
                auto currentTouchPos = inputStateManager.VGetPointingPosInWorldSpace(mScene.GetCamera().GetViewMatrix(), mScene.GetCamera().GetProjMatrix());
                float targetDx = (currentTouchPos.x - mSwipeCurrentPos.x);
                
                if (firstSceneObject->mPosition.x + targetDx > mContainerCutoffValues.t)
                {
                    targetDx /= (firstSceneObject->mPosition.x + targetDx - mContainerCutoffValues.t) * OVERSWIPE_DAMPING;
                }
                else if (lastSceneObject->mPosition.x + targetDx < mContainerCutoffValues.s)
                {
                    targetDx /= -(lastSceneObject->mPosition.x + targetDx -mContainerCutoffValues.s) * OVERSWIPE_DAMPING;
                }

                for (auto& entry: mItems)
                {
                    auto entrySceneObject = entry.mSceneObject;
                    entrySceneObject->mPosition.x += targetDx;
                }
                
                // Direction reversal check
                float deltaNoiseThreshold = SWIPE_DELTA_DIRECTION_CHANGE_NOISE_THRESHOLD;
                float newDelta = math::Abs(currentTouchPos.x - mSwipeCurrentPos.x) > deltaNoiseThreshold ? currentTouchPos.x - mSwipeCurrentPos.x : mSwipeDelta;
                if ((mSwipeDelta > 0.0f && newDelta < 0.0f) ||
                    (mSwipeDelta < 0.0f && newDelta > 0.0f))
                {
                    mSwipeDurationMillis = 0.0f;
                    mSwipeStartPos = glm::vec3(currentTouchPos.x, currentTouchPos.y, 0.0f);
                }
                
                mSwipeDelta = newDelta;
                mSwipeCurrentPos = glm::vec3(currentTouchPos.x, currentTouchPos.y, 0.0f);
            }
        }
        else if (!inputStateManager.VButtonPressed(input::Button::MAIN_BUTTON) && mItems.size() > MIN_ITEMS_TO_ANIMATE && firstSceneObject != nullptr && lastSceneObject != nullptr)
        {
            if (firstSceneObject->mPosition.x > mContainerCutoffValues.t)
            {
                float xOffset = mContainerCutoffValues.t - firstSceneObject->mPosition.x;
                for (auto& entry: mItems)
                {
                    auto entrySceneObject = entry.mSceneObject;
                    auto targetPosition = entrySceneObject->mPosition;
                    targetPosition.x += xOffset;
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(entrySceneObject, targetPosition, entrySceneObject->mScale, RUBBER_BANDING_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [](){}, RUBBER_BANDING_ANIMATION_NAME);
                }
            }
            else if (lastSceneObject->mPosition.x < mContainerCutoffValues.s)
            {
                float xOffset = mContainerCutoffValues.s - lastSceneObject->mPosition.x;
                for (auto& entry: mItems)
                {
                    auto entrySceneObject = entry.mSceneObject;
                    auto targetPosition = entrySceneObject->mPosition;
                    targetPosition.x += xOffset;
                    animationManager.StartAnimation(std::make_unique<rendering::TweenPositionScaleAnimation>(entrySceneObject, targetPosition, entrySceneObject->mScale, RUBBER_BANDING_ANIMATION_DURATION, animation_flags::NONE, 0.0f, math::ElasticFunction, math::TweeningMode::EASE_IN), [](){}, RUBBER_BANDING_ANIMATION_NAME);
                }
            }
            
            if (mHasStartedSwipe && !animationManager.IsAnimationPlaying(RUBBER_BANDING_ANIMATION_NAME))
            {
                mHasStartedSwipe = false;
                auto currentTouchPos = inputStateManager.VGetPointingPosInWorldSpace(mScene.GetCamera().GetViewMatrix(), mScene.GetCamera().GetProjMatrix());
                mSwipeVelocityDelta = (currentTouchPos.x - mSwipeStartPos.x)/mSwipeDurationMillis;
                mSwipeDurationMillis = 0.0f;
                mSwipeDelta = 0.0f;
            }
            else if (!animationManager.IsAnimationPlaying(RUBBER_BANDING_ANIMATION_NAME))
            {
                float targetDx = mSwipeVelocityDelta * dtMillis;
                
                if (firstSceneObject->mPosition.x + targetDx > mContainerCutoffValues.t)
                {
                    float xOffset = mContainerCutoffValues.t - firstSceneObject->mPosition.x;
                    for (auto& entry: mItems)
                    {
                        auto entrySceneObject = entry.mSceneObject;
                        entrySceneObject->mPosition.x += xOffset;
                    }
                    
                    mSwipeVelocityDelta = 0.0f;
                    targetDx = 0.0f;
                }
                else if (lastSceneObject->mPosition.x + targetDx < mContainerCutoffValues.s)
                {
                    float xOffset = mContainerCutoffValues.s - lastSceneObject->mPosition.x;
                    for (auto& entry: mItems)
                    {
                        auto entrySceneObject = entry.mSceneObject;
                        entrySceneObject->mPosition.x += xOffset;
                    }
                    
                    mSwipeVelocityDelta = 0.0f;
                    targetDx = 0.0f;
                }
                
                for (auto& entry: mItems)
                {
                    auto entrySceneObject = entry.mSceneObject;
                    entrySceneObject->mPosition.x += targetDx;
                }
            }
        }
        
        return updateResult;
    }
    
    std::vector<ContainerEntryT>& GetItems()
    {
        return mItems;
    }
    
private:
    void ResetSwipeData()
    {
        mHasStartedSwipe = false;
        mSwipeDurationMillis = 0.0f;
        mSwipeVelocityDelta = 0.0f;
        mSwipeDelta = 0.0f;
    }
    
private:
    const SwipeDirection mValidSwipeDirection;
    const math::Rectangle mContainerBounds;
    const glm::vec2 mContainerCutoffValues;
    const strutils::StringId mContainerName;
    const float mContainerItemsZ;
    scene::Scene& mScene;
    std::vector<ContainerEntryT> mItems;
    glm::vec3 mSwipeStartPos;
    glm::vec3 mSwipeCurrentPos;
    bool mHasStartedSwipe;
    float mSwipeDurationMillis = 0.0f;
    float mSwipeVelocityDelta = 0.0f;
    float mSwipeDelta = 0.0f;
};

///------------------------------------------------------------------------------------------------

#endif /* SwipeableContainer_h */
