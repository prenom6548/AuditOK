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

#ifndef SERVICES_GEN_AI_NLP_SERVICE_H
#define SERVICES_GEN_AI_NLP_SERVICE_H

#include <gio/gio.h>
#include <kylin-ai/aiengine.h>

#include <map>
#include <memory>
#include <set>

#include "chatbot.h"
#include "engineselector/engineselector.h"
#include "genainlpmodelconfig.h"
#include "gennlpserviceglue.h"

class GenAiNlpService : public std::enable_shared_from_this<GenAiNlpService> {
public:
    GenAiNlpService(GDBusConnection& connection, EngineSelector& engineSelector);

    ~GenAiNlpService();

    void stop();

private:
    void exportSkeleton();
    void unexportSkeleton();
    void connectSignal();
    std::string getProtocolVersion();
    void setRuntimeProtocolVersion(const std::string& version);
    int generateRandomSessionId();
    void releaseRandomSessionId(int sessionId);
    void onChatResult(const Chatbot::ChatResult& chatResult, int sessionId);
    void onToolHandlerResult(const Chatbot::ToolHandlerResult& toolHandlerResult, int sessionId);
    std::shared_ptr<GenAiNlpService> getSharedObject();

private:
    bool isSessionIdValid(int sessionId);

    static bool onHandleInit(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation, const gchar* config,
                             gpointer userData);

    static bool onHandleDestroy(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation, int sessionId,
                                gpointer userData);

    static bool onHandleChat(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation, gint sessionId,
                             gchar* question, gpointer userData);

    static bool onHandleGenerateContent(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                        gint sessionId, gchar* prompt, gpointer userData);

    static bool onHandleClearChatMessage(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                         gint sessionId, gpointer userData);

    static bool onHandleStopChat(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation, gint sessionId,
                                 gpointer userData);
    static bool onHandleStopChatWithRequestId(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                              gint sessionId, const gchar* requestId, gpointer userData);

    static bool onHandleAddSystemMessage(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                         gint sessionId, const gchar* message, gpointer userData);

    static bool onHandleSetChatSystemPromptId(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                              gint sessionId, gint promptId, gpointer userData);

    static bool onHandleChatWithHistoryMessages(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                                gint sessionId, const gchar* messages, gpointer userData);

    static bool onHandleGetNlpModelList(AiRuntimeGenAiNlpService* delegate, GDBusMethodInvocation* invocation,
                                        gint sessionId, gpointer userData);

    static bool onHandleRegisterTool(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                     gint sessionId, const gchar* toolName, gint toolType, const gchar* jsonSchema,
                                     gpointer userData);

    static bool onHandleClearTools(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation, gint sessionId,
                                   gpointer userData);

    static bool onHandleSubmitToolResult(AiRuntimeGenAiNlpService* service, GDBusMethodInvocation* invocation,
                                         gint sessionId, const gchar* request, const gchar* resultJson,
                                         gpointer userData);

private:
    bool isExported_ = false;
    AiRuntimeGenAiNlpService* delegate_{nullptr};
    GDBusConnection& connection_;
    EngineSelector& engineSelector_;
    std::set<int> sessionIdSet_;
    std::map<int, std::shared_ptr<Chatbot> > chatbots_;
    std::vector<gulong> handlerIds_;
};

#endif  // SERVICES_GEN_AI_NLP_SERVICE_H
