///------------------------------------------------------------------------------------------------
///  BaseDataFileSerializer.cpp
///  Predators
///
///  Created by Alex Koukoulas on 16/12/2023
///------------------------------------------------------------------------------------------------

#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/BaseDataFileSerializer.h>
#include <engine/utils/Logging.h>
#include <engine/utils/PersistenceUtils.h>
#include <engine/utils/StringUtils.h>
#include <filesystem>

///------------------------------------------------------------------------------------------------

namespace serial
{

///------------------------------------------------------------------------------------------------

BaseDataFileSerializer::BaseDataFileSerializer(const std::string& fileNameWithoutExtension, const DataFileType& dataFileType, const bool forceWriteBinary /* = false */)
: mWriteBinary(forceWriteBinary)
{
#if defined(NDEBUG)
    mWriteBinary = true;
#endif
    
    std::string dataFileExtension = mWriteBinary ? ".bin" : ".json";
    
    mFilename = fileNameWithoutExtension + dataFileExtension;
    
    if (dataFileType == DataFileType::PERSISTENCE_FILE_TYPE)
    {
#if defined(DESKTOP_FLOW)
        std::filesystem::create_directory(persistence_utils::GetPersistentDataDirectoryPath());
#endif
        
        if (mWriteBinary)
        {
            mFile.open(persistence_utils::GetPersistentDataDirectoryPath() + mFilename, std::ios::binary);
        }
        else
        {
            mFile.open(persistence_utils::GetPersistentDataDirectoryPath() + mFilename);
        }
    }
    else if (dataFileType == DataFileType::ASSET_FILE_TYPE)
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

///------------------------------------------------------------------------------------------------

void BaseDataFileSerializer::FlushStateToFile()
{
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
            const auto binVec = nlohmann::json::to_bson(mState);
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

}

///------------------------------------------------------------------------------------------------
