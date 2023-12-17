///------------------------------------------------------------------------------------------------
///  BaseDataFileDeserializer.h
///  Predators                                                                                            
///                                                                                                
///  Created by Alex Koukoulas on 27/10/2023                                                       
///------------------------------------------------------------------------------------------------

#ifndef BaseDataFileDeserializer_h
#define BaseDataFileDeserializer_h

///------------------------------------------------------------------------------------------------

#include <engine/utils/SerializationDefinitions.h>
#include <engine/utils/StringUtils.h>
#include <nlohmann/json.hpp>

///------------------------------------------------------------------------------------------------

namespace serial
{

///------------------------------------------------------------------------------------------------

class BaseDataFileDeserializer
{
public:
    BaseDataFileDeserializer(const std::string& fileNameWithoutExtension, const DataFileType& dataFileType, const bool skipCheckSumValidation = false);
    virtual ~BaseDataFileDeserializer() = default;
    
    const nlohmann::json& GetState() const;
    
protected:
    nlohmann::json mState;
};

///------------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* BaseDataFileDeserializer_h */
