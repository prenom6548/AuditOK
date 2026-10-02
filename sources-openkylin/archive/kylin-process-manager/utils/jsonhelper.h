#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <vector>
#include <string>
#include <unordered_set>
#include <jsoncpp/json/json.h>

namespace json_helper
{
std::vector<std::string> parse_string_vector(const Json::Value strVector);
std::string remove_chars_copy(const std::string& original, const std::unordered_set<char>& to_remove = {'=', '_', '-', '.'});
}

#endif // !JSONHELPER_H