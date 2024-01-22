///------------------------------------------------------------------------------------------------
///  CloudKitUtils.cpp
///  Predators
///
///  Created by Alex Koukoulas on 22/01/2024.
///-----------------------------------------------------------------------------------------------

#include <platform_utilities/AppleUtils.h>
#include <platform_utilities/CloudKitUtils.h>
#include <fstream>
#include <sstream>

#import <CloudKit/CloudKit.h>

///-----------------------------------------------------------------------------------------------

namespace cloudkit_utils
{

///-----------------------------------------------------------------------------------------------

void QueryPlayerProgress()
{
    NSString* containerIdentifier = @"iCloud.com.alexkoukoulas2074245k.Predators";
    CKContainer* customContainer = [CKContainer containerWithIdentifier:containerIdentifier];
    CKDatabase* privateDatabase = [customContainer privateCloudDatabase];
    
    CKQuery* query = [[CKQuery alloc] initWithRecordType:@"PlayerProgress" predicate:[NSPredicate predicateWithValue:YES]];
    
    [privateDatabase performQuery:query inZoneWithID:nil completionHandler:^(NSArray *results, NSError *error) {
        if (error)
        {
            NSLog(@"Error querying progress: %@", error);
        }
        else
        {
            if (results.count > 0)
            {
                CKRecord* progressRecord = results.firstObject;
                NSData* persistentData = progressRecord[@"persistent"];
                NSData* storyData = progressRecord[@"story"];
                NSData* lastBattleData = progressRecord[@"last_battle"];
                
                NSString* persistentDataString = [[NSString alloc] initWithData:persistentData encoding:NSUTF8StringEncoding];
                NSString* storyDataString = [[NSString alloc] initWithData:storyData encoding:NSUTF8StringEncoding];
                NSString* lastBattleDataString = [[NSString alloc] initWithData:lastBattleData encoding:NSUTF8StringEncoding];
                
                NSLog(@"Player's persistent: %@", persistentDataString);
                NSLog(@"Player's story: %@", storyDataString);
                NSLog(@"Player's last_battle: %@", lastBattleDataString);
            }
            else
            {
                NSLog(@"No progress data found");
            }
        }
    }];
}

///-----------------------------------------------------------------------------------------------

void SavePlayerProgress()
{
    CKRecord *progressRecord = [[CKRecord alloc] initWithRecordType:@"PlayerProgress"];
    
    auto persistentDataFileReaderLambda = [](const std::string& persistentFileNameWithoutExtension)
    {
    #if !defined(NDEBUG)
        std::string dataFileExtension = ".json";
        
        auto filePath = apple_utils::GetPersistentDataDirectoryPath() + persistentFileNameWithoutExtension + dataFileExtension;
        
        std::ifstream dataFile(filePath);
    #else
        std::string dataFileExtension = ".bin";
        auto filePath = apple_utils::GetPersistentDataDirectoryPath() + persistentFileNameWithoutExtension + dataFileExtension;
        
        std::ifstream dataFile(filePath, std::ios::binary);
    #endif
        
        if (dataFile.is_open())
        {
            return std::string((std::istreambuf_iterator<char>(dataFile)), std::istreambuf_iterator<char>());
        }
        
        return std::string();
    };
    
    NSData* persistentData = [[NSString stringWithUTF8String:persistentDataFileReaderLambda("persistent").c_str()] dataUsingEncoding:NSUTF8StringEncoding];
    NSData* storyData = [[NSString stringWithUTF8String:persistentDataFileReaderLambda("story").c_str()] dataUsingEncoding:NSUTF8StringEncoding];
    NSData* lastBattleData = [[NSString stringWithUTF8String:persistentDataFileReaderLambda("last_battle").c_str()] dataUsingEncoding:NSUTF8StringEncoding];

    progressRecord[@"persistent"] = persistentData;
    progressRecord[@"story"] = storyData;
    progressRecord[@"last_battle"] = lastBattleData;

    NSString* containerIdentifier = @"iCloud.com.alexkoukoulas2074245k.Predators";
    CKContainer* customContainer = [CKContainer containerWithIdentifier:containerIdentifier];
    CKDatabase* privateDatabase = [customContainer privateCloudDatabase];
    
    [privateDatabase saveRecord:progressRecord completionHandler:^(CKRecord *record, NSError *error) {
        if (error) {
            NSLog(@"Error saving progress: %@", error);
        } else {
            NSLog(@"Progress saved successfully");
        }
    }];
}

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------
