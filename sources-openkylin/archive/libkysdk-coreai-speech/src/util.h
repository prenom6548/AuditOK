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

#include <string>
#include <vector>
#include <cstdint>

namespace util {

std::string base64Encode(const void *data, std::size_t size);

std::string base64Encode(const std::vector<uint8_t> &input);

std::string base64Encode(const std::string &input);

std::vector<uint8_t> base64Decode(const std::string &input);

bool stringContains(const std::string &str, const std::string &sub);

}  // namespace util

#endif  // UTIL_H
