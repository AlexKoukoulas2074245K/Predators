///------------------------------------------------------------------------------------------------
///  ValueWithDelayedDisplay.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 29/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef ValueWithDelayedDisplay_h
#define ValueWithDelayedDisplay_h

///------------------------------------------------------------------------------------------------

template<class T>
class ValueWithDelayedDisplay
{
public:
    ValueWithDelayedDisplay(const T initValue, const T initDisplayedValue = T())
        : mValue(initValue)
        , mDisplayedValue(initDisplayedValue)
    {
        
    }
    
    const T& GetValue() const { return mValue; }
    void SetValue(const T& value) { mValue = value; }
    
    const T& GetDisplayedValue() const { return mDisplayedValue; }
    void SetDisplayedValue(const T& displayedValue) { mDisplayedValue = displayedValue; }
    
private:
    T mValue;
    T mDisplayedValue;
};

///------------------------------------------------------------------------------------------------

#endif /* ValueWithDelayedDisplay_h */
