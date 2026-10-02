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

#ifndef APIPLATFORM_H
#define APIPLATFORM_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "applicationInfos/applicationinfos.h"
#include "systemapi.h"

class ApiPlatform {
public:
    static ApiPlatform &instance();

    ApiPlatform(const ApiPlatform &) = delete;
    ApiPlatform &operator=(const ApiPlatform &) = delete;

    std::vector<std::string> apiNames() const;

    std::unique_ptr<SystemApi> createApi(const std::string &name, const std::string &valueName = "default",
                                         const std::vector<ApiParam> &params = {});

private:
    ApiPlatform();

    std::unique_ptr<SystemApi> createCommandApi(const std::string &commandName,
                                                const std::string &valueName = "default",
                                                const std::vector<ApiParam> &params = {});
    std::unique_ptr<SystemApi> createParamsCommandApi(const std::string &commandName,
                                                      const std::vector<ApiParam> &params);
    std::unique_ptr<SystemApi> createAppLaunchApi(const std::string &appName);

    void loadSystemCommandApis();
    void loadApplicationInfos();
    std::string findDesktopFileWithName(const std::string &name);

    bool isParamsSystemCommand(const std::string &name);
    bool isNumericSystemCommand(const std::string &name);
    bool isFixedValueSystemCommand(const std::string &name, const std::string &value);
    bool isDbusSystemCommand(const std::string &name);

private:
    const std::vector<std::string> paramsSystemCommandNames_;
    const std::vector<std::string> numericSystemCommandNames_{
        "设置显示器亮度",     "音量控制",         "设置显示器屏幕缩放",   "设置系统字体",
        "设置鼠标指针速度",   "设置鼠标滚轮速度", "设置鼠标光标闪烁频率", "设置鼠标双击间隔时长",
        "设置触控板指针速度",
    };
    const std::vector<std::string> dbusSystemCommandNames_{
        "设置显示器亮度", "设置系统字体",   "设置显示器屏幕缩放", "合并任务栏上的图标",   "设置任务栏在屏幕上的位置",
        "设置任务栏大小", "自动隐藏任务栏", "锁定任务栏",         "任务栏多任务视图启用", "查询天气"};
    const std::map<std::string, std::vector<std::string>> fixedValueSystemCommandNames_{
        {"音量控制", {"静音", "取消静音"}}};
    // <valueName, cmd>
    using SystemCommandDetail = std::map<std::string, std::string>;
    std::map<std::string, SystemCommandDetail> systemCommandApis_;

    appinfo::ApplicationInfos appInfos_;
};

#endif  // APIPLATFORM_H
