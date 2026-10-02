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

#include "response.h"

#include <glib.h>

#include "log.h"

const char* BUSINESS_RESPONSE_ERROR_NO = "err_no";
const char* BUSINESS_RESPONSE_ERROR_MSG = "err_msg";
const char* BUSINESS_RESPONSE_DATA = "data";

Response::Response(ERROR_CODE errCode) : errCode(errCode) {
    errMsg = message(errCode);
    data = nlohmann::json();
}

Response::Response(ERROR_CODE errCode, const nlohmann::json& data) : errCode(errCode), data(data) {
    errMsg = message(errCode);
}

Response::Response(ERROR_CODE errCode, const std::string& errMsg, const nlohmann::json& data)
    : errCode(errCode), errMsg(errMsg), data(data) {}

Response Response::success() { return Response(SUCCESS); }

Response Response::success(const nlohmann::json& data) { return Response(SUCCESS, data); }

void Response::add(const std::string& key, const nlohmann::json& value) { data[key] = value; }

/**
 * 将response转换为字符串，做为返回结果
 * @return
 */
std::string Response::toString() const {
    nlohmann::json output;
    output[BUSINESS_RESPONSE_ERROR_NO] = errCode;
    output[BUSINESS_RESPONSE_ERROR_MSG] = errMsg;
    if (!data.is_null()) {
        output[BUSINESS_RESPONSE_DATA] = data;
    }
    return output.dump();
}

void Response::clear() {
    errCode = SUCCESS;
    errMsg = message(SUCCESS);
    data.clear();
}

/**
 * 异步流式返回的结构是 GVariant，将GVariant转换为Response统一处理
 * @param input
 * @return
 */
Response Response::parseFromGVariant(void* input) {
    auto* param = static_cast<GVariant*>(input);
    char* result = nullptr;
    g_variant_get(param, "(s)", &result);
    PRINT_INFO("Parse from GVariant result: %s\n", result);

    nlohmann::json root = nlohmann::json::parse(result, nullptr, false);
    if (root.is_discarded()) {
        PRINT_ERROR("Parse from GVariant failed, input: %s\n", result);
        g_free(result);
        return Response(PARSE_RESPONSE_FAILED);
    }

    PRINT_INFO("Parse from GVariant after json parse: %s\n", root.dump().c_str());
    g_free(result);

    if (!root.contains(BUSINESS_RESPONSE_ERROR_NO) || !root.contains(BUSINESS_RESPONSE_ERROR_MSG)) {
        PRINT_ERROR("Parse from GVariant not contain err_no or err_msg. %s\n", root.dump().c_str());
        return Response(PARSE_RESPONSE_FAILED);
    }

    if (root.contains(BUSINESS_RESPONSE_DATA)) {
        return Response(static_cast<ERROR_CODE>(root[BUSINESS_RESPONSE_ERROR_NO].get<int>()),
                        root[BUSINESS_RESPONSE_ERROR_MSG], root[BUSINESS_RESPONSE_DATA]);
    }

    return Response(static_cast<ERROR_CODE>(root[BUSINESS_RESPONSE_ERROR_NO].get<int>()),
                    root[BUSINESS_RESPONSE_ERROR_MSG]);
}

Response Response::parseFromString(const std::string& input) {
    nlohmann::json jsonValue = nlohmann::json::parse(input, nullptr, false);
    if (jsonValue.is_discarded()) {
        PRINT_ERROR("Parse from %s to response failed.\n", input.c_str());
        return Response(PARSE_RESPONSE_FAILED);
    }

    bool hasError = jsonValue.contains(BUSINESS_RESPONSE_ERROR_NO);
    bool hasErrorMsg = jsonValue.contains(BUSINESS_RESPONSE_ERROR_MSG);
    bool hasData = jsonValue.contains(BUSINESS_RESPONSE_DATA) && !jsonValue[BUSINESS_RESPONSE_DATA].is_null();

    if (!hasError || !hasErrorMsg) {
        return Response(PARSE_RESPONSE_FAILED);
    }
    if (!jsonValue[BUSINESS_RESPONSE_ERROR_NO].is_number_integer()) {
        return Response(PARSE_RESPONSE_FAILED);
    }
    if (!jsonValue[BUSINESS_RESPONSE_ERROR_MSG].is_string()) {
        return Response(PARSE_RESPONSE_FAILED);
    }

    auto error = static_cast<ERROR_CODE>(jsonValue[BUSINESS_RESPONSE_ERROR_NO].get<int>());
    auto errorMsg = jsonValue[BUSINESS_RESPONSE_ERROR_MSG].get<std::string>();
    return hasData ? Response(error, errorMsg, jsonValue[BUSINESS_RESPONSE_DATA]) : Response(error, errorMsg);
}

/**
 * 从data中找到key的值
 * @param key
 * @return
 */
std::string Response::getString(const std::string& key) {
    if (!data.contains(key)) {
        PRINT_ERROR("Cannot find %s in response: %s.\n", key.c_str(), this->toString().c_str());
        return std::string();
    }
    return data[key].get<std::string>();
}

int Response::getInt(const std::string& key) {
    if (!data.contains(key)) {
        PRINT_ERROR("Cannot find %s in response: %s.\n", key.c_str(), this->toString().c_str());
        return 0;
    }
    return data[key].get<int>();
}
