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

const char* CONF_FILE_DEFAULT_DIR = "/usr/share/kylin-ai/kyai-business-framework/";  // 配置文件默认目录
const char* KEY_SERVICE_NAME = "service_name";
const char* KEY_LOGLEVEL = "log_level";

const char* SERVER_ADDRESS_PRE = "unix:abstract=/tmp/.kylin-ai-business-unix/";  // GDBUS 监听地址默认前缀
const char* OBJECT_PATH_PRE = "/com/kylin/AiBusiness/";
const char* SINGLE_PRE = "handle-";
extern const int TASK_QUEUE_MAX_SIZE = 50;  // 不加 extern 会报未定义

// 定义具体场景服务的名称
const char* SERVICE_NAME_KNOWLEDGEBASE = "Knowledgebase";
const char* SERVICE_DATA_MANAGEMENT = "DataManagement";
const char* SERVICE_NAME_KNOWLEDGEBASESERVICE = "KnowledgeBaseService";
