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

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <gio/gio.h>
#include <jsoncpp/json/value.h>

#include <memory>

#include "assistant/error.h"
#include "assistant/session.h"

class RequestHandler {
public:
    virtual ~RequestHandler() {}

    virtual assistant::ErrorCode handleRequest(GDBusConnection *connection, const Json::Value &jsonMessage,
                                               assistant::session::DialogueSession &session) = 0;

    void setNextHandler(std::shared_ptr<RequestHandler> nextHandler) { nextHandler_ = nextHandler; }

    static void sendChatResult(GDBusConnection *connection, int sessionId, const std::string &result);

protected:
    std::shared_ptr<RequestHandler> nextHandler_;
};

#endif  // REQUESTHANDLER_H
