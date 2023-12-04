///------------------------------------------------------------------------------------------------
///  GameSceneTransitionManager.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 04/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef GameSceneTransitionManager_h
#define GameSceneTransitionManager_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <game/ISceneLogicManager.h>
#include <stack>
#include <unordered_map>
#include <vector>

///------------------------------------------------------------------------------------------------

class GameSceneTransitionManager final
{
public:
    GameSceneTransitionManager();
    
    template <typename SceneLogicManagerT>
    void RegisterSceneLogicManager()
    {
        static_assert(std::is_default_constructible<SceneLogicManagerT>());
        static_assert(std::is_base_of<ISceneLogicManager, SceneLogicManagerT>());
        
        SceneLogicManagerEntry entry;
        entry.mSceneLogicManager = std::make_unique<SceneLogicManagerT>();
        
        for (const auto& applicableSceneName: entry.mSceneLogicManager->VGetApplicableSceneNames())
        {
            entry.mSceneInitStatusMap[applicableSceneName] = false;
        }
        
        mRegisteredSceneLogicManagers.emplace_back(std::move(entry));
    }
    
    ISceneLogicManager* GetActiveSceneLogicManager();
    
    void Update(const float dtMillis);
    void ChangeToScene
    (
        const strutils::StringId& sceneName,
        const bool isModal,
        const float targetTransitionDurationSecs = 0.0f,
        const float maxTransitionDarkeningAlpha = 0.0f
    );
    void PopModalScene
    (
         const float targetTransitionDurationSecs = 0.0f,
         const float maxTransitionDarkeningAlpha = 0.0f
    );
    
private:
    void InitializeActiveSceneLogicManager();
    void DestroyActiveSceneLogicManager();
    
private:
    struct SceneLogicManagerEntry
    {
        std::unique_ptr<ISceneLogicManager> mSceneLogicManager = nullptr;
        std::unordered_map<strutils::StringId, bool, strutils::StringIdHasher> mSceneInitStatusMap;
    };
    
    std::vector<SceneLogicManagerEntry> mRegisteredSceneLogicManagers;
    
    struct ActiveSceneEntry
    {
        ISceneLogicManager* mActiveSceneLogicManager = nullptr;
        strutils::StringId mActiveSceneName;
    };
    
    std::stack<ActiveSceneEntry> mActiveSceneStack;
};

///------------------------------------------------------------------------------------------------

#endif /* GameSceneTransitionManager_h */
