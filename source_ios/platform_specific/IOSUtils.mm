///------------------------------------------------------------------------------------------------
///  IOSUtils.h
///  Predators
///
///  Created by Alex Koukoulas on 16/11/2023.
///-----------------------------------------------------------------------------------------------

#ifndef Logging_h
#define Logging_h

///-----------------------------------------------------------------------------------------------

#include <platform_specific/IOSUtils.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIDevice.h>
#import <platform_specific/Reachability.h>

///-----------------------------------------------------------------------------------------------

namespace ios_utils
{

///-----------------------------------------------------------------------------------------------

bool IsIPad()
{
    NSString *deviceType = [UIDevice currentDevice].model;
    if([deviceType isEqualToString:@"iPhone"]) {
        return false;
    }
    else if([deviceType isEqualToString:@"iPod touch"]) {
        return false;
    }
    
    return true;
}

///-----------------------------------------------------------------------------------------------

bool IsConnectedToTheInternet()
{
    return !([[Reachability reachabilityForInternetConnection] currentReachabilityStatus] == NotReachable);
}

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------

#endif /* IOSUtils_h */
