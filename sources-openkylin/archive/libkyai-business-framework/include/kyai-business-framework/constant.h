/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

extern const char* CONF_FILE_DEFAULT_DIR;  // 配置文件默认目录
extern const char* KEY_SERVICE_NAME;
extern const char* KEY_LOGLEVEL;

extern const char* SERVER_ADDRESS_PRE;  // GDBUS 监听地址默认前缀
extern const char* SINGLE_PRE;
extern const char* OBJECT_PATH_PRE;
extern const int TASK_QUEUE_MAX_SIZE;

// 定义具体场景服务的名称
extern const char* SERVICE_NAME_KNOWLEDGEBASE;
extern const char* SERVICE_DATA_MANAGEMENT;
extern const char* SERVICE_NAME_KNOWLEDGEBASESERVICE;
