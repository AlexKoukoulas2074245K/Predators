///------------------------------------------------------------------------------------------------
///  AppleUtils.cpp
///  Predators
///
///  Created by Alex Koukoulas on 20/01/2024.
///-----------------------------------------------------------------------------------------------

#include <platform_utilities/AppleUtils.h>
#import <Foundation/Foundation.h>
#import <platform_utilities/Reachability.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/PlatformMacros.h>

#if __has_include(<UIKit/UIKit.h>)
#import <UIKit/UIKit.h>
#endif

///-----------------------------------------------------------------------------------------------

namespace apple_utils
{

///-----------------------------------------------------------------------------------------------

bool IsConnectedToTheInternet()
{
    return !([[Reachability reachabilityForInternetConnection] currentReachabilityStatus] == NotReachable);
}

///-----------------------------------------------------------------------------------------------

std::string GetPersistentDataDirectoryPath()
{
#if defined(MACOS)
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* path = [paths objectAtIndex:0];
    return std::string([path UTF8String]) + "/";
#else
    return std::string(getenv("HOME")) + "/Documents/";
#endif
}

///-----------------------------------------------------------------------------------------------

std::string GetDeviceId()
{
#if defined(MACOS)
    NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
    NSString* deviceId = [NSString stringWithFormat:@"%@ %ld.%ld.%ld", [[NSHost currentHost] localizedName], (long)version.majorVersion, (long)version.minorVersion, (long)version.patchVersion];
#else
    NSString* deviceId = [[[UIDevice currentDevice] identifierForVendor] UUIDString];
#endif
    
    return std::string([deviceId UTF8String]);
}

///-----------------------------------------------------------------------------------------------

std::string GetDeviceName()
{
#if defined(MACOS)
    return "iMac/MacBook";
#else
    UIDevice *device = [UIDevice currentDevice];
    return std::string([device.name UTF8String]);
#endif
    
    return "Unknown Device";
}

///-----------------------------------------------------------------------------------------------

void SetAssetFolder()
{
    NSString *launchPath=[NSBundle.mainBundle
        pathForResource:@"assets"
        ofType:nil
    ];
    
    resources::ResourceLoadingService::RES_ROOT = std::string([launchPath UTF8String]) + "/";
}

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------
