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

#include "startuptools.h"

#include <jsoncpp/json/json.h>

#include <filesystem>
#include <string>
#include <vector>

#include "assistant/message.h"
#include "utils/dirinfo.h"
#include "utils/fileinfolist.h"
#include "utils/logger.h"
#include "utils/settings.h"
#include "utils/util.h"

namespace {
const std::string EXIT_KEY1 = "关闭";
const std::string EXIT_KEY2 = "退出";

namespace fs = std::filesystem;

}  // namespace

std::set<std::string> StartUpTools::brightnessUpSet_ = StartUpTools::initConfigBrightnessUpSet();
std::set<std::string> StartUpTools::brightnessDownSet_ = StartUpTools::initConfigBrightnessDownSet();
std::map<std::string, std::string> StartUpTools::playMap_ = StartUpTools::initConfigPlayMap();
std::map<std::string, std::string> StartUpTools::nmMap_ = StartUpTools::initConfigNmMap();
std::map<std::string, std::string> StartUpTools::normalMap_{};
std::map<std::string, std::string> StartUpTools::exitMap_{};
util::StringList StartUpTools::fullFillPathList_{};
util::StringList StartUpTools::androidDesktopFnList_{};

StartUpTools &StartUpTools::getInstance() {
    static StartUpTools instance{};
    return instance;
}

std::set<std::string> StartUpTools::initConfigBrightnessUpSet() {
    std::set<std::string> set{
        "调高亮度",
        "增加亮度",
        "调高屏幕亮度",
        "增加屏幕亮度",
    };
    return set;
}

std::set<std::string> StartUpTools::initConfigBrightnessDownSet() {
    std::set<std::string> set{
        "降低亮度", "调低亮度", "减小亮度", "降低屏幕亮度", "调低屏幕亮度", "减小屏幕亮度",
    };
    return set;
}

std::map<std::string, std::string> StartUpTools::initConfigPlayMap() {
    std::map<std::string, std::string> map{
        {"下一首", "next"},
        {"上一首", "previous"},
        {"快进", "seek:10000000"},
    };
    return map;
}

std::map<std::string, std::string> StartUpTools::initConfigNmMap() {
    std::map<std::string, std::string> map{
        {"打开无线", "nmcli radio wifi on"},       {"启动无线", "nmcli radio wifi on"},
        {"打开WIFI", "nmcli radio wifi on"},       {"启动WIFI", "nmcli radio wifi on"},
        {"关闭WIFI", "nmcli radio wifi off"},      {"打开WiFi", "nmcli radio wifi on"},
        {"启动WiFi", "nmcli radio wifi on"},       {"关闭WiFi", "nmcli radio wifi off"},
        {"打开wifi", "nmcli radio wifi on"},       {"启动wifi", "nmcli radio wifi on"},
        {"关闭wifi", "nmcli radio wifi off"},      {"打开无线网络", "nmcli radio wifi on"},
        {"启动无线网络", "nmcli radio wifi on"},   {"关闭无线", "nmcli radio wifi off"},
        {"关闭无线网络", "nmcli radio wifi off"},  {"打开无线局域网", "nmcli radio wifi on"},
        {"启动无线局域网", "nmcli radio wifi on"}, {"关闭无线局域网", "nmcli radio wifi off"},
    };
    return map;
}

std::map<std::string, std::string> StartUpTools::initConfigExitMap(util::Settings settings) {
    std::map<std::string, std::string> map;
    settings.beginReadArray("Applications");
    size_t count = settings.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            settings.setArrayIndex(i);
            std::string appName = settings.value("AppName");
            std::string appExec = settings.value("AppExec");

            if (util::stringContains(appExec, "\"")) {
                // 去除引号
                util::stringRemove(appExec, wchar_t('"'));
            }
            if (util::stringContains(appExec, "/")) {  // 从exe名开始位置提取
                int startIndex = util::stringLastIndexOf(appExec, "/") + 1;
                appExec = util::stringMid(appExec, startIndex);
            }
            // 排除 " " 及其后面的内容，比如 %U %F -A gui -d 400 -style=gtk2 --no-sandbox --unity-launch等
            appExec = util::stringSplit(appExec, " ").front();

            // 排除 "." 及其后面的内容，比如.sh
            appExec = util::stringSplit(appExec, ".").front();

            if (appExec == "kylin-software-center" || appExec == "/usr/bin/kylin-software-center") {
                map.insert({"应用商店", "kylin-software-center"});
                map.insert({"软件商店", "kylin-software-center"});
            }
            map.insert({"投影仪", "ukydisplayswitch"});
            map.insert({"投屏", "ukydisplayswitch"});
            map.insert({"系统投屏", "ukydisplayswitch"});

            if (appExec == "kylin-note" || appExec == "/usr/bin/kylin-note") {
                map.insert({"记事本", "kylin-note"});
                map.insert({"麒麟速记", "kylin-note"});
            }
            if (appExec == "kylin-music" || appExec == "/opt/kaiming-tools/bin/kylin-music") {
                map.insert({"音乐", "kylin-music"});
            }
            if (appExec == "kylin-video" || appExec == "/opt/kaiming-tools/bin/kylin-video") {
                map.insert({"影音", "kylin-video"});
            }
            if (appExec == "kylin-calculator" || appExec == "/opt/kaiming-tools/bin/kaiming") {
                map.insert({"计算器", "kylin-calculator"});
            }
        }
        settings.endArray();
    } else {
        settings.endArray();
    }
    return map;
}

std::map<std::string, std::string> StartUpTools::initConfigNormalMap(util::Settings settings) {
    std::map<std::string, std::string> map;
    settings.beginReadArray("Applications");
    size_t count = settings.count();
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            settings.setArrayIndex(i);
            std::string appName = settings.value("AppName");
            std::string appExec = settings.value("AppExec");
            // 去除引号
            if (util::stringContains(appExec, "\"")) {
                util::stringRemove(appExec, '"');
            }
            if (util::stringContains(appExec, "/")) {
                // 从exe名开始位置提取
                int startIndex = util::stringLastIndexOf(appExec, "/") + 1;
                appExec = util::stringMid(appExec, startIndex);
            }
            // 排除 " " 及其后面的内容，比如 %U %F -A gui -d 400 -style=gtk2 --no-sandbox --unity-launch等
            appExec = util::stringSplit(appExec, " ").front();
            // 排除 "." 及其后面的内容，比如.sh
            appExec = util::stringSplit(appExec, ".").front();

            if (appExec == "kylin-software-center" || appExec == "/usr/bin/kylin-software-center") {
                map.insert({"应用商店", "kylin-software-center"});
                map.insert({"软件商店", "kylin-software-center"});
            }
            map.insert({"投影仪", "ukydisplayswitch"});
            map.insert({"投屏", "ukydisplayswitch"});
            map.insert({"系统投屏", "ukydisplayswitch"});

            if (appExec == "kylin-note" || appExec == "/usr/bin/kylin-note") {
                map.insert({"记事本", "kylin-note"});
                map.insert({"麒麟速记", "kylin-note"});
            }

            if (appExec == "kylin-music" || appExec == "/opt/kaiming-tools/bin/kylin-music") {
                map.insert({"音乐", "kylin-music"});
            }
            if (appExec == "kylin-video" || appExec == "/opt/kaiming-tools/bin/kylin-video") {
                map.insert({"影音", "kylin-video"});
            }
            if (appExec == "kylin-calculator" || appExec == "/opt/kaiming-tools/bin/kaiming/kylin-calculator") {
                map.insert({"计算器", "kylin-calculator"});
            }
        }
        settings.endArray();
    } else {
        settings.endArray();
    }
    return map;
}

StartUpTools::StartUpTools() = default;

StartUpTools::~StartUpTools() = default;

void StartUpTools::initConfig() {
    settings_.beginWriteArray("Applications");
    getDesktopFilePath();

    for (int i = 0; i < fullFillPathList_.count(); ++i) {
        settings_.setArrayIndex(i);
        std::string filePathStr = fullFillPathList_.at(i);

        char *filepath = filePathStr.data();
        // 通过glib库函数获取Name字段，防止特殊情况（含有字段X-Ubuntu-Gettext-Domain）
        GKeyFile *keyfile = g_key_file_new();
        if (!g_key_file_load_from_file(keyfile, filepath, G_KEY_FILE_NONE, nullptr)) {
            g_key_file_free(keyfile);
            return;
        }
        char *_name = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME,
                                                   nullptr, nullptr);
        if (_name != nullptr) {
            std::string name{_name};
            settings_.setValue("AppName", name);
        }

        char *_exec = g_key_file_get_locale_string(keyfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC,
                                                   nullptr, nullptr);
        if (_exec != nullptr) {
            std::string exec{_exec};
            settings_.setValue("AppExec", exec);
        }
        g_key_file_free(keyfile);
    }
    settings_.endArray();
    normalMap_ = initConfigNormalMap(settings_);
    exitMap_ = initConfigExitMap(settings_);
}

std::string StartUpTools::systemControl(const std::string &message) {
    // 识别是否为系统控制命令
    // TODO 这个变量需要重命名
    bool flag = false;
    if (inspectCtrl(message)) {
        usleep(800 * 1000);
        flag = true;
    }

    std::string key = findNmKey(message);
    if (!key.empty()) {
        usleep(800 * 1000);
        startUp(key);
        flag = true;
    }

    OperationType operationType = isExit(message);
    std::string target = util::stringMidChinese(message, 2);
    if (operationType != OTHER) {
        key = findKey(target, operationType);
        LOGD("key {}", key);
        // 3、判断是否为可支持应用
        if (!key.empty()) {
            usleep(800 * 1000);
            LOGD("The wake word was matched successfully!---key==={}", key);
            if (key == "ukydisplayswitch") {
                // 投屏操作额外处理，无论是打开还是关闭操作，都是打开投屏页面
                startUp(key);
            } else {
                if (operationType == CLOSE) {
                    killProcess(key);
                } else if (operationType == OPEN) {
                    startUp(key);
                }
            }
            flag = true;
        }
    }
    if (!flag) {
        return std::string{};
    }
    std::string punctuation = ".,;:!？。";
    // 删除标点符号
    std::string result = message;
    for (int i = 0; i < punctuation.length(); ++i) {
        util::stringRemove(result, punctuation[i]);
    }

    std::string temp = assistant::message::createChatMessage(fmt::format("好的，已为您执行：{}", result));
    LOGD("temp {}", temp);
    return temp;
}

util::StringList StartUpTools::getDesktopFilePath() {
    fullFillPathList_.clear();
    getAndroidApp();
    recursiveSearchFile("/usr/share/applications/");
    recursiveSearchFile("/var/lib/snapd/desktop/applications/");
    recursiveSearchFile("/var/lib/flatpak/exports/share/applications/");
    recursiveSearchFile("/var/opt/kaiming/share/applications/");
    recursiveSearchFile("/opt/system/resource/applications");  // 为了麒麟速记，V11不可变系统特有目录

    fullFillPathList_.removeAll("/usr/share/applications/software-properties-livepatch.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/mate-color-select.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/blueman-adapters.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/mate-user-guide.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/nm-connection-editor.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/debian-uxterm.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/debian-xterm.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/im-config.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/fcitx.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/fcitx-configtool.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/onboard-settings.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/info.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/ukui-power-preferences.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/ukui-power-statistics.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/software-properties-drivers.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/software-properties-gtk.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/gnome-session-properties.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/org.gnome.font-viewer.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/xdiagnose.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/gnome-language-selector.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/mate-notification-properties.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/transmission-gtk.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/mpv.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/system-config-printer.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/org.gnome.DejaDup.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/yelp.desktop");
    // v10
    fullFillPathList_.removeAll("/usr/share/applications/mate-about.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/time.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/network.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/shares.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/mate-power-statistics.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/display-im6.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/display-im6.q16.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/openjdk-8-policytool.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/kylin-io-monitor.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/wps-office-uninstall.desktop");
    fullFillPathList_.removeAll("/usr/share/applications/wps-office-misc.desktop");

    util::StringList desktopList;
    for (int i = 0; i < fullFillPathList_.count(); ++i) {
        std::string filepath = fullFillPathList_.at(i);
        int list_index = util::stringLastIndexOf(filepath, "/");
        std::string desktopName = util::stringRight(filepath, filepath.length() - list_index - 1);

        if (desktopList.contains(desktopName)) {
            fullFillPathList_.removeAll(filepath);
            i--;
        } else {
            desktopList.append(desktopName);
        }
    }

    return fullFillPathList_;
}

void StartUpTools::getAndroidApp() {
    androidDesktopFnList_.clear();
    std::vector<util::StringList> androidVector;
    androidVector.clear();
    fs::path path = util::homePath() / ".local" / "share" / "applications";
    util::DirInfo dir{path};
    if (!dir.exists()) {
        return;
    }

    util::FileInfoList list = dir.entryInfoList();
    if (list.size() < 1) {
        return;
    }

    int i = 0;
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyfile = g_key_file_new();

    do {
        util::FileInfo fileInfo = list.at(i);
        if (!fileInfo.isFile()) {
            i++;
            continue;
        }
        std::string filePathStr = fileInfo.filePath();
        if (!util::stringEndWith(filePathStr, ".desktop")) {
            i++;
            continue;
        }
        char *filepath = filePathStr.data();
        // 判断文件是否可读
        if (0 != access(filepath, R_OK)) {  // 判断文件是否可读
            i++;
            continue;
        }

        keyfile_ = g_key_file_new();

        if (!g_key_file_load_from_file(keyfile_, filepath, flags_, error_)) {
            return;
        }

        char *ret_1 = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "NoDisplay", nullptr, nullptr);

        if (ret_1 != nullptr) {
            std::string str{ret_1};
            g_free(ret_1);
            if (util::stringContains(str, "true")) {
                g_key_file_free(keyfile_);
                i++;
                continue;
            }
        }

        char *ret_2 = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "NotShowIn", nullptr, nullptr);

        if (ret_2 != nullptr) {
            std::string str{ret_2};
            g_free(ret_2);
            if (util::stringContains(str, "UKUI")) {
                g_key_file_free(keyfile_);
                i++;
                continue;
            }
        }

        // 过滤LXQt、KDE
        char *ret = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "OnlyShowIn", nullptr, nullptr);

        if (ret != nullptr) {
            std::string str{ret};
            g_free(ret);
            if (util::stringContains(str, "LXQt") || util::stringContains(str, "KDE")) {
                g_key_file_free(keyfile_);
                i++;
                continue;
            }
        }
        g_key_file_free(keyfile_);
        fullFillPathList_.append(filePathStr);
        androidDesktopFnList_.append(fileInfo.fileName());
        i++;
    } while (i < list.size());
    g_key_file_free(keyfile);
}

void StartUpTools::recursiveSearchFile(const std::string &_filePath) {
    util::DirInfo dir{_filePath};
    if (!dir.exists()) {
        return;
    }
    util::FileInfoList list = dir.entryInfoList();
    list.removeAll(util::FileInfo{"/usr/share/applications/screensavers"});

    if (list.size() < 1) {
        return;
    }
    int i = 0;

    // 递归算法的核心部分
    do {
        util::FileInfo fileInfo = list.at(i);
        // LOGD("fileInfo {}", fileInfo.filePath());
        // 如果是文件夹，递归
        if (bool isDir = fileInfo.isDir()) {
            recursiveSearchFile(fileInfo.filePath());
        } else {
            // 过滤后缀不是.desktop的文件
            std::string filePathStr = fileInfo.filePath();
            if (!util::stringEndWith(filePathStr, ".desktop")) {
                i++;
                continue;
            }
            char *filepath = filePathStr.data();
            // 判断文件是否可读
            if (0 != access(filepath, R_OK)) {
                i++;
                continue;
            }

            keyfile_ = g_key_file_new();

            if (!g_key_file_load_from_file(keyfile_, filepath, flags_, error_)) {
                g_key_file_free(keyfile_);
                i++;
                continue;
            }

            char *ret_0 = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "Categories", nullptr, nullptr);

            if (ret_0 != nullptr) {
                std::string str{ret_0};
                g_free(ret_0);
                if (util::stringContains(str, "Android")) {
                    g_key_file_free(keyfile_);
                    i++;
                    continue;
                }
            }

            char *ret_1 = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "NoDisplay", nullptr, nullptr);

            if (ret_1 != nullptr) {
                std::string str{ret_1};
                g_free(ret_1);
                if (util::stringContains(str, "true")) {
                    g_key_file_free(keyfile_);
                    i++;
                    continue;
                }
            }

            char *ret_2 = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "NotShowIn", nullptr, nullptr);

            if (ret_2 != nullptr) {
                std::string str{ret_2};
                g_free(ret_2);
                if (util::stringContains(str, "UKUI")) {
                    g_key_file_free(keyfile_);
                    i++;
                    continue;
                }
            }

            // 过滤LXQt、KDE
            char *ret = g_key_file_get_locale_string(keyfile_, "Desktop Entry", "OnlyShowIn", nullptr, nullptr);

            if (ret != nullptr) {
                std::string str{ret};
                g_free(ret);
                if (util::stringContains(str, "LXQt") || util::stringContains(str, "KDE")) {
                    g_key_file_free(keyfile_);
                    i++;
                    continue;
                }
            }

            g_key_file_free(keyfile_);
            fullFillPathList_.append(filePathStr);
        }
        i++;
    } while (i < list.size());
}

bool StartUpTools::inspectCtrl(const std::string &key) {
    std::string keyCopy = key;
    std::string punctuation = ".,;:!？。";

    // 删除标点符号
    for (int i = 0; i < punctuation.length(); ++i) {
        util::stringRemove(keyCopy, punctuation[i]);
    }

    if (brightnessUpSet_.count(keyCopy) > 0) {
        ctrl_.setBrightness(true);
        return true;
    }

    if (brightnessDownSet_.count(keyCopy) > 0) {
        ctrl_.setBrightness(false);
        return true;
    }

    if (playMap_.find(keyCopy) != playMap_.end()) {
        std::string funcName = playMap_[keyCopy];
        if (!funcName.empty()) {
            ctrl_.controlMprisPlayerDbus(funcName);
            return true;
        }
    }

    if (keyCopy == "打开蓝牙") {
        ctrl_.setDefaultAdapterSwitchStatus(true);
        return true;
    }
    if (keyCopy == "关闭蓝牙") {
        ctrl_.setDefaultAdapterSwitchStatus(false);
        return true;
    }
    return false;
}

std::string StartUpTools::findNmKey(const std::string &key) {
    std::string keyCopy = key;
    std::string punctuation = ".,;:!？。";
    for (int i = 0; i < punctuation.length(); ++i) {
        // 删除标点符号
        util::stringRemove(keyCopy, punctuation[i]);
    }

    if (nmMap_.find(keyCopy) == nmMap_.end()) {
        return std::string{};
    }
    std::string path = nmMap_[keyCopy];

    return path;
}

void StartUpTools::startUp(const std::string &path) {
    std::string command = path + " &";
    LOGI("Start pulling up the app: {}", command);
    system(command.c_str());
}

OperationType StartUpTools::isExit(const std::string &key) {
    if (util::stringStartsWith(key, EXIT_KEY1) || util::stringStartsWith(key, EXIT_KEY2)) {
        return CLOSE;
    } else if (util::stringStartsWith(key, "打开") || util::stringStartsWith(key, "启动") ||
               util::stringStartsWith(key, "唤醒") || util::stringStartsWith(key, "开启")) {
        return OPEN;
    } else {
        return OTHER;
    }
}

std::string StartUpTools::findKey(const std::string &key, OperationType type) {
    std::string keyCopy = key;
    std::string punctuation = ".,;:!？。";
    // 删除标点符号
    for (int i = 0; i < punctuation.length(); ++i) {
        util::stringRemove(keyCopy, punctuation[i]);
    }

    std::string path;
    if (type == OPEN) {
        if (normalMap_.find(keyCopy) == normalMap_.end()) {
            return std::string{};
        }
        path = normalMap_[keyCopy];
    } else if (type == CLOSE) {
        if (exitMap_.find(keyCopy) == exitMap_.end()) {
            return std::string{};
        }
        path = exitMap_[keyCopy];
    }

    return path;
}

void StartUpTools::killProcess(const std::string &key) {
    std::string command = "killall " + key;
    LOGI("Closing the app: {}", command);
    system(command.c_str());
}
