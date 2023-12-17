///------------------------------------------------------------------------------------------------
///  BaseDataFileDeserializer.cpp
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#include <engine/resloading/ResourceLoadingService.h>
#include <engine/utils/BaseDataFileDeserializer.h>
#include <engine/utils/OSMessageBox.h>
#include <engine/utils/PersistenceUtils.h>
#include <engine/utils/Logging.h>
#include <engine/utils/StringUtils.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

///------------------------------------------------------------------------------------------------

namespace serial
{

///------------------------------------------------------------------------------------------------

template<class T>
bool ValidateChecksum(T& contentsContainer)
{
    std::string checkSumString;
    
    while (!contentsContainer.empty())
    {
        if (contentsContainer.back() == '&')
        {
            break;
        }
        checkSumString = char(contentsContainer.back()) + checkSumString;
        contentsContainer.pop_back();
    }
    
    contentsContainer.pop_back();
    
    if (contentsContainer.empty())
    {
        return false;
    }
    
#if !defined(NDEBUG)
    auto fileCheckSumString = std::to_string(strutils::StringId(nlohmann::json::parse(contentsContainer).dump(4)).GetStringId());
    if (checkSumString == fileCheckSumString)
    {
        return true;
    }
#else
    if (checkSumString == std::to_string(strutils::StringId(nlohmann::json::from_ubjson(contentsContainer).dump(4)).GetStringId()))
    {
        return true;
    }
#endif
    
    return false;
}

///------------------------------------------------------------------------------------------------

BaseDataFileDeserializer::BaseDataFileDeserializer(const std::string& fileNameWithoutExtension, const DataFileType& dataFileType, const bool skipCheckSumValidation /* = false */)
{
#if !defined(NDEBUG)
    std::string dataFileExtension = ".json";
#else
    std::string dataFileExtension = ".bin";
#endif
    
#if !defined(NDEBUG)
    auto filePath = (dataFileType == DataFileType::PERSISTENCE_FILE_TYPE ? persistence_utils::GetPersistentDataDirectoryPath() : resources::ResourceLoadingService::RES_DATA_ROOT) + fileNameWithoutExtension + dataFileExtension;
    std::ifstream dataFile(filePath);
    if (dataFile.is_open())
    {
        std::stringstream buffer;
        buffer << dataFile.rdbuf();
        auto contents = buffer.str();
        
        if (!skipCheckSumValidation && !ValidateChecksum(contents))
        {
            ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Corrupted file", ("Data File " + filePath + " is corrupted.").c_str());
            return;
        }
        
        if (contents.size() > 1)
        {
            mState = nlohmann::json::parse(contents);
#else
    auto filePath = (dataFileType == DataFileType::PERSISTENCE_FILE_TYPE ? persistence_utils::GetPersistentDataDirectoryPath() : resources::ResourceLoadingService::RES_DATA_ROOT) + fileNameWithoutExtension + dataFileExtension;
    logging::Log(logging::LogType::INFO, "Loading binary: %s", filePath.c_str());
    std::ifstream dataFile(filePath, std::ios::binary);
    if (dataFile.is_open())
    {
        std::vector<std::uint8_t> contents((std::istreambuf_iterator<char>(dataFile)), std::istreambuf_iterator<char>());
        
        if (!skipCheckSumValidation && !ValidateChecksum(contents))
        {
            ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "Corrupted file", ("Data File " + filePath + " is corrupted.").c_str());
            return;
        }
        
        if (contents.size() > 1)
        {
            mState = nlohmann::json::from_ubjson(contents);
#endif
        }
    }
    else
    {
        ospopups::ShowMessageBox(ospopups::MessageBoxType::ERROR, "File not found", ("Data File " + filePath + " not found.").c_str());
    }
            
    dataFile.close();
}

///------------------------------------------------------------------------------------------------

const nlohmann::json& BaseDataFileDeserializer::GetState() const
{
    return mState;
}

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------
