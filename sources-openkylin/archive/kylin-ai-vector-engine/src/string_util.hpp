// Copyright (C) 2019-2024 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#pragma once

#include <folly/Format.h>
#include <folly/String.h>
#include <string>
#include <regex>
#include <algorithm>
#include <vector>

namespace milvus::local {

namespace string_util {

inline std::string
ToLower(const std::string& str) {
    std::string lower_str;
    std::transform(str.begin(),
                   str.end(),
                   std::back_inserter(lower_str),
                   [](unsigned char c) { return std::tolower(c); });
    return lower_str;
}

inline std::string
ToUpper(const std::string& str) {
    std::string upper_str;
    std::transform(str.begin(),
                   str.end(),
                   std::back_inserter(upper_str),
                   [](unsigned char c) { return std::toupper(c); });
    return upper_str;
}

inline std::string
Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

template <class... Args>
inline std::string
SFormat(const std::string& str, Args&&... args) {
    return folly::sformat(str, args...);
}

template <class Delim, class Container>
inline std::string
Join(const Delim& delimiter, const Container& container) {
    return folly::join(delimiter, container);
}

inline bool
IsAlpha(char c) {
    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z')) {
        return false;
    }
    return true;
}

inline std::string
Base64Decode(const std::string& val) {
    const std::string base64_chars = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "+/"};

    std::string encode_str = val.substr(0, val.find_last_not_of('=') + 1);
    auto len = encode_str.size();
    auto len_ret = (len + 3) / 4 * 3;
    std::string ret;
    ret.reserve(len_ret);

    size_t pos = 0;
    size_t tmp;
    bool flag = false;
    while (pos < len) {
        tmp = base64_chars.find(val[pos + 0]) << 2;

        if (pos + 1 >= len) {
            flag = true;
            break;
        }

        tmp += ((base64_chars.find(val[pos + 1]) & 0x30) >> 4);
        ret.push_back(static_cast<char>(tmp));
        tmp = (base64_chars.find(val[pos + 1]) & 0x0f) << 4;

        if (pos + 2 >= len) {
            flag = true;
            break;
        }

        tmp += ((base64_chars.find(val[pos + 2]) & 0x3C) >> 2);
        ret.push_back(static_cast<char>(tmp));
        tmp = (base64_chars.find(val[pos + 2]) & 0x03) << 6;

        if (pos + 3 >= len) {
            flag = true;
            break;
        }

        tmp += base64_chars.find(val[pos + 3]);
        ret.push_back(static_cast<char>(tmp));
        pos += 4;
    }

    if (flag && tmp != '\0') {
        ret.push_back(static_cast<char>(tmp));
    }
    return ret;
}

}  // namespace string_util

}  // namespace milvus::local
