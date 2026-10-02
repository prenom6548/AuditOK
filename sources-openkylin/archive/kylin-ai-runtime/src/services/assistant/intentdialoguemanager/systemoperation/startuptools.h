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

#ifndef SERVICES_ASSISTANT_START_UP_TOOLS_H
#define SERVICES_ASSISTANT_START_UP_TOOLS_H

#include <glib.h>
#include <jsoncpp/json/json.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ctrl.h"
#include "utils/settings.h"
#include "utils/stringlist.h"

enum OperationType { CLOSE = 0, OPEN, OTHER };

class StartUpTools {
public:
    static StartUpTools &getInstance();

    static std::set<std::string> initConfigBrightnessUpSet();
    static std::set<std::string> initConfigBrightnessDownSet();
    static std::map<std::string, std::string> initConfigNmMap();
    static std::map<std::string, std::string> initConfigPlayMap();
    static std::map<std::string, std::string> initConfigExitMap(util::Settings settings);
    static std::map<std::string, std::string> initConfigNormalMap(util::Settings settings);

public:
    StartUpTools();

    ~StartUpTools();

    void initConfig();

    std::string systemControl(const std::string &message);

private:
    // 获取系统desktop文件路径
    util::StringList getDesktopFilePath();

    // 获取安卓应用
    void getAndroidApp();

    // 遍历/usr/share/applications/文件夹
    void recursiveSearchFile(const std::string &_filePath);

    bool inspectCtrl(const std::string &key);

    std::string findNmKey(const std::string &key);

    void startUp(const std::string &path);

    OperationType isExit(const std::string &key);

    std::string findKey(const std::string &key, OperationType type);

    void killProcess(const std::string &key);

public:
    static std::set<std::string> brightnessUpSet_;
    static std::set<std::string> brightnessDownSet_;
    static std::map<std::string, std::string> playMap_;
    static std::map<std::string, std::string> nmMap_;
    static std::map<std::string, std::string> normalMap_;
    static std::map<std::string, std::string> exitMap_;
    static util::StringList androidDesktopFnList_;
    static util::StringList fullFillPathList_;

private:
    Ctrl ctrl_;
    util::Settings settings_;

    GError **error_{nullptr};
    GKeyFileFlags flags_ = G_KEY_FILE_NONE;
    GKeyFile *keyfile_{nullptr};
};

#endif
