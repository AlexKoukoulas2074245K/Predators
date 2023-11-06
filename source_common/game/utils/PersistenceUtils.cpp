///------------------------------------------------------------------------------------------------
///  PersistenceUtils.cpp                                                                                        
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 14/10/2023
///------------------------------------------------------------------------------------------------

#include <engine/utils/PlatformMacros.h>
#include <engine/utils/StringUtils.h>
#include <fstream>
#include <sstream>

///------------------------------------------------------------------------------------------------

namespace persistence_utils
{

///------------------------------------------------------------------------------------------------

static const char* PROGRESS_SAVE_FILE_NAME = "progress_save.json";

///------------------------------------------------------------------------------------------------

std::string GetProgressDirectoryPath()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    auto appDataLocation = getenv("APPDATA");
    return std::string(appDataLocation) + "/PredatorsSave/";
#else
    #if defined(DESKTOP_FLOW)
        return std::string(getenv("HOME")) + "/PredatorsSave/";
    #else
        return std::string(getenv("HOME")) + "/Documents/";
    #endif
#endif
}

///------------------------------------------------------------------------------------------------

std::string GetProgressSaveFilePath()
{
    return GetProgressDirectoryPath() + PROGRESS_SAVE_FILE_NAME;
}

///------------------------------------------------------------------------------------------------

bool ProgressSaveFileExists()
{
    return std::ifstream(GetProgressSaveFilePath()).good();
}

///------------------------------------------------------------------------------------------------

void LoadFromProgressSaveFile()
{
}

///------------------------------------------------------------------------------------------------

void GenerateNewProgressSaveFile()
{
}

///------------------------------------------------------------------------------------------------

}
