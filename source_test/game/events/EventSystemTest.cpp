///------------------------------------------------------------------------------------------------
///  StringUtilsTest.cpp
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023
///------------------------------------------------------------------------------------------------

#include <gtest/gtest.h>
#include <game/events/EventSystem.h>

///------------------------------------------------------------------------------------------------

class TestEvent final
{
public:
    TestEvent(int val) : mVal(val) {}
    int GetVal() const { return mVal; }
    
private:
    int mVal;
};

class TestEvent2 final
{
public:
    TestEvent2(int val) : mVal(val) {}
    int GetVal() const { return mVal; }
    
private:
    int mVal;
};

///------------------------------------------------------------------------------------------------

class TestEventListener final: public events::IListener
{
public:
    void OnTestEvent(const TestEvent& event)
    {
        mVal = event.GetVal();
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
        events::EventSystem::GetInstance().RegisterForEvent<TestEvent>(&mTestListener, &TestEventListener::OnTestEvent);
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

TEST_F(EventSystemTests, TestUnregistrationFromEventFollowedByReRegistrationTriggersCallbackForSubsequentDispatches)
{
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    EXPECT_EQ(mTestListener.GetVal(), 1);
    
    events::EventSystem::GetInstance().UnregisterForEvent<TestEvent>(&mTestListener);    
    events::EventSystem::GetInstance().RegisterForEvent<TestEvent>(&mTestListener, &TestEventListener::OnTestEvent);
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(3);
    EXPECT_EQ(mTestListener.GetVal(), 3);
}

///------------------------------------------------------------------------------------------------

TEST_F(EventSystemTests, TestListenerDeallocationDoesNotTriggerCallbackForSubsequentDispatches)
{
    static int sEventsListenedTo = 0;
    class NotSoLongLivedTestEventListener final: public events::IListener
    {
    public:
        NotSoLongLivedTestEventListener()
        {
            events::EventSystem::GetInstance().RegisterForEvent<TestEvent>(this, &NotSoLongLivedTestEventListener::OnTestEvent);
        }
        
    private:
        void OnTestEvent(const TestEvent&)
        {
            sEventsListenedTo++;
        }
    };
    
    {
        NotSoLongLivedTestEventListener listener;
        
        events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
        EXPECT_EQ(sEventsListenedTo, 1);
    }
    
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    EXPECT_EQ(sEventsListenedTo, 1);
}

///------------------------------------------------------------------------------------------------

TEST_F(EventSystemTests, TestListenerDeallocationDoesNotTriggerCallbackForSubsequentDispatchesOfAllRegisteredEvents)
{
    static int sEvents1ListenedTo = 0;
    static int sEvents2ListenedTo = 0;
    
    class NotSoLongLivedTestEventListener final: public events::IListener
    {
    public:
        NotSoLongLivedTestEventListener()
        {
            events::EventSystem::GetInstance().RegisterForEvent<TestEvent>(this, &NotSoLongLivedTestEventListener::OnTestEvent1);
            events::EventSystem::GetInstance().RegisterForEvent<TestEvent2>(this, &NotSoLongLivedTestEventListener::OnTestEvent2);
        }
        
    private:
        void OnTestEvent1(const TestEvent&)
        {
            sEvents1ListenedTo++;
        }
        
        void OnTestEvent2(const TestEvent2&)
        {
            sEvents2ListenedTo++;
        }
    };
    
    {
        NotSoLongLivedTestEventListener listener;
        
        events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
        EXPECT_EQ(sEvents1ListenedTo, 1);
        EXPECT_EQ(sEvents2ListenedTo, 0);
        
        events::EventSystem::GetInstance().DispatchEvent<TestEvent2>(1);
        EXPECT_EQ(sEvents1ListenedTo, 1);
        EXPECT_EQ(sEvents2ListenedTo, 1);
    }
    
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    events::EventSystem::GetInstance().DispatchEvent<TestEvent2>(1);
    
    EXPECT_EQ(sEvents1ListenedTo, 1);
    EXPECT_EQ(sEvents2ListenedTo, 1);
}

///------------------------------------------------------------------------------------------------

TEST_F(EventSystemTests, TestEventRegistrationWithLambda)
{
    static int sEventsListenedTo = 0;
    class NotSoLongLivedTestEventListener final: public events::IListener
    {
    public:
        void OnTestEvent(const TestEvent&)
        {
            sEventsListenedTo++;
        }
    };
    
    NotSoLongLivedTestEventListener listener;
    {
        auto listenerHandle = events::EventSystem::GetInstance().RegisterForEvent<TestEvent>([&](const TestEvent& e){listener.OnTestEvent(e); });
        events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    }
    
    events::EventSystem::GetInstance().DispatchEvent<TestEvent>(1);
    EXPECT_EQ(sEventsListenedTo, 1);
}

///------------------------------------------------------------------------------------------------
