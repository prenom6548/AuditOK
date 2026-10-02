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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/imagegenerationengine.h>
#include <kylin-ai/ai-engine/imageprocessengine.h>

#include "assistant/session.h"

namespace assistant::message {

bool isSourceUrlToolCall(const Json::Value &jsonMessage);

bool isStreamChatResponse(const Json::Value &json);

int toolCallIdFromMessage(const Json::Value &json);

std::string createErrorMessageResult(int errorCode, const std::string &errorMessage);

struct ImageData {
    bool success{false};
    std::string modelName;
    std::string imageBase64Data;
    std::string imageFormat;
    int errorCode{};
    std::string errorMessage;
};

std::string createImageDataMessage(const ImageData &imageData, bool isGenerating, const std::string &feedbackUrl = {});

std::string createImageDataMessageWithErrorCode(const std::string &modelName, const std::string &messageType, int error,
                                                const std::string &errorMessage);

std::string createTextMessageFromStreamChatResult(const Json::Value &jsonResult, const std::string &modelName,
                                                  const std::string &feedbackUrl);

session::InformationSource getInformationSourceFromUserGuidePath(const std::string &path);
std::string getAppNameFromSourceId(int id);

Json::Value createJsonInformationSources(const std::set<session::InformationSource> &sources);

Json::Value createToolInformationSources(const std::set<DialogueCallback::KBChatInfo> &sources);

std::string createChatEndMessageResult();

std::string createChatMessage(const std::string &result);

};  // namespace assistant::message

#endif
