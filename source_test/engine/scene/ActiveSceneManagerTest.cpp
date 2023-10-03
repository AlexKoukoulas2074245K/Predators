///------------------------------------------------------------------------------------------------
///  ActiveSceneManagerTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 03/10/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <engine/scene/ActiveSceneManager.h>
#include <engine/scene/Scene.h>

TEST(ActiveSceneManagerOperationTests, TestBasicInsertionAndRetrieval)
{
    const strutils::StringId NAME("ABCD");
    
    scene::ActiveSceneManager sceneManager;
    
    auto testScene = sceneManager.CreateScene(strutils::StringId(NAME));
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 1);
    
    auto sameTestScene = sceneManager.FindScene(NAME);
    
    EXPECT_NE(sameTestScene, nullptr);
    
    EXPECT_EQ(sameTestScene->GetName(), NAME);
}

TEST(ActiveSceneManagerOperationTests, TestPointerValidityPostMassInsertion)
{
    const strutils::StringId NAME("ABCD");
    
    scene::ActiveSceneManager sceneManager;
    
    auto testScene = sceneManager.CreateScene(strutils::StringId(NAME));
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 1);
    
    auto sameTestScene = sceneManager.FindScene(NAME);
    
    EXPECT_NE(sameTestScene, nullptr);
    
    EXPECT_EQ(sameTestScene->GetName(), NAME);
    
    for (int i = 0; i < 9999; i++)
    {
        auto so = sceneManager.CreateScene();
    }
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 10000);
    
    sameTestScene = sceneManager.FindScene(NAME);
    
    EXPECT_NE(sameTestScene, nullptr);
    
    EXPECT_EQ(testScene->GetName(), sameTestScene->GetName());
}

TEST(ActiveSceneManagerOperationTests, TestBasicInsertionAndRemoval)
{
    const strutils::StringId NAME("ABCD");
    
    scene::ActiveSceneManager sceneManager;
    
    auto testScene = sceneManager.CreateScene(strutils::StringId(NAME));
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 1);
    
    auto sameTestScene = sceneManager.FindScene(NAME);
    
    EXPECT_NE(sameTestScene, nullptr);
    
    sceneManager.RemoveScene(NAME);
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 0);
    
    EXPECT_EQ(sceneManager.FindScene(NAME), nullptr);
}

TEST(ActiveSceneManagerOperationTests, TestRemovalOfEmptyName)
{
    const strutils::StringId EMPTY_NAME;
    const strutils::StringId NAME("ABCD");
    
    scene::ActiveSceneManager sceneManager;
    
    auto testScene = sceneManager.CreateScene(strutils::StringId(NAME));
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 1);
    
    sceneManager.RemoveScene(EMPTY_NAME);
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 1);
    
    auto emptyNameTestSceneObject = sceneManager.CreateScene(EMPTY_NAME);

    EXPECT_EQ(sceneManager.GetSceneCount(), 2);
    
    sceneManager.RemoveScene(EMPTY_NAME);
    
    EXPECT_EQ(sceneManager.GetSceneCount(), 1);
}
