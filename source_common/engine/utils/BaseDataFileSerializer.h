///------------------------------------------------------------------------------------------------
///  BaseDataFileSerializer.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BaseDataFileSerializer_h
#define BaseDataFileSerializer_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/SerializationDefinitions.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

///------------------------------------------------------------------------------------------------

namespace serial
{

///------------------------------------------------------------------------------------------------

class BaseDataFileSerializer
{
public:
    BaseDataFileSerializer(const std::string& fileNameWithoutExtension, const DataFileType& dataFileType, const bool forceWriteBinary = false);
    virtual ~BaseDataFileSerializer() = default;
    
    void FlushStateToFile();
    
    nlohmann::json& GetState();
    
protected:
    nlohmann::json mState;
    
private:
    std::string mFilename;
    std::ofstream mFile;
    bool mWriteBinary;
};

///------------------------------------------------------------------------------------------------

};

///------------------------------------------------------------------------------------------------

#endif /* BaseDataFileSerializer_h */
