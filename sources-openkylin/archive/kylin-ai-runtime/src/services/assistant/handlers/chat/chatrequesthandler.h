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

#ifndef CHATREQUESTHANDLER_H
#define CHATREQUESTHANDLER_H

#include "assistant/handlers/requesthandler.h"

// TODO ChatRequestHandler, RequestHandler, RequestHandlerManager可以合并
class ChatRequestHandler : public RequestHandler {
public:
    ChatRequestHandler() = default;
    ~ChatRequestHandler() override;

    assistant::ErrorCode handleRequest(GDBusConnection *connection, const Json::Value &jsonMessage,
                                       assistant::session::DialogueSession &session) override;

private:
    std::future<void> future_;
};

#endif
