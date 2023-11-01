///------------------------------------------------------------------------------------------------
///  StringUtilsTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <game/events/EventSystem.h>

///------------------------------------------------------------------------------------------------

class TestEvent final: public events::IEvent
{
public:
    TestEvent(int val) : mVal(val) {}
    int GetVal() const { return mVal; }
    
private:
    int mVal;
};

///------------------------------------------------------------------------------------------------

class TestEventListener final: public events::IListener
{
public:
    void OnTestEvent(const events::IEvent& event)
    {
        mVal = static_cast<const TestEvent&>(event).GetVal();
    }
    int GetVal() const { return mVal; }
private:
    int mVal;
};

///------------------------------------------------------------------------------------------------

class EventSystemTests : public testing::Test
{
protected:
    void SetUp() override
    {
        events::EventSystem::GetInstance().RegisterForEvent<TestEvent>(&mTestListener, [&](const events::IEvent& event) { mTestListener.OnTestEvent(event); });
    }
    
protected:
    TestEventListener mTestListener;
};


///------------------------------------------------------------------------------------------------

TEST_F(EventSystemTests, TestMultipleEventDispatchesTriggerCallback)
{
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    EXPECT_EQ(mTestListener.GetVal(), 1);
    
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(2);
    EXPECT_EQ(mTestListener.GetVal(), 2);
}

///------------------------------------------------------------------------------------------------

TEST_F(EventSystemTests, TestUnregistrationFromEventDoesNotTriggerCallbackForSubsequentDispatches)
{
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    EXPECT_EQ(mTestListener.GetVal(), 1);
    
    events::EventSystem::GetInstance().UnregisterForEvent<TestEvent>(&mTestListener);
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(2);
    EXPECT_EQ(mTestListener.GetVal(), 1);
}

///------------------------------------------------------------------------------------------------

TEST_F(EventSystemTests, TestListenerDeallocationDoesNotTriggerCallbackForSubsequentDispatches)
{
    static int sEventsListenedTo = 0;
    class NotSoLongLivedTestEventListener final: public events::IListener
    {
    public:
        void OnTestEvent(const events::IEvent&)
        {
            sEventsListenedTo++;
        }
    };
    
    {
        NotSoLongLivedTestEventListener listener;
        events::EventSystem::GetInstance().RegisterForEvent<TestEvent>(&listener, [&](const events::IEvent& event) { listener.OnTestEvent(event); });
        events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
        EXPECT_EQ(sEventsListenedTo, 1);
    }
    
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    EXPECT_EQ(sEventsListenedTo, 1);
}

///------------------------------------------------------------------------------------------------
