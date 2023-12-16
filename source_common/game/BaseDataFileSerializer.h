///------------------------------------------------------------------------------------------------
///  BaseDataFileSerializer.h                                                                                          
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 16/12/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BaseDataFileSerializer_h
#define BaseDataFileSerializer_h

///------------------------------------------------------------------------------------------------

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

///------------------------------------------------------------------------------------------------

class BaseDataFileSerializer
{
public:
    BaseDataFileSerializer(const std::string& fileNameWithoutExtension);
    virtual ~BaseDataFileSerializer() = default;
    
    void FlushStateToFile();
    
protected:
    nlohmann::json mState;
    
private:
    const std::string mFilename;
    std::ofstream mFile;
};

///------------------------------------------------------------------------------------------------

#endif /* BaseDataFileSerializer_h */
