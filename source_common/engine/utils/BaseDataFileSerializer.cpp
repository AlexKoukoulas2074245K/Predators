///------------------------------------------------------------------------------------------------
///  BaseDataFileSerializer.cpp
///  Predators
///
///  Created by Alex Koukoulas on 16/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/BaseDataFileSerializer.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PlatformMacros.h>
#include <engine/utils/StringUtils.h>
#if defined(MACOS) || defined(MOBILE_FLOW)
#include <platform_utilities/AppleUtils.h>
#elif defined(WINDOWS)
#include <platform_utilities/WindowsUtils.h>
#endif
#include <filesystem>

///------------------------------------------------------------------------------------------------

namespace serial
{

///------------------------------------------------------------------------------------------------

BaseDataFileSerializer::BaseDataFileSerializer(const std::string& fileNameWithoutExtension, const DataFileType& dataFileType, const DataFileOpeningBehavior fileOpeningBehavior, const bool forceWriteBinary /* = false */)
    : mDataFileType(dataFileType)
    , mWriteBinary(forceWriteBinary)
{
#if defined(NDEBUG)
    mWriteBinary = true;
#endif
    
    std::string dataFileExtension = mWriteBinary ? ".bin" : ".json";
    
    mFilename = fileNameWithoutExtension + dataFileExtension;
    
    if (fileOpeningBehavior == DataFileOpeningBehavior::OPEN_DATA_FILE_ON_CONSTRUCTION)
    {
        OpenDataFile();
    }
}

///------------------------------------------------------------------------------------------------

void BaseDataFileSerializer::FlushStateToFile()
{
    OpenDataFile();
    
#if defined(MACOS) || defined(MOBILE_FLOW)
    mState["device_id"] = apple_utils::GetDeviceId();
#elif defined(WINDOWS)
#endif
    
    if (mFile.is_open())
    {
        auto checksumString = "&" + std::to_string(strutils::StringId(mState.dump(4)).GetStringId());
        
        if (!mWriteBinary)
        {
            mFile << mState.dump(4);
            mFile << checksumString;
        }
        else
        {
            const auto binVec = nlohmann::json::to_ubjson(mState);
            mFile.write(reinterpret_cast<const char*>(&binVec[0]), binVec.size() * sizeof(std::uint8_t));
            mFile.write(reinterpret_cast<const char*>(&checksumString[0]), checksumString.size() * sizeof(char));
        }
        
        mFile.close();
    }
}

///------------------------------------------------------------------------------------------------

nlohmann::json& BaseDataFileSerializer::GetState()
{
    return mState;
}

///------------------------------------------------------------------------------------------------

void BaseDataFileSerializer::OpenDataFile()
{
    if (!mFile.is_open())
    {
        if (mDataFileType == DataFileType::PERSISTENCE_FILE_TYPE)
        {
    #if defined(MACOS) || defined(MOBILE_FLOW)
            auto directoryPath = apple_utils::GetPersistentDataDirectoryPath();
    #elif defined(WINDOWS)
            auto directoryPath = windows_utils::GetPersistentDataDirectoryPath();
    #endif
            
    #if defined(DESKTOP_FLOW)
            std::filesystem::create_directory(directoryPath);
    #endif
            
            if (mWriteBinary)
            {
                mFile.open(directoryPath + mFilename, std::ios::binary);
            }
            else
            {
                mFile.open(directoryPath + mFilename);
            }
        }
        else if (mDataFileType == DataFileType::ASSET_FILE_TYPE)
        {
            if (mWriteBinary)
            {
                mFile.open(resources::ResourceLoadingService::RES_DATA_ROOT + mFilename, std::ios::binary);
            }
            else
            {
                mFile.open(resources::ResourceLoadingService::RES_DATA_ROOT + mFilename);
            }
        }
    }
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
