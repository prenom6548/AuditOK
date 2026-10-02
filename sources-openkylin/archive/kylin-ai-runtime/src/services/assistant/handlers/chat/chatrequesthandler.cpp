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

#include "chatrequesthandler.h"

#include "assistant/error.h"
#include "assistant/message.h"
#include "modelconfig/modelstatustracker.h"
#include "threadpool/async.h"
#include "utils/logger.h"
#include "utils/util.h"

ChatRequestHandler::~ChatRequestHandler() {
    if (future_.valid()) {
        future_.wait();
    }
}

assistant::ErrorCode ChatRequestHandler::handleRequest(GDBusConnection* connection, const Json::Value& jsonMessage,
                                                       assistant::session::DialogueSession& session) {
    future_ = cpr::async([jsonMessage, &session] {
        std::string userInput = util::jsonToString(jsonMessage);
        session.intentionRecognition.dialogueControlMain(session.intentionRecognitionFirstRound, userInput);
        if (session.intentionRecognitionFirstRound) {
            session.intentionRecognitionFirstRound = false;
        }
    });

    return assistant::ErrorCode::Ok;
}
