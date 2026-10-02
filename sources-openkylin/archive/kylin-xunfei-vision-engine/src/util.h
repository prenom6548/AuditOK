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

#include <unordered_map>
#include <jsoncpp/json/json.h>

namespace xunfei_vision_util {

Json::Value formatJsonFromString(const std::string &stringJson);
std::string base64Encode(const std::string &input);
std::string base64Decode(const std::string &input);
std::string urlEncode(const std::string &input);
std::unordered_map<std::string, std::string> parseURL(const std::string &url);
std::string hmacSha256Encode(const std::string &data, const std::string &key);
std::string hmacSha1Encode(const std::string &data, const std::string &key);
std::string getGmtTime();

} // namespace xunfei_vision_util

#endif // UTIL_H
