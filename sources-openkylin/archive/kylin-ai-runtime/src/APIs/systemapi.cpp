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

#include "systemapi.h"

#include <gio/gdesktopappinfo.h>

#include <array>
#include <cstdio>

#include "utils/logger.h"

std::map<std::string, std::function<std::string(const std::string &)>> DBusCommandSystemApi::apiParamsMappers_;

CommandSystemApi::CommandSystemApi(const std::string &command) : command_(command) {}

bool CommandSystemApi::execute() {
    g_unsetenv("LD_LIBRARY_PATH");
    system(command_.c_str());
    g_setenv("LD_LIBRARY_PATH", "usr/lib/kylin-ai/depends", true);
    return true;
}

ParamsCommandSystemApi::ParamsCommandSystemApi(const std::string &command, const std::vector<std::string> &paramNames,
                                               const std::map<std::string, std::string> &paramValues)
    : CommandSystemApi(command), paramNames_(paramNames), paramValues_(paramValues) {}

bool ParamsCommandSystemApi::execute() {
    // 构建完整的命令字符串
    std::string fullCommand = command_;

    // 为每个参数名添加对应的参数值
    for (const auto &paramName : paramNames_) {
        auto it = paramValues_.find(paramName);
        if (it != paramValues_.end()) {
            std::string paramValue = it->second;

            // 转义参数值中的引号
            std::string escapedValue;
            for (char c : paramValue) {
                if (c == '"') {
                    escapedValue += "\\\"";
                } else {
                    escapedValue += c;
                }
            }

            // 使用引号包裹参数值以确保安全（符合用户示例中的格式）
            fullCommand += " --" + paramName + "=\"" + escapedValue + "\"";
        }
    }

    LOGI("执行动态参数命令: {}", fullCommand);

    // 调用父类的 execute，但需要先设置构建好的命令
    command_ = fullCommand;
    return CommandSystemApi::execute();
}

NumericCommandSystemApi::NumericCommandSystemApi(const std::string &cmdName, const std::string &valueName,
                                                 const std::map<std::string, std::string> &commandDetail)
    : cmdName_(cmdName), valueName_(valueName), commandDetail_(commandDetail) {
    command_ = commandDetail_["set"].append(value());
    LOGE("Numeric command: {}", command_);
}

NumericCommandSystemApi::NumericCommandSystemApi(const std::string &cmdName, const std::string &valueName,
                                                 const std::string &currentValue)
    : cmdName_(cmdName), valueName_(valueName), currentValue_(currentValue) {
    value_ = value();
    LOGE("Numeric command: {}", command_);
}
std::string NumericCommandSystemApi::getFontSizeValue(const std::string &valueName) const {
    static const std::vector<std::string> fontSizeLevels = {"s", "m", "l", "xl"};

    // 获取当前字体大小索引
    auto it = std::find(fontSizeLevels.begin(), fontSizeLevels.end(), currentValue_);
    if (it == fontSizeLevels.end()) {
        LOGE("Invalid current font size: {}", currentValue_);
        return "";  // 默认值
    }
    int currentIndex = std::distance(fontSizeLevels.begin(), it);
    int targetIndex = currentIndex;

    if (valueName == "最大") {
        targetIndex = fontSizeLevels.size() - 1;
    } else if (valueName == "最小") {
        targetIndex = 0;
    } else if (valueName == "调高" || valueName == "增大") {
        targetIndex = std::min(currentIndex + 1, static_cast<int>(fontSizeLevels.size()) - 1);
    } else if (valueName == "调低" || valueName == "减小") {
        targetIndex = std::max(currentIndex - 1, 0);
    } else {
        LOGE("Unknown font size command: {}", valueName);
        return "";
    }

    if (targetIndex != currentIndex) {
        LOGI("Changing font size from {} to {}", fontSizeLevels[currentIndex], fontSizeLevels[targetIndex]);
    }

    return fontSizeLevels[targetIndex];
}

std::string NumericCommandSystemApi::getScreenScaleValue(const std::string &valueName) const {
    static const std::vector<std::string> screenScaleLevels = {"1.000000", "1.250000", "1.50000"};

    // 获取当前缩放大小索引
    auto it = std::find(screenScaleLevels.begin(), screenScaleLevels.end(), currentValue_);
    if (it == screenScaleLevels.end()) {
        LOGE("Invalid current screen scale: {}", currentValue_);
        return "";  // 默认值
    }
    int currentIndex = std::distance(screenScaleLevels.begin(), it);
    int targetIndex = currentIndex;

    if (valueName == "最大") {
        targetIndex = screenScaleLevels.size() - 1;
    } else if (valueName == "最小") {
        targetIndex = 0;
    } else if (valueName == "调高" || valueName == "增大") {
        targetIndex = std::min(currentIndex + 1, static_cast<int>(screenScaleLevels.size()) - 1);
    } else if (valueName == "调低" || valueName == "减小") {
        targetIndex = std::max(currentIndex - 1, 0);
    } else {
        LOGE("Unknown screen scale command: {}", valueName);
        return "";
    }

    if (targetIndex != currentIndex) {
        LOGI("Changing screen scale from {} to {}", screenScaleLevels[currentIndex], screenScaleLevels[targetIndex]);
    }

    return screenScaleLevels[targetIndex];
}

std::string NumericCommandSystemApi::value() {
    if (isNumber(valueName_)) {
        return valueName_;
    }
    std::string type = std::get<0>(commandRangeMap_.at(cmdName_));
    double min = std::get<1>(commandRangeMap_.at(cmdName_));
    double max = std::get<2>(commandRangeMap_.at(cmdName_));
    double value;
    if (isPercentage(valueName_)) {
        value = strtod(valueName_.substr(0, valueName_.length() - 1).c_str(), nullptr) / 100;
        if (type == "percent") {
            return std::to_string(value);
        }
        value = min + value * (max - min);
    }
    if (numericWordMap_.count(valueName_)) {
#ifdef RUN_IN_V11_2603
        if (cmdName_ == "设置系统字体") {
            return getFontSizeValue(valueName_);
        }
#endif
        value = min + numericWordMap_.at(valueName_) * (max - min);
    }
    if (deltaWordMap_.count(valueName_)) {
#ifdef RUN_IN_V11_2603
        if (cmdName_ == "设置系统字体") {
            return getFontSizeValue(valueName_);
        }
        if (cmdName_ == "设置显示器屏幕缩放") {
            return getScreenScaleValue(valueName_);
        }
#endif
        double current;
#ifdef RUN_IN_V11_2603
        if (currentValue_.empty()) {
            current = strtod(currentValue().c_str(), nullptr);
        } else {
            current = strtod(currentValue_.c_str(), nullptr);
        }
#else
        current = strtod(currentValue().c_str(), nullptr);
#endif

        if ((cmdName_ == "设置鼠标光标闪烁频率" || cmdName_ == "设置鼠标双击间隔时长") &&
            (valueName_ == "加快" || valueName_ == "减慢")) {
            value = current - deltaWordMap_.at(valueName_) * (max - min);
        } else {
            value = current + deltaWordMap_.at(valueName_) * (max - min);
        }
        value = std::max(min, std::min(max, value));
    }
    if (type == "int") {
        if (cmdName_ == "设置鼠标光标闪烁频率") {
            return std::to_string(int(max - value));
        }
        return std::to_string(int(value));
    }

    return std::to_string(value);
    LOGE("Unknown numeric command value: {}", valueName_);
    return "";
}

std::string NumericCommandSystemApi::currentValue() {
    auto cmd = commandDetail_["get"];
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        LOGE("Failed to run command: {}", cmd);
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

const std::map<std::string, double> NumericCommandSystemApi::numericWordMap_ = {
    {"最大", 1}, {"最小", 0}, {"中间", 0.5}};

const std::map<std::string, double> NumericCommandSystemApi::deltaWordMap_ = {
    {"调高", 0.2}, {"调低", -0.2}, {"加快", 0.2}, {"减慢", -0.2}, {"增大", 0.1}, {"减小", -0.1}};

const std::map<std::string, std::tuple<std::string, double, double>> NumericCommandSystemApi::commandRangeMap_ = {
    {"设置显示器亮度", {"double", 0.0, 100.0}},    {"音量控制", {"int", 0, 100}},
    {"设置显示器屏幕缩放", {"percent", 1.0, 1.5}}, {"设置系统字体", {"double", 10.0, 15.0}},
    {"设置鼠标指针速度", {"double", 1.0, 10.0}},   {"设置鼠标滚轮速度", {"int", 0, 10}},
    {"设置鼠标光标闪烁频率", {"int", 100, 2500}},  {"设置鼠标双击间隔时长", {"int", 170, 1000}},
    {"设置触控板指针速度", {"int", 1, 8}}};

const std::map<std::string, GDbusInfo> DBusCommandSystemApi::dbusInterfaceMap_{
    {"设置显示器亮度",
     {"org.ukui.SettingsDaemon", "/GlobalBrightness", "org.ukui.SettingsDaemon.Brightness", "setPrimaryBrightness",
      "getPrimaryBrightness", "(u)", true}},
    {"设置系统字体",
     {"org.ukui.Framework", "/org/ukui/Framework/UI/Font", "org.ukui.Framework.UI.Font", "SetFontSizeWithString",
      "GetFontSizeWithType", "(s)", true}},
    {
        "设置显示器屏幕缩放",
        {"org.ukui.Framework", "/org/ukui/Framework/Devices/Screen", "org.ukui.Framework.Devices.Screen",
         "SetScreenScale", "GetScreenScale", "(d)", true},
    },
    {"合并任务栏上的图标",
     {"org.ukui.Framework", "/org/ukui/Framework/UI/Panel", "org.ukui.Framework.UI.Panel", "SetMergeIcon",
      "GetMergeIcons", "(i)", false}},
    {"设置任务栏在屏幕上的位置",
     {"org.ukui.Framework", "/org/ukui/Framework/UI/Panel", "org.ukui.Framework.UI.Panel", "SetPanelLocation",
      "GetPanelLocation", "(i)", false}},
    {"设置任务栏大小",
     {"org.ukui.Framework", "/org/ukui/Framework/UI/Panel", "org.ukui.Framework.UI.Panel", "SetPanelSizePolicy",
      "GetPanelSizePolicy", "(i)", false}},
    {"自动隐藏任务栏",
     {"org.ukui.Framework", "/org/ukui/Framework/UI/Panel", "org.ukui.Framework.UI.Panel", "SetPanelAutoHide",
      "GetPanelAutoHide", "(b)", false}},
    {
        "锁定任务栏",
        {"org.ukui.Framework", "/org/ukui/Framework/UI/Panel", "org.ukui.Framework.UI.Panel", "SetPanelLockState",
         "GetPanelLockState", "(b)", false},
    },
    {"任务栏多任务视图启用",
     {"org.ukui.Framework", "/org/ukui/Framework/UI/Panel", "org.ukui.Framework.UI.Panel", "SetShowTaskView",
      "GetShowTaskView", "(b)", false}},
    {"查询天气",
     {"org.kylin.weather", "/org/kylin/weather", "org.kylin.weather", "showCityWeather", "", "(s)", false}}};

AppLaunchSystemApi::AppLaunchSystemApi(const std::string &desktopFile) : desktopFile_(desktopFile) {}

bool AppLaunchSystemApi::execute() {
    GDesktopAppInfo *appInfo = g_desktop_app_info_new_from_filename(desktopFile_.c_str());
    if (!appInfo) {
        LOGE("Failed to create GDesktopAppInfo from {}", desktopFile_);
        return false;
    }
    g_app_info_launch_uris_async(G_APP_INFO(appInfo), nullptr, nullptr, nullptr, nullptr, nullptr);
    return true;
}

DBusCommandSystemApi::DBusCommandSystemApi(const std::string &command, const std::string &valueName)
    : cmdName_(command), valueName_(valueName) {
    auto iter = dbusInterfaceMap_.find(cmdName_);
    if (iter != dbusInterfaceMap_.end() && iter->second.isNumeric) {
        numericCommandSystemApi_ = std::make_unique<NumericCommandSystemApi>(
            cmdName_, valueName_, getDbusInterfaceValue(dbusInterfaceMap_.at(cmdName_)));
        value_ = numericCommandSystemApi_->getValue();
        return;
    }
    auto it = apiParamsMappers_.find(cmdName_);
    if (it != apiParamsMappers_.end()) {
        value_ = it->second(valueName_);
    } else {
        // 如果没有注册映射函数，尝试作为字符串直接使用
        value_ = valueName_;
    }
}

bool DBusCommandSystemApi::callDbusInterface(const GDbusInfo &info, const std::string &value) {
    if (value.empty() && info.valueType != "(s)") {
        LOGE("Invalid value, callDbusInterface failed.");
        return false;
    }
    const gchar *typeStr = info.valueType.c_str();
    GError *error = nullptr;
    GDBusProxy *proxy =
        g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, info.busName.c_str(),
                                      info.objectPath.c_str(), info.interfaceName.c_str(), nullptr, &error);
    if (error != nullptr) {
        g_printerr("Error creating proxy : %s\n", error->message);
        g_error_free(error);
        return false;
    }

    GVariant *param = nullptr;
    if (strcmp(typeStr, "(u)") == 0) {
        param = g_variant_new("(u)", std::stoi(value));
    }
    if (strcmp(typeStr, "(i)") == 0) {
        param = g_variant_new("(i)", std::stoi(value));
    }
    if (strcmp(typeStr, "(s)") == 0) {
        param = g_variant_new("(s)", value.c_str());
    }
    if (strcmp(typeStr, "(d)") == 0) {
        param = g_variant_new("(d)", strtod(value.c_str(), nullptr));
    }
    if (strcmp(typeStr, "(b)") == 0) {
        gboolean boolValue = (value == "true" || value == "True" || value == "TRUE" || value == "1");
        param = g_variant_new("(b)", boolValue);
    }

    GVariant *result =
        g_dbus_proxy_call_sync(proxy, info.setMethodName.c_str(), param, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (error != nullptr) {
        g_printerr("Error calling method: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    g_variant_unref(result);
    g_object_unref(proxy);

    return true;
}

std::string DBusCommandSystemApi::getDbusInterfaceValue(const GDbusInfo &info) {
    const gchar *typeStr = info.valueType.c_str();
    std::string getValue;
    GError *error = nullptr;
    GDBusProxy *proxy =
        g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, nullptr, info.busName.c_str(),
                                      info.objectPath.c_str(), info.interfaceName.c_str(), nullptr, &error);
    if (error != nullptr) {
        g_printerr("Error creating proxy : %s\n", error->message);
        g_error_free(error);
        return "";
    }

    GVariant *result =
        g_dbus_proxy_call_sync(proxy, info.getMethodName.c_str(), nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error);

    if (error != nullptr) {
        g_printerr("Error calling method: %s\n", error->message);
        g_error_free(error);
        return "";
    }

    if (strcmp(typeStr, "(u)") == 0) {
        uint32_t val;
        g_variant_get(result, "(u)", &val);
        getValue = std::to_string(val);
    } else if (strcmp(typeStr, "(i)") == 0) {
        int32_t val;
        g_variant_get(result, "(i)", &val);
        getValue = std::to_string(val);
    } else if (strcmp(typeStr, "(d)") == 0) {
        double val;
        g_variant_get(result, "(d)", &val);
        getValue = std::to_string(val);
    } else if (strcmp(typeStr, "(s)") == 0) {
        const char *val;
        g_variant_get(result, "(s)", &val);
        if (val != nullptr) {
            getValue = val;
        } else {
            LOGE("Received null string from DBus");
        }
    } else if (strcmp(typeStr, "(b)") == 0) {
        gboolean val;
        g_variant_get(result, "(b)", &val);
        getValue = val ? "true" : "false";
    }

    g_print("Get dbus interface current value is: %s\n", getValue.c_str());

    g_variant_unref(result);
    g_object_unref(proxy);

    return getValue;
}

bool DBusCommandSystemApi::execute() { return callDbusInterface(dbusInterfaceMap_.at(cmdName_), value_); }

void DBusCommandSystemApi::registerCommandToApiMapper(const std::string &cmd, ApiParamsMapper mapper) {
    apiParamsMappers_[cmd] = std::move(mapper);
}
