/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include <jsoncpp/json/json.h>

#include <string>
#include <vector>

namespace util {

std::string base64Encode(const void *data, std::size_t size);

std::string base64Encode(const std::vector<uint8_t> &input);

std::string base64Encode(const std::string &input);

std::vector<unsigned char> base64Decode(const std::string &encoded);

bool parseJsonString(const std::string &jsonStr, Json::Value &root);

bool fileExists(const std::string &filePath);

bool hasReadPermission(const std::string &filePath);

}  // namespace util

#endif  // UTIL_H
