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
#include <chrono>
#import <CloudKit/CloudKit.h>

///-----------------------------------------------------------------------------------------------

namespace cloudkit_utils
{

///-----------------------------------------------------------------------------------------------

CKRecord* currentProgressRecord = nil;
std::unique_ptr<std::chrono::system_clock::time_point> sTimePointToSaveNext = nullptr;

///-----------------------------------------------------------------------------------------------

void QueryPlayerProgress(std::function<void(QueryResultData)> onQueryCompleteCallback)
{
    NSString* containerIdentifier = @"iCloud.com.alexkoukoulas2074245k.Predators";
    CKContainer* customContainer = [CKContainer containerWithIdentifier:containerIdentifier];
    CKDatabase* privateDatabase = [customContainer privateCloudDatabase];
    
    CKQuery* query = [[CKQuery alloc] initWithRecordType:@"PlayerProgress" predicate:[NSPredicate predicateWithValue:YES]];
    query.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey:@"modificationDate" ascending:NO]];
    
    [privateDatabase performQuery:query inZoneWithID:nil completionHandler:^(NSArray *results, NSError *error) {
        QueryResultData resultData;
        if (error)
        {
            NSLog(@"Error querying progress: %@", error);
        }
        else
        {
            if (results.count > 0)
            {
                currentProgressRecord = results.firstObject;
                NSData* persistentData = currentProgressRecord[@"persistent"];
                NSData* storyData = currentProgressRecord[@"story"];
                NSData* lastBattleData = currentProgressRecord[@"last_battle"];
                
                NSString* persistentDataString = [[NSString alloc] initWithData:persistentData encoding:NSUTF8StringEncoding];
                NSString* storyDataString = [[NSString alloc] initWithData:storyData encoding:NSUTF8StringEncoding];
                NSString* lastBattleDataString = [[NSString alloc] initWithData:lastBattleData encoding:NSUTF8StringEncoding];
                
                resultData.mPersistentProgressRawString = std::string([persistentDataString UTF8String]);
                resultData.mStoryProgressRawString = std::string([storyDataString UTF8String]);
                resultData.mLastBattleRawString = std::string([lastBattleDataString UTF8String]);
                resultData.mSuccessfullyQueriedAtLeastOneFileField = true;
                
                onQueryCompleteCallback(resultData);
            }
            else
            {
                currentProgressRecord = [[CKRecord alloc] initWithRecordType:@"PlayerProgress"];
                NSLog(@"No progress data found");
            }
        }
    }];
}

///-----------------------------------------------------------------------------------------------

void SavePlayerProgress()
{
    if (!currentProgressRecord)
    {
        return;
    }
    
    auto persistentDataFileReaderLambda = [](const std::string& persistentFileNameWithoutExtension)
    {
        std::string dataFileExtension = ".json";
        
        auto filePath = apple_utils::GetPersistentDataDirectoryPath() + persistentFileNameWithoutExtension + dataFileExtension;
        
        std::ifstream dataFile(filePath);
        
        if (dataFile.is_open())
        {
            return std::string((std::istreambuf_iterator<char>(dataFile)), std::istreambuf_iterator<char>());
        }
        
        return std::string();
    };
    
    NSData* persistentData = [[NSString stringWithUTF8String:persistentDataFileReaderLambda("persistent").c_str()] dataUsingEncoding:NSUTF8StringEncoding];
    NSData* storyData = [[NSString stringWithUTF8String:persistentDataFileReaderLambda("story").c_str()] dataUsingEncoding:NSUTF8StringEncoding];
    NSData* lastBattleData = [[NSString stringWithUTF8String:persistentDataFileReaderLambda("last_battle").c_str()] dataUsingEncoding:NSUTF8StringEncoding];
    
    currentProgressRecord[@"persistent"] = persistentData;
    currentProgressRecord[@"story"] = storyData;
    currentProgressRecord[@"last_battle"] = lastBattleData;
    
    // Batch all cloud writes in 1-second blocks
    if (sTimePointToSaveNext)
    {
        return;
    }
    
    sTimePointToSaveNext = std::make_unique<std::chrono::system_clock::time_point>(std::chrono::system_clock::now());
    (*sTimePointToSaveNext) += std::chrono::milliseconds(1000);
}

///-----------------------------------------------------------------------------------------------

void CheckForCloudSaving()
{
    if (sTimePointToSaveNext)
    {
        if (std::chrono::system_clock::now() > *sTimePointToSaveNext)
        {
            NSString* containerIdentifier = @"iCloud.com.alexkoukoulas2074245k.Predators";
            CKContainer* customContainer = [CKContainer containerWithIdentifier:containerIdentifier];
            CKDatabase* privateDatabase = [customContainer privateCloudDatabase];
            
            [privateDatabase saveRecord:currentProgressRecord completionHandler:^(CKRecord *record, NSError *error) {
                if (error) {
                    NSLog(@"Error saving progress: %@", error);
                } else {
                    NSLog(@"Progress saved successfully");
                    currentProgressRecord = record;
                }
            }];
            
            sTimePointToSaveNext = nullptr;
        }
    }
}

///-----------------------------------------------------------------------------------------------

}

///-----------------------------------------------------------------------------------------------
