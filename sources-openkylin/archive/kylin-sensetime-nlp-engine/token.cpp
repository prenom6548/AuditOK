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
#include <iostream>

#include <ctime>
#include <jwt.h>
#include "servererror.h"
// #include "util.h"

std::string sensetime_nlp_token::getSensetimeToken(const std::string &apiKey,
                                                   const std::string &secretKey,
                                                   std::string &errorMessage) {
    if (apiKey.empty() || secretKey.empty()) {
        errorMessage = "apiKey or secretKey is empty";
        return std::string();
    }

    try {
        time_t now_time_t = time(nullptr);
        auto now_jwt_time = static_cast<uint64_t>(now_time_t);

        jwt_t *jwt = nullptr;
        jwt_new(&jwt);

        jwt_add_header(jwt, "typ", "JWT");

        jwt_add_grant(jwt, "iss", apiKey.c_str());
        jwt_add_grant_int(jwt, "exp",
                          static_cast<int64_t>(now_jwt_time + 1800));
        jwt_add_grant_int(jwt, "nbf", static_cast<int64_t>(now_jwt_time - 5));

        jwt_set_alg(jwt, JWT_ALG_HS256,
                    reinterpret_cast<const unsigned char *>(secretKey.c_str()),
                    secretKey.size());

        char *token = jwt_encode_str(jwt);

        errorMessage = "";

        jwt_free(jwt);

        return std::string(token);

    } catch (const std::exception &e) {
        errorMessage = e.what();
        return std::string();
    }
}

bool sensetime_nlp_token::isSensetimeTokenForbidden(const std::string &result) {
    return sensetime_nlp_server_error::parseErrorCode(result) == 16;
}