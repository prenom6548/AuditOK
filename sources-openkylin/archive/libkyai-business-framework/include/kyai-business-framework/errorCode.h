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

#include <string>
#include <unordered_map>

enum ERROR_CODE : int {
    // 0 - 100 公共错误码
    SYSTEM_ERR = -1,
    SUCCESS = 0,
    ASYNC_FLOW_END = 1,

    PARSE_CONFIG_FILE_FAIL = 11,
    INVALID_CONFIG = 12,
    INIT_DBUS_SERVER_FAIL = 13,
    PARSE_PARAM_FAILED = 14,
    PARSE_RESPONSE_FAILED = 15,

    CHUNK_FILE_INFO_FAILED = 50,
    EMBEDDING_FAILED = 51,
    INSERT_FILE_INFO_SQLITE_FAILED = 52,
    DELETE_FILE_INFO_SQLITE_FAILED = 53,
    UPDATE_FILE_INFO_SQLITE_FAILED = 54,
    CREATE_VECTOR_DATABASE_FAILED = 55,
    INSERT_VECTOR_DATABASE_FAILED = 56,
    DELETE_VECTOR_DATABASE_FAILED = 57,
    UPDATE_VECTOR_DATABASE_FAILED = 58,
    SEARCH_FILE_INFO_FAILED = 59,
    NLP_FAILED = 60,
    // 100 - 199 搜索场景和数据管家场景业务错误码
    DM_ERROR_UNKNOWN = 100,            // 未知错误
    DM_SESSION_ERROR = 101,            // 会话错误
    DM_ERROR_INVALID_INPUT = 102,      // 无效输入
    DM_ERROR_PERMISSION_DENIED = 103,  // 权限被拒绝

    // 200 - 299 文档问答错误码
    TEXT_CHUNKS_UNSUPPORT = 200,

    // 300 - 399 向量数据库错误码
    VECTOR_DATABASE_DATA_UNAVAILABLE = 300,

    // 400 - 1000 xxx

    ERROR_MAX = 10000,
};

// 错误码对应的中文话术
const std::unordered_map<ERROR_CODE, std::string> ERROR_MSG = {
    {SYSTEM_ERR, "系统默认错误"},
    {SUCCESS, "请求成功"},
    {ASYNC_FLOW_END, "异步流式返回结束"},

    {PARSE_CONFIG_FILE_FAIL, "解析配置文件失败"},
    {INVALID_CONFIG, "缺少必要配置"},
    {INIT_DBUS_SERVER_FAIL, "初始化GDBUS服务失败"},
    {PARSE_PARAM_FAILED, "解析参数失败"},
    {PARSE_RESPONSE_FAILED, "解析Response失败"},

    {CHUNK_FILE_INFO_FAILED, "文件内容分段失败"},
    {EMBEDDING_FAILED, "向量化失败"},
    {NLP_FAILED, "获取标签，总结失败"},
    {INSERT_FILE_INFO_SQLITE_FAILED, "数据库存入文件信息失败"},
    {DELETE_FILE_INFO_SQLITE_FAILED, "数据库删除文件信息失败"},
    {UPDATE_FILE_INFO_SQLITE_FAILED, "数据库更新文件信息失败"},
    {CREATE_VECTOR_DATABASE_FAILED, "创建向量数据库失败"},
    {INSERT_VECTOR_DATABASE_FAILED, "向量数据库存入信息失败"},
    {DELETE_VECTOR_DATABASE_FAILED, "向量数据库删除信息失败"},
    {UPDATE_VECTOR_DATABASE_FAILED, "向量数据库更新信息失败"},
    {SEARCH_FILE_INFO_FAILED, "搜索文件信息失败"},
    {TEXT_CHUNKS_UNSUPPORT, "不支持的文本长度"},
    {VECTOR_DATABASE_DATA_UNAVAILABLE, "向量数据库数据不可用，数据库可能损坏或无法访问"},
};

extern std::string message(ERROR_CODE err);
