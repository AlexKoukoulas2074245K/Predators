///------------------------------------------------------------------------------------------------
///  AppleUtils.cpp
///  Predators
///
///  Created by Alex Koukoulas on 20/01/2024.
///-----------------------------------------------------------------------------------------------

#include <platform_utilities/AppleUtils.h>
#import <Foundation/Foundation.h>
#import <platform_utilities/Reachability.h>

///-----------------------------------------------------------------------------------------------

namespace apple_utils
{

///-----------------------------------------------------------------------------------------------

bool IsConnectedToTheInternet()
{
    return !([[Reachability reachabilityForInternetConnection] currentReachabilityStatus] == NotReachable);
}

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------
