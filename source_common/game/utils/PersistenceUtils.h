///------------------------------------------------------------------------------------------------
///  PersistenceUtils.h                                                                                          
///  Predators
///                                                                                                
///  Created by Alex Koukoulas on 14/10/2023
///------------------------------------------------------------------------------------------------

#ifndef PersistenceUtils_h
#define PersistenceUtils_h

///------------------------------------------------------------------------------------------------

#include <string>

///------------------------------------------------------------------------------------------------

namespace persistence_utils
{
    std::string GetProgressSaveFilePath();
    bool ProgressSaveFileExists();
    void LoadFromProgressSaveFile();
    void GenerateNewProgressSaveFile();
}

///------------------------------------------------------------------------------------------------

#endif /* PersistenceUtils_h */
