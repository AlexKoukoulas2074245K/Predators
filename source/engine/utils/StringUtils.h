///------------------------------------------------------------------------------------------------
///  StringUtils.h
///  Predators
///
///  Created by Alex Koukoulas on 19/09/2023.
///-----------------------------------------------------------------------------------------------

#ifndef StringUtils_h
#define StringUtils_h

///-----------------------------------------------------------------------------------------------

#include <cassert>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

///-----------------------------------------------------------------------------------------------

namespace strutils
{

///-----------------------------------------------------------------------------------------------

static std::hash<std::string> hashFunction;

///-----------------------------------------------------------------------------------------------
/// Compute a unique hash for a given string.
/// @param[in] s the input string.
/// @returns the hashed input string.
inline std::size_t GetStringHash(const std::string& s)
{
    auto result = hashFunction(s);
    assert(result > 2);
    return result;
}

///-----------------------------------------------------------------------------------------------
/// Checks whether the given string represents an integer number.
/// @param[in] s the string to check.
/// @returns whether or not the given string can be cast to an int.
inline bool StringIsInt(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

///-----------------------------------------------------------------------------------------------
/// Checks whether the given string starts with a given pattern.
/// @param[in] s the string to check.
/// @param[in] pattern the pattern to check for in string s.
/// @returns whether or not the given pattern appears at the start of string s.
inline bool StringStartsWith(const std::string& s, const std::string& pattern)
{
    if (s.size() < pattern.size()) return false;
    
    for (auto i = 0U; i < pattern.size(); ++i)
    {
        if (s[i] != pattern[i]) return false;
    }
    
    return true;
}

///-----------------------------------------------------------------------------------------------
/// Checks whether the given string contains at least one occurence of the given pattern.
/// @param[in] s the string to check.
/// @param[in] pattern the pattern to check for in string s.
/// @returns whether or not the given pattern appears at least once in s.
inline bool StringContains(const std::string& s, const std::string& pattern)
{
    if (s.size() < pattern.size()) return false;
    
    return s.find(pattern) != s.npos;
}

///-----------------------------------------------------------------------------------------------
/// Checks whether the given string ends with a given pattern.
/// @param[in] s the string to check.
/// @param[in] pattern the pattern to check for in string s.
/// @returns whether or not the given pattern appears at the end of string s.
inline bool StringEndsWith(const std::string& s, const std::string& pattern)
{
    if (s.size() < pattern.size()) return false;
    
    for (auto i = 0U; i < pattern.size(); ++i)
    {
        if (s[s.size() - pattern.size() + i] != pattern[i]) return false;
    }
    
    return true;
}

///-----------------------------------------------------------------------------------------------
/// Returns a copy of the given string in uppercase.
/// @param[in] s the input string.
/// @returns a copy of the input string transformed to uppercase.
inline std::string StringToUpper(const std::string& s)
{
    auto stringCopy = s;
    std::transform(stringCopy.begin(), stringCopy.end(), stringCopy.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(static_cast<int>(c))); });
    
    return stringCopy;
}

///-----------------------------------------------------------------------------------------------
/// Returns a copy of the given string in lowercase.
/// @param[in] s the input string.
/// @returns a copy of the input string transformed to lowercase.
inline std::string StringToLower(const std::string& s)
{
    auto stringCopy = s;
    std::transform(stringCopy.begin(), stringCopy.end(), stringCopy.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(static_cast<int>(c))); });
    
    return stringCopy;
}

///-----------------------------------------------------------------------------------------------
/// Splits the given string based on a delimiter character.
/// @param[in] s the input string.
/// @param[in] delim the delimiter character to split the original string based on.
/// @returns a vector of the original string's components split based on the delimiter provided.
inline std::vector<std::string> StringSplit(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        if (!item.empty()) elems.push_back(item);
    }
    
    return elems;
}

///-----------------------------------------------------------------------------------------------
/// Returns the formatted time string HH:MM from the given number of seconds.
/// @param[in] seconds the number of seconds to format.
/// @returns the formatted time string.
inline std::string GetHoursMinutesStringFromSeconds(const int seconds)
{
    const auto minutes = seconds / 60;
    const auto hours   = minutes / 60;
    
    const auto hoursString = std::to_string(hours);
    auto minutesString = std::to_string(minutes % 60);
    
    if (minutesString.size() == 1)
    {
        minutesString = "0" + minutesString;
    }
    
    return hoursString + ":" + minutesString;
}

///-----------------------------------------------------------------------------------------------
/// Replace all occurences of the string 'pattern' with the string 'replacement' in the input string 's'.
/// @param[in] pattern the string pattern to detect in the given input string s.
/// @param[in] replacement the string pattern to replace in the given input string s.
/// @param[out] s the input string on which the replacement will take place.
inline void StringReplaceAllOccurences(const std::string& pattern, const std::string& replacement, std::string& s)
{
    s = regex_replace(s, std::regex(pattern), replacement);
}

///-----------------------------------------------------------------------------------------------
/// Converts float to string with given precision
/// @param[in] val the float value to convert to string s.
/// @param[in] decimalPlaces of the stringified float.
/// @returns the parsed string.
inline std::string FloatToString(const float val, const int decimalPlaces)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(decimalPlaces) << val;
    return ss.str();
}

///-----------------------------------------------------------------------------------------------
/// Provides a unique identifier for a string, aimed at optimizing string comparisons
class StringId final
{
public:
    StringId()
    : mString("")
    , mStringId(0)
    {
    }
    
    explicit StringId(const std::string& str)
    : mString(str)
    , mStringId(GetStringHash(str))
    {
    }
    
    operator size_t () { return mStringId; }
    bool operator < (const StringId& rhs) { return mStringId < rhs.GetStringId(); }
    
    bool isEmpty() const { return mStringId == 0; }
    const std::string& GetString() const { return mString; }
    size_t GetStringId() const { return mStringId; }
    
    void fromAddress(const void* address)
    {
        std::stringstream ss;
        ss << address;
        mString = ss.str();
        mStringId = GetStringHash(mString);
    }
    
private:
    std::string mString;
    size_t      mStringId;
};

///-----------------------------------------------------------------------------------------------
/// Custom less operator for StringIds to be used indirectly by stl containers
inline bool operator < (const StringId& lhs, const StringId& rhs)
{
    return lhs.GetStringId() < rhs.GetStringId();
}

///-----------------------------------------------------------------------------------------------
/// Custom equality operator for StringIds to be used indirectly by stl containers
inline bool operator == (const StringId& lhs, const StringId& rhs) { return lhs.GetStringId() == rhs.GetStringId(); }

///-----------------------------------------------------------------------------------------------
/// Custom inequality operator for StringIds to be used indirectly by stl containers
inline bool operator != (const StringId& lhs, const StringId& rhs) { return lhs.GetStringId() != rhs.GetStringId(); }

///-----------------------------------------------------------------------------------------------
/// Custom StringId hasher to be used in stl containers
struct StringIdHasher
{
    std::size_t operator()(const StringId& key) const
    {
        return key.GetStringId();
    }
};

///-----------------------------------------------------------------------------------------------
/// Custom StringId comprator to be used in stl map
struct StringIdStdMapComparator
{
    bool operator()(const StringId& lhs, const StringId& rhs) const
    {
        return lhs.GetString() < rhs.GetString();
    }
};

///-----------------------------------------------------------------------------------------------

}

///------------------------------------------------------------------------------------------------

#endif /* StringUtils_h */
