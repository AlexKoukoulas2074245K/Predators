///------------------------------------------------------------------------------------------------
///  GameSceneTransitionManagerTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 04/12/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <game/GameSceneTransitionManager.h>
#include <engine/scene/Scene.h>

///------------------------------------------------------------------------------------------------

TEST(SceneManagerOperationTests, TestCorrectLogicSceneManagerGetsUpdated)
{
    static const strutils::StringId SCENE_NAME("ABCD");
    static int sUpdateCounter = 0;
    
    class DummySceneLogicManager final: public ISceneLogicManager
    {
    public:
        const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override
        {
            static std::vector<strutils::StringId> applicableScenes = {SCENE_NAME};
            return applicableScenes;
        };
        
        void VInitScene(std::shared_ptr<scene::Scene>) override {};
        void VUpdate(const float, std::shared_ptr<scene::Scene>) override { sUpdateCounter++; };
        void VDestroyScene(std::shared_ptr<scene::Scene>) override {}
    };
    
    GameSceneTransitionManager gstm;
    gstm.RegisterSceneLogicManager<DummySceneLogicManager>();
    gstm.ChangeToScene(SCENE_NAME, false);
    gstm.Update(0.0f);
    
    EXPECT_EQ(sUpdateCounter, 1);
}

TEST(SceneManagerOperationTests, TestAssertTriggeredOnMultipleLogicSceneManagersBeingApplicable)
{
    static const strutils::StringId SCENE_NAME("ABCD");
    
    class DummySceneLogicManagerA final: public ISceneLogicManager
    {
    public:
        const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override
        {
            static std::vector<strutils::StringId> applicableScenes = {SCENE_NAME};
            return applicableScenes;
        };
        
        void VInitScene(std::shared_ptr<scene::Scene>) override {};
        void VUpdate(const float, std::shared_ptr<scene::Scene>) override {};
        void VDestroyScene(std::shared_ptr<scene::Scene>) override {}
    };
    
    class DummySceneLogicManagerB final: public ISceneLogicManager
    {
    public:
        const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override
        {
            static std::vector<strutils::StringId> applicableScenes = {SCENE_NAME};
            return applicableScenes;
        };
        
        void VInitScene(std::shared_ptr<scene::Scene>) override {};
        void VUpdate(const float, std::shared_ptr<scene::Scene>) override {};
        void VDestroyScene(std::shared_ptr<scene::Scene>) override {}
    };
    
    GameSceneTransitionManager gstm;
    gstm.RegisterSceneLogicManager<DummySceneLogicManagerA>();
    gstm.RegisterSceneLogicManager<DummySceneLogicManagerB>();
    
    EXPECT_DEBUG_DEATH(gstm.ChangeToScene(SCENE_NAME, false), "");
}

TEST(SceneManagerOperationTests, TestCorrectSceneLogicManagerInitsUpdatesAndDestructionsOnPushedAndPoppedModal)
{
    static const strutils::StringId SCENE_NAME("ABCD");
    static const strutils::StringId MODAL_SCENE_NAME("MODAL_ABCD");
    
    static int initCounterA = 0;
    static int updateCounterA = 0;
    static int destructionCounterA = 0;
    class DummySceneLogicManagerA final: public ISceneLogicManager
    {
    public:
        const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override
        {
            static std::vector<strutils::StringId> applicableScenes = {SCENE_NAME};
            return applicableScenes;
        };
        
        void VInitScene(std::shared_ptr<scene::Scene>) override { initCounterA++; };
        void VUpdate(const float, std::shared_ptr<scene::Scene>) override { updateCounterA++; };
        void VDestroyScene(std::shared_ptr<scene::Scene>) override { destructionCounterA++; }
    };
    
    static int initCounterB = 0;
    static int updateCounterB = 0;
    static int destructionCounterB = 0;
    class DummySceneLogicManagerB final: public ISceneLogicManager
    {
    public:
        const std::vector<strutils::StringId>& VGetApplicableSceneNames() const override
        {
            static std::vector<strutils::StringId> applicableScenes = {MODAL_SCENE_NAME};
            return applicableScenes;
        };
        
        void VInitScene(std::shared_ptr<scene::Scene>) override { initCounterB++; };
        void VUpdate(const float, std::shared_ptr<scene::Scene>) override { updateCounterB++; };
        void VDestroyScene(std::shared_ptr<scene::Scene>) override { destructionCounterB++; }
    };
    
    GameSceneTransitionManager gstm;
    gstm.RegisterSceneLogicManager<DummySceneLogicManagerA>();
    gstm.RegisterSceneLogicManager<DummySceneLogicManagerB>();
    
    gstm.ChangeToScene(SCENE_NAME, false);
    EXPECT_EQ(initCounterA, 1);
    EXPECT_EQ(updateCounterA, 0);
    EXPECT_EQ(destructionCounterA, 0);
    EXPECT_EQ(initCounterB, 0);
    EXPECT_EQ(updateCounterB, 0);
    EXPECT_EQ(destructionCounterB, 0);
    
    gstm.Update(0.0f);
    EXPECT_EQ(initCounterA, 1);
    EXPECT_EQ(updateCounterA, 1);
    EXPECT_EQ(destructionCounterA, 0);
    EXPECT_EQ(initCounterB, 0);
    EXPECT_EQ(updateCounterB, 0);
    EXPECT_EQ(destructionCounterB, 0);
    
    gstm.ChangeToScene(MODAL_SCENE_NAME, true);
    EXPECT_EQ(initCounterA, 1);
    EXPECT_EQ(updateCounterA, 1);
    EXPECT_EQ(destructionCounterA, 0);
    EXPECT_EQ(initCounterB, 1);
    EXPECT_EQ(updateCounterB, 0);
    EXPECT_EQ(destructionCounterB, 0);
    
    gstm.Update(0.0f);
    EXPECT_EQ(initCounterA, 1);
    EXPECT_EQ(updateCounterA, 1);
    EXPECT_EQ(destructionCounterA, 0);
    EXPECT_EQ(initCounterB, 1);
    EXPECT_EQ(updateCounterB, 1);
    EXPECT_EQ(destructionCounterB, 0);
    
    gstm.PopModalScene();
    EXPECT_EQ(initCounterA, 1);
    EXPECT_EQ(updateCounterA, 1);
    EXPECT_EQ(destructionCounterA, 0);
    EXPECT_EQ(initCounterB, 1);
    EXPECT_EQ(updateCounterB, 1);
    EXPECT_EQ(destructionCounterB, 1);
    
    gstm.Update(0.0f);
    EXPECT_EQ(initCounterA, 1);
    EXPECT_EQ(updateCounterA, 2);
    EXPECT_EQ(destructionCounterA, 0);
    EXPECT_EQ(initCounterB, 1);
    EXPECT_EQ(updateCounterB, 1);
    EXPECT_EQ(destructionCounterB, 1);
}
