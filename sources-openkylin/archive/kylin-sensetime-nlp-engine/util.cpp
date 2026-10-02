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

#include "util.h"
#include <climits>
#include <memory>
#include <utility>

Json::Value
sensetime_nlp_util::formatJsonFromString(const std::string &stringJson) {
    Json::Value obj;
    std::string error;
    Json::CharReaderBuilder crbuilder;
    std::unique_ptr<Json::CharReader> reader(crbuilder.newCharReader());
    reader->parse(stringJson.c_str(), stringJson.c_str() + stringJson.size(),
                  &obj, &error);
    return obj;
}

std::vector<std::string>
sensetime_nlp_util::splitDataString(const std::string &dataStr) {
    std::vector<std::string> datas;
    datas.clear();

    int oldIndex = 0;
    int newIndex = 0;
    while ((newIndex = dataStr.find_first_of("\n\n", newIndex)) !=
           std::string::npos) {
        datas.push_back(dataStr.substr(oldIndex, newIndex - oldIndex));
        newIndex += 2;
        oldIndex = newIndex;
    }
    return std::move(datas);
}

std::vector<std::string>
sensetime_nlp_util::extractJsonStrings(const std::string &input) {
    std::vector<std::string> jsonStrings;
    std::stack<size_t> bracketPositions;
    size_t startJsonPos = std::string::npos;

    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (ch == '{') {
            if (startJsonPos == std::string::npos) {
                // json开始位置
                startJsonPos = i;
            }
            bracketPositions.push(i);
        } else if (ch == '}') {
            if (bracketPositions.empty()) {
                // 无效json
                startJsonPos = std::string::npos;
                continue;
            }
            bracketPositions.pop();
            if (bracketPositions.empty()) {
                // 找到JSON结束位置
                jsonStrings.emplace_back(
                    input.substr(startJsonPos, i - startJsonPos + 1));
                startJsonPos = std::string::npos;
            }
        }
    }

    return std::move(jsonStrings);
}
