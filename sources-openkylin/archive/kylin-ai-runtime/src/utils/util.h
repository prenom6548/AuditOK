/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTIL_H
#define UTIL_H

#include <glib.h>
#include <jsoncpp/json/json.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <filesystem>
#include <map>
#include <optional>

namespace util {

std::string jsonToString(const Json::Value &value);
Json::Value stringToJson(const std::string &string);
// 移除 JSON 字符串中的尾随逗号（如 ,]  ,}），便于解析 LLM 等可能输出非标准 JSON 的源
std::string removeJsonTrailingCommas(const std::string &jsonStr);
std::optional<Json::Value> stringToJson2(const std::string &string);
std::string currentDate(const std::string &format);
std::vector<uint8_t> base64Decode(const std::string &input);
std::string base64Encode(const char *data, int length);
std::string base64Encode(const std::string &input);
void removeSequenceFromString(std::string &str, const std::string &sequence);
void removeZeroWidthChar(std::string &str);
bool stringContains(const std::string &str, const std::string &sub);
const std::string readFile(const std::string &fileName);
inline auto thisThreadId() { return syscall(SYS_gettid); }
bool stringEndWith(const std::string &str, const std::string &suffix);
int stringLastIndexOf(const std::string &str, const std::string &substr, size_t from = std::string::npos);
std::string stringRight(const std::string &str, size_t length);
// 移除所有匹配的字符
void stringRemove(std::string &str, wchar_t ch);
std::vector<std::string> stringSplit(const std::string &str, const std::string &delim);
std::string stringMid(const std::string &str, size_t position, int n = -1);
std::string stringMidChinese(const std::string &str, size_t position, int n = -1);
inline std::filesystem::path homePath() { return std::filesystem::path{g_get_home_dir()}; }
bool stringStartsWith(const std::string &str, const std::string &prefix);
std::string stringReplace(const std::string &str, const std::string &find, const std::string &replace);

template <typename Key, typename Value>
bool mapContains(const std::map<Key, Value> &map, const Key &key) {
    return map.find(key) != map.end();
}

}  // namespace util

#endif