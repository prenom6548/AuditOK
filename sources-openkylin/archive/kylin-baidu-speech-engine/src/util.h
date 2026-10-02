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

#include <jsoncpp/json/json.h>

namespace baidu_speech_util {
inline std::string jsonToString(const Json::Value &value) { return value.toStyledString(); }

inline Json::Value stringToJson(const std::string &string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return Json::nullValue;
    }
}

Json::Value formatJsonFromString(const std::string &stringJson);
std::string base64Encode(const std::string &input);
std::string base64Decode(const std::string &input);
std::string urlEncode(const std::string &input);
std::vector<std::string> segmentString(const std::string &str);
void sleepForSec(int sec);
void sleepForMs(int ms);

void replaceAll(std::string &str, const std::string &from, const std::string &to);

int64_t getTimestampMs();

std::vector<std::string> split(const std::string &str, const std::string &delim);

void parseAudioBasicContentTypeMessage(const std::string &message, std::string &format, int &rate, int &channel);

bool isAllDigits(const std::string &string);

} // namespace baidu_speech_util

#endif // UTIL_H
