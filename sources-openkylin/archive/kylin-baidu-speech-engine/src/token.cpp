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

#include "token.h"
#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineresult.h>
#include <kylin-ai/common/error.h>

#include "HttpClient.h"
#include "cpr/api.h"
#include "logger.h"
#include "util.h"

std::string baidu_speech_token::getBaiduSpeechToken(const std::string &apiKey, const std::string &secretKey,
                                                    int &errorCode, std::string &errorMessage) {
    const std::string tokenUrl{std::string("https://openapi.baidu.com/oauth/2.0/token?client_id=" + apiKey +
                                           "&client_secret=" + secretKey + "&grant_type=client_credentials")};
    cpr::Url url(tokenUrl);
    cpr::Response response =
        cpr::Post(url, cpr::Header{{"Content-Type", "application/json"}, {"Accept", "application/json"}});

    if (response.error.code != cpr::ErrorCode::OK) {
        errorCode = int(response.error.code);
        errorMessage = response.error.message;
        return {};
    }

    Json::Value jsonObj = baidu_speech_util::formatJsonFromString(response.text);
    if (!jsonObj.isMember("access_token")) {
        errorMessage = response.text;
        return {};
    }

    return jsonObj["access_token"].asString();
}

bool baidu_speech_token::isBaiduTokenExpiredByResult(const Json::Value &result) {
    if (!result.isObject() || result.isNull()) {
        return false;
    }
    if (result.isMember("error_code") && result.isMember("error_msg")) {
        return result["error_code"].asInt() == 111;
    }
    if (result.isMember("err_no") && result.isMember("err_msg")) {
        return result["err_no"].asInt() == 502;
    }
    return false;
}

ai_engine::result::AiEngineBaseResult baidu_speech_token::getToken(const std::string &apiKey,
                                                                   const std::string &secretKey, std::string &token) {

    std::string url = "https://openapi.baidu.com/oauth/2.0/token?";
    url += "client_id=" + apiKey;
    url += "&client_secret=" + secretKey;
    url += "&grant_type=client_credentials";

    HttpRequest request;
    request.method = HTTP_POST;
    request.url = url;
    request.headers["Content-Type"] = "application/json";
    request.headers["Accept"] = "application/json";

    HttpResponse response;

    hv::HttpClient client;
    int ret = client.send(&request, &response);
    if (ret != 0) {
        LOGE("HTTP request error.");
        return {false, {AI_COMMON_NET_ERROR, "HTTP request error."}};
    }

    LOGI("Http status:", response.status_code, response.status_message());
    LOGI("Http body  :", response.body);
    int statusCode = response.status_code;
    std::string statusMessage = response.status_message();

    if (statusCode == 200) {
        Json::Value value = baidu_speech_util::stringToJson(response.body);
        if ((!value.isMember("access_token")) || (!value["access_token"].isString())) {
            return {false, {AI_COMMON_NET_ERROR, "The format of the response message is incorrect."}};
        }
        token = value["access_token"].asString();
        return {true, {}};
    }
    if (statusCode == 401) {
        return {false, {AI_COMMON_AUTHENTICATION_FAILURE, statusMessage}};
    }
    if (statusCode == 411) {
        // TODO: libhv在未认证的时候服务端返回411错误，后续需要继续调查
        return {false, {AI_COMMON_SSL_CERTIFICATE_ERROR, statusMessage}};
    }
    return {false, {AI_COMMON_NET_ERROR, "Unknown network error " + std::to_string(statusCode) + " ."}};
}
