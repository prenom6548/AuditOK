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

#include <nlohmann/json.hpp>
#include <string>

#include "errorCode.h"

class Response {
public:
    explicit Response(ERROR_CODE errorNo);
    Response(ERROR_CODE errCode, const nlohmann::json& data);
    Response(ERROR_CODE errCode, const std::string& errorMsg, const nlohmann::json& data = nlohmann::json());

    static Response success();
    static Response success(const nlohmann::json& data);

    bool isSuccess() const { return errCode == 0; }
    void add(const std::string& key, const nlohmann::json& value);
    void clear();
    void setErrCode(ERROR_CODE code) { errCode = code; }
    std::string toString() const;

    ERROR_CODE getErrCode() const { return errCode; }
    std::string getErrMsg() const { return errMsg; }
    nlohmann::json getData() const { return data; }

    // SDK 客户端会使用到的方法
    static Response parseFromGVariant(void* input);
    static Response parseFromString(const std::string& input);
    // 从data中找到key的值
    std::string getString(const std::string& key);
    int getInt(const std::string& key);

protected:
    ERROR_CODE errCode;
    std::string errMsg;
    nlohmann::json data;
};
