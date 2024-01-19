///------------------------------------------------------------------------------------------------
///  EventSceneLogicManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 28/12/2023
///------------------------------------------------------------------------------------------------

#ifndef EventSceneLogicManager_h
#define EventSceneLogicManager_h

///------------------------------------------------------------------------------------------------

#include <game/events/EventSystem.h>
#include <game/scenelogicmanagers/ISceneLogicManager.h>
#include <memory>
#include <functional>
#include <vector>

///------------------------------------------------------------------------------------------------

class AnimatedButton;
class GuiObjectManager;
class EventSceneLogicManager final: public ISceneLogicManager, public events::IListener
{
public:
    EventSceneLogicManager();
    ~EventSceneLogicManager();
    
    const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override;
    
    void VInitSceneCamera(std::shared_ptr<scene::Scene> scene) override;
    void VInitScene(std::shared_ptr<scene::Scene> scene) override;
    void VUpdate(const float dtMillis, std::shared_ptr<scene::Scene> activeScene) override;
    void VDestroyScene(std::shared_ptr<scene::Scene> scene) override;
    std::shared_ptr<GuiObjectManager> VGetGuiObjectManager() override;
    
private:
    void RegisterForEvents();
    void OnWindowResize(const events::WindowResizeEvent& event);
    void SelectRandomStoryEvent();
    void TransitionToEventScreen(const int screenIndex);
    void CreateEventScreen(const int screenIndex);
    
private:
    class StoryRandomEventButtonData
    {
    public:
        StoryRandomEventButtonData(const std::string& buttonText, const int nextScreenIndex, const std::function<void()> onClickCallback = nullptr, const int randomCounter = 0)
            : mButtonText(buttonText)
            , mNextScreenIndex(nextScreenIndex)
            , mOnClickCallback(onClickCallback)
            , mRandomCounter(randomCounter)
        {
        }
        
        const std::string mButtonText;
        int mNextScreenIndex;
        const std::function<void()> mOnClickCallback;
        int mRandomCounter;
    };
    
    class StoryRandomEventScreenData
    {
    public:
        StoryRandomEventScreenData
        (
            const std::string& eventScreenPortraitTextureFilename,
            const std::vector<std::string>& eventScreenDescriptionSentences,
            const std::vector<StoryRandomEventButtonData>& eventScreenButtons
        )
            : mEventScreenPortraitTextureFilename(eventScreenPortraitTextureFilename)
            , mEventScreenDescriptionSentences(eventScreenDescriptionSentences)
            , mEventScreenButtons(eventScreenButtons)
        {
        }
        
        const std::string mEventScreenPortraitTextureFilename;
        const std::vector<std::string> mEventScreenDescriptionSentences;
        const std::vector<StoryRandomEventButtonData> mEventScreenButtons;
    };
    
    class StoryRandomEventData
    {
    public:
        StoryRandomEventData(const std::vector<StoryRandomEventScreenData>& eventScreens, std::function<bool()> applicabilityFunction)
            : mEventScreens(eventScreens)
            , mApplicabilityFunction(applicabilityFunction)
        {
        }
    
        const std::vector<StoryRandomEventScreenData> mEventScreens;
        const std::function<bool()> mApplicabilityFunction;
    };
    
private:
    std::vector<std::unique_ptr<AnimatedButton>> mCurrentEventButtons;
    std::vector<StoryRandomEventData> mRegisteredStoryEvents;
    std::shared_ptr<GuiObjectManager> mGuiManager;
    std::shared_ptr<scene::Scene> mScene;
    int mCurrentEventIndex;
    int mCurrentEventScreenIndex;
    bool mTransitioning;
};

///------------------------------------------------------------------------------------------------

#endif /* EventSceneLogicManager_h */
