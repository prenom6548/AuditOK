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

#include <jsoncpp/json/json.h>

#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <variant>
#include <vector>

#include "dialoguemessage.h"
#include "utils/logger.h"

namespace TypeTrans {

// 将system_response变量转换为Json::Value的函数
Json::Value systemResponseToJsonValue(const std::variant<std::string, std::list<std::string>> &system_response) {
    Json::Value jsonResponse;
    if (std::holds_alternative<std::list<std::string>>(system_response)) {
        std::list<std::string> value = std::get<std::list<std::string>>(system_response);

        for (const auto &element : value) {
            jsonResponse.append(element.c_str());
        }

    } else if (std::holds_alternative<std::string>(system_response)) {
        std::string value = std::get<std::string>(system_response);
        jsonResponse.append(value.c_str());
    }

    return jsonResponse;
}

// 将Slot类型转换为Json::Value的函数
Json::Value slotToJsonValue(const std::vector<DialogueData::Slot> &slot) {
    Json::Value jsonSlot(Json::arrayValue);

    for (const auto &element : slot) {
        Json::Value jsonElement(Json::objectValue);

        jsonElement["name"] = element.name.c_str();
        jsonElement["isRequired"] = element.isRequired;

        Json::Value jsonValues(Json::arrayValue);
        for (const auto &value : element.values) {
            jsonValues.append(value.c_str());
        }
        jsonElement["values"] = jsonValues;

        jsonSlot.append(jsonElement);
    }
    return jsonSlot;
}

// 获取当前时间
std::string getTimeStr() {
    // 获取当前时间
    time_t currentTime = time(nullptr);
    // 将time_t类型的时间转换为本地时间结构体tm
    struct tm *localTime = localtime(&currentTime);
    // 将当前时间按照指定格式输出到字符串流中
    std::stringstream localTimeStrStream;

    localTimeStrStream << (localTime->tm_year + 1900) << "-" << (localTime->tm_mon + 1) << "-" << localTime->tm_mday
                       << " ";
    localTimeStrStream << localTime->tm_hour << ":" << localTime->tm_min << ":" << localTime->tm_sec;

    // 从字符串流中获取格式化后的字符串
    std::string localTimeStr = localTimeStrStream.str();

    return localTimeStr;
}

DialogueData::APINameSearchResult findAPIName(const std::string &searchText, const Json::Value &apiName,
                                              const std::string &attachmentType) {
    DialogueData::APINameSearchResult resultFind = {false, ""};
    std::string errorMessage{};

    // 检查 attachmentType 是否为有效的值
    if (attachmentType != "图片" && attachmentType != "文档") {
        errorMessage = "无效的 attachmentType，必须为 '图片' 或 '文档'。";
    } else if (!apiName.isMember(attachmentType)) {
        errorMessage = "API名称列表中 '" + attachmentType + "' 类别不存在。";
    } else {
        const Json::Value &apis = apiName[attachmentType];
        // 检查是否为数组类型
        if (!apis.isArray()) {
            errorMessage = "API名称列表中 '" + attachmentType + "' 类别不是数组类型。";
        } else {
            for (const auto &api : apis) {
                // 检查是否为字符串类型
                if (api.isString() && api.asString() == searchText) {
                    resultFind.found = true;
                    resultFind.category = attachmentType;
                    return resultFind;
                }
            }
        }
    }

    if (!errorMessage.empty()) {
        LOGE("查找API名称时出错: {}", errorMessage);
        resultFind.found = false;
        resultFind.category = "ERROR";
    }

    return resultFind;
}

std::list<std::string> jsonArrayToStdList(const Json::Value &jsonArray) {
    std::list<std::string> resultList;

    if (jsonArray.isArray()) {
        int count = 0;
        for (const auto &element : jsonArray) {
            if (count < DialogueConfig::numberForGuidedFunction) {
                resultList.push_back(element.asString());
                count++;
            } else {
                break;
            }
        }
    }
    return resultList;
}

std::list<std::string> findElementContainingString(const std::vector<std::string> &myList,
                                                   const std::string &targetString) {
    std::list<std::string> tempList;

    for (const auto &element : myList) {
        size_t splitIndex = element.find('(');
        if (element.find(targetString) != std::string::npos) {
            tempList.push_back(element);
        }
    }

    return tempList;
}

std::string checkList(const std::list<std::string> &myList) {
    if (myList.size() == 1 && myList.front() == "文档") {
        return "文档";
    } else if (myList.size() == 1 && myList.front() == "图片") {
        return "图片";
    } else {
        return "multiCheckFalse";
    }
    return {};
}

bool isStringInList(const std::list<std::string> &myList, const std::string &targetString) {
    for (const auto &element : myList) {
        if (element == targetString) {
            return true;
        }
    }
    return false;
}

// 将CallSuccessIdentifier结构体转换为Json::Value
Json::Value callSuccessIdentifierToJson(const DialogueData::CallSuccessIdentifier &identifier) {
    Json::Value jsonValue;

    jsonValue["successRes"] = identifier.successRes;
    jsonValue["responseContent"] = identifier.responseContent;
    jsonValue["additionalContent"] = identifier.additionalContent;
    jsonValue["slotMissing"] = identifier.slotMissing;
    jsonValue["errorCode"] = identifier.errorCode;
    jsonValue["errorMessage"] = identifier.errorMessage;

    return jsonValue;
}

bool checkString(const std::string &input) {
    if (input.find('%') != std::string::npos) {
        return true;
    }

    try {
        double num;
        std::stringstream ss(input);
        ss >> num;
        int intValue;
        std::stringstream ss2(input);
        ss2 >> intValue;

        if (num != intValue) {  // 小数
            return true;
        }
    } catch (...) {
        return false;
    }

    return false;
}

bool checkAndSetScaling(std::vector<DialogueData::Slot> &inputVector) {
    bool displayScaling = true;
    for (const auto &element : inputVector) {
        for (const auto &slotValue : element.values) {
            displayScaling = checkString(slotValue);
            if (!displayScaling) {
                break;
            }
        }
    }
    return displayScaling;
}

std::vector<std::string> readTxtToList(const std::string &readFilePath) {
    std::vector<std::string> lines;
    std::ifstream readFile(readFilePath);
    if (readFile.is_open()) {
        std::string line;
        while (std::getline(readFile, line)) {  // 逐行读取文件内容
            lines.push_back(line);              // 将每行内容添加到list中
        }
        readFile.close();
    } else {
        LOGE("无法打开文件");
    }

    return lines;
}

std::string replaceChinesePunctuation(const std::string &input) {
    std::string result = input;
    std::vector<std::pair<std::regex, std::string>> replacements = {
        {std::regex("“"), "\""}, {std::regex("”"), "\""}, {std::regex("："), ":"}, {std::regex("，"), ","},
        {std::regex("。"), "."}, {std::regex("【"), "["}, {std::regex("】"), "]"}, {std::regex("《"), "<"},
        {std::regex("》"), ">"}, {std::regex("（"), "("}, {std::regex("）"), ")"}, {std::regex("－"), "-"},
        {std::regex("～"), "~"}, {std::regex("‘"), "'"},  {std::regex("’"), "'"},  {std::regex("；"), ";"},
        {std::regex("、"), ","}, {std::regex("？"), "?"}};

    for (const auto &replacement : replacements) {
        result = std::regex_replace(result, replacement.first, replacement.second);
    }

    return result;
}

bool isStringInJsonArray(const Json::Value &array, const std::string &str) {
    if (array.isArray()) {
        for (const auto &value : array) {
            if (value.isString() && value.asString() == str) {
                return true;
            }
        }
    }
    return false;
}

bool isPureUTF8ChineseText(const std::string &str) {
    for (size_t i = 0; i < str.length();) {
        unsigned char firstByte = static_cast<unsigned char>(str[i]);
        if ((firstByte & 0xF0) == 0xE0) {  // 检查是否是 3 字节 UTF - 8 字符的首字节
            if (i + 2 >= str.length()) {
                return false;  // 长度不足，不是有效的 UTF - 8 中文字符
            }
            unsigned char secondByte = static_cast<unsigned char>(str[i + 1]);
            unsigned char thirdByte = static_cast<unsigned char>(str[i + 2]);
            if ((secondByte & 0xC0) == 0x80 && (thirdByte & 0xC0) == 0x80) {
                i += 3;  // 是有效的 UTF - 8 中文字符，跳过 3 个字节
            } else {
                return false;  // 后续字节不符合 UTF - 8 编码规则
            }
        } else {
            return false;  // 不是 UTF - 8 编码的中文字符首字节
        }
    }
    return true;
}

// 判断字符串是否为整数
bool isInteger(const std::string &str) {
    if (str.empty()) return false;
    int i = 0;
    // 处理可能的正负号
    if (str[0] == '+' || str[0] == '-') {
        if (str.length() == 1) return false;
        i = 1;
    }

    for (; i < str.length(); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

// 判断字符串是否为小数
bool isDecimal(const std::string &str) {
    if (str.empty()) return false;
    int dotCount = 0;
    int i = 0;
    // 处理可能的正负号
    if (str[0] == '+' || str[0] == '-') {
        if (str.length() == 1) return false;
        i = 1;
    }
    for (; i < str.length(); ++i) {
        if (str[i] == '.') {
            ++dotCount;
            if (dotCount > 1) return false;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return dotCount == 1;
}

// 去除字符串末尾的 % 或 Hz（不区分大小写）
std::string removeSuffix(const std::string &str) {
    std::string result = str;
    std::string lowerResult = result;
    std::transform(lowerResult.begin(), lowerResult.end(), lowerResult.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::string suffixes[] = {"%", "hz", "ms", "s", "毫秒", "秒"};
    for (const auto &suffix : suffixes) {
        if (lowerResult.length() >= suffix.length() &&
            lowerResult.compare(lowerResult.length() - suffix.length(), suffix.length(), suffix) == 0) {
            result.resize(result.length() - suffix.length());
            break;
        }
    }
    return result;
}

// 判断 slotStr 是否在对应意图 intent 的 integerScenarios 取值范围内
std::pair<std::string, std::string> isSlotInRange(
    const std::string &slotStr, const std::string &intent,
    const std::map<std::string, std::pair<int, int>> &integerScenariosScope) {
    int slotValue;
    std::istringstream iss(slotStr);

    if (!(iss >> slotValue)) {
        return {"", "无法将输入的字符串转换为有效的整数，请重新输入。"};
    }

    // 在 integerScenarios 中查找指定的意图
    auto it = integerScenariosScope.find(intent);
    if (it != integerScenariosScope.end()) {
        const auto &range = it->second;

        if (slotValue < range.first) {
            std::cout << "值 " << slotValue << " 小于范围下限，已调整为下限值 " << range.first << std::endl;
            return {std::to_string(range.first), "已对槽位进行规范化处理"};
        } else if (slotValue > range.second) {
            std::cout << "值 " << slotValue << " 大于范围上限，已调整为上限值 " << range.second << std::endl;
            return {std::to_string(range.second), "已对槽位进行规范化处理"};
        } else {
            return {slotStr, "规范的槽位"};
        }
    }
    return {"", "该场景不支持数值型槽位，请按提示重新输入"};
}

// 判断 slotStr 是否在对应意图 intent 的取值范围内
std::pair<std::string, std::string> isSlotInDecimalRange(
    const std::string &slotStr, const std::string &intent,
    const std::map<std::string, std::pair<int, int>> &decimalScenariosScope,
    const std::map<std::string, std::pair<int, int>> &integerScenariosScope) {
    double slotValue;
    std::istringstream iss(slotStr);
    if (!((iss >> slotValue) && iss.eof())) {
        return {"", "无法将输入的字符串转换为有效的小数，请重新输入。"};
    }

    // 先检查小数场景
    auto decimalIt = decimalScenariosScope.find(intent);
    if (decimalIt != decimalScenariosScope.end()) {
        const auto &range = decimalIt->second;
        if (slotValue < range.first) {
            std::string newVal = std::to_string(range.first);
            return {newVal, "值小于小数范围下限，已调整为下限值 " + newVal};
        } else if (slotValue > range.second) {
            std::string newVal = std::to_string(range.second);
            return {newVal, "值大于小数范围上限，已调整为上限值 " + newVal};
        } else {
            return {slotStr, "规范的槽位"};
        }
    }

    // 若不在小数场景范围，检查整数场景
    auto integerIt = integerScenariosScope.find(intent);
    if (integerIt != integerScenariosScope.end()) {
        const auto &range = integerIt->second;
        int intValue = static_cast<int>(std::floor(slotValue));
        if (intValue < range.first) {
            std::string newVal = std::to_string(range.first);
            return {newVal, "值转换为整数后小于整数范围下限，已调整为下限值 " + newVal};
        } else if (intValue > range.second) {
            std::string newVal = std::to_string(range.second);
            return {newVal, "值转换为整数后大于整数范围上限，已调整为上限值 " + newVal};
        } else {
            std::string intStr = std::to_string(intValue);
            return {intStr, "值已从小数转换为整数 " + intStr};
        }
    }

    return {"", "该场景不支持数值型槽位，请按提示重新输入"};
}

int countEnglishChars(const std::string &str) {
    int cnt = 0;
    for (char c : str) {
        if (isalpha(c) || isdigit(c) || ispunct(c)) {
            cnt++;
        }
    }
    return cnt;
}
}  // namespace TypeTrans
