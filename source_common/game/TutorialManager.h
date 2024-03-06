///------------------------------------------------------------------------------------------------
///  TutorialManager.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 06/03/2024
///------------------------------------------------------------------------------------------------

#ifndef TutorialManager_h
#define TutorialManager_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <game/events/EventSystem.h>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

///------------------------------------------------------------------------------------------------

inline const strutils::StringId TEST_TUTORIAL = strutils::StringId("test_tutorial");

///------------------------------------------------------------------------------------------------

struct TutorialDefinition
{
    TutorialDefinition(const strutils::StringId& tutorialName, const std::string& tutorialDescription)
        : mTutorialName(tutorialName)
        , mTutorialDescription(tutorialDescription)
    {
    }
    
    const strutils::StringId mTutorialName;
    const std::string mTutorialDescription;
};

///------------------------------------------------------------------------------------------------

namespace scene { class SceneObject; }
class AnimatedButton;
class TutorialManager final : public events::IListener
{
    friend class Game;
public:
    TutorialManager();
    ~TutorialManager();
    TutorialManager(const TutorialManager&) = delete;
    TutorialManager(TutorialManager&&) = delete;
    const TutorialManager& operator = (const TutorialManager&) = delete;
    TutorialManager& operator = (TutorialManager&&) = delete;
    
    const std::unordered_map<strutils::StringId, TutorialDefinition, strutils::StringIdHasher>& GetTutorialDefinitions() const;
    bool HasAnyActiveTutorial() const;
    bool IsTutorialActive(const strutils::StringId& tutorialName) const;
    void LoadTutorialDefinitions();
    void Update(const float dtMillis);
    
private:
    void CreateTutorial();
    void FadeOutTutorial();
    void DestroyTutorial();
    void UpdateActiveTutorial(const float dtMillis);
    void OnTutorialTrigger(const events::TutorialTriggerEvent&);
    void ToggleCheckbox();
    void SetCheckboxValue(const bool checkboxValue);
    
private:
    std::vector<strutils::StringId> mActiveTutorials;
    std::unordered_map<strutils::StringId, TutorialDefinition, strutils::StringIdHasher> mTutorialDefinitions;
    std::vector<std::shared_ptr<scene::SceneObject>> mTutorialSceneObjects;
    std::unique_ptr<AnimatedButton> mContinueButton;
};

///------------------------------------------------------------------------------------------------

#endif /* TutorialManager_h */
