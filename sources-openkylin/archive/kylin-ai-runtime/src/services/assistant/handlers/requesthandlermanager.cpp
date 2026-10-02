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

#include "requesthandlermanager.h"

#include <jsoncpp/json/json.h>

#include "assistant/error.h"
#include "assistant/message.h"
#include "chat/chatrequesthandler.h"
#include "utils/logger.h"
#include "utils/util.h"

RequestHandlerManager::RequestHandlerManager() { init(); }

RequestHandlerManager::~RequestHandlerManager() { LOGI("RequestHandlerManager::~RequestHandlerManager"); }

assistant::ErrorCode RequestHandlerManager::handleRequest(GDBusConnection *connection, const std::string &message,
                                                          assistant::session::DialogueSession &session) {
    const Json::Value jsonMessage = util::stringToJson(message);
    if (!jsonMessage.isObject()) {
        LOGE("Assistant::onHandleChat: Parse message failed.{}", message);
        handleRequestWithInvalidParam(connection, session);
        return assistant::ErrorCode::ParamError;
    }

    return chatRequestHandler_->handleRequest(connection, jsonMessage, session);
}

void RequestHandlerManager::init() { chatRequestHandler_ = std::make_shared<ChatRequestHandler>(); }

void RequestHandlerManager::handleRequestWithInvalidParam(GDBusConnection *connection,
                                                          assistant::session::DialogueSession &session) {
    const std::string errorMessage = assistant::message::createErrorMessageResult(int(assistant::ErrorCode::ParamError),
                                                                                  "Parse json message failed.");
    RequestHandler::sendChatResult(connection, session.sessionId, errorMessage);
}
