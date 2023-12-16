///------------------------------------------------------------------------------------------------
///  BaseDataFileSerializer.cpp                                                                                        
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/12/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/utils/Logging.h>
#include <engine/utils/StringUtils.h>
#include <game/BaseDataFileSerializer.h>
#include <game/utils/PersistenceUtils.h>

//#define TEST_BINARY_FLOW

///------------------------------------------------------------------------------------------------

#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
static const std::string DATA_FILE_EXTENSION = ".json";
#else
static const std::string DATA_FILE_EXTENSION = ".bin";
#endif

///------------------------------------------------------------------------------------------------

BaseDataFileSerializer::BaseDataFileSerializer(const std::string& fileNameWithoutExtension)
    : mFilename(fileNameWithoutExtension + DATA_FILE_EXTENSION)
{
#if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
    #if defined(DESKTOP_FLOW)
        std::filesystem::create_directory(persistence_utils::GetProgressDirectoryPath());
    #endif
        mFile.open(persistence_utils::GetProgressDirectoryPath() + mFilename);
#else
    mFile.open(persistence_utils::GetProgressDirectoryPath() + mFilename, std::ios::binary);
#endif
}

///------------------------------------------------------------------------------------------------

void BaseDataFileSerializer::FlushStateToFile()
{
    if (mFile.is_open())
    {
        logging::Log(logging::LogType::INFO, "Writing game data file to %s %s", (persistence_utils::GetProgressDirectoryPath() + mFilename).c_str(), mState.dump(4).c_str());
        auto checksumString = "&" + std::to_string(strutils::StringId(mState.dump(4)).GetStringId());
        
    #if !defined(NDEBUG) && !defined(TEST_BINARY_FLOW)
        mFile << mState.dump(4);
        mFile << checksumString;
    #else
        const auto binVec = nlohmann::json::to_bson(mState);
        mFile.write(reinterpret_cast<const char*>(&binVec[0]), binVec.size() * sizeof(std::uint8_t));
        mFile.write(reinterpret_cast<const char*>(&checksumString[0]), checksumString.size() * sizeof(char));
    #endif
        mFile.close();
    }
}

///------------------------------------------------------------------------------------------------
