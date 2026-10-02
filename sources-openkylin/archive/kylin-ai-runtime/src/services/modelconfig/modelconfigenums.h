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

#ifndef SERVICES_MODEL_CONFIG_MODEL_CONFIG_ENUMS_H
#define SERVICES_MODEL_CONFIG_MODEL_CONFIG_ENUMS_H

enum class AiCapability {
    Nlp,
    Vision,
    Speech,
};

enum class DeployType {
    Unknown = -1,
    Local,
    PublicCloud,
    Custom,
    Shared,
};

enum class ModelChangeType {
    Unknown,
    DeployTypeChanged,      // 部署类型发生改变
    PriorityChanged,        // 优先级发生改变
    CurrentModelChanged,    // 当前模型发生改变
    AuthenticationChanged,  // 认证信息发生改变
};

#endif  // SERVICES_MODEL_CONFIG_MODEL_CONFIG_ENUMS_H
