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

#include "systemapidbusmap.h"

#include <map>

#include "systemapi.h"

namespace systemapi {
std::string mapPanelSizePolicyToApiParam(const std::string &value) {
    static const std::map<std::string, PanelSizePolicy> mapping = {{"小", PanelSizePolicy::Small},
                                                                   {"中", PanelSizePolicy::Medium},
                                                                   {"大", PanelSizePolicy::Large},
                                                                   {"自定义", PanelSizePolicy::Custom}};
    auto it = mapping.find(value);
    return it != mapping.end() ? std::to_string(it->second) : "";
}

std::string mapPanelLocationToApiParam(const std::string &value) {
    static const std::map<std::string, PanelLocation> mapping = {{"底部", PanelLocation::Bottom},
                                                                 {"左侧", PanelLocation::Left},
                                                                 {"顶部", PanelLocation::Top},
                                                                 {"右侧", PanelLocation::Right}};
    auto it = mapping.find(value);
    return it != mapping.end() ? std::to_string(it->second) : "";
}

std::string mapMergeIconsToApiParam(const std::string &value) {
    static const std::map<std::string, MergeIcons> mapping = {{"始终", MergeIcons::Always},
                                                              {"从不", MergeIcons::Never}};
    auto it = mapping.find(value);
    return it != mapping.end() ? std::to_string(it->second) : "";
}

std::string mapIsAutoHiddenToApiParam(const std::string &value) {
    static const std::map<std::string, bool> mapping = {{"打开", true}, {"关闭", false}};
    auto it = mapping.find(value);
    return it != mapping.end() ? std::to_string(it->second) : "";
}

std::string mapIsPanelLockedToApiParam(const std::string &value) {
    static const std::map<std::string, bool> mapping = {{"锁定", true}, {"解锁", false}};
    auto it = mapping.find(value);
    return it != mapping.end() ? std::to_string(it->second) : "";
}

std::string mapIsMultiTaskViewOnToApiParam(const std::string &value) {
    static const std::map<std::string, bool> mapping = {{"打开", true}, {"关闭", false}};
    auto it = mapping.find(value);
    return it != mapping.end() ? std::to_string(it->second) : "";
}

// 注册所有默认的映射关系
void registerDefaultApiParamsConverters() {
    DBusCommandSystemApi::registerCommandToApiMapper("设置任务栏大小", mapPanelSizePolicyToApiParam);
    DBusCommandSystemApi::registerCommandToApiMapper("设置任务栏在屏幕上的位置", mapPanelLocationToApiParam);
    DBusCommandSystemApi::registerCommandToApiMapper("合并任务栏上的图标", mapMergeIconsToApiParam);
    DBusCommandSystemApi::registerCommandToApiMapper("自动隐藏任务栏", mapIsAutoHiddenToApiParam);
    DBusCommandSystemApi::registerCommandToApiMapper("锁定任务栏", mapIsPanelLockedToApiParam);
    DBusCommandSystemApi::registerCommandToApiMapper("任务栏多任务视图启用", mapIsMultiTaskViewOnToApiParam);
}
}  // namespace systemapi