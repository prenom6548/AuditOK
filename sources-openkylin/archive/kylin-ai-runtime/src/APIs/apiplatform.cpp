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

#include "apiplatform.h"

#include <gio-unix-2.0/gio/gdesktopappinfo.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>

#include <cstdio>
#include <fstream>

#include "applicationInfos/applicationinfos.h"
#include "complexsystemapi.h"
#include "systemapidbusmap.h"
#include "utils/logger.h"

namespace {
const char *commandConfigFile = "/etc/kylin-ai/APIs/cmd.json";

std::vector<std::string> searchDesktopFiles(const std::string &searchStr) {
    std::vector<std::string> appDesktopFiles;
    char ***results = g_desktop_app_info_search(searchStr.c_str());
    char ***groups, **desktopNames;
    for (groups = results; *groups; ++groups) {
        for (desktopNames = *groups; *desktopNames; ++desktopNames) {
            if (!g_utf8_validate(*desktopNames, -1, nullptr)) {
                **desktopNames = '\0';
            }
            appDesktopFiles.push_back(*desktopNames);
        }
    }

    for (int i = 0; results[i]; i++) {
        g_strfreev(results[i]);
    }
    g_free(results);
    return appDesktopFiles;
}

}  // namespace

ApiPlatform::ApiPlatform() {
    loadSystemCommandApis();
    loadApplicationInfos();
}

ApiPlatform &ApiPlatform::instance() {
    static ApiPlatform apiPlatform;
    return apiPlatform;
}

std::vector<std::string> ApiPlatform::apiNames() const {
    std::vector<std::string> commandNames;
    for (auto &cmd : systemCommandApis_) {
        commandNames.push_back(cmd.first);
    }
    commandNames.push_back("应用控制");
    return commandNames;
}
std::unique_ptr<SystemApi> ApiPlatform::createApi(const std::string &name, const std::string &valueName,
                                                  const std::vector<ApiParam> &params) {
    if (name == "应用控制") {
        return createAppLaunchApi(valueName);
    }

    return createCommandApi(name, valueName, params);
}
std::unique_ptr<SystemApi> ApiPlatform::createCommandApi(const std::string &commandName, const std::string &valueName,
                                                         const std::vector<ApiParam> &params) {
#ifdef RUN_IN_V11_2603
    if (isDbusSystemCommand(commandName)) {
        systemapi::registerDefaultApiParamsConverters();
        return std::make_unique<DBusCommandSystemApi>(commandName, valueName);
    }
#endif
    if (!systemCommandApis_.count(commandName)) {
        return nullptr;
    }
    if (isParamsSystemCommand(commandName)) {
        return createParamsCommandApi(commandName, params);
    }
    if (isFixedValueSystemCommand(commandName, valueName)) {
        const std::string cmd = systemCommandApis_[commandName][valueName];
        return std::make_unique<CommandSystemApi>(cmd);
    }
    if (isNumericSystemCommand(commandName)) {
        auto cmdDetail = systemCommandApis_[commandName];
        return std::make_unique<NumericCommandSystemApi>(commandName, valueName, cmdDetail);
    }
    if (!systemCommandApis_[commandName].count(valueName)) {
        return nullptr;
    }
    const std::string cmd = systemCommandApis_[commandName][valueName];
    return std::make_unique<CommandSystemApi>(cmd);
}

std::unique_ptr<SystemApi> ApiPlatform::createParamsCommandApi(const std::string &commandName,
                                                               const std::vector<ApiParam> &params) {
    if (commandName == "应用商店") {
        return std::make_unique<AppStoreSystemApi>(params);
    }

    if (commandName == "闹钟设置") {
        return std::make_unique<AlarmSystemApi>(params);
    }

    LOGE("未知的动态参数命令: {}", commandName);
    return nullptr;
}

std::unique_ptr<SystemApi> ApiPlatform::createAppLaunchApi(const std::string &appName) {
    const std::string desktopFile = findDesktopFileWithName(appName);
    if (desktopFile.empty()) {
        LOGE("Failed to find desktop file with name {}", appName);
        return nullptr;
    }
    return std::make_unique<AppLaunchSystemApi>(desktopFile);
}

void ApiPlatform::loadSystemCommandApis() {
    std::ifstream jsonFile(commandConfigFile);
    if (!jsonFile.is_open()) {
        LOGE("Failed to open cmd.json");
        return;
    }

    Json::Value root;
    jsonFile >> root;

    for (auto &cmd : root) {
        const std::string cmdName = cmd["name"].asString();
        const auto cmds = cmd["cmds"];
        for (unsigned int i = 0; i < cmds.size(); i++) {
            const std::string cmdValue = cmds[i]["value"].asString();
            const std::string cmd = cmds[i]["cmd"].asString();
            systemCommandApis_[cmdName].insert({cmdValue, cmd});
        }
    }
}

void ApiPlatform::loadApplicationInfos() { appInfos_ = appinfo::getAllApplicationInfos(); }

std::string ApiPlatform::findDesktopFileWithName(const std::string &name) {
    for (const auto &appInfo : appInfos_) {
        if (appInfo.name == name) {
            return appInfo.desktopFile;
        }
    }

    std::vector<std::string> appDesktopFiles = searchDesktopFiles(name);

    return appDesktopFiles.empty() ? "" : appDesktopFiles[0];
}

bool ApiPlatform::isParamsSystemCommand(const std::string &name) {
    return std::find(paramsSystemCommandNames_.begin(), paramsSystemCommandNames_.end(), name) !=
           paramsSystemCommandNames_.end();
}

bool ApiPlatform::isNumericSystemCommand(const std::string &name) {
    return std::find(numericSystemCommandNames_.begin(), numericSystemCommandNames_.end(), name) !=
           numericSystemCommandNames_.end();
}
bool ApiPlatform::isFixedValueSystemCommand(const std::string &name, const std::string &value) {
    auto it = fixedValueSystemCommandNames_.find(name);
    if (it != fixedValueSystemCommandNames_.end()) {
        const std::vector<std::string> &values = it->second;
        return std::find(values.begin(), values.end(), value) != values.end();
    }
    return false;
}

bool ApiPlatform::isDbusSystemCommand(const std::string &name) {
    return std::find(dbusSystemCommandNames_.begin(), dbusSystemCommandNames_.end(), name) !=
           dbusSystemCommandNames_.end();
}