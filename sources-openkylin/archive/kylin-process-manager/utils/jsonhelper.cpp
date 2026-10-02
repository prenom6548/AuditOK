#include "jsonhelper.h"

std::vector<std::string> json_helper::parse_string_vector(const Json::Value strVector)
{
        std::vector<std::string> ret;
        for (const Json::Value& str : strVector){
            ret.push_back(str.asString());
        }
        return ret;
}

std::string json_helper::remove_chars_copy(const std::string& original, const std::unordered_set<char>& toRemove)
{
    std::string result;
    for (char c : original) {
        if (toRemove.find(c) == toRemove.end()) {
            result.push_back(c);
        }
    }
    return result;
}