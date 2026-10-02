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

std::string xunfei_vision_token::getAuthenticationUrl(const std::string &apiKey, const std::string &secretKey,
                                                      const std::string &hostUrl) {
    auto urlData = xunfei_vision_util::parseURL(hostUrl);
    std::string date = xunfei_vision_util::getGmtTime();
    std::string originSignature{"host: " + urlData["Host"] + "\n" + "date: " + date + "\n" + "POST " + urlData["Path"] +
                                " HTTP/1.1"};
    std::string shaSignature = xunfei_vision_util::hmacSha256Encode(originSignature, secretKey);
    std::string authorization = "api_key=\"" + apiKey +
                                "\", algorithm=\"hmac-sha256\", headers=\"host "
                                "date request-line\", signature=\"" +
                                shaSignature + "\"";
    authorization = xunfei_vision_util::base64Encode(authorization);
    std::string callurl = hostUrl + "?" + "authorization=" + xunfei_vision_util::urlEncode(authorization) +
                          "&date=" + xunfei_vision_util::urlEncode(date) + "&host=" + urlData["Host"];
    return callurl;
}