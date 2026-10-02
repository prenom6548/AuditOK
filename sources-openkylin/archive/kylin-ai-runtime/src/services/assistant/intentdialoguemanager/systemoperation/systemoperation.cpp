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

#include "systemoperation.h"

#include <iostream>

#include "assistant/message.h"
#include "utils/logger.h"
#include "utils/util.h"

namespace {
size_t getUtf8CharLength(const std::string& s, size_t pos) {
    if (pos >= s.size()) return 0;
    const unsigned char c = s[pos];
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;  // 默认按 ASCII 处理
}
}  // namespace

DialogueData::CallSuccessIdentifier SystemOperation::callSystemControlInterface(
    const DialogueData::Intent& scenarioIntent) {
    DialogueData::CallSuccessIdentifier result{false, ""};

    if (scenarioIntent.name.empty()) {
        LOGE("Error: Invalid scenarioIntent");
        return result;
    }

    size_t pos = scenarioIntent.name.find("_");
    if (pos == std::string::npos) {
        return result;
    }
    std::string cmdName = scenarioIntent.name.substr(pos + 1);

    if (cmdName.empty()) {
        LOGE("Error: Invalid scenarioIntent");
        return result;
    }

    auto cmdType = systemCommandDiscrimination(cmdName);
    LOGD("系统控制命令为：{}", cmdName);
    switch (cmdType) {
        case SystemCommandType::systemControlWithSlot: {
            result.successRes = systemControlWithSlotHandle(cmdName, scenarioIntent.slots);
            return result;
        }
        case SystemCommandType::systemControlWithoutSlot: {
            result.successRes = systemControlWithoutSlotHandle(cmdName, scenarioIntent.slots);
            return result;
        }
        case SystemCommandType::thirdPartyAppControl: {
            result.successRes = thirdPartyAppControlHandle(cmdName, scenarioIntent.slots);
            return result;
        }
        case SystemCommandType::scheduleManagement: {
            result.successRes = scheduleManagementHandle(cmdName, scenarioIntent.slots);
        }
        case SystemCommandType::localDataSearch: {
            break;
        }
        default: {
            LOGE("未识别的系统控制命令");
            break;
        }
    }
    return result;
}

void SystemOperation::callSourceUrlTool(int toolId) {
    const std::string appName = assistant::message::getAppNameFromSourceId(toolId);
    if (appName.empty()) {
        LOGE("App name is empty.");
        return;
    }
    std::string cmd = "kylin-user-guide -A " + appName + " &";
    LOGI("cmd: {}.", cmd);
    // TODO 不安全
    system(cmd.c_str());
}

DialogueData::CallSuccessIdentifier SystemOperation::callSystemScheduleManagementInterface(std::string inputContent) {
    DialogueData::CallSuccessIdentifier result;
    inputContent = formatJsonFromSchedulingInputContent(inputContent);
    Json::Value inputContentJson = util::stringToJson(inputContent);
    if (inputContentJson.isNull() || !inputContentJson.isObject() || !inputContentJson.isMember("日程主题") ||
        !inputContentJson.isMember("日程开始时间") || !inputContentJson.isMember("日程结束时间")) {
        result.successRes = false;
        LOGE("SystemScheduleManagement input content is invalid.");
        return result;
    }
    SchedulingInfo schedulingInfo = getSchedulingInfo(inputContentJson);
    if (schedulingInfo.script.empty()) {
        result.successRes = false;
        LOGE("SystemScheduleManagement input content is invalid.");
        return result;
    }
    if (!callDbusCalendar(schedulingInfo, SchedulingType::createSchedule)) {
        result.successRes = false;
        LOGE("Create Schedule failed");
        return result;
    }
    result.scheduleDate = std::to_string(schedulingInfo.startTime.year) + "-" +
                          std::to_string(schedulingInfo.startTime.month) + "-" +
                          std::to_string(schedulingInfo.startTime.day);
    LOGI("Return OpenSchedule date is{}", result.scheduleDate);
    result.successRes = true;
    return result;
}

std::string SystemOperation::formatJsonFromSchedulingInputContent(const std::string& input) {
    static const std::regex pattern(R"(^\s*```(?:json)?\s*|\s*```\s*$)");  // 清理 ``` 和 ```json
    static const std::regex leadingSpaces(R"(^\s*)");                      // 开头空白
    static const std::regex trailingWhitespace(R"(\s*$)");                 // 结尾空白

    std::string formattedContent = std::regex_replace(input, pattern, "");

    formattedContent = std::regex_replace(formattedContent, leadingSpaces, "");
    formattedContent = std::regex_replace(formattedContent, trailingWhitespace, "");

    // 补全 JSON 大括号
    if (!formattedContent.empty()) {
        if (formattedContent.back() != '}') {
            formattedContent += '}';
        }
        if (formattedContent.front() != '{') {
            formattedContent = '{' + formattedContent;
        }
    }

    return formattedContent;
}

DialogueData::CallSuccessIdentifier SystemOperation::callSystemToDoListInterface(const std::string& inputContent) {
    DialogueData::CallSuccessIdentifier result;
    std::vector<SchedulingInfo> schedulingInfos = getSchedulingInfo(inputContent);
    if (schedulingInfos.empty()) {
        result.successRes = false;
        return result;
    }
    int schedulingErrorCount = 0;
    for (auto info : schedulingInfos) {
        LOGI("To do list is {}", info.script);
        LOGI("Start time is {}年{}月{}日 {}:{}", info.startTime.year, info.startTime.month, info.startTime.day,
             info.startTime.hour, info.startTime.minute);
        LOGI("End time is {}年{}月{}日 {}:{}", info.endTime.year, info.endTime.month, info.endTime.day,
             info.endTime.hour, info.endTime.minute);
        if (!callDbusCalendar(info, SchedulingType::createSchedule)) {
            schedulingErrorCount += 1;
            LOGE("Create to-do list failed");
        }
    }
    if (schedulingErrorCount == schedulingInfos.size()) {
        result.successRes = false;
        return result;
    }
    result.scheduleDate = std::to_string(schedulingInfos.front().startTime.year) + "-" +
                          std::to_string(schedulingInfos.front().startTime.month) + "-" +
                          std::to_string(schedulingInfos.front().startTime.day);
    LOGI("Return OpenSchedule date is{}", result.scheduleDate);
    result.successRes = true;
    return result;
}

DialogueData::CallSuccessIdentifier SystemOperation::callSystemOpenScheduleInterface(const std::string& inputContent) {
    DialogueData::CallSuccessIdentifier result;
    SchedulingInfo openSchedulingTime;
    if (inputContent.empty()) {
        result.successRes = false;
        return result;
    }
    if (!parseTime(inputContent, openSchedulingTime.startTime, SchedulingType::openSchedule)) {
        result.successRes = false;
        LOGE("Invalid openSchedulingTime {}", inputContent.c_str());
        return result;
    }
    if (!callDbusCalendar(openSchedulingTime, SchedulingType::openSchedule)) {
        result.successRes = false;
        LOGE("Open Schedule failed");
        return result;
    }
    result.successRes = true;
    return result;
}

SystemCommandType SystemOperation::systemCommandDiscrimination(const std::string& name) {
    if (std::find(SystemCommandNamesWithoutSlot_.begin(), SystemCommandNamesWithoutSlot_.end(), name) !=
        SystemCommandNamesWithoutSlot_.end()) {
        return SystemCommandType::systemControlWithoutSlot;
    };

    if (std::find(SystemCommandNamesWithSlot_.begin(), SystemCommandNamesWithSlot_.end(), name) !=
        SystemCommandNamesWithSlot_.end()) {
        return SystemCommandType::systemControlWithSlot;
    };

    if (std::find(AppCommandNames_.begin(), AppCommandNames_.end(), name) != AppCommandNames_.end()) {
        return SystemCommandType::thirdPartyAppControl;
    };

    if (std::find(ScheduleManagementNames_.begin(), ScheduleManagementNames_.end(), name) !=
        ScheduleManagementNames_.end()) {
        return SystemCommandType::scheduleManagement;
    };

    if (name == "全局搜索") {
        return SystemCommandType::localDataSearch;
    }

    return SystemCommandType::typeNotRecognized;
}

bool SystemOperation::systemControlWithSlotHandle(const std::string& cmdName,
                                                  const std::vector<DialogueData::Slot>& slots) const {
    if (slots.empty()) {
        return false;
    }
    for (auto& slot : slots) {
        if (slot.values.empty()) {
            return false;
        }
        return systemControlOperation(cmdName, slot.values.front(), slots);
    }
    return false;
}

bool SystemOperation::systemControlWithoutSlotHandle(const std::string& cmdName,
                                                     const std::vector<DialogueData::Slot>& slots) const {
    std::string value = "default";
    return systemControlOperation(cmdName, value, slots);
}

bool SystemOperation::scheduleManagementHandle(const std::string& cmdName,
                                               const std::vector<DialogueData::Slot>& slots) const {
    if (cmdName == "闹钟设置") {
        return systemControlOperation("闹钟设置", "打开", slots);
    }
    return false;
}

bool SystemOperation::thirdPartyAppControlHandle(const std::string& cmdName,
                                                 const std::vector<DialogueData::Slot>& slots) const {
    if (cmdName == "应用控制") {
        return systemAppLaunchOperation(slots);
    }
    if (cmdName == "多媒体-播放器控制") {
        return systemMediaPlayerOperation(slots);
    }
    if (cmdName == "多媒体-音乐播放") {
        return systemControlOperation("音乐播放", "打开", slots);
    }
    if (cmdName == "多媒体-视频播放") {
        return systemControlOperation("视频播放", "打开", slots);
    }
    return false;
}
bool SystemOperation::systemControlOperation(const std::string& cmdName, const std::string& value,
                                             const std::vector<DialogueData::Slot>& slots) const {
    return callCreateSystemApi(cmdName, value, slots);
}
bool SystemOperation::systemAppLaunchOperation(const std::vector<DialogueData::Slot>& slots) const {
    std::string cmdName = "应用控制";

    if (slots.empty()) {
        LOGE("应用控制命令缺少参数");
        return false;
    }

    std::string actionType = getAppActionType(slots);
    std::string appName = getAppName(slots);

    if (actionType.empty() || appName.empty()) {
        return false;
    }

    if (actionType == "open_store") {
        cmdName = "应用商店";
        appName = "打开";
    }
    if (appName == "放大镜") {
        return callDbusMagnifier();
    }
    if (appName == "日历") {
        SchedulingInfo time;
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        time.startTime.year = now->tm_year + 1900;
        time.startTime.month = now->tm_mon + 1;
        time.startTime.day = now->tm_mday;
        return callDbusCalendar(time, SchedulingType::openSchedule);
    }
    if (AppNamesMap_.count(appName) > 0) {
        appName = AppNamesMap_.at(appName);
    }
    return callCreateSystemApi(cmdName, appName, slots);
}

bool SystemOperation::systemMediaPlayerOperation(const std::vector<DialogueData::Slot>& slots) const {
    if (slots.empty()) {
        LOGE("应用控制命令缺少参数");
        return false;
    }
    std::string cmdName;
    std::string value;
    for (const auto& slot : slots) {
        if (slot.values.empty()) {
            continue;
        }
        if (slot.name == "类型") {
            if (slot.values.front() == "音乐") {
                cmdName = "音乐播放器控制";
            }
        } else if (slot.name == "操作") {
            value = slot.values.front();
        }
    }
    std::vector<DialogueData::Slot> slotsToUse = slots;
    // 端侧模型在识别视频播放器媒体控制时，会将"上一集/下一集"识别为"上一首/下一首"，需转换
    if (cmdName == "视频播放器控制" && (value == "上一首" || value == "下一首")) {
        value = (value == "上一首") ? "上一集" : "下一集";
        for (auto& s : slotsToUse) {
            if (s.name == "操作" && !s.values.empty()) {
                *s.values.begin() = value;
                break;
            }
        }
    }
    return callCreateSystemApi(cmdName, value, slotsToUse);
}

std::string SystemOperation::getAppActionType(const std::vector<DialogueData::Slot>& slots) const {
    for (const auto& slot : slots) {
        if (slot.name == "操作类型" && !slot.values.empty()) {
            std::string action = slot.values.front();
            if (action == "打开") {
                return "launch";
            }
            if (action == "更新" || action == "卸载" || action == "下载") {
                return "open_store";
            }
            if (action == "关闭") {
                return "close";
            }
        }
    }
    return "";
}

std::string SystemOperation::getAppName(const std::vector<DialogueData::Slot>& slots) const {
    for (auto& slot : slots) {
        if (slot.name == "桌面应用名称") {
            if (not slot.values.empty()) {
                return slot.values.front();
            }
        }
    }
    return "";
}

bool SystemOperation::callCreateSystemApi(const std::string& cmdName, const std::string& value,
                                          const std::vector<DialogueData::Slot>& slots) const {
    std::vector<ApiParam> params;
    params.reserve(slots.size());
    for (const auto& s : slots) {
        ApiParam p;
        p.name = s.name;
        p.values.assign(s.values.begin(), s.values.end());
        params.push_back(std::move(p));
    }

    auto cmd = apiPlatform_.createApi(cmdName, value, params);
    if (!cmd) {
        LOGE("创建api失败");
        return false;
    }
    if (cmd->execute()) {
        LOGD("API命令已执行");
        return true;
    }
    LOGE("Api命令执行失败");
    return false;
}

SchedulingInfo SystemOperation::getSchedulingInfo(const Json::Value& input) {
    SchedulingInfo schedulingInfo;
    schedulingInfo.script = input["日程主题"].asString();
    if (!parseTime(input["日程开始时间"].asCString(), schedulingInfo.startTime, SchedulingType::createSchedule)) {
        LOGE("Invalid SchedulingInfo: startTime format is not correct{}", input["日程开始时间"].asCString());
        return {};
    }
    if (!parseTime(input["日程结束时间"].asCString(), schedulingInfo.endTime, SchedulingType::createSchedule)) {
        LOGE("Invalid SchedulingInfo: endTime format is not correct{}", input["日程结束时间"].asCString());
        return {};
    }
    if (schedulingInfo.startTime.year == 0 || schedulingInfo.startTime.month == 0 ||
        schedulingInfo.startTime.day == 0 || schedulingInfo.endTime.year == 0 || schedulingInfo.endTime.month == 0 ||
        schedulingInfo.endTime.day == 0) {
        LOGE("Invalid SchedulingInfo {}-{}-{} {}:{}", schedulingInfo.startTime.year, schedulingInfo.startTime.month,
             schedulingInfo.startTime.day, schedulingInfo.endTime.year, schedulingInfo.endTime.month,
             schedulingInfo.endTime.day);
        return {};
    }
    LOGI("SchedulingInfo {}", schedulingInfo.script);
    LOGI("SchedulingInfo start time is {}-{}-{} {}:{}", schedulingInfo.startTime.year, schedulingInfo.startTime.month,
         schedulingInfo.startTime.day, schedulingInfo.startTime.hour, schedulingInfo.startTime.minute);
    LOGI("SchedulingInfo end time is {}-{}-{} {}:{}", schedulingInfo.endTime.year, schedulingInfo.endTime.month,
         schedulingInfo.endTime.day, schedulingInfo.endTime.hour, schedulingInfo.endTime.minute);
    return schedulingInfo;
}

bool SystemOperation::isTaskLine(const std::string& str) { return str[0] == '#'; }

bool SystemOperation::isValidContentLine(const std::string& str) { return str != "- 无" && str[0] == '-'; }

std::vector<SchedulingInfo> SystemOperation::getSchedulingInfo(const std::string& input) {
    std::vector<SchedulingInfo> schedules;
    std::istringstream ss(input);
    std::string line, script;
    SchedulingTime startTime = {0, 0, 0, 0, 0}, endTime = {0, 0, 0, 0, 0};

    auto saveSchedule = [&]() {
        if (!script.empty() && startTime.year != 0) {
            schedules.push_back({script, startTime, endTime});
            script.clear();
            startTime = {0, 0, 0, 0, 0};
            endTime = {0, 0, 0, 0, 0};
        }
    };

    while (std::getline(ss, line)) {
        if (line.empty()) {
            saveSchedule();
        } else {
            if (isTaskLine(line)) {
                saveSchedule();
                extractDateFromTaskLine(line, startTime, endTime);
            } else {
                extractTimeAndScriptFromContentLine(line, script, startTime, endTime);
            }
        }
    }
    saveSchedule();
    return schedules;
}

void SystemOperation::extractDateFromTaskLine(const std::string& input, SchedulingTime& startTime,
                                              SchedulingTime& endTime) {
    parseTime(input.substr(2), startTime, SchedulingType::todoList);
    endTime = startTime;
}

void SystemOperation::extractTimeAndScriptFromContentLine(const std::string& input, std::string& script,
                                                          SchedulingTime& startTime, SchedulingTime& endTime) {
    if (isValidContentLine(input)) {
        std::string content = input.substr(1);
        const auto commaPos = content.find("，");
        if (commaPos != std::string::npos) {
            const auto hyphenPos = content.find('-');
            if (hyphenPos != std::string::npos && hyphenPos < commaPos) {
                parseTime(content.substr(1, hyphenPos - 1), startTime, SchedulingType::todoList);
                parseTime(content.substr(hyphenPos + 1, commaPos - hyphenPos - 1), endTime, SchedulingType::todoList);
                script += content.substr(commaPos + getUtf8CharLength(content, commaPos));
            } else {
                script += content;
            }
            LOGI("Parsed task: {}", content);
        } else {
            script += content;
            LOGI("Parsed task without time range: {}", content);
        }
    }
}

bool SystemOperation::parseTime(const std::string& input, SchedulingTime& time, const SchedulingType type) {
    std::istringstream ss(input);
    int year, month, day, hour, minute;
    if (type == SchedulingType::todoList) {
        if (sscanf(input.c_str(), "%d年%d月%d日", &year, &month, &day) == 3) {
            time.year = year;
            time.month = month;
            time.day = day;
            // 待办未指定时间采用9:00
            time.hour = 9;
            time.minute = 0;
            return true;
        }
        if (sscanf(input.c_str(), "%d:%d", &hour, &minute) == 2) {
            // 待办指定时间
            time.hour = hour;
            time.minute = minute;
            return true;
        }
        LOGE("Invalid date format in to do list: {}", input);
        return false;
    }
    if (type == SchedulingType::createSchedule) {
        if (sscanf(input.c_str(), "%d-%d-%d %d:%d", &year, &month, &day, &hour, &minute) == 5) {
            time.year = year;
            time.month = month;
            time.day = day;
            time.hour = hour;
            time.minute = minute;
            return true;
        }
        return false;
    }
    if (type == SchedulingType::openSchedule) {
        if (sscanf(input.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
            time.year = year;
            time.month = month;
            time.day = day;
            return true;
        }
        return false;
    }
    return false;
}

bool SystemOperation::callDbusCalendar(const SchedulingInfo& input, SchedulingType type) const {
    gboolean success = false;
    GError* error = nullptr;
    GDBusProxy* proxy = g_dbus_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.ukui.calendar.Notifications",
        "/org/ukui/calendar/Notifications", "org.ukui.calendar.Notifications", nullptr, &error);
    if (error != nullptr) {
        g_printerr("Error creating proxy : %s\n", error->message);
        g_error_free(error);
        return false;
    }
    if (type == SchedulingType::createSchedule) {
        GVariant* result =
            g_dbus_proxy_call_sync(proxy, "ScheduleNotification",
                                   g_variant_new("(iiiiiii&s)", input.startTime.hour, input.startTime.minute,
                                                 input.startTime.day, input.startTime.month, input.startTime.year,
                                                 input.endTime.minute, input.endTime.hour, input.script.c_str()),
                                   G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
#ifdef RUN_IN_V11_2603
        g_variant_get(result, "(b)", &success);
        if (!success) {
            g_printerr("Error calling ScheduleNotification method");
            return false;
        }
#endif
        if (error != nullptr) {
            g_printerr("Error calling method: %s\n", error->message);
            g_error_free(error);
            return false;
        }
        g_variant_unref(result);
        g_object_unref(proxy);
    }
    if (type == SchedulingType::openSchedule) {
        GVariant* result = g_dbus_proxy_call_sync(
            proxy, "OpenSchedule",
            g_variant_new("(iii)", input.startTime.day, input.startTime.month, input.startTime.year),
            G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
#ifdef RUN_IN_V11_2603
        g_variant_get(result, "(b)", &success);
        if (!success) {
            g_printerr("Error calling OpenSchedule method");
            return false;
        }
#endif
        if (error != nullptr) {
            g_printerr("Error calling method: %s\n", error->message);
            g_error_free(error);
            return false;
        }
        g_variant_unref(result);
        g_object_unref(proxy);
    }

    return true;
}

bool SystemOperation::callDbusMagnifier() const {
    GError* error = nullptr;
    GDBusProxy* proxy =
        g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, "org.ukui.magnifier",
                                      "/Magnifier", "org.ukui.magnifier", nullptr, &error);
    if (error != nullptr) {
        g_printerr("Error creating proxy : %s\n", error->message);
        g_error_free(error);
        return false;
    }

    GVariant* result = g_dbus_proxy_call_sync(proxy, "ZoomIn", nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);
    if (error != nullptr) {
        g_printerr("Error calling method: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    g_variant_unref(result);
    g_object_unref(proxy);

    return true;
}