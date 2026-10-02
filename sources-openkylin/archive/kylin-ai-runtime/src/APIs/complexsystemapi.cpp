/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#include "complexsystemapi.h"

#include <cstdio>
#include <ctime>

#include "utils/logger.h"

AppStoreSystemApi::AppStoreSystemApi(const std::vector<ApiParam> &params) {
    for (const auto &param : params) {
        if (param.name == "桌面应用名称" && !param.values.empty()) {
            appName_ = param.values.front();
        } else if (param.name == "操作类型" && !param.values.empty()) {
            actionType_ = param.values.front();
        }
    }
    buildCommand();
}

void AppStoreSystemApi::buildCommand() {
    if (appName_.empty()) {
        LOGE("应用商店命令缺少应用名称");
        command_.clear();
        return;
    }

    std::string searchType = "Default";
    if (actionType_ == "更新") {
        searchType = "OnlyUpdate";
    } else if (actionType_ == "卸载") {
        searchType = "OnlyUninstall";
    } else if (actionType_ == "下载") {
        searchType = "Default";
    }

    std::string fullCommand = "kylin-software-center";

    // 转义应用名称中的引号
    std::string escapedAppName;
    for (char c : appName_) {
        if (c == '"') {
            escapedAppName += "\\\"";
        } else {
            escapedAppName += c;
        }
    }

    // 转义搜索类型中的引号
    std::string escapedSearchType;
    for (char c : searchType) {
        if (c == '"') {
            escapedSearchType += "\\\"";
        } else {
            escapedSearchType += c;
        }
    }

    fullCommand += " --SearchKeyword=\"" + escapedAppName + "\"" + "> /dev/null 2>&1 &";
    fullCommand += " --SearchType=\"" + escapedSearchType + "\"" + "> /dev/null 2>&1 &";
    command_ = fullCommand;
}

bool AppStoreSystemApi::execute() {
    if (command_.empty()) {
        return false;
    }
    return CommandSystemApi::execute();
}

AlarmSystemApi::AlarmSystemApi(const std::vector<ApiParam> &params)
    : hour_(0), minute_(0), remindLater_("无"), repeat_("不重复") {
    parseSlots(params);
    buildCommand();
}

void AlarmSystemApi::parseSlots(const std::vector<ApiParam> &params) {
    std::string dateStr;
    eventStr_ = "闹钟";  // 默认值

    for (const auto &param : params) {
        if (param.name == "日期" && !param.values.empty()) {
            dateStr = param.values.front();
        } else if (param.name == "事件" && !param.values.empty()) {
            eventStr_ = param.values.front();
        } else if (param.name == "操作" && !param.values.empty()) {
            operationStr_ = param.values.front();
        }
    }

    // 如果操作不是"添加"，则设置命令为空（目前只支持添加闹钟）
    if (operationStr_ != "添加" && !operationStr_.empty()) {
        LOGE("闹钟设置目前只支持添加操作，当前操作: {}", operationStr_);
        command_.clear();
        return;
    }

    if (!dateStr.empty()) {
        parseTime(dateStr);
    }
}

void AlarmSystemApi::parseTime(const std::string &dateStr) {
    // 优先处理相对时间格式："X分钟后"或"X小时后"
    int timeValue = 0;
    bool isRelativeTime = false;

    if (dateStr.find("分钟后") != std::string::npos) {
        // 尝试解析阿拉伯数字
        if (sscanf(dateStr.c_str(), "%d", &timeValue) == 1) {
            isRelativeTime = true;
            remindLater_ = dateStr;  // 已经是阿拉伯数字格式
            // 计算当前时间加上分钟数
            std::time_t now = std::time(nullptr);
            std::tm *localTime = std::localtime(&now);
            int totalMinutes = localTime->tm_hour * 60 + localTime->tm_min + timeValue;
            hour_ = (totalMinutes / 60) % 24;
            minute_ = totalMinutes % 60;
        } else {
            // 尝试解析中文数字（如"十分钟后"）
            size_t minutePos = dateStr.find("分钟后");
            if (minutePos != std::string::npos && minutePos > 0) {
                std::string numStr = dateStr.substr(0, minutePos);
                int chineseNum = chineseNumberToInt(numStr);
                if (chineseNum >= 0) {
                    isRelativeTime = true;
                    // 转换为阿拉伯数字+"分钟后"格式
                    remindLater_ = std::to_string(chineseNum) + "分钟后";
                    // 计算当前时间加上分钟数
                    std::time_t now = std::time(nullptr);
                    std::tm *localTime = std::localtime(&now);
                    int totalMinutes = localTime->tm_hour * 60 + localTime->tm_min + chineseNum;
                    hour_ = (totalMinutes / 60) % 24;
                    minute_ = totalMinutes % 60;
                }
            }
        }
    } else if (dateStr.find("小时后") != std::string::npos) {
        // 尝试解析阿拉伯数字
        if (sscanf(dateStr.c_str(), "%d", &timeValue) == 1) {
            isRelativeTime = true;
            // 转换为分钟格式（小时转分钟）
            int totalMinutes = timeValue * 60;
            remindLater_ = std::to_string(totalMinutes) + "分钟后";
            // 计算当前时间加上小时数
            std::time_t now = std::time(nullptr);
            std::tm *localTime = std::localtime(&now);
            hour_ = (localTime->tm_hour + timeValue) % 24;
            minute_ = localTime->tm_min;
        } else {
            // 尝试解析中文数字（如"两小时后"）
            size_t hourPos = dateStr.find("小时后");
            if (hourPos != std::string::npos && hourPos > 0) {
                std::string numStr = dateStr.substr(0, hourPos);
                int chineseNum = chineseNumberToInt(numStr);
                if (chineseNum >= 0) {
                    isRelativeTime = true;
                    // 转换为阿拉伯数字+"分钟后"格式（小时转分钟）
                    int totalMinutes = chineseNum * 60;
                    remindLater_ = std::to_string(totalMinutes) + "分钟后";
                    // 计算当前时间加上小时数
                    std::time_t now = std::time(nullptr);
                    std::tm *localTime = std::localtime(&now);
                    hour_ = (localTime->tm_hour + chineseNum) % 24;
                    minute_ = localTime->tm_min;
                }
            }
        }
    }

    // 如果不是相对时间，则解析绝对时间格式
    if (!isRelativeTime) {
        // 匹配 18:25 格式
        if (sscanf(dateStr.c_str(), "%d:%d", &hour_, &minute_) == 2) {
            // 成功解析
        }
        // 匹配 18点25分 或 18时25分 格式
        else if (sscanf(dateStr.c_str(), "%d点%d分", &hour_, &minute_) == 2 ||
                 sscanf(dateStr.c_str(), "%d时%d分", &hour_, &minute_) == 2) {
            // 成功解析
        }
        // 匹配 18点 格式（只有小时）
        else if (sscanf(dateStr.c_str(), "%d点", &hour_) == 1 || sscanf(dateStr.c_str(), "%d时", &hour_) == 1) {
            minute_ = 0;
        }
        // 尝试匹配中文时间格式：十点、十点二十五分等
        else {
            // 查找"点"或"时"的位置
            size_t dotPos = dateStr.find("点");
            if (dotPos == std::string::npos) {
                dotPos = dateStr.find("时");
            }
            if (dotPos != std::string::npos && dotPos > 0) {
                // 提取"点"或"时"之前的中文数字作为小时
                std::string hourStr = dateStr.substr(0, dotPos);
                int chineseHour = chineseNumberToInt(hourStr);
                if (chineseHour >= 0 && chineseHour < 24) {
                    hour_ = chineseHour;
                    // 尝试提取分钟
                    size_t minuteStart = dotPos + 3;  // "点"或"时"是3字节UTF-8字符
                    if (minuteStart < dateStr.length()) {
                        size_t minuteEnd = dateStr.find("分", minuteStart);
                        if (minuteEnd != std::string::npos) {
                            std::string minuteStr = dateStr.substr(minuteStart, minuteEnd - minuteStart);
                            int chineseMinute = chineseNumberToInt(minuteStr);
                            if (chineseMinute >= 0 && chineseMinute < 60) {
                                minute_ = chineseMinute;
                            } else {
                                minute_ = 0;
                            }
                        } else {
                            minute_ = 0;
                        }
                    } else {
                        minute_ = 0;
                    }
                }
            }
        }
    }

    // 检查是否包含重复模式关键词
    if (dateStr.find("工作日") != std::string::npos) {
        repeat_ = "工作日";
    } else if (dateStr.find("每天") != std::string::npos || dateStr.find("每日") != std::string::npos) {
        repeat_ = "每天";
    } else if (dateStr.find("周末") != std::string::npos) {
        repeat_ = "周末";
    } else if (dateStr.find("仅一次") != std::string::npos || dateStr.find("一次") != std::string::npos) {
        repeat_ = "仅一次";
    }
}

int AlarmSystemApi::chineseNumberToInt(const std::string &chineseNum) {
    std::map<std::string, int> chineseDigits = {{"零", 0}, {"一", 1}, {"二", 2}, {"三", 3}, {"四", 4},
                                                {"五", 5}, {"六", 6}, {"七", 7}, {"八", 8}, {"九", 9}};

    // 直接匹配单个数字
    auto it = chineseDigits.find(chineseNum);
    if (it != chineseDigits.end()) {
        return it->second;
    }

    // 处理"十"、"十一"到"十九"
    if (chineseNum == "十") {
        return 10;
    }
    if (chineseNum.length() >= 6 && chineseNum.substr(0, 3) == "十") {
        std::string remainder = chineseNum.substr(3);
        auto digitIt = chineseDigits.find(remainder);
        if (digitIt != chineseDigits.end()) {
            return 10 + digitIt->second;
        }
    }

    // 处理"二十"、"二十一"到"二十九"
    if (chineseNum == "二十") {
        return 20;
    }
    if (chineseNum.length() >= 6 && chineseNum.substr(0, 6) == "二十") {
        std::string remainder = chineseNum.substr(6);
        if (remainder.empty()) {
            return 20;
        }
        auto digitIt = chineseDigits.find(remainder);
        if (digitIt != chineseDigits.end()) {
            return 20 + digitIt->second;
        }
    }

    // 处理"三十"、"三十一"到"三十九"
    if (chineseNum == "三十") {
        return 30;
    }
    if (chineseNum.length() >= 6 && chineseNum.substr(0, 6) == "三十") {
        std::string remainder = chineseNum.substr(6);
        if (remainder.empty()) {
            return 30;
        }
        auto digitIt = chineseDigits.find(remainder);
        if (digitIt != chineseDigits.end()) {
            return 30 + digitIt->second;
        }
    }

    // 处理"四十"、"四十一"到"四十九"
    if (chineseNum == "四十") {
        return 40;
    }
    if (chineseNum.length() >= 6 && chineseNum.substr(0, 6) == "四十") {
        std::string remainder = chineseNum.substr(6);
        if (remainder.empty()) {
            return 40;
        }
        auto digitIt = chineseDigits.find(remainder);
        if (digitIt != chineseDigits.end()) {
            return 40 + digitIt->second;
        }
    }

    // 处理"五十"、"五十一"到"五十九"
    if (chineseNum == "五十") {
        return 50;
    }
    if (chineseNum.length() >= 6 && chineseNum.substr(0, 6) == "五十") {
        std::string remainder = chineseNum.substr(6);
        if (remainder.empty()) {
            return 50;
        }
        auto digitIt = chineseDigits.find(remainder);
        if (digitIt != chineseDigits.end()) {
            return 50 + digitIt->second;
        }
    }

    return -1;  // 无法解析
}

std::string AlarmSystemApi::escapeJsonString(const std::string &str) {
    std::string escaped;
    for (char c : str) {
        if (c == '"') {
            escaped += "\\\"";
        } else if (c == '\\') {
            escaped += "\\\\";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

std::string AlarmSystemApi::escapeShellString(const std::string &str) {
    std::string escaped;
    for (char c : str) {
        if (c == '\'') {
            escaped += "'\\''";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

void AlarmSystemApi::buildCommand() {
    // 构建 JSON 字符串
    std::string jsonStr = "{";
    jsonStr += "\"clockName\":\"" + escapeJsonString(eventStr_) + "\",";
    jsonStr += "\"hour\":" + std::to_string(hour_) + ",";
    jsonStr += "\"minute\":" + std::to_string(minute_) + ",";
    jsonStr += "\"remindLater\":\"" + escapeJsonString(remindLater_) + "\",";
    jsonStr += "\"repeat\":\"" + escapeJsonString(repeat_) + "\"";
    jsonStr += "}";

    // 转义 JSON 字符串中的单引号（因为外层使用单引号包裹）
    std::string escapedJson = escapeShellString(jsonStr);

    // 构建完整的 dbus-send 命令
    command_ =
        "dbus-send --session --print-reply "
        "--dest=org.kylin.dbus.ukuiclock "
        "/org/kylin/dbus/ukuiclock "
        "org.kylin.dbus.ukuiclock.addAlarmRpc "
        "string:'" +
        escapedJson + "'";

    LOGI("构建闹钟设置命令: {}", command_);
}

bool AlarmSystemApi::execute() {
    if (command_.empty()) {
        return false;
    }
    return CommandSystemApi::execute();
}
