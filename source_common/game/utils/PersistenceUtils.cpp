///------------------------------------------------------------------------------------------------
///  PersistenceUtils.cpp                                                                                        
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 14/10/2023
///------------------------------------------------------------------------------------------------

#include <engine/utils/StringUtils.h>
#include <fstream>
#include <sstream>

///------------------------------------------------------------------------------------------------

namespace persistence_utils
{

///------------------------------------------------------------------------------------------------

static const char* PROGRESS_SAVE_FILE_NAME = "progress_save.json";

///------------------------------------------------------------------------------------------------

std::string GetProgressSaveFilePath()
{
 #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    auto appDataLocation = getenv("APPDATA");
#else
    auto appDataLocation = getenv("HOME");
#endif
    return std::string(appDataLocation) + "/Documents/" + PROGRESS_SAVE_FILE_NAME;
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
