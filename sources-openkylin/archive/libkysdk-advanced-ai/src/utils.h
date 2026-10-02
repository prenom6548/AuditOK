#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <set>
#include <string>

#include "jsoncpp/json/json.h"
namespace util {
bool isValidUTF8(const char *str);
std::string buildJsonString(const std::map<std::string, std::string> &fields);
std::string buildJsonArray(
    const std::vector<std::map<std::string, std::string>> &items);
std::string buildJsonArray(const std::vector<std::string> &items);
Json::Value stringToJson(const std::string &string);
std::string buildNestedJsonString(
    const std::map<std::string, std::string> &fields,
    const std::set<std::string> &forceStringFields = {});

}  // namespace util

#endif