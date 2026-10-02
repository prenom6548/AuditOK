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

#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineresult.h>

namespace baidu_speech_token {

std::string getBaiduSpeechToken(const std::string &apiKey, const std::string &secretKey, int &errorCode,
                                std::string &errorMessage);

bool isBaiduTokenExpiredByResult(const Json::Value &result);

/**
 * 重构版获取token接口函数
 * @param apiKey        百度API Key
 * @param secretKey     百度Secret Key
 * @param token         获取到的token
 */
ai_engine::result::AiEngineBaseResult getToken(const std::string &apiKey, const std::string &secretKey,
                                               std::string &token);

} // namespace baidu_speech_token

#endif // TOKEN_H
