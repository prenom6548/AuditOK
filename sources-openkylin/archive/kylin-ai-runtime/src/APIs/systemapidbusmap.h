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

#ifndef SYSTEMAPIDBUSMAP_H
#define SYSTEMAPIDBUSMAP_H

#include <string>
namespace systemapi {

enum MergeIcons { Always = 0, Never };
enum PanelLocation { Bottom = 0, Left, Top, Right };
enum PanelSizePolicy { Small = 0, Medium, Large, Custom };

// 声明映射函数
std::string mapPanelSizePolicyToApiParam(const std::string &value);
std::string mapPanelLocationToApiParam(const std::string &value);
std::string mapMergeIconsToApiParam(const std::string &value);
std::string mapIsAutoHiddenToApiParam(const std::string &value);
std::string mapIsPanelLockedToApiParam(const std::string &value);
std::string mapIsMultiTaskViewOnToApiParam(const std::string &value);

// 注册所有默认的映射关系
void registerDefaultApiParamsConverters();
}  // namespace systemapi

#endif  // SYSTEMAPIDBUSMAP_H
