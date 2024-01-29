///------------------------------------------------------------------------------------------------
///  AppleUtils.cpp
///  Predators
///
///  Created by Alex Koukoulas on 20/01/2024.
///-----------------------------------------------------------------------------------------------

#include <platform_utilities/AppleUtils.h>
#import <Foundation/Foundation.h>
#import <platform_utilities/PredatorsReachability.h>
#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/utils/StringUtils.h>
#include <codecvt>
#import <StoreKit/StoreKit.h>
#if __has_include(<UIKit/UIKit.h>)
#import <UIKit/UIKit.h>
#endif

///-----------------------------------------------------------------------------------------------

@interface InAppPurchaseManager : NSObject <SKProductsRequestDelegate, SKPaymentTransactionObserver>

- (void) addObserverToTransactionQueue;
- (void) requestProductInformationWithProductIdentifiers:(NSSet<NSString*>*)productIdentifiers;
- (void) initiatePurchaseWithProductId:(NSString*)productId;

@end

///-----------------------------------------------------------------------------------------------

static std::function<void(apple_utils::PurchaseResultData)> purchaseFinishedCallback = nullptr;
static InAppPurchaseManager* purchaseManager = nil;
static NSArray* products = nil;

@implementation InAppPurchaseManager

- (void)addObserverToTransactionQueue
{
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
}

// Method to request product information with inline block
- (void)requestProductInformationWithProductIdentifiers:(NSSet<NSString*>*)productIdentifiers
{
    SKProductsRequest *productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:productIdentifiers];
    productsRequest.delegate = self;
    [productsRequest start];
}

// Method to initiate a product purchase
- (void) initiatePurchaseWithProductId:(NSString*)productId
{
    for (SKProduct* product in products)
    {
        if ([product.productIdentifier isEqualToString:productId])
        {
            SKPayment *payment = [SKPayment paymentWithProduct:product];
            SKPaymentQueue* paymentQueue = [SKPaymentQueue defaultQueue];
            [paymentQueue addPayment:payment];
        }
    }
}

// Delegate method to handle product information response
- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    products = response.products;
    for (SKProduct* product in products)
    {
        NSLog(@"Product ID: %@, Title: %@, Price: %@%@, Description: %@", product.productIdentifier, product.localizedTitle, product.priceLocale.currencySymbol, product.price, product.localizedDescription);
    }
}

// Delegate method to handle transaction updates
- (void)paymentQueue:(nonnull SKPaymentQueue *)queue updatedTransactions:(nonnull NSArray<SKPaymentTransaction *> *)transactions
{
    if (!purchaseFinishedCallback)
    {
        return;
    }
    
    for (SKPaymentTransaction* transaction in transactions)
    {
        switch (transaction.transactionState)
        {
            case SKPaymentTransactionStatePurchased:
            {
                NSLog(@"Transaction %@ at Date %@ Sucessful transaction", transaction.transactionIdentifier, transaction.transactionDate);
                purchaseFinishedCallback({ std::string([transaction.transactionIdentifier UTF8String]), std::string([transaction.payment.productIdentifier UTF8String]), true});
                [queue finishTransaction:transaction];
                
            } break;
                
            case SKPaymentTransactionStateFailed:
                NSLog(@"Transaction %@ at Date %@ Failed transaction", transaction.transactionIdentifier, transaction.transactionDate);
                purchaseFinishedCallback({ "", std::string([transaction.payment.productIdentifier UTF8String]), false});
                [queue finishTransaction:transaction];
                break;
            case SKPaymentTransactionStateRestored:
                NSLog(@"Transaction %@ at Date %@ Transaction restored", transaction.transactionIdentifier, transaction.transactionDate);
                purchaseFinishedCallback({ std::string([transaction.transactionIdentifier UTF8String]), std::string([transaction.payment.productIdentifier UTF8String]), true});
                [queue finishTransaction:transaction];
                break;
            case SKPaymentTransactionStatePurchasing:
                NSLog(@"Transaction %@ at Date %@Transaction pending purchasing", transaction.transactionIdentifier, transaction.transactionDate);
            default:
                break;
        }
    }
}

@end

///-----------------------------------------------------------------------------------------------

namespace apple_utils
{

///-----------------------------------------------------------------------------------------------

bool IsConnectedToTheInternet()
{
    return !([[PredatorsReachability reachabilityForInternetConnection] currentReachabilityStatus] == NotReachable);
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

bool HasLoadedProducts()
{
    return products != nil;
}

///-----------------------------------------------------------------------------------------------

void LoadStoreProducts(const std::vector<std::string>& productIdsToLoad)
{
    // Instantiate InAppPurchaseManager
    if (!purchaseManager)
    {
        purchaseManager = [[InAppPurchaseManager alloc] init];
        [purchaseManager addObserverToTransactionQueue];
    }
    
    // Request product information
    NSMutableSet<NSString*>* productIdSet = [NSMutableSet set];
    for (const auto& productId: productIdsToLoad)
    {
        NSString* nsStringProductId = [NSString stringWithUTF8String:productId.c_str()];
        [productIdSet addObject:nsStringProductId];
    }
    
    [purchaseManager requestProductInformationWithProductIdentifiers:productIdSet];
}

///-----------------------------------------------------------------------------------------------

std::string GetProductPrice(const std::string& productId)
{
    if (purchaseManager && products)
    {
        NSString* nsStringProductId = [NSString stringWithUTF8String:productId.c_str()];
        for (SKProduct* product in products)
        {
            if ([product.productIdentifier isEqualToString:nsStringProductId])
            {
                NSNumberFormatter *formatter = [NSNumberFormatter new];
                [formatter setNumberStyle:NSNumberFormatterCurrencyStyle];
                [formatter setLocale:product.priceLocale];
                NSString* cost = [formatter stringFromNumber:product.price];
                
                std::string cppString([cost UTF8String]);
                cppString = " " + cppString;
                
                // Euro
                strutils::StringReplaceAllOccurences("\xe2\x82\xac", "\x80", cppString);
                
                // ANSI currencies. Just need to strip \xc2
                strutils::StringReplaceAllOccurences("\xc2", "", cppString);
                
                return cppString;
            }
        }
    }
    
    return "";
}

///-----------------------------------------------------------------------------------------------

void InitiateProductPurchase(const std::string& productId, std::function<void(PurchaseResultData)> onPurchaseFinishedCallback)
{
    if (purchaseManager && products)
    {
        NSString* nsStringProductId = [NSString stringWithUTF8String:productId.c_str()];
        purchaseFinishedCallback = onPurchaseFinishedCallback;
        [purchaseManager initiatePurchaseWithProductId:nsStringProductId];
    }
}

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------
