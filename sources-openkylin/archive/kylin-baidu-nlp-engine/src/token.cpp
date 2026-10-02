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
#include "cpr/api.h"
#include "util.h"

std::string baidu_nlp_token::getBaiduToken(const std::string &apiKey, const std::string &secretKey, int &errorCode,
                                           std::string &errorMessage) {
    const std::string tokenUrl{std::string("https://aip.baidubce.com/oauth/2.0/token?client_id=" + apiKey +
                                           "&client_secret=" + secretKey + "&grant_type=client_credentials")};
    cpr::Url url(tokenUrl);
    cpr::Response response =
        cpr::Post(url, cpr::Header{{"Content-Type", "application/json"}, {"Accept", "application/json"}},
                  cpr::ConnectTimeout{10000}, cpr::LowSpeed{20, 10});

    if (response.error.code != cpr::ErrorCode::OK) {
        errorCode = int(response.error.code);
        errorMessage = response.error.message;
        return {};
    }

    Json::Value jsonObj = baidu_nlp_util::formatJsonFromString(response.text);
    if (!jsonObj.isMember("access_token")) {
        errorMessage = response.text;
        return {};
    }

    return jsonObj["access_token"].asString();
}

bool baidu_nlp_token::isBaiduTokenExpiredByResult(const Json::Value &result) {
    if (!result.isObject() || result.isNull()) {
        return false;
    }
    if (result.isMember("error_code") && result.isMember("error_msg")) {
        return result["error_code"].asInt() == 111;
    }
    if (result.isMember("err_no") && result.isMember("err_msg")) {
        return result["err_no"].asInt() == 111;
    }
    return false;
}